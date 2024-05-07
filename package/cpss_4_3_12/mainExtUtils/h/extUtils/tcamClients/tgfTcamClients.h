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
* @file tgfTcamClients.h
*
* @brief TCAM Clients utils
*
* @version   1
********************************************************************************
*/

#ifndef __tgfTcamClients_h
#define __tgfTcamClients_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>

/* number of entries for Application registered fields in DB */
#define TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_MAX_AMOUNT_CNS 64

/* amount of UDBs supported by HW */
#define TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS 60

/* amount of UDBs supported by HW */
#define TGF_TCAM_CLIENTS_PCL_MAX_UDB_CFG_OFFSET_CNS 255

/* amount of supported UDB Packet type sets */
#define TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS 4

/* maximal amount of default UDB confugurations */
#define TGF_TCAM_CLIENTS_PCL_MAX_UDB_DEFAULT_CONFIG_CNS 4

/* maximal amount mux table entries in default key header */
#define TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS 4

/* bitmap of UDB indexes */
typedef GT_U32 TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR[
    (TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS + 31) / 32];

/* bitmap of UDB offsets */
typedef GT_U32 TGF_TCAM_CLIENTS_PCL_UDB_OFFSETS_BMP_ARR[
    (TGF_TCAM_CLIENTS_PCL_MAX_UDB_CFG_OFFSET_CNS + 31) / 32];

/* bitmap of UDB Packet types */
typedef GT_U32 TGF_TCAM_CLIENTS_PCL_UDB_PKT_TYPES_BMP;

/**
* @enum TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ENT
 *
 * @brief Defines fields used in IPCL RULES.
*/
typedef enum
{

    /** @brief UDB calculated in TTI by configuration field
     *  The indexes of such UDB assgned by this library - not by Application
     *  These UDBs Anchor and offset configured by this library.
     *  An Application requires using of such UDBs using well known and registred
     *  feilds needing by them in sub-keys.
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E,

    /** @brief UDB configured by Apllication field
     *  Such as UDBs updated by PCL action
     *  Or if UDB positions in super-key contain bitmap of UDB49 values
     *  instead ov UDB values,
     *  These UDBs or UDB position replacements not configured by this library.
     *  An Application requires using of such UDBs positions of super-key
     *  using well known and registred feilds needing by them in sub-keys.
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_APP_E,

    /** @brief Action Modified Data field - selected or not in superkey
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ACTION_MODIFIED_E,

    /** @brief Fixed Data field in superkey IPCL and EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_FIXED_E,

    /** @brief Fixed Data field in superkey MPCL, additional to IPCL list of fields
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MPCL_LIST_OF_FIELDS_E,

    /** @brief max value of this enum
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MAX_E,

} TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ENT;

/**
* @enum TGF_TCAM_CLIENTS_PCL_SUPER_KEY_BASE_ENT
 *
 * @brief Base inside IPCL/MPCL/EPCL Superkey.
*/
typedef enum
{
    /** @brief UDB0-59 part of IPCL/EPCL
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_UDB_E,

    /** @brief UDB10-59 part of MPCL
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_UDB_E,

    /** @brief MPCL List of Additiomal Fields
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_LIST_OF_FIELDS_E,

    /** @brief IPCL Action modified selected fields
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_AMF_SEL_E,

    /** @brief MPCL Action modified selected fields
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_AMF_SEL_E,

    /** @brief IPCL and MPCL common fixed fields
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_FIXED_E,

    /** @brief EPCL fixed fields
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_EPCL_FIXED_E,

    /** @brief MPCL Additiomal fields
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_FIXED_E,

    /** @brief Number of superkey bases
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_NUM_OF_TYPES_E

} TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT;


/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC
 *
 * @brief Info for fields used in PCL RULES.
*/
typedef struct
{
    /** @brief field type
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ENT fieldType;

    /** @brief UDB Anchor type
     *  Relevant for UDB_CFG fields only
     */
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT udbAnchor;

    /** @brief offset in bits from type-dependend origin
     * for UDB_CFG fields - from origin pointed by UDB anchor (in bits)
     * for UDB_APP fields - the index of UDB 0-59 (in bytes)
     * for Action Modified Data - from the Data origin (before selection to superkey) (in bits)
     * for Fixed - from Superkey origin (in bits)
     */
    GT_U16 bitOffset;

    /** @brief field lengh
     *  for UDB_CFG fields in bits
     *  for UDB_APP fields length in bytes
     *  for Action Modified Data  in bits
     *  for Fixed  in bits
     */
    GT_U16 bitLength;

} TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC;


/**
* @enum TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT
 *
 * @brief Defines fields used in IPCL RULES.
*/
typedef enum
{

    /** @brief Stamp value for end of list/array
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E = 0,

    /* ===== UDB Metadata fields _IMETA_ IPCL ===== */

    /** @brief Base of UDB related field Ids
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_UDB_IDS_BASE_E,

    /** @brief Applicable Flow Sub-template
     *  1 = IPv4_TCP
     *  2 = IPv4_UDP
     *  4 = MPLS
     *  8 = IPv4_FRAGMENT
     *  16 = IPv4_OTHER
     *  32 = ETHERNET_OTHER
     *  64 = IPv6
     *  128 = UDE0-4
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_PKT_TYPE_BMP_E =
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_UDB_IDS_BASE_E,

    /** @brief Is IP
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_IP_E,

    /** @brief Is IPV4
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_IPV4_E,

    /** @brief Is IPV6
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_IPV6_E,

    /** @brief Is ARP
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_ARP_E,

    /** @brief Is MPLS
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_MPLS_E,

    /** @brief Is L3 Invalid
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_L3_INVALID_E,

    /** @brief Is L4 Valid
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_L4_VALID_E,

    /** @brief Ingress Tag0 exists
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_EXISTS_E,

    /** @brief Egress Tag0 exists
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_TAG0_EXISTS_E,

    /** @brief Ingress Tag1 exists
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG1_EXISTS_E,

    /** @brief Egress Tag1 exists
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_TAG1_EXISTS_E,

    /** @brief Ingress IPV4 or IPV6 protocol
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IP_PROTOCOL_E,

    /** @brief Egress IPV4 or IPV6 protocol
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IP_PROTOCOL_E,

    /** @brief Source Physical Port
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_PHY_PORT_E,

    /** @brief Ingress Local Source ePort
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_SRC_EPORT_E,

    /** @brief Egress Local Source ePort
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_SRC_PHY_PORT_E,

    /** @brief Ingress QoS Profile
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_QOS_PROFILE_E,

    /** @brief Egress QoS Profile
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_QOS_PROFILE_E,

    /** @brief Ingress VID0
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_VID0_E,

    /** @brief Egress VID0
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_VID0_E,

    /** @brief Ingress VID1
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_VID1_E,

    /** @brief Egress VID1
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_VID1_E,

    /** @brief Ingress UP0
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_UP0_E,

    /** @brief Egress UP0
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_UP0_E,

    /** @brief Ingress UP1
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_UP1_E,

    /** @brief Egress UP1
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_UP1_E,

    /** @brief Ingress DSCP
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_DSCP_E,

    /** @brief Ingress Tag0 TPID Index
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG0_TPID_INDEX_E,

    /** @brief Ingress Tag1 TPID Index
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_TAG1_TPID_INDEX_E,

    /** @brief Ingress Is Ptp
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IMETA_IS_PTP_E,

    /** @brief Egress Is Ptp
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_IS_PTP_E,

    /** @brief Egress Packet Command
     *  0x0 = TO_CPU
     *  0x1 = FROM_CPU
     *  0x2 = TO_ANALYZER
     *  0x3 = FORWARD
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EMETA_EGR_PKT_CMD_E,

    /* ===== UDB Packet parts fields _L2_, _L3_, _L4_, IPCL, MPCL, EPCL ===== */

    /** @brief Destination MAC Address
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_DST_MAC_E,

    /** @brief Source MAC Address
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L2_SRC_MAC_E,

    /** @brief Ethernet Type
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_ETHERTYPE_E,

    /** @brief IPV4 DSCP
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV4_DSCP_E,

    /** @brief IPV6 DSCP
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L3_IPV6_DSCP_E,

    /** @brief TCP/UDP Source Port
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L4_TCP_SRC_PORT_E,

    /** @brief TCP/UDP Destination Port
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L4_TCP_DST_PORT_E,

    /** @brief Last of UDB related field Ids
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_UDB_IDS_LAST_E =
        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_L4_TCP_DST_PORT_E,


    /* ====== IPCL/MPCL Action Modified Fields _AMF_ ===== */

    /** @brief Action Modified Field eVlan
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_EVLAN_E,

    /** @brief Action Modified Field Source EPG
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_SRC_EPG_E,

    /** @brief Action Modified Field Source Is VPORT
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_SRC_IS_VPORT_E,

    /** @brief Action Modified Field Source VPORT extension
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_SRC_VPORT_EXT_E,

    /** @brief Action Modified Field Destination EPG
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_DST_EPG_E,

    /** @brief Action Modified Field QoS Profile
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_QOS_PROFILE_E,

    /** @brief Action Modified Field Source ePort (15 bits or 16 LSB of VPORT)
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_SRC_EPORT_E,

    /** @brief Action Modified Field Tag1 VLAN Id
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TAG1_VID_E,

    /** @brief Action Modified Field Tag1 User priority
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TAG1_UP_E,

    /** @brief Action Modified Field Tag1 CFI
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TAG1_CFI_E,

    /** @brief Action Modified Field VRF Id
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_VRF_ID_E,

    /** @brief Action Modified Field Source Id
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_SRC_ID_E,

    /** @brief Action Modified Field Hash bytes
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_HASH_BYTES_E,

    /** @brief Action Modified Field Generic Classification
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_GEN_CLASS_E,

    /** @brief Action Modified Field Packet Command
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_PACKET_CMD_E,

    /** @brief Action Modified Field CPU Code
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_CPU_CODE_E,

    /** @brief Action Modified Field Target Device
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_DEV_E,

    /** @brief Action Modified Field Target is VIDX
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_IS_VIDX_E,

    /** @brief Action Modified Field Target is Trunk
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_IS_TRUNK_E,

    /** @brief Action Modified Field Target is VPort
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_IS_VPORT_E,

    /** @brief Action Modified Field Target VPORT Extention
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_VPORT_EXT_E,

    /** @brief Action Modified Field 16-bit muxed Target:
     *  eVidx (16 bit), ePort (15 bit), TrunkId (12 bit)
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_TRG_PORT_E,

    /** @brief Action Modified Field Is Tunnel Start
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_IS_TS_E,

    /** @brief Action Modified Field ARP Or Tunnel Start Pointer
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_ARP_OR_TS_PTR_E,

    /** @brief Action Modified Field EM Profile Id1
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_EM_PRPFILE_ID1_E,

    /** @brief Action Modified Field EM Profile Id2
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_EM_PRPFILE_ID2_E,

    /** @brief Action Modified Field Policer Index
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_POLICER_PTR_E,

    /** @brief Action Modified Field Billing Enable
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_BILLING_ENABLE_E,

    /** @brief Action Modified Field Metering Enable
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_METERING_ENABLE_E,

    /** @brief Action Modified Field Copy-reserved
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_COPY_RESERVED_E,

    /** @brief Action Modified Field Flow Id
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_FLOW_ID_E,

    /** @brief Action Modified Field PCL Id2
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_AMF_PCL_ID2_E,

    /* ===== Common PCL Fixed Fields _FIX_, Ingress _IFIX_, Egress _EFIX_ ======= */

    /** @brief PCL ID - 12 bit, IPCL/MPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_PCL_ID_E,

    /** @brief UDB Valid - 1 bit, IPCL/MPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_UDB_VALID_E,

    /** @brief Mac To Me - 1 bit, IPCL/MPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC2ME_E,

    /** @brief Source Device Is Own - 1 bit, IPCL/MPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_SRC_DEV_IS_OWN_E,

    /** @brief Destination MAC Address - 48 bit, IPCL/MPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_IFIX_MAC_DA_E,

    /** @brief IPV4 or ARP Destination IP Address - 32 bit, IPCL/MPCL/EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV4_DIP_E,

    /** @brief IPV6 Destination IP Address - 128 bit, IPCL/MPCL/EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV6_DIP_E,

    /** @brief IPV4 or ARP Source IP Address - 32 bit, IPCL/MPCL/EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV4_SIP_E,

    /** @brief IPV6 Source IP Address - 128 bit, IPCL/MPCL/EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_FIX_IPV6_SIP_E,

    /** @brief PCL ID - 10 bit, EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EFIX_PCL_ID_E,

    /** @brief UDB Valid - 1 bit, EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EFIX_UDB_VALID_E,

    /** @brief Source EPG - 12 bit, EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EFIX_SRC_EPG_E,

    /** @brief Destination EPG -  bit, EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EFIX_DST_EPG_E,

    /** @brief  - EM Profile - 8 bit, EPCL
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_EFIX_EM_PROFILE_E,

    /* The MPCL only Fixed Field List will be added _MFIX_ */

    /** @brief Application Registred Field Ids base
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E,
    /** @brief Amout of this Enum Elements
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD___LAST___E =
        (TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E
         + TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_MAX_AMOUNT_CNS)

} TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT;

/**
* @enum TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT
 *
 * @brief Defines IPCL/MPCL/EPCL lookups and for EPCL profiles.
*/
typedef enum
{
    /** @brief IPCL0
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E,

    /** @brief IPCL1
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E,

    /** @brief IPCL2
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E,

    /** @brief MPCL
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,

    /** @brief Number of Ingress (IPCL and MPCL) lookups
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E,

    /** @brief Egress PCL lookup
     *  Used only for TCAM profiles used by EPCL
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E =
        TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E,

    /** @brief Total Number of lookups
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E

} TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT;

/* ===== Subkey default header ====== */

/**
* @struct TGF_TCAM_CLIENTS_UDB_CFG_STC
 *
 *  @brief Configuration of one UDB
*/
typedef struct
{
    /** @brief UDB index
     */
    GT_U8 udbIndex;

    /** @brief UDB anchor
     */
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT udbAnchor;

    /** @brief UDB offset (in bytes)
     */
    GT_U8 udbOffset;

} TGF_TCAM_CLIENTS_UDB_CFG_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_SUBKEY_COMMON_HEADER_STC
 *
 *  @brief Common header of all subkeys
*/
typedef struct
{

    /** @brief Number of Common Ingress UDB configurations
     */
    GT_U32 numOfIngressUdbCfg;

    /** @brief Common Ingress UDB configurations for all packet types
     */
    TGF_TCAM_CLIENTS_UDB_CFG_STC ingressHeaderUdbConfigArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_DEFAULT_CONFIG_CNS];

    /** @brief Number of default Egress UDB configurations
     */
    GT_U32 numOfEgressUdbCfg;

    /** @brief Common Egress UDB configurations for all packet types
     */
    TGF_TCAM_CLIENTS_UDB_CFG_STC egressHeaderUdbConfigArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_DEFAULT_CONFIG_CNS];

    /** @brief Number of Mux entries for IPCL Common Key Header
     */
    GT_U32 ipclNumOfMuxEntries;

    /** @brief Superkey bases for IPCL Common Key Header.
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT ipclMuxSuperkeyBasesArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

    /** @brief Relative Superkey offsets in byte-pair resolution for IPCL Common Key Header
     */
    GT_U32 ipclMuxSuperkeyOffsetsArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

    /** @brief Number of Mux entries for MPCL Common Key Header
     */
    GT_U32 mpclNumOfMuxEntries;

    /** @brief Superkey bases for Common MPCL Key Header.
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT mpclMuxSuperkeyBasesArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

    /** @brief Relative Superkey offsets in byte-pair resolution for MPCL Common Key Header
     */
    GT_U32 mpclMuxSuperkeyOffsetsArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

    /** @brief Number of Mux entries for EPCL Common Key Header
     */
    GT_U32 epclNumOfMuxEntries;

    /** @brief Superkey bases for Common EPCL Key Header.
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT epclMuxSuperkeyBasesArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

    /** @brief Relative Superkey offsets in byte-pair resolution for EPCL Common Key Header
     */
    GT_U32 epclMuxSuperkeyOffsetsArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

} TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_SUBKEY_COMMON_HEADER_STC;

/* ===== Ingress and Egress UDB Configuration ====== */

/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC
 *
 *  @brief Subfield definition, fields greater or equal than 256 bits = 32 bytes not supported
*/
typedef struct
{
    /** @brief Set of UDB L2 fields or subfields spaces in 1 byte resolution
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT fieldId;

    /** @brief Offset of subfield inside field in bits
     */
    GT_U8 bitOffset;

    /** @brief Length of subfield in bits
     *  0 - means up to the end of the field
     */
    GT_U8 bitLength;

} TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC;

/**
 * @struct TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC
 *
 *  @brief UDB Sub-fields used in ingress lookups
 */
typedef struct
{
    /** @brief sub-fields data
     */
    TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC subField;

    /** @brief bitmap of indexes of UDB Packet Type groups using this sub-field
     */
    GT_U8 udbPktTypeGroupsBmpArr[
        TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E];

} TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC
 *
 *  @brief UDB Sub-fields used in egress lookup
*/
typedef struct
{
    /** @brief sub-fields data
     */
    TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC subField;

    /** @brief bitmap of indexes of UDB Packet Type groups using this sub-field
     */
    GT_U8 udbPktTypeGroupsBmp;

} TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_UDB_INGRESS_CFG_ALL_FIELD_SET_STC
 *
 *  @brief Set of Ingress UDB CFG fields for all packet types and all lookups
*/
typedef struct
{
    /** @brief Bitmap of TTI configured UDBs Available for this lookup
     *  The UDBs assigned by PCL action or by other features and UDBs replaced
     *  by other data in the super-key of this lookup and
     *  (For MPCL UDB0-9 not present in super-key) must be excluded.
     *  1 - UDB avaiailable, 0 - UDB excluded
     *  These bitmaps should not contain UDBs used in default Subkey header
     */
    TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR udbIndexesBmpArr[
        TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E];


    /** @brief Pointer to array of UDB_CFG sub-field use enrtries
     *  with _STAMP_LIST_END_ at the end
     */
     const TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC *ingressSubFieldsUseArrPtr;

} TGF_TCAM_CLIENTS_PCL_RULE_UDB_INGRESS_CFG_ALL_FIELD_SET_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_UDB_EGRESS_CFG_ALL_FIELD_SET_STC
 *
 *  @brief Set of UDB CFG fields for all packet types for EPCL
*/
typedef struct
{
    /** @brief Bitmap of HA configured UDBs Available for EPCL
     *  The UDBs assigned by PCL action or by other features and UDBs replaced
     *  by other data in the super-key of EPCL.
     *  1 - UDB avaiailable, 0 - UDB excluded
     *  These bitmaps should not contain UDBs used in default EPCL Subkey header
     */
    TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR udbIndexesBmp;

    /** @brief Pointer to array of UDB_CFG sub-field use enrtries
     *  with _STAMP_LIST_END_ at the end
     */
     const TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC *egressSubFieldsUseArrPtr;

} TGF_TCAM_CLIENTS_PCL_RULE_UDB_EGRESS_CFG_ALL_FIELD_SET_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_ACTION_MODIFIED_SUB_FIELD_USE_STC
 *
 *  @brief Action Modified Sub-fields used in IPCL lookups
*/
typedef struct
{
    /** @brief sub-fields data
     */
    TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC subField;

    /** @brief bitmap of indexes IPCL lookups using this sub-field
     */
    GT_U8 ipclLookupsBmp;

} TGF_TCAM_CLIENTS_PCL_RULE_ACTION_MODIFIED_SUB_FIELD_USE_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_SUPERKEY_ALL_FIELD_SET_STC
 *
*   @brief Sets of UDB CFG fields for all packet types and all lookups
*          and Action modified fields for all lookups
*/
typedef struct
{
    /** @brief UDB_CFG field sets for all packet-type-sets and lookups
     */
    TGF_TCAM_CLIENTS_PCL_RULE_UDB_INGRESS_CFG_ALL_FIELD_SET_STC udbCfgAllFields;

    /** @brief Pointer to Action Modified field use array
    *  with _STAMP_LIST_END_ at the end
     */
    const TGF_TCAM_CLIENTS_PCL_RULE_ACTION_MODIFIED_SUB_FIELD_USE_STC *actionModifiedSubfieldsArrPtr;

} TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_SUPERKEY_ALL_FIELD_SET_STC;

/* ====== Mapping Pair of <UDB-Packet-type-group, PCL-CFG-ProfileId> to TCAM profile ====== */

/**
* @struct TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC
 *
 *  @brief Maps of Pcl Configurationm Profile Id and packet types to TCAM Profile Id
*/
typedef struct
{
    /** @brief PCL Configuration Table Entry Profile Id value
     *  (APPLICABLE RANGE 0..15)
     *  0xFF for END-OF-LIST stamp
     */
    GT_U8 pclCfgTableProfileId;

    /** @brief Array of TCAM Profile Ids for each set of UDB packet types
     *  The sets of UDB Packet types defined in RULE_UDB_CFG_ALL_FIELD_SET structure
     */
    GT_U8 tcamProfileIdArr[TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS];

} TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_STC
 *
 *  @brief Maps of All Pcl Configurationm Profile Id and packet types to All TCAM Profile Ids
*/
typedef struct
{
    /** @brief Pointer to array of mapping IPCL Config Profile Id to TCAM Profile ids
     *  pclCfgTableProfileId == 0xFFFFFFFF for END-OF-LIST stamp
     */
    const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  *ipclCfgToTcamProfileIdMapArrPtr;

    /** @brief Pointer to array of mapping MPCL Config Profile Id to TCAM Profile ids
     *  pclCfgTableProfileId == 0xFFFFFFFF for END-OF-LIST stamp
     */
    const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  *mpclCfgToTcamProfileIdMapArrPtr;

    /** @brief Pointer to array of mapping EPCL Config Profile Id to TCAM Profile ids
     *  pclCfgTableProfileId == 0xFFFFFFFF for END-OF-LIST stamp
     */
    const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC  *epclCfgToTcamProfileIdMapArrPtr;

} TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_STC;

/* ===== TCAM Profles contents ====== */

/**
* @struct TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC
 *  @brief TCAM Profiles Configuration
*/
typedef struct
{
    /** @brief TCAM ProfileId.
     *  tcamProfileId == 0xFFFFFFFF for END-OF-LIST stamp
     */
    GT_U32 tcamProfileId;

    /** @brief PCL lookup - to determinate TCAM Client Group
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT pclLookup;

    /** @brief Configuration of TCAM Profile
     *  Referres Mux Table Lines and contains sub Key sizes
     *  The muxing table configured using data below
     */
    CPSS_DXCH_TCAM_CLIENT_GROUP_LOOKUP_PROFILE_CFG_STC tcamProfileCfg;

} TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_FIELD_ENTRY_CFG_STC
 *
 *  @brief field in Subkeys entry
*/
typedef struct
{
    /** @brief Sub-field structure, the _STAMP_LIST_END_ used at the end of array the end
     */
    TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC subField;

    /** @brief Bitmap of word indexes of Muxing Table entry
     *  that should contain the given subfield.
     *  Mux table line is 6 words containing each 5 byte-pair places
     */
    GT_U8 muxWordIndexesBmp;

} TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC
 *  @brief TCAM Muxing table Line Configuration
*/
typedef struct
{
    /** @brief Mux Table Line Index - 0xFF - END-OF-LIST entry
     */
    GT_U8 subKeyMuxTableLineIndex;

    /** @brief Index Packet Type Group - to back map fields to UDBs.
     */
    GT_U8 pktTypeGroupIndex;

    /** @brief PCL lookup - to determinate TCAM Client Group and to find
     *  Action modified fields.
     *  For engress configuration referres to Egress UDBs instead of Ingress.
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT pclLookup;

    /** @brief Bitmap of word indexes of Muxing Table entry
     *  that should bbegin from the Common Header.
     *  Mux table line is 6 words containing each 5 byte-pair places
     */
    GT_U32 muxWordWithCommonHeaderIndexesBmp;

    /** @brief Pointer to array of subfields used by profile
     */
    const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC *muxedSubfieldEntryArrPtr;

} TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC
 *
 *  @brief Set of UDB CFG fields for all packet types and all lookups
*/
typedef struct
{
    /** @brief IPCL/MPCL/EPCL Subkey default headers
     */
    const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_SUBKEY_COMMON_HEADER_STC *subkeyCommonHeaderPtr;

    /** @brief Amount of UDB Packet Type Groups
     */
    GT_U32 numOfPktTypeGroups;

    /** @brief Array UDB Packet types bitmap
     */
    TGF_TCAM_CLIENTS_PCL_UDB_PKT_TYPES_BMP udbPktTypesBmpArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS];

    /** @brief Ingress Super Key Fields Set
     */
    TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_SUPERKEY_ALL_FIELD_SET_STC ingressSuperKeyFieldsSet;

    /** @brief Egress Super Key Fields Set
     */
    TGF_TCAM_CLIENTS_PCL_RULE_UDB_EGRESS_CFG_ALL_FIELD_SET_STC egressSuperKeyFieldsSet;

    /** @brief mapping Pcl Config Profile Id to TCAM Profile ids
     */
    TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_STC  cfgToTcamProfileIdMap;

    /** @brief Pointer to array of TCAM Profile Subkey Configurations
     */
    const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC *profileSubkeyCfgArrPtr;

    /** @brief Pointer to array of TCAM Muxing Table Lines Configurations
     */
    const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC *muxingTableSubfieldsCfgArrPtr;

} TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC;

/**
* @struct TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC
 *
 *   @brief sub-field condition in rule, subfields larger than 32 bits not supported
 *
 */
typedef struct
{
    /** @brief Sub-field structure, the _STAMP_LIST_END_ used at the end of array the end
     */
    TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC subField;

    /** @brief mask
     */
    GT_U32 mask;

    /** @brief pattern
     */
    GT_U32 pattern;

} TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC;

typedef void* TGF_TCAM_CLIENTS_DB_HANDLE;

/**
* @internal tgfTcamClientsLibInit function
* @endinternal
*
* @brief   Initialyze TCAM Client Library.
*
* @retval - none
*/
void tgfTcamClientsLibInit
(
    GT_VOID
);

/**
* @internal tgfTcamClientsPclFieldRegister function
* @endinternal
*
* @brief   Redister Application defined field.
*
* @param[in] fieldId           - field Id in Aplication defind range
* @param[in] fieldsInfoPtr     - (pointer to) field Info
*
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*/
GT_STATUS tgfTcamClientsPclFieldRegister
(
    IN  TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId,
    IN  const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr
);

/**
* @internal tgfTcamClientsConfigurationDbCreate function
* @endinternal
*
* @brief   Create Configuration DB by Application configuration
*
* @param[in] appCfgPtr          - (pointer to) full application configuration structure
* @param[out] dbHandlePtr     - (pointer to) DB Handler
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_CPU_MEM    - on memory allocation fail
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS tgfTcamClientsConfigurationDbCreate
(
    IN      const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr,
    OUT     TGF_TCAM_CLIENTS_DB_HANDLE                     *dbHandlePtr
);

/**
* @internal tgfTcamClientsPclFieldDefinitionGet function
* @endinternal
*
* @brief   Get Field Definition.
*
* @param[in] fieldId           - field Id in Aplication defind range
* @param[out] fieldsInfoPtr     - (pointer to) field Info
*
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
* @retval GT_BAD_STATE         - on wrong found field parameters
*/
GT_STATUS tgfTcamClientsPclFieldDefinitionGet
(
    IN  TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId,
    OUT TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr
);

/**
* @internal tgfTcamClientsConfigurationDbDelete function
* @endinternal
*
* @brief   Delete Configuration DB Configuration
*
* @param[out] dbHandle     - (pointer to) DB Handler
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PTR           - on null pointer
*
*/
GT_STATUS tgfTcamClientsConfigurationDbDelete
(
    IN      TGF_TCAM_CLIENTS_DB_HANDLE                     dbHandle
);

/**
* @internal tgfTcamClientsDeviceConfigure function
* @endinternal
*
* @brief   Configure device by Configuration DB
*
* @param[in] devNum       - device number
* @param[in] dbHandle     - (pointer to) DB Handler
*
* @retval GT_OK                    - on OK
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS tgfTcamClientsDeviceConfigure
(
    IN      GT_U8                                    devNum,
    IN      TGF_TCAM_CLIENTS_DB_HANDLE               dbHandle
);

/**
* @internal tgfTcamClientsPclCfgTblSet function
* @endinternal
*
* @brief   PCL Configuration table entry's lookup configuration for interface.
*          Functiom simplified - does not support UDB49 values bitmap
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] interfaceInfoPtr         - interface data: port, VLAN, or index for setting a
*                                       specific PCL Configuration Table entry
* @param[in] pclLookup                - PCL lookup.
* @param[in] enable                    - GT_TRUE - enable, GT_FALSE - disnable.
* @param[in] pclCfgProfileId          - PCL Configuration Table Profile Id.
* @param[in] pclId                    - PCL Configuration Table Profile Id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - otherwise
*
*
*/
GT_STATUS tgfTcamClientsPclCfgTblSet
(
    IN GT_U8                            devNum,
    IN CPSS_INTERFACE_INFO_STC          *interfaceInfoPtr,
    IN TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  pclLookup,
    IN GT_BOOL                          enable,
    IN GT_U32                           pclCfgProfileId,
    IN GT_U32                           pclId
);

/**
* @internal tgfTcamClientsPclRuleBuild function
* @endinternal
*
* @brief   The function builds the Policy Rule Mask and Pattern
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                    - device number
* @param[in] dbHandle                  - Handle of DB of global configurations
* @param[in] pclCfgProfileId           - PCL Configuration Table Profile Id (use DB).
*                                        The parameters marked (use DB) used to determinate
*                                        rule size and fields layout in the rule by internal DB.
* @param[in] udbPacketTypeGroupIndex   - Index of Group of UDB Packet Type (use DB).
* @param[in] pclLookup                 - PCL lookup (use DB).
* @param[in] tcamSubkeyIndex           - TCAM parallel sublookup Subkey Index (use DB).
* @param[in] commonKeyHeaderMaskPtr    - (Pointer to)rule Common Header Mask.
* @param[in] commonKeyHeaderPatternPtr - (Pointer to)rule Common Header Pattern.
* @param[in] subFieldConditionArrPtr   - (Pointer to) array of Subfield entries with Mask and Pattern.
*                                        subfields with bitLength > 32 not supported
* @param[out] tcamRuleSizePtr          - (Pointer to) rule size (TCAM Library enum).
* @param[out] maskPtrPtr               - (Pointer to)(Pointer to) rule mask built in work memory.
* @param[out] patternPtrPtr            - (Pointer to)(Pointer to) rule pattern built in work memory.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS tgfTcamClientsPclRuleBuild
(
    IN  GT_U8                                           devNum,
    IN  TGF_TCAM_CLIENTS_DB_HANDLE                      dbHandle,
    IN  GT_U32                                          pclCfgProfileId,
    IN  GT_U32                                          udbPacketTypeGroupIndex,
    IN  TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                 pclLookup,
    IN  GT_U32                                          tcamSubkeyIndex,
    IN  GT_U32                                          *commonKeyHeaderMaskPtr,
    IN  GT_U32                                          *commonKeyHeaderPatternPtr,
    IN  TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    *subFieldConditionArrPtr,
    OUT CPSS_DXCH_TCAM_RULE_SIZE_ENT                    *tcamRuleSizePtr,
    OUT GT_U32                                          **maskPtrPtr,
    OUT GT_U32                                          **patternPtrPtr
);

/**
* @internal tgfTcamClientsPclRuleBuildForPclRuleSet function
* @endinternal
*
* @brief   The function builds the Policy Rule Mask and Pattern in PCL Library formats
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                    - device number
* @param[in] dbHandle                  - Handle of DB of global configurations
* @param[in] pclCfgProfileId           - PCL Configuration Table Profile Id (use DB).
*                                        The parameters marked (use DB) used to determinate
*                                        rule size and fields layout in the rule by internal DB.
* @param[in] udbPacketTypeGroupIndex   - Index of Group of UDB Packet Type (use DB).
* @param[in] pclLookup                 - PCL lookup (use DB).
* @param[in] tcamSubkeyIndex           - TCAM parallel sublookup Subkey Index (use DB).
* @param[in] commonKeyHeaderMaskPtr    - (Pointer to)rule Common Header Mask.
* @param[in] commonKeyHeaderPatternPtr - (Pointer to)rule Common Header Pattern.
* @param[in] subFieldConditionArrPtr   - (Pointer to) array of Subfield entries with Mask and Pattern.
*                                        subfields with bitLength > 32 not supported
* @param[out] pclRuleFormatPtr         - (Pointer to) rule format (PCL Library enum).
* @param[out] pclMaskPtr               - (Pointer to) rule mask (PCL Library union).
* @param[out] patternPtrPtr            - (Pointer to) rule pattern (PCL Library union).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS tgfTcamClientsPclRuleBuildForPclRuleSet
(
    IN  GT_U8                                           devNum,
    IN  TGF_TCAM_CLIENTS_DB_HANDLE                      dbHandle,
    IN  GT_U32                                          pclCfgProfileId,
    IN  GT_U32                                          udbPacketTypeGroupIndex,
    IN  TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                 pclLookup,
    IN  GT_U32                                          tcamSubkeyIndex,
    IN  GT_U32                                          *commonKeyHeaderMaskPtr,
    IN  GT_U32                                          *commonKeyHeaderPatternPtr,
    IN  TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    *subFieldConditionArrPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT              *pclRuleFormatPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                   *pclMaskPtr,
    OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT                   *pclPatternPtr
);

/**
* @internal tgfTcamClientsPclRuleSet function
* @endinternal
*
* @brief   The function sets the Policy Rule Mask, Pattern and Action
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                    - device number
* @param[in] dbHandle                  - Handle of DB of global configurations
* @param[in] pclCfgProfileId           - PCL Configuration Table Profile Id (use DB).
*                                        The parameters marked (use DB) used to determinate
*                                        rule size and fields layout in the rule by internal DB.
* @param[in] udbPacketTypeGroupIndex   - Index of Group of UDB Packet Type (use DB).
* @param[in] pclLookup                 - PCL lookup (use DB).
* @param[in] tcamSubkeyIndex           - TCAM parallel sublookup Subkey Index (use DB).
* @param[in] writeValidRule            - GT_TRUE - valid, GT_FALSE - invalid.
* @param[in] ruleIndex                 - index of the rule in the TCAM.
*                                        See the comment in cpssDxChPclRuleSet.
* @param[in] commonKeyHeaderMaskPtr    - (Pointer to)rule Common Header Mask.
* @param[in] commonKeyHeaderPatternPtr - (Pointer to)rule Common Header Pattern.
* @param[in] subFieldConditionArrPtr   - (Pointer to) array of Subfield entries with Mask and Pattern.
*                                        subfields with bitLength > 32 not supported
* @param[in] actionPtr                 - (Pointer to)rule action .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_OUT_OF_RANGE          - one of the parameters is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS tgfTcamClientsPclRuleSet
(
    IN GT_U8                                           devNum,
    IN TGF_TCAM_CLIENTS_DB_HANDLE                      dbHandle,
    IN GT_U32                                          pclCfgProfileId,
    IN GT_U32                                          udbPacketTypeGroupIndex,
    IN TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                 pclLookup,
    IN GT_U32                                          tcamSubkeyIndex,
    IN GT_U32                                          ruleIndex,
    IN GT_BOOL                                         writeValidRule,
    IN GT_U32                                          *commonKeyHeaderMaskPtr,
    IN GT_U32                                          *commonKeyHeaderPatternPtr,
    IN TGF_TCAM_CLIENTS_PCL_SUB_FIELD_CONDITION_STC    *subFieldConditionArrPtr,
    IN CPSS_DXCH_PCL_ACTION_STC                        *actionPtr
);

/**
* @internal tgfTcamClientsPclDirLookupToClientLookup function
* @endinternal
*
* @brief   The function converts PCL Direction Lookup To PCL Client Lookup
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] direction                - PCL direction
* @param[in] lookupNum                - lookup Number
*
* @retval conversion result
*
*
*/
TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT tgfTcamClientsPclDirLookupToClientLookup
(
    IN CPSS_PCL_DIRECTION_ENT       direction,
    IN CPSS_PCL_LOOKUP_NUMBER_ENT   lookupNum
);

/**
* @internal tgfTcamClientsSamplePclConfigurationGet function
* @endinternal
*
* @brief   Get Sample PCL configuration.
*
* @retval Address of sample Configuration
*/
const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* tgfTcamClientsSamplePclConfigurationGet
(
    GT_VOID
);

/**
* @internal tgfTcamClientsOneLookupPclConfigurationBuild function
* @endinternal
*
* @brief   Build One Lookup PCL configuration.
*
* @param[in] pclLookup                - PCL lookup
* @param[in] tcamKeySize              - TCAM Key Size
* @param[in] subFieldArrPtr           - (pointer to)array of subfields included in TCAM Key
*
* @retval pointer to static nemory with the fulfilled structure
*/
const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* tgfTcamClientsOneLookupPclConfigurationBuild
(
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT               pclLookup,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  tcamKeySize,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *subFieldArrPtr
);

/**
* @internal tgfTcamClientsIngressAndEgressPclConfigurationBuild function
* @endinternal
*
* @brief   Build One Ingress (or Midway) Lookup and Egress Lookup PCL configuration.
*
* @param[in] ipclLookup               - ingress PCL lookup
* @param[in] ipclReservedUdbBmpArr    - (pointer to) bitmap of reserved IPCL UDBs
* @param[in] ipclReservedUdbBmpArr    - (pointer to) bitmap of reserved EPCL UDBs
* @param[in] ipclTcamKeySize          - IPCL TCAM Key Size
* @param[in] epclTcamKeySize          - EPCL TCAM Key Size
* @param[in] ipclSubFieldArrPtr       - (pointer to)array of subfields included in IPCL TCAM Key
* @param[in] epclSubFieldArrPtr       - (pointer to)array of subfields included in EPCL TCAM Key
*
* @retval pointer to static nemory with the fulfilled structure
*/
const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC* tgfTcamClientsIngressAndEgressPclConfigurationBuild
(
    IN   TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT               ipclLookup,
    IN   GT_U32                                        ipclReservedUdbBmpArr[],
    IN   GT_U32                                        epclReservedUdbBmpArr[],
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  ipclTcamKeySize,
    IN   CPSS_DXCH_TCAM_RULE_SIZE_ENT                  epclTcamKeySize,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *ipclSubFieldArrPtr,
    IN   const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC *epclSubFieldArrPtr
);

/*===================================================================================*/
/* Centralized Application data For using the library */
#define PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_MAX_NUM_CNS 4
typedef struct
{
    GT_BOOL   tcamClientsLibInitDone;
    struct
    {
        TGF_TCAM_CLIENTS_DB_HANDLE   tcamClientsDbHandle;
        GT_U32 initializedDevicesBmpArr[(PRV_CPSS_MAX_PP_DEVICES_CNS + 31) / 32];
    } handleArr[PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_MAX_NUM_CNS];
} PRV_TGF_TCAM_CLIENTS_LIB_COMMON_HANDLERS_STC;

GT_BOOL prvTgfTcamClientCommomHandlersLibInitDoneGet(GT_VOID);
GT_VOID prvTgfTcamClientCommomHandlersLibInitDoneSet(GT_VOID);
TGF_TCAM_CLIENTS_DB_HANDLE prvTgfTcamClientCommomHandlersHandleGet(GT_U32 index);
/* when handle set to NULL device bitmap cleared */
GT_VOID prvTgfTcamClientCommomHandlersHandleSet(GT_U32 index,  TGF_TCAM_CLIENTS_DB_HANDLE handle);
GT_U32 prvTgfTcamClientCommomHandlersDeviceConfiguredGet(GT_U32 index, GT_U8 devNum);
GT_VOID prvTgfTcamClientCommomHandlersDeviceConfiguredSet(GT_U32 index, GT_U8 devNum);

/*===================================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTcamClients_h */