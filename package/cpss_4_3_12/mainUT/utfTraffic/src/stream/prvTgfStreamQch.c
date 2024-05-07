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
* @file prvTgfStreamQch.c
*
* @brief 802.1Qch Cyclic Queuing and Forwarding feature use case testing.
*
*        Features that are tested:
*
*        - Combination of 2 features:
*          - Qci (Per Stream Filtering and Policing) that is applied on the Ingress
*          - Qbv (Time aware shaper windows per port) that is applied on the Egress
*
*        1 test is added to check above features:
*        - prvTgfStreamQch
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
*        - Select random values for Qci gate ID, Qci table set ID and Qbv table set ID
*        - Configure Qci:
*          - Set IPCL with Qci parameters to assign specific gate id
*          - Set 2 Qos profiles
*            - profile#7 with TC set to 7
*            - profile#6 with TC set to 6
*          - Set 2 time slots and cycle time of 2 time slots
*            - time slot duration 1 mili seconds
*            - cycle time duration 2 mili seconds
*          - Set first time slot to open gate and with IPV value of 7
*            - packets in this time slot will use Qos profile#7 and be forwarded to Queue 7
*          - Set second time slot to open gate and with IPV value of 6
*            - packets in this time slot will use Qos profile#6 and be forwarded to Queue 6
*        - Configure Qbv:
*          - Bind table set to target port (port#4)
*          - Set the same number of time slots and the same time slot duration as in Qci
*          - Set the same cycle time as in Qci
*          - For first time slot close gate for Queue 7 and open gate for Queue 6
*          - For seond time slot open gate for Queue 7 and close gate for Queue 6
*
*        - Send 2 packets that belong to the same Qci gate's stream
*           - Send first packet (in time slot x)
*           - Wait for 1 mili second (time slot duration)
*           - Send second packet (in time slot x+1)
*           - Wait another 1 mili second to make sure packets are out from the Queus
*        - Check results
*          - expect both packets to reach target port
*            - For first time slot packet receives at Q7 and is out from Q6
*            - For second timeslot packet receives at Q6 and is out from Q7
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
#include <stream/prvTgfStreamQch.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCncGen.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamGateControl.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>


/*************************** Constants definitions ***********************************/

/* Qci maximum valid Gate ID value */
#define PRV_TGF_SGC_MAX_GATE_ID_NUM_CNS         510

/* Qci maximum number of Table Sets */
#define PRV_TGF_SGC_MAX_TABLE_SETS_NUM_CNS      58

/* Qbv maximum number of Table Sets */
#define PRV_TGF_STREAM_EGRESS_TABLESET_NUM_CNS  29

/* Number of gate time slots to use in the test */
#define PRV_TGF_GATE_TIME_SLOTS_NUM_CNS         2

/* Number of gate interval max to use in the test */
#define PRV_TGF_GATE_IMX_PROFILES_NUM_CNS       2

/* Used Qos profile 3 */
#define PRV_TGF_SGC_TEST_QOS_PROFILE3           3

/* Used Qos profile 6 */
#define PRV_TGF_SGC_TEST_QOS_PROFILE6           6

/* Used Qos profile 7 */
#define PRV_TGF_SGC_TEST_QOS_PROFILE7           7

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS                 2

/* Ports index allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS            0
#define PRV_TGF_TARGET_PORT_IDX_CNS             1

/* VLAN Id #6 */
#define PRV_TGF_VLANID_6_CNS                    6

/* Size of maximum output packets in bytes include CRC */
#define PRV_TGF_MAX_PACKET_SIZE_CNS             72

/* CNC block that is used in test to count cnc indexes from 0 - 1023 */
#define PRV_TGF_CNC_GEN_BLOCK_NUM_CNS           0

/* PCL rule index */
#define PRV_TGF_PCL_RULE_INDEX_CNS              1


/*************************** static DB definitions ***********************************/

/* Selected queues that are used in test for Qci and Qbv */
static GT_U32  queueArr[PRV_TGF_GATE_TIME_SLOTS_NUM_CNS] = {7,6};

/* Time slot duration (in nano seconds) used in test for Qci and Qbv */
static GT_U32  timeSlotDuration = 1000000; /*1 mili seconds*/

/* Struct to hold packet information */
typedef struct
{
    GT_U32                 totalLen;       /* not include CRC                  */
    GT_U32                 numOfParts;     /* num of elements in of partsArray */
    TGF_PACKET_PART_STC    *partsArray;    /* partsArray data                  */
    TGF_PACKET_L2_STC      *packetL2Part;  /* packet L2 information            */
} TGF_PACKET_INFO_STC;

/* Holds gate ID and table set ID to be used in this test */
static GT_U32  gateIdQci;
static GT_U32  tableSetIdQci;
static GT_U32  tableSetIdQbv;

/* Input packet information */
static TGF_PACKET_STC   packetInfo;

/* Target port interface information */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Expected values of VLAN tags
   vlan<up> is changed during test
 */
static GT_U32    expectedVlan0 = (TGF_ETHERTYPE_8100_VLAN_TAG_CNS << 16) |
                                 (7 << 13) /*UP*/                        |
                                 (0 << 12) /*CFI*/                       |
                                 (PRV_TGF_VLANID_6_CNS);

static GT_U32    expectedVlan1 = (TGF_ETHERTYPE_8100_VLAN_TAG_CNS << 16) |
                                 (6 << 13) /*UP*/                        |
                                 (0 << 12) /*CFI*/                       |
                                 (PRV_TGF_VLANID_6_CNS);

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

/* Packet payload data + EtherType */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),    /* dataLength */
    prvTgfPayloadDataArr             /* dataPtr */
};

/* Parts of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Parameters that are needed to be restored */
static struct
{
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT               genericActionMode;
    CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT passDropMode;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC              globalParams;
    CPSS_QOS_PORT_TRUST_MODE_ENT                        portQosTrustMode;
    CPSS_QOS_ENTRY_STC                                  qosEntry;
    GT_U32                                              gate[8];
} prvTgfRestoreCfg;


/******************************************************************************\
 *   Configurations functions                                                 *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcGateIdAndTableSetIdSelect function
* @endinternal
*
* @brief   Select random Gate ID and TableSet ID
*          - Qci Gate ID range: 0..510
*          - Qci TableSet ID range: 0..57
*          - Qbv TableSet ID range: 0..28
*/
static GT_VOID prvTgfStreamSgcGateIdAndTableSetIdSelect
(
    GT_VOID
)
{
    /* Generate random value for Qci Gate ID in range 0:510 */
    gateIdQci = cpssOsRand() % PRV_TGF_SGC_MAX_GATE_ID_NUM_CNS;

    /* Generate random value for Qci TableSet ID in range 0:57 */
    tableSetIdQci = cpssOsRand() % (PRV_TGF_SGC_MAX_TABLE_SETS_NUM_CNS-1);

    /* Generate random value for Qbv TableSet ID in range 0:28 */
    tableSetIdQbv = cpssOsRand() % (PRV_TGF_STREAM_EGRESS_TABLESET_NUM_CNS-1);

    PRV_UTF_LOG3_MAC("Gate & TableSet IDs that are used in this test: "
                     "Qci gateId:[%d], Qci tableSetId:[%d], Qbv tableSetId:[%d] \n"
                     ,gateIdQci,tableSetIdQci, tableSetIdQbv);
}

/**
* @internal prvTgfStreamSgcBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*          - Packet egress with Tag0 as defined in ingress pipe for all ports
*          - Forward to target ePort according to MAC destination
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

    /* AUTODOC: Add FDB entry with MAC 00:00:00:00:34:02, VLAN ID 6, target port.
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
*          - Configures 3 QoS profiles
*             - profile#3 with priority & TC 3
*             - profile#6 with priority & TC 6
*             - profile#7 with priority & TC 7
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

    /* AUTODOC: Set profile number 3 */
    qosProfile.userPriority = 3;
    qosProfile.trafficClass = 3;
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, PRV_TGF_SGC_TEST_QOS_PROFILE3, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");

    /* AUTODOC: Set profile number 6 */
    qosProfile.userPriority = queueArr[1]; /*6*/
    qosProfile.trafficClass = queueArr[1]; /*6*/
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, PRV_TGF_SGC_TEST_QOS_PROFILE6, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");

    /* AUTODOC: Set profile number 7 */
    qosProfile.userPriority = queueArr[0]; /*7*/
    qosProfile.trafficClass = queueArr[0]; /*7*/
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, PRV_TGF_SGC_TEST_QOS_PROFILE7, &qosProfile);
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
    pclQciActions = (cncIndxMode << 11) | (maxSduSizeProfileIndx << 9) | gateIdQci ;
    action.redirect.data.genericActionInfo.genericAction = pclQciActions ;

    /* AUTODOC: Set PCL rule */
    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                          PRV_TGF_PCL_RULE_INDEX_CNS,
                          &mask,
                          &pattern,
                          &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStreamGateControlPclConfigSet");

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
* @internal prvTgfStreamQciGateConfigSet function
* @endinternal
*
* @brief   Qci gate configurations
*/
static GT_VOID prvTgfStreamQciGateConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                 rc = GT_OK;
    GT_U32                                    i;
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
    globalParams.intervalMaxOctetExceededException.cpuCode = CPSS_NET_USER_DEFINED_5_E;
    globalParams.gateClosedException.command               = CPSS_PACKET_CMD_DROP_HARD_E;
    globalParams.gateClosedException.cpuCode               = CPSS_NET_USER_DEFINED_6_E;
    globalParams.sgcIgnoreTimestampUseTod                  = GT_FALSE;
    globalParams.sgcCountingModeL3                         = GT_FALSE;
    rc = cpssDxChStreamSgcGlobalConfigSet(prvTgfDevNum, &globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigSet");

    /**** Set gate parameters ****/

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));

    /*
       AUTODOC: Set Interval max and Time slot parameters
                - Set Interval Max value to 500 bytes for all time slots
                - Set each time slot with time to advance value of 1 mili second
                - Set each time slot with the profile index to Interval Max Table
                - Change packet's TC: first time slot to 7, second time slot to 6
                - Enable length aware
                - Open the gate for all time slots
                - Do not reset the Octet counter in time slot transition
     */
    for (i=0;i<PRV_TGF_GATE_TIME_SLOTS_NUM_CNS;i++)
    {
        gateParams.intervalMaxArr[i]                          = 500;  /*big enough to pass 2 packets*/
        gateParams.timeSlotInfoArr[i].timeToAdvance           = (i+1)*timeSlotDuration; /*1 mili seconds*/
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_FALSE; /*change TC to IPV*/
        gateParams.timeSlotInfoArr[i].ipv                     = queueArr[i]; /*7,6*/
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_TRUE;
        gateParams.timeSlotInfoArr[i].streamGateState         = CPSS_DXCH_STREAM_SGC_GATE_STATE_OPEN_E;
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_FALSE;
    }

    /* AUTODOC: Set Table Set time configuration parameters. */
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_FALSE;
    gateParams.tableSetInfo.byteCountAdjust         = 0;
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.remainingBitsFactor     = 255;    /* 255 bits every 128nsec */
    gateParams.tableSetInfo.cycleTime = timeSlotDuration*PRV_TGF_GATE_TIME_SLOTS_NUM_CNS; /*2 mili seconds*/
    gateParams.tableSetInfo.ingressTimestampOffset  = 0;
    gateParams.tableSetInfo.ingressTodOffset        = 0;

    /* AUTODOC: Call SGC API to set these parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateIdQci,
                                        tableSetIdQci,
                                        PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                        PRV_TGF_GATE_IMX_PROFILES_NUM_CNS,
                                        &gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");
}

/**
* @internal prvTgfStreamSgcStatisticsConfigSet function
* @endinternal
*
* @brief  Set packet Qci gate id statistics
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
    cncCfg.blockNum = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;

    /* AUTODOC: Set CNC counter index in 802.1Qci format:
                |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
     */
    /* AUTODOC: Set gate state to open */
    gateState = 0x0;

    /* AUTODOC: modulo block size since index can not be larger than maximum block size  */
    cncCfg.counterNum = ( ((gateState << 9) | gateIdQci) % blockSize);

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
}

/**
* @internal prvTgfStreamQbvConfigSet function
* @endinternal
*
* @brief  Set Qbv configurations
*         - Update 2 time slots with time slot duration of 1 mili seconds each
*         - 1st time slot: gate is open for queue 6 and close for queue 7
*         - 2nd time slot: gate is close for queue 6 and open for queue 7
*         - Set cycle time to 2 mili seconds
*         - Bind table set to target port
*/
static GT_VOID prvTgfStreamQbvConfigSet
(
    GT_VOID
)
{
    GT_STATUS                                     rc = GT_OK;
    GT_U32                                        i;
    GT_U32                                        queueOffset;
    GT_U32                                        gate;
    CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  bindEntry;
    GT_U8                                         gateStateBmpArr[PRV_TGF_GATE_TIME_SLOTS_NUM_CNS];
    CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC    timeSlotInfoArr[PRV_TGF_GATE_TIME_SLOTS_NUM_CNS];

    /**** Set tableSet with time slots parameters ****/

    /* Gate state (close:1, open:0)
                   | Q7 | Q6 | Q5 | Q4 | Q3 | Q2 | Q1 | Q0 |                                                                                                                                           |
       timeSlot 0  | 1  | 0  | 1  | 1  | 1  | 1  | 1  | 1  |
       timeSlot 1  | 0  | 1  | 1  | 1  | 1  | 1  | 1  | 1  |                                                                            |
     */
    gateStateBmpArr[0] = ~(1<<queueArr[1]); /* 0xBF */
    gateStateBmpArr[1] = ~(1<<queueArr[0]); /* 0x7F */

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&timeSlotInfoArr, 0, sizeof(timeSlotInfoArr));

    for (i=0;i<PRV_TGF_GATE_TIME_SLOTS_NUM_CNS;i++)
    {
        timeSlotInfoArr[i].gateStateBmp  = gateStateBmpArr[i];
        timeSlotInfoArr[i].timeToAdvance = (i+1)*(timeSlotDuration); /*1 mili seconds*/
        timeSlotInfoArr[i].hold          = GT_FALSE;
    }

    /* Configures tableSet with time slots parameters */
    rc = cpssDxChStreamEgressTableSetConfigSet(prvTgfDevNum,
                                               tableSetIdQbv,
                                               PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                               timeSlotInfoArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChStreamEgressTableSetConfigSet");


    /****
      Bind port to tableSet
      Configures request parameters
      Sets configuration time to take place
     ****/

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&bindEntry, 0, sizeof(bindEntry));

    /* Set gate re-configuration request parameters */
    bindEntry.gateReconfigRequestParam.bitsFactorResolution = 7; /*128 nanosec*/
    bindEntry.gateReconfigRequestParam.cycleTime = timeSlotDuration*PRV_TGF_GATE_TIME_SLOTS_NUM_CNS; /*2 mili seconds*/
    bindEntry.gateReconfigRequestParam.egressTodOffset = 1000; /*1000 nano seconds*/
    bindEntry.gateReconfigRequestParam.lastEntry = 1;
    bindEntry.gateReconfigRequestParam.lengthAware = GT_TRUE;
    bindEntry.gateReconfigRequestParam.maxAlwdBcOffset = 0;
    bindEntry.gateReconfigRequestParam.remainingBitsFactor = 255; /* 255 bits every 128nsec */

    /* Set gate re-configuration time
       Set it to small value such as 10 nano seconds so it will occure almost immediately */
    bindEntry.gateReconfigTimeParam.configChangeTimeExtension = 0;
    bindEntry.gateReconfigTimeParam.todMsb.l[0]= 0;
    bindEntry.gateReconfigTimeParam.todMsb.l[1]= 0;
    bindEntry.gateReconfigTimeParam.todLsb= 10;

    rc = cpssDxChStreamEgressPortBindSet(prvTgfDevNum,
                                         prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                         tableSetIdQbv,
                                         &bindEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChStreamEgressPortBindSet");


    /****
      Map gate to Queue

     Queue:     Q7 Q6 Q5 Q4 Q3 Q2 Q1 Q0
     Gate :      7  6  5  4  3  2  1  0
     ****/
    for(i=0;i<8;i++)
    {
        queueOffset = i;
        gate = i;

        /* AUTODOC: First Save mapping of gate to queue for restore */
        rc = cpssDxChStreamEgressPortQueueGateGet(prvTgfDevNum,
                                                  prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                  queueOffset,
                                                  &prvTgfRestoreCfg.gate[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChStreamEgressPortQueueGateSet");

        /* AUTODOC: Then set new mapping of gate to queue */
        rc = cpssDxChStreamEgressPortQueueGateSet(prvTgfDevNum,
                                                  prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                  queueOffset,
                                                  gate);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChStreamEgressPortQueueGateSet");
    }
}

/**
* internal prvTgfStreamQchConfigSet function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding use case configurations
*/
GT_VOID prvTgfStreamQchConfigSet
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
    prvTgfStreamQciGateConfigSet();

    /* AUTODOC: Gate statistics configuration */
    prvTgfStreamSgcStatisticsConfigSet();

    /* AUTODOC: Qbv configurations */
    prvTgfStreamQbvConfigSet();
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
*         - Check vlan tag return its value
*         - Check payload data of passenger
*/
static GT_VOID tgfStreamSgcPacketVerification
(
    IN  GT_U8  *packetBuf,
    OUT GT_U32 *recVlan
)
{
    GT_U32              pktOffset = 0;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U32              recPassengerPayload, expPassengerPayload;
    GT_BOOL             isVlanOk = GT_TRUE;

    PRV_UTF_LOG0_MAC("======= Check content of output packet  =======\n");

    /**** Check MAC DA SA ****/
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

    *recVlan = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                       (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    if ((*recVlan != expectedVlan0) && (*recVlan != expectedVlan1))
    {
        isVlanOk = GT_FALSE;
    }
    /* AUTODOC: Compare received vlan vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE,isVlanOk,
                                 "Failure in tgfStreamGateControlPacketVerification: Mismatch in vlan tag \n"
                                 "expect:0x%x or 0x%x but received 0x%x \n",
                                 expectedVlan0,
                                 expectedVlan1,
                                 *recVlan);

    /**** Check Passenger payload ****/
    /* AUTODOC: Increment offset by 4B of VLAN tag to point to passenger payload (include EtherType) */
    pktOffset += TGF_VLAN_TAG_SIZE_CNS;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfPacketPayloadPart.dataLength;i+=4,pktOffset+=4)
    {
        /* AUTODOC: Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* AUTODOC: Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfPayloadDataArr[i+3] | (prvTgfPayloadDataArr[i+2] << 8) |
                                           (prvTgfPayloadDataArr[i+1] << 16) | (prvTgfPayloadDataArr[i] << 24));

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
    rc = prvCpssStreamSgcOctetCountersGet(prvTgfDevNum,
                                          gateIdQci,
                                          tableSetIdQci,
                                          &octetCounterActual);
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
    cncCheck.counterNum = ( ((gateState << 9) | gateIdQci) % blockSize);

    /* AUTODOC: Update number of expected packets to pass to target port */
    cncCheck.counterValue.packetCount.l[0] = expectedPacketsNumCount;
    /* AUTODOC: Update number of expected bytes (include 4 CRC bytes) */
    cncCheck.counterValue.byteCount.l[0] = expectedBytesNumCount;

    /* AUTODOC: Call CNC API to read and verify CNC counter */
    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* internal prvTgfStreamQchVerification function
* @endinternal
*
* @brief  Check packets and counters for the 2 packets that were sent
*
*         - Expect 2 packets to reach target port.
*            - Each packet from different queue and different time slot
*         - Check packets content
*         - Check Qbv configuration time has indeed matched
*         - Read below counters:
*            - read CNC counters
*            - read octet counter table to get actual number of bytes passed through that gate
*/
GT_VOID prvTgfStreamQchVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U32             cncBlockNum;
    GT_U32             expectedPacketsNumCount=0,expectedBytesNumCount=0;
    GT_U32             gateState;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_MAX_PACKET_SIZE_CNS,PRV_TGF_MAX_PACKET_SIZE_CNS};
    GT_U32             packetActualLength = 0;
    GT_U8              queue = 0;
    TGF_NET_DSA_STC    rxParam;
    GT_U8              pktNum;
    GT_BOOL            getFirst = GT_TRUE;
    GT_U32             recVlanArr[PRV_TGF_BURST_COUNT_CNS] = {0,0};
    GT_BOOL            isVlanOk = GT_TRUE;

    /**** Check Qbv configuration took place ****/
    /* AUTODOC: Verify Qbv configuration time matched.
                Enable it to be used again for next configurations */
    rc = cpssDxChStreamEgressPortBindComplete(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamEgressPortBindComplete");

    /**** Check packets content ****/
    /* AUTODOC: Loop over 2 packets */
    for (pktNum=0; pktNum<PRV_TGF_BURST_COUNT_CNS; pktNum++)
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

        /* AUTODOC: Check content of this packet */
        tgfStreamSgcPacketVerification(packetBuf[pktNum],&recVlanArr[pktNum]);

        /* AUTODOC: Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }

    /* AUTODOC: Check first packet vlan tag and second packet vlan tag are not the same
       since packets received in different time slots and therefore sends to different queues
       vlan.up should not be the same */
    if(recVlanArr[0] == recVlanArr[1])
    {
        isVlanOk = GT_FALSE;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE,isVlanOk,
          "Failure in prvTgfStreamQchVerification: got the same vlan tag 0x%x for both packets \n",
           recVlanArr[0]);


    /**** Check counters for packets that passed ****/
    /* AUTODOC: Read CNC counter */

    /* AUTODOC: Gate is opened (bits[10:9] in CNC index number) */
    gateState   = 0x0;
    /* AUTODOC: CNC block number 0 that is used to count indexes of 0 - 1023 range  */
    cncBlockNum = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    /* AUTODOC: Expect 2 packets to pass since gate is opened and no exception should occur */
    expectedPacketsNumCount = 2;
    expectedBytesNumCount = 2*(TGF_L2_HEADER_SIZE_CNS       +  \
                               TGF_VLAN_TAG_SIZE_CNS        +  \
                               sizeof(prvTgfPayloadDataArr) +  \
                               TGF_CRC_LEN_CNS);
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
* @internal prvTgfStreamQchTraffic function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding use case traffic.
*
*          - Send 2 packets of 68 bytes each with vlan tag
*          - Set delay of 1 mili second between the packets
*          - Enable capture on target port
*          - Read counters from target port
*/
GT_VOID prvTgfStreamQchTraffic
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;
    GT_U32                           portIter;
    GT_U32                           i;

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

    /* AUTODOC: Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo.partsArray = prvTgfPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    for (i=0;i<PRV_TGF_BURST_COUNT_CNS;i++)
    {
        /* AUTODOC: Print debug TOD timing */
        rc = prvCpssStreamSgcDebugTodDump(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvCpssStreamSgcDebugTodDump");

        /* AUTODOC: Send packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* AUTODOC: Read counters from target port and save them to be verified */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");

        /* AUTODOC: Sleep for 850 msec which is actually 1msec in emulator time
                    generate delay of 1 time slot duration which was set to 1msec
         */
        cpssOsTimerWkAfter(850);
    }
    cpssOsTimerWkAfter(850);
    /* AUTODOC: Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}


/******************************************************************************\
 *   Restore functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamQchConfigRestore function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore QoS configurstions
*          3. Restore Qci gate counters Configuration
*          4. Restore PCL Configuration
*          5. Restore Qci gate configuration
*          6. Restore Qbv configuration
*          7. Restore base Configuration
*/
GT_VOID prvTgfStreamQchConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC         cncCfg;
    GT_U32                                gateState;
    CPSS_DXCH_COS_PROFILE_STC             qosProfile;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC    gateParams;
    GT_U32                                i;
    GT_U32                                queueOffset;
    CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC timeSlotInfoArr[PRV_TGF_GATE_TIME_SLOTS_NUM_CNS];

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
    /* AUTODOC: Reset profile#3 */
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, PRV_TGF_SGC_TEST_QOS_PROFILE3, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");
    /* AUTODOC: Reset profile#6 */
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, PRV_TGF_SGC_TEST_QOS_PROFILE6, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChCosProfileEntrySet");
    /* AUTODOC: Reset profile#7 */
    rc = cpssDxChCosProfileEntrySet(prvTgfDevNum, PRV_TGF_SGC_TEST_QOS_PROFILE7, &qosProfile);
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
     * 3. Restore Qci gate counters Configuration
     */
    /* AUTODOC: Clear CNC configuration structure */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));

    /* AUTODOC: Set CNC client */
    cncCfg.clientType = PRV_TGF_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;

    /* AUTODOC: Set same CNC block number that was set in test */
    cncCfg.blockNum = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;

    /* gateState open */
    gateState = 0;

    /* AUTODOC: Set CNC counter index */
    cncCfg.counterNum = (gateState<<9) | gateIdQci;

    /* AUTODOC: Set CNC counter format to mode 0 */
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* AUTODOC: Disable CNC client */
    cncCfg.clientEnable = GT_FALSE;

    /* AUTODOC: Reset CNC counter index range */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(&cncCfg, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncGenConfigure");

    /* AUTODOC: Set packet pass/drop mode */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,
                                                   prvTgfRestoreCfg.passDropMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeGet");

    /* -------------------------------------------------------------------------
     * 4. Restore PCL Configuration
     */
    /* AUTODOC: invalidate PCL rule #1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, PRV_TGF_PCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, RuleIndex = %d", 1);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: Set back PCL Generic Action mode */
    rc = cpssDxChPclGenericActionModeSet(
        prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E, 0, prvTgfRestoreCfg.genericActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeSet");

    /* -------------------------------------------------------------------------
     * 5. Restore Qci Gate Configuration
     */
    /* AUTODOC: Set SGC global parameters */
    rc = cpssDxChStreamSgcGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigSet");

    /* AUTODOC: Clear structure before filling it */
    cpssOsMemSet(&gateParams, 0, sizeof(gateParams));

    /* AUTODOC: Set interval max octet profile index with defaukt value */
    for (i=0;i<CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;i++)
    {
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = 0xFF;
    }

    /* AUTODOC: Reset gate parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateIdQci,
                                        tableSetIdQci,
                                        CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS,
                                        CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS,
                                        &gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");

    /* AUTODOC: Clear octet counter */
    rc = prvCpssStreamSgcOctetCountersSet(prvTgfDevNum, gateIdQci, tableSetIdQci, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");

    /* -------------------------------------------------------------------------
     * 6. Restore Qbv Configuration
     */
    /* AUTODOC: Clear structure */
    cpssOsMemSet(&timeSlotInfoArr, 0, sizeof(timeSlotInfoArr));

    /* Configures tableSet with time slots parameters */
    rc = cpssDxChStreamEgressTableSetConfigSet(prvTgfDevNum,
                                               tableSetIdQbv,
                                               PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                               timeSlotInfoArr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChStreamEgressTableSetConfigSet");

    /* AUTODOC: Restore mapping of gate to queue */
    for (queueOffset=0;queueOffset<8;queueOffset++)
    {
        rc = cpssDxChStreamEgressPortQueueGateSet(prvTgfDevNum,
                                                  prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                                  queueOffset,
                                                  prvTgfRestoreCfg.gate[queueOffset]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChStreamEgressPortQueueGateSet");
    }

    /* AUTODOC: Unbind port from tableSet */
    rc = cpssDxChStreamEgressPortUnbindSet(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"cpssDxChStreamEgressPortUnbindSet");

    /* -------------------------------------------------------------------------
     * 7. Restore base Configuration
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
