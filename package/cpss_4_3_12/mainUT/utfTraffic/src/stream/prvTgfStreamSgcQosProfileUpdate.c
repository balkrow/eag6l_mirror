/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfStreamSgcQosProfileUpdate.c
*
* @brief SGC gate QoS profile update use case testing.
*
*        Features that are tested:
*
*        QoS profile update:
*          - If the Internal Priority Value (IPV) returned by Gate Control List (GCL) is not NULL IPV
*            the LS 3 bits of Desc<QosProfile> are overwritten by the IPV value (0-7)
*          - The QosProfile table in EQ will assign a TC according to the LS 3 bits of the QoSProfile
*
*        Gate statistics:
*           - The legacy Pre-egress CNC "Pass/Drop" Client is used to count the packet Gate-ID statistics.
*             In Ironman, a new mode was added to the EQ Global<Pass Drop CNC Mode>: "Stream Gate Mode".
*             When configured in "Stream Gate Mode", the CNC index format is as follows:
*             - |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
*           - Read gate Octet counter
*
*        1 test is added to check above features:
*        - prvTgfStreamSgcQosProfileUpdate
*
*        Ports allocation:
*         - Port#0: Ingress port
*         - Port#4: Target port
*
*        Packet flow:
*         1.  Ethernet frame packet enters the device via Ingress Port#0
*         2.  The packet is sent towards target port#4
*
*        Test description:
*        - Set IPCL actions with 802.1Qci parameters (assign specific gate id)
*        - Configure the specific gate.
*          - Set Interval max values to be large enough to pass 2 packets
*          - Set gate state to open for all time slots
*          - Set IPV value to NULL so TC won't be changed by GCL
*        - Send 2 packets with vlan tag:
*           - Send first packet
*           - configure gate again this time
*               - set IPV to valid value
*               - expect Desc<QosProfile> to be changed by GCL causing packet TC to change
*           - Send second packet
*        - Check results
*          - read packets content
*          - read port counters
*          - read CNC counters
*          - read gate Octet counter
*
* @version   1
********************************************************************************
*/
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStreamGateControl.h>
#include <stream/prvTgfStreamSgcQosProfileUpdate.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCncGen.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamGateControl.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>


/*************************** Constants definitions ***********************************/

/* Maximum valid Gate ID value */
#define PRV_TGF_SGC_MAX_GATE_ID_NUM_CNS       510

/* Number of Table Sets */
#define PRV_TGF_SGC_MAX_TABLE_SETS_NUM_CNS    58

/* Used Qos profile 3 */
#define PRV_TGF_SGC_TEST_QOS_PROFILE3         3

/* Used Qos profile 7 */
#define PRV_TGF_SGC_TEST_QOS_PROFILE7         7

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS         2

/* Ports index allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS    0
#define PRV_TGF_TARGET_PORT_IDX_CNS     1

/* VLAN Id #6 */
#define PRV_TGF_VLANID_6_CNS            6


/* Source ID parameters */
#define SOURCE_ID_MASK_CNS              0xFFF
#define SOURCE_ID_CNS                   0x800

/* Size of maximum output packets in bytes include CRC */
#define PRV_TGF_MAX_PACKET_SIZE_CNS     72


/*************************** static DB definitions ***********************************/

/* Input packet information */
static TGF_PACKET_STC   packetInfo;

/* Target port interface information */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Actual port counters values per packet */
static GT_U32    actualPacketsNum[PRV_TGF_BURST_COUNT_CNS];
static GT_U32    actualBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* Expected bytes number per packet */
static GT_U32    expectedBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* PCL rule id */
static GT_U32    ruleId = 1;

/* Drop code to be used in test in case of Gate Interval Max Octet Exceeded exception */
static CPSS_NET_RX_CPU_CODE_ENT    dropCodeGateExceeded = CPSS_NET_USER_DEFINED_5_E;

/* Drop code to be used in test in case of Gate Closed exception */
static CPSS_NET_RX_CPU_CODE_ENT    dropCodeGateClosed = CPSS_NET_USER_DEFINED_6_E;

/* Gate id */
static GT_U32    gateId = 0;

/* Table Set id */
static GT_U32    tableSetId = 0;

/* CNC block 0 that is used in test to count cnc indexes from 0 - 1023 */
static GT_U32    blockNum0 = 0;

/* CNC block 1 that is used in test to count cnc indexes from 1024 - 2047 */
static GT_U32    blockNum1 = 1;

/* Expected value of VLAN tag
   vlan<up> is changed during test
 */
static GT_U32    expectedVlan = (TGF_ETHERTYPE_8100_VLAN_TAG_CNS << 16) |
                                (1 << 13) /*UP*/                        |
                                (0 << 12) /*CFI*/                       |
                                (PRV_TGF_VLANID_6_CNS);

/*
   Packet byte count adjust parameters:
   -  byteCountAdjustPolarity: GT_FALSE => - byteCountAdjust
   -  byteCountAdjustPolarity: GT_TRUE => + byteCountAdjust

   Octet counter = Desc<bc> +/- byteCountAdjust
*/
static GT_U32    byteCountAdjust;
static GT_BOOL   byteCountAdjustPolarity;

/* Number of time slots to use in the test */
static GT_U32    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

/* Number of interval max profiles to use in the test */
static GT_U32    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

/* QoS profiles numbers configured in this test
   Set when QoS profile is configured
   Used in verification function
 */
static GT_U32    profileIndexArr[PRV_TGF_BURST_COUNT_CNS] = {0,0};


/* Ingress Packet: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}   /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,    /* etherType        */
    1, 0, PRV_TGF_VLANID_6_CNS          /* pri, cfi, VlanId */
};

/* Short DATA of packet */
static GT_U8 prvTgfShortPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* Short PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfShortPacketPayloadPart = {
    sizeof(prvTgfShortPayloadDataArr),    /* dataLength */
    prvTgfShortPayloadDataArr             /* dataPtr */
};

/* Short PARTS of packet */
static TGF_PACKET_PART_STC prvTgfShortPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfShortPacketPayloadPart}
};

/* Parameters that are needed to be restored */
static struct
{
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT               genericActionMode;
    GT_BOOL                                             dropEnable;
    CPSS_DXCH_CFG_PROBE_PACKET_STC                      probeCfg;
    GT_U32                                              srcIdMask;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC                  gateParams;
    CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT passDropMode;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC              globalParams;
    CPSS_QOS_PORT_TRUST_MODE_ENT                        portQosTrustMode;
    CPSS_QOS_ENTRY_STC                                  qosEntry;
} prvTgfRestoreCfg;


/******************************************************************************\
 *   Configurations functions                                                 *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcGateIdAndTableSetIdSelect function
* @endinternal
*
* @brief   Select random Gate ID and TableSet ID
*          - Gate ID range: 0..510
*          - TableSet ID range: 0..57
*/
static GT_VOID prvTgfStreamSgcGateIdAndTableSetIdSelect
(
    GT_VOID
)
{
    /* Generate random value for Gate ID in range 0:510 */
    gateId = cpssOsRand() % PRV_TGF_SGC_MAX_GATE_ID_NUM_CNS;

    /* Generate random value for TableSet ID in range 0:57 */
    tableSetId = cpssOsRand() % (PRV_TGF_SGC_MAX_TABLE_SETS_NUM_CNS-1);

    PRV_UTF_LOG2_MAC("Gate & TableSet IDs that are used in this test: gateId:[%d], tableSetId:[%d] \n"
                     ,gateId,tableSetId);
}

/**
* @internal prvTgfStreamSgcBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*          - Packet egress with Tag0 as defined in ingress pipe for all ports
*          - Forward to ePort#1 according to MAC destination
*/
static GT_VOID prvTgfStreamSgcBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS    rc;

    /* AUTODOC: Save default VLAN 1 */
    prvTgfBrgVlanEntryStore(1);

    /* AUTODOC: Packet egress with Tag0 as defined in ingress pipe for all ports */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_6_CNS,
                                                  PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    /* AUTODOC: Add FDB entry with MAC 00:00:00:00:34:02, VLAN ID 1, target port.
                Forward the packet to target port according to MAC DA address.
     */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_VLANID_6_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
}

/**
* @internal prvTgfStreamSgcCosConfigSet function
* @endinternal
*
* @brief   QoS test configurations
*          - Configures 2 QoS profiles
*             - profile#3 with priority 3
*             - profile#7 with priority 7
*          - Configures the port's QoS attributes
*             - to use profile#3
*             - allow modification to UP field
*          - Configures port to no trust mode
*/
static GT_VOID prvTgfStreamSgcCosConfigSet
(
    GT_VOID
)
{
    GT_STATUS                  rc = GT_OK;
    CPSS_DXCH_COS_PROFILE_STC  qosProfile;
    CPSS_QOS_ENTRY_STC         qosEntry;

    /**** Configure QoS profiles ****/

    /* AUTODOC: Clear qosProfile structure */
    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

    /* AUTODOC: Configure first profile */
    /* AUTODOC: Set profile number 3 */
    profileIndexArr[0]      = PRV_TGF_SGC_TEST_QOS_PROFILE3;
    /* AUTODOC: Set priority (any number from 0 to 7) for this profile */
    qosProfile.userPriority = 3;
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, profileIndexArr[0], &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");

    /* AUTODOC: Configure second profile */
    /* AUTODOC: Set profile number 7 */
    profileIndexArr[1]      = PRV_TGF_SGC_TEST_QOS_PROFILE7;
    /* AUTODOC: Set priority (any number from 0 to 7) for this profile */
    qosProfile.userPriority = 7;
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, profileIndexArr[1], &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");


    /**** Configures the port's QoS attributes ****/

    /* Save ingress port Qos attributes for restore */
    rc = cpssDxChCosPortQosConfigGet(prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                     &prvTgfRestoreCfg.qosEntry);

    /* AUTODOC: Clear qosEntry structure */
    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));

    /* AUTODOC: Packet's attribute assignment can be overridden by subsequent assignment mechanism */
    qosEntry.assignPrecedence  = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    /* AUTODOC: Enable modification of UP field */
    qosEntry.enableModifyUp    = CPSS_PACKET_ATTRIBUTE_MODIFY_ENABLE_E;
    /* AUTODOC: Disable modification of DSCP field */
    qosEntry.enableModifyDscp  = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    /* AUTODOC: Apply the above for profile 3 */
    qosEntry.qosProfileId      = PRV_TGF_SGC_TEST_QOS_PROFILE3;
    rc = cpssDxChCosPortQosConfigSet(prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                     &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosPortQosConfigSet");


    /**** Configures port's Trust Mode ****/

    /* AUTODOC: Set to no trust.
                Packet QosProfile is assigned as the port's default PortQosProfile.
     */
    rc = cpssDxChCosPortQosTrustModeGet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                        &prvTgfRestoreCfg.portQosTrustMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosPortQosTrustModeGet");


    rc = cpssDxChCosPortQosTrustModeSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                        CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosPortQosTrustModeSet");
}

/**
* @internal prvTgfStreamSgcPclConfigSet function
* @endinternal
*
* @brief   PCL test configurations
*          - Set IPCL rule: according to MAC DA
*          - Set IPCL actions with 802.1Qci parameters
*            - Set redirect command to work with Generic action of 802.1Qci
*            - Set 12 bits generic action data with | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) |
*/
static GT_VOID prvTgfStreamSgcPclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT              mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                   action;
    GT_BIT                                   cncIndxMode;
    GT_U32                                   pclQciActions;
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT    genericActionMode;
    GT_U32                                   maxSduSizeProfileIndx;

    /* AUTODOC: Init PCL Engine for ingress port */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_NUMBER_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit");

    /* AUTODOC: Set PCL rule: according to MAC DA */

    /* AUTODOC: Mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFFFF, sizeof(mask.ruleStdNotIp.macDa.arEther));

    /* AUTODOC: Pattern for MAC DA address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: Clear action structure */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: Set PCL actions */

    /* AUTODOC: Set packet command to forward */
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    /* AUTODOC: Set 802.1Qci related actions:
       - set redirectCmd to Generic action
       - set Generic action with 802.1Qci parameters:
         - cncIndxMode,
         - maxSduSizeProfileIndx,
         - gateId
     */
    /* AUTODOC: Set IPCL action redirect command to generic action type */
    action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_GENERIC_ACTION_E;

    /* AUTODOC: Set 802.1QCi IPCL action parameters */
    cncIndxMode           = 0;
    maxSduSizeProfileIndx = 0;
    /* AUTODOC: Set generic action data with 12 bits of | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) | */
    pclQciActions = (cncIndxMode << 11) | (maxSduSizeProfileIndx << 9) | gateId ;
    action.redirect.data.genericActionInfo.genericAction = pclQciActions ;

    /* AUTODOC: Set sourceId to mark probe packets for reading the drop counter in case of exception */
    action.sourceId.assignSourceId = GT_TRUE;
    action.sourceId.sourceIdValue = SOURCE_ID_CNS;

    /* AUTODOC: Set PCL rule */
    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                          ruleId,
                          &mask,
                          &pattern,
                          &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStreamGateControlPclConfigSet");

    /* AUTODOC: Save mask sourceId for restore */
    rc = prvTgfPclSourceIdMaskGet(prvTgfDevNum,CPSS_PCL_LOOKUP_0_E, &prvTgfRestoreCfg.srcIdMask);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d", prvTgfDevNum);

    /* AUTODOC: Mask sourceId */
    rc = prvTgfPclSourceIdMaskSet(CPSS_PCL_LOOKUP_0_E, SOURCE_ID_MASK_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclSourceIdMaskSet: %d", prvTgfDevNum);

    /* AUTODOC: Save generic action mode for restore */
    rc = cpssDxChPclGenericActionModeGet(
        prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E, 0, &prvTgfRestoreCfg.genericActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeGet");

    /* AUTODOC: Set generic action to 802.1Qci mode */
    genericActionMode = CPSS_DXCH_PCL_GENERIC_ACTION_MODE_802_1_QCI_E;
    rc = cpssDxChPclGenericActionModeSet(
        prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E, 0, genericActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeSet");
}

/**
* @internal prvTgfStreamSgcIngressDropCounterConfigSet function
* @endinternal
*
* @brief   Ingress drop counter configurations
*/
static GT_VOID prvTgfStreamSgcIngressDropCounterConfigSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    CPSS_DXCH_CFG_PROBE_PACKET_STC   probeCfg;

    /* AUTODOC: Reset the ingress drop counter */
    rc = prvTgfCfgIngressDropCntrSet(0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrSet");

    /* AUTODOC: Save drop state for restore */
    rc = prvTgfCfgIngressDropEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.dropEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropEnableGet");

    /* AUTODOC: Enable packet drop in EQ */
    rc = cpssDxChCfgIngressDropEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");

    /* AUTODOC: Save probe configuration for restore */
    rc = cpssDxChCfgProbePacketConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.probeCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgProbePacketConfigGet: %d");

    /* AUTODOC: Set the probe packet configuration to get drop code */
    probeCfg.bitLocation = 11;
    probeCfg.enable      = GT_TRUE;
    rc = cpssDxChCfgProbePacketConfigSet(prvTgfDevNum, &probeCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgProbePacketConfigSet");
}

/**
* @internal prvTgfStreamSgcGateConfigSet function
* @endinternal
*
* @brief   Gate configurations
*/
static GT_VOID prvTgfStreamSgcGateConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                 rc = GT_OK;
    GT_U32                                    i;
    GT_U32                                    cycleTime;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC        gateParams;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC    globalParams;

    /**** Set global parameters ****/

    /* AUTODOC: Save SGC global parameters for restore */
    rc = cpssDxChStreamSgcGlobalConfigGet(prvTgfDevNum, &prvTgfRestoreCfg.globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigGet");

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&globalParams, 0, sizeof(globalParams));

    /*
       AUTODOC: Set global parameters:
                - enable SGC,
                - set exceptions parameters
                - Use timestamp
                - Set byte count mode to L2 mode
     */
    globalParams.sgcGlobalEnable                           = GT_TRUE;
    globalParams.intervalMaxOctetExceededException.command = CPSS_PACKET_CMD_DROP_HARD_E;
    globalParams.intervalMaxOctetExceededException.cpuCode = dropCodeGateExceeded;
    globalParams.gateClosedException.command               = CPSS_PACKET_CMD_DROP_HARD_E;
    globalParams.gateClosedException.cpuCode               = dropCodeGateClosed;
    globalParams.sgcIgnoreTimestampUseTod                  = GT_FALSE;
    globalParams.sgcCountingModeL3                         = GT_FALSE;
    rc = cpssDxChStreamSgcGlobalConfigSet(prvTgfDevNum, &globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigSet");

    /**** Set gate parameters ****/

    /* AUTODOC: Save gate parameters for restore */
    rc = cpssDxChStreamSgcGateConfigGet(prvTgfDevNum,
                                        gateId,
                                        tableSetId,
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &prvTgfRestoreCfg.gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigGet");

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));

    /*
       AUTODOC: Set Interval max and Time slot parameters
                - Set Interval Max value to 500 bytes for all time slots
                - Set each time slot with time to advance value of +1000 nsec
                - Set each time slot with the profile index to Interval Max Table
                - Do not change packet's TC
                - No length aware
                - Open the gate for all time slots
                - Do not reset the Octet counter in time slot transition
     */
    for (i=0;i<timeSlotsNum;i++)
    {
        gateParams.intervalMaxArr[i]                          = 500;  /*big enough to pass 2 packets*/
        gateParams.timeSlotInfoArr[i].timeToAdvance          += 1000;
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_TRUE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x4;
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_FALSE;
        gateParams.timeSlotInfoArr[i].streamGateState         = CPSS_DXCH_STREAM_SGC_GATE_STATE_OPEN_E;
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_FALSE;
    }

    /* AUTODOC: Set Table Set time configuration parameters. */
    byteCountAdjustPolarity = GT_FALSE;
    byteCountAdjust         = 0;
    gateParams.tableSetInfo.byteCountAdjustPolarity = byteCountAdjustPolarity;
    gateParams.tableSetInfo.byteCountAdjust         = byteCountAdjust;
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.remainingBitsFactor     = 255;        /* 255 bits every 128nsec */
    cycleTime = 1000 * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    gateParams.tableSetInfo.cycleTime = cycleTime;                /* 1000nsec * 256 time slots */
    gateParams.tableSetInfo.ingressTimestampOffset  = 0;
    gateParams.tableSetInfo.ingressTodOffset        = 0;

    /* AUTODOC: Call SGC API to set these parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateId,
                                        tableSetId,
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");
}

/**
* @internal prvTgfStreamSgcStatisticsConfigSet function
* @endinternal
*
* @brief  Set packet gate id statistics
*         Use CNC counter with pre-egress "Pass/Drop" client configured to Stream Gate mode
*/
static GT_VOID prvTgfStreamSgcStatisticsConfigSet
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;
    GT_U32                           blockSize;
    GT_U32                           gateState;
    PRV_TGF_CNC_COUNTER_STC          counterValue;

    /**** Set CNC counters  ****/

    /* AUTODOC: Save pass/drop cnc mode for restore */
    rc = cpssDxChCncPacketTypePassDropToCpuModeGet(prvTgfDevNum,
                                                   &prvTgfRestoreCfg.passDropMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeSet");

    /* AUTODOC: Set pass/drop cnc mode to Stream Gate mode */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,
                                                   CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_STREAM_GATE_MODE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeSet");

    /* AUTODOC: Get block maximum mumber of entries */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    /* AUTODOC: Clear CNC structure */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));

    /* AUTODOC: Set CNC client to pre-egress packet type pass/drop */
    cncCfg.clientType = PRV_TGF_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;

    /* AUTODOC: Set CNC block number 0 to be used for cnc indexes 0-1023 */
    cncCfg.blockNum = blockNum0;

    /* AUTODOC: Set CNC counter index in 802.1Qci format:
                |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
     */
    /* AUTODOC: Set gate state to open */
    gateState = 0x0;

    /* AUTODOC: modulo block size since index can not be larger than maximum block size  */
    cncCfg.counterNum = ( ((gateState << 9) | gateId) % blockSize);

    /* AUTODOC: Set CNC counter format to mode 0 */
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* AUTODOC: Enable CNC counter */
    cncCfg.configureCountingUnitEnable = GT_FALSE;
    cncCfg.countingUnitEnable          = GT_FALSE;
    cncCfg.configureClientEnable       = GT_TRUE;
    cncCfg.clientEnable                = GT_TRUE;

    /* AUTODOC: Set bit#0 for CNC counter index range of 0-1023 (for gate state open) */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       = BIT_0;

    /* AUTODOC: Set CNC counter value to '0' */
    cncCfg.configureCounterValue = GT_TRUE;

    /* AUTODOC: Set CNC counter byte count mode */
    cncCfg.configureByteCountMode = GT_TRUE;
    cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

    /* AUTODOC: Call CNC API to apply above parameters */
    rc = prvTgfCncGenConfigure(&cncCfg, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenConfigure");

    /* Clear this counter by reading it */
    rc = prvTgfCncCounterGet(cncCfg.blockNum,
                             cncCfg.counterNum,
                             cncCfg.counterFormat,
                             &counterValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    /* AUTODOC: Use another CNC block to count cnc indexes of 1024 to 2047
                for gate state Interval Max Exceeded
     */
    /* AUTODOC: Bind CNC pass/drop client to CNC block 1 to be used for Interval Max Exceeded use case */
    cncCfg.blockNum = blockNum1;

    /* AUTODOC: Set gate state to Interval Max Exceeded */
    gateState = 0x2;

    /* AUTODOC: modulo block size since index can not be larger than maximum block size  */
    cncCfg.counterNum = ( ((gateState << 9) | gateId) % blockSize);

    /* AUTODOC: Set bit#1 for CNC indexes range of 1024-2047 which is the case for Interval Max Exceeded */
    cncCfg.indexRangeBitMap[0] = BIT_1;

    /* AUTODOC: Call CNC API to apply above parameters */
    rc = prvTgfCncGenConfigure(&cncCfg, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenConfigure");

    /* Clear this counter by reading it */
    rc = prvTgfCncCounterGet(cncCfg.blockNum,
                             cncCfg.counterNum,
                             cncCfg.counterFormat,
                             &counterValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    /**** Set Ingress drop counters as well so we can get and verify drop code value ****/

    /* AUTODOC: Ingress Drop counter Configuration */
    prvTgfStreamSgcIngressDropCounterConfigSet();
}

/**
* internal prvTgfStreamSgcQosProfileUpdateConfigSet function
* @endinternal
*
* @brief   SGC gate QoS profile update use case configurations
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: Select random Gate ID and TableSet ID for this test */
    prvTgfStreamSgcGateIdAndTableSetIdSelect();

    /* AUTODOC: Bridge configuration */
    prvTgfStreamSgcBridgeConfigSet();

    /* AUTODOC: QoS configuration */
    prvTgfStreamSgcCosConfigSet();

    /* AUTODOC: PCL configuration */
    prvTgfStreamSgcPclConfigSet();

    /* AUTODOC: Gate configuration */
    prvTgfStreamSgcGateConfigSet();

    /* AUTODOC: Gate statistics configuration */
    prvTgfStreamSgcStatisticsConfigSet();
}


/******************************************************************************\
 *   Verification functions                                                   *
\******************************************************************************/


/**
* internal tgfStreamSgcPacketVerification function
* @endinternal
*
* @brief   Check content of packet
*         - Check MAC addresses of passenger
*         - Check vlan tag
*         - Check payload data of passenger
*/
static GT_VOID tgfStreamSgcPacketVerification
(
    IN  GT_U8  * packetBuf
)
{
    GT_U32              pktOffset,payloadPktOffset;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U32              recPassengerPayload, expPassengerPayload, recVlan;

    PRV_UTF_LOG0_MAC("======= Check content of output packet  =======\n");

    /**** Check MAC DA SA ****/
    /* AUTODOC: Set packet offset to point to packet MAC DA  */
    pktOffset =0;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfPacketL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,"Failure in tgfStreamGateControlPacketVerification: Mismatch in MAC DA address\n");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfPacketL2Part.daMac[0],prvTgfPacketL2Part.daMac[1],prvTgfPacketL2Part.daMac[2],
                          prvTgfPacketL2Part.daMac[3],prvTgfPacketL2Part.daMac[4],prvTgfPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }

    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr, "Failure in tgfStreamGateControlPacketVerification: Mismatch in MAC SA address\n");
    if (saMacErr != 0)
    {
        /* AUTODOC: Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfPacketL2Part.saMac[0],prvTgfPacketL2Part.saMac[1],prvTgfPacketL2Part.saMac[2],
                          prvTgfPacketL2Part.saMac[3],prvTgfPacketL2Part.saMac[4],prvTgfPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }


    /**** Check packet vlan ****/
    /* AUTODOC: Increment offset by 12B of MACs to point to vlan tag */
    pktOffset += TGF_L2_HEADER_SIZE_CNS;

    recVlan = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                      (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: Compare received vlan vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedVlan,recVlan,"Failure in tgfStreamGateControlPacketVerification: Mismatch in vlan tag \n");


    /**** Check Passenger payload ****/
    /* AUTODOC: Increment offset by 4B of VLAN tag to point to passenger payload (include EtherType) */
    pktOffset += TGF_VLAN_TAG_SIZE_CNS;

    /* AUTODOC: Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfShortPacketPayloadPart.dataLength;i+=4)
    {
        /* AUTODOC: Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* AUTODOC: Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* AUTODOC: Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfShortPayloadDataArr[i+3] | (prvTgfShortPayloadDataArr[i+2] << 8) |
                                           (prvTgfShortPayloadDataArr[i+1] << 16) | (prvTgfShortPayloadDataArr[i] << 24));

        /* AUTODOC: Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }

    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0,passengerPayloadErr,"Failure in tgfStreamGateControlPacketVerification: Mismatch in passenger payload data \n");
}

/**
* internal prvTgfStreamSgcOctetCounterCheck function
* @endinternal
*
* @brief  Read gate Octet counter value
*         Compare the value to the expected value
*/
static GT_VOID prvTgfStreamSgcOctetCounterCheck
(
    GT_U32    octetCounterExpected
)
{
    GT_STATUS    rc;
    GT_U32       octetCounterActual;

    /* AUTODOC: Get octet counter value for the gate that was used */
    rc = prvCpssStreamSgcOctetCountersGet(prvTgfDevNum, gateId, tableSetId, &octetCounterActual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(octetCounterExpected, octetCounterActual, "Octet counter (gate[%d], tableSet[%d]) is not as expected");
}

/**
* internal prvTgfStreamSgcCncCountersGet function
* @endinternal
*
* @brief  Read CNC counters
*/
static GT_VOID prvTgfStreamSgcCncCountersGet
(
    GT_U32 gateState,
    GT_U32 cncBlockNum,
    GT_U32 expectedPacketsNumCount,
    GT_U32 expectedBytesNumCount
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_CNC_CHECK_STC    cncCheck;
    GT_U32                   blockSize;

    /* AUTODOC: Get CNC block maximum mumber of entries */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    /* AUTODOC: Clear cnc structure */
    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));

    /* AUTODOC: Set CNC block 0 */
    cncCheck.blockNum            = cncBlockNum;
    cncCheck.counterFormat       = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    cncCheck.compareCounterValue = GT_TRUE;

    /* AUTODOC: Modulo block size since index can not be larger than maximum block size  */
    cncCheck.counterNum = ( ((gateState << 9) | gateId) % blockSize);

    /* AUTODOC: Expect 1 packet to pass to target port */
    cncCheck.counterValue.packetCount.l[0] = expectedPacketsNumCount;
    /* AUTODOC: Expect number of first send packet +4 CRC bytes */
    cncCheck.counterValue.byteCount.l[0] = expectedBytesNumCount;

    /* AUTODOC: Call CNC API to read CNC counter */
    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* internal prvTgfStreamSgcQosProfileUpdateVerification function
* @endinternal
*
* @brief  Check packets and counters for the 2 packets that were sent
*
*         Expect 2 packets to reach target port since this gate is opened and packet
*         size is less than configured in Interval Max table for this gate and time slot
*
*         For first packet:
*         - Gate was configured with NULL IPV and therefore packet priority
*           should not be changed by the Gate Control List (GCL).
*         - vlan<up> should be set as the up that was configured for QoS profile (#3)
*
*         For second packet:
*         - Gate was configured with valid IPV and therefore packet priority should
*           be changed by the Gate Control List (GCL).
*         - the 3 LS bits of Desc<QoS profile> are overwritten by the IPV value and
*           therefore vlan<up> should now be set as the up that was configured for
*           the modified QoS profile (#7)
*
*         Read below counters:
*         - read CNC counters
*         - read octet counter table to get actual number of bytes passed through that gate
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateVerification
(
    GT_VOID
)
{
    GT_STATUS                  rc = GT_OK;
    GT_U32                     cncBlockNum;
    GT_U32                     expectedPacketsNumCount,expectedBytesNumCount;
    GT_U32                     gateState;
    GT_U8                      packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}, {0}};
    GT_U32                     packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_MAX_PACKET_SIZE_CNS,PRV_TGF_MAX_PACKET_SIZE_CNS};
    GT_U32                     packetActualLength = 0;
    GT_U8                      queue = 0;
    TGF_NET_DSA_STC            rxParam;
    GT_U8                      pktNum;
    GT_BOOL                    getFirst = GT_TRUE;
    GT_U32                     vlanUpOffset = 13;
    GT_U32                     vlanUpLength = 3;
    CPSS_DXCH_COS_PROFILE_STC  qosProfile;

    /**** Check packets content ****/

    /* AUTODOC: Loop over 2 packets */
    for (pktNum=0; pktNum<2; pktNum++)
    {
        /* AUTODOC: Get the captured packets */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                                                                TGF_PACKET_TYPE_CAPTURE_E,
                                                                getFirst,
                                                                GT_TRUE,
                                                                packetBuf[pktNum],
                                                                &packetLen[pktNum],
                                                                &packetActualLength,
                                                                &prvTgfDevNum,
                                                                &queue,
                                                                &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

        /* AUTODOC: Get vlan UP as it was set in QoS profile */
        rc = cpssDxChCosProfileEntryGet(prvTgfDevNum, profileIndexArr[pktNum], &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");

        /* AUTODOC: Build expected vlan tag with the expected UP */
        U32_SET_FIELD_MAC(expectedVlan,vlanUpOffset,vlanUpLength,qosProfile.userPriority);

        /* AUTODOC: Check content of this packet */
        tgfStreamSgcPacketVerification(packetBuf[pktNum]);

        /* AUTODOC: Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }


    /**** Check counters for packets that passed ****/

    /* AUTODOC: Read CNC counter */

    /* AUTODOC: Gate is opened (bits[10:9] in CNC index number) */
    gateState   = 0x0;
    /* AUTODOC: CNC block number 0 that is used to count indexes of 0 - 1023 range  */
    cncBlockNum = blockNum0;
    /* AUTODOC: Expect 2 packets to pass since gate is opened and no exception should occur */
    expectedPacketsNumCount = 2;
    expectedBytesNumCount = expectedBytesNum[0] + expectedBytesNum[1] + 8/*CRC for 2 packets*/;
    prvTgfStreamSgcCncCountersGet(gateState,
                                  cncBlockNum,
                                  expectedPacketsNumCount,
                                  expectedBytesNumCount);

    /* AUTODOC: Read Gate Octet counter */

    /* AUTODOC: Check gate Octet byte counter */
    prvTgfStreamSgcOctetCounterCheck(expectedBytesNumCount);
}


/******************************************************************************\
 *   Traffic functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcQosProfileUpdateTraffic function
* @endinternal
*
* @brief   Stream Gate Control QoS profile use case traffic.
*
*          - Send 2 packets of 68 bytes each with vlan tag
*            - before sending first packet set gate to keep TC
*            - before sending second packet set gate to change TC according to IPV value
*          - Enable capture on target port
*          - Read counters from target port and save them to be verified
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateTraffic
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC         portCntrs;
    GT_U32                                portIter;
    GT_U32                                i;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC    gateParams;


    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d", prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");

    /* AUTODOC: Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfShortPacketPartArray) / sizeof(prvTgfShortPacketPartArray[0]);
    packetInfo.partsArray = prvTgfShortPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* AUTODOC: Update packet size */
    expectedBytesNum[0] = sizeof(prvTgfPacketL2Part) + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfShortPayloadDataArr);

    /* AUTODOC: Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* AUTODOC: Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[0] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[0] = portCntrs.goodOctetsRcv.l[0];

    /**** Change gate configuration before sending second packet ****/

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));

    /* AUTODOC: Get last gate parameters */
    rc = cpssDxChStreamSgcGateConfigGet(prvTgfDevNum,
                                        gateId,
                                        tableSetId,
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigGet");

    /* AUTODOC: Update gate configurations
                - Go over all gate time slots
                - Set to change packet TC according to IPV value
     */
    for (i=0;i<timeSlotsNum;i++)
    {
        gateParams.timeSlotInfoArr[i].keepTc = GT_FALSE;
        gateParams.timeSlotInfoArr[i].ipv = 0x7;
    }

    /* AUTODOC: Call SGC API to set these parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateId,
                                        tableSetId,
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");


    PRV_UTF_LOG0_MAC("======= Send second packet =======\n");

    /* AUTODOC: Update packet size */
    expectedBytesNum[1] = sizeof(prvTgfPacketL2Part) + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfShortPayloadDataArr);

    /* AUTODOC: Send second packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[1] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[1] = portCntrs.goodOctetsRcv.l[0];

    /* AUTODOC: Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}


/******************************************************************************\
 *   Restore functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcQosProfileUpdateConfigRestore function
* @endinternal
*
* @brief   Stream Gate Control configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore QoS configurstions
*          3. Restore gate counters Configuration
*          4. Restore PCL Configuration
*          5. Restore gate configuration
*          6. Restore base Configuration
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;
    GT_U32                           gateState;
    CPSS_DXCH_COS_PROFILE_STC        qosProfile;

    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* AUTODOC: invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_6_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgDefVlanEntryInvalidate");

    /* AUTODOC: Restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);

    /* -------------------------------------------------------------------------
     * 2. Restore QoS Configurations
     */
    /**** Reset QoS profiles ****/
    /* AUTODOC: Clear qosProfile structure */
    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
    /* AUTODOC: Reset first profile */
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, profileIndexArr[0], &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");
    /* AUTODOC: Reset second profile */
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, profileIndexArr[1], &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");

    /**** Restore port's QoS attributes ****/
    rc = cpssDxChCosPortQosConfigSet(prvTgfDevNum,
                                     prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                     &prvTgfRestoreCfg.qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosPortQosConfigSet");

    /**** Reset port's Trust Mode ****/
    /* AUTODOC: Set to no trust. */
    rc = cpssDxChCosPortQosTrustModeSet(prvTgfDevNum,
                                        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                        prvTgfRestoreCfg.portQosTrustMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosPortQosTrustModeSet");

    /* -------------------------------------------------------------------------
     * 2. Restore gate counters Configuration
     */

    /**** CNC counters ****/

    /* AUTODOC: Clear CNC configuration structure */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));

    /* AUTODOC: Set CNC client */
    cncCfg.clientType = PRV_TGF_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;

    /* AUTODOC: Set same CNC block number that was set in test */
    cncCfg.blockNum = blockNum0;

    /* gateState open */
    gateState = 0;

    /* AUTODOC: Set CNC counter index */
    cncCfg.counterNum = (gateState<<9) | gateId;

    /* AUTODOC: Set CNC counter format to mode 0 */
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* AUTODOC: Disable CNC client */
    cncCfg.clientEnable = GT_FALSE;

    /* AUTODOC: Reset CNC counter index range */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(&cncCfg, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncGenConfigure");

    /* gateState Interval Max Exceeded */
    gateState = 2;

    /* AUTODOC: Set CNC counter index */
    cncCfg.counterNum = (gateState<<9) | gateId;

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(&cncCfg, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncGenConfigure");

    /* AUTODOC: Set packet pass/drop mode */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,
                                                   prvTgfRestoreCfg.passDropMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeGet");

    /**** Ingress Drop counters ****/

    /* AUTODOC: Reset the ingress drop counter */
    rc = prvTgfCfgIngressDropCntrSet(0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrSet");

    /* AUTODOC: Enable packet drop in EQ */
    rc = cpssDxChCfgIngressDropEnableSet(prvTgfDevNum, prvTgfRestoreCfg.dropEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrSet");

    /* AUTODOC: Set the probe packet configuration */
    rc = cpssDxChCfgProbePacketConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.probeCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgProbePacketConfigSet");

    /* -------------------------------------------------------------------------
     * 3. Restore PCL Configuration
     */
    /* AUTODOC: invalidate PCL rule #1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, ruleId, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, RuleIndex = %d", 1);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: Set back PCL Generic Action mode */
    rc = cpssDxChPclGenericActionModeSet(
        prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E, 0, prvTgfRestoreCfg.genericActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeSet");

    /* -------------------------------------------------------------------------
     * 4. Restore Gate Configuration
     */

    /* AUTODOC: Set SGC global parameters */
    rc = cpssDxChStreamSgcGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigSet");

    /* AUTODOC: Set gate parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateId,
                                        tableSetId,
                                        timeSlotsNum,
                                        imxProfilesNum,
                                        &prvTgfRestoreCfg.gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");

    /* AUTODOC: Clear octet counter */
    rc = prvCpssStreamSgcOctetCountersSet(prvTgfDevNum, gateId, tableSetId, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");

    /* -------------------------------------------------------------------------
     * 5. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}
