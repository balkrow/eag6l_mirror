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
* @file prvTgfIfaUseCase.c
*
* @brief IFA use cases testing.
*
* @version   1
********************************************************************************
*/
#include <tam/prvTgfIfaUseCase.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfMirror.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/utfMain.h>
#include <extUtils/trafficEngine/tgfTrafficTable.h>
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTam.h>
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTamIfa.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTam.h>
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTamIfa.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <common/tgfCncGen.h>

/* port number for traffic test */
#define PRV_TGF_TX_PORT_IDX_CNS        0
#define PRV_TGF_RX_PORT_IDX_CNS        3
#define PRV_TGF_RX_PORT2_IDX_CNS       2
#define PRV_TGF_LB_PORT_IDX_CNS        1

/* PHA fw thread IDs */
#define PRV_TGF_PHA_THREAD_ID_IFA_INGRESS_HEADER_CNS   71
#define PRV_TGF_PHA_THREAD_ID_IFA_INGRESS_METADATA_CNS 72
#define PRV_TGF_PHA_THREAD_ID_IFA_TRANSIT_METADATA_CNS 73
#define PRV_TGF_PHA_THREAD_ID_IFA_EGRESS_MIRROR_CNS    74
#define PRV_TGF_PHA_THREAD_ID_IFA_EGRESS_METADATA_CNS  75

/* VLAN-ID */
#define PRV_TGF_VLANID_CNS                              5

/* Tunnel attributes */
#define PRV_TGF_TUNNEL_VLANID_CNS                       10
#define PRV_TGF_TUNNEL_TTL_CNS                          20
#define PRV_TGF_TUNNEL_IFA_UDP_DST_PORT_CNS             0x1289
#define PRV_TGF_TUNNEL_IFA_UDP_SRC_PORT_CNS             0xABEF

/*EPCL rule index*/
#define PRV_TGF_ENHANCED_IFA_INGRESS_NODE_EPCL_IDX_CNS  0

/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS              1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS             10

#define PRV_TGF_DEFAULT_EPORT_CNS                       1024
#define PRV_TGF_PASS2_EPORT_CNS                         1027

/* Packet flow Id to set by PCL Action  */
#define PRV_TGF_EPCL_INGRESS_NODE_IFA_FLOW_ID_CNS       0x95
#define PRV_TGF_EPCL_TRANSIT_NODE_IFA_FLOW_ID_CNS       0x1FF
#define PRV_TGF_EPCL_EGRESS_NODE_IFA_FLOW_ID_CNS        0x12C

/*analyzer index for mirroring */
#define PRV_TGF_IFA_MIRROR_IDX_CNS                      2

/*analyzer index for egress mirroring */
#define PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS               5

/*sampling rate*/
#define PRV_TGF_IFA_MIRROR_RATE_CNS                     1

#define PRV_TGF_TTI_INDEX_CNS                           1
#define PRV_TGF_TTI_INDEX2_CNS                          2
#define PRV_TGF_TTI_INDEX3_CNS                          10
#define PRV_TGF_TTI_PCL_INDEX_CNS                       2
#define PRV_TGF_EPCL_MATCH1_INDEX_CNS                   prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(5)
#define PRV_TGF_EPCL_MATCH2_INDEX_CNS                   prvTgfEpclTcamAbsoluteIndexWithinTheLookupGet(10)

/* size of packet */
#define PRV_TGF_PACKET_SIZE_CNS                         300
#define PRV_TGF_IPV4_TTL_OFFSET_CNS                     8
#define PRV_TGF_IPV6_HOP_LIMIT_OFFSET_CNS               7
#define PRV_TGF_IFA_HEADER_PROBE1_SIZE_CNS              4
#define PRV_TGF_IFA_HEADER_PROBE2_SIZE_CNS              4
#define PRV_TGF_IFA_HEADER_TELEMETRY_REQUEST_SIZE_CNS   2
#define PRV_TGF_IFA_HEADER_TELEMETRY_ACTION_SIZE_CNS    2
#define PRV_TGF_TTL_OFFSET_CNS                          8

/* IFA Node Types */
#define PRV_TGF_IFA_INGRESS_NODE_CNS                    0
#define PRV_TGF_IFA_TRANSIT_NODE_CNS                    1
#define PRV_TGF_IFA_EGRESS_NODE_CNS                     2

/* IFA Packet Types */
#define PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS            0
#define PRV_TGF_IFA_PACKET_TYPE_IPv6_TCP_CNS            1


static CPSS_DXCH_PHA_THREAD_INFO_TYPE_IFA_HEADER_STC ifaHeaderPart = {
    BIT_4,
    0xFF,
    BIT_0,
    0xDEADBEEF,
    0xA5A5DEED
};

static CPSS_DXCH_PHA_THREAD_INFO_TYPE_IFA_METADATA_STC ifaMetadataPart = {
    0x19af
};

static CPSS_DXCH_PHA_IFA_FLOW_ENTRY_STC         ifaEntry;
static CPSS_INTERFACE_INFO_STC                  egressPortInterface;
static GT_U32                                   counterIdx;
static CPSS_DXCH_CNC_COUNTER_STC                counter2;

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

/******************************* UDP packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},                /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11}                 /* saMac */
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    TGF_ETHERTYPE_0800_IPV4_TAG_CNS
};

/* VLAN_TAG part of packet */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                     /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                             /* pri, cfi, VlanId */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    4,                  /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4UdpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    3,                  /* typeOfService */
    0x26,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_UDP_E, /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS, /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* DATA of UDP packet */
static GT_U8 prvTgfPacketUdpPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketUdpPayloadPart = {
    sizeof(prvTgfPacketUdpPayloadDataArr),                 /* dataLength */
    prvTgfPacketUdpPayloadDataArr                          /* dataPtr */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfIpv4UdpPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4UdpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketUdpPayloadPart}
};

static TGF_PACKET_STC prvTgfIpv4UdpPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketUdpPayloadDataArr)),                      /* totalLen */
    sizeof(prvTgfIpv4UdpPacketArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfIpv4UdpPacketArray                                         /* partsArray */
};

/* DATA of IFA packet */
static GT_U8 prvTgfPacketIfaPayloadDataArr[] = {
    0xde, 0xad, 0xbe, 0xef, 0xa5, 0xa5, 0xde, 0xed, 0x10, 0x01,
    0x01, 0x10, 0x00, 0xFF, 0x00, 0x01, 0x05, 0x01, 0x00, 0x00,
    0x05, 0xdc, 0x00, 0x20, 0xab, 0xcd, 0x00, 0x00, 0x00, 0x00,
    0x19, 0xaf, 0x10, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0c, 0xc3, 0x13, 0x37, 0xb3, 0xd1, 0x13, 0x37, 0xbf, 0x54,
    0x00, 0x00, 0x00, 0x77, 0x00, 0x41, 0x00, 0x00, 0x00, 0x08,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
};

/* IFA PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketIfaPayloadPart = {
    sizeof(prvTgfPacketIfaPayloadDataArr),                 /* dataLength */
    prvTgfPacketIfaPayloadDataArr                          /* dataPtr */
};

/* PARTS of IFA packet */
static TGF_PACKET_PART_STC prvTgfIpv4UdpIfaPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4UdpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIfaPayloadPart}
};

static TGF_PACKET_STC prvTgfIpv4UdpIfaPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketIfaPayloadDataArr)),                          /* totalLen */
    sizeof(prvTgfIpv4UdpIfaPacketArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfIpv4UdpIfaPacketArray                                         /* partsArray */
};


/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypeIpV6Part = {
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6TcpPart =
{
    6,                                  /* version */
    0,                                  /* trafficClass */
    0,                                  /* flowLabel */
    (20  + sizeof(prvTgfPacketIfaPayloadDataArr)),    /* payloadLen - TCP + IFA */
    6,                                  /* TCP */
    0x41,                               /* hopLimit */
    {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17},   /* srcAddr */
    {0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, 0x81}    /* dstAddr */
};

static TGF_PACKET_TCP_STC prvTgfPacketTcpIfaPart =
{
    30,                  /* src port */
    70,                  /* dst port */
    654321,             /* sequence number */
    333333,             /* acknowledgment number */
    5,                  /* data offset */
    0x00,               /* reserved */
    0x10,               /* flags */
    2048,               /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    0                   /* urgent pointer */
};

/* PARTS of IFA packet */
static TGF_PACKET_PART_STC prvTgfIpv6TcpIfaPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypeIpV6Part},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6TcpPart},
    {TGF_PACKET_PART_TCP_E,       &prvTgfPacketTcpIfaPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketIfaPayloadPart}
};


static TGF_PACKET_STC prvTgfIpv6TcpIfaPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV6_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS
     + sizeof(prvTgfPacketIfaPayloadDataArr)),                          /* totalLen */
    sizeof(prvTgfIpv6TcpIfaPacketArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfIpv6TcpIfaPacketArray                                         /* partsArray */
};


/* DATA of IFA packet on Egress Node */
static GT_U8 prvTgfEgressPacketIfaPayloadDataArr[] = {
    0xde, 0xad, 0xbe, 0xef, 0xa5, 0xa5, 0xde, 0xed, 0x10, 0x01,
    0x01, 0x10, 0x00, 0xff, 0x00, 0x01, 0x05, 0x02, 0x00, 0x00,
    0x05, 0xdc, 0x00, 0x40, 0xab, 0xcd, 0x00, 0x00, 0x00, 0x00,
    0x19, 0xb0, 0x10, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x60, 0x03, 0x31, 0xe9, 0xe7, 0x03, 0x31, 0xef, 0xb1,
    0x00, 0x00, 0x00, 0x77, 0x00, 0x41, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00,
    0x19, 0xaf, 0x10, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0c, 0xc3, 0x13, 0x37, 0xb3, 0xd1, 0x13, 0x37, 0xbf, 0x54,
    0x00, 0x00, 0x00, 0x77, 0x00, 0x41, 0x00, 0x00, 0x00, 0x08,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d
};

/* IFA PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfEgressPacketIfaPayloadPart = {
    sizeof(prvTgfEgressPacketIfaPayloadDataArr),                 /* dataLength */
    prvTgfEgressPacketIfaPayloadDataArr                          /* dataPtr */
};

/* PARTS of IFA packet */
static TGF_PACKET_PART_STC prvTgfEgressIpv4UdpIfaPacketArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4UdpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfEgressPacketIfaPayloadPart}
};

static TGF_PACKET_STC prvTgfEgressIpv4UdpIfaPacketInfo = {
    (TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS
     + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS
     + sizeof(prvTgfEgressPacketIfaPayloadDataArr)),                          /* totalLen */
    sizeof(prvTgfEgressIpv4UdpIfaPacketArray) / sizeof(TGF_PACKET_PART_STC),  /* numOfParts */
    prvTgfEgressIpv4UdpIfaPacketArray                                         /* partsArray */
};

/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    PRV_TGF_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdbsIfa;

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          interface;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC          interface1;
    GT_BOOL                                        samplingEn;
    GT_U32                                         samplingRatio;
    GT_BOOL                                        ttiLookupEnable;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_ENT       srcPortType;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT       srcPortInfo;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_ENT       trgPortType;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT       trgPortInfo;
    GT_U16                                         pvid;
    CPSS_INTERFACE_INFO_STC                        phyPortInfo;
    CPSS_INTERFACE_INFO_STC                        phyPortInfo1;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT   pclPortAccessModeCfgGet;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclTsAccessModeCfgGet;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT                  phaFwImageIdGet;
    GT_BOOL                                        ttiLookup;
    GT_BOOL                                        clientEnableGet;
    CPSS_DXCH_CNC_COUNTER_FORMAT_ENT               counterFormatGet;
    CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT     clientCountingModeGet;
    CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT              countingModeGet;
    GT_U32                                         queueGroupBaseGet;
    CPSS_DXCH_CNC_COUNTER_STC                      cncCounterGet;
    GT_U64                                         indexRangesGet;
    CPSS_INTERFACE_INFO_STC                        phyPortInfo2;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC               egressInfo2;
} prvTgfRestoreCfg;

static prvTgfPclUdbsIfa prvTgfIfaEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x03},   /* egress Mtag Cmd[0:1] */
     {4 , PRV_TGF_PCL_OFFSET_METADATA_E, 85, 0x03},   /* src phy port[8:9] */
     {5 , PRV_TGF_PCL_OFFSET_METADATA_E, 16, 0xFF},   /* src phy port[0:7] */
     {6 , PRV_TGF_PCL_OFFSET_METADATA_E, 82, 0xFF},   /* flow-id[0:7] */

     /* must be last */
     {7, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};

/**
* @internal prvTgfIngressNodeIfaBridgeConfigSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfIngressNodeIfaBridgeConfigSet()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryStore(1);

    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /*remove all, except ingress, remote ports from vlan 5*/
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS]);

    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.pvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
}

/**
* @internal prvTgfIngressNodeIfaMirrorPortConfigSet function
* @endinternal
*
* @brief   Ingress mirror port config
*           Enable source port mirroring for RX port and Set analyzer index to 1
*           map analyzer index to eport, and set sampling ratio
*           map eport to phy port#2 and enable TS for the eport
*
*/
static GT_VOID prvTgfIngressNodeIfaMirrorPortConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{

    GT_STATUS                             rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    CPSS_INTERFACE_INFO_STC               phyPortInfo;

    /* AUTODOC: Save analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum,
    (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_IFA_MIRROR_IDX_CNS : PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS,
    &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, eport = 1024 , phy port=2 */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = PRV_TGF_DEFAULT_EPORT_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet((nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_IFA_MIRROR_IDX_CNS : PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS,
                                           &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", PRV_TGF_IFA_MIRROR_IDX_CNS);

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS] :
                                                                               prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &phyPortInfo );

    /* AUTODOC: Enable EgressMirroredAsIngressMirrored on analyzer index 1 */
    rc = cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet(prvTgfDevNum,
                                                                     PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
         "[TGF]: cpssDxChMirrorTreatEgressMirroredAsIngressMirroredEnableSet FAILED , rc = [%d]", rc);

    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(prvTgfDevNum,
                                                              (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_IFA_MIRROR_IDX_CNS :
                                                                                                           PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS,
                                                              &prvTgfRestoreCfg.samplingEn, &prvTgfRestoreCfg.   samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet");
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum,
        (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_IFA_MIRROR_IDX_CNS : PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS,
        GT_TRUE, PRV_TGF_IFA_MIRROR_RATE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");
}

/**
* @internal prvTgfIngresssNodeIfaLoopbackPortConfigSet function
* @endinternal
*
* @brief   Loopback config
*
*/
static GT_VOID prvTgfIngresssNodeIfaLoopbackPortConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS] :
                                                                                   prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /*give port mgr time to config the LB*/
    cpssOsTimerWkAfter(10);

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
         CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet(prvTgfDevNum, 0x8100);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCscdCentralizedChassisRemotePhysicalPortVlanTagTpidSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

}

/**
* @internal prvTgfIngressNodeIfaPhaConfigSet function
*
* @endinternal
*
* @brief   PHA thread config
*
*/
static GT_VOID prvTgfIngressNodeIfaPhaConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{
    GT_STATUS                                rc;
    CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_UNT sourcePortInfo;
    CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_UNT targetPortInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;

    cpssOsMemSet(&sourcePortInfo, 0, sizeof(sourcePortInfo));
    cpssOsMemSet(&targetPortInfo, 0, sizeof(targetPortInfo));
    cpssOsMemSet(&ifaEntry,       0, sizeof(ifaEntry));

    /* AUTODOC: Assign PHA firmware image ID 02 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &(prvTgfRestoreCfg.phaFwImageIdGet));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaFwImageIdGet FAILED with rc = [%d]", rc);

    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", CPSS_DXCH_PHA_FW_IMAGE_ID_02_E, rc);

    /* AUTODOC: Set the thread entry */
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    extInfo.ifaHeader.flags                       = ifaHeaderPart.flags;
    extInfo.ifaHeader.telemetryRequestVector      = ifaHeaderPart.telemetryRequestVector;
    extInfo.ifaHeader.telemetryActionVector       = ifaHeaderPart.telemetryActionVector;
    extInfo.ifaHeader.probeMarker1                = ifaHeaderPart.probeMarker1;
    extInfo.ifaHeader.probeMarker2                = ifaHeaderPart.probeMarker2;

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
            PRV_TGF_PHA_THREAD_ID_IFA_INGRESS_HEADER_CNS,
            &commonInfo,
            CPSS_DXCH_PHA_THREAD_TYPE_IFA_INGRESS_HEADER_E,
            &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));

    extInfo.ifaEgressNodeMirror.ifaAnalyzerIndex = PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS;

    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
            PRV_TGF_PHA_THREAD_ID_IFA_EGRESS_MIRROR_CNS,
            &commonInfo,
            CPSS_DXCH_PHA_THREAD_TYPE_IFA_EGRESS_MIRROR_E,
            &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    extInfo.ifaMetadata.deviceId                  = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ?
                                                     ifaMetadataPart.deviceId + PRV_TGF_IFA_INGRESS_NODE_CNS:
                                                   ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ?
                                                     ifaMetadataPart.deviceId + PRV_TGF_IFA_TRANSIT_NODE_CNS :
                                                     ifaMetadataPart.deviceId + PRV_TGF_IFA_EGRESS_NODE_CNS);

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
            (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_PHA_THREAD_ID_IFA_INGRESS_METADATA_CNS :
            ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ? PRV_TGF_PHA_THREAD_ID_IFA_TRANSIT_METADATA_CNS :
            PRV_TGF_PHA_THREAD_ID_IFA_EGRESS_METADATA_CNS),
            &commonInfo,
            (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? CPSS_DXCH_PHA_THREAD_TYPE_IFA_INGRESS_METADATA_E :
            ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ? CPSS_DXCH_PHA_THREAD_TYPE_IFA_TRANSIT_METADATA_E :
            CPSS_DXCH_PHA_THREAD_TYPE_IFA_EGRESS_METADATA_E),
            &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

    rc = cpssDxChPhaSourcePortEntryGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                       &prvTgfRestoreCfg.srcPortType, &prvTgfRestoreCfg.srcPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntryGet");

    sourcePortInfo.ifaSourcePort.portId = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    rc = cpssDxChPhaSourcePortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                       CPSS_DXCH_PHA_SOURCE_PORT_ENTRY_TYPE_IFA_E,  &sourcePortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");

    rc = cpssDxChPhaTargetPortEntryGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                       &prvTgfRestoreCfg.trgPortType, &prvTgfRestoreCfg.trgPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntryGet");

    targetPortInfo.ifaTargetPort.portId = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = cpssDxChPhaTargetPortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                       CPSS_DXCH_PHA_TARGET_PORT_ENTRY_TYPE_IFA_E,  &targetPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");

    ifaEntry.maxLength = 1500;
    ifaEntry.senderHandle = 0xABCD;
    ifaEntry.hopLimit = 5;
    ifaEntry.ctrl = 0;
    ifaEntry.rep = CPSS_DXCH_PHA_IFA_REP_TYPE_NONE_E;
    ifaEntry.egressPort = nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS ? prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS] :
                                                                    prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = cpssDxChPhaSharedMemoryIfaEntrySet(prvTgfDevNum,
        (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_EPCL_INGRESS_NODE_IFA_FLOW_ID_CNS :
        ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ? PRV_TGF_EPCL_TRANSIT_NODE_IFA_FLOW_ID_CNS :
        PRV_TGF_EPCL_EGRESS_NODE_IFA_FLOW_ID_CNS),
        &ifaEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSharedMemoryIfaEntrySet");
}

/**
* @internal prvTgfIngressNodeIfaTtiConfigSet function
* @endinternal
*
* @brief IFA TTI Configuration
*/
static GT_VOID prvTgfIngressNodeIfaTtiConfigSet(GT_VOID)
{
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_STATUS                rc = GT_OK;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");
    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
                                &prvTgfRestoreCfg.ttiLookup);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet: %d", prvTgfDevNum);
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
     /* PCL ID */
    ttiPattern.udbArray.udb[0] = PRV_TGF_TTI_PCL_INDEX_CNS;
    ttiMask.udbArray.udb[0] = 0x1F;
    /* IPv4 Total Length */
    ttiPattern.udbArray.udb[1] = prvTgfPacketIpv4UdpPart.totalLen;
    ttiMask.udbArray.udb[1] = 0xFF;

    ttiAction.mirrorToIngressAnalyzerEnable    = GT_TRUE;
    ttiAction.mirrorToIngressAnalyzerIndex     = PRV_TGF_IFA_MIRROR_IDX_CNS;
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);


    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX_CNS, PRV_TGF_TTI_RULE_UDB_10_E,
                            &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, 0, PRV_TGF_TTI_OFFSET_METADATA_E, 22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, 1, PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 5);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, PRV_TGF_TTI_PCL_INDEX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
     /* PCL ID */
    ttiPattern.udbArray.udb[0] = PRV_TGF_TTI_PCL_INDEX_CNS;
    ttiMask.udbArray.udb[0] = 0x1F;
    /* IPv4 Total Length */
    ttiPattern.udbArray.udb[1] = prvTgfPacketIpv4UdpPart.totalLen + TGF_IFA_HEADER_SIZE_CNS + TGF_IFA_METADATA_SIZE_CNS;
    ttiMask.udbArray.udb[1] = 0xFF;

    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    ttiAction.flowId = PRV_TGF_TTI_PCL_INDEX_CNS;
    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX2_CNS, PRV_TGF_TTI_RULE_UDB_10_E,
                            &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

static GT_VOID prvTgfIngresssNodeIfaUdbConfigSet
(
)
{
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    GT_STATUS                           rc;

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if (prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress Mtag Cmd[1:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx] = prvTgfIfaEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_IPV6_TCP_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
}

static GT_VOID prvTgfIngresssNodeIfaEpcl1ConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U32                              srcPhyPort;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS] : prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS] :
                                                                                       prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(
                           (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS] :
                                                                        prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(
                           (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS] :
                                                                        prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    srcPhyPort = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[0] = (PRV_TGF_DEFAULT_EPORT_CNS&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[0]    = prvTgfIfaEpclMetadataUdbInfo[0].byteMask;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[1] = ((PRV_TGF_DEFAULT_EPORT_CNS>>7)&0x3f);
    mask.ruleEgrUdbOnly.udb[1]    = prvTgfIfaEpclMetadataUdbInfo[1].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[2] = (PRV_TGF_DEFAULT_EPORT_CNS>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[2]    = prvTgfIfaEpclMetadataUdbInfo[2].byteMask;

    /*egress pkt cmd = TO_ANALYZER */
    pattern.ruleEgrUdbOnly.udb[3] = 0x02;
    mask.ruleEgrUdbOnly.udb[3]    = prvTgfIfaEpclMetadataUdbInfo[3].byteMask;

    if(nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS)
    {
        /*src phy port [8:9] of the original packet*/
        pattern.ruleEgrUdbOnly.udb[4] = (srcPhyPort >> 8) & 0x3;
        mask.ruleEgrUdbOnly.udb[4] = prvTgfIfaEpclMetadataUdbInfo[4].byteMask;

        /*src phy port [0:7] of the original packet*/
        pattern.ruleEgrUdbOnly.udb[5] = srcPhyPort & 0xFF;
        mask.ruleEgrUdbOnly.udb[5] = prvTgfIfaEpclMetadataUdbInfo[5].byteMask;

        /*flow-id of the original packet*/
        pattern.ruleEgrUdbOnly.udb[6] = 0x0;
        mask.ruleEgrUdbOnly.udb[6] = 0x0;
    }

    ruleIndex                                            = PRV_TGF_EPCL_MATCH1_INDEX_CNS;
    ruleFormat                                           = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.egressPolicy                                  = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode         = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId                       = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_PHA_THREAD_ID_IFA_INGRESS_HEADER_CNS :
                                                                                                        PRV_TGF_PHA_THREAD_ID_IFA_EGRESS_METADATA_CNS;
    action.epclPhaInfo.phaThreadType                     = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_IFA_E;
    action.epclPhaInfo.phaThreadUnion.epclIfaFlow.flowId = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_EPCL_INGRESS_NODE_IFA_FLOW_ID_CNS :
                                                                                                        PRV_TGF_EPCL_EGRESS_NODE_IFA_FLOW_ID_CNS;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

static GT_VOID prvTgfIngresssNodeIfaEpcl2ConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U32                              srcPhyPort;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for receive port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on receive port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    srcPhyPort = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[0] = 0x0;
    mask.ruleEgrUdbOnly.udb[0]    = 0x0;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[1] = 0x0;
    mask.ruleEgrUdbOnly.udb[1]    = 0x0;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[2] = 0x0;
    mask.ruleEgrUdbOnly.udb[2]    = 0x0;

    /*egress pkt cmd = FORWARD */
    pattern.ruleEgrUdbOnly.udb[3] = 0x03;
    mask.ruleEgrUdbOnly.udb[3]    = prvTgfIfaEpclMetadataUdbInfo[3].byteMask;

    /*src phy port [8:9] of the original packet*/
    pattern.ruleEgrUdbOnly.udb[4] = (srcPhyPort >> 8) & 0x3;
    mask.ruleEgrUdbOnly.udb[4] = prvTgfIfaEpclMetadataUdbInfo[4].byteMask;

    /*src phy port [0:7] of the original packet*/
    pattern.ruleEgrUdbOnly.udb[5] = srcPhyPort & 0xFF;
    mask.ruleEgrUdbOnly.udb[5] = prvTgfIfaEpclMetadataUdbInfo[5].byteMask;

    if(nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS)
    {
        /* flow-id [0:7] of the original packet*/
        pattern.ruleEgrUdbOnly.udb[6] = PRV_TGF_TTI_PCL_INDEX_CNS & 0xFF;
        mask.ruleEgrUdbOnly.udb[6] = prvTgfIfaEpclMetadataUdbInfo[6].byteMask;
    }
    else
    {
        /* flow-id [0:7] of the original packet*/
        pattern.ruleEgrUdbOnly.udb[6] = 0x0;
        mask.ruleEgrUdbOnly.udb[6] = 0x0;
    }

    ruleIndex                                            = PRV_TGF_EPCL_MATCH2_INDEX_CNS;
    ruleFormat                                           = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.egressPolicy                                  = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode         = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId                       = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_PHA_THREAD_ID_IFA_INGRESS_METADATA_CNS :
                                                          ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ? PRV_TGF_PHA_THREAD_ID_IFA_TRANSIT_METADATA_CNS :
                                                                                                        PRV_TGF_PHA_THREAD_ID_IFA_EGRESS_MIRROR_CNS);
    action.epclPhaInfo.phaThreadType                     = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_IFA_E;
    action.epclPhaInfo.phaThreadUnion.epclIfaFlow.flowId = (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? PRV_TGF_EPCL_INGRESS_NODE_IFA_FLOW_ID_CNS :
                                                                                                        PRV_TGF_EPCL_TRANSIT_NODE_IFA_FLOW_ID_CNS;
    if(nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS)
    {
        action.pktCmd                                        = CPSS_PACKET_CMD_DROP_SOFT_E;
    }

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfIngresssNodeIfaCncConfigSet function
* @endinternal
*
* @brief   CNC configurations
*/
static GT_VOID prvTgfIngresssNodeIfaCncConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{
    GT_STATUS                      rc;
    GT_U64                         indexRangesBmp;
    GT_U32                         queueGroup;
    PRV_TGF_CNC_COUNTER_STC        counter;
    GT_PHYSICAL_PORT_NUM           portNum;

    indexRangesBmp.l[0] = 1;
    indexRangesBmp.l[1] = 0x0;

    /* clear counter */
    counter2.packetCount.l[0] = 0;
    counter2.packetCount.l[1] = 0;
    if(nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS)
    {
        counter2.byteCount.l[0] = 0x1;
        counter2.byteCount.l[1] = 0x1;
    }
    else
    {
        counter2.byteCount.l[0] = 0x10;
        counter2.byteCount.l[1] = 0x10;
    }
    counter.byteCount.l[0] = 0;
    counter.byteCount.l[1] = 0;
    counter.packetCount.l[0] = 0;
    counter.packetCount.l[1] = 0;

    /* enable CNC block#0 to CNC client */
    rc = cpssDxChCncBlockClientEnableGet(prvTgfDevNum, 0/*blockNum*/,
                                         CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E/*client*/, &prvTgfRestoreCfg.clientEnableGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncBlockClientEnableGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncBlockClientEnableSet(prvTgfDevNum, 0/*blockNum*/, CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E/*client*/, GT_TRUE/*enable*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncBlockClientEnableSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* set block index range */
    rc = cpssDxChCncBlockClientRangesGet(prvTgfDevNum, 0/*blockNum*/,
                                         CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E/*client*/, &prvTgfRestoreCfg.indexRangesGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncBlockClientRangesGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncBlockClientRangesSet(prvTgfDevNum, 0/*blockNum*/, CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E/*client*/, &indexRangesBmp);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncBlockClientRangesSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* set counter format in block */
    rc = cpssDxChCncCounterFormatGet(prvTgfDevNum, 0/*blockNum*/, &prvTgfRestoreCfg.counterFormatGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterFormatGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncCounterFormatSet(prvTgfDevNum, 0/*blockNum*/, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E/*format:PKT_0_BC_64*/);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterFormatSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* set client Reduced mode for Egress Queue client */
    rc = cpssDxChCncEgressQueueClientModeGet(prvTgfDevNum, &prvTgfRestoreCfg.clientCountingModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncEgressQueueClientModeGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncEgressQueueClientModeSet(prvTgfDevNum, CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_TAIL_DROP_REDUCED_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncEgressQueueClientModeSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* for CNC client set BYTE_COUNT_MODE_L2 mode */
    rc = cpssDxChCncClientByteCountModeGet(prvTgfDevNum, CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, &prvTgfRestoreCfg.countingModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncClientByteCountModeGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncClientByteCountModeSet(prvTgfDevNum, CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncClientByteCountModeSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* Set the Queue group base value for specific port and CNC client */
    if(nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS)
    {
        queueGroup = 0;
    }
    else if(nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS)
    {
        queueGroup = 8;
    }
    else
    {
        queueGroup = 9;
    }
    portNum = (nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS] : prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = cpssDxChCncPortQueueGroupBaseGet(prvTgfDevNum, portNum,
                                          CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, &prvTgfRestoreCfg.queueGroupBaseGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncPortQueueGroupBaseGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncPortQueueGroupBaseSet(prvTgfDevNum, portNum,
                                          CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, queueGroup);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncPortQueueGroupBaseSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* set the CNC counter clear by read globally configured value */
    rc = cpssDxChCncCounterClearByReadValueGet(prvTgfDevNum, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E, &prvTgfRestoreCfg.cncCounterGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterClearByReadValueGet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncCounterClearByReadValueSet(prvTgfDevNum, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E, &counter2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterClearByReadValueSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* set the CNC counter contents */
    counterIdx = queueGroup << 1 | 1;
    rc = prvTgfCncCounterSet(0, counterIdx, PRV_TGF_CNC_COUNTER_FORMAT_MODE_4_E, &counter);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncCounterSet prvTgfDevNum:%d\n",prvTgfDevNum);
}

/**
* @internal prvTgfIngresssNodeIfaConfigSet function
* @endinternal
*
* @brief IFA configurations
*/
static GT_VOID prvTgfIngresssNodeIfaConfigSet
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */
)
{
#ifndef ASIC_SIMULATION
    GT_STATUS                  rc;
    CPSS_DXCH_IFA_PORT_CFG_STC ifaPortCfg;
    CPSS_DXCH_IFA_PARAMS_STC   ifaGlobalCfg;

    cpssOsMemSet(&ifaGlobalCfg, 0, sizeof(ifaGlobalCfg));
    cpssOsMemSet(&ifaPortCfg,   0, sizeof(ifaPortCfg));

    rc = cpssDxChTamInit(prvTgfDevNum, 16);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChTamInit prvTgfDevNum:%d\n",prvTgfDevNum);

    ifaGlobalCfg.egressPortDropCountingMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_BYTE_BASED_DROP_COUNTING_MODE_E;
    ifaGlobalCfg.egressPortDropByteMode     = CPSS_DXCH_TAM_IFA_EGRESS_PORT_L2_BASED_DROP_BYTE_MODE_E;
    ifaGlobalCfg.samplePeriod               = 5;
    rc = cpssDxChIfaGlobalConfigSet(prvTgfDevNum, &ifaGlobalCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChIfaGlobalConfigSet prvTgfDevNum:%d\n",prvTgfDevNum);

    ifaPortCfg.portNum         = (nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS) ? prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS] :
                                                                             prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    ifaPortCfg.speed           = CPSS_PORT_SPEED_50000_E;
    ifaPortCfg.cncBlockNum     = 0;
    ifaPortCfg.cncBaseEntryNum = counterIdx;
    ifaPortCfg.numOfCncCounter = 8;
    rc = cpssDxChIfaPortConfigSet(prvTgfDevNum, 1, &ifaPortCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChIfaPortConfigSet prvTgfDevNum:%d\n",prvTgfDevNum);

    rc = cpssDxChIfaEnableSet(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChIfaEnableSet prvTgfDevNum:%d\n",prvTgfDevNum);
#endif
}

/**
* @internal prvTgfIngressNodeIfaConfigurationSet function
* @endinternal
*
* @brief  IFA ingress node configuration settings
*/
void prvTgfIngressNodeIfaConfigurationSet()
{

    /* Bridge config */
    prvTgfIngressNodeIfaBridgeConfigSet();

    /* TTI config */
    prvTgfIngressNodeIfaTtiConfigSet();

    /* Mirror port config */
    prvTgfIngressNodeIfaMirrorPortConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);

    /* Loopback port config */
    prvTgfIngresssNodeIfaLoopbackPortConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);

    /* CNC config for IFA */
    prvTgfIngresssNodeIfaCncConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);

    /* EPCL UDB config */
    prvTgfIngresssNodeIfaUdbConfigSet();

    /* Loopback port EPCL config */
    prvTgfIngresssNodeIfaEpcl1ConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);

    /* Receive port EPCL config */
    prvTgfIngresssNodeIfaEpcl2ConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);

    /* PHA config */
    prvTgfIngressNodeIfaPhaConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);

    /* IFA config */
    prvTgfIngresssNodeIfaConfigSet(PRV_TGF_IFA_INGRESS_NODE_CNS);
}

/**
* internal prvTgfIngressNodeIfaPacketValidate function
* @endinternal
* @brief   IFA Header and Metadata use case verification
       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         Probe Marker (1)                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         Probe Marker (2)                      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  Ver  |Rep|C|E|   Ctrl  |MType|  RSVD |  TID  |     Flag      |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   Telemetry Request Vector    |   Telemetry Action  Vector    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |   Hop Limit   |   Hop Count   |         Must Be Zero          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |         Maximum Length        |        Current Length         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |        Sender's Handle        |        Sequence Number        |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

                       IFA Header Format

       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                         Device ID.                            |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | TID=1 |  CN   | Eg Port Drop U|     TTL       |   Queue ID    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                        Rx Time Stamp U.                       |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |      Rx Time Stamp L.         |   Rx Time Stamp ns U.         |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |      Rx Time Stamp ns L.      |        Tx Time Stamp ns U.    |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |      Tx Time Stamp ns L.      |      Egr Port Utilization     |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |       Ingress Port ID.        |      Egress Port ID.          |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |                    Eg Port Drop L                             |
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

               IFA Metadata Format when TID = 1

*/
GT_VOID prvTgfIngressNodeIfaPacketValidate
(
    GT_U32 nodeType /* 0:Ingress, 1:Transit, 2:Egress */,
    GT_U32 inPacketType /* 0- IPv4 UDP, 1 - IPv6 TCP */
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U8                               packetBuf[PRV_TGF_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetLen = PRV_TGF_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0, ii;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              pktParse = 0;
    GT_U32                              pktParseTunnel = 0;
    GT_U32                              pktCount;
    GT_U32                              probe1, probe2;
    GT_U32                              telemetryRequest, telemetryAction;
    GT_U32                              deviceId;
    GT_U16                              etherType;
    GT_U8                               byteValue;
    GT_U16                              l4Port;

    for(pktCount = 0; pktCount < 2; pktCount++)
    {
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egressPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet: portInterface %d", prvTgfDevNum);
        getFirst = GT_FALSE;
        packetLen = PRV_TGF_PACKET_SIZE_CNS;

        if(nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS)
        {
            /* Tunnel MAC DA Verification */
            for (ii = 0; ii < 6; ii++)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfMirroredPacketTunnelL2Part.daMac[ii],
                        packetBuf[pktParse],
                        "Tunnel Packet DA is not matching at index[%d]", ii);
                pktParse++;
            }

            pktParse = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_TUNNEL_VLANID_CNS,
                                         (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)) & 0xFFF,
                                         "Tunnel packet Vlan-ID is not matching expected %d received %d",
                                          PRV_TGF_TUNNEL_VLANID_CNS,
                                          (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)) & 0xFFF);

            pktParse = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
            UTF_VERIFY_EQUAL2_STRING_MAC(TGF_ETHERTYPE_0800_IPV4_TAG_CNS,
                                         (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                         "Tunnel packet Ether Type is not matching expected %d received %d",
                                         TGF_ETHERTYPE_0800_IPV4_TAG_CNS,
                                         (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)));

            pktParse = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + PRV_TGF_TTL_OFFSET_CNS;
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_TUNNEL_TTL_CNS,
                                         packetBuf[pktParse],
                                         "Tunnel packet IPv4 TTL is not matching expected %d received %d",
                                         PRV_TGF_TUNNEL_TTL_CNS,
                                         (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)));

            pktParse = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS;

            /* UDP Source Port Verification */
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_TUNNEL_IFA_UDP_SRC_PORT_CNS,
                                         (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                         "Tunnel Packet UDP Source Port is not matching expected %d received %d",
                                         PRV_TGF_TUNNEL_IFA_UDP_SRC_PORT_CNS,
                                         (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)));

            /* UDP Destination Port Verification */
            pktParse = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + 2;
            UTF_VERIFY_EQUAL2_STRING_MAC(PRV_TGF_TUNNEL_IFA_UDP_DST_PORT_CNS,
                                         (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                         "Tunnel packet UDP Destination Port is not matching expected %d received %d",
                                          PRV_TGF_TUNNEL_IFA_UDP_DST_PORT_CNS,
                                          (packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)));

            pktParse = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS;
            pktParseTunnel = pktParse;
        }

        /* MAC DA Verification */
        for (ii = 0; ii < 6; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPacketL2Part.daMac[ii],
                    packetBuf[pktParse],
                    "Packet DA is not matching at index[%d]", ii);
            pktParse++;
        }

        /* MAC SA Verification */
        pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS / 2;
        for (ii = 0; ii < 5; ii++)
        {
            UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPacketL2Part.saMac[ii],
                    packetBuf[pktParse],
                    "Packet SA is not matching at index[%d]", ii);
            pktParse++;
        }

        etherType = (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ?
                     TGF_ETHERTYPE_0800_IPV4_TAG_CNS : TGF_ETHERTYPE_86DD_IPV6_TAG_CNS;
        /* Ether Type Verification */
        pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS;
        UTF_VERIFY_EQUAL0_STRING_MAC(etherType,
                                     (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                     "IPv4 Ether Type is not matching");

        pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;

        /* TTL/HOP LIMIT Verification */
        pktParse += ((inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ?
                     PRV_TGF_IPV4_TTL_OFFSET_CNS : PRV_TGF_IPV6_HOP_LIMIT_OFFSET_CNS);
        byteValue = (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ?
                     prvTgfPacketIpv4UdpPart.timeToLive : prvTgfPacketIpv6TcpPart.hopLimit;
        UTF_VERIFY_EQUAL1_STRING_MAC(byteValue,
                                     packetBuf[pktParse],
                                     "TTL/HOP LIMIT is not matching for packet count %d", pktCount);

        /* SIP Verification */
        pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;

        if (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS)
        {
            pktParse += TGF_IPV4_HEADER_SIZE_CNS - 8;
            for (ii = 0; ii < 4; ii++)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPacketIpv4UdpPart.srcAddr[ii], packetBuf[pktParse],
                        "IPv4 SIP is not matching at index[%d]", ii);
                pktParse++;
            }
        }
        else
        {
            pktParse += 8;
            for (ii = 0; ii < 8; ii++)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPacketIpv6TcpPart.srcAddr[ii],
                                             ((packetBuf[pktParse] << 8) | packetBuf[pktParse+1]) ,
                        "IPv6 SIP is not matching at index[%d]", ii);
                pktParse += 2;
            }
        }

        /* DIP Verification */
        pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;

        if (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS)
        {
            pktParse += TGF_IPV4_HEADER_SIZE_CNS - 4;
            for (ii = 0; ii < 4; ii++)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPacketIpv4UdpPart.dstAddr[ii], packetBuf[pktParse],
                        "IPv4 DIP is not matching at index[%d]", ii);
                pktParse++;
            }
        }
        else
        {
            pktParse += (8 + 16);
            for (ii = 0; ii < 8; ii++)
            {
                UTF_VERIFY_EQUAL1_STRING_MAC(prvTgfPacketIpv6TcpPart.dstAddr[ii],
                                             ((packetBuf[pktParse] << 8) | packetBuf[pktParse+1]),
                        "IPv6 DIP is not matching at index[%d]", ii);
                pktParse += 2;
            }
        }

        /* UDP/TCP Source Port Verification */
        pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
        pktParse += (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ? TGF_IPV4_HEADER_SIZE_CNS : TGF_IPV6_HEADER_SIZE_CNS;
        l4Port = (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ? prvTgfPacketUdpPart.srcPort : prvTgfPacketTcpIfaPart.srcPort;
        UTF_VERIFY_EQUAL1_STRING_MAC(l4Port,
                                     (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                     "UDP Source Port is not matching for packet count %d", pktCount);

        /* UDP/TCP Destination Port Verification */
        pktParse += 2;
        l4Port = (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ? prvTgfPacketUdpPart.dstPort : prvTgfPacketTcpIfaPart.dstPort;
        UTF_VERIFY_EQUAL1_STRING_MAC(l4Port,
                                     (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                     "UDP Destination Port is not matching for packet count %d", pktCount);

        /* IFA Packet */
        if(pktCount == 1 || nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS)
        {
            pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
            pktParse += (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS) ? (TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS) :
                                              (TGF_IPV6_HEADER_SIZE_CNS + TGF_TCP_HEADER_SIZE_CNS);
            probe1 = packetBuf[pktParse + 3] | (packetBuf[pktParse + 2] << 8) | (packetBuf[pktParse + 1]<<16) | (packetBuf[pktParse] << 24);
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaHeaderPart.probeMarker1, probe1,
            "Probe Marker 1 is not matching received 0x%x actual 0x%x", probe1, ifaHeaderPart.probeMarker1);

            pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS +
                       TGF_UDP_HEADER_SIZE_CNS + PRV_TGF_IFA_HEADER_PROBE1_SIZE_CNS;
            probe2 = packetBuf[pktParse + 3] | (packetBuf[pktParse + 2] << 8) | (packetBuf[pktParse + 1]<<16) | (packetBuf[pktParse] << 24);
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaHeaderPart.probeMarker2, probe2,
            "Probe Marker 2 is not matching received 0x%x actual 0x%x", probe2, ifaHeaderPart.probeMarker2);

            pktParse = pktParseTunnel + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS +
                       TGF_UDP_HEADER_SIZE_CNS + PRV_TGF_IFA_HEADER_PROBE1_SIZE_CNS + PRV_TGF_IFA_HEADER_PROBE2_SIZE_CNS;

            UTF_VERIFY_EQUAL2_STRING_MAC(0x1, (packetBuf[pktParse] >> 4) & 0xF,
                                         "IFA Header version is not matching received 0x%x actual 0x%x",(packetBuf[pktParse] >> 4) & 0xF, 0x1);
            pktParse++;
            UTF_VERIFY_EQUAL2_STRING_MAC(0x1, packetBuf[pktParse] & 0x7,
                                         "IFA Header Mtype is not matching received 0x%x actual 0x%x",packetBuf[pktParse] & 0x7, 0x1);
            pktParse++;
            UTF_VERIFY_EQUAL2_STRING_MAC(0x1, packetBuf[pktParse] & 0xF,
                                         "IFA Header TID is not matching received 0x%x actual 0x%x",packetBuf[pktParse] & 0xF, 0x1);
            pktParse++;
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaHeaderPart.flags, packetBuf[pktParse],
                                         "IFA Header Flags is not matching received 0x%x actual 0x%x",packetBuf[pktParse] & 0xF, ifaHeaderPart.flags);
            pktParse++;
            telemetryRequest = (packetBuf[pktParse + 1]) | (packetBuf[pktParse] << 8);
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaHeaderPart.telemetryRequestVector, telemetryRequest,
                                         "IFA Header Telemetry Request is not matching received 0x%x actual 0x%x",
                                          telemetryRequest, ifaHeaderPart.telemetryRequestVector);

            pktParse += PRV_TGF_IFA_HEADER_TELEMETRY_REQUEST_SIZE_CNS;
            telemetryAction = (packetBuf[pktParse + 1]) | (packetBuf[pktParse] << 8);
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaHeaderPart.telemetryActionVector, telemetryAction,
                                         "IFA Header Telemetry Action is not matching received 0x%x actual 0x%x",
                                          telemetryAction, ifaHeaderPart.telemetryActionVector);

            pktParse += PRV_TGF_IFA_HEADER_TELEMETRY_ACTION_SIZE_CNS;
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaEntry.hopLimit, packetBuf[pktParse],
                                         "IFA Header Hop Limit is not matching received 0x%x actual 0x%x",packetBuf[pktParse], ifaEntry.hopLimit);

            pktParse++;
            UTF_VERIFY_EQUAL2_STRING_MAC(nodeType+1, packetBuf[pktParse],
                                         "IFA Header Hop Count is not matching received 0x%x actual 0x%x",packetBuf[pktParse], nodeType+1);

            pktParse+=3;
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaEntry.maxLength, packetBuf[pktParse + 1] | packetBuf[pktParse] << 8,
                                         "IFA Header Max Length is not matching received 0x%x actual 0x%x",
                                         packetBuf[pktParse + 1] | packetBuf[pktParse] << 8, ifaEntry.maxLength);
            pktParse+=2;
            UTF_VERIFY_EQUAL2_STRING_MAC(TGF_IFA_METADATA_SIZE_CNS*(nodeType+1), packetBuf[pktParse + 1] | packetBuf[pktParse] << 8,
                                         "IFA Header Current Length is not matching received 0x%x actual 0x%x",
                                         packetBuf[pktParse + 1] | packetBuf[pktParse] << 8, TGF_IFA_METADATA_SIZE_CNS*(nodeType+1));
            pktParse+=2;
            UTF_VERIFY_EQUAL2_STRING_MAC(ifaEntry.senderHandle, packetBuf[pktParse + 1] | packetBuf[pktParse] << 8,
                                         "IFA Header Current Length is not matching received 0x%x actual 0x%x",
                                         packetBuf[pktParse], ifaEntry.senderHandle);
            pktParse+=4;
            deviceId = packetBuf[pktParse + 3] | (packetBuf[pktParse + 2] << 8) | (packetBuf[pktParse + 1]<<16) | (packetBuf[pktParse] << 24);
            UTF_VERIFY_EQUAL2_STRING_MAC((nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? ifaMetadataPart.deviceId + PRV_TGF_IFA_INGRESS_NODE_CNS :
                                         ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ? ifaMetadataPart.deviceId + PRV_TGF_IFA_TRANSIT_NODE_CNS :
                                                                                       ifaMetadataPart.deviceId + PRV_TGF_IFA_EGRESS_NODE_CNS),
                                         deviceId,
            "IFA Metadata DeviceId is not matching received 0x%x actual 0x%x", deviceId,
            (nodeType == PRV_TGF_IFA_INGRESS_NODE_CNS) ? ifaMetadataPart.deviceId + PRV_TGF_IFA_INGRESS_NODE_CNS :
            ((nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS) ? ifaMetadataPart.deviceId + PRV_TGF_IFA_TRANSIT_NODE_CNS :
                                                         ifaMetadataPart.deviceId + PRV_TGF_IFA_EGRESS_NODE_CNS));

            pktParse+=4;
            UTF_VERIFY_EQUAL2_STRING_MAC(0x1, (packetBuf[pktParse] >> 4) & 0xF,
                                         "IFA Metadata TID is not matching received 0x%x actual 0x%x",(packetBuf[pktParse] >> 4) & 0xF, 0x1);
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketIpv4UdpPart.typeOfService, packetBuf[pktParse] & 0xF,
                                         "IFA Metadata CN is not matching received 0x%x actual 0x%x",packetBuf[pktParse] & 0xF,
                                         prvTgfPacketIpv4UdpPart.typeOfService);
            pktParse+=1;
#ifndef ASIC_SIMULATION
            UTF_VERIFY_EQUAL2_STRING_MAC(counter2.byteCount.l[1]*8/*PGs*/, packetBuf[pktParse],
            "IFA Metadata CNC Counter 1MSB is not matching received 0x%x actual 0x%x",counter2.byteCount.l[1]*8, packetBuf[pktParse]);
#endif

            pktParse+=1;
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPacketIpv4UdpPart.timeToLive,
                                         packetBuf[pktParse],
                                         "IFA Metadata TTL is not matching received 0x%x actual 0x%x",
                                         packetBuf[pktParse], prvTgfPacketIpv4UdpPart.timeToLive);
            pktParse+=18;
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                         (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                         "IFA Metadata Ingress Port is not matching received 0x%x actual 0x%x",
                                         (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)), prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
            pktParse+=2;
            UTF_VERIFY_EQUAL2_STRING_MAC(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                         (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)),
                                         "IFA Metadata Egress Port is not matching received 0x%x actual 0x%x",
                                         (GT_U16)(packetBuf[pktParse + 1] | (packetBuf[pktParse] << 8)), prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS]);
            pktParse+=2;
#ifndef ASIC_SIMULATION
            UTF_VERIFY_EQUAL2_STRING_MAC(counter2.byteCount.l[0]*8/*PGs*/,
            packetBuf[pktParse + 3] | (packetBuf[pktParse + 2] << 8) | (packetBuf[pktParse + 1]<<16) | (packetBuf[pktParse] << 24),
            "IFA Metadata CNC Counter 4LSB is not matching received 0x%x actual 0x%x",
            packetBuf[pktParse + 3] | (packetBuf[pktParse + 2] << 8) | (packetBuf[pktParse + 1]<<16) | (packetBuf[pktParse] << 24),
            counter2.byteCount.l[0]*8);
#endif
        }
        pktParse = 0;
        if(nodeType == PRV_TGF_IFA_TRANSIT_NODE_CNS)
        {
            if (inPacketType == PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS)
            {
                /* restore the IPv4 total length */
                prvTgfPacketIpv4UdpPart.totalLen -= (TGF_IFA_HEADER_SIZE_CNS + TGF_IFA_METADATA_SIZE_CNS);
            }
            break;
        }
        if(nodeType == PRV_TGF_IFA_EGRESS_NODE_CNS)
        {
            /* restore the IPv4 total length */
            prvTgfPacketIpv4UdpPart.totalLen -= (TGF_IFA_HEADER_SIZE_CNS + PRV_TGF_IFA_EGRESS_NODE_CNS*TGF_IFA_METADATA_SIZE_CNS);
            break;
        }
    }
    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfIngressNodeIfaTrafficSend function
* @endinternal
*
* @brief  Traffic test for IFA ingress node
*/
void prvTgfIngressNodeIfaTrafficSend()
{
    GT_STATUS                rc                                 = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Enable capture on receive port */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4UdpPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    cpssOsTimerWkAfter(1000);
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4UdpPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /*verify the IFA packet*/
    prvTgfIngressNodeIfaPacketValidate(PRV_TGF_IFA_INGRESS_NODE_CNS,PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS);
}

/**
* @internal prvTgfIngressNodeIfaBridgeConfigReset function
* @endinternal
*
* @brief  Bridge config restore
*/
static void prvTgfIngressNodeIfaBridgeConfigReset()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryRestore(1);
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: port %d, vlan %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
}

/**
* @internal prvTgfIngressNodeIfaMirrorPortConfigReset function
* @endinternal
*
* @brief   Ingress mirror config restore
*
*/
static GT_VOID prvTgfIngressNodeIfaMirrorPortConfigReset
(
)
{

    GT_STATUS                           rc;

    /* AUTODOC: Restore analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_IFA_MIRROR_IDX_CNS,
                                           &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", prvTgfDevNum);
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS,
                                           &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", prvTgfDevNum);

    /* AUTODOC: Restore ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /* AUTODOC: Restore ingress statistical mirroring configuration */
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_IFA_MIRROR_IDX_CNS,
                                                              prvTgfRestoreCfg.samplingEn, prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_IFA_EGRESS_MIRROR_IDX_CNS,
                                                              prvTgfRestoreCfg.samplingEn, prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

}

/**
* @internal prvTgfIngressNodeIfaEpclConfigRestore function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfIngressNodeIfaEpclConfigRestore()
{
    GT_STATUS rc;

    /* AUTODOC: Invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_EPCL_MATCH1_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_EPCL_MATCH2_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Loopback port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: disable EPCL on Rx port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: disable EPCL on Rx port2 per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: disable ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Restore "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");
}

/**
* @internal prvTgfIngressNodeIfaPhaConfigRestore function
* @endinternal
*
* @brief PHA test configurations restore
*/
static GT_VOID prvTgfIngressNodeIfaPhaConfigRestore()
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: Restore PHA Configuration */
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_FALSE, prvTgfRestoreCfg.phaFwImageIdGet);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", prvTgfRestoreCfg.phaFwImageIdGet, rc);

    rc = cpssDxChPhaSourcePortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS],
                                       prvTgfRestoreCfg.srcPortType, &prvTgfRestoreCfg.srcPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");

    rc = cpssDxChPhaTargetPortEntrySet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                       prvTgfRestoreCfg.trgPortType, &prvTgfRestoreCfg.trgPortInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR in cpssDxChPhaSourcePortEntrySet");
}

/**
* @internal prvTgfIngressNodeIfaTtiConfigReset function
* @endinternal
*
* @brief TTI test configurations restore
*/
static GT_VOID prvTgfIngressNodeIfaTtiConfigReset()
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: Restore TTI Configuration */
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, prvTgfRestoreCfg.ttiLookup);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS], PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, prvTgfRestoreCfg.ttiLookup);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX2_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI_INDEX3_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);
}

/**
* @internal prvTgfIngressNodeIfaTamConfigRestore function
* @endinternal
*
* @brief  Restore TAM/IFA ingress node configurations
*/
void prvTgfIngressNodeIfaTamConfigRestore()
{
#ifndef ASIC_SIMULATION
    GT_STATUS rc = GT_OK;

    rc = cpssDxChIfaEnableSet(prvTgfDevNum, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChIfaEnableSet prvTgfDevNum:%d\n",prvTgfDevNum);

    rc = cpssDxChTamDelete(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChTamDelete prvTgfDevNum:%d\n",prvTgfDevNum);
#endif
}

/**
* @internal prvTgfIngressNodeIfaCncConfigRestore function
* @endinternal
*
* @brief  Restore CNC ingress node configurations
*/
void prvTgfIngressNodeIfaCncConfigRestore()
{
    GT_STATUS                      rc = GT_OK;

    /* disable CNC block#0 to CNC client */
    rc = cpssDxChCncBlockClientEnableSet(prvTgfDevNum, 0/*blockNum*/,
                                         CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E/*client*/, prvTgfRestoreCfg.clientEnableGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncBlockClientEnableSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* restore block index range */
    rc = cpssDxChCncBlockClientRangesSet(prvTgfDevNum, 0/*blockNum*/,
                                         CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E/*client*/, &prvTgfRestoreCfg.indexRangesGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncBlockClientRangesSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* restore counter format in block */
    rc = cpssDxChCncCounterFormatSet(prvTgfDevNum, 0/*blockNum*/, prvTgfRestoreCfg.counterFormatGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterFormatSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* restore client Reduced mode for Egress Queue client */
    rc = cpssDxChCncEgressQueueClientModeSet(prvTgfDevNum, prvTgfRestoreCfg.clientCountingModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncEgressQueueClientModeSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* restore CNC client mode */
    rc = cpssDxChCncClientByteCountModeSet(prvTgfDevNum, CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, prvTgfRestoreCfg.countingModeGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncClientByteCountModeSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* restore the Queue group base value for specific port and CNC client */
    rc = cpssDxChCncPortQueueGroupBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, prvTgfRestoreCfg.queueGroupBaseGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncPortQueueGroupBaseSet prvTgfDevNum:%d\n",prvTgfDevNum);
    rc = cpssDxChCncPortQueueGroupBaseSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                          CPSS_DXCH_CNC_CLIENT_EGRESS_QUEUE_PASS_DROP_E, prvTgfRestoreCfg.queueGroupBaseGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncPortQueueGroupBaseSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* restore the CNC counter clear by read globally configured value */
    rc = cpssDxChCncCounterClearByReadValueSet(prvTgfDevNum, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E, &prvTgfRestoreCfg.cncCounterGet);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterClearByReadValueSet prvTgfDevNum:%d\n",prvTgfDevNum);

    /* read the CNC counters to clear them */
    rc = cpssDxChCncCounterGet(prvTgfDevNum, 0, counterIdx, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_4_E, &counter2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in cpssDxChCncCounterGet prvTgfDevNum:%d\n",prvTgfDevNum);

}

/**
* @internal prvTgfIngressNodeIfaLoopbackConfigReset function
* @endinternal
*
* @brief  Restore loopback configurations
*/
void prvTgfIngressNodeIfaLoopbackConfigReset()
{
    GT_STATUS                      rc = GT_OK;
    CPSS_INTERFACE_INFO_STC        portInterface;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* reset loopback mode on port */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",
                                 prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    /* reset loopback mode on port */
    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS];

    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",
                                 prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

}

/**
* @internal prvTgfIngressNodeIfaConfigurationRestore function
* @endinternal
*
* @brief  Restore IFA ingress node configurations
*/
void prvTgfIngressNodeIfaConfigurationRestore()
{
    /* restore Bridge config */
    prvTgfIngressNodeIfaBridgeConfigReset();

    /* restore TTI config */
    prvTgfIngressNodeIfaTtiConfigReset();

    /* restore Mirror port config */
    prvTgfIngressNodeIfaMirrorPortConfigReset();

    /* restore loopback config */
    prvTgfIngressNodeIfaLoopbackConfigReset();

    /* restore EPCL configuration */
    prvTgfIngressNodeIfaEpclConfigRestore();

    /* restore PHA config */
    prvTgfIngressNodeIfaPhaConfigRestore();

    /* restore TAM/IFA config */
    prvTgfIngressNodeIfaTamConfigRestore();

    /* restore CNC config */
    prvTgfIngressNodeIfaCncConfigRestore();

}


/**
* @internal prvTgfTransitNodeIfaConfigurationSet function
* @endinternal
*
* @brief  IFA transit node configuration settings
*/
void prvTgfTransitNodeIfaConfigurationSet()
{

    /* Bridge config */
    prvTgfIngressNodeIfaBridgeConfigSet();

    /* CNC config for IFA */
    prvTgfIngresssNodeIfaCncConfigSet(PRV_TGF_IFA_TRANSIT_NODE_CNS);

    /* EPCL UDB config */
    prvTgfIngresssNodeIfaUdbConfigSet();

    /* Receive port EPCL config */
    prvTgfIngresssNodeIfaEpcl2ConfigSet(PRV_TGF_IFA_TRANSIT_NODE_CNS);

    /* PHA config */
    prvTgfIngressNodeIfaPhaConfigSet(PRV_TGF_IFA_TRANSIT_NODE_CNS);

    /* IFA config */
    prvTgfIngresssNodeIfaConfigSet(PRV_TGF_IFA_TRANSIT_NODE_CNS);
}

/**
* @internal prvTgfTransitNodeIfaTrafficSend function
* @endinternal
*
* @brief  Traffic test for IFA transit node
*/
void prvTgfTransitNodeIfaTrafficSend()
{
    GT_STATUS                rc                                 = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Enable capture on receive port */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    prvTgfPacketIpv4UdpPart.totalLen += TGF_IFA_HEADER_SIZE_CNS + TGF_IFA_METADATA_SIZE_CNS;
    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4UdpIfaPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    cpssOsTimerWkAfter(10);
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv4UdpIfaPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /*verify the IFA packet*/
    prvTgfIngressNodeIfaPacketValidate(PRV_TGF_IFA_TRANSIT_NODE_CNS, PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS);

    /* IPv6 TCP packet check */

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Enable capture on receive port */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfIpv6TcpIfaPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /*verify the IFA packet*/
    prvTgfIngressNodeIfaPacketValidate(PRV_TGF_IFA_TRANSIT_NODE_CNS, PRV_TGF_IFA_PACKET_TYPE_IPv6_TCP_CNS);

}

/**
* @internal prvTgfTransitNodeIfaConfigurationRestore function
* @endinternal
*
* @brief  Restore IFA transit node configurations
*/
void prvTgfTransitNodeIfaConfigurationRestore()
{
    /* restore Bridge config */
    prvTgfIngressNodeIfaBridgeConfigReset();

    /* restore EPCL configuration */
    prvTgfIngressNodeIfaEpclConfigRestore();

    /* restore PHA config */
    prvTgfIngressNodeIfaPhaConfigRestore();

    /* restore TAM/IFA config */
    prvTgfIngressNodeIfaTamConfigRestore();

    /* restore CNC config */
    prvTgfIngressNodeIfaCncConfigRestore();
}

/**
* @internal prvTgfIngressNodeIfaTtiConfigSet function
* @endinternal
*
* @brief IFA TTI Configuration
*/
static GT_VOID prvTgfLoopbackNodeIfaTtiConfigSet(GT_VOID)
{
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;
    GT_STATUS                rc = GT_OK;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");
    /* AUTODOC: enable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    prvTgfTtiTcamIndexIsRelativeSet(GT_TRUE);
    rc = prvTgfTtiPortLookupEnableGet(prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
                                &prvTgfRestoreCfg.ttiLookup);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableGet: %d", prvTgfDevNum);
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_RX_PORT2_IDX_CNS],
                                PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
                                GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum,
                            PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E,
                            PRV_TGF_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, 0, PRV_TGF_TTI_OFFSET_METADATA_E, 22);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, 1, PRV_TGF_TTI_OFFSET_L3_MINUS_2_E, 5);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiPclIdSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV4_UDP_E, PRV_TGF_TTI_PCL_INDEX_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPclIdSet: %d", prvTgfDevNum);

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
     /* PCL ID */
    ttiPattern.udbArray.udb[0] = PRV_TGF_TTI_PCL_INDEX_CNS;
    ttiMask.udbArray.udb[0] = 0x1F;
    /* IPv4 Total Length */
    ttiPattern.udbArray.udb[1] = sizeof(prvTgfEgressPacketIfaPayloadDataArr) + TGF_IFA_METADATA_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS;
    ttiMask.udbArray.udb[1] = 0xFF;

    ttiAction.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    ttiAction.egressInterface.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.egressInterface.devPort.portNum   = PRV_TGF_PASS2_EPORT_CNS;
    ttiAction.flowId                            = PRV_TGF_TTI_PCL_INDEX_CNS;
    ttiAction.bridgeBypass                      = GT_TRUE;
    ttiAction.ingressPipeBypass                 = GT_TRUE;
    ttiAction.redirectCommand                   = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.tag0VlanPrecedence                = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    ttiAction.tag0VlanCmd                       = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    ttiAction.tag0VlanId                        = PRV_TGF_VLANID_CNS;

    rc = prvTgfTtiRuleUdbSet(PRV_TGF_TTI_INDEX3_CNS, PRV_TGF_TTI_RULE_UDB_10_E,
                            &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal prvTgfEgressNodeIfaMirrorPortConfigSet function
* @endinternal
*
*/
static GT_VOID prvTgfEgressNodeIfaMirrorPortConfigSet()
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC phyPortInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC     egressInfo;
    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_PASS2_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo2));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS, &phyPortInfo );
    /* AUTODOC: Set analyzer ePort#1 attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 10, tsPassenger = ETHERNET */
    /*Get eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS,
                                                       &prvTgfRestoreCfg.egressInfo2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo2;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS,
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfEgressNodeIfaTsConfigSet function
* @endinternal
*
* @brief   TS configurations for IPv4 tunnel
*           Set Tunnel Start entry 10 with:
*           tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=5, ipHeaderProtocol = UDP, udp port = 0x1289/0xABEF
*           MACDA = 00:00:00:00:44:04 , DIP = 20.1.1.2, SIP=20.1.1.3
*           Tunnel start profile with 16B zeros
*
*/
static GT_VOID prvTgfEgressNodeIfaTsConfigSet()
{
    GT_STATUS rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_TUNNEL_VLANID_CNS;
    tunnelEntry.ipv4Cfg.ttl                    = PRV_TGF_TUNNEL_TTL_CNS;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv4Cfg.udpDstPort             = PRV_TGF_TUNNEL_IFA_UDP_DST_PORT_CNS;
    tunnelEntry.ipv4Cfg.udpSrcPort             = PRV_TGF_TUNNEL_IFA_UDP_SRC_PORT_CNS;

    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet ");

}

/**
* @internal prvTgfEgressNodeIfaConfigurationSet function
* @endinternal
*
* @brief  IFA egress node configuration settings
*/
void prvTgfEgressNodeIfaConfigurationSet()
{
    /* Bridge config */
    prvTgfIngressNodeIfaBridgeConfigSet();

    /* Mirror port config */
    prvTgfIngressNodeIfaMirrorPortConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

    /* Loopback port config */
    prvTgfIngresssNodeIfaLoopbackPortConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

    /* Loopback port TTI config */
    prvTgfLoopbackNodeIfaTtiConfigSet();

    /* EPCL UDB config */
    prvTgfIngresssNodeIfaUdbConfigSet();

    /* Receive port EPCL config */
    prvTgfIngresssNodeIfaEpcl2ConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

    /* Loopback port EPCL config */
    prvTgfIngresssNodeIfaEpcl1ConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

    /* PHA config */
    prvTgfIngressNodeIfaPhaConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

    /* Mirror config */
    prvTgfEgressNodeIfaMirrorPortConfigSet();

    /* TS config */
    prvTgfEgressNodeIfaTsConfigSet();

    /* IFA config */
    prvTgfIngresssNodeIfaConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

    /* CNC config for IFA */
    prvTgfIngresssNodeIfaCncConfigSet(PRV_TGF_IFA_EGRESS_NODE_CNS);

}

/**
* @internal prvTgfEgressNodeIfaTrafficSend function
* @endinternal
*
* @brief  Traffic test for IFA egress node
*/
void prvTgfEgressNodeIfaTrafficSend()
{
    GT_STATUS                rc                                 = GT_OK;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Enable capture on receive port */
    egressPortInterface.type              = CPSS_INTERFACE_PORT_E;
    egressPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    egressPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&egressPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    prvTgfPacketIpv4UdpPart.totalLen += (TGF_IFA_HEADER_SIZE_CNS + PRV_TGF_IFA_EGRESS_NODE_CNS*TGF_IFA_METADATA_SIZE_CNS);
    /* setup transmit params */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEgressIpv4UdpIfaPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    cpssOsTimerWkAfter(1000);
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEgressIpv4UdpIfaPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egressPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /*verify the IFA packet*/
    prvTgfIngressNodeIfaPacketValidate(PRV_TGF_IFA_EGRESS_NODE_CNS,PRV_TGF_IFA_PACKET_TYPE_IPv4_UDP_CNS);

}

/**
* @internal prvTgfEgressIfaePortConfigReset function
* @endinternal
*
* @brief   ePort config restore
*
*/
static GT_VOID prvTgfEgressIfaePortConfigReset()
{

    GT_STATUS   rc;

    /* AUTODOC: Restore ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo2));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /* AUTODOC: Restore ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo2));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfEgressNodeIfaTsConfigRestore function
* @endinternal
*
* @brief TS test configurations restore
*/
static GT_VOID prvTgfEgressNodeIfaTsConfigRestore()
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    /* AUTODOC: Restore tunnel start entry configuration */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
}

/**
* @internal prvTgfEgressNodeIfaConfigurationRestore function
* @endinternal
*
* @brief  Restore IFA egress node configurations
*/
void prvTgfEgressNodeIfaConfigurationRestore()
{
    /* restore Bridge config */
    prvTgfIngressNodeIfaBridgeConfigReset();

    /* restore EPCL configuration */
    prvTgfIngressNodeIfaEpclConfigRestore();

    /* restore Loopback configuration */
    prvTgfIngressNodeIfaLoopbackConfigReset();

    /* restore PHA config */
    prvTgfIngressNodeIfaPhaConfigRestore();

    /* restore TTI config */
    prvTgfIngressNodeIfaTtiConfigReset();

    /* restore Mirror port config */
    prvTgfIngressNodeIfaMirrorPortConfigReset();

    /* restore TAM/IFA config */
    prvTgfIngressNodeIfaTamConfigRestore();

    /* restore CNC config */
    prvTgfIngressNodeIfaCncConfigRestore();

    /* restore ePort config */
    prvTgfEgressIfaePortConfigReset();

    /* restore TS config */
    prvTgfEgressNodeIfaTsConfigRestore();
}
