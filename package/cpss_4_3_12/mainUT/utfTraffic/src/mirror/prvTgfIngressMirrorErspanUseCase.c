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
* @file prvTgfIngressMirrorErspanUseCase.c
*
* @brief Ingress ERSPAN use cases testing.
*        Supporting ERSPAN Type II use case
*
*        Ports allocation:
*           ^ Port#1: Analyzer port
*           ^ Port#2: Target port
*           ^ Port#3: Ingress port
*
*        Packet flow:
*           1.  Ethernet frame packet enters the device via Ingress port. RX mirroring is executed.
*           2.  One packet which is the origin is sent towards target Port.
*           3.  Second packet is replicated and sent to analyzer port:
*               -   IP GRE tunnel + place holder for ERSPAN header are added by HA unit
*               -   PHA thread is triggered in which fw sets ERSPAN header fields and adds
*                   platform sub header (only in Type III)
*               -   Mirror packet exits the device from analyser port
*
*           The above flow is done for both use cases ERSPAN Type II and ERSPAN Type III
*
* @version   1
********************************************************************************
*/
#include <mirror/prvTgfIngressMirrorErspanUseCase.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeL2EcmpGen.h>
#include <common/tgfCscdGen.h>
#include <common/tgfMirror.h>
#include <common/tgfIpGen.h>
#include <common/tgfCosGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortLoopback.h>

/*************************** Constants definitions ***********************************/

/* Source Falcon device number */
#define SOURCE_FALCON_DEVICE_NUMBER_CNS                2
#define SOURCE_FALCON_CHECK_DEVICE_NUMBER_CNS          2

/* Target Falcon device number */
#define TARGET_FALCON_DEVICE_NUMBER_CNS                0
#define TARGET_FALCON_CHECK_DEVICE_NUMBER_CNS          0

/* Source LC device number */
#define SOURCE_LC_DEVICE_NUMBER_CNS                    9

/* Target LC device number */
#define TARGET_LC_DEVICE_NUMBER_CNS                    8

/* Target LC device number Tx Mirror */
#define TARGET_LC_DEVICE_NUMBER_TX_MIRROR_CNS         0x20

/* Target LC device number */
#define TARGET_LC_PORT_NUMBER_CNS                      7

/* Source LC port number */
#define SOURCE_LC_PORT_NUMBER_CNS                      1

/* Source LC ePort number */
#define SOURCE_LC_EPORT_NUMBER_CNS                     1

/* Target LC ePort number */
#define TARGET_LC_EPORT_NUMBER_CNS                     0

/* 'Remote device' number */
#define REMOTE_DEVICE_NUMBER1_CNS                      16

/* 'Remote device' number */
#define REMOTE_DEVICE_NUMBER2_CNS                      17

/* 'Remote device' number */
#define REMOTE_DEVICE_NUMBER3_CNS                      19

/* 'Remote device' port number */
#define REMOTE_DEVICE_PORT_NUM_CNS                     50

/* 'Remote LC' port number - use source port number.
   CC algorithm assigns this local ePort for packet with Forward eDSA */
#define REMOTE_LC_PORT_NUM_CNS                         (prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS])

/* 'Remote device' analyzer index */
#define REMOTE_DEVICE_ANALYZER_NUM_CNS                 2

/* Ethernet VLAN Id */
#define PRV_TGF_ETHERNET_VLANID_CNS                    5

/* Push Tag Ethernet VLAN Id */
#define PRV_TGF_PUSH_TAG_ETHERNET_VLANID_CNS           0xF

/* Push Tag TPID Entry Index */
#define PRV_TGF_PUSH_TAG_TPID_INDEX_CNS                2

/* Global ePort Representing an L2 ECMP Group */
#define PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS          1025

/* Deafult ePort in CC representing physical port */
#define PRV_TGF_CC_DEFAULT_EPORT_CNS                   prvTgfPortsArray[2]

/* ePort that is represented by the primary ePort PRV_TGF_DESTINATION_PRIMERY_EPORT */
#define PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS       700

/* number of members in the ECMP group */
#define PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS       1

/* The index to the L2 ECMP LTT is <Target ePort>-<Base ePort> */
#define PRV_TGF_INDEX_BASE_EPORT_CNS                   0

/* member 0 of the ECMP group */
#define ECMP_ENTRY_INDEX_0_CNS                         0

/* PHA fw thread IDs of Ingress mirroring ERSPAN Type II */
#define PRV_TGF_PHA_THREAD_ID_SOURCE_FALCON_INGRESS_ERSPAN_TYPE_II_IPV4_CNS  10
#define PRV_TGF_PHA_THREAD_ID_TARGET_FALCON_INGRESS_ERSPAN_TYPE_II_IPV4_CNS  6
#define PRV_TGF_PHA_THREAD_ID_TARGET_FALCON_INGRESS_ERSPAN_TYPE_II_IPV6_CNS  7

/* PHA Shared Memory having ERSPAN Device ID of Central Switch */
#define PRV_TGF_PHA_SHARED_MEMORY_ERSPAN_DEVICE_ID_CNS 2

/* Size of packets in bytes include CRC */
#define PRV_TGF_PACKET_SIZE_ERSPAN_II_CNS              160

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS                        1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS                         4

/* Size of packet */
#define PRV_TGF_MAX_PACKET_SIZE_CNS                    160

/* Ports number allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS                   3
#define PRV_TGF_TARGET_PORT_IDX_CNS                    2
#define PRV_TGF_ANALYZER_PORT1_IDX_CNS                 1
#define PRV_TGF_ANALYZER_PORT2_IDX_CNS                 0

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS                     1

/* port base ePort */
#define PRV_TGF_PORT_BASE_EPORT_CNS                    15

/* trunk base ePort */
#define PRV_TGF_TRUNK_BASE_EPORT_CNS                   60

/* Packet flow Id to set by PCL Action  */
#define PRV_TGF_EPCL_FLOW_ID_CNS                       25
#define PRV_TGF_EPCL_FLOW_ID2_CNS                      26

/* src trunk lsb amount */
#define PRV_TGF_SRC_TRUNK_LSB_AMOUNT_CNS               5

/* src port lsb amount */
#define PRV_TGF_SRC_PORT_LSB_AMOUNT_CNS                4

/* src dev lsb amount */
#define PRV_TGF_SRC_DEV_LSB_AMOUNT_CNS                 0

/* Billing flow Id based counting index base  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS 10

/* Billing flow Id minimal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS        20

/* Billing flow Id maximal for counting  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS        50

/* Billing flow Id maximal for counting  */
#define PRV_TGF_ERSPAN_PACKET_MAX_CNS                   122

/* Billing flow Id based counting index  */
#define PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS   \
    (PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS \
     + PRV_TGF_EPCL_FLOW_ID_CNS                      \
     - PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS)

/* GRE protocol type for ERSPAN Type I,II */
#define PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS         0x88BE

/* GRE protocol type for ERSPAN Type III */
#define PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_III_CNS        0x22EB

/* GRE flags for ERSPAN Type II and Type III */
#define PRV_TGF_GRE_FLAGS_ERSPAN_II_CNS                 0x1000

/* GRE flags for ERSPAN Type I */
#define PRV_TGF_GRE_FLAGS_ERSPAN_I_CNS                  0x0000

/* IPv4 addr offset in IPv4 header */
#define PRV_TGF_IPV4_ADDR_OFFSET_CNS                    12

/* IPv6 addr offset in IPv6 header */
#define PRV_TGF_IPV6_ADDR_OFFSET_CNS                    8

/* ERSPAN III type  */
#define PRV_TGF_ERSPAN_TYPE_III_CNS  3

/* ERSPAN II type  */
#define PRV_TGF_ERSPAN_TYPE_II_CNS   2

/* ERSPAN I type  */
#define PRV_TGF_ERSPAN_TYPE_I_CNS    1

/* Offset to TO_ANALYZER part in the output packet */
#define PRV_TGF_TO_ANALYER_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS)

/* Offset to Ether Type part in the output packet */
#define PRV_TGF_ETH_TYPE_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS)

/* Offset to IP part in the output packet */
#define PRV_TGF_IP_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS)

/* Offset to GRE part in the ipv4 output packet */
#define PRV_TGF_IPV4_GRE_PKT_OFFSET_CNS  (PRV_TGF_IP_PKT_OFFSET_CNS + TGF_IPV4_HEADER_SIZE_CNS)

/* Offset to GRE part in the ipv6 output packet */
#define PRV_TGF_IPV6_GRE_PKT_OFFSET_CNS  (PRV_TGF_IP_PKT_OFFSET_CNS + TGF_IPV6_HEADER_SIZE_CNS)

/* Offset to eDSA part in the output packet */
#define PRV_TGF_EDSA_PKT_OFFSET_CNS  (TGF_L2_HEADER_SIZE_CNS)

/*Eport of the analyzer*/
#define PRV_TGF_DEFAULT_EPORT_CNS     1024
/*Ingress port for user traffic*/
#define PRV_TGF_TX_PORT_IDX_CNS        0
/*Egress port for user traffic*/
#define PRV_TGF_RX_PORT_IDX_CNS        1
/*Analyser port for mirrored traffic*/
#define PRV_TGF_ANALYZER_PORT_IDX_CNS  2
/*Sampling rate for the ERSPAN mirror traffic*/
#define PRV_TGF_SAMPLE_MIRROR_RATE_CNS 3
/*Loopback port */
#define PRV_TGF_LB_PORT_IDX_CNS        3
/*Loopback profile 1 */
#define PRV_TGF_LB_PROFILE_1_CNS       1
/*PHA thread IDs for ERSPAN II same dev mirroring*/
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_II_SAME_DEV_MIRROR_IPV4_CNS  64
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_II_SAME_DEV_MIRROR_IPV6_CNS  65
/*PHA thread IDs for ERSPAN I same dev mirroring*/
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_I_SAME_DEV_MIRROR_IPV4_CNS  68
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_I_SAME_DEV_MIRROR_IPV6_CNS  69
/*PHA thread IDs for ERSPAN III same dev mirroring*/
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV4_CNS              81
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_FIRST_PASS_CNS   82
#define PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_SECOND_PASS_CNS  83
/*PHA thread IDs for CC ERSPAN III orig/trg dev mirroring*/
#define PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_ORIG_DEV_MIRROR_CNS                84
#define PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_TAGGED_CNS     85
#define PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_UNTAGGED_CNS   86
#define PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_TAGGED_CNS     87
#define PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_UNTAGGED_CNS   88
/*ERSPAN port info in ERSPAN header*/
#define PRV_TGF_ERSPAN_SAME_DEV_INDEX 0xABCD
#define PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS 10
#define PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_HW_ID_CNS     60
#define PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_SWITCH_ID_CNS 0xAB
/*GRE and ERSPAN header size*/
#define PRV_TGF_GRE_HEADER_SIZE_CNS           8
#define PRV_TGF_ERSPAN_II_HEADER_SIZE_CNS     8
/* Cleanup TBD during EnhancedUT development */

/*************************** static DB definitions ***********************************/
/* store value of the mode before test */
static PRV_TGF_POLICER_STAGE_METER_MODE_ENT stageModeSave;

/* Analyzer index */
static GT_U32           analyzerIndex1 = 1;

/* OAM index */
static GT_U32           oamIndex = 1;

/* DB to hold analyzer port interface */
static CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* DB to hold relevant PHA threads information */
static CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;
static CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
static CPSS_DXCH_PHA_THREAD_SHARED_INFO_TYPE_ERSPAN_STC erspanEntry;

static GT_U32 prvTgfVlanId = 5;

/* DB to hold actual LM counter values taken from the output packet */
static GT_U32   lmCounter[PRV_TGF_BURST_COUNT_CNS];

/* DB to hold actual IP total length field taken from the output packet */
static GT_U16   ipTotalLength[PRV_TGF_BURST_COUNT_CNS];

/* Mirrored packet, Tunnel part */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfMirroredPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};

/* L3 part */
/* IPv4*/
static TGF_PACKET_IPV4_STC prvTgfMirroredPacketTunnelIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    1,                  /* flags */
    0,                  /* offset */
    0x2,                /* timeToLive */
    0x04,               /* protocol */
    0x5EA0,             /* csum */
    {20,  1,  1,  3},   /* srcAddr */
    {20,  1,  1,  2}    /* dstAddr */
};

/* IPv6 */
static TGF_PACKET_IPV6_STC prvTgfMirroredPacketTunnelIpv6Part =
{
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x02,                                    /* payloadLen */
    0x3b,                                    /* nextHeader */
    0x40,                                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0xff02, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* Original Ingress Packet */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}                /* saMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* EtherType part (for untagged packet) = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    sizeof(prvTgfPayloadDataArr)
};

/* VLAN_TAG part - Tag0 */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_ETHERNET_VLANID_CNS                   /* pri, cfi, VlanId */
};

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfDsaTag = {
    TGF_DSA_CMD_TO_ANALYZER_E, /*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E, /*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        5,        /*vid*/
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

static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfDsaInfo_rxMirrorToAnalyzer = {
    GT_TRUE,/*rxSniffer - ingress mirroring */
    GT_TRUE,/*isTagged. Will be filled later */

    {/* the sniffed port info - src (for rx mirror) or dest (from tx mirror) */
        0,/*hwDevNum*/
        0,/*portNum*/
        0/*ePort*/
    },/*devPort*/

    CPSS_INTERFACE_PORT_E,/*analyzerTrgType*/
    /* union */
    /*
    union{
        struct{
            GT_U16                          analyzerEvidx;
        }multiDest;

        struct{
            GT_BOOL                         analyzerIsTrgPortValid;
            GT_HW_DEV_NUM                   analyzerHwTrgDev;
            GT_PHYSICAL_PORT_NUM            analyzerTrgPort;
            GT_PORT_NUM                     analyzerTrgEport;
        }devPort;
    } extDestInfo;
    */
    {{0}}, /* will be filled later */
    0
};

static TGF_DSA_DSA_FORWARD_STC  prvTgfDsaInfo_forward = {
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E, /*srcIsTagged*/
    0,/*srcHwDev */
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/
    },/*source;*/
    0,/*srcId */ /* will be filled later */

    GT_FALSE,/*egrFilterRegistered - packet is unregistered UC */
    GT_FALSE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_VID_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            0/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        0,/*vlanId*/
        0,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_FALSE,/*isTrgPhyPortValid*/
    0,/*dstEport */
    0,/*tag0TpidIndex */
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /* union */
    {
        /*trunkId*/
        0/*portNum */
    },/*origSrcPhy*/
    GT_TRUE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/

};

static TGF_DSA_DSA_TO_ANALYZER_STC prvTgfDsaInfo_txMirrorToAnalyzer = {
    GT_FALSE,/* rxSniffer - egress mirroring */
    GT_TRUE,/* isTagged */

    {/* the sniffed port info - src (for rx mirror) or dest (from tx mirror) */
        0,/* hwDevNum */
        0,/* portNum */
        0 /* ePort */
    },/* devPort */

    CPSS_INTERFACE_PORT_E,/* analyzerTrgType */
    /* union */
    /*
    union{
        struct{
            GT_U16                          analyzerEvidx;
        }multiDest;

        struct{
            GT_BOOL                         analyzerIsTrgPortValid;
            GT_HW_DEV_NUM                   analyzerHwTrgDev;
            GT_PHYSICAL_PORT_NUM            analyzerTrgPort;
            GT_PORT_NUM                     analyzerTrgEport;
        }devPort;
    } extDestInfo;
    */
    {{0}}, /* will be filled later */
    0 /* tag0TpidIndex */  /* will be filled later */
};

/****************************** PACKETS LENGTH ********************************/
/* size of the tagged packet that came from network port  */
#define PRV_TGF_NETW_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_VLAN_TAG_SIZE_CNS          \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))

/* size of the tagged packet that came from cascade port */
#define PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_eDSA_TAG_SIZE_CNS          \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))

/* ERSPAN III header size (in bytes) */
#define PRV_TGF_ERSPAN_III_HEADER_SIZE_CNS  20

/* size of the tagged ERSPAN III packet that came from cascade port */
#define PRV_TGF_ERSPAN_III_CSCD_TAG_PACKET_SIZE_CNS (PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS    \
                                          + PRV_TGF_ERSPAN_III_HEADER_SIZE_CNS)


/**************************** PACKET PARTS ARRAY ******************************/

/* parts of the tagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
     {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };

/* a cascade tagged packet info */
static TGF_PACKET_STC prvTgfCscdTaggedPacketInfo = {
    PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfCscdPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCscdPacketPartsArray                                       /* partsArray */
};

/* Ingress mirrored ERSPAN III */
static GT_U8 prvTgfPacketIngressErspanIIIData[] = {
    0x20, 0x00, 0x20, 0x00,
    0x2f, 0x9f, 0xde, 0x77,
    0x00, 0x00, 0x83, 0xc5,
    0x14, 0xab, 0xab, 0xcd,
    0x00, 0x06, 0xfd, 0xa8
};

/* struct for any kind of wildcard/'Unknown format' */
static TGF_PACKET_WILDCARD_STC  prvTgfPacketIngressErspanIIIPart =
{
    sizeof(prvTgfPacketIngressErspanIIIData),                       /* dataLength */
    prvTgfPacketIngressErspanIIIData                                /* dataPtr */
};

/* parts of the tagged ingress mirrored packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfIngressErspanIIICscdPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
     {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
     {TGF_PACKET_PART_WILDCARD_E,  &prvTgfPacketIngressErspanIIIPart},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };

/* a cascade tagged packet info */
static TGF_PACKET_STC prvTgfIngressErspanIIICscdPacketInfo = {
    PRV_TGF_ERSPAN_III_CSCD_TAG_PACKET_SIZE_CNS,                                     /* totalLen */
    sizeof(prvTgfIngressErspanIIICscdPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfIngressErspanIIICscdPacketPartsArray                                       /* partsArray */
};

/* Egress mirrored ERSPAN III */
static GT_U8 prvTgfPacketEgressErspanIIIData[] = {
    0x20, 0x00, 0x20, 0x00,
    0x10, 0x64, 0x70, 0xfe,
    0x00, 0x00, 0x83, 0xcd,
    0x14, 0xab, 0xab, 0xcd,
    0x00, 0x01, 0xc1, 0x82
};

/* struct for any kind of wildcard/'Unknown format' */
static TGF_PACKET_WILDCARD_STC  prvTgfPacketEgressErspanIIIPart =
{
    sizeof(prvTgfPacketEgressErspanIIIData),                       /* dataLength */
    prvTgfPacketEgressErspanIIIData                                /* dataPtr */
};

/* parts of the egress mirrored packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfEgressErspanIIICscdPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
     {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
     {TGF_PACKET_PART_WILDCARD_E,  &prvTgfPacketEgressErspanIIIPart},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };

/* a cascade tagged packet info */
static TGF_PACKET_STC prvTgfEgressErspanIIICscdPacketInfo = {
    PRV_TGF_ERSPAN_III_CSCD_TAG_PACKET_SIZE_CNS,                                    /* totalLen */
    sizeof(prvTgfEgressErspanIIICscdPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfEgressErspanIIICscdPacketPartsArray                                       /* partsArray */
};

/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    PRV_TGF_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdbErspan;

static prvTgfPclUdbErspan prvTgfErspanEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0xFF},   /* Analyzer Trg Dev[0:1] Analyzer ePort[12:7] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 55, 0xFF},   /* Analyzer Trg Dev[9:2] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */

     {4 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x03},   /* egress marvell tagged[2] TO_ANALYZER */

     /* must be last */
     {5, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};
static prvTgfPclUdbErspan prvTgfErspanSameDevEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x03},   /* egress marvell tagged[0:1] TO_ANALYZER */

     /* must be last */
     {4, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};

static prvTgfPclUdbErspan prvTgfErspanIIISameDevEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x23},   /* egress mtag cmd[1:0] TO_ANALYZER
                                                         * ingress marvell tagged[5] */

     /* must be last */
     {4, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};

static prvTgfPclUdbErspan prvTgfCCErspanIIIEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 27, 0x20},   /* Tag0 Exist[5] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {4 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x27},   /* egress mtag cmd[1:0] TO_ANALYZER
                                                         * egress marvell tagged[2] */

     /* must be last */
     {5, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};

static prvTgfPclUdbErspan prvTgfErspanSourceFalconEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x07},   /* egress marvell tagged[2] TO_ANALYZER */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 75, 0x20},   /* Word2 [Bit21] of eDSA tag */

     /* must be last */
     {2, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};


/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC           interface;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC               ecmpLttEntryGet;
    GT_BOOL                                         prvTgfEnableMirror;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    PRV_TGF_OAM_COMMON_CONFIG_STC                   oamConfig;
    GT_BOOL                                         lmStampingEnable;
    GT_BOOL                                         meteringCountingStatus;
    GT_BOOL                                         origOamEnStatus;
    GT_BOOL                                         centralizedChassisModeEnable;
    GT_BOOL                                         ecmpEnableGet;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                    global;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                    ecmp;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                    dlb;
    GT_PORT_NUM                                     portBaseEport;
    GT_PORT_NUM                                     trunkBaseEport;
    GT_PORT_NUM                                     ecmpIndexBaseEportGet;
    GT_U32                                          srcTrunkLsbAmount;
    GT_U32                                          srcPortLsbAmount;
    GT_U32                                          srcDevLsbAmount;
    GT_HW_DEV_NUM                                   savedDevNum;
    PRV_TGF_TRUNK_LBH_GLOBAL_MODE_ENT               prvTgfDefHashMode;
    CPSS_CSCD_LINK_TYPE_STC                         cascadeLinkGet;
    PRV_TGF_DEV_MAP_LOOKUP_MODE_ENT                 lookupModeGet;
    PRV_TGF_CSCD_TRUNK_LINK_HASH_ENT                srcPortTrunkHashEn;
    GT_HW_DEV_NUM                                   targetHwDevNumGet;
    CPSS_INTERFACE_INFO_STC                         phyPortInfoGet;
} prvTgfRestoreCfg;

/*************************************************************************/
/*Falcon ERSPAN type II same dev mirroring*/
/*************************************************************************/
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC      interface;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           egressInfo;
    GT_BOOL                                    samplingEn;
    GT_U32                                     samplingRatio;
    CPSS_INTERFACE_INFO_STC                    phyPortInfo;
    PRV_TGF_OAM_COMMON_CONFIG_STC              oamConfig;
    GT_BOOL                                    oamEnStatus;
    PRV_TGF_POLICER_STAGE_METER_MODE_ENT       stageModeSave;
    GT_BOOL                                    meteringCountingStatus;
    GT_BOOL                                    lmStampingEnable;
    GT_BOOL                                    rxMirrorEnable;
    GT_BOOL                                    txMirrorEnable;
    GT_BOOL                                    rxMirrorIdx;
    GT_BOOL                                    txMirrorIdx;
    GT_BOOL                                    phaEnable;
    GT_U32                                     phaThreadId;
    GT_U16                                     pvid;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclPortAccessModeCfgGet;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclTsAccessModeCfgGet;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT    srcPortType;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT    srcPortInfo;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT    trgPortType;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT    trgPortInfo;
    CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_ENT    vlanCmd;
    GT_BOOL                                     vlanTagEnable;
    CPSS_BRG_TPID_SIZE_TYPE_ENT                 type[2];
    GT_U32                                      tpIdIndex[2];
    GT_U32                                      pushTpIdIndex;
    GT_BOOL                                     lmCntrEnable;
    CPSS_DXCH_POLICER_BILLING_INDEX_CFG_STC     dxChBillingIndexCfg;
    /* Analyzer index loopback config */
    GT_PHYSICAL_PORT_NUM                        loopbackPortNum;
    GT_BOOL                                     loopbackEnable;
    GT_BOOL                                     lbEnArr[CPSS_TC_RANGE_CNS];
    GT_U32                                      srcProfile;
    GT_U32                                      trgProfile;
    /* Cascade port */
    CPSS_CSCD_PORT_TYPE_ENT                     cscdPortType;
}prvTgfErspanSameDevRestoreCfg;
/* parts of the original packet */
static TGF_PACKET_PART_STC prvTgfOriginalPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };
/* final original packet totalLen */
#define PRV_TGF_PACKET_TOTAL_LEN_CNS    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + sizeof(prvTgfPayloadDataArr))

/* final original packet info */
static TGF_PACKET_STC prvTgfOriginalPacketInfo = {
    PRV_TGF_PACKET_TOTAL_LEN_CNS,                                        /* totalLen */
    sizeof(prvTgfOriginalPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfOriginalPacketPartsArray                                       /* partsArray */
};


/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/**
* @internal prvTgfIngressErspanBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*/
static GT_VOID prvTgfIngressErspanBridgeConfigSet
(
    GT_U32 dev
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8       tagArray[]  = {1, 1, 1, 1};

    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &prvTgfRestoreCfg.savedDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    rc = cpssDxChCfgHwDevNumSet(prvTgfDevNum, dev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgHwDevNumSet \n");

    /* TO_ANALYZER is a control packet and do not require any bridge procesing
     */
    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_ETHERNET_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_ETHERNET_VLANID_CNS);
}

/**
* @internal prvTgfFalconMirrorIngressErspanBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*
*    Note:
*    isOrigDevMirror is applicable for ERSPAN III CC use case only.
*    When this API is used for UTs other than for ERSPAN III CC use case feature,
*    this param is set to GT_TRUE.
*/
static GT_VOID prvTgfFalconMirrorIngressErspanBridgeConfigSet
(
    GT_U32 dev,
    GT_BOOL direction,
    GT_BOOL isOrigDevMirror
)
{
    GT_STATUS                                rc = GT_OK;
    GT_U8                                    tagArray[]  = {1, 1, 1, 1};
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC  interface;
    GT_HW_DEV_NUM                            hwDevNum;
    CPSS_MAC_ENTRY_EXT_STC                   macEntry;

    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &prvTgfRestoreCfg.savedDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    rc = cpssDxChCfgHwDevNumSet(prvTgfDevNum, dev);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCfgHwDevNumSet \n");
    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet %d\n",hwDevNum);

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with tagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_ETHERNET_VLANID_CNS,
            prvTgfPortsArray, NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfBrgDefVlanEntryWithPortsSet: %d", PRV_TGF_ETHERNET_VLANID_CNS);

    /* AUTODOC: set analyzer interface index=2: */
    /* AUTODOC:   analyzer devNum=0(remote device), port=1025(global ePort) */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum = TARGET_FALCON_DEVICE_NUMBER_CNS;
    interface.interface.devPort.portNum  = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS;

    rc = cpssDxChMirrorAnalyzerInterfaceSet(prvTgfDevNum, REMOTE_DEVICE_ANALYZER_NUM_CNS, &interface);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorAnalyzerInterfaceSet: ePort %d %d",
                                 PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS, GT_TRUE);

    if(isOrigDevMirror == GT_TRUE)
    {
        if(direction == 0)
        {
            /* AUTODOC: enable Rx mirroring based on ePort=3(CC default ePort), index=2 */
            rc = cpssDxChMirrorRxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE, GT_TRUE, REMOTE_DEVICE_ANALYZER_NUM_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                         prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
        }
        else
        {
            /* AUTODOC: enable Tx mirroring based on ePort=2(CC default ePort), index=2 */
            rc = cpssDxChMirrorTxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_FALSE, GT_TRUE, REMOTE_DEVICE_ANALYZER_NUM_CNS);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                         prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE);
        }
    }

    cpssOsMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));
    TGF_MASK_CURR_PORT_GROUPS_BMP_INSTANCE_PER_TILE_MAC(currPortGroupsBmp);

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_ETHERNET_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum= hwDevNum;
    macEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    macEntry.isStatic                     = GT_TRUE;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5*/
    rc = cpssDxChBrgFdbPortGroupMacEntrySet(prvTgfDevNum,currPortGroupsBmp,&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Disable bridge bypass */
    rc = cpssDxChCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
}

/**
* @internal prvTgfIngressErspanEcmpConfigSet function
* @endinternal
*
* @brief   L2 ECMP test configurations
*/
static GT_VOID prvTgfIngressErspanEcmpConfigSet
(
    GT_U8 dev
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                ecmp;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                global;
    PRV_TGF_CFG_GLOBAL_EPORT_STC                dlb;
    PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC           ecmpLttEntry;
    CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC             ecmpEntry;
    GT_U8                                       devNum;
    CPSS_CSCD_LINK_TYPE_STC                     cascadeLink;
    CPSS_INTERFACE_INFO_STC                     phyPortInfo;
    CPSS_INTERFACE_INFO_STC                     phyPortInfo2;
    GT_HW_DEV_NUM                               targetHwDevNum;
    GT_BOOL                                     egressAttributesLocallyEn;

    devNum = prvTgfDevNum;
    /* AUTODOC: SETUP CONFIGURATION: */

    /* AUTODOC: globally enable the L2ECMP */

    rc = prvTgfBrgL2EcmpEnableGet(devNum, &prvTgfRestoreCfg.ecmpEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpEnableGet");

    rc = prvTgfBrgL2EcmpEnableSet(devNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpEnableSet");

    /* create L2 ECMP LTT entries */
    cpssOsMemSet(&ecmpLttEntry, 0, sizeof(PRV_TGF_BRG_L2_ECMP_LTT_ENTRY_STC));
    ecmpLttEntry.ecmpNumOfPaths = PRV_TGF_NUM_OF_MEMBERS_IN_ECMP_GROUP_CNS; /* the number of members in the ECMP group */
    ecmpLttEntry.ecmpStartIndex = 0; /* the start index of the ECMP block; Each ECMP group is stored as a set of consecutive entries in this table, and the L2 ECMP
                                        LTT<L2 ECMP Start Index> indicates the first entry of an ECMP group */
    ecmpLttEntry.ecmpEnable = GT_TRUE; /* whether the packet is load balanced over an ECMP */
    ecmpLttEntry.hashBitSelectionProfile = 0;

    /* Map the global ePort PRV_TGF_DESTINATION_PRIMERY_EPORT to a pointer of linked-list of L2ECMP */
    /* AUTODOC: configure L2 ECMP LTT entry info: */
    /* AUTODOC:   ecmpNumOfPaths=2, ecmpStartIndex=0 */
    rc = prvTgfBrgL2EcmpLttTableGet(devNum, PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS, &prvTgfRestoreCfg.ecmpLttEntryGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpLttTableGet");
    rc = prvTgfBrgL2EcmpLttTableSet(devNum, PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS, &ecmpLttEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpLttTableSet");

    /* Defines the first ePort number in the L2 ECMP ePort number range.
       The index to the L2 ECMP LTT is <Target ePort>-<Base ePort> */
    /* AUTODOC: define first ePort 0 in the L2 ECMP ePort number range */
    rc = prvTgfBrgL2EcmpIndexBaseEportGet(devNum, &prvTgfRestoreCfg.ecmpIndexBaseEportGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpIndexBaseEportGet");
    rc = prvTgfBrgL2EcmpIndexBaseEportSet(devNum, PRV_TGF_INDEX_BASE_EPORT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpIndexBaseEportSet");

     /* create L2 ECMP entries */
    cpssOsMemSet(&ecmpEntry, 0, sizeof(CPSS_DXCH_BRG_L2_ECMP_ENTRY_STC));

    /* Set L2 ECMP entry 0 info */
    ecmpEntry.targetEport = (dev == (SOURCE_FALCON_DEVICE_NUMBER_CNS)) ? PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS :
                                                                         PRV_TGF_CC_DEFAULT_EPORT_CNS /*PRV_TGF_DESTINATION_SECONDARY1_EPORT_CNS*/;
    /* Idea is Source Falcon based on L2ECMP lookup must assign Target Falcon
     * device as Analyzer Trg Dev and Target Falcon based on L2ECMP lookup must
     * provide the final ERSPAN target destination as LC device number >=8 to
     * trigger PHA TrgDevLcMiroring thread based on EPCL which adds the
     * required ERSPAN tunnel
     */
    ecmpEntry.targetHwDevice = (dev == (SOURCE_FALCON_DEVICE_NUMBER_CNS)) ? (TARGET_FALCON_DEVICE_NUMBER_CNS) :
                                                                             TARGET_FALCON_DEVICE_NUMBER_CNS/*Target falcon is same device */;

    /* AUTODOC: set L2 ECMP entry 0 with targetEport=700 */
    rc = cpssDxChBrgL2EcmpTableSet(devNum, ECMP_ENTRY_INDEX_0_CNS, &ecmpEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpTableSet entry 0");

    /* save global ePort configuration */
    rc = prvTgfCfgGlobalEportGet(devNum, &prvTgfRestoreCfg.global , &prvTgfRestoreCfg.ecmp, &prvTgfRestoreCfg.dlb);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgGlobalEportGet);

    /* set PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS as global ePort: define global ePort range as 1024-2048 */
    cpssOsMemSet(&ecmp, 0, sizeof(PRV_TGF_CFG_GLOBAL_EPORT_STC));
    cpssOsMemSet(&global, 0, sizeof(PRV_TGF_CFG_GLOBAL_EPORT_STC));
    cpssOsMemSet(&dlb, 0, sizeof(PRV_TGF_CFG_GLOBAL_EPORT_STC));

    /* min/max range global ePort configuration */
    ecmp.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_ENABLE_RANGE_E;
    ecmp.minValue = 1024; /* support all global ePorts in range 1024-2048 */
    ecmp.maxValue = 2048; /* support all global ePorts in range 1024-2048 */

    global = ecmp;
    dlb.enable = PRV_TGF_CFG_GLOBAL_EPORT_CONFG_DISABLE_E;

    /* AUTODOC: configure Global ePorts ranges as 1024-2048 */
    rc = prvTgfCfgGlobalEportSet(devNum, &global , &ecmp, &dlb);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, prvTgfCfgGlobalEportSet);

    rc =  prvTgfCscdDevMapLookupModeGet(prvTgfDevNum,&prvTgfRestoreCfg.lookupModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    rc = prvTgfCscdDevMapLookupModeSet(PRV_TGF_DEV_MAP_LOOKUP_MODE_TRG_DEV_E);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set E2PHY mapping table for ePort PRV_TGF_CC_DEFAULT_EPORT_CNS */
    if(dev == TARGET_FALCON_DEVICE_NUMBER_CNS)
    {
        phyPortInfo.type = CPSS_INTERFACE_PORT_E;
        phyPortInfo.devPort.hwDevNum = TARGET_LC_DEVICE_NUMBER_CNS;
        phyPortInfo.devPort.portNum =  TARGET_LC_PORT_NUMBER_CNS;
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS, &prvTgfRestoreCfg.phyPortInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",dev, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS, &phyPortInfo);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",dev, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
    }
    if(dev == SOURCE_FALCON_DEVICE_NUMBER_CNS)
    {
        phyPortInfo2.type = CPSS_INTERFACE_PORT_E;
        phyPortInfo2.devPort.hwDevNum = SOURCE_FALCON_DEVICE_NUMBER_CNS;
        phyPortInfo2.devPort.portNum =  PRV_TGF_CC_DEFAULT_EPORT_CNS;
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS, &prvTgfRestoreCfg.phyPortInfoGet);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",dev, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
        rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS, &phyPortInfo2);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",dev, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
    }
    /* cascade link info */
    cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
    /* set this 'Remote device' to point to the device map table via cascade port */
    cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];
    targetHwDevNum = (dev == (SOURCE_FALCON_DEVICE_NUMBER_CNS)) ? TARGET_FALCON_DEVICE_NUMBER_CNS : TARGET_LC_DEVICE_NUMBER_CNS;
    prvTgfRestoreCfg.targetHwDevNumGet = targetHwDevNum;
    egressAttributesLocallyEn = (dev == (SOURCE_FALCON_DEVICE_NUMBER_CNS)) ? GT_FALSE : GT_TRUE;

    /* save the current cascade map table */
    rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, prvTgfRestoreCfg.targetHwDevNumGet,
            0,0,
            &(prvTgfRestoreCfg.cascadeLinkGet),&(prvTgfRestoreCfg.srcPortTrunkHashEn));
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = cpssDxChCscdDevMapTableSet(prvTgfDevNum, targetHwDevNum, 0, 0, 0, &cascadeLink,
                                    0, egressAttributesLocallyEn);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* set ALL the egress physical ports as 'egress' cascade port so we can
       capture with eDSA */
    /* AUTODOC: set egress physical ports as cascaded mode (TX direction) */
    if(dev == SOURCE_FALCON_DEVICE_NUMBER_CNS)
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",dev, prvTgfPortsArray[1]);
    }
    else
    {

        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",dev, prvTgfPortsArray[1]);

    }

    /* AUTODOC: setup my physical port of central device connected to remote device to be a cascade port */
    rc = prvTgfCscdPortTypeSet(devNum, CPSS_PORT_DIRECTION_RX_E,
                               prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

    rc = prvTgfCscdPortTypeSet(devNum, CPSS_PORT_DIRECTION_TX_E,
                               prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet");

}

/**
* @internal prvTgfIngressErspanTargetFalconHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*/
static GT_VOID prvTgfIngressErspanTargetFalconHaConfigSet
(
    GT_U32 protocol, /* 0:ipv4, 1:ipv6 */
    GT_BOOL isVoQ    /* 1: VoQ, 0: no VoQ */
)
{
    GT_STATUS                           rc = GT_OK;

    /* AUTODOC: Set the thread entry - PRV_TGF_PHA_THREAD_ID_TARGET_FALCON_INGRESS_ERSPAN_TYPE_II_IPV4_CNS */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    if(protocol == 0)/* IPv4 */
    {
        rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                         PRV_TGF_PHA_THREAD_ID_TARGET_FALCON_INGRESS_ERSPAN_TYPE_II_IPV4_CNS,
                                         &commonInfo,
                                         CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV4_E,
                                         &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
    }
    else
    {
        rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                         PRV_TGF_PHA_THREAD_ID_TARGET_FALCON_INGRESS_ERSPAN_TYPE_II_IPV6_CNS,
                                         &commonInfo,
                                         CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_TRG_DEV_LC_IPV6_E,
                                         &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
    }

    /* AUTODOC: Set the PHA ERSPAN device id in the shared memory */
    rc = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(prvTgfDevNum, PRV_TGF_PHA_SHARED_MEMORY_ERSPAN_DEVICE_ID_CNS, isVoQ);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaSharedMemoryErspanGlobalConfigSet");

    /* AUTODOC: Set the PHA ERSPAN entry in the shared memory */
    cpssOsMemSet(&erspanEntry, 0, sizeof(erspanEntry));

    cpssOsMemCpy(&erspanEntry.l2Info.macDa.arEther, &prvTgfMirroredPacketTunnelL2Part.daMac[0], sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(&erspanEntry.l2Info.macSa.arEther, &prvTgfMirroredPacketTunnelL2Part.saMac[0], sizeof(TGF_MAC_ADDR));

    erspanEntry.l2Info.tpid       = 0x8100;
    erspanEntry.l2Info.up         = 0;
    erspanEntry.l2Info.cfi        = 1;
    erspanEntry.l2Info.vid        = 1;

    erspanEntry.protocol          = protocol;
    if(protocol == 0)/* IPv4 */
    {
        erspanEntry.ipInfo.ipv4.dscp  = prvTgfMirroredPacketTunnelIpv4Part.typeOfService;
        erspanEntry.ipInfo.ipv4.flags = prvTgfMirroredPacketTunnelIpv4Part.flags;
        erspanEntry.ipInfo.ipv4.dscp  = prvTgfMirroredPacketTunnelIpv4Part.timeToLive;

        cpssOsMemCpy(&erspanEntry.ipInfo.ipv4.sipAddr.arIP[0], &prvTgfMirroredPacketTunnelIpv4Part.srcAddr[0], sizeof(TGF_IPV4_ADDR));
        cpssOsMemCpy(&erspanEntry.ipInfo.ipv4.dipAddr.arIP[0], &prvTgfMirroredPacketTunnelIpv4Part.dstAddr[0], sizeof(TGF_IPV4_ADDR));
    }
    else/* IPv6 */
    {
        erspanEntry.ipInfo.ipv6.tc            = prvTgfMirroredPacketTunnelIpv6Part.trafficClass;
        erspanEntry.ipInfo.ipv6.flowLabel     = prvTgfMirroredPacketTunnelIpv6Part.flowLabel;
        erspanEntry.ipInfo.ipv6.hopLimit      = prvTgfMirroredPacketTunnelIpv6Part.hopLimit;

        prvTgfIpv6Convert(&prvTgfMirroredPacketTunnelIpv6Part.srcAddr, &erspanEntry.ipInfo.ipv6.sipAddr);
        prvTgfIpv6Convert(&prvTgfMirroredPacketTunnelIpv6Part.dstAddr, &erspanEntry.ipInfo.ipv6.dipAddr);
    }

    rc = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(prvTgfDevNum, analyzerIndex1, &erspanEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet");
    rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E, PRV_TGF_PUSH_TAG_TPID_INDEX_CNS, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet: %d", prvTgfDevNum);

    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS, PRV_TGF_PUSH_TAG_TPID_INDEX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet: %d", prvTgfDevNum);

    /* Set Push Vlan Command */
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum,
                                               PRV_TGF_CC_DEFAULT_EPORT_CNS,
                                               CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_PASSENGER_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d", prvTgfDevNum);

    /* configure pushTag VLAN-ID */
    rc = cpssDxChBrgVlanPortPushedTagValueSet(prvTgfDevNum,
                                              PRV_TGF_CC_DEFAULT_EPORT_CNS,
                                              PRV_TGF_PUSH_TAG_ETHERNET_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagValueSet: %d", prvTgfDevNum);

}

/**
* @internal prvTgfIngressErspanSourceFalconHaConfigSet function
* @endinternal
*
* @brief   HA test configurations
*/
static GT_VOID prvTgfIngressErspanSourceFalconHaConfigSet
(
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol,
    GT_BOOL                       isVoq,
    GT_U32                        phaThreadId
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U16                              erspanDevId = 0;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT       extType = CPSS_DXCH_PHA_THREAD_TYPE___LAST___E;

    /* AUTODOC: Set the thread entry */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;
    if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
    {
        erspanDevId = PRV_TGF_PHA_SHARED_MEMORY_ERSPAN_DEVICE_ID_CNS;
        extType = CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_II_SRC_DEV_E;
        phaThreadId = PRV_TGF_PHA_THREAD_ID_SOURCE_FALCON_INGRESS_ERSPAN_TYPE_II_IPV4_CNS;
    }
    else if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        erspanDevId = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_SWITCH_ID_CNS;
        switch(phaThreadId)
        {
            case PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_ORIG_DEV_MIRROR_CNS:
                extType = CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_III_ORIG_DEV_E;
                break;
            case PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_TAGGED_CNS:
                extType = CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_III_TRG_DEV_IPV4_TAGGED_E;
                break;
            case PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_UNTAGGED_CNS:
                extType = CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_III_TRG_DEV_IPV4_UNTAGGED_E;
                break;
            case PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_TAGGED_CNS:
                extType = CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_III_TRG_DEV_IPV6_TAGGED_E;
                break;
            case PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_UNTAGGED_CNS:
                extType = CPSS_DXCH_PHA_THREAD_TYPE_CC_ERSPAN_TYPE_III_TRG_DEV_IPV6_UNTAGGED_E;
                break;
            default:
                /* Do Nothing */
                break;
        }
    }

    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     phaThreadId,
                                     &commonInfo,
                                     extType,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

    /* AUTODOC: Set the PHA ERSPAN device id in the shared memory */
    rc = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(prvTgfDevNum, erspanDevId, isVoq);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaSharedMemoryErspanGlobalConfigSet");

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /* AUTODOC: Set the PHA ERSPAN entry in the shared memory */
        cpssOsMemSet(&erspanEntry, 0, sizeof(erspanEntry));

        cpssOsMemCpy(&erspanEntry.l2Info.macDa.arEther, &prvTgfMirroredPacketTunnelL2Part.daMac[0], sizeof(TGF_MAC_ADDR));
        cpssOsMemCpy(&erspanEntry.l2Info.macSa.arEther, &prvTgfMirroredPacketTunnelL2Part.saMac[0], sizeof(TGF_MAC_ADDR));

        erspanEntry.l2Info.tpid       = 0x8100;
        erspanEntry.l2Info.up         = 0;
        erspanEntry.l2Info.cfi        = 1;
        erspanEntry.l2Info.vid        = PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS;

        erspanEntry.protocol          = protocol;
        if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
        {
            erspanEntry.ipInfo.ipv4.dscp  = prvTgfMirroredPacketTunnelIpv4Part.typeOfService;
            erspanEntry.ipInfo.ipv4.flags = prvTgfMirroredPacketTunnelIpv4Part.flags;
            erspanEntry.ipInfo.ipv4.dscp  = prvTgfMirroredPacketTunnelIpv4Part.timeToLive;

            cpssOsMemCpy(&erspanEntry.ipInfo.ipv4.sipAddr.arIP[0], &prvTgfMirroredPacketTunnelIpv4Part.srcAddr[0], sizeof(TGF_IPV4_ADDR));
            cpssOsMemCpy(&erspanEntry.ipInfo.ipv4.dipAddr.arIP[0], &prvTgfMirroredPacketTunnelIpv4Part.dstAddr[0], sizeof(TGF_IPV4_ADDR));
        }
        else/* IPv6 */
        {
            erspanEntry.ipInfo.ipv6.tc            = prvTgfMirroredPacketTunnelIpv6Part.trafficClass;
            erspanEntry.ipInfo.ipv6.flowLabel     = prvTgfMirroredPacketTunnelIpv6Part.flowLabel;
            erspanEntry.ipInfo.ipv6.hopLimit      = prvTgfMirroredPacketTunnelIpv6Part.hopLimit;

            prvTgfIpv6Convert(&prvTgfMirroredPacketTunnelIpv6Part.srcAddr, &erspanEntry.ipInfo.ipv6.sipAddr);
            prvTgfIpv6Convert(&prvTgfMirroredPacketTunnelIpv6Part.dstAddr, &erspanEntry.ipInfo.ipv6.dipAddr);
        }

        erspanEntry.hwId = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_HW_ID_CNS;

        rc = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(prvTgfDevNum, analyzerIndex1, &erspanEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet");
    }
}

/**
* @internal prvTgfIngressErspanEpclConfigSet function
* @endinternal
*
* @brief   EPCL test configurations
*/
static GT_VOID prvTgfIngressErspanEpclConfigSet
(
    GT_U32 protocol
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer Trg Dev[0:1] Analyzer ePort[12:7] */
        /* AUTODOC:   offset 55 Analyzer Trg Dev[9:2] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[2] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[0] = (PRV_TGF_CC_DEFAULT_EPORT_CNS&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfErspanEpclMetadataUdbInfo[0].byteMask;

    /* bits 0:5 is ePort[12:7], bit 6:7 is Analyzer Trg Dev[0:1] */
    pattern.ruleEgrUdbOnly.udb[1] = ((PRV_TGF_CC_DEFAULT_EPORT_CNS>>7)&0x3f)  | ((TARGET_LC_DEVICE_NUMBER_CNS&0x3) <<6);
    mask.ruleEgrUdbOnly.udb[1] = prvTgfErspanEpclMetadataUdbInfo[1].byteMask;

    /* bits 0:7 is Analyzer Trg Dev[2:9] */
    pattern.ruleEgrUdbOnly.udb[2] = (TARGET_LC_DEVICE_NUMBER_CNS>>2)&0xff;
    mask.ruleEgrUdbOnly.udb[2] = prvTgfErspanEpclMetadataUdbInfo[2].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[3] = (PRV_TGF_CC_DEFAULT_EPORT_CNS>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[3] = prvTgfErspanEpclMetadataUdbInfo[3].byteMask;

    pattern.ruleEgrUdbOnly.udb[4] = 0x06; /* egress marvell tagged */
    mask.ruleEgrUdbOnly.udb[4] = prvTgfErspanEpclMetadataUdbInfo[4].byteMask;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_TRUE;
    action.oam.oamProfile                        = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId                                = PRV_TGF_EPCL_FLOW_ID_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = (protocol == 0) ? THR_ID_6 : THR_ID_7;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex = 1;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfIngressErspanSourceFalconEpclConfigSet function
* @endinternal
*
* @brief   Source Falcon Ingress EPCL test configurations
*/
static GT_VOID prvTgfIngressErspanSourceFalconEpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;


    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");
    /* AUTODOC: enable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E */
        /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer Trg Dev[0:1] Analyzer ePort[12:7] */
        /* AUTODOC:   offset 55 Analyzer Trg Dev[9:2] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[2] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    pattern.ruleEgrUdbOnly.udb[0] = 0x06; /* egress marvell tagged mtag cmd TO_ANALYZER */
    mask.ruleEgrUdbOnly.udb[0] = prvTgfErspanSourceFalconEpclMetadataUdbInfo[0].byteMask;
    pattern.ruleEgrUdbOnly.udb[1] = 0x00; /* word2 bit21 eDSA reserved */
    mask.ruleEgrUdbOnly.udb[1] = prvTgfErspanSourceFalconEpclMetadataUdbInfo[1].byteMask;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = THR_ID_10;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfEgressErspanSourceFalconEpclConfigSet function
* @endinternal
*
* @brief   Source Falcon Ingress EPCL test configurations
*/
static GT_VOID prvTgfEgressErspanSourceFalconEpclConfigSet
(
    GT_VOID
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;


    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");
    /* AUTODOC: enable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_TCP_E */
        /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer Trg Dev[0:1] Analyzer ePort[12:7] */
        /* AUTODOC:   offset 55 Analyzer Trg Dev[9:2] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[2] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfErspanSourceFalconEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    pattern.ruleEgrUdbOnly.udb[0] = 0x06; /* egress marvell tagged mtag cmd TO_ANALYZER */
    mask.ruleEgrUdbOnly.udb[0] = prvTgfErspanSourceFalconEpclMetadataUdbInfo[0].byteMask;
    pattern.ruleEgrUdbOnly.udb[1] = 0x00; /* word2 bit21 eDSA reserved */
    mask.ruleEgrUdbOnly.udb[1] = prvTgfErspanSourceFalconEpclMetadataUdbInfo[1].byteMask;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = THR_ID_10;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfIngressErspanEoamConfigSet function
* @endinternal
*
* @brief   EOAM test configurations
*/
static GT_VOID prvTgfIngressErspanEoamConfigSet
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_OAM_COMMON_CONFIG_STC   oamConfig;

    PRV_UTF_LOG0_MAC("======= Setting EOAM Configuration =======\n");

    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));

    /* Get OAM common configuration and save it for restortion */
    prvTgfOamConfigurationGet(&oamConfig);
    cpssOsMemCpy(&(prvTgfRestoreCfg.oamConfig), &oamConfig, sizeof(oamConfig));

    /* AUTODOC: Set common OAM configurations */
    oamConfig.oamEntry.oamPtpOffsetIndex      = 0;
    oamConfig.oamEntry.opcodeParsingEnable    = GT_FALSE;
    oamConfig.oamEntry.lmCountingMode         = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
    oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;
    oamConfig.oamEntryIndex = PRV_TGF_EPCL_FLOW_ID_CNS;
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set OAM LM offset table */
    rc = prvTgfOamLmOffsetTableSet(oamIndex, PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 4*oamIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamLmOffsetTableSet");

    /* AUTODOC: save OAM Enabler status */
    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, &prvTgfRestoreCfg.origOamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableGet");

    /* AUTODOC: Enable OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}


/**
* @internal prvTgfIngressErspanEplrConfigSet function
* @endinternal
*
* @brief   EPLR test configurations
*/
static GT_VOID prvTgfIngressErspanEplrConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC   billingIndexCfg;
    PRV_TGF_POLICER_BILLING_ENTRY_STC       prvTgfBillingCntr;

    PRV_UTF_LOG0_MAC("======= Setting EPLR Configuration =======\n");

    /* Save stage mode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);
    /* AUTODOC: test works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Set Egress policer counter based on Flow Id */
    cpssOsMemSet(&billingIndexCfg, 0, sizeof(billingIndexCfg));
    billingIndexCfg.billingIndexMode       = PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
    billingIndexCfg.billingFlowIdIndexBase = PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS;
    billingIndexCfg.billingMinFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS;
    billingIndexCfg.billingMaxFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS;
    rc = prvTgfPolicerFlowIdCountingCfgSet(PRV_TGF_POLICER_STAGE_EGRESS_E, &billingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerFlowIdCountingCfgSet");

    /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerCountingWriteBackCacheFlush");

    /* AUTODOC: Configure counters mode */
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
    prvTgfBillingCntr.packetSizeMode  = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    prvTgfBillingCntr.lmCntrCaptureMode = PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
    prvTgfBillingCntr.billingCntrAllEnable = GT_TRUE;
    rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                                      PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerBillingEntrySet,prvTgfDevNum:%d", prvTgfDevNum);

    /* AUTODOC: Set LM counter capture enable */
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

    /* Get Egress to analyzer metering and counting status for restortion */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(prvTgfDevNum,
                                        &prvTgfRestoreCfg.meteringCountingStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet");

    /* AUTODOC: Enable Metering and counting for "TO_ANALYZER" packets */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");
}


/**
* @internal prvTgfIngressErspanErmrkConfigSet function
* @endinternal
*
* @brief   ERMRK test configurations
*/
static GT_VOID prvTgfIngressErspanErmrkConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting ERMRK Configuration =======\n");

    /* AUTODOC: save current state of LM stamping counter for restortion */
    rc = cpssDxChOamLmStampingEnableGet(prvTgfDevNum, &prvTgfRestoreCfg.lmStampingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChOamLmStampingEnableGet");

    /* AUTODOC: Disable Stamping of LM counter in packet (since fw does it instead) */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChOamLmStampingEnableSet");

}

/**
* @internal prvTgfIngressErspanCCConfigSet function
* @endinternal
*
* @brief   CC test configurations
*/
static GT_VOID prvTgfIngressErspanCCConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting CC Configuration =======\n");

    /* Centralized Chassis Mode Enable */
    rc = prvTgfCscdCentralizedChassisModeEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                            &prvTgfRestoreCfg.centralizedChassisModeEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisModeEnableGet");

    rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisModeEnableSet");

    /* configure default ePort mapping assignment on ingress centralized chassis enabled ports, */
    /* when packets are received from line-card port/trunk accordingly. */
    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                            &prvTgfRestoreCfg.portBaseEport, &prvTgfRestoreCfg.trunkBaseEport);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisLineCardDefaultEportBaseGet");

    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], PRV_TGF_TRUNK_BASE_EPORT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet");

    /* configure the amount of least significant bits taken from DSA tag
       for assigning a default source ePort on CC ports,
       for packets received from line-card device trunks/physical ports accordingly. */
    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountGet(&prvTgfRestoreCfg.srcTrunkLsbAmount,
                                                               &prvTgfRestoreCfg.srcPortLsbAmount,
                                                               &prvTgfRestoreCfg.srcDevLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountGet");
    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet(PRV_TGF_SRC_TRUNK_LSB_AMOUNT_CNS,
                                                               PRV_TGF_SRC_PORT_LSB_AMOUNT_CNS,
                                                               PRV_TGF_SRC_DEV_LSB_AMOUNT_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet");

}

/**
* internal prvTgfLcMirrorSourceFalconIngressErspanConfigSet function
* @endinternal
*
* @brief  LC Ingress Mirroring Source Falcon use case configurations
*/
GT_VOID prvTgfLcMirrorSourceFalconIngressErspanConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanSourceFalconHaConfigSet(PRV_TGF_ERSPAN_TYPE_II_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE, 0/* not relevant */);

    /* EPCL Configuration */
    prvTgfIngressErspanSourceFalconEpclConfigSet();

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();
}

/**
* internal prvTgfLcMirrorTargetFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   LC Ingress Mirroring Target Falcon use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconIngressErspanConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(0,GT_TRUE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(0);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* internal prvTgfLcMirrorTargetFalconIngressErspanIpv6ConfigSet function
* @endinternal
*
* @brief   LC Ingress Mirroring Target Falcon IPv6 use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconIngressErspanIpv6ConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(1,GT_TRUE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(1);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* internal prvTgfFalconMirrorSourceFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Ingress Mirroring Source Falcon use case configurations
*/
GT_VOID prvTgfFalconMirrorSourceFalconIngressErspanConfigSet
(
    GT_VOID
)
{

    /* CC Configuration */
    prvTgfIngressErspanCCConfigSet();

    /* Bridge Configuration */
    prvTgfFalconMirrorIngressErspanBridgeConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS,0,GT_TRUE);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanSourceFalconHaConfigSet(PRV_TGF_ERSPAN_TYPE_II_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE, 0/* not relevant */);

    /* EPCL Configuration */
    prvTgfIngressErspanSourceFalconEpclConfigSet();

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* internal prvTgfFalconMirrorTargetFalconIngressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Ingress Mirroring Target Falcon use case configurations
*/
GT_VOID prvTgfFalconMirrorTargetFalconIngressErspanConfigSet
(
    GT_VOID
)
{

    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(0,GT_TRUE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(0);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* internal prvTgfFalconMirrorTargetFalconIngressErspanIpv6ConfigSet function
* @endinternal
*
* @brief   Falcon Ingress Mirroring Target Falcon Ipv6 use case configurations
*/
GT_VOID prvTgfFalconMirrorTargetFalconIngressErspanIpv6ConfigSet
(
    GT_VOID
)
{

    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(1,GT_TRUE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(1);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* internal prvTgfLcMirrorSourceFalconEgressErspanConfigSet function
* @endinternal
*
* @brief  LC Egress Mirroring Source Falcon use case configurations
*/
GT_VOID prvTgfLcMirrorSourceFalconEgressErspanConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanSourceFalconHaConfigSet(PRV_TGF_ERSPAN_TYPE_II_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE, 0/* not relevant */);

    /* EPCL Configuration */
    prvTgfIngressErspanSourceFalconEpclConfigSet();

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();
}

/**
* internal prvTgfLcMirrorTargetFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   LC Egress Mirroring Target Falcon use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconEgressErspanConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(0,GT_TRUE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(0);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();
}

/**
* internal prvTgfLcMirrorTargetFalconWithoutVoQEgressErspanConfigSet function
* @endinternal
*
* @brief   LC Egress Mirroring Target Falcon use case configurations
*/
GT_VOID prvTgfLcMirrorTargetFalconWithoutVoQEgressErspanConfigSet
(
    GT_VOID
)
{
    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(0, GT_FALSE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(0);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();
}

/**
* internal prvTgfFalconMirrorSourceFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Egress Mirroring Source Falcon use case configurations
*/
GT_VOID prvTgfFalconMirrorSourceFalconEgressErspanConfigSet
(
    GT_VOID
)
{
    /* CC Configuration */
    prvTgfIngressErspanCCConfigSet();

    /* Bridge Configuration */
    prvTgfFalconMirrorIngressErspanBridgeConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS,1,GT_TRUE);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(SOURCE_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanSourceFalconHaConfigSet(PRV_TGF_ERSPAN_TYPE_II_CNS, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE, 0/* not relevant */);

    /* EPCL Configuration */
    prvTgfEgressErspanSourceFalconEpclConfigSet();

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* internal prvTgfFalconMirrorTargetFalconEgressErspanConfigSet function
* @endinternal
*
* @brief   Falcon Egress Mirroring Target Falcon use case configurations
*/
GT_VOID prvTgfFalconMirrorTargetFalconEgressErspanConfigSet
(
    GT_VOID
)
{

    /* Bridge Configuration */
    prvTgfIngressErspanBridgeConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* L2ECMP Configuration */
    prvTgfIngressErspanEcmpConfigSet(TARGET_FALCON_DEVICE_NUMBER_CNS);

    /* HA Configuration */
    prvTgfIngressErspanTargetFalconHaConfigSet(0,GT_TRUE);

    /* EPCL Configuration */
    prvTgfIngressErspanEpclConfigSet(0);

    /* EOAM Configuration */
    prvTgfIngressErspanEoamConfigSet();

    /* EPLR Configuration */
    prvTgfIngressErspanEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();

}

/**
* @internal prvTgfIngressMirrorErspanConfigRestore function
* @endinternal
*
* @brief   Ingress Mirroring ERSPAN configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore Mirroring Configuration
*          3. Restore HA Configuration
*          4. Restore EPCL Configuration
*          5. Restore EOAM Configuration
*          6. Restore EPLR Configuration
*          7. Restore ERMRK Configuration
*          8. Restore base Configuration
*/
GT_VOID prvTgfIngressMirrorErspanConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIter;

    PRV_UTF_LOG0_MAC("======= Restore configurations =======\n");

    /* -------------------------------------------------------------------------
     * 1. Restore HW device Configuration
     */
    /* AUTODOC: restore HW device number */
    rc = cpssDxChCfgHwDevNumSet(prvTgfDevNum, prvTgfRestoreCfg.savedDevNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* -------------------------------------------------------------------------
     * 2. Restore CC Configuration
     */
    /* AUTODOC: restore CC configuration */
    rc = prvTgfCscdCentralizedChassisModeEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], prvTgfRestoreCfg.centralizedChassisModeEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisModeEnableSet");

    rc = prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                          prvTgfRestoreCfg.portBaseEport, prvTgfRestoreCfg.trunkBaseEport);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisLineCardDefaultEportBaseSet");

    rc = prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet(prvTgfRestoreCfg.srcTrunkLsbAmount,
                                                               prvTgfRestoreCfg.srcPortLsbAmount,
                                                               prvTgfRestoreCfg.srcDevLsbAmount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdCentralizedChassisMappingDsaSrcLsbAmountSet");

    /* -------------------------------------------------------------------------
     * 3. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ETHERNET_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_ETHERNET_VLANID_CNS);

    /* AUTODOC: Enable bridge bypass */
    rc = cpssDxChCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");

    /* AUTODOC: Disable Rx mirroring based on ePort=3(CC default ePort), index=2 */
    rc = cpssDxChMirrorRxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE, GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE);
    /* AUTODOC: Disable Tx mirroring based on ePort=2(CC default ePort), index=2 */
    rc = cpssDxChMirrorTxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_FALSE, GT_FALSE, 0);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                 prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_FALSE);

    /* -------------------------------------------------------------------------
     * 4. Restore L2ECMP Configuration
     */
    /* AUTODOC: restore global ePort configuration */
    rc = prvTgfBrgL2EcmpEnableSet(prvTgfDevNum, prvTgfRestoreCfg.ecmpEnableGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpEnableGet");

    rc = prvTgfBrgL2EcmpIndexBaseEportSet(prvTgfDevNum, prvTgfRestoreCfg.ecmpIndexBaseEportGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgL2EcmpIndexBaseEportGet");

    rc = prvTgfCfgGlobalEportSet(prvTgfDevNum, &prvTgfRestoreCfg.global , &prvTgfRestoreCfg.ecmp, &prvTgfRestoreCfg.dlb);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK,rc);

    rc = cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS, &prvTgfRestoreCfg.phyPortInfoGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgEportToPhysicalPortTargetMappingTableSet: %d %d",prvTgfDevNum, PRV_TGF_CC_DEFAULT_EPORT_CNS);

    /* restore cascade device map lookup mode */
    rc = prvTgfCscdDevMapLookupModeSet(prvTgfRestoreCfg.lookupModeGet);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    rc = prvTgfCscdDevMapTableSet(prvTgfDevNum,prvTgfRestoreCfg.targetHwDevNumGet,
            0,0,
            &(prvTgfRestoreCfg.cascadeLinkGet),
            prvTgfRestoreCfg.srcPortTrunkHashEn,
            GT_FALSE);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* AUTODOC: (restore) set egress physical ports as regular network ports */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
        prvTgfPortsArray[3],
        CPSS_CSCD_PORT_NETWORK_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[0]);
    for(portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS - 1; portIter++)
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
            prvTgfPortsArray[portIter],
            CPSS_CSCD_PORT_NETWORK_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                     prvTgfDevNum, prvTgfPortsArray[1]);
    }

    /* -------------------------------------------------------------------------
     * 5. Restore HA Configuration
     */

    /* AUTODOC: Restore Push Vlan Configuration */
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum,
                                               PRV_TGF_CC_DEFAULT_EPORT_CNS,
                                               CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_NONE_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushVlanCommandSet: %d", prvTgfDevNum);


    /* -------------------------------------------------------------------------
     * 6. Restore EPCL Configuration
     */
    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* -------------------------------------------------------------------------
     * 7. Restore EOAM Configuration
     */
    /* AUTODOC: restore common EOAM configurations */
    prvTgfOamConfigurationSet(&prvTgfRestoreCfg.oamConfig);

    /* AUTODOC: restore OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, prvTgfRestoreCfg.origOamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");


    /* -------------------------------------------------------------------------
     * 8. Restore EPLR Configuration
     */
    /* AUTODOC: Restore Policer Egress to Analyzer metering and counting status */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum,
                                                prvTgfRestoreCfg.meteringCountingStatus);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");

    /* AUTODOC: Restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 stageModeSave);

    /* -------------------------------------------------------------------------
     * 9. Restore ERMRK Configuration
     */
    /* AUTODOC: Restore LM Counter status */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, prvTgfRestoreCfg.lmStampingEnable);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChOamLmStampingEnableSet");


    /* -------------------------------------------------------------------------
     * 10. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");

    rc = prvUtfHwDeviceNumberSet(prvTgfDevNum, prvTgfRestoreCfg.savedDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberSet \n");
}



/**
* internal tgfIngressMirrorErspanTypeIIVerification function
* @endinternal
*
* @brief  Ingress Mirroring ERSPAN Type II use case verification
*         Do the following:
*         - Save IPv4<Total length> field. Will be verified in next packet.
*         - Check GRE protocol = 0x88BE (ERSPAN Type II)
*         - Save GRE Sequence number. Will be verified in next packet.
*         - Check ERSPAN high part = 0x100000 (ERSPN<Ver>=1)
*         - Check ERSPAN low part = 0x0
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/


GT_VOID tgfIngressMirrorErspanTypeIIVerification
(
    IN  GT_U8   * packetBuf,
    IN  GT_U8   isLcMirror,
    IN  GT_BOOL direction,
    IN  GT_BOOL protocol,
    IN  GT_BOOL isVoQ
)
{
    GT_U32              pktOffset = 0, payloadPktOffset;
    GT_U32              greProtocolType, greFlags, erspanHighPart, erspanLowPart, erspanExpectedVal;
    GT_U32              i;
    GT_U32              daMacErr=0, saMacErr=0, passengerPayloadErr=0, saIpErr = 0, daIpErr = 0;
    GT_U32              recPassengerPayload, expPassengerPayload;
    TGF_PACKET_L2_STC   receivedMac;
    GT_U16              greFlagsExpectedVal, erspanExpectedSessId, erspanReceivedSessId;
    GT_U8               erspanExpectedCos, erspanReceivedCos, erspanExpectedEn, erspanReceivedEn, erspanExpectedT, erspanReceivedT;
    GT_U8               erspanExpectedIndexType, erspanReceivedIndexType, erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId;
    GT_U8               erspanExpectedIndexSrcDev, erspanReceivedIndexSrcDev, erspanExpectedIndexSrcPort, erspanReceivedIndexSrcPort;
    GT_U8               erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev, erspanExpectedIndexTrgPort, erspanReceivedIndexTrgPort;
    TGF_PACKET_L2_STC   erspanReceivedMacDa, erspanReceivedMacSa;
    TGF_IPV4_ADDR       erspanReceivedSrcAddr, erspanReceivedDstAddr;
    GT_U16              erspanReceivedSrc6Addr[8], erspanReceivedDst6Addr[8];
    GT_U16              daIp6Err = 0, saIp6Err = 0;
    GT_U16              ethTypeReceived, ethTypeExpected;

    cpssOsMemSet(&erspanReceivedMacDa.daMac[0], 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&erspanReceivedMacSa.saMac[0], 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&receivedMac.daMac[0], 0, sizeof(TGF_MAC_ADDR));
    cpssOsMemSet(&receivedMac.saMac[0], 0, sizeof(TGF_MAC_ADDR));

    PRV_UTF_LOG0_MAC("======= Packet verification of ERSPAN Type II =======\n");

    /* AUTODOC: Get ERSPAN MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        erspanReceivedMacDa.daMac[i] = packetBuf[pktOffset + i];
        erspanReceivedMacSa.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfMirroredPacketTunnelL2Part.daMac)];
        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += erspanReceivedMacDa.daMac[i] - prvTgfMirroredPacketTunnelL2Part.daMac[i] ;
        saMacErr += erspanReceivedMacSa.saMac[i] - prvTgfMirroredPacketTunnelL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfMirroredPacketTunnelL2Part.daMac[0],prvTgfMirroredPacketTunnelL2Part.daMac[1],prvTgfMirroredPacketTunnelL2Part.daMac[2],
                          prvTgfMirroredPacketTunnelL2Part.daMac[3],prvTgfMirroredPacketTunnelL2Part.daMac[4],prvTgfMirroredPacketTunnelL2Part.daMac[5],
                          erspanReceivedMacDa.daMac[0],erspanReceivedMacDa.daMac[1],erspanReceivedMacDa.daMac[2],
                          erspanReceivedMacDa.daMac[3],erspanReceivedMacDa.daMac[4],erspanReceivedMacDa.daMac[5]);
    }


    /*********************** Check Origin MAC SA ************************************/
    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfMirroredPacketTunnelL2Part.saMac[0],prvTgfMirroredPacketTunnelL2Part.saMac[1],prvTgfMirroredPacketTunnelL2Part.saMac[2],
                          prvTgfMirroredPacketTunnelL2Part.saMac[3],prvTgfMirroredPacketTunnelL2Part.saMac[4],prvTgfMirroredPacketTunnelL2Part.saMac[5],
                          erspanReceivedMacDa.saMac[0],erspanReceivedMacDa.saMac[1],erspanReceivedMacDa.saMac[2],
                          erspanReceivedMacDa.saMac[3],erspanReceivedMacDa.saMac[4],erspanReceivedMacDa.saMac[5]);
    }

    /*********************** ERSPAN TO_ANALYZER 8B DSA Check *********************************************/
    /* AUTODOC: Get ERSPAN TO_ANALYZER */
    pktOffset = PRV_TGF_TO_ANALYER_PKT_OFFSET_CNS;
    UTF_VERIFY_EQUAL0_STRING_MAC(0x2, (packetBuf[pktOffset] >> 6)&0x3,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in TO_ANALYZER 8B DSA Tag Command \n");

    UTF_VERIFY_EQUAL0_STRING_MAC((direction == 0) ? 0x1 : 0x0, (packetBuf[pktOffset+1] >> 2)&0x1,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in TO_ANALYZER 8B DSA rx_sniff \n");

    UTF_VERIFY_EQUAL0_STRING_MAC(erspanEntry.l2Info.vid, ((((packetBuf[pktOffset+2])&0xF) << 4) | (packetBuf[pktOffset+3]&0xFF)),
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in TO_ANALYZER 8B DSA VID \n");
    UTF_VERIFY_EQUAL0_STRING_MAC(erspanEntry.l2Info.up, ((packetBuf[pktOffset+2] >> 5)&0x7),
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in TO_ANALYZER 8B DSA UP \n");
    UTF_VERIFY_EQUAL0_STRING_MAC(erspanEntry.l2Info.cfi, ((packetBuf[pktOffset+1])&0x1),
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in TO_ANALYZER 8B DSA CFI \n");

    /*********************** ERSPAN Eth Type Check *********************************************/
    /* AUTODOC: Get ERSPAN Packet Ether Type */
    pktOffset = PRV_TGF_ETH_TYPE_PKT_OFFSET_CNS;
    ethTypeReceived = (packetBuf[pktOffset + 1] | (packetBuf[pktOffset] << 8));
    ethTypeExpected = (protocol == 0) ? TGF_ETHERTYPE_0800_IPV4_TAG_CNS : TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;
    UTF_VERIFY_EQUAL0_STRING_MAC(ethTypeExpected, ethTypeReceived,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in Ether Type \n");

    /*********************** IPv4 Total length *********************************************/
    /* AUTODOC: Get IPv4 Total length field */
    pktOffset = PRV_TGF_IP_PKT_OFFSET_CNS;
    ipTotalLength[0] = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8));

    pktOffset += (protocol == 0) ? PRV_TGF_IPV4_ADDR_OFFSET_CNS : PRV_TGF_IPV6_ADDR_OFFSET_CNS;

    if(protocol == 0)
    {
        /* AUTODOC: Get ERSPAN SRC IP & DST IP */
        for (i=0;i<(TGF_IPV4_HEADER_SIZE_CNS/5);i++)
        {
            erspanReceivedSrcAddr[i] = packetBuf[pktOffset + i];
            erspanReceivedDstAddr[i] = packetBuf[pktOffset + i + sizeof(prvTgfMirroredPacketTunnelIpv4Part.srcAddr)];

            /* Decrment received with expected to check if there is any mismatch */
            saIpErr += erspanReceivedSrcAddr[i] - prvTgfMirroredPacketTunnelIpv4Part.srcAddr[i] ;
            daIpErr += erspanReceivedDstAddr[i] - prvTgfMirroredPacketTunnelIpv4Part.dstAddr[i] ;
        }

        /* AUTODOC: compare received SRC IP vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, saIpErr,
                                    "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                    "Mismatch in SRC IP address");
        if (saIpErr != 0)
        {
            /* Print received and expected SRC IP */
            PRV_UTF_LOG8_MAC("Expected SRC IP ADDR:%.2x%.2x%.2x%.2x \n"
                              "Received SRC IP ADDR:%.2x%.2x%.2x%.2x \n",
                              prvTgfMirroredPacketTunnelIpv4Part.srcAddr[0],prvTgfMirroredPacketTunnelIpv4Part.srcAddr[1],
                              prvTgfMirroredPacketTunnelIpv4Part.srcAddr[2],prvTgfMirroredPacketTunnelIpv4Part.srcAddr[3],
                              erspanReceivedSrcAddr[0],erspanReceivedSrcAddr[1],
                              erspanReceivedSrcAddr[2],erspanReceivedSrcAddr[3]);
        }


        /*********************** Check Origin DST IP ************************************/
        /* AUTODOC: compare received DST IP vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, daIpErr,
                                    "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                    "Mismatch in DST IP address \n");
        if (daIpErr != 0)
        {
            /* Print received and expected DST IP */
            PRV_UTF_LOG8_MAC("Expected DST IP ADDR:%.2x%.2x%.2x%.2x \n"
                              "Received DST IP ADDR:%.2x%.2x%.2x%.2x \n",
                              prvTgfMirroredPacketTunnelIpv4Part.dstAddr[0],prvTgfMirroredPacketTunnelIpv4Part.dstAddr[1],
                              prvTgfMirroredPacketTunnelIpv4Part.dstAddr[2],prvTgfMirroredPacketTunnelIpv4Part.dstAddr[3],
                              erspanReceivedDstAddr[0],erspanReceivedDstAddr[1],
                              erspanReceivedDstAddr[2],erspanReceivedDstAddr[3]);
        }
    }
    else
    {

        /* AUTODOC: Get ERSPAN SRC IP & DST IP */
        for (i=0;i<16;i+=2)
        {
            erspanReceivedSrc6Addr[i/2] = (GT_U16)packetBuf[pktOffset + i + 1] | (packetBuf[pktOffset + i] << 8);
            erspanReceivedDst6Addr[i/2] = packetBuf[pktOffset + i + 1 + sizeof(prvTgfMirroredPacketTunnelIpv6Part.srcAddr)] |
                                        (packetBuf[pktOffset + i + sizeof(prvTgfMirroredPacketTunnelIpv6Part.srcAddr)] << 8);

            /* Decrment received with expected to check if there is any mismatch */
            saIp6Err = erspanReceivedSrc6Addr[i/2] - prvTgfMirroredPacketTunnelIpv6Part.srcAddr[i/2];
            daIp6Err = erspanReceivedDst6Addr[i/2] - prvTgfMirroredPacketTunnelIpv6Part.dstAddr[i/2];

            /* AUTODOC: compare received SRC IP vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, saIp6Err,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "Mismatch in SRC IP address");
            if (saIp6Err != 0)
            {
                /* Print received and expected SRC IP */
                PRV_UTF_LOG2_MAC("Expected byte of SRC IP ADDR:%.2x \n"
                                  "Received byte of SRC IP ADDR:%.2x \n",
                                  prvTgfMirroredPacketTunnelIpv6Part.srcAddr[i/2],
                                  erspanReceivedSrc6Addr[i/2]);
            }

            /*********************** Check Origin DST IP ************************************/
            /* AUTODOC: compare received DST IP vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL0_STRING_MAC(0, daIp6Err,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "Mismatch in DST IP address \n");
            if (daIp6Err != 0)
            {
                /* Print received and expected DST IP */
                PRV_UTF_LOG2_MAC("Expected byte of DST IP ADDR:%.2x \n"
                                  "Received byte of DST IP ADDR:%.2x \n",
                                  prvTgfMirroredPacketTunnelIpv6Part.dstAddr[i/2],
                                  erspanReceivedDst6Addr[i/2]);
            }
        }
    }

    pktOffset = PRV_TGF_IP_PKT_OFFSET_CNS;
    ipTotalLength[0] = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8));


    /*********************** Check GRE protocol *********************************************/
    /* Update offset to point to GRE header */
    pktOffset = (protocol == 0) ? PRV_TGF_IPV4_GRE_PKT_OFFSET_CNS : PRV_TGF_IPV6_GRE_PKT_OFFSET_CNS;

    /* AUTODOC: set expected value with GRE protocol type of ERSPAN II 0x88BE as defined in standart */
    erspanExpectedVal = PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS;

    /* AUTODOC: set expected value with GRE flags for ERSPAN II as defined in standart */
    greFlagsExpectedVal = PRV_TGF_GRE_FLAGS_ERSPAN_II_CNS;

    /* AUTODOC: get the first 2B of GRE header to verify GRE flags */
    greFlags = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));

    /* AUTODOC: compare received GRE flags vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(greFlagsExpectedVal, greFlags,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "GRE flags is not as expected => Expected:0x%x != Received:0x%x \n",
                                 greFlagsExpectedVal, greFlags);

    /* AUTODOC: get the next 2B of GRE header to verify GRE protocol */
    greProtocolType = (GT_U32)((packetBuf[pktOffset + 2] << 8) | packetBuf[pktOffset + 3]);

    /* AUTODOC: compare received GRE protocol type vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, greProtocolType,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "GRE protocol type is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, greProtocolType);


    /*********************** Get GRE Seq. number *********************************************/
    /* Increment offset by 4 bytes to point to GRE header low part  */
    pktOffset +=4;

    /* AUTODOC: Get the next 4B of GRE header to verify GRE Seq.number. Will be verified in next packet */
    lmCounter[0] = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                        (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: compare received GRE sequence number vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_BURST_COUNT_CNS, lmCounter[0],
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "GRE sequence number is not as expected => Expected:0x%x != Received:0x%x \n",
                                 PRV_TGF_BURST_COUNT_CNS, lmCounter[0]);


    /*********************** Check ERSPAN header high part ************************************/
    /* Increment offset by 4 bytes to point to ERSPAN header  */
    pktOffset +=4;

    /* AUTODOC: Get the first 2B of ERSPAN header to verify ERSPAN Version and rest of bits are zero */
    erspanHighPart = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));

    /* AUTODOC: set expected value. All bits should be zero except for ERSPAN<Ver> = 1 */
    erspanExpectedVal = 0x1000;

    /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanHighPart);

    /* AUTODOC: Get the next 2B of ERSPAN header to verify ERSPAN COS, En, T, Session ID */
    erspanHighPart = (GT_U32)((packetBuf[pktOffset + 2] << 8) | (packetBuf[pktOffset + 3]));

    /* AUTODOC: set expected ERSPAN COS value */
    erspanExpectedCos = prvTgfDsaTag.commonParams.vpt;

    /* AUTODOC: Get the ERSPAN COS */
    erspanReceivedCos = (erspanHighPart >> 13) & 0x7;

    /* AUTODOC: compare received ERSPAN header COS vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedCos, erspanReceivedCos,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedCos, erspanReceivedCos);

    /* AUTODOC: set expected ERSPAN En value */
    erspanExpectedEn = 0x3;

    /* AUTODOC: Get the ERSPAN En */
    erspanReceivedEn = (erspanHighPart >> 11) & 0x3;

    /* AUTODOC: compare received ERSPAN header En vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedEn, erspanReceivedEn,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedEn, erspanReceivedEn);

    /* AUTODOC: set expected ERSPAN T value */
    erspanExpectedT = 0x0;

    /* AUTODOC: Get the ERSPAN T */
    erspanReceivedT = (erspanHighPart >> 10) & 0x1;

    /* AUTODOC: compare received ERSPAN header En vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedT, erspanReceivedT,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedT, erspanReceivedT);

    /* AUTODOC: set expected ERSPAN session ID value */
    erspanExpectedSessId = 0x0;

    /* AUTODOC: Get the ERSPAN session ID */
    erspanReceivedSessId = erspanHighPart & 0x3FF;

    /* AUTODOC: compare received ERSPAN session ID vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedSessId, erspanReceivedSessId,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN Session ID is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedSessId, erspanReceivedSessId);

    /*********************** Check ERSPAN header low part ************************************/
    /* Increment offset by 4 bytes to point to ERSPAN header low part  */
    pktOffset +=4;

    /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
    erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                           (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: set expected value. All bits should be zero */
    erspanExpectedVal = 0x000;

    /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 20) & 0xFFF,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, (erspanLowPart >> 20) & 0xFFF);

    if (isLcMirror)
    {
        if(direction == 0)
        {
            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexType = 0x0;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexType = (erspanLowPart >> 18) & 0x3;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexType, erspanReceivedIndexType,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexType, erspanReceivedIndexType);

            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexFalconDevId = 0x2;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexFalconDevId = (erspanLowPart >> 14) & 0xF;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId);

            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexSrcDev = 0x9;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexSrcDev = (erspanLowPart >> 9) & 0x1F;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexSrcDev, erspanReceivedIndexSrcDev,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexSrcDev, erspanReceivedIndexSrcDev);

            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexSrcPort = 0x1;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexSrcPort = erspanLowPart & 0x1FF;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexSrcPort, erspanReceivedIndexSrcPort,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexSrcPort, erspanReceivedIndexSrcPort);
        }
        else
        {
            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexType = isVoQ ? 0x2 : 0x0;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexType = (erspanLowPart >> 18) & 0x3;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexType, erspanReceivedIndexType,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexType, erspanReceivedIndexType);

            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexFalconDevId = 0x2;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexFalconDevId = (erspanLowPart >> 14) & 0xF;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId);

            if(isVoQ)
            {
                /* AUTODOC: set expected value. */
                erspanExpectedIndexTrgDev = (prvTgfDevNum == (TARGET_LC_DEVICE_NUMBER_TX_MIRROR_CNS - 1)) ?
                                            (TARGET_LC_DEVICE_NUMBER_TX_MIRROR_CNS + 1) :
                                            (TARGET_LC_DEVICE_NUMBER_TX_MIRROR_CNS - 1);

                /* AUTODOC: set expected value. All bits should be zero */
                erspanReceivedIndexTrgDev = erspanLowPart & 0x3FFF;

                /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
                UTF_VERIFY_EQUAL3_STRING_MAC(erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev,
                                            "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                            "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x string 0x%x\n",
                                             erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev, erspanLowPart);
            }
            else
            {
                /* AUTODOC: set expected value. */
                erspanExpectedIndexTrgDev = (prvTgfDevNum == (TARGET_LC_DEVICE_NUMBER_CNS - 1)) ?
                                            (TARGET_LC_DEVICE_NUMBER_CNS + 1) :
                                            (TARGET_LC_DEVICE_NUMBER_CNS - 1);

                /* AUTODOC: set expected value. */
                erspanReceivedIndexTrgDev = (erspanLowPart >> 9) & 0x1F;

                /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
                UTF_VERIFY_EQUAL3_STRING_MAC(erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev,
                                            "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                            "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x string 0x%x\n",
                                             erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev, erspanLowPart);

                /* AUTODOC: set expected value. */
                erspanExpectedIndexTrgPort = SOURCE_LC_PORT_NUMBER_CNS;

                /* AUTODOC: set expected value. */
                erspanReceivedIndexTrgPort = erspanLowPart & 0x1FF;

                /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
                UTF_VERIFY_EQUAL3_STRING_MAC(erspanExpectedIndexTrgPort, erspanReceivedIndexTrgPort,
                                            "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                            "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x string 0x%x\n",
                                             erspanExpectedIndexTrgPort, erspanReceivedIndexTrgPort, erspanLowPart);
            }
        }
    }
    else
    {
        /* AUTODOC: set expected value. All bits should be zero */
        erspanExpectedIndexType = (direction == 0) ? 0x1 : 0x0;

        /* AUTODOC: set expected value. All bits should be zero */
        erspanReceivedIndexType = (erspanLowPart >> 18) & 0x3;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexType, erspanReceivedIndexType,
                                    "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedIndexType, erspanReceivedIndexType);

        /* AUTODOC: set expected value. All bits should be zero */
        erspanExpectedIndexFalconDevId = 0x2;

        /* AUTODOC: set expected value. All bits should be zero */
        erspanReceivedIndexFalconDevId = (erspanLowPart >> 14) & 0xF;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId,
                                    "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedIndexFalconDevId, erspanReceivedIndexFalconDevId);

        if(direction == 0)
        {
            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexSrcPort = 0x1;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexSrcPort = erspanLowPart & 0x3FFF;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexSrcPort, erspanReceivedIndexSrcPort,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexSrcPort, erspanReceivedIndexSrcPort);
        }
        else
        {
            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexTrgDev = 0x2;

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexTrgDev = (erspanLowPart >> 9) & 0x1F;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexTrgDev, erspanReceivedIndexTrgDev);

            /* AUTODOC: set expected value. All bits should be zero */
            erspanExpectedIndexTrgPort = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];

            /* AUTODOC: set expected value. All bits should be zero */
            erspanReceivedIndexTrgPort = erspanLowPart & 0x1FF;

            /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
            UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedIndexTrgPort, erspanReceivedIndexTrgPort,
                                        "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                        "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                         erspanExpectedIndexTrgPort, erspanReceivedIndexTrgPort);
        }
    }

    /*********************** Check Origin MAC DA ************************************/
    /* Increment offset by 4B to point to MAC DA  */
    pktOffset +=4;

    /* AUTODOC: Get actual MAC DA & MAC SA */
    for (i=0;i<(TGF_L2_HEADER_SIZE_CNS/2);i++)
    {
        receivedMac.daMac[i] = packetBuf[pktOffset + i];
        receivedMac.saMac[i] = packetBuf[pktOffset + i + sizeof(prvTgfOriginalPacketL2Part.daMac)];

        /* Decrment received with expected to check if there is any mismatch */
        daMacErr += receivedMac.daMac[i] - prvTgfOriginalPacketL2Part.daMac[i] ;
        saMacErr += receivedMac.saMac[i] - prvTgfOriginalPacketL2Part.saMac[i] ;
    }

    /* AUTODOC: compare received MAC DA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, daMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC DA address");
    if (daMacErr != 0)
    {
        /* Print received and expected MAC DA */
        PRV_UTF_LOG12_MAC("Expected MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC DA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.daMac[0],prvTgfOriginalPacketL2Part.daMac[1],prvTgfOriginalPacketL2Part.daMac[2],
                          prvTgfOriginalPacketL2Part.daMac[3],prvTgfOriginalPacketL2Part.daMac[4],prvTgfOriginalPacketL2Part.daMac[5],
                          receivedMac.daMac[0],receivedMac.daMac[1],receivedMac.daMac[2],
                          receivedMac.daMac[3],receivedMac.daMac[4],receivedMac.daMac[5]);
    }


    /*********************** Check Origin MAC SA ************************************/
    /* AUTODOC: compare received MAC SA vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, saMacErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in MAC SA address \n");
    if (saMacErr != 0)
    {
        /* Print received and expected MAC SA */
        PRV_UTF_LOG12_MAC("Expected MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n"
                          "Received MAC SA:%.2x%.2x%.2x%.2x%.2x%.2x \n",
                          prvTgfOriginalPacketL2Part.saMac[0],prvTgfOriginalPacketL2Part.saMac[1],prvTgfOriginalPacketL2Part.saMac[2],
                          prvTgfOriginalPacketL2Part.saMac[3],prvTgfOriginalPacketL2Part.saMac[4],prvTgfOriginalPacketL2Part.saMac[5],
                          receivedMac.saMac[0],receivedMac.saMac[1],receivedMac.saMac[2],
                          receivedMac.saMac[3],receivedMac.saMac[4],receivedMac.saMac[5]);
    }

    /*********************** Check Passenger payload ************************************/
    /* Increment offset by 12B of MACs to point to passenger payload (EtherType)  */
    pktOffset += TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS /* push tag */ + TGF_ETHERTYPE_SIZE_CNS /* ether type */;

    /* Anchor offset to passenger */
    payloadPktOffset = pktOffset;

    /* AUTODOC: Get actual passenger payload  */
    for (i=0; i< prvTgfPacketPayloadPart.dataLength; i+=4)
    {
        /* Update offset every 4 bytes */
        pktOffset = payloadPktOffset + i;

        /* Get actual payload */
        recPassengerPayload = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                                   (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        /* Get expected payload */
        expPassengerPayload = (GT_U32)(prvTgfPayloadDataArr[i+3] | (prvTgfPayloadDataArr[i+2] << 8) |
                                           (prvTgfPayloadDataArr[i+1] << 16) | (prvTgfPayloadDataArr[i] << 24));

        UTF_VERIFY_EQUAL2_STRING_MAC(expPassengerPayload, recPassengerPayload,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in passenger payload data expected 0x%0x, received 0x%0x\n",
                                expPassengerPayload, recPassengerPayload);
        /* Decrement received with expected to check if there is any mismatch */
        passengerPayloadErr += (recPassengerPayload - expPassengerPayload) ;
    }

    /* AUTODOC: compare received passenger payload vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, passengerPayloadErr,
                                "--- Failure in tgfEgressMirrorErspanTypeIIVerification --- \n"
                                "Mismatch in passenger payload data ");
}

/**
* internal tgfLcMirrorSourceFalconIngressErspanVerification function
* @endinternal
*
* @brief  Ingress Mirroring ERSPAN Type II use case verification
*         Do the following:
*         - Save IPv4<Total length> field. Will be verified in next packet.
*         - Check GRE protocol = 0x88BE (ERSPAN Type II)
*         - Save GRE Sequence number. Will be verified in next packet.
*         - Check ERSPAN high part = 0x100000 (ERSPN<Ver>=1)
*         - Check ERSPAN low part = 0x0
*         - Check MAC addresses of passenger are not damaged
*         - Check payload data of passenger is not damaged
*/


GT_VOID tgfLcMirrorSourceFalconIngressErspanVerification
(
    IN  GT_U8  * packetBuf,
    IN  GT_U8  srcDevId,
    IN  GT_BOOL direction
)
{
    GT_U32              pktOffset;
    TGF_DSA_CMD_ENT     dsaCommand;
    GT_BOOL             srcTrgTagged;
    GT_U8               srcDev;
    GT_BOOL             rxSniffer;
    GT_HW_DEV_NUM       analyzerHwTrgDev;
    GT_PORT_NUM         analyzerTrgEport;
    GT_BOOL             word2Extend;
    GT_BOOL             deviceIdSet;

    PRV_UTF_LOG0_MAC("======= Packet verification of Lc Mirror Source Falcon TO_ANALYZER eDSA =======\n");

    /*********************** Save IPv4 Total length *********************************************/
    /* AUTODOC: Get IPv4 Total length field and save it for later usage. Will be verified in next packet */
    pktOffset = PRV_TGF_EDSA_PKT_OFFSET_CNS;

    dsaCommand = (packetBuf[pktOffset] >> 6 & 0x3);
    UTF_VERIFY_EQUAL2_STRING_MAC(0x2, dsaCommand,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER dsa command is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x2, dsaCommand);

    srcTrgTagged = (packetBuf[pktOffset] >> 5 & 0x1);
    UTF_VERIFY_EQUAL2_STRING_MAC(0x1, srcTrgTagged,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER srcTrgTagged is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x1, srcTrgTagged);

    srcDev = (packetBuf[pktOffset] & 0x1F);
    UTF_VERIFY_EQUAL2_STRING_MAC(srcDevId, srcDev,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER srcDev is not as expected => Expected:0x%x != Received:0x%x \n",
                                 srcDevId, srcDev);

    rxSniffer = (packetBuf[pktOffset + 1] >> 2 & 0x1);
    if(direction)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(0x0, rxSniffer,
                                    "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                    "Lc Mirror Source Falcon TO_ANALYZER rxSniffer is not as expected => Expected:0x%x != Received:0x%x \n",
                                     0x0, rxSniffer);
    }
    else
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(0x1, rxSniffer,
                                    "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                    "Lc Mirror Source Falcon TO_ANALYZER rxSniffer is not as expected => Expected:0x%x != Received:0x%x \n",
                                     0x1, rxSniffer);
    }

    pktOffset += 4;
    analyzerHwTrgDev = (((packetBuf[pktOffset] & 0x3) << 10) | ((packetBuf[pktOffset + 1]) << 2) | (packetBuf[pktOffset + 2] >> 6 & 0x3));
    UTF_VERIFY_EQUAL2_STRING_MAC(0x0, analyzerHwTrgDev,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER Trg Dev is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x0, analyzerHwTrgDev);

    pktOffset += 4;
    word2Extend = (packetBuf[pktOffset + 1] >> 5 & 0x1);
    UTF_VERIFY_EQUAL2_STRING_MAC(0x1, word2Extend,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER DSA tag is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x1, word2Extend);

    deviceIdSet = (packetBuf[pktOffset + 1] >> 6 & 0x3) | ((packetBuf[pktOffset] & 0x1) << 2);
    UTF_VERIFY_EQUAL2_STRING_MAC(0x2, deviceIdSet,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER Falcon Device Id Set is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x1, deviceIdSet);

    pktOffset += 4;
    analyzerTrgEport = ((packetBuf[pktOffset + 1] << 9) | ((packetBuf[pktOffset + 2]) << 1) | (packetBuf[pktOffset + 3] >> 7 & 0x1));
    UTF_VERIFY_EQUAL2_STRING_MAC(0x401, analyzerTrgEport,
                                "--- Failure in tgfLcMirrorSourceFalconIngressErspanVerification --- \n"
                                "Lc Mirror Source Falcon TO_ANALYZER Trg ePort is not as expected => Expected:0x%x != Received:0x%x \n",
                                 0x401, analyzerTrgEport);
}

/**
* internal prvTgfLcMirrorSourceFalconIngressErspanVerification function
* @endinternal
*
* @brief  Ingress Mirroring ERSPAN use case verification
*         Do the following:
*         - Loop over 2 iterations:
*              ^ First one check packet of ERSPAN Type II
*              ^ Second one check packet of ERSPAN Type III
*         - Check Policer billing counters are set to 2 packets
*/

/**
* internal prvTgfLcMirrorSourceFalconIngressErspanVerification function
* @endinternal
*
* @brief  Ingress Mirroring ERSPAN use case verification
*         Do the following:
*         - Loop over 2 iterations:
*              ^ First one check packet of ERSPAN Type II
*              ^ Second one check packet of ERSPAN Type III
*         - Check Policer billing counters are set to 2 packets
*/
GT_VOID prvTgfLcMirrorSourceFalconIngressErspanVerification
(
    GT_U8 srcDev,
    GT_BOOL direction
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}};
    GT_U32                              packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_ERSPAN_II_CNS};
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U8                               i = 0;
    GT_BOOL                             getFirst = GT_TRUE;


    /* Loop over 2 packets while first packet is with ERSPAN Type II and second with ERSPAN Type III */
    for (i = 0; i<PRV_TGF_BURST_COUNT_CNS; i++)
    {
        /* AUTODOC: Get the captured packets */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[i],
                                                    &packetLen[i], &packetActualLength,
                                                    &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

        /* TO_ANALYZER eDSA */
        tgfLcMirrorSourceFalconIngressErspanVerification(packetBuf[i], srcDev, direction);

        /* Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }
}

/**
* internal prvTgfIngressMirrorErspanVerification function
* @endinternal
*
* @brief  Ingress Mirroring ERSPAN use case verification
*         Do the following:
*         - Loop over 2 iterations:
*              ^ First one check packet of ERSPAN Type II
*              ^ Second one check packet of ERSPAN Type III
*         - Check Policer billing counters are set to 2 packets
*/
GT_VOID prvTgfIngressMirrorErspanVerification
(
    GT_BOOL isLcMirror,
    GT_BOOL direction,
    GT_BOOL protocol,
    GT_BOOL isVoQ
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_POLICER_BILLING_ENTRY_STC   prvTgfBillingCntr;
    GT_U8                               packetBuf[PRV_TGF_BURST_COUNT_CNS][PRV_TGF_MAX_PACKET_SIZE_CNS] = {{0}};
    GT_U32                              packetLen[PRV_TGF_BURST_COUNT_CNS] = {PRV_TGF_PACKET_SIZE_ERSPAN_II_CNS};
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_U8                               i = 0;
    GT_BOOL                             getFirst = GT_TRUE;


    /* Loop over 2 packets while first packet is with ERSPAN Type II and second with ERSPAN Type III */
    for (i = 0; i<PRV_TGF_BURST_COUNT_CNS; i++)
    {
        /* AUTODOC: Get the captured packets */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                                                    TGF_PACKET_TYPE_CAPTURE_E,
                                                    getFirst, GT_TRUE, packetBuf[i],
                                                    &packetLen[i], &packetActualLength,
                                                    &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

        /* ERSPAN Type II */
        tgfIngressMirrorErspanTypeIIVerification(packetBuf[i], isLcMirror, direction,protocol,isVoQ);

        /* Set to false for next iteration in order to get the next packet from next Rx CPU entry */
        getFirst = GT_FALSE;
    }

    /* AUTODOC: Get Policer Billing Counters */
    rc = prvTgfPolicerBillingEntryGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      GT_FALSE, &prvTgfBillingCntr);

    /* AUTODOC: Verify Policer Billing Counters */
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerBillingEntryGet");
    UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_BURST_COUNT_CNS, prvTgfBillingCntr.greenCntr.l[0],
                                 "billingCntr_Green mismatch => Expected:%d != Received:%d",
                                 PRV_TGF_BURST_COUNT_CNS, prvTgfBillingCntr.greenCntr.l[0]);
}


/**
* @internal prvTgfIngressMrrorTargetFalconErspanTest function
* @endinternal
*
* @brief   Ingress Mirroring ERSPAN use case test.
*          Perform Ingress mirroring over IP GRE plus ERSPAN header tunnel.
*          During this process PHA fw thread is triggered in which ERSPAN header is added.
*          Send 1 packet with ERSPAN Type II
*          At the end packets are captured and content is checked to verify right behaviour.
*/
GT_VOID prvTgfIngressMirrorTargetFalconErspanTest
(
    GT_BOOL direction, /* 0:ingress 1:egress */
    GT_BOOL isVoQ
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;
    GT_HW_DEV_NUM                   hwDevNum = 0;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Analyzer Port \n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************** FILL EDSA'S AND PACKET'S STRUCTURES ******************/

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */

    /* set vlan id inside a packet's vlan tag and inside an eDSA tag */
    prvTgfPacketVlanTag0Part.vid       = prvTgfVlanId;
    prvTgfDsaTag.commonParams.vid      = prvTgfPacketVlanTag0Part.vid;

    /* eDSA rx sniffed port's attributes */
    if(direction == 0) /* ingress erspan */
    {
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.hwDevNum = SOURCE_LC_DEVICE_NUMBER_CNS;                  /* dummy device LC */
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.portNum  = SOURCE_LC_PORT_NUMBER_CNS;                    /* dummy LC port   */
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.ePort    = SOURCE_LC_PORT_NUMBER_CNS;                    /* dummy LC eport  */

        prvTgfDsaInfo_rxMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = TARGET_FALCON_DEVICE_NUMBER_CNS;       /* target falcon */
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

        /* assign an analyzer part of eDSA tag at run time because a union is used */
        prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_rxMirrorToAnalyzer;
    }
    else /* egress erspan */
    {
        /* Need to use hwDevNum which is not in the DB - which means
           prvCpssPpconfig[swDevNum] == NULL, otherwise TG engine will use the hwDevNum
           from the config register when building the packet */
        hwDevNum = (isVoQ == GT_TRUE) ? TARGET_LC_DEVICE_NUMBER_TX_MIRROR_CNS :
                                        TARGET_LC_DEVICE_NUMBER_CNS;           /* LC port id */
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.hwDevNum = (prvTgfDevNum == (hwDevNum - 1)) ?
                                                            (hwDevNum + 1) : (hwDevNum - 1);
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.portNum  = SOURCE_LC_PORT_NUMBER_CNS;                    /* dummy LC port   */
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.ePort    = SOURCE_LC_PORT_NUMBER_CNS;                    /* dummy LC eport  */
        prvTgfDsaInfo_txMirrorToAnalyzer.rxSniffer        = GT_FALSE;                                     /* egress mirroring  */

        prvTgfDsaInfo_txMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = TARGET_FALCON_DEVICE_NUMBER_CNS;       /* target falcon */
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

        /* assign an analyzer part of eDSA tag at run time because a union is used */
        prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_txMirrorToAnalyzer;
    }

    /* Set input packet interface structure  */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCscdTaggedPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    PRV_UTF_LOG0_MAC("======= Print Captured Packet and counters on Analyzer port =======\n");
    /* Print captured packets from analyzer port */
    PRV_UTF_LOG1_MAC("Port [%d] Mirrored capturing:\n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");

    /* Read counters from analyzer port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
}

/**
* @internal prvTgfFalconIngressMirrorTargetFalconErspanTest function
* @endinternal
*
* @brief   Ingress Mirroring ERSPAN use case test.
*          Perform Ingress mirroring over IP GRE plus ERSPAN header tunnel.
*          During this process PHA fw thread is triggered in which ERSPAN header is added.
*          Send 1 packet with ERSPAN Type II
*          At the end packets are captured and content is checked to verify right behaviour.
*/
GT_VOID prvTgfFalconIngressMirrorTargetFalconErspanTest
(
    GT_BOOL direction /* 0:ingress 1:egress */
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Analyzer Port \n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Analyzer Port \n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
    /* Enable capture on target port */
    targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************** FILL EDSA'S AND PACKET'S STRUCTURES ******************/

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */
    cpssOsMemSet(&prvTgfDsaTag, 0, sizeof(TGF_PACKET_DSA_TAG_STC));

    /* set vlan id inside a packet's vlan tag and inside an eDSA tag */
    prvTgfPacketVlanTag0Part.vid             = prvTgfVlanId;
    prvTgfDsaTag.commonParams.vid            = prvTgfPacketVlanTag0Part.vid;
    prvTgfDsaTag.dsaInfo.toAnalyzer.isTagged = GT_TRUE;
    prvTgfDsaTag.dsaCommand                  = TGF_DSA_CMD_TO_ANALYZER_E;
    prvTgfDsaTag.dsaType                     = TGF_DSA_4_WORD_TYPE_E;

    /* eDSA rx sniffed port's attributes */
    if(direction == 0) /* ingress erspan */
    {
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.hwDevNum = SOURCE_FALCON_DEVICE_NUMBER_CNS;                  /* source falcon device */
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.portNum  = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];   /* source falcon physical port   */
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.ePort    = SOURCE_LC_PORT_NUMBER_CNS;                        /* source falcon ePort  */

        prvTgfDsaInfo_rxMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = TARGET_FALCON_DEVICE_NUMBER_CNS;       /* target falcon */
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_CC_DEFAULT_EPORT_CNS/*PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS*/; /* global eport */

        /* assign an analyzer part of eDSA tag at run time because a union is used */
        prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_rxMirrorToAnalyzer;
    }
    else /* egress erspan */
    {
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.hwDevNum = SOURCE_FALCON_DEVICE_NUMBER_CNS;                  /* source falcon device */
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.portNum  = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];   /* source falcon physical port   */
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.ePort    = SOURCE_LC_PORT_NUMBER_CNS;                        /* source falcon ePort  */

        prvTgfDsaInfo_txMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = TARGET_FALCON_DEVICE_NUMBER_CNS;       /* target falcon */
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

        /* assign an analyzer part of eDSA tag at run time because a union is used */
        prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_txMirrorToAnalyzer;
    }
    /* Set input packet interface structure  */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCscdTaggedPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
    /* Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    PRV_UTF_LOG0_MAC("======= Print Captured Packet and counters on Analyzer port =======\n");
    /* Print captured packets from analyzer port */
    PRV_UTF_LOG1_MAC("Port [%d] Mirrored capturing:\n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");

    /* Read counters from analyzer port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
}

/**
* @internal prvTgfIngressMirrorSourceFalconErspanTest function
* @endinternal
*
* @brief   Ingress Mirroring ERSPAN use case test.
*          Perform Ingress mirroring over IP GRE plus ERSPAN header tunnel.
*          During this process PHA fw thread is triggered in which ERSPAN header is added.
*          Send 1 packet with ERSPAN Type II.
*          At the end packets are captured and content is checked to verify right behaviour.
*/
GT_VOID prvTgfIngressMirrorSourceFalconErspanTest
(
    GT_BOOL direction /* 0:ingress 1:egress */
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Analyzer Port \n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************** FILL EDSA'S AND PACKET'S STRUCTURES ******************/

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */
    cpssOsMemSet(&prvTgfDsaTag, 0, sizeof(TGF_PACKET_DSA_TAG_STC));

    /* set vlan id inside a packet's vlan tag and inside an eDSA tag */
    prvTgfPacketVlanTag0Part.vid             = prvTgfVlanId;
    prvTgfDsaTag.commonParams.vid            = prvTgfPacketVlanTag0Part.vid;
    prvTgfDsaTag.dsaInfo.toAnalyzer.isTagged = GT_TRUE;
    prvTgfDsaTag.dsaCommand                  = TGF_DSA_CMD_TO_ANALYZER_E;
    prvTgfDsaTag.dsaType                     = TGF_DSA_4_WORD_TYPE_E;

    /* eDSA rx sniffed port's attributes */
    if(direction == 0) /* ingress erspan */
    {
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.hwDevNum = SOURCE_LC_DEVICE_NUMBER_CNS;                  /* dummy device LC */
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.portNum  = SOURCE_LC_PORT_NUMBER_CNS;                    /* dummy LC port   */
        prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.ePort    = SOURCE_LC_EPORT_NUMBER_CNS;                   /* dummy LC eport  */

        prvTgfDsaInfo_rxMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = SOURCE_FALCON_CHECK_DEVICE_NUMBER_CNS ;/* source falcon */
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
        prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

        /* assign an analyzer part of eDSA tag at run time because a union is used */
        prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_rxMirrorToAnalyzer;
    }
    else /* egress erspan */
    {
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.hwDevNum = SOURCE_LC_DEVICE_NUMBER_CNS;                  /* dummy device LC */
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.portNum  = SOURCE_LC_PORT_NUMBER_CNS;                    /* dummy LC port   */
        prvTgfDsaInfo_txMirrorToAnalyzer.devPort.ePort    = SOURCE_LC_EPORT_NUMBER_CNS;                   /* dummy LC eport  */
        prvTgfDsaInfo_txMirrorToAnalyzer.rxSniffer        = GT_FALSE;                                     /* egress mirroring  */

        prvTgfDsaInfo_txMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = SOURCE_FALCON_CHECK_DEVICE_NUMBER_CNS ;/* source falcon */
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
        prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

        /* assign an analyzer part of eDSA tag at run time because a union is used */
        prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_txMirrorToAnalyzer;
    }

    /* Set input packet interface structure  */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCscdTaggedPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");
    PRV_UTF_LOG0_MAC("======= Print Captured Packet and counters on Analyzer port =======\n");
    /* Print captured packets from analyzer port */
    PRV_UTF_LOG1_MAC("Port [%d] Mirrored capturing:\n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");
    /* Read counters from analyzer port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                    GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}

/**
* @internal prvTgfFalconIngressMirrorErspanTest function
* @endinternal
*
* @brief   Ingress Mirroring ERSPAN use case test.
*          Perform Ingress mirroring over IP GRE plus ERSPAN header tunnel.
*          During this process PHA fw thread is triggered in which ERSPAN header is added.
*          Send 1 packet with ERSPAN Type II.
*          At the end packets are captured and content is checked to verify right behaviour.
*/
GT_VOID prvTgfFalconIngressMirrorErspanTest
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;
    CPSS_INTERFACE_INFO_STC         targetPortInterface;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Analyzer Port \n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /**************** FILL EDSA'S AND PACKET'S STRUCTURES ******************/

    /* AUTODOC: initialize eDSA fields that can be filled only at runtime. */

   /* AUTODOC: complete an initialization of packets structures  */
    prvTgfDsaInfo_forward.origSrcPhy.portNum  = REMOTE_LC_PORT_NUM_CNS/*SOURCE_LC_PORT_NUMBER_CNS*/;

    prvTgfDsaInfo_forward.srcHwDev            = SOURCE_LC_DEVICE_NUMBER_CNS;
    prvTgfDsaInfo_forward.source.portNum      = REMOTE_LC_PORT_NUM_CNS/*SOURCE_LC_PORT_NUMBER_CNS*/;

    /* bind the DSA tag FORWARD part at run time because a union is used */
    prvTgfDsaTag.dsaInfo.forward    = prvTgfDsaInfo_forward;
    prvTgfDsaTag.dsaCommand         = TGF_DSA_CMD_FORWARD_E;
    prvTgfDsaTag.commonParams.vid   = prvTgfVlanId;

    /* Set input packet interface structure  */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCscdTaggedPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    PRV_UTF_LOG0_MAC("======= Print Captured Packet and counters on Target port =======\n");
    /* Print captured packets from target port */
    PRV_UTF_LOG1_MAC("Port [%d] Original capturing:\n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");

    /* Read counters from target port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                   GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");

     PRV_UTF_LOG0_MAC("======= Print Captured Packet and counters on Analyzer port =======\n");
     /* Print captured packets from analyzer port */
    PRV_UTF_LOG1_MAC("Port [%d] Mirrored capturing:\n", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    rc = prvTgfPortCapturedPacketPrint(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
     PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in prvTgfPortCapturedPacketPrint");

     /* Read counters from analyzer port */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                    GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
}

/*************************************************************************/
/*Falcon ERSPAN type II same dev mirroring*/
/*************************************************************************/

/**
* @internal prvTgfFalconErspanSameDevMirrorBridgeConfigSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfFalconErspanSameDevMirrorBridgeConfigSet()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryStore(1);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", 1);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", 1);
    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /*remove all, except ingress, remote ports from vlan 5*/
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", PRV_TGF_VLANID_CNS);
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.pvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d", PRV_TGF_VLANID_CNS);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d", PRV_TGF_VLANID_CNS);
}
/**
* @internal prvTgfFalconErspanSameDevMirrorConfig function
* @endinternal
*
* @brief   Ingress mirror config
*           Enable source port mirroring for RX port and Set analyzer index to 1
*           map analyzer index to eport, and set sampling ratio
*           map eport to phy port#2 and enable TS for the eport
*           set source port and analyzer port loopback profile to 1
*           enable loopback and assign a loopback port to analyzer index config
*           enable FTL for loopback profiles {source #1, target #1} & for all traffic classes
*
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorConfig
(
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{

    GT_STATUS rc;
    GT_U32    i;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    CPSS_INTERFACE_INFO_STC phyPortInfo;

    /* AUTODOC: Save analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex1, &prvTgfErspanSameDevRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, eport = 1024 , phy port=2 */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = PRV_TGF_DEFAULT_EPORT_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex1, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex1);

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfErspanSameDevRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &phyPortInfo );
    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(prvTgfDevNum, analyzerIndex1, &prvTgfErspanSameDevRestoreCfg.samplingEn, &prvTgfErspanSameDevRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet");
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, analyzerIndex1, GT_TRUE, PRV_TGF_SAMPLE_MIRROR_RATE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        /* AUTODOC: Save source port loopback profile */
        rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                            CPSS_DIRECTION_INGRESS_E, &prvTgfErspanSameDevRestoreCfg.srcProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileGet");

        /* AUTODOC: set source port loopback profile to 1 */
        rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                            CPSS_DIRECTION_INGRESS_E, PRV_TGF_LB_PROFILE_1_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

        /* AUTODOC: Save analyzer port loopback profile */
        rc = cpssDxChPortLoopbackProfileGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                            CPSS_DIRECTION_EGRESS_E, &prvTgfErspanSameDevRestoreCfg.trgProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileGet");

        /* AUTODOC: set target port loopback profile to 1 */
        rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                            CPSS_DIRECTION_EGRESS_E, PRV_TGF_LB_PROFILE_1_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

        /* AUTODOC: Save TO_ANALYZER loopback config */
        rc = cpssDxChPortLoopbackPktTypeToAnalyzerGet(prvTgfDevNum, analyzerIndex1,
                                                 &prvTgfErspanSameDevRestoreCfg.loopbackPortNum,
                                                 &prvTgfErspanSameDevRestoreCfg.loopbackEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeToAnalyzerGet");

        /* AUTODOC:  Enable loopback and assign a loopback port to analyzer index config */
        rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum, analyzerIndex1,
                                                      prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeToAnalyzerSet");

        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            /* AUTODOC: save general FTL trigger status */
            rc = cpssDxChPortLoopbackEnableGet(prvTgfDevNum,
                                               PRV_TGF_LB_PROFILE_1_CNS,
                                               PRV_TGF_LB_PROFILE_1_CNS,
                                               i,
                                               &prvTgfErspanSameDevRestoreCfg.lbEnArr[i]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableGet, i:%d", i);

            /* AUTODOC: enable FTL for loopback profiles {source #1, target #1} & for all traffic classes */
            rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                               PRV_TGF_LB_PROFILE_1_CNS,
                                               PRV_TGF_LB_PROFILE_1_CNS,
                                               i,
                                               GT_TRUE);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);
        }
    }
}

/**
* @internal prvTgfFalconErspanSameDevMirrorEpclConfigSet function
* @endinternal
*
* @brief   EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB keys with eport, egr mtag cmd = TO_ANALYZER
*           Action = trigger PHA thread, flow based OAM
* @note    isFirstPass is applicable for ERSPAN III ipv6 packets only
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorEpclConfigSet
(
    GT_U32             protocol,
    CPSS_DIRECTION_ENT dir,
    GT_U32             erspanType,
    GT_BOOL            isFirstPass
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    GT_U32                              portNum;
    GT_HW_DEV_NUM                       hwDevNum;
    GT_PORT_NUM                         eport = PRV_TGF_DEFAULT_EPORT_CNS;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /*For egress mirroring, orig target port is used in HA/EPCL
     * Ingress mirror, the new target port is used*/
    portNum = (dir == CPSS_DIRECTION_EGRESS_E) ? prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS] : prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS] ;
    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        if((dir == CPSS_DIRECTION_INGRESS_E) && (isFirstPass == GT_TRUE))
        {
            portNum = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];
        }
        else if((dir == CPSS_DIRECTION_EGRESS_E) && (isFirstPass == GT_TRUE))
        {
            portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
        }
    }
    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            portNum,
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          portNum,
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(portNum,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfErspanSameDevRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(portNum,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
        {
            if(prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
                break;

            /* AUTODOC:   configure User Defined Byte(UDB) */
            /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
            /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
            /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
            /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
            /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
            /* AUTODOC:   offset 57 Analyzer ePort[13] */
            /* AUTODOC:   offset 64 egress Mtag cmd[1:0] egress marvell tagged[2] ingress marvell tagged[5] */
            rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_EGRESS_E,
                                            prvTgfErspanIIISameDevEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                            prvTgfErspanIIISameDevEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                            prvTgfErspanIIISameDevEpclMetadataUdbInfo[udbSelectidx].offset);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

            udbSelect.udbSelectArr[udbSelectidx]=prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].udbIndex;
            udbSelectidx++;
        }
        else if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
        {
            if(prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
                break;

            /* AUTODOC:   configure User Defined Byte(UDB) */
            /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
            /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
            /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
            /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
            /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
            /* AUTODOC:   offset 57 Analyzer ePort[13] */
            /* AUTODOC:   offset 64 egress Mtag cmd[1:0] */
            rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_EGRESS_E,
                                            prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                            prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                            prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].offset);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

            udbSelect.udbSelectArr[udbSelectidx]=prvTgfErspanSameDevEpclMetadataUdbInfo[udbSelectidx].udbIndex;
            udbSelectidx++;
        }
        else if(erspanType == PRV_TGF_ERSPAN_TYPE_I_CNS)
        {
            if(prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
                break;

            /* AUTODOC:   configure User Defined Byte(UDB) */
            /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
            /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
            /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
            /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
            /* AUTODOC:   offset 54 Analyzer Trg Dev[0:1] Analyzer ePort[12:7] */
            /* AUTODOC:   offset 55 Analyzer Trg Dev[9:2] */
            /* AUTODOC:   offset 57 Analyzer ePort[13] */
            /* AUTODOC:   offset 64 egress marvell tagged[2] */
            rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_EGRESS_E,
                                            prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                            prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                            prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].offset);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

            udbSelect.udbSelectArr[udbSelectidx]=prvTgfErspanEpclMetadataUdbInfo[udbSelectidx].udbIndex;
            udbSelectidx++;
        }
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /* bits 1:7 is ePort[0:6] */
        pattern.ruleEgrUdbOnly.udb[0] = (eport&0x7f) <<1;
        mask.ruleEgrUdbOnly.udb[0] = prvTgfErspanIIISameDevEpclMetadataUdbInfo[0].byteMask;

        /* bits 0:5 is ePort[12:7] */
        pattern.ruleEgrUdbOnly.udb[1] = ((eport>>7)&0x3f);
        mask.ruleEgrUdbOnly.udb[1] = prvTgfErspanIIISameDevEpclMetadataUdbInfo[1].byteMask;

        /* bits 5 is ePort[13] */
        pattern.ruleEgrUdbOnly.udb[2] = (eport>>8)&0x20;
        mask.ruleEgrUdbOnly.udb[2] = prvTgfErspanIIISameDevEpclMetadataUdbInfo[2].byteMask;

        /*ingress marvell tagged[5] egress mtag cmd[1:0] */
        pattern.ruleEgrUdbOnly.udb[3] = (isFirstPass << 5) | 0x02;
        mask.ruleEgrUdbOnly.udb[3] = prvTgfErspanIIISameDevEpclMetadataUdbInfo[3].byteMask;
    }
    else if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
    {
        /* bits 1:7 is ePort[0:6] */
        pattern.ruleEgrUdbOnly.udb[0] = (PRV_TGF_DEFAULT_EPORT_CNS&0x7f) <<1;
        mask.ruleEgrUdbOnly.udb[0] = prvTgfErspanSameDevEpclMetadataUdbInfo[0].byteMask;

        /* bits 0:5 is ePort[12:7] */
        pattern.ruleEgrUdbOnly.udb[1] = ((PRV_TGF_DEFAULT_EPORT_CNS>>7)&0x3f);
        mask.ruleEgrUdbOnly.udb[1] = prvTgfErspanSameDevEpclMetadataUdbInfo[1].byteMask;

        /* bits 5 is ePort[13] */
        pattern.ruleEgrUdbOnly.udb[2] = (PRV_TGF_DEFAULT_EPORT_CNS>>8)&0x20;
        mask.ruleEgrUdbOnly.udb[2] = prvTgfErspanSameDevEpclMetadataUdbInfo[2].byteMask;

        /*egress pkt cmd*/
        pattern.ruleEgrUdbOnly.udb[3] = 0x02;
        mask.ruleEgrUdbOnly.udb[3] = prvTgfErspanSameDevEpclMetadataUdbInfo[3].byteMask;
    }
    else if(erspanType == PRV_TGF_ERSPAN_TYPE_I_CNS)
    {
        /* get HW device number */
        rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &hwDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet: %d",prvTgfDevNum);

        /* bits 1:7 is ePort[0:6] */
        pattern.ruleEgrUdbOnly.udb[0] = (PRV_TGF_DEFAULT_EPORT_CNS&0x7f) <<1;
        mask.ruleEgrUdbOnly.udb[0] = prvTgfErspanEpclMetadataUdbInfo[0].byteMask;

        /* bits 0:5 is ePort[12:7], bit 6:7 is Analyzer Trg Dev[0:1] */
        pattern.ruleEgrUdbOnly.udb[1] = ((PRV_TGF_DEFAULT_EPORT_CNS>>7)&0x3f)  | ((hwDevNum&0x3) <<6);
        mask.ruleEgrUdbOnly.udb[1] = prvTgfErspanEpclMetadataUdbInfo[1].byteMask;

        /* bits 0:7 is Analyzer Trg Dev[2:9] */
        pattern.ruleEgrUdbOnly.udb[2] = (hwDevNum>>2)&0xff;
        mask.ruleEgrUdbOnly.udb[2] = prvTgfErspanEpclMetadataUdbInfo[2].byteMask;

        /* bits 5 is ePort[13] */
        pattern.ruleEgrUdbOnly.udb[3] = (PRV_TGF_DEFAULT_EPORT_CNS>>8)&0x20;
        mask.ruleEgrUdbOnly.udb[3] = prvTgfErspanEpclMetadataUdbInfo[3].byteMask;

        pattern.ruleEgrUdbOnly.udb[4] = 0x02; /* egress marvell tagged */
        mask.ruleEgrUdbOnly.udb[4] = prvTgfErspanEpclMetadataUdbInfo[4].byteMask;
    }

    ruleIndex                                    = isFirstPass;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_TRUE;
    action.oam.oamProfile                        = PRV_TGF_OAM_PROFILE_INDEX_CNS;
    action.flowId                                = PRV_TGF_EPCL_FLOW_ID_CNS;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        action.epclPhaInfo.phaThreadId               = (protocol == CPSS_IP_PROTOCOL_IPV4_E) ?
                                                        PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV4_CNS :
                                                        ((isFirstPass == GT_FALSE) ?
                                                        PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_FIRST_PASS_CNS :
                                                        PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_SECOND_PASS_CNS);

        if(action.epclPhaInfo.phaThreadId == PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_SECOND_PASS_CNS)
        {
            action.oam.oamProcessEnable                  = GT_FALSE;
            action.oam.oamProfile                        = 0;
        }
    }
    else if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
    {
        action.epclPhaInfo.phaThreadId               = (protocol == CPSS_IP_PROTOCOL_IPV4_E) ?
                                                        PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_II_SAME_DEV_MIRROR_IPV4_CNS :
                                                        PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_II_SAME_DEV_MIRROR_IPV6_CNS;
    }
    else if(erspanType == PRV_TGF_ERSPAN_TYPE_I_CNS)
    {
        action.epclPhaInfo.phaThreadId               = (protocol == CPSS_IP_PROTOCOL_IPV4_E) ?
                                                       PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_I_SAME_DEV_MIRROR_IPV4_CNS :
                                                       PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_I_SAME_DEV_MIRROR_IPV6_CNS;
    }
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex = analyzerIndex1;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}
/**
* @internal prvTgfFalconErspanSameDevMirrorEoamConfigSet function
* @endinternal
*
* @brief   EOAM config
*            <OAM Opcode Parsing En>=0
*            <LM Counter Capture En>=1
*            <LM Counting Mode>= Enable Counting
*            <OAM PTP Offset Index>=0
*
*/
static GT_VOID  prvTgfFalconErspanSameDevMirrorEoamConfigSet
(
    GT_U32 oamEntryIndex
)
{
    GT_STATUS                       rc = GT_OK;
    PRV_TGF_OAM_COMMON_CONFIG_STC   oamConfig;
    GT_U32           oamIndex = 1;

    PRV_UTF_LOG0_MAC("======= Setting EOAM Configuration =======\n");

    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    oamConfig.oamEntryIndex = oamEntryIndex;

    /* Get OAM common configuration and save it for restortion */
    prvTgfOamConfigurationGet(&oamConfig);
    cpssOsMemCpy(&(prvTgfErspanSameDevRestoreCfg.oamConfig), &oamConfig, sizeof(oamConfig));

    /* AUTODOC: Set common OAM configurations */
    oamConfig.oamEntry.oamPtpOffsetIndex      = 0;
    oamConfig.oamEntry.opcodeParsingEnable    = GT_FALSE;
    oamConfig.oamEntry.lmCountingMode         = PRV_TGF_OAM_LM_COUNTING_MODE_ENABLE_E;
    oamConfig.oamEntry.lmCounterCaptureEnable = GT_TRUE;
    prvTgfOamConfigurationSet(&oamConfig);

    /* AUTODOC: Set OAM LM offset table */
    rc = prvTgfOamLmOffsetTableSet(oamIndex, PRV_TGF_OAM_TLV_DATA_OFFSET_STC + 4*oamIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamLmOffsetTableSet");

    /* AUTODOC: save OAM Enabler status */
    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, &prvTgfErspanSameDevRestoreCfg.oamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableGet");

    /* AUTODOC: Enable OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}
/**
* @internal prvTgfFalconErspanSameDevMirrorEplrConfigSet function
* @endinternal
*
* @brief   EPLR test configurations
*           Global<Billing Index Mode> = FlowID
*            <LM Counter Capture Mode>=Insert
*            <Billing Color Counting Mode>=0x1 (ALL)
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorEplrConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_TGF_POLICER_BILLING_INDEX_CFG_STC   billingIndexCfg;
    PRV_TGF_POLICER_BILLING_ENTRY_STC       prvTgfBillingCntr;

    PRV_UTF_LOG0_MAC("======= Setting EPLR Configuration =======\n");

    /* Save stage mode */
    rc = prvTgfPolicerStageMeterModeGet(prvTgfDevNum, PRV_TGF_POLICER_STAGE_EGRESS_E, &prvTgfErspanSameDevRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeGet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);
    /* AUTODOC: test works in Flow mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, PRV_TGF_POLICER_STAGE_METER_MODE_FLOW_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 PRV_TGF_POLICER_STAGE_EGRESS_E);

    /* AUTODOC: Set Egress policer counter based on Flow Id */
    rc = cpssDxChPolicerFlowIdCountingCfgGet(prvTgfDevNum,
                                            CPSS_DXCH_POLICER_STAGE_EGRESS_E,
                                            &prvTgfErspanSameDevRestoreCfg.dxChBillingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerFlowIdCountingCfgGet");
    cpssOsMemSet(&billingIndexCfg, 0, sizeof(billingIndexCfg));
    billingIndexCfg.billingIndexMode       = PRV_TGF_POLICER_BILLING_INDEX_MODE_FLOW_ID_E;
    billingIndexCfg.billingFlowIdIndexBase = PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_BASE_CNS;
    billingIndexCfg.billingMinFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MIN_CNS;
    billingIndexCfg.billingMaxFlowId       = PRV_TGF_BILLING_COUNTERS_FLOW_ID_MAX_CNS;
    rc = prvTgfPolicerFlowIdCountingCfgSet(PRV_TGF_POLICER_STAGE_EGRESS_E, &billingIndexCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerFlowIdCountingCfgSet");

    /* AUTODOC: Flush internal Write Back Cache (WBC) of counting entries */
    rc = prvTgfPolicerCountingWriteBackCacheFlush(PRV_TGF_POLICER_STAGE_EGRESS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerCountingWriteBackCacheFlush");

    /* AUTODOC: Configure counters mode */
    cpssOsMemSet(&prvTgfBillingCntr, 0, sizeof(prvTgfBillingCntr));
    prvTgfBillingCntr.billingCntrMode = PRV_TGF_POLICER_BILLING_CNTR_PACKET_E;
    prvTgfBillingCntr.packetSizeMode  = CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E;
    prvTgfBillingCntr.lmCntrCaptureMode = PRV_TGF_POLICER_LM_COUNTER_CAPTURE_MODE_INSERT_E;
    prvTgfBillingCntr.billingCntrAllEnable = GT_TRUE;
    rc = prvTgfPolicerBillingEntrySet(prvTgfDevNum,
                                      PRV_TGF_POLICER_STAGE_EGRESS_E,
                                      PRV_TGF_BILLING_COUNTERS_FLOW_ID_INDEX_CNS,
                                      &prvTgfBillingCntr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerBillingEntrySet,prvTgfDevNum:%d", prvTgfDevNum);

    /* AUTODOC: Set LM counter capture enable */
    rc = cpssDxChPolicerLossMeasurementCounterCaptureEnableGet(prvTgfDevNum, CPSS_DXCH_POLICER_STAGE_EGRESS_E, &prvTgfErspanSameDevRestoreCfg.lmCntrEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerLossMeasurementCounterCaptureEnableGet");
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(PRV_TGF_POLICER_STAGE_EGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

    /* Get Egress to analyzer metering and counting status for restortion */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet(prvTgfDevNum,
                                        &prvTgfErspanSameDevRestoreCfg.meteringCountingStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableGet");

    /* AUTODOC: Enable Metering and counting for "TO_ANALYZER" packets */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");
}
/**
* @internal prvTgfFalconErspanSameDevMirrorErmrkConfigSet function
* @endinternal
*
* @brief   ERMRK test configurations
*           <LM Stamping En> is disabled, as the LM counter is inserted by the PHA
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorErmrkConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting ERMRK Configuration =======\n");

    /* AUTODOC: save current state of LM stamping counter for restortion */
    rc = cpssDxChOamLmStampingEnableGet(prvTgfDevNum, &prvTgfErspanSameDevRestoreCfg.lmStampingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChOamLmStampingEnableGet");

    /* AUTODOC: Disable Stamping of LM counter in packet (since fw does it instead) */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChOamLmStampingEnableSet");

}
/**
* @internal prvTgfFalconErspanSameDevMirrorPhaConfig function
* @endinternal
*
* @brief   PHA thread config
*           Set the FW thread id 64/65 with ERSPAN IPv4/v6 thread id
*           Set the FW thread id 81, 82, 83 with ERSPAN III IPv4/v6 thread id
*           Set the shared memory with tunnel start + ERSPAN data format
*
* @note     isFirstPass is applicable for ERSPAN III ipv6 packets only
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorPhaConfig(CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_U32 erspanType)
{
    GT_STATUS rc;

    /* AUTODOC: Set the thread entry */
    CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;

    /* load default FW image */
    prvTgfPhaFWLoad(CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E);

    extInfo.notNeeded = 0;
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
        {
            rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                             PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV4_CNS,
                                             &commonInfo,
                                             CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_III_SAME_DEV_IPV4_E,
                                             &extInfo);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
        }
        else if(protocol == CPSS_IP_PROTOCOL_IPV6_E)
        {
            rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                             PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_FIRST_PASS_CNS,
                                             &commonInfo,
                                             CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_III_SAME_DEV_IPV6_FIRST_PASS_E,
                                             &extInfo);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

            rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                             PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_III_SAME_DEV_MIRROR_IPV6_SECOND_PASS_CNS,
                                             &commonInfo,
                                             CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_III_SAME_DEV_IPV6_SECOND_PASS_E,
                                             &extInfo);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
        }
    }

    if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
    {
        rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                         protocol == CPSS_IP_PROTOCOL_IPV4_E ?
                                         PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_II_SAME_DEV_MIRROR_IPV4_CNS :
                                         PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_II_SAME_DEV_MIRROR_IPV6_CNS,
                                         &commonInfo,
                                         protocol == CPSS_IP_PROTOCOL_IPV4_E ?
                                         CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV4_E :
                                         CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_II_SAME_DEV_IPV6_E,
                                         &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
    }

    if(erspanType == PRV_TGF_ERSPAN_TYPE_I_CNS)
    {
        rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                         protocol == CPSS_IP_PROTOCOL_IPV4_E ?
                                         PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_I_SAME_DEV_MIRROR_IPV4_CNS :
                                         PRV_TGF_PHA_THREAD_ID_ERSPAN_TYPE_I_SAME_DEV_MIRROR_IPV6_CNS,
                                         &commonInfo,
                                         protocol == CPSS_IP_PROTOCOL_IPV4_E ?
                                         CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_I_SAME_DEV_IPV4_E :
                                         CPSS_DXCH_PHA_THREAD_TYPE_ERSPAN_TYPE_I_SAME_DEV_IPV6_E,
                                         &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
    }

    /* AUTODOC: Set the PHA ERSPAN entry in the shared memory */
    cpssOsMemSet(&erspanEntry, 0, sizeof(erspanEntry));

    cpssOsMemCpy(&erspanEntry.l2Info.macDa.arEther, &prvTgfMirroredPacketTunnelL2Part.daMac[0], sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(&erspanEntry.l2Info.macSa.arEther, &prvTgfMirroredPacketTunnelL2Part.saMac[0], sizeof(TGF_MAC_ADDR));

    erspanEntry.l2Info.tpid       = 0x8100;
    erspanEntry.l2Info.up         = 0;
    erspanEntry.l2Info.cfi        = 1;
    erspanEntry.l2Info.vid        = PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS;

    erspanEntry.protocol          = protocol;
    if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        erspanEntry.ipInfo.ipv4.dscp  = prvTgfMirroredPacketTunnelIpv4Part.typeOfService;
        erspanEntry.ipInfo.ipv4.flags = prvTgfMirroredPacketTunnelIpv4Part.flags;
        erspanEntry.ipInfo.ipv4.dscp  = prvTgfMirroredPacketTunnelIpv4Part.timeToLive;

        cpssOsMemCpy(&erspanEntry.ipInfo.ipv4.sipAddr.arIP[0], &prvTgfMirroredPacketTunnelIpv4Part.srcAddr[0], sizeof(TGF_IPV4_ADDR));
        cpssOsMemCpy(&erspanEntry.ipInfo.ipv4.dipAddr.arIP[0], &prvTgfMirroredPacketTunnelIpv4Part.dstAddr[0], sizeof(TGF_IPV4_ADDR));
    }
    else/* IPv6 */
    {
        erspanEntry.ipInfo.ipv6.tc            = prvTgfMirroredPacketTunnelIpv6Part.trafficClass;
        erspanEntry.ipInfo.ipv6.flowLabel     = prvTgfMirroredPacketTunnelIpv6Part.flowLabel;
        erspanEntry.ipInfo.ipv6.hopLimit      = prvTgfMirroredPacketTunnelIpv6Part.hopLimit;

        prvTgfIpv6Convert(&prvTgfMirroredPacketTunnelIpv6Part.srcAddr, &erspanEntry.ipInfo.ipv6.sipAddr);
        prvTgfIpv6Convert(&prvTgfMirroredPacketTunnelIpv6Part.dstAddr, &erspanEntry.ipInfo.ipv6.dipAddr);
    }

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        erspanEntry.hwId = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_HW_ID_CNS;
    }

    rc = cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet(prvTgfDevNum, analyzerIndex1, &erspanEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaSharedMemoryErspanIpTemplateEntrySet");

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        rc = cpssDxChPhaSharedMemoryErspanGlobalConfigSet(prvTgfDevNum,
            PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_SWITCH_ID_CNS, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaSharedMemoryErspanGlobalConfigSet");
    }
}

/**
* @internal prvTgfFalconErspanSameDevMirrorLoopbackPortConfig function
* @endinternal
*
* @brief   Loopback config
*
*/
static GT_VOID  prvTgfFalconErspanSameDevMirrorLoopbackPortConfig
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", PRV_TGF_VLANID_CNS);

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* AUTODOC: Save loopback port cascade port type */
    rc = prvTgfCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                               CPSS_PORT_DIRECTION_BOTH_E,
                               &prvTgfErspanSameDevRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeGet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /* AUTODOC: set loopback port cascade port type to cascade */
    rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_BOTH_E,
                               prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                               CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /*give port mgr time to config the LB*/
    cpssOsTimerWkAfter(10);
}

/**
* @internal prvTgfFalconErspanSameDevMirrorLoopbackPortConfigReset function
* @endinternal
*
* @brief   Loopback config reset
*
*/
static GT_VOID  prvTgfFalconErspanSameDevMirrorLoopbackPortConfigReset
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* AUTODOC: Restore loopback port cascad port type */
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                               CPSS_PORT_DIRECTION_BOTH_E,
                               prvTgfErspanSameDevRestoreCfg.cscdPortType);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeSet: %d %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* Restore loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /*give port mgr time to config the LB*/
    cpssOsTimerWkAfter(10);
}

/**
* @internal prvTgfFalconErspanSameDevMirrorConfigCommon function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*    Note:
*    isFirstPass is applicable for ERSPAN III ipv6 packets only
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorConfigCommon
(
    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    CPSS_DIRECTION_ENT         dir,
    GT_U32                     erspanType,
    GT_BOOL                    isFirstPass
)
{
    prvTgfFalconErspanSameDevMirrorBridgeConfigSet();
    /*mirror port config*/
    prvTgfFalconErspanSameDevMirrorConfig(erspanType, protocol);

    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        /* loopback port config*/
        prvTgfFalconErspanSameDevMirrorLoopbackPortConfig();

        /* AUTODOC: EPCL trigger for PHA */
        prvTgfFalconErspanSameDevMirrorEpclConfigSet(protocol, dir, erspanType, GT_FALSE);
    }

    /* AUTODOC: EPCL trigger for PHA */
    prvTgfFalconErspanSameDevMirrorEpclConfigSet(protocol, dir, erspanType, isFirstPass);

    /* EOAM Configuration */
    prvTgfFalconErspanSameDevMirrorEoamConfigSet(PRV_TGF_EPCL_FLOW_ID_CNS);

    /* EPLR Configuration */
    prvTgfFalconErspanSameDevMirrorEplrConfigSet();

    /* ERMRK Configuration */
    prvTgfFalconErspanSameDevMirrorErmrkConfigSet();

    /*PHA config*/
    prvTgfFalconErspanSameDevMirrorPhaConfig(protocol, erspanType);

}
/**
* @internal prvTgfFalconErspanSameDevMirrorConfigIngress function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    enable Rx mirroring for ingress port;
*    configure PHA source port lookup for ingress port;
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorConfigIngress()
{
    GT_STATUS rc;

    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT portInfo;

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    rc= prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.rxMirrorEnable, &prvTgfErspanSameDevRestoreCfg.rxMirrorIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorRxPortGet");
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE, analyzerIndex1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorRxPortSet");

    rc = cpssDxChPhaSourcePortEntryGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.srcPortType, &prvTgfErspanSameDevRestoreCfg.srcPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntryGet");
    portInfo.erspanSameDevMirror.erspanIndex = PRV_TGF_ERSPAN_SAME_DEV_INDEX;
    rc = cpssDxChPhaSourcePortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E, &portInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");
}
/**
* @internal prvTgfFalconErspanSameDevMirrorConfigEgress function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    enable Rx mirroring for egress port;
*    configure PHA target port lookup for egress port;
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorConfigEgress()
{
    GT_STATUS rc;

    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT portInfo;

    /* AUTODOC: enable Tx mirroring on port=1, index=0 */
    rc = prvTgfMirrorTxPortGet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.txMirrorEnable, &prvTgfErspanSameDevRestoreCfg.txMirrorIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortGet");
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], GT_TRUE, analyzerIndex1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortSet");

    rc = cpssDxChPhaTargetPortEntryGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.trgPortType, &prvTgfErspanSameDevRestoreCfg.trgPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntryGet");
    portInfo.erspanSameDevMirror.erspanIndex = PRV_TGF_ERSPAN_SAME_DEV_INDEX;
    rc = cpssDxChPhaTargetPortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E, &portInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");
}
/**
* @internal prvTgfFalconErspanSameDevMirrorIpv4ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
void    prvTgfFalconErspanSameDevMirrorIpv4ConfigurationSet
(
    CPSS_DIRECTION_ENT dir,
    GT_U32             erspanType,
    GT_BOOL            isFirstPass
)
{
    prvTgfFalconErspanSameDevMirrorConfigCommon(CPSS_IP_PROTOCOL_IPV4_E, dir, erspanType, isFirstPass);
    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        prvTgfFalconErspanSameDevMirrorConfigEgress();
    }
    else
    {
        prvTgfFalconErspanSameDevMirrorConfigIngress();
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorTrafficVerify function
* @endinternal
*
* @brief  verify the outer IP+GRE+ERSPAN encapsulation fields
*    Note: isCC and isTagged is relevant for ERSPAN III CC use case only
*/
static GT_VOID  prvTgfFalconErspanSameDevMirrorTrafficVerify
(
    GT_BOOL                    isOrigDevMirror,
    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    GT_U8*                     packetBuf,
    GT_U8                      pktCnt,
    GT_U32                     erspanType,
    CPSS_DIRECTION_ENT         dir,
    GT_BOOL                    isCC,
    GT_BOOL                    isTagged
)
{
    GT_U32                     greProtocolType, greFlags, erspanHighPart, erspanLowPart, erspanExpectedVal, erspanReceivedVal;
    GT_U16                     greFlagsExpectedVal, erspanExpectedSessId, erspanReceivedSessId;
    GT_U8                      erspanExpectedCos, erspanReceivedCos, erspanExpectedEn, erspanReceivedEn;
    GT_U8                      erspanExpectedBso, erspanReceivedBso, erspanExpectedT, erspanReceivedT;
    GT_U8                      pktOffset;

    if(isOrigDevMirror == GT_TRUE)
    {
        /*********************** Check ERSPAN header high part ************************************/
        /* Increment offset by 4 bytes to point to ERSPAN header  */
        pktOffset = TGF_L2_HEADER_SIZE_CNS + TGF_eDSA_TAG_SIZE_CNS;

        /* AUTODOC: Get the first 2B of ERSPAN header to verify ERSPAN Version and rest of bits are zero */
        erspanHighPart = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));

        /* AUTODOC: set expected value. All bits should be zero
         * except ERSPAN<Ver> = 2 for ERSPAN Type III */
        erspanExpectedVal = 0x2000;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanHighPart);

        /* AUTODOC: Get the next 2B of ERSPAN header to verify ERSPAN COS, En, T, Session ID */
        erspanHighPart = (GT_U32)((packetBuf[pktOffset + 2] << 8) | (packetBuf[pktOffset + 3]));

        /* AUTODOC: set expected ERSPAN COS value */
        erspanExpectedCos = 1;

        /* AUTODOC: Get the ERSPAN COS */
        erspanReceivedCos = (erspanHighPart >> 13) & 0x7;

        /* AUTODOC: compare received ERSPAN header COS vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedCos, erspanReceivedCos,
                                    "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedCos, erspanReceivedCos);

        /* AUTODOC: set expected ERSPAN BSO value */
        erspanExpectedBso = 0x0;

        /* AUTODOC: Get the ERSPAN BSO */
        erspanReceivedBso = (erspanHighPart >> 11) & 0x3;

        /* AUTODOC: compare received ERSPAN header BSO vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedBso, erspanReceivedBso,
                                    "ERSPAN BSO is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedBso, erspanReceivedBso);

        /* AUTODOC: set expected ERSPAN T value */
        erspanExpectedT = 0x0;

        /* AUTODOC: Get the ERSPAN T */
        erspanReceivedT = (erspanHighPart >> 10) & 0x1;

        /* AUTODOC: compare received ERSPAN header En vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedT, erspanReceivedT,
                                    "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedT, erspanReceivedT);

        /* AUTODOC: set expected ERSPAN session ID value */
        erspanExpectedSessId = 0x0;

        /* AUTODOC: Get the ERSPAN session ID */
        erspanReceivedSessId = erspanHighPart & 0x3FF;

        /* AUTODOC: compare received ERSPAN session ID vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedSessId, erspanReceivedSessId,
                                    "ERSPAN Session ID is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedSessId, erspanReceivedSessId);

        /* Increment offset by 4 bytes to point to ERSPAN header mid part  */
        pktOffset +=4;
        /*********************** Check ERSPAN header low part ************************************/
        /* Increment offset by 4 bytes to point to ERSPAN header low part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

        /* AUTODOC: set expected value for SGT . All bits should be zero */
        erspanExpectedVal = 0x0000;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 16) & 0xFFFF,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 16) & 0xFFFF);


        /* AUTODOC: set expected value for P.*/
        erspanExpectedVal =  0x1;    /* ethernet protocol frame */

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 15) & 0x1,
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 15) & 0x1);

        /* AUTODOC: set expected value for FT. All bits should be zero */
        erspanExpectedVal = 0x0;    /* Ethernet (802.3 frame) */

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 10) & 0x1F,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 10) & 0x1F);


        /* AUTODOC: set expected value for HW ID. */
        erspanExpectedVal = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_HW_ID_CNS;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 4) & 0x3F,
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 4) & 0x3F);

        /* AUTODOC: set expected value for D. Ingress (0) or Egress (1) */
        erspanExpectedVal = (dir == CPSS_DIRECTION_INGRESS_E) ? 0 : 1;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 3) & 0x1,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 3) & 0x1);

        /* AUTODOC: set expected value for Gra. */
        erspanExpectedVal = 0x2;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 1) & 0x3,
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 1) & 0x3);

        /* AUTODOC: set expected value for O. */
        erspanExpectedVal = 0x1;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanLowPart & 0x1,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanLowPart & 0x1);

        /*********************** Check ERSPAN Platform Specific Sub-header high part ************************/
        /* Increment offset by 4 bytes to point to ERSPAN sub-header high part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanHighPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

        /* AUTODOC: set expected value for platform ID. */
        erspanExpectedVal = 0x5;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanHighPart >> 26) & 0x3F,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanHighPart >> 26) & 0x3F);

        /* AUTODOC: set expected value for switch ID. */
        erspanExpectedVal = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_SWITCH_ID_CNS;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanHighPart >> 16) & 0x3FF,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanHighPart >> 16) & 0x3FF);

        /* AUTODOC: TODO: set expected value for index.*/
        erspanExpectedVal = PRV_TGF_ERSPAN_SAME_DEV_INDEX;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart & 0xFFFF,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanHighPart & 0xFFFF);

        /*********************** Check ERSPAN Platform Specific Sub-header low part  ************************/
        /* Increment offset by 4 bytes to point to ERSPAN sub-header low part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
        return;
    }
    pktOffset = TGF_L2_HEADER_SIZE_CNS;
    /*No vlan tag if vid = 0*/
    if (PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS != 0)
    {
        erspanExpectedVal = 0x8100;
        erspanReceivedVal = packetBuf[pktOffset] << 8 | packetBuf[pktOffset+1];
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                "outer ether type is not as expected => Expected:0x%x != Received:0x%x \n",
                erspanExpectedVal, erspanReceivedVal);

        erspanExpectedVal = PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS;
        erspanReceivedVal = packetBuf[pktOffset+TGF_ETHERTYPE_SIZE_CNS] << 8 | packetBuf[pktOffset+TGF_ETHERTYPE_SIZE_CNS+1];
        erspanReceivedVal &= 0xFFF;
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                "outer vlan id is not as expected => Expected:0x%x != Received:0x%x \n",
                erspanExpectedVal, erspanReceivedVal);

        pktOffset += TGF_VLAN_TAG_SIZE_CNS ;
    }
    pktOffset += TGF_ETHERTYPE_SIZE_CNS ;

    /* AUTODOC: set expected value for IP total length */
    erspanExpectedVal = (isCC == GT_TRUE) ? PRV_TGF_ERSPAN_III_CSCD_TAG_PACKET_SIZE_CNS : PRV_TGF_PACKET_TOTAL_LEN_CNS;
    erspanExpectedVal += (isTagged == GT_TRUE) ? TGF_VLAN_TAG_SIZE_CNS : 0;

    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        erspanExpectedVal += (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS) ? 0x24 : 0x2C;
        erspanExpectedVal -= (isCC == GT_TRUE) ? PRV_TGF_ERSPAN_III_HEADER_SIZE_CNS : 0;
        erspanExpectedVal -= ((isCC == GT_TRUE) ? (TGF_CRC_LEN_CNS): 0);
        /* AUTODOC: get received value for IPv4 total length */
        erspanReceivedVal = ((packetBuf[pktOffset + 2] << 8) | packetBuf[pktOffset + 3]);

        /* AUTODOC: compare received total length vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                                    "Total length is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanReceivedVal);

        pktOffset += TGF_IPV4_HEADER_SIZE_CNS;
    }
    else
    {
        if(isCC == GT_TRUE)
        {
            erspanExpectedVal += TGF_eDSA_TAG_SIZE_CNS;
            erspanExpectedVal -= (isCC == GT_TRUE) ? PRV_TGF_ERSPAN_III_HEADER_SIZE_CNS : 0;
        }
        else
        {
            erspanExpectedVal += PRV_TGF_GRE_HEADER_SIZE_CNS;
            erspanExpectedVal += (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS) ? PRV_TGF_ERSPAN_II_HEADER_SIZE_CNS : PRV_TGF_ERSPAN_III_HEADER_SIZE_CNS;
#ifndef ASIC_SIMULATION
            erspanExpectedVal -= (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS) ? 0 : TGF_VLAN_TAG_SIZE_CNS;    /* Dummy vlan */
#endif
        }
        erspanExpectedVal += (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS) ? 0x10 : TGF_VLAN_TAG_SIZE_CNS;    /* Dummy vlan */
        /* AUTODOC: get received value for IPv6 total length */
        if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
        {
            erspanExpectedVal -= TGF_CRC_LEN_CNS;
        }
        erspanReceivedVal = ((packetBuf[pktOffset + 4] << 8) | packetBuf[pktOffset + 5]);

        /* AUTODOC: compare received total length vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                                    "Total length is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanReceivedVal);

        pktOffset += TGF_IPV6_HEADER_SIZE_CNS;
    }

    /* AUTODOC: set expected value with GRE protocol type of ERSPAN II 0x88BE or ERSPAN III 0x22EB */
    erspanExpectedVal = (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS) ?
                        PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS : PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_III_CNS;

    /* AUTODOC: set expected value with GRE flags for ERSPAN II and ERSPAN III as defined in standard */
    greFlagsExpectedVal = PRV_TGF_GRE_FLAGS_ERSPAN_II_CNS;

    /* AUTODOC: get the first 2B of GRE header to verify GRE flags */
    greFlags = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));

    /* AUTODOC: compare received GRE flags vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(greFlagsExpectedVal, greFlags,
                                "GRE flags is not as expected => Expected:0x%x != Received:0x%x \n",
                                 greFlagsExpectedVal, greFlags);

    /* AUTODOC: get the next 2B of GRE header to verify GRE protocol */
    greProtocolType = (GT_U32)((packetBuf[pktOffset + 2] << 8) | packetBuf[pktOffset + 3]);

    /* AUTODOC: compare received GRE protocol type vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, greProtocolType,
                                "GRE protocol type is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, greProtocolType);


    /*********************** Get GRE Seq. number *********************************************/
    /* Increment offset by 4 bytes to point to GRE header low part  */
    pktOffset +=4;

    /* AUTODOC: Get the next 4B of GRE header to verify GRE Seq.number. Will be verified in next packet */
    lmCounter[0] = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                        (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

    /* AUTODOC: compare received GRE sequence number vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(((dir == CPSS_DIRECTION_EGRESS_E) || (isCC == GT_TRUE)) ? pktCnt : 1, lmCounter[0],
                                "GRE sequence number is not as expected => Expected:0x%x != Received:0x%x \n",
                                 (dir == CPSS_DIRECTION_EGRESS_E) ? pktCnt : 1, lmCounter[0]);


    /*********************** Check ERSPAN header high part ************************************/
    /* Increment offset by 4 bytes to point to ERSPAN header  */
    pktOffset +=4;

    /* AUTODOC: Get the first 2B of ERSPAN header to verify ERSPAN Version and rest of bits are zero */
    erspanHighPart = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));

    /* AUTODOC: set expected value. All bits should be zero
     * except for ERSPAN<Ver> = 1 for ERSPAN Type II or ERSPAN<Ver> = 2 for ERSPAN Type III */
    erspanExpectedVal = (erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS) ? 0x1000 : 0x2000;

    /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart,
                                "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, erspanHighPart);

    /* AUTODOC: Get the next 2B of ERSPAN header to verify ERSPAN COS, En, T, Session ID */
    erspanHighPart = (GT_U32)((packetBuf[pktOffset + 2] << 8) | (packetBuf[pktOffset + 3]));

    /* AUTODOC: set expected ERSPAN COS value */
    erspanExpectedCos = prvTgfDsaTag.commonParams.vpt;

    /* AUTODOC: Get the ERSPAN COS */
    erspanReceivedCos = (erspanHighPart >> 13) & 0x7;

    /* AUTODOC: compare received ERSPAN header COS vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedCos, erspanReceivedCos,
                                "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedCos, erspanReceivedCos);

    if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
    {
        /* AUTODOC: set expected ERSPAN En value */
        erspanExpectedEn = 0x3;

        /* AUTODOC: Get the ERSPAN En */
        erspanReceivedEn = (erspanHighPart >> 11) & 0x3;

        /* AUTODOC: compare received ERSPAN header En vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedEn, erspanReceivedEn,
                                    "ERSPAN En is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedEn, erspanReceivedEn);
    }

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /* AUTODOC: set expected ERSPAN BSO value */
        erspanExpectedBso = 0x0;

        /* AUTODOC: Get the ERSPAN BSO */
        erspanReceivedBso = (erspanHighPart >> 11) & 0x3;

        /* AUTODOC: compare received ERSPAN header BSO vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedBso, erspanReceivedBso,
                                    "ERSPAN BSO is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedBso, erspanReceivedBso);
    }

    /* AUTODOC: set expected ERSPAN T value */
    erspanExpectedT = 0x0;

    /* AUTODOC: Get the ERSPAN T */
    erspanReceivedT = (erspanHighPart >> 10) & 0x1;

    /* AUTODOC: compare received ERSPAN header En vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedT, erspanReceivedT,
                                "ERSPAN COS is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedT, erspanReceivedT);

    /* AUTODOC: set expected ERSPAN session ID value */
    erspanExpectedSessId = 0x0;

    /* AUTODOC: Get the ERSPAN session ID */
    erspanReceivedSessId = erspanHighPart & 0x3FF;

    /* AUTODOC: compare received ERSPAN session ID vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedSessId, erspanReceivedSessId,
                                "ERSPAN Session ID is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedSessId, erspanReceivedSessId);

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /* Increment offset by 4 bytes to point to ERSPAN header mid part  */
        pktOffset +=4;
    }
    /*********************** Check ERSPAN header low part ************************************/

    if(erspanType == PRV_TGF_ERSPAN_TYPE_II_CNS)
    {
        /* Increment offset by 4 bytes to point to ERSPAN header low part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

        /* AUTODOC: set expected value for reserved . All bits should be zero */
        erspanExpectedVal = 0x000;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 20) & 0xFFF,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 20) & 0xFFF);

        /* AUTODOC: set expected value for index.*/
        erspanExpectedVal =  PRV_TGF_ERSPAN_SAME_DEV_INDEX; /*direction == 0  ? prvTgfPortsArray[] :;*/

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart & 0xFFFF),
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart & 0xFFFF));
    }

    if(erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /* Increment offset by 4 bytes to point to ERSPAN header low part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

        /* AUTODOC: set expected value for SGT . All bits should be zero */
        erspanExpectedVal = 0x0000;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 16) & 0xFFFF,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 16) & 0xFFFF);


        /* AUTODOC: set expected value for P.*/
        erspanExpectedVal =  0x1;    /* ethernet protocol frame */

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 15) & 0x1,
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 15) & 0x1);

        /* AUTODOC: set expected value for FT. All bits should be zero */
        erspanExpectedVal = 0x0;    /* Ethernet (802.3 frame) */

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 10) & 0x1F,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 10) & 0x1F);


        /* AUTODOC: set expected value for HW ID. */
        erspanExpectedVal = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_HW_ID_CNS;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 4) & 0x3F,
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 4) & 0x3F);

        /* AUTODOC: set expected value for D. Ingress (0) or Egress (1) */
        erspanExpectedVal = (dir == CPSS_DIRECTION_INGRESS_E) ? 0 : 1;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 3) & 0x1,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 3) & 0x1);

        /* AUTODOC: set expected value for Gra. */
        erspanExpectedVal = 0x2;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanLowPart >> 1) & 0x3,
                                    "ERSPAN index is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanLowPart >> 1) & 0x3);

        /* AUTODOC: set expected value for O. */
        erspanExpectedVal = 0x1;

        /* AUTODOC: compare received ERSPAN header low part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanLowPart & 0x1,
                                    "ERSPAN low part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanLowPart & 0x1);

        /*********************** Check ERSPAN Platform Specific Sub-header high part ************************/
        /* Increment offset by 4 bytes to point to ERSPAN sub-header high part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanHighPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));

        /* AUTODOC: set expected value for platform ID. */
        erspanExpectedVal = 0x5;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanHighPart >> 26) & 0x3F,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanHighPart >> 26) & 0x3F);

        /* AUTODOC: set expected value for switch ID. */
        erspanExpectedVal = PRV_TGF_ERSPAN_III_SAME_DEV_MIRROR_SWITCH_ID_CNS;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, (erspanHighPart >> 16) & 0x3FF,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, (erspanHighPart >> 16) & 0x3FF);

        /* AUTODOC: TODO: set expected value for index.*/
        erspanExpectedVal = PRV_TGF_ERSPAN_SAME_DEV_INDEX;

        /* AUTODOC: compare received ERSPAN header high part vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanHighPart & 0xFFFF,
                                    "ERSPAN high part is not as expected => Expected:0x%x != Received:0x%x \n",
                                     erspanExpectedVal, erspanHighPart & 0xFFFF);

        /*********************** Check ERSPAN Platform Specific Sub-header low part  ************************/
        /* Increment offset by 4 bytes to point to ERSPAN sub-header low part  */
        pktOffset +=4;

        /* AUTODOC: Get the next 4B of ERSPAN header to verify all bits */
        erspanLowPart = (GT_U32)(packetBuf[pktOffset + 3] | (packetBuf[pktOffset + 2] << 8) |
                               (packetBuf[pktOffset + 1] << 16) | (packetBuf[pktOffset] << 24));
    }

    if(isCC != GT_TRUE)
    {
        /*verfiy start of passenger*/
        pktOffset +=4;
        erspanExpectedVal = 0;
        erspanReceivedVal = cpssOsMemCmp(prvTgfOriginalPacketL2Part.daMac, &packetBuf[pktOffset], 6);
        /* AUTODOC: compare received passenger L2 da mac vs expected and notify if there is any mismatch */
        UTF_VERIFY_EQUAL0_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                                    "passenger L2 da mac is not as expected \n");
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorTraffic function
* @endinternal
*
* @brief
*           Test the captures for the configs same device mirroring ERSPAN type II.
*           RX:
*               L2 pkt for Vlan5
*           TX:
*               2 pkts on 2 Egr analyzer ports.
*               1. IPv4 encapsulated
*               2. IPv6 encapsulated
*/
GT_VOID prvTgfFalconErspanSameDevMirrorTraffic
(
    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    GT_U32                     erspanType,
    CPSS_DIRECTION_ENT         dir
)
{
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U32                numOfPacketsToAnalyze = 0;
    GT_U32                portIter = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL               getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS             rc = GT_OK;
    GT_U8                 pktCount=PRV_TGF_SAMPLE_MIRROR_RATE_CNS;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOriginalPacketInfo, pktCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* AUTODOC: Get the rx pkts on egress port */
    numOfPacketsToAnalyze = (erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) ?
                            pktCount : pktCount/PRV_TGF_SAMPLE_MIRROR_RATE_CNS;
    pktCount = 0;

    /* provide time for TO_CPU packet RX process */
    cpssOsTimerWkAfter(10);

    while(pktCount != numOfPacketsToAnalyze)
    {
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        if (GT_OK != rc && GT_NO_MORE != rc)
        {
            PRV_UTF_LOG1_MAC("Error: failed capture packet rc=%d\n",rc);
            return;
        }

        getFirst = GT_FALSE;

        /*verify the packet*/
        pktCount++;
        prvTgfFalconErspanSameDevMirrorTrafficVerify(GT_FALSE, protocol, packetBuf, pktCount, erspanType, dir, GT_FALSE, GT_FALSE);
    }

    /* Disable capture on analyzer port 1*/
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* -------------------------------------------------------------------------
     * Dump Ethernet Counters
     */

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorBridgeConfigReset function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorBridgeConfigReset()
{
    GT_STATUS rc;
    prvTgfBrgVlanEntryRestore(1);
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);
    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfErspanSameDevRestoreCfg.pvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d", prvTgfErspanSameDevRestoreCfg.pvid);
}
/**
* @internal prvTgfFalconErspanSameDevMirrorConfigReset function
* @endinternal
*
* @brief   Ingress mirror config restore
*           Disable mirroring for rx port
*           Restore eport config, and e2phy map
*
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorConfigReset
(
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{

    GT_STATUS rc;
    GT_U32    i;

    /* AUTODOC: Restore analyzer interface from index 1 */
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex1, &prvTgfErspanSameDevRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex1);

    /*Restore sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, analyzerIndex1, prvTgfErspanSameDevRestoreCfg.samplingEn, prvTgfErspanSameDevRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

    /* AUTODOC: Restore analyzer ePort#1 attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfErspanSameDevRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        /* AUTODOC: Restore source port loopback profile */
        rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                            CPSS_DIRECTION_INGRESS_E, prvTgfErspanSameDevRestoreCfg.srcProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

        /* AUTODOC: Restore analyzer port loopback profile */
        rc = cpssDxChPortLoopbackProfileSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                            CPSS_DIRECTION_EGRESS_E, prvTgfErspanSameDevRestoreCfg.trgProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackProfileSet");

        /* AUTODOC: Restore TO_ANALYZER loopback config */
        rc = cpssDxChPortLoopbackPktTypeToAnalyzerSet(prvTgfDevNum, analyzerIndex1,
                                                 prvTgfErspanSameDevRestoreCfg.loopbackPortNum,
                                                 prvTgfErspanSameDevRestoreCfg.loopbackEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackPktTypeToAnalyzerSet");

        for (i = 0; i < CPSS_TC_RANGE_CNS; i++)
        {
            /* AUTODOC: Restore general FTL trigger status */
            rc = cpssDxChPortLoopbackEnableSet(prvTgfDevNum,
                                               PRV_TGF_LB_PROFILE_1_CNS,
                                               PRV_TGF_LB_PROFILE_1_CNS,
                                               i,
                                               prvTgfErspanSameDevRestoreCfg.lbEnArr[i]);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPortLoopbackEnableSet, i:%d", i);
        }
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorEpclConfigReset function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorEpclConfigReset
(
    GT_U32             protocol,
    CPSS_DIRECTION_ENT dir,
    GT_U32             erspanType
)
{
    GT_STATUS   rc;
    GT_U32      portNum;

    portNum = (dir == CPSS_DIRECTION_EGRESS_E) ?prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS] :prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];

    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        /* AUTODOC: Invalidate PCL rule 1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 1, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);
    }

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          portNum,
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        if(dir == CPSS_DIRECTION_INGRESS_E)
        {
            portNum = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];
        }
        else if(dir == CPSS_DIRECTION_EGRESS_E)
        {
            portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
        }
        /* AUTODOC: disable EPCL on Analyzer port per packet type */
        rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                              portNum,
                                              PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorEoamConfigReset function
* @endinternal
*
* @brief   EOAM test configurations restore
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorEoamConfigReset()
{
    GT_STATUS                           rc = GT_OK;
    /* -------------------------------------------------------------------------
     * Restore EOAM Configuration
     */
    /* AUTODOC: restore common EOAM configurations */
    prvTgfOamConfigurationSet(&prvTgfErspanSameDevRestoreCfg.oamConfig);

    /* AUTODOC: restore OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, prvTgfErspanSameDevRestoreCfg.oamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}

/**
* @internal prvTgfFalconErspanSameDevMirrorEplrConfigReset function
* @endinternal
*
* @brief   Egress policer test configurations restore
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorEplrConfigReset()
{
    GT_STATUS                           rc = GT_OK;

    /* -------------------------------------------------------------------------
     * Restore EPLR Configuration
     */
    /* AUTODOC: Restore Policer Egress to Analyzer metering and counting status */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum,
                                                prvTgfErspanSameDevRestoreCfg.meteringCountingStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(rc,GT_OK, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");

    /* AUTODOC: Restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, prvTgfErspanSameDevRestoreCfg.stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 prvTgfErspanSameDevRestoreCfg.stageModeSave);
    rc = prvTgfPolicerLossMeasurementCounterCaptureEnableSet(PRV_TGF_POLICER_STAGE_EGRESS_E, prvTgfErspanSameDevRestoreCfg.lmCntrEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPolicerLossMeasurementCounterCaptureEnableSet");

}
/**
* @internal prvTgfFalconErspanSameDevMirrorErmrkConfigReset function
* @endinternal
*
* @brief   ERMRK test configurations restore
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorErmrkConfigReset()
{
    GT_STATUS                           rc = GT_OK;
    /* -------------------------------------------------------------------------
     * Restore ERMRK Configuration
     */
    /* AUTODOC: Restore LM Counter status */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, prvTgfErspanSameDevRestoreCfg.lmStampingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_OK, "Failure in cpssDxChOamLmStampingEnableSet");
}
/**
* @internal prvTgfFalconErspanSameDevMirrorConfigurationReset function
* @endinternal
*
* @brief   ERSPAN same dev mirroring test configurations restore
*/
GT_VOID prvTgfFalconErspanSameDevMirrorConfigurationReset
(
    CPSS_DIRECTION_ENT            dir,
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{
    GT_STATUS rc;

    prvTgfFalconErspanSameDevMirrorBridgeConfigReset();
    prvTgfFalconErspanSameDevMirrorConfigReset(erspanType, protocol);

    if((erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) && (protocol == CPSS_IP_PROTOCOL_IPV6_E))
    {
        /* Restore loopback port config */
        prvTgfFalconErspanSameDevMirrorLoopbackPortConfigReset();
    }
    if (dir == CPSS_DIRECTION_EGRESS_E)
    {
        rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                prvTgfErspanSameDevRestoreCfg.txMirrorEnable,
                                prvTgfErspanSameDevRestoreCfg.txMirrorIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortSet");
        rc = cpssDxChPhaTargetPortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                            prvTgfErspanSameDevRestoreCfg.trgPortType,
                                            &prvTgfErspanSameDevRestoreCfg.trgPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaTargetPortEntrySet");
    }
    else
    {
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                prvTgfErspanSameDevRestoreCfg.rxMirrorEnable,
                                prvTgfErspanSameDevRestoreCfg.rxMirrorIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortSet");
        rc = cpssDxChPhaSourcePortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                            prvTgfErspanSameDevRestoreCfg.srcPortType,
                                            &prvTgfErspanSameDevRestoreCfg.srcPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");
    }
    prvTgfFalconErspanSameDevMirrorEpclConfigReset(protocol, dir, erspanType);
    prvTgfFalconErspanSameDevMirrorEoamConfigReset();
    prvTgfFalconErspanSameDevMirrorEplrConfigReset();
    prvTgfFalconErspanSameDevMirrorErmrkConfigReset();
    /*restore PHA FW image */
    prvTgfPhaFWRestore();
    /*restore all buffers and reset mac counters*/
    tgfTrafficTableRxPcktTblClear();
    prvTgfEthCountersReset(prvTgfDevNum);
}

/*ERSPAN same dev mirroring IPV6 encapsulation*/

/**
* @internal prvTgfFalconErspanSameDevMirrorIpv6ConfigExtra function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*          Push 8B+4B tags for the mirrored packet
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorIpv6ConfigExtra
(
    GT_U32    erspanType
)
{
    GT_STATUS rc;

    /*push dummy vlan + RSPAN tag for IPv6*/
    GT_U32 tpidIndex = 3;

    /*config to push extra vlan on the mirror packet*/
    rc = cpssDxChBrgVlanPortPushVlanCommandGet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, &prvTgfErspanSameDevRestoreCfg.vlanCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChBrgVlanPortPushVlanCommandGet");
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, CPSS_DXCH_BRG_VLAN_PUSH_VLAN_COMMAND_PUSH_SINGLE_TAG_ON_TUNNEL_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChBrgVlanPortPushVlanCommandSet");

#ifndef ASIC_SIMULATION
    if(erspanType != PRV_TGF_ERSPAN_TYPE_III_CNS)
#endif
    {
        /*enable to push extra vlan on analyser port*/
        rc = cpssDxChMirrorAnalyzerVlanTagEnableGet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, &prvTgfErspanSameDevRestoreCfg.vlanTagEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorAnalyzerVlanTagEnableGet");
        rc = cpssDxChMirrorAnalyzerVlanTagEnable(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, GT_TRUE );
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorAnalyzerVlanTagEnable");
    }

    /*config the tpid 1 to be pushed*/
    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                                         &prvTgfErspanSameDevRestoreCfg.pushTpIdIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressGet ind %d",tpidIndex +1);
    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, tpidIndex+1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet ind %d",tpidIndex );

    /*config 4B at TPID 0 */
    rc = cpssDxChBrgVlanTpidTagTypeGet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
            tpidIndex, &prvTgfErspanSameDevRestoreCfg.type[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");
    rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
            tpidIndex, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");

    if(erspanType != PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /*config 8B at TPID 1 */
        rc = cpssDxChBrgVlanTpidTagTypeGet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                tpidIndex+1, &prvTgfErspanSameDevRestoreCfg.type[1]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");
        rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                tpidIndex+1, CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");
    }

    /*push 4B using TPID 0*/
    rc = cpssDxChBrgVlanPortEgressTpidGet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,
                                            &prvTgfErspanSameDevRestoreCfg.tpIdIndex[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortEgressTpidGet type %d ind %d",
                                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,tpidIndex );
    rc = cpssDxChBrgVlanPortEgressTpidSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, tpidIndex);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortEgressTpidSet type %d ind %d",
                                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,tpidIndex );

    if(erspanType != PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /*push 8B using TPID 1*/
        rc = cpssDxChBrgVlanPortEgressTpidGet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E,
                                                &prvTgfErspanSameDevRestoreCfg.tpIdIndex[1]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortEgressTpidGet type %d ind %d",
                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E,tpidIndex+1 );
        rc = cpssDxChBrgVlanPortEgressTpidSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, tpidIndex+1);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortEgressTpidSet type %d ind %d",
                                  CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E,tpidIndex+1 );
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorIpv6ConfigExtraReset function
* @endinternal
*
* @brief remove 8B+4B tags for the mirrored packet
*/
static GT_VOID prvTgfFalconErspanSameDevMirrorIpv6ConfigExtraReset
(
    GT_U32    erspanType
)
{
    GT_STATUS rc;

    /*push dummy vlan + RSPAN tag for IPv6*/
    GT_U32 tpidIndex = 3;

    /*config to push extra vlan on the mirror packet*/
    rc = cpssDxChBrgVlanPortPushVlanCommandSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, prvTgfErspanSameDevRestoreCfg.vlanCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChBrgVlanPortPushVlanCommandSet");

    /*enable to push extra vlan on analyser port*/
#ifndef ASIC_SIMULATION
    if(erspanType != PRV_TGF_ERSPAN_TYPE_III_CNS)
#endif
    {
        rc = cpssDxChMirrorAnalyzerVlanTagEnable(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS, prvTgfErspanSameDevRestoreCfg.vlanTagEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorAnalyzerVlanTagEnable");
    }

    /*config the tpid 1 to be pushed*/
    rc = cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                                         prvTgfErspanSameDevRestoreCfg.pushTpIdIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortPushedTagTpidSelectEgressSet ind %d",tpidIndex +1);

    /*config 4B at TPID 0 */
    rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
            tpidIndex, prvTgfErspanSameDevRestoreCfg.type[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");

    if(erspanType != PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /*config 8B at TPID 1 */
        rc = cpssDxChBrgVlanTpidTagTypeSet(prvTgfDevNum, CPSS_DIRECTION_EGRESS_E,
                tpidIndex+1, prvTgfErspanSameDevRestoreCfg.type[1]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet");
    }

    /*push 4B using TPID 0*/
    rc = cpssDxChBrgVlanPortEgressTpidSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,
                                            prvTgfErspanSameDevRestoreCfg.tpIdIndex[0]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortEgressTpidSet type %d ind %d",
                                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,tpidIndex );

    if(erspanType != PRV_TGF_ERSPAN_TYPE_III_CNS)
    {
        /*push 8B using TPID 1*/
        rc = cpssDxChBrgVlanPortEgressTpidSet(prvTgfDevNum, PRV_TGF_DEFAULT_EPORT_CNS,
                                                CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E,
                                                prvTgfErspanSameDevRestoreCfg.tpIdIndex[1]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPortEgressTpidSet type %d ind %d",
                                      CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E,tpidIndex+1 );
    }
}
/**
* @internal prvTgfFalconErspanSameDevMirrorIpv6ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv6 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
GT_VOID prvTgfFalconErspanSameDevMirrorIpv6ConfigurationSet
(
    CPSS_DIRECTION_ENT dir,
    GT_U32             erspanType,
    GT_BOOL            isFirstPass
)
{
    prvTgfFalconErspanSameDevMirrorConfigCommon(CPSS_IP_PROTOCOL_IPV6_E, dir, erspanType, isFirstPass);
    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        prvTgfFalconErspanSameDevMirrorConfigEgress();
    }
    else
    {
        prvTgfFalconErspanSameDevMirrorConfigIngress();
    }
    prvTgfFalconErspanSameDevMirrorIpv6ConfigExtra(erspanType);
}
/**
* @internal prvTgfFalconErspanSameDevMirrorIpv6ConfigurationReset function
* @endinternal
*
* @brief   ERSPAN same dev mirroring test configurations restore for
*           IPv6 tunnel encapsulation
*/
GT_VOID prvTgfFalconErspanSameDevMirrorIpv6ConfigurationReset
(
    CPSS_DIRECTION_ENT            dir,
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{
    prvTgfFalconErspanSameDevMirrorIpv6ConfigExtraReset(erspanType);
    prvTgfFalconErspanSameDevMirrorConfigurationReset(dir, erspanType, protocol);
}

/**
* @internal prvTgfFalconErspanISameDevMirrorConfigCommon function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN I encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
static GT_VOID prvTgfFalconErspanISameDevMirrorConfigCommon(CPSS_IP_PROTOCOL_STACK_ENT protocol, CPSS_DIRECTION_ENT dir)
{
    prvTgfFalconErspanSameDevMirrorBridgeConfigSet();
    /*mirror port config*/
    prvTgfFalconErspanSameDevMirrorConfig(PRV_TGF_ERSPAN_TYPE_I_CNS, protocol);

    /* AUTODOC: EPCL trigger for PHA */
    prvTgfFalconErspanSameDevMirrorEpclConfigSet(protocol, dir, PRV_TGF_ERSPAN_TYPE_I_CNS, GT_FALSE);

    /*PHA config*/
    prvTgfFalconErspanSameDevMirrorPhaConfig(protocol, PRV_TGF_ERSPAN_TYPE_I_CNS);

}

/**
* @internal prvTgfFalconErspanISameDevMirrorConfigEgress function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN I encapsulation
*    enable Rx mirroring for egress port;
*/
static GT_VOID prvTgfFalconErspanISameDevMirrorConfigEgress()
{
    GT_STATUS rc;

    /* AUTODOC: enable Tx mirroring on port=1, index=0 */
    rc = prvTgfMirrorTxPortGet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.txMirrorEnable, &prvTgfErspanSameDevRestoreCfg.     txMirrorIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortGet");
    rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS], GT_TRUE, analyzerIndex1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortSet");
}

/**
* @internal prvTgfFalconErspanISameDevMirrorConfigIngress function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN I encapsulation
*    enable Rx mirroring for ingress port;
*/
static GT_VOID prvTgfFalconErspanISameDevMirrorConfigIngress()
{
    GT_STATUS rc;

    /* AUTODOC: enable Rx mirroring on port=1, index=0 */
    rc= prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.rxMirrorEnable, &prvTgfErspanSameDevRestoreCfg.      rxMirrorIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorRxPortGet");
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE, analyzerIndex1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorRxPortSet");
}

/**
* @internal prvTgfFalconErspanISameDevMirrorTrafficVerify function
* @endinternal
*
* @brief  verify the outer IP+GRE encapsulation fields
*/
static GT_VOID prvTgfFalconErspanISameDevMirrorTrafficVerify(CPSS_IP_PROTOCOL_STACK_ENT protocol, GT_U8* packetBuf)
{
    GT_U32              greProtocolType, greFlags, erspanExpectedVal, erspanReceivedVal;
    GT_U16              greFlagsExpectedVal;
    GT_U8               pktOffset;

    pktOffset = TGF_L2_HEADER_SIZE_CNS;
    /*No vlan tag if vid = 0*/
    if (PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS != 0)
    {
        erspanExpectedVal = 0x8100;
        erspanReceivedVal = packetBuf[pktOffset] << 8 | packetBuf[pktOffset+1];
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                "outer ether type is not as expected => Expected:0x%x != Received:0x%x \n",
                erspanExpectedVal, erspanReceivedVal);

        erspanExpectedVal = PRV_TGF_ERSPAN_SAME_DEV_OUTER_VLAN_ID_CNS;
        erspanReceivedVal = packetBuf[pktOffset+TGF_ETHERTYPE_SIZE_CNS] << 8 | packetBuf[pktOffset+TGF_ETHERTYPE_SIZE_CNS+1];
        erspanReceivedVal &= 0xFFF;
        UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                "outer vlan id is not as expected => Expected:0x%x != Received:0x%x \n",
                erspanExpectedVal, erspanReceivedVal);

        pktOffset += TGF_VLAN_TAG_SIZE_CNS ;
    }
    pktOffset += TGF_ETHERTYPE_SIZE_CNS ;
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
        pktOffset += TGF_IPV4_HEADER_SIZE_CNS;
    else
        pktOffset += TGF_IPV6_HEADER_SIZE_CNS;

    /* AUTODOC: set expected value with GRE protocol type of ERSPAN I 0x88BE  */
    erspanExpectedVal = PRV_TGF_GRE_PROTOCOL_TYPE_ERSPAN_II_CNS;

    /* AUTODOC: set expected value with GRE flags for ERSPAN I as defined in standard */
    greFlagsExpectedVal = PRV_TGF_GRE_FLAGS_ERSPAN_I_CNS;

    /* AUTODOC: get the first 2B of GRE header to verify GRE flags */
    greFlags = (GT_U32)((packetBuf[pktOffset] << 8) | (packetBuf[pktOffset + 1]));

    /* AUTODOC: compare received GRE flags vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(greFlagsExpectedVal, greFlags,
                                "--- Failure in tgfEgressMirrorErspanTypeIVerification --- \n"
                                "GRE flags is not as expected => Expected:0x%x != Received:0x%x \n",
                                 greFlagsExpectedVal, greFlags);

    /* AUTODOC: get the next 2B of GRE header to verify GRE protocol */
    greProtocolType = (GT_U32)((packetBuf[pktOffset + 2] << 8) | packetBuf[pktOffset + 3]);

    /* AUTODOC: compare received GRE protocol type vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL2_STRING_MAC(erspanExpectedVal, greProtocolType,
                                "--- Failure in tgfEgressMirrorErspanTypeIVerification --- \n"
                                "GRE protocol type is not as expected => Expected:0x%x != Received:0x%x \n",
                                 erspanExpectedVal, greProtocolType);

    /*verfiy start of passenger*/
    pktOffset +=4;
    erspanExpectedVal = 0;
    erspanReceivedVal = cpssOsMemCmp(prvTgfOriginalPacketL2Part.daMac, &packetBuf[pktOffset], 6);
    /* AUTODOC: compare received passenger L2 da mac vs expected and notify if there is any mismatch */
    UTF_VERIFY_EQUAL0_STRING_MAC(erspanExpectedVal, erspanReceivedVal,
                                "--- Failure in tgfEgressMirrorErspanTypeIVerification --- \n"
                                "passenger L2 da mac is not as expected \n");
}

/**
* @internal prvTgfFalconErspanISameDevMirrorTraffic function
* @endinternal
*
* @brief
*           Test the captures for the configs same device mirroring ERSPAN type I.
*           RX:
*               L2 pkt for Vlan5
*           TX:
*               2 pkts on 2 Egr analyzer ports.
*               1. IPv4 encapsulated
*               2. IPv6 encapsulated
*/
GT_VOID prvTgfFalconErspanISameDevMirrorTraffic(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL               getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS             rc = GT_OK;
    GT_U8                 pktCount=PRV_TGF_SAMPLE_MIRROR_RATE_CNS;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfOriginalPacketInfo, pktCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* AUTODOC: Get the first rx pkt on egress port */
    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
            TGF_PACKET_TYPE_CAPTURE_E,
            getFirst, GT_TRUE, packetBuf,
            &packetLen, &packetActualLength,
            &dev, &queue, &rxParam);
    if (GT_OK != rc && GT_NO_MORE != rc)
    {
        PRV_UTF_LOG1_MAC("Error: failed capture packet rc=%d\n",rc);
        return;
    }
    getFirst = GT_FALSE;

    /* Disable capture on analyzer port 1*/
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /*verify the packet*/
    prvTgfFalconErspanISameDevMirrorTrafficVerify(protocol, packetBuf);
}

/**
* @internal prvTgfFalconErspanISameDevMirrorIpv4ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN I encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
void    prvTgfFalconErspanISameDevMirrorIpv4ConfigurationSet(CPSS_DIRECTION_ENT dir)
{
    prvTgfFalconErspanISameDevMirrorConfigCommon(CPSS_IP_PROTOCOL_IPV4_E, dir);
    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        prvTgfFalconErspanISameDevMirrorConfigEgress();
    }
    else
    {
        prvTgfFalconErspanISameDevMirrorConfigIngress();
    }
}

/**
* @internal prvTgfFalconErspanISameDevMirrorConfigurationReset function
* @endinternal
*
* @brief   ERSPAN I same dev mirroring test configurations restore
*/
GT_VOID prvTgfFalconErspanISameDevMirrorConfigurationReset
(
    CPSS_DIRECTION_ENT            dir,
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{
    GT_STATUS rc;

    prvTgfFalconErspanSameDevMirrorBridgeConfigReset();
    prvTgfFalconErspanSameDevMirrorConfigReset(erspanType, protocol);
    if (dir == CPSS_DIRECTION_EGRESS_E)
    {
        rc = prvTgfMirrorTxPortSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                prvTgfErspanSameDevRestoreCfg.txMirrorEnable,
                                prvTgfErspanSameDevRestoreCfg.txMirrorIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortSet");
    }
    else
    {
        rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                prvTgfErspanSameDevRestoreCfg.rxMirrorEnable,
                                prvTgfErspanSameDevRestoreCfg.rxMirrorIdx);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in prvTgfMirrorTxPortSet");
    }
    prvTgfFalconErspanSameDevMirrorEpclConfigReset(protocol, dir, erspanType);
    /*restore PHA FW image*/
    prvTgfPhaFWRestore();
    /*restore all buffers and reset mac counters*/
    tgfTrafficTableRxPcktTblClear();
    prvTgfEthCountersReset(prvTgfDevNum);
}

/**
* @internal prvTgfFalconErspanISameDevMirrorIpv6ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with ERSPAN I encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv6 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of ERSPAN header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send traffic and verify expected mirrored traffic on analyzer port;
*/
GT_VOID prvTgfFalconErspanISameDevMirrorIpv6ConfigurationSet(CPSS_DIRECTION_ENT dir)
{
    prvTgfFalconErspanISameDevMirrorConfigCommon(CPSS_IP_PROTOCOL_IPV6_E, dir);
    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        prvTgfFalconErspanSameDevMirrorConfigEgress();
    }
    else
    {
        prvTgfFalconErspanSameDevMirrorConfigIngress();
    }
}


/**
* @internal prvTgfFalconErspanISameDevMirrorIpv6ConfigurationReset function
* @endinternal
*
* @brief   ERSPAN I same dev mirroring test configurations restore
*/
GT_VOID    prvTgfFalconErspanISameDevMirrorIpv6ConfigurationReset
(
    CPSS_DIRECTION_ENT            dir,
    GT_U32                        erspanType,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{
    prvTgfFalconErspanISameDevMirrorConfigurationReset(dir, erspanType, protocol);
}

/*************************************************************************/
/*Falcon ERSPAN type III Orig/Trg dev mirroring*/
/*************************************************************************/

/**
* @internal prvTgfFalconCCErspanIIIBridgeConfigExtraSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfFalconCCErspanIIIBridgeConfigExtraSet()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryStore(1);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", 1);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", 1);

    /*remove all, except ingress, remote ports from vlan 5*/
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: %d", PRV_TGF_VLANID_CNS);
}

/**
* @internal prvTgfFalconCCErspanIIICscdConfigSet function
* @endinternal
*
* @brief   Falcon ERSPAN III Cascade test configurations
*/
static GT_VOID prvTgfFalconCCErspanIIICscdConfigSet
(
    GT_BOOL    isOrigDevMirror
)
{
    GT_STATUS                  rc = GT_OK;
    CPSS_CSCD_LINK_TYPE_STC    cascadeLink;

    if(isOrigDevMirror == GT_TRUE)
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                     SOURCE_FALCON_DEVICE_NUMBER_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);

        /* cascade link info */
        cascadeLink.linkType = CPSS_CSCD_LINK_TYPE_PORT_E;
        /* set this 'Remote device' to point to the device map table via cascade port */
        cascadeLink.linkNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];
        prvTgfRestoreCfg.targetHwDevNumGet = TARGET_FALCON_DEVICE_NUMBER_CNS;
        /* save the current cascade map table */
        rc = prvTgfCscdDevMapTableGet(prvTgfDevNum, prvTgfRestoreCfg.targetHwDevNumGet,
                0,0,
                &(prvTgfRestoreCfg.cascadeLinkGet),&(prvTgfRestoreCfg.srcPortTrunkHashEn));
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

        rc = cpssDxChCscdDevMapTableSet(prvTgfDevNum, TARGET_FALCON_DEVICE_NUMBER_CNS, 0, 0, 0, &cascadeLink,
                                        0, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                                   prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                   CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                     TARGET_FALCON_DEVICE_NUMBER_CNS, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    }
}

/**
* @internal prvTgfFalconCCErspanIIIMirrorConfigSet function
* @endinternal
*
* @brief   Target Device mirror config
*           map eport to phy port#1
*           Set ingress sampling ratio
*
*/
static GT_VOID prvTgfFalconCCErspanIIIMirrorConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC phyPortInfo;

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS,
                                                           &(prvTgfErspanSameDevRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");

    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];

    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS, &phyPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");
}

/**
* @internal prvTgfFalconCCErspanIIIPhaPortEntryConfigSet function
* @endinternal
*
* @brief   Falcon ERSPAN III Pha port entry test configurations
*/
static GT_VOID prvTgfFalconCCErspanIIIPhaPortEntryConfigSet
(
    CPSS_DIRECTION_ENT    dir
)
{
    GT_STATUS    rc;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT trgPortInfo;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT srcPortInfo;

    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        rc = cpssDxChPhaTargetPortEntryGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.trgPortType, &prvTgfErspanSameDevRestoreCfg.trgPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaTargetPortEntryGet");

        trgPortInfo.erspanSameDevMirror.erspanIndex = PRV_TGF_ERSPAN_SAME_DEV_INDEX;
        rc = cpssDxChPhaTargetPortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E, &trgPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaTargetPortEntrySet");
    }
    else
    {
        rc = cpssDxChPhaSourcePortEntryGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], &prvTgfErspanSameDevRestoreCfg.srcPortType, &prvTgfErspanSameDevRestoreCfg.srcPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntryGet");
        srcPortInfo.erspanSameDevMirror.erspanIndex = PRV_TGF_ERSPAN_SAME_DEV_INDEX;
        rc = cpssDxChPhaSourcePortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ERSPAN_TYPE_II_E, &srcPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");
    }
}

/**
* @internal prvTgfFalconCCErspanIIIEpclConfigSet function
* @endinternal
*
* @brief   Falcon ERSPAN III EPCL test configurations
*/
static GT_VOID prvTgfFalconCCErspanIIIEpclConfigSet
(
    GT_BOOL                       isOrigDevMirror,
    CPSS_DIRECTION_ENT            dir,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    GT_U32                              portNum;
    GT_PORT_NUM                         eport = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS;


    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    portNum = ((dir == CPSS_DIRECTION_EGRESS_E) && (isOrigDevMirror == GT_TRUE)) ?
              prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS] : prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            portNum,
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");
    /* AUTODOC: enable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          portNum,
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          portNum,
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          portNum,
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(portNum,
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfCCErspanIIIEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 27 Tag0 Exist[5] */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[2] egress mtag cmd[1:0] TO_ANALYZER */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfCCErspanIIIEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfCCErspanIIIEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfCCErspanIIIEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfCCErspanIIIEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[1] = (eport&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[1] = prvTgfCCErspanIIIEpclMetadataUdbInfo[1].byteMask;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[2] = ((eport>>7)&0x3f);
    mask.ruleEgrUdbOnly.udb[2] = prvTgfCCErspanIIIEpclMetadataUdbInfo[2].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[3] = (eport>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[3] = prvTgfCCErspanIIIEpclMetadataUdbInfo[3].byteMask;

    if(isOrigDevMirror == GT_TRUE)
    {
        /*ingress marvell tagged[5] egress marvell tagged[2] egress mtag cmd[1:0] */
        pattern.ruleEgrUdbOnly.udb[4] = 0x06;
        mask.ruleEgrUdbOnly.udb[4] = prvTgfCCErspanIIIEpclMetadataUdbInfo[4].byteMask;
    }
    else
    {
        /*ingress marvell tagged[5] egress marvell tagged[2] egress mtag cmd[1:0] */
        pattern.ruleEgrUdbOnly.udb[4] = 0x22;
        mask.ruleEgrUdbOnly.udb[4] = prvTgfCCErspanIIIEpclMetadataUdbInfo[4].byteMask;
    }

    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ERSPAN_TYPE_II_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclErspanTypeIImirror.erspanAnalyzerIndex = 1;

    if(isOrigDevMirror == GT_TRUE)
    {
        ruleIndex = 0;
        action.epclPhaInfo.phaThreadId = PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_ORIG_DEV_MIRROR_CNS;
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
    }
    else
    {
        /* rule 0 for untagged packets */
        mask.ruleEgrUdbOnly.udb[0] = prvTgfCCErspanIIIEpclMetadataUdbInfo[0].byteMask;

        ruleIndex = 0;
        action.oam.oamProcessEnable  = GT_TRUE;
        action.oam.oamProfile        = PRV_TGF_OAM_PROFILE_INDEX_CNS;
        action.flowId                = PRV_TGF_EPCL_FLOW_ID_CNS;
        action.policer.policerEnable = PRV_TGF_PCL_POLICER_ENABLE_COUNTER_ONLY_E;

        if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
        {
            action.epclPhaInfo.phaThreadId = PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_UNTAGGED_CNS;
        }
        else if(protocol == CPSS_IP_PROTOCOL_IPV6_E)
        {
            action.epclPhaInfo.phaThreadId = PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_UNTAGGED_CNS;
        }
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
        /* rule 1 for tagged packets */
        /* Tag0 Exist[5] */
        pattern.ruleEgrUdbOnly.udb[0] = 0x20;

        ruleIndex = 1;
        if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
        {
            action.epclPhaInfo.phaThreadId = PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_TAGGED_CNS;
        }
        else if(protocol == CPSS_IP_PROTOCOL_IPV6_E)
        {
            action.epclPhaInfo.phaThreadId = PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_TAGGED_CNS;
        }
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
    }
}

/**
* internal prvTgfFalconCCErspanIIIOrigDevMirroringConfigSet function
* @endinternal
*
* @brief   Falcon Original Device Mirroring ERSPAN III CC use case configurations
*/
GT_VOID prvTgfFalconCCErspanIIIOrigDevMirroringConfigSet
(
    GT_BOOL                       isOrigDevMirror,
    GT_BOOL                       isTagged,
    CPSS_DIRECTION_ENT            dir,
    CPSS_IP_PROTOCOL_STACK_ENT    protocol
)
{
    GT_U32    dev = (isOrigDevMirror == GT_TRUE) ?
                    SOURCE_FALCON_DEVICE_NUMBER_CNS : TARGET_FALCON_DEVICE_NUMBER_CNS;
    GT_U32    phaThreadId = 0;

    /* Bridge Configuration */
    prvTgfFalconMirrorIngressErspanBridgeConfigSet(dev, (dir == CPSS_DIRECTION_INGRESS_E) ? 0 : 1, isOrigDevMirror);
    prvTgfFalconCCErspanIIIBridgeConfigExtraSet();

    /* Cascade Configuration */
    prvTgfFalconCCErspanIIICscdConfigSet(isOrigDevMirror);

    if(isOrigDevMirror != GT_TRUE)
    {
        /* Mirror Configuration */
        prvTgfFalconCCErspanIIIMirrorConfigSet();
    }

    /* HA Configuration */
    if(isOrigDevMirror == GT_TRUE)
    {
        phaThreadId = PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_ORIG_DEV_MIRROR_CNS;
    }
    else
    {
        if(protocol == CPSS_IP_PROTOCOL_IPV4_E)
        {
            phaThreadId = (isTagged == GT_TRUE) ?
                          PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_TAGGED_CNS :
                          PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV4_UNTAGGED_CNS;
        }

        else if(protocol == CPSS_IP_PROTOCOL_IPV6_E)
        {
            phaThreadId = (isTagged == GT_TRUE) ?
                          PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_TAGGED_CNS :
                          PRV_TGF_PHA_THREAD_ID_CC_ERSPAN_TYPE_III_TRG_DEV_MIRROR_IPV6_UNTAGGED_CNS;
        }
    }

    prvTgfIngressErspanSourceFalconHaConfigSet(PRV_TGF_ERSPAN_TYPE_III_CNS, protocol, GT_TRUE, phaThreadId);

    /* Pha Port Entry Configuration */
    prvTgfFalconCCErspanIIIPhaPortEntryConfigSet(dir);

    /* EPCL Configuration */
    prvTgfFalconCCErspanIIIEpclConfigSet(isOrigDevMirror, dir, protocol);

    if(isOrigDevMirror != GT_TRUE)
    {
        /* EOAM Configuration */
        prvTgfIngressErspanEoamConfigSet();

        /* EPLR Configuration */
        prvTgfIngressErspanEplrConfigSet();
    }

    /* ERMRK Configuration */
    prvTgfIngressErspanErmrkConfigSet();
}

/**
* @internal prvTgfFalconCCErspanIIIMirrorTraffic function
* @endinternal
*
* @brief
*           Test the captures for the configs CC ERSPAN type III mirroring.
*           RX:
*               L2 pkt for Vlan5
*           TX:
*               2 pkts on 2 Egr analyzer ports.
*               1. IPv4 encapsulated
*               2. IPv6 encapsulated
*/
GT_VOID prvTgfFalconCCErspanIIIMirrorTraffic
(
    GT_BOOL                    isOrigDevMirror,
    GT_BOOL                    isTagged,
    CPSS_IP_PROTOCOL_STACK_ENT protocol,
    GT_U32                     erspanType,
    CPSS_DIRECTION_ENT         dir
)
{
    TGF_PACKET_STC        prvTgfPacketInfo;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;
    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U32                numOfPacketsToAnalyze = 0;
    GT_U32                portIter = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL               getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS             rc = GT_OK;
    GT_U8                 pktCount=PRV_TGF_SAMPLE_MIRROR_RATE_CNS;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];

    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    if(isOrigDevMirror != GT_TRUE)
    {
        prvTgfDsaTag.commonParams.vpt = 1;
        prvTgfDsaTag.commonParams.vid = 5;

        /* eDSA rx sniffed port's attributes */
        if(dir == CPSS_DIRECTION_INGRESS_E) /* ingress erspan */
        {
            prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.hwDevNum = SOURCE_FALCON_DEVICE_NUMBER_CNS;              /* Original device */
            prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.portNum  = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];  /* src port   */
            prvTgfDsaInfo_rxMirrorToAnalyzer.devPort.ePort    = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];  /* src eport  */

            prvTgfDsaInfo_rxMirrorToAnalyzer.isTagged = isTagged;

            prvTgfDsaInfo_rxMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
            prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
            prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = TARGET_FALCON_DEVICE_NUMBER_CNS ;/* target Falcon */
            prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
            prvTgfDsaInfo_rxMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

            /* assign an analyzer part of eDSA tag at run time because a union is used */
            prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_rxMirrorToAnalyzer;
        }
        else if(dir == CPSS_DIRECTION_EGRESS_E) /* egress erspan */
        {
            prvTgfDsaInfo_txMirrorToAnalyzer.devPort.hwDevNum = SOURCE_FALCON_DEVICE_NUMBER_CNS;              /* Original device */
            prvTgfDsaInfo_txMirrorToAnalyzer.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];  /* src port   */
            prvTgfDsaInfo_txMirrorToAnalyzer.devPort.ePort    = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];  /* src eport  */
            prvTgfDsaInfo_txMirrorToAnalyzer.rxSniffer        = GT_FALSE;                                     /* egress mirroring  */

            prvTgfDsaInfo_txMirrorToAnalyzer.isTagged = isTagged;

            prvTgfDsaInfo_txMirrorToAnalyzer.analyzerTrgType                            = CPSS_INTERFACE_PORT_E;
            prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerIsTrgPortValid = GT_FALSE;
            prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerHwTrgDev       = TARGET_FALCON_DEVICE_NUMBER_CNS;/* source falcon */
            prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgPort        = REMOTE_DEVICE_PORT_NUM_CNS - 1;        /* dummy port */
            prvTgfDsaInfo_txMirrorToAnalyzer.extDestInfo.devPort.analyzerTrgEport       = PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS; /* global eport */

            /* assign an analyzer part of eDSA tag at run time because a union is used */
            prvTgfDsaTag.dsaInfo.toAnalyzer = prvTgfDsaInfo_txMirrorToAnalyzer;
        }
    }

    if(isOrigDevMirror == GT_TRUE)
    {
        prvTgfPacketInfo = prvTgfOriginalPacketInfo;
    }
    else
    {
        if(dir == CPSS_DIRECTION_INGRESS_E)
        {
            prvTgfPacketInfo = prvTgfIngressErspanIIICscdPacketInfo;
        }
        else
        {
            prvTgfPacketInfo = prvTgfEgressErspanIIICscdPacketInfo;
        }
    }
    rc = prvTgfSetTxSetupEth(prvTgfDevNum,
                             &prvTgfPacketInfo,
                             pktCount, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* provide time for TO_CPU packet RX process */
    cpssOsTimerWkAfter(10);

    /* Disable capture on analyzer port 1*/
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* AUTODOC: Get the rx pkts on egress port */
    numOfPacketsToAnalyze = (erspanType == PRV_TGF_ERSPAN_TYPE_III_CNS) ?
                            pktCount : pktCount/PRV_TGF_SAMPLE_MIRROR_RATE_CNS;
    pktCount = 0;
    while(pktCount != numOfPacketsToAnalyze)
    {
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        if (GT_OK != rc && GT_NO_MORE != rc)
        {
            PRV_UTF_LOG1_MAC("Error: failed capture packet rc=%d\n",rc);
            return;
        }

        getFirst = GT_FALSE;

        /*verify the packet*/
        pktCount++;
        prvTgfFalconErspanSameDevMirrorTrafficVerify(isOrigDevMirror, protocol, packetBuf, pktCount, erspanType, dir, GT_TRUE, isTagged);
    }

    /* Restoring common param values */
    prvTgfDsaTag.commonParams.vpt = 0;
    prvTgfDsaTag.commonParams.vid = 5;
    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        prvTgfDsaInfo_txMirrorToAnalyzer.isTagged = GT_TRUE;
    }
    else
    {
        prvTgfDsaInfo_rxMirrorToAnalyzer.isTagged = GT_TRUE;
    }

    /* -------------------------------------------------------------------------
     * Dump Ethernet Counters
     */

    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* internal prvTgfFalconMirrorIngressErspanBridgeConfigReset function
* @endinternal
*
* @brief   Falcon ERSPAN III CC use case Bridge config restore
*/
static GT_VOID prvTgfFalconMirrorIngressErspanBridgeConfigReset
(
    CPSS_DIRECTION_ENT    dir
)
{
    GT_STATUS    rc;

    /* -------------------------------------------------------------------------
     * 1. Restore HW device Configuration
     */
    /* AUTODOC: restore HW device number */
    rc = cpssDxChCfgHwDevNumSet(prvTgfDevNum, prvTgfRestoreCfg.savedDevNum);
    UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);

    /* -------------------------------------------------------------------------
     * 2. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_ETHERNET_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_ETHERNET_VLANID_CNS);

    /* AUTODOC: Enable bridge bypass */
    rc = cpssDxChCscdPortBridgeBypassEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");

    if(dir == CPSS_DIRECTION_INGRESS_E)
    {
        /* AUTODOC: disable Rx mirroring based on ePort=3(CC default ePort), index=2 */
        rc = cpssDxChMirrorRxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                     GT_FALSE, GT_FALSE, REMOTE_DEVICE_ANALYZER_NUM_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorRxPortSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], GT_FALSE);
    }
    else if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        /* AUTODOC: disable Tx mirroring based on ePort=2(CC default ePort), index=2 */
        rc = cpssDxChMirrorTxPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                     GT_FALSE, GT_FALSE, REMOTE_DEVICE_ANALYZER_NUM_CNS);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChMirrorTxPortSet: %d %d",
                                     prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_FALSE);
    }
    /* AUTODOC: restore default vlan config */
    prvTgfBrgVlanEntryRestore(1);
}

/**
* internal prvTgfFalconCCErspanIIICscdConfigReset function
* @endinternal
*
* @brief   Falcon ERSPAN III CC use case Cascade config restore
*/
static GT_VOID prvTgfFalconCCErspanIIICscdConfigReset
(
    GT_BOOL    isOrigDevMirror
)
{
    GT_STATUS    rc = GT_OK;

    /* -------------------------------------------------------------------------
     * 1. Restore Cascade Configuration
     */
    if(isOrigDevMirror == GT_TRUE)
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_TX_E,
                                   prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS],
                                   CPSS_CSCD_PORT_NETWORK_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                     SOURCE_FALCON_DEVICE_NUMBER_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS]);
        rc = prvTgfCscdDevMapTableSet(
            prvTgfDevNum, TARGET_FALCON_DEVICE_NUMBER_CNS, 0, 0,
                &(prvTgfRestoreCfg.cascadeLinkGet),
                prvTgfRestoreCfg.srcPortTrunkHashEn, GT_FALSE);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, rc);
    }
    else
    {
        rc = prvTgfCscdPortTypeSet(prvTgfDevNum, CPSS_PORT_DIRECTION_RX_E,
                                   prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS],
                                   CPSS_CSCD_PORT_NETWORK_E);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfCscdPortTypeSet: %d %d",
                                     TARGET_FALCON_DEVICE_NUMBER_CNS, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    }
}

/**
* @internal prvTgfFalconCCErspanIIIMirrorConfigReset function
* @endinternal
*
* @brief   Falcon ERSPAN III CC use case Mirror config restore
*/
static GT_VOID prvTgfFalconCCErspanIIIMirrorConfigReset
(
    GT_VOID
)
{

    GT_STATUS rc;

    /* AUTODOC: Restore ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           PRV_TGF_DESTINATION_PRIMARY_EPORT_CNS,
                                                           &prvTgfErspanSameDevRestoreCfg.phyPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");
}

/**
* internal prvTgfFalconCCErspanIIIPhaPortEntryConfigReset function
* @endinternal
*
* @brief   Falcon ERSPAN III CC use case PHA port entry config restore
*/
static GT_VOID prvTgfFalconCCErspanIIIPhaPortEntryConfigReset
(
    CPSS_DIRECTION_ENT    dir
)
{
    GT_STATUS rc = GT_OK;
    /* -------------------------------------------------------------------------
     * 1. Restore PHA Port Entry Configuration
     */
    if(dir == CPSS_DIRECTION_EGRESS_E)
    {
        rc = cpssDxChPhaTargetPortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], prvTgfErspanSameDevRestoreCfg.trgPortType, &prvTgfErspanSameDevRestoreCfg.trgPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaTargetPortEntrySet");
    }
    else if(dir == CPSS_DIRECTION_INGRESS_E)
    {
        rc = cpssDxChPhaSourcePortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS], prvTgfErspanSameDevRestoreCfg.srcPortType, &prvTgfErspanSameDevRestoreCfg.srcPortInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");
    }
}

/**
* @internal prvTgfFalconCCErspanIIIEpclConfigReset function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfFalconCCErspanIIIEpclConfigReset
(
    GT_BOOL            isOrigDevMirror,
    CPSS_DIRECTION_ENT dir
)
{
    GT_STATUS   rc;
    GT_U32      portNum;

    portNum = ((dir == CPSS_DIRECTION_EGRESS_E) && (isOrigDevMirror == GT_TRUE)) ?
              prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS] : prvTgfPortsArray[PRV_TGF_ANALYZER_PORT1_IDX_CNS];

    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    if(isOrigDevMirror != GT_TRUE)
    {
        /* AUTODOC: Invalidate PCL rule 1 */
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 1, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);
    }

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Analyzer port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          portNum,
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

}

/**
* @internal prvTgfIngressErspanEoamConfigReset function
* @endinternal
*
* @brief   Restore EOAM config
*/
static GT_VOID prvTgfIngressErspanEoamConfigReset
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* -------------------------------------------------------------------------
     *  Restore EOAM Configuration
     */
    /* AUTODOC: restore common EOAM configurations */
    prvTgfOamConfigurationSet(&prvTgfRestoreCfg.oamConfig);

    /* AUTODOC: restore OAM processing - Egress */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_EGRESS_E, prvTgfRestoreCfg.origOamEnStatus);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfOamEnableSet");
}

/**
* @internal prvTgfIngressErspanEplrConfigReset function
* @endinternal
*
* @brief   Restore EPLR config
*/
static GT_VOID prvTgfIngressErspanEplrConfigReset
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* -------------------------------------------------------------------------
     *  Restore EPLR Configuration
     */
    /* AUTODOC: Restore Policer Egress to Analyzer metering and counting status */
    rc = cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet(prvTgfDevNum,
                                                prvTgfRestoreCfg.meteringCountingStatus);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChPolicerEgressToAnalyzerMeteringAndCountingEnableSet");

    /* AUTODOC: Restore stage mode */
    rc = prvTgfPolicerStageMeterModeSet(PRV_TGF_POLICER_STAGE_EGRESS_E, stageModeSave);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPolicerStageMeterModeSet: %d",
                                 stageModeSave);
}

/**
* @internal prvTgfIngressErspanErmrkConfigReset function
* @endinternal
*
* @brief   Restore ERMRK config and traffic engine config
*/
static GT_VOID prvTgfIngressErspanErmrkConfigReset
(
    GT_VOID
)
{
    GT_STATUS   rc;
    /* -------------------------------------------------------------------------
     * Restore ERMRK Configuration
     */
    /* AUTODOC: Restore LM Counter status */
    rc = cpssDxChOamLmStampingEnableSet(prvTgfDevNum, prvTgfRestoreCfg.lmStampingEnable);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in cpssDxChOamLmStampingEnableSet");

    /* -------------------------------------------------------------------------
     *  Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}

/**
* internal prvTgfFalconCCErspanIIIOrigDevMirroringConfigReset function
* @endinternal
*
* @brief   Falcon Original Device Mirroring ERSPAN III CC use case reset
*/
GT_VOID prvTgfFalconCCErspanIIIOrigDevMirroringConfigReset
(
    GT_BOOL               isOrigDevMirror,
    CPSS_DIRECTION_ENT    dir
)
{
    /* Bridge Reset */
    prvTgfFalconMirrorIngressErspanBridgeConfigReset(dir);

    /* Cascade Reset */
    prvTgfFalconCCErspanIIICscdConfigReset(isOrigDevMirror);

    if(isOrigDevMirror != GT_TRUE)
    {
        /* Mirror Reset */
        prvTgfFalconCCErspanIIIMirrorConfigReset();
    }

    /* Pha Port Entry Reset */
    prvTgfFalconCCErspanIIIPhaPortEntryConfigReset(dir);

    /* EPCL Reset */
    prvTgfFalconCCErspanIIIEpclConfigReset(isOrigDevMirror, dir);

    if(isOrigDevMirror != GT_TRUE)
    {
        /* EOAM Reset */
        prvTgfIngressErspanEoamConfigReset();

        /* EPLR Reset */
        prvTgfIngressErspanEplrConfigReset();
    }

    /* ERMRK Reset */
    prvTgfIngressErspanErmrkConfigReset();

}
