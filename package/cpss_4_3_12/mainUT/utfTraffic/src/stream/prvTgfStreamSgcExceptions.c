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
* @file prvTgfStreamSgcExceptions.c
*
* @brief SGC gate exceptions use cases testing.
*
*        Features that are tested:
*
*        Gate Interval Max Octet Exceeded exception:
*           The number of bytes allowed per time slot is budgeted. IntervalMax exceed implies that
*           the number of bytes that passed through the gate on the current time interval exceeded
*           the maximum allowed. Per gate octet counter, counts the transmitted bytes during the
*           current time slot. The gate IMX profile is the index to the maximum transmitted number.
*
*        Gate Closed exception:
*           If packet arrives in specific time slot in which gate assignment is closed an exception
*           should occur.
*
*        Gate byte count adjust:
*           Packet byte count adjust.If BC is known (different than 0x3fff)
*           --> BC = <descriptor>.BC +/- adjust
*
*        Gate statistics:
*           - The legacy Pre-egress CNC "Pass/Drop" Client is used to count the packet Gate-ID statistics.
*             In Ironman, a new mode was added to the EQ Global<Pass Drop CNC Mode>: "Stream Gate Mode".
*             When configured in "Stream Gate Mode", the CNC index format is as follows:
*             - |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
*           - Read gate Octet counter
*
*        3 tests are added to check above features:
*        - prvTgfStreamSgcGateIntervalMaxOctetExceededException
*        - prvTgfStreamSgcGateClosedException
*        - prvTgfStreamSgcGateByteCountAdjust
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
*          - Set Interval max value (all time slots) as:1 packet size < Interval Max < 2 packets size
*          - Set gate state to open for all time slots
*          - Configure gate exception parameters (packet command & drop code)
*        - Send 2 packets:
*           - Send first packet
*           - configure gate again
*             - For gate closed exception test
*               - Close the specific gate for all time slots
*               - Increase Interval Max value for all time slots
*             - For gate byte count adjust test
*               - set byte count adjust to reduce 40 bytes from Octet counter
*           - Send second packet
*        - Check results
*          - read port counters
*          - read CNC counters
*          - read ingress drop counters
*          - read gate Octet counter
*
* @version   1
********************************************************************************
*/
#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStreamGateControl.h>
#include <stream/prvTgfStreamSgcExceptions.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCncGen.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamGateControl.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStreamGateControl.h>


/*************************** Constants definitions ***********************************/

/* Maximum valid Gate ID value */
#define PRV_TGF_SGC_MAX_GATE_ID_NUM_CNS       510

/* Number of Table Sets */
#define PRV_TGF_SGC_MAX_TABLE_SETS_NUM_CNS    58

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS         2

/* Ports index allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS    0
#define PRV_TGF_TARGET_PORT_IDX_CNS     1

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS      1

/* Source ID parameters */
#define SOURCE_ID_MASK_CNS              0xFFF
#define SOURCE_ID_CNS                   0x800


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


/* Ingress Packet: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}   /* saMac */
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
*          Forward to ePort#1 according to MAC destination
*/
static GT_VOID prvTgfStreamSgcBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN ID 1, target port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_DEFAULT_VLANID_CNS,
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
                - Set Interval Max value to 100 bytes for all time slots
                - Set each time slot with time to advance value of +1000 nsec
                - Set each time slot with the profile index to Interval Max Table
                - Do not change packet's TC
                - No length aware
                - Open the gate for all time slots
                - Do not reset the Octet counter in time slot transition
     */
    for (i=0;i<timeSlotsNum;i++)
    {
        gateParams.intervalMaxArr[i]                          = 100;
        gateParams.timeSlotInfoArr[i].timeToAdvance          += 1000;
        gateParams.timeSlotInfoArr[i].intervalMaxOctetProfile = i;
        gateParams.timeSlotInfoArr[i].keepTc                  = GT_FALSE;
        gateParams.timeSlotInfoArr[i].ipv                     = 0x7;
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
* internal prvTgfStreamSgcExceptionsConfigSet function
* @endinternal
*
* @brief   SGC exceptions use case configurations
*/
GT_VOID prvTgfStreamSgcExceptionsConfigSet
(
    GT_VOID
)
{
    /* AUTODOC: Select random Gate ID and TableSet ID for this test */
    prvTgfStreamSgcGateIdAndTableSetIdSelect();

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
* internal prvTgfStreamSgcExceptionsOctetCounterCheck function
* @endinternal
*
* @brief  Read gate Octet counter value
*         Compare the value to the expected value
*/
static GT_VOID prvTgfStreamSgcExceptionsOctetCounterCheck
(
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType
)
{
    GT_STATUS                rc;
    GT_U32                   octetCounterActual,octetCounterExpected=0;

    /* AUTODOC: Get octet counter value for the gate that was used */
    rc = prvCpssStreamSgcOctetCountersGet(prvTgfDevNum, gateId, tableSetId, &octetCounterActual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersGet");

    /* Set expected number of bytes pending test type */
    if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_INTERVAL_MAX_OCTET_EXCEEDED_E)
    {
        /* Expect 2 packets to pass through that gate (add 8 bytes CRC) */
        octetCounterExpected = expectedBytesNum[0] + expectedBytesNum[1] + 8;
    }
    else if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_CLOSED_E)
    {
        /* Since gate is close before sending the second packet expect only the first packet to pass.
           Therefore number of bytes for 1 packet + 4 CRC bytes */
        octetCounterExpected = expectedBytesNum[1] + 4;
    }
    else if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_BYTE_COUNT_ADJUST_E)
    {
        /* Before sending the second packet byte count adjust was set as following:
           - byteCountAdjustPolarity: GT_FALSE (decrease byte count)
           - byteCountAdjust: 40 (40 bytes)
           Therefore number of bytes: first packet + second packet - 40 + 8 CRC bytes */
        octetCounterExpected = expectedBytesNum[0] + expectedBytesNum[1] + 8;

        if(byteCountAdjustPolarity == GT_TRUE)
        {
            octetCounterExpected += byteCountAdjust;
        }
        else
        {
            octetCounterExpected -= byteCountAdjust;
        }
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(octetCounterExpected, octetCounterActual, "Octet counter (gate[%d], tableSet[%d]) is not as expected");
}

/**
* internal prvTgfStreamSgcExceptionsCncCountersGet function
* @endinternal
*
* @brief  Read CNC counters
*/
static GT_VOID prvTgfStreamSgcExceptionsCncCountersGet
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
* internal prvTgfStreamSgcExceptionsVerification function
* @endinternal
*
* @brief  Check counters for the 2 packets that were sent
*         First packet:
*         - expect packet to reach target port since this gate is opened and packet size
*           is less than configured in Interval Max table for this gate and time slot
*         Second packet:
*         - For Gate Interval Max Octet Exceeded test:
*           - Expect packet to be dropped. Bytes number of this packet plus the previous one
*             is larger than configured in Interval Max table for this gate & time slot
*         - For Gate Closed test:
*           - Expect packet to be dropped. The specific gate is closed for all time slots
*         - For Gate Byte Count Adjust test:
*           - Expect packet to pass since byte count adjust was set to reduce 40 bytes
*             from the Octet count and therefore less than Interval Max value
*
*         Read below counters:
*         - read CNC counters
*         - read Ingress drop counter and check drop code
*         - read octet counter table to get actual number of bytes passed through that gate
*/
GT_VOID prvTgfStreamSgcExceptionsVerification
(
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType
)
{
    GT_STATUS                rc = GT_OK;
    GT_U32                   dropCounter;
    GT_U32                   cncBlockNum;
    GT_U32                   expectedPacketsNumCount,expectedBytesNumCount;
    GT_U32                   gateState;
    CPSS_NET_RX_CPU_CODE_ENT dropCodeActual;
    CPSS_NET_RX_CPU_CODE_ENT dropCodeExpected = 0;


    /********************************************************************************************
      Check counters for packets that passed
     ********************************************************************************************/


    if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_BYTE_COUNT_ADJUST_E)
    {
        /* AUTODOC: Read CNC counter */

        /* AUTODOC: Gate is opened (bits[10:9] in CNC index number) */
        gateState   = 0x0;
        /* AUTODOC: CNC block number 0 that is used to count indexes of 0 - 1023 range  */
        cncBlockNum = blockNum0;
        /* AUTODOC: Expect 2 packets to pass */
        expectedPacketsNumCount = 2;
        expectedBytesNumCount   = expectedBytesNum[0] + expectedBytesNum[1] + 8;
        prvTgfStreamSgcExceptionsCncCountersGet(gateState,
                                                cncBlockNum,
                                                expectedPacketsNumCount,
                                                expectedBytesNumCount);

        /* AUTODOC: Check gate Octet byte counter */
        prvTgfStreamSgcExceptionsOctetCounterCheck(testType);

        return;
    }
    else
    {

        /* AUTODOC: Read CNC counter */

        /* AUTODOC: Gate is opened (bits[10:9] in CNC index number) */
        gateState   = 0x0;
        /* AUTODOC: CNC block number 0 that is used to count indexes of 0 - 1023 range  */
        cncBlockNum = blockNum0;
        /* AUTODOC: Expect 1 packet to pass */
        expectedPacketsNumCount = 1;
        expectedBytesNumCount   = expectedBytesNum[0] + 4;
        prvTgfStreamSgcExceptionsCncCountersGet(gateState,
                                                cncBlockNum,
                                                expectedPacketsNumCount,
                                                expectedBytesNumCount);
    }

    /********************************************************************************************
      Check counters for packets that were dropped
     ********************************************************************************************/

    /* AUTODOC: Read CNC counter */

    if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_INTERVAL_MAX_OCTET_EXCEEDED_E)
    {
        /* AUTODOC: Set gate state to Interval Max Exceeded */
        gateState = 0x2;
        /* AUTODOC: Use blockNum1 to catch CNC index in range 1024-2047 */
        cncBlockNum = blockNum1;

        expectedPacketsNumCount = 1;
        expectedBytesNumCount = expectedBytesNum[1] + 4;
    }
    else if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_CLOSED_E)
    {
        /* AUTODOC: Set gate state to Close */
        gateState = 0x1;
        /* AUTODOC: Use blockNum0 to catch CNC index in range 0-1023 */
        cncBlockNum = blockNum0;
    }
    prvTgfStreamSgcExceptionsCncCountersGet(gateState,
                                            cncBlockNum,
                                            expectedPacketsNumCount,
                                            expectedBytesNumCount);

    /* AUTODOC: Check ingress drop counters to match drop code value */

    if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_INTERVAL_MAX_OCTET_EXCEEDED_E)
    {
        /* AUTODOC: Set drop code for Gate Interval Max Octet Exceeded exception  */
        dropCodeExpected = dropCodeGateExceeded;
    }
    else if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_CLOSED_E)
    {
        /* AUTODOC: Set drop code for Gate Closed exception  */
        dropCodeExpected = dropCodeGateClosed;
    }

    /* AUTODOC: Check Ingress packet drop in EQ */
    rc = prvTgfCfgIngressDropCntrGet(&dropCounter);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCfgIngressDropCntrGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, dropCounter, "Ingress drop counter is not as expected");

    /* AUTODOC: Verify the Probe Packet Drop Code at the ingress*/
    rc = cpssDxChCfgProbePacketDropCodeGet(prvTgfDevNum,CPSS_DIRECTION_INGRESS_E,&dropCodeActual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"cpssDxChCfgProbePacketDropCodeGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(dropCodeExpected, dropCodeActual, "Drop code is not as expected");

    /* AUTODOC: Check gate Octet byte counter */
    prvTgfStreamSgcExceptionsOctetCounterCheck(testType);
}


/******************************************************************************\
 *   Traffic functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcExceptionsTraffic function
* @endinternal
*
* @brief   Stream Gate Control exceptions use case traffic.
*
*          - Send 2 packets of 64 bytes each
*            - for Gate closed test after first packet:
*              - close the specific gate
*            - for Gate byte count adjust test after first packet:
*              - set byte count adjust to reduce 40 bytes from Octet counter
*          - Enable capture on target port
*          - Read counters from target port and save them to be verified
*/
GT_VOID prvTgfStreamSgcExceptionsTraffic
(
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType
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
    expectedBytesNum[0] = sizeof(prvTgfPacketL2Part) + sizeof(prvTgfShortPayloadDataArr);

    /* AUTODOC: Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* AUTODOC: Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[0] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[0] = portCntrs.goodOctetsRcv.l[0];

    /**** Change gate configuration before sending second packet ****/

    if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_CLOSED_E)
    {
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

        /* AUTODOC: Go over all time slots and close the gate */
        for (i=0;i<timeSlotsNum;i++)
        {
            gateParams.intervalMaxArr[i]                  = 500;      /* 500 bytes */
            gateParams.timeSlotInfoArr[i].streamGateState = CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E;
        }

        /* AUTODOC: Call SGC API to set these parameters */
        rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                            gateId,
                                            tableSetId,
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");
    }
    else if (testType == STREAM_SGC_EXCEPTION_TESTS_GATE_BYTE_COUNT_ADJUST_E)
    {
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

        byteCountAdjustPolarity = GT_FALSE;
        byteCountAdjust = 40;
        gateParams.tableSetInfo.byteCountAdjustPolarity = byteCountAdjustPolarity;
        gateParams.tableSetInfo.byteCountAdjust         = byteCountAdjust;

        /* AUTODOC: Call SGC API to set these parameters */
        rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                            gateId,
                                            tableSetId,
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");
    }

    PRV_UTF_LOG0_MAC("======= Send second packet =======\n");

    /* AUTODOC: Update packet size */
    expectedBytesNum[1] = sizeof(prvTgfPacketL2Part) + sizeof(prvTgfShortPayloadDataArr);

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
* @internal prvTgfStreamSgcExceptionsConfigRestore function
* @endinternal
*
* @brief   Stream Gate Control configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore gate counters Configuration
*          3. Restore PCL Configuration
*          4. Restore gate configuration
*          5. Restore base Configuration
*/
GT_VOID prvTgfStreamSgcExceptionsConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;
    GT_U32                           gateState;

    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

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
