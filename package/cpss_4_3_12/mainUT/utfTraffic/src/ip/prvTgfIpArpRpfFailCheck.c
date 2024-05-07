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
* @file prvTgfIpArpRpfFailCheck.c
*
* @brief Unicast RPF Fail Check
*
* @version   1
********************************************************************************
*/
#include <ip/prvTgfIpArpRpfFailCheck.h>

#include <utf/utfMain.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS           (5)

/* nextHop VLAN Id */
#define PRV_TGF_NEXTHOP_VLANID_CNS        (6)

/* ARP table Index for Source Mac*/
#define PRV_TGF_ARP_SRC_ENTRY_IDX_CNS     (5)

/* ARP table Index for Dest Mac*/
#define PRV_TGF_ARP_DST_ENTRY_IDX_CNS     (6)

/* Route entry SIP/DIP index */
#define PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS   (4)
#define PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS   (3)

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         (0)

/* bridge port number to receive traffic from */
#define PRV_TGF_BRIDGE_PORT_IDX_CNS      (2)

/* nextHop port number to receive traffic from */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS      (3)

/* Number of ports */
#define PRV_TGF_PORT_COUNT_CNS            (4)

/* the counter set for a route entry is linked to */
#define PRV_TGF_COUNT_SET_CNS             CPSS_IP_CNT_SET0_E

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS   (1)

/* the LPM DB id for LPM Table */
#define PRV_TGF_LPM_DB_ID_CNS             (0)


/*************************** Saved data ***************************************/
static GT_ETHERADDR arpEntrySrc;
static GT_ETHERADDR arpEntryDst;
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC sipRouteEntry;
static PRV_TGF_IP_UC_ROUTE_ENTRY_STC dipRouteEntry;
static GT_IPADDR sip;
static GT_IPADDR dip;
static GT_BOOL ucRpfFailServiceEnabled;
static GT_BOOL sndPortRoutingEnabled;
static GT_BOOL portSipLookupEnable;
static GT_BOOL globalRoutingEnable;
static GT_BOOL enableRouterTriggerArpTrapGet;
static GT_BOOL enableRouterTriggerArpSoftDropGet;
static CPSS_PACKET_CMD_ENT     arpBcModeGet;
/* stored default Vlan ID and ARP commands */
static GT_U16               prvTgfDefVlanId = 0;
static CPSS_PACKET_CMD_ENT  saveTrappingCmd;
static CPSS_PACKET_CMD_ENT  exceptionCmdGet;
/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x11, 0x01}    /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_SEND_VLANID_CNS                       /* pri, cfi, VlanId */
};

/* packet's ethertype */
/*static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};*/

/* ethertype part of ARP packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketArpEtherTypePart = {TGF_ETHERTYPE_0806_ARP_TAG_CNS};

/* packet's ARP header */
static TGF_PACKET_ARP_STC prvTgfPacketArpPart = {
    0x01,                                   /* HW Type */
    0x0800,                                 /* Protocol (IPv4= 0x0800) */
    0x06,                                   /* HW Len = 6 */
    0x04,                                   /* Proto Len = 4 */
    0x01,                                   /* Opcode */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},   /* HW Address of Sender (MAC SA) */
    {0x11, 0x12, 0x13, 0x14},               /* Protocol Address of Sender(SIP)*/
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},   /* HW Address of Target (MAC DA) */
    {0x30, 0x31, 0x32, 0x33}                /* Protocol Address of Target(DIP)*/
};

/* DATA of packet */
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

/* PARTS of packet ARP */
static TGF_PACKET_PART_STC prvTgfPacketArpPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part}, /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketArpEtherTypePart},
    {TGF_PACKET_PART_ARP_E,       &prvTgfPacketArpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};
static CPSS_PORT_MAC_COUNTER_SET_STC emptyCounters;

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal convertMac function
* @endinternal
*
* @brief   Converts TGF_MAC_ADDR to GT_ETHERADDR
*
* @param[in] utfMacAddr               - MAC address in UTF format
*
* @param[out] cpssMacAddrPtr           - ptr to MAC address in CPSS format
*                                       None
*/
static GT_VOID convertMac
(
    IN  TGF_MAC_ADDR utfMacAddr,
    OUT GT_ETHERADDR *cpssMacAddrPtr
)
{
    cpssOsMemCpy(cpssMacAddrPtr->arEther, utfMacAddr,
        sizeof(TGF_MAC_ADDR));
}

/**
* @internal countersEq function
* @endinternal
*
* @brief   Checks if counter sets are equal
*
* @param[in] counters1                - the first set of counters
* @param[in] counters2                - second set of counters
*                                       Equality of two counter sets. GT_TRUE=equal
*/
static GT_BOOL countersEq
(
    IN CPSS_PORT_MAC_COUNTER_SET_STC counters1,
    IN CPSS_PORT_MAC_COUNTER_SET_STC counters2
)
{
    GT_BOOL retval;

    retval =
        counters1.goodOctetsSent.l[0] == counters2.goodOctetsSent.l[0] &&
        counters1.goodPktsSent.l[0]   == counters2.goodPktsSent.l[0] &&
        counters1.ucPktsSent.l[0]     == counters2.ucPktsSent.l[0] &&
        counters1.brdcPktsSent.l[0]   == counters2.brdcPktsSent.l[0] &&
        counters1.mcPktsSent.l[0]     == counters2.mcPktsSent.l[0] &&
        counters1.goodOctetsRcv.l[0]  == counters2.goodOctetsRcv.l[0] &&
        counters1.goodPktsRcv.l[0]    == counters2.goodPktsRcv.l[0] &&
        counters1.ucPktsRcv.l[0]      == counters2.ucPktsRcv.l[0] &&
        counters1.brdcPktsRcv.l[0]    == counters2.brdcPktsRcv.l[0] &&
        counters1.mcPktsRcv.l[0]      == counters2.mcPktsRcv.l[0];
    return retval;
}

/**
* @internal printCounters function
* @endinternal
*
* @brief   Checks if counters are equal
*
* @param[in] counters                 -  to print
*                                       None
*/
static GT_VOID printCounters
(
    IN CPSS_PORT_MAC_COUNTER_SET_STC counters
)
{
    PRV_UTF_LOG1_MAC("  goodOctetsSent = %d\n", counters.goodOctetsSent.l[0]);
    PRV_UTF_LOG1_MAC("    goodPktsSent = %d\n", counters.goodPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("      ucPktsSent = %d\n", counters.ucPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("    brdcPktsSent = %d\n", counters.brdcPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("      mcPktsSent = %d\n", counters.mcPktsSent.l[0]);
    PRV_UTF_LOG1_MAC("   goodOctetsRcv = %d\n", counters.goodOctetsRcv.l[0]);
    PRV_UTF_LOG1_MAC("     goodPktsRcv = %d\n", counters.goodPktsRcv.l[0]);
    PRV_UTF_LOG1_MAC("       ucPktsRcv = %d\n", counters.ucPktsRcv.l[0]);
    PRV_UTF_LOG1_MAC("     brdcPktsRcv = %d\n", counters.brdcPktsRcv.l[0]);
    PRV_UTF_LOG1_MAC("       mcPktsRcv = %d\n", counters.mcPktsRcv.l[0]);
    PRV_UTF_LOG0_MAC("\n");

}

/**
* @internal resetCounters function
* @endinternal
*
* @brief   Resets counters for all ports including IP counters
*         Clears Rx Tx tables
*/
static GT_VOID resetCounters
(
    GT_VOID
)
{
    GT_U32 portIter;
    PRV_TGF_IP_ROUTING_MODE_ENT routingMode;
    GT_STATUS rc;

    rc = prvTgfIpRoutingModeGet(&routingMode);
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* Reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: "
            "port=%d", prvTgfPortsArray[portIter]);

        if (routingMode == PRV_TGF_IP_ROUTING_MODE_IP_LTT_ENTRY_E)
        {
            /* Reset IP couters and set ROUTE_ENTRY mode for all ports */
            rc = prvTgfCountersIpSet(prvTgfPortsArray[portIter], portIter);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfCountersIpSet: %d",
                prvTgfPortsArray[portIter]);
        }
    }
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

/**
* @internal prvTgfIpArpRpfFailCheckConfig function
* @endinternal
*
* @brief   AUTODOC: Set Configuration:
*         1. Create two VLANs SENDER and NEXTHOP
*         2. Save unicast routing state for sender port
*         3. Enable unicast routing on sender port
*         4. Assign virtual router to sender VLAN
*         5. Enable unicast routing in sender VLAN
*         6. Create FDB entries for DA/SA MACs
*         7. Save ARP table entries
*         8. Write ARP table entries for DA/SA MACs
*         9. Save route entries
*         10. Set up route entries to route packets between two VLANs
*         11. Add prefix rules for SIP and DIP which use the route entries
*         12. Save UC RPF Fail check service state
*         13. Enable UC RPF Fail check service
*/
GT_VOID prvTgfIpArpRpfFailCheckConfig
(
    GT_VOID
)
{
    GT_ETHERADDR arpMacAddr;
    GT_STATUS rc;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC routeEntry;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    GT_U8 sndVlanTagArray[] = {1, 1, 1};
    PRV_TGF_BRG_MAC_ENTRY_STC   macEntry;
    CPSS_DXCH_IP_BRG_EXCP_CMD_ENT     bridgeExceptionCmd;
    GT_BOOL                           enableRouterTrigger;

     /* get default vlanId */
    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 &prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                 prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                 PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                                 "ERROR of prvTgfBrgVlanPortVidSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with all untagged ports */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray,
                                           NULL, sndVlanTagArray, prvTgfPortsNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
                               "ERROR of prvTgfBrgDefVlanEntryWithPortsSet: %d",
                                 prvTgfDevNum);

    /* AUTODOC: global routing enable */
    rc =  cpssDxChIpRoutingEnableGet(prvTgfDevNum,&globalRoutingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChIpRoutingEnableGet:");

    rc =  cpssDxChIpRoutingEnable(prvTgfDevNum,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChIpRoutingEnable:");

    /* AUTODOC: Save Unicast IPv4 Routing on sender port enabled state */
    rc = prvTgfIpPortRoutingEnableGet(
        PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, &sndPortRoutingEnabled);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnableGet: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Enable Unicast IPv4 Routing on sender port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnable: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Update VRF Id PRV_TGF_SEND_VLANID_CNS */
    rc = prvTgfBrgVlanVrfIdSet(PRV_TGF_SEND_VLANID_CNS, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanVrfIdSet: vlanid=%d",
        PRV_TGF_SEND_VLANID_CNS);

    /* AUTODOC: Enable IPv4 Unicast Routing on sender Vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpVlanRoutingEnable: vlanid=%d, enable=%d",
        PRV_TGF_SEND_VLANID_CNS, GT_TRUE);

    /* AUTODOC: Add FDB entry for SA/DA mac*/
    cpssOsMemSet(&macEntry, 0, sizeof(macEntry));
    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
        prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    macEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    macEntry.key.key.macVlan.vlanId       = PRV_TGF_SEND_VLANID_CNS;
    macEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum;
    macEntry.dstInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_BRIDGE_PORT_IDX_CNS];
    macEntry.isStatic                 = GT_TRUE;
    macEntry.daCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.saCommand                = PRV_TGF_PACKET_CMD_FORWARD_E;
    macEntry.daRoute                  = GT_TRUE;
    macEntry.sourceId                 = 0;
    macEntry.userDefined              = 0;
    macEntry.daQosIndex               = 0;
    macEntry.saQosIndex               = 0;
    macEntry.daSecurityLevel          = 0;
    macEntry.saSecurityLevel          = 0;
    macEntry.appSpecificCpuCode       = GT_FALSE;
    macEntry.mirrorToRxAnalyzerPortEn = GT_FALSE;

    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");

    cpssOsMemCpy(macEntry.key.key.macVlan.macAddr.arEther,
        prvTgfPacketL2Part.saMac, sizeof(TGF_MAC_ADDR));
    macEntry.dstInterface.devPort.portNum   =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    macEntry.key.key.macVlan.vlanId         = PRV_TGF_SEND_VLANID_CNS;
    rc = prvTgfBrgFdbMacEntrySet(&macEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntryWrite");

    /* AUTODOC: Save addresses from ARP table */
    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        &arpEntrySrc);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: "
        "Idx=%d\n", PRV_TGF_ARP_SRC_ENTRY_IDX_CNS);

    rc = prvTgfIpRouterArpAddrRead(prvTgfDevNum, PRV_TGF_ARP_DST_ENTRY_IDX_CNS,
        &arpEntryDst);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrRead: "
        "Idx=%d\n", PRV_TGF_ARP_DST_ENTRY_IDX_CNS);

    /* AUTODOC: Write to ARP table new addresses */
    convertMac(prvTgfPacketL2Part.saMac, &arpMacAddr);
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_SRC_ENTRY_IDX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpMacAddr.arEther[0], arpMacAddr.arEther[1], arpMacAddr.arEther[2],
        arpMacAddr.arEther[3], arpMacAddr.arEther[4], arpMacAddr.arEther[5]);

    convertMac(prvTgfPacketL2Part.daMac, &arpMacAddr);
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_DST_ENTRY_IDX_CNS, &arpMacAddr);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_DST_ENTRY_IDX_CNS,
        arpMacAddr.arEther[0], arpMacAddr.arEther[1], arpMacAddr.arEther[2],
        arpMacAddr.arEther[3], arpMacAddr.arEther[4], arpMacAddr.arEther[5]);

    /* AUTODOC: Save route entries */
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,
        PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS, &sipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);

    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,
        PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS, &dipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS);

    /* AUTODOC: Write new route entries */
    cpssOsMemSet(&routeEntry, 0, sizeof(PRV_TGF_IP_UC_ROUTE_ENTRY_STC));
    routeEntry.cmd = CPSS_PACKET_CMD_ROUTE_E;
    routeEntry.siteId = CPSS_IP_SITE_ID_INTERNAL_E;
    routeEntry.nextHopVlanId = PRV_TGF_SEND_VLANID_CNS;
    routeEntry.nextHopARPPointer = PRV_TGF_ARP_SRC_ENTRY_IDX_CNS;
    routeEntry.trapMirrorArpBcEnable = GT_TRUE;
    routeEntry.nextHopInterface.devPort.portNum =
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    routeEntry.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;

    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS,
        &routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);

    routeEntry.nextHopVlanId = PRV_TGF_NEXTHOP_VLANID_CNS;
    routeEntry.nextHopARPPointer = PRV_TGF_ARP_DST_ENTRY_IDX_CNS;
    routeEntry.nextHopInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_NEXTHOP_PORT_IDX_CNS];
    routeEntry.nextHopInterface.devPort.hwDevNum = prvTgfDevNum;
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS,
        &routeEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS);

    /* AUTODOC: Write new prefix rules */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    sip.arIP[0] = prvTgfPacketArpPart.srcIp[0];
    sip.arIP[1] = prvTgfPacketArpPart.srcIp[1];
    sip.arIP[2] = prvTgfPacketArpPart.srcIp[2];
    sip.arIP[3] = prvTgfPacketArpPart.srcIp[3];
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }

    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex =PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_TRUE;

    rc = prvTgfIpLpmIpv4UcPrefixAdd(PRV_TGF_LPM_DB_ID_CNS, 0, sip, 32,&nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd");

    dip.arIP[0] = prvTgfPacketArpPart.dstIp[0];
    dip.arIP[1] = prvTgfPacketArpPart.dstIp[1];
    dip.arIP[2] = prvTgfPacketArpPart.dstIp[2];
    dip.arIP[3] = prvTgfPacketArpPart.dstIp[3];
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS;
    rc = prvTgfIpLpmIpv4UcPrefixAdd(PRV_TGF_LPM_DB_ID_CNS, 0, dip, 32,&nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd");

    /* AUTODOC: Save the unicast RPF fail check state */
    rc = prvTgfIpBridgeServiceEnableGet(prvTgfDevNum,
        PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E,
        &ucRpfFailServiceEnabled);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpBridgeServiceEnableGet: Service=%d, Mode=%d\n",
        PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E);

    /* AUTODOC: Enable UC RPF fail check */
    prvTgfIpBridgeServiceEnable(prvTgfDevNum,
        PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E,
        GT_TRUE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "prvTgfIpBridgeServiceEnable: Service=%d, Mode=%d, Enable=%d\n",
        PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E,
        GT_TRUE);

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChLpmPortSipLookupEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &portSipLookupEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableGet");

        rc = cpssDxChLpmPortSipLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableSet");
    }

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E;
    rc = cpssDxChIpSpecialRouterTriggerEnableGet(prvTgfDevNum, bridgeExceptionCmd, &enableRouterTriggerArpTrapGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpSpecialRouterTriggerEnableGet");

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_SOFT_DROP_EXCP_CMD_E;
    rc = cpssDxChIpSpecialRouterTriggerEnableGet(prvTgfDevNum, bridgeExceptionCmd, &enableRouterTriggerArpSoftDropGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpSpecialRouterTriggerEnableGet");


    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E;
    enableRouterTrigger = GT_TRUE;

    rc = cpssDxChIpSpecialRouterTriggerEnable(prvTgfDevNum, bridgeExceptionCmd, enableRouterTrigger);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpSpecialRouterTriggerEnable");

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_SOFT_DROP_EXCP_CMD_E;
    enableRouterTrigger = GT_TRUE;

    rc = cpssDxChIpSpecialRouterTriggerEnable(prvTgfDevNum, bridgeExceptionCmd, enableRouterTrigger);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpSpecialRouterTriggerEnable");

    rc = prvTgfIpArpBcModeGet(&arpBcModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpArpBcModeGet");

    rc = prvTgfIpArpBcModeSet(CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpArpBcModeSet");


    rc = cpssDxChBrgVlanUnkUnregFilterSet(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                          CPSS_DXCH_BRG_VLAN_PACKET_UNREG_NON_IPV4_BCAST_E,
                                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChBrgVlanUnkUnregFilterSet");

    rc = cpssDxChBrgVlanUnkUnregFilterSet(prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS,
                                          CPSS_DXCH_BRG_VLAN_PACKET_UNREG_IPV4_BCAST_E,
                                          CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChBrgVlanUnkUnregFilterSet");


    /* AUTODOC: enable  IP control traffic trapping to CPU */
    rc = prvTgfBrgVlanIpCntlToCpuSet(PRV_TGF_SEND_VLANID_CNS,
                                     PRV_TGF_BRG_IP_CTRL_IPV4_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanIpCntlToCpuSet");

    /* AUTODOC: Save trapping to CPU ARP Broadcast packets for all VLANs */
    rc = prvTgfBrgGenArpBcastToCpuCmdGet(PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,&saveTrappingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgGenArpBcastToCpuCmdGet");

    /* AUTODOC: enable trapping to CPU ARP Broadcast packets for all VLANs */
    rc = prvTgfBrgGenArpBcastToCpuCmdSet(PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E, CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgGenArpBcastToCpuCmdSet");

    /* AUTODOC: Save the exception command for UC RPF Check fail */
    rc = cpssDxChIpExceptionCommandGet(prvTgfDevNum, CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E,CPSS_IP_PROTOCOL_IPV4_E,&exceptionCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChIpExceptionCommandGet");

     /* AUTODOC: Set the exception command for UC RPF Check fail */
    rc = cpssDxChIpExceptionCommandSet(prvTgfDevNum, CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E,CPSS_IP_PROTOCOL_IPV4_E,CPSS_PACKET_CMD_TRAP_TO_CPU_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChIpExceptionCommandGet");
}

/**
* @internal prvTgfIpArpRpfFailCheckGenerate function
* @endinternal
*
* @brief   AUTODOC: Perform the test:
*         UC RPF Fail check enabled, nexthopvlanID == packet vlan - packet passed
*         UC RPF Fail check enabled, nexthopvlanID != packet vlan  - packet dropped
*         UC RPF Fail check disabled, nexthopvlanID == packet vlan - packet passed
*         UC RPF Fail check disabled, nexthopvlanID != packet vlan - packet passed
*/
GT_VOID prvTgfIpArpRpfFailCheckGenerate
(
    GT_VOID
)
{
    GT_STATUS                       rc;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    TGF_PACKET_STC                  packetInfo;
    GT_U32                          portIter    = 0;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   packetArrivedCntrs;
    GT_ETHERADDR                    arpMacAddr;
    PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT nextHopInfo;
    PRV_TGF_IP_UC_ROUTE_ENTRY_STC   sipRouteEntryTobeChanged;
    GT_U32                          exceptionPacketCount = 0;
    GT_U32                          extraPacketCount=0;

    GT_BOOL     getFirst = GT_TRUE;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;


    /* Reset counters */
    resetCounters();

    /* reset Exception counters */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntSet(prvTgfDevNum,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntSet");

    /* AUTODOC: Form packet and enable capturing */
    /* Setup bridge portInterface for capturing */
    portInterface.type             = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum = prvTgfDevNum;
    portInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_BRIDGE_PORT_IDX_CNS];

    /* Enable capture on bridge port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
        TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorPortTxEthCaptureSet: port=%d\n",
         prvTgfPortsArray[PRV_TGF_BRIDGE_PORT_IDX_CNS]);

    /* Number of parts in packet */
    partsCount = sizeof(prvTgfPacketArpPartArray) /
        sizeof(prvTgfPacketArpPartArray[0]);

    /* Calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketArpPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet");

    /* Build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = prvTgfPacketArpPartArray;

    /* Setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo,
        PRV_TGF_BURST_COUNT_DEFAULT_CNS, 0,
        NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetup");

    /* Setup counters for arrived packet */
    packetArrivedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) *
        PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.goodPktsSent.l[0]   = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.ucPktsSent.l[0]     = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.brdcPktsSent.l[0]   = 0;
    packetArrivedCntrs.mcPktsSent.l[0]     = 0;
    packetArrivedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.goodPktsRcv.l[0]    = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.ucPktsRcv.l[0]      = PRV_TGF_BURST_COUNT_DEFAULT_CNS;
    packetArrivedCntrs.brdcPktsRcv.l[0]    = 0;
    packetArrivedCntrs.mcPktsRcv.l[0]      = 0;


    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");


   /* AUTODOC: Case with UC RPF Fail check enabled and nexthopvlanID == packet_vlan
      (values were set when nextHop was defined), routing is enabled on send_port and send_vlan */
    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* we expect packet to be trapped to CPU */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
        case PRV_TGF_BRIDGE_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

     /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

#ifdef ASIC_SIMULATION
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* for SIP4 devices , in case UC RPF Check is enabled
           there is a counter update also for the packet mirror
           to the CPU from the egress port

           for SIP5 devices we disabled this option by disabling SIP lookup on the recieved port
           for SIP4 we do not have this option

           ON board this do not happen - TBD check why
           */
        extraPacketCount=1;
    }
#endif

    /* Exception counter get - no UC RPF fail */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");

    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", 0+extraPacketCount,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(0+extraPacketCount, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 0+extraPacketCount, exceptionPacketCount);

    resetCounters();

    /* AUTODOC: Case with UC RPF Fail check enabled and nexthopvlanID != packet_vlan */

    /* Set wrong nexthopvlanID in the routeEntry */

    /* AUTODOC: NO Need to enable Unicast IPv4 Routing on sender port
                NO Need yo enable IPv4 Unicast Routing on sender Vlan
                UC RPF Check should be done also on  non routed packets */

    /* AUTODOC: Disable Unicast IPv4 Routing on sender port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnable: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Disable Unicast IPv4 Routing on sender vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpVlanRoutingEnable: vlanid=%d, enable=%d",
        PRV_TGF_SEND_VLANID_CNS, GT_FALSE);

    /* AUTODOC: Disable Router engine process of ARP Broadcast packets */
    rc = prvTgfIpArpBcModeSet(CPSS_PACKET_CMD_NONE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpArpBcModeSet");

     /* AUTODOC: Save route entries */
    rc = prvTgfIpUcRouteEntriesRead(prvTgfDevNum,
        PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS, &sipRouteEntryTobeChanged, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesRead: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);

    sipRouteEntryTobeChanged.nextHopVlanId = PRV_TGF_SEND_VLANID_CNS + 1; /* wrong vlan */

    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS,
        &sipRouteEntryTobeChanged, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);


    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

   /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* Packet is not trapped to CPU */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table - expect packet to be trapped to CPU
       due to RPF fail check */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");

    PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
    /* check CPU code */
    UTF_VERIFY_EQUAL0_STRING_MAC(CPSS_NET_IP_UC_RPF_FAIL_E, rxParam.cpuCode, "Wrong CPU Code");


    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       GT_FALSE, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* Exception counter get */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[bigger then %d], receivedValue[%d]\n", 0 ,exceptionPacketCount);/* expected value different from 0*/
    UTF_VERIFY_NOT_EQUAL2_STRING_MAC(0, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 0, exceptionPacketCount);
    resetCounters();

    /* reset Exception counters */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntSet(prvTgfDevNum,0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntSet");


    /* AUTODOC: Case with UC RPF Fail check disabled and nexthopvlanID != packet_vlan
       since we will not have UC RPF Fail we expect the packet to be routed --> enable routing on port and vlan */

    /* AUTODOC: Enable Unicast IPv4 Routing on sender port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnable: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: Enable Unicast IPv4 Routing on sender vlan */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpVlanRoutingEnable: vlanid=%d, enable=%d",
        PRV_TGF_SEND_VLANID_CNS, GT_FALSE);

    /* Disable check UC RPF Fail in prefix rules */
    cpssOsMemSet(&nextHopInfo, 0, sizeof(PRV_TGF_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    if (PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_REGULAR_ROUTE_ENTRY_GROUP_E;
    }
    else
    {
        nextHopInfo.ipLttEntry.routeType = PRV_TGF_IP_ECMP_ROUTE_ENTRY_GROUP_E;
    }
    nextHopInfo.ipLttEntry.numOfPaths = 0;
    nextHopInfo.ipLttEntry.routeEntryBaseIndex = PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS;
    nextHopInfo.ipLttEntry.ucRPFCheckEnable = GT_FALSE;

    rc = prvTgfIpLpmIpv4UcPrefixAdd(PRV_TGF_LPM_DB_ID_CNS, 0, sip, 32,
        &nextHopInfo, GT_TRUE, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixAdd");

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* Packet act according to bridge */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
        case PRV_TGF_BRIDGE_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table - there was no trap to CPU */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");

    /* Exception counter get  - no UC RPF Fail */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    if(!PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        /* for SIP4 devices , in case UC RPF Chcek is enabled
           there is a counter update also for the packet mirror
           to the CPU from the egress port

           for SIP5 devices we disabled this option by disabling SIP lookup on the recieved port
           for SIP4 we do not have this option
           */
        extraPacketCount=1;
    }
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", 0,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(0, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 0, exceptionPacketCount);

    resetCounters();

    /* AUTODOC: Case with UC RPF Fail check disabled and correct nextHopVlanId */
    /* Set back to nextHopVlanId correct and see packet passes */
    sipRouteEntryTobeChanged.nextHopVlanId = PRV_TGF_SEND_VLANID_CNS;

    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS,
        &sipRouteEntryTobeChanged, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);
    convertMac(prvTgfPacketL2Part.saMac, &arpMacAddr);

    /* clear table */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum,
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfStartTransmitting: port=%d\n",
         prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    cpssOsTimerWkAfter(200);

    /* Read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter],
            GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
            "prvTgfReadPortCountersEth: port=%d\n", prvTgfPortsArray[portIter]);

        /* Packet act accordig to bridge */
        switch (portIter)
        {
        case PRV_TGF_SEND_PORT_IDX_CNS:
        case PRV_TGF_BRIDGE_PORT_IDX_CNS:
            expectedCntrs = packetArrivedCntrs;
            break;
        default:
            expectedCntrs = emptyCounters;
            /* For other ports */
        }

        rc = countersEq(expectedCntrs, portCntrs);
        UTF_VERIFY_EQUAL0_STRING_MAC(rc, GT_TRUE,
            "Got another counters values.");

        /* Print expected values if bug */
        if (rc != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            printCounters(expectedCntrs);
        }
    }

    /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table - there was no trap to CPU */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuff,
                                       &buffLen, &packetActualLength,
                                       &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");


    /* Exception counter get  - no UC RPF Fail */
    rc = cpssDxChIpRouterBridgedPacketsExceptionCntGet(prvTgfDevNum,&exceptionPacketCount);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc , "Error in cpssDxChIpRouterBridgedPacketsExceptionCntGet");
    PRV_UTF_LOG2_MAC("Exception Count : expectedValue[%d], receivedValue[%d]\n", 0,exceptionPacketCount);
    UTF_VERIFY_EQUAL2_STRING_MAC(0, exceptionPacketCount, "ERROR in exception count: %d, %d\n",
                                 0, exceptionPacketCount);

    resetCounters();

    /* AUTODOC: Disable capture on bridge port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface,
        TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "tgfTrafficGeneratorPortTxEthCaptureSet: port=%d\n",
        prvTgfPortsArray[PRV_TGF_BRIDGE_PORT_IDX_CNS]);

}

/**
* @internal prvTgfIpArpRpfFailCheckRestore function
* @endinternal
*
* @brief   AUTODOC: Restore previous configuration
*         1. Restore UC RPF Fail check state
*         2. Delete prefix rules
*         3. Restore route entries
*         4. Restore ARP entries
*         5. Flush FDB
*         6. Disable routing in sender VLAN
*         7. Restore routing enabled state for sender port
*         8. Invalidate VLANs
*/
GT_VOID prvTgfIpArpRpfFailCheckRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_DXCH_IP_BRG_EXCP_CMD_ENT     bridgeExceptionCmd;

    /* AUTODOC: Restore the global routing enable state */
    rc =  cpssDxChIpRoutingEnable(prvTgfDevNum,globalRoutingEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"cpssDxChIpRoutingEnable:");

    /* AUTODOC: Restore the unicast RPF fail check state */
    rc = prvTgfIpBridgeServiceEnable(prvTgfDevNum,
        PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E,
        ucRpfFailServiceEnabled);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc,
        "prvTgfIpBridgeServiceEnableGet: Service=%d, Mode=%d, Enable=%d\n",
        PRV_TGF_IP_UC_RPF_CHECK_BRG_SERVICE_E,
        PRV_TGF_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E,
        ucRpfFailServiceEnabled);

    /* AUTODOC: Delete prefix rules */
    rc = prvTgfIpLpmIpv4UcPrefixDel(PRV_TGF_LPM_DB_ID_CNS, 0, sip, 32);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel");
    rc = prvTgfIpLpmIpv4UcPrefixDel(PRV_TGF_LPM_DB_ID_CNS, 0, dip, 32);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpLpmIpv4UcPrefixDel");

    /* AUTODOC: Restore route entries */
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS,
        &sipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_SIP_IDX_CNS);
    rc = prvTgfIpUcRouteEntriesWrite(PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS,
        &dipRouteEntry, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfIpUcRouteEntriesWrite: "
        "Idx=%d\n", PRV_TGF_ROUTE_ENTRY_DIP_IDX_CNS);

    /* AUTODOC: Restore entries in ARP table */
    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        &arpEntrySrc);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpEntrySrc.arEther[0], arpEntrySrc.arEther[1], arpEntrySrc.arEther[2],
        arpEntrySrc.arEther[3], arpEntrySrc.arEther[4], arpEntrySrc.arEther[5]);

    rc = prvTgfIpRouterArpAddrWrite(PRV_TGF_ARP_DST_ENTRY_IDX_CNS,
        &arpEntryDst);
    UTF_VERIFY_EQUAL7_STRING_MAC(GT_OK, rc, "prvTgfIpRouterArpAddrWrite: "
        "Idx=%d, Value=%02x:%02x:%02x:%02x:%02x:%02x\n",
        PRV_TGF_ARP_SRC_ENTRY_IDX_CNS,
        arpEntryDst.arEther[0], arpEntryDst.arEther[1], arpEntryDst.arEther[2],
        arpEntryDst.arEther[3], arpEntryDst.arEther[4], arpEntryDst.arEther[5]);

    /* AUTODOC: Flush FDB entries added */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitting: "
        "includeStatic=%d\n", GT_TRUE);

    /* AUTODOC: Disable UC routing on sender VLAN */
    rc = prvTgfIpVlanRoutingEnable(PRV_TGF_SEND_VLANID_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
        "prvTgfIpVlanRoutingEnable: vlanid=%d, enable=%d",
        PRV_TGF_SEND_VLANID_CNS, GT_FALSE);

    /* AUTODOC: Restore routing enabled state on sender port */
    rc = prvTgfIpPortRoutingEnable(PRV_TGF_SEND_PORT_IDX_CNS,
        CPSS_IP_UNICAST_E, CPSS_IP_PROTOCOL_IPV4_E, sndPortRoutingEnabled);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfIpPortRoutingEnable: port=%d",
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: restore default vlanId to all ports */
    rc = prvTgfBrgVlanPvidSet(prvTgfDefVlanId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPvidSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefVlanId);

    /* AUTODOC: Invalidate VLANs */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_SEND_VLANID_CNS);
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_NEXTHOP_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc,
        "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_NEXTHOP_VLANID_CNS);

    if(PRV_CPSS_SIP_5_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChLpmPortSipLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], portSipLookupEnable);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChLpmPortSipLookupEnableSet");
    }

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E;
    rc = cpssDxChIpSpecialRouterTriggerEnable(prvTgfDevNum, bridgeExceptionCmd, enableRouterTriggerArpTrapGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpSpecialRouterTriggerEnable");

    bridgeExceptionCmd = CPSS_DXCH_IP_BRG_ARP_SOFT_DROP_EXCP_CMD_E;
    rc = cpssDxChIpSpecialRouterTriggerEnable(prvTgfDevNum, bridgeExceptionCmd, enableRouterTriggerArpSoftDropGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpSpecialRouterTriggerEnable");

    rc = prvTgfIpArpBcModeSet(arpBcModeGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpArpBcModeGet");

    /* AUTODOC: restore trapping to CPU ARP Broadcast packets for all VLANs */
    rc = prvTgfBrgGenArpBcastToCpuCmdSet(PRV_TGF_BRG_ARP_BCAST_CMD_MODE_VLAN_E,
                                         saveTrappingCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "prvTgfBrgGenArpBcastToCpuCmdSet: %d");

   /* AUTODOC: Restore the exception command for UC RPF Check fail */
    rc = cpssDxChIpExceptionCommandSet(prvTgfDevNum, CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E,CPSS_IP_PROTOCOL_IPV4_E,exceptionCmdGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                 "cpssDxChIpExceptionCommandGet");
}


