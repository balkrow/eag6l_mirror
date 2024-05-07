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
* @file prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck.c
*
* @brief Egress mirroring of Qci stream with Qci slot ID added to the mirrored copy.
*
*        Features that are tested:
*
*        EPCL new UDB Metadata fields:
*        - Add descriptor fields <Qci Slot-ID>, <Gate State> and <Gate-ID> to EPCL UDB Metadata.
*        - Mirror Qci stream and add the time-slot to the mirrored copy:
*          - For egress mirrored copy goes to the host CPU:
*            - EPCL is applied on the original packet, assigns Flow-ID = Qci Slice-ID
*              and triggers egress mirroring
*            - Egress mirrored copy is sent to CPU with eDSA TO_CPU <Flow-ID>=Qci Slice-ID
*
*        Time slot functionality:
*           - Use 4 time slots with gate set to open
*           - Set cycle time to 4 time slots
*
*        Debug TOD:
*           - Read from debug TOD HW registers to get TOD timing
*
*        1 test is added to check above features:
*        - prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck
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
*        - Configure 4 time slots with gate set to open
*        - Set IPCL rule for each packet
*          - assign the configured gate to all packets
*        - Set EPCL rule for each packet
*          - use EPCL UDB metadata to set rule for each time slot
*          - Set flow ID to time slot ID
*          - Mirror the packet to CPU
*        - Send 4 packets with 1 time slot duration gap between each packet
*          this is done so each packet will be sent on different time slot
*        - Check results
*          - Expect all 4 packets to pass through that gate
*          - Expect each packet to have different time slot ID
*          - Check DSA<flowId> is equal = time slot per packet
*
* @version   1
********************************************************************************
*/
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStreamGateControl.h>
#include <stream/prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamGateControl.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCncGen.h>


/*************************** Constants definitions ***********************************/

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS               4

/* Ports index allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS          0
#define PRV_TGF_TARGET_PORT_IDX_CNS           1

/* VLAN Id #6 */
#define PRV_TGF_VLANID_6_CNS                  6

/* Maximum number of bytes of output packet (CRC included) */
#define PRV_TGF_MAX_PACKET_SIZE_CNS           68

/* Number of gate time slots to use in the test */
#define PRV_TGF_GATE_TIME_SLOTS_NUM_CNS       4

/* Number of gate interval max to use in the test */
#define PRV_TGF_GATE_IMX_PROFILES_NUM_CNS     4


/*************************** static DB definitions ***********************************/

/* Struct to hold packet information */
typedef struct
{
    GT_U32                 totalLen;       /* not include CRC                  */
    GT_U32                 numOfParts;     /* num of elements in of partsArray */
    TGF_PACKET_PART_STC    *partsArray;    /* partsArray data                  */
    TGF_PACKET_L2_STC      *packetL2Part;  /* packet L2 information            */
} TGF_PACKET_INFO_STC;

/* Gate information to use in test */
typedef struct
{
    GT_U32    gateId;       /* gate ID                                */
    GT_U32    tableSetId;   /* tableSet ID to be mapped to gate ID    */
} TGF_GATE_INFO_STC;

/* Gate information
   - 4 time slots, all set to open gate
   - 4 packets are sent while each packet is in different time slot
   */
static TGF_GATE_INFO_STC  gateInfo = {
/* gateId, tableSetId */
      170,         30
};

/* Packet send information */
static TGF_PACKET_STC   packetInfo;

/* Expected time slot Ids */
static  GT_U32    qciTimeSlotIdsArr[PRV_TGF_BURST_COUNT_CNS] = {0,1,2,3};

/* IPCL rule index */
static GT_U32     ipclRuleIndex = 0;

/* EPCL rule index */
static GT_U32     epclRuleIndex = 10;


/**** Packets definitions ****/

/**** Packet#0 ****/
/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x03}   /* saMac */
};

/* Packet's payload (including EtherType) */
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

/* Packet#0 parts */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Global packet Information structure */
static TGF_PACKET_INFO_STC  prvTgfGlobalPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,
    sizeof(prvTgfPacketPartArray)/sizeof(TGF_PACKET_PART_STC),
    prvTgfPacketPartArray,
    &prvTgfPacketL2Part
};

/* Parameters that are needed to be restored */
static struct
{
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT     genericActionMode;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC        gateParams;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC    globalParams;
} prvTgfRestoreCfg;


/******************************************************************************\
 *   Configurations functions                                                 *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*          Forward all packets to ePort#1 according to their MAC destination
*/
static GT_VOID prvTgfStreamSgcBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS    rc;

    /* AUTODOC: Save default VLAN 1 */
    prvTgfBrgVlanEntryStore(1);

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfGlobalPacketInfo.packetL2Part->daMac,
                                          PRV_TGF_VLANID_6_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
}

/**
* @internal prvTgfStreamSgcPclConfigSet function
* @endinternal
*
* @brief   PCL test configurations
*          - Set IPCL rule:
*            - according to MAC DA
*          - Set IPCL actions with 802.1Qci parameters
*            - Set redirect command to work with Generic action of 802.1Qci
*            - Set 12 bits generic action data with | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) |
*            - assign the configured gate to all packets
*          - Set EPCL rule:
*            - Use UDB and set rule according to QCI Slot ID UDB medatdata
*          - Set EPCL actions:
*            - set flowId = Qci Slot ID
*            - mirror the Qci packet to CPU
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
    GT_U32                                   udbIndex;
    GT_U32                                   udbAnchorOffset;
    GT_U32                                   ruleIndex = 0;
    GT_U32                                   i;

/**** IPCL configurations ****/

    /* AUTODOC: Init PCL Engine for ingress port */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_NUMBER_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                              PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit");

    /* AUTODOC: Set PCL actions */

    /* AUTODOC: Clear action structure */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: Forward packet */
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

    /* AUTODOC: Mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFFFF, sizeof(mask.ruleStdNotIp.macDa.arEther));

    /* AUTODOC: Pattern for MAC DA address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                 prvTgfGlobalPacketInfo.packetL2Part->daMac,
                 TGF_L2_HEADER_SIZE_CNS/2);

    /* AUTODOC: Set generic action data with 12 bits:
               | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) | */
    pclQciActions = (cncIndxMode << 11) | (maxSduSizeProfileIndx << 9) | (gateInfo.gateId);
    action.redirect.data.genericActionInfo.genericAction = pclQciActions ;

    /* AUTODOC: rule index */
    ruleIndex = ipclRuleIndex;

    /* AUTODOC: Set PCL rule */
    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                          ruleIndex,
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


/**** EPCL configurations ****/

/*
  Use UDB to set Qci time slot ID as the rule to catch the egress packet
  Set 4 rules for 4 different qci time slot IDs
  if rule matches:
  - mirror the packet to CPU
  - assign flowId with the compatible qci time slot ID
 */

    /* AUTODOC: UDB configurations for qci time slot pointer */
    udbIndex        = 0;
    udbAnchorOffset = 117; /*Anchor offset:117, bit offset: 0-7*/
    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                     PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                     CPSS_PCL_DIRECTION_EGRESS_E,
                                     udbIndex,
                                     PRV_TGF_PCL_OFFSET_METADATA_E,
                                     udbAnchorOffset);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet");

    /* AUTODOC: Init PCL Engine for Egress PCL */
    rc = prvTgfPclDefPortInit(prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                              CPSS_PCL_DIRECTION_EGRESS_E,
                              CPSS_PCL_LOOKUP_0_E,
                              PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                              PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                              PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit");

    /* AUTODOC: Clear action structure */
    cpssOsMemSet(&mask,    0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action,  0, sizeof(action));

    /* AUTODOC: Mirror the packet to CPU */
    action.pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    action.mirror.cpuCode = CPSS_NET_USER_DEFINED_5_E;
    action.egressPolicy = GT_TRUE;

    /* AUTODOC: Need to use EPCL rule indexes > 3 in order not to overwrite previous IPCL rule.
       Since standart IPCL rule is 30 bytes spans on indexes 0,1,2 */
    ruleIndex = epclRuleIndex;

    for (i=0;i<PRV_TGF_BURST_COUNT_CNS;i++, ruleIndex++)
    {
        /* AUTODOC: Set rule to QCi time slot ID */
        mask.ruleEgrUdbOnly.udb[0]    = 0xFF;
        pattern.ruleEgrUdbOnly.udb[0] = qciTimeSlotIdsArr[i];

        /* AUTODOC: Assign Qci time slot ID to flowId */
        action.flowId = qciTimeSlotIdsArr[i];

        /* AUTODOC: Set Egress PCL rule */
        rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                              ruleIndex,
                              &mask,
                              &pattern,
                              &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStreamGateControlPclConfigSet");
    }
}

/**
* @internal prvTgfStreamSgcGateConfigSet function
* @endinternal
*
* @brief   Gate configurations
*          Configure one gate with 4 time slots
*          - Set each time slot to open gate
*          - Set time slot duration for 1 mili second
*          - Set cycle time t0 4 mili seconds
*/
static GT_VOID prvTgfStreamSgcGateConfigSet
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
                - Set Interval Max to big enough value (100000 bytes) to pass 4 packets
                - Set each time slot with time to advance value of +1 mili sec
                - Set each time slot with the profile index to Interval Max Table
                - Do not change packet's TC
                - No length aware
                - Open the gate for all time slots
                - Do not reset the Octet counter in time slot transition
     */
    for (i=0;i<PRV_TGF_GATE_TIME_SLOTS_NUM_CNS;i++)
    {
        gateParams.intervalMaxArr[i]                          = 100000;
        gateParams.timeSlotInfoArr[i].timeToAdvance           = (i+1)*(1000000); /*1 mili seconds*/
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_TRUE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x4;
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_FALSE;
        gateParams.timeSlotInfoArr[i].streamGateState         = CPSS_DXCH_STREAM_SGC_GATE_STATE_OPEN_E;
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_FALSE;
    }

    /* AUTODOC: Set Table Set time configuration parameters. */
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_FALSE;
    gateParams.tableSetInfo.byteCountAdjust         = 0;
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.remainingBitsFactor     = 255;        /* 255 bits every 128nsec */
    gateParams.tableSetInfo.cycleTime = 1000000*PRV_TGF_GATE_TIME_SLOTS_NUM_CNS; /*4 mili seconds*/
    gateParams.tableSetInfo.ingressTimestampOffset  = 0;
    gateParams.tableSetInfo.ingressTodOffset        = 10000;

    /* AUTODOC: Save gate parameters for restore */
    rc = cpssDxChStreamSgcGateConfigGet(prvTgfDevNum,
                                        gateInfo.gateId,
                                        gateInfo.tableSetId,
                                        PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                        PRV_TGF_GATE_IMX_PROFILES_NUM_CNS,
                                        &prvTgfRestoreCfg.gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigGet");

    /* AUTODOC: Clear imx profiles of first 2 time slots that are used */
    for (i=0;i<PRV_TGF_GATE_TIME_SLOTS_NUM_CNS;i++)
    {
        prvTgfRestoreCfg.gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = 0;
    }

    /* AUTODOC: Call API to set these parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateInfo.gateId,
                                        gateInfo.tableSetId,
                                        PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                        PRV_TGF_GATE_IMX_PROFILES_NUM_CNS,
                                        &gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");

    /* AUTODOC: Clear HW gate octet counter */
    rc = prvCpssStreamSgcOctetCountersSet(prvTgfDevNum,
                                          gateInfo.gateId,
                                          gateInfo.tableSetId,
                                          0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");
}

/**
* internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigSet function
* @endinternal
*
* @brief   SGC EPCL UDB Metadata Qci Slot Id Check configurations
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: Bridge configuration */
    prvTgfStreamSgcBridgeConfigSet();

    /* AUTODOC: PCL configuration */
    prvTgfStreamSgcPclConfigSet();

    /* AUTODOC: Gate configuration */
    prvTgfStreamSgcGateConfigSet();
}


/******************************************************************************\
 *   Verification functions                                                   *
\******************************************************************************/

/**
* internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckVerification function
* @endinternal
*
* @brief  Check DSA<flowId> for the 4 packets that were sent
*
*         - Expect all packets to pass since gates are opened for all time slots
*         - Expect time slot ID to be added to the mirrored copy DSA<flowId> = time slot ID
*         - Since each packet was sent in different time slot expect each packet
*           to have different flowId
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U32             packetActualLength = 0;
    GT_U8              queue = 0;
    TGF_NET_DSA_STC    rxParam;
    GT_U8              pktNum;
    GT_BOOL            getFirst = GT_TRUE;
    GT_U32             receivedflowId[PRV_TGF_BURST_COUNT_CNS];
    GT_BOOL            isFlowIdOk = GT_TRUE;
    GT_U32             recGateOctetCounter;
    GT_U32             expectedBytesNum;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {
                                                                      {0}, {0}, {0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS};

    /**** Check gate counter ****/

    /* AUTODOC: get octet counter value for the gate that was used */
    rc = prvCpssStreamSgcOctetCountersGet(prvTgfDevNum,
                                          gateInfo.gateId,
                                          gateInfo.tableSetId,
                                          &recGateOctetCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersGet");

    /* AUTODOC: get expected number of bytes per packet */
    expectedBytesNum = TGF_L2_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr) + TGF_CRC_LEN_CNS ;

    /* AUTODOC: now for all packets */
    expectedBytesNum *= PRV_TGF_BURST_COUNT_CNS;

    /* AUTODOC: expect all 4 packets to pass through that gate */
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedBytesNum, recGateOctetCounter,
                                 "Gate Octet counter is not as expected \n");


    /**** Check mirror packets content ****/

    /* AUTODOC: loop over all packets in tests */
    for (pktNum=0; pktNum< PRV_TGF_BURST_COUNT_CNS; pktNum++)
    {
        /* AUTODOC: get next entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst,
                                       GT_TRUE,
                                       packetBuf[pktNum],
                                       &packetLen[pktNum],
                                       &packetActualLength,
                                       &prvTgfDevNum,
                                       &queue,
                                       &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuGet");

        /* AUTODOC: set to false to get next packets in line */
        getFirst = GT_FALSE;

        /* AUTODOC: save received flow ID */
        receivedflowId[pktNum] = rxParam.flowIdTtOffset.flowId;

        /* AUTODOC: Check DSA parameters of mirrored packet */
        UTF_VERIFY_EQUAL2_STRING_MAC(1, rxParam.dsaCmdIsToCpu,
                                     "For packet[%d], expect rxParam.dsaCmdIsToCpu to be '1' but got %d \n",
                                      pktNum, rxParam.dsaCmdIsToCpu);

        UTF_VERIFY_EQUAL2_STRING_MAC(CPSS_NET_USER_DEFINED_5_E, rxParam.cpuCode,
                                     "For packet[%d], expect rxParam.cpuCode to be '505' but got %d \n",
                                      pktNum, rxParam.cpuCode);

        /* AUTODOC: check received flow ID is in valid range */
        if ( rxParam.flowIdTtOffset.flowId > (PRV_TGF_GATE_TIME_SLOTS_NUM_CNS-1) )
        {
            isFlowIdOk = GT_FALSE;
        }
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isFlowIdOk,
                                     "For packet[%d], received flow ID of %d which is bigger than %d (maximum valid value) \n",
                                      pktNum, rxParam.flowIdTtOffset.flowId,PRV_TGF_GATE_TIME_SLOTS_NUM_CNS-1);
    }

    /* AUTODOC: Since packets were sent in different time slots
                Verify flow ID is not the same for all received packets
     */
    if ( (receivedflowId[0] == receivedflowId[1]) &&
         (receivedflowId[0] == receivedflowId[2]) &&
         (receivedflowId[0] == receivedflowId[3]))
    {
        isFlowIdOk = GT_FALSE;
    }
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_TRUE, isFlowIdOk,
                                 "Since packets were sent in different time slots "
                                 "Expect different flow IDs but all received packets have the same flow ID of %d \n",
                                  receivedflowId[0]);
}


/******************************************************************************\
 *   Traffic functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckTraffic function
* @endinternal
*
* @brief   SGC EPCL UDB Metadata Qci slot id traffic.
*
*          - Send 4 packets with 1 time slot duration gap between each packet
*          - Enable capture on target port
*          - Read counters from target port and save them to be verified
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckTraffic
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;
    GT_U32                           portIter;
    GT_U32                           packetIter;

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

    /* AUTODOC: start capture*/
    tgfTrafficTableRxStartCapture(GT_TRUE);

    /* AUTODOC: Set packet info */
    packetInfo.numOfParts =  prvTgfGlobalPacketInfo.numOfParts;
    packetInfo.partsArray =  prvTgfGlobalPacketInfo.partsArray;
    packetInfo.totalLen   =  prvTgfGlobalPacketInfo.totalLen;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* AUTODOC: Send all packets in tests */
    for (packetIter = 0; packetIter < PRV_TGF_BURST_COUNT_CNS; packetIter++)
    {
        /* AUTODOC: Send packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* AUTODOC: Read counters from target port and save them to be verified */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");

        /* AUTODOC: Print debug TOD timing */
        rc = prvCpssStreamSgcDebugTodDump(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvCpssStreamSgcDebugTodDump");

        /* AUTODOC: Sleep for 665 msec which is actually 1msec in emulator time
                    generate delay of 1 time slot duration which was set to 1msec
         */
        cpssOsTimerWkAfter(665);
    }

    /* AUTODOC: stop capture */
    tgfTrafficTableRxStartCapture(GT_FALSE);
}


/******************************************************************************\
 *   Restore functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigRestore function
* @endinternal
*
* @brief   SGC EPCL UDB Metadata Qci slot id configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore PCL Configuration
*          3. Restore gate configuration
*          4. Restore base Configuration
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigRestore
(
    GT_VOID
)
{
    GT_STATUS    rc = GT_OK;
    GT_U32       ruleIndex;
    GT_U32       i;

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
     * 2. Restore PCL Configuration
     */
    ruleIndex = ipclRuleIndex;
    /* AUTODOC: invalidate IPCL rules that were used in test */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, ruleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, RuleIndex = %d", 1);

    /* AUTODOC: invalidate EPCL rules that were used in test */
    ruleIndex = epclRuleIndex;
    for (i=0;i<PRV_TGF_BURST_COUNT_CNS;i++,ruleIndex++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, RuleIndex = %d", 1);
    }

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: Set back PCL Generic Action mode */
    rc = cpssDxChPclGenericActionModeSet(
        prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_NUMBER_0_E, 0, prvTgfRestoreCfg.genericActionMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeSet");

    /* -------------------------------------------------------------------------
     * 3. Restore Gate Configuration
     */
    /* AUTODOC: Set SGC global parameters */
    rc = cpssDxChStreamSgcGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigSet");

    /* AUTODOC: Set back gates parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateInfo.gateId,
                                        gateInfo.tableSetId,
                                        PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                        PRV_TGF_GATE_IMX_PROFILES_NUM_CNS,
                                        &prvTgfRestoreCfg.gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");

    /* AUTODOC: Clear HW gate octet counter */
    rc = prvCpssStreamSgcOctetCountersSet(prvTgfDevNum,
                                          gateInfo.gateId,
                                          gateInfo.tableSetId,
                                          0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");

    /* -------------------------------------------------------------------------
     * 4. Restore base Configuration
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
