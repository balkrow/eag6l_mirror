/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file prvTgfIpfixFw.c
*
* @brief IPFIX Manager functional testing
*
* @version   1
********************************************************************************
*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>

#include <ipFix/prvAppIpfixFw.h>
#include <ipFix/prvAppIpfixFwDbg.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <pcl/prvTgfPclEgressTrapMirror.h>
#include <ipfix/prvTgfIpfixFw.h>
#include <bridge/prvTgfBrgBasicJumboFrame.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           1

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port index to next hop traffic to */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS         3

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

#define PRV_TGF_IPFIX_FW_JUMBO_PKT_PAYLOAD_SIZE   1960

/* default number of packets to send */
static GT_U32        prvTgfBurstCount   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;

/*capture type*/
static TGF_CAPTURE_MODE_ENT captureType = TGF_CAPTURE_MODE_MIRRORING_E;

/****************************** L2 Header and PayLoad *************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* DATA of packet */
/*static GT_U8 prvTgfPayloadDataArr[PRV_TGF_JUMBO_PKT_PAYLOAD_SIZE] = {*/
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/******************************* L3 Ipv4 UDP packet **********************************/

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv4EtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketUdpIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_UDP_E, /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 0x20, 0x21, 0x22, 0x23},   /* srcAddr */
    { 0x30, 0x31, 0x32, 0x33}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4UdpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* IPv4 TCP packet Info */
static TGF_PACKET_STC prvTgfPacketIpv4UdpInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPacketIpv4UdpPartArray)
        / sizeof(prvTgfPacketIpv4UdpPartArray[0])), /* numOfParts */
    prvTgfPacketIpv4UdpPartArray                    /* partsArray */
};

/******************************* L3 Ipv4 TCP packet **********************************/

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketTcpIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_TCP_E, /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 0x20, 0x21, 0x22, 0x23},   /* srcAddr */
    { 0x30, 0x31, 0x32, 0x33}    /* dstAddr */
};

/* packet's TCP part */
static TGF_PACKET_TCP_STC prvTgfPacketTcpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    1,                  /* sequence number */
    2,                  /* acknowledgment number */
    5,                  /* data offset */
    0,                  /* reserved */
    0x10,               /* flags -> SYN flag is on */
    4096,               /* window */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,/* csum */
    0                   /* urgent pointer */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv4TcpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketTcpIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* IPv4 TCP packet Info */
static TGF_PACKET_STC prvTgfPacketIpv4TcpInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPacketIpv4TcpPartArray)
        / sizeof(prvTgfPacketIpv4TcpPartArray[0])), /* numOfParts */
    prvTgfPacketIpv4TcpPartArray                    /* partsArray */
};


/****************************** Jumbo packet ************************************/

/* Jumbo packet payload */
static GT_U8 prvTgfJumboPayLoadDataArr[PRV_TGF_IPFIX_FW_JUMBO_PKT_PAYLOAD_SIZE] =
{
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketJumboPayloadPart = {
    sizeof(prvTgfJumboPayLoadDataArr),                       /* dataLength */
    prvTgfJumboPayLoadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketJumboPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv4EtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketJumboPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + \
    TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS +\
     sizeof(prvTgfJumboPayLoadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* global Packet Info structure */
static TGF_PACKET_STC  prvTgfJumboPacketInfo =
{
    PRV_TGF_PACKET_LEN_CNS,                                 /* totalLen */
    sizeof(prvTgfPacketJumboPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfPacketJumboPartArray                                       /* partsArray */
};

/****************************** IpV6 Udp packet ************************************/

/* Packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketIpv6EtherTypePart =
{
    TGF_ETHERTYPE_86DD_IPV6_TAG_CNS
};

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketUdpIpv6Part =
{
    6,                                      /* version */
    0,                                      /* trafficClass */
    0,                                      /* flowLabel */
    0x18,                                   /* payloadLen */
    TGF_PROTOCOL_UDP_E,                     /* nextHeader */
    0x40,                                   /* hopLimit */
    {0x2244, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x1010, 0x3212},/* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0x2020, 0x2020, 0x2020, 0x2020, 0x2020, 0xCCDD, 0xEEFF} /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of IpV6 UDP packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv6UdpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketUdpIpv6Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Ipv6 UDP packet Info */
static TGF_PACKET_STC prvTgfPacketIpv6UdpInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPacketIpv6UdpPartArray)
        / sizeof(prvTgfPacketIpv6UdpPartArray[0])), /* numOfParts */
    prvTgfPacketIpv6UdpPartArray                    /* partsArray */
};


/****************************** IpV6 Tcp packet ************************************/

/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketTcpIpv6Part =
{
    6,                                      /* version */
    0,                                      /* trafficClass */
    0,                                      /* flowLabel */
    0x18,                                   /* payloadLen */
    TGF_PROTOCOL_TCP_E,                     /* nextHeader */
    0x40,                                   /* hopLimit */
    {0x6688, 0, 0, 0, 0, 0, 0x0000, 0x3212},/* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xCCDD, 0xEEFF} /* TGF_IPV6_ADDR dstAddr */
};

/* PARTS of IpV6 TCP packet */
static TGF_PACKET_PART_STC prvTgfPacketIpv6TcpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketIpv6EtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketTcpIpv6Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketTcpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* IPv6 TCP packet Info */
static TGF_PACKET_STC prvTgfPacketIpv6TcpInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPacketIpv6TcpPartArray)
        / sizeof(prvTgfPacketIpv6TcpPartArray[0])), /* numOfParts */
    prvTgfPacketIpv6TcpPartArray                    /* partsArray */
};


/********************************************************************************/

/**
 * @internal prvTgfIpfixFwVlanConfigSet function
 * @endinternal
 *
 * @brief Set vlan configuration.
 *
 * @retval GT_VOID
 */
GT_VOID prvTgfIpfixFwVlanConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8     tagArray[] = {1, 1, 1, 1};

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

}

/**
 * @internal prvTgfIpfixFwDataPktsCheck function
 * @endinternal
 *
 * @brief Check data packet
 *
 * @retval GT_VOID
 */
GT_VOID prvTgfIpfixFwDataPktsCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    (void) rc;
    /* [TBD] */
}

GT_VOID prvTgfIpfixFwTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount,
    IN GT_BOOL  captureEnable
)
{
    GT_STATUS                       rc             = GT_OK;
    GT_U32                          sendPortNum    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                          nextHopPortNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    GT_U32                          partsCount     = 0;
    GT_U32                          packetSize     = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          numTriggers    = 0;
    TGF_VFD_INFO_STC                vfdArray1[2];
    TGF_VFD_INFO_STC                vfdArray2[2];
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U8                           expectVlan =  PRV_TGF_SEND_VLANID_CNS;

    (void) prvTgfBurstCount;

    if (sendPortIdx < 4)
    {
        sendPortNum = prvTgfPortsArray[sendPortIdx];
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = nextHopPortNum;

    if (captureEnable == GT_TRUE)
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;

        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    partsCount = sizeof(prvTgfPacketIpv4UdpPartArray) / sizeof(prvTgfPacketIpv4UdpPartArray[0]);
    packetPartsPtr = prvTgfPacketIpv4UdpPartArray;

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    cpssOsMemSet(vfdArray1, 0, sizeof(vfdArray1));
    vfdArray1[0].mode = TGF_VFD_MODE_INCREMENT_E;
    vfdArray1[0].modeExtraInfo = 0;
    vfdArray1[0].offset = 33;
    vfdArray1[0].cycleCount = 1;
    vfdArray1[0].incValue = 1;

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, burstCount, 1, vfdArray1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(1, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */
    PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

    /* get trigger counters where packet has Vlan as expected */
    cpssOsMemSet(vfdArray2, 0, sizeof(vfdArray2));
    vfdArray2[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray2[0].modeExtraInfo = 0;
    vfdArray2[0].offset = 15;
    vfdArray2[0].cycleCount = 1;

    cpssOsMemCpy(vfdArray2[0].patternPtr, &expectVlan, 1);

    if (captureEnable == GT_TRUE)
    {
        /* disable capture on nexthope port , before check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, captureType, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                        prvTgfDevNum, captureType);
    }
    /* check the packets to the CPU */
    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray2, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    if(portInterface.type  == CPSS_INTERFACE_PORT_E)
    {
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                        portInterface.devPort.hwDevNum, portInterface.devPort.portNum);
    }
    else
    {
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, \n",
                                        prvTgfDevNum);
    }

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfIpfixFwAllL4TrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount
)
{
    GT_STATUS                       rc             = GT_OK;
    GT_U32                          sendPortNum    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                          nextHopPortNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    TGF_PACKET_STC                  *packetInfoPtr = NULL;
    GT_U32                          numTriggers    = 0;
    TGF_VFD_INFO_STC                vfdArray1[2];
    TGF_VFD_INFO_STC                vfdArray2[2];
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U8                           expectVlan =  PRV_TGF_SEND_VLANID_CNS;
    PRV_TGF_IPFIX_FW_PKT_TYPE_ENT   pktType;

    (void) prvTgfBurstCount;

    if (sendPortIdx < 4)
    {
        sendPortNum = prvTgfPortsArray[sendPortIdx];
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = nextHopPortNum;

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    for (pktType = PRV_TGF_IPFIX_FW_PKT_TYPE_IPV4_UDP_E;
         pktType <= PRV_TGF_IPFIX_FW_PKT_TYPE_IPV6_TCP_E; pktType++)
    {
        cpssOsMemSet(vfdArray1, 0, sizeof(vfdArray1));
        vfdArray1[0].mode = TGF_VFD_MODE_INCREMENT_E;
        vfdArray1[0].modeExtraInfo = 0;
        vfdArray1[0].cycleCount = 1;
        vfdArray1[0].incValue = 1;

        switch (pktType)
        {
        case PRV_TGF_IPFIX_FW_PKT_TYPE_IPV4_UDP_E:
            packetInfoPtr = &prvTgfPacketIpv4UdpInfo;
            vfdArray1[0].offset = 33;
            break;
        case PRV_TGF_IPFIX_FW_PKT_TYPE_IPV4_TCP_E:
            packetInfoPtr = &prvTgfPacketIpv4TcpInfo;
            vfdArray1[0].offset = 33;
            break;
        case PRV_TGF_IPFIX_FW_PKT_TYPE_IPV6_UDP_E:
            packetInfoPtr = &prvTgfPacketIpv6UdpInfo;
            vfdArray1[0].offset = 58;
            break;
        case PRV_TGF_IPFIX_FW_PKT_TYPE_IPV6_TCP_E:
            packetInfoPtr = &prvTgfPacketIpv6TcpInfo;
            vfdArray1[0].offset = 58;
            break;
        default:
            break;
        }

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetInfoPtr, burstCount, 1, vfdArray1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        /* set transmit timings */
        rc = prvTgfSetTxSetup2Eth(1, 1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

        /* send packet -- send from specific port -- even if member of a trunk */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                     prvTgfDevNum, sendPortNum);

        /* -------------------------------------------------------------------------
         * 4. Get Trigger Counters
         */
        PRV_UTF_LOG1_MAC("Port [%d] capturing:\n", nextHopPortNum);

        /* get trigger counters where packet has Vlan as expected */
        cpssOsMemSet(vfdArray2, 0, sizeof(vfdArray2));
        vfdArray2[0].mode = TGF_VFD_MODE_STATIC_E;
        vfdArray2[0].modeExtraInfo = 0;
        vfdArray2[0].offset = 15;
        vfdArray2[0].cycleCount = 1;

        cpssOsMemCpy(vfdArray2[0].patternPtr, &expectVlan, 1);

        /* check the packets to the CPU */
        rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray2, &numTriggers);
        PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

        /* check TriggerCounters */
        rc = rc == GT_NO_MORE ? GT_OK : rc;
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                     portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

        /* restore transmit timings */
        rc = prvTgfSetTxSetup2Eth(0, 0);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfIpfixFwFlowsLearnFullCapacityTrafficGenerate
(
    IN  GT_U32      maxFlows,
    OUT GT_U32      *collisonCountPtr
)
{
    GT_STATUS                       rc             = GT_OK;
    GT_U32                          sendPortNum    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                          nextHopPortNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    GT_U32                          partsCount     = 0;
    GT_U32                          packetSize     = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    TGF_VFD_INFO_STC                vfdArray1[2];
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          sendPortIdx = 1;
    GT_U32                          burstCount = 1;
    GT_BOOL                         captureEnable = GT_FALSE;
    GT_U32                          i = 0;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC flowAllocStatus;
    CPSS_DXCH_EXACT_MATCH_ENTRY_STC keyParams;
    GT_U32                          collisonCount = 0;
    GT_U32                          portsLoopMax = (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 4 : 1;
    GT_U32                          falconPortNumArray[4] = {40, 120, 180, 240};
    GT_U32                          *portNumArrayPtr = NULL;

    (void) prvTgfBurstCount;

    portNumArrayPtr = (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E) ?
                      &prvTgfPortsArray[0] : &falconPortNumArray[0];
    if (sendPortIdx < 4)
    {
        sendPortNum = prvTgfPortsArray[sendPortIdx];
    }

    cpssOsMemSet(&flowAllocStatus, 0, sizeof(flowAllocStatus));
    cpssOsMemSet(&keyParams, 0, sizeof(keyParams));
    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = nextHopPortNum;

    if (captureEnable == GT_TRUE)
    {
        captureType = TGF_CAPTURE_MODE_MIRRORING_E;

        /* enable capture on next hop port/trunk */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, %d\n",
                                     prvTgfDevNum, captureType);
    }

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    collisonCount = 0;

    for(sendPortIdx = 0; sendPortIdx < portsLoopMax; sendPortIdx++)
    {
        sendPortNum = portNumArrayPtr[sendPortIdx];
        for (i=0; i<maxFlows; i++)
        {
            prvTgfPacketUdpPart.dstPort = i;

            partsCount = sizeof(prvTgfPacketIpv4UdpPartArray) / sizeof(prvTgfPacketIpv4UdpPartArray[0]);
            packetPartsPtr = prvTgfPacketIpv4UdpPartArray;

            /* calculate packet size */
            rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

            /* build packet */
            packetInfo.totalLen   = packetSize;
            packetInfo.numOfParts = partsCount;
            packetInfo.partsArray = packetPartsPtr;

            cpssOsMemSet(vfdArray1, 0, sizeof(vfdArray1));
            /*vfdArray1[0].mode = TGF_VFD_MODE_INCREMENT_E;
            vfdArray1[0].modeExtraInfo = 0;
            vfdArray1[0].offset = 33;
            vfdArray1[0].cycleCount = 1;
            vfdArray1[0].incValue = 1;*/

            /* setup packet */
            rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, burstCount, 1, vfdArray1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

            /* set transmit timings */
            rc = prvTgfSetTxSetup2Eth(1, 1);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

            /* send packet -- send from specific port -- even if member of a trunk */
            rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                         prvTgfDevNum, sendPortNum);

            /*cpssOsTimerWkAfter(10);*/
            if(PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
            {
                rc = cpssDxChExactMatchPortGroupAutoLearnCollisionKeyParamsGet(prvTgfDevNum, CPSS_DXCH_EXACT_MATCH_UNIT_0_E,
                                                                          CPSS_PORT_GROUP_UNAWARE_MODE_CNS, &keyParams);
                if (rc != GT_FAIL)
                {
                    cpssOsPrintf("[Info] Collision KeySize: %d, lookup num: %d, pattern: %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
                                 keyParams.key.keySize, keyParams.lookupNum, keyParams.key.pattern[0], keyParams.key.pattern[1],
                                 keyParams.key.pattern[2], keyParams.key.pattern[3], keyParams.key.pattern[4], keyParams.key.pattern[5],
                                 keyParams.key.pattern[6], keyParams.key.pattern[7], keyParams.key.pattern[8], keyParams.key.pattern[9],
                                 keyParams.key.pattern[10], keyParams.key.pattern[11], keyParams.key.pattern[12], keyParams.key.pattern[13]);
                    collisonCount++;
                }
            }
            if (i%200 == 0)
            {
                cpssOsPrintf("[DBG] portIdx = %d, flowNum: %d\n", sendPortIdx, i);
            }
        }
    }
    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    *collisonCountPtr = collisonCount;
    cpssOsPrintf("CollisonCount: %d\n", collisonCount);

}

GT_VOID prvTgfIpfixFwElephantFlowsTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount,
    IN GT_BOOL  isJumbo
)
{
    GT_STATUS                       rc             = GT_OK;
    GT_U32                          sendPortNum    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                          partsCount     = 0;
    GT_U32                          packetSize     = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    TGF_VFD_INFO_STC                vfdArray1[2];

    if (sendPortIdx < 4)
    {
        sendPortNum = prvTgfPortsArray[sendPortIdx];
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    if (isJumbo == GT_TRUE)
    {
        prvTgfPacketUdpIpv4Part.totalLen = 0x7D6;
        partsCount = sizeof(prvTgfPacketJumboPartArray) / sizeof(prvTgfPacketJumboPartArray[0]);
        packetPartsPtr = prvTgfPacketJumboPartArray;

        prvTgfBrgBasicJumboFrameConfigSet();

    }
    else
    {
        partsCount = sizeof(prvTgfPacketIpv4UdpPartArray) / sizeof(prvTgfPacketIpv4UdpPartArray[0]);
        packetPartsPtr = prvTgfPacketIpv4UdpPartArray;
    }

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize+4;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    cpssOsMemSet(vfdArray1, 0, sizeof(vfdArray1));
    vfdArray1[0].mode = TGF_VFD_MODE_INCREMENT_E;
    vfdArray1[0].modeExtraInfo = 0;
    vfdArray1[0].offset = 33;
    vfdArray1[0].cycleCount = 1;
    vfdArray1[0].incValue = 1;

    if (isJumbo == GT_TRUE)
    {
        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfJumboPacketInfo, burstCount, 1, vfdArray1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);
    }
    else
    {
        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, burstCount, 1, vfdArray1);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);
    }

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);

    if (isJumbo == GT_TRUE)
    {
        prvTgfBrgBasicJumboFrameConfigRestore();
    }

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

GT_VOID prvTgfIpfixFwConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: invalidate vlan entries 5,6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);

    prvTgfPclRestore();
}

GT_VOID prvTgfIpfixFwNewFlowsLearn
(
    IN  GT_BOOL withFw,
    IN  GT_BOOL checkDataPkts,
    IN  GT_BOOL hwAutoLearn
)
{
    GT_STATUS   rc = GT_OK;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      portIdx = 1;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC alStatus;
    GT_U32      i=0;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC flow;
    GT_U32      startFlowId;
    CPSS_DXCH_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32      expectedPktCount = 3;
    GT_U32      burstCount = 4;
    GT_U32      startPortIdx = 1;
    GT_U32      numPorts = 3;

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixFwMain(prvTgfDevNum, sCpuNum, hwAutoLearn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwMain failed");

#if 0
    rc = appDemoIpfixFwCpuRxDumpEnable(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwCpuRxDumpEnable failed");
#endif

    for (portIdx = startPortIdx; portIdx < startPortIdx+numPorts; portIdx++)
    {
        /* 1.2: Send 4 flows and check they are learned */
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE /* captureEnable*/);
    }

    cpssOsTimerWkAfter(1000);
    appDemoIpfixFwDebugCountersDump();

    if ((withFw == GT_FALSE) && (hwAutoLearn == GT_TRUE))
    {
        /* check auto Learning */
        cpssOsMemSet(&alStatus, 0, sizeof(alStatus));
        rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(prvTgfDevNum, CPSS_DXCH_EXACT_MATCH_UNIT_0_E,
                                                                      1, &alStatus);
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, alStatus.numAllocated,
                                         "Auto Learn flows check failed");
    }

    /* Wait for some time so that the new flows are relocated */
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_IPFIX_ENTRY_RELOCATION);

    for (portIdx = startPortIdx; portIdx < startPortIdx+numPorts; portIdx++)
    {
        for (i=0; i<expectedPktCount; i++)
        {
            /* 1.3: Send same 4 flows again and check they are not learned again */
            prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE /* captureEnable*/);
        }
    }

    cpssOsTimerWkAfter(4000);
    appDemoIpfixFwDebugCountersDump();
    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);

    /* Packets are not mirrored for transmit of just one packet  */
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount*numPorts, dbgCounters.firstPktsFetchedByHost,
                                 "First packets fetch check failed");

    if (withFw == GT_TRUE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * numPorts, dbgCounters.activeFlows,
                                     "Active flows check failed");
        if (checkDataPkts == GT_TRUE)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_E],
                                         "Received config errors");
            UTF_VERIFY_EQUAL0_STRING_MAC(dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E],
                                         dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E],
                                         "DataGetAll is not matching export completion");

            startFlowId = 0;
            for (i=0; i<burstCount; i++)
            {
                cpssOsMemSet(&flow, 0, sizeof(flow));
                rc = appDemoIpfixFwNextActiveFlowGet(prvTgfDevNum, startFlowId, &flow);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwNextActiveFlowGet failed");
                if (rc != GT_OK)
                {
                    break;
                }

                UTF_VERIFY_EQUAL1_STRING_MAC(expectedPktCount, flow.dataRecord.packetCount[0],
                                             "Packet count didn't match for flow: %d", flow.dataRecord.flowId);
                startFlowId = flow.info.flowId + 1;
            }
        }
    }
    else if (checkDataPkts == GT_TRUE)
    {
        startFlowId = 0;
        for (i=0; i<burstCount; i++)
        {
            cpssOsMemSet(&flow, 0, sizeof(flow));
            rc = appDemoIpfixFwNextActiveFlowGet(prvTgfDevNum, startFlowId, &flow);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwNextActiveFlowGet failed");
            if (rc != GT_OK)
            {
                break;
            }

            cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
            rc = cpssDxChIpfixEntryGet(prvTgfDevNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                       (flow.info.flowId & 0xFFF), CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E,
                                       &ipfixEntry);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpfixEntryGet failed");
            UTF_VERIFY_EQUAL1_STRING_MAC(expectedPktCount*numPorts, ipfixEntry.packetCount,
                                         "Packet count didn't match for flow: %d", flow.info.flowId);

            startFlowId = flow.info.flowId + 1;

        }
    }
#if 0
    rc = appDemoIpfixFwCpuRxDumpEnable(GT_FALSE);
#endif
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_DELETE_SEND);
    /* 1.4: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearn);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @internal prvTgfIpfixFwConfigSet function
 * @endinternal
 *
 * @brief Set IPFIX configuration
 *
 * @param[in] devNum   - device number
 * @param[in] sCpuNum  - service CPU number
 * @param[in] testType - test type
 *
 * @return GT_OK - on success
 */
GT_STATUS prvTgfIpfixFwConfigSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   sCpuNum,
    IN  PRV_TGF_IPFIX_FW_TEST_TYPE_ENT testType
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    globalCfg;
    CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC    exportCfg;
    CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  elephantCfg;
    CPSS_DXCH_NET_DSA_PARAMS_STC            dsaInfo;
    GT_U8                                   dsaTag[PRV_APP_IPFIX_FW_TX_DSA_TAG_LEN_CNS];
    GT_HW_DEV_NUM                           hwDevNum;
    GT_U32                                  i;

    /* Set CPSS ipfix fw global configuration */
    cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
    globalCfg.ipfixEnable                = GT_FALSE; /* has no effect */
    globalCfg.idleAgingOffload           = GT_FALSE;
    globalCfg.longAgingOffload           = GT_FALSE;
    globalCfg.countersExtensionOffload   = GT_FALSE; /* has no effect */
    globalCfg.timeStampsExtensionOffload = GT_FALSE; /* has no effect */
    globalCfg.firstPolicerStage          = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    globalCfg.maxInTransitIndex          = PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_CNS(devNum);
    globalCfg.idleTimeout                = 0; /* has no effect */
    globalCfg.activeTimeout              = 0; /* has no effect */
    globalCfg.scanCycleInterval          = 20; /* 1 Seconds i.e. 10 * 100 ms*/
    globalCfg.scanLoopItems              = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 256 :     PRV_APP_IPFIX_FW_SCAN_LOOP_ITEMS_CNS(devNum); /* ipfix entries scan */
    globalCfg.secondPolicerStage         = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
    globalCfg.reportNewFlows             = GT_TRUE;
    globalCfg.hwAutoLearnEnable          = GT_TRUE;
    globalCfg.maxFwIndex                 = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1;
    globalCfg.maxAppIndex                = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1;
    globalCfg.firstPolicerMaxIpfixIndex  = 0; /* has no effect */
    globalCfg.secondPolicerMaxIpfixIndex = 0; /* has no effect */

    if ((testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_TIMEOUT_E) ||
        (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_ELEPHANT_AND_AGING_BASIC_E))
    {
        globalCfg.idleAgingOffload           = GT_TRUE;
        globalCfg.idleTimeout                = PRV_TGF_IPFIX_FW_IDLE_TIMEOUT_IN_SEC_CNS;
    }
    else if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_ACTIVE_TIMEOUT_E)
    {
        globalCfg.longAgingOffload           = GT_TRUE;
        globalCfg.activeTimeout              = PRV_TGF_IPFIX_FW_ACTIVE_TIMEOUT_IN_SEC_CNS;
    }
    else if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_IDLE_TIMEOUT_E)
    {
        globalCfg.idleAgingOffload           = GT_TRUE;
        globalCfg.idleTimeout                = PRV_TGF_IPFIX_FW_MAX_IDLE_TIMEOUT_IN_SEC_CNS;
    }
    else if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_EXTENDED_IDLE_TIMEOUT_E)
    {
        globalCfg.idleAgingOffload           = GT_TRUE;
        globalCfg.idleTimeout                = PRV_TGF_IPFIX_FW_MAX_EXTENDED_IDLE_TIMEOUT_IN_SEC_CNS;
        globalCfg.timeStampsExtensionOffload = GT_TRUE;
    }
    else if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_ACTIVE_TIMEOUT_E)
    {
        globalCfg.longAgingOffload           = GT_TRUE;
        globalCfg.activeTimeout              = PRV_TGF_IPFIX_FW_MAX_ACTIVE_TIMEOUT_IN_SEC_CNS;
    }
    else if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_EXTENDED_ACTIVE_TIMEOUT_E)
    {
        globalCfg.longAgingOffload           = GT_TRUE;
        globalCfg.activeTimeout              = PRV_TGF_IPFIX_FW_MAX_EXTENDED_ACTIVE_TIMEOUT_IN_SEC_CNS;
        globalCfg.timeStampsExtensionOffload = GT_TRUE;
    }
    else if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_ACTIVE_TIMEOUT_E)
    {
        globalCfg.idleAgingOffload           = GT_TRUE;
        globalCfg.longAgingOffload           = GT_TRUE;
        globalCfg.idleTimeout                = PRV_TGF_IPFIX_FW_IDLE_TIMEOUT_IN_IDLE_ACTIVE_SEC_CNS;
        globalCfg.activeTimeout              = PRV_TGF_IPFIX_FW_ACTIVE_TIMEOUT_IN_IDLE_ACTIVE_SEC_CNS;
        globalCfg.timeStampsExtensionOffload = GT_TRUE;
    }

    rc = cpssDxChIpfixFwGlobalConfigSet(devNum, sCpuNum, &globalCfg);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChIpfixFwGlobalConfigSet failed rc = %d\n", rc);
        return rc;
    }

    /* Set CPSS ipfix fw export configuration */
    /* Build Extended TO_CPU DSA tag; 4 words */
    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChCfgHwDevNumGet failed rc = %d\n", rc);
        return rc;
    }
    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));
    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
    dsaInfo.dsaInfo.toCpu.hwDevNum = (hwDevNum + 1) % 32;
    dsaInfo.dsaInfo.toCpu.cpuCode = PRV_APP_IPFIX_FW_DATA_PKTS_CPU_CODE;
    /* build DSA Tag buffer */
    cpssOsMemSet(dsaTag, 0, sizeof(dsaTag));
    rc = cpssDxChNetIfDsaTagBuild(devNum, &dsaInfo, &dsaTag[0]);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChNetIfDsaTagBuild failed rc = %d\n", rc);
        return rc;
    }

    cpssOsMemSet(&exportCfg, 0, sizeof(exportCfg));
    exportCfg.dataPktMtu             = PRV_APP_IPFIX_FW_DATA_PKTS_MTU_CNS;
    exportCfg.localQueueNum          = PRV_APP_IPFIX_FW_DATA_PKTS_QUEUE_NUM_CNS;
    exportCfg.mgNum                  = 0;
    exportCfg.periodicExportEnable   = GT_FALSE;
    exportCfg.periodicExportInterval = 1;
    exportCfg.exportLastCpuCode      = GT_TRUE;
    exportCfg.exportLastPktCmd       = GT_TRUE;
    for(i=0; i<PRV_APP_IPFIX_FW_TX_DSA_TAG_LEN_CNS; i++)
    {
        exportCfg.txDsaTag[i/4] |= (dsaTag[i] << (8*(i%4)));
    }

    if (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_PERIODIC_DATA_EXPORT_E)
    {
        exportCfg.periodicExportEnable   = GT_TRUE;
    }

    rc = cpssDxChIpfixFwExportConfigSet(devNum, sCpuNum, &exportCfg);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChIpfixFwExportConfigSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Set CPSS ipfix fw elephant configuration */
    cpssOsMemSet(&elephantCfg, 0, sizeof(elephantCfg));
    elephantCfg.elephantDetectionOffload = GT_FALSE;
    elephantCfg.reportMiceEnable         = GT_TRUE;
    elephantCfg.reportEmIndexEnable      = GT_TRUE;
    elephantCfg.pktCntThreshold[0]       = 0x0; /* change later */
    elephantCfg.pktCntThreshold[1]       = 0;
    elephantCfg.byteCntThreshold[0]      = 0x40;
    elephantCfg.byteCntThreshold[1]      = 0;
    elephantCfg.crossCntThresholdHigh    = 0x1;
    elephantCfg.crossCntThresholdLow     = 0x0;
    elephantCfg.startThreshold           = 0x0;
    if ((testType == PRV_TGF_IPFIX_FW_TEST_TYPE_ELEPHANT_BASIC_E) ||
        (testType == PRV_TGF_IPFIX_FW_TEST_TYPE_ELEPHANT_AND_AGING_BASIC_E))
    {
        elephantCfg.elephantDetectionOffload = GT_TRUE;
    }
    rc = cpssDxChIpfixFwElephantConfigSet(devNum, sCpuNum, &elephantCfg);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChIpfixFwElephantConfigSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Get configurations and check or update params */
#ifndef ASIC_SIMULATION
    rc = appDemoIpfixFwConfigGetAndVerify(devNum, sCpuNum, 0x7, GT_TRUE, GT_FALSE);
    if (rc != GT_OK)
    {
        cpssOsPrintf("appDemoIpfixFwConfigGetAndVerify failed: rc=%d\n", rc);
        return rc;
    }
#endif

    return GT_OK;
}


/**
 * @internal    prvTgfIpfixFwMain function
 * @endinternal
 *
 * @brief   Perform all initializations and start IPFIX
 *          Manager tasks
 *
 * @param[in] devNum   -   device number
 * @param[in] sCpuNum  -   service CPU Number
 * @param[in] testType -   test type
 *
 * @retval GT_OK - on success
 */
GT_STATUS prvTgfIpfixFwMain
(
    IN  GT_U8   devNum,
    IN  GT_U8   sCpuNum,
    IN  PRV_TGF_IPFIX_FW_TEST_TYPE_ENT testType,
    IN  GT_BOOL hwAutoLearnEnable
)
{
    GT_STATUS                               rc = GT_OK;

    /* Initialize application ipfix manager */
    rc = appDemoIpfixFwDbCreate(devNum);
    if (rc != GT_OK)
    {
        cpssOsPrintf("appDemoIpfixFwDbCreate failed: rc=%d\n", rc);
        return rc;
    }

    /* Enable debug prints */
    appDemoIpfixFwDebugPrintsEnable(GT_TRUE);
    /* appDemoIpfixFwCpuRxDumpEnable(GT_TRUE); */

    rc = appDemoIpfixFwPreReqInit(devNum, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E, hwAutoLearnEnable);
    if (rc != GT_OK)
    {
        cpssOsPrintf("appDemoIpfixFwPreReqInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Load FW */
    rc = cpssDxChIpfixFwInit(devNum, sCpuNum);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChIpfixFwInit failed: rc=%d\n", rc);
        return rc;
    }

    /* FW config Set */
    rc = prvTgfIpfixFwConfigSet(devNum, sCpuNum, testType);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChIpfixFwEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Ipfix manager enable */
    rc = cpssDxChIpfixFwEnableSet(devNum, sCpuNum, GT_TRUE);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssDxChIpfixFwEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixFwThreadsStart(devNum, sCpuNum, 0xF);
    if (rc != GT_OK)
    {
        cpssOsPrintf("appDemoIpfixFwThreadsStart failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}

