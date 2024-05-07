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
* @file prvTgfStreamSgcMultipleGates.c
*
* @brief SGC multiple gates use case testing.
*
*        Features that are tested:
*
*        Multiple gates:
*           - Use different combinations of gates and tableSets
*
*        Gate statistics:
*           - The legacy Pre-egress CNC "Pass/Drop" Client is used to count the packet Gate-ID statistics.
*             In Ironman, a new mode was added to the EQ Global<Pass Drop CNC Mode>: "Stream Gate Mode".
*             When configured in "Stream Gate Mode", the CNC index format is as follows:
*             - |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
*           - Read gate Octet counter
*
*        1 test is added to check above features:
*        - prvTgfStreamSgcMultipleGates
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
*        - Configure 4 different gates.
*          - Set Interval max values to be large enough to pass 2 packets
*          - Set gate state to open for all time slots
*        - Set IPCL rule for each packet
*          - assign gate id# 509 to first packet
*          - assign gate id# 510 to second packet
*          - assign gate id# 100 to third packet
*          - assign gate id# 0 to fourth packet
*          - assign gate id# 509 to fifth packet
*        - Send 5 packets with different sizes
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
#include <stream/prvTgfStreamSgcMultipleGates.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCncGen.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamGateControl.h>


/*************************** Constants definitions ***********************************/

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS               5

/* Ports index allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS          0
#define PRV_TGF_TARGET_PORT_IDX_CNS           1

/* VLAN Id #6 */
#define PRV_TGF_VLANID_6_CNS                  6

/* Maximum number of bytes of output packet (CRC included) */
#define PRV_TGF_MAX_PACKET_SIZE_CNS           96

/* Number of gates that are used in the test */
#define PRV_TGF_GATES_NUM_CNS                 4

/* Number of PCL rules that are used in the test */
#define PRV_TGF_PCL_RULES_NUM_CNS             5


/*************************** static DB definitions ***********************************/

/* Received packet counters in the test */
typedef struct
{
    GT_U32    pktsNum;
    GT_U32    bytesNum;
} TGF_PACKET_COUNTERS_STC;

/* Struct to hold packet information */
typedef struct
{
    GT_U32                   totalLen;           /* not include CRC                  */
    GT_U32                   numOfParts;         /* num of elements in of partsArray */
    TGF_PACKET_PART_STC      *partsArray;        /* partsArray data                  */
    TGF_PACKET_PAYLOAD_STC   *packetPayloadPart; /* packet payload information       */
    TGF_PACKET_L2_STC        *packetL2Part;      /* packet L2 information            */
    TGF_PACKET_COUNTERS_STC  *receivedPacket;    /* actual received packet counters  */
} TGF_PACKET_INFO_STC;

/* Gate information to use in test */
typedef struct
{
    GT_U32    gateId;       /* gate ID                                */
    GT_U32    tableSetId;   /* tableSet ID to be mapped to gate ID    */
    GT_U32    gateState;    /* gate state                             */
    GT_U32    expPktsNum;   /* expected number of packets for gate ID */
    GT_U32    expBytesNum;  /* expected number of bytes for gate ID   */
} TGF_GATE_INFO_STC;

/* Array of gates */
static TGF_GATE_INFO_STC  gateInfoArr[PRV_TGF_GATES_NUM_CNS] = {
/*   gateId, tableSetId, gateState, expPktsNum, expBytesNum   */
    {   509,         57,         0,          0,           0},    /* 1st gate */
    {   510,         57,         0,          0,           0},    /* 2nd gate */
    {   100,         10,         0,          0,           0},    /* 3rd gate */
    {     0,          0,         0,          0,           0}     /* 4th gate */
};

/* Number of time slots to use in the test */
static GT_U32    timeSlotsNum = CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

/* Number of interval max profiles to use in the test */
static GT_U32    imxProfilesNum = CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS;

/* CNC block 0 that is used in test to count cnc indexes from 0 - 1023 */
static GT_U32    blockNum0 = 0;

/* Target port interface information */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Array to hold actual received packet counters */
static TGF_PACKET_COUNTERS_STC prvTgfReceivedPacketCountersArr[PRV_TGF_BURST_COUNT_CNS] = {
/* pktsNum, bytesNum */
   {     0,        0},  /* 1st packet */
   {     0,        0},  /* 2nd packet */
   {     0,        0},  /* 3rd packet */
   {     0,        0},  /* 4th packet */
   {     0,        0}   /* 5th packet */
};

/* Packet send information */
static TGF_PACKET_STC   packetInfo;


/**** Packets definitions ****/

/**** Packet#0 ****/
/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacket0L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x03}   /* saMac */
};

/* Packet's payload (including EtherType) */
static GT_U8 prvTgfPayloadData0Arr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket0PayloadPart = {
    sizeof(prvTgfPayloadData0Arr),    /* dataLength */
    prvTgfPayloadData0Arr             /* dataPtr */
};

/* Packet#0 parts */
static TGF_PACKET_PART_STC prvTgfPacket0PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket0L2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket0PayloadPart}
};

/**** Packet#1 ****/
/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacket1L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x22, 0x03}   /* saMac */
};

/* Packet's payload (including EtherType) */
static GT_U8 prvTgfPayloadData1Arr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35/* length 56 bytes */
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket1PayloadPart = {
    sizeof(prvTgfPayloadData1Arr),    /* dataLength */
    prvTgfPayloadData1Arr             /* dataPtr */
};

/* Packet#1 parts */
static TGF_PACKET_PART_STC prvTgfPacket1PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket1L2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket1PayloadPart}
};

/**** Packet#2 ****/
/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacket2L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x33, 0x03}   /* saMac */
};

/* Packet's payload (including EtherType) */
static GT_U8 prvTgfPayloadData2Arr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d/* length 64 bytes */
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPayloadData2Arr),    /* dataLength */
    prvTgfPayloadData2Arr             /* dataPtr */
};

/* Packet#2 parts */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket2L2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PayloadPart}
};

/**** Packet#3 ****/
/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacket3L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x03}   /* saMac */
};

/* Packet's payload (including EtherType) */
static GT_U8 prvTgfPayloadData3Arr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,
    0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45/* length 72 bytes */
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket3PayloadPart = {
    sizeof(prvTgfPayloadData3Arr),    /* dataLength */
    prvTgfPayloadData3Arr             /* dataPtr */
};

/* Packet#3 parts */
static TGF_PACKET_PART_STC prvTgfPacket3PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket3L2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket3PayloadPart}
};

/**** Packet#4 ****/
/* L2 part */
static TGF_PACKET_L2_STC prvTgfPacket4L2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x55, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x55, 0x03}   /* saMac */
};

/* Packet's payload (including EtherType) */
static GT_U8 prvTgfPayloadData4Arr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,
    0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d/* length 80 bytes */
};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket4PayloadPart = {
    sizeof(prvTgfPayloadData4Arr),    /* dataLength */
    prvTgfPayloadData4Arr             /* dataPtr */
};

/* Packet#4 parts */
static TGF_PACKET_PART_STC prvTgfPacket4PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacket4L2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket4PayloadPart}
};

/* Global packet Information structure */
static TGF_PACKET_INFO_STC  prvTgfGlobalPacketInfo[PRV_TGF_BURST_COUNT_CNS] =
{
        /* Packet#0 */
        {
            TGF_PACKET_AUTO_CALC_LENGTH_CNS,
            sizeof(prvTgfPacket0PartArray)/sizeof(TGF_PACKET_PART_STC),
            prvTgfPacket0PartArray,
            &prvTgfPacket0PayloadPart,
            &prvTgfPacket0L2Part,
            &prvTgfReceivedPacketCountersArr[0]
        },
        /* Packet#1 */
        {
            TGF_PACKET_AUTO_CALC_LENGTH_CNS,
            sizeof(prvTgfPacket1PartArray)/sizeof(TGF_PACKET_PART_STC),
            prvTgfPacket1PartArray,
            &prvTgfPacket1PayloadPart,
            &prvTgfPacket1L2Part,
            &prvTgfReceivedPacketCountersArr[1]
        },
        /* Packet#2 */
        {
            TGF_PACKET_AUTO_CALC_LENGTH_CNS,
            sizeof(prvTgfPacket2PartArray)/sizeof(TGF_PACKET_PART_STC),
            prvTgfPacket2PartArray,
            &prvTgfPacket2PayloadPart,
            &prvTgfPacket2L2Part,
            &prvTgfReceivedPacketCountersArr[2]
        },
        /* Packet#3 */
        {
            TGF_PACKET_AUTO_CALC_LENGTH_CNS,
            sizeof(prvTgfPacket3PartArray)/sizeof(TGF_PACKET_PART_STC),
            prvTgfPacket3PartArray,
            &prvTgfPacket3PayloadPart,
            &prvTgfPacket3L2Part,
            &prvTgfReceivedPacketCountersArr[3]
        },
        /* Packet#4 */
        {
            TGF_PACKET_AUTO_CALC_LENGTH_CNS,
            sizeof(prvTgfPacket4PartArray)/sizeof(TGF_PACKET_PART_STC),
            prvTgfPacket4PartArray,
            &prvTgfPacket4PayloadPart,
            &prvTgfPacket4L2Part,
            &prvTgfReceivedPacketCountersArr[4]
        }
};

/* Parameters that are needed to be restored */
static struct
{
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT               genericActionMode;
    CPSS_DXCH_STREAM_SGC_GATE_INFO_STC                  gateParamsArr[PRV_TGF_BURST_COUNT_CNS];
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
    GT_U32       packetIter;

    /* AUTODOC: Save default VLAN 1 */
    prvTgfBrgVlanEntryStore(1);

    /* AUTODOC: Loop over all packets in tests and forward them to target port */
    for (packetIter=0; packetIter<PRV_TGF_BURST_COUNT_CNS;packetIter++)
    {
        rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfGlobalPacketInfo[packetIter].packetL2Part->daMac,
                                              PRV_TGF_VLANID_6_CNS,
                                              prvTgfDevNum,
                                              prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                              GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
    }
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
*              - Set first packet to gate ID 509
*              - Set second packet to gate ID 510
*              - Set third packet to gate ID 100
*              - Set fourth packet to gate ID 0
*              - Set fifth packet to gate ID 509
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
    GT_U32                                   ruleId;

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

    /* AUTODOC: Loop over all PCL rules in tests */
    for (ruleId=0; ruleId<PRV_TGF_PCL_RULES_NUM_CNS; ruleId++)
    {
        /* AUTODOC: Pattern for MAC DA address */
        cpssOsMemSet(&pattern, 0, sizeof(pattern));
        cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther,
                     prvTgfGlobalPacketInfo[ruleId].packetL2Part->daMac,
                     TGF_L2_HEADER_SIZE_CNS/2);

        /* AUTODOC: Set generic action data with 12 bits:
                   | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) | */
        pclQciActions = (cncIndxMode << 11) | (maxSduSizeProfileIndx << 9) | \
                        (gateInfoArr[ruleId % PRV_TGF_GATES_NUM_CNS].gateId);
        action.redirect.data.genericActionInfo.genericAction = pclQciActions ;

        /* AUTODOC: Set PCL rule */
        rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                              ruleId,
                              &mask,
                              &pattern,
                              &action);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStreamGateControlPclConfigSet");
    }

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
*          Configure 4 different gates with different TableSets
*          - gate ID 509 mapped to table Set ID 57
*          - gate ID 510 mapped to table Set ID 57
*          - gate ID 100 mapped to table Set ID 10
*          - gate ID 0 mapped to table Set ID 0
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
    GT_U32                                    gateNum;

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
    gateParams.tableSetInfo.byteCountAdjustPolarity = GT_FALSE;
    gateParams.tableSetInfo.byteCountAdjust         = 0;
    gateParams.tableSetInfo.remainingBitsResolution = CPSS_DXCH_STREAM_SGC_TABLE_SET_REMAINING_BITS_RESOLUTION_128NS_E;
    gateParams.tableSetInfo.remainingBitsFactor     = 255;        /* 255 bits every 128nsec */
    cycleTime = 1000 * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;
    gateParams.tableSetInfo.cycleTime = cycleTime;                /* 1000nsec * 256 time slots */
    gateParams.tableSetInfo.ingressTimestampOffset  = 0;
    gateParams.tableSetInfo.ingressTodOffset        = 0;

    /* AUTODOC: Loop over all gates in tests */
    for (gateNum=0; gateNum<PRV_TGF_GATES_NUM_CNS;gateNum++)
    {
        /* AUTODOC: Save gate parameters for restore */
        rc = cpssDxChStreamSgcGateConfigGet(prvTgfDevNum,
                                            gateInfoArr[gateNum].gateId,
                                            gateInfoArr[gateNum].tableSetId,
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &prvTgfRestoreCfg.gateParamsArr[gateNum]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigGet");

        /* AUTODOC: Call API to set these parameters */
        rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                            gateInfoArr[gateNum].gateId,
                                            gateInfoArr[gateNum].tableSetId,
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &gateParams);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");
   }
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
    GT_U32                           gateNum;

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

    /* AUTDOC: Loop over all gates to set CNC counter number per gate */
    for (gateNum=0;gateNum<PRV_TGF_GATES_NUM_CNS;gateNum++)
    {
        /* AUTODOC: Set CNC counter index in 802.1Qci format:
                    |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
                    modulo block size since index can not be larger than maximum block size */
        cncCfg.counterNum = ( ((gateInfoArr[gateNum].gateState<<9) | gateInfoArr[gateNum].gateId) % blockSize);

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
                                              gateInfoArr[gateNum].gateId,
                                              gateInfoArr[gateNum].tableSetId,
                                              0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");
    }
}

/**
* internal prvTgfStreamSgcMultipleGatesConfigSet function
* @endinternal
*
* @brief   SGC multiple gates use case configurations
*/
GT_VOID prvTgfStreamSgcMultipleGatesConfigSet
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
* internal tgfStreamSgcPacketVerification function
* @endinternal
*
* @brief   Check content of packet
*         - Check MAC addresses of passenger
*         - Check payload data of passenger
*/
static GT_VOID tgfStreamSgcPacketVerification
(
    IN  GT_U8  * packetBuf,
    IN  GT_U32 packetNum
)
{
    GT_U32              pktOffset,payloadPktOffset;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U32              recPassengerPayload, expPassengerPayload;

    PRV_UTF_LOG0_MAC("======= Check content of output packet  =======\n");

    /**** Check MAC DA SA ****/
    /* AUTODOC: Set packet offset to point to packet MAC DA  */
    pktOffset =0;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfPacket0L2Part.daMac)];

        /* AUTODOC: Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,"Failure in tgfStreamGateControlPacketVerification: Mismatch in MAC DA address\n");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[0],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[1],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[2],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[3],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[4],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->daMac[5],
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
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[0],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[1],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[2],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[3],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[4],
                          prvTgfGlobalPacketInfo[packetNum].packetL2Part->saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }

    /**** Check Passenger payload ****/
    /* AUTODOC: Increment offset by 12B of MACs to point to payload part (include EtherType) */
    pktOffset += TGF_L2_HEADER_SIZE_CNS;

    /* AUTODOC: Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0;i<prvTgfGlobalPacketInfo[packetNum].packetPayloadPart->dataLength;i+=4)
    {
        /* AUTODOC: Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* AUTODOC: Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

        /* AUTODOC: Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfGlobalPacketInfo[packetNum].packetPayloadPart->dataPtr[i+3] |
                                      (prvTgfGlobalPacketInfo[packetNum].packetPayloadPart->dataPtr[i+2] << 8) |
                                      (prvTgfGlobalPacketInfo[packetNum].packetPayloadPart->dataPtr[i+1] << 16) |
                                      (prvTgfGlobalPacketInfo[packetNum].packetPayloadPart->dataPtr[i] << 24) );

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
    TGF_GATE_INFO_STC    *gateInfo
)
{
    GT_STATUS    rc;
    GT_U32       octetCounterActual;

    /* AUTODOC: Get octet counter value for the gate that was used */
    rc = prvCpssStreamSgcOctetCountersGet(prvTgfDevNum,
                                          gateInfo->gateId,
                                          gateInfo->tableSetId,
                                          &octetCounterActual);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersGet");
    UTF_VERIFY_EQUAL2_STRING_MAC(gateInfo->expBytesNum, octetCounterActual, "Octet counter of gate:%d with tableSet:%d is not as expected",
                                 gateInfo->gateId,gateInfo->tableSetId);
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
    cncCheck.counterNum = ( ((gateInfo->gateState << 9) | gateInfo->gateId) % blockSize);

    /* AUTODOC: Expected number of packets */
    cncCheck.counterValue.packetCount.l[0] = gateInfo->expPktsNum;
    /* AUTODOC: Expect number of bytes (include CRC bytes) */
    cncCheck.counterValue.byteCount.l[0] = gateInfo->expBytesNum;

    /* AUTODOC: Call CNC API to read CNC counter */
    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenCheck");
}

/**
* internal prvTgfStreamSgcMultipleGatesVerification function
* @endinternal
*
* @brief  Check packets and counters for the 5 packets that were sent
*
*         - Expect all 5 packets to reach target port since all packets
*           were mapped to gates that are opened.
*         - Expect first and last packets to go through gate# 509 and therefore
*           gate Octet counter should hold number of bytes of 2 packets.
*           Rest of Octet counters should hold number of bytes of 1 packet, each
*           packet with different size.
*           - Gate#509: packet#0 + packet#4
*           - Gate#510: packet#1
*           - Gate#100: packet#2
*           - Gate#0  : packet#3
*         - Check packet content
*
*         Read below counters:
*         - read CNC counters
*         - read octet counter table to get actual number of bytes passed through that gates
*/
GT_VOID prvTgfStreamSgcMultipleGatesVerification
(
    GT_VOID
)
{
    GT_STATUS          rc = GT_OK;
    GT_U32             cncBlockNum;
    GT_U32             packetActualLength = 0;
    GT_U8              queue = 0;
    TGF_NET_DSA_STC    rxParam;
    GT_U8              pktNum;
    GT_BOOL            getFirst = GT_TRUE;
    GT_U32             gateNum;
    GT_U8              packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {
                                                                  {0}, {0}, {0}, {0}, {0}};
    GT_U32             packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS,
                                                             PRV_TGF_MAX_PACKET_SIZE_CNS};

    /**** Check packets content ****/

    /* AUTODOC: Loop over all packets in tests */
    for (pktNum=0; pktNum< PRV_TGF_BURST_COUNT_CNS; pktNum++)
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
        tgfStreamSgcPacketVerification(packetBuf[pktNum],pktNum);

        /* AUTODOC: Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }


    /**** Check counters ****/

    /* AUTODOC: CNC block number 0 that is used to count indexes of 0 - 1023 range  */
    cncBlockNum = blockNum0;

    /* AUTODOC: Loop over all gates to read CNC & Gate counters per gate */
    for (gateNum=0; gateNum< PRV_TGF_GATES_NUM_CNS; gateNum++)
    {
        /* AUTODOC: Read and check CNC counter */
        prvTgfStreamSgcCncCountersCheck(&gateInfoArr[gateNum], cncBlockNum);

        /* AUTODOC: Read and check Gate Octet counter */
        prvTgfStreamSgcOctetCounterCheck(&gateInfoArr[gateNum]);
    }
}


/******************************************************************************\
 *   Traffic functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcMultipleGatesTraffic function
* @endinternal
*
* @brief   SGC multiple gates use case traffic.
*
*          - Send 5 packets with different sizes using different gates
*          - Enable capture on target port
*          - Read counters from target port and save them to be verified
*/
GT_VOID prvTgfStreamSgcMultipleGatesTraffic
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC         portCntrs;
    GT_U32                                portIter;
    GT_U32                                packetIter;

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

    /* AUTODOC: Send all packets in tests */
    for (packetIter = 0; packetIter < PRV_TGF_BURST_COUNT_CNS; packetIter++)
    {
        packetInfo.numOfParts =  prvTgfGlobalPacketInfo[packetIter].numOfParts;
        packetInfo.partsArray =  prvTgfGlobalPacketInfo[packetIter].partsArray;
        packetInfo.totalLen   =  prvTgfGlobalPacketInfo[packetIter].totalLen;
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        /* AUTODOC: Send packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* AUTODOC: Read counters from target port and save them to be verified */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");

        prvTgfGlobalPacketInfo[packetIter].receivedPacket->pktsNum = portCntrs.goodPktsRcv.l[0];
        prvTgfGlobalPacketInfo[packetIter].receivedPacket->bytesNum = portCntrs.goodOctetsRcv.l[0];

        /* AUTODOC: Update expected number of packets and bytes */
        gateInfoArr[packetIter%PRV_TGF_GATES_NUM_CNS].expPktsNum += 1;
        gateInfoArr[packetIter%PRV_TGF_GATES_NUM_CNS].expBytesNum +=           \
            prvTgfGlobalPacketInfo[packetIter].packetPayloadPart->dataLength + \
            TGF_L2_HEADER_SIZE_CNS                                           + \
            TGF_CRC_LEN_CNS;
    }

    /* AUTODOC: Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}


/******************************************************************************\
 *   Restore functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcMultipleGatesConfigRestore function
* @endinternal
*
* @brief   SGC multiple gates configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore gate statistics Configuration
*          3. Restore PCL Configuration
*          4. Restore gate configuration
*          5. Restore base Configuration
*/
GT_VOID prvTgfStreamSgcMultipleGatesConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;
    GT_U32                           ruleId;
    GT_U32                           gateNum;
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

    /* AUTODOC: Go over all gates that were used in this test */
    for (gateNum=0;gateNum<PRV_TGF_GATES_NUM_CNS;gateNum++)
    {
        /* AUTODOC: Set CNC counter index in 802.1Qci format:
                    |zero padding(15:11)|gate state(10:9)|gate ID(8:0)|
                    modulo block size since index can not be larger than maximum block size */
        cncCfg.counterNum = ( ((gateInfoArr[gateNum].gateState<<9) | gateInfoArr[gateNum].gateId) % blockSize);

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
                                              gateInfoArr[gateNum].gateId,
                                              gateInfoArr[gateNum].tableSetId,
                                              0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc,"prvCpssStreamSgcOctetCountersSet");

        /* AUTODOC: Clear SW expected counters */
        gateInfoArr[gateNum].expPktsNum = 0;
        gateInfoArr[gateNum].expBytesNum = 0;
    }

    /* AUTODOC: Set packet pass/drop mode */
    rc = cpssDxChCncPacketTypePassDropToCpuModeSet(prvTgfDevNum,
                                                   prvTgfRestoreCfg.passDropMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCncPacketTypePassDropToCpuModeGet");

    /* -------------------------------------------------------------------------
     * 3. Restore PCL Configuration
     */
    /* AUTODOC: invalidate all PCL rules that were used in test */
    for (ruleId=0; ruleId<PRV_TGF_PCL_RULES_NUM_CNS; ruleId++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, ruleId, GT_FALSE);
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
     * 4. Restore Gate Configuration
     */
    /* AUTODOC: Set SGC global parameters */
    rc = cpssDxChStreamSgcGlobalConfigSet(prvTgfDevNum, &prvTgfRestoreCfg.globalParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGlobalConfigSet");

    /* AUTODOC: Set back gates parameters and clear gates counters */
    for (gateNum=0; gateNum<PRV_TGF_GATES_NUM_CNS;gateNum++)
    {
        /* AUTODOC: Set gate parameters */
        rc = cpssDxChStreamSgcGateConfigSet(prvTgfDevNum,
                                            gateInfoArr[gateNum].gateId,
                                            gateInfoArr[gateNum].tableSetId,
                                            timeSlotsNum,
                                            imxProfilesNum,
                                            &prvTgfRestoreCfg.gateParamsArr[gateNum]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChStreamSgcGateConfigSet");
    }

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
