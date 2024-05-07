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
* @file prvTgfStreamSgcGateTimeSlotsCheck.c
*
* @brief SGC gate time slots functionality testing.
*
*        Features that are tested:
*
*        Time slot functionality:
*           - Use only 2 time slots with different parameters for each timeslot
*           - Check packet behavior for each time slot
*           - Check tableSet is indeed wrapped
*
*        Debug TOD:
*           - Read from debug TOD HW registers to get TOD timing
*
*        Gate statistics:
*           - The legacy Pre-egress CNC "Pass/Drop" Client is used to count the packet Gate-ID statistics.
*             In Ironman, a new mode was added to the EQ Global<Pass Drop CNC Mode>: "Stream Gate Mode".
*             When configured in "Stream Gate Mode", the CNC index format is as follows:
*             - |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
*           - Read gate Octet counter
*
*        1 test is added to check above features:
*        - prvTgfStreamSgcGateTimeSlotsCheck
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
*        - Configure only 2 time slots
*          - Set first time slot to open gate
*          - Set second time slot to close gate
*          - Set tableSet cycle time to 2 time slots * time slot duration
*        - Set IPCL rule for each packet
*          - assign the configured gate to all packets
*        - Send 4 packets with 1 time slot duration gap between each packet
*          this is done so each packet will be sent on different time slot
*        - Check results
*          since packets are sent each time in different time slot expect
*          first packet to pass then next packet to drop and so on.
*          so if we send 4 packets expect at least 1 to pass and 1 to drop.
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
#include <stream/prvTgfStreamSgcGateTimeSlotsCheck.h>
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
#define PRV_TGF_GATE_TIME_SLOTS_NUM_CNS       2

/* Number of gate interval max to use in the test */
#define PRV_TGF_GATE_IMX_PROFILES_NUM_CNS     2


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
    GT_U32    expPktsNum;   /* expected number of packets for gate ID */
    GT_U32    expBytesNum;  /* expected number of bytes for gate ID   */
} TGF_GATE_INFO_STC;

/* Gate information
   - 2 time slots, first is set to open second is set to close
   - 4 packets are sent while each packet is in different time slot
   - expect at least 1 packet to pass and 1 packet to drop
   */
static TGF_GATE_INFO_STC  gateInfo = {
/* gateId, tableSetId, expPktsNum, expBytesNum */
      270,         40,          1,        1*68
};

/* CNC block 0 that is used in test to count cnc indexes from 0 - 1023 */
static GT_U32    blockNum0 = 0;

/* Target port interface information */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Packet send information */
static TGF_PACKET_STC   packetInfo;

/* IPCL rule ID */
static  GT_U32    ruleId = 0;


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
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT               genericActionMode;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC                  gateParams;
    CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT passDropMode;
    CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC              globalParams;
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
*          - Set IPCL rule: according to MAC DA
*          - Set IPCL actions with 802.1Qci parameters
*            - Set redirect command to work with Generic action of 802.1Qci
*            - Set 12 bits generic action data with | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) |
*            - assign the configured gate to all packets
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

    /* AUTODOC: Set PCL actions */

    /* AUTODOC: Clear action structure */
    cpssOsMemSet(&action, 0, sizeof(action));

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

    /* AUTODOC: Set PCL rule */
    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                          ruleId,
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
* @internal prvTgfStreamSgcGateConfigSet function
* @endinternal
*
* @brief   Gate configurations
*          Configure one gate with 2 time slots
*          - Set first time slot to open gate
*          - Set second time slot to close gate
*          - Set tableSet cycle time to 2 time slots * time slot duration
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
    CPSS_DXCH_STREAM_SGC_GATE_STATE_ENT       gateStateArr[2] =
                                                { CPSS_DXCH_STREAM_SGC_GATE_STATE_OPEN_E,
                                                  CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E };

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
                - Set Interval Max to big enough value (100000 bytes) to pass 10 packets
                - Set each time slot with time to advance value of +1 mili sec
                - Set each time slot with the profile index to Interval Max Table
                - Do not change packet's TC
                - No length aware
                - Close the gate for all time slots
                - Do not reset the Octet counter in time slot transition
                - Set gate to open for first time slot and gate close for second time slot
     */
    for (i=0;i<PRV_TGF_GATE_TIME_SLOTS_NUM_CNS;i++)
    {
        gateParams.intervalMaxArr[i]                          = 100000;
        gateParams.timeSlotInfoArr[i].timeToAdvance           = (i+1)*(1000000); /*1 mili seconds*/
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_TRUE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x4;
        gateParams.timeSlotInfoArr[i].lengthAware             = GT_FALSE;
        gateParams.timeSlotInfoArr[i].streamGateState         = gateStateArr[i];
        gateParams.timeSlotInfoArr[i].newSlot                 = GT_FALSE;
    }

    /* AUTODOC: Set Table Set time configuration parameters. */
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_FALSE;
    gateParams.tableSetInfo.byteCountAdjust         = 0;
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.remainingBitsFactor     = 255;        /* 255 bits every 128nsec */
    gateParams.tableSetInfo.cycleTime = 1000000*PRV_TGF_GATE_TIME_SLOTS_NUM_CNS; /*2 mili seconds*/
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

    /* AUTODOC: Set CNC block number 0 to be used for cnc indexes 0-1023 */
    cncCfg.blockNum = blockNum0;

    /**** CNC counter for open gate (gate state:0) ****/
    /* AUTODOC: Set CNC counter index in 802.1Qci format:
                |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
                modulo block size since index can not be larger than maximum block size */
    cncCfg.counterNum = ( ((0<<9) | gateInfo.gateId) % blockSize);

    /* AUTODOC: Call CNC API to apply above parameters */
    rc = prvTgfCncGenConfigure(&cncCfg, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenConfigure");

    /* AUTODOC: Clear this counter by reading it */
    rc = prvTgfCncCounterGet(cncCfg.blockNum,
                             cncCfg.counterNum,
                             cncCfg.counterFormat,
                             &counterValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    /**** CNC counter for close gate (gate state:1) ****/
    /* AUTODOC: Set CNC counter index in 802.1Qci format:
                |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
                modulo block size since index can not be larger than maximum block size */
    cncCfg.counterNum = ( ((1<<9) | gateInfo.gateId) % blockSize);

    /* AUTODOC: Call CNC API to apply above parameters */
    rc = prvTgfCncGenConfigure(&cncCfg, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenConfigure");

    /* AUTODOC: Clear this counter by reading it */
    rc = prvTgfCncCounterGet(cncCfg.blockNum,
                             cncCfg.counterNum,
                             cncCfg.counterFormat,
                             &counterValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    /* AUTODOC: Clear HW gate octet counter */
    rc = prvCpssStreamSgcOctetCountersSet(prvTgfDevNum,
                                          gateInfo.gateId,
                                          gateInfo.tableSetId,
                                          0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");
}

/**
* internal prvTgfStreamSgcGateTimeSlotsCheckConfigSet function
* @endinternal
*
* @brief   SGC gate time slots configurations
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckConfigSet
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

    /* AUTODOC: Gate statistics configuration */
    prvTgfStreamSgcStatisticsConfigSet();
}


/******************************************************************************\
 *   Verification functions                                                   *
\******************************************************************************/

/**
* internal prvTgfStreamSgcOctetCounterCheck function
* @endinternal
*
* @brief  Read gate Octet counter value
*         Compare the value to the expected value
*/
static GT_VOID prvTgfStreamSgcOctetCounterCheck
(
    TGF_GATE_INFO_STC    *gateInfo
)
{
    GT_STATUS    rc;
    GT_U32       octetCounterActual;
    GT_BOOL      isCountersOk = GT_TRUE;

    /* AUTODOC: Get octet counter value for the gate that was used */
    rc = prvCpssStreamSgcOctetCountersGet(prvTgfDevNum,
                                          gateInfo->gateId,
                                          gateInfo->tableSetId,
                                          &octetCounterActual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersGet");

    /* AUTODOC: Compare to expected */
    if (octetCounterActual < gateInfo->expBytesNum)
    {
        isCountersOk = GT_FALSE;
    }
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_TRUE, isCountersOk,
                                 "Octet counter of gate:%d with tableSet:%d is not as expected \n"
                                 "Expected at least %d bytes but received only %d bytes \n",
                                 gateInfo->gateId,gateInfo->tableSetId,gateInfo->expBytesNum,octetCounterActual);
}

/**
* internal prvTgfStreamSgcCncCountersGet function
* @endinternal
*
* @brief  Read CNC counters
*/
static GT_VOID prvTgfStreamSgcCncCountersCheck
(
    TGF_GATE_INFO_STC    *gateInfo,
    GT_U32               cncBlockNum
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   blockSize;
    GT_U32                   counterNum;
    PRV_TGF_CNC_COUNTER_STC  counterValue;
    GT_BOOL                  isCountersOk = GT_TRUE;

    /* AUTODOC: Get CNC block maximum mumber of entries */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    /**** Check counter for open gate */
    /* AUTODOC: Modulo block size since index can not be larger than maximum block size  */
    counterNum = ( ((0/*gate open*/ << 9) | gateInfo->gateId) % blockSize);

    rc = prvTgfCncCounterGet(cncBlockNum,
                             counterNum,
                             PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
                             &(counterValue));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    /* AUTODOC: Compare to expected */
    if (counterValue.packetCount.l[0] < gateInfo->expPktsNum)
    {
        isCountersOk = GT_FALSE;
    }
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isCountersOk,
                                 "CNC counter 0x%x (gate open) is not as expected \n"
                                 "Expected at least %d packets to pass but received only %d packets \n",
                                 counterNum,gateInfo->expPktsNum,counterValue.packetCount.l[0]);

    /**** Check counter for close gate */
    /* AUTODOC: Modulo block size since index can not be larger than maximum block size  */
    counterNum = ( ((1/*gate close*/ << 9) | gateInfo->gateId) % blockSize);
    rc = prvTgfCncCounterGet(cncBlockNum,
                             counterNum,
                             PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E,
                             &(counterValue));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");

    if (counterValue.packetCount.l[0] < gateInfo->expPktsNum)
    {
        isCountersOk = GT_FALSE;
    }
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_TRUE, isCountersOk,
                                 "CNC counter 0x%x (gate close) is not as expected \n"
                                 "Expected at least %d packets to drop but received only %d packets \n",
                                 counterNum,gateInfo->expPktsNum,counterValue.packetCount.l[0]);
}

/**
* internal prvTgfStreamSgcGateTimeSlotsCheckVerification function
* @endinternal
*
* @brief  Check packets and counters for the 4 packets that were sent
*
*         - Expect each packet to arrive in different time slot
*         - Since only 2 time slots configured one to pass and second to drop
*           for 4 packets expect 2 packets to pass and 2 packets to drop
*         - Since not sure each packet will indeed arrive at different time slot
*           to be on the safe side to pass the test expect at least 1 packet to
*           pass and 1 packet to drop
*
*         Read below counters:
*         - read CNC counters
*         - read octet counter table to get actual number of bytes passed through that gates
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckVerification
(
    GT_VOID
)
{
    GT_U32    cncBlockNum;

    /**** Check counters ****/

    /* AUTODOC: CNC block number 0 that is used to count indexes of 0 - 1023 range  */
    cncBlockNum = blockNum0;

    /* AUTODOC: Read and check CNC counter */
    prvTgfStreamSgcCncCountersCheck(&gateInfo, cncBlockNum);

    /* AUTODOC: Read and check Gate Octet counter */
    prvTgfStreamSgcOctetCounterCheck(&gateInfo);
}


/******************************************************************************\
 *   Traffic functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcGateTimeSlotsCheckTraffic function
* @endinternal
*
* @brief   SGC gate time slots traffic.
*
*          - Send 4 packets with 1 time slot duration gap between each packet
*          - Enable capture on target port
*          - Read counters from target port and save them to be verified
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckTraffic
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

    /* AUTODOC: Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

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

    /* AUTODOC: Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}


/******************************************************************************\
 *   Restore functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcGateTimeSlotsCheckConfigRestore function
* @endinternal
*
* @brief   SGC gate time slots configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore gate statistics Configuration
*          3. Restore PCL Configuration
*          4. Restore gate configuration
*          5. Restore base Configuration
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;
    GT_U32                           blockSize;
    PRV_TGF_CNC_COUNTER_STC          counterValue;

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
     * 2. Restore gate counters Configuration
     */
    /* AUTODOC: Clear CNC configuration structure */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));

    /* AUTODOC: Set CNC client */
    cncCfg.clientType = PRV_TGF_CNC_CLIENT_PACKET_TYPE_PASS_DROP_E;

    /* AUTODOC: Set same CNC block number that was set in test */
    cncCfg.blockNum = blockNum0;

    /* AUTODOC: Set CNC counter format to mode 0 */
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* AUTODOC: Disable CNC client */
    cncCfg.clientEnable = GT_FALSE;

    /* AUTODOC: Reset CNC counter index range */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;

    /* AUTODOC: Get block maximum mumber of entries */
    blockSize = prvTgfCncFineTuningBlockSizeGet();

    /**** CNC counter for open gate (gate state:0) ****/
    /* AUTODOC: Set CNC counter index in 802.1Qci format:
                |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
                modulo block size since index can not be larger than maximum block size */
    cncCfg.counterNum = ( ((0<<9) | gateInfo.gateId) % blockSize);

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(&cncCfg, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncGenConfigure");

    /* AUTODOC: Clear HW CNC counter by reading it */
    rc = prvTgfCncCounterGet(cncCfg.blockNum,
                             cncCfg.counterNum,
                             cncCfg.counterFormat,
                             &counterValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");


    /**** CNC counter for close gate (gate state:1) ****/
    /* AUTODOC: Set CNC counter index in 802.1Qci format:
                |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
                modulo block size since index can not be larger than maximum block size */
    cncCfg.counterNum = ( ((1<<9) | gateInfo.gateId) % blockSize);

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(&cncCfg, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncGenConfigure");

    /* AUTODOC: Clear HW CNC counter by reading it */
    rc = prvTgfCncCounterGet(cncCfg.blockNum,
                             cncCfg.counterNum,
                             cncCfg.counterFormat,
                             &counterValue);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncCounterGet");


    /* AUTODOC: Clear HW gate octet counter */
    rc = prvCpssStreamSgcOctetCountersSet(prvTgfDevNum,
                                          gateInfo.gateId,
                                          gateInfo.tableSetId,
                                          0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");


    /* AUTODOC: Set packet pass/drop mode */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,
                                                   prvTgfRestoreCfg.passDropMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeGet");

    /* -------------------------------------------------------------------------
     * 3. Restore PCL Configuration
     */
    /* AUTODOC: invalidate all PCL rules that were used in test */
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

    /* AUTODOC: Set back gates parameters */
    rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                        gateInfo.gateId,
                                        gateInfo.tableSetId,
                                        PRV_TGF_GATE_TIME_SLOTS_NUM_CNS,
                                        PRV_TGF_GATE_IMX_PROFILES_NUM_CNS,
                                        &prvTgfRestoreCfg.gateParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");

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
