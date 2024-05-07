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
* @file prvTgfTunnelTermMplsMultiLabel.c
*
* @brief Test for popping 6 MPLS labels
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfCncGen.h>
#include <common/tgfTunnelGen.h>
#include <tunnel/prvTgfTunnelTermMplsMultiLabel.h>
#include <common/tgfCutThrough.h>
#include <cpss/dxCh/dxChxGen/cutThrough/cpssDxChCutThrough.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* original PCL id for MPLS packets -- in the TTi lookup */
static GT_U32   origMplsTtiPclId;

/* TTI index */
#define PRV_TGF_TTI_INDEX_CNS            5

/* PCLID for the rule - for the MPLS packets */
#define PRV_TGF_TTI_MPLS_PCL_ID_CNS      1

/* Ingress port number to send traffic */
#define PRV_TGF_INGRESS_PORT_IDX_CNS     0

/* Egress port number to receive traffic */
#define PRV_TGF_EGRESS_PORT_IDX_CNS 1

/* VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS          5

/* Number of packets to send */
#define PRV_TGF_PACKET_NUM_CNS           1

/******************************* MPLS packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x01},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x11}                 /* saMac */
};

/* VLAN Tag part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketMplsEtherType = {TGF_ETHERTYPE_8847_MPLS_TAG_CNS};

/* packet's MPLS Label1 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls1 =
{
    0xAB,  /* label */
    0,     /* CoS */
    0,     /* S */
    0x15   /* TTL */
};

/* packet's MPLS Label2 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls2 =
{
    0xBC,  /* label */
    0,     /* CoS */
    0,     /* S */
    0x15   /* TTL */
};

/* packet's MPLS Label3 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls3 =
{
    0xBE,  /* label */
    0,     /* CoS */
    0,     /* S */
    0x15   /* TTL */
};

/* packet's MPLS Label4 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls4 =
{
    0xCA,  /* label */
    0,     /* CoS */
    0,     /* S */
    0x15   /* TTL */
};

/* packet's MPLS Label5 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls5 =
{
    0xCC,   /* label */
    0,      /* CoS */
    0,      /* S */
    0x15    /* TTL */
};

/* packet's MPLS Label6 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls6 =
{
    0xCF,   /* label */
    0,      /* CoS */
    1,      /* S */
    0x15    /* TTL */
};


/* packet's MPLS Label6 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls6_1 =
{
    0xCF,   /* label */
    0,      /* CoS */
    0,      /* S */
    0x15    /* TTL */
};
/* packet's MPLS Label7 */
static TGF_PACKET_MPLS_STC prvTgfPacketMpls7 =
{
    0xD3,   /* label */
    0,      /* CoS */
    1,      /* S */
    0x15    /* TTL */
};


/* packet's Ethernet over MPLS (start of passenger part) */
static TGF_PACKET_L2_STC prvTgfPacketPassengerL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x12}                /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherType},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls1},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls2},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls3},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls4},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls5},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls6},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET info */
static TGF_PACKET_STC prvTgfMplsPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                            /* totalLen */
    sizeof(prvTgfMplsPacketArray)/sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfMplsPacketArray                                       /* partsArray */
};

/* PARTS of packet MPLS */
static TGF_PACKET_PART_STC prvTgfMplsPacketArray2[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherType},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls1},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls2},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls3},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls4},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls5},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls6_1},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls7},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PACKET info */
static TGF_PACKET_STC prvTgfMplsPacketInfo2 = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                             /* totalLen */
    sizeof(prvTgfMplsPacketArray2)/sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfMplsPacketArray2                                       /* partsArray */
};

/****** MPLS packet for testcase tgfTunnelTermImplicitMplsPop1Label *******/

static TGF_PACKET_MPLS_STC prvTgfPacketMpls =
{
    0xAB,  /* label */
    0,     /* CoS */
    1,     /* S */
    0x15   /* TTL */
};

static TGF_PACKET_PART_STC prvTgfMplsPacketArray1[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherType},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMpls},
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketPassengerL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

static TGF_PACKET_STC prvTgfMplsPacketInfo1 = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                            /* totalLen */
    sizeof(prvTgfMplsPacketArray1)/sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfMplsPacketArray1                                      /* partsArray */
};

CPSS_PACKET_CMD_ENT restoreMplsExceptionPktCmd;

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x1ea,              /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0xff,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    {10, 10, 10, 10},   /* srcAddr */
    { 4,  4,  4,  4}    /* dstAddr */
};

/* packet's MPLS Label1 */
static TGF_PACKET_MPLS_STC prvTgfPacketMplsCheck =
{
    0xAB,  /* label */
    0,     /* CoS */
    1,     /* S */
    0x15   /* TTL */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr1[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart1 = {
    sizeof(prvTgfPayloadDataArr1),                       /* dataLength */
    prvTgfPayloadDataArr1                                /* dataPtr */
};

static TGF_PACKET_PART_STC prvTgfMplsCheckPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketMplsEtherType},
    {TGF_PACKET_PART_MPLS_E,      &prvTgfPacketMplsCheck},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart1}
};

/* PACKET info */
static TGF_PACKET_STC prvTgfMplsCheckPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                            /* totalLen */
    sizeof(prvTgfMplsCheckPacketArray)/sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfMplsCheckPacketArray                                       /* partsArray */
};

/* parameters that are needed to be restored */
static struct
{
    CPSS_TUNNEL_TYPE_ENT       tunnelTypeRestore;
    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelStartEntryRestore;
    PRV_TGF_EARLY_PROCESSING_MODE_ENT earlyProcessingModeRestore;
    GT_BOOL     cutThroughPortEnable;
    GT_BOOL     cutThroughUntaggedEnable;
    GT_BOOL     mplsCutThroughEnable;
} prvTgfMplsCutThroughTestRestoreCfg;

GT_U32   prvTgfPclRuleIndex = 1;
GT_U32   prvTgfTunnelStartLineIndex = 8;


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTunnelTermMplsMultiLabelBridgeConfigSet function
* @endinternal
*
* @brief   Configure VLAN Ids and FDB tables
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc;
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* Create VLAN */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* Bind VLAN with port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgVlanPortVidSet");

    /* Add FDB entry with Packet's DMAC(00:00:00:00:34:02), VLAN and egress port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_SEND_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefFdbMacEntryOnPortSet: %d", prvTgfDevNum);
}


/**
* @internal prvTgfTunnelTermMplsMultiLabelTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc;
    PRV_TGF_TTI_RULE_UNT    pattern;
    PRV_TGF_TTI_RULE_UNT    mask;
    PRV_TGF_TTI_ACTION_STC  ruleAction;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* Set TTI TCAM relative index as FALSE */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* clear entry */
    cpssOsMemSet(&pattern,   0, sizeof(pattern));
    cpssOsMemSet(&mask,      0, sizeof(mask));
    cpssOsMemSet(&ruleAction,0, sizeof(ruleAction));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate                 = GT_TRUE;
    ruleAction.command                         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand                 = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.passengerPacketType             = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.bindToCentralCounter            = GT_TRUE;
    ruleAction.bridgeBypass                    = GT_TRUE;
    ruleAction.interfaceInfo.type              = CPSS_INTERFACE_PORT_E;
    ruleAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.interfaceInfo.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
    ruleAction.tunnelStart                     = GT_FALSE;
    ruleAction.tunnelStartPtr                  = 0;
    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId                      = 0;

    pattern.mpls.common.pclId           = PRV_TGF_TTI_MPLS_PCL_ID_CNS;
    pattern.mpls.common.srcIsTrunk      = GT_FALSE;
    pattern.mpls.common.srcPortTrunk    = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];
    pattern.mpls.common.vid             = PRV_TGF_SEND_VLANID_CNS;
    /* Add the First 3 MPLS labels as pattern entry */
    pattern.mpls.label0                 = prvTgfPacketMpls1.label;
    pattern.mpls.exp0                   = prvTgfPacketMpls1.exp;
    pattern.mpls.label1                 = prvTgfPacketMpls2.label;
    pattern.mpls.exp1                   = prvTgfPacketMpls2.exp;
    pattern.mpls.label2                 = prvTgfPacketMpls3.label;
    pattern.mpls.exp2                   = prvTgfPacketMpls3.exp;
    pattern.mpls.numOfLabels            = 2; /* No. of labels added to pattern -1 */
    pattern.mpls.protocolAboveMPLS      = 2; /* Ethernet over MPLS */
    pattern.mpls.common.dsaSrcIsTrunk   = GT_FALSE;
    pattern.mpls.common.dsaSrcPortTrunk = 0;
    pattern.mpls.common.dsaSrcDevice    = prvTgfDevNum;

    mask.mpls.common.srcIsTrunk         = 1;
    mask.mpls.common.pclId              = BIT_10 - 1;
    mask.mpls.label0                    = BIT_20 - 1;/*20 bits field */
    mask.mpls.label1                    = BIT_20 - 1;/*20 bits field */
    mask.mpls.label2                    = BIT_20 - 1;/*20 bits field */
    mask.mpls.exp0                      = BIT_3 - 1;/*20 bits field */
    mask.mpls.exp1                      = BIT_3 - 1;/*20 bits field */
    mask.mpls.exp2                      = BIT_3 - 1;/*20 bits field */

    /* Set the TTI Rule */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_RULE_MPLS_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* Get the Old PCL Id for restoration */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &origMplsTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* Set the PCL Id */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, PRV_TGF_TTI_MPLS_PCL_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");
}

/**
* @internal prvTgfTunnelTermMplsMultiLabelTrafficGenerate function
* @endinternal
*
* @brief   Generates traffic with packet definitions and sends it
*          to ingress port
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U8                   dev      = 0;
    GT_U8                   queue    = 0;
    GT_U32                  buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                  packetActualLength = 0;
    TGF_NET_DSA_STC         rxParam;
    static GT_U8            packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    CPSS_INTERFACE_INFO_STC portInterface;

    /* Generate Traffic with Capture */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          &prvTgfMplsPacketInfo,
                                          PRV_TGF_PACKET_NUM_CNS, 0, NULL,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                          TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* Enable capture on Egress interface */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                                            TGF_PACKET_TYPE_CAPTURE_E,
                                                            GT_TRUE, GT_TRUE, packetBuff,
                                                            &buffLen, &packetActualLength,
                                                            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    if (GT_OK == rc)
    {
        /* All the 6 MPLS labels should be removed
         * and packet should start with the L2 header */
         if (0 != cpssOsMemCmp(&prvTgfPacketPassengerL2Part.daMac, &(packetBuff[0]), 6))
         {
              UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet Dest MAC is not as expected");
         }
         if (0 != cpssOsMemCmp(&prvTgfPacketPassengerL2Part.saMac, &(packetBuff[6]), 6))
         {
              UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet Src MAC is not as expected");
         }
    }

    /* Get the Packet Command for the Exception PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E */
    rc = prvTgfTtiExceptionCmdGet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E,
                                  &restoreMplsExceptionPktCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* Set the new Packet Command for the Exception PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E,
                                  CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* Generate Traffic with Capture */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          &prvTgfMplsPacketInfo2,
                                          PRV_TGF_PACKET_NUM_CNS, 0, NULL,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                          TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* Capture the CPU mirrored packet */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_TRUE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &dev, &queue, &rxParam);

    /* We want to verify packet was trapped to CPU */
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParam.dsaCmdIsToCpu);
    UTF_VERIFY_EQUAL0_PARAM_MAC(CPSS_NET_TT_MPLS_HEADER_CHECK_E, rxParam.cpuCode);

    PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParam.portNum);
    PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParam.cpuCode);
}

/**
* @internal prvTgfTunnelTermMplsMultiLabelConfigRestore function
* @endinternal
*
* @brief   Restore the bridge and TTI configurations
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelConfigRestore
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* Invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* Restore default PVID for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], 1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], 1);

    /* Invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* Disable the TTI lookup for MPLS at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* Restore pclId for mpls key */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, origMplsTtiPclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* Set TTI TCAM relative index as FALSE */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* Restore the packet command  */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E,
                                  restoreMplsExceptionPktCmd);
}

/**
* @internal prvTgfTunnelTermImplicitMplsPop1LabelTtiConfigSet function
* @endinternal
*
* @brief   Set TTI configurations
*/
GT_VOID prvTgfTunnelTermImplicitMplsPop1LabelTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc;
    PRV_TGF_TTI_RULE_UNT    pattern;
    PRV_TGF_TTI_RULE_UNT    mask;
    PRV_TGF_TTI_ACTION_2_STC  ruleAction;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* Set TTI TCAM relative index as FALSE */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* clear entry */
    cpssOsMemSet(&pattern,   0, sizeof(pattern));
    cpssOsMemSet(&mask,      0, sizeof(mask));
    cpssOsMemSet(&ruleAction,0, sizeof(ruleAction));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate                   = GT_FALSE;
    ruleAction.command                           = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ruleAction.ttPassengerPacketType             = PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E;
    ruleAction.bindToCentralCounter              = GT_TRUE;
    ruleAction.bridgeBypass                      = GT_TRUE;
    ruleAction.ingressPipeBypass                 = GT_TRUE;
    ruleAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ruleAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ruleAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
    ruleAction.tunnelStart                       = GT_FALSE;
    ruleAction.tunnelStartPtr                    = 0;
    ruleAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId                        = 0;
    ruleAction.mplsCommand                       = PRV_TGF_TTI_MPLS_POP1_LABEL_COMMAND_E;
    ruleAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;

    /* Configure TTI rule pattern */
    pattern.mpls.common.pclId           = PRV_TGF_TTI_MPLS_PCL_ID_CNS;
    pattern.mpls.common.srcIsTrunk      = GT_FALSE;
    pattern.mpls.common.srcPortTrunk    = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];
    pattern.mpls.common.vid             = PRV_TGF_SEND_VLANID_CNS;
    pattern.mpls.label0                 = prvTgfPacketMpls.label;
    pattern.mpls.exp0                   = prvTgfPacketMpls.exp;
    pattern.mpls.numOfLabels            = 0; /* No. of labels added to pattern -1 */
    pattern.mpls.protocolAboveMPLS      = 2; /* Ethernet over MPLS */
    pattern.mpls.common.dsaSrcIsTrunk   = GT_FALSE;
    pattern.mpls.common.dsaSrcPortTrunk = 0;
    pattern.mpls.common.dsaSrcDevice    = prvTgfDevNum;

    /* Configure TTI rule mask */
    mask.mpls.common.srcIsTrunk         = 1;
    mask.mpls.common.pclId              = BIT_10 - 1;
    mask.mpls.label0                    = BIT_20 - 1;/*20 bits field */
    mask.mpls.exp0                      = BIT_3 - 1;/*20 bits field */

    /* Set the TTI Rule */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_KEY_MPLS_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* Get the Old PCL Id for restoration */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &origMplsTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* Set the PCL Id */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, PRV_TGF_TTI_MPLS_PCL_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");
}

/**
* @internal prvTgfTunnelTermImplicitMplsPop1LabelTrafficGenerate function
* @endinternal
*
* @brief   Restore the bridge and TTI configurations
*/
GT_VOID prvTgfTunnelTermImplicitMplsPop1LabelTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U8                   dev      = 0;
    GT_U8                   queue    = 0;
    GT_U32                  buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                  packetActualLength = 0;
    TGF_NET_DSA_STC         rxParam;
    static GT_U8            packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    CPSS_INTERFACE_INFO_STC portInterface;

    /* Generate Traffic with Capture */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          &prvTgfMplsPacketInfo1,
                                          PRV_TGF_PACKET_NUM_CNS, 0, NULL,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                          TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* Enable capture on Egress interface */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                                            TGF_PACKET_TYPE_CAPTURE_E,
                                                            GT_TRUE, GT_TRUE, packetBuff,
                                                            &buffLen, &packetActualLength,
                                                            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    if (GT_OK == rc)
    {
        /* The MPLS labels should be removed
         * and packet should start with the L2 header */
         if (0 != cpssOsMemCmp(&prvTgfPacketPassengerL2Part.daMac, &(packetBuff[0]), 6))
         {
              UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet Dest MAC is not as expected");
         }
         if (0 != cpssOsMemCmp(&prvTgfPacketPassengerL2Part.saMac, &(packetBuff[6]), 6))
         {
              UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "packet Src MAC is not as expected");
         }
    }
}


/**
* @internal prvTgfTunnelMplsCutThroughConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*/
GT_VOID prvTgfTunnelMplsCutThroughConfigSet
(
    GT_VOID
)
{
    GT_STATUS               rc;

    PRV_TGF_TTI_RULE_UNT    pattern;
    PRV_TGF_TTI_RULE_UNT    mask;
    PRV_TGF_TTI_ACTION_STC  ruleAction;

    PRV_TGF_PCL_RULE_FORMAT_UNT pclMask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pclPattern;
    PRV_TGF_PCL_ACTION_STC      pclAction;

    PRV_TGF_TUNNEL_START_ENTRY_UNT tunnelEntry;
    TGF_MAC_ADDR prvTgfStartEntryMac = {0x00, 0x00, 0x00, 0x00, 0x00, 0x11};

    CPSS_DXCH_EARLY_PROCESSING_MODE_ENT   processingMode;

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* Set TTI TCAM relative index as FALSE */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);

    /* enable the TTI lookup for PRV_TGF_TTI_KEY_MPLS_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_MPLS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* clear entry */
    cpssOsMemSet(&pattern,   0, sizeof(pattern));
    cpssOsMemSet(&mask,      0, sizeof(mask));
    cpssOsMemSet(&ruleAction,0, sizeof(ruleAction));

    /* configure TTI rule action */
    ruleAction.tunnelTerminate                 = GT_TRUE;
    ruleAction.command                         = CPSS_PACKET_CMD_FORWARD_E;
    ruleAction.redirectCommand                 = PRV_TGF_TTI_NO_REDIRECT_E/*PRV_TGF_TTI_REDIRECT_TO_EGRESS_E*/;
    ruleAction.passengerPacketType             = PRV_TGF_TTI_PASSENGER_IPV4_E/*PRV_TGF_TTI_PASSENGER_ETHERNET_NO_CRC_E*/;
    ruleAction.bindToCentralCounter            = GT_TRUE;
    ruleAction.bridgeBypass                    = GT_FALSE;
    ruleAction.tunnelStart                     = GT_FALSE;
    ruleAction.tunnelStartPtr                  = 0;
    ruleAction.tag0VlanCmd                     = PRV_TGF_TTI_VLAN_DO_NOT_MODIFY_E;
    ruleAction.tag0VlanId                      = 0;

    pattern.mpls.common.pclId           = PRV_TGF_TTI_MPLS_PCL_ID_CNS;
    pattern.mpls.common.srcIsTrunk      = GT_FALSE;
    pattern.mpls.common.srcPortTrunk    = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];
    pattern.mpls.common.vid             = PRV_TGF_SEND_VLANID_CNS;
    /* Add the MPLS labels as pattern entry */
    pattern.mpls.label0                 = prvTgfPacketMplsCheck.label;
    pattern.mpls.exp0                   = prvTgfPacketMplsCheck.exp;
    pattern.mpls.numOfLabels            = 0; /* No. of labels added to pattern -1 */
    pattern.mpls.protocolAboveMPLS      = 2; /* Ethernet over MPLS */
    pattern.mpls.common.dsaSrcIsTrunk   = GT_FALSE;
    pattern.mpls.common.dsaSrcPortTrunk = 0;
    pattern.mpls.common.dsaSrcDevice    = prvTgfDevNum;

    mask.mpls.common.srcIsTrunk         = 1;
    mask.mpls.common.pclId              = BIT_10 - 1;
    mask.mpls.label0                    = BIT_20 - 1;/*20 bits field */
    mask.mpls.exp0                      = BIT_3 - 1;/*20 bits field */

    /* Set the TTI Rule */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_RULE_MPLS_E,
                           &pattern, &mask, &ruleAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");

    /* Get the Old PCL Id for restoration */
    rc = prvTgfTtiPclIdGet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, &origMplsTtiPclId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdGet");

    /* Set the PCL Id */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, PRV_TGF_TTI_MPLS_PCL_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet");

    PRV_UTF_LOG0_MAC("======= Setting Tunnel Start Entry Configuration =======\n");

    /* AUTODOC: get and check added TS Entry */
    rc = prvTgfTunnelStartEntryGet(prvTgfDevNum, prvTgfTunnelStartLineIndex,
                                   &prvTgfMplsCutThroughTestRestoreCfg.tunnelTypeRestore,
                                   &prvTgfMplsCutThroughTestRestoreCfg.tunnelStartEntryRestore);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntryGet: %d", prvTgfDevNum);

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.mplsCfg.tagEnable        = GT_FALSE;
    tunnelEntry.mplsCfg.vlanId           = 0;
    cpssOsMemCpy(tunnelEntry.mplsCfg.macDa.arEther, prvTgfStartEntryMac, sizeof(TGF_MAC_ADDR));
    tunnelEntry.mplsCfg.ttl              = 0xEE;
    tunnelEntry.mplsCfg.exp1             = 0x7;
    tunnelEntry.mplsCfg.numLabels        = 1;
    tunnelEntry.mplsCfg.label1           = 0x66;
    tunnelEntry.mplsCfg.mplsEthertypeSelect = PRV_TGF_TUNNEL_START_MPLS_ETHER_TYPE_UC_E;

    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex, CPSS_TUNNEL_X_OVER_MPLS_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");

    PRV_UTF_LOG0_MAC("======= Setting Cut Through Configuration =======\n");

    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChCutThroughEarlyProcessingModeGet(prvTgfDevNum, &processingMode);
        /*Convert*/
        switch(processingMode)
        {
            case CPSS_DXCH_EARLY_PROCESSING_CUT_THROUGH_E:
                prvTgfMplsCutThroughTestRestoreCfg.earlyProcessingModeRestore = PRV_TGF_EARLY_PROCESSING_CUT_THROUGH_E;
                break;
            case CPSS_DXCH_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E:
                prvTgfMplsCutThroughTestRestoreCfg.earlyProcessingModeRestore = PRV_TGF_EARLY_PROCESSING_STORE_AND_FORWARD_REDUCED_LATENCY_E;
                break;
            default:
                PRV_UTF_LOG1_MAC(
                    "[TGF]:Bad Cut Through Processing mode [%d]", processingMode);
        }
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCutThroughEarlyProcessingModeGet");

        rc = prvTgfCutThroughEarlyProcessingModeSet(PRV_TGF_EARLY_PROCESSING_CUT_THROUGH_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughEarlyProcessingModeSet");
    }

    rc = cpssDxChCutThroughPortEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                         &prvTgfMplsCutThroughTestRestoreCfg.cutThroughPortEnable,
                                         &prvTgfMplsCutThroughTestRestoreCfg.cutThroughUntaggedEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCutThroughPortEnableGet");
    rc = prvTgfCutThroughPortEnableSet(prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS], GT_FALSE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");

    rc = cpssDxChCutThroughMplsPacketEnableGet(prvTgfDevNum, &prvTgfMplsCutThroughTestRestoreCfg.mplsCutThroughEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCutThroughMplsPacketEnableGet");
    rc = cpssDxChCutThroughMplsPacketEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCutThroughMplsPacketEnableSet");

    PRV_UTF_LOG0_MAC("======= Setting PCL Configuration =======\n");

    prvTgfPclTunnelTermForceVlanModeEnableSet(GT_FALSE);

    /* mask for MAC address */
    cpssOsMemSet(&pclMask, 0, sizeof(pclMask));

    /* define mask, pattern and action */
    cpssOsMemSet(&pclPattern, 0, sizeof(pclPattern));

    /* action redirect */
    cpssOsMemSet(&pclAction, 0, sizeof(pclAction));
    pclAction.pktCmd               = CPSS_PACKET_CMD_FORWARD_E;
    pclAction.bypassBridge         = GT_TRUE;
    pclAction.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;

    pclAction.redirect.data.outIf.outlifType                              = PRV_TGF_OUTLIF_TYPE_TUNNEL_E;
    pclAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.ptr        = prvTgfTunnelStartLineIndex;
    pclAction.redirect.data.outIf.outlifPointer.tunnelStartPtr.tunnelType = PRV_TGF_PCL_ACTION_REDIRECT_TUNNEL_TYPE_IP_E;

    pclAction.redirect.data.outIf.outInterface.type             = CPSS_INTERFACE_PORT_E;
    pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum = prvTgfDevNum;
    pclAction.redirect.data.outIf.outInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    /* Init PCL Engine for send port 1: */
    /*     ingress direction, lookup0 */
    /*     nonIpKey INGRESS_STD_NOT_IP */
    /*     ipv4Key INGRESS_STD_IP_L2_QOS */
    /*     ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);

    /* set PCL rule 1 with: */
    /*     format INGRESS_STD_IP_L2_QOS, cmd=FORWARD */
    rc = prvTgfPclRuleSet(PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                          prvTgfPclRuleIndex, &pclMask, &pclPattern, &pclAction);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
                                 prvTgfDevNum, prvTgfPclRuleIndex);

}

/**
* @internal prvTgfTunnelMplsCutThroughTrafficGenerate function
* @endinternal
*
* @brief   Generates traffic with packet definitions and sends it
*          to ingress port
*/
GT_VOID prvTgfTunnelMplsCutThroughTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;
    GT_U8                   dev      = 0;
    GT_U8                   queue    = 0;
    GT_U32                  buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32                  packetActualLength = 0;
    TGF_NET_DSA_STC         rxParam;
    static GT_U8            packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    CPSS_INTERFACE_INFO_STC portInterface;

    /* Generate Traffic with Capture */
    rc = prvTgfTransmitPacketsWithCapture(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          &prvTgfMplsCheckPacketInfo,
                                          PRV_TGF_PACKET_NUM_CNS, 0, NULL,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                          TGF_CAPTURE_MODE_MIRRORING_E, 10);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);

    /* Enable capture on Egress interface */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];

    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&portInterface,
                                                            TGF_PACKET_TYPE_CAPTURE_E,
                                                            GT_TRUE, GT_TRUE, packetBuff,
                                                            &buffLen, &packetActualLength,
                                                            &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    if (GT_OK == rc)
    {
        /* Check Packet Length is 512 */
         if (packetActualLength != 512)
         {
              UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "Packet Length is not as expected");
         }
    }
}

/**
* @internal prvTgfTunnelMplsCutThroughConfigRestore function
* @endinternal
*
* @brief   Restore the bridge and TTI configurations
*/
GT_VOID prvTgfTunnelMplsCutThroughConfigRestore
(
    GT_VOID
)
{
    GT_STATUS               rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Restoring Configuration =======\n");

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d",
                                 prvTgfDevNum, GT_TRUE);

    /* Invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* Restore default PVID for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], 1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], 1);

    /* Invalidate TTI rule 1 */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet: %d", prvTgfDevNum);

    /* Disable the TTI lookup for MPLS at port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], PRV_TGF_TTI_KEY_MPLS_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* Restore pclId for mpls key */
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_MPLS_E, origMplsTtiPclId);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* Set TTI TCAM relative index as FALSE */
    prvTgfTtiTcamIndexIsRelativeSet(GT_FALSE);

    /* Restore the packet command  */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_MPLS_UNSUPPORTED_ERROR_E,
                                  restoreMplsExceptionPktCmd);

    /* Restore Tunnel Start Entry */
    rc = prvTgfTunnelStartEntrySet(prvTgfTunnelStartLineIndex,
                                   prvTgfMplsCutThroughTestRestoreCfg.tunnelTypeRestore,
                                   &prvTgfMplsCutThroughTestRestoreCfg.tunnelStartEntryRestore);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTunnelStartEntrySet");

    /* Restore Cut Through Configs */
    if (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum))
    {
        rc = prvTgfCutThroughEarlyProcessingModeSet(prvTgfMplsCutThroughTestRestoreCfg.earlyProcessingModeRestore);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughEarlyProcessingModeSet");
    }

    rc = prvTgfCutThroughPortEnableSet(prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS],
                                       prvTgfMplsCutThroughTestRestoreCfg.cutThroughPortEnable,
                                       prvTgfMplsCutThroughTestRestoreCfg.cutThroughUntaggedEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCutThroughPortEnableSet");

    rc = cpssDxChCutThroughMplsPacketEnableSet(prvTgfDevNum, prvTgfMplsCutThroughTestRestoreCfg.mplsCutThroughEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCutThroughMplsPacketEnableSet");

    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_EXT_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 CPSS_PCL_RULE_SIZE_EXT_E, prvTgfPclRuleIndex, GT_FALSE);

    /* Clear EPCL Engine configuration tables */
    prvTgfPclPortsRestore(CPSS_PCL_DIRECTION_INGRESS_E, CPSS_PCL_LOOKUP_0_E);
}
