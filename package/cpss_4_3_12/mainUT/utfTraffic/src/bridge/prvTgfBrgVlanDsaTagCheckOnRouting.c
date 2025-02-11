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
* @file prvTgfBrgVlanDsaTagCheckOnRouting.h
*
* @brief DSA tag Vlan Id assignment for untagged packet
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <common/tgfCommon.h>
#include <common/tgfMirror.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfNetIfGen.h>
#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/private/prvTgfTrafficParser.h>
#include <bridge/prvTgfBrgVlanDsaTagCheckOnRouting.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* prvTgfBrgVlanDsaTagCheckOnRouting - tested port config/usage:
 *     TrafGen-SEND-byCPU_
 *       norm or CSCD     \  IP-FWD
 *                         \/      \CSCD-port analyzed by CPU
 *     prvTgfPortsArray[0] [1] [2] [3]
 * Step1: untagged from NET-port => Analyze on CSCD port_%d = eDSA taged
 * Step2: eDSA from CSCD-port => Analyze on CSCD port_%d = eDSA taged
 *
 * VlanToCpuMirroring test-step3 - by default is disabled
 *   To enable simulator on TGF port prvTgfPortsArray[0] call:
 *         shell-execute prvTgfprvTgfBrgVlanDsaTagMirrorPortSet -1
 *   To enable non-TGF external analyser's portNN call:
 *         shell-execute prvTgfprvTgfBrgVlanDsaTagMirrorPortSet NN
 *
 *    TrafGen-SEND-byCPU_
 *    VlanTagged ID=5    \    MAC-FWD"ToCPU"
 *    PktSize=(4+96)      \  /          \
 *                         \/VlanID=1    \
 *     prvTgfPortsArray[0] [1] [2] [3]  CPU=63(PktSize=4+96+DSA16B)
 *                       \_______________/
 *                      Mirror,ANALYZER_IDX=3
 *                    PktSize=96 without VlanTag
 * Step3: vlan-tagged NET-ToCPU => Analyze on MIRROR port_%d = UnTagged
 * NOTE:
 *  an "external" mirroring port could be used instead any prvTgfPortsArray[],
 *  this port would not passe over tgfTrafficGeneratorPortForceLinkUpEnableSet
 *  and is real link to real external traffic analyzer.
 */
/* VlanToCpuMirroring: Mirror-Analyze port from CPU */
#define PRV_TGF_VLAN_TO_CPU_MIRRORING_TEST_DISABLE  ((GT_U32)(-2))
#define PRV_TGF_VLAN_TO_CPU_MIRRORING_ON_TGF_PORT   ((GT_U32)(-1))
#define PRV_TGF_EGR_ANALYZE_PORT_IDX    0
#define PRV_TGF_EGR_ANALYZER_IDX        3

/* RX port index (TrafGen port) */
#define PRV_TGF_SEND_PORT_IDX 1
/* RX cascade port */
#define PRV_TGF_CSCD_SEND_PORT_IDX  PRV_TGF_SEND_PORT_IDX

/* TX port index */
#define PRV_TGF_RECV_PORT_IDX 2

/* cascade port index */
#define PRV_TGF_CSCD_PORT_IDX 3

/* TX port Vlan ID */
#define PRV_TGF_RECV_PORT_VLAN_ID 6

/* RX port Vlan ID */
#define PRV_TGF_SEND_PORT_VLAN_ID 5
#define PRV_TGF_TO_CPU_VLAN_ID    PRV_TGF_SEND_PORT_VLAN_ID

/* Designated device number index */
#define PRV_TGF_DEVICE_NUM_INDEX_CNS    5

/* Designated device number */
#define PRV_TGF_ORIG_REMOTE_DEV_CNS 25
static GT_U8 prvTgfTrgHwDevNum = 25;
static GT_BOOL targetDevChanged = GT_FALSE;

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS     CPSS_IP_CNT_SET1_E

/* Cacade target devide number */
#define PRV_TGF_CSCD_TARGET_DEV_NUM 15

/* Cacade target port number */
#define PRV_TGF_CSCD_TARGET_PORT_NUM 6

/* dsa tag vid */
#define PRV_TGF_DSA_TAG_VLAN_ID 0

/* the Arp Address index of the Router ARP Table
   to write to the UC Route entry Arp nextHopARPPointer field */
static GT_U32        prvTgfRouterArpIndex      = 1;

/* the Route entry index for UC Route entry Table */
static GT_U32        prvTgfRouteEntryBaseIndex = 3;

/* default number of packets to send */
static GT_U32       prvTgfBurstCount   = 1;

static GT_BOOL prvTgfIsCscd = GT_FALSE;
static GT_BOOL prvTgfVlanToCpuMirroring;
static GT_U32 prvTgfMirrorPort = PRV_TGF_VLAN_TO_CPU_MIRRORING_TEST_DISABLE;

/******************************* Test packet **********************************/
/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0xaa, 0xaa},               /* dstMac */
    {0x00, 0xbb, 0x00, 0x00, 0x00, 0x08},               /* srcMac */

};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x11,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 0x3,  0x3,  0x3,  0x3},   /* srcAddr */
    { 0xa,  0xa,  0xa,  0xa}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] =
{
    0x00, 0x02, 0x00, 0x09, 0x05, 0xc4, 0xd0, 0x5c, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
    0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21,
    0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
    0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0xEE, /* 0xEE Data-End-flag */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                        /* dataLength */
    prvTgfPayloadDataArr                                 /* dataPtr */
};

/* Ether type of packet*/
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
        TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* PARTS of normal networkpacket */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of normal packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS +  \
    TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* normal network PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArray) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArray                                        /* partsArray */
};

static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E , /*srcIsTagged*/
    16,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        21/*portNum*/
    },/*source;*/

    13,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            PRV_TGF_CSCD_TARGET_DEV_NUM,/*devNum*/
            PRV_TGF_CSCD_TARGET_PORT_NUM/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPortValid*/
    0,/*dstEport*/
    0,/*tag0TpidIndex*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        0/*portNum*/
    },/*origSrcPhy*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_1_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        PRV_TGF_DSA_TAG_VLAN_ID, /*vid*/
        GT_FALSE, /*dropOnSource*/
        GT_FALSE  /*packetIsLooped*/
    },/*commonParams*/


    {
        {
            GT_FALSE, /* isEgressPipe */
            GT_FALSE, /* isTagged */
            0,        /* hwDevNum */
            GT_FALSE, /* srcIsTrunk */
            {
                0, /* srcTrunkId */
                0, /* portNum */
                0  /* ePort */
            },        /* interface */
            0,        /* cpuCode */
            GT_FALSE, /* wasTruncated */
            0,        /* originByteCount */
            0,        /* timestamp */
            GT_FALSE, /* packetIsTT */
            {
                0 /* flowId */
            },        /* flowIdTtOffset */
            0
        } /* TGF_DSA_DSA_TO_CPU_STC */

    }/* dsaInfo */
};

/* PARTS of forward dsa packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArrayCscd[] =
{
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_DSA_TAG_E,  &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* LENGTH of forward dsa packet */
#define PRV_TGF_CSCD_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + \
    TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* normal network PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoCscd =
{
    PRV_TGF_CSCD_PACKET_LEN_CNS,                                      /* totalLen */
    sizeof(prvTgfPacketPartArrayCscd) / sizeof(TGF_PACKET_PART_STC), /* numOfParts */
    prvTgfPacketPartArrayCscd                                        /* partsArray */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_TO_CPU_VLAN_ID                        /* pri, cfi, VlanId */
};

static TGF_PACKET_PART_STC prvTgfPacketPartArrayTag[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

#define PRV_TGF_TAG0_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + \
    TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* Tagged network PACKET to send */
static TGF_PACKET_STC prvTgfPacketInfoTag =
{
    PRV_TGF_TAG0_PACKET_LEN_CNS,                                     /* totalLen */
    sizeof(prvTgfPacketPartArrayTag) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfPacketPartArrayTag                                         /* partsArray */
};
/*************************** Restore config ***********************************/
/* parameters that is needed to be restored */
static struct
{
    GT_U16    txVid;
    GT_U16    rxVid;
    GT_BOOL   newDsaEnabled;
    GT_BOOL   enableOwnDevFltr;
    CPSS_CSCD_LINK_TYPE_STC cascadeLink;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT srcPortTrunkHashEn;
    PRV_TGF_BRG_VLAN_ENTRY_DATA_STC vlanEntry;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
} prvTgfRestoreCfg;

/******************************************************************************/

static GT_STATUS prvTgfNetIfSdmaRxCountersGetPrint
(
    IN  GT_U8   devNum,
    IN  GT_BOOL print /* print vs read-clear-only */
)
{
    CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC  rxCounters;
    GT_STATUS rc;

    cpssOsMemSet(&rxCounters, 0, sizeof(rxCounters));
    rc = cpssDxChNetIfSdmaRxCountersGet(devNum, 0, &rxCounters);
    if (print == GT_TRUE)
    {
        PRV_UTF_LOG4_MAC(
            "dev %d port %d RX packets %d, bytes %d\n",
            devNum, CPSS_CPU_PORT_NUM_CNS, rxCounters.rxInPkts, rxCounters.rxInOctets);
    }
    return rc;
}

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfTxMirrorSet function
* @endinternal
*
* @brief      set/use mirroring port with link to external traffic analyzer
*/
GT_STATUS prvTgfprvTgfBrgVlanDsaTagMirrorPortSet
(
    IN  GT_32 portNum
)
{
    prvTgfMirrorPort = portNum;
    return GT_OK; /* just for CLI */
}

/**
* @internal prvTgfTxMirrorSet function
* @endinternal
*
* @brief    Set or clear Tx/Egress mirroring
*
* @param[in] set             - set or clear (restore) action
* @param[in] srcMirroredPort - egress port to be mirrored from
* @param[in] srcPort       - mirr port to be mirrored to
*                            None
*/
static GT_VOID prvTgfTxMirrorSet
(
    IN GT_BOOL  set,
    IN GT_U32   srcMirroredPort,
    IN GT_U32   analyzerPort
)
{
    static PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_MODE_ENT origMode;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    GT_U32          index;
    GT_STATUS       rc;

    if (set == GT_TRUE)
    {
        rc = prvTgfMirrorToAnalyzerForwardingModeGet(prvTgfDevNum, &origMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTxMirrorSet(%d) ModeGet", set);
        /* ModeSet(end2end) changed for SIP5, for SIP6 keept hop2hop; legacy-SIPs are not supported */
        rc = prvTgfMirrorToAnalyzerForwardingModeSet(PRV_TGF_MIRROR_TO_ANALYZER_FORWARDING_END_TO_END_E);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTxMirrorSet(%d) ModeSet End2end", set);
    }
    else
    {
        rc = prvTgfMirrorToAnalyzerForwardingModeSet(origMode);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTxMirrorSet(%d) ModeSet restore", set);
    }

    index = PRV_TGF_EGR_ANALYZER_IDX;

    /* Configure mirroring-analyzer port */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum = prvTgfDevNum;
    interface.interface.devPort.portNum = analyzerPort;
    rc = prvTgfMirrorAnalyzerInterfaceSet(index, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTxMirrorSet(%d) analyzer", set);

    /* Configure source/mirrored port */
    /*rc = prvTgfMirrorTxPortSet(srcMirroredPort, set, index);*/
    rc = cpssDxChMirrorTxPortSet(/*prvTgfDevNum*/0, srcMirroredPort, GT_TRUE, set, index);
    UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfTxMirrorSet(%u:%u, set%d, %d)", prvTgfDevNum, srcMirroredPort, set, index);
}

/**
* @internal prvTgfVlanToCpuMirroringConfigSet function
* @endinternal
*
* @brief   Initial Port configuration.
*         1. Configure Vlan 5 and 6 for test ports
*         2. Set vid=5 to ingress port
*         3. Configure a port to Cascade port.
*         4. Configure device table to redirect TO_CPU packets to cascade port
*/
static GT_VOID prvTgfVlanToCpuMirroringConfigSet
(
    GT_VOID
)
{
    CPSS_MAC_ENTRY_EXT_STC  macEntry;
    GT_HW_DEV_NUM           hwDevNum;
    GT_U8                   idx;
    GT_STATUS               rc;
    GT_U8                   tagArray[] = {1, 1, 1};

    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_TO_CPU_VLAN_ID, prvTgfPortsArray + 1, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_TO_CPU_VLAN_ID, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd(%u,)", CPSS_CPU_PORT_NUM_CNS);
    for (idx = 0; idx < 4; idx++)
    {
        if (idx == PRV_TGF_EGR_ANALYZE_PORT_IDX)
        {
            /* Don't set VLAN for mirror - just for strict test condition */
            continue;
        }
        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum,
                                      PRV_TGF_TO_CPU_VLAN_ID,
                                      prvTgfPortsArray[idx],
                                      GT_TRUE); /*tagged with CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd(%u,)", prvTgfPortsArray[idx]);
    }
    rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, PRV_TGF_TO_CPU_VLAN_ID, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd(%u,)", CPSS_CPU_PORT_NUM_CNS);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX],
                                 PRV_TGF_TO_CPU_VLAN_ID);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet");

    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet: %d", prvTgfDevNum);

    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    macEntry.key.entryType                  = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_TO_CPU_VLAN_ID;
    macEntry.dstInterface.type              = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = hwDevNum;
    macEntry.dstInterface.devPort.portNum   = CPSS_CPU_PORT_NUM_CNS;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.daCommand                      = CPSS_MAC_TABLE_FRWRD_E;
    macEntry.saCommand                      = CPSS_MAC_TABLE_FRWRD_E;

    /* AUTODOC: add FDB entry with DST-MAC 00:00:00:00:aa:aa, VLAN 5 to port 63 */
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    rc = cpssDxChBrgFdbMacEntrySet(prvTgfDevNum, &macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgFdbMacEntrySet dstMac");

    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanForceNewDsaToCpuEnableSet failed");
}

/**
* @internal prvTgfPortConfigSet function
* @endinternal
*
* @brief   Initial Port configuration.
*         1. Configure Vlan 5 and 6 for test ports
*         2. Set vid=5 to ingress port
*         3. Configure a port to Cascade port.
*         4. Configure device table to redirect TO_CPU packets to cascade port
*/
static GT_VOID prvTgfPortConfigSet
(
    GT_VOID
)
{
    GT_STATUS        rc;
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;
    CPSS_CSCD_LINK_TYPE_STC     cascadeLink;
    GT_U8     tagArray[] = {1, 1, 1, 1};
    GT_PORT_NUM sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX];
    GT_HW_DEV_NUM               hwDevNum;

    /* AUTODOC: SETUP CONFIGURATION: */
    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet: %d",
                                 prvTgfDevNum);

    /*the device map table is not accessed for LOCAL DEVICE traffic*/
    if ((hwDevNum == prvTgfTrgHwDevNum) || (prvTgfDevNum == prvTgfTrgHwDevNum))
    {
        targetDevChanged = GT_TRUE;
        prvTgfTrgHwDevNum -= 1;
    }

    /* AUTODOC: create VLAN 5 with untagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_PORT_VLAN_ID, prvTgfPortsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet failed");

    /* AUTODOC: create VLAN 6 with tagged ports 2,3 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_RECV_PORT_VLAN_ID, prvTgfPortsArray, NULL, tagArray, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
         "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* save default vlanId for restore */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, sendPort, &prvTgfRestoreCfg.rxVid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", &prvTgfRestoreCfg.rxVid);

    /* AUTODOC: set PVID 5 for rx port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, sendPort, PRV_TGF_SEND_PORT_VLAN_ID);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d", sendPort);

    /* save and set enabled state of new DSA tag forcing */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.newDsaEnabled);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanForceNewDsaToCpuEnableGet: %d", prvTgfDevNum);

    /* set force new dsa to false */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanForceNewDsaToCpuEnableSet failed");

    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CSCD_PORT_IDX],
            GT_TRUE, CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");

    /* AUTODOC set CPU code table - CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E */
    cpssOsMemSet(&entryInfo, 0, sizeof(entryInfo));
    entryInfo.designatedDevNumIndex = PRV_TGF_DEVICE_NUM_INDEX_CNS;
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
                                    &entryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC set CPU code designated device table */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(prvTgfDevNum,
                                    PRV_TGF_DEVICE_NUM_INDEX_CNS,
                                    prvTgfTrgHwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum,
                                  prvTgfTrgHwDevNum,
                                  0,
                                  0,
                                  &(prvTgfRestoreCfg.cascadeLink),
                                  &(prvTgfRestoreCfg.srcPortTrunkHashEn));
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableGet: %d,%d",
                                 prvTgfDevNum, prvTgfTrgHwDevNum);

    /* AUTODOC: assign remote device with egress cascade port (device map table) */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_CSCD_PORT_IDX];
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                prvTgfTrgHwDevNum,
                                0,
                                0,
                                &cascadeLink,
                                0,
                                GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfCscdDevMapTableSet");
}

/**
* @internal prvTgfIpv4RoutingConfigSet function
* @endinternal
*
* @brief   Initial Ipv4 routing configuration.
*         5. Add mac entry with daCmd as mirror to CPU
*         6. Create the Route entry (Next hop) in Route table and Router ARP Table
*/
GT_VOID prvTgfIpv4RoutingConfigSet
(
        GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           routeEntriesArray[1];
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC           *regularEntryPtr;
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    GT_U32                                   lpmDBId = 0;
    GT_U32                                   vrId = 0;
    GT_U32                                   prefixLen = 0x20;
    GT_BOOL                                  override = GT_FALSE;
    GT_BOOL                                  defragmentationEnable = GT_FALSE;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT     nextHopInfo;
    GT_IPADDR                               ipAddr;

    /* create a macEntry with .daRoute = GT_TRUE and .daCommand mirror to cpu */
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                  = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_PORT_VLAN_ID;
    macEntry.isStatic                       = GT_TRUE;
    macEntry.saCommand                      = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                        = GT_TRUE;
    macEntry.sourceId                       = 0;
    macEntry.userDefined                    = 0;
    macEntry.daQosIndex                     = 0;
    macEntry.saQosIndex                     = 0;
    macEntry.daSecurityLevel                = 0;
    macEntry.saSecurityLevel                = 0;
    macEntry.appSpecificCpuCode             = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn       = GT_FALSE;
    macEntry.dstInterface.vlanId            = PRV_TGF_SEND_PORT_VLAN_ID;
    macEntry.dstInterface.type              = CPSS_INTERFACE_VID_E;
    macEntry.daCommand                      = PRV_TGF_PACKET_CMD_MIRROR_TO_CPU_E;

    /* AUTODOC: add FDB entry for rx port*/
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");

    /* Enable Routing for sendport */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, 0);

    /* AUTODOC: enable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_PORT_VLAN_ID, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d",
                                 prvTgfDevNum, PRV_TGF_SEND_PORT_VLAN_ID);

    /* write a UC Route entry to the Route Table */
    cpssOsMemSet(routeEntriesArray, 0, sizeof(routeEntriesArray));

    regularEntryPtr = &routeEntriesArray[0];

    regularEntryPtr->cmd                        = CPSS_PACKET_CMD_ROUTE_E;
    regularEntryPtr->cpuCodeIndex               = 0;
    regularEntryPtr->appSpecificCpuCodeEnable   = GT_FALSE;
    regularEntryPtr->unicastPacketSipFilterEnable = GT_FALSE;
    regularEntryPtr->ttlHopLimitDecEnable       = GT_TRUE;
    regularEntryPtr->ttlHopLimDecOptionsExtChkByPass = GT_FALSE;
    regularEntryPtr->ingressMirror              = GT_FALSE;
    regularEntryPtr->qosProfileMarkingEnable    = GT_FALSE;
    regularEntryPtr->qosProfileIndex            = 0;
    regularEntryPtr->qosPrecedence              = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    regularEntryPtr->modifyUp                   = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->modifyDscp                 = CPSS_PACKET_ATTRIBUTE_MODIFY_KEEP_PREVIOUS_E;
    regularEntryPtr->countSet                   = PRV_TGF_COUNT_SET_CNS;
    regularEntryPtr->trapMirrorArpBcEnable      = GT_FALSE;
    regularEntryPtr->sipAccessLevel             = 0;
    regularEntryPtr->dipAccessLevel             = 0;
    regularEntryPtr->ICMPRedirectEnable         = GT_FALSE;
    regularEntryPtr->scopeCheckingEnable        = GT_FALSE;
    regularEntryPtr->siteId                     = CPSS_IP_SITE_ID_INTERNAL_E;
    regularEntryPtr->mtuProfileIndex            = 0;
    regularEntryPtr->isTunnelStart              = GT_FALSE;
    regularEntryPtr->nextHopVlanId              = PRV_TGF_RECV_PORT_VLAN_ID;
    regularEntryPtr->nextHopInterface.type      = CPSS_INTERFACE_PORT_E;
    regularEntryPtr->nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    regularEntryPtr->nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RECV_PORT_IDX];
    regularEntryPtr->nextHopARPPointer          = prvTgfRouterArpIndex;
    regularEntryPtr->nextHopTunnelPointer       = 0;

    /* AUTODOC: read and save the ipv4 uc default route entry */
    cpssOsMemSet(prvTgfRestoreCfg.routeEntriesArray, 0, sizeof(prvTgfRestoreCfg.routeEntriesArray));
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum, prvTgfRouteEntryBaseIndex, prvTgfRestoreCfg.routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: %d", prvTgfDevNum);

    /* AUTODOC: add UC route entry with nexthop VLAN 6 and nexthop port 3 */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);

    /* fill a nexthop info for the prefix */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(nextHopInfo));
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = prvTgfRouteEntryBaseIndex;
    nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_MAC;

    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* Add Ipv4 unicast prefix */
    rc = prvTgfIpLpmIpv4UcPrefixAdd(lpmDBId, vrId, ipAddr, prefixLen, &nextHopInfo, override, defragmentationEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfPortConfigRestore function
* @endinternal
*
* @brief   Restore to default portconfiguration
*         1. Restore vlan configuration
*         2. Restore port vid setting
*         3. Restore cascade settings
*         4. Restore device table configuration
*/
static GT_VOID prvTgfPortConfigRestore
(
        GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_PORT_NUM sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX];
    PRV_TGF_NET_CPU_CODE_TABLE_ENTRY_STC entryInfo;

    /* AUTODOC: Restore PVID */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, sendPort, prvTgfRestoreCfg.rxVid);
    UTF_VERIFY_EQUAL3_PARAM_MAC(GT_OK, rc, prvTgfDevNum, sendPort,
                prvTgfRestoreCfg.rxVid);

    /* AUTODOC: Invalidate VLAN */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RECV_PORT_VLAN_ID);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_RECV_PORT_VLAN_ID);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_PORT_VLAN_ID);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, PRV_TGF_SEND_PORT_VLAN_ID);

    /* AUTODOC: Restore force new DSA enabled state */
    rc = prvTgfBrgVlanForceNewDsaToCpuEnableSet(prvTgfRestoreCfg.newDsaEnabled);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfRestoreCfg.newDsaEnabled);

    /* AUTODOC: Restore Cascade enable */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CSCD_SEND_PORT_IDX], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");

    /* AUTODOC: Restore Cascade enable */
    rc = tgfTrafficGeneratorEgressCscdPortEnableSet(
        prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CSCD_PORT_IDX], GT_FALSE, CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet: [%d] \n");

    /* Restore Bridge Global Configuration Regiser */
    rc = prvTgfCscdDsaSrcDevFilterSet(prvTgfRestoreCfg.enableOwnDevFltr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssDrvHwPpWriteRegister \n");

    /* AUTODOC: restore cascade map table */
    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,
                                  prvTgfTrgHwDevNum,
                                  0,
                                  0,
                                  &(prvTgfRestoreCfg.cascadeLink),
                                  prvTgfRestoreCfg.srcPortTrunkHashEn,
                                  GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdDevMapTableSet: %d,%d",
                                 prvTgfDevNum, prvTgfTrgHwDevNum);

    /* clear entry in cpu code table*/
    cpssOsMemSet(&entryInfo, 0, sizeof(entryInfo));
    rc = prvTgfNetIfCpuCodeTableSet(prvTgfDevNum,
                                    CPSS_NET_CONTROL_SRC_DST_MAC_TRAP_E,
                                    &entryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeTableSet");

    /* AUTODOC Reset CPU code designated device table */
    rc = prvTgfNetIfCpuCodeDesignatedDeviceTableSet(prvTgfDevNum,
                                    PRV_TGF_DEVICE_NUM_INDEX_CNS,
                                    prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfNetIfCpuCodeDesignatedDeviceTableSet");

    if (targetDevChanged)
    {
        prvTgfTrgHwDevNum = PRV_TGF_ORIG_REMOTE_DEV_CNS;
    }

}

/**
* @internal prvTgfIpv4RoutingConfigSet function
* @endinternal
*
* @brief   Restore Ipv4 routing configuration.
*         5. Flush fdb table
*         6. Restore Ipv4 routing configuration
*/
static GT_VOID prvTgfIpv4RoutingConfigRestore
(
        GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32                                   lpmDBId = 0;
    GT_IPADDR                               ipAddr;
    GT_U32                                   prefixLen = 0x20;

    /* AUTODOC: Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, GT_TRUE);

    /* fill the destination IP address for Ipv4 prefix in Virtual Router */
    cpssOsMemCpy(ipAddr.arIP, prvTgfPacketIpv4Part.dstAddr, sizeof(ipAddr.arIP));

    /* AUTODOC: delete the Ipv4 prefix */
    rc = prvTgfIpLpmIpv4UcPrefixDel(lpmDBId, 0, ipAddr, prefixLen);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel: %d", prvTgfDevNum);

    /* disable Routing for sendport */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpPortRoutingEnable: %d %d", prvTgfDevNum, 0);

    /* AUTODOC: disable IPv4 Unicast Routing on Vlan 5 */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_PORT_VLAN_ID, CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfIpVlanRoutingEnable: %d %d", prvTgfDevNum, PRV_TGF_SEND_PORT_VLAN_ID);

    /* AUTODOC: restore UC route entry */
    rc = prvTgfIpUcRouteEntriesWrite(prvTgfRouteEntryBaseIndex, prvTgfRestoreCfg.routeEntriesArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: %d", prvTgfDevNum);
}

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*        i. Set ingress and egress port configuration
*        ii. Set Ipv4 Routing configuration
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingConfigSet
(
    GT_VOID
)
{
    prvTgfIsCscd = GT_FALSE;

    /* -------------------------------------------------------------------------
    *  i. Set ingress and egress port configuration
    */
    prvTgfPortConfigSet();

    /* -------------------------------------------------------------------------
    *  ii. Set Ipv4 Routing configuration
    */
    prvTgfIpv4RoutingConfigSet();
}

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingCscdConfigSet function
* @endinternal
*
* @brief   Set test configuration:
*        iii. Set ingress port as cascade port
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingCscdConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    prvTgfIsCscd = GT_TRUE;

    /* Save and Set Bridge Global configure register dropOnSourceIsLocal bit field.
     * If not set, packets originated from the device (TO_CPU) will be set to hard drop.
    */
    rc = prvTgfCscdDsaSrcDevFilterGet(&prvTgfRestoreCfg.enableOwnDevFltr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet \n");

    rc = prvTgfCscdDsaSrcDevFilterSet(GT_FALSE);/*disable the filter*/
    /* AUTODOC: set the egress port to be DSA tagged */
    rc = tgfTrafficGeneratorIngressCscdPortEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CSCD_SEND_PORT_IDX],
            GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorEgressCscdPortEnableSet \n");

    prvTgfPacketDsaTagPart_forward.source.portNum = prvTgfPortsArray[PRV_TGF_CSCD_SEND_PORT_IDX];
    prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;
}

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic.
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32 packetLen;
    GT_U8  devNum;
    GT_PORT_NUM sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX];
    GT_PORT_NUM cscdPort = prvTgfPortsArray[PRV_TGF_CSCD_PORT_IDX];
    CPSS_INTERFACE_INFO_STC portInt;
    GT_PORT_NUM             capturePort;
    GT_U8                   packetBufPtr[128] = {0};
    GT_U32                  packetBufLen = 128;
    GT_U8                   queueCpu;
    TGF_NET_DSA_STC         rxParams;
    TGF_PACKET_TYPE_ENT     packetType = TGF_PACKET_TYPE_ANY_E;
    GT_U32                  isToCPU;
    GT_U32                  obsVid;
    CPSS_PP_FAMILY_TYPE_ENT devFamily = 0;
    GT_BOOL                 capturePortIsNonTgf;

    capturePort = (prvTgfVlanToCpuMirroring) ? prvTgfMirrorPort : cscdPort;
    capturePortIsNonTgf = prvTgfVlanToCpuMirroring &&
                         (prvTgfMirrorPort != prvTgfPortsArray[PRV_TGF_EGR_ANALYZE_PORT_IDX]);

    /* AUTODOC: Reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);  /* ForceLinkUp is inside */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");
    rc  = prvTgfNetIfSdmaRxCountersGetPrint(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfSdmaRxCountersGetPrint clear");

    if (capturePortIsNonTgf)
    {
        goto set_packet_and_transmit; /* no capture settings */
    }

    /* enable capture on a MC subscriber port */
    portInt.type            = CPSS_INTERFACE_PORT_E;
    portInt.devPort.hwDevNum  = prvTgfDevNum;
    portInt.devPort.portNum = capturePort;

    /* AUTODOC: start capture on egress port*/
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet failed");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

set_packet_and_transmit:
    if (prvTgfVlanToCpuMirroring)
    {
        /* AUTODOC: Setup forward Vlan tagged packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoTag, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);
        packetType = TGF_PACKET_TYPE_CAPTURE_E;
    }
    else if (prvTgfIsCscd)
    {
        /* AUTODOC: Setup forward dsa packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfoCscd, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);
    }
    else
    {
        /* AUTODOC: Setup normal network packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);
    }

    /* AUTODOC: Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPort);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n", prvTgfDevNum, sendPort);

    if (capturePortIsNonTgf)
    {
        goto finalizing; /* no capture, analyze, clear-capture */
    }

    cpssOsTimerWkAfter(100);

    /* AUTODOC: capture packet by trapping to CPU*/
    rc = tgfTrafficGeneratorRxInCpuGet(packetType,
                                       GT_TRUE, GT_TRUE,
                                       packetBufPtr,
                                       &packetBufLen,
                                       &packetLen,
                                       &devNum,
                                       &queueCpu,
                                       &rxParams);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"tgfTrafficGeneratorRxInCpuGet failed");

    if (prvTgfVlanToCpuMirroring == GT_TRUE)
    {
        /* Mirrored packet is not the same as sent but has no VlanTag (CPSS-16484/HA-3809) */
        GT_U16  rxed = packetBufPtr[12] << 8 | packetBufPtr[13];

        UTF_VERIFY_EQUAL1_STRING_MAC(TGF_ETHERTYPE_0800_IPV4_TAG_CNS, rxed,
                                     "Received Packet is tagged (expected 0x0800, RXed 0x%04x)",
                                     rxed);
        UTF_VERIFY_EQUAL1_STRING_MAC(0x45, packetBufPtr[14],
                                     "Received Packet is tagged (expected 0x45, RXed 0x%02x)",
                                     packetBufPtr[14]);
    }
    else
    {
        /* AUTODOC: check DSA tag fields for TO_CPU dsa tag */
        isToCPU = packetBufPtr[14] & 0xC0;
        UTF_VERIFY_EQUAL0_STRING_MAC(0x00, isToCPU, "Received Packet is not TO_CPU packet");

        rc = prvUtfDeviceFamilyGet(prvTgfDevNum, &devFamily);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"prvUtfDeviceFamilyGet failed");

        /* AUTODOC: check DSA tag fields for vlan ID */
        obsVid = ((packetBufPtr[14] & 0xf) << 8) | packetBufPtr[15];

        /* TO_CPU DSA tag contains port vid for all devices except aldrin 2 and later devices which contains next hop vid*/
        if(prvTgfIsCscd && (devFamily < CPSS_PP_FAMILY_DXCH_ALDRIN2_E) && (devFamily > CPSS_PP_FAMILY_START_DXCH_E))
            UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SEND_PORT_VLAN_ID, obsVid, "Vid in DSA Tag is incorrect");
        else
            UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_RECV_PORT_VLAN_ID, obsVid, "Vid in DSA Tag is incorrect");
    }

    /* AUTODOC: restore port capture setting */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInt, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
            "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
            prvTgfDevNum, portInt.devPort.portNum);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

finalizing:
    /* AUTODOC: Print and Reset ETH counters */
    rc = prvTgfPrintPortCountersEth();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPrintPortCountersEth");
    rc  = prvTgfNetIfSdmaRxCountersGetPrint(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfNetIfSdmaRxCountersGetPrint print");
}

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingConfigRestore function
* @endinternal
*
* @brief   Restore test configuration
*        1. Restore ingress and egress port configuration
*        2. Set Ipv4 Routing configuration
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingConfigRestore
(
    GT_VOID
)
{
    /* -------------------------------------------------------------------------
    *  1. Restore ingress and egress port configuration
    */
    prvTgfPortConfigRestore();

    /* -------------------------------------------------------------------------
    *  2. Set Ipv4 Routing configuration
    */
    prvTgfIpv4RoutingConfigRestore();

}

/**
* @internal prvTgfBrgVlanDsaTagCheckOnRoutingAllSteps function
* @endinternal
*
* @brief   Multi-step test with different config combinations:
*           with/without Analyze on external Mirroring port
*/
GT_VOID prvTgfBrgVlanDsaTagCheckOnRoutingAllSteps
(
    GT_VOID
)
{
    prvTgfVlanToCpuMirroring = GT_FALSE;

    PRV_UTF_LOG1_MAC("\n-- Step1: untagged from NET-port => Analyze on CSCD port_%d = eDSA taged --\n",
                     prvTgfPortsArray[PRV_TGF_CSCD_PORT_IDX]);
    prvTgfIsCscd = GT_FALSE;
    prvTgfBrgVlanDsaTagCheckOnRoutingConfigSet();
    prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate();

    PRV_UTF_LOG1_MAC("\n-- Step2: eDSA from CSCD-port => Analyze on CSCD port_%d = eDSA taged --\n",
                     prvTgfPortsArray[PRV_TGF_CSCD_PORT_IDX]);
    prvTgfBrgVlanDsaTagCheckOnRoutingCscdConfigSet();   /*prvTgfIsCscd=TRUE is inside*/
    prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate();

    prvTgfBrgVlanDsaTagCheckOnRoutingConfigRestore();

    if (prvTgfMirrorPort == PRV_TGF_VLAN_TO_CPU_MIRRORING_TEST_DISABLE)
    {
        PRV_UTF_LOG0_MAC("\n-- SKIP Step3: vlan-tagged NET-ToCPU --\n");
        return;
    }
    /* This step checks that ToCPU Mirrored packet is not the same as sent
     * but has no VlanTag (CPSS-16484/HA-3809)
     */
    prvTgfVlanToCpuMirroring = GT_TRUE;
    if (prvTgfMirrorPort == PRV_TGF_VLAN_TO_CPU_MIRRORING_ON_TGF_PORT)
    {
        prvTgfMirrorPort = prvTgfPortsArray[PRV_TGF_EGR_ANALYZE_PORT_IDX];
    }
    PRV_UTF_LOG1_MAC("\n-- Step3: vlan-tagged NET-ToCPU => Analyze on MIRROR port_%d = UnTagged --\n",
                     prvTgfMirrorPort);
    prvTgfVlanToCpuMirroringConfigSet();
    prvTgfTxMirrorSet(GT_TRUE, CPSS_CPU_PORT_NUM_CNS, prvTgfMirrorPort);
    prvTgfBrgVlanDsaTagCheckOnRoutingTrafficGenerate();
    prvTgfTxMirrorSet(GT_FALSE, CPSS_CPU_PORT_NUM_CNS, prvTgfMirrorPort);
    prvTgfPortConfigRestore();
}
