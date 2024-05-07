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
* @file prvTgfTcamClients.h
*
* @brief TCAM Clients utils - internal data
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfTcamClients_h
#define __prvTgfTcamClients_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <extUtils/tcamClients/tgfTcamClients.h>

/* amount of PCL CFG profile Ids supported by HW */
#define PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS 16

/* amount of TCAM profiles Ids supported by This Library */
#define PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_PROFILES_CNS 32

/* amount of TCAM profiles Ids supported by This Library */
#define PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_MUX_LINES_CNS 32

/* amount of PCL UDB Anchors supported by This Library */
#define PRV_TGF_TCAM_CLIENTS_PCL_UDB_ANCHORS_CNS 8

/* amount of TCAM Client Groups supported by This Library */
#define PRV_TGF_TCAM_CLIENTS_TCAM_CLIENT_GROUPS_CNS 6

/* amount of TCAM profile Ids supported by HW */
#define PRV_TGF_TCAM_CLIENTS_TCAM_PROFILE_IDS_MAX_CNS 64

/* amount of TCAM Mux Table Lines supported by HW */
#define PRV_TGF_TCAM_CLIENTS_TCAM_MUX_LINES_MAX_CNS 48

/* maximal Action Modified Field Byte-pair selection supported by HW */
#define PRV_TGF_TCAM_CLIENTS_ACTION_MODIFIED_SEL_MAX_CNS 24

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC
 *
 *   @brief Superkeys contents.
*/
typedef struct
{
    /** @brief Ingress UDB anchors of all Packet Type Groups for all UDBs
     */
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT udbIngressAnchorArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];

    /** @brief Ingress UDB offsets (in bytes) of all Packet Type Groups for all UDBs
     */
    GT_U8 udbIngressOffsetArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];

    /** @brief Egress UDB anchors of all Packet Type Groups for all UDBs
     */
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT udbEgressAnchorArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];

    /** @brief Egress UDB offsets (in bytes) of all Packet Type Groups for all UDBs
     */
    GT_U8 udbEgressOffsetArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];

    /** @brief Action Modified Field Selection per Ingress Lookups
     */
    CPSS_DXCH_PCL_LOOKUP_ACTION_MODIFIED_FIELDS_SELECT_STC actionModifiedFieldsSelectArr[
        TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E];

    /** @brief Numbers of Common PCL UDB configurations
     */
    GT_U32 numOfIngressHeaderUdbCfg;
    GT_U32 numOfEgressHeaderUdbCfg;

    /** @brief Common PCL UDB configurations for all packet types
     */
    TGF_TCAM_CLIENTS_UDB_CFG_STC ingressHeaderUdbConfigArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_DEFAULT_CONFIG_CNS];
    TGF_TCAM_CLIENTS_UDB_CFG_STC egressHeaderUdbConfigArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_DEFAULT_CONFIG_CNS];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC;


/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_PCL_CFG_TO_TCAM_PROFILE_STC
 *
 *   @brief Internal DB to PCL Cfg Profile Id to TCAM Profile.
*/
typedef struct
{
    /** @brief PCL Cfg profile to TCAM Profile Table
     */
    GT_U8 ipclCfgProfileToTcamProfileArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS];

    /** @brief MPCL PCL Cfg profile to TCAM Profile Table
     */
    GT_U8 mpclCfgProfileToTcamProfileArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS];

    /** @brief EPCL PCL Cfg profile to TCAM Profile Table
     */
    GT_U8 epclCfgProfileToTcamProfileArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_PCL_CFG_TO_TCAM_PROFILE_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_PROFILE_ENTRY_STC
 *
 *   @brief TCAM Profile Entry.
*/
typedef struct
{
    /** @brief PCL lookup - to determinate TCAM Client Group
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT pclLookup;

    /** @brief TCAM Profile Id.
     */
    GT_U8 tcamProfileId;

    /** @brief Sizes of 4 subkeys in 5 byte-pair units.
     */
    GT_U8 tcamSubkeySizesArr[4];

    /** @brief TCAM Mixing table indexes Array.
     *  Each line contains 30 addresses of byte-pairs for selection from superkey.
     *  Elements 0-3 for first 60 bytes of subkeys, Element 4 - for shared subkeys bytes 60-79.
     */
    GT_U8 tcamMuxTableLineIndexesArr[5];

    /** @brief TCAM Mixing table offsets Array.
     *  Offsets in resolution of 5 byte-pairs.
     *  Elements 0-3 for first 60 bytes of subkeys, Element 4 - for shared subkeys bytes 60-79.
     */
    GT_U8 tcamMuxTableLineOffsetsArr[5];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_PROFILE_ENTRY_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC
 *
 *   @brief TCAM Mux Table Line Entry.
*/
typedef struct
{
    /** @brief PCL lookup - to determinate TCAM Client Group
     */
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT pclLookup;

    /** @brief TCAM Mux Line Index (in the table).
     */
    GT_U8 tcamMuxLineIndex;

    /** @brief Array of indexes of selected byte-pairs from superkey.
     */
    GT_U8 tcamSuperkeyBytePairSelectArr[30];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC
 *
 *   @brief TCAM Subkey Configurations.
*/
typedef struct
{
    /** @brief TCAM Profile Amount.
     */
    GT_U32 tcamProfilesAmount;

    /** @brief Array of TCAM Profile Configurations.
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_PROFILE_ENTRY_STC tcamProfilesArr[
        PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_PROFILES_CNS];

    /** @brief TCAM Mux Lines Amount.
     */
    GT_U32 tcamMuxLinesAmount;

    /** @brief Array of TCAM Profile Configurations.
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC tcamMuxLinesArr[
        PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_MUX_LINES_CNS];

    /** @brief Number of Mux entries for PCL common Key Headers
     */
    GT_U8 ipclHeaderNumOfMuxEntries;
    GT_U8 mpclHeaderNumOfMuxEntries;
    GT_U8 epclHeaderNumOfMuxEntries;

    /** @brief Suprkey bases for PCL common Key Headers.
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT ipclHeaderMuxSuperkeyBasesArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT mpclHeaderMuxSuperkeyBasesArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT epclHeaderMuxSuperkeyBasesArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

    /** @brief Suprkey offsets in byte-pair resolution for PCL common Key Headers
     */
    GT_U8 ipclHeaderMuxSuperkeyOffsetsArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];
    GT_U8 mpclHeaderMuxSuperkeyOffsetsArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];
    GT_U8 epclHeaderMuxSuperkeyOffsetsArr[
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_UDB_SEARCH_DICTIONNARY_STC
 *
 *   @brief Data dedicated to fast find UDB index used for field
 *   by anchor and offset.
*/
typedef struct
{

    /** @brief Thee data below are tripples of UDB <index, anchor, offset>
     *  For all configured UDBs
     *  sorted by tupple <anchor, offset>
     *  Instead of anchors array used anchor-start array having index of not-used place
     *  in the last element.
     */
    GT_U8 anchorStartArr[CPSS_DXCH_PCL_OFFSET_INVALID_E + 2]; /*Klockwork needs "+2" but "+1" is enough*/

    /** @brief Array of UDB offsets
     */
    GT_U8 udbOffsetArr[TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];

    /** @brief Array of UDB indexes
     */
    GT_U8 udbIndexArr[TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];

} PRV_TGF_TCAM_CLIENTS_UDB_SEARCH_DICTIONNARY_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_INDEXES_STC
 *
 *   @brief Data dedicated to build the rule by list of subfields patterns and masks.
 *   It is set of additional indexes for fast access to the data kept in Internal DB.
 *   This part related to superkey data,
*/
typedef struct
{
    /* given data to find subfields location in subkey:                                           */
    /* pclCfgProfileId, udbPacketTypeGroupIndex, pclLookup, tcamSubkeyIndex, subfieldData         */
    /* When the rule built used 2 buffers sized as superkey - pattern and mask                    */
    /* the buffers cleared and patterns and masks of subfields copied to these buffers one by one */
    /* At the next stage the data from these buffers used to build rule  pattern and mask         */
    /* just as HW builds the subkey from the key                                                  */

    /* ===== Field location in superkey ===== */

    /* Field ID to UDB index - data below */
    /* Field ID to Action Modiefied Location using actionModifiedFieldsSelectArr in DB */
    /* Other fields have permanent location in superkey */

    /** @brief Ingress UDB search dictionnaries for all UDB Packet type sets
     */
    PRV_TGF_TCAM_CLIENTS_UDB_SEARCH_DICTIONNARY_STC ingressUdbSearchDictArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS];

    /** @brief Egress UDB search dictionnaries for all UDB Packet type sets
     */
    PRV_TGF_TCAM_CLIENTS_UDB_SEARCH_DICTIONNARY_STC egressUdbSearchDictArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS];

    /** @brief Action Modified Selection inverted index
     *  0xFF - byte pair not selected
     */
    GT_U8 amfSelectionIdexArr[
        TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E][
        PRV_TGF_TCAM_CLIENTS_ACTION_MODIFIED_SEL_MAX_CNS];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_INDEXES_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_INDEXES_STC
 *
 *   @brief Data dedicated to build the rule by list of subfields patterns and masks.
 *   It is set of additional indexes for fast access to the data kept in Internal DB.
 *   This part related to subkey data,
*/
typedef struct
{
    /* ===== Field location in subkey ===== */
    /* pclCfgProfileId, udbPacketTypeGroupIndex, pclLookup to TCAM Profile Id using in DB */

    /** @brief Index of TCAM Profile Cfg in DB (in tcamProfilesArr) 0xFF - not configured
     */
    GT_U8 tcamProfileCfgIndexArr[
        TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E][
        PRV_TGF_TCAM_CLIENTS_TCAM_PROFILE_IDS_MAX_CNS];

    /** @brief Index of TCAM Mux Table Lines in DB (in tcamMuxLinesArr) 0xFF - not configured
     */
    GT_U8 tcamMuxLinexIndexArr[
        TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E][
        PRV_TGF_TCAM_CLIENTS_TCAM_MUX_LINES_MAX_CNS];

    /** @brief Amount of PCL Common Header TCAM Mux Tables Entries
     */
    GT_U8 ipclCommonHeaderMuxAmount;
    GT_U8 mpclCommonHeaderMuxAmount;
    GT_U8 epclCommonHeaderMuxAmount;

    /** @brief PCL Common Header TCAM Mux Tables Entries
     */
    GT_U8 ipclCommonHeaderMuxArr[TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];
    GT_U8 mpclCommonHeaderMuxArr[TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];
    GT_U8 epclCommonHeaderMuxArr[TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS];

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_INDEXES_STC;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC
 *
 *   @brief Internal DB to keep configuration to be used for API call.
 *   Needed to find subfields positions in the subkey/rule by
 *   PCL Cfg Profile Id, PCL Lookup Id, paralell subkey index.
*/
typedef struct
{
    /** @brief Amount of UDB Packet Type Groups
     */
    GT_U32 numOfPktTypeGroups;

    /** @brief Array UDB Packet types bitmap
     */
    TGF_TCAM_CLIENTS_PCL_UDB_PKT_TYPES_BMP udbPktTypesBmpArr[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS];

    /** @brief Superkey Contents
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC superkeyData;

    /** @brief Superkey Indexes - data for fast calculation of fields layout
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_INDEXES_STC superkeyIndexes;

    /** @brief PCL Cfg profile to TCAM Profile Table
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_PCL_CFG_TO_TCAM_PROFILE_STC pclCfgToTcamProfileId;

    /** @brief PCL Lookup Mapped to TCAM Client Group
     */
    GT_U32 pclLookupToTcamClienGroup[TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E];

    /** @brief TCAM Subkey Configurations
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC tcamSubkeysCfg;

    /** @brief Subkey Indexes - data for fast calculation of fields layout
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_INDEXES_STC subkeyIndexes;

} PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC;

/**
 * @struct TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT
 *
 *   @brief Union of work memory structures.
*/
typedef union
{
    /** @brief work memory for rule build
     *  Maximal superkey is 120 bytes = 30 32-bit words
     *  Maximal subkey is 80 bytes = 20 32-bit words
     */
    struct
    {
        GT_U32 superkeyMaskArr[30];
        GT_U32 superkeyPatternArr[30];
        GT_U32 subkeyMaskArr[20];
        GT_U32 subkeyPatternArr[20];
    } buildRule;

    /** @brief work memory for UDB configuration by UDB anchor
     */
    struct
    {
        /* available UDB indexes spent and updated per anchor */
        TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR udbIndexesBmpArr[
            TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E];
        /* reuse for each anchor */
        CPSS_DXCH_PCL_OFFSET_TYPE_ENT udbAnchor;
        TGF_TCAM_CLIENTS_PCL_UDB_OFFSETS_BMP_ARR udbOffsetsBmpArr[
            TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E];
        TGF_TCAM_CLIENTS_PCL_UDB_OFFSETS_BMP_ARR workUdbOffsetsBmp;
        TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR workUdbIndexesBmp;
    } udbAccomulate;

    /** @brief work memory for ordering Action Modified Fieldsr
     */
    struct
    {
        /* Action Modified Data is 384 bit = 24 * 16-bit, i.e 24 byte-pairs */
        /* 10 byte-pairs from them selected per lookup                      */
        /* 4 lookups IPCL0-2 and MPCL - 4-bits bitmap per byte-pair         */
        GT_U8 amfLookupsBmpArr[24];
        /* 10 selectted byte-pairs per lookup - used entries */
        GT_U8 usedSeletionLookupBmpArr[CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS];
    } actionModifiedCfg;

    /** @brief work memory for UDB configuration sorting to build index
     */
    struct
    {
        /* bits[23:16] - anchor, bits[15:8] - offset, bits[7:0] - UDB index */
        GT_U32 cfgSortArr[TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS];
    } udbSort;

} PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT;

/**
 * @struct PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC
 *
 *   @brief Union of work memory structures.
*/
typedef struct
{
    /** @brief Database
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC db;

    /** @brief work memory
     */
    PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT workMemo;

} PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC;


/**
* @internal prvTgfTcamClientsPclFieldIdCheck function
* @endinternal
*
* @brief   Check Field id - range and registerd status.
*
* @param[in] fieldId           - field Id in Aplication defind range
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PTR           - on null pointer in DB
* @retval GT_BAD_PARAM         - on wrong field type in DB
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*/
GT_STATUS prvTgfTcamClientsPclFieldIdCheck
(
    IN  TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId
);

/**
* @internal prvTgfTcamClientsAddIngressUdbFieldsByTypeToSpace function
* @endinternal
*
* @brief   Add Ingress UDB_CFG subfields of array to space by type and lookup.
*
* @param[in] fieldSetPtr          - (pointer to) subfields set
* @param[in] udbAnchor            - UDB Anchor - relevant to UDB_CFG fields only
* @param[in] ingressLookup        - ingress lookup
* @param[in] udbPktTypeGroupIndex - undex of UDB packet type group
* @param[out] fieldsSpacePtr      - (pointer to) fields type related allocation space
*
* @retval GT_OK                   - on OK
* @retval GT_BAD_PARAM            - on wrong parameter value
* @retval GT_BAD_PTR              - on null pointer
* @retval GT_OUT_OF_RANGE         - on out of range parameter value
*/
GT_STATUS prvTgfTcamClientsAddIngressUdbFieldsByTypeToSpace
(
    IN     const TGF_TCAM_CLIENTS_PCL_RULE_INGRESS_UDB_SUB_FIELD_USE_STC  *fieldSetPtr,
    IN     CPSS_DXCH_PCL_OFFSET_TYPE_ENT                                  udbAnchor,
    IN     TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                                ingressLookup,
    IN     GT_U32                                                         udbPktTypeGroupIndex,
    INOUT  GT_U32                                                         *fieldsSpacePtr
);

/**
* @internal prvTgfTcamClientsAddEgressUdbFieldsByTypeToSpace function
* @endinternal
*
* @brief   Add Egress UDB_CFG subfields of array to space by type.
*
* @param[in] fieldSetPtr          - (pointer to) subfields set
* @param[in] udbAnchor            - UDB Anchor - relevant to UDB_CFG fields only
* @param[in] udbPktTypeGroupIndex - undex of UDB packet type group
* @param[out] fieldsSpacePtr      - (pointer to) fields type related allocation space
*
* @retval GT_OK                   - on OK
* @retval GT_BAD_PARAM            - on wrong parameter value
* @retval GT_BAD_PTR              - on null pointer
* @retval GT_OUT_OF_RANGE         - on out of range parameter value
*/
GT_STATUS prvTgfTcamClientsAddEgressUdbFieldsByTypeToSpace
(
    IN     const TGF_TCAM_CLIENTS_PCL_RULE_EGRESS_UDB_SUB_FIELD_USE_STC   *fieldSetPtr,
    IN     CPSS_DXCH_PCL_OFFSET_TYPE_ENT                                  udbAnchor,
    IN     GT_U32                                                         udbPktTypeGroupIndex,
    INOUT  GT_U32                                                         *fieldsSpacePtr
);

/**
* @internal prvTgfTcamClientsIngressUdbConfigurationAllocate function
* @endinternal
*
* @brief   Allocate and store to DB configuration of Ingress UDBs.
*
* @param[in] udbFieldsPtr          - (pointer to) Ingress UDB fields application data
* @param[in] numOfPktTypeGroups    - amount of used UDB Packet Type Groups
* @param[inout] workMemoPtr        - (pointer to) work memory provided by caller
* @param[out] udbCfgPtr            - (pointer to)superkey related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR            - on null pointer
* @retval GT_OUT_OF_RANGE       - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsIngressUdbConfigurationAllocate
(
    IN      const TGF_TCAM_CLIENTS_PCL_RULE_UDB_INGRESS_CFG_ALL_FIELD_SET_STC   *udbFieldsPtr,
    IN      GT_U32                                                              numOfPktTypeGroups,
    INOUT   PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT                         *workMemoPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC                       *udbCfgPtr
);

/**
* @internal prvTgfTcamClientsEgressUdbConfigurationAllocate function
* @endinternal
*
* @brief   Allocate and store to DB configuration of Egress UDBs.
*
* @param[in] udbFieldsPtr          - (pointer to) Egress UDB fields application data
* @param[in] numOfPktTypeGroups    - amount of used UDB Packet Type Groups
* @param[inout] workMemoPtr        - (pointer to) work memory provided by caller
* @param[out] udbCfgPtr            - (pointer to)superkey related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR            - on null pointer
* @retval GT_OUT_OF_RANGE       - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsEgressUdbConfigurationAllocate
(
    IN      const TGF_TCAM_CLIENTS_PCL_RULE_UDB_EGRESS_CFG_ALL_FIELD_SET_STC    *udbFieldsPtr,
    IN      GT_U32                                                              numOfPktTypeGroups,
    INOUT   PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT                         *workMemoPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC                       *udbCfgPtr
);

/**
* @internal prvTgfTcamClientsActionModifiedFieldsToInternalDb function
* @endinternal
*
* @brief   Convert and store Action Modified Fields configuration to intenal DB.
*
* @param[in] actionModifiedArrPtr  - (pointer to) Action modified fields application data
* @param[inout] workMemoPtr        - (pointer to) work memory provided by caller
* @param[out] udbCfgPtr            - (pointer to)superkey related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsActionModifiedFieldsToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_RULE_ACTION_MODIFIED_SUB_FIELD_USE_STC  *actionModifiedArrPtr,
    INOUT   PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT                        *workMemoPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC                      *udbCfgPtr
);

/**
* @internal prvTgfTcamClientsCommonHeaderUdbToInternalDb function
* @endinternal
*
* @brief   Store Common Header UDB configuration to intenal DB.
*
* @param[in] commonHeaderPtr       - (pointer to) Common Header configuration
* @param[in] numOfPktTypeGroups    - amount of used UDB Packet Type Groups
* @param[out] udbCfgPtr            - (pointer to)superkey related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsCommonHeaderUdbToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_SUBKEY_COMMON_HEADER_STC  *commonHeaderPtr,
    IN      GT_U32                                                            numOfPktTypeGroups,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC                     *udbCfgPtr
);

/**
* @internal prvTgfTcamClientsBuildUdbSearchIndex function
* @endinternal
*
* @brief Build UDB search dictionnary
*
* @param[in] anchorsArrPtr        - (pointer to) UDB anchors per index - not configured - 0xFF
* @param[in] offsetsArrPtr        - (pointer to) UDB offsets per index - not configured - 0xFF
* @param[inout] workSortArrPtr    - (pointer to) work memory provided by caller
* @param[out] searchDict          - (pointer to) UDB search dictionnary
*
* @brief build search dictionnary for one UDB Packet Type Group  .
* */
void prvTgfTcamClientsBuildUdbSearchIndex
(
    IN    const CPSS_DXCH_PCL_OFFSET_TYPE_ENT              *anchorsArrPtr,
    IN    const GT_U8                                      *offsetsArrPtr,
    INOUT GT_U32                                           *workSortArrPtr,
    OUT   PRV_TGF_TCAM_CLIENTS_UDB_SEARCH_DICTIONNARY_STC  *searchDictPtr
);

/**
* @internal prvTgfTcamClientsBiuldInternalDbSuperkeyIndex function
* @endinternal
*
* @brief   Convert and store Action Modified Fields configuration to intenal DB.
*
* @param[in] superkeyCfgPtr       - (pointer to) superkey related internal DB data
* @param[in] numOfPktTypeGroups   - amount of used UDB Packet Type Groups
* @param[inout] workMemoPtr       - (pointer to) work memory provided by caller
* @param[out] superkeyIndexesPtr  - (pointer to) index for superkey related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsBiuldInternalDbSuperkeyIndex
(
    IN      const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_STC       *superkeyCfgPtr,
    IN      GT_U32                                                    numOfPktTypeGroups,
    INOUT   PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT               *workMemoPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_INDEXES_STC     *superkeyIndexesPtr
);

/**
* @internal prvTgfTcamClientsSuperkeySubfieldLayoutGet function
* @endinternal
*
* @brief   Get subfield layout in superkey.
*
* @param[in] internalDbPtr          - (pointer to) internal DB data
* @param[in] udbPacketTypeGroupIndex - index of field UDB Packet Type Group
* @param[in] pclLookup               - PCL Lookup
* @param[in] fieldId                 - field Id
* @param[in] fieldBitOffset          - Offset in bits from field origin
* @param[in] superkeyBitOffsetPtr    - (pointer to)Offset in bits from superkey origin
* @param[in] superkeyBitLengthPtr    - (pointer to)Length in bits of contigous
*                                      following part of this field in superkey
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsSuperkeySubfieldLayoutGet
(
    IN      const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC               *internalDbPtr,
    IN      GT_U32                                                    udbPacketTypeGroupIndex,
    IN      TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT                           pclLookup,
    IN      TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT                       fieldId,
    IN      GT_U32                                                    fieldBitOffset,
    OUT     GT_U32                                                    *superkeyBitOffsetPtr,
    OUT     GT_U32                                                    *superkeyBitLengthPtr
);

/**
* @internal prvTgfTcamClientsProfileIdMappingToInternalDb function
* @endinternal
*
* @brief   Store Profile Id mapping to intenal DB.
*
* @param[in] appProfileMappingPtr  - (pointer to) Applicatin profile mapping data
* @param[in] numOfPktTypeGroups    - amount of used UDB Packet Type Groups
* @param[out] udbCfgPtr            - (pointer to)profile mapping related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsProfileIdMappingToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_STC       *appProfileMappingPtr,
    IN      GT_U32                                                       numOfPktTypeGroups,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_PCL_CFG_TO_TCAM_PROFILE_STC *dbProfileMappingPtr
);

/**
* @internal prvTgfTcamClientsCommonHeaderMuxToInternalDb function
* @endinternal
*
* @brief   Store Common Header Muxing Table Data configuration to intenal DB.
*
* @param[in] commonHeaderPtr       - (pointer to) Common Header configuration
* @param[out] subkeyCfgPtr         - (pointer to) subkey related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsCommonHeaderMuxToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_SUBKEY_COMMON_HEADER_STC  *commonHeaderPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC                       *subkeyCfgPtr
);

/**
* @internal prvTgfTcamClientsCommonHeaderMuxBuildInternalDbIndex function
* @endinternal
*
* @brief   Build Common Header Muxing Table Data configuration index for intenal DB.
*
* @param[in] subkeyCfgPtr         - (pointer to) subkey related internal DB data
* @param[out] subkeyIndexPtr      - (pointer to) Subkey index structure
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsCommonHeaderMuxBuildInternalDbIndex
(
    IN     const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC        *subkeyCfgPtr,
    OUT    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_INDEXES_STC      *subkeyIndexPtr
);

/**
* @internal prvTgfTcamClientsProfileCfgToInternalDb function
* @endinternal
*
* @brief   Store Profile configurations to intenal DB.
*
* @param[in] appProfileCfgArrPtr  - (pointer to) Application profile configuration data
* @param[out] dbSubkeyCfgPtr      - (pointer to) profile configuration related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsProfileCfgToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC *appProfileCfgArrPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC     *dbSubkeyCfgPtr
);

/**
* @internal prvTgfTcamClientsMuxingTableCfgToInternalDb function
* @endinternal
*
* @brief   Store Muxing Table configurations to intenal DB.
*
* @param[in] appCfgPtr          - (pointer to) full application configuration structure
* @param[in] internalDbPtr      - (pointer to) internal DB structure (needed for fields to suprkey location)
* @param[out] dbSubkeyCfgPtr    - (pointer to) Muxing Table configuration related internal DB data
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsMuxingTableCfgToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC *appMuxingCfgArrPtr,
    IN      const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC                     *internalDbPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_STC                    *dbSubkeyCfgPtr
);

/**
* @internal prvTgfTcamClientsProfilesAndMuxingCfgToSubkeyIndex function
* @endinternal
*
* @brief   Store Muxing Table configurations to intenal DB.
*
* @param[in] internalDbPtr      - (pointer to) internal DB structure (needed for fields to suprkey location)
* @param[out] dbSubkeyIdxPtr    - (pointer to) Subkey internal DB index
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR           - on null pointer
* @retval GT_OUT_OF_RANGE      - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsProfilesAndMuxingCfgToSubkeyIndex
(
    IN      const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC            *internalDbPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_INDEXES_STC   *dbSubkeyIdxPtr
);

/**
* @internal prvTgfTcamClientsAppConfigurationToInternalDb function
* @endinternal
*
* @brief   Convert Application configuration and store to internal DB.
*
* @param[in] appCfgPtr          - (pointer to) full application configuration structure
* @param[inout] workMemoPtr     - (pointer to) work memory provided by caller
* @param[out] internalDbPtr     - (pointer to) internal DB structure
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR            - on null pointer
* @retval GT_OUT_OF_RANGE       - on out of range parameter value
*
*/
GT_STATUS prvTgfTcamClientsAppConfigurationToInternalDb
(
    IN      const TGF_TCAM_CLIENTS_PCL_FULL_CFG_SET_STC    *appCfgPtr,
    INOUT   PRV_TGF_TCAM_CLIENTS_INTERNAL_WORK_MEMO_UNT    *workMemoPtr,
    OUT     PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC           *internalDbPtr
);

/**
* @internal prvTgfTcamClientsInternalDbDump function
* @endinternal
*
* @brief   Debud dump internal DB.
*
* @param[in] internalDbPtr     - (pointer to) internal DB structure
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PARAM         - on wrong parameter value
* @retval GT_BAD_PTR            - on null pointer
*
*/
GT_STATUS prvTgfTcamClientsInternalDbDump
(
    IN const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC           *internalDbPtr
);

/**
* @internal prvTgfTcamClientsDeviceConfigurationPrint function
* @endinternal
*
* @brief   Print device Configuration
*
* @param[in] devNum       - device number
*
* @retval GT_OK                    - on OK
* @retval GT_BAD_PARAM             - on wrong parameter value
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvTgfTcamClientsDeviceConfigurationPrint
(
    IN      GT_U8                                    devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTcamClients_h */