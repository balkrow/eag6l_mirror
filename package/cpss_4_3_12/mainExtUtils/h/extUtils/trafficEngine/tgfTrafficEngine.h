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
* @file tgfTrafficEngine.h
*
* @brief -- packet generator , parser.
* -- with trace capabilities
* -- each packet can be built from :
* -- L2 - mac da,sa
* -- vlan tag (optional) -- single/double tag(Q-in-Q)
* -- Ethernet encapsulation (optional) -- LLC/SNAP/etherII/Novell
* -- ether type
* -- L3 (optional) - ipv4/6 / mpls / arp / ..
* -- L4 (optional) - tcp / udp / icmp / IGMPv2,3,6 / MLD / RIPv1,2 ..
* -- payload
* -- CRC32
* -- support tunneling (tunnel + passenger)
* -- each passenger is like a new packet
* -- each packet has - total length
* -- other specific info:
* -- table index , num packets , waitTime
* -- Tx parameters (TGF_DSA_TX_PARAMS_STC):
* - DSA tag info , txSynch , invokeTxBufferQueueEvent , txQueue ...
* -- for Rx traffic : CPU code (for 'to_cpu')
* -- the engine need CB function to get the CPU code of this frame.
*
* -- special CPU codes:
* list of CPU codes that can be attached to CB for advanced purposes:
* distinguish between the 'rx in cpu'(any CPU code) and the 'Tx port capture' (tx_analyzer cpu code)
*
*
* -- basic init test capabilities:
* -- set port in loopback mode
* -- set port with force link up
* -- emulate SMB port counters:
* -- read/clear port counter : all/rx/tx all/packets/bytes
* -- 'capture' - set port as 'Egress mirror port' / use STC Sampling to cpu
* - set the CPU as the 'Tx analyzer port'
* - CPU will get those frames with specific CPU code.
*
* -- emulate SMB send to port:
* -- fill table of frames to 'Ingress the device' from the port
* (port should be in: loopback mode and in link up)
* -- send all / specific table index
* -- done in specific task.
* -- this also allow use to break the sending if we want from terminal
* -- set FLAG 'Break Sending' from terminal so task will stop sending
*
*
*
* -- support for multi devices types (Dx/ExMxPm/ExMx):
* -- the engine of build/parse should not care about it.
* -- the send/receive of packet (a 'table')in CPU will call CB with specific
* Tx parameters (TGF_DSA_TX_PARAMS_STC) /
* Rx parameter (TGF_DSA_RX_PARAMS_STC)
* and attach those parameter to the frame -->
* each entry in table point to ' cookie + CB' that hold the specific Rx/Tx info
*
*
* -- SMP port emulation:
* -- read/clear counters - attach CB for read counters (per device , per port)
* -- 'capture' - attach CB for set CPU as analyzer , CB for set port as tx mirrored port
* CB for set Tx port with Sampling to CPU
*
*
*
*
* @note 'test' can distinguish between : rx and ' capture of tx' by the
* mechanism of : 'Special CPU codes'
*
* -SMB trigger emulation - TBD
* -Expected results - TBD
* -Predefined traffic flows - TBD
*
*
* @version   34
********************************************************************************
*/
#ifndef __tgfTrafficEngineh
#define __tgfTrafficEngineh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cos/cpssCosTypes.h>
#include <cpss/common/networkIf/cpssGenNetIfTypes.h>
#ifdef CHX_FAMILY
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#else
typedef enum { CPSS_NET_RX_CPU_CODE_ENT_E } CPSS_NET_RX_CPU_CODE_ENT;
#endif /*CHX_FAMILY*/

/******************************************************************************\
 *                              Define section                                *
\******************************************************************************/

/* 4 bytes for CRC*/
#define TGF_CRC_LEN_CNS                 4

/* ether type used for "vlan tag" */
#define TGF_ETHERTYPE_8100_VLAN_TAG_CNS 0x8100

/* ether type example that can be used for "double vlan tag" */
#define TGF_ETHERTYPE_9100_TAG_CNS      0x9100

/* ether type used for L2 */
#define TGF_ETHERTYPE_88F7_L2_TAG_CNS   0x88F7

/* ether type used for MPLS */
#define TGF_ETHERTYPE_8847_MPLS_TAG_CNS 0x8847

/* ether type used for IPv4 */
#define TGF_ETHERTYPE_0800_IPV4_TAG_CNS 0x0800

/* ether type used for IPv6 */
#define TGF_ETHERTYPE_86DD_IPV6_TAG_CNS 0x86DD

/* ether type used for ARP */
#define TGF_ETHERTYPE_0806_ARP_TAG_CNS  0x0806

/* ether type used for MIM */
#define TGF_ETHERTYPE_88E7_MIM_TAG_CNS  0x88E7

/* ether type used for 'Service provider vlan tag' */
#define TGF_ETHERTYPE_88A8_SERVICE_TAG_CNS  0x88A8

/* ether type used for TRILL */
#define TGF_ETHERTYPE_22F3_TRILL_TAG_CNS  0x22F3

/* ether type used for MACSec */
#define TGF_ETHERTYPE_88E5_MACSEC_TAG_CNS  0x88E5

/* ether type used for OAM */
#define TGF_ETHERTYPE_8902_OAM_TAG_CNS  0x8902

/* ether type used for FCOE */
#define TGF_ETHERTYPE_8906_OAM_TAG_CNS  0x8906

/* ether type used for NSH (Network Service Header) */
#define TGF_ETHERTYPE_894F_NSH_TAG_CNS  0x894F

/* Flow Control Packet Ethertype */
#define TGF_ETHERTYPE_8808_FC_TAG_CNS   0x8808

/* Security Group Protocol - Packet Ethertype */
#define TGF_ETHERTYPE_8909_SGT_TAG_CNS   0x8909

/* ether type that not used by any protocol   */
#define TGF_ETHERTYPE_NON_VALID_TAG_CNS  0xFFFF

/* IEEE 802.3x Flow Control opcode */
#define TGF_FC_OPCODE_0001_TAG_CNS 0x0001

/* IEEE 802.3bd/802.1Qbb PFC opcode */
#define TGF_PFC_OPCODE_0101_TAG_CNS 0x0101

/* L2 header size (in bytes) */
#define TGF_L2_HEADER_SIZE_CNS          12

/* VLAN Tag size (in bytes) */
#define TGF_VLAN_TAG_SIZE_CNS           4

/* ETHERTYPE size (in bytes) */
#define TGF_ETHERTYPE_SIZE_CNS          2

/* MPLS header size (in bytes) */
#define TGF_MPLS_HEADER_SIZE_CNS        4

/* IPv4 header size (in bytes) */
#define TGF_IPV4_HEADER_SIZE_CNS        20

/* IPv6 header size (in bytes) */
#define TGF_IPV6_HEADER_SIZE_CNS        40

/* TCP header size (in bytes) */
#define TGF_TCP_HEADER_SIZE_CNS         20

/* UDP header size (in bytes) */
#define TGF_UDP_HEADER_SIZE_CNS         8

/* ARP header size (in bytes) */
#define TGF_ARP_HEADER_SIZE_CNS         28

/* ARPv6 header size (in bytes) */
#define TGF_ARPV6_HEADER_SIZE_CNS       52

/* ICMP header size (in bytes) */
#define TGF_ICMP_HEADER_SIZE_CNS        8

/* TRILL header size (in bytes) */
#define TGF_TRILL_HEADER_SIZE_CNS       6

/* VXLAN-GPE header size (in bytes) */
#define TGF_VXLAN_GPE_HEADER_SIZE_CNS   8

/* NSH header size in bytes. Includes Base hdr(4B)+Service Path hdr(4B)+NSH metadata(16B) */
#define TGF_NSH_HEADER_SIZE_CNS         24

/* GRE header size (in bytes) */
#define TGF_GRE_HEADER_SIZE_CNS         4

/* IOAM Shim header size (in bytes) */
#define TGF_IOAM_SHIM_HEADER_SIZE_CNS   4

/* IOAM Trace header size (in bytes) */
#define TGF_IOAM_TRACE_HEADER_SIZE_CNS  4

/* DSA tag size (in bytes) */
#define TGF_DSA_TAG_SIZE_CNS            8

/* SGT tag size (in bytes) */
#define TGF_SGT_TAG_SIZE_CNS            8

/* eDSA tag size (in bytes) */
#define TGF_eDSA_TAG_SIZE_CNS           16

/* PTP V2 Header size (in bytes) */
#define TGF_PTP_V2_HDR_SIZE_CNS         34

/* SRH Header size size (in bytes) */
#define TGF_SRH_HEADER_SIZE_CNS          8

/* number of bytes in the pattern of VFD */
#define TGF_VFD_PATTERN_BYTES_NUM_CNS   16

#define TGF_IPV4_GRE_PROTOCOL_CNS       0x2f

/* IFA header size (in bytes) */
#define TGF_IFA_HEADER_SIZE_CNS         28

/* IFA metadata size (in bytes) */
#define TGF_IFA_METADATA_SIZE_CNS       32

/* set compiler to avoid non used parameter of the function */
#define TGF_PARAM_NOT_USED(x)   (GT_VOID) x

/* reset parameter according to it's size */
#define TGF_RESET_PARAM_MAC(param) cpssOsMemSet(&(param), 0, sizeof(param))

/* check GT_STATUS rc parameter */
#define TGF_RC_CHECK_MAC(rc) \
    if (GT_OK != rc)         \
    {                        \
        return rc;           \
    }


/******************************************************************************\
 *                              Common types section                          *
\******************************************************************************/

/* define common types */
typedef GT_U8   TGF_MAC_ADDR[6];
typedef GT_U8   TGF_IPV4_ADDR[4];
typedef GT_U16  TGF_IPV6_ADDR[8];
typedef GT_U16  TGF_VLAN_ID;
typedef GT_U8   TGF_CFI;
typedef GT_U8   TGF_PRI;
typedef GT_U16  TGF_L4_PORT;
typedef GT_U16  TGF_ETHER_TYPE;
typedef GT_U8   TGF_PROT_VER;
typedef GT_U8   TGF_TYPE_OF_SERVICE;
typedef GT_U16  TGF_IPV4_ID_FIELD;
typedef GT_U16  TGF_ICMP_ID_FIELD;
typedef GT_U8   TGF_TCP_FLAGS;
typedef GT_U8   TGF_TTL;
typedef GT_U8   TGF_PROT;
typedef GT_U16  TGF_HEADER_CRC;
typedef GT_U8   TGF_TRAFFIC_CLASS;
typedef GT_U32  TGF_FLOW_LABEL;
typedef GT_U8   TGF_HEXT_HEADER;
typedef GT_U8   TGF_HOP_LIMIT;
typedef GT_U16  TGF_TCP_URGENT_PTR;
typedef GT_U16  TGF_ARP_HW_TYPE;
typedef GT_U16  TGF_PROT_TYPE;
typedef GT_U8   TGF_MPLS_STACK;
typedef GT_U8   TGF_MPLS_EXP;
typedef GT_U32  TGF_MPLS_LBL;
typedef GT_U8   TGF_FLAG;
typedef GT_U16  TGF_NEXT_HEADER;


/******************************************************************************\
 *                          Enumerations definitions                          *
\******************************************************************************/
/**
* @enum PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT
 *
 * @brief Enumeration of tagging egress commands
*/
typedef enum{

    /** @brief if Tag0 and/or Tag1
     *  were classified in
     *  the incoming packet,
     *  they are removed
     *  from the packet.
     */
    PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E,

    /** @brief packet egress with
     *  Tag0 (Tag0 as
     *  defined in ingress
     *  pipe).
     */
    PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E,

    /** @brief packet egress with
     *  Tag1 (Tag1 as
     *  defined in ingress
     *  pipe).
     */
    PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E,

    /** @brief Outer Tag0, Inner
     *  Tag1 (tag swap).
     */
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E,

    /** @brief Outer Tag1, Inner
     *  Tag0 (tag swap).
     */
    PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E,

    /** @brief TAG0 is added to
     *  the packet
     *  regardless of
     *  whether Tag0 and
     *  TAG1 were
     *  classified in the
     *  incoming packet.
     */
    PRV_TGF_BRG_VLAN_PORT_PUSH_TAG0_CMD_E,

    /** @brief the incoming packet
     *  outer tag is
     *  removed, regardless
     *  of whether it is
     *  Tag0 or Tag1. This
     *  operation is a NOP
     *  if the packet
     *  arrived with neither
     *  Tag0 nor Tag1
     *  classified.
     */
    PRV_TGF_BRG_VLAN_PORT_POP_OUTER_TAG_CMD_E,

    /** @brief Do not modify any tag,
     *  packet tags are sent
     *  as received (APPLICABLE DEVICES: Lion2)
     *  Note:
     *  Relevant for devices with TR101 feature support
     */
    PRV_TGF_BRG_VLAN_PORT_DO_NOT_MODIFY_TAG_CMD_E

} PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT;

/* enumeration that hold the types of VFD modes */
typedef enum
{
    TGF_VFD_MODE_OFF_E,
    TGF_VFD_MODE_STATIC_E,
    TGF_VFD_MODE_INCREMENT_E,
    TGF_VFD_MODE_DECREMENT_E,
    TGF_VFD_MODE_RANDOM_E,

    TGF_VFD_MODE_ARRAY_PATTERN_E,  /* mode to transmit array of patterns */

    TGF_VFD_MODE_INCREMENT_VALUE_E,/* increment by value --> see parameter incValue */
    TGF_VFD_MODE_DECREMENT_VALUE_E,/* decrement by value --> see parameter incValue */

    TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E,    /* TGF_VFD_MODE_STATIC_E BUT with meaning
                    that the VFD expected to not match , the packet !
                    this flag used by 'check triggers' -->
                    tgfTrafficGeneratorPortTxEthTriggerCountersGet
                    */

    TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E,    /* TGF_VFD_MODE_STATIC_E BUT with meaning
                    that the VFD that not match packet is ignored for error !
                    this flag used by 'check triggers' -->
                    tgfTrafficGeneratorPortTxEthTriggerCountersGet
                    */

    TGF_VFD_MODE___LAST___E
} TGF_VFD_MODE_ENT;

/**
* @enum TGF_DSA_TYPE_ENT
 *
 * @brief Enumeration of DSA tag types
*/
typedef enum{

    /** regular DSA tag - single word - 4 bytes */
    TGF_DSA_1_WORD_TYPE_E = 0,

    /** extended DSA tag- two words  - 8 bytes */
    TGF_DSA_2_WORD_TYPE_E,

    /** extended DSA tag- 3 words   - 12 bytes */
    TGF_DSA_3_WORD_TYPE_E,

    /** extended DSA tag- 4 words   - 16 bytes */
    TGF_DSA_4_WORD_TYPE_E,

    TGF_DSA_LAST_TYPE_E

} TGF_DSA_TYPE_ENT;

/* number of bytes that the DSA part hold */
#define TGF_DSA_NUM_BYTES_MAC(dsaTagPartPtr) (4 * (((TGF_PACKET_DSA_TAG_STC *)dsaTagPartPtr)->dsaType + 1))

/**
* @enum TGF_DSA_CMD_ENT
 *
 * @brief Enumeration of DSA tag commands
*/
typedef enum{

    /** DSA command is "To CPU" */
    TGF_DSA_CMD_TO_CPU_E = 0 ,

    /** DSA command is "FROM CPU" */
    TGF_DSA_CMD_FROM_CPU_E    ,

    /** DSA command is "TO Analyzer" */
    TGF_DSA_CMD_TO_ANALYZER_E ,

    /** DSA command is "FORWARD" */
    TGF_DSA_CMD_FORWARD_E

} TGF_DSA_CMD_ENT;

/* enumeration that holds the types of the packet's parts :

    TGF_PACKET_PART_DSA_TAG_E -  the DSA TAG sent as 'payload' that the
                                 device is not processing on the egress
                                 when sent from the CPU.
                                 only on the ingress of the 'loopback' port the
                                 device will recognize this DSA as 'DSA to parse'

                                 --> for types,commands see :
                                    TGF_DSA_CMD_ENT , TGF_DSA_TYPE_ENT
*/
typedef enum
{
    TGF_PACKET_PART_L2_E,
    TGF_PACKET_PART_VLAN_TAG_E,
    TGF_PACKET_PART_SGT_TAG_E,
    TGF_PACKET_PART_DSA_TAG_E,
    TGF_PACKET_PART_ETHERTYPE_E,
    TGF_PACKET_PART_MPLS_E,
    TGF_PACKET_PART_IPV4_E,
    TGF_PACKET_PART_IPV6_E,
    TGF_PACKET_PART_TCP_E,
    TGF_PACKET_PART_UDP_E,
    TGF_PACKET_PART_ARP_E,
    TGF_PACKET_PART_ICMP_E,
    TGF_PACKET_PART_WILDCARD_E,
    TGF_PACKET_PART_PAYLOAD_E,
    TGF_PACKET_PART_TRILL_E,
    TGF_PACKET_PART_TRILL_FIRST_OPTION_E,
    TGF_PACKET_PART_TRILL_GENERAL_OPTION_E,
    TGF_PACKET_PART_GRE_E,
    TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,
    TGF_PACKET_PART_CRC_E,
    TGF_PACKET_PART_PTP_V2_E,
    TGF_PACKET_PART_TEMPLATE_E,
    TGF_PACKET_PART_VXLAN_GPE_E,
    TGF_PACKET_PART_NSH_E,
    TGF_PACKET_PART_SRH_E,
    /*********** special 'parts' ************/
    TGF_PACKET_PART_SKIP_E, /* only place holder - used to 'skip' part of packet
                               that currently not needed */

    TGF_PACKET_PART_MAX_E
} TGF_PACKET_PART_ENT;


/*
 * Protocol Numbers
 *
 * Note: In the Internet Protocol version 4 (IPv4) [RFC791] there is a field
 * called "Protocol" to identify the next level protocol.  This is an 8
 * bit field.  In Internet Protocol version 6 (IPv6) [RFC1883], this field
 * is called the "Next Header" field.
 *
 * Decimal  Keyword          Protocol                                 References
 * -------  ---------------  ---------------------------------------  ----------
 * 0        HOPOPT           IPv6 Hop-by-Hop Option                   [RFC1883]
 * 1        ICMP             Internet Control Message                 [RFC792]
 * 2        IGMP             Internet Group Management                [RFC1112]
 * 4        IP               IP in IP (encapsulation)                 [RFC2003]
 * 6        TCP              Transmission Control                     [RFC793]
 * 17       UDP              User Datagram                            [RFC768][JBP]
 * 41       IPv6             Ipv6                                     [Deering]
 * 43       IPv6-Route       Routing Header for IPv6                  [Deering]
 * 44       IPv6-Frag        Fragment Header for IPv6                 [Deering]
 * 58       IPv6-ICMP        ICMP for IPv6                            [RFC1883]
 * 59       IPv6-NoNxt       No Next Header for IPv6                  [RFC1883]
 * 60       IPv6-Opts        Destination Options for IPv6             [RFC1883]
 * 140-252  Unassigned                                                [IANA]
 * 253      Use for experimentation and testing                       [RFC3692]
 * 254      Use for experimentation and testing                       [RFC3692]
 * 255      Reserved                                                  [IANA]
 */
typedef enum
{
    TGF_PROTOCOL_HOPOPT_E =  0,
    TGF_PROTOCOL_ICMP_E   =  1,
    TGF_PROTOCOL_IGMP_E   =  2,
    TGF_PROTOCOL_IP_E     =  4,
    TGF_PROTOCOL_TCP_E    =  6,
    TGF_PROTOCOL_UDP_E    = 17, /* 0x11 */
    TGF_PROTOCOL_ICMPV6_E = 58, /* 0x3A */

    TGF_PROTOCOL_MAX_E = 255
} TGF_PROTOCOL_ENT;

/*
 * ICMP Message Types
 *
 *   0  Echo Reply Message
 *   3  Destination Unreachable Message
 *   4  Source Quench Message
 *   5  Redirect Message
 *   8  Echo Message
 *   11  Time Exceeded Message
 *   12  Parameter Problem Message
 *   13  Timestamp Message
 *   14  Timestamp Reply Message
 *   15  Information Request Message
 *   16  Information Reply Message
 */
typedef enum
{
    TGF_ICMP_TYPE_ECHO_REPLY_E              =  0,
    TGF_ICMP_TYPE_DESTINATION_UNREACHABLE_E =  3,
    TGF_ICMP_TYPE_SOURCE_QUENCH_E           =  4,
    TGF_ICMP_TYPE_REDIRECT_E                =  5,
    TGF_ICMP_TYPE_ECHO_E                    =  8,
    TGF_ICMP_TYPE_TIME_EXCEEDED_E           = 11,
    TGF_ICMP_TYPE_PARAMETER_PROBLEM_E       = 12,
    TGF_ICMP_TYPE_TIMESTAMP_E               = 13,
    TGF_ICMP_TYPE_TIMESTAMP_REPLY_E         = 14,
    TGF_ICMP_TYPE_INFORMATION_E             = 15,
    TGF_ICMP_TYPE_INFORMATION_REPLY_E       = 16,

    TGF_ICMP_TYPE_MAX_E = 255
} TGF_ICMP_TYPE_ENT;

/******************************************************************************\
 *                               Struct definitions                           *
\******************************************************************************/

/* struct for L2 part (mac addresses) */
typedef struct
{
    TGF_MAC_ADDR        daMac;
    TGF_MAC_ADDR        saMac;
} TGF_PACKET_L2_STC;

/* struct for vlan tag part  */
typedef struct
{
    TGF_ETHER_TYPE      etherType;
    TGF_PRI             pri;
    TGF_CFI             cfi;
    TGF_VLAN_ID         vid;
} TGF_PACKET_VLAN_TAG_STC;

/*
 * typedef: structure TGF_DSA_DSA_COMMON_STC
 *
 * Description: structure of common parameters in all DSA tag formats
 *
 *  dsaTagType - DSA type regular/extended
 *
 *  vpt - the value vlan priority tag (0..7)
 *  cfiBit - CFI bit of the vlan tag  (0..1)
 *  vid - the value of vlan id (0..4095)
 *  dropOnSource - Drop packet on its source device indicator,
 *                 for Fast Stack Fail over Recovery Support.
 *                 GT_TRUE: If packet is looped (packetIsLooped = 1)and
 *                          SrcDev = OwnDev then packet is dropped
 *                 GT_FALSE: If packet is looped and SrcDev = OwnDev, the
 *                           packet is not dropped.
 *                 (relevant only for CH3 devices)
 *  packetIsLooped - Packet is looped indicator, for Fast Stack Fail
 *                   over Recovery Support.
 *                  GT_FALSE: Packet is not looped and is being routed via its
 *                            designated route in the Stack.
 *                  GT_TRUE: Packet is looped and its being routed on an
 *                           alternative route in the Stack.
 *                 (relevant only for CH3 devices).
 *
 */
typedef struct{
/*    TGF_DSA_TYPE_ENT                dsaTagType;*/

    GT_U8                           vpt;
    GT_U8                           cfiBit;
    GT_U16                          vid;
    GT_BOOL                         dropOnSource;
    GT_BOOL                         packetIsLooped;
}TGF_DSA_DSA_COMMON_STC;

/* structure for FROM_CPU extra dest multi interface info */
typedef struct{
    GT_BOOL                         excludeInterface;
    CPSS_INTERFACE_INFO_STC         excludedInterface; /* port/trunk */
    GT_BOOL                         mirrorToAllCPUs;

    /* supported in extended, 4 words,  DSA tag */
    GT_BOOL                         excludedIsPhyPort;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT   srcIsTagged;

}TGF_DSA_FROM_CPU_EXT_MULTI_DEST_STC;

/* structure for FROM_CPU extra dest single interface info */
typedef struct{
    GT_BOOL                     dstIsTagged;
    GT_BOOL                     mailBoxToNeighborCPU;
}TGF_DSA_FROM_CPU_EXT_SINGLE_DEST_STC;

/* union for FROM_CPU extra dest multi/single interface info */
typedef union{
    TGF_DSA_FROM_CPU_EXT_MULTI_DEST_STC     multiDest;
    TGF_DSA_FROM_CPU_EXT_SINGLE_DEST_STC    devPort;
}TGF_DSA_FROM_CPU_EXT_DEST_UNT;

/*
 * typedef: structure TGF_DSA_DSA_FROM_CPU_STC
 *
 * Description: structure of "from CPU" DSA tag parameters
 *
 * fields:
 *      dstInterface  - Packet's destination interface:
 *         type == CPSS_INTERFACE_TRUNK_E --> not supported !
 *         when type == CPSS_INTERFACE_PORT_E
 *              devPort.tgtDev - the target device that packet will be send to.
 *                               NOTE : HW device number !!
 *              devPort.tgtPort- the target port on device that packet will be
 *                             send to.
 *         when type == CPSS_INTERFACE_VIDX_E
 *              vidx     - the vidx group that packet will be sent to.
 *         when type == CPSS_INTERFACE_VID_E
 *              vlanId   - must be equal to commonParams.vid !!!
 *
 *      tc - the traffic class for the transmitted packet
 *
 *      dp  - the drop precedence for the transmitted packet.
 *            dp is globally set using the cpssDxChNetIfFromCpuDpSet API.
 *
 *      egrFilterEn - Egress filtering enable.
 *                  GT_FALSE - Packets from the CPU are not egress filtered.
 *                             Unicast packets are forwarded regardless of the
 *                             Egress port Span State or VLAN Membership.
 *                             Multi-destination packets are forwarded to the
 *                             Multicast group members specified in this tag,
 *                             regardless of the target port's Span state.
 *                  GT_TRUE -  Packets from CPU are Egress filtered.
 *
 *      cascadeControl - This field indicates which TC is assigned to the packet
 *                       when it is forwarded over a cascading/stacking port.
 *                  GT_FALSE - If the packet is queued on a port that is enabled
 *                             for Data QoS mapping (typically a cascade port),
 *                             the packet is queued according to the data
 *                             traffic {TC, DP} mapping table, which maps the
 *                             DSA tag TC and DP to a cascade port TC and DP.
 *                             On a port that is disabled for Data QoS mapping (
 *                             a non-cascade ports), the packet is queued
 *                             according to the DSA tag TC and DP.
 *                   GT_TRUE - If the packet is queued on a port that is enabled
 *                             for Control QoS mapping (typically a cascade
 *                             port), the packet is queued according to the
 *                             configured Control TC and DP. On a port that is
 *                             enabled for Control QoS mapping (non-cascade
 *                             ports), the packet is queued according to the DSA
 *                             tag TC and DP.
 *
 *      egrFilterRegistered -  Indicates that the packet is Egress filtered as a
 *                             registered packet.
 *                  GT_FALSE - Packet is egress filtered as an Unregistered
 *                             packet and is forwarded to an Egress port
 *                             according to its type (Unicast or Multicast) and
 *                             the configuration of the Egress port Port<n>
 *                             UnkFilterEn if packet is Unicast, and according
 *                             to the configuration of Port<n> UnregFilterEn if
 *                             the packet is Multicast.
 *                  GT_TRUE  - Packet is egress filtered as a registered packet
 *                             according to the members of the Multicast group.
 *                             NOTE:
 *                             When this field is 0, the type of the packet
 *                             Multicast or Unicast is set according to the
 *                             packet's MAC DA[40].
 *
 *       srcId              - Packet's Source ID
 *
 *       srcHwDev           - Packet's Source HW Device Number.
 *                            NOTE : HW device number !!
 *
 *       extDestInfo - extra destination information:
 *          multiDest - info about multi destination destination , used when:
 *                      type == CPSS_INTERFACE_VIDX_E or
 *                      type == CPSS_INTERFACE_VID_E
 *             excludeInterface - to exclude a "source" interface from the
 *                      destination flooding.
 *                      GT_FALSE - no "source" interface to exclude
 *                      GT_TRUE  - use "source" interface to exclude , see
 *                                 parameter excludedInterface.
 *             excludedInterface - the "source" interface to exclude from the
 *                          Multicast group.
 *                          NOTE: relevant only
 *                                when excludeInterface == GT_TRUE
 *                          when type == CPSS_INTERFACE_PORT_E
 *                              the {devPort.deviceNum,devPort.portNum} will be
 *                              excluded from the vid/vidx
 *                          when type == CPSS_INTERFACE_TRUNK_E
 *                              the trunkId will be excluded from the vid/vidx
 *                          type == CPSS_INTERFACE_VIDX_E not supported
 *                          type == CPSS_INTERFACE_VID_E  not supported
 *
 *             excludedIsPhyPort - supported in extended, 4 words,  DSA tag
 *                          This field determines whether the value in
 *                          <excludedInterface> contains a 20b
 *                           ePort number to exclude from an L2MLL multicast group,
 *                          or a physical 8b port number to exclude from an L2 MC group
 *                          GT_FALSE = The <excludedInterface> field
 *                                    contains a 20b ePort number to exclude
 *                          GT_TRUE =  The <excludedInterface> field
 *                                    contains an 8b Physical port number to exclude
 *
 *             srcIsTagged - source packet received tagged.
 *                valid options:
 *                  PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E
 *
 *             mirrorToAllCPUs - Mirror the packet to all CPUs
 *                          Enable a CPU to send a Multicast/Broadcast packet to
 *                          all CPUs in the system.
 *                          NOTE: relevant only :
 *                               when (excludeInterface == GT_FALSE) or
 *                               when ((excludeInterface == GT_TRUE) and
 *                                (excludedInterface.type == CPSS_INTERFACE_TRUNK_E))
 *                          GT_FALSE - The Multicast packet sent by the CPU is
 *                                     not mirrored to the CPU attached to the
 *                                     receiving device.
 *                          GT_TRUE - The Multicast packet sent by the CPU is
 *                                    mirrored to the CPU attached to the
 *                                    receiving device with a TO_CPU DSA Tag and
 *                                    MIRROR_TO_ALL_CPUS CPU Code.
 *       devPort - {device,port} info (single destination), used when:
 *                          type == CPSS_INTERFACE_PORT_E
 *             dstIsTagged - GT_FALSE - Packet is sent via network port untagged
 *                           GT_TRUE  - Packet is sent via network port tagged
 *             mailBoxToNeighborCPU - Mail box to Neighbor CPU, for CPU to CPU
 *                              mail box communication.
 *                           NOTE: As a Mail message is sent to a CPU with
 *                              unknown Device Number, the TrgDev
 *                              (dstInterface.devPort.tgtDev) must be set to
 *                              the local device number and TrgPort
 *                              (dstInterface.devPort.tgtPort) must be set to
 *                              the Cascading port number in the local device
 *                              through which this packet is to be transmitted.
 *
 *      isTrgPhyPortValid - Indicates whether the target physical port field
 *                      <TrgPort> in this DSA tag is valid.
 *                      GT_FALSE:The value of <TrgPort> is not valid
 *                      GT_TRUE: The value of <TrgPort> is valid
 *
 *      dstEport     - The assigned destination ePort (dstInterface.type == CPSS_INTERFACE_PORT_E  )
 *
 *      tag0TpidIndex - The TPID index of Tag0.
 *
 */
typedef struct{
    CPSS_INTERFACE_INFO_STC         dstInterface; /* vid/vidx/port */
    GT_U8                           tc;
    CPSS_DP_LEVEL_ENT               dp;
    GT_BOOL                         egrFilterEn;
    GT_BOOL                         cascadeControl;
    GT_BOOL                         egrFilterRegistered;

    GT_U32                          srcId;
    GT_HW_DEV_NUM                   srcHwDev;

    TGF_DSA_FROM_CPU_EXT_DEST_UNT   extDestInfo;

   /* supported in extended, 4 words,  DSA tag */
    GT_BOOL                         isTrgPhyPortValid;
    GT_PORT_NUM                     dstEport; /* for port but not for vid/vidx */
    GT_U32                          tag0TpidIndex;

}TGF_DSA_DSA_FROM_CPU_STC;

/* union for FORWARD src interface info */
typedef union
{
    GT_TRUNK_ID                 trunkId;
    GT_PORT_NUM                 portNum;
}TGF_DSA_FORWARD_SRC_INTERFACE_UNT;

/*
 * typedef: structure TGF_DSA_DSA_FORWARD_STC
 *
 * Description: structure of "forward" DSA tag parameters
 *
 * fields:
 *
 *  srcIsTagged - source packet received tagged.
 *                valid options:
 *                  PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E
 *                  PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E
 *
 *  srcHwDev - Packet's Source HW Device Number.
 *           NOTE : HW device number !!
 *
 *  srcIsTrunk - the packet was received from trunk
 *     source.trunkId - - Packet's Source trunk id
 *                  NOTE : relevant when srcIsTrunk = GT_TRUE
 *     source.portNum - - Packet's Source port num
 *                  NOTE : relevant when srcIsTrunk = GT_FALSE
 *
 *  srcId              - Packet's Source ID
 *
 *  egrFilterRegistered -  Indicates that the packet is Egress filtered as a
 *                         registered packet.
 *              GT_FALSE - Packet is egress filtered as an Unregistered
 *                         packet and is forwarded to an Egress port
 *                         according to its type (Unicast or Multicast) and
 *                         the configuration of the Egress port Port<n>
 *                         UnkFilterEn if packet is Unicast, and according
 *                         to the configuration of Port<n> UnregFilterEn if
 *                         the packet is Multicast.
 *              GT_TRUE  - Packet is egress filtered as a registered packet
 *                         according to the members of the Multicast group.
 *                         NOTE:
 *                         When this field is 0, the type of the packet
 *                         Multicast or Unicast is set according to the
 *                         packet's MAC DA[40].
 *
 *  wasRouted - Indicates whether the packet is routed.
 *              GT_FALSE - Packet has not be Layer 3 routed.
 *              GT_TRUE  - Packet has been Layer 3 routed.
 *
 * qosProfileIndex - Packet's QoS Profile.
 *
 * dstInterface  - Packet's destination interface:
 *         type == CPSS_INTERFACE_TRUNK_E --> not supported !
 *         when type == CPSS_INTERFACE_PORT_E
 *              devPort.tgtDev - the target device that packet will be send to.
 *                               NOTE : HW device number !!
 *              devPort.tgtPort- the target port on device that packet will be
 *                             send to.
 *         when type == CPSS_INTERFACE_VIDX_E
 *              vidx     - the vidx group that packet will be sent to.
 *         when type == CPSS_INTERFACE_VID_E
 *              vlanId   - must be equal to commonParams.vid !!!
 *
 *  isTrgPhyPortValid - Indicates whether the target physical port field
 *                      <TrgPort> in this DSA tag is valid.
 *                      GT_FALSE: The value of <TrgPort> is not valid
 *                      GT_TRUE:  The value of <TrgPort> is valid
 *
 *  dstEport - The target ePort assigned to the packet by the ingress device
 *             (dstInterface.type == CPSS_INTERFACE_PORT_E  )
 *
 *  tag0TpidIndex - The TPID index of Tag0.
 *
 *  origSrcPhyIsTrunk - Indicates whether the packet was received in the ingress
 *                      device on a physical port that is a trunk member
 *                      GT_FALSE: Packet arrived on physical port that is
 *                                configured as non-trunk member
 *                      GT_TRUE:  Packet arrived on physical port that is
 *                                configured as a trunk member
 *
 *     origSrcPhy.trunkId - - Packet's Source physical ingress trunk id
 *                  NOTE : relevant when origSrcPhyIsTrunk = GT_TRUE
 *     origSrcPhy.portNum - - Packet's Source physical ingress port num
 *                  NOTE : relevant when origSrcPhyIsTrunk = GT_FALSE
 *
 *  phySrcMcFilterEnable - Indicates to the remote device whether to pereform VIDX MC
 *                         source filtering based on <origSrcPhy.portNum/origSrcPhy.trunkId>
 *                         GT_FALSE: Do not perform VIDX physical source filtering
 *                         GT_TRUE:  Perform VIDX physical source filtering
 * skipFdbSaLookup       - Skip automatic Source MAC Address learning by receiving device
 *                         GT_FALSE: Do learning, GT_TRUE: Skip learning.
 *                         (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
 *
 */
typedef struct{
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT srcIsTagged;
    GT_HW_DEV_NUM                   srcHwDev;
    GT_BOOL                         srcIsTrunk;
    TGF_DSA_FORWARD_SRC_INTERFACE_UNT   source;

    GT_U32                          srcId;

    GT_BOOL                         egrFilterRegistered;
    GT_BOOL                         wasRouted;
    GT_U32                          qosProfileIndex;

    CPSS_INTERFACE_INFO_STC         dstInterface; /* vid/vidx/port */

   /* supported in extended, 4 words,  DSA tag */
    GT_BOOL                         isTrgPhyPortValid;
    GT_PORT_NUM                     dstEport; /* for port but not for vid/vidx */
    GT_U32                          tag0TpidIndex;
    GT_BOOL                         origSrcPhyIsTrunk;
    TGF_DSA_FORWARD_SRC_INTERFACE_UNT  origSrcPhy;
    GT_BOOL                         phySrcMcFilterEnable;
    GT_U32                          hash;
    GT_BOOL                         skipFdbSaLookup;
}TGF_DSA_DSA_FORWARD_STC;

/* structure for TO_ANALYZER extra mutli-dest info */
typedef struct{
    GT_U16                          analyzerEvidx;
}TGF_DSA_TO_ANALYZER_EXT_MULTI_DEST_STC;

/* structure for TO_ANALYZER extra single-dest info */
typedef struct{
    GT_BOOL                         analyzerIsTrgPortValid;
    GT_HW_DEV_NUM                   analyzerHwTrgDev;
    GT_PHYSICAL_PORT_NUM            analyzerTrgPort;
    GT_PORT_NUM                     analyzerTrgEport;
}TGF_DSA_TO_ANALYZER_EXT_SINGLE_DEST_STC;

/* union for TO_ANALYZER extra multi/single-dest info */
typedef union{
    TGF_DSA_TO_ANALYZER_EXT_MULTI_DEST_STC  multiDest;
    TGF_DSA_TO_ANALYZER_EXT_SINGLE_DEST_STC devPort;
}TGF_DSA_TO_ANALYZER_EXT_DEST_UNT;

/* structure for TO_ANALYZER trg interface */
typedef struct
{
    GT_HW_DEV_NUM               hwDevNum;
    GT_PHYSICAL_PORT_NUM        portNum;

    /* supported in extended, 4 words,  DSA tag */
    GT_PORT_NUM                 ePort;
}TGF_DSA_TO_ANALYZER_TRG_INTERFACE_STC;

/*
 * typedef: structure TGF_DSA_DSA_TO_ANALYZER_STC
 *
 * Description: structure of "to analyzer" DSA tag parameters
 *
 * fields:
 *
 *      rxSniffer - is this packet destined to the Rx or to the Tx Analyzer port
 *          GT_FALSE - Packet was Tx sniffed and is forwarded to Target Tx
 *                     sniffer (Analyzer).
 *          GT_TRUE  - Packet was Rx sniffed and is forwarded to Target Rx
 *                     sniffer (Analyzer).
 *
 *      isTagged -  When rxSniffer == GT_TRUE:
 *                      This field is srcTagged.
 *                      srcTagged -
 *                          GT_FALSE - Packet was received from a regular
 *                                     network port untagged and is forwarded
 *                                     to the Ingress analyzer untagged.
 *                          GT_TRUE - Packet was received from a regular network
 *                                    port tagged and is forwarded to the Ingress
 *                                    analyzer tagged, with the same VID and UP
 *                                     with which it was received.
 *                      When rxSniffer == GT_FALSE:
 *                      This field is trgTagged.
 *                      trgTagged -
 *                          GT_FALSE - Packet was sent via a regular network
 *                                     port untagged and is forwarded to the Egress
 *                                     analyzer untagged.
 *                          GT_TRUE - Packet was sent via a regular network port
 *                                    tagged and is forwarded to the Egress
 *                                    analyzer tagged.
 *
 *      devPort - src/dst {dev,port} info :
 *              devPort.hwDevNum -
 *                      SrcDev:
 *                      When rx_sniff = GT_TRUE or when tag is not extended:
 *                      SrcDev indicates the packet's original Ingress port.
 *                      When rx_sniff = GT_FALSE and the tag is extended:
 *                      TrgDev:
 *                      TrgDev indicates the packet's Egress port
 *                      NOTE: When the tag is not extended, this field always
 *                      indicates SrcDev.
 *              devPort.portNum -
 *                      When rx_sniff = GT_TRUE or when tag is not extended:
 *                      SrcPort:
 *                      indicates the packet's original Ingress port.
 *                      When rx_sniff = GT_FALSE and the tag is extended:
 *                      TrgPort:
 *                      indicates the packet's Egress port.
 *                      NOTE: When this tag is not extended, port is a 5-bit
 *                      field.
 *                      When this tag is not extended, this field always
 *                      indicates SrcPort.
 *              devPort.ePort-
 *                      When in source based forwarding mode: TBD.
 *                      When in hop-by-hop mode:
 *                      When <rxSniffer>= GT_TRUE: SRCePort:
 *                      Together with SrcDev, SRCePort indicates the packet's
 *                      original Ingress ePort assignment.
 *                      When <rxSniffer> = GT_FALSE: TRGePort:
 *                      Together with TrgDev, TRGePort indicates the packet's
 *                      assigned egress ePort.
 *
 *
 *      analyzerTrgType - The target Analyzer type :
 *                          CPSS_INTERFACE_PORT_E - see extDestInfo.devPort
 *                          CPSS_INTERFACE_VIDX_E - see extDestInfo.multiDest
 *                          CPSS_INTERFACE_VID_E  - ignore extDestInfo and implicit use vidx=0xFFF
 *      extDestInfo - extra destination (target) information:
 *          multiDest - info about multi destination , used when: analyzerTrgType = CPSS_INTERFACE_VIDX_E
 *              multiDest.analyzerEvidx       - The target analyzer interface eVIDX
 *
 *          devPort - {device,port} info (single destination), used when: analyzerTrgType = CPSS_INTERFACE_PORT_E
 *              devPort.analyzerIsTrgPortValid      - The target Analyzer interface indication that the
 *                      target analyzer port field devPort.analyzerTrgPort has a valid value
 *              devPort.analyzerHwTrgDev    - The target Analyzer interface HW device number
 *              devPort.analyzerTrgPort     - The target Analyzer interface physical port number
 *              devPort.analyzerTrgEport    - The target analyzer interface ePort
 *
 *      tag0TpidIndex - The TPID index of Tag0.
 *                      (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X)(APPLICABLE RANGES: 0..7)
 */
typedef struct{
    GT_BOOL                         rxSniffer;

    GT_BOOL                         isTagged;
    TGF_DSA_TO_ANALYZER_TRG_INTERFACE_STC  devPort;

    /* supported in extended, 4 words,  DSA tag */
    CPSS_INTERFACE_TYPE_ENT         analyzerTrgType;

    TGF_DSA_TO_ANALYZER_EXT_DEST_UNT    extDestInfo;

    GT_U32                          tag0TpidIndex;

}TGF_DSA_DSA_TO_ANALYZER_STC;

/* union for ttOffset/flowId */
typedef union
{
   GT_U32                           flowId;
   GT_U32                           ttOffset;
}TGF_DSA_FLOW_ID_OR_TT_OFFSET_UNT;

/* structure for TO_CPU trg interface */
typedef struct
{
   GT_TRUNK_ID                  srcTrunkId;
   GT_PHYSICAL_PORT_NUM         portNum;
  /* supported in extended, 4 words,  DSA tag */
   GT_PORT_NUM                  ePort;
}TGF_DSA_TO_CPU_TRG_INTERFACE_STC;

/*
 * typedef: structure TGF_DSA_DSA_TO_CPU_STC
 *
 * Description: structure of "To CPU" DSA tag parameters
 *
 * fields:
 *
 *      isEgressPipe - indicates the type of data forwarded to the CPU.
 *                     the data came from egress/ingress pipe (in PP).
 *          GT_FALSE - The packet was forwarded to the CPU by the Ingress pipe
 *                     and this tag contains the packet's source information.
 *          GT_TRUE  - The packet was forwarded to the CPU by the Egress pipe
 *                     and this tag contains the packet's destination
 *                     information.
 *          NOTE: When this tag is not extended, <isEgressPipe> should be
 *                     considered like GT_FALSE.
 *
 *      isTagged -  When isEgressPipe == GT_FALSE:
 *                      This field is srcTagged.
 *                      srcTagged - This tag contains Source Port information
 *                                  and this bit indicates the VLAN Tag format,
 *                                  in which the packet was received on the
 *                                  network port:
 *                          GT_FALSE - Packet was received from a network port
 *                                     untagged.
 *                          GT_TRUE - Packet was received from a network port
 *                                     tagged.
 *                      When isEgressPipe == GT_TRUE:
 *                      This field is trgTagged.
 *                      trgTagged - This tag contains Target Port information
 *                                  and this bit indicates the VLAN Tag format,
 *                                  in which the packet was transmitted via the
 *                                  network port:
 *                          GT_FALSE - Packet was transmitted to a regular
 *                                     network port untagged.
 *                          GT_TRUE - Packet was transmitted to a regular
 *                                     network port tagged.
 *
 *               NOTE: When isEgressPipe == GT_FALSE and the packet
 *                     forwarded to the CPU is received on a customer port on
 *                     which Nested VLAN is implemented, srcTagged is set to
 *                     GT_FALSE, regardless of the packet's VLAN tag format.
 *                     As the packet is considered untagged, when the packet is
 *                     forwarded to the CPU, the customer's VLAN tag (if any)
 *                     resides after the DSA tag.
 *
 *      hwDevNum - When isEgressPipe == GT_FALSE:
 *                  NOTE : HW device number !!
 *                  This field is srcDev.
 *                      This tag contains Source Port information and this field
 *                      indicates the number of the Source Device on which the
 *                      packet was received.
 *                  When isEgressPipe == GT_TRUE:
 *                      This field is trgDev.
 *                      This tag contains Target Port information and this field
 *                      indicates the number of the Destination Device through
 *                      which the packet was transmitted.
 *
 *      srcIsTrunk -    when dsaTagType == CPSS_DXCH_NET_DSA_TYPE_REGULAR_E or
 *                           dsaTagType == CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E :
 *                          indication if interface.srcTrunkId is valid (srcIsTrunk = GT_TRUE)
 *                          or interface.portNum is valid (srcIsTrunk = GT_FALSE)
 *                          NOTE: interface.ePort is never valid !
 *
 *                          When a packet's DSA Tag is replaced from FORWARD to
 *                                TO_CPU and FORWARD_DSA<SrcIsTrunk> = GT_TRUE and
 *                                isEgressPipe = GT_FALSE, this field is set to
 *                                GT_TRUE and This Tag hold interface.trunkId which is the
 *                                Trunk number parsed from the FORWARD DSA Tag
 *                                When isEgressPipe == GT_FALSE:
 *                                  GT_FALSE - the field of <interface.portNum> indicates the source
 *                                           port number
 *                                  GT_TRUE - the field of <interface.srcTrunkId> indicates the source
 *                                           trunk number
 *                                When isEgressPipe == GT_TRUE:
 *                                  this field should be GT_FALSE.
 *                                  GT_FALSE - the field of <interface.portNum> indicates the
 *                                           destination Port through which the packet was
 *                                           transmitted.
 *                                  GT_TRUE  - not supported !
 *
 *                      when dsaTagType == CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT :
 *                          indication if <interface.srcTrunkId> is valid (srcIsTrunk = GT_TRUE)
 *                          or <interface.ePort> is valid (srcIsTrunk = GT_FALSE)
 *                          NOTE: interface.portNum is always valid !
 *
 *                                When isEgressPipe == GT_FALSE:
 *                                  GT_FALSE - <interface.ePort> indicates the source eport number. or
 *                                      (assigned by the ingress device)
 *                                  GT_TRUE - <interface.srcTrunkId> indicates the source trunk number.
 *                                      (assigned by the ingress device)
 *                                When isEgressPipe == GT_TRUE:
 *                                  GT_FALSE - <interface.ePort> indicates the target eport number.
 *                                      (assigned by the egress device)
 *                                  GT_TRUE  - not supported !
 *
 *      interface - interface info
 *             srcTrunkId  - see comments about srcIsTrunk field
 *             portNum     - see comments about srcIsTrunk field
 *             ePort       - see comments about srcIsTrunk field
 *
 *
 *      cpuCode - CPU codes that indicate the reason for sending a packet to
 *                the CPU.
 *                Note : this is unified CPU code format (and not HW format))
 *
 *      wasTruncated - Was packet sent to CPU truncated?
 *                GT_FALSE - packet was not truncated.
 *                GT_TRUE - Indicates that only the first 128 bytes of the
 *                          packet are sent to the CPU. The packet's original
 *                          byte count is in <originByteCount> field.
 *
 *      originByteCount - The packet's original byte count.
 *
 *      timestamp       - IPFIX timestamping. Relevant only for xCat and
 *                        above devices, and used only in case that packet was
 *                        mirrored to the CPU due to IPFIX sampling.
 *      packetIsTT - An indication to the CPU that the trapped/mirrored packet
 *                   was tunnel-terminated by the device processing pipes.
 *                   GT_FALSE: The packet was not tunnel-terminated by the device
 *                             processing engines
 *                   GT_TRUE:  The packet was tunnel-terminated by the device
 *                             processing engines
 *
 *      flowIdTtOffset.flowId - The flow-ID assigned to the packet
 *      flowIdTtOffset.ttOffset - the passenger packet offset in the tunnel
 *
 *      tag0TpidIndex - The TPID index of Tag0.
 */
typedef struct{
   GT_BOOL                          isEgressPipe;

   GT_BOOL                          isTagged;
   GT_HW_DEV_NUM                    hwDevNum;
   GT_BOOL                          srcIsTrunk;

   TGF_DSA_TO_CPU_TRG_INTERFACE_STC interface;

   CPSS_NET_RX_CPU_CODE_ENT         cpuCode;
   GT_BOOL                          wasTruncated;
   GT_U32                           originByteCount;
   GT_U32                           timestamp;

   /* supported in extended, 4 words,  DSA tag */
   GT_BOOL                          packetIsTT;

   TGF_DSA_FLOW_ID_OR_TT_OFFSET_UNT flowIdTtOffset;

   GT_U32                           tag0TpidIndex;

}TGF_DSA_DSA_TO_CPU_STC;

/* union of DSA tag types */
typedef union{
    TGF_DSA_DSA_TO_CPU_STC         toCpu;
    TGF_DSA_DSA_FROM_CPU_STC       fromCpu;
    TGF_DSA_DSA_TO_ANALYZER_STC    toAnalyzer;
    TGF_DSA_DSA_FORWARD_STC        forward;
}TGF_PACKET_DSA_TAG_UNT;

/* struct for vlan tag part  */
typedef struct
{
    TGF_DSA_CMD_ENT                 dsaCommand;
    TGF_DSA_TYPE_ENT                dsaType;

    TGF_DSA_DSA_COMMON_STC          commonParams;

    TGF_PACKET_DSA_TAG_UNT          dsaInfo;

}TGF_PACKET_DSA_TAG_STC;


/* struct for ethertype part  */
typedef struct
{
    TGF_ETHER_TYPE      etherType;
} TGF_PACKET_ETHERTYPE_STC;

/* MPLS part of single frame */
typedef struct
{
    TGF_MPLS_LBL        label;
    TGF_MPLS_EXP        exp;
    TGF_MPLS_STACK      stack;
    TGF_TTL             timeToLive;
} TGF_PACKET_MPLS_STC;

/* TRILL first option part of single frame */
typedef struct
{
    GT_BOOL             chbh; /* bit 31 (MSbit in network order) */
    GT_BOOL             clte; /* bit 30 */
    GT_U32              info; /* bit 0..29 */
} TGF_PACKET_TRILL_FIRST_OPTION_STC;

/* TRILL general options part of single frame (can replace the TRILL first option 'TGF_PACKET_TRILL_FIRST_OPTION_STC') */
typedef struct
{
    GT_U32              info;
} TGF_PACKET_TRILL_GEN_OPTION_STC;

/* auto calc checksum fields */
#define TGF_PACKET_AUTO_CALC_CHECKSUM_CNS       0xEEEE
/* auto calc length field */
#define TGF_PACKET_AUTO_CALC_LENGTH_CNS         0xFFFF

/* in ipv4 header - build typeOfService field according to dscp */
#define TGF_PACKET_IPV4_DSCP_TO_TYPE_OF_SERVICE_MAC(dscp)   \
    (TGF_TYPE_OF_SERVICE)(((dscp) & 0x3f) << 2)

/* in ipv6 header - build trafficClass field according to dscp */
#define TGF_PACKET_IPV6_DSCP_TO_TRAFFIC_CLASS_MAC(dscp)   \
    (TGF_TRAFFIC_CLASS)(((dscp) & 0x3f) << 2)

/* IPv4 part of single frame */
typedef struct
{
    TGF_PROT_VER        version;
    GT_U8               headerLen;
    TGF_TYPE_OF_SERVICE typeOfService;
    GT_U16              totalLen;
    TGF_IPV4_ID_FIELD   id;
    TGF_FLAG            flags;
    GT_U16              offset;
    TGF_TTL             timeToLive;
    TGF_PROT            protocol;
    TGF_HEADER_CRC      csum;     /* can use TGF_PACKET_AUTO_CALC_CHECKSUM_CNS for auto checksum */
    TGF_IPV4_ADDR       srcAddr;
    TGF_IPV4_ADDR       dstAddr;
} TGF_PACKET_IPV4_STC;

/* GRE part of single frame */
typedef struct
{
    GT_BOOL             checkSumPresent;
    GT_U16              reserved0;
    TGF_PROT_VER        version;
    GT_U16              protocol;
    GT_U16              checksum;
    GT_U16              reserved1;
} TGF_PACKET_GRE_STC;


/* VXLAN-GPE part of single frame */
typedef struct
{
    GT_U8     flags;
    GT_U16    reserved0;
    GT_U8     nextProtocol;
    GT_U32    vni;
    GT_U8     reserved1;
} TGF_PACKET_VXLAN_GPE_STC;

/* VXLAN-GBP part of single frame */
typedef struct
{
    GT_U16     flags;
    GT_U16     groupPolicyId;
    GT_U32     vni;
    GT_U8      reserved0;
} TGF_PACKET_VXLAN_GBP_STC;

/* NSH part of single frame */
typedef struct
{
    GT_U32    baseHdr;
    GT_U32    servicePathHdr;
    GT_U32    metaDataWord3Hdr;
    GT_U32    metaDataWord2Hdr;
    GT_U32    metaDataWord1Hdr;
    GT_U32    metaDataWord0Hdr;
} TGF_PACKET_NSH_STC;


/* IPv6 part of single frame */
typedef struct
{
    TGF_PROT_VER        version;
    TGF_TRAFFIC_CLASS   trafficClass;
    TGF_FLOW_LABEL      flowLabel;
    GT_U16              payloadLen;
    TGF_NEXT_HEADER     nextHeader; /* like IPv4 protocol */
    TGF_HOP_LIMIT       hopLimit;   /* like IPv4 timeToLive (TTL)*/
    TGF_IPV6_ADDR       srcAddr;
    TGF_IPV6_ADDR       dstAddr;
} TGF_PACKET_IPV6_STC;

/* TRILL part of single frame */
typedef struct
{
    TGF_PROT_VER        version;
    GT_BOOL             mBit;
    GT_U32              opLength;
    GT_U32              hopCount;
    GT_U16              eRbid;
    GT_U16              iRbid;
} TGF_PACKET_TRILL_STC;

/* TCP part of single frame */
typedef struct
{
    TGF_L4_PORT         srcPort;
    TGF_L4_PORT         dstPort;
    GT_U32              sequenceNum;
    GT_U32              acknowledgeNum;
    GT_U8               dataOffset;
    GT_U8               reserved;
    TGF_TCP_FLAGS       flags;
    GT_U16              windowSize;
    TGF_HEADER_CRC      csum;/* can use TGF_PACKET_AUTO_CALC_CHECKSUM_CNS for auto checksum */
    TGF_TCP_URGENT_PTR  urgentPtr;
} TGF_PACKET_TCP_STC;

/* UDP part of single frame */
typedef struct
{
    TGF_L4_PORT         srcPort;
    TGF_L4_PORT         dstPort;
    GT_U16              length;  /* can use TGF_PACKET_AUTO_CALC_LENGTH_CNS to build from ipv4/6 info */
    TGF_HEADER_CRC      csum;/* can use TGF_PACKET_AUTO_CALC_CHECKSUM_CNS for auto checksum */
} TGF_PACKET_UDP_STC;

/* ARP part of single frame */
typedef struct
{
    TGF_ARP_HW_TYPE     hwType;
    TGF_PROT_TYPE       protType;
    GT_U8               hwLen;
    GT_U8               protLen;
    GT_U16              opCode;
    TGF_MAC_ADDR        srcMac;
    TGF_IPV4_ADDR       srcIp;
    TGF_MAC_ADDR        dstMac;
    TGF_IPV4_ADDR       dstIp;
} TGF_PACKET_ARP_STC;

/* ARPv6 part of single frame */
typedef struct
{
    TGF_ARP_HW_TYPE     hwType;
    TGF_PROT_TYPE       protType;
    GT_U8               hwLen;
    GT_U8               protLen;
    GT_U16              opCode;
    TGF_MAC_ADDR        srcMac;
    TGF_IPV6_ADDR       srcIp;
    TGF_MAC_ADDR        dstMac;
    TGF_IPV6_ADDR       dstIp;
} TGF_PACKET_ARPV6_STC;

/* ICMP part of single frame */
typedef struct
{
    TGF_TYPE_OF_SERVICE typeOfService;
    GT_U8               code;
    TGF_HEADER_CRC      csum;
    TGF_ICMP_ID_FIELD   id;
    GT_U16              sequenceNum;
    GT_U32              getwayAddr;
    GT_U8               errorPointer;
} TGF_PACKET_ICMP_STC;

/* struct for any kind of wildcard/'Unknown format' */
typedef struct WILDCARD_STC
{
    GT_U32              numOfBytes;
    GT_U8              *bytesPtr;
} TGF_PACKET_WILDCARD_STC;

typedef struct WILDCARD_STC TGF_PACKET_TUNNEL_HEADER_FORCE_TILL_PASSENGER_STC;
typedef struct WILDCARD_STC TGF_PACKET_CRC_STC;

/* struct for payload */
typedef struct
{
    GT_U32              dataLength;
    GT_U8              *dataPtr;
} TGF_PACKET_PAYLOAD_STC;

/* struct for SGT Tag */
typedef struct
{
    TGF_ETHER_TYPE      etherType;      /* 16 Bits */
    GT_U8               version;        /* 8  Bits */
    GT_U16              length;         /* 12 Bits */
    GT_U16              optionType;     /* 12 Bits */
    GT_U16              sgtValue;       /* 16 Bits */
} TGF_PACKET_SGT_TAG_STC;


/* struct for tunnel start generic template */
typedef struct
{
    GT_U32              dataLength;
    GT_U8              *dataPtr;
} TGF_PACKET_TEMPLATE16_STC;

typedef struct
{
    GT_U32              dataLength;
    GT_U8              *dataPtr;
} TGF_PACKET_TEMPLATE8_STC;

/* struct for PTP_V2 header */
typedef struct
{
    GT_U32              messageType;               /*byte0 bits[3:0]*/
    GT_U32              transportSpecific;         /*byte0 bits[7:4]*/
    GT_U32              ptpVersion;                /*byte1 bits[3:0]*/
    GT_U32              reserved4;                 /*byte1 bits[7:4]*/
    GT_U32              messageLength;             /*bytes 3:2 - network order*/
    GT_U32              domainNumber;              /*byte4*/
    GT_U32              reserved8;                 /*byte5*/
    GT_U32              flagField;                 /*bytes 7:6*/
    GT_U32              correctionField[2];        /*bytes 15:8  - network order*/
    GT_U32              reserved32;                /*bytes 19:16 - network order*/
    GT_U8               sourcePortIdentify[10];    /*bytes 29:20 - network order */
    GT_U32              sequenceId;                /*bytes 31:30 - network order */
    GT_U32              controlField;              /*byte 32*/
    GT_U32              logMessageInterval;        /*byte 33*/

} TGF_PACKET_PTP_V2_STC;

/* struct for SRH */
typedef struct
{
    GT_U8               nextHeader;      /* 8  Bits */
    GT_U8               headerLength;    /* 8  Bits */
    GT_U8               routingType;     /* 8  Bits */
    GT_U8               segmentsLeft;    /* 8  Bits */
    GT_U8               lastEntry;       /* 8  Bits */
    GT_U8               flags;           /* 8  Bits */
    GT_U16              tag;             /* 16 Bits */
} TGF_PACKET_SRH_STC;

/* struct that holds info about the part */
typedef struct
{
    TGF_PACKET_PART_ENT type;
    GT_VOID            *partPtr;
} TGF_PACKET_PART_STC;

/* struct hold the packet info */
typedef struct
{
    GT_U32               totalLen;   /* not include CRC */ /* support TGF_PACKET_AUTO_CALC_LENGTH_CNS */
    GT_U32               numOfParts; /* num of elements in of partsArray */
    TGF_PACKET_PART_STC *partsArray;
} TGF_PACKET_STC;

/* struct hold the expected egress packet info */
typedef struct{
    GT_PHYSICAL_PORT_NUM    portNum;    /* the egress portNum */
    GT_U32                  burstCount; /* value TGF_AS_INGRESS_CNS means using 'ingress burstCount'
                                                */
    GT_U32                  numBytes;   /* ignored when  pktInfo != NULL (calculated from pktInfo)
                                                */
    TGF_PACKET_STC         *pktInfo;    /* when not NULL this is the expected packet to egress (and the length) */
    GT_U32                 *byteNumMaskListPtr;    /* pointer to byte array that will be skipped in function tgfTrafficGeneratorPortTxEthCaptureCompare() */
    GT_U32                  byteNumMaskSize;        /* size of byte array that will be skipped in function tgfTrafficGeneratorPortTxEthCaptureCompare() */
}TGF_EXPECTED_EGRESS_INFO_STC;

/* struct that holds the VFD info */
typedef struct
{
    TGF_VFD_MODE_ENT    mode;
    GT_U32              modeExtraInfo; /* info relate to mode :
                                                for increment,decrement  it is the current step (0,1,2...)
                                                and for static (to optimize copies)
                                       */
    GT_U32              incValue; /* info relate to mode :
                                                for increment_value,decrement_value  it is the value for inc/dec
                                  */
    GT_U32              offset; /* offset in the packet to override */
    GT_U8               patternPtr[TGF_VFD_PATTERN_BYTES_NUM_CNS];/* the pattern for the packet in the needed offset */
    GT_U8               bitMaskPtr[TGF_VFD_PATTERN_BYTES_NUM_CNS];/* inverted bitwise bitMask for patternPtr checking:
                                                                     0 - to check appropriate PatternPtr bit,
                                                                     1 - to skip.
                                                                     Relevant only for modes:
                                                                     TGF_VFD_MODE_STATIC_E,
                                                                     TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E,
                                                                     TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E.
                                                                   */
    GT_U32              cycleCount;/* number of bytes in patternPtr and maskPtr */

    GT_U8*              arrayPatternPtr; /* buffer of arrays in bytes for each burst */
    GT_U32              arrayPatternOffset; /* offset in patternt buffer -- start number of array in pattern */
    GT_U32              arrayPatternSize; /* size (in bytes) of pattern for single burst */
} TGF_VFD_INFO_STC;

/* enum of checksum fields that may be in packet */
typedef enum{
    TGF_AUTO_CHECKSUM_FIELD_IPV4_E,
    TGF_AUTO_CHECKSUM_FIELD_TCP_E,
    TGF_AUTO_CHECKSUM_FIELD_UDP_E,
    TGF_AUTO_CHECKSUM_FIELD_GRE_E,


    TGF_AUTO_CHECKSUM_FIELD_LAST_E /* must be last */
}TGF_AUTO_CHECKSUM_FIELD_ENT;

/* struct for auto checksum calculations */
typedef struct {
    GT_BOOL enable;     /* do we need auto recalc */
    GT_U8*  startPtr;   /* pointer to start of packet to do checksum */
    GT_U32  numBytes;   /* number of bytes to do checksum on */
    GT_U8*  checkSumPtr;/* pointer to checksum field */
}TGF_AUTO_CHECKSUM_INFO_STC;

/* the max number of sets of headers(ipv4,udp,tcp) that may need auto checksum calculation */
/* each set can occupy IPV4,UDP,TCP (see TGF_AUTO_CHECKSUM_FIELD_ENT)*/
#define TGF_AUTO_CHECKSUM_NUM_OF_HEADERS_CNS    8


/* typedef for single instance needed for auto checksum calculations */
typedef TGF_AUTO_CHECKSUM_INFO_STC   TGF_AUTO_CHECKSUM_INSTANCE_INFO_TYP   [TGF_AUTO_CHECKSUM_FIELD_LAST_E];

/* struct for single instance needed for auto checksum calculations
each struct can occupy IPV4,UDP,TCP (see TGF_AUTO_CHECKSUM_FIELD_ENT)*/
typedef struct {
    TGF_AUTO_CHECKSUM_INFO_STC singleInfoArr[TGF_AUTO_CHECKSUM_FIELD_LAST_E];
}TGF_AUTO_CHECKSUM_FULL_INFO_STC;


/* the IPv6 pseudo header need 40 bytes :
16 - sip
16 - dip
4  - Upper-Layer Packet Length
3  - zero
1  - Next Header

the Ipv4 pseudo header need 12 bytes:
4 - sip
4 - dip
1 - zero
1 - protocol
2 - TCP/UDP Length
*/
#define TGF_AUTO_CHECKSUM_PSEUDO_NUM_BYTES_CNS  40

/* struct for extra checksum info */
typedef struct{
    struct{
        GT_U8   buffer[TGF_AUTO_CHECKSUM_PSEUDO_NUM_BYTES_CNS];/* pseudo buffer length */
        GT_U32  numBytes;/* actual number of bytes used in the buffer */
    }pseudo;/*pseudo header bytes for TCP/UDP header checksum calc */
    GT_BOOL  isIpv4;/* ipv4 or ipv6*/
    GT_U8*   startL3HeadrPtr;/*pointer to start of IP header (ipv4/6)*/

    GT_U8*  udpLengthFieldPtr;/* pointer to UDP field , for auto calc from the IPv4/6 header
                                when NULL --> ignored */

}TGF_AUTO_CHECKSUM_EXTRA_INFO_STC;

/* Traffic-gen command related macros */
#define TGF_TRAFFIC_GEN_L2_CUSTOM_TAG_LEN_CNS       16
#define TGF_TRAFFIC_GEN_IPV4_CUSTOM_TAG_LEN_CNS     12
#define TGF_TRAFFIC_GEN_IPV6_CUSTOM_TAG_LEN_CNS     8
#define TGF_TRAFFIC_GEN_UDP_CUSTOM_TAG_LEN_CNS      4
#define TGF_TRAFFIC_GEN_TCP_CUSTOM_TAG_LEN_CNS      16

/**
* @enum TGF_TRAFFIC_GEN_PKT_TYPE_ENT
 *
 * @brief This enum defines the type of packets for traffic-gen command
*/
typedef enum{
    /* Basic packet type */
    TGF_TRAFFIC_GEN_PKT_TYPE_L2_E,       /*0*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L3_E,       /*1*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L4_E,       /*2*/

    /* Tunnel packet types */
    TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L2_E, /*3*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L3_E, /*4*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L2_IN_L4_E, /*5*/

    TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L2_E, /*6*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L3_E, /*7*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L3_IN_L4_E, /*8*/

    TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L2_E, /*9*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L3_E, /*10*/
    TGF_TRAFFIC_GEN_PKT_TYPE_L4_IN_L4_E, /*11*/

    TGF_TRAFFIC_GEN_PKT_TYPE_CTRL_ARP_E, /*12*/
    TGF_TRAFFIC_GEN_PKT_TYPE_CTRL_ICMP_E /*13*/
} TGF_TRAFFIC_GEN_PKT_TYPE_ENT;

/**
* @enum TGF_TRAFFIC_GEN_STREAM_TYPE_ENT
 *
 * @brief This enum defines the type of stream for traffic-gen command
*/
typedef enum{
    /** Same input packet based stream */
    TGF_TRAFFIC_GEN_STREAM_TYPE_INPUT_PKT_E,

    /** Incremental SA MAC based stream */
    TGF_TRAFFIC_GEN_STREAM_TYPE_INCR_SA_MAC_E,

    /** Random SA MAC based stream */
    TGF_TRAFFIC_GEN_STREAM_TYPE_RAND_SA_MAC_E,

    /** Incremental Source IP based stream */
    TGF_TRAFFIC_GEN_STREAM_TYPE_INCR_SOURCE_IP_E,

    /** Random Source IP based stream */
    TGF_TRAFFIC_GEN_STREAM_TYPE_RAND_SOURCE_IP_E
} TGF_TRAFFIC_GEN_STREAM_TYPE_ENT;

/**
* @union TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT
 *
 * @brief Defines custom tag for TCP/UDP tag
 *
*/
typedef union{
    /** UDP custom tag */
    GT_U8 udp[TGF_TRAFFIC_GEN_UDP_CUSTOM_TAG_LEN_CNS];

    /** TCP custom tag */
    GT_U8 tcp[TGF_TRAFFIC_GEN_TCP_CUSTOM_TAG_LEN_CNS];
} TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT;

/**
* @union TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT
 *
 * @brief Defines custom tag for ipv4/ipv6 tag
 *
*/
typedef union{
    /** IPv4 custom tag */
    GT_U8 ipv4[TGF_TRAFFIC_GEN_IPV4_CUSTOM_TAG_LEN_CNS];

    /** IPv6 custom tag */
    GT_U8 ipv6[TGF_TRAFFIC_GEN_IPV6_CUSTOM_TAG_LEN_CNS];
} TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT;

/**
* @struct TGF_TRAFFIC_GEN_PARAMS_STC
 *
 * @brief Key parameters of the traffic gen utility
*/
typedef struct
{
    TGF_TRAFFIC_GEN_PKT_TYPE_ENT            pktType;
    TGF_TRAFFIC_GEN_STREAM_TYPE_ENT         streamType;
    GT_U32                                  pktCnt;
    GT_PORT_NUM                             port;
    GT_SW_DEV_NUM                           devNum;
    GT_U32                                  pktSize;

    GT_U16                                  vlanId;
    GT_ETHERADDR                            saMac;
    GT_ETHERADDR                            daMac;
    GT_U8                                   customTagL2[TGF_TRAFFIC_GEN_L2_CUSTOM_TAG_LEN_CNS];
    GT_U8                                   l2tagSize;    /* Supported 4, 8, 12, 16 */

    /* L3, IPv4/IPv6 details */
    GT_BOOL                                 isIpv6;
    GT_BOOL                                 isCustomTagL3;
    GT_IP_ADDR_TYPE_UNT                     ipSrcAddr;
    GT_IP_ADDR_TYPE_UNT                     ipDstAddr;
    TGF_TRAFFIC_GEN_L3_IP_CUSTOM_TAG_UNT    customTagL3;

    /* L4, TCP/UDP details */
    GT_BOOL                                 isTcp;
    GT_BOOL                                 isCustomTagL4;
    GT_U16                                  tcpSrcPort;
    GT_U16                                  tcpDstPort;
    GT_U16                                  udpSrcPort;
    GT_U16                                  udpDstPort;
    TGF_TRAFFIC_GEN_L4_CUSTOM_TAG_UNT       customTagL4;
} TGF_TRAFFIC_GEN_PARAMS_STC;


/******************************************************************************\
 *                            Public API section                              *
\******************************************************************************/

/**
* @internal tgfTrafficEnginePacketForceDsaOnPacket function
* @endinternal
*
* @brief   force DSA tag into a packet that do not hold DSA.
*         also allow the DSA to replace the outer VLAN (if exists)
*         this function allow the 'send packet' and the 'compare packet' functions
*         to not be aware to the DSA that need to be in the packet.
*         this is useful before: prvTgfStartTransmitingEth , tgfTrafficGeneratorPortTxEthCaptureCompare
*         since both use function tgfTrafficEnginePacketBuild
* @param[in] dsaPartPtr               - pointer to the DSA part.
* @param[in] replaceVlanTagIfExists   - indication to replace the outer VLAN (if exists)
*
* @retval GT_OK                    - on success.
*
* @note Caller must call it with dsaPartPtr = NULL , to remove the configurations.
*
*/
GT_STATUS tgfTrafficEnginePacketForceDsaOnPacket
(
    IN TGF_PACKET_DSA_TAG_STC   *dsaPartPtr,
    IN GT_BOOL                  replaceVlanTagIfExists
);

/**
* @internal tgfTrafficEnginePacketBuild function
* @endinternal
*
* @brief   Build packet from input fields
*
* @param[in] packetInfoPtr            - packet fields info
* @param[in,out] numOfBytesInBuffPtr      - (pointer to)number of bytes in the bufferPtr
*
* @param[out] bufferPtr                - (pointer to) the buffer that hold the packet bytes
* @param[in,out] numOfBytesInBuffPtr      - (pointer to)number of bytes actually set in the bufferPtr
*                                      checksumInfoArr - (array of) info about instance of fields that need auto checksum build
* @param[out] checksumExtraInfoPtr     -  (array of) extra info for auto checksum build
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note Buffer must be allocated for CRC (4 bytes)
*
*/
GT_STATUS tgfTrafficEnginePacketBuild
(
    IN  TGF_PACKET_STC    *packetInfoPtr,
    OUT GT_U8             *bufferPtr,
    INOUT  GT_U32         *numOfBytesInBuffPtr,
    OUT TGF_AUTO_CHECKSUM_FULL_INFO_STC *checksumInfoPtr,
    OUT TGF_AUTO_CHECKSUM_EXTRA_INFO_STC *checksumExtraInfoPtr
);

/**
* @internal tgfTrafficEnginePacketParse function
* @endinternal
*
* @brief   Parse packet from bytes to parts and fields
*
* @param[in] bufferPtr                - (pointer to) the buffer that hold the packet bytes
* @param[in] bufferLength             - length of the buffer
*
* @param[out] packetInfoPtrPtr         - (pointer to) packet fields info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*
* @note The packetInfoPtrPtr is pointer to static memory that is 'reused' for
*       every new packet that need 'parse' , so use this packet info before
*       calling to new 'parse'
*
*/
GT_STATUS tgfTrafficEnginePacketParse
(
    IN  GT_U8              *bufferPtr,
    IN  GT_U32              bufferLength,
    OUT TGF_PACKET_STC    **packetInfoPtrPtr
);

/**
* @internal tgfTrafficEnginePacketVfdApply function
* @endinternal
*
* @brief   Apply VFD info on the buffer
*
* @param[in] vfdPtr                   - (pointer to) VFD info
* @param[in,out] bufferPtr                - (pointer to) the buffer that hold the packet bytes
*                                      before modification
* @param[in] bufferLength             - the number of bytes in the buffer (not include CRC bytes)
* @param[in,out] bufferPtr                - (pointer to) the buffer that hold the packet bytes
*                                      after modification
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficEnginePacketVfdApply
(
    IN    TGF_VFD_INFO_STC       *vfdPtr,
    INOUT GT_U8                  *bufferPtr,
    IN    GT_U32                  bufferLength
);

/**
* @internal tgfTrafficEnginePacketVfdCheck function
* @endinternal
*
* @brief   Check if the VFD parameters match the buffer
*
* @param[in] vfdPtr                   - (pointer to) VFD info
* @param[in] bufferPtr                - (pointer to) the buffer that hold the packet bytes
* @param[in] bufferLength             - the number of bytes in the buffer (not include CRC bytes)
*
* @param[out] matchPtr                 - (pointer to) do we have a match
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note vfdPtr->mode should be TGF_VFD_MODE_STATIC_E or
*       TGF_VFD_MODE_STATIC_EXPECTED_NO_MATCH_E or
*       TGF_VFD_MODE_STATIC_NO_MATCH_IGNORED_E
*
*/
GT_STATUS tgfTrafficEnginePacketVfdCheck
(
    IN  TGF_VFD_INFO_STC       *vfdPtr,
    IN  GT_U8                  *bufferPtr,
    IN  GT_U32                  bufferLength,
    OUT GT_BOOL                *matchPtr
);

/**
* @internal tgfTrafficEnginePacketCheckSum16BitsCalc function
* @endinternal
*
* @brief   Calculate checksum of 16 bits
*
* @param[in] bytesPtr                 - (pointer) to start of section in packet need to be calculated
* @param[in] numBytes                 - number of bytes need to be included in the calculation
* @param[in] pseudoBytesPtr           - (pointer to ) start of pseudo info
* @param[in] pseudoNumBytes           - number of pseudo bytes
*
* @param[out] checkSumArray[2]         - array of 2 bytes with the calculated check sum
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note If there's a field CHECKSUM within the input-buffer
*       it supposed to be zero before calling this function.
*
*/
GT_STATUS tgfTrafficEnginePacketCheckSum16BitsCalc
(
    IN  GT_U8              *bytesPtr,
    IN  GT_U32              numBytes,
    IN  GT_U8              *pseudoBytesPtr,
    IN  GT_U32              pseudoNumBytes,
    OUT GT_U8               checkSumArray[2]
);

/**
* @internal tgfTrafficTracePacket function
* @endinternal
*
* @brief   Trace the packet bytes
*
* @param[in] bufferPtr                - (pointer to) the buffer that hold the packet bytes
* @param[in] length                   -  of packet
* @param[in] isCrcIncluded            - enable\disable printout CRC in log
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note Before use this fucntion set 'GT_TRUE' in tgfTrafficTracePacketByteSet
*
*/
GT_STATUS tgfTrafficTracePacket
(
    IN GT_U8           *bufferPtr,
    IN GT_U32           length,
    IN GT_BOOL          isCrcIncluded
);

/**
* @internal tgfTrafficTraceFieldInPartInPacketSet function
* @endinternal
*
* @brief   Open/Close tracing for a field
*
* @param[in] packetPartType           - the part that represent the 'context' of field
* @param[in] trace                    -  enable/disable
* @param[in] fieldOffset              - offset in bytes of field from start of structure that
*                                      represent the packetPartType
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong packetPartType parameter.
* @retval GT_OUT_OF_RANGE          - on out of range fieldNum
*/
GT_STATUS tgfTrafficTraceFieldInPartInPacketSet
(
    IN TGF_PACKET_PART_ENT   packetPartType,
    IN GT_BOOL               trace,
    IN GT_U32                fieldOffset
);

/**
* @internal tgfTrafficTraceAllFieldsInPartSet function
* @endinternal
*
* @brief   Open/Close the tracing for all fields in specific packet
*
* @param[in] packetPartType           - the part that represent the 'context' of field
* @param[in] trace                    -  enable/disable
*
* @retval GT_OK                    - on success.
*/
GT_STATUS tgfTrafficTraceAllFieldsInPartSet
(
    IN TGF_PACKET_PART_ENT   packetPartType,
    IN GT_BOOL               trace
);

/**
* @internal tgfTrafficTraceAllFieldsInPartsInPacketSet function
* @endinternal
*
* @brief   Open/Close the tracing for all fields in packet (all parts)
*
* @param[in] trace                    -  enable/disable
*
* @retval GT_OK                    - on success.
*/
GT_STATUS tgfTrafficTraceAllFieldsInPartsInPacketSet
(
    IN GT_BOOL              trace
);

/**
* @internal tgfTrafficTracePacketByteSet function
* @endinternal
*
* @brief   Open/Close the tracing on bytes of packets
*
* @param[in] trace                    -  enable/disable
*
* @retval GT_OK                    - on success.
*/
GT_STATUS tgfTrafficTracePacketByteSet
(
    IN GT_BOOL              trace
);

/**
* @internal tgfTrafficTraceInit function
* @endinternal
*
* @brief   Initialize the DB of the trace utility
*
* @retval GT_OK                    - on success.
* @retval GT_INIT_ERROR            - on error
*/
GT_STATUS tgfTrafficTraceInit
(
    GT_VOID
);

/**
* @internal tgfTrafficEnginePacketLengthCalc function
* @endinternal
*
* @brief   Calculate length of packet by summary the length of the parts and ignoring
*         packetInfoPtr->totalLen
*         (the function will automatically ADD 4 bytes for CRC)
* @param[in] packetInfoPtr            - packet fields info (the function will automatically ADD 4 bytes for CRC)
*
* @param[out] lengthPtr                - (pointer to) the calculated length of the packet.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong parameter.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_STATE             - parser caused bad state that should not happen
*/
GT_STATUS tgfTrafficEnginePacketLengthCalc
(
    IN  TGF_PACKET_STC    *packetInfoPtr,
    OUT GT_U32            *lengthPtr
);

/*******************************************************************************
* prvTgfTrafficGeneratorPoolGetBuffer
*
* DESCRIPTION:
*       Allocates buffer from pool for sending packets
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_VOID* - Pointer to the new allocated buffer. NULL is returned in case
*               no buffer is not available.
*
* COMMENTS:
*
*******************************************************************************/
GT_VOID* prvTgfTrafficGeneratorPoolGetBuffer
(
    GT_VOID
);

/**
* @internal prvTgfTrafficGeneratorPoolFreeBuffer function
* @endinternal
*
* @brief   Frees buffer from pool for sending packets
*
* @param[in] bufPtr                   - Pointer to buffer to be freed
*
* @retval GT_OK                    - For successful operation.
* @retval GT_BAD_PTR               - The returned buffer is not belongs to that pool
*/
GT_STATUS prvTgfTrafficGeneratorPoolFreeBuffer
(
    IN GT_VOID *bufPtr
);

/**
* @internal prvTgfTrafficGeneratorPoolInit function
* @endinternal
*
* @brief   Init the pool for sending packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvTgfTrafficGeneratorPoolInit
(
    GT_VOID
);

/**
* @internal UTF_MEMORY_LEAK_PAUSE_SET_FUN function
* @endinternal
*
* @brief   This function changed memory leak test control state.
*
* @param[in] onPause                  - GT_TRUE temporary disabled memLeak tests,
*                                      GT_FALSE to continue.
*
* @retval GT_OK                    - on success (no memory leak).
* @retval GT_BAD_VALUE             - on illegal input parameters value
* @retval GT_FAIL                  - on failure (memory leak detected).
*
* @note none
*
*/
typedef GT_STATUS (*UTF_MEMORY_LEAK_PAUSE_SET_FUN)
(
    GT_BOOL onPause
);

/* memory leakage pause/start management callback */
extern UTF_MEMORY_LEAK_PAUSE_SET_FUN utfMemoryLeakPauseSet_func;


/**
* @internal UTF_DMA_WINDOW_GET_FUN function
* @endinternal
*
* @brief   The function support DMA multi-windows (each device may hold different window) .
*
* @param[in] devNum                - the devNum to get the windowId
* @param[out] windowIdPtr          - (pointer to) the windowId
*
* @retval GT_OK                    - on success .
* @retval GT_FAIL                  - on failure .
*
* @note none
*
*/
typedef GT_STATUS (*UTF_DMA_WINDOW_GET_FUN)
(
    IN GT_SW_DEV_NUM   devNum,
    OUT GT_U32         *windowIdPtr
);


/* function to state the window that a device uses for DMA */
extern UTF_DMA_WINDOW_GET_FUN utfDmaWindowGet_func;



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTrafficEngineh */

