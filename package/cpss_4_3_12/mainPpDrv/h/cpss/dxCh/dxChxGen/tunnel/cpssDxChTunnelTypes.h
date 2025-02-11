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
* @file cpssDxChTunnelTypes.h
*
* @brief CPSS definitions for tunnel.
*
* @version   20
********************************************************************************
*/

#ifndef __cpssDxChTunnelTypesh
#define __cpssDxChTunnelTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/tunnel/cpssGenTunnelTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>

/* max size of template data in bits that generates by Profile table entry */
#define CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_MAX_SIZE_CNS 128
/**
* @enum CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT
 *
 * @brief Enumeration of tunnel start ethernet over x vlan tag mode.
*/
typedef enum{

    /** Vlan tag mode is set according to the tag bit in the vlan entry. */
    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_VLAN_E   = 0,

    /** Vlan tag mode is set according to the <EthernetOverXPassangerTagged> entry */
    CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_FROM_ENTRY_E  = 1

} CPSS_DXCH_TUNNEL_START_ETHERNET_OVER_X_TAG_MODE_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_QOS_MODE_ENT
 *
 * @brief Enumeration of QoS modes.
*/
typedef enum{

    /** @brief keep QoS profile assignment from
     *  previous mechanisms
     */
    CPSS_DXCH_TUNNEL_QOS_KEEP_PREVIOUS_E        = 0,

    /** @brief trust passenger protocol 802.1p user
     *  priority; relevant to EthernetoverMPLS
     *  packets only
     */
    CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_UP_E        = 1,

    /** trust passenger protocol IP DSCP */
    CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_DSCP_E      = 2,

    /** @brief trust passenger protocol 802.1p user
     *  priority and IP DSCP
     */
    CPSS_DXCH_TUNNEL_QOS_TRUST_PASS_UP_DSCP_E   = 3,

    /** @brief trust outer label MPLS EXP in the
     *  tunneling header
     */
    CPSS_DXCH_TUNNEL_QOS_TRUST_MPLS_EXP_E       = 4,

    /** @brief untrust packet QoS, assign QoS profile
     *  according to tunnel termination entry
     *  QoS profile assignment
     */
    CPSS_DXCH_TUNNEL_QOS_UNTRUST_PKT_E          = 5

} CPSS_DXCH_TUNNEL_QOS_MODE_ENT;


/**
* @enum CPSS_DXCH_TUNNEL_TERM_EXCEPTION_ENT
 *
 * @brief Enumeration for tunnel termination exceptions.
*/
typedef enum{

    /** @brief IPv4 tunnel termination header error.
     *  IPv4 Tunnel Termination Header Error exception is detected if ANY of
     *  the following criteria are NOT met:
     *  - IPv4 header <Checksum> is correct
     *  - IPv4 header <Version> = 4
     *  - IPv4 header <IHL> (IP Header Length) >= 5 (32-bit words)
     *  - IPv4 header <IHL> (IP Header Length) <= IPv4 header <Total Length> / 4
     *  - IPv4 header <Total Length> + packet L3 Offset + 4 (CRC length) <= MAC layer packet byte count
     *  - IPv4 header <SIP> != IPv4 header <DIP>
     */
    CPSS_DXCH_TUNNEL_TERM_IPV4_HEADER_ERROR_E       = 0,

    /** @brief IPv4 tunnel termination option/fragment error.
     *  An IPv4 Tunnel Termination Option/Fragment error occurs if the packet
     *  has either IPv4 Options or is fragmented.
     *  - IPv4 Options are detected if the IPv4 header <IP Header Len> > 5.
     *  - The packet is considered fragmented if either the IPv4 header flag
     *  <More Fragments> is set, or if the IPv4 header <Fragment offset> > 0.
     *  A tunnel-terminated fragmented packet must be reassembled prior
     *  to further processing of the passenger packet.
     */
    CPSS_DXCH_TUNNEL_TERM_IPV4_OPTION_FRAG_ERROR_E  = 1,

    /** @brief IPv4 tunnel termination unsupported GRE header error.
     *  An IPv4 Tunnel Termination GRE Header error occurs if the IPv4 header
     *  <IP Protocol> = 47 (GRE) and the 16 most significant bits of the GRE
     *  header are not all 0.
     */
    CPSS_DXCH_TUNNEL_TERM_IPV4_UNSUP_GRE_ERROR_E    = 2

} CPSS_DXCH_TUNNEL_TERM_EXCEPTION_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT
 *
 * @brief Enumeration of tunnel start MPLS LSR TTL operation.
*/
typedef enum{

    /** Set TTL to incoming TTL Assignment */
    CPSS_DXCH_TUNNEL_START_TTL_TO_INCOMING_TTL_E             = 0,

    /** Set TTL to the TTL of the swapped label */
    CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_E               = 1,

    /** Set TTL to the TTL of the swapped label 1 */
    CPSS_DXCH_TUNNEL_START_TTL_TO_SWAP_LABEL_MINUS_ONE_E     = 2,

    /** Set TTL to the TTL of the popped outer label */
    CPSS_DXCH_TUNNEL_START_TTL_TO_POP_OUTER_E             = 3

} CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT
 *
 * @brief Enumeration of tunnel start entry QoS mark modes.
*/
typedef enum{

    /** Mark based on the explicit QoS fields in Tunnel Start entry. */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_ENTRY_E                = 0,

    /** @brief Mark based on the packet QoS Profile assignment. The QoS Profile table
     *  entry provides the tunnel header QoS fields.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_FROM_PACKET_QOS_PROFILE_E   = 1

} CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT;


/**
* @enum CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT
 *
*  @brief Enumeration of tunnel start MPLS PW EXP mark modes.
*/
typedef enum{

    /** @brief EXP bits of the Pushed PW label are taken from
     *         ePort: MPLS PW Label EXP
     **/
    CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_EPORT_E                = 0,

    /** @brief EXP bits of the Pushed PW label are mapped from
     *         the QoS profile by using the QoS Profile-to-EXP
     *         mapping table
     **/
    CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_FROM_PACKET_QOS_PROFILE_E   = 1

}CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT;


/**
* @enum CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT
 *
 * @brief Tunnel passenger protocol types.
*/
typedef enum{

    /** ethernet passenger type */
    CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E           = 0,

    /** other passenger type */
    CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E              = 1

} CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_HEADER_LENGTH_ANCHOR_TYPE_ENT
 *
 *  @brief Enumeration of Tunnel Header Length Anchor Type
 *       The Anchor Type used by the TTI Action "Tunnel Header Length" This field
 *       is relevant if TTI action "Tunnel Terminate" = Enabled
 *       or TTI action "Passenger Parsing of Non-MPLS Transit Tunnels Enable" = Enabled.
*/
typedef enum{
    /** TTI Action "Tunnel Header Length/Profile" is relative to the start of the L3.
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_TUNNEL_HEADER_LENGTH_L3_ANCHOR_TYPE_E             = 0,

    /** TTI Action "Tunnel Header Length/Profile" is relative to the start of the L4.
     * (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_TUNNEL_HEADER_LENGTH_L4_ANCHOR_TYPE_E             = 1,

    /** TTI Action "Tunnel Header Length/Profile" is an index to the Tunnel
     * (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_TUNNEL_HEADER_LENGTH_PROFILE_BASED_ANCHOR_TYPE_E  = 2

} CPSS_DXCH_TUNNEL_HEADER_LENGTH_ANCHOR_TYPE_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of tunnel start MIM I-SID assign mode
*/
typedef enum{

    /** ISID assigned from the tunnel start entry */
    CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_TS_ENTRY_E   ,

    /** ISID assigned from the VLAN table to VLAN ServiceID */
    CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_FROM_VLAN_ENTRY_E

} CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT;


/**
* @enum CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of tunnel start MIM B-DA (Backbone
 * destination address) assign mode
*/
typedef enum{

    /** @brief BDA is fully assigned from Tunnelstart entry: <Tunnel MAC DA>. This
     *  mode is used for known unicast packets.
     */
    CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_E,

    /** @brief The mostsignificant bits of the BDA(47:24) are assigned from the entry
     *  <Tunnel MAC DA> and the 24 least-significant bits(23:0) are assigned from
     *  the eVLAN <Service-ID> field. This mode is used for unknown unicast,
     *  multicast, and broadcast packets.
     */
    CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_FROM_TS_ENTRY_AND_VLAN_ENTRY_E

} CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT;


/**
* @enum CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of tunnel start MIM B-SA (Backbone
 * source address) assign mode
*/
typedef enum{

    /** @brief The MAC SA is selected from a dedicated 256 global MAC SA table
     *  based on the target ePort <Router MAC SA Index>.
     */
    CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_TARGET_EPORT_E,

    /** @brief The MAC SA is selected from a dedicated 256 global MAC SA table
     *  based on the source ePort <Router MAC SA Index>.
     */
    CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_FROM_SOURCE_EPORT_E

} CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT;



/**
* @enum CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT
 *
 * @brief Enumeration of tunnel start entry mpls ethertypes.
*/
typedef enum{

    /** Set MPLS ethertype to 0x8847 (Unicast). */
    CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_UC_E   = 0,

    /** Set MPLS ethertype to 0x8848 (Multicast). */
    CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_MC_E   = 1

} CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT;



/**
* @enum CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT
 *
 * @brief Enumeration of tunnel start ip header protocol.
 * This enumerator is determined value in protocol field of
 * generated IP Tunnel Header.
*/
typedef enum{

    CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_IP_E,

    /** @brief GRE protocol.
     *  The protocol value in IP Tunnel Header is 47.
     */
    CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E,

    /** @brief UDP protocol.
     *  The protocol value in IP Tunnel Header is 17.
     */
    CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E,

    /** Protocol is according to <IP Protocol> field */
    CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E

} CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT;


/**
* @enum CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT
 *
 * @brief Enumeration of tunnel start template data size.
 * This enumerator is determined value of template data size
 * in Generic IP Tunnel start Profile table entry.
 * Based on the template data size, the Profile table entry template generates up to 16
 * bytes of data after the 4B GRE header or after the 8B UDP header.
*/
typedef enum{

    /** No data is generated by this profile template; */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_NONE_E,

    /** 4 byte data is generated by this profile template. */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_4B_E,

    /** 8 byte data is generated by this profile template. */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_8B_E,

    /** 12 byte data is generated by this profile template. */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_12B_E,

    /** 16 byte data is generated by this profile template. */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E

} CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT;



/**
* @enum CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT
 *
 * @brief Enumeration of tunnel start template data configuration.
 * It describes configuration options for each bit of template data.
 * The template is used to generate up to 16B (128 bits) of GRE extension
 * data or UPD shim header.
 * So there are 128 instances of this field, one for each bit generated. It is
 * repeated for bytes y = 0 to y = 15, and for each byte y it is repeated for bits
 * x = 0 to x = 7 (bit 0 is the least significant bit of the byte).
*/
typedef enum{

    /** the relevant bit value should be constant 0. */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_0_E,

    /** the relevant bit value should be constant 1. */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_CONST_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of ePort <TS Extension> .
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of ePort <TS Extension> .
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 2 of ePort <TS Extension> .
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 3 of ePort <TS Extension> .
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_3_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of eVLAN <ServiceID>.
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of eVLAN <ServiceID>.
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 2 of eVLAN <ServiceID>.
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EVLAN_I_SID_2_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of the ShiftedHash value.
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of the ShiftedHash value.
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SHIFT_HASH_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of the Tag1 value.
     *  (APPLICABLE DEVICES: Ironman)
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_TAG1_0_E,

   /** @brief the relevant bit value is taken from respective
    *  bit x of byte 1 of the Tag1 value.
     *  (APPLICABLE DEVICES: Ironman)
    */
   CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_TAG1_1_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 0 of the SrcEpg value.
     *  (APPLICABLE DEVICES: Ironman)
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SRC_EPG_0_E,

    /** @brief the relevant bit value is taken from respective
     *  bit x of byte 1 of the SrcEpg value.
     *  (APPLICABLE DEVICES: Ironman)
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_SRC_EPG_1_E

} CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT;



/**
* @enum CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of udp source port mode .
 * This enumerator describes Tunnel-start UDP source port
 * assignment options.
*/
typedef enum{

    /** @brief UDP header
     *  source port is assigned an explicit value from the IP Generic TS entry <UDP Source Port>.
     */
    CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_IP_GEN_TS_ENTRY_E,

    /** @brief UDP header
     *  source port is assigned with the packet hash value.
     */
    CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_FROM_PACKET_HASH_VALUE_E

} CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT;



/**
* @enum CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT
 *
 * @brief Enumeration of flow label assign mode .
 * This enumerator describes Tunnel-start flow label
 * assignment options.
*/
typedef enum{

    /** Set the Flow Label to 0. */
    CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_ZERO_E,

    /** Set the Flow Label to packet hash value. */
    CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_TO_PACKET_HASH_VALUE_E

} CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT
 *
 * @brief Enumeration of ECN mode.
 * This enumerator describes Tunnel-start ECN modes (according to rfc 6040).
*/
typedef enum{

    /** compatibility mode */
    CPSS_DXCH_TUNNEL_START_ECN_COMPATIBILITY_MODE_E,

    /** normal mode */
    CPSS_DXCH_TUNNEL_START_ECN_NORMAL_MODE_E

} CPSS_DXCH_TUNNEL_START_ECN_MODE_ENT;

/**
* @enum CPSS_DXCH_TUNNEL_START_GENERIC_TYPE_ENT
 *
 * @brief Enumeration of generic tunnel type.
 * This enumerator describes generic tunenl types entries.
*/
typedef enum{

    /** @brief short generic tunnel start [96 bits] */
    CPSS_DXCH_TUNNEL_START_GENERIC_SHORT_TYPE_E,

    /** @brief medium generic tunnel start [192 bits] */
    CPSS_DXCH_TUNNEL_START_GENERIC_MEDIUM_TYPE_E,

    /** @brief long generic tunnel start [384 bits] */
    CPSS_DXCH_TUNNEL_START_GENERIC_LONG_TYPE_E

} CPSS_DXCH_TUNNEL_START_GENERIC_TYPE_ENT;

/**
* @struct CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC
 *
 * @brief Struct of configuration for Generic IP Tunnel start Profile table entry.
 * serves as logical extension to the Generic IPv4 Tunnel-Start and
 * Generic IPv6 Tunnel-Start when the IPv4/6 tunnel header includes
 * either a GRE or UDP header.
*/
typedef struct{

    /** @brief template data size. Template data is used to generate up to 16Byte.
     *  Template bytes are concatenated in the following order: Byte0,
     *  Byte1,..., Byte15, while each byte format is: (Bit7, Bit6,..., Bit1, Bit0).
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_SIZE_ENT templateDataSize;

    /** @brief consecutive configuration for each bit in byte template:
     *  Template data is used to generate up to 128 bits (16 Bytes),
     *  so there are 128 instances of this field, one for each bit generated.
     *  It selects the source info for each bit in the template.
     *  For example, user wants to configure bit 6 of byte 5 in template to be taken from
     *  tunnel start extension byte 2. In order to do that bit 46 (5*8+6 = 46) in array
     *  templateDataBitsCfg is configured to CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_EPORT_TS_EXT_2_E.
     */
    CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_CFG_ENT templateDataBitsCfg[CPSS_DXCH_TUNNEL_START_TEMPLATE_DATA_MAX_SIZE_CNS];

    /** @brief The number of bits to circular shift left the internal 12-bit Packet Hash
     *  value. (APPLICABLE RANGES: 0...7). The result is a 16-bit 'Shifted-Hash' value.
     */
    GT_U32 hashShiftLeftBitsNumber;

    /** The Tunnel-start UDP source port assignment mode. */
    CPSS_DXCH_TUNNEL_START_UDP_SRC_PORT_ASSIGN_MODE_ENT udpSrcPortMode;

    /** @brief The number of bits to circularly shift the eVLAN attribute <Service
     *  The result is the Shifted Service-ID value.
     *  (APPLICABLE RANGES: 0...23),Values 24-31 are reserved
     *  This value is used for the Profile entry <MAC DA mode> and/or
     *  Profile entry <Destination IP mode> if the respective field is
     *  configured to use the shifted Service-ID in its LS bits.
     *  This shifted Service-ID is not used by the Profile template header generation.
     */
    GT_U32 serviceIdCircularShiftSize;

    /** @brief 0 = Set MAC DA according to TS entry <MAC DA>.
     *  x (1-24) = The x LS bits are taken from the Shifted eVLAN attribute <Service-ID>.
     *  The MS bits are taken from the MS bits of the TS entry <MAC DA>
     *  (APPLICABLE RANGES: 0...24), 25-31 = Reserved
     */
    GT_U32 macDaMode;

    /** @brief 0 = Set IP address according to TS entry <DIP>
     *  x (1-24) = The x LS bits are taken from the Shifted eVLAN <Service-ID> attribute.
     *  The MS bits are taken from the MS bits of the TS entry's <DIP>.
     *  (APPLICABLE RANGES: 0...24), 25-31 = Reserved
     */
    GT_U32 dipMode;

    /** @brief when controlWordEnable is set to GT_TRUE, insert
     *  control word (indexed by this field) after the MPLS labels.
     *  (APPLICABLE RANGES: 0..6)
     */
    GT_U32 controlWordIndex;

    /** @brief whether to insert an MPLS control word to the MPLS tunnel
     */
    GT_BOOL controlWordEnable;

} CPSS_DXCH_TUNNEL_START_GEN_IP_PROFILE_STC;

/**
* @struct CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC
 *
 * @brief Struct of configuration for Generic IPv4 tunnel start.
*/
typedef struct{

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
     */
    GT_U16 vlanId;

    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to packet assigned QoS attributes
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** @brief Options:
     *  - Set DSCP according to entry <DSCP>.
     *  - Set DSCP according to packet QoS Profile assignment.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT dscpMarkMode;

    /** @brief The DSCP in the IPv4 tunnel header (APPLICABLE RANGES: 0..63)
     *  Relevant only if entry <dscpMarkMode> is according
     *  to entry <DSCP>.
     */
    GT_U32 dscp;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** Don't fragment flag in the tunnel IPv4 header */
    GT_BOOL dontFragmentFlag;

    /** @brief tunnel IPv4 header TTL (APPLICABLE RANGES: 0..255)
     *  0:   use passenger packet TTL
     *  1-255: use this field for header TTL
     */
    GT_U32 ttl;

    /** @brief Relevant for IPv6
     *  if GT_TRUE, the IPv4 header DIP is derived from IPv6
     *  passenger packet and destIp field is ignored
     */
    GT_BOOL autoTunnel;

    /** @brief Relevant for IPv6
     *  If <autoTunnel> is GT_TRUE, this field is the offset
     *  of IPv4 destination address inside IPv6 destination
     *  address. Offset is measured in bytes between LSBs of
     *  the addresses:
     *  1. IPv4 compatible & ISATAP = 0
     *  2. 6to4 = 10 (decimal)
     */
    GT_U32 autoTunnelOffset;

    /** Tunnel destination IP */
    GT_IPADDR destIp;

    /** Tunnel source IP */
    GT_IPADDR srcIp;

    /** @brief CFI bit assigned for TS packets (xCat3; Lion2)
     *  0: Canonical MAC
     *  1: non-Canonical MAC
     */
    GT_U32 cfi;

    /** @brief If the passenger packet is Ethernet, this field determines whether the original
     *  passenger Ethernet CRC is retain or stripped. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2)
     *  GT_TRUE: The Ethernet passenger packet 4-bytes of CRC are
     *  not removed, nor modified, and the tunneled packet is
     *  transmitted with two CRC fields, the passenger packet
     *  original, unmodified CRC field and the newly generated CRC
     *  for the entire packet. GT_FALSE: The Ethernet passenger
     *  packet 4-bytes of CRC are removed and the tunneled packet is
     *  transmitted with a newly generated CRC for the entire packet
     */
    GT_BOOL retainCRC;

    /** IP Header protocol field in the tunnel header.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman) */
    CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT ipHeaderProtocol;

    /** @brief This field contains the <Protocol> field in the IP header.Valid if ipHeaderProtocol is equal to
     *  CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E(APPLICABLE DEVICES: Caelum; Aldrin;
     *  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 ipProtocol;

    /** @brief The profile table index of this Generic IPv4 TS entry.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
     *  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  (APPLICABLE RANGES: 0...7).
     *  The Generic Tunnel-start Profile table serves as logical extension to the
     *  Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes either
     *  GRE or UDP header.
     *  There are 8 profiles available to use. But if application is
     *  going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
     *  (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
     *  tunnel types take place , and these tunnels are not required profile data generation - empty
     *  profile should be reserved. In this case, in order to save empty profiles number, it is
     *  recommended to utilize profile 7 as well.
     */
    GT_U32 profileIndex;

    /** @brief GRE Protocol for Ethernet Passenger. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E. If the passenger packet is Ethernet,
     *  this field defines the GRE protocol.(The least-significant 16 bits of
     *  the first word in the GRE header are generated in accordance with this value)
     *  NOTE: If the passenger is IPv4, the GRE protocol is fixed: 0x0800.
     *  If the passenger is IPv6, the GRE protocol is fixed: 0x86DD.
     */
    GT_U32 greProtocolForEthernet;

    /** @brief GRE flags and version. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E.
     *  The most significant 16-bits of the GRE header that includes the GRE flag bits and the version number.
     */
    GT_U32 greFlagsAndVersion;

    /** @brief UDP destination port. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E.
     */
    GT_U32 udpDstPort;

    /** @brief UDP source port. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E and the Profile <UDP
     *  source port mode> is not set to Hash Mode.
     */
    GT_U32 udpSrcPort;

} CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC;



/**
* @struct CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC
 *
 * @brief Struct of configuration for Generic IPv6 tunnel start.
*/
typedef struct{

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0...4095).
     */
    GT_U16 vlanId;

    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to packet assigned QoS attributes
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (APPLICABLE RANGES: 0...7).
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** @brief Options:
     *  - Set DSCP according to entry <DSCP>.
     *  - Set DSCP according to packet QoS Profile assignment.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT dscpMarkMode;

    /** @brief The DSCP in the IPv4 tunnel header (APPLICABLE RANGES: 0...63).
     *  Relevant only if entry <dscpMarkMode> is according
     *  to entry <DSCP>.
     */
    GT_U32 dscp;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** @brief tunnel IPv6 header hopLimit (APPLICABLE RANGES: 0...255).
     *  0:   use passenger packet hopLimit
     *  1-255: use this field for header hopLimit
     */
    GT_U32 ttl;

    /** Tunnel destination IP */
    GT_IPV6ADDR destIp;

    /** Tunnel source IP */
    GT_IPV6ADDR srcIp;

    /** @brief If the passenger packet is Ethernet, this field determines whether the original
     *  passenger Ethernet CRC is retain or stripped.
     *  GT_TRUE: The Ethernet passenger packet 4-bytes of CRC are not removed, nor modified,
     *  and the tunneled packet is transmitted with two CRC fields, the passenger
     *  packet original, unmodified CRC field and the newly generated CRC for
     *  the entire packet.
     *  GT_FALSE: The Ethernet passenger packet 4-bytes of CRC are removed and
     *  the tunneled packet is transmitted with a newly generated CRC
     *  for the entire packet
     */
    GT_BOOL retainCRC;

    /** IP Header protocol field in the tunnel header. */
    CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_ENT ipHeaderProtocol;

    /** @brief This field contains the <Protocol> field in the IP header.Valid if ipHeaderProtocol is equal to
     *  CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GENERIC_E(APPLICABLE DEVICES: Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_U32 ipProtocol;

    /** @brief The profile table index of this Generic IPv4 TS entry.(APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  (APPLICABLE RANGES: 0...7).
     *  The Generic Tunnel-start Profile table serves as logical extension to the
     *  Generic IPv4/6 Tunnel-Start when the IPv4/6 tunnel header includes either
     *  GRE or UDP header.
     *  There are 8 profiles available to use. But if application is
     *  going to use legacy ip tunnel types, cpss is reserved profile index 7 for legacy support
     *  (empty profile: no extension data is added in tunnel header). Also if usage of new ipv4/6
     *  tunnel types take place , and these tunnels are not required profile data generation - empty
     *  profile should be reserved. In this case, in order to save empty profiles number, it is
     *  recommended to utilize profile 7 as well.
     */
    GT_U32 profileIndex;

    /** @brief GRE Protocol for Ethernet Passenger. This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E. If the passenger packet is Ethernet,
     *  this field defines the GRE protocol.(The least-significant 16 bits of
     *  the first word in the GRE header are generated in accordance with this value)
     *  NOTE: If the passenger is IPv4, the GRE protocol is fixed: 0x0800.
     *  If the passenger is IPv6, the GRE protocol is fixed: 0x86DD.
     */
    GT_U32 greProtocolForEthernet;

    /** @brief UDP destination port. This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E.
     */
    GT_U32 udpDstPort;

    /** @brief GRE flags and version. This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_GRE_E.
     *  The most significant 16-bits of the GRE header that includes the
     *  GRE flag bits and the version number.
     */
    GT_U32 greFlagsAndVersion;

    /** @brief UDP source port. This field is relevant only if ipHeaderProtocol
     *  is CPSS_DXCH_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E and the Profile <UDP
     *  source port mode> is not set to Hash Mode.
     */
    GT_U32 udpSrcPort;

    /** ipv6 flow label mode. */
    CPSS_DXCH_TUNNEL_START_FLOW_LABEL_ASSIGN_MODE_ENT flowLabelMode;

} CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC;


/**
* @struct CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC
 *
 * @brief Struct of configuration for X-over-MPLS tunnel start
*/
typedef struct{

    /** If set, the packet is sent with VLAN tag */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
     */
    GT_U16 vlanId;

    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to packet assigned QoS attributes
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** Tunnel next hop MAC DA */
    GT_ETHERADDR macDa;

    /** @brief number of MPLS labels
     *  xCat3 and above: (APPLICABLE RANGES: 1..3)
     */
    GT_U32 numLabels;

    /** @brief tunnel header TTL (APPLICABLE RANGES: 0..255)
     *  0: <ttlMode> determines how the TTL is assigned to new MPLS
     *  labels 1-255: use this field for header TTL
     */
    GT_U32 ttl;

    /** @brief MPLS LSR TTL operations;
     *  Relevant when <ttl> = 0
     */
    CPSS_DXCH_TUNNEL_START_TTL_MODE_ENT ttlMode;

    /** @brief MPLS label 1; (APPLICABLE RANGES: 0..1048575) if <numLabels> > 1 then this
     *  is the inner label.
     */
    GT_U32 label1;

    /** @brief Options:
     *  - Set EXP1 according to entry <exp1>.
     *  - Set EXP1 according to packet QoS Profile assignment
     *  to EXP mapping table.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT exp1MarkMode;

    /** @brief The EXP1 value (APPLICABLE RANGES: 0..7)
     *  Relevant if entry <exp1MarkMode> is set according to
     *  entry <EXP1>.
     */
    GT_U32 exp1;

    /** @brief MPLS label 2; (APPLICABLE RANGES: 0..1048575) Relevant only when number of
     *  labels is 2 or more; if <numLabels> = 2 then this is the outer
     *  label, immediately following the MPLS Ether Type.
     */
    GT_U32 label2;

    /** @brief Relevant only when number of labels is 2;
     *  Options:
     *  - Set EXP2 according to entry <exp2>.
     *  - Set EXP2 according to packet QoS Profile assignment
     *  to EXP mapping table.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT exp2MarkMode;

    /** @brief The EXP2 value (APPLICABLE RANGES: 0..7)
     *  Relevant only when number of labels is 2 and if entry
     *  <exp2MarkMode> is set to according to entry <EXP2>.
     */
    GT_U32 exp2;

    /** @brief MPLS label 3; (APPLICABLE RANGES: 0..1048575) Relevant only when number of
     *  labels is 3; if <numLabels> = 3 then this is the outer
     *  label, immediately following the MPLS Ether Type.
     */
    GT_U32 label3;

    /** @brief Relevant only when number of labels is 3;
     *  Options:
     *  - Set EXP3 according to entry <exp3>.
     *  - Set EXP3 according to packet QoS Profile assignment
     *  to EXP mapping table.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT exp3MarkMode;

    /** @brief The EXP3 value (APPLICABLE RANGES: 0..7)
     *  Relevant only when number of labels is 3 and if entry
     *  <exp3MarkMode> is set to according to entry <EXP3>.
     */
    GT_U32 exp3;

    /** @brief Relevant for Ethernet
     *  the passenger packet CRC and add additional 4-byte CRC
     *  based on the tunneling header packet.
     */
    GT_BOOL retainCRC;

    /** @brief <GT_FALSE>: S-bit is cleared
     *         <GT_TRUE>: S-bit is set on Label1
     *  If <Number of MPLS Labels> is two or three, Label1 is
     *  the inner label. This flag is typically enabled at
     *  an MPLS LER, and disabled at an LSR.
     *  (For Bobcat2, Caelum, Bobcat3, Aldrin, AC3X, Aldrin2, Falcon
     *  this bit is set automatically by the device)
     */
    GT_BOOL setSBit;

    /** @brief CFI bit assigned for TS packets (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     *  0: Canonical MAC
     *  1: non-Canonical MAC
     */
    GT_U32 cfi;

    /** @brief whether to insert an MPLS control word to the MPLS tunnel
     *  (APPLICABLE DEVICES: xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL controlWordEnable;

    /** @brief the MPLS Control word to insert to the MPLS tunnel.
     *  Applicable when controlWordEnable is set to GT_TRUE.
     *  (APPLICABLE DEVICES: xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     *  Bobcat2; Caelum; Bobcat3; Aldrin2; Aldrin; AC3X; Falcon; AC5P: (APPLICABLE RANGES: 0..14)
     *  xCat3: (APPLICABLE RANGES: 0..6)
     */
    GT_U32 controlWordIndex;

    /** @brief Select which of the 2 global MPLS EtherTypes to use in
     *  the outgoing packet. Per RFC 5332, there are separate
     *  MPLS EtherTypes for downstream assigned labels and
     *  multicast upstream assigned labels.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_TUNNEL_START_MPLS_ETHER_TYPE_ENT mplsEthertypeSelect;

    /** @brief whether to push Entropy Label Indicator (ELI) and
     *  Entropy Label (EL) after label1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL pushEliAndElAfterLabel1;

    /** @brief whether to push Entropy Label Indicator (ELI) and
     *  Entropy Label (EL) after label2.
     *  Relevant if numLabels is bigger than 1.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL pushEliAndElAfterLabel2;

    /** @brief whether to push Entropy Label Indicator (ELI) and
     *  Entropy Label (EL) after label3.
     *  Relevant if numLabels is bigger than 2.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL pushEliAndElAfterLabel3;

    /** @brief MPLS PW EXP marking mode */
    CPSS_DXCH_TUNNEL_START_MPLS_PW_EXP_MARK_MODE_ENT pwExpMarkMode;

} CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC;

/**
* @struct CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC
 *
 * @brief Struct of configuration for MacInMac tunnel start entry
 * APPLICABLE DEVICES:
 * xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
 * NOT APPLICABLE DEVICES:
*/
typedef struct{

    /** @brief GT_TRUE: the packet is sent with VLAN tag.
     *  GT_FALSE: the packet is send without VLAN tag.
     */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
     */
    GT_U16 vlanId;

    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to up assigned by previous
     *  ingress engines.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (APPLICABLE RANGES: 0..7).
     *  Relevant only if entry <tagEnable> is set and entry.
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** Tunnel header MAC DA. */
    GT_ETHERADDR macDa;

    /** @brief Relevant for Ethernet passenger packet.
     *  GT_TRUE: retain passenger packet CRC and add
     *  additional 4-byte CRC based on the tunneling header
     *  packet.
     *  GT_FALSE: remove passenger packet CRC.
     */
    GT_BOOL retainCrc;

    /** MacInMac inner service identifier (24 bits). */
    GT_U32 iSid;

    /** @brief inner user priority field (PCP) in the 802.1ah iTag.
     *  (APPLICABLE RANGES: 0..7).
     */
    GT_U32 iUp;

    /** @brief Marking mode to determine assignment of iUp. Options:
     *  - Set iUp according to entry (iUp).
     *  - Set iUp according to iUp assigned by previous ingress
     *  engines.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT iUpMarkMode;

    /** drop precedence in the iTag (APPLICABLE RANGES: 0..1). */
    GT_U32 iDp;

    /** @brief I
     *  - Set I-DEI according to entry <iDei>
     *  - Set I-DEI according to DP assigned by previous ingress
     *  engines, and according to DP to CFI mapping table
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT iDpMarkMode;

    /** @brief The reserved bits to be set in the iTag (APPLICABLE RANGES: 0..15).
     *  Bits 1-2: Res1
     *  Bit  3: Res2
     *  Bit  4: UCA (Use Customer Address)
     */
    GT_U32 iTagReserved;

    /** @brief I-SID assignment mode.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3)
     */
    CPSS_DXCH_TUNNEL_START_MIM_I_SID_ASSIGN_MODE_ENT iSidAssignMode;

    /** @brief B-DA (backbone destination address) asignment mode.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_TUNNEL_START_MIM_B_DA_ASSIGN_MODE_ENT bDaAssignMode;

    /** @brief B-SA (backbone source address) asignment mode.
     *  (APPLICABLE DEVICES: Caelum; Aldrin; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_TUNNEL_START_MIM_B_SA_ASSIGN_MODE_ENT bSaAssignMode;

} CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC;

/**
* @struct CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC
 *
 * @brief Struct of configuration for generic tunnel start
 * APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 * NOT APPLICABLE DEVICES:
 * xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*/
typedef struct
{
    /** @brief Relevant only if entry <tagEnable> is set.
     *  Options:
     *  - Set user priority according to entry <802.1p UP>.
     *  - Set user priority according to up assigned by previous
     *  ingress engines.
     */
    CPSS_DXCH_TUNNEL_START_QOS_MARK_MODE_ENT upMarkMode;

    /** @brief The tag 802.1p user priority (APPLICABLE RANGES: 0..7)
     *  Relevant only if entry <tagEnable> is set and entry
     *  <upMarkMode> is set according to entry <802.1p UP>.
     */
    GT_U32 up;

    /** @brief If set, the packet is sent with VLAN tag. */
    GT_BOOL tagEnable;

    /** @brief Relevant only if entry <tagEnable> is set. This
     *  field is the VID in the Tunnel VLAN tag (APPLICABLE RANGES: 0..4095)
     */
    GT_U16 vlanId;

    /** @brief Defines generic tunnel start type. */
    CPSS_DXCH_TUNNEL_START_GENERIC_TYPE_ENT genericType;

    /** @brief Relevant for Ethernet passenger packet.
     *  GT_TRUE: retain passenger packet CRC and add
     *  additional 4-byte CRC based on the tunneling header
     *  packet.
     *  GT_FALSE: remove passenger packet CRC.
     */
    GT_BOOL retainCrc;

    /** @brief Tunnel header MAC DA. */
    GT_ETHERADDR macDa;

    /** @brief Generic tunnel start header ether type. */
    GT_U32 etherType;

    /** @brief Bytes that are added after L2 header.
     *  Relevant for <Generic TS Type>=Medium or Long.
     *  If <Generic TS Type>=Medium only 12 first bytes are relevant.
    */
    GT_U8 data[36];
}
CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC;

/* @enum CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_MODE_ENT
 *
 * @brief Enumeration of tunnel L2 Header VLAN Tag {UP, DEI} field assignment mode.
*/
typedef enum{

    /* Mark is based on Ingress_assignment;
     * {UP, DEI} is determined by the ingress pipe assignment */
    CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_INGRESS_ASSIGNMENT_E = 0,

    /* Mark is based on tunnel_based;
     * The {UP, DEI} is taken from the TS entry */
    CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_TUNNEL_BASED_E = 1,

    /* Mark is based on circuit_based;
     * The {UP, DEI} is as determined for the Circuit */
    CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_CIRCUIT_BASED_E = 2

} CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_MODE_ENT;

/* @enum CPSS_DXCH_TUNNEL_START_MPLS_EXP_MAPPING_MODE_ENT
 *
 * @brief Specifies how the label’s EXP is mapped from <TC, DP>
*/
typedef enum{

    /* Global; The mapping table (Egress Global EXP Mapping table (8 rows))
     * accessed by <QoS Mapping table index> is used to map
     * packet’s <TC, DP> to EXP; */
    CPSS_DXCH_TUNNEL_START_MPLS_EXP_MAPPING_GLOBAL_E = 0,

    /* Interface; The target circuit mapping (Egress Circuit-Based
     * EXP Mapping table (72 rows)) table is used to map
     * packet’s <TC, DP> to EXP */
    CPSS_DXCH_TUNNEL_START_MPLS_EXP_MAPPING_INTERFACE_E = 1,

    /* Custom; The EXP is not mapped from <TC, DP>
     * and the entry <EXP> is used; */
    CPSS_DXCH_TUNNEL_START_MPLS_EXP_MAPPING_CUSTOM_E = 2

} CPSS_DXCH_TUNNEL_START_MPLS_EXP_MAPPING_MODE_ENT;

/**
* @struct CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STC
 *
 * @brief Struct for MPLS Label Info
*/
typedef struct{

    /** @brief Specifies how the label’s EXP is mapped from <TC, DP> */
    CPSS_DXCH_TUNNEL_START_MPLS_EXP_MAPPING_MODE_ENT expMappingMode;

    /** @brief Label Value */
    GT_U32 label;

    /** @brief This entry EXP. Used to set the label EXP when for
     * this label the QoS mode is pipe mode (See <Last QoS Uniform label>)
     * and <QoS Pipe Mode>= Custom */
    GT_U32 exp;

    /** @brief This entry TTL. Used when for this label the
     * TTL mode is pipe mode. See <Last TTL Uniform label>*/
    GT_U32 ttl;

    /** @brief If enabled <ELI, EL> labels are pushed
     * before pushing this label */
    GT_BOOL isElOrEliInsertion;

} CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STC;

#define CPSS_DXCH_TUNNEL_START_VPORT_MPLS_LABELS_CNS 15
/**
* @struct CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STACK_STC
 *
 * @brief Struct for MPLS Label Stack Configuration
*/
typedef struct{


    /** @brief The last label that is subject to the QoS uniform model.
     * 0 - Indicates that no label is subject to the uniform mode.
     * x - Indicates that the labels in the range [1:x] are subject to the QoS uniform model
     * Labels [x+1 : Number of labels] are subject to the pipe model.
     * The uniform model for those labels can be implemented by configuring
     * adjacent labels with the same QoS attributes*/
    GT_U32 lastQosUniformLabel;

    /** @brief Pointer to Egress Global EXP mapping table (8 rows) */
    GT_U32 globaExplMappingTableIndex;

    /** @brief The last label that is subject to the TTL uniform model.
     * 0 - Indicates that no label
     * x - Indicates that the labels in the range [1:x] are subject to the TTL uniform model
     * Labels [x+1 : Number of labels] are subject to the pipe model.
     * The uniform model for those labels can be implemented by configuring
     * adjacent labels with the same TTL value*/
    GT_U32 lastTtlUniformLabel;

    /** @brief Control word index:
     * 0 - Do not add MPSL control word
     * 1-15 - Insert Control word (indexed by this field) to the MPLS tunnel */
    GT_U32 cwIndex;

    /** @brief Flow Label Insertion */
    GT_U32 flowLabelInsertion;

    /** @brief Defines the number of labels in the MPLS tunnel.
     * Used for building the Stack of Labels.
     * Length 0 is a valid value */
    GT_U32 numOfLabels;

    /** @brief MPLS Labels Info. Max of 15 label entries */
    CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STC mplsLabels[CPSS_DXCH_TUNNEL_START_VPORT_MPLS_LABELS_CNS];

} CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STACK_STC;

/**
* @struct CPSS_DXCH_TUNNEL_START_VPORT_MPLS_CONFIG_STC
 *
 * @brief Struct of configuration for X-over-MPLS tunnel start on VPort
*/
typedef struct{

    /** @brief Relevant if VLAN Tag0 is added to the tunnel header*/
    CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_MODE_ENT tag0QosMarkMode;

    /** @brief Relevant if VLAN Tag1 is added to the tunnel header*/
    CPSS_DXCH_TUNNEL_START_VLAN_QOS_MARK_MODE_ENT tag1QosMarkMode;

    /** @brief VLAN Tag0 UP field.
     * Relevant if <Tag0 QoS Marking Mode>==Tunnel-based */
    GT_U32 up0;

    /** @brief VLAN Tag0 DEI field.
     * Relevant if <Tag0 QoS Marking Mode>==Tunnel-based */
    GT_U32 dei0;

    /** @brief VLAN Tag1 UP field.
     * Relevant if <Tag0 QoS Marking Mode>==Tunnel-based */
    GT_U32 up1;

    /** @brief VLAN Tag1 DEI field.
     * Relevant if <Tag0 QoS Marking Mode>==Tunnel-based */
    GT_U32 dei1;

    /** @brief Specifies the egress thread ID associated with this entry.
     * If no thread is associated with this profile the Thread ID should be 0 */
    GT_U32 threadId;

    /** @brief A non-zero value means that mirroring to the analyzer is enabled,
     * and that the analyzer index is specified in this field */
    GT_U32 analyzerIndex;

    /** @brief Egress Mirroring modes */

    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirrorMode;
    /** @brief If set the MPLS labels are added as is without TTL and EXP modifications.
     * It can be used as Generic TS */
    GT_U32 rawLabels;

    /** @brief Selects which of the 8 global MPLS EtherTypes to use in the outgoing packet */
    GT_U32 mplsEthertypeSelect;

    /** @brief Pointer to CNC counters. CNC counting is triggered if the pointer value is not 0. */
    GT_U32 cncPtr;

    /** @brief The Profile ID of assigning <Flow-ID/GCF bits> field into
     * Flow-ID and Generic Classification Field */
    GT_U32 flowOrGcfProfileId;

    /** @brief The Profile ID of assigning <Flow-ID/GCF Profile ID> field into
     * Flow-ID and Generic Classification Field */
    GT_U32 flowOrGcfBits;

    /** @brief MPLS Label Stack Configuration */
    CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STACK_STC mplsLabelStackCfg;

} CPSS_DXCH_TUNNEL_START_VPORT_MPLS_CONFIG_STC;

/**
* @union CPSS_DXCH_TUNNEL_START_CONFIG_UNT
 *
 * @brief Union for configuration for tunnel start
 *
*/
typedef union{
    /** configuration for generic ipv4 tunnel start. */
    CPSS_DXCH_TUNNEL_START_IPV4_CONFIG_STC ipv4Cfg;

    /** @brief configuration for generic ipv6 tunnel start.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman).
     */
    CPSS_DXCH_TUNNEL_START_IPV6_CONFIG_STC ipv6Cfg;

    /** configuration for X-over-MPLS tunnel start. */
    CPSS_DXCH_TUNNEL_START_MPLS_CONFIG_STC mplsCfg;

    /** configuration for MIM tunnel start. */
    CPSS_DXCH_TUNNEL_START_MIM_CONFIG_STC mimCfg;

    /** generic tunnel start configuration */
    CPSS_DXCH_TUNNEL_START_GENERIC_CONFIG_STC genCfg;

    /** configuration for X-over-MPLS tunnel start on VPort */
    CPSS_DXCH_TUNNEL_START_VPORT_MPLS_CONFIG_STC mplsVportCfg;

} CPSS_DXCH_TUNNEL_START_CONFIG_UNT;

/**
* @struct CPSS_DXCH_HFT_COMMON_CONFIG_STC
 *
 * @brief Struct of configuration for Header Field Table MPLS Label Entry
*/
typedef struct{

    /** @brief Entry Subtype. Indicates if the entry is for L2VPN or L3VPN */
    GT_U32 subType;

    /** @brief A non-zero value means that mirroring to the analyzer is enabled,
     * and that the analyzer index is specified in this field. */
    GT_U32 analyzerIndex;

    /** @brief Egress Mirroring modes */
    CPSS_DXCH_MIRROR_EGRESS_MODE_ENT egressMirrorMode;

    /** @brief Added to TS Thread ID to determine the PHA egress thread id */
    GT_U32 subThreadId;

    /** @brief CNC index. Used when the HFT is triggered and it is selected as CNC client,
     * CNC counting and the ptr value is not 0. */
    GT_U32 cncPtr;

    /** @brief The Profile ID of assigning GFC bits field into
     * the Generic Classification Field */
    GT_U32 gcfProfileId;

    /** @brief Bits assigned Generic Classification Field according
     * to <GCF profile id> */
    GT_U32 gcfBits0To4;

} CPSS_DXCH_HFT_COMMON_CONFIG_STC;

/**
* @struct CPSS_DXCH_HFT_MPLS_LABEL_CONFIG_STC
 *
 * @brief Struct of configuration for Header Field Table MPLS Label Entry
*/
typedef struct{

    /** @brief HFT Common Configuration */
    CPSS_DXCH_HFT_COMMON_CONFIG_STC hftCommonCfg;

    /** @brief MPLS Label Stack Configuration */
    CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STACK_STC mplsLabelStackCfg;

} CPSS_DXCH_HFT_MPLS_LABEL_CONFIG_STC;

/**
* @struct CPSS_DXCH_HFT_MPLS_LABEL_EXTENDED_CONFIG_STC
 *
 * @brief Struct of configuration for Header Field Table MPLS Label Entry
*/
typedef struct{

    /** @brief HFT Common Configuration */
    CPSS_DXCH_HFT_COMMON_CONFIG_STC hftCommonCfg;

    /** @brief Bits assigned Generic Classification Field according
     * to <GCF profile id> */
    GT_U32 gcfBits5To47[2];

    /** @brief Flow-ID */
    GT_U32 flowId;

    /** @brief MPLS Label Stack Configuration */
    CPSS_DXCH_TUNNEL_START_MPLS_LABEL_STACK_STC mplsLabelStackCfg;

} CPSS_DXCH_HFT_MPLS_LABEL_EXTENDED_CONFIG_STC;

/* @enum CPSS_DXCH_HFT_TABLE_ENT
 *
 * @brief Header Fields Table(HFT) ID
*/
typedef enum{

    /* Table 0 */
    CPSS_DXCH_HFT_TABLE_0_E = 0,

    /* Table 1 */
    CPSS_DXCH_HFT_TABLE_1_E = 1

} CPSS_DXCH_HFT_TABLE_ENT;

/* @enum CPSS_DXCH_HFT_ENTRY_TYPE_ENT
 *
 * @brief Header Fields Table(HFT) Entry Types
*/
typedef enum{

    /* Generic Table */
    CPSS_DXCH_HFT_GENERIC_E = 0,

    /* Generic Extended Table */
    CPSS_DXCH_HFT_GENERIC_EXT_E = 1,

    /* MPLS Table */
    CPSS_DXCH_HFT_MPLS_E = 2,

    /* MPLS Extended Table */
    CPSS_DXCH_HFT_MPLS_EXT_E = 3

} CPSS_DXCH_HFT_ENTRY_TYPE_ENT;

/**
* @union CPSS_DXCH_HFT_CONFIG_UNT
 *
 * @brief Union for configuration of Header Fields Table
 *
*/
typedef union{
    /** @brief for MPLS Label Entry */
    CPSS_DXCH_HFT_MPLS_LABEL_CONFIG_STC mplsLabelCfg;

    /** @brief configuration for MPLS Label Extended Entry */
    CPSS_DXCH_HFT_MPLS_LABEL_EXTENDED_CONFIG_STC mplsLabelExtCfg;

} CPSS_DXCH_HFT_CONFIG_UNT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTunnelTypesh */

