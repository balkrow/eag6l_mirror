/*******************************************************************************
*              (C), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfCfgReservedFlows.c
*
* @brief    Test PHA threads for Reserved flows
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficEngine.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfCscdGen.h>

#include <config/prvTgfCfgReservedFlows.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* Reserved flow test vlan id */
#define PRV_TGF_RESERVED_FLOW_TEST_VID_CNS    5

/* Target Eport */
#define PRV_TGF_CFG_TARGET_EPORT_CNS    _1K

/* port to send traffic to */
#define PRV_TGF_CFG_SEND_PORT_IDX_CNS   0

/* port number to receive traffic from */
#define PRV_TGF_CFG_RECV_PORT_IDX_CNS   1

/* test ports */
#define PRV_TGF_CFG_PORT2_IDX_CNS       2
#define PRV_TGF_CFG_PORT3_IDX_CNS       3

/* next hop port num */
#define PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS    PRV_TGF_CFG_PORT3_IDX_CNS

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_L2_INDEX_CNS         10

/* MAC SA global index used
 * to select TS MAC SA during HA
*/
#define PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS           7

/* PCL copyReservedMask to set */
#define PRV_TGF_COPY_RESERVED_MASK                ((PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE) ? 0x7FFFF : 0x3FFFF)

/* Reserved eth type */
#define PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS     0x1701

/* Packet payload length */
#define PRV_TGF_RESERVED_FLOWS_PACKET_PAYLOAD_LEN     0x30

/* Reserved flows field MACROs */
#define PRV_TGF_RESERVED_FLOWS_MAX_M              0x1
#define PRV_TGF_RESERVED_FLOWS_MAX_X              0x3
#define PRV_TGF_RESERVED_FLOWS_MAX_Y              0x7
#define PRV_TGF_RESERVED_FLOWS_MAX_W              0xFFFF
#define PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT      0xFF
#define PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL     0xFFFF
#define PRV_TGF_RESERVED_FLOWS_MAX_K              0xFFFF
#define PRV_TGF_RESERVED_FLOWS_MAX_Z              0x1FFFFF
#define PRV_TGF_RESERVED_FLOWS_MAX_LOOKUP_KEY     0x1FFF

#define PRV_TGF_RESERVED_FLOWS_MAX_BYTE_VAL       0xFF

/* MAC addr size in bytes */
#define PRV_TGF_L2_MAC_ADDR_SIZE_CNS              (TGF_L2_HEADER_SIZE_CNS >> 1)

/* Number of flows corresponding to thread id */
#define PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_MAX_FLOWS      2
#define PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_MAX_FLOWS      2
#define PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_MAX_FLOWS      4

/* Router SA Modification thread ID */
#define PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS  90

/* INQA ECN thread ID */
#define PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS                91

/* VXLAN DCI VNI REMAP thread ID */
#define PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS     92

/* MPLS LSR In Stacking System thread ID */
#define PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS     93

/* max target device value. For random generation*/
#define PRV_TGF_MAX_TRG_DEV                               _1K

#define PRV_TGF_TCP_UDP_ENTRY_INDEX                       0
#define PRV_TGF_UDP_VXLAN_PORT_NUM                        4789

#define PRV_TGF_PORT_COUNT_CNS                            4

/* default number of packets to send */
static GT_U32                       prvTgfBurstCount = 3;

/* copyReserved to set for three different packets 0:MAX, 1:MIN and 2:RANDOM */
static GT_U32                       copyReservedArray[] = {0x3FFFE, 0x00, 0x11};

/* flowId to set for three different packets 0:MIN, 1:MAX and 2:RANDOM
 * MAX 13 bits are allowed for flowId
*/
static GT_U32                       flowIdArray[] = {0x00, 0x1FFF, 0x12};

/* macSaMsb to set for three different packets 0:MAX, 1:MIN and 2:RANDOM
 * using 2 bytes only as macSaLsb is 4 bytes
 */
static GT_U32                       macSaMsbArray[] = {0xFFFF, 0x0000, 0x12};

/* macSaLsb to set for three different packets 0:MIN, 1:MAX and 2:RANDOM */
static GT_U32                       macSaLsbArray[] = {0x00, 0xFFFFFFFF, 0x12};

/* INQA ECN and Reserved Flag to set for three different packets 0:MIN, 1:MAX and 2:RANDOM
 * ECN = inqaEcnArray[index] bits[1:0] and reserved flag = inqaEcnArray[index] bit[2]*/
static GT_U32                       inqaEcnArray[] = {0x00, 0x07, 0x02};

/* vxlanVni to set for three different packets 0:MIN, 1:MAX and 2:RANDOM */
static GT_U32                       vxlanVniArray[] = {0x00, 0xFFFFFF, 0x12};
static GT_U32                       oldVxlanVni;

/* MPLS LSR In Stacking System prvTgfTrgDev, ownHwDevNum and targetDevSelectBit */
static GT_HW_DEV_NUM                prvTgfTrgDev;
static GT_HW_DEV_NUM                ownHwDevNum;
static GT_U32                       targetDevSelectBit;

/************************* General packet's parts *****************************/

/* Array of three different packet Rx0 parts */
static TGF_PACKET_RX0_STC prvTgfCfgPacketRx0PartArray[] = {
    /* Packet 1 RX0 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        PRV_TGF_RESERVED_FLOWS_MAX_M,          /* MAX_M     */
        0x00,                                  /* pType     */
        0x00,                                  /* X         */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,          /* MAX_Y     */
        0x03,                                  /* lookupKey */
        0x0000                                 /* W         */
    },
    /* Packet 2 RX0 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        0x00,                                  /* M         */
        0x00,                                  /* pType     */
        PRV_TGF_RESERVED_FLOWS_MAX_X,          /* MAX_X     */
        0x00,                                  /* Y         */
        0x03,                                  /* lookupKey */
        PRV_TGF_RESERVED_FLOWS_MAX_W           /* MAX_W     */
    },
    /* Packet 3 RX0 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        0x00,                                  /* M         */
        0x00,                                  /* pType     */
        0x00,                                  /* X         */
        0x00,                                  /* Y         */
        0x03,                                  /* lookupKey */
        0x1234                                 /* W random  */
    }
};

/* Array of three different packet Rx1 parts */
static TGF_PACKET_RX1_STC prvTgfCfgPacketRx1PartArray[] = {
    /* Packet 1 RX1 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType     */
        PRV_TGF_RESERVED_FLOWS_MAX_M,          /* MAX_M         */
        0x02,                                  /* pType         */
        0x00,                                  /* X             */
        PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT,  /* MAX_hopLimit  */
        0x0000,                                /* frwdLabel     */
        PRV_TGF_RESERVED_FLOWS_MAX_K,          /* MAX_K         */
        0x00,                                  /* Y             */
        0x03,                                  /* lookupKey     */
        0x0000                                 /* W             */
    },
    /* Packet 2 RX1 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType     */
        0x00,                                  /* M             */
        0x02,                                  /* pType         */
        PRV_TGF_RESERVED_FLOWS_MAX_X,          /* MAX_X         */
        0x00,                                  /* hopLimit      */
        PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL, /* MAX_frwdLabel */
        0x0000,                                /* K             */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,          /* Y             */
        0x03,                                  /* lookupKey     */
        PRV_TGF_RESERVED_FLOWS_MAX_W           /* MAX_W         */
    },
    /* Packet 3 RX1 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType     */
        0x00,                                  /* M             */
        0x02,                                  /* pType         */
        0x00,                                  /* X             */
        0x00,                                  /* hopLimit      */
        0x0000,                                /* frwdLabel     */
        0x0000,                                /* K             */
        0x00,                                  /* Y             */
        0x03,                                  /* lookupKey     */
        0x1234                                 /* W random      */
    }
};

/* Array of three different packet Rx2 parts */
static TGF_PACKET_RX2_STC prvTgfCfgPacketRx2PartArray[] = {
    /* Packet 1 RX2 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        PRV_TGF_RESERVED_FLOWS_MAX_M,          /* MAX_M     */
        0x06,                                  /* pType     */
        0x00,                                  /* X         */
        PRV_TGF_RESERVED_FLOWS_MAX_K,          /* MAX_K     */
        0x00,                                  /* Y         */
        PRV_TGF_RESERVED_FLOWS_MAX_Z,          /* MAX_Z     */
        0x0000                                 /* W         */
    },
    /* Packet 2 RX2 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        0x00,                                  /* M         */
        0x06,                                  /* pType     */
        PRV_TGF_RESERVED_FLOWS_MAX_X,          /* MAX_X     */
        0x00,                                  /* K         */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,          /* MAX_Y     */
        0x00,                                  /* Z         */
        PRV_TGF_RESERVED_FLOWS_MAX_W           /* MAX_W     */
    },
    /* Packet 3 RX2 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        0x00,                                  /* M         */
        0x06,                                  /* pType     */
        0x00,                                  /* X         */
        PRV_TGF_RESERVED_FLOWS_MAX_K,          /* MAX_K     */
        0x00,                                  /* Y         */
        PRV_TGF_RESERVED_FLOWS_MAX_Z,          /* MAX_Z     */
        0x1234                                 /* W random  */
    }
};

/* Array of three different packet Rx3 parts */
static TGF_PACKET_RX3_STC prvTgfCfgPacketRx3PartArray[] = {
    /* Packet 1 RX3 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType     */
        PRV_TGF_RESERVED_FLOWS_MAX_M,          /* MAX_M         */
        0x07,                                  /* pType         */
        0x00,                                  /* X             */
        PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT,  /* MAX_hopLimit  */
        0x0000,                                /* frwdLabel     */
        PRV_TGF_RESERVED_FLOWS_MAX_K,          /* MAX_K         */
        0x00,                                  /* Y             */
        PRV_TGF_RESERVED_FLOWS_MAX_Z,          /* MAX_Z         */
        0x0000                                 /* W             */
    },
    /* Packet 2 RX3 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType     */
        0x00,                                  /* M             */
        0x07,                                  /* pType         */
        PRV_TGF_RESERVED_FLOWS_MAX_X,          /* MAX_X         */
        0x00,                                  /* hopLimit      */
        PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL, /* MAX_frwdLabel */
        0x0000,                                /* K             */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,          /* Y             */
        0x00,                                  /* Z             */
        PRV_TGF_RESERVED_FLOWS_MAX_W           /* MAX_W         */
    },
    /* Packet 3 RX3 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType     */
        0x00,                                  /* M             */
        0x07,                                  /* pType         */
        0x00,                                  /* X             */
        0x00,                                  /* hopLimit      */
        0x0000,                                /* frwdLabel     */
        0x0000,                                /* K             */
        0x00,                                  /* Y             */
        0x03,                                  /* Z             */
        0x1234                                 /* W random      */
    }
};

/* Array of three different packet RXP6 parts */
static TGF_PACKET_RXP_Type_6_STC prvTgfCfgPacketRx6PartArray[] = {

    /* Packet 1 RX6 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        PRV_TGF_RESERVED_FLOWS_MAX_M,          /* M:1       */
        0x06,                                  /* PType:5   */
        0x0,                                   /* X:2       */
        PRV_TGF_RESERVED_FLOWS_MAX_K,          /* K:16      */
        0x00,                                  /* Y:3       */
        PRV_TGF_RESERVED_FLOWS_MAX_Z,          /* Z:21      */
        0x0000                                 /* W:16      */
    },
    /* Packet 2 RX6 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        0x0,                                   /* M:1       */
        0x06,                                  /* PType:5   */
        PRV_TGF_RESERVED_FLOWS_MAX_X,          /* X:2       */
        0x0000,                                /* K:16      */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,          /* Y:3       */
        0x000000,                              /* Z:21      */
        PRV_TGF_RESERVED_FLOWS_MAX_W           /* W:16      */
    },

    /* Packet 3 RX6 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, /* etherType */
        0x0,                                   /* M:1       */
        0x06,                                  /* PType:5   */
        0x0,                                   /* X:2       */
        0x0000,                                /* K:16      */
        0x00,                                  /* Y:3       */
        0x000000,                              /* Z:21      */
        1234                                   /* W:16      */
    }
};

/* Array of three different packet JTM7 parts */
static TGF_PACKET_JTM_Type_7_STC prvTgfCfgPacketJTM7PartArray[] = {

    /* Packet 1 JTM7 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS,       /* etherType */
        0x0,                                         /* M:1 */
        0x7,                                         /* PType:5 */
        PRV_TGF_RESERVED_FLOWS_MAX_X,                /* X:2 */
        0x00,                                        /* Hop_Limit:8 */
        PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL,       /* Forwarding_Label:16 */
        0x0000,                                      /* K:16 */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,                /* MAX_Y:3 */
        0x000000,                                    /* Z:21 */
        PRV_TGF_RESERVED_FLOWS_MAX_W                 /* W:16 */
    },
    /* Packet 2 JTM7 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS,       /* etherType */
        0x1,                                         /* M:1 */
        0x7,                                         /* PType:5 */
        0x00,                                        /* X:2 */
        PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT,        /* Hop_Limit:8 */
        0x00,                                        /* Forwarding_Label:16 */
        PRV_TGF_RESERVED_FLOWS_MAX_K,                /* K:16 */
        0x00,                                        /* MAX_Y:3 */
        PRV_TGF_RESERVED_FLOWS_MAX_Z,                /* Z:21 */
        0x00                                         /* W:16 */
    },

    /* Packet 3 JTM7 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS,       /* etherType */
        0x1,                                         /* M:1 */
        0x7,                                         /* PType:5 */
        0x00,                                        /* X:2 */
        0x00,                                        /* Hop_Limit:8 */
        0x00,                                        /* Forwarding_Label:16 */
        0x00,                                        /* K:16 */
        0x00,                                        /* MAX_Y:3 */
        0x00,                                        /* Z:21 */
        0x00                                         /* W:16 */
    }
};

/* Array of three different packet JTM7 parts */
static TGF_PACKET_JTM_Type_2_STC prvTgfCfgPacketJTM2PartArray[] = {

    /* Packet 1 JTM2 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS,       /* etherType */
        0x0,                                         /* M:1 */
        0x2,                                         /* PType:5 */
        PRV_TGF_RESERVED_FLOWS_MAX_X,                /* X:2 */
        0x00,                                        /* Hop_Limit:8 */
        PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL,       /* Forwarding_Label:16 */
        0x0000,                                      /* K:16 */
        PRV_TGF_RESERVED_FLOWS_MAX_Y,                /* Y:3 */
        0x000000,                                    /* lookupKey:13 */
        PRV_TGF_RESERVED_FLOWS_MAX_W                 /* W:16 */
    },
    /* Packet 2 JTM2 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS,       /* etherType */
        0x0,                                         /* M:1 */
        0x2,                                         /* PType:5 */
        0x00,                                        /* X:2 */
        PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT,        /* Hop_Limit:8 */
        0x0,                                         /* Forwarding_Label:16 */
        PRV_TGF_RESERVED_FLOWS_MAX_K,                /* K:16 */
        0x0,                                         /* Y:3 */
        PRV_TGF_RESERVED_FLOWS_MAX_LOOKUP_KEY,       /* lookupKey:13 */
        0x0                                          /* W:16 */
    },

    /* Packet 3 JTM2 part */
    {
        PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS,       /* etherType */
        0x0,                                         /* M:1 */
        0x2,                                         /* PType:5 */
        0x0,                                         /* X:2 */
        0x0,                                         /* Hop_Limit:8 */
        0x0,                                         /* Forwarding_Label:16 */
        0x0,                                         /* K:16 */
        0x0,                                         /* Y:3 */
        0x0,                                         /* lookupKey:13 */
        0x0                                          /* W:16 */
    }
};

/* Original Ingress Packet */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfCfgOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x01},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x04}                /* saMac */
};

/* Save least significant byte of original L2 MAC DA */
static const GT_U8 prvTgfCfgOriginalL2MacDaLsb = 0x01;

/* reserved Flow packet, Tunnel part */
static TGF_PACKET_TS_MEDIUM_STC prvTgfRxTxPacketTunnelPart = {
    {{0x00, 0x00, 0x00, 0x00, 0x44, 0x44},               /* TS daMac     */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x45}},               /* TS saMac     */
    {0xCA, 0xFF, 0xBA, 0xBE, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B}    /* TS data_0_11 */
};

/* Reserved flow packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {
    PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS
};

/* Data of packet */
static GT_U8 prvTgfCfgPacketPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28
};

/* Payload part of packet */
static TGF_PACKET_PAYLOAD_STC prvTgfCfgPacketPayloadPart =
{
    sizeof(prvTgfCfgPacketPayloadDataArr), /* dataLength */
    prvTgfCfgPacketPayloadDataArr          /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfCfgPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfCfgOriginalPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfCfgPacketPayloadPart}
};

/* final original packet 1 info */
static TGF_PACKET_STC prvTgfCfgPacketInfo = {
    TGF_L2_HEADER_SIZE_CNS+TGF_ETHERTYPE_SIZE_CNS+sizeof(prvTgfCfgPacketPayloadDataArr),       /* totalLen */
    sizeof(prvTgfCfgPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),    /* numOfParts */
    prvTgfCfgPacketPartsArray                                           /* partsArray */
};

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_RESERVED_FLOW_TEST_VID_CNS            /* pri, cfi, VlanId */
};

/* Ipv4 packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart_Ipv4 = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

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
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 1,  1,  1,  1},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
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

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartsArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv4},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* final original Ipv4 packet info */
static TGF_PACKET_STC prvTgfCfgIpv4PacketInfo = {
    TGF_L2_HEADER_SIZE_CNS+TGF_VLAN_TAG_SIZE_CNS+
    TGF_ETHERTYPE_SIZE_CNS+TGF_IPV4_HEADER_SIZE_CNS+sizeof(prvTgfPayloadDataArr),    /* totalLen */
    sizeof(prvTgfPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),                    /* numOfParts */
    prvTgfPacketPartsArray                                                           /* partsArray */
};

/******************************************************************************/

/******************************* UDP packet **********************************/

/* packet's IPv4 part */
static TGF_PACKET_IPV4_STC prvTgfPacketUdpIpPart =
{
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x36,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    17,                 /* protocol */
    0x4C88,             /* csum */
    {22, 22, 22, 22},   /* srcAddr */
    { 1,  1,  1,  3}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    0,                                             /* src port */
    PRV_TGF_UDP_VXLAN_PORT_NUM,                    /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS              /* csum */
};

/* PARTS of packet UDP */
static TGF_PACKET_PART_STC prvTgfUdpPacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart_Ipv4},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketUdpIpPart},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Length of UDP packet */
#define PRV_TGF_UDP_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS \
    + TGF_UDP_HEADER_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* Length of UDP packet with CRC */
#define PRV_TGF_UDP_PACKET_CRC_LEN_CNS  PRV_TGF_UDP_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/* UDP packet to send */
static TGF_PACKET_STC prvTgfUdpPacketInfo =
{
    PRV_TGF_UDP_PACKET_LEN_CNS,                                             /* totalLen */
    sizeof(prvTgfUdpPacketPartArray) / sizeof(prvTgfUdpPacketPartArray[0]), /* numOfParts */
    prvTgfUdpPacketPartArray                                                /* partsArray */
};

/********************** DSA tag  ***************************/
static TGF_DSA_DSA_FORWARD_STC  prvTgfPacketDsaTagPart_forward = {
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E , /*srcIsTagged*/
    17,/*srcHwDev*/
    GT_FALSE,/* srcIsTrunk */
    /*union*/
    {
        /*trunkId*/
        10/*portNum*/  /* Set in runtime to avoid BE init problem for the union */
    },/*source;*/

    0,/*srcId*/

    GT_FALSE,/*egrFilterRegistered*/
    GT_TRUE,/*wasRouted*/
    0,/*qosProfileIndex*/

    /*CPSS_INTERFACE_INFO_STC         dstInterface*/
    {
        CPSS_INTERFACE_PORT_E,/*type*/

        /*struct*/{
            0,/*devNum*/
            1/*portNum*/
        },/*devPort*/

        0,/*trunkId*/
        0, /*vidx*/
        PRV_TGF_RESERVED_FLOW_TEST_VID_CNS,/*vlanId*/
        16,/*devNum*/
        0,/*fabricVidx*/
        0 /*index*/
    },/*dstInterface*/
    GT_TRUE,/*isTrgPortValid*/
    PRV_TGF_CFG_TARGET_EPORT_CNS,/*dstEport*/
    0,/*TPID Index*/
    GT_FALSE,/*origSrcPhyIsTrunk*/
    /*union*/
    {
        /*trunkId*/
        0/*portNum*/ /* Set in runtime to avoid BE init problem for the union */
    },/*origSrcPhy;*/
    GT_FALSE,/*phySrcMcFilterEnable*/
    0, /* hash */
    GT_TRUE /*skipFdbSaLookup*/
};

static TGF_PACKET_DSA_TAG_STC  prvTgfPacketDsaTagPart = {
    TGF_DSA_CMD_FORWARD_E ,/*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E ,/*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,                                  /*vpt*/
        0,                                  /*cfiBit*/
        1,                                  /*vid*/
        GT_FALSE,                           /*dropOnSource*/
        GT_FALSE                            /*packetIsLooped*/
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

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfDsaPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTagPart},
    {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfPacketDsaTagPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfCfgPacketPayloadPart}
};

/* LENGTH of packet */
#define PRV_TGF_DSA_PACKET_LEN_CNS \
 (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_DSA_TAG_SIZE_CNS \
                         + sizeof(prvTgfCfgPacketPayloadDataArr))

/* LENGTH of packet with CRC */
#define PRV_TGF_DSA_PACKET_CRC_LEN_CNS  (PRV_TGF_DSA_PACKET_LEN_CNS + TGF_CRC_LEN_CNS)

/* PACKET to send info */
static TGF_PACKET_STC prvTgfDsaPacketInfo = {
    PRV_TGF_DSA_PACKET_CRC_LEN_CNS,                                /* totalLen */
    sizeof(prvTgfDsaPacketPartArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfDsaPacketPartArray                                       /* partsArray */
};

/* Array storing flowNum corresponding to thread Id */
static const GT_U32 prvTgfPhaThreadIdReserved_1_0_flowNumArray[] = {1, 4};
static const GT_U32 prvTgfPhaThreadIdReserved_1_1_flowNumArray[] = {5, 8};
static const GT_U32 prvTgfPhaThreadIdReserved_1_2_flowNumArray[] = {2, 3, 6, 7};

/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdb;

static prvTgfPclUdb prvTgfRouterSaModificationEpclMetadataUdbInfo[] = {

     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 40, 0x01},   /* Routed[0] */
     {1 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 58, 0x60},   /* isTunneled[5] tsPassengerType[6] */

     /* must be last */
     {2, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

static prvTgfPclUdb prvTgfInqaEcnEpclMetadataUdbInfo[] = {

     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 41, 0x01},   /* isIp[0] */

     /* must be last */
     {1, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

static prvTgfPclUdb prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[] = {

     {0 , CPSS_DXCH_PCL_OFFSET_METADATA_E, 42, 0x01},   /* TCP/UDP Port Comparators */
     {1 , CPSS_DXCH_PCL_OFFSET_L4_E,       12, 0xFF},   /* oldVxlanVni[23:16] */
     {2 , CPSS_DXCH_PCL_OFFSET_L4_E,       13, 0xFF},   /* oldVxlanVni[15:8]  */
     {3 , CPSS_DXCH_PCL_OFFSET_L4_E,       14, 0xFF},   /* oldVxlanVni[7:0]   */

     /* must be last */
     {4, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC                  egressInfo;
    GT_ETHERADDR                                      macSaAddr;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT                     phaFwImageId;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT      pclPortAccessModeCfgGet;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT    pclTsAccessModeCfgGet;
    GT_U32                                            copyReservedMask;
    GT_U32                                            routerMacSaIndex;
    GT_U16                                            pvid;
    GT_U8                                             originalL2MacDaLsb;
    CPSS_L4_PROTOCOL_PORT_TYPE_ENT                    l4PortType;
    CPSS_COMPARE_OPERATOR_ENT                         compareOperator;
    GT_U16                                            tcpUdpComparatorValue;
    GT_U8                                             udpPayload[3];
    CPSS_INTERFACE_INFO_STC                           physicalInfo;
} prvTgfRestoreCfg;

/******************************************************************************/

/*******************************************************************************/

/**
* @internal prvTgfCfgReservedFlowPacketInfoGet function
* @endinternal
*
* @brief    Get Reserved flow packet Info
*
*/
static GT_VOID prvTgfCfgReservedFlowPacketInfoGet
(
    IN  GT_U8                 M,
    IN  GT_U8                 pType,
    IN  GT_U16                K,
    IN  GT_U16                W,
    IN  GT_U8                 X,
    IN  GT_U8                 Y,
    IN  GT_U32                Z,
    IN  GT_U16                lookupKey,
    IN  GT_U8                 hopLimit,
    IN  GT_U16                frwdLabel,
    IN  GT_U32                packetNum,
    IN  GT_U32                flowNum,
    OUT TGF_PACKET_STC        *prvTgfCfgPacketInfoPtr
)
{
    GT_U32    iter = 0;
    GT_U8     offset = 0;
    GT_U8     payloadData = 0;

    prvTgfCfgPacketInfoPtr = prvTgfCfgPacketInfoPtr;
    switch(packetNum % prvTgfBurstCount)
    {
        case 0:
        case 1:
        {
            /* Payload data */
            /* If packetNum is 0, appending all ones after W in packet
             * else appending all zeros after W in packet */
            payloadData = ((packetNum % prvTgfBurstCount)== 0) ? 0xFF : 0x00;
            for(iter = 0; iter < PRV_TGF_RESERVED_FLOWS_PACKET_PAYLOAD_LEN; iter++)
            {
                prvTgfCfgPacketPayloadDataArr[iter] = payloadData;
            }

            break;
        }
        case 2:
        {
            /* Payload data */
            /* Appending random numbers after W in packet */
            for(iter = 0; iter < PRV_TGF_RESERVED_FLOWS_PACKET_PAYLOAD_LEN; iter++)
            {
                payloadData = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_BYTE_VAL + 1);
                prvTgfCfgPacketPayloadDataArr[iter] = payloadData;
            }

            break;
        }
        default:
        {
            PRV_UTF_LOG1_MAC("Invalid packet number:%d\n", packetNum);
            break;
        }
    }

    /* M[0] & pType[0:4] & X[0:1] */
    prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((M << 7) | ((pType & 0x1F) << 2) | (X & 0x03));

    switch(flowNum)
    {
        case 4:
        {
            /* Hop Limit[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(hopLimit);
            /* Forwarding Label[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(frwdLabel >> 8);
            /* Forwarding Label[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(frwdLabel);
            /* K[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K >> 8);
            /* K[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K);

            GT_ATTR_FALLTHROUGH;
        }
        case 1:
        {
            /* Y[0:2] & lookupKey[12:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(((Y & 0x07) << 5) | ((lookupKey >> 8) & 0x1F));
            /* lookupKey[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(lookupKey);
            /* W[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W >> 8);
            /* W[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W);

            break;
        }
        case 8:
        {
            /* Hop Limit[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(hopLimit);
            /* Forwarding Label[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(frwdLabel >> 8);
            /* Forwarding Label[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(frwdLabel);

            GT_ATTR_FALLTHROUGH;
        }
        case 5:
        {
            /* K[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K >> 8);
            /* K[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K);

            /* Y[0:2] & Z[20:16] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(((Y & 0x07) << 5) | ((Z >> 16) & 0x1F));

            /* Z[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(Z >> 8);
            /* Z[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(Z);

            /* W[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W >> 8);
            /* W[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W);

            break;
        }

        case 6:
        {
            /* K[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K >> 8);
            /* K[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K);
            /* Y [0:2] & Z[20:16] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(((Y & 0x7) << 5) | ((Z & 0x1F0000) >> 16));
            /* Z[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((Z & 0xFF00) >> 8);
            /* Z[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(Z & 0xFF);
            /* W[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((W & 0xFF00) >> 8);
            /* W[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W & 0xFF);
            break;
        }

        case 7:
        {
            /* Hop_Limit[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(hopLimit);
            /* Forwarding_Lable[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((frwdLabel & 0xFF00) >> 8) ;
            /* Forwarding_Lable[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(frwdLabel & 0xFF) ;
            /* K[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((K & 0xFF00) >> 8);
            /* K[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K & 0xFF);
            /* Y[7:5] & Z[20:16]  */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(((Y & 0x7) << 5) | ((Z & 0x1F0000) >> 16));
            /* Z[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((Z & 0xFF00 )>> 8);
            /* Z[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(Z & 0xFF);
            /* W[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((W & 0xFF00) >> 8);
            /* W[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W & 0xFF);
            break;
        }

        case 2:
        case 3:
        {
            /* Hop Limit */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(hopLimit);
            /* Forwarding Lable[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((frwdLabel >> 8) & 0xFF);
            /* Forwarding Lable[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(frwdLabel & 0xFF);
            /* K[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((K & 0xFF00) >> 8);
            /* K[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(K & 0xFF);
            /* Y [0:2] & lookupKey[12:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(((Y & 0x7) << 5) | ((lookupKey & 0x1F00) >> 8));
            /* lookupKey[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(lookupKey  & 0xFF );
            /* W[15:8] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)((W & 0xFF00) >> 8);
            /* W[7:0] */
            prvTgfCfgPacketPayloadDataArr[offset++] = (GT_U8)(W & 0xFF);
            break;
        }

        default:
        {
            PRV_UTF_LOG1_MAC("Invalid reserved flow number:%d\n", flowNum);
            break;
        }
    }

    /* Incrementing MAC DA least significant byte by 1
     * except when packetNum % prvTgfBurstCount equals 0.
     * packetNum % prvTgfBurstCount indicates its the first packet of the flow
     */
    if((packetNum % prvTgfBurstCount) != 0)
    {
        prvTgfCfgOriginalPacketL2Part.daMac[PRV_TGF_L2_MAC_ADDR_SIZE_CNS - 1]++;
    }
    else
    {
        prvTgfCfgOriginalPacketL2Part.daMac[PRV_TGF_L2_MAC_ADDR_SIZE_CNS - 1] = prvTgfCfgOriginalL2MacDaLsb;
    }

    *prvTgfCfgPacketInfoPtr = prvTgfCfgPacketInfo;

    return;
}

/**
* @internal prvTgfCfgReservedFlowsBridgeConfigSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfCfgReservedFlowsBridgeConfigSet()
{
    GT_STATUS rc;

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);

    /* Remove all, except ingress and egress ports from vlan 5 */
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_RESERVED_FLOW_TEST_VID_CNS, prvTgfPortsArray[PRV_TGF_CFG_PORT2_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_CFG_PORT2_IDX_CNS]);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_RESERVED_FLOW_TEST_VID_CNS, prvTgfPortsArray[PRV_TGF_CFG_PORT3_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_CFG_PORT3_IDX_CNS]);

    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS], &prvTgfRestoreCfg.pvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS]);

    /* Set Port Vid 5 to ingress port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS], PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS]);
}

/**
* @internal prvTgfCfgReservedFlowsPortConfig function
* @endinternal
*
* @brief    Target ePort Attribute assignments
*            Enable tunnel start
*            Set tunnel start pointer = 10
*            Set tunnel generic entry genericType = medium, ethertype = 0x1701
*            and MAC DA = 00:00:00:00:44:44
*
*/
static GT_VOID prvTgfCfgReservedFlowsPortConfig()
{

    GT_STATUS                             rc;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC      egressInfo;

    /***********************************************/
    /* Egress Port TS = Generic TS */
    /***********************************************/
    /* AUTODOC: Egress ePort TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel generic Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                         = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart             = GT_TRUE;
    egressInfo.tunnelStartPtr          = PRV_TGF_TUNNEL_START_L2_INDEX_CNS;
    egressInfo.tsPassengerPacketType   = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=10 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfCfgReservedFlowsTsConfig function
* @endinternal
*
* @brief   TS configurations for generic tunnel
*           Set Tunnel Start entry 10 with:
*           tunnelType = Generic, ethertype = 0x1701,
*           MAC DA = 00:00:00:00:44:44
*           Set Global MAC SA = 00:00:00:00:45:45
*
*/
static GT_VOID prvTgfCfgReservedFlowsTsConfig(GT_VOID)
{
    GT_STATUS                         rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT    tunnelEntry;
    GT_ETHERADDR                      macSaAddr;
    GT_U8                             iter;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    tunnelEntry.genCfg.etherType = PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS;
    tunnelEntry.genCfg.genericType = PRV_TGF_TUNNEL_START_GENERIC_MEDIUM_TYPE_E;
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgfRxTxPacketTunnelPart.tsL2part.daMac, sizeof(TGF_MAC_ADDR));

    for(iter = 0; iter < PRV_TGF_TS_MEDIUM_TYPE_DATA1_SIZE_CNS; iter++)
    {
        tunnelEntry.genCfg.data[iter] = prvTgfRxTxPacketTunnelPart.data1[iter];
    }

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_L2_INDEX_CNS, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

     /*  AUTODOC: configure tunnel start mac SA*/
    rc = prvTgfIpRouterPortGlobalMacSaIndexGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS], &prvTgfRestoreCfg.routerMacSaIndex);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS], PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaGet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &prvTgfRestoreCfg.macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexGet");

    /* Copy prvTgfRxTxPacketTunnelPart.tsL2part.saMac to macSaAddr */
    macSaAddr.arEther[0] = prvTgfRxTxPacketTunnelPart.tsL2part.saMac[0];
    macSaAddr.arEther[1] = prvTgfRxTxPacketTunnelPart.tsL2part.saMac[1];
    macSaAddr.arEther[2] = prvTgfRxTxPacketTunnelPart.tsL2part.saMac[2];
    macSaAddr.arEther[3] = prvTgfRxTxPacketTunnelPart.tsL2part.saMac[3];
    macSaAddr.arEther[4] = prvTgfRxTxPacketTunnelPart.tsL2part.saMac[4];
    macSaAddr.arEther[5] = prvTgfRxTxPacketTunnelPart.tsL2part.saMac[5];

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");
}

/**
* @internal prvTgfCfgReservedFlowsEpclConfigSet function
* @endinternal
*
* @brief   EPCL configurations
*           Enable target port for EPCL trigger
*           Enable port mode for config table
*           Set copy reserved mask to PRV_TGF_COPY_RESERVED_MASK
*           Set following three EPCL rules
*           1. 10B UDB keys with eport, MAC DA least significant byte = "01"
*           Action = copyReserved = copyReservedArray[0] and flowId = flowIdArray[0]
*           2. 10B UDB keys with eport, MAC DA least significant byte = "02"
*           Action = copyReserved = copyReservedArray[1] and flowId = flowIdArray[1]
*           3. 10B UDB keys with eport, MAC DA least significant byte = "03"
*           Action = copyReserved = copyReservedArray[2] and flowId = flowIdArray[2]
*/
static GT_VOID prvTgfCfgReservedFlowsEpclConfigSet()
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

    cpssOsSrand(cpssOsTime());
    /* AUTODOC: init PCL Engine for Egress PCL for  */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on target port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                    PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                    CPSS_PCL_DIRECTION_EGRESS_E,
                                    udbSelectidx,
                                    PRV_TGF_PCL_OFFSET_L2_E,
                                    PRV_TGF_L2_MAC_ADDR_SIZE_CNS - 1);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

    udbSelect.udbSelectArr[udbSelectidx]=udbSelectidx;

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* Get copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskGet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, &prvTgfRestoreCfg.copyReservedMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskGet in egress direction for lookup 0");

    /* Set copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, PRV_TGF_COPY_RESERVED_MASK);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskSet in egress direction for lookup 0");

    /* AUTODOC: set PCL rule 0, 1 and 2 */
    for(ruleIndex = 0; ruleIndex < prvTgfBurstCount; ruleIndex++)
    {
        cpssOsMemSet(&mask, 0, sizeof(mask));
        cpssOsMemSet(&pattern, 0, sizeof(pattern));
        cpssOsMemSet(&action, 0, sizeof(action));

        pattern.ruleEgrUdbOnly.udb[udbSelectidx] = prvTgfCfgOriginalL2MacDaLsb + ruleIndex;
        mask.ruleEgrUdbOnly.udb[udbSelectidx] = 0xFF;

        ruleFormat                          = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
        action.copyReserved.assignEnable    = GT_TRUE;
        /* Using random value for copy reserved for the packet 3
         * copyReserved MAX value is at index 0 of copyReservedArray
         */
        copyReservedArray[ruleIndex]        = (ruleIndex == 2) ? cpssOsRand() % (copyReservedArray[0] + 1) :
                                              copyReservedArray[ruleIndex];
        action.copyReserved.copyReserved    = copyReservedArray[ruleIndex];
        /* Using random value for flowId for the packet 3
         * flowId MAX value is at index 1 of flowIdArray
         */
        flowIdArray[ruleIndex]              = (ruleIndex == 2) ?
                                              cpssOsRand() % (flowIdArray[1] + 1) :
                                              flowIdArray[ruleIndex];
        action.flowId                       = flowIdArray[ruleIndex];
        action.egressPolicy                 = GT_TRUE;
        rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
    }
}

/**
* @internal prvTgfCfgReservedFlowsPhaConfig function
* @endinternal
*
* @brief   PHA thread config
*           Set the FW thread id = phaThreadId
*
*/
static GT_VOID prvTgfCfgReservedFlowsPhaConfig
(
    IN GT_U32    phaThreadId
)
{
    GT_STATUS rc;

    /* AUTODOC: Set the thread entry */
    CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT            phaThreadType = CPSS_DXCH_PHA_THREAD_TYPE___LAST___E;

    /* Load FW img 02 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaFwImageIdGet");
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, CPSS_DXCH_PHA_FW_IMAGE_ID_02_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit");

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* AUTODOC: Assign phaThreadId fw thread to target port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                    GT_TRUE, phaThreadId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");

    /* Get PHA thread type for given phaThreadId */
    switch(phaThreadId)
    {
        case PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS:
        {
            phaThreadType = CPSS_DXCH_PHA_THREAD_TYPE_RESERVED_1_0_E;
            break;
        }
        case PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS:
        {
            phaThreadType = CPSS_DXCH_PHA_THREAD_TYPE_RESERVED_1_1_E;
            break;
        }
        case PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS:
        {
            phaThreadType = CPSS_DXCH_PHA_THREAD_TYPE_RESERVED_1_2_E;
            break;
        }
        default:
        {
            PRV_UTF_LOG1_MAC("Invalid reserved flow PHA thread id:%d\n", phaThreadId);
            break;
        }
    }

    /* AUTODOC: Config the thread type = phaThreadType */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                phaThreadId,
                &commonInfo,
                phaThreadType,
                &extInfo);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet for thread id:%d type:%d",
                phaThreadId, phaThreadType);
}

/**
* @internal prvTgfCfgReservedFlowsConfigurationSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId for corresponding Reserved flows.
*    configure VLAN
*    Configure E2phy and Generic TS for eport
*    configure EPCL rule to set flowId and copy_reserved
*    send UC traffic with ethertype 0x1701 and verify expected traffic on target port
*/
void prvTgfCfgReservedFlowsConfigurationSet(GT_U32 phaThreadId)
{
    /* bridge config */
    prvTgfCfgReservedFlowsBridgeConfigSet();

    /* port config */
    prvTgfCfgReservedFlowsPortConfig();

    /* TS Configuration */
    prvTgfCfgReservedFlowsTsConfig();

    /* Configuring EPCL for thread ids = PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS, PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS */
    if(phaThreadId != PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS)
    {
        /* EPCL config to set flowId and copyReserved needed for flow 1,2,3,4,6,7 */
        prvTgfCfgReservedFlowsEpclConfigSet();
    }

    /* PHA config */
    prvTgfCfgReservedFlowsPhaConfig(phaThreadId);
}

static void prvTgfCfgReservedFlowsPacketValidate
(
    IN GT_U8*                packetBuf,
    IN GT_U8                 M,
    IN GT_U16                K,
    IN GT_U16                W,
    IN GT_U8                 X,
    IN GT_U8                 Y,
    IN GT_U32                Z,
    IN GT_U16                lookupKey,
    IN GT_U8                 hopLimit,
    IN GT_U16                frwdLabel,
    IN GT_U32                packetCount,
    IN GT_U8                 flowNum,
    IN GT_U32                numOfFlows
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  dataFormat;
    GT_U32                  offset;
    GT_U32                  copyReserved = 0;
    GT_U32                  copyReservedMask = 0;
    static GT_U32           expectedPacketCount = 0;
    GT_U8                   index   = (packetCount - 1) % prvTgfBurstCount;
    GT_U8                   is_dec_Hop_Limit = (prvTgfRxTxPacketTunnelPart.data1[1] & 0x1);
    GT_U8                   pType = 0;

    /* Components to be verified for all flows
    1. MAC DA with TS MAC DA
    2. MAC SA with TS MAC SA
    3. Ethertype with 0x1701 custom ethertype
    4. M - 1 bit
    5. ptype - 5 bits
    6. X - 2 bits
    7. Y - 3 bits
    8. Validate W
    */
    /* Components to be verified for flow 1 and 4
    1. A - 5 bits with flowId[4:0]
    2. B[9:0] with flowId[14:5]
    3. B[23:10] with copyReserved[16:3]
       bits [16:3] are used for copyReserved as internally copyReserved is set as copyReserved << 1.
       Therefore, instead of using bits [17:4] bits [16:3] are used.
    */
    /* Components to be verified for flow 5
    1. K - 16 bits
    2. Z - 21 bits
    */

    /* Components to be verified
    Flow 6 & 7:
    1. MAC DA with TS MAC DA
    2. MAC SA with TS MAC SA
    3. Ethertype with 0x1701 custom ethertype
    4. M                    - 1 bit
    5. ptype                - 5 bits
    6. X                    - 2 bits
    7. Hop Limit            - 8 bits
    8. Forwarding Label     - 16 bits
    9. K                    - 16 bits
    a. Y                    - 3 bits
    b. Z                    - 21 bits
    c. W                    - 16 bits

    Flow 2 & 3:
    1. MAC DA with TS MAC DA
    2. MAC SA with TS MAC SA
    3. Ethertype with 0x1701 custom ethertype
    4. M                    - 1 bit
    5. ptype                - 5 bits
    6. X                    - 2 bits
    7. Hop Limit            - 8 bits
    8. Forwarding Label     - 16 bits
    9. K                    - 16 bits
    a. Y                    - 3 bits
    b. Lookup Key           - 13 bits
    c. W                    - 16 bits  */

    /* Get copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskGet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, &copyReservedMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskGet in egress direction for lookup 0");
    /* HW Copy Reserved Bits[19:1] = SW Copy Reserved Bits[18:0] */
    copyReservedMask <<= 1;

    offset = 0;
    /* Validate MAC DA */
    rc = cpssOsMemCmp(&packetBuf[offset], prvTgfRxTxPacketTunnelPart.tsL2part.daMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC DA mismatch");

    /* Validate MAC SA */
    offset = PRV_TGF_L2_MAC_ADDR_SIZE_CNS;
    rc = cpssOsMemCmp(&packetBuf[offset], prvTgfRxTxPacketTunnelPart.tsL2part.saMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC SA mismatch");

    /* Validate ethertype */
    offset = TGF_L2_HEADER_SIZE_CNS;
    dataFormat = (packetBuf[offset] << 8) | packetBuf[offset+1];
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS, dataFormat, "ether type mismatch");

    /* Validate M */
    offset += TGF_ETHERTYPE_SIZE_CNS;
    dataFormat = packetBuf[offset] >> 7;
    UTF_VERIFY_EQUAL0_STRING_MAC(M, dataFormat, "M mismatch");
    /* Validate pType */
    dataFormat = (packetBuf[offset] >> 2) & 0x1F;
    switch(flowNum)
    {
        case 1:
        case 4:
        {
            pType = 0x4;
            break;
        }

        case 5:
        case 8:
        {
            pType = 0x6;
            break;
        }

        case 2:
        case 3:
        {
            pType = 0x2;
            break;

        }
        case 6:
        case 7:
        {
            pType = 0x7;
            break;
        }
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(pType, dataFormat, "pType mismatch");
    /* Validate X */
    dataFormat = packetBuf[offset] & 0x03;
    UTF_VERIFY_EQUAL0_STRING_MAC(X & 0x03, dataFormat, "X mismatch");

    switch(flowNum)
    {
        case 1:
        case 4:
        {
            /* Validate Y */
            offset++;
            dataFormat = packetBuf[offset] >> 5;
            UTF_VERIFY_EQUAL0_STRING_MAC(Y & 0x07, dataFormat, "Y mismatch");
            /* Validate A with flowId[4:0] */
            dataFormat = packetBuf[offset] & 0x1F;
            UTF_VERIFY_EQUAL0_STRING_MAC(flowIdArray[(packetCount-1) % prvTgfBurstCount] & 0x1F, dataFormat, "A mismatch");

            /* Validate B[9:0] with flowId[14:5] */
            offset++;
            dataFormat = (packetBuf[offset+1] & 0x03) | packetBuf[offset+2];
            UTF_VERIFY_EQUAL0_STRING_MAC((flowIdArray[(packetCount-1) % prvTgfBurstCount] >> 5) & 0x3FF,
                dataFormat, "B with flowId mismatch");
            /* Validate B[23:10] with copyReserved[16:3] */
            dataFormat = (packetBuf[offset] << 6) | (packetBuf[offset+1] >> 2);
            /* Getting final copyReserved value set */
            copyReserved = copyReservedArray[(packetCount-1) % prvTgfBurstCount] & copyReservedMask;
            UTF_VERIFY_EQUAL0_STRING_MAC((copyReserved >> 3) & 0x3FFF, dataFormat, "B with copyReserved mismatch");

            /* Validate W */
            offset = offset + 3;
            dataFormat = (packetBuf[offset] << 8) | packetBuf[offset+1];
            UTF_VERIFY_EQUAL0_STRING_MAC(W, dataFormat, "W mismatch");

            break;
        }
        case 5:
        case 8:
        {
            /* Validate K */
            offset++;
            dataFormat = (packetBuf[offset] << 8) | packetBuf[offset+1];
            UTF_VERIFY_EQUAL0_STRING_MAC(K, dataFormat, "K mismatch");

            /* Validate Y */
            offset += 2;
            dataFormat = (packetBuf[offset] >> 5);
            UTF_VERIFY_EQUAL0_STRING_MAC(Y & 0x07, dataFormat, "Y mismatch");

            /* Validate Z */
            dataFormat = ((packetBuf[offset] & 0x1F) << 16) | (packetBuf[offset+1] << 8) | packetBuf[offset+2];
            UTF_VERIFY_EQUAL0_STRING_MAC(Z, dataFormat, "Z mismatch");

            /* Validate W */
            offset += 3;
            dataFormat = (packetBuf[offset] << 8) | packetBuf[offset+1];
            UTF_VERIFY_EQUAL0_STRING_MAC(W, dataFormat, "W mismatch");

            break;
        }
        case 6:
        case 7:
        case 2:
        case 3:
        {
            /* Validate Hop Limit */
            offset++;
            dataFormat = packetBuf[offset];
            if (flowNum != 6)
            {
                if (is_dec_Hop_Limit)
                {
                    if (hopLimit > 1)
                    {
                        hopLimit = hopLimit - 1;
                    }
                }
            }
            else
                copyReserved = (copyReservedArray[index] >> 7);

            UTF_VERIFY_EQUAL0_STRING_MAC(
                ((flowNum == 6) ? ((GT_U8)(copyReserved)) : hopLimit),(GT_U8)(dataFormat), "Hop Limit mismatch");

            /* Validate Forwarding Label  */
            offset++;
            dataFormat = (packetBuf[offset] << 8) | (packetBuf[offset+1]);
            frwdLabel = (prvTgfRxTxPacketTunnelPart.data1[2] << 8) | (prvTgfRxTxPacketTunnelPart.data1[3]);
            UTF_VERIFY_EQUAL0_STRING_MAC(frwdLabel, dataFormat, "Forwarding Label mismatch");

            /* Validate K */
            offset += 2;
            dataFormat = (packetBuf[offset] << 8) | (packetBuf[offset+1]);
            UTF_VERIFY_EQUAL0_STRING_MAC(K, dataFormat, "K mismatch");

            /* Validate Y */
            offset += 2;
            dataFormat = packetBuf[offset] >> 5;
            UTF_VERIFY_EQUAL0_STRING_MAC(Y & 0x07, dataFormat, "Y mismatch");

            switch(flowNum)
            {
                case 6:
                case 7:
                {
                    /* Validate Z */
                    dataFormat = ((packetBuf[offset] & 0x1F) << 16) | (packetBuf[offset+1] << 8) | packetBuf[offset + 2];
                    UTF_VERIFY_EQUAL0_STRING_MAC(Z & 0x1FFFFF, dataFormat, "Z mismatch");
                    offset += 3;
                    break;
                }

                case 2:
                case 3:
                {
                    /* Validate Lookup_Key */
                    dataFormat = (((packetBuf[offset] & 0x1F) << 8) | packetBuf[offset+1]) ;
                    UTF_VERIFY_EQUAL0_STRING_MAC(lookupKey & 0x1FFF, dataFormat, "Lookup_Key mismatch");
                    offset += 2;
                    break;
                }

                default:
                {
                    PRV_UTF_LOG1_MAC("Invalid reserved flow number:%d\n", flowNum);
                    break;
                }

            }
            /* Validate W */
            dataFormat = (packetBuf[offset] << 8) | packetBuf[offset+1];
            UTF_VERIFY_EQUAL0_STRING_MAC(W, dataFormat, "W mismatch");
            break;
        }

        default:
        {
            PRV_UTF_LOG1_MAC("Invalid reserved flow number:%d\n", flowNum);
            break;
        }
    }

    expectedPacketCount %= (prvTgfBurstCount * numOfFlows);
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPacketCount + 1, packetCount, "packet count mismatch");
    expectedPacketCount++;

    if((expectedPacketCount % (prvTgfBurstCount * numOfFlows)) == 0)
    {
        expectedPacketCount = 0;
    }
}

/**
* @internal prvTgfCfgReservedFlowsTrafficSend function
* @endinternal
*
* @brief  Traffic test for PHA thread Reserved flows with generic TS encapsulation
*/
void prvTgfCfgReservedFlowsTrafficSend
(
    IN GT_U32    phaThreadId
)
{

    CPSS_INTERFACE_INFO_STC  targetPortInterface;
    TGF_PACKET_STC           prvTgfCfgPacketInfoGet;
    GT_U8                    packetBuf[256];
    GT_U32                   packetLen          = sizeof(packetBuf);
    GT_U32                   packetActualLength = 0;
    GT_U8                    queue              = 0;
    GT_U8                    dev                = 0;
    GT_BOOL                  getFirst           = GT_TRUE;
    TGF_NET_DSA_STC          rxParam;
    GT_STATUS                rc                 = GT_OK;
    GT_U32                   iter               = 0;
    GT_U8                    M                  = 0;
    GT_U8                    pType              = 0;
    GT_U16                   K                  = 0;
    GT_U16                   W                  = 0;
    GT_U8                    X                  = 0;
    GT_U8                    Y                  = 0;
    GT_U32                   Z                  = 0;
    GT_U16                   lookupKey          = 0;
    GT_U8                    hopLimit           = 0;
    GT_U16                   frwdLabel          = 0;
    GT_U32                   flowNum            = 0;
    GT_U32                   index              = 0;
    GT_U32                   *flowNumArrayPtr   = NULL;
    GT_U32                   numOfFlows         = 0;
    GT_U8                    skip_ver = GT_FALSE;
    GT_U8                    is_dec_Hop_Limit = (prvTgfRxTxPacketTunnelPart.data1[1] & 0x1);

    cpssOsSrand(cpssOsTime());
    /* Get Number of flows and pointer to flowNum Array for given phaThreadId */
    switch(phaThreadId)
    {
        case PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS:
        {
            flowNumArrayPtr = (GT_U32 *)&prvTgfPhaThreadIdReserved_1_0_flowNumArray;
            numOfFlows      = PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_MAX_FLOWS;
            break;
        }
        case PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS:
        {
            flowNumArrayPtr = (GT_U32 *)&prvTgfPhaThreadIdReserved_1_1_flowNumArray;
            numOfFlows      = PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_MAX_FLOWS;
            break;
        }
        case PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS:
        {
            flowNumArrayPtr = (GT_U32 *)&prvTgfPhaThreadIdReserved_1_2_flowNumArray;
            numOfFlows      = PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_MAX_FLOWS;
            break;
        }
        default:
        {
            PRV_UTF_LOG1_MAC("Invalid reserved flow PHA thread id:%d\n", phaThreadId);
            break;
        }
    }

    /* Sending three packets for each flow corresponding to phaThreadId */
    PRV_UTF_LOG1_MAC("======= Send %d packet =======\n", (prvTgfBurstCount * numOfFlows));
    for(iter = 0; iter < (prvTgfBurstCount * numOfFlows); iter++)
    {
        cpssOsMemSet(&prvTgfCfgPacketInfoGet, 0, sizeof(prvTgfCfgPacketInfoGet));

        index   = iter % prvTgfBurstCount;
        flowNum = flowNumArrayPtr[iter / prvTgfBurstCount];

        /* Get packet info based on flowNum */
        switch(flowNum)
        {
            case 1:
            {
                if(index == 2)
                {
                    prvTgfCfgPacketRx0PartArray[index].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketRx0PartArray[index].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketRx0PartArray[index].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketRx0PartArray[index].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                }

                M         = prvTgfCfgPacketRx0PartArray[index].M;
                pType     = prvTgfCfgPacketRx0PartArray[index].pType;
                K         = 0; /* K value is NA for flow 1 */
                W         = prvTgfCfgPacketRx0PartArray[index].W;
                X         = prvTgfCfgPacketRx0PartArray[index].X;
                Y         = prvTgfCfgPacketRx0PartArray[index].Y;
                Z         = 0; /* Z value is NA for flow 1 */
                hopLimit  = 0; /* hopLimit value is NA for flow 1 */
                frwdLabel = 0; /* frwdLabel value is NA for flow 1 */
                lookupKey = prvTgfCfgPacketRx0PartArray[index].lookupKey;

                break;
            }
            case 4:
            {
                if(index == 2)
                {
                    prvTgfCfgPacketRx1PartArray[index].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketRx1PartArray[index].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketRx1PartArray[index].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketRx1PartArray[index].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                    prvTgfCfgPacketRx1PartArray[index].K = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_K + 1);
                    prvTgfCfgPacketRx1PartArray[index].hopLimit = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT + 1);
                    prvTgfCfgPacketRx1PartArray[index].frwdLabel = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL + 1);
                }

                M         = prvTgfCfgPacketRx1PartArray[index].M;
                pType     = prvTgfCfgPacketRx1PartArray[index].pType;
                K         = prvTgfCfgPacketRx1PartArray[index].K;
                W         = prvTgfCfgPacketRx1PartArray[index].W;
                X         = prvTgfCfgPacketRx1PartArray[index].X;
                Y         = prvTgfCfgPacketRx1PartArray[index].Y;
                Z         = 0; /* Z value is NA for flow 4 */
                lookupKey = prvTgfCfgPacketRx1PartArray[index].lookupKey;
                hopLimit  = prvTgfCfgPacketRx1PartArray[index].hopLimit;
                frwdLabel = prvTgfCfgPacketRx1PartArray[index].frwdLabel;

                break;
            }
            case 5:
            {
                if(index == 2)
                {
                    prvTgfCfgPacketRx2PartArray[index].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketRx2PartArray[index].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketRx2PartArray[index].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketRx2PartArray[index].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                    prvTgfCfgPacketRx2PartArray[index].K = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_K + 1);
                    prvTgfCfgPacketRx2PartArray[index].Z = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Z + 1);
                }

                M         = prvTgfCfgPacketRx2PartArray[index].M;
                pType     = prvTgfCfgPacketRx2PartArray[index].pType;
                K         = prvTgfCfgPacketRx2PartArray[index].K;
                W         = prvTgfCfgPacketRx2PartArray[index].W;
                X         = prvTgfCfgPacketRx2PartArray[index].X;
                Y         = prvTgfCfgPacketRx2PartArray[index].Y;
                Z         = prvTgfCfgPacketRx2PartArray[index].Z;
                lookupKey = 0; /* lookupKey value is NA for flow 5 */
                hopLimit  = 0; /* hopLimit value is NA for flow 5  */
                frwdLabel = 0; /* frwdLabel value is NA for flow 5 */

                break;
            }
            case 8:
            {
                if(index == 2)
                {
                    prvTgfCfgPacketRx3PartArray[index].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketRx3PartArray[index].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketRx3PartArray[index].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketRx3PartArray[index].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                    prvTgfCfgPacketRx3PartArray[index].K = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_K + 1);
                    prvTgfCfgPacketRx3PartArray[index].Z = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Z + 1);
                    prvTgfCfgPacketRx3PartArray[index].hopLimit = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT + 1);
                    prvTgfCfgPacketRx3PartArray[index].frwdLabel = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL + 1);
                }

                M         = prvTgfCfgPacketRx3PartArray[index].M;
                pType     = prvTgfCfgPacketRx3PartArray[index].pType;
                K         = prvTgfCfgPacketRx3PartArray[index].K;
                W         = prvTgfCfgPacketRx3PartArray[index].W;
                X         = prvTgfCfgPacketRx3PartArray[index].X;
                Y         = prvTgfCfgPacketRx3PartArray[index].Y;
                Z         = prvTgfCfgPacketRx3PartArray[index].Z;
                lookupKey = 0; /* lookupKey value is NA for flow 7 */
                hopLimit  = prvTgfCfgPacketRx3PartArray[index].hopLimit;
                frwdLabel = prvTgfCfgPacketRx3PartArray[index].frwdLabel;

                break;
            }
            case 6:
            {
                if(index == 2)
                {
                    prvTgfCfgPacketRx6PartArray[index].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketRx6PartArray[index].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketRx6PartArray[index].K = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_K + 1);
                    prvTgfCfgPacketRx6PartArray[index].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketRx6PartArray[index].Z = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Z + 1);
                    prvTgfCfgPacketRx6PartArray[index].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                }
                M     = prvTgfCfgPacketRx6PartArray[index].M;
                pType = prvTgfCfgPacketRx6PartArray[index].PType;
                X     = prvTgfCfgPacketRx6PartArray[index].X;
                K     = prvTgfCfgPacketRx6PartArray[index].K;
                Y     = prvTgfCfgPacketRx6PartArray[index].Y;
                Z     = prvTgfCfgPacketRx6PartArray[index].Z;
                W     = prvTgfCfgPacketRx6PartArray[index].W;
                break;
            }
            case 7:
            {
                if((iter % prvTgfBurstCount) == 2)
                {
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].Hop_Limit = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].Forwarding_Label = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].K = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_K + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].Z = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Z + 1);
                    prvTgfCfgPacketJTM7PartArray[iter % prvTgfBurstCount].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                }
                M = prvTgfCfgPacketJTM7PartArray[index].M;
                pType = prvTgfCfgPacketJTM7PartArray[index].PType;
                X = prvTgfCfgPacketJTM7PartArray[index].X;
                hopLimit = prvTgfCfgPacketJTM7PartArray[index].Hop_Limit;
                frwdLabel = prvTgfCfgPacketJTM7PartArray[index].Forwarding_Label;
                K = prvTgfCfgPacketJTM7PartArray[index].K;
                Y = prvTgfCfgPacketJTM7PartArray[index].Y;
                Z = prvTgfCfgPacketJTM7PartArray[index].Z;
                W = prvTgfCfgPacketJTM7PartArray[index].W;
                break;
            }
            case 2:
            case 3:
            {
                if(index == 2)
                {
                    prvTgfCfgPacketJTM2PartArray[index].M = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_M + 1);
                    prvTgfCfgPacketJTM2PartArray[index].X = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_X + 1);
                    prvTgfCfgPacketJTM2PartArray[index].Hop_Limit = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_HOP_LIMIT + 1);
                    prvTgfCfgPacketJTM2PartArray[index].Forwarding_Label = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_FRWD_LABEL + 1);
                    prvTgfCfgPacketJTM2PartArray[index].K = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_K + 1);
                    prvTgfCfgPacketJTM2PartArray[index].Y = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_Y + 1);
                    prvTgfCfgPacketJTM2PartArray[index].Lookup_Key = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_LOOKUP_KEY + 1);
                    prvTgfCfgPacketJTM2PartArray[index].W = cpssOsRand() % (PRV_TGF_RESERVED_FLOWS_MAX_W + 1);
                }
                M           = prvTgfCfgPacketJTM2PartArray[index].M;
                pType       = prvTgfCfgPacketJTM2PartArray[index].PType;
                X           = prvTgfCfgPacketJTM2PartArray[index].X;
                hopLimit    = prvTgfCfgPacketJTM2PartArray[index].Hop_Limit;
                frwdLabel   = prvTgfCfgPacketJTM2PartArray[index].Forwarding_Label;
                K           = prvTgfCfgPacketJTM2PartArray[index].K;
                Y           = prvTgfCfgPacketJTM2PartArray[index].Y;
                lookupKey   = prvTgfCfgPacketJTM2PartArray[index].Lookup_Key;
                W           = prvTgfCfgPacketJTM2PartArray[index].W;
                break;
            }

            default:
            {
                PRV_UTF_LOG1_MAC("Invalid reserved flow number:%d\n", flowNum);
                break;
            }
        }

        prvTgfCfgReservedFlowPacketInfoGet(M, pType, K, W, X, Y, Z,
            lookupKey, hopLimit, frwdLabel,
            iter, flowNum, &prvTgfCfgPacketInfoGet);

        /* Enable capture on target port */
        targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
        targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCfgPacketInfoGet, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* Disable capture on target port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* AUTODOC: Get the first rx pkt on target port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);

        if (flowNum == 2 || flowNum == 3 || flowNum == 7)
        {
            if (hopLimit == 0 && !is_dec_Hop_Limit)
            {
                skip_ver = GT_TRUE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                     "Error: failed capture on inPortId %d outPortId %d \n" );
            }

            else if ((is_dec_Hop_Limit == 1 && (hopLimit <= 1)) )
            {
                skip_ver = GT_TRUE;
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_NO_MORE, rc,
                                     "Error: failed capture on inPortId %d outPortId %d \n" );
            }

        }

        else
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                                         "Error: failed capture on inPortId %d outPortId %d \n" );
        tgfTrafficTableRxPcktTblClear();
        if (!skip_ver)
        {
            /*verify the packet*/
            prvTgfCfgReservedFlowsPacketValidate(packetBuf, M, K, W, X, Y, Z,
                lookupKey, hopLimit, frwdLabel, iter + 1, flowNum, numOfFlows);
        }
    }
}

/**
* @internal prvTgfCfgReservedFlowsBridgeConfigReset function
* @endinternal
*
* @brief  Bridge config restore
*/
static void prvTgfCfgReservedFlowsBridgeConfigReset()
{
    GT_STATUS rc;

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* Restore pvid of the ingress port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: port %d, vlan %d", prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
}

/**
* @internal prvTgfCfgReservedFlowsPortConfigReset function
* @endinternal
*
* @brief   port config restore
*
*/
static GT_VOID prvTgfCfgReservedFlowsPortConfigReset()
{

    GT_STATUS   rc;

    /* AUTODOC: Restore ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfCfgReservedFlowsEpclConfigRestore function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfCfgReservedFlowsEpclConfigRestore()
{
    GT_STATUS rc;
    GT_U8     ruleIndex;

    /* AUTODOC: Invalidate PCL rule 0, 1 and 2 */
    for(ruleIndex = 0; ruleIndex < prvTgfBurstCount; ruleIndex++)
    {
        rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, ruleIndex, GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", ruleIndex);
    }

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on target port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: disable ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Restore "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* Set copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, prvTgfRestoreCfg.copyReservedMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskSet in egress direction for lookup 0");
}

/**
* @internal prvTgfCfgReservedFlowsTsConfigRestore function
* @endinternal
*
* @brief TS test configurations restore
*/
static GT_VOID prvTgfCfgReservedFlowsTsConfigRestore()
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    /* AUTODOC: Restore tunnel start entry configuration */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_L2_INDEX_CNS, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

    /* AUTODOC: restore Router MAC SA configuration */
    rc = prvTgfIpRouterPortGlobalMacSaIndexSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS], prvTgfRestoreCfg.routerMacSaIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterPortGlobalMacSaIndexSet");

    rc = prvTgfIpRouterGlobalMacSaSet(prvTgfDevNum, PRV_TGF_MAC_SA_GLOBAL_INDEX_CNS, &prvTgfRestoreCfg.macSaAddr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpRouterGlobalMacSaSet");
}

/**
* @internal prvTgfCfgReservedFlowsPhaConfigRestore function
* @endinternal
*
* @brief PHA test configurations restore
*/
static GT_VOID prvTgfCfgReservedFlowsPhaConfigRestore
(
    IN GT_U32    phaThreadId
)
{
    GT_STATUS rc = GT_OK;

    /* AUTODOC: Restore PHA Configuration */
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaInit FAILED for phaFwImageId = [%d], rc = [%d]", prvTgfRestoreCfg.phaFwImageId, rc);

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                    GT_FALSE, phaThreadId);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for phaThreadId = [%d], rc = [%d]", rc, phaThreadId);
}

/**
* @internal prvTgfCfgReservedFlowsConfigurationRestore function
* @endinternal
*
* @brief  Restore PHA threads for Reserved flows for given phaThreadId.
*/
void prvTgfCfgReservedFlowsConfigurationRestore
(
    IN GT_U32    phaThreadId
)
{
    /* restore bridge config */
    prvTgfCfgReservedFlowsBridgeConfigReset();

    /* restore port config */
    prvTgfCfgReservedFlowsPortConfigReset();

    /* restore TS Configuration */
    prvTgfCfgReservedFlowsTsConfigRestore();

    /* Restoring EPCL for thread ids = PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS, PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS */
    if(phaThreadId != PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS)
    {
        /* restore EPCL configuration */
        prvTgfCfgReservedFlowsEpclConfigRestore();
    }

    /* restore PHA configuration */
    prvTgfCfgReservedFlowsPhaConfigRestore(phaThreadId);
}

/**
* @internal prvTgfCfgRouterSaModificationPortConfig function
* @endinternal
*
* @brief    Target ePort Attribute assignments
*            Enable tunnel start
*            Set tunnel start pointer = 10
*            Set tunnel generic entry genericType = medium, ethertype = 0x1701
*            and MAC DA = 00:00:00:00:44:44
*
*/
static GT_VOID prvTgfCfgRouterSaModificationPortConfig()
{

    GT_STATUS                             rc;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC      egressInfo;

    /***********************************************/
    /* Egress Port TS = Generic TS */
    /***********************************************/
    /* AUTODOC: Egress ePort TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel generic Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                         = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart             = GT_TRUE;
    egressInfo.tunnelStartPtr          = PRV_TGF_TUNNEL_START_L2_INDEX_CNS;
    egressInfo.tsPassengerPacketType   = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=10 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS],
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfCfgRouterSaModificationTsConfig function
* @endinternal
*
* @brief   TS configurations for generic tunnel
*           Set Tunnel Start entry 10 with:
*           tunnelType = Generic, ethertype = 0x1701,
*           MAC DA = 00:00:00:00:44:44
*
*/
static GT_VOID prvTgfCfgRouterSaModificationTsConfig(GT_VOID)
{
    GT_STATUS                         rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT    tunnelEntry;
    GT_U8                             iter;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    tunnelEntry.genCfg.etherType = PRV_TGF_RESERVED_FLOWS_ETHER_TYPE_CNS;
    tunnelEntry.genCfg.genericType = PRV_TGF_TUNNEL_START_GENERIC_MEDIUM_TYPE_E;
    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgfRxTxPacketTunnelPart.tsL2part.daMac, sizeof(TGF_MAC_ADDR));

    for(iter = 0; iter < PRV_TGF_TS_MEDIUM_TYPE_DATA1_SIZE_CNS; iter++)
    {
        tunnelEntry.genCfg.data[iter] = prvTgfRxTxPacketTunnelPart.data1[iter];
    }

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_L2_INDEX_CNS, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
}

/**
* @internal prvTgfCfgGenericPhaConfig function
* @endinternal
*
* @brief   PHA test configurations
*/
static GT_VOID prvTgfCfgGenericPhaConfig
(
    GT_U32    phaThreadId
)
{
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;
    CPSS_DXCH_PHA_THREAD_TYPE_ENT           extType = CPSS_DXCH_PHA_THREAD_TYPE___LAST___E;
    GT_STATUS                               rc = GT_OK;

    /* Load FW img 01 */
    rc = cpssDxChPhaFwImageIdGet(prvTgfDevNum, &prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaFwImageIdGet");
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE,
                         (phaThreadId == PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS) ?
                         CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E : CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit");

    /* AUTODOC: Set the thread entry */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));

    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    switch(phaThreadId)
    {
	case PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS:
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_ROUTER_SA_MODIFICATION_E;
	    break;
	}
	case PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS:
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_INQA_ECN_E;
	    break;
	}
	case PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS:
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_VXLAN_DCI_VNI_REMAP_E;
	    break;
	}
	case PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS:
        {
            extType = CPSS_DXCH_PHA_THREAD_TYPE_MPLS_LSR_IN_STACKING_SYSTEM_E;
            extInfo.mplsLsrInStackingSystem.ownHwDevNum        = ownHwDevNum;
            extInfo.mplsLsrInStackingSystem.targetDevSelectBit = targetDevSelectBit;

            /* AUTODOC: Assign phaThreadId fw thread to target port */
            rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                            GT_TRUE, phaThreadId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");
	    break;
	}
	default:
        {
            PRV_UTF_LOG1_MAC("Invalid PHA thread id:%d\n", phaThreadId);
            break;
	}
    }

    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     phaThreadId,
                                     &commonInfo,
                                     extType,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");
}

/**
* @internal prvTgfCfgRouterSaModificationEpclConfig function
* @endinternal
*
* @brief   EPCL configurations
*           Enable target port for EPCL trigger
*           Enable port mode for config table
*           Set copy reserved mask to PRV_TGF_COPY_RESERVED_MASK
*           Set following EPCL rules
*           10B UDB keys with Routed = 1, isTunneled = 1 and tsPassengerType = 0
*           Action: copyReserved = macSaMsbArray[index] << 2 and macSaLsb = macSaLsbArray[index]
*/
static GT_VOID prvTgfCfgRouterSaModificationEpclConfig
(
    GT_U8    index
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex = 0;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    PRV_TGF_PCL_OFFSET_TYPE_ENT         tgfUdbOffsetType;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    cpssOsSrand(cpssOsTime());
    /* AUTODOC: init PCL Engine for Egress PCL for  */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on target port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfRouterSaModificationEpclMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        switch (prvTgfRouterSaModificationEpclMetadataUdbInfo[udbSelectidx].offsetType)
        {
            case CPSS_DXCH_PCL_OFFSET_L2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L3_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L3_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L4_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L4_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_IPV6_EXT_HDR_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_IPV6_EXT_HDR_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TCP_UDP_COMPARATOR_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L3_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_METADATA_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_METADATA_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L4_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L4_E;
                break;
            default: return;
        }

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 40 Routed[0] */
        /* AUTODOC:   offset 58 isTunneled[5] tsPassengerType[6] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfRouterSaModificationEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        tgfUdbOffsetType,
                                        prvTgfRouterSaModificationEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfRouterSaModificationEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                            CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* Get copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskGet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, &prvTgfRestoreCfg.copyReservedMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskGet in egress direction for lookup 0");

    /* Set copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, PRV_TGF_COPY_RESERVED_MASK);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskSet in egress direction for lookup 0");

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bit 0 is Routed[0] */
    pattern.ruleEgrUdbOnly.udb[0] = 0x01;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfRouterSaModificationEpclMetadataUdbInfo[0].byteMask;

    /* bit 5 is isTunneled and bit 6 is tsPassengerType[6] */
    pattern.ruleEgrUdbOnly.udb[1] = 0x20;
    mask.ruleEgrUdbOnly.udb[1] = prvTgfRouterSaModificationEpclMetadataUdbInfo[1].byteMask;

    ruleFormat                          = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.copyReserved.assignEnable    = GT_TRUE;
    /* Using random value for copy reserved for the packet 3
     * copyReserved MAX value is at index 0 of macSaMsbArray
     */
    macSaMsbArray[index]                = (index == 2) ? cpssOsRand() % (macSaMsbArray[0] + 1) :
                                          macSaMsbArray[index];

    /* For Falcon, copyReserved bit[1:0] are reserved.
     * This means when we try to set 0x1 it actually set 0x4(1 << 2) in the system.
     * Router SA Modification test uses 19:4 of copyReserved.
     * therefore, when we set copyReserved to macSaMsb << 2, it will have macSaMsb at [19:4]
     * For Sip6_10 devices, copyReserved bit[0] is reserved.
     * This means when we try to set 0x1 it actually set 0x2(1 << 1) in the system.
     * Therefore, when we set copyReserved to macSaMsb << 3, it will have macSaMsb at [19:4]
     */
    action.copyReserved.copyReserved    = macSaMsbArray[index] <<
                                          ((PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE) ? 3 : 2);
    action.egressPolicy                 = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ROUTER_SA_MODIFICATION_E;
    action.epclPhaInfo.phaThreadId               = PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS;
    /* Using random value for copy macSaLsb for the packet 3
     * macSaLsb MAX value is at index 1 of macSaLsbArray
     */
    macSaLsbArray[index] = (index == 2) ? cpssOsRand() % macSaLsbArray[1] : macSaLsbArray[index];
    action.epclPhaInfo.phaThreadUnion.epclRouterSaModification.macSaLsb = macSaLsbArray[index];

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfCfgRouterSaModificationConfigurationSet function
* @endinternal
*
* @brief    Target ePort Attribute assignments config
*            Enable tunnel start
*            Set tunnel start pointer = 10
*            Set tunnel generic entry genericType = medium, ethertype = 0x1701
*            and MAC DA = 00:00:00:00:44:44
*            PHA config set entry for Router Sa Modification
*
*/
GT_VOID prvTgfCfgRouterSaModificationConfigurationSet
(
    GT_VOID
)
{
    /* port config */
    prvTgfCfgRouterSaModificationPortConfig();

    /* TS config */
    prvTgfCfgRouterSaModificationTsConfig();

    /* PHA config */
    prvTgfCfgGenericPhaConfig(PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS);
}

/**
* @internal prvTgfCfgRouterSaModificationPacketValidate function
* @endinternal
*
* @brief   Packet Validation
*
*/
static GT_VOID prvTgfCfgRouterSaModificationPacketValidate
(
    IN GT_U8*    packetBuf,
    IN GT_U32    packetCount
)
{
    TGF_MAC_ADDR            saMac;
    GT_STATUS               rc = GT_OK;
    GT_U32                  offset;
    static GT_U32           expectedPacketCount = 0;

    offset = 0;
    /* Validate MAC DA */
    rc = cpssOsMemCmp(&packetBuf[offset], prvTgfRxTxPacketTunnelPart.tsL2part.daMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC DA mismatch");

    /* Validate MAC SA */
    offset = PRV_TGF_L2_MAC_ADDR_SIZE_CNS;

    saMac[0] = (macSaMsbArray[(packetCount - 1)] >> 8) & 0xFF;
    saMac[1] = macSaMsbArray[(packetCount - 1)] & 0xFF;
    saMac[2] = (macSaLsbArray[(packetCount - 1)] >> 24) & 0xFF;
    saMac[3] = (macSaLsbArray[(packetCount - 1)] >> 16) & 0xFF;
    saMac[4] = (macSaLsbArray[(packetCount - 1)] >> 8) & 0xFF;
    saMac[5] = macSaLsbArray[(packetCount - 1)] & 0xFF;

    rc = cpssOsMemCmp(&packetBuf[offset], saMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC SA mismatch");

    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPacketCount + 1, packetCount, "packet count mismatch");
    expectedPacketCount++;

    if((expectedPacketCount % prvTgfBurstCount) == 0)
    {
        expectedPacketCount = 0;
    }
}

/**
* @internal prvTgfCfgRouterSaModificationTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and set EPCL rule for Router Sa Modification
*
*/
GT_VOID prvTgfCfgRouterSaModificationTrafficGenerate
(
    GT_VOID
)
{

    CPSS_INTERFACE_INFO_STC          targetPortInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;
    GT_U8                            packetBuf[256];
    GT_U32                           packetLen          = sizeof(packetBuf);
    GT_U32                           packetActualLength = 0;
    GT_U32                           portIter           = 0;
    GT_U8                            queue              = 0;
    GT_U8                            dev                = 0;
    GT_U8                            iter               = 0;
    GT_BOOL                          getFirst           = GT_TRUE;
    TGF_NET_DSA_STC                  rxParam;
    GT_STATUS                        rc                 = GT_OK;

    /* Sending three packets for each flow corresponding to phaThreadId */
    PRV_UTF_LOG1_MAC("======= Send %d packet =======\n", prvTgfBurstCount);
    for(iter = 0; iter < prvTgfBurstCount; iter++)
    {
        /* EPCL config */
        prvTgfCfgRouterSaModificationEpclConfig(iter);

        /* Enable capture on target port */
        targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
        targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCfgIpv4PacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* Disable capture on target port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* AUTODOC: Get the first rx pkt on target port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);

        tgfTrafficTableRxPcktTblClear();

        /*verify the packet*/
        prvTgfCfgRouterSaModificationPacketValidate(packetBuf, iter + 1);
    }

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfCfgRouterSaModificationPortConfigReset function
* @endinternal
*
* @brief    Reset Port Configuration
*
*/
static GT_VOID prvTgfCfgRouterSaModificationPortConfigReset()
{

    GT_STATUS    rc;

    /* AUTODOC: Reset Egress ePort TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Reset Egress tunnel generic Configuration =======\n");

    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       prvTgfPortsArray[3],
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfCfgRouterSaModificationTsConfigReset function
* @endinternal
*
* @brief   Reset TS configurations for generic tunnel
*
*/
static GT_VOID prvTgfCfgRouterSaModificationTsConfigReset
(
    GT_VOID
)
{
    GT_STATUS                         rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT    tunnelEntry;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_L2_INDEX_CNS, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
}

/**
* @internal prvTgfCfgGenericPhaConfigReset function
* @endinternal
*
* @brief   Reset PHA test configurations
*/
static GT_VOID prvTgfCfgGenericPhaConfigReset
(
    IN GT_U32    phaThreadId
)
{
    GT_STATUS    rc = GT_OK;

    /* AUTODOC: Reset PHA Configuration */
    rc = cpssDxChPhaInit(prvTgfDevNum, GT_TRUE, prvTgfRestoreCfg.phaFwImageId);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaInit");

    if(phaThreadId == PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS)
    {
        rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                        GT_FALSE, phaThreadId);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                     "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for phaThreadId = [%d], rc = [%d]", rc, phaThreadId);
    }
}

/**
* @internal prvTgfCfgGenericEpclConfigReset function
* @endinternal
*
* @brief   Reset EPCL configurations
*/
static GT_VOID prvTgfCfgGenericEpclConfigReset
(
    GT_U32    phaThreadId
)
{
    GT_STATUS rc;
    GT_U8     portIndex = 0;

    switch(phaThreadId)
    {
	case PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS:
        {
	    portIndex = PRV_TGF_CFG_NEXT_HOP_PORT_IDX_CNS;
	    break;
	}
	case PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS:
	case PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS:
        {
	    portIndex = PRV_TGF_CFG_SEND_PORT_IDX_CNS;
	    break;
	}
	default:
        {
            PRV_UTF_LOG1_MAC("Invalid PHA thread id:%d\n", phaThreadId);
            break;
	}
    }

    if(phaThreadId == PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS)
    {
        /* Reset TCP port comparator value */
        rc = prvTgfPclTcpUdpPortComparatorSet(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_L4_PROTOCOL_UDP_E,
                                              PRV_TGF_TCP_UDP_ENTRY_INDEX, prvTgfRestoreCfg.l4PortType,
                                              prvTgfRestoreCfg.compareOperator, prvTgfRestoreCfg.tcpUdpComparatorValue);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclTcpUdpPortComparatorSet: %d", PRV_TGF_TCP_UDP_ENTRY_INDEX);
    }

    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet");

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on target port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[portIndex],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: disable ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[portIndex],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Restore "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* Set copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, prvTgfRestoreCfg.copyReservedMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskSet in egress direction for lookup 0");
}

/**
* @internal prvTgfCfgRouterSaModificationConfigurationRestore function
* @endinternal
*
* @brief    Restore Router SA Modification configuration
*
*/
GT_VOID prvTgfCfgRouterSaModificationConfigurationRestore
(
    GT_VOID
)
{
    /* Reset port config */
    prvTgfCfgRouterSaModificationPortConfigReset();

    /* Reset TS config */
    prvTgfCfgRouterSaModificationTsConfigReset();

    /* Reset PHA config */
    prvTgfCfgGenericPhaConfigReset(PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS);

    /* Reset EPCL config */
    prvTgfCfgGenericEpclConfigReset(PRV_TGF_PHA_THREAD_ID_ROUTER_SA_MODIFICATION_CNS);
}

/**
* @internal prvTgfCfgInqaEcnEpclConfigSet function
* @endinternal
*
* @brief   EPCL configurations
*           Enable target port for EPCL trigger
*           Enable port mode for config table
*           Set copy reserved mask to PRV_TGF_COPY_RESERVED_MASK
*           Set following EPCL rules
*           10B UDB keys with ethertype 0x0800
*           Action: copyReserved = inqaEcnArray[index] << 2 for AC5P and AC5X
*           Action: copyReserved = inqaEcnArray[index] << 1 for Falcon
*/
static GT_VOID prvTgfCfgInqaEcnEpclConfigSet
(
    GT_U8    index
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex = 0;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    PRV_TGF_PCL_OFFSET_TYPE_ENT         tgfUdbOffsetType;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    cpssOsSrand(cpssOsTime());
    /* AUTODOC: init PCL Engine for Egress PCL for  */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on target port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfInqaEcnEpclMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        switch (prvTgfInqaEcnEpclMetadataUdbInfo[udbSelectidx].offsetType)
        {
            case CPSS_DXCH_PCL_OFFSET_L2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L3_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L3_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L4_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L4_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_IPV6_EXT_HDR_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_IPV6_EXT_HDR_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TCP_UDP_COMPARATOR_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L3_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_METADATA_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_METADATA_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L4_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L4_E;
                break;
            default: return;
        }
        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 41 isIp[0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfInqaEcnEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        tgfUdbOffsetType,
                                        prvTgfInqaEcnEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfInqaEcnEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_IPV4_OTHER_E,
                                            CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    /* Get copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskGet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, &prvTgfRestoreCfg.copyReservedMask);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskGet in egress direction for lookup 0");

    /* Set copy reserved mask in egress direction for lookup 0 */
    rc = cpssDxChPclCopyReservedMaskSet(prvTgfDevNum, CPSS_PCL_DIRECTION_EGRESS_E,
                                        CPSS_PCL_LOOKUP_0_E, PRV_TGF_COPY_RESERVED_MASK);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclCopyReservedMaskSet in egress direction for lookup 0");

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bit 0 is isIp[0] */
    pattern.ruleEgrUdbOnly.udb[0] = 0x01;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfInqaEcnEpclMetadataUdbInfo[0].byteMask;

    ruleFormat                          = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.copyReserved.assignEnable    = GT_TRUE;
    /* Using random value for copy reserved for the packet 3
     * copyReserved MAX value is at index 1 of inqaEcnArray
     */
    inqaEcnArray[index]                 = (index == 2) ? cpssOsRand() % (inqaEcnArray[1] + 1) :
                                          inqaEcnArray[index];

    /* For Falcon, copyReserved bit[1:0] are reserved.
     * This means when we try to set 0x1 it actually set 0x4(1 << 2) in the system.
     * INQA ECN test uses 5:3 of copyReserved.
     * therefore, when we set copyReserved to inqaEcn << 1, it will have macSaMsb at [5:3]
     * For Sip6_10 devices, copyReserved bit[0] is reserved.
     * This means when we try to set 0x1 it actually set 0x2(1 << 1) in the system.
     * Therefore, when we set copyReserved to inqaEcn << 2, it will have macSaMsb at [5:3]
     */
    action.copyReserved.copyReserved    = inqaEcnArray[index] <<
                                          ((PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum) == GT_TRUE) ? 2 : 1);
    action.egressPolicy                 = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadType    = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_TYPE_NOT_USED_E;
    action.epclPhaInfo.phaThreadId      = PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfInqaEcnConfigurationSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId = PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS.
*    configure VLAN
*    configure PHA
*/
void prvTgfCfgInqaEcnConfigurationSet
(
    GT_VOID
)
{
    /* bridge config */
    prvTgfCfgReservedFlowsBridgeConfigSet();

    /* PHA config */
    prvTgfCfgGenericPhaConfig(PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS);
}

/**
* @internal prvTgfCfgInqaEcnPacketValidate function
* @endinternal
*
* @brief   Packet Validation
*
*/
static GT_VOID prvTgfCfgInqaEcnPacketValidate
(
    IN GT_U8*    packetBuf,
    IN GT_U32    packetCount
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  offset;
    static GT_U32           expectedPacketCount = 0;
    GT_U8                   ecnBits = 0;
    GT_U8                   reservedFlag = 0;

    offset = 0;
    /* Validate MAC DA */
    rc = cpssOsMemCmp(&packetBuf[offset], prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC DA mismatch");

    /* inqaEcnArray[packetCount - 1][2:0] indicates values at copy_reserved[5:3]*/
    /* Validate ECN bits */
    offset  = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + 1; /* This gives offset of dscp field in IP header*/

    /* pcketBuf[offset][1:0] should be equal to copy_reserved[4:3]*/
    ecnBits = 0x03 & inqaEcnArray[packetCount - 1];
    UTF_VERIFY_EQUAL0_STRING_MAC(ecnBits, packetBuf[offset] & 0x03, "ECN bits mismatch");

    /* Validate ECN bits */
    offset  += 5; /* This gives offset of flags in IP header */

    /* pcketBuf[offset][7] should be equal to copy_reserved[5]*/
    reservedFlag = 0x04 & inqaEcnArray[packetCount - 1];
    UTF_VERIFY_EQUAL0_STRING_MAC(reservedFlag, (packetBuf[offset] >> 5) & 0x04, "reservedFlag bits mismatch");

    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPacketCount + 1, packetCount, "packet count mismatch");
    expectedPacketCount++;

    if((expectedPacketCount % prvTgfBurstCount) == 0)
    {
        expectedPacketCount = 0;
    }
}

/**
* @internal prvTgfCfgInqaEcnTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and set EPCL rule for INQA ECN
*          send IP UC traffic with verify expected traffic on target port
*
*/
GT_VOID prvTgfCfgInqaEcnTrafficGenerate
(
    GT_VOID
)
{

    CPSS_INTERFACE_INFO_STC          targetPortInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;
    GT_U8                            packetBuf[256];
    GT_U32                           packetLen          = sizeof(packetBuf);
    GT_U32                           packetActualLength = 0;
    GT_U32                           portIter           = 0;
    GT_U8                            queue              = 0;
    GT_U8                            dev                = 0;
    GT_U8                            iter               = 0;
    GT_BOOL                          getFirst           = GT_TRUE;
    TGF_NET_DSA_STC                  rxParam;
    GT_STATUS                        rc                 = GT_OK;

    /* Sending three packets for each flow corresponding to phaThreadId */
    PRV_UTF_LOG1_MAC("======= Send %d packet =======\n", prvTgfBurstCount);
    for(iter = 0; iter < prvTgfBurstCount; iter++)
    {
        /* EPCL config */
        prvTgfCfgInqaEcnEpclConfigSet(iter);

        /* Enable capture on target port */
        targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
        targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCfgIpv4PacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* Disable capture on target port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* AUTODOC: Get the first rx pkt on target port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);

        tgfTrafficTableRxPcktTblClear();

        /*verify the packet*/
        prvTgfCfgInqaEcnPacketValidate(packetBuf, iter + 1);
    }

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfCfgInqaEcnConfigurationRestore function
* @endinternal
*
* @brief    Restore INQA ECN configuration
*
*/
GT_VOID prvTgfCfgInqaEcnConfigurationRestore
(
    GT_VOID
)
{
    /* Reset Bridge config */
    prvTgfCfgReservedFlowsBridgeConfigReset();

    /* Reset PHA config */
    prvTgfCfgGenericPhaConfigReset(PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS);

    /* Reset EPCL config */
    prvTgfCfgGenericEpclConfigReset(PRV_TGF_PHA_THREAD_ID_INQA_ECN_CNS);
}

/**
* @internal prvTgfCfgVxlanDciVniRemapEpclConfigSet function
* @endinternal
*
* @brief   EPCL configurations
*           Enable target port for EPCL trigger
*           Enable port mode for config table
*           Set the following EPCL rule
*           10B UDB keys with IPv4 UDP
*           Action: vxlanVni = vxlanVniArray[index]
*/
static GT_VOID prvTgfCfgVxlanDciVniRemapEpclConfigSet
(
    GT_U8    index
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex = 0;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    PRV_TGF_PCL_OFFSET_TYPE_ENT         tgfUdbOffsetType;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    cpssOsSrand(cpssOsTime());
    /* AUTODOC: init PCL Engine for Egress PCL for  */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on target port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* Get TCP port comparator value */
    rc = prvTgfPclTcpUdpPortComparatorGet(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_L4_PROTOCOL_UDP_E,
                                          PRV_TGF_TCP_UDP_ENTRY_INDEX, &prvTgfRestoreCfg.l4PortType,
                                          &prvTgfRestoreCfg.compareOperator, &prvTgfRestoreCfg.tcpUdpComparatorValue);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclTcpUdpPortComparatorGet: %d", PRV_TGF_TCP_UDP_ENTRY_INDEX);

    /* Configure TCP port comparator value */
    rc = prvTgfPclTcpUdpPortComparatorSet(CPSS_PCL_DIRECTION_EGRESS_E, CPSS_L4_PROTOCOL_UDP_E,
                                          PRV_TGF_TCP_UDP_ENTRY_INDEX, CPSS_L4_PROTOCOL_PORT_DST_E,
                                          CPSS_COMPARE_OPERATOR_LTE, PRV_TGF_UDP_VXLAN_PORT_NUM);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclTcpUdpPortComparatorSet: %d", PRV_TGF_TCP_UDP_ENTRY_INDEX);

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[udbSelectidx].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
            break;

        switch (prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[udbSelectidx].offsetType)
        {
            case CPSS_DXCH_PCL_OFFSET_L2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L3_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L3_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L4_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L4_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_IPV6_EXT_HDR_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_IPV6_EXT_HDR_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TCP_UDP_COMPARATOR_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TCP_UDP_COMPARATOR_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_L3_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_MPLS_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L3_MINUS_2_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_METADATA_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_METADATA_E;
                break;
            case CPSS_DXCH_PCL_OFFSET_TUNNEL_L4_E:
                tgfUdbOffsetType = PRV_TGF_PCL_OFFSET_TUNNEL_L4_E;
                break;
            default: return;
        }
        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 42  TCP/UDP Port Comparators */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_L4_E */
        /* AUTODOC:   offset 12  oldVxlanVni[23:16] */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_L4_E */
        /* AUTODOC:   offset 13  oldVxlanVni[15:8] */
        /* AUTODOC:   offsetType CPSS_DXCH_PCL_OFFSET_L4_E */
        /* AUTODOC:   offset 14  oldVxlanVni[7:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        tgfUdbOffsetType,
                                        prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                            PRV_TGF_PCL_PACKET_TYPE_IPV4_UDP_E,
                                            CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);

    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    oldVxlanVni = cpssOsRand() % (vxlanVniArray[1] + 1);

    /* bit 0 is TCP/UDP Port Comparators for entryIndex PRV_TGF_TCP_UDP_ENTRY_INDEX */
    pattern.ruleEgrUdbOnly.udb[0] = 0x01;
    mask.ruleEgrUdbOnly.udb[0]    = prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[0].byteMask;
    pattern.ruleEgrUdbOnly.udb[1] = (oldVxlanVni >> 16) & 0xFF; /* oldVxlanVni[23:16] */
    mask.ruleEgrUdbOnly.udb[1]    = prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[1].byteMask;
    pattern.ruleEgrUdbOnly.udb[2] = (oldVxlanVni >> 8) & 0xFF;  /* oldVxlanVni[15:8] */
    mask.ruleEgrUdbOnly.udb[2]    = prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[2].byteMask;
    pattern.ruleEgrUdbOnly.udb[3] = oldVxlanVni & 0xFF;         /* oldVxlanVni[7:0] */
    mask.ruleEgrUdbOnly.udb[3]    = prvTgfVxlanDciVniRemapEpclMetadataUdbInfo[3].byteMask;

    ruleFormat                          = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    /* Using random value for VXLAN VNI for the packet 3
     * vxlanVni MAX value is at index 1 of vxlanVniArray
     */
    vxlanVniArray[index]                = (index == 2) ? cpssOsRand() % (vxlanVniArray[1] + 1) : vxlanVniArray[index];
    action.egressPolicy                 = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadType    = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_VXLAN_DCI_VNI_REMAP_E;
    action.epclPhaInfo.phaThreadId      = PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS;
    action.epclPhaInfo.phaThreadUnion.epclVxlanVni.vxlanVni = vxlanVniArray[index];

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                                "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfCfgVxlanDciVniRemapConfigurationSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId = PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS.
*    configure VLAN
*    configure PHA
*/
void prvTgfCfgVxlanDciVniRemapConfigurationSet
(
    GT_VOID
)
{
    /* bridge config */
    prvTgfCfgReservedFlowsBridgeConfigSet();

    /* PHA config */
    prvTgfCfgGenericPhaConfig(PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS);
}

/**
* @internal prvTgfCfgVxlanDciVniRemapPacketValidate function
* @endinternal
*
* @brief   Packet Validation
*
*/
static GT_VOID prvTgfCfgVxlanDciVniRemapPacketValidate
(
    IN GT_U8*    packetBuf,
    IN GT_U32    packetCount
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  offset = 0;
    static GT_U32           expectedPacketCount = 0;
    GT_U32                  vni = 0;

    offset = 0;
    /* Validate MAC DA */
    rc = cpssOsMemCmp(&packetBuf[offset], prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC DA mismatch");

    /* Validate VXLAN VNI */
    offset += TGF_L2_HEADER_SIZE_CNS;
    offset += TGF_ETHERTYPE_SIZE_CNS;
    offset += TGF_IPV4_HEADER_SIZE_CNS;
    offset += TGF_UDP_HEADER_SIZE_CNS;
    offset += (TGF_VXLAN_GPE_HEADER_SIZE_CNS >> 1); /* This gives offset of VXLAN field in UDP header */

    vni = ((packetBuf[offset] << 16) | (packetBuf[offset + 1] << 8) | packetBuf[offset + 2]);
    UTF_VERIFY_EQUAL0_STRING_MAC(vxlanVniArray[packetCount - 1], vni, "VXLAN VNI mismatch");

    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPacketCount + 1, packetCount, "packet count mismatch");
    expectedPacketCount++;

    if((expectedPacketCount % prvTgfBurstCount) == 0)
    {
        expectedPacketCount = 0;
    }
}

/**
* @internal prvTgfCfgVxlanDciVniRemapTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic and set EPCL rule for VXLAN DCI VNI REMAP
*          send IP UDP UC traffic with verify expected traffic on target port
*
*/
GT_VOID prvTgfCfgVxlanDciVniRemapTrafficGenerate
(
    GT_VOID
)
{

    CPSS_INTERFACE_INFO_STC          targetPortInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;
    GT_U8                            packetBuf[256];
    GT_U32                           packetLen          = sizeof(packetBuf);
    GT_U32                           packetActualLength = 0;
    GT_U32                           portIter           = 0;
    GT_U8                            queue              = 0;
    GT_U8                            dev                = 0;
    GT_U8                            iter               = 0;
    GT_BOOL                          getFirst           = GT_TRUE;
    TGF_NET_DSA_STC                  rxParam;
    GT_STATUS                        rc                 = GT_OK;

    /* Sending three packets for each flow corresponding to phaThreadId */
    PRV_UTF_LOG1_MAC("======= Send %d packet =======\n", prvTgfBurstCount);
    for(iter = 0; iter < prvTgfBurstCount; iter++)
    {
        /* EPCL config */
        prvTgfCfgVxlanDciVniRemapEpclConfigSet(iter);

        /* Enable capture on target port */
        targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
        targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* save VNI bytes */
        prvTgfRestoreCfg.udpPayload[0] = prvTgfPayloadDataArr[4];
        prvTgfRestoreCfg.udpPayload[1] = prvTgfPayloadDataArr[5];
        prvTgfRestoreCfg.udpPayload[2] = prvTgfPayloadDataArr[6];

        prvTgfPayloadDataArr[4] = (oldVxlanVni >> 16) & 0xFF; /* oldVxlanVni[23:16] */
        prvTgfPayloadDataArr[5] = (oldVxlanVni >> 8) & 0xFF;  /* oldVxlanVni[15:8] */
        prvTgfPayloadDataArr[6] = oldVxlanVni & 0xFF;         /* oldVxlanVni[7:0] */

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfUdpPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* Disable capture on target port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* AUTODOC: Get the first rx pkt on target port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);

        tgfTrafficTableRxPcktTblClear();

        /*verify the packet*/
        prvTgfCfgVxlanDciVniRemapPacketValidate(packetBuf, iter + 1);

        /* Restore VNI bytes */
        prvTgfPayloadDataArr[4] = prvTgfRestoreCfg.udpPayload[0];
        prvTgfPayloadDataArr[5] = prvTgfRestoreCfg.udpPayload[1];
        prvTgfPayloadDataArr[6] = prvTgfRestoreCfg.udpPayload[2];
    }

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfCfgVxlanDciVniRemapConfigurationRestore function
* @endinternal
*
* @brief    Restore VXLAN DCI VNI REMAP configuration
*
*/
GT_VOID prvTgfCfgVxlanDciVniRemapConfigurationRestore
(
    GT_VOID
)
{
    /* Reset Bridge config */
    prvTgfCfgReservedFlowsBridgeConfigReset();

    /* Reset PHA config */
    prvTgfCfgGenericPhaConfigReset(PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS);

    /* Reset EPCL config */
    prvTgfCfgGenericEpclConfigReset(PRV_TGF_PHA_THREAD_ID_VXLAN_DCI_VNI_REMAP_CNS);
}

/**
* @internal prvTgfCfgMplsLsrInStackingSystemBridgeConfigSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*         Add FDB entry with MAC 00:00:00:00:34:01, port prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS]
*         Set E2PHY mapping for port prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS]
*/
static void prvTgfCfgMplsLsrInStackingSystemBridgeConfigSet()
{
    PRV_TGF_BRG_MAC_ENTRY_STC    prvTgfMacEntry;
    CPSS_INTERFACE_INFO_STC      physicalInfo;
    GT_STATUS                    rc;


    /* AUTODOC: create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d",
                                 PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);
    /* get HW device number */
    rc = prvUtfHwDeviceNumberGet(prvTgfDevNum, &prvTgfTrgDev);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                                 "prvUtfHwDeviceNumberGet: %d, %d",
                                 prvTgfDevNum, prvTgfTrgDev);

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:01, port prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS] */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    /* AUTODOC: Set FDB entry with remoteDev, ePort */
    prvTgfMacEntry.key.key.macVlan.vlanId = PRV_TGF_RESERVED_FLOW_TEST_VID_CNS;
    prvTgfMacEntry.isStatic = GT_TRUE;
    prvTgfMacEntry.daRoute  = GT_TRUE;
    prvTgfMacEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
    prvTgfMacEntry.dstInterface.devPort.hwDevNum = prvTgfTrgDev;
    prvTgfMacEntry.dstInterface.hwDevNum = prvTgfTrgDev;
    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /* AUTODOC: Set E2PHY mapping for port prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS]
     * This is requred when prvTgfDevNum != 16 as by default E2PHY uses hwDevNum = 16
     */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                           prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                                           &(prvTgfRestoreCfg.physicalInfo));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableGet: %d", prvTgfDevNum);

    physicalInfo.type             = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = prvTgfDevNum;
    physicalInfo.devPort.portNum  = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                                           &physicalInfo);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCfgMplsLsrInStackingSystemConfigSet function
* @endinternal
*
* @brief  Test PHA thread with given phaThreadId = PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS.
*         configure Bridge
*/
void prvTgfCfgMplsLsrInStackingSystemConfigSet
(
    GT_VOID
)
{
    cpssOsSrand(cpssOsTime());
    /* bridge config */
    prvTgfCfgMplsLsrInStackingSystemBridgeConfigSet();
}

/**
* @internal prvTgfCfgMplsLsrInStackingSystemPacketValidate function
* @endinternal
*
* @brief   Packet Validation
*
*/
static GT_VOID prvTgfCfgMplsLsrInStackingSystemPacketValidate
(
    IN GT_U8*    packetBuf,
    IN GT_U32    packetCount
)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  offset = 0;
    static GT_U32           expectedPacketCount = 0;
    GT_HW_DEV_NUM           trgDev   = 0;
    GT_U32                  trgEport = 0;
    GT_HW_DEV_NUM           expectedTrgDev   = 0;
    GT_U32                  expectedTrgEport = 0;

    offset = 0;
    /* Validate MAC DA */
    rc = cpssOsMemCmp(&packetBuf[offset], prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "MAC DA mismatch");

    /* Validate eDSA Trg eport */
    offset += TGF_L2_HEADER_SIZE_CNS;

    /* This gives byte 0 of word[1] of eDSA tag */
    offset += (TGF_DSA_TAG_SIZE_CNS - 1);

    /* trgDev[4:0] */
    trgDev = packetBuf[offset] & 0x1F;

    /* This gives byte 2 of word[3] of eDSA tag */
    offset += 6;

    /* trgEport[15:0] */
    trgEport = ((packetBuf[offset] & 0x7F) << 9) | (packetBuf[offset + 1] << 1) | ((packetBuf[offset + 2] >> 7) & 0x01);

    /* trgDev[9:0] */
    trgDev |= ((packetBuf[offset + 2] & 0x1F) << 5);

    /* Validate eDSA Trg Dev */
    expectedTrgDev = (packetCount == 1) ? prvTgfTrgDev ^ targetDevSelectBit : prvTgfTrgDev;
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedTrgDev, trgDev, "Target Dev mismatch");

    /* Validate eDSA Trg ePort */
    expectedTrgEport = (packetCount == 1) ? prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS] : PRV_TGF_CFG_TARGET_EPORT_CNS;
    UTF_VERIFY_EQUAL0_STRING_MAC(expectedTrgEport, trgEport, "Target ePort mismatch");

    UTF_VERIFY_EQUAL0_STRING_MAC(expectedPacketCount + 1, packetCount, "packet count mismatch");
    expectedPacketCount++;

    if((expectedPacketCount % (prvTgfBurstCount - 1)) == 0)
    {
        expectedPacketCount = 0;
    }
}

/**
* @internal prvTgfCfgMplsLsrInStackingSystemTrafficGenerate function
* @endinternal
*
* @brief   PHA config
*          Generate traffic
*          send DSA tagged traffic verify expected traffic on target port
*
*/
GT_VOID prvTgfCfgMplsLsrInStackingSystemTrafficGenerate
(
    GT_VOID
)
{

    CPSS_INTERFACE_INFO_STC          targetPortInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC    portCntrs;
    GT_U8                            packetBuf[256];
    GT_U32                           packetLen          = sizeof(packetBuf);
    GT_U32                           packetActualLength = 0;
    GT_U32                           portIter           = 0;
    GT_U8                            queue              = 0;
    GT_U8                            dev                = 0;
    GT_U8                            iter               = 0;
    GT_BOOL                          getFirst           = GT_TRUE;
    TGF_NET_DSA_STC                  rxParam;
    GT_STATUS                        rc                 = GT_OK;
    GT_HW_DEV_NUM                    hwDevNum           = 0;
    GT_U32                           portNum            = 0;
    GT_U32                           dstEport           = 0;

    /* Sending two packets */
    PRV_UTF_LOG1_MAC("======= Send %d packet =======\n", prvTgfBurstCount - 1);
    for(iter = 0; iter < prvTgfBurstCount - 1; iter++)
    {
        if(iter == 0)
        {
            do
            {
                ownHwDevNum = cpssOsRand() % PRV_TGF_MAX_TRG_DEV;
            } while (ownHwDevNum == prvTgfTrgDev);
        }
        else
        {
            ownHwDevNum = prvTgfTrgDev;
        }

        targetDevSelectBit = cpssOsRand() % BIT_1;
        PRV_UTF_LOG3_MAC("======= prvTgfTrgDev 0x%08x ownHwDevNum 0x%08x targetDevSelectBit %d =======\n",
                         prvTgfTrgDev, ownHwDevNum, targetDevSelectBit);

        /* PHA config */
        prvTgfCfgGenericPhaConfig(PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS);

        /* Enable capture on target port */
        targetPortInterface.type              = CPSS_INTERFACE_PORT_E;
        targetPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        targetPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* Save hwDevNum, portNum and dstEport */
        hwDevNum = prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum;
        portNum  = prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum;
        dstEport = prvTgfPacketDsaTagPart_forward.dstEport;

        prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = prvTgfTrgDev;
        prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS];
        prvTgfPacketDsaTagPart_forward.dstEport                      = PRV_TGF_CFG_TARGET_EPORT_CNS;

        prvTgfPacketDsaTagPart.dsaInfo.forward = prvTgfPacketDsaTagPart_forward;

        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfDsaPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_CFG_RECV_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* Disable capture on target port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* AUTODOC: Get the first rx pkt on target port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&targetPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);

        tgfTrafficTableRxPcktTblClear();

        /*verify the packet*/
        prvTgfCfgMplsLsrInStackingSystemPacketValidate(packetBuf, iter + 1);

        /* Restore hwDevNum, portNum and dstEport */
        prvTgfPacketDsaTagPart_forward.dstInterface.devPort.hwDevNum = hwDevNum;
        prvTgfPacketDsaTagPart_forward.dstInterface.devPort.portNum  = portNum;
        prvTgfPacketDsaTagPart_forward.dstEport                      = dstEport;
    }

    /* read and check ethernet counters */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);
    }
}

/**
* @internal prvTgfCfgMplsLsrInStackingSystemBridgeConfigReset function
* @endinternal
*
* @brief  Restore Bridge config
*         Restore E2PHY mapping
*/
static void prvTgfCfgMplsLsrInStackingSystemBridgeConfigReset()
{
    GT_STATUS    rc;


    /* invalidate VLAN entry */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d",
                                 PRV_TGF_RESERVED_FLOW_TEST_VID_CNS);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", GT_TRUE);

    /* AUTODOC: Restore E2PHY mapping */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                           prvTgfPortsArray[PRV_TGF_CFG_SEND_PORT_IDX_CNS],
                                                           &(prvTgfRestoreCfg.physicalInfo));
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgEportToPhysicalPortTargetMappingTableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfCfgMplsLsrInStackingSystemConfigRestore function
* @endinternal
*
* @brief    Restore MPLS LSR In Stacking System configuration
*
*/
GT_VOID prvTgfCfgMplsLsrInStackingSystemConfigRestore
(
    GT_VOID
)
{
    /* Reset Bridge config */
    prvTgfCfgMplsLsrInStackingSystemBridgeConfigReset();

    /* Reset PHA config */
    prvTgfCfgGenericPhaConfigReset(PRV_TGF_PHA_THREAD_ID_MPLS_LSR_IN_STACKING_SYSTEM_CNS);
}
