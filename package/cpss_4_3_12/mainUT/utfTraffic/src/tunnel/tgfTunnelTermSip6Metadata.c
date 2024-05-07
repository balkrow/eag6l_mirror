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
* @file tgfTunnelTermSip6Metadata.c
*
* @brief Verify the functionality of SIP6 TTI metadata fields
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <utf/private/prvUtfExtras.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <tunnel/tgfTunnelTermSip6Metadata.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfTunnelGen.h>
#include <common/tgfPacketGen.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_CNS         5

/* default VLAN Id */
#define PRV_TGF_EGR_VLANID_CNS          6

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR_PORT_IDX_CNS        2

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

/* tti ipv6 extended header id */
#define PRV_TGF_TTI_IPV6_EXT_HEADER_ID_CNS  0

/* tti ipv6 extended header value */
#define PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_1_CNS  1

#define PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_2_CNS  0

#define PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_3_CNS  43 /* extention header routing */

/* the TTI Rule index */
#define PRV_TGF_TTI0_INDEX0_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)
#define PRV_TGF_TTI0_INDEX1_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 6)

/* default number of packets to send */
static GT_U32 prvTgfBurstCount = 1;

/*  IPv6 Template */
static TGF_PACKET_STC packetInfo;

/* TTI non HBH error command */
static CPSS_PACKET_CMD_ENT exceptionCmd;

/******************************* Test packet **********************************/

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
/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_86DD_IPV6_TAG_CNS};
/* packet's IPv6 */
static TGF_PACKET_IPV6_STC prvTgfPacketIpv6Part = {
    6,                  /* version */
    0,                  /* trafficClass */
    0,                  /* flowLabel */
    0x02,               /* payloadLen */
    PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_1_CNS, /* nextHeader */
    0x40,               /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0x1122, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    43,                                  /* Next header */
    0x00,                                /* Entension header length */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  /* Options and padding */
    0x11,                   /* Next header - Authentication Header*/
    0x02,                   /* Entension header length */
    0x02,                   /* Routing Type */
    0x01,                   /* Segmets Left */
    0x00, 0x00, 0x00, 0x00, /* Reserved */
    0x11, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xbb, 0xaa, 0xee, 0xff, /* IPv6 Address */
    /* UDP */
    0xB3, 0xAA, 0xC3, 0x50, 0x00, 0x48, 0xBE, 0x8F,                                                 /*UDP Header: srcPort=0xB3AA=45994 destPort=0xC350=50000*/
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B,                         /*payload*/
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,                         /*payload*/
    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23,                         /*payload*/
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,                         /*payload*/
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,                         /*payload*/
    0x3C, 0x3D, 0x3E, 0x3F
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr1[] = {
    59,                     /* Next header */
    0x02,                   /* Entension header length */
    0x04,                   /* Routing Type */
    0x01,                   /* Segmets Left */
    0x00, 0x00, 0x00, 0x00, /* Reserved */
    0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0x00, 0x99, 0xbb, 0xaa, 0xee, 0xff, /* IPv6 Address */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         /*payload*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         /*payload*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         /*payload*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         /*payload*/
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                         /*payload*/
    0x00, 0x00, 0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart1 = {
    sizeof(prvTgfPayloadDataArr1),                       /* dataLength */
    prvTgfPayloadDataArr1                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray1[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV6_E,      &prvTgfPacketIpv6Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart1}
};


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal tgfTunnelTermSip6MetadataBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID tgfTunnelTermSip6MetadataBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_EGR_VLANID_CNS, prvTgfPortsArray + 2,
                                           NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermSip6MetadataTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_VOID tgfTunnelTermSip6MetadataTtiConfigSet
(
    IN GT_U32  extensionHeader
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction, ttiAction2;
    PRV_TGF_TTI_RULE_UNT     ttiPattern, ttiPattern2;
    PRV_TGF_TTI_RULE_UNT     ttiMask, ttiMask2;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiPattern2, 0, sizeof(ttiPattern2));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiMask2,    0, sizeof(ttiMask2));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));
    cpssOsMemSet((GT_VOID*) &ttiAction2, 0, sizeof(ttiAction2));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern and Mask */
    ttiPattern.udbArray.udb[0] = 0x80; /* L4 is valid */
    ttiPattern.udbArray.udb[1] = PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_1_CNS;
    ttiPattern.udbArray.udb[2] = (GT_U8)prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiPattern.udbArray.udb[3] = (GT_U8)((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] >> 8) & 0xFF);
    if (extensionHeader == 3)
    {
        ttiPattern.udbArray.udb[4] = 0x01; /* Holds the Segment Routing Extension Header <Segment Left> field */
        ttiPattern.udbArray.udb[5] = 0x01; /* Indicates that Segment Routing Extension Header exists in the packet's header */
    }

    ttiMask.udbArray.udb[0] = 0x80;
    ttiMask.udbArray.udb[1] = 0xFF;
    ttiMask.udbArray.udb[2] = 0xFF;
    ttiMask.udbArray.udb[3] = 0x03;
    if (extensionHeader == 3)
    {
        ttiMask.udbArray.udb[1] = 0x00;
        ttiMask.udbArray.udb[4] = 0xFF;
        ttiMask.udbArray.udb[5] = 0xFF;
    }

    /* AUTODOC: set the TTI Rule Action for matched packet (not l4 valid) */
    ttiAction.command                   = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    ttiAction.userDefinedCpuCode        = CPSS_NET_FIRST_USER_DEFINED_E + 1;
    if (extensionHeader == 3)
    {
        ttiAction.userDefinedCpuCode        = CPSS_NET_FIRST_USER_DEFINED_E + 3;
    }

    ttiPattern2.udbArray.udb[0] = 0x80; /* L4 is valid */
    ttiPattern2.udbArray.udb[1] = 0x11; /* IPvX protocol */
    ttiPattern2.udbArray.udb[2] = (GT_U8)prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    ttiPattern2.udbArray.udb[3] = (GT_U8)((prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] >> 8) & 0xFF);
    ttiPattern2.udbArray.udb[4] = 0xB3; /* UDP source port*/
    ttiPattern2.udbArray.udb[5] = 0xAA;

    ttiMask2.udbArray.udb[0] = 0x80; /*0x80*/
    ttiMask2.udbArray.udb[1] = 0xFF; /*0xFF*/
    ttiMask2.udbArray.udb[2] = 0xFF;
    ttiMask2.udbArray.udb[3] = 0x03;
    ttiMask2.udbArray.udb[4] = 0x00; /* UDP source port mask */
    ttiMask2.udbArray.udb[5] = 0x00;

    ttiAction2.command                   = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    ttiAction2.userDefinedCpuCode        = CPSS_NET_FIRST_USER_DEFINED_E + 2;

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                        0, PRV_TGF_TTI_OFFSET_METADATA_E, 21);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 36 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                        1, PRV_TGF_TTI_OFFSET_METADATA_E, 36);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                        2, PRV_TGF_TTI_OFFSET_METADATA_E, 34);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                        3, PRV_TGF_TTI_OFFSET_METADATA_E, 35);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    if (extensionHeader == 3)
    {
        /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 32 */
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                            4, PRV_TGF_TTI_OFFSET_METADATA_E, 32);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

        /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 33 */
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                            5, PRV_TGF_TTI_OFFSET_METADATA_E, 33);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    }

        /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                        0, PRV_TGF_TTI_OFFSET_METADATA_E, 21);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 36 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                        1, PRV_TGF_TTI_OFFSET_METADATA_E, 36);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                        2, PRV_TGF_TTI_OFFSET_METADATA_E, 34);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                        3, PRV_TGF_TTI_OFFSET_METADATA_E, 35);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                        4, PRV_TGF_TTI_OFFSET_L4_E, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                        5, PRV_TGF_TTI_OFFSET_L4_E, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        PRV_TGF_TTI_KEY_UDB_IPV6_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key PRV_TGF_TTI_KEY_UDB_IPV6_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key PRV_TGF_TTI_KEY_UDB_IPV6_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_IPV6_E on port 0 VLAN 5 with action: redirect to egress */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX0_CNS, PRV_TGF_TTI_KEY_UDB_IPV6_E,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_IPV6_E on port 0 VLAN 5 with action: redirect to egress */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX1_CNS, PRV_TGF_TTI_KEY_UDB_IPV6_UDP_E,
                                       &ttiPattern2, &ttiMask2, &ttiAction2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");

    /* AUTODOC: save IPv6 TTI non HBH error command (for configuration restore) */
    rc = prvTgfTtiExceptionCmdGet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E,
                                                        &exceptionCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdGet");

    /* AUTODOC: set IPv6 TTI non HBH error command CPSS_PACKET_CMD_FORWARD_E (prevent packet drop) */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E,
                                                        CPSS_PACKET_CMD_FORWARD_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");
}


/**
* @internal tgfTunnelTermSip6MetadataTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] expectL4Valid            - GT_TRUE: expect L4 is valid
*                                       GT_FALSE: expect L4 is invalid
*
*/
GT_VOID tgfTunnelTermSip6MetadataTrafficGenerate
(
    IN GT_U32  extensionHeader
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          portIter    = 0;
    CPSS_INTERFACE_INFO_STC         portInterface;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    GT_U32      expectedCpuCode;

    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

     /* AUTODOC: build IPv6 Packet */
    if (extensionHeader == 3)
    {
        packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray1) / sizeof(prvTgfPacketPartArray1[0]);
        packetInfo.partsArray = prvTgfPacketPartArray1;
        packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    }
    else
    {
        packetInfo.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
        packetInfo.partsArray = prvTgfPacketPartArray;
        packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    }

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: setup egress portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS];

    /* AUTODOC: enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: start Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    if (extensionHeader == 2)
    {
        prvTgfPacketIpv6Part.nextHeader = PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_2_CNS;
        expectedCpuCode = 502;
    }
    else if (extensionHeader == 1)
    {
        prvTgfPacketIpv6Part.nextHeader = PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_1_CNS;
        expectedCpuCode = 501;
    }
    else
    {
        prvTgfPacketIpv6Part.nextHeader = PRV_TGF_TTI_IPV6_EXT_HEADER_VALUE_3_CNS;
        expectedCpuCode = 503;
    }

    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, prvTgfBurstCount, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled ipv6 packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* AUTODOC: disable capture on nexthope port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_EGR_PORT_IDX_CNS]);

    /* AUTODOC: stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                    GT_TRUE, GT_TRUE, packetBuff,
                                    &buffLen, &packetActualLength,
                                    &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                "tgfTrafficGeneratorRxInCpuGet");

    PRV_UTF_LOG1_MAC("CPU Code %d\n", rxParam.cpuCode);
    /* check CPU code */
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedCpuCode, rxParam.cpuCode, "Wrong CPU Code");

    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                    GT_FALSE, GT_TRUE, packetBuff,
                                    &buffLen, &packetActualLength,
                                    &devNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc, "tgfTrafficGeneratorRxInCpuGet");
    return;
}

/**
* @internal tgfTunnelTermSip6MetadataConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
GT_VOID tgfTunnelTermSip6MetadataConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_EGR_VLANID_CNS);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MPLS */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_IPV6_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX0_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX1_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /* AUTODOC: restore IPv6 TTI non HBH error command */
    rc = prvTgfTtiExceptionCmdSet(prvTgfDevNum, PRV_TGF_TTI_EXCEPTION_IPV6_NON_HBH_ERROR_E,
                                                        exceptionCmd);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiExceptionCmdSet");

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal tgfTunnelTermSip6Metadata function
* @endinternal
*
* @brief   Test SIP6 TTI metadata fields
*/
GT_VOID tgfTunnelTermSip6Metadata
(
    GT_VOID
)
{
    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    /* AUTODOC: create VLAN 6 with untagged ports [2,3] */
    tgfTunnelTermSip6MetadataBridgeConfigSet();

    /* AUTODOC: add TTI rule 1 with Ipv6 UDB key on port 0 VLAN 5 with */
    /* AUTODOC: TTI action: modify vid to 6, redirect to port 36 */

    /* AUTODOC: set the TTI Rule Pattern and Mask to match packets with <isL4Valid> is not set*/
    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_IPV6_E: anchor Metadata, offset 21 */
    /* AUTODOC: set the TTI Rule Action for matched packet (not l4 valid) */
    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_E at the port 0 */
    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key PRV_TGF_TTI_KEY_UDB_IPV6_E */
    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_IPV6_E on port 0 VLAN 5 with action: redirect to egress */
    tgfTunnelTermSip6MetadataTtiConfigSet(1);

    /*------------------------------------------------------------*/
    /* AUTODOC: send packet and expect TTI match rule with IPvX protocol 0x01 */
    tgfTunnelTermSip6MetadataTrafficGenerate(1);

    /* AUTODOC: send packet and expect TTI match rule with IPvX protocol 0x11 */
    tgfTunnelTermSip6MetadataTrafficGenerate(2);

    /* AUTODOC: change UDB setting for SRH packet */
    tgfTunnelTermSip6MetadataTtiConfigSet(3);

    /* AUTODOC: send packet and expect TTI match rule with IPv6 Segment routing header */
    tgfTunnelTermSip6MetadataTrafficGenerate(3);

    /*------------------------------------------------------------*/
    /* AUTODOC: test configuration restore */
    tgfTunnelTermSip6MetadataConfigRestore();
}

/*============================================================================================*/
/* VID1 Exist for 8-byte TAG1 */

/* DATA of packet */
static GT_U8 prvTgfTag1DataArrSip6_10Tag1ExistMatched[] = {
    /* VLAN TPID 0x8909, pattern oxAAAAAAAAA, UP1 = 5, CFI1 = 1, VID1 = 7 */
    0x89, 0x09, 0xAA, 0xAA, 0xAA, 0xAA, 0x90, 0x07
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketTag1PartSip6_10Tag1ExistMatched = {
    sizeof(prvTgfTag1DataArrSip6_10Tag1ExistMatched),                       /* dataLength */
    prvTgfTag1DataArrSip6_10Tag1ExistMatched                                /* dataPtr */
};

/* DATA of packet */
static GT_U8 prvTgfTag1DataArrSip6_10Tag1ExistUnmatched[] = {
    /* VLAN TPID 0x8909, pattern oxAAAAAAAAA, UP1 = 5, CFI1 = 1, VID1 = 7 */
    0x89, 0x09, 0x11, 0x11, 0x11, 0x11, 0x90, 0x07
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketTag1PartSip6_10Tag1ExistUnmatched = {
    sizeof(prvTgfTag1DataArrSip6_10Tag1ExistUnmatched),                       /* dataLength */
    prvTgfTag1DataArrSip6_10Tag1ExistUnmatched                                /* dataPtr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArrSip6_10Tag1Exist[] = {
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
    0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
    0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPartSip6_10Tag1Exist = {
    sizeof(prvTgfPayloadDataArrSip6_10Tag1Exist),                       /* dataLength */
    prvTgfPayloadDataArrSip6_10Tag1Exist                                /* dataPtr */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePartSip6_10Tag1Exist = {0x3333};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArraymSip6_10Tag1ExistMatched[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketTag1PartSip6_10Tag1ExistMatched},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartSip6_10Tag1Exist},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPartSip6_10Tag1Exist}
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArraymSip6_10Tag1ExistUnmatched[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketTag1PartSip6_10Tag1ExistUnmatched},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePartSip6_10Tag1Exist},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPartSip6_10Tag1Exist}
};


/**
* @internal tgfTunnelTermSip6_10Tag1ExistTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] toMatch - GT_TRUE: matched packet
*                      GT_FALSE: unmatched packet
*
*/
static GT_VOID tgfTunnelTermSip6_10Tag1ExistTrafficGenerate
(
    IN GT_BOOL  toMatch
)
{
    GT_STATUS                       rc          = GT_OK;
    TGF_PACKET_STC                  pktInfo;
    GT_U32                          portIter    = 0;
    GT_U8       packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32      buffLen  = TGF_RX_BUFFER_MAX_SIZE_CNS;
    GT_U32      packetActualLength = 0;
    GT_U8       devNum;
    GT_U8       queue;
    TGF_NET_DSA_STC rxParam;
    /* AUTODOC: GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic =======\n");

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* AUTODOC: reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

     /* AUTODOC: build IPv6 Packet */

    pktInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    if (toMatch != GT_FALSE)
    {
        pktInfo.numOfParts =
            sizeof(prvTgfPacketPartArraymSip6_10Tag1ExistMatched)
            / sizeof(prvTgfPacketPartArraymSip6_10Tag1ExistMatched[0]);
        pktInfo.partsArray = prvTgfPacketPartArraymSip6_10Tag1ExistMatched;
    }
    else
    {
        pktInfo.numOfParts =
            sizeof(prvTgfPacketPartArraymSip6_10Tag1ExistUnmatched)
            / sizeof(prvTgfPacketPartArraymSip6_10Tag1ExistUnmatched[0]);
        pktInfo.partsArray = prvTgfPacketPartArraymSip6_10Tag1ExistUnmatched;
    }

    /* start capture */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");

    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    /* AUTODOC: setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &pktInfo, 1/*prvTgfBurstCount*/, 0, NULL);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* AUTODOC: send tunneled ipv6 packet from port 0 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_ANY_E,
                                    GT_TRUE, GT_TRUE, packetBuff,
                                    &buffLen, &packetActualLength,
                                    &devNum, &queue, &rxParam);
    if (toMatch != GT_FALSE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                    "tgfTrafficGeneratorRxInCpuGet");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                    "tgfTrafficGeneratorRxInCpuGet");
    }
    /* stop capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @internal tgfTunnelTermSip6_10Tag1ExistBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
static GT_VOID tgfTunnelTermSip6_10Tag1ExistBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC   parseConfig;
    GT_U32 ii;
    GT_U16 etherType;

    etherType = (GT_U16)
        (prvTgfTag1DataArrSip6_10Tag1ExistMatched[1]
        | (prvTgfTag1DataArrSip6_10Tag1ExistMatched[0] << 8));

    PRV_UTF_LOG0_MAC("======= Setting Bridge Configuration =======\n");

    /* AUTODOC: create VLAN 5 with untagged ports [0,1] */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_CNS, prvTgfPortsArray, NULL, NULL, 2);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    rc = cpssDxChBrgVlanTpidEntrySet(
        prvTgfDevNum, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 6/*entryIndex*/,  etherType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidEntrySet: %d", prvTgfDevNum);

    rc = cpssDxChBrgVlanTpidTagTypeSet(
        prvTgfDevNum, CPSS_DIRECTION_INGRESS_E, 6/*index*/,CPSS_BRG_TPID_SIZE_TYPE_8_BYTES_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanTpidTagTypeSet: %d", prvTgfDevNum);

    parseConfig.hsrTag = GT_FALSE;
    parseConfig.enable = GT_TRUE;
    parseConfig.extractedValueOffset = 4;
    for (ii = 0; (ii < 6); ii++)
    {
        parseConfig.tagTciMask[ii]  = prvTgfTag1DataArrSip6_10Tag1ExistMatched[ii + 2];
        parseConfig.tagTciValue[ii] = 0xFF;
    }

    rc = cpssDxChBrgVlanPort8BTagInfoSet(
        prvTgfDevNum, CPSS_DIRECTION_INGRESS_E, 6/*tpidEntryIndex*/, &parseConfig);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChBrgVlanPort8BTagInfoSet: %d", prvTgfDevNum);
}

/**
* @internal tgfTunnelTermSip6_10Tag1ExistTtiConfigSet function
* @endinternal
*
* @brief   build TTI Basic rule
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on HW error
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - on NULL pointer
*/
static GT_VOID tgfTunnelTermSip6_10Tag1ExistTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_2_STC ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set the TTI Rule Pattern and Mask */
    ttiPattern.udbArray.udb[0] = 0x01; /* Tag1 exist */
    ttiMask.udbArray.udb[0] = 0x01;

    /* AUTODOC: set the TTI Rule Action for matched packet (not l4 valid) */
    ttiAction.command                   = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    ttiAction.userDefinedCpuCode        = CPSS_NET_FIRST_USER_DEFINED_E + 1;

    /* AUTODOC: set UDB byte for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E: anchor Metadata, offset 16 */
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                        0, PRV_TGF_TTI_OFFSET_METADATA_E, 16);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);

    /* AUTODOC: enable the TTI lookup for PRV_TGF_TTI_KEY_UDB_IPV6_E at the port 0 */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                        PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet");

    /* AUTODOC: set key size PRV_TGF_TTI_KEY_SIZE_30_B_E for the key PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPacketTypeKeySizeSet(prvTgfDevNum, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, PRV_TGF_TTI_KEY_SIZE_30_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: add TTI rule 1 with PRV_TGF_TTI_KEY_UDB_IPV6_E on port 0 VLAN 5 with action: redirect to egress */
    rc = prvTgfTtiRule2Set(PRV_TGF_TTI0_INDEX0_CNS, PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                       &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}

/**
* @internal tgfTunnelTermSip6_10Tag1ExistConfigRestore function
* @endinternal
*
* @brief   Restore Global test configuration
*/
static GT_VOID tgfTunnelTermSip6_10Tag1ExistConfigRestore
(
    GT_VOID
)
{
    CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC   parseConfig;

    /* AUTODOC: invalidate vlan entries (and reset vlan entry) */
    prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SEND_VLANID_CNS);

    cpssDxChBrgVlanTpidEntrySet(
        prvTgfDevNum, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, 6/*entryIndex*/,  0x8100/*etherType*/);

    cpssDxChBrgVlanTpidTagTypeSet(
        prvTgfDevNum, CPSS_DIRECTION_INGRESS_E, 6/*index*/,CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E);

    cpssOsMemSet(&parseConfig, 0, sizeof(parseConfig));
    cpssDxChBrgVlanPort8BTagInfoSet(
        prvTgfDevNum, CPSS_DIRECTION_INGRESS_E, 0/*tpidEntryIndex*/, &parseConfig);

    /* AUTODOC: disable TTI lookup for port 0, key TTI_KEY_MPLS */
    prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
        PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_FALSE);

    /* AUTODOC: invalidate TTI rules */
    prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX0_CNS, GT_FALSE);

    /* AUTODOC: flush FDB include static entries */
    prvTgfBrgFdbFlush(GT_TRUE);

    /* stop capture */
    tgfTrafficTableRxStartCapture(GT_FALSE);

    /* AUTODOC: clear capturing RxPcktTable */
    tgfTrafficTableRxPcktTblClear();
}

/**
* @internal tgfTunnelTermSip6_10Tag1Exist function
* @endinternal
*
* @brief   Test SIP6_10 TTI metadata field Tag1Exist
*/
GT_VOID tgfTunnelTermSip6_10Tag1Exist
(
    GT_VOID
)
{
    tgfTunnelTermSip6_10Tag1ExistBridgeConfigSet();

    tgfTunnelTermSip6_10Tag1ExistTtiConfigSet();

    tgfTunnelTermSip6_10Tag1ExistTrafficGenerate(GT_TRUE);

    tgfTunnelTermSip6_10Tag1ExistTrafficGenerate(GT_FALSE);

    tgfTunnelTermSip6_10Tag1ExistConfigRestore();
}

