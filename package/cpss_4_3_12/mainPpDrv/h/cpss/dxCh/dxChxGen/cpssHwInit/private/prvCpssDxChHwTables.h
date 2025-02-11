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
* @file prvCpssDxChHwTables.h
*
* @brief Private API definition for tables access of the Cheetah and Cheetah 2.
*
* @version   95
********************************************************************************
*/

#ifndef __prvCpssDxChHwTablesh
#define __prvCpssDxChHwTablesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/private/prvCpssMisc.h>
#include <cpss/common/cpssHwInit/private/prvCpssHwMultiPortGroups.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChTables.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTablesSip6.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTablesSip7.h>


/* max number of words in entry */
#define PRV_CPSS_DXCH_TABLE_MAX_ENTRY_SIZE_CNS  128

/* value for field of 'fieldWordNum' that indicates that the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'Global bit offset' and not
   offset in specific word

   see relevant APIs :
   prvCpssDxChReadTableEntryField(...)
   prvCpssDxChWriteTableEntryField(...)
   prvCpssDxChPortGroupReadTableEntryField(...)
   prvCpssDxChPortGroupWriteTableEntryField(...)
*/
#define PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS   0xFFFFFFFF

/* value for field of 'fieldWordNum' that indicates that the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'field name' and not
   offset in specific word.

   important: the field 'fieldLength' is used ONLY if  it's value != 0
              and then it means 'Mumber of consecutive fields' !!!

   see relevant APIs :
   prvCpssDxChReadTableEntryField(...)
   prvCpssDxChWriteTableEntryField(...)
   prvCpssDxChPortGroupReadTableEntryField(...)
   prvCpssDxChPortGroupWriteTableEntryField(...)
*/
#define PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS   0xEEEEEEEE

#define PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS     0

/* value of 'sub field' for field of 'fieldWordNum' that indicates that
   the 'word' is not used,
   and instead the field of 'fieldOffset' represents 'field name' and not
   offset in specific word.
   and the 'fieldLength' is used as 'sub field offset' and as 'sub field length'.
   use macro

   see relevant APIs :
   prvCpssDxChReadTableEntryField(...)
   prvCpssDxChWriteTableEntryField(...)
   prvCpssDxChPortGroupReadTableEntryField(...)
   prvCpssDxChPortGroupWriteTableEntryField(...)
*/
#define PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS 0xDDDDDDDD

/* macro to use with PRV_CPSS_DXCH_TABLE_WORD_INDICATE_SUB_FIELD_IN_FIELD_NAME_CNS
    for the field of fieldLength
   The 'fieldLength' is used as 'sub field offset' and as 'sub field length'.
   use macro
   see relevant APIs :
   prvCpssDxChReadTableEntryField(...)
   prvCpssDxChWriteTableEntryField(...)
   prvCpssDxChPortGroupReadTableEntryField(...)
   prvCpssDxChPortGroupWriteTableEntryField(...)
*/
#define PRV_CPSS_DXCH_TABLE_SUB_FIELD_OFFSET_AND_LENGTH_MAC(_offset,_length)    \
    (((_offset)& 0xFFFF) | ((_length) << 16))

/* flag in param <*numBitsPerEntryPtr> in
    prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert*/
/* indication that the number in the lower 30 bits is 'fraction' (1/x) and not (x) */
#define FRACTION_INDICATION_CNS         BIT_31

/* flag in param <*numBitsPerEntryPtr> in
    prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert*/
/* indication that the number in the lower 30 bits is 'vertical' :
            the x entries in the first 'line' are not : 0,1
            the x entries in the first 'line' are     : 0,depth
 */
#define VERTICAL_INDICATION_CNS         BIT_30

/* flag in param <*numBitsPerEntryPtr> in
    prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert*/
/* indication that the number in the lower 30 bits is 'fraction' (1/x) and not (x)
    but 1/2 of the entry in second half of the table

    meaning that 'read'  of entry requires 2 'read' lines . from 2 half indexes in the table.
    meaning that 'write' of entry requires 2 'write' lines. to 2 half indexes in the table.
*/
#define FRACTION_HALF_TABLE_INDICATION_CNS         BIT_29

/* number of elements in static array (non dynamic alloc) */
#define NUM_ELEMENTS_IN_ARR_MAC(_array) \
    sizeof(_array)/sizeof(_array[0])

/* bind the array of fields formats to the DB of tables format according to table index
    tblDbPtr -  PRV_TABLE_FORMAT_INFO(devNum)
    tblFormatIndex - value from PRV_CPSS_DXCH_TABLE_FORMAT_ENT
    tblFormatArr - static array of type PRV_CPSS_ENTRY_FORMAT_TABLE_STC
*/
#define BIND_FIELDS_FORMAT_TO_TABLE_MAC(tblDbPtr , tblFormatIndex , tblFormatArr ) \
    tblDbPtr[tblFormatIndex].fieldsInfoPtr = tblFormatArr;                       \
    tblDbPtr[tblFormatIndex].numFields =  NUM_ELEMENTS_IN_ARR_MAC(tblFormatArr); \
    tblDbPtr[tblFormatIndex].patternNum = 0;                                     \
    tblDbPtr[tblFormatIndex].patternBits = 0

/* bind the array of fields formats to the DB of tables format according to table index
    tblDbPtr -  PRV_TABLE_FORMAT_INFO(devNum)
    tblFormatIndex - value from PRV_CPSS_DXCH_TABLE_FORMAT_ENT
    tblFormatArr - static array of type PRV_CPSS_ENTRY_FORMAT_TABLE_STC
    tblPatternBits - number of bits in pattern
    tblPatterNum - number of patterns in table entry
*/
#define BIND_PATTERN_FIELDS_FORMAT_TO_TABLE_MAC(tblDbPtr , tblFormatIndex , tblFormatArr, tblPatternBits, tblPatternNum ) \
    tblDbPtr[tblFormatIndex].fieldsInfoPtr = tblFormatArr;                      \
    tblDbPtr[tblFormatIndex].numFields =  NUM_ELEMENTS_IN_ARR_MAC(tblFormatArr); \
    tblDbPtr[tblFormatIndex].patternBits = tblPatternBits;                       \
    tblDbPtr[tblFormatIndex].patternNum = tblPatternNum

/* This value is used when control registor relates only to one table.
   In this case there is no need to update table index in control register.
   The action that is done for all tables is: table index << table index offset
   0 << 0 doesn't have any impact */
#define PRV_CPSS_DXCH_TABLE_NON_MULTI_MODE_CNS           0

/**
* @internal copyBits function
* @endinternal
*
* @brief   copy bits from source to target.
*         NOTE: function not supports overlap of copied memories.
*         for overlapping use copyBitsInMemory(...)
* @param[in] targetMemPtr             - (pointer to) target memory to write to.
* @param[in] targetStartBit           - start bit in the target to write to.
* @param[in] sourceMemPtr             - (pointer to) source memory to copy from.
* @param[in] sourceStartBit           - start bit in the source to copy from.
* @param[in] numBits                  - number of bits (unlimited num of bits)
*/
void  copyBits(
    IN GT_U32                  *targetMemPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  *sourceMemPtr,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
);
/**
* @internal copyBitsInMemory function
* @endinternal
*
* @brief   copy bits from source to target in a memory.
*         the function support overlap of copied bits from src to dst
* @param[in] memPtr                   - (pointer to) memory to update.
* @param[in] targetStartBit           - start bit in the memory to write to.
* @param[in] sourceStartBit           - start bit in the memory to copy from.
* @param[in] numBits                  - number of bits (unlimited num of bits)
*/
void  copyBitsInMemory(
    IN GT_U32                  *memPtr,
    IN GT_U32                  targetStartBit,
    IN GT_U32                  sourceStartBit,
    IN GT_U32                  numBits
);

/**
* @internal resetBitsInMemory function
* @endinternal
*
* @brief   reset bits in memory.
*
* @param[in] memPtr                   - (pointer to) memory to update.
* @param[in] startBit                 - start bit in the memory to reset.
* @param[in] numBits                  - number of bits (unlimited num of bits)
*/
void  resetBitsInMemory(
    IN GT_U32                  *memPtr,
    IN GT_U32                  startBit,
    IN GT_U32                  numBits
);

/**
* @enum PRV_CPSS_DXCH_ACCESS_ACTION_ENT
 *
 * @brief access action for non-direct memory of cheetah
*/
typedef enum{

    /** read action */
    PRV_CPSS_DXCH_ACCESS_ACTION_READ_E = 0,

    /** write action */
    PRV_CPSS_DXCH_ACCESS_ACTION_WRITE_E

} PRV_CPSS_DXCH_ACCESS_ACTION_ENT;


/* common used types */
/**
* @enum PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT
 *
 * @brief type of access to cheetah's table
*/
typedef enum{

    /** direct access */
    PRV_CPSS_DXCH_DIRECT_ACCESS_E,

    /** indirect access */
    PRV_CPSS_DXCH_INDIRECT_ACCESS_E,

    /** access by MGCAM mechanism for Write access.
     *  Direct access info is used in this case.
     *  APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Aldrin2; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman */
    PRV_CPSS_DXCH_MGCAM_ACCESS_E

} PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT;


/**
* @struct PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC
 *
 * @brief A structure to hold device tables format info.
*/
typedef struct{

    /** @brief number of fields in table entry
     *  fieldsInfoPtr  - pointer to table fields info
     */
    GT_U32 numFields;

    const PRV_CPSS_ENTRY_FORMAT_TABLE_STC *   fieldsInfoPtr;

    /** number of patterns in table entry */
    GT_U32 patternNum;

    /** number of bits in pattern */
    GT_U32 patternBits;

} PRV_CPSS_DXCH_PP_TABLE_FORMAT_INFO_STC;

/**
* @enum PRV_CPSS_DXCH_TABLE_FORMAT_ENT
 *
 * @brief Enumerator of tables formats names
*/
typedef enum{

    /** PreTTI lookup ingress ePort table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_TTI_DEFAULT_EPORT_E,

    /** TTI Physical port attributes table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,

    /** Post TTI lookup ingress table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E,

    /** Vlan port protocol table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E,

    /** Ingress VLAN table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EVLAN_E,

    /** Bridge ingress ePort table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_BRIDGE_INGRESS_EPORT_E,

    /** FDB table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E,

    /** IP MLL table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E,

    /** L2 MLL table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E,

    /** OAM Ingress/Egress table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_OAM_E,

    /** EGF_QAG egress ePort table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_EGRESS_EPORT_E,

    /** EGF_QAG target Port mapping table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_TARGET_PORT_MAPPER_E,

    /** Egress VLAN table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EVLAN_E,

    /** egress VLAN attributes table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_VLAN_ATTRIBUTES_E,

    /** ePort egress table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_SHT_EGRESS_EPORT_E,

    /** Header Alteration egress ePort table 1 format */
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_1_E,

    /** Header Alteration egress ePort table 2 format */
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_EGRESS_EPORT_2_E,

    /** Header Alteration physical port table 1 table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E,

    /** Header Alteration physical port table 2 table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E,

    /** TS table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E,

    /** HA Generic TS table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_GENERIC_TUNNEL_START_E,

    /** Generic Tunnel Start Profile table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_GENERIC_TS_PROFILE_E,

    /** IPCL action format */
    PRV_CPSS_DXCH_TABLE_FORMAT_IPCL_ACTION_E,

    /** EPCL action format */
    PRV_CPSS_DXCH_TABLE_FORMAT_EPCL_ACTION_E,

    /** @brief AU message format
     *  PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E , for all legacy fields that exists
     *  also in sip6.
     *  need to use this table format only for the updated/new fields of SIP6
     */
    PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E,

    /** IPvX router next hop format */
    PRV_CPSS_DXCH_TABLE_FORMAT_IPVX_ROUTER_NEXT_HOP_E,

    /** NAT44 table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E,

    /** NAT66 table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E,

    /** IPvX router ePort format */
    PRV_CPSS_DXCH_TABLE_FORMAT_ROUTER_EPORT_E,

    /** PLR metering entry format */
    PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_E,

    /** PLR metering entry format */
    PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFIG_E,

    /** PLR metering conformance sign entry format */
    PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFORM_SIGN_E,

    /** @brief EGF_QAG : physical port target attribute table
     *  new in sip_5_20
     */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,

    /** TTI action entry format (from sip5) */
    PRV_CPSS_DXCH_TABLE_FORMAT_TTI_ACTION_E,

    /** @brief SIP6 full format for
     *  'PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E' , but CPSS code should keep using
     */
    PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_SIP6_DIFF_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_PORT_PROFILE_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_PROFILES_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_QUEUE_CONFIGURATIONS_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_QUEUE_PORT_MAPPING_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_QUEUE_PRIO_MAPPING_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_TARGET_PHYSICAL_PORT_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_E,

    /* Sip 6.10 - Exact Match */
    PRV_CPSS_DXCH_TABLE_FORMAT_EXACT_MATCH_AUTO_LEARNED_ENTRY_INDEX_E,

    /* Sip 6.10 - PPU*/
    PRV_CPSS_DXCH_TABLE_FORMAT_PPU_ACTION_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_PPU_DAU_PROFILE_E,

    /* Sip 6.30 - SMU */
    PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_SNG_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_COUNTERS_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E,

    /* Sip6.30 - PREQ */
    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_MAPPING_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_CONFIG_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_COUNTERS_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_DAEMON_E,

    /* SIP7 */
    /* MPCL action format */
    PRV_CPSS_DXCH_TABLE_FORMAT_MPCL_ACTION_E,

    /* SIP7 - HA tables */
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_SRC_EPORT_1_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_SRC_EPORT_2_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_SRC_PHYSICAL_PORT_2_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_TARGET_CIRCUITS_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_ANALYZERS_TARGET_CIRCUITS_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_QUEUE_MAPPING_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_QOS_MAPPING_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_HEADER_FIELDS_E,

    /* SIP7 - IPVX Tables */
    PRV_CPSS_DXCH_TABLE_FORMAT_IPVX_ROUTER_L3NHE_PROFILE_E,

    PRV_CPSS_DXCH_TABLE_FORMAT_HA_IPV4_VPORT_TUNNEL_START_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_IPV6_VPORT_TUNNEL_START_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_HA_MPLS_VPORT_TUNNEL_START_E,

    /* SIP7 -- PPU for IPE tables*/
    PRV_CPSS_DXCH_TABLE_FORMAT_PPU_FOR_IPE_ACTION_E,
    /* SIP7 - IPE Tables */
    PRV_CPSS_DXCH_TABLE_FORMAT_IPE_STATE_ADDRESS_PROFILE_E,
    PRV_CPSS_DXCH_TABLE_FORMAT_IPE_STATE_DATA_CMD_PROFILE_E,
    /*SIP7-EGF*/
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_QAG_QUEUE_MAP_E,
    /* SIP7 - EGF-EFT */
    PRV_CPSS_DXCH_TABLE_FORMAT_EGF_EFT_EVLAN_E,

    /* SIP7 - IP MLL BIER table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_BIER_E,

    /* SIP7 - L2 MLL BIER table format */
    PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_BIER_E,


    PRV_CPSS_DXCH_TABLE_LAST_FORMAT_E

} PRV_CPSS_DXCH_TABLE_FORMAT_ENT;

/**
* @struct PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC
 *
 * @brief A structure to hold Cheetah's indirect access table entry info.
*/
typedef struct{

    /** address of the control register */
    GT_U32 controlReg;

    /** address of the data register of the entry */
    GT_U32 dataReg;

    /** the bit num that trig the action in the control register */
    GT_U32 trigBit;

    /** bit where to write the index (of entry) */
    GT_U32 indexBit;

    /** @brief some tables share the same control registers ,
     *  this value is the specific table
     */
    GT_U32 specificTableValue;

    /** start bit for the specificTableValue */
    GT_U32 specificTableBit;

    /** the bit number where to specify the action (read/write) */
    GT_U32 actionBit;

} PRV_CPSS_DXCH_TABLES_INFO_INDIRECT_STC;

/**
* @struct PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC
 *
 * @brief A structure to hold Cheetah's direct access table entry info.
*/
typedef struct{

    /** base address */
    GT_U32 baseAddress;

    /** step in entry promotion */
    GT_U32 step;

    /** next word offest value in bytes. */
    GT_U32 nextWordOffset;

} PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC;


/**
* @struct PRV_CPSS_DXCH_TABLES_INFO_STC
 *
 * @brief A structure to hold Cheetah's direct access table entry info.
*/
typedef struct{

    /** number of entries */
    GT_U32 maxNumOfEntries;

    /** size in words */
    GT_U32 entrySize;

    /** @brief direct/indirect access for read action
     *  readTablePtr   - address of table for read action
     */
    PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT readAccessType;

    const GT_VOID *readTablePtr;

    /** @brief direct/indirect access for write action
     *  writeTablePtr  - address of table for write action
     */
    PRV_CPSS_DXCH_TABLE_ACCESS_TYPE_ENT writeAccessType;

    const GT_VOID *writeTablePtr;

} PRV_CPSS_DXCH_TABLES_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC
 *
 * @brief A structure to hold direct access table entry info per unit.
*/
typedef struct{

    /** table index */
    CPSS_DXCH_TABLE_ENT globalIndex;

    /** unit index */
    PRV_CPSS_DXCH_UNIT_ENT unitIndex;

    /** direct access table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC directAccessInfo;

    /** indication that base address was already aligned. */
    GT_BOOL initWasDone;

} PRV_CPSS_DXCH_TABLES_INFO_DIRECT_UNIT_STC;

/**
* @struct PRV_CPSS_DXCH_TABLES_INFO_EXT_STC
 *
 * @brief A structure to override table info.
*/
typedef struct
{
    /** index of overriden entry */
    CPSS_DXCH_TABLE_ENT              globalIndex;

    /** overriding info */
    PRV_CPSS_DXCH_TABLES_INFO_STC    tableInfo;

} PRV_CPSS_DXCH_TABLES_INFO_EXT_STC;

/**************************************************/
/********** TTI UNIT START ************************/
/**************************************************/
/**
* @enum SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of default Eport table.
*/
typedef enum{
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_QOS_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PROT_BASED_VLAN_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_UP0_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_VLAN_TRANSLATION_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_VID_PRECEDENCE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_P_EVLAN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_QOS_PROFILE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_EXP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_DSCP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_UP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_DSCP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_MODIFY_UP_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PORT_QOS_PRECEDENCE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_ENGINE_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OAM_LINK_LAYER_PDU_TRAP_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_OAM_LINK_LAYER_LOOPBACK_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_LOOKUP0_PCL_CFG_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_LOOKUP1_PCL_CFG_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_LOOKUP2_PCL_CFG_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ASSIGN_VF_ID_EN_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE_E,
    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_MRU_INDEX_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_SOURCE_EPG_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_COPY_TAG1_VID_TO_SRC_EPG_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_UP1_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI1_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI0_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E,
    SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_EVLAN_EXTENSION_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_RANGE_SOURCE_PORT_PROFILE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM_PORT_PROFILE_ID_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_CLASSIFICATION_FIELD_DEFAULT_VALUE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM1_LOOKUP_KEY_MODE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM2_LOOKUP_KEY_MODE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_MIDWAY_PCL_CFG_MODE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_EXCESSIVE_L2_TAG_EXCEPTION_ENABLE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_INGRESS_CASCASDE_DLB_ENABLE_E,
    SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_EXTENSION_E,

    SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP5_TTI_DEFAULT_EPORT_TABLE_FIELDS_ENT;




/**
* @enum SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of physical port table.
*/
typedef enum{
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE_E,
    SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E, /* used for array size */

    SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE_E = SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E,
    SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP_FIELD_E,
    SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E, /* used for array size */

    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID_MODE_E = SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_PROFILE_ID_MODE_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID1_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID2_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID_MODE_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PPU_PROFILE_E,
    SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP5_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENT;




/**
* @enum SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of ePort Attributes Table.
*/
typedef enum{
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_POLICY_EN_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_DIS_IPCL0_FOR_ROUTED_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_HASH_MASK_INDEX_E,
    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN_E,

    SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP5_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ENT;




/**
* @enum SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of Default Port Protocol eVLAN and QoS Configuration table.
*/
typedef enum{
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_EVLAN_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE_E,
    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE_E,

    SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS___LAST_VALUE___E /* used for array size */

}SIP5_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_ENT;


/* the fields of the (TTI) action table in Sip5 */
typedef enum{
    SIP5_TTI_ACTION_TABLE_FIELDS_COMMAND_E
,   SIP5_TTI_ACTION_TABLE_FIELDS_CPU_CODE_E
,   SIP5_TTI_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E
,   SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_CMD_E

/*If <Redirect Command> = No Redirect (0)*/
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_FLOW_ID_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_IPCL_UDB_CONFIGURATION_TABLE_UDE_INDEX_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_RESERVED_E
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_ENABLE_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_TTI_RESERVED_VALUE_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY2_LOOKUP_MODE_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY1_LOOKUP_MODE_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_POLICY0_LOOKUP_MODE_E
,      SIP5_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_IPCL_PROFILE_INDEX_E

/*else If <Redirect Command> = Redirect to Egress Interface (1)*/
    /*    if <UseVIDX> = 0*/
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_IS_TRUNK_E
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TRUNK_NUMBER_E
    /*            SIP5_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EPORT_NUMBER_E
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_DEVICE_NUMBER_E
    /*    else, when <UseVIDX> = 1*/
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_EVIDX_E
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_RESERVED_E

,        SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_USE_EVIDX_E
,        SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_VNT_L2_ECHO_E
    /*    Tunnel Start*/
,        SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_E

    /*    if (<Tunnel Start> == 1)*/
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_INDEX_E
,            SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_TUNNEL_START_PASSENGER_TYPE_E
    /*            SIP5_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else, <Tunnel Start> == 0*/
,           SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_INTERFACE_ARP_POINTER_E

,       SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_DA_E
,       SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_EGRESS_MODIFY_MAC_SA_E


/*else If <Redirect Command> = Redirect To Router (2)*/
,       SIP5_TTI_ACTION_TABLE_FIELDS_REDIRECT_TO_ROUTER_ROUTER_LTT_INDEX_E
    /*        SIP5_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved
            SIP5_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/
/*else If <Redirect Command> = Assign VRF-ID (4)*/
,       SIP5_TTI_ACTION_TABLE_FIELDS_ASSIGN_VRF_ID_VRF_ID_E
    /*        SIP5_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved
            SIP5_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved
            SIP5_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/

,    SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_POLICER_INDEX_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_ID_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_ACTION_STOP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_BRIDGE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_PRECEDENCE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_NESTED_VLAN_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_EVID_CMD_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_EVLAN_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_VID1_CMD_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_VID1_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_QOS_PROFILE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_DSCP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_MODIFY_UP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_KEEP_PREVIOUS_QOS_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_UP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_DSCP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_EXP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_MAP_DSCP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_UP0_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_UP1_COMMAND_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_UP1_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PACKET_TYPE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_COPY_TTL_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TUNNEL_TERMINATION_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_MPLS_COMMAND_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_HASH_MASK_INDEX_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_L_LSP_QOS_PROFILE_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TTL_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_DEC_TTL_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_ASSIGNMENT_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_SOURCE_EPORT_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TT_HEADER_LENGTH_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_IS_PTP_PACKET_E

/*If <Is PTP PAcket> = 0*/
,       SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_TIMESTAMP_ENABLE_E
,       SIP5_TTI_ACTION_TABLE_FIELDS_NON_PTP_PACKET_OFFSET_INDEX_E
    /*        SIP5_TTI_ACTION_TABLE_FIELDS_    reserved*/
/*else If <Is PTP PAcket> =1*/
,       SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_TRIGGER_TYPE_E
,       SIP5_TTI_ACTION_TABLE_FIELDS_PTP_PACKET_PTP_OFFSET_E

,    SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_OAM_PROCESSING_WHEN_GAL_OR_OAL_EXISTS_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_CW_BASED_PSEUDO_WIRE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_TTL_EXPIRY_VCCV_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_PWE3_FLOW_LABEL_EXIST_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_PW_CW_BASED_E_TREE_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_RX_IS_PROTECTION_PATH_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_RX_ENABLE_PROTECTION_SWITCHING_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_SET_MAC2ME_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_OAM_PROFILE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_APPLY_NON_DATA_CW_COMMAND_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_PW_TAG_MODE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_ENABLE_TTI_1_LOOKUP_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_TRANSIT_MPLS_TUNNEL_ENABLE_E
,    SIP5_TTI_ACTION_TABLE_FIELDS_PASSENGER_PARSING_OF_NON_MPLS_TRANSIT_TUNNEL_ENABLE_E
,    SIP6_TTI_ACTION_TABLE_FIELDS_SKIP_FDB_SA_LOOKUP_ENABLE_E
,    SIP6_TTI_ACTION_TABLE_FIELDS_IPV6_SEGMENT_ROUTING_END_NODE_ENABLE_E
,    SIP6_TTI_ACTION_TABLE_FIELDS_TUNNEL_HEADER_START_L4_ENABLE_E
,    SIP6_TTI_ACTION_TABLE_FIELDS_RESERVED_E
,    SIP6_TTI_ACTION_TABLE_FIELDS_TTI_OVER_EXACT_MATCH_ENABLE_E
,    SIP6_10_TTI_ACTION_TABLE_FIELDS_TRIGER_CNC_HASH_CLIENT_E
,    SIP6_10_TTI_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E
,    SIP6_10_TTI_ACTION_TABLE_FIELDS_PPU_PROFILE_E
,    SIP6_30_TTI_ACTION_TABLE_FIELDS_GENERIC_ACTION_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_TRG_VPORT_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_USE_TARGET_VPORT_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_STOP_MPLS_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_SRV6_PSP_ENABLED_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_QCN_DLB_PROC_EN_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_GENERIC_CLASSIFICATION_VALUE_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_QOS_MAPPING_MODE_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_FORCE_DP_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_QOS_MODEL_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_TTL_MODEL_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_SOURCE_VPORT_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_SOURCE_INTERFACE_ASSIGNMENT_EN_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_SOURCE_VPORT_PROFILE_ASSIGNMENT_EN_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_DISABLE_EM_LOOKUP_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_CANCEL_DROP_E
,    SIP7_TTI_ACTION_TABLE_FIELDS_NO_REDIRECT_TO_EGRESS_INTERFACE_MPCL_LOOKUP_MODE_E

,    SIP5_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_TTI_ACTION_TABLE_FIELDS_ENT;


/**************************************************/
/********** TTI UNIT END   ************************/
/**************************************************/

/**************************************************/
/********** IPCL UNIT START ***********************/
/**************************************************/

/**
* @enum SIP5_IPCL_ACTION_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of IPCL Action table - sip5.
*/
typedef enum {
     SIP5_IPCL_ACTION_TABLE_FIELDS_COMMAND_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_0_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_RESERVED_46_15_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_RESERVED_17_15_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VIDX_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_29_0_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_30_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_SET_ENABLE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_SOURCE_ID_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VID0_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_2_1_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_VID1_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_UP1_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E
    ,SIP5_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E

    ,SIP5_IPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_IPCL_ACTION_TABLE_FIELDS_ENT;

/*
 * Typedef: enum SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT
 *
 * Description:
 *      enumeration to hold fields of IPCL Action table  - sip5_20.
 *
 */
/* the fields of the (IPCL) action table in Sip5_20 (reordered from sip5 !!!) */
typedef enum {
     SIP5_20_IPCL_ACTION_TABLE_FIELDS_CPU_CODE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_COMMAND_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_REDIRECT_COMMAND_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTION_STOP_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_MAC2ME_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICY1_LOOKUP_MODE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICY2_LOOKUP_MODE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_PROFILE_INDEX_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_USE_VIDX_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VIDX_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_IS_TRUNK_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TRG_PORT_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TARGET_DEVICE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TRUNK_ID_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VNT_L2_ECHO_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_ARP_POINTER_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_POINTER_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TUNNEL_START_PASSENGER_TYPE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_PBR_MODE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_IP_NEXT_HOP_ENTRY_INDEX_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VRF_ID_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_27_0_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MAC_SA_47_28_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_METER_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_POLICER_PTR_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_RESERVED_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENABLE_MIRROR_TCP_RST_OR_FIN_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MIRROR_TO_ANALYZER_PORT_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_DA_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_MAC_SA_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_EGRESS_FILTER_REGISTERED_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_BRIDGE_BYPASS_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_BYPASS_INGRESS_PIPE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_EN_NESTED_VLAN_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_SET_SST_ID_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_SST_ID_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_USER_AC_ENABLE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_ACTIVATE_COUNTER_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_COUNTER_INDEX_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN_PRECEDENCE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN_COMMAND_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VID0_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_MARKING_ENABLE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PRECEDENCE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_QOS_PROFILE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_MODIFY_UP_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VLAN1_CMD_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_UP1_CMD_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_VID1_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_UP1_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_RESERVED_EN_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_IPCL_TO_TXQ_RESERVED_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_TIMESTAMP_EN_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_OFFSET_INDEX_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_EN_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_FLOW_ID_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_RESERVED_1_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_ASSIGN_SRC_EPORT_EN_E
    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS_SRC_EPORT_E

    ,SIP5_20_IPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_20_IPCL_ACTION_TABLE_FIELDS_ENT;



/**************************************************/
/********** IPCL UNIT END *************************/
/**************************************************/

/**************************************************/
/********** L2I UNIT START ************************/
/**************************************************/


/**
* @enum SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of L2i (bridge) ingress VLAN table.
*/
typedef enum{
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VALID_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NEW_SRC_ADDR_IS_NOT_SECURITY_BREACH_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IP_MULTICAST_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_MULTICAST_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV6_MULTICAST_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKNOWN_UNICAST_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IGMP_TO_CPU_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_MODE_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_MODE_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MIRROR_TO_INGRESS_ANALYZER_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_ICMP_TO_CPU_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_CONTROL_TO_CPU_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_IPM_BRIDGING_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_IPM_BRIDGING_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_IPV4_BC_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREGISTERED_NON_IPV4_BC_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_UNICAST_ROUTE_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MULTICAST_ROUTE_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_UNICAST_ROUTE_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MULTICAST_ROUTE_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_SITEID_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_AUTO_LEARN_DIS_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_MRU_INDEX_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_BC_UDP_TRAP_MIRROR_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_CONTROL_TO_CPU_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_EVIDX_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_VRF_ID_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UC_LOCAL_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FLOOD_VIDX_MODE_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV4_MC_BC_TO_MIRROR_ANLYZER_IDX_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_IPV6_MC_TO_MIRROR_ANALYZER_IDX_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FID_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNKOWN_MAC_SA_CMD_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FCOE_FORWARDING_EN_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_MODE_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_UNREG_IPM_EVIDX_E,
    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E,

    SIP7_L2I_INGRESS_VLAN_TABLE_FIELDS_ENABLE_TRAP_MIRROR_ARP_BC_WITH_MY_IP_E,
    SIP7_L2I_INGRESS_VLAN_TABLE_FIELDS_GRATUITOUS_ARP_REPLY_BC_TO_CPU_EN_E,

    SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP5_L2I_INGRESS_VLAN_TABLE_FIELDS_ENT;






/**
* @enum SIP5_L2I_EPORT_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of L2i (bridge) EPORT table.
*/
typedef enum{
    SIP5_L2I_EPORT_TABLE_FIELDS_NA_MSG_TO_CPU_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_AUTO_LEARN_DIS_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_NA_STORM_PREV_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_NEW_SRC_ADDR_SECURITY_BREACH_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_VLAN_INGRESS_FILTERING_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_ACCEPT_FRAME_TYPE_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_UC_LOCAL_CMD_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_UNKNOWN_SRC_ADDR_CMD_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_PORT_VLAN_IS_TRUNK_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_TRG_EPORT_TRUNK_NUM_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_PORT_PVLAN_TRG_DEV_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_ALL_PKT_TO_PVLAN_UPLINK_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_IGMP_TRAP_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_ARP_BC_TRAP_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ON_TRAP_IEEE_RSRV_MC_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_IEEE_RSVD_MC_TABLE_SEL_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_PORT_SOURCE_ID_FORCE_MODE_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_SRC_ID_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_ARP_MAC_SA_MIS_DROP_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNKNOWN_UC_FILTER_CMD_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_UNREG_MC_FILTER_CMD_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_PORT_BC_FILTER_CMD_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_EPORT_STP_STATE_MODE_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_INGRESS_EPORT_SPANNING_TREE_STATE_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_IPV4_CONTROL_TRAP_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_IPV6_CONTROL_TRAP_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_BC_UDP_TRAP_OR_MIRROR_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_EN_LEARN_ORIG_TAG1_VID_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV4_ROUTING_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_FDB_UC_IPV6_ROUTING_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_FDB_FCOE_ROUTING_EN_E,
    SIP5_L2I_EPORT_TABLE_FIELDS_MOVED_MAC_SA_CMD_E,
    /* sip7 fields*/
    SIP7_L2I_EPORT_TABLE_FIELDS_ENABLE_TRAP_MIRROR_ARP_BC_WITH_MY_IP_E,
    SIP7_L2I_EPORT_TABLE_FIELDS_ENABLE_EPORT_GRATUITOUS_ARP_REPLY_BC_TO_CPU_E,

    SIP5_L2I_EPORT_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP5_L2I_EPORT_TABLE_FIELDS_ENT;




/**************************************************/
/********** L2I UNIT END   ************************/
/**************************************************/

/**************************************************/
/********** FDB UNIT START ************************/
/**************************************************/


/**
* @enum SIP5_FDB_FDB_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of FDB table.(FDB unit)
*/
typedef enum {

     SIP5_FDB_FDB_TABLE_FIELDS_VALID_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SKIP_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_AGE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_FDB_ENTRY_TYPE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_FID_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DEV_ID_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_5_0_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IS_TRUNK_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DIP_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SIP_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_VIDX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_TRUNK_NUM_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_EPORT_NUM_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_USER_DEFINED_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_RESERVED_99_103_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_11_9_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DA_ACCESS_LEVEL_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SA_ACCESS_LEVEL_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SOURCE_ID_8_6_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_RESERVED_116_118_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_ORIG_VID1_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IS_STATIC_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_MULTIPLE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DA_CMD_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SA_CMD_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DA_ROUTE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SP_UNKNOWN_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_VRF_ID_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV6_SCOPE_CHECK_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_FCOE_D_ID_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV4_DIP_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DST_SITE_ID_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_INDEX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_UP_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_MODIFY_DSCP_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_COUNTER_SET_INDEX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_DIP_ACCESS_LEVEL_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_EN_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_MTU_INDEX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_USE_VIDX_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_TARGET_DEVICE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_NEXT_HOP_EVLAN_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_START_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_TYPE_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_ARP_PTR_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_TUNNEL_PTR_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_0_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_1_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_2_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_IPV6_DIP_3_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_NH_DATA_BANK_NUM_E
    ,SIP5_FDB_FDB_TABLE_FIELDS_NH_KEY_HW_INDEX_E    /* Field for un-used bits of ipv6-data */

    ,SIP5_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E/* used for array size */

    ,SIP5_10_FDB_FDB_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E = SIP5_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E

    ,SIP5_10_FDB_FDB_TABLE_FIELDS___LAST_VALUE___E


}SIP5_FDB_FDB_TABLE_FIELDS_ENT;




/* macro to set value to field of (FDB) FDB entry format in buffer */
#define SIP5_FDB_ENTRY_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (FDB) FDB entry format in buffer */
#define SIP5_FDB_ENTRY_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        _fieldName,                                                 \
        _value)


/* macro to set value to MAC ADDR field of (FDB) FDB entry format in buffer */
#define SIP5_FDB_ENTRY_FIELD_MAC_ADDR_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                   \
        _macAddrBytes)

/* macro to get value to MAC ADDR field of (FDB) FDB entry format in buffer */
#define SIP5_FDB_ENTRY_FIELD_MAC_ADDR_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_E].fieldsInfoPtr,                  \
        SIP5_FDB_FDB_TABLE_FIELDS_MAC_ADDR_E,                   \
        _macAddrBytes)


/**
* @enum SIP5_FDB_AU_MSG_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of AU messages formats.(FDB unit)
*/
typedef enum {
     SIP5_FDB_AU_MSG_TABLE_FIELDS_MESSAGE_ID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_MSG_TYPE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_ENTRY_TYPE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_VALID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SKIP_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_AGE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_8_0_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_CHAIN_TOO_LONG_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_OFFSET_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_ENTRY_FOUND_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DEV_ID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_5_0_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DIP_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SIP_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_FID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_VIDX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_TRUNK_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_EPORT_NUM_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_TRUNK_NUM_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_USER_DEFINED_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_RESERVED_109_113_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_11_9_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_ORIG_VID1_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ACCESS_LEVEL_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_ACCESS_LEVEL_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SOURCE_ID_8_6_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SP_UNKNOWN_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SEARCH_TYPE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_INDEX_20_9_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_MULTIPLE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_ROUTE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_QOS_PARAM_SET_IDX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_QOS_PARAM_SET_IDX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_IS_STATIC_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_CMD_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_CMD_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_SA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_DA_LOOKUP_INGRESS_MIRROR_TO_ANALYZER_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_FDB_LOOKUP_KEY_MODE_E

    /* additions for the  format of : MAC NA moved update Message */
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_UP0_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_IS_MOVED_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_IS_TRUNK_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_EPORT_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_TRUNK_NUM_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_DEVICE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_NA_MOVED_OLD_SRC_ID_E

    /* fields for FDB Routing */
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MAC_ADDR_INDEX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_VRF_ID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_SCOPE_CHECK_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_FCOE_D_ID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV4_DIP_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DST_SITE_ID_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DEC_TTL_OR_HOP_COUNT_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_INGRESS_MIRROR_TO_ANALYZER_INDEX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_MARKING_EN_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_INDEX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_QOS_PROFILE_PRECEDENCE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_UP_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MODIFY_DSCP_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_COUNTER_SET_INDEX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_BC_TRAP_MIRROR_EN_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_DIP_ACCESS_LEVEL_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ICMP_REDIRECT_EXCEP_MIRROR_EN_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_MTU_INDEX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_USE_VIDX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IS_TRUNK_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EVIDX_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TARGET_DEVICE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TRUNK_NUM_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_EPORT_NUM_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NEXT_HOP_EVLAN_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_START_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_TYPE_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_ARP_PTR_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TUNNEL_PTR_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_0_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_1_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_2_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_IPV6_DIP_3_E
    ,SIP5_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_NH_DATA_BANK_NUM_E

    ,SIP5_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E/* used for array size */

    ,SIP5_10_FDB_AU_MSG_TABLE_FIELDS_UC_ROUTE_TS_IS_NAT_E = SIP5_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E

    ,SIP5_10_FDB_AU_MSG_TABLE_FIELDS___LAST_VALUE___E

}SIP5_FDB_AU_MSG_TABLE_FIELDS_ENT;


/* macro to set value to field of (FDB) AU message format in buffer */
#define SIP5_FDB_AU_MSG_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (FDB) AU message format in buffer */
#define SIP5_FDB_AU_MSG_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)


/* macro to set value to MAC ADDR field of (FDB) AU message format in buffer */
#define SIP5_FDB_AU_MSG_FIELD_MAC_ADDR_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E].fieldsInfoPtr,             \
        SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_E,                   \
        _macAddrBytes)

/* macro to get value to MAC ADDR field of (FDB) AU message format in buffer */
#define SIP5_FDB_AU_MSG_FIELD_MAC_ADDR_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_FDB_AU_MSG_E].fieldsInfoPtr,             \
        SIP5_FDB_AU_MSG_TABLE_FIELDS_MAC_ADDR_E,                   \
        _macAddrBytes)

/**************************************************/
/********** FDB UNIT END   ************************/
/**************************************************/

/**************************************************/
/********** EGF UNIT START ************************/
/**************************************************/

/**
* @enum SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EGF_QAG egress EPort table.(EGF_QAG unit)
*/
typedef enum {
     SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_DP_TO_CFI_REMAP_ENABLE_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_MODE_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EPORT_TAG_STATE_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_COMMAND_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_COMMAND_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_COMMAND_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_COMMAND_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EVLAN_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_VID1_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP0_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_UP1_E
    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_1_IF_RX_WITHOUT_TAG_1_E
    ,SIP6_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E
    /* sip 6.30 */
    ,SIP6_30_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_REMOVE_VLAN_TAG_0_IF_RX_WITHOUT_TAG_0_E

    ,SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_EGRESS_EGF_QAG_EGRESS_EPORT_TABLE_FIELDS_ENT;

/**
* @enum SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EGF_QAG egress EPort table.(EGF_QAG unit)
*/
typedef enum {
     SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E
    ,SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E
    ,SIP6_30_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_ENABLE_EVLAN_Q_OFFSET_E
    ,SIP6_30_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_ENABLE_PRIORITY_Q_OFFSET_E
    ,SIP6_30_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_UNKNOWN_CT_ENABLE_E

    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_QUEUE_GROUP_INDEX_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DP_CORE_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_Q_MODE_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TC_PROFILE_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_TC_PROF_MODE_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_FIRST_BIT_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_LAST_BIT_E
    ,SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TRG_IS_CASCADE_E

    ,SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
} SIP5_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_ENT;

/* the fields of the EGF_QAG target port entry format in Sip5_20 */
typedef enum{
     SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E
    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_TC_PROFILE_E
    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E
    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_EUSE_VLAN_TAG_1_FOR_TAG_STATE_E
    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ETARGET_PHYSICAL_PORT_ENQ_PROFILE_E
    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ELOOPBACK_ENQ_PROFILE_E
    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ELOOPBACK_PORT_E

    ,SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E/* used for array size */

}SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_ENT;

/* the fields of the EGF_QAG source port entry format in Sip5_20 */
typedef enum{
     SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E
    ,SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E
    ,SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E

    ,SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E/* used for array size */

}SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_ENT;

/**
* @enum SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EGF_SHT egress EVlan table.(EGF_SHT unit)
*/
typedef enum{

    SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_VALID_E

    ,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_PORT_X_MEMBER_E

    ,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS___LAST_VALUE___E

} SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_TABLE_FIELDS_ENT;




/**
* @enum SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EGF_SHT egress EVlan Attributes table.(EGF_SHT unit)
*/
typedef enum{

    SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_LOCAL_SWITHING_EN_E

    ,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_PORT_ISOLATION_VLAN_CMD_E

    ,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRROR_TO_ANALYZER_INDEX_E

    ,SIP6_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E

    ,SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E

} SIP5_EGRESS_EGF_SHT_EGRESS_EVLAN_ATTRIBUTES_TABLE_FIELDS_ENT;




/**
* @enum SIP5_EGRESS_EGF_SHT_EGRESS_EPORTS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EGF_SHT egress EPort Attributes table.(EGF_SHT unit)
*/
typedef enum {
     SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_FROM_CPU_FWD_RESTRICT_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_BRIDGED_FWD_RESTRICT_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_ROUTED_FWD_RESTRICT_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_IPMC_ROUTED_FILTER_EN_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_EGRESS_EVLAN_FILTER_EN_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_PORT_ISOLATION_MODE_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_EPORT_STP_STATE_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_STP_STATE_MODE_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_ODD_ONLY_SRC_ID_FILTER_EN_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_EGRESS_EPORT_VLAN_FILTER_EN_E
    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS_MESH_ID_E

    ,SIP5_EGRESS_EGF_SHT_EGRESS_EPORT_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_EGRESS_EGF_SHT_EGRESS_EPORTS_TABLE_FIELDS_ENT;




/**************************************************/
/********** EGF UNIT END   ************************/
/**************************************************/

/**************************************************/
/********** MLL UNIT START   **********************/
/**************************************************/

/**
* @enum SIP5_L2_MLL_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of L2MLL table.(MLL unit)
*/
typedef enum {

     SIP5_L2_MLL_TABLE_FIELDS_LAST_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_VIDX_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0_E
    ,SIP5_L2_MLL_TABLE_FIELDS_RESERVED_1_E

    ,SIP5_L2_MLL_TABLE_FIELDS_LAST_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_VIDX_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1_E
    ,SIP5_L2_MLL_TABLE_FIELDS_RESERVED_2_E

    ,SIP5_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR_E
    ,SIP5_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR_E

    /* sip7 new fields */
    ,SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0_E
    ,SIP7_L2_MLL_TABLE_FIELDS_VPORT_0_E
    ,SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1_E
    ,SIP7_L2_MLL_TABLE_FIELDS_VPORT_1_E

    ,SIP5_L2_MLL_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_L2_MLL_TABLE_FIELDS_ENT;




/* macro to set value to field of (MLL) L2 MLL entry format in buffer */
#define SIP5_L2_MLL_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)                                    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                 \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _value)

/* macro to get value to field of (MLL) L2 MLL entry format in buffer */
#define SIP5_L2_MLL_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)                                    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                 \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _value)

/* macro to get field's info (start bit, length) of IP MLL entry */
#define SIP5_L2_MLL_FIELD_INFO_GET_MAC(_dev,_fieldName,_startBit, _numOfBits)                           \
    GET_FIELD_INFO_BY_ID_MAC(                                                                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _startBit,                                                                                      \
        _numOfBits)

/* macro to set value to field of L2 MLL entry format in buffer nad raise bits appropriate to field
 * in the mask buffer */
#define SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(_dev, _hwDataArr, _hwMaskArr, _fieldName, _value)            \
    U32_SET_FIELD_AND_MASK_BY_ID_MAC(                                                                   \
        _hwDataArr, _hwMaskArr,                                                                         \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_L2_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _value)
/* the fields of the (MLL) L2MLL entry format in Sip5 */
typedef enum {

     SIP5_IP_MLL_TABLE_FIELDS_LAST_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_EVIDX_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_RESEREVED_2_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0_E
    ,SIP5_IP_MLL_TABLE_FIELDS_LAST_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_MLL_EVID_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_USE_VIDX_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_EVIDX_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TRG_EPORT_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_RESEREVED_3_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TRG_DEV_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1_E
    ,SIP5_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR_E
    /* sip7 new fields */
    ,SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0_E
    ,SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0_E
    ,SIP7_IP_MLL_TABLE_FIELDS_VPORT_0_E
    ,SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1_E
    ,SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1_E
    ,SIP7_IP_MLL_TABLE_FIELDS_VPORT_1_E

    ,SIP5_IP_MLL_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_IP_MLL_TABLE_FIELDS_ENT;




/* macro to set value to field of (MLL) IP MLL entry format in buffer */
#define SIP5_IP_MLL_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)                                    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                 \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _value)

/* macro to get value to field of (MLL) IP MLL entry format in buffer */
#define SIP5_IP_MLL_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)                                    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                 \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _value)


/* macro to get field's info (start bit, length) of IP MLL entry */
#define SIP5_IP_MLL_FIELD_INFO_GET_MAC(_dev, _fieldName,_startBit, _numOfBits) \
    GET_FIELD_INFO_BY_ID_MAC(                                                                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _startBit,                                                                                      \
        _numOfBits)
/* macro to set value to field of IP MLL entry format in buffer nad raise bits appropriate to field
 * in the mask buffer */
#define SIP5_IP_MLL_FIELD_AND_MASK_SET_MAC(_dev, _hwDataArr, _hwMaskArr, _fieldName, _value)            \
    U32_SET_FIELD_AND_MASK_BY_ID_MAC(                                                                   \
        _hwDataArr, _hwMaskArr,                                                                         \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_IP_MLL_E].fieldsInfoPtr, \
        _fieldName,                                                                                     \
        _value)


/**************************************************/
/********** MLL UNIT END   ************************/
/**************************************************/

/*************************************************/
/********** HA UNIT START ************************/
/*************************************************/
/* the fields of the (HA) physical port table 1 */
typedef enum{
     SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTED_MAC_SA_MOD_EN_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTER_MAC_SA_ASSIGNMENT_MODE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PER_UP0_KEEP_VLAN1_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_IP_TUNNEL_LENGTH_OFFSET_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_LOOKUP_CONFIGURATION_MODE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_NON_TS_DATA_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_TS_DATA_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_DATA_PACKETS_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_CONTROL_PACKETS_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_GROUP_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_LIST_BIT_VECTOR_OFFSET_E

    /* new in sip6.10 (AC5P) */
    ,SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PCL_ID2_E      /*27..50*/
    ,SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E   /*51*/
    ,SIP6_10_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E       /*52..57*/

    /* new in sip6.30 (Ironman) */
    ,SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_DESTINATION_EPG_E  /*58*/
    ,SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_DESTINATION_EPG_E      /*59..70*/
    ,SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E   /*71..72*/
    ,SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E     /*73..74*/
    ,SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E /*75..80*/


    ,SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ENT;

/* the fields of the (HA) physical port table 2 */
typedef enum{
     SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DISABLE_CRC_ADDING_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_RETAIN_EXTERNAL_CRC_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PACKET_ID_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_ANALYZER_PACKETS_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_CPU_PACKETS_PORT_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID_E
   /* new in sip5_15 */
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA_E
   /* new in sip5_20 */
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FORWARD_DSA_ENABLE_E
   /* new in SIP6_10 */
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E
    ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E

    /* new in sip6.30 (Ironman) */
    ,SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E /*95..100*/

   ,SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ENT;





/* the fields of the (HA) eport table 1 */
typedef enum{
     SIP5_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_TS_EXT_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_TS_POINTER_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL_E
    ,SIP5_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL_E

    ,SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER_E

    ,SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN_E

    ,SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E     /*97..98*/
    ,SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E       /*99..102*/
    ,SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E         /*103*/
    ,SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E              /*46..61*/

   ,SIP5_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_EPORT_TABLE_1_FIELDS_ENT;





/* the fields of the (HA) eport table 2 */
typedef enum{
     SIP5_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS_E
    ,SIP5_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN_E
    ,SIP5_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND_E
    ,SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT_E
    ,SIP5_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE_E
    ,SIP5_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND_E
    ,SIP5_HA_EPORT_TABLE_2_UP_E
    ,SIP5_HA_EPORT_TABLE_2_CFI_E
    ,SIP5_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT_E

   ,SIP5_HA_EPORT_TABLE_2_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_EPORT_TABLE_2_FIELDS_ENT;

/* the fields of the (HA) tunnel start table */
typedef enum{
     SIP5_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UP_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_VID_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TTL_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_KEY_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_1_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_1_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_2_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RID_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_CAPWAP_FLAGS_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_ENABLE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_0_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_4_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL1_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_3_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL_E /* MPLS Ethertype Select */
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_7_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL2_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_5_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_LABEL3_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_6_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_UP_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VER_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_DIP_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_SIP_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE_E

     /* new field in Bobcat B0 , was not in Bobcat A0 */
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE_E

    /* new fields in BobK */
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2_E
    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3_E

    /* sip7 fields */
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_IPV4_MAC_SA_INDEX_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_IPV6_MAC_SA_INDEX_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_SEGMENTS_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_TAG0_QOS_MARKING_MODE_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_TAG1_QOS_MARKING_MODE_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_UP0_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_DEI0_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_UP1_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_DEI1_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_THREAD_ID_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_ANALYZER_INDEX_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_EGRESS_MIRROR_MODE_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_RAW_LABELS_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_MPLS_ETHERTYPE_SELECT_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_CNC_PTR_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_OR_GCF_PROFILE_ID_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_OR_GCF_BITS_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_LABELS_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_LAST_QOS_UNIFORM_LABEL_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_GLOBAL_EXP_MAPPING_TBL_INDEX_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_LAST_TTL_UNIFORM_LABEL_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_CW_INDEX_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_INSERTION_E
    ,SIP7_HA_TUNNEL_START_TABLE_FIELDS_MPLS_VPORT_LABELS_E

    ,SIP5_HA_TUNNEL_START_TABLE_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_TUNNEL_START_TABLE_FIELDS_ENT;

/* the fields of the (HA) tunnel start table */
typedef enum{
     SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TUNNEL_TYPE_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_MARKING_MODE_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_UP_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_TAG_ENABLE_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_VID_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_GENERIC_TUNNEL_START_TYPE_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RESERVED_1_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RETAIN_INNER_CRC_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RESERVED_2_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FILEDS_MAC_DA_E
    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_ETHER_TYPE_E

    /* sip7 fields */
    ,SIP7_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_RESERVED_3_E
    ,SIP7_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_MAC_SA_INDEX_E
    ,SIP7_HA_TUNNEL_START_TABLE_GENERIC_FIELDS_NUMBER_OF_SEGMENTS_E

    ,SIP6_HA_TUNNEL_START_TABLE_GENERIC_FIELDS____LAST_VALUE___E/* used for array size */
}SIP6_HA_TUNNEL_START_TABLE_FIELDS_ENT;

/* the fields of the (HA) NAT44 table */
typedef enum{
     SIP5_HA_NAT44_TABLE_FIELDS_MAC_DA_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_DIP_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_NEW_DIP_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_SIP_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_NEW_SIP_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_DST_PORT_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_DST_PORT_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_SRC_PORT_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_SRC_PORT_E
    ,SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_SIP_PREFIX_LENGTH_E
    ,SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_DIP_PREFIX_LENGTH_E
    ,SIP5_HA_NAT44_TABLE_FIELDS_NAT_ENTRY_TYPE_E

    ,SIP5_HA_NAT44_TABLE_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_NAT44_TABLE_FIELDS_ENT;

/* the fields of the (HA) NAT6 table */
typedef enum{
     SIP5_HA_NAT66_TABLE_FIELDS_MAC_DA_E
    ,SIP5_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND_E
    ,SIP5_HA_NAT66_TABLE_FIELDS_ADDRESS_E
    ,SIP5_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE_E
    ,SIP5_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE_E

    ,SIP5_HA_NAT66_TABLE_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_NAT66_TABLE_FIELDS_ENT;

/* the fields of the (HA) generic tunnel start profile table */
typedef enum{
    SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE0_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE1_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE2_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE3_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE4_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE5_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE6_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE7_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE8_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE9_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE10_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE11_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE12_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE13_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE14_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT0_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT1_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT2_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT3_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT4_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT5_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT6_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_BYTE15_BIT7_E

   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_UDP_SRC_PORT_MODE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_SERVICE_ID_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_MAC_DA_MODE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE_E
   ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_CONTROL_WORD_INDEX_E

    /* sip7 new fields */
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IPV6_SHR_ROUTING_TYPE_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IPV6_SHR_MODE_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_THREAD_ID_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IP_PROTOCOL_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_UDP_DST_PORT_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_GRE_PROTOCOL_E
    ,SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_SRV6_PACKET_EDITOR_MODE_E


    ,SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS____LAST_VALUE___E/* used for array size */
}SIP5_HA_GEN_TUNNEL_START_PROFILE_TABLE_FIELDS_ENT;


/* macro to set value to field of (HA) Tunnel Start format in buffer */
#define SIP5_HA_TUNNEL_START_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (HA) Tunnel Start format in buffer */
#define SIP5_HA_TUNNEL_START_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to set value to field of (HA) Tunnel Start format in buffer */
#define SIP6_HA_TUNNEL_START_GENERIC_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_GENERIC_TUNNEL_START_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (HA) Tunnel Start format in buffer */
#define SIP6_HA_TUNNEL_START_GENERIC_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_GENERIC_TUNNEL_START_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to set value to NEXT_HOP_MAC_DA field of (HA) Tunnel Start entry format in buffer */
#define SIP5_HA_TUNNEL_START_FIELD_NEXT_HOP_MAC_DA_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr,             \
        SIP5_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA_E,       \
        _macAddrBytes)

/* macro to get value to NEXT_HOP_MAC_DA field of (HA) Tunnel Start entry format in buffer */
#define SIP5_HA_TUNNEL_START_FIELD_NEXT_HOP_MAC_DA_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr,             \
        SIP5_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA_E,       \
        _macAddrBytes)

/* macro to set value to MAC_DA field of (HA) Generic Tunnel Start entry format in buffer */
#define SIP6_HA_TUNNEL_START_GENERIC_FIELD_MAC_DA_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_GENERIC_TUNNEL_START_E].fieldsInfoPtr,             \
        SIP6_HA_TUNNEL_START_TABLE_GENERIC_FILEDS_MAC_DA_E,       \
        _macAddrBytes)


/* macro to get value from MAC_DA field of (HA) Generic Tunnel Start entry format in buffer */
#define SIP6_HA_TUNNEL_START_GENERIC_FIELD_MAC_DA_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_GENERIC_TUNNEL_START_E].fieldsInfoPtr,             \
        SIP6_HA_TUNNEL_START_TABLE_GENERIC_FILEDS_MAC_DA_E,       \
        _macAddrBytes)

/* macro to set value to field of (HA) Generic Tunnel Start Profile format in buffer */
#define SIP5_HA_GEN_TUNNEL_START_PROFILE_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_GENERIC_TS_PROFILE_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (HA) Generic Tunnel Start Profile format in buffer */
#define SIP5_HA_GEN_TUNNEL_START_PROFILE_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_GENERIC_TS_PROFILE_E].fieldsInfoPtr,             \
        _fieldName,                                                 \
        _value)

/* macro to set value to field of (HA) NAT44 format in buffer */
#define SIP5_HA_NAT44_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (HA) NAT44 format in buffer */
#define SIP5_HA_NAT44_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to set value to MAC_DA field of (HA) NAT44 entry format in buffer */
#define SIP5_HA_NAT44_FIELD_MAC_DA_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E].fieldsInfoPtr,             \
        SIP5_HA_NAT44_TABLE_FIELDS_MAC_DA_E,       \
        _macAddrBytes)

/* macro to get value to MAC_DA field of (HA) NAT44 entry format in buffer */
#define SIP5_HA_NAT44_FIELD_MAC_DA_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT44_E].fieldsInfoPtr,             \
        SIP5_HA_NAT44_TABLE_FIELDS_MAC_DA_E,       \
        _macAddrBytes)

/* macro to set value to field of (HA) NAT66 format in buffer */
#define SIP5_HA_NAT66_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (HA) NAT66 format in buffer */
#define SIP5_HA_NAT66_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to set value to MAC_DA field of (HA) NAT66 entry format in buffer */
#define SIP5_HA_NAT66_FIELD_MAC_DA_SET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E].fieldsInfoPtr,             \
        SIP5_HA_NAT66_TABLE_FIELDS_MAC_DA_E,       \
        _macAddrBytes)

/* macro to get value to MAC_DA field of (HA) NAT66 entry format in buffer */
#define SIP5_HA_NAT66_FIELD_MAC_DA_GET_MAC(_dev,_hwDataArr,_macAddrBytes)   \
    MAC_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E].fieldsInfoPtr,             \
        SIP5_HA_NAT66_TABLE_FIELDS_MAC_DA_E,       \
        _macAddrBytes)

/* macro to set value to IPv6 address field of (HA) NAT66 entry format in buffer */
#define SIP5_HA_NAT66_FIELD_ADDRESS_SET_MAC(_dev,_hwDataArr,_ipv6AddrBytes)   \
    IPV6_ADDR_SET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E].fieldsInfoPtr,             \
        SIP5_HA_NAT66_TABLE_FIELDS_ADDRESS_E,       \
        _ipv6AddrBytes)

/* macro to get value to IPv6 address field of (HA) NAT66 entry format in buffer */
#define SIP5_HA_NAT66_FIELD_ADDRESS_GET_MAC(_dev,_hwDataArr,_ipv6AddrBytes)   \
    IPV6_ADDR_GET_FIELD_BY_ID_MAC(_hwDataArr,                           \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_NAT66_E].fieldsInfoPtr,             \
        SIP5_HA_NAT66_TABLE_FIELDS_ADDRESS_E,       \
        _ipv6AddrBytes)

#define SIP7_HA_HFT_FIELD_SET_MAC(_dev, _hwDataArr, _fieldName, _value)                             \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                                     \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_HEADER_FIELDS_E].fieldsInfoPtr, \
        _fieldName,                                                                                         \
        _value)

#define SIP7_HA_HFT_FIELD_GET_MAC(_dev, _hwDataArr, _fieldName, _value)                             \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                                     \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_HA_HEADER_FIELDS_E].fieldsInfoPtr, \
        _fieldName,                                                                                         \
        _value)

/*************************************************/
/********** HA UNIT END   ************************/
/*************************************************/

/**************************************************/
/********** EPCL UNIT START ***********************/
/**************************************************/

/**
* @enum SIP5_EPCL_ACTION_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EPCL Action table.
*/
typedef enum {
     SIP5_EPCL_ACTION_TABLE_FIELDS_COMMAND_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_DSCP_EXP_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_UP0_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_MODIFY_DSCP_EXP_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP0_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_CNC_COUNTER_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_CNC_COUNTER_INDEX_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_CMD_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_ENABLE_MODIFY_UP1_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TAG1_VID_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_UP1_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VLAN_CMD_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TAG0_VID_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_COUNTER_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_BIND_TO_POLICER_METER_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_POLICER_INDEX_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_RESERVED_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_FLOW_ID_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROCESSING_ENABLE_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_OAM_PROFILE_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_ENABLE_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TIME_STAMP_OFFSET_INDEX_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_CHANNEL_TYPE_TO_OPCODE_MAPPING_EN_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_ENABLE_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_EPCL_RESERVED_E
    ,SIP5_EPCL_ACTION_TABLE_FIELDS_TM_QUEUE_ID_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_ASSIGN_ENABLE_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_ASSIGN_ENABLE_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_PHA_THREAD_NUMBER_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_CPU_CODE_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_PROFILE_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_LATENCY_MONITORING_ENABLE_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_CUT_THROUGH_TERMINATE_ID_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_RESERVED_E
    ,SIP6_EPCL_ACTION_TABLE_FIELDS_PCL_OVER_EXACT_MATCH_ENABLE_E
    ,SIP6_10_EPCL_ACTION_TABLE_FIELDS_IPFIX_ENABLE_E
    ,SIP6_10_EPCL_ACTION_TABLE_FIELDS_PHA_METADATA_PART2_E
    ,SIP6_30_EPCL_ACTION_TABLE_FIELDS_EGRESS_CNC_INDEX_MODE_E
    ,SIP6_30_EPCL_ACTION_TABLE_FIELDS_ENABLE_EGRESS_MAX_SDU_SIZE_CHECK_E
    ,SIP6_30_EPCL_ACTION_TABLE_FIELDS_EGRESS_MAX_SDU_SIZE_PROFILE_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_OAM_PTP_OFFSET_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_GEN_CLASSIFY_ASSIGN_CMD_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_GEN_CLASSIFY_VALUE_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_TCAM_PRIORITY_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_CANCEL_DROP_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_EGRESS_FORWARD_CONTROL_ENABLE_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_EGRESS_FORWARD_LOOPBACK_SELECTOR_E
    ,SIP7_EPCL_ACTION_TABLE_FIELDS_EGRESS_FORWARD_DISABLE_E

    ,SIP5_EPCL_ACTION_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_EPCL_ACTION_TABLE_FIELDS_ENT;

/**************************************************/
/********** EPCL UNIT END *************************/
/**************************************************/

/**************************************************/
/********** IPvX UNIT START ***********************/
/**************************************************/

/**
* @enum SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of IPv4 Multicast Nexthop entry.
*/
typedef enum {
     SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COMMAND_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_BYPASS_TTL_OPTIONS_OR_HOP_EXTENSION_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_CPU_CODE_INDEX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INGRESS_MIRROR_TO_ANALYZER_INDEX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_PRECEDENCE_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_MARKING_EN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_QOS_PROFILE_INDEX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_DSCP_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MODIFY_UP_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MC_RPF_EVLAN_MRST_INDEX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_28_34_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_35_47_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_FAIL_CMD_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_RPF_FAIL_CMD_MODE_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_52_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_INTERNAL_MLL_PTR_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_MSB_BITS_15_13_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_72_74_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MULTICAST_INGRESS_VLAN_CHECK_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EXTERNAL_MLL_PTR_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_RESERVED_90_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_DEST_SITE_ID_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_IPV6_MULTICAST_SCOPE_CHECK_EN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_VID1_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_21_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_NEXT_HOP_EVLAN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_USE_VIDX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_EVIDX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_52_59_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_TRUNK_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_EPORT_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_DEV_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TRG_TRUNK_ID_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_49_59_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DIP_ACCESS_LEVEL_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_ACCESS_LEVEL_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_START_OF_TUNNEL_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_PTR_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TUNNEL_PTR_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_88_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TS_IS_NAT_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_TUNNEL_TYPE_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_DEST_SITE_ID_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_UNICAST_SCOPE_CHECK_EN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_COUNTER_SET_INDEX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_MTU_INDEX_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ARP_BC_TRAP_MIRROR_EN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_APP_SPECIFIC_CPU_CODE_EN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_RESERVED_101_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_SIP_FILTER_EN_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_DEC_TTL_OR_HOP_COUNT_E
    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ICMP_REDIRECT_EXCEP_MIRROR_E
    ,SIP6_10_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_EPG_E

    ,SIP7_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_IS_VPORT_E
    ,SIP7_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_VPORT_E
    ,SIP7_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_TARGET_NHE_ROW_MODE

    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_PROFILE_NUMBER_0_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_TERGET_VPORT_0_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_PROFILE_NUMBER_1_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_TERGET_VPORT_1_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_PROFILE_NUMBER_2_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_TERGET_VPORT_2_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_PROFILE_NUMBER_3_MODE
    ,SIP7_IPVX_NEXT_HOP_TABLE_FIELDS_L3NHE_ENTRY_TERGET_VPORT_3_MODE

    ,SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_IPVX_ROUTER_NEXTHOP_TABLE_FIELDS_ENT;

/* macro to set value to field of IPvX entry format in buffer */
#define SIP5_IPVX_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_IPVX_ROUTER_NEXT_HOP_E].fieldsInfoPtr,                     \
        _fieldName,                                                 \
        _value)

/* macro to get value to field of IPvX entry format in buffer */
#define SIP5_IPVX_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_IPVX_ROUTER_NEXT_HOP_E].fieldsInfoPtr,                     \
        _fieldName,                                                 \
        _value)

/**
* @enum SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_ENT
 *
 * @brief Enumeration to hold fields of Router ePort table entry.
*/
typedef enum {
    SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_ROUTER_PER_EPORT_SIP_SA_CHECK_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_FCOE_FORWARDING_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV4_UC_ROUTING_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV4_MC_ROUTING_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV6_UC_ROUTING_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_IPV6_MC_ROUTING_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_URPF_ENABLE_E
   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_DISABLE_SIP_LOOKUP

   ,SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_IPVX_ROUTER_EPORT_TABLE_FIELDS_ENT;


/**************************************************/
/********** IPvX UNIT END *************************/
/**************************************************/

/**************************************************/
/********** PLR UNIT START ************************/
/**************************************************/
/* the fields of the (PLR) policer metering (IPLR0,1,EPLR) in Sip5 */
/* NOTE: in sip5_15 this table called 'Metering Token Bucket Entry' */
typedef enum{
     SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME0_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_LAST_UPDATE_TIME1_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR0_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_WRAP_AROUND_INDICATOR1_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE0_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_RATE_TYPE1_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_RATE0_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_RATE1_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE0_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE1_E
    ,SIP5_PLR_METERING_TABLE_FIELDS_COLOR_MODE_E                               /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_POLICER_MODE_E                             /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_MG_COUNTERS_SET_EN_E                       /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_BILLING_PTR_E                              /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E             /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_PACKET_SIZE_MODE_E                         /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E      /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E                  /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E                    /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E              /* in sip5_15 moved to 'Metering Configuration Entry' */
    /* ingress only fields*/
    ,SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_RED_CMD_E                          /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_YELLOW_CMD_E                       /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_GREEN_CMD_E                        /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E                      /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_MODIFY_UP_E                        /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_INGRESS_QOS_PROFILE_E                      /* in sip5_15 moved to 'Metering Configuration Entry' */
    /* egress only fields*/
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_REMARK_MODE_E                       /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_DROP_RED_E                          /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E                     /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E                    /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E                      /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E                      /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E                      /* in sip5_15 moved to 'Metering Configuration Entry' */
    ,SIP5_PLR_METERING_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E             /* in sip5_15 moved to 'Metering Configuration Entry' */

    /* sip5_15 only fields */
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_PRIORITY_E
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_PRIORITY_E
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_0_COLOR_E
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_BUCKET_1_COLOR_E
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_E
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_COUPLING_FLAG_0_E
    ,SIP5_15_PLR_METERING_TABLE_FIELDS_EIR_AND_CIR_MAX_INDEX_E

    /* NOTE: this enum is emulating field that exists in 'meter config' entry
        and is used to 'virtualization' of single entry ... although actual info
        split to several tables
    */

    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E___EMULATION_ONLY_E

    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN___EMULATION_ONLY_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN___EMULATION_ONLY_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN___EMULATION_ONLY_E

    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN___EMULATION_ONLY_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN___EMULATION_ONLY_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN___EMULATION_ONLY_E

    ,SIP5_PLR_METERING_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_PLR_METERING_TABLE_FIELDS_ENT;

/* INTERNAL : macro to set value to field of PLR meter entry format in buffer */
#define SIP5_PLR_METER_FIELD_SET_MAC(_dev,_hwMeterArr,_hwMeterConfigArr,_fieldName,_value)    \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssDxChTablePlrMeterFieldBuild(_dev,_hwMeterArr,_hwMeterConfigArr,_fieldName,_value); \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}

/* INTERNAL : macro to get value to field of PLR meter entry format in buffer */
#define SIP5_PLR_METER_FIELD_GET_MAC(_dev,_hwMeterArr,_hwMeterConfigArr,_fieldName,_value)    \
{                                                                               \
    GT_STATUS _rc;                                                              \
    _rc = prvCpssDxChTablePlrMeterFieldParse(_dev,_hwMeterArr,_hwMeterConfigArr,_fieldName,&_value); \
    if (_rc != GT_OK)                                                           \
    {                                                                           \
        return _rc;                                                             \
    }                                                                           \
}


/* INTERNAL : macro to set value to field of PLR meter entry format in buffer */
#define __SIP5_PLR_METER_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* INTERNAL : macro to get value to field of PLR meter entry format in buffer */
#define __SIP5_PLR_METER_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* the fields of the (PLR) policer Metering Configuration Entry (IPLR0,1,EPLR) in Sip5_15 */
typedef enum{
     SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_COLOR_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_POLICER_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_MG_COUNTERS_SET_EN_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BILLING_PTR_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_BYTE_OR_PACKET_COUNTING_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_PACKET_SIZE_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E
    /* ingress only fields*/
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_CMD_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_CMD_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_CMD_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_DSCP_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MODIFY_UP_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_QOS_PROFILE_E
    /* egress only fields*/
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_REMARK_MODE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_DROP_RED_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_EXP_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DSCP_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_TC_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_UP_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_MODIFY_DP_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_EN_YELLOW_ECN_MARKING_E

    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_MEF_10_3_ENV_SIZE_E
    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_MEF_10_3_ENV_SIZE_E

    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_GREEN_MIRROR_EN_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_YELLOW_MIRROR_EN_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_INGRESS_RED_MIRROR_EN_E

    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_GREEN_MIRROR_EN_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_YELLOW_MIRROR_EN_E
    ,SIP_6_PLR_METERING_CONFIG_TABLE_FIELDS_EGRESS_RED_MIRROR_EN_E

    ,SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_ENT;

/* the fields of the (PLR) policer Metering Conformance Sign Entry (IPLR0,1,EPLR) in Sip5_15 */
typedef enum{
     PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_BUCKET0_SIGN_E
    ,PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_BUCKET1_SIGN_E

    ,PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}PRV_CPSS_DXCH_PLR_METERING_CONFORM_SIGN_TABLE_FIELDS_ENT;

typedef enum{
     SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_TD_DYNAMIC_FACTOR_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_TD_GUARANTIED_LIMIT_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_TD_DROP_PROBABILITY_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_TD_WRED_SIZE_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_PORT_QCN_DYNAMIC_FACTOR_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_QCN_LIMIT_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_CN_AWARE_E
    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_PORT_STATUS_LIMIT_E
    ,SIP7_PREQ_PORT_PROFILE_TABLE_FIELDS_PORT_EXP_E

    ,SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
} SIP6_PREQ_PORT_PROFILE_TABLE_FIELDS_ENT;

typedef enum{
     SIP6_PREQ_PROFILES_TABLE_FIELDS_QUEUE_MAX_LIMIT_E
    ,SIP6_PREQ_PROFILES_TABLE_FIELDS_QUEUE_QCN_DYNAMIC_FACTOR_E
    ,SIP6_PREQ_PROFILES_TABLE_FIELDS_QCN_LIMIT_E
    ,SIP6_PREQ_PROFILES_TABLE_FIELDS_CN_AWARE_E
    ,SIP6_PREQ_PROFILES_TABLE_FIELDS_TELEMETRY_SELECT_E
    ,SIP6_PREQ_PROFILES_TABLE_FIELDS_QUEUE_STATUS_LIMIT_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_QUEUE_EXP_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_QUEUE_POOL_LIMIT_EN_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_QUEUE_CN_RATE_GUARANTEED_LIMIT_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_QUEUE_CN_RATE_DYNAMIC_FACTOR_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_QUEUE_CN_RATE_MAX_LIMIT_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_PDX_BURST_FIFO_GUARANTEED_LIMIT_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_PDX_BURST_FIFO_DYNAMIC_FACTOR_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_ECN_OFFSET_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_WRED_SIZE_0_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_WRED_SIZE_1_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_WRED_SIZE_2_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_DROP_PROBABILITY_0_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_DROP_PROBABILITY_1_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_DROP_PROBABILITY_2_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_GUARANTEED_LIMIT_0_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_GUARANTEED_LIMIT_1_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_GUARANTEED_LIMIT_2_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_DYNAMIC_FACTOR_0_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_DYNAMIC_FACTOR_1_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_TD_DYNAMIC_FACTOR_2_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_RATE_POSITIVE_THRESHOLD_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_LATENCY_DROP_RATE_THRESHOLD_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_SCALE_LOGIC_EN_E
    ,SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_ECN_ENABLE_E

    ,SIP6_PREQ_PROFILES_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP6_PREQ_PROFILES_TABLE_FIELDS_ENT;

typedef enum{
     SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS_TD_DYNAMIC_FACTOR_E
    ,SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS_TD_GUARANTIED_LIMIT_E
    ,SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS_TD_DROP_PROBABILITY_E
    ,SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS_TD_WRED_SIZE_E
    ,SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS_ECN_ENABLE_E

    ,SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP6_PREQ_QUEUE_CONFIGURATIONS_TABLE_FIELDS_ENT;

typedef enum{
     SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_PASS_DROP_PORT_BASE_E
    ,SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_STATUS_PORT_BASE_E

    ,SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP6_PREQ_QUEUE_PORT_MAPPING_TABLE_FIELDS_ENT;

typedef enum{
     SIP6_PREQ_QUEUE_PRIO_MAPPING_TABLE_FIELDS_QUEUE_PRIO_PROFILE_E

    ,SIP6_PREQ_QUEUE_PRIO_MAPPING_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP6_PREQ_QUEUE_PRIO_MAPPING_TABLE_FIELDS_ENT;

typedef enum{
     SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_VLAN_COUNTING_ENABLE_E
    ,SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_EGRESS_MIRRORING_CODE_E
    ,SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_EGRESS_ANALYZER_INDEX_E
    ,SIP6_30_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_SRF_TRG_PHY_PORT_E

    ,SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E/* used for array size */
}SIP6_PREQ_TARGET_PHYSICAL_PORT_TABLE_FIELDS_ENT;


/**
* @enum SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream Mapping table.
*/
typedef enum{
     SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E
    ,SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E
    ,SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E
    ,SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_E

    ,SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_ENT;

/* macro to set value to field of (PREQ) SRF mapping entry format in buffer */
#define SIP6_30_PREQ_SRF_MAPPING_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_MAPPING_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (PREQ) SRF mapping entry format in buffer */
#define SIP6_30_PREQ_SRF_MAPPING_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_MAPPING_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/**
* @enum SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream config table.
*/
typedef enum{
     SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E
    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E

    ,SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ENT;

/* macro to set value to field of (PREQ) SRF config entry format in buffer */
#define SIP6_30_PREQ_SRF_CONFIG_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_CONFIG_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (PREQ) SRF config entry format in buffer */
#define SIP6_30_PREQ_SRF_CONFIG_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_CONFIG_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/**
* @enum SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream counters table.
*/
typedef enum{
     SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E
    ,SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E
    ,SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E
    ,SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E
    ,SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E
    ,SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E

    ,SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_ENT;

/* macro to get field value of (PREQ) SRF counters entry format in buffer */
#define SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_COUNTERS_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/**
* @enum SIP6_30_PREQ_SRF_CONFIG_DAEMON_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream daemon table.
*/
typedef enum{
     SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E
    ,SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E
    ,SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E
    ,SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_DIFFERENCE_E
    ,SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E
    ,SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E

    ,SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_ENT;

/* macro to set value to field of (PREQ) SRF daemon entry format in buffer */
#define SIP6_30_PREQ_SRF_DAEMON_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_DAEMON_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (PREQ) SRF daemon entry format in buffer */
#define SIP6_30_PREQ_SRF_DAEMON_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PREQ_SRF_DAEMON_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/* the fields of the PPU action table in Sip6.10 */
typedef enum{
    /* ROT 0 */
    SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SRC_REG_SEL_E

    /** When srcRegSel == _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SRC_CMD_4BYTE_E

    /** when srcRegSel != _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SRC_OFFSET_E
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SRC_NUM_VAL_BITS_E

        /** when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SHIFT_LEFT_RIGHT_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SHIFT_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SET_BITS_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SET_BITS_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_SET_BITS_OFFSET_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_ADD_SUB_CONST_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_ADD_SUB_CONST_SEL_E

        /** when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_CMP_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_CMP_FALSE_VAL_LD_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_CMP_COND_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_CMP_MASK_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_CMP_TRUE_VAL_LD_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_FUNC_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_FUNC_SECOND_OPERAND_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_TARGET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_LD_SP_BUS_NUM_BYTES_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_LD_SP_BUS_OFFSET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_INTERRUPT_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_0_INTERRUPT_INDEX_E

    /* ROT 1 */
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SRC_REG_SEL_E

    /** When srcRegSel == _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SRC_CMD_4BYTE_E

    /** when srcRegSel != _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SRC_OFFSET_E
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SRC_NUM_VAL_BITS_E

        /** when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SHIFT_LEFT_RIGHT_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SHIFT_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SET_BITS_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SET_BITS_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_SET_BITS_OFFSET_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_ADD_SUB_CONST_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_ADD_SUB_CONST_SEL_E

        /** when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_CMP_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_CMP_FALSE_VAL_LD_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_CMP_COND_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_CMP_MASK_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_CMP_TRUE_VAL_LD_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_FUNC_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_FUNC_SECOND_OPERAND_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_TARGET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_LD_SP_BUS_NUM_BYTES_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_LD_SP_BUS_OFFSET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_INTERRUPT_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_1_INTERRUPT_INDEX_E

    /* ROT 2 */
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SRC_REG_SEL_E

    /** When srcRegSel == _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SRC_CMD_4BYTE_E

    /** when srcRegSel != _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SRC_OFFSET_E
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SRC_NUM_VAL_BITS_E

        /** when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SHIFT_LEFT_RIGHT_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SHIFT_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SET_BITS_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SET_BITS_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_SET_BITS_OFFSET_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_ADD_SUB_CONST_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_ADD_SUB_CONST_SEL_E

        /** when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_CMP_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_CMP_FALSE_VAL_LD_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_CMP_COND_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_CMP_MASK_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_CMP_TRUE_VAL_LD_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_FUNC_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_FUNC_SECOND_OPERAND_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_TARGET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_LD_SP_BUS_NUM_BYTES_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_LD_SP_BUS_OFFSET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_INTERRUPT_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_2_INTERRUPT_INDEX_E

    /* ROT 3 */
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SRC_REG_SEL_E

    /** When srcRegSel == _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SRC_CMD_4BYTE_E

    /** when srcRegSel != _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SRC_OFFSET_E
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SRC_NUM_VAL_BITS_E

        /** when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SHIFT_LEFT_RIGHT_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SHIFT_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SET_BITS_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SET_BITS_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_SET_BITS_OFFSET_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_ADD_SUB_CONST_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_ADD_SUB_CONST_SEL_E

        /** when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_CMP_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_CMP_FALSE_VAL_LD_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_CMP_COND_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_CMP_MASK_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_CMP_TRUE_VAL_LD_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_FUNC_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_FUNC_SECOND_OPERAND_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_TARGET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_LD_SP_BUS_NUM_BYTES_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_LD_SP_BUS_OFFSET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_INTERRUPT_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_3_INTERRUPT_INDEX_E

    /* ROT 4 */
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SRC_REG_SEL_E

    /** When srcRegSel == _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SRC_CMD_4BYTE_E

    /** when srcRegSel != _4BYTE_SET_CMD */
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SRC_OFFSET_E
        ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SRC_NUM_VAL_BITS_E

        /** when srcRegSel != _4BYTE_SET_CMD and func != CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SHIFT_LEFT_RIGHT_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SHIFT_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SET_BITS_NUM_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SET_BITS_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_SET_BITS_OFFSET_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_ADD_SUB_CONST_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_ADD_SUB_CONST_SEL_E

        /** when srcRegSel != _4BYTE_SET_CMD and func == CMP_REG */
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_CMP_VAL_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_CMP_FALSE_VAL_LD_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_CMP_COND_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_CMP_MASK_E
            ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_CMP_TRUE_VAL_LD_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_FUNC_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_FUNC_SECOND_OPERAND_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_TARGET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_LD_SP_BUS_NUM_BYTES_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_LD_SP_BUS_OFFSET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_INTERRUPT_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_ROT_4_INTERRUPT_INDEX_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_SET_NEXT_STATE_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_SET_CONST_NEXT_SHIFT_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_SET_LOOPBACK_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_COUNTER_SET_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_COUNTER_SET_VAL_E
    ,SIP6_10_PPU_ACTION_TABLE_FIELDS_COUNTER_OPER_E

    ,SIP6_10_PPU_ACTION_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_10_PPU_ACTION_TABLE_FIELDS_ENT;

/* fields in the PPU DAU profile table in Sip6_10 */
typedef enum{
    /* Byte 0 */
     SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_0_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_0_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_0_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_0_TARGET_OFFSET_E

    /* Byte 1 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_1_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_1_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_1_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_1_TARGET_OFFSET_E

    /* Byte 2 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_2_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_2_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_2_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_2_TARGET_OFFSET_E

    /* Byte 3 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_3_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_3_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_3_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_3_TARGET_OFFSET_E

    /* Byte 4 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_4_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_4_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_4_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_4_TARGET_OFFSET_E

    /* Byte 5 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_5_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_5_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_5_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_5_TARGET_OFFSET_E

    /* Byte 6 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_6_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_6_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_6_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_6_TARGET_OFFSET_E

    /* Byte 7 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_7_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_7_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_7_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_7_TARGET_OFFSET_E

    /* Byte 8 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_8_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_8_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_8_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_8_TARGET_OFFSET_E

    /* Byte 9 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_9_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_9_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_9_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_9_TARGET_OFFSET_E

    /* Byte 10 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_10_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_10_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_10_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_10_TARGET_OFFSET_E

    /* Byte 11 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_11_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_11_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_11_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_11_TARGET_OFFSET_E

    /* Byte 12 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_12_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_12_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_12_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_12_TARGET_OFFSET_E

    /* Byte 13 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_13_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_13_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_13_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_13_TARGET_OFFSET_E

    /* Byte 14 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_14_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_14_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_14_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_14_TARGET_OFFSET_E

    /* Byte 15 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_15_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_15_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_15_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_15_TARGET_OFFSET_E

    /* Byte 16 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_16_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_16_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_16_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_16_TARGET_OFFSET_E

    /* Byte 17 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_17_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_17_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_17_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_17_TARGET_OFFSET_E

    /* Byte 18 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_18_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_18_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_18_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_18_TARGET_OFFSET_E

    /* Byte 19 */
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_19_WR_EN_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_19_NUM_BITS_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_19_SRC_OFFSET_E
    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_SP_BYTE_19_TARGET_OFFSET_E

    ,SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_10_PPU_DAU_PROFILE_TABLE_FIELDS_ENT;

/* INTERNAL : macro to set value to field of PLR meter config entry format in buffer */
#define __SIP5_15_PLR_METER_CONFIG_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFIG_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* INTERNAL : macro to get value to field of PLR meter config entry format in buffer */
#define __SIP5_15_PLR_METER_CONFIG_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_PLR_METER_CONFIG_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/**************************************************/
/********** SMU UNIT START ************************/
/**************************************************/
/**
* @enum SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU IRF SNG table.
*/
typedef enum{
     SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_INDIVIDUAL_RECOVERY_ENABLED_E
    ,SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E
    ,SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_TAKE_ANY_E
    ,SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E
    ,SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E
    ,SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E

    ,SIP6_30_SMU_IRF_SNG_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_ENT;

/* macro to set value to field of (SMU) IRF/SNG entry format in buffer */
#define SIP6_30_SMU_IRF_SNG_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_SNG_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)


/* macro to get field value of (SMU) IRF/SNG entry format in buffer */
#define SIP6_30_SMU_IRF_SNG_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_SNG_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/**
 * @enum SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU IRF counters table.
*/
typedef enum{
     SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E
    ,SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E
    ,SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_OUT_OF_ORDER_PACKETS_COUNTER_E
    ,SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_NUMBER_OF_TAGLESS_PACKETS_COUNTER_E
    ,SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E

    ,SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_ENT;

/* macro to get field value of (SMU) IRF counters entry format in buffer */
#define SIP6_30_SMU_IRF_COUNTERS_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_IRF_COUNTERS_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/**
* @enum SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Table Set Time Configurations table.
*/
typedef enum{
     SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TOD_OFFSET_E
    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TIMESTAMP_OFFSET_E
    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_CYCLE_TIME_E
    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_FACTOR_E
    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_E
    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_POLARITY_E
    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_RESOLUTION_E

    ,SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_ENT;

/* macro to set value to field of (SMU) SGC Table Set Time Configurations entry format in buffer */
#define SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (SMU) SGC Table Set Time Configurations entry format in buffer */
#define SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/**
* @enum SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Time Slot Attributes table.
*/
typedef enum{
     SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_0_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_0_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_0_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_0_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_0_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_1_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_1_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_1_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_1_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_1_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_2_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_2_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_2_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_2_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_2_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_3_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_3_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_3_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_3_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_3_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_4_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_4_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_4_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_4_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_4_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_5_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_5_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_5_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_5_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_5_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_6_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_6_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_6_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_6_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_6_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_7_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_7_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_7_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_7_E
    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_7_E

    ,SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_ENT;

/* macro to set value to field of (SMU) SGC Time Slot Attributes entry format in buffer */
#define SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/* macro to get field value of (SMU) SGC Time Slot Attributes entry format in buffer */
#define SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)    \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                             \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].fieldsInfoPtr, \
        _fieldName,                                                 \
        _value)

/**************************************************/
/********** SMU UNIT END   ************************/
/**************************************************/


/**
* @enum SIP5_OAM_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of OAM table (ingress/egress OAM).
*/
typedef enum{
     SIP5_OAM_TABLE_FIELDS_OAM_OPCODE_PARSING_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_LM_COUNTER_CAPTURE_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_DUAL_ENDED_LM_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_LM_COUNTING_MODE_E
    ,SIP5_OAM_TABLE_FIELDS_MEG_LEVEL_CHECK_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_MEG_LEVEL_E
    ,SIP5_OAM_TABLE_FIELDS_KEEPALIVE_AGING_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_AGE_STATE_E
    ,SIP5_OAM_TABLE_FIELDS_AGING_PERIOD_INDEX_E
    ,SIP5_OAM_TABLE_FIELDS_AGING_THRESHOLD_E
    ,SIP5_OAM_TABLE_FIELDS_HASH_VERIFY_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_LOCK_HASH_VALUE_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_FLOW_HASH_E
    ,SIP5_OAM_TABLE_FIELDS_OAM_PTP_OFFSET_INDEX_E
    ,SIP5_OAM_TABLE_FIELDS_TIMESTAMP_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_PACKET_COMMAND_PROFILE_E
    ,SIP5_OAM_TABLE_FIELDS_CPU_CODE_OFFSET_E
    ,SIP5_OAM_TABLE_FIELDS_SOURCE_INTERFACE_CHECK_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_SOURCE_IS_TRUNK_E
    ,SIP5_OAM_TABLE_FIELDS_SOURCE_INTERFACE_CHECK_MODE_E
    ,SIP5_OAM_TABLE_FIELDS_ORIG_DEV_SRC_EPORT_OR_ORIG_DEV_SRC_TRUNK_E
    ,SIP5_OAM_TABLE_FIELDS_ENABLE_PROTECTION_LOC_UPDATE_E
    ,SIP5_OAM_TABLE_FIELDS_EXCESS_KEEPALIVE_DETECTION_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_EXCESS_KEEPALIVE_PERIOD_COUNTER_E
    ,SIP5_OAM_TABLE_FIELDS_EXCESS_KEEPALIVE_PERIOD_THRESHOLD_E
    ,SIP5_OAM_TABLE_FIELDS_EXCESS_KEEPALIVE_MESSAGE_COUNTER_E
    ,SIP5_OAM_TABLE_FIELDS_EXCESS_KEEPALIVE_MESSAGE_THRESHOLD_E
    ,SIP5_OAM_TABLE_FIELDS_RDI_CHECK_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_RDI_STATUS_E
    ,SIP5_OAM_TABLE_FIELDS_PERIOD_CHECK_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_KEEPALIVE_TX_PERIOD_FIELD_E
    ,SIP5_OAM_TABLE_FIELDS_PROTECTION_ON_RDI_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_PROTECTION_RDI_STATE_E
    ,SIP5_OAM_TABLE_FIELDS_IGNORE_BAD_KEEPALIVE_IN_AGING_EN_E
    ,SIP5_OAM_TABLE_FIELDS_LOC_DETECTION_ENABLE_E
    ,SIP5_OAM_TABLE_FIELDS_TX_PERIOD_STORM_SUPPRESS_EN_E
    ,SIP5_OAM_TABLE_FIELDS_INVALID_HASH_KEEPALIVE_STORM_SUPPRESS_EN_E
    ,SIP5_OAM_TABLE_FIELDS_SOURCE_INTERFACE_STORM_SUPPRESS_EN_E
    ,SIP5_OAM_TABLE_FIELDS_MEG_LEVEL_STORM_SUPPRESS_EN_E
    ,SIP5_OAM_TABLE_FIELDS_KEEPALIVE_AGING_STORM_SUPPRESS_EN_E
    ,SIP5_OAM_TABLE_FIELDS_HASH_VERIFICATION_STATUS_E
    ,SIP5_OAM_TABLE_FIELDS_TX_PERIOD_STATUS_E
    ,SIP5_OAM_TABLE_FIELDS_SOURCE_INTERFACE_STATUS_E
    ,SIP5_OAM_TABLE_FIELDS_MEG_LEVEL_STATUS_E
    ,SIP5_OAM_TABLE_FIELDS_KEEPALIVE_AGING_STATUS_E
    ,SIP7_OAM_TABLE_FIELDS_KEEPALIVE_RECOVERY_IRQ_EN_E

    ,SIP7_OAM_TABLE_FIELDS_SRC_IS_VPORT_E
    ,SIP7_OAM_TABLE_FIELDS_SRC_VPORT_E

    ,SIP5_OAM_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SIP5_OAM_TABLE_FIELDS_ENT;

/* macro to set value to field of OAM entry format in buffer */
#define SIP5_OAM_ENTRY_FIELD_SET_MAC(_dev,_hwDataArr,_fieldName,_value)                 \
    U32_SET_FIELD_BY_ID_MAC(_hwDataArr,                                                 \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_OAM_E].fieldsInfoPtr,    \
        _fieldName,                                                                     \
        _value)

/* macro to get field value of OAM entry format in buffer */
#define SIP5_OAM_ENTRY_FIELD_GET_MAC(_dev,_hwDataArr,_fieldName,_value)                 \
    U32_GET_FIELD_BY_ID_MAC(_hwDataArr,                                                 \
        PRV_TABLE_FORMAT_INFO(_dev)[PRV_CPSS_DXCH_TABLE_FORMAT_OAM_E].fieldsInfoPtr,    \
        _fieldName,                                                                     \
        _value)

/**
* @internal prvCpssDxChReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssDxChReadTableEntry
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    OUT GT_U32                 *entryValuePtr
);

/**
* @internal prvCpssDxChWriteTableEntry function
* @endinternal
*
* @brief   Write a whole entry to the table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChWriteTableEntry
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

/**
* @internal prvCpssDxChReadTableEntryField function
* @endinternal
*
* @brief   Read a field from the table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
*
* @param[out] fieldValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChReadTableEntryField
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                   fieldWordNum,
    IN GT_U32                   fieldOffset,
    IN GT_U32                   fieldLength,
    OUT GT_U32                 *fieldValuePtr
);

/**
* @internal prvCpssDxChWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChWriteTableEntryField
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                   fieldWordNum,
    IN GT_U32                   fieldOffset,
    IN GT_U32                   fieldLength,
    IN GT_U32                   fieldValue
);

/**
* @internal prvCpssDxChTableNumEntriesGet function
* @endinternal
*
* @brief   get the number of entries in a table
*         needed for debug purpose
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Bobcat3; Aldrin; Aldrin2; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] numEntriesPtr            - (pointer to) number of entries in the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTableNumEntriesGet
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    OUT GT_U32                  *numEntriesPtr
);

/**
* @internal prvCpssDxChTableEntrySizeGet function
* @endinternal
*
* @brief   get the entry size in 32 bit words
*         needed for debug purpose
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Bobcat3; Aldrin; Aldrin2; AC3X; Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] entrySizePtr            - (pointer to) entry size in 32 bit words
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTableEntrySizeGet
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    OUT GT_U32                  *entrySizePtr
);

/**
* @internal prvCpssDxChTablesAccessInit function
* @endinternal
*
* @brief   Initializes all structures for table access.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] regularInit              - GT_TRUE normal init procedure
*                                       GT_FALSE short init procedure,
* @param[in] initTti              - GT_TRUE TTI HW is initialized with default values
*                                       GT_FALSE skip TTI HW initialization  with default values

*                                                fine tuning is not available.
*/
GT_STATUS  prvCpssDxChTablesAccessInit
(
    IN GT_U8    devNum,
    IN GT_BOOL  regularInit,
    IN GT_BOOL  initTti
);

/**
* @internal prvCpssDxChPortGroupWriteTableEntryFieldList function
* @endinternal
*
* @brief   Write a list of fields to the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - HW table Id
* @param[in] entryIndex               - entry Index
* @param[in] entryMemoBufArr[]        - the work memory for read, update and write the entry
* @param[in] fieldsAmount             - amount of updated fields in the entry
* @param[in] fieldOffsetArr[]         - (array) the offset of the field in bits
*                                      from the entry origin
* @param[in] fieldLengthArr[]         - (array) the length of the field in bits,
*                                      0 - means to skip this subfield
* @param[in] fieldValueArr[]          - (array) the value of the field
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryFieldList
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN CPSS_DXCH_TABLE_ENT                tableType,
    IN GT_U32                                 entryIndex,
    IN GT_U32                                 entryMemoBufArr[],
    IN GT_U32                                 fieldsAmount,
    IN GT_U32                                 fieldOffsetArr[],
    IN GT_U32                                 fieldLengthArr[],
    IN GT_U32                                 fieldValueArr[]
);

/**
* @internal prvCpssDxChPortGroupReadTableEntryFieldList function
* @endinternal
*
* @brief   Read a list of fields from the table.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableId                  - HW table Id
* @param[in] entryIndex               - entry Index
* @param[in] entryMemoBufArr[]        - the work memory for read, update and write the entry
* @param[in] fieldsAmount             - amount of updated fields in the entry
* @param[in] fieldOffsetArr[]         - (array) the offset of the field in bits
*                                      from the entry origin
* @param[in] fieldLengthArr[]         - (array) the length of the field in bits,
*                                      0 - means to skip this subfield
*
* @param[out] fieldValueArr[]          - (array) the value of the field
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - one of the input parameters is not valid.
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntryFieldList
(
    IN  GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN  CPSS_DXCH_TABLE_ENT                tableId,
    IN  GT_U32                                 entryIndex,
    IN  GT_U32                                 entryMemoBufArr[],
    IN  GT_U32                                 fieldsAmount,
    IN  GT_U32                                 fieldOffsetArr[],
    IN  GT_U32                                 fieldLengthArr[],
    OUT GT_U32                                 fieldValueArr[]
);


/**
* @internal prvCpssDxChPortGroupWriteTableEntry function
* @endinternal
*
* @brief   Write (for specific portGroupId) a whole entry to the HW table.
*         If Shadow DB table exists it will be updated too.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntry
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr
);

/**
* @internal prvCpssDxChPortGroupWriteTableEntryMasked function
* @endinternal
*
* @brief   Either write a whole entry into HW table or update HW entry bits
*         specified by a mask. If Shadow DB table exists it will be updated too.
*         Works for specific portGroupId only.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask array.
*                                      Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryMasked
(
    IN GT_U8                    devNum,
    IN GT_U32                   portGroupId,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr
);

/**
* @internal prvCpssDxChWriteTableEntryMasked function
* @endinternal
*
* @brief   Either write a whole entry into HW table or update HW entry bits
*         specified by a mask. If Shadow DB table exists it will be updated too.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data that will be written to the table
*                                      may be NULL in the case of indirect table.
* @param[in] entryMaskPtr             - (pointer to) a mask array.
*                                      Ignored if entryValuePtr is NULL.
*                                      If NULL -  the entry specified by entryValuePtr will be
*                                      written to HW table as is.
*                                      If not NULL  - only bits that are raised in the mask
*                                      will be updated in the HW entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send write entry command to device. And entry is taken from the
*       data registers of a indirect table.
*
*/
GT_STATUS prvCpssDxChWriteTableEntryMasked
(
    IN GT_U8                    devNum,
    IN CPSS_DXCH_TABLE_ENT      tableType,
    IN GT_U32                   entryIndex,
    IN GT_U32                  *entryValuePtr,
    IN GT_U32                  *entryMaskPtr
);


/**
* @internal prvCpssDxChPortGroupReadTableEntry function
* @endinternal
*
* @brief   Read a whole entry from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] entryValuePtr            - (pointer to) the data read from the table
*                                      may be NULL in the case of indirect table.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In the case of entryValuePtr == NULL and indirect table the function
*       just send read entry command to device. And entry is stored in the
*       data registers of a indirect table
*
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntry
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                  entryIndex,
    OUT GT_U32                 *entryValuePtr
);

/**
* @internal prvCpssDxChPortGroupReadTableEntryField function
* @endinternal
*
* @brief   Read a field from the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
*
* @param[out] fieldValuePtr            - (pointer to) the data read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPortGroupReadTableEntryField
(
    IN GT_U8                  devNum,
    IN GT_U32                  portGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    OUT GT_U32                *fieldValuePtr
);

/**
* @internal prvCpssDxChPortGroupWriteTableEntryField function
* @endinternal
*
* @brief   Write a field to the table. - for specific portGroupId
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id , to support multi-port-group devices that need to access
*                                      specific port group
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
* @param[in] fieldWordNum             - field word number
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS
*                                      if need global offset in the field of fieldOffset
*                                      use PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      if need 'field name' in the field of fieldOffset
* @param[in] fieldOffset              - field offset
* @param[in] fieldLength              - field length
*                                      when fieldWordNum == PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS
*                                      the field 'fieldLength' is used ONLY if  it's value != 0
*                                      and then it means 'Mumber of consecutive fields' !!!
* @param[in] fieldValue               - the data write to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note In xCat A1 and above devices the data is updated only when the last
*       word in the entry was written.
*
*/
GT_STATUS prvCpssDxChPortGroupWriteTableEntryField
(
    IN GT_U8                  devNum,
    IN GT_U32                 portGroupId,
    IN CPSS_DXCH_TABLE_ENT tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                 fieldWordNum,
    IN GT_U32                 fieldOffset,
    IN GT_U32                 fieldLength,
    IN GT_U32                 fieldValue
);

/**
* @internal prvCpssDxChWriteTableMultiEntry function
* @endinternal
*
* @brief   Write number of entries to the table in consecutive indexes.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to write
* @param[in] entryValueArrayPtr       - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChWriteTableMultiEntry
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT          tableType,
    IN GT_U32                           startIndex,
    IN GT_U32                           numOfEntries,
    IN GT_U32                           *entryValueArrayPtr
);

/**
* @internal prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert function
* @endinternal
*
* @brief   check if the table entry is actually implemented as
*         'several entries' in single 'line'
*         NOTE: if the table is not of type 'multi entries' in line , the function
*         not update the INOUT parameters
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in,out] entryIndexPtr            - (pointer to) entry index , as 'unaware' that there are
*                                      multi entries in single line.
* @param[in,out] fieldOffsetPtr           - (pointer to) offset of the 'field' in the entry ,
*                                      as 'unaware' that there are multi entries in single line.
* @param[in,out] entryIndexPtr            - (pointer to) LINE index , as 'aware' that there are
*                                      multi entries in single line.
*                                      0x0FFFFFFF means impossible value!
* @param[in,out] fieldOffsetPtr           - (pointer to) offset of the 'field' in the LINE ,
*                                      as 'aware' that there are multi entries in single line.
*
* @param[out] numBitsPerEntryPtr       - (pointer to) the number of bits that the 'entry' is
*                                      using in the LINE.
*                                      NOTE: can be NULL
* @param[out] numEntriesPerLinePtr     - (pointer to) the number of entries in single LINE.
*                                      NOTE: can be NULL
*                                      NOTE: if value holds FRACTION_INDICATION_CNS meaning that the
*                                      number in the lower 31 bits is 'fraction' (1/x) and not (x)
*
* @retval GT_TRUE                  - convert done
* @retval GT_FALSE                 - no convert done
*/
GT_BOOL prvCpssDxChTableEngineMultiEntriesInLineIndexAndGlobalBitConvert
(
    IN GT_U8                   devNum,
    IN CPSS_DXCH_TABLE_ENT tableType,
    INOUT GT_U32               *entryIndexPtr,
    INOUT GT_U32               *fieldOffsetPtr,
    OUT GT_U32                 *numBitsPerEntryPtr,
    OUT GT_U32                 *numEntriesPerLinePtr
);


/**
* @internal prvCpssDxChReadTableMultiEntry function
* @endinternal
*
* @brief   Read number of entries from the table in consecutive indexes.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to read
*
* @param[out] entryValueArrayPtr       - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChReadTableMultiEntry
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_TABLE_ENT          tableType,
    IN  GT_U32                           startIndex,
    IN  GT_U32                           numOfEntries,
    OUT GT_U32                           *entryValueArrayPtr
);


/**
* @internal prvCpssDxChWriteTableEntry_indirect function
* @endinternal
*
* @brief   Write a whole entry to table by indirect access method.
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - the port group Id
* @param[in] tableType                - the specific table name
* @param[in] tableInfoPtr             - pointer to table information
* @param[in] entryIndex               - index in the table
* @param[in] entryValuePtr            - (pointer to) the data for writing into the table
* @param[in] shadowEntryMaskPtr       - (pointer to) a mask. Ignored if entryValuePtr is NULL.
*                                      Used for updating shadow table entry only.
*                                      If NULL -  the entry specified by entryValuePtr
*                                      will be written to a shadow table as is.
*                                      If not NULL  - only bits that are raised in the
*                                      mask will be updated in the shadow table entry.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChWriteTableEntry_indirect
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DXCH_TABLE_ENT              tableType,
    IN const PRV_CPSS_DXCH_TABLES_INFO_STC    *tableInfoPtr,
    IN GT_U32                           entryIndex,
    IN GT_U32                           *entryValuePtr,
    IN GT_U32                           *shadowEntryMaskPtr
);

/**
* @internal prvCpssDxChTableEntryIndirectPrevDoneCheck function
* @endinternal
*
* @brief   Check that previous access to indirect table done.
*          Function use busy-wait pooling till it done.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupId              - The port group id.
*                                      relevant only to 'multi-port-group' devices.
*                                      supports value CPSS_PORT_GROUP_UNAWARE_MODE_CNS
* @param[in] tableType                - the specific table name
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_TIMEOUT               - after max number of retries checking if PP ready
*/
GT_STATUS prvCpssDxChTableEntryIndirectPrevDoneCheck
(
    IN GT_U8                            devNum,
    IN GT_U32                           portGroupId,
    IN CPSS_DXCH_TABLE_ENT              tableType
);

/**
* @internal prvCpssDxChTableBaseAddrGet function
* @endinternal
*
* @brief   This function retrieves base address of table.(for 'Direct access' tables)
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The PP's device number to init the struct for.
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
*
* @param[out] errorPtr                 - (pointer to) indication that function did error. (can be NULL)
*
* @retval on success               - return the base address
* @retval on error                 - return address that ((address % 4) != 0)
*/
GT_U32  prvCpssDxChTableBaseAddrGet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT          tableType,
    OUT GT_BOOL                         *errorPtr
);

/**
* @internal prvCpssDxChTablePortsBmpCheck function
* @endinternal
*
* @brief   This function checks that the bmp of ports can be supported by the device.
*         assumption that it is bmp of port of 'this' device.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] portsBmpPtr              - pointer to the bmp of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when bmp hold ports that not supported by the device
*/
GT_STATUS   prvCpssDxChTablePortsBmpCheck
(
    IN GT_U8                     devNum,
    IN CPSS_PORTS_BMP_STC       *portsBmpPtr
);

/**
* @internal prvCpssDxChTablePortsBmpLimitedNumCheck function
* @endinternal
*
* @brief   This function checks that the bmp of ports can be supported by the device.
*         assumption that it is bmp of port of 'this' device.
*         with limited number of ports that supported
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] limitedNum               - limited number of ports in the BMP that allowed
*                                      to be different from zero.
* @param[in] portsBmpPtr              - pointer to the bmp of ports
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when bmp hold ports that not supported by the device or
*                                       bit is set over 'limitedNum'
*/
GT_STATUS   prvCpssDxChTablePortsBmpLimitedNumCheck
(
    IN GT_U8                     devNum,
    IN GT_U32                    limitedNum,
    IN CPSS_PORTS_BMP_STC       *portsBmpPtr
);

/**
* @internal prvCpssDxChTablePortsBmpAdjustToDevice function
* @endinternal
*
* @brief   This function removes none exists ports from bmp of ports.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] IN_portsBmpPtr           - pointer to the bmp of ports
*
* @param[out] OUT_portsBmpPtr          - pointer to the bmp of ports , that hold no ports
*                                      that not supported by the device
*
* @retval GT_OK                    - on success
*/
GT_STATUS   prvCpssDxChTablePortsBmpAdjustToDevice
(
    IN GT_U8                     devNum,
    IN  CPSS_PORTS_BMP_STC       *IN_portsBmpPtr,
    OUT CPSS_PORTS_BMP_STC       *OUT_portsBmpPtr
);

/**
* @internal prvCpssDxChTablePortsBmpAdjustToDeviceLimitedNum function
* @endinternal
*
* @brief   This function removes none exists ports from bmp of ports.
*         with limited number of ports that supported
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] limitedNum               - limited number of ports in the BMP that allowed
*                                      to be different from zero.
* @param[in] IN_portsBmpPtr           - pointer to the bmp of ports
*
* @param[out] OUT_portsBmpPtr          - pointer to the bmp of ports , that hold no ports
*                                      that not supported by the device
*
* @retval GT_OK                    - on success
*/
GT_STATUS   prvCpssDxChTablePortsBmpAdjustToDeviceLimitedNum
(
    IN GT_U8                     devNum,
    IN GT_U32                    limitedNum,
    IN  CPSS_PORTS_BMP_STC       *IN_portsBmpPtr,
    OUT CPSS_PORTS_BMP_STC       *OUT_portsBmpPtr
);

/**
* @internal prvCpssDxChTablePlrMeterFieldBuild function
* @endinternal
*
* @brief   This function build the value into needed field in the proper buffer.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwMeterArr[]             - pointer to 'meter' entry (in 5_15 'token bucket' entry)
* @param[in] hwMeterConfigArr[]       - pointer to 'meter config' entry (relevant only to 5_15)
* @param[in] fieldName                - name of the field to set
* @param[in] value                    -  to set
*
* @retval GT_OK                    - on success
* @retval GT_NOT_IMPLEMENTED       - on non implemented case
* @retval GT_NOT_APPLICABLE_DEVICE - on non relevant field to the device
* @retval GT_BAD_PARAM             - on value that not fit into field
*/
GT_STATUS   prvCpssDxChTablePlrMeterFieldBuild
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwMeterArr[],
    IN GT_U32                   hwMeterConfigArr[],
    IN SIP5_PLR_METERING_TABLE_FIELDS_ENT      fieldName,
    IN GT_U32                   value
);

/**
* @internal prvCpssDxChTablePlrMeterFieldParse function
* @endinternal
*
* @brief   This function parse the value for needed field from the proper buffer.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] hwMeterArr[]             - pointer to 'meter' entry (in 5_15 'token bucket' entry)
* @param[in] hwMeterConfigArr[]       - pointer to 'meter config' entry (relevant only to 5_15)
* @param[in] fieldName                - name of the field to set
*
* @retval GT_OK                    - on success
* @retval GT_NOT_IMPLEMENTED       - on non implemented case
* @retval GT_NOT_APPLICABLE_DEVICE - on non relevant field to the device
* @retval GT_BAD_PARAM             - on value that not fit into field
*/
GT_STATUS   prvCpssDxChTablePlrMeterFieldParse
(
    IN GT_U8                    devNum,
    IN GT_U32                   hwMeterArr[],
    IN GT_U32                   hwMeterConfigArr[],
    IN SIP5_PLR_METERING_TABLE_FIELDS_ENT      fieldName,
    IN GT_U32                   *valuePtr
);

/**
* @internal prvCpssDxChIndexAsPortNumConvert function
* @endinternal
*
* @brief   convert index that is portNum ('Global port num') to portGroupId and
*         'Local port num'
*         for tables that are not with index = portNum , no modification in the
*         index , and portGroupId will be 'first active port group == 0'
*         for non multi-port-groups device : portGroupId is 'all port groups' , no index conversion
*
* @note   APPLICABLE DEVICES:       xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] entryIndex               - index in the table
*
* @param[out] portGroupIdPtr           - (pointer to) the portGroupId to use
* @param[out] updatedEntryIndexPtr     - (pointer to) the updated index
* @param[out] updatedTableTypePtr      - (pointer to) the NEW specific table name to use
*                                       none
*/
GT_VOID prvCpssDxChIndexAsPortNumConvert
(
    IN GT_U8                devNum,
    IN CPSS_DXCH_TABLE_ENT  tableType,
    IN GT_U32               entryIndex,
    OUT GT_U32              *portGroupIdPtr,
    OUT GT_U32              *updatedEntryIndexPtr,
    OUT CPSS_DXCH_TABLE_ENT *updatedTableTypePtr
);

/**
* @internal prvCpssDxChPbrTableConfigSet function
* @endinternal
*
* @brief   This function set PBR virtual table configuration
*
* @note   APPLICABLE DEVICES:           Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                          PP's device number.
* @param[in] baseAddress                     base address.
* @param[in] maxNumOfEntries                 maximum number of PBR entries.
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_NOT_FOUND         - PBR table entry not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssDxChPbrTableConfigSet
(
    IN GT_U8    devNum,
    IN GT_U32   baseAddress,
    IN GT_U32   maxNumOfEntries
);

/**
* @internal prvCpssDxChTableMgcamEnableSet function
* @endinternal
*
* @brief   Enables/disables using MGCAM for table access.
*
* @note   APPLICABLE DEVICES:      Aldrin2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] enable                   - enable MGCAM access for the TCAM table.
*                                       GT_TRUE - enable
*                                       GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_U32 prvCpssDxChTableMgcamEnableSet
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT              tableType,
    IN GT_BOOL                          enable
);

/**
* @internal prvCpssDxChTableReadFromShadowEnableSet function
* @endinternal
*
* @brief   Set enable reading table from shadow instead of HW
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad param value
* @retval GT_BAD_STATE             - shadow for the table not supported
*/
GT_STATUS prvCpssDxChTableReadFromShadowEnableSet
(
    IN GT_U8                 devNum,
    IN CPSS_DXCH_TABLE_ENT   tableType,
    IN GT_BOOL               enable
);

/**
* @internal prvCpssDxChTableReadFromShadowEnableGet function
* @endinternal
*
* @brief   Get enable reading table from shadow instead of HW
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  none.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] tableType                - the 'HW table'
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - bad param value
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS prvCpssDxChTableReadFromShadowEnableGet
(
    IN  GT_U8                 devNum,
    IN  CPSS_DXCH_TABLE_ENT   tableType,
    OUT GT_BOOL               *enablePtr
);

GT_VOID initTablesFormatDb
(
  IN GT_U8  devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwTablesh */


