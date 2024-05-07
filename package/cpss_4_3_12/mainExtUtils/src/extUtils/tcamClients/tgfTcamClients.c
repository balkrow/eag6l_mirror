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
* @file tgfTcamClients.c
*
* @brief TCAM Clients utils
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <extUtils/tcamClients/tgfTcamClients.h>
#include <extUtils/tcamClients/prvTgfTcamClients.h>

#define FLD_UDB_CFG        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E
#define FLD_UDB_APP        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_APP_E
#define FLD_ACT_MOD        TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ACTION_MODIFIED_E
#define FLD_FIXED          TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_FIXED_E
#define FLD_ID(id)         TGF_TCAM_CLIENTS_PCL_RULE_FIELD_##id##_E
#define FLD_ANCHOR(anchor) CPSS_DXCH_PCL_OFFSET_##anchor##_E
#define FLD_NO_ANCHOR      FLD_ANCHOR(INVALID)
#define FLD_METADATA       FLD_ANCHOR(METADATA)
#define FLD_L2             FLD_ANCHOR(L2)
#define FLD_L3             FLD_ANCHOR(L3_MINUS_2)
#define FLD_L4             FLD_ANCHOR(L4)

/**
* @struct TGF_TCAM_CLIENTS_IPCL_RULE_FIELD_INFO_STC
 *
 * @brief Info for fields used in PCL RULES.
 *        (APPLICABLE DEVICES: AAS)
*/
typedef struct
{
    /** @brief field type
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT fieldId;

    /** @brief field type
     */
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC fieldInfo;

} TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ID_INFO_STC;

/* The offsets of UDB_CFG fields related to packet headers */
/* specified in network order as is all standards          */
/* the nework order is order of bits being serialized      */
/* at transmit and deserialized at receive                 */
/* it means that:                                          */
/* 1. the network bit numeration in byte array is swapped :        */
/* byte:                  0                      |  1                       |...*/
/* computer bit in byte:  0  1  2  3  4  5  6  7 |  0  1  2  3  4  5  6  7  |...*/
/* network  bit number:   7  6  5  4  3  2  1  0 | 15 14 13 12 11 10  9  8  |...*/
/* 2. bits of any fiels in any header of any protocol presented in LOW-END order   */
/*    offset is the network bit with the least nuber - the MSB of the field        */
/* example: MAC_SA - bytes 6-11, bits 48-95 */
/* the bit 47 of MAC_SA is bit 48 in network numeration and it is bit 7 of byte 6 */
/* the bit 0 of MAC_SA is bit 95 in network numeration and it is bit 0 of byte 11 */
/* example: IPV6_DSCP - IPV6 Header bytes 0-1, network bits 4-9 byte1[7:6],byte0[3:0] */
/* byte:                  0                      |  1                       |...*/
/* computer bit in byte:  0  1  2  3  4  5  6  7 |  0  1  2  3  4  5  6  7  |...*/
/* network  bit number:   7  6  5  4  3  2  1  0 | 15 14 13 12 11 10  9  8  |...*/
/* field bit number:      2  3  4  5             |                    0  1  |...*/

static TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ID_INFO_STC wellKnownPclFieldsArr[] =
{
    /* IPCL, MPCL metadata fields - offset in metadata */
    {FLD_ID(IMETA_PKT_TYPE_BMP),  {FLD_UDB_CFG, FLD_METADATA,  728,  8}},
    {FLD_ID(IMETA_IS_L3_INVALID), {FLD_UDB_CFG, FLD_METADATA,  328,  1}},
    {FLD_ID(IMETA_IS_L4_VALID),   {FLD_UDB_CFG, FLD_METADATA,  409,  1}},
    {FLD_ID(IMETA_IS_IP),         {FLD_UDB_CFG, FLD_METADATA,  329,  1}},
    {FLD_ID(IMETA_IS_IPV4),       {FLD_UDB_CFG, FLD_METADATA,  330,  1}},
    {FLD_ID(IMETA_IS_IPV6),       {FLD_UDB_CFG, FLD_METADATA,  331,  1}},
    {FLD_ID(IMETA_IS_ARP),        {FLD_UDB_CFG, FLD_METADATA,  333,  1}},
    {FLD_ID(IMETA_IS_MPLS),       {FLD_UDB_CFG, FLD_METADATA,  512,  1}},
    {FLD_ID(IMETA_TAG0_EXISTS),   {FLD_UDB_CFG, FLD_METADATA,  208,  1}},
    {FLD_ID(EMETA_TAG0_EXISTS),   {FLD_UDB_CFG, FLD_METADATA,  209,  1}},
    {FLD_ID(IMETA_TAG1_EXISTS),   {FLD_UDB_CFG, FLD_METADATA,  221,  1}},
    {FLD_ID(EMETA_TAG1_EXISTS),   {FLD_UDB_CFG, FLD_METADATA,  222,  1}},
    {FLD_ID(IMETA_IP_PROTOCOL),   {FLD_UDB_CFG, FLD_METADATA,  240,  8}},
    {FLD_ID(EMETA_IP_PROTOCOL),   {FLD_UDB_CFG, FLD_METADATA,  248,  8}},
    {FLD_ID(IMETA_SRC_PHY_PORT),  {FLD_UDB_CFG, FLD_METADATA,   80,  8}},
    {FLD_ID(IMETA_SRC_EPORT),     {FLD_UDB_CFG, FLD_METADATA,   40, 12}},
    {FLD_ID(EMETA_SRC_PHY_PORT),  {FLD_UDB_CFG, FLD_METADATA,  128,  8}},
    {FLD_ID(IMETA_QOS_PROFILE),   {FLD_UDB_CFG, FLD_METADATA,  553, 10}},
    {FLD_ID(EMETA_QOS_PROFILE),   {FLD_UDB_CFG, FLD_METADATA,  472, 10}},
    {FLD_ID(IMETA_VID0),          {FLD_UDB_CFG, FLD_METADATA,  792, 13}}, /*227:216 Orig VID, 807:792 eVLAN*/
    {FLD_ID(EMETA_VID0),          {FLD_UDB_CFG, FLD_METADATA,  208, 13}}, /*220:208 eVlan, 207:196 Orig VID*/
    {FLD_ID(IMETA_VID1),          {FLD_UDB_CFG, FLD_METADATA,  808, 12}},
    {FLD_ID(EMETA_VID1),          {FLD_UDB_CFG, FLD_METADATA,  232, 12}},
    {FLD_ID(IMETA_UP0),           {FLD_UDB_CFG, FLD_METADATA,  840,  3}},
    {FLD_ID(EMETA_UP0),           {FLD_UDB_CFG, FLD_METADATA,  224,  3}},
    {FLD_ID(IMETA_UP1),           {FLD_UDB_CFG, FLD_METADATA,  820,  3}},
    {FLD_ID(EMETA_UP1),           {FLD_UDB_CFG, FLD_METADATA,  228,  3}},
    {FLD_ID(IMETA_DSCP),          {FLD_UDB_CFG, FLD_METADATA, 1394,  6}}, /*399:1392    Packet TOS*/
    {FLD_ID(IMETA_TAG0_TPID_INDEX),{FLD_UDB_CFG, FLD_METADATA, 232,  3}},
    {FLD_ID(IMETA_TAG1_TPID_INDEX),{FLD_UDB_CFG, FLD_METADATA, 235,  3}},
    {FLD_ID(IMETA_IS_PTP),        {FLD_UDB_CFG, FLD_METADATA,  456,  1}},
    {FLD_ID(EMETA_IS_PTP),        {FLD_UDB_CFG, FLD_METADATA,  344,  1}},
    {FLD_ID(EMETA_EGR_PKT_CMD),   {FLD_UDB_CFG, FLD_METADATA,  512,  2}},
    /* IPCL, MPCL EPCL Packet Part fields - offset in packet from anchor */
    {FLD_ID(L2_DST_MAC),         {FLD_UDB_CFG, FLD_L2,           0,  48}},
    {FLD_ID(L2_SRC_MAC),         {FLD_UDB_CFG, FLD_L2,          48,  48}},
    {FLD_ID(L3_ETHERTYPE),       {FLD_UDB_CFG, FLD_L3,           0,  16}},
    {FLD_ID(L3_IPV4_DSCP),       {FLD_UDB_CFG, FLD_L3,          24,   6}}, /* network bit order */
    {FLD_ID(L3_IPV6_DSCP),       {FLD_UDB_CFG, FLD_L3,          20,   6}}, /* network bit order */
    {FLD_ID(L4_TCP_SRC_PORT),    {FLD_UDB_CFG, FLD_L4,           0,  16}}, /* network bit order */
    {FLD_ID(L4_TCP_DST_PORT),    {FLD_UDB_CFG, FLD_L4,          16,  16}}, /* network bit order */
    /* Action modified fields IPCL/MPCL  */
    {FLD_ID(AMF_EVLAN),          {FLD_ACT_MOD,   FLD_NO_ANCHOR,     0, 15}},
    {FLD_ID(AMF_SRC_EPG),        {FLD_ACT_MOD,   FLD_NO_ANCHOR,    16, 12}},
    {FLD_ID(AMF_SRC_IS_VPORT),   {FLD_ACT_MOD,   FLD_NO_ANCHOR,    28,  1}},
    {FLD_ID(AMF_SRC_VPORT_EXT),  {FLD_ACT_MOD,   FLD_NO_ANCHOR,    29,  2}},
    {FLD_ID(AMF_DST_EPG),        {FLD_ACT_MOD,   FLD_NO_ANCHOR,    32, 12}},
    {FLD_ID(AMF_QOS_PROFILE),    {FLD_ACT_MOD,   FLD_NO_ANCHOR,    48, 10}},
    {FLD_ID(AMF_SRC_EPORT),      {FLD_ACT_MOD,   FLD_NO_ANCHOR,    64, 16}},
    {FLD_ID(AMF_TAG1_VID),       {FLD_ACT_MOD,   FLD_NO_ANCHOR,    80, 12}},
    {FLD_ID(AMF_TAG1_UP),        {FLD_ACT_MOD,   FLD_NO_ANCHOR,    92,  3}},
    {FLD_ID(AMF_TAG1_CFI),       {FLD_ACT_MOD,   FLD_NO_ANCHOR,    95,  1}},
    {FLD_ID(AMF_VRF_ID),         {FLD_ACT_MOD,   FLD_NO_ANCHOR,    96, 15}},
    {FLD_ID(AMF_SRC_ID),         {FLD_ACT_MOD,   FLD_NO_ANCHOR,   112, 12}},
    {FLD_ID(AMF_HASH_BYTES),     {FLD_ACT_MOD,   FLD_NO_ANCHOR,   128, 32}},
    {FLD_ID(AMF_GEN_CLASS),      {FLD_ACT_MOD,   FLD_NO_ANCHOR,   160, 32}},
    {FLD_ID(AMF_PACKET_CMD),     {FLD_ACT_MOD,   FLD_NO_ANCHOR,   192,  3}},
    {FLD_ID(AMF_CPU_CODE),       {FLD_ACT_MOD,   FLD_NO_ANCHOR,   195,  8}},
    {FLD_ID(AMF_TRG_DEV),        {FLD_ACT_MOD,   FLD_NO_ANCHOR,   208, 10}},
    {FLD_ID(AMF_TRG_IS_VIDX),    {FLD_ACT_MOD,   FLD_NO_ANCHOR,   218,  1}},
    {FLD_ID(AMF_TRG_IS_TRUNK),   {FLD_ACT_MOD,   FLD_NO_ANCHOR,   219,  1}},
    {FLD_ID(AMF_TRG_IS_VPORT),   {FLD_ACT_MOD,   FLD_NO_ANCHOR,   220,  1}},
    {FLD_ID(AMF_TRG_VPORT_EXT),  {FLD_ACT_MOD,   FLD_NO_ANCHOR,   221,  2}},
    {FLD_ID(AMF_TRG_PORT),       {FLD_ACT_MOD,   FLD_NO_ANCHOR,   224, 16}},
    {FLD_ID(AMF_IS_TS),          {FLD_ACT_MOD,   FLD_NO_ANCHOR,   240,  1}},
    {FLD_ID(AMF_ARP_OR_TS_PTR),  {FLD_ACT_MOD,   FLD_NO_ANCHOR,   241, 19}},
    {FLD_ID(AMF_EM_PRPFILE_ID1), {FLD_ACT_MOD,   FLD_NO_ANCHOR,   264,  4}},
    {FLD_ID(AMF_EM_PRPFILE_ID2), {FLD_ACT_MOD,   FLD_NO_ANCHOR,   258,  4}},
    {FLD_ID(AMF_POLICER_PTR),    {FLD_ACT_MOD,   FLD_NO_ANCHOR,   272, 17}},
    {FLD_ID(AMF_BILLING_ENABLE), {FLD_ACT_MOD,   FLD_NO_ANCHOR,   289,  1}},
    {FLD_ID(AMF_METERING_ENABLE),{FLD_ACT_MOD,   FLD_NO_ANCHOR,   290,  1}},
    {FLD_ID(AMF_COPY_RESERVED),  {FLD_ACT_MOD,   FLD_NO_ANCHOR,   296, 20}},
    {FLD_ID(AMF_FLOW_ID),        {FLD_ACT_MOD,   FLD_NO_ANCHOR,   320, 18}},
    {FLD_ID(AMF_PCL_ID2),        {FLD_ACT_MOD,   FLD_NO_ANCHOR,   344, 24}},
    /* Fixed fields - offset are relative */
    {FLD_ID(IFIX_PCL_ID),          {FLD_FIXED,   FLD_NO_ANCHOR,     0,  12}},
    {FLD_ID(IFIX_UDB_VALID),       {FLD_FIXED,   FLD_NO_ANCHOR,    12,   1}},
    {FLD_ID(IFIX_MAC2ME),          {FLD_FIXED,   FLD_NO_ANCHOR,    13,   1}},
    {FLD_ID(IFIX_SRC_DEV_IS_OWN),  {FLD_FIXED,   FLD_NO_ANCHOR,    14,   1}},
    {FLD_ID(IFIX_MAC_DA),          {FLD_FIXED,   FLD_NO_ANCHOR,    16,  48}},
    {FLD_ID(FIX_IPV4_DIP),         {FLD_FIXED,   FLD_NO_ANCHOR,    64,  32}},
    {FLD_ID(FIX_IPV6_DIP),         {FLD_FIXED,   FLD_NO_ANCHOR,    64, 128}},
    {FLD_ID(FIX_IPV4_SIP),         {FLD_FIXED,   FLD_NO_ANCHOR,   192,  32}},
    {FLD_ID(FIX_IPV6_SIP),         {FLD_FIXED,   FLD_NO_ANCHOR,   192, 128}},
    {FLD_ID(EFIX_PCL_ID),          {FLD_FIXED,   FLD_NO_ANCHOR,     0,  10}},
    {FLD_ID(EFIX_UDB_VALID),       {FLD_FIXED,   FLD_NO_ANCHOR,    10,   1}},
    {FLD_ID(EFIX_SRC_EPG),         {FLD_FIXED,   FLD_NO_ANCHOR,    16,  12}},
    {FLD_ID(EFIX_DST_EPG),         {FLD_FIXED,   FLD_NO_ANCHOR,    32,  12}},
    {FLD_ID(EFIX_EM_PROFILE),      {FLD_FIXED,   FLD_NO_ANCHOR,    48,   8}}
};

/* info for Application registered PCL Fields */
static TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC appRegistredFieldsInfoArr[
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_MAX_AMOUNT_CNS];

static TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *allFieldsInfoPtrArr[
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD___LAST___E];

/**
* @struct PRV_TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_INFO_STC
 *
 * @brief Info for fields used in PCL RULES.
 *        (APPLICABLE DEVICES: AAS)
*/
typedef struct
{
    /** @brief field type
     */
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT superkeyBaseId;

    /** @brief Superkey Base Offset in byte-pair resolution
     */
    GT_U32 superkeyBaseOffset;

    /** @brief Superkey Base Size in byte-pair resolution
     */
    GT_U32 superkeyBaseSize;

} PRV_TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_INFO_STC;

/* offsets and sizes of different Superkey zones */
static const PRV_TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_INFO_STC superkeyBasesInfoArr[
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_NUM_OF_TYPES_E] =
{
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_UDB_E,                 0,  30},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_UDB_E,                0,  25},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_LIST_OF_FIELDS_E,    35,   5},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_AMF_SEL_E,            30,  10},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_AMF_SEL_E,           25,  10},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_FIXED_E,              40,  20},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_EPCL_FIXED_E,             30,  20},
    {TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_FIXED_E,             40,  20}
};

/* Supported UDB Anchors - Metadata must be first to be treated first */
static const CPSS_DXCH_PCL_OFFSET_TYPE_ENT supportedUdbAnchorsArr[] =
{
    CPSS_DXCH_PCL_OFFSET_METADATA_E,
    CPSS_DXCH_PCL_OFFSET_L2_E,
    CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
    CPSS_DXCH_PCL_OFFSET_L4_E,
    CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E,
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E,
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E,
    CPSS_DXCH_PCL_OFFSET_TUNNEL_L4_E
};
static const GT_U32 supportedUdbAnchorsAmount =
    (sizeof(supportedUdbAnchorsArr) / sizeof(supportedUdbAnchorsArr[0]));

static const CPSS_PCL_LOOKUP_NUMBER_ENT lookupToPclLookupNumberArr[] =
{ /**/
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E        */  CPSS_PCL_LOOKUP_0_0_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E        */  CPSS_PCL_LOOKUP_0_1_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E        */  CPSS_PCL_LOOKUP_1_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,        */  CPSS_PCL_LOOKUP_0_0_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E =       */  CPSS_PCL_LOOKUP_0_0_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E */  CPSS_PCL_LOOKUP_0_0_E
};

static const CPSS_DXCH_TCAM_CLIENT_ENT lookupToTcamClientArr[] =
{ /**/
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E        */  CPSS_DXCH_TCAM_IPCL_0_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E        */  CPSS_DXCH_TCAM_IPCL_1_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E        */  CPSS_DXCH_TCAM_IPCL_2_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E,        */  CPSS_DXCH_TCAM_MPCL_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E =       */  CPSS_DXCH_TCAM_EPCL_E,
    /*TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E */  CPSS_DXCH_TCAM_IPCL_0_E
};

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
)
{
    GT_U32 ndx;
    GT_U32 maxVal;

    /* Application Registerd Fields Info */
    cpssOsMemSet(appRegistredFieldsInfoArr, 0xFF, sizeof(appRegistredFieldsInfoArr));
    /* all fields pointers ID -> Info */
    cpssOsMemSet(allFieldsInfoPtrArr, 0, sizeof(allFieldsInfoPtrArr));

    maxVal = TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_MAX_AMOUNT_CNS;
    for (ndx = 0; (ndx < maxVal); ndx++)
    {
        allFieldsInfoPtrArr[TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E + ndx]
            = &(appRegistredFieldsInfoArr[ndx]);
    }

    maxVal = sizeof(wellKnownPclFieldsArr) / sizeof(wellKnownPclFieldsArr[0]);
    for (ndx = 0; (ndx < maxVal); ndx++)
    {
        allFieldsInfoPtrArr[wellKnownPclFieldsArr[ndx].fieldId]
            = &(wellKnownPclFieldsArr[ndx].fieldInfo);
    }
}

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
    IN  TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT            fieldId,
    IN  const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr
)
{
    GT_U32 fieldIdx;
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoPtr);

    if (fieldId < TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E)
    {
        return GT_OUT_OF_RANGE;
    }

    fieldIdx = (fieldId - TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_BASE_E);
    if (fieldIdx >= TGF_TCAM_CLIENTS_PCL_RULE_FIELD_APP_REGISTRED_MAX_AMOUNT_CNS)
    {
        return GT_OUT_OF_RANGE;
    }

    if ((GT_U32)fieldsInfoPtr->fieldType
        >= (GT_U32)TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MAX_E)
    {
        return GT_BAD_PARAM;
    }

    if (fieldsInfoPtr->fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
    {
        for (ii = 0; (ii < supportedUdbAnchorsAmount); ii++)
        {
            if (fieldsInfoPtr->udbAnchor == supportedUdbAnchorsArr[ii]) break;
        }
        if (ii >= supportedUdbAnchorsAmount)
        {
            return GT_BAD_PARAM;
        }
    }
    appRegistredFieldsInfoArr[fieldIdx] = *fieldsInfoPtr;
    return GT_OK;
}

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
)
{
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(fieldsInfoPtr);
    if (fieldId >= TGF_TCAM_CLIENTS_PCL_RULE_FIELD___LAST___E)
    {
        return GT_OUT_OF_RANGE;
    }
    *fieldsInfoPtr = *(allFieldsInfoPtrArr[fieldId]);

    if ((GT_U32)fieldsInfoPtr->fieldType
        >= (GT_U32)TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MAX_E)
    {
        return GT_BAD_STATE;
    }

    if (fieldsInfoPtr->fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
    {
        for (ii = 0; (ii < supportedUdbAnchorsAmount); ii++)
        {
            if (fieldsInfoPtr->udbAnchor == supportedUdbAnchorsArr[ii]) break;
        }
        if (ii >= supportedUdbAnchorsAmount)
        {
            return GT_BAD_STATE;
        }
    }
    return GT_OK;
}

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
)
{
    GT_U32 ii;
    const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr;

    if (fieldId >= TGF_TCAM_CLIENTS_PCL_RULE_FIELD___LAST___E)
    {
        return GT_OUT_OF_RANGE;
    }

    fieldsInfoPtr = allFieldsInfoPtrArr[fieldId];
    if (fieldsInfoPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    if ((GT_U32)fieldsInfoPtr->fieldType
        >= (GT_U32)TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MAX_E)
    {
        return GT_BAD_PARAM;
    }

    if (fieldsInfoPtr->fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
    {
        for (ii = 0; (ii < supportedUdbAnchorsAmount); ii++)
        {
            if (fieldsInfoPtr->udbAnchor == supportedUdbAnchorsArr[ii]) break;
        }
        if (ii >= supportedUdbAnchorsAmount)
        {
            return GT_BAD_PARAM;
        }
    }
    return GT_OK;
}

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
)
{
    const GT_U32 bitResolution = 8;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId;
    const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr;
    GT_U32 subFieldAbsoluteOffset;
    GT_U32 subFieldLength;
    GT_U32 idx;
    GT_U32 bitNdx;
    GT_U32 bitBase;
    GT_U32 bitBaseBound;

    CPSS_NULL_PTR_CHECK_MAC(fieldSetPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsSpacePtr);

    for (idx = 0; (1); idx++)
    {
        fieldId = fieldSetPtr[idx].subField.fieldId;
        if (fieldId
            == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E)
        {
            break;
        }
        if (prvTgfTcamClientsPclFieldIdCheck(fieldId) != GT_OK)
        {
            return GT_BAD_PARAM;
        }

        fieldsInfoPtr = allFieldsInfoPtrArr[fieldId];
        if (fieldsInfoPtr->fieldType != TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E) continue;
        if (fieldsInfoPtr->udbAnchor != udbAnchor) continue;
        if ((fieldSetPtr[idx].udbPktTypeGroupsBmpArr[ingressLookup] & (1 << udbPktTypeGroupIndex)) == 0) continue;

        if (udbAnchor == CPSS_DXCH_PCL_OFFSET_METADATA_E)
        {
            subFieldAbsoluteOffset =
                fieldsInfoPtr->bitOffset + fieldSetPtr[idx].subField.bitOffset;
        }
        else
        {
            /* field in network order - the LSBs in the last byte */
            subFieldAbsoluteOffset =
                (fieldsInfoPtr->bitOffset + fieldsInfoPtr->bitLength)
                - (fieldSetPtr[idx].subField.bitOffset + fieldSetPtr[idx].subField.bitLength);
        }
        subFieldLength         =
            fieldSetPtr[idx].subField.bitLength;
        if ((fieldSetPtr[idx].subField.bitOffset + subFieldLength) > fieldsInfoPtr->bitLength)
        {
            return GT_BAD_PARAM;
        }

        bitBaseBound = (subFieldAbsoluteOffset + subFieldLength + bitResolution - 1);
        bitBaseBound -= (bitBaseBound % bitResolution);
        for (bitBase = subFieldAbsoluteOffset;
              (bitBase < bitBaseBound);
              bitBase += bitResolution)
        {
            bitNdx = (bitBase / bitResolution);
            fieldsSpacePtr[bitNdx / 32] |= (1 << (bitNdx % 32));
        }
    }
    return GT_OK;
}

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
)
{
    const GT_U32 bitResolution = 8;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId;
    const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr;
    GT_U32 subFieldAbsoluteOffset;
    GT_U32 subFieldLength;
    GT_U32 idx;
    GT_U32 bitNdx;
    GT_U32 bitBase;
    GT_U32 bitBaseBound;

    CPSS_NULL_PTR_CHECK_MAC(fieldSetPtr);
    CPSS_NULL_PTR_CHECK_MAC(fieldsSpacePtr);

    for (idx = 0; (1); idx++)
    {
        fieldId = fieldSetPtr[idx].subField.fieldId;
        if (fieldId
            == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E)
        {
            break;
        }
        if (prvTgfTcamClientsPclFieldIdCheck(fieldId) != GT_OK)
        {
            return GT_BAD_PARAM;
        }

        fieldsInfoPtr = allFieldsInfoPtrArr[fieldId];
        if (fieldsInfoPtr->fieldType != TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E) continue;
        if (fieldsInfoPtr->udbAnchor != udbAnchor) continue;
        if ((fieldSetPtr[idx].udbPktTypeGroupsBmp & (1 << udbPktTypeGroupIndex)) == 0) continue;

        if (udbAnchor == CPSS_DXCH_PCL_OFFSET_METADATA_E)
        {
            subFieldAbsoluteOffset =
                fieldsInfoPtr->bitOffset + fieldSetPtr[idx].subField.bitOffset;
        }
        else
        {
            /* field in network order - the LSBs in the last byte */
            subFieldAbsoluteOffset =
                (fieldsInfoPtr->bitOffset + fieldsInfoPtr->bitLength)
                - (fieldSetPtr[idx].subField.bitOffset + fieldSetPtr[idx].subField.bitLength);
        }
        subFieldLength         =
            fieldSetPtr[idx].subField.bitLength;
        if ((fieldSetPtr[idx].subField.bitOffset + subFieldLength) > fieldsInfoPtr->bitLength)
        {
            return GT_BAD_PARAM;
        }

        bitBaseBound = (subFieldAbsoluteOffset + subFieldLength + bitResolution - 1);
        bitBaseBound -= (bitBaseBound % bitResolution);
        for (bitBase = subFieldAbsoluteOffset;
              (bitBase < bitBaseBound);
              bitBase += bitResolution)
        {
            bitNdx = (bitBase / bitResolution);
            fieldsSpacePtr[bitNdx / 32] |= (1 << (bitNdx % 32));
        }
    }
    return GT_OK;
}

/* find in bitmap the nearest one-bit at less or equal position */
/* if not found returns 0xFFFFFFFF                              */
static GT_U32 getLTEIndexOfOneInBmp(GT_U32 *bmpPtr, GT_U32 curPos)
{
    GT_U32 w;
    GT_U32 ndx;
    GT_U32 shft;
    GT_U32 mask;

    ndx  = (curPos / 32);
    shft = (curPos % 32);
    mask = (shft >= 31) ? 0xFFFFFFFF : (GT_U32)((1 << (shft + 1)) - 1);
    w = bmpPtr[ndx] & mask;
    while (ndx > 0)
    {
        /* value remained from previous ndx */
        if (w) break;
        ndx-- ;
        w = bmpPtr[ndx];
        shft = 31; /* not the word with curPos */
    }
    if (w == 0)
    {
        return 0xFFFFFFFF;
    }
    while (1)
    {
        if (w & (1 << shft))
        {
            return ((32 * ndx) + shft);
        }
        if (shft == 0)
        {
            return 0xFFFFFFFF;
        }
        shft--;
    }
    return 0xFFFFFFFF; /* never executed, needed for compiler */
}

/**
* @internal prvTgfTcamClientsIngressUdbConfigurationAllocate function
* @endinternal
*
* @brief   Allocate and store to DB configuration of Ingress UDBs.
*
* @param[in] udbFieldsPtr          - (pointer to) ngress UDB fields application data
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
)
{
    GT_STATUS rc;
    GT_U32 wordsInOffsetsBmp;
    GT_U32 wordsInIndexesBmp;
    GT_U32 pktTypeGroupIndex;
    GT_U32 udbAnchorIdx;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT  ingressLookup;
    GT_U32 minUdbIndex;
    GT_U32 wordIdx;
    GT_U32 udbOffset;
    GT_U32 udbIndex;

    CPSS_NULL_PTR_CHECK_MAC(udbFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(workMemoPtr);
    CPSS_NULL_PTR_CHECK_MAC(udbCfgPtr);

    /* erase output memory by invalid value 0xFF */
    cpssOsMemSet(
        &(udbCfgPtr->udbIngressAnchorArr),
        0xFF, sizeof(udbCfgPtr->udbIngressAnchorArr));
    cpssOsMemSet(
        &(udbCfgPtr->udbIngressOffsetArr),
        0xFF, sizeof(udbCfgPtr->udbIngressOffsetArr));

    wordsInOffsetsBmp = sizeof(TGF_TCAM_CLIENTS_PCL_UDB_OFFSETS_BMP_ARR) / sizeof(GT_U32);
    wordsInIndexesBmp = sizeof(TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR) / sizeof(GT_U32);

    for (pktTypeGroupIndex = 0; (pktTypeGroupIndex < numOfPktTypeGroups); pktTypeGroupIndex++)
    {
        /* copy bitmaps of available UDB indexes per packet type groups */
        /* UDB indexes found here and removed after use                 */
        cpssOsMemCpy(
            &(workMemoPtr->udbAccomulate.udbIndexesBmpArr),
            &(udbFieldsPtr->udbIndexesBmpArr),
            sizeof(workMemoPtr->udbAccomulate.udbIndexesBmpArr));

        for (udbAnchorIdx = 0; (udbAnchorIdx < supportedUdbAnchorsAmount); udbAnchorIdx++)
        {
            cpssOsMemSet(
                &(workMemoPtr->udbAccomulate.udbOffsetsBmpArr),
                0, sizeof(workMemoPtr->udbAccomulate.udbOffsetsBmpArr));
            workMemoPtr->udbAccomulate.udbAnchor = supportedUdbAnchorsArr[udbAnchorIdx];
            minUdbIndex = 0;
            if (workMemoPtr->udbAccomulate.udbAnchor == CPSS_DXCH_PCL_OFFSET_METADATA_E)
            {
                minUdbIndex = 12;
            }
            for (ingressLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E;
                (ingressLookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E);
                ingressLookup++)
            {
                rc = prvTgfTcamClientsAddIngressUdbFieldsByTypeToSpace(
                    udbFieldsPtr->ingressSubFieldsUseArrPtr,
                    workMemoPtr->udbAccomulate.udbAnchor,
                    ingressLookup, pktTypeGroupIndex,
                    workMemoPtr->udbAccomulate.udbOffsetsBmpArr[ingressLookup]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            for (wordIdx = 0; (wordIdx < wordsInOffsetsBmp); wordIdx++)
            {
                workMemoPtr->udbAccomulate.workUdbOffsetsBmp[wordIdx] = 0;
                for (ingressLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E;
                    (ingressLookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E);
                    ingressLookup++)
                {
                    workMemoPtr->udbAccomulate.workUdbOffsetsBmp[wordIdx] |=
                        workMemoPtr->udbAccomulate.udbOffsetsBmpArr[ingressLookup][wordIdx];
                }
            }
            udbOffset = (wordsInOffsetsBmp * 32) - 1;
            while (1)
            {
                udbOffset = getLTEIndexOfOneInBmp(
                    workMemoPtr->udbAccomulate.workUdbOffsetsBmp, udbOffset);
                if (udbOffset >= (wordsInOffsetsBmp * 32)) break; /*Klockwork*/
                if (udbOffset == 0xFFFFFFFF) break;
                for (wordIdx = 0; (wordIdx < wordsInIndexesBmp); wordIdx++)
                {
                    workMemoPtr->udbAccomulate.workUdbIndexesBmp[wordIdx] = 0xFFFFFFFF;
                    for (ingressLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E;
                        (ingressLookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E);
                        ingressLookup++)
                    {
                        if (workMemoPtr->udbAccomulate.udbOffsetsBmpArr[ingressLookup][udbOffset / 32]
                            & (1 << (udbOffset % 32)))
                        {
                            workMemoPtr->udbAccomulate.workUdbIndexesBmp[wordIdx] &=
                                workMemoPtr->udbAccomulate.udbIndexesBmpArr[pktTypeGroupIndex][wordIdx];
                        }
                    }
                }
                udbIndex = (wordsInIndexesBmp * 32) - 1;
                udbIndex = getLTEIndexOfOneInBmp(
                    workMemoPtr->udbAccomulate.workUdbIndexesBmp, udbIndex);
                if (udbIndex >= (wordsInIndexesBmp * 32))/*Klockwork*/
                {
                    return GT_NO_RESOURCE;
                }
                if (udbIndex == 0xFFFFFFFF)
                {
                    return GT_NO_RESOURCE;
                }
                if (udbIndex < minUdbIndex)
                {
                    return GT_NO_RESOURCE;
                }
                if (udbIndex >= TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS)
                {/*Klockwork*/
                    return GT_BAD_PARAM;
                }
                /* store output data */
                udbCfgPtr->udbIngressAnchorArr[pktTypeGroupIndex][udbIndex] =
                    workMemoPtr->udbAccomulate.udbAnchor;
                udbCfgPtr->udbIngressOffsetArr[pktTypeGroupIndex][udbIndex] =
                    udbOffset;
                /* remove udbIndex from work bitmaps per lookup */
                for (ingressLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E;
                    (ingressLookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E);
                    ingressLookup++)
                {
                    workMemoPtr->udbAccomulate.udbIndexesBmpArr[ingressLookup][udbIndex / 32]
                        &= (~ (1 << (udbIndex % 32)));
                }
                /* next loop */
                if (udbOffset == 0) break;
                udbOffset --;
            }
        }
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32 wordsInOffsetsBmp;
    GT_U32 wordsInIndexesBmp;
    GT_U32 pktTypeGroupIndex;
    GT_U32 udbAnchorIdx;
    GT_U32 minUdbIndex;
    GT_U32 udbOffset;
    GT_U32 udbIndex;

    CPSS_NULL_PTR_CHECK_MAC(udbFieldsPtr);
    CPSS_NULL_PTR_CHECK_MAC(workMemoPtr);
    CPSS_NULL_PTR_CHECK_MAC(udbCfgPtr);

    /* erase output memory by invalid value 0xFF */
    cpssOsMemSet(
        &(udbCfgPtr->udbEgressAnchorArr),
        0xFF, sizeof(udbCfgPtr->udbEgressAnchorArr));
    cpssOsMemSet(
        &(udbCfgPtr->udbEgressOffsetArr),
        0xFF, sizeof(udbCfgPtr->udbEgressOffsetArr));

    /* copy bitmaps of available UDB indexes per packet type groups */
    /* UDB indexes found here and removed after use                 */
    cpssOsMemCpy(
        &(workMemoPtr->udbAccomulate.udbIndexesBmpArr[0]),
        &(udbFieldsPtr->udbIndexesBmp),
        sizeof(workMemoPtr->udbAccomulate.udbIndexesBmpArr[0]));

    wordsInOffsetsBmp = sizeof(TGF_TCAM_CLIENTS_PCL_UDB_OFFSETS_BMP_ARR) / sizeof(GT_U32);
    wordsInIndexesBmp = sizeof(TGF_TCAM_CLIENTS_PCL_UDB_INDEXES_BMP_ARR) / sizeof(GT_U32);

    for (pktTypeGroupIndex = 0; (pktTypeGroupIndex < numOfPktTypeGroups); pktTypeGroupIndex++)
    {
        cpssOsMemCpy(
            &(workMemoPtr->udbAccomulate.workUdbIndexesBmp),
            &(workMemoPtr->udbAccomulate.udbIndexesBmpArr[0]),
            sizeof(workMemoPtr->udbAccomulate.workUdbIndexesBmp));
        for (udbAnchorIdx = 0; (udbAnchorIdx < supportedUdbAnchorsAmount); udbAnchorIdx++)
        {
            cpssOsMemSet(
                &(workMemoPtr->udbAccomulate.udbOffsetsBmpArr[0]),
                0, sizeof(workMemoPtr->udbAccomulate.udbOffsetsBmpArr[0]));
            workMemoPtr->udbAccomulate.udbAnchor = supportedUdbAnchorsArr[udbAnchorIdx];
            minUdbIndex = 0;
            if (workMemoPtr->udbAccomulate.udbAnchor == CPSS_DXCH_PCL_OFFSET_METADATA_E)
            {
                minUdbIndex = 12;
            }
            rc = prvTgfTcamClientsAddEgressUdbFieldsByTypeToSpace(
                udbFieldsPtr->egressSubFieldsUseArrPtr,
                workMemoPtr->udbAccomulate.udbAnchor,
                pktTypeGroupIndex,
                workMemoPtr->udbAccomulate.udbOffsetsBmpArr[0]);
            if (rc != GT_OK)
            {
                return rc;
            }
            udbOffset = (wordsInOffsetsBmp * 32) - 1;
            while (1)
            {
                udbOffset = getLTEIndexOfOneInBmp(
                    workMemoPtr->udbAccomulate.udbOffsetsBmpArr[0], udbOffset);
                if (udbOffset >= (wordsInOffsetsBmp * 32)) break; /*Klockwork*/
                if (udbOffset == 0xFFFFFFFF) break;
                udbIndex = (wordsInIndexesBmp * 32) - 1;
                udbIndex = getLTEIndexOfOneInBmp(
                    workMemoPtr->udbAccomulate.workUdbIndexesBmp, udbIndex);
                if (udbOffset >= (wordsInOffsetsBmp * 32))/*Klockwork*/
                {
                    return GT_NO_RESOURCE;
                }
                if (udbIndex == 0xFFFFFFFF)
                {
                    return GT_NO_RESOURCE;
                }
                if (udbIndex < minUdbIndex)
                {
                    return GT_NO_RESOURCE;
                }
                if (udbIndex >= TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS)
                {/*Klockwork*/
                    return GT_BAD_PARAM;
                }
                /* store output data */
                udbCfgPtr->udbEgressAnchorArr[pktTypeGroupIndex][udbIndex] =
                    workMemoPtr->udbAccomulate.udbAnchor;
                udbCfgPtr->udbEgressOffsetArr[pktTypeGroupIndex][udbIndex] =
                    udbOffset;
                /* remove udbIndex from work bitmaps per lookup */
                workMemoPtr->udbAccomulate.workUdbIndexesBmp[udbIndex / 32]
                    &= (~ (1 << (udbIndex % 32)));
                /* next loop */
                if (udbOffset == 0) break;
                udbOffset --;
            }
        }
    }

    return GT_OK;
}


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
)
{
    /* order of fields allocation to place all common to the same place */
    /* all possible not-zero bitmaps of 4 lookups ordered by amount of  */
    /* included lookups                                                 */
    static const GT_U8 lookupBmpArr[] =
        {0xF, 0xE, 0xD, 0xB, 7, 0xC, 0xA, 9, 6, 5, 3, 8, 4, 2, 1};
    static const GT_U32 lookupBmpArrSize = sizeof(lookupBmpArr) / sizeof(lookupBmpArr[0]);
    GT_U8  lookupBmp;
    GT_U32 lookupBmpIdx;
    GT_U32 lookup;
    GT_U32 idx;
    const TGF_TCAM_CLIENTS_PCL_RULE_SUB_FIELD_STC  *subFieldPtr;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT      fieldId;
    const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr;
    GT_U32 subFieldAbsoluteOffset;
    GT_U32 subFieldLength;
    GT_U32 unitResolution = 16; /*byte-pairs*/
    GT_U32 unitNumOf;
    GT_U32 unitBase;
    GT_U32 unitNdx;
    GT_U32 unitBaseBound;

    CPSS_NULL_PTR_CHECK_MAC(actionModifiedArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(workMemoPtr);
    CPSS_NULL_PTR_CHECK_MAC(udbCfgPtr);

    /* Clear an output data */
    cpssOsMemSet(
        udbCfgPtr->actionModifiedFieldsSelectArr,
        0, sizeof(udbCfgPtr->actionModifiedFieldsSelectArr));

    /* Accomulate lookup BMPs per each of 24 byte-pairs of Action Modified Fields */
    cpssOsMemSet(
        workMemoPtr->actionModifiedCfg.amfLookupsBmpArr,
        0, sizeof(workMemoPtr->actionModifiedCfg.amfLookupsBmpArr));
    unitNumOf =
        sizeof(workMemoPtr->actionModifiedCfg.amfLookupsBmpArr)
            / sizeof(workMemoPtr->actionModifiedCfg.amfLookupsBmpArr[0]);

    for (idx = 0; (1); idx++)
    {
        subFieldPtr = &(actionModifiedArrPtr[idx].subField);
        fieldId = subFieldPtr->fieldId;
        if (fieldId == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E) break;
        if (prvTgfTcamClientsPclFieldIdCheck(fieldId) != GT_OK)
        {
            return GT_BAD_PARAM;
        }

        fieldsInfoPtr = allFieldsInfoPtrArr[fieldId];
        if (fieldsInfoPtr->fieldType !=
            TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ACTION_MODIFIED_E) continue;
        subFieldAbsoluteOffset =
            fieldsInfoPtr->bitOffset + subFieldPtr->bitOffset;
        subFieldLength         = subFieldPtr->bitLength;
        if ((subFieldAbsoluteOffset + subFieldLength)
            > (fieldsInfoPtr->bitOffset + fieldsInfoPtr->bitLength))
        {
            return GT_BAD_PARAM;
        }

        unitBaseBound = subFieldAbsoluteOffset + subFieldLength + unitResolution - 1;
        unitBaseBound -= unitBaseBound % unitResolution;
        for (unitBase = subFieldAbsoluteOffset;
              (unitBase < unitBaseBound);
              unitBase += unitResolution)
        {
            unitNdx = (unitBase / unitResolution);
            if (unitNdx >= unitNumOf)
            {
                return GT_BAD_PARAM;
            }
            workMemoPtr->actionModifiedCfg.amfLookupsBmpArr[unitNdx]
                |= actionModifiedArrPtr[idx].ipclLookupsBmp;
        }
    }

    /* filling selection arrays attempting to order common fields */
    /* at the same positions of superkey                          */
    cpssOsMemSet(
        workMemoPtr->actionModifiedCfg.usedSeletionLookupBmpArr,
        0, sizeof(workMemoPtr->actionModifiedCfg.usedSeletionLookupBmpArr));

    for (lookupBmpIdx = 0; (lookupBmpIdx < lookupBmpArrSize); lookupBmpIdx++)
    {
        lookupBmp = lookupBmpArr[lookupBmpIdx];
        for (unitNdx = 0; (unitNdx < unitNumOf); unitNdx++)
        {
            if (lookupBmp == workMemoPtr->actionModifiedCfg.amfLookupsBmpArr[unitNdx])
            {
                /* look for common place for all lookups using the field */
                for (idx = 0; (idx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS); idx++)
                {
                    if ((lookupBmp & workMemoPtr->actionModifiedCfg.usedSeletionLookupBmpArr[idx]) == 0) break;
                }
                if (idx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS)
                { /* found common place for all lookups */
                    workMemoPtr->actionModifiedCfg.usedSeletionLookupBmpArr[idx] |= lookupBmp;
                    for (lookup = 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E); lookup++)
                    {
                        if ((lookupBmp & (1 << lookup)) == 0) continue;
                        udbCfgPtr->actionModifiedFieldsSelectArr[lookup].bytePairOffsets[idx] = unitNdx;
                    }
                }
                else
                {/* search for each lookup separately */
                    for (lookup = 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E); lookup++)
                    {
                        if ((lookupBmp & (1 << lookup)) == 0) continue;
                        for (idx = 0; (idx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS); idx++)
                        {
                            if ((1 << lookup) & workMemoPtr->actionModifiedCfg.usedSeletionLookupBmpArr[idx]) continue;
                            udbCfgPtr->actionModifiedFieldsSelectArr[lookup].bytePairOffsets[idx] = unitNdx;
                            workMemoPtr->actionModifiedCfg.usedSeletionLookupBmpArr[idx] |= (1 << lookupBmp);
                            break;
                        }
                        if (idx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS)
                        {
                            return GT_NO_RESOURCE;
                        }
                    }
                }
            }
        }
    }

    return GT_OK;
}

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
)
{
    GT_U32 idx;
    GT_U32 pktTypeGroupIdx;
    const TGF_TCAM_CLIENTS_UDB_CFG_STC *udbEntryPtr;

    CPSS_NULL_PTR_CHECK_MAC(commonHeaderPtr);
    CPSS_NULL_PTR_CHECK_MAC(udbCfgPtr);

    if (commonHeaderPtr->numOfIngressUdbCfg >=
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS)
    {
        return GT_BAD_PARAM;
    }

    if (commonHeaderPtr->numOfEgressUdbCfg >=
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS)
    {
        return GT_BAD_PARAM;
    }

    for (idx = 0; (idx < commonHeaderPtr->numOfIngressUdbCfg); idx++)
    {
        udbCfgPtr->ingressHeaderUdbConfigArr[idx] =
            commonHeaderPtr->ingressHeaderUdbConfigArr[idx];
    }
    udbCfgPtr->numOfIngressHeaderUdbCfg = (GT_U8)commonHeaderPtr->numOfIngressUdbCfg;

    for (idx = 0; (idx < commonHeaderPtr->numOfEgressUdbCfg); idx++)
    {
        udbCfgPtr->egressHeaderUdbConfigArr[idx] =
            commonHeaderPtr->egressHeaderUdbConfigArr[idx];
    }
    udbCfgPtr->numOfEgressHeaderUdbCfg = (GT_U8)commonHeaderPtr->numOfEgressUdbCfg;

    /* ingress UDB */
    for (idx = 0; (idx < commonHeaderPtr->numOfIngressUdbCfg); idx++)
    {
        udbEntryPtr = &(commonHeaderPtr->ingressHeaderUdbConfigArr[idx]);
        for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
        {
            udbCfgPtr->udbIngressAnchorArr[pktTypeGroupIdx][udbEntryPtr->udbIndex] =
                udbEntryPtr->udbAnchor;
            udbCfgPtr->udbIngressOffsetArr[pktTypeGroupIdx][udbEntryPtr->udbIndex] =
                udbEntryPtr->udbOffset;
        }
    }

    /* egress UDB */
    for (idx = 0; (idx < commonHeaderPtr->numOfEgressUdbCfg); idx++)
    {
        udbEntryPtr = &(commonHeaderPtr->egressHeaderUdbConfigArr[idx]);
        for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
        {
            udbCfgPtr->udbEgressAnchorArr[pktTypeGroupIdx][udbEntryPtr->udbIndex] =
                udbEntryPtr->udbAnchor;
            udbCfgPtr->udbEgressOffsetArr[pktTypeGroupIdx][udbEntryPtr->udbIndex] =
                udbEntryPtr->udbOffset;
        }
    }

    return GT_OK;
}

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
)
{
    GT_U32 udbIndex;
    GT_U32 workIndex;
    GT_U32 udbAmount;
    GT_U32 w, i, found;
    GT_U8  anchorFound;
    GT_U8  anchor;

    /* clear output data */
    cpssOsMemSet(searchDictPtr, 0, sizeof(*searchDictPtr));

    /* copy UDB configuration to sorted work array */
    workIndex = 0;
    for (udbIndex = 0;
          (udbIndex < TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS);
          udbIndex++)
    {
        if ((anchorsArrPtr[udbIndex] & 0xFF) == 0xFF) continue;
        /* bits[23:16] - anchor, bits[15:8] - offset, bits[7:0] - UDB index */
        workSortArrPtr[workIndex] =
            ((anchorsArrPtr[udbIndex] & 0xFF) << 16)
            | ((offsetsArrPtr[udbIndex] & 0xFF) << 8)
            | udbIndex;
        workIndex ++;
    }
    udbAmount = workIndex;
    if (udbAmount == 0) return;

    /* bubble sorting of work array */
    for (workIndex = (udbAmount - 1); (workIndex > 0); workIndex--)
    {
        found = 0;
        for (i = 0; (i < workIndex); i++)
        {
            w = workSortArrPtr[i + 1];
            if (w < workSortArrPtr[i])
            {
                found = 1;
                workSortArrPtr[i + 1] = workSortArrPtr[i];
                workSortArrPtr[i] = w;
            }
        }
        if (found == 0) break; /* already sorted */
    }

    /* store to dictionnary */
    anchor = 0;
    for (workIndex = 0; (workIndex < udbAmount); workIndex++)
    {
        w = workSortArrPtr[workIndex];
        searchDictPtr->udbOffsetArr[workIndex] = (GT_U8)((w >> 8) & 0xFF);
        searchDictPtr->udbIndexArr[workIndex]  = (GT_U8)(w & 0xFF);
        anchorFound = (GT_U8)((w >> 16) & 0xFF);
        for (/*no init*/; (anchor <= anchorFound); anchor++)
        {
            searchDictPtr->anchorStartArr[anchor] = workIndex;
        }
    }
    for (/*no init*/; (anchor <= CPSS_DXCH_PCL_OFFSET_INVALID_E); anchor++)
    {
        searchDictPtr->anchorStartArr[anchor] = workIndex;
    }
}

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
)
{
    GT_U32 lookup;
    GT_U32 bytePairSelIdx;
    GT_U32 bytePairIdx;
    GT_U32 udbPktTypeGroup;

    CPSS_NULL_PTR_CHECK_MAC(superkeyCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(workMemoPtr);
    CPSS_NULL_PTR_CHECK_MAC(superkeyIndexesPtr);

    /* ingress UDB fields index */
    for (udbPktTypeGroup = 0; (udbPktTypeGroup < numOfPktTypeGroups); udbPktTypeGroup++)
    {
        prvTgfTcamClientsBuildUdbSearchIndex(
            superkeyCfgPtr->udbIngressAnchorArr[udbPktTypeGroup],
            superkeyCfgPtr->udbIngressOffsetArr[udbPktTypeGroup],
            workMemoPtr->udbSort.cfgSortArr,
            &(superkeyIndexesPtr->ingressUdbSearchDictArr[udbPktTypeGroup]));
    }

    /* egress UDB fields index */
    for (udbPktTypeGroup = 0; (udbPktTypeGroup < numOfPktTypeGroups); udbPktTypeGroup++)
    {
        prvTgfTcamClientsBuildUdbSearchIndex(
            superkeyCfgPtr->udbEgressAnchorArr[udbPktTypeGroup],
            superkeyCfgPtr->udbEgressOffsetArr[udbPktTypeGroup],
            workMemoPtr->udbSort.cfgSortArr,
            &(superkeyIndexesPtr->egressUdbSearchDictArr[udbPktTypeGroup]));
    }

    /* action modified fields index */
    /* not configured selections at source set to zeros */
    cpssOsMemSet(
        &(superkeyIndexesPtr->amfSelectionIdexArr),
        0xFF, sizeof(superkeyIndexesPtr->amfSelectionIdexArr));
    for (lookup = 0;
          (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E);
          lookup++)
    {
        for (bytePairSelIdx = 0;
              (bytePairSelIdx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS);
              bytePairSelIdx++)
        {
            bytePairIdx =
                superkeyCfgPtr->actionModifiedFieldsSelectArr[lookup]
                .bytePairOffsets[bytePairSelIdx];
            /* if the same byte pair selected more than once use the first instance only */
            if (superkeyIndexesPtr->amfSelectionIdexArr[lookup][bytePairIdx] != 0xFF) continue;
            superkeyIndexesPtr->amfSelectionIdexArr[lookup][bytePairIdx] = bytePairSelIdx;
        }
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUPERKEY_INDEXES_STC   *superkeyIndexesPtr;
    const TGF_TCAM_CLIENTS_PCL_RULE_FIELD_INFO_STC *fieldsInfoPtr;
    GT_U32    maxOffset;
    GT_U32    fieldTypeSuperkeyBase;
    GT_U32    fieldTypeSuperkeySize;
    GT_U32    ii;
    GT_U32    offset;
    GT_U8     udbOffset;
    GT_U8     udbIndex;
    GT_U8     low, high, mid;
    const PRV_TGF_TCAM_CLIENTS_UDB_SEARCH_DICTIONNARY_STC *udbDictPtr;
    GT_U32    fieldBitOffsetNear;
    GT_U32    startBitInByte;
    GT_U32    numOfBitsInByte;
    TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_ENT superkeyBase;

    CPSS_NULL_PTR_CHECK_MAC(internalDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(superkeyBitOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(superkeyBitLengthPtr);

    superkeyIndexesPtr = &(internalDbPtr->superkeyIndexes);

    rc = prvTgfTcamClientsPclFieldIdCheck(fieldId);
    if (rc != GT_OK)
    {
        return rc;
    }
    fieldsInfoPtr = allFieldsInfoPtrArr[fieldId];
    switch (fieldsInfoPtr->fieldType)
    {
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E:
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_APP_E:
            if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E)
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_UDB_E;
            }
            else
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_UDB_E;
            }
            break;
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ACTION_MODIFIED_E:
            if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E)
            {
                return GT_BAD_PARAM;;
            }
            else if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E)
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_AMF_SEL_E;
            }
            else /*IPCL*/
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_AMF_SEL_E;
            }
            break;
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_FIXED_E:
            if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E)
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_EPCL_FIXED_E;
            }
            else if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E)
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_FIXED_E;
            }
            else /*IPCL*/
            {
                superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_PCL_FIXED_E;
            }
            break;
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MPCL_LIST_OF_FIELDS_E:
            superkeyBase = TGF_TCAM_CLIENTS_PCL_SUPERKEY_BASE_MPCL_LIST_OF_FIELDS_E;
            break;
        default: return GT_BAD_PARAM;
    }

    fieldTypeSuperkeySize = 0;
    for (ii = 0;
          (ii < (sizeof(superkeyBasesInfoArr) / sizeof(superkeyBasesInfoArr[0]))); ii++)
    {
        if (superkeyBasesInfoArr[ii].superkeyBaseId == superkeyBase)
        {
            fieldTypeSuperkeyBase = (superkeyBasesInfoArr[ii].superkeyBaseOffset * 16);
            fieldTypeSuperkeySize = (superkeyBasesInfoArr[ii].superkeyBaseSize * 16);
            break;
        }
    }
    if (fieldTypeSuperkeySize == 0)
    {
        return GT_BAD_PARAM;
    }

    if (fieldsInfoPtr->fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_APP_E)
    {
        /* fieldsInfoPtr->bitOffset is UDB index */
        maxOffset = (fieldsInfoPtr->bitLength * 8);
        if (fieldBitOffset >= maxOffset)
        {
            return GT_BAD_PARAM;
        }
        udbIndex = fieldsInfoPtr->bitOffset + ((fieldBitOffset + 7) / 8);
        fieldBitOffsetNear = fieldBitOffset % 8;
        if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E)
        {
            if (udbIndex < 10) /* UDB_APP bitOffset */
            {
                return GT_BAD_PARAM;
            }
            *superkeyBitOffsetPtr =
                fieldTypeSuperkeyBase + ((udbIndex - 10) * 8) + fieldBitOffsetNear;
            *superkeyBitLengthPtr = maxOffset - (udbIndex * 8);
        }
        else
        {
            *superkeyBitOffsetPtr =
                fieldTypeSuperkeyBase + (udbIndex * 8) + fieldBitOffsetNear;
            *superkeyBitLengthPtr = maxOffset - (udbIndex * 8);
        }
        return GT_OK;
    }

    if (fieldsInfoPtr->fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_UDB_CFG_E)
    {
        if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E)
        {
            udbDictPtr = &(superkeyIndexesPtr->egressUdbSearchDictArr[udbPacketTypeGroupIndex]);
        }
        else
        {
            udbDictPtr = &(superkeyIndexesPtr->ingressUdbSearchDictArr[udbPacketTypeGroupIndex]);
        }
        if (fieldBitOffset >= fieldsInfoPtr->bitLength)
        {
            return GT_BAD_PARAM;
        }
        if (fieldsInfoPtr->udbAnchor == CPSS_DXCH_PCL_OFFSET_METADATA_E)
        {
            offset = fieldsInfoPtr->bitOffset + fieldBitOffset;
            udbOffset = (GT_U8)(offset / 8);
            startBitInByte = (offset % 8);
        }
        else
        {
            /* network order */
            /* fieldsInfoPtr->bitOffset is the offset of the first bit in the */
            /* byte - first in network order and last on Little Endian Order  */
            maxOffset = fieldsInfoPtr->bitOffset + fieldsInfoPtr->bitLength - 1;
            offset    = maxOffset - fieldBitOffset;
            udbOffset = (GT_U8)(offset / 8) ;
            startBitInByte = 7 - (offset % 8);
        }
        low  = udbDictPtr->anchorStartArr[fieldsInfoPtr->udbAnchor];
        high = udbDictPtr->anchorStartArr[fieldsInfoPtr->udbAnchor + 1];
        if (low >= high)
        {
            return GT_NOT_FOUND;
        }
        if (udbOffset < udbDictPtr->udbOffsetArr[low])
        {
            return GT_NOT_FOUND;
        }
        if (udbOffset > udbDictPtr->udbOffsetArr[high - 1])
        {
            return GT_NOT_FOUND;
        }
        mid = low; /* for case ((low + 1) == high) */
        while ((low + 1) < high)
        {
            mid = ((low + high) / 2);
            if (udbDictPtr->udbOffsetArr[mid] == udbOffset)
            {
                break;
            }
            else if (udbDictPtr->udbOffsetArr[mid] > udbOffset)
            {
                high = mid;
            }
            else /*(udbDictPtr->udbOffsetArr[mid] < udbOffset)*/
            {
                low = mid;
            }
        }
        if (udbDictPtr->udbOffsetArr[mid] > udbOffset)
        {
            mid = low;
            if (udbDictPtr->udbOffsetArr[mid] != udbOffset)
            {
                return GT_NOT_FOUND;
            }
        }
        udbIndex = udbDictPtr->udbIndexArr[mid];
        numOfBitsInByte = (8 - startBitInByte);
        if ((numOfBitsInByte + fieldBitOffset) > fieldsInfoPtr->bitLength)
        {
            numOfBitsInByte = fieldsInfoPtr->bitLength - fieldBitOffset;
        }

        if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E)
        {
            if (udbIndex < 10) /* UDB_APP bitOffset */
            {
                return GT_BAD_PARAM;
            }
            *superkeyBitOffsetPtr =
                fieldTypeSuperkeyBase + ((udbIndex - 10) * 8) + startBitInByte;
            *superkeyBitLengthPtr = numOfBitsInByte;
        }
        else
        {
            *superkeyBitOffsetPtr =
                fieldTypeSuperkeyBase + (udbIndex * 8) + startBitInByte;
            *superkeyBitLengthPtr = numOfBitsInByte;
        }
        return GT_OK;
    }

    if (fieldBitOffset >= fieldsInfoPtr->bitLength)
    {
        return GT_BAD_PARAM;
    }
    maxOffset = fieldsInfoPtr->bitOffset + fieldsInfoPtr->bitLength;
    offset    = fieldsInfoPtr->bitOffset + fieldBitOffset;

    if (fieldsInfoPtr->fieldType == TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_ACTION_MODIFIED_E)
    {
        if (pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E)
        {
            return GT_BAD_PARAM;
        }
        ii = offset / 16; /*byte-pair resolution*/
        if (ii >= PRV_TGF_TCAM_CLIENTS_ACTION_MODIFIED_SEL_MAX_CNS)
        {
            return GT_BAD_PARAM;
        }
        ii = superkeyIndexesPtr->amfSelectionIdexArr[pclLookup][ii];
        *superkeyBitOffsetPtr =
            fieldTypeSuperkeyBase + (ii * 16) + (offset % 16);
        fieldBitOffsetNear = maxOffset - offset;
        ii = 16 - (offset % 16);
        if (fieldBitOffsetNear > ii)
        {
            fieldBitOffsetNear = ii;
        }
        *superkeyBitLengthPtr = fieldBitOffsetNear;
        return GT_OK;
    }

    switch (fieldsInfoPtr->fieldType)
    {
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_FIXED_E:
        case TGF_TCAM_CLIENTS_PCL_RULE_FIELD_TYPE_MPCL_LIST_OF_FIELDS_E:
            *superkeyBitOffsetPtr =
                fieldTypeSuperkeyBase + offset;
            *superkeyBitLengthPtr = maxOffset - offset;
            return GT_OK;
            break;
        default: return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal prvTgfTcamClientsProfileIdMappingToInternalDb function
* @endinternal
*
* @brief   Store Profile Id mapping to intenal DB.
*
* @param[in] appProfileMappingPtr  - (pointer to) Application profile mapping data
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
)
{
    GT_U32 direction;
    const TGF_TCAM_CLIENTS_PCL_CFG_TO_TCAM_PROFILE_MAP_ENTRY_STC *appProfileEntryPtr;
    typedef GT_U8 DB_PROFILE_ENTRY[
        TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS][
        PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS];
    DB_PROFILE_ENTRY *dbProfileMapArrPtr;
    GT_U8  pclCfgProfileId;
    GT_U8  tcamProfileId;
    GT_U32 pktTypeGroupIdx;

    CPSS_NULL_PTR_CHECK_MAC(appProfileMappingPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbProfileMappingPtr);

    /* clear output data */
    cpssOsMemSet(
        dbProfileMappingPtr, 0,
        sizeof(PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_PCL_CFG_TO_TCAM_PROFILE_STC));

    for (direction = 0; (direction < 3); direction++)
    {
        switch (direction)
        {
            default: /* never occups */
            case 0:
                /* IPCL */
                appProfileEntryPtr  = appProfileMappingPtr->ipclCfgToTcamProfileIdMapArrPtr;
                dbProfileMapArrPtr  = &(dbProfileMappingPtr->ipclCfgProfileToTcamProfileArr);
                break;
            case 1:
                /* MPCL */
                appProfileEntryPtr  = appProfileMappingPtr->mpclCfgToTcamProfileIdMapArrPtr;
                dbProfileMapArrPtr  = &(dbProfileMappingPtr->mpclCfgProfileToTcamProfileArr);
                break;
            case 2:
                /* EPCL */
                appProfileEntryPtr  = appProfileMappingPtr->epclCfgToTcamProfileIdMapArrPtr;
                dbProfileMapArrPtr  = &(dbProfileMappingPtr->epclCfgProfileToTcamProfileArr);
                break;
        }
        if (appProfileEntryPtr)
        {
            for (/*no init*/; (appProfileEntryPtr->pclCfgTableProfileId != 0xFF); appProfileEntryPtr++)
            {
                pclCfgProfileId = appProfileEntryPtr->pclCfgTableProfileId;
                if (pclCfgProfileId >= PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS)
                {
                    return GT_BAD_PARAM;
                }
                for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
                {
                    tcamProfileId = appProfileEntryPtr->tcamProfileIdArr[pktTypeGroupIdx];
                    if (tcamProfileId >= PRV_TGF_TCAM_CLIENTS_TCAM_PROFILE_IDS_MAX_CNS)
                    {
                        return GT_BAD_PARAM;
                    }
                    (*dbProfileMapArrPtr)[pktTypeGroupIdx][pclCfgProfileId] = tcamProfileId;
                }
            }
        }
    }

    return GT_OK;
}

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
)
{
    GT_U32 ii;

    CPSS_NULL_PTR_CHECK_MAC(commonHeaderPtr);
    CPSS_NULL_PTR_CHECK_MAC(subkeyCfgPtr);

    if (commonHeaderPtr->ipclNumOfMuxEntries >=
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (commonHeaderPtr->mpclNumOfMuxEntries >=
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (commonHeaderPtr->mpclNumOfMuxEntries >=
        TGF_TCAM_CLIENTS_PCL_MAX_MUXES_IN_DEFAULT_KEY_HEADER_CNS)
    {
        return GT_BAD_PARAM;
    }

    for (ii = 0; (ii < commonHeaderPtr->ipclNumOfMuxEntries); ii++)
    {
        subkeyCfgPtr->ipclHeaderMuxSuperkeyBasesArr[ii] =
            commonHeaderPtr->ipclMuxSuperkeyBasesArr[ii];
        subkeyCfgPtr->ipclHeaderMuxSuperkeyOffsetsArr[ii] =
            (GT_U8)commonHeaderPtr->ipclMuxSuperkeyOffsetsArr[ii];
    }
    subkeyCfgPtr->ipclHeaderNumOfMuxEntries =
        (GT_U8)commonHeaderPtr->ipclNumOfMuxEntries;

    for (ii = 0; (ii < commonHeaderPtr->mpclNumOfMuxEntries); ii++)
    {
        subkeyCfgPtr->mpclHeaderMuxSuperkeyBasesArr[ii] =
            commonHeaderPtr->mpclMuxSuperkeyBasesArr[ii];
        subkeyCfgPtr->mpclHeaderMuxSuperkeyOffsetsArr[ii] =
            (GT_U8)commonHeaderPtr->mpclMuxSuperkeyOffsetsArr[ii];
    }
    subkeyCfgPtr->mpclHeaderNumOfMuxEntries =
        (GT_U8)commonHeaderPtr->mpclNumOfMuxEntries;

    for (ii = 0; (ii < commonHeaderPtr->epclNumOfMuxEntries); ii++)
    {
        subkeyCfgPtr->epclHeaderMuxSuperkeyBasesArr[ii] =
            commonHeaderPtr->epclMuxSuperkeyBasesArr[ii];
        subkeyCfgPtr->epclHeaderMuxSuperkeyOffsetsArr[ii] =
            (GT_U8)commonHeaderPtr->epclMuxSuperkeyOffsetsArr[ii];
    }
    subkeyCfgPtr->epclHeaderNumOfMuxEntries =
        (GT_U8)commonHeaderPtr->epclNumOfMuxEntries;

    return GT_OK;
}

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
)
{
    GT_U8  selIdx;
    GT_U8  baseIdx;
    GT_U8  numOfBases;

    CPSS_NULL_PTR_CHECK_MAC(subkeyCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(subkeyIndexPtr);

    numOfBases = sizeof(superkeyBasesInfoArr) / sizeof(superkeyBasesInfoArr[0]);

    /* already checked */
    subkeyIndexPtr->ipclCommonHeaderMuxAmount =
        subkeyCfgPtr->ipclHeaderNumOfMuxEntries;
    subkeyIndexPtr->mpclCommonHeaderMuxAmount =
        subkeyCfgPtr->mpclHeaderNumOfMuxEntries;
    subkeyIndexPtr->epclCommonHeaderMuxAmount =
        subkeyCfgPtr->epclHeaderNumOfMuxEntries;

    /* IPCL */
    for (selIdx = 0; (selIdx < subkeyIndexPtr->ipclCommonHeaderMuxAmount); selIdx++)
    {
        for (baseIdx = 0; (baseIdx < numOfBases); baseIdx++)
        {
            if (superkeyBasesInfoArr[baseIdx].superkeyBaseId == subkeyCfgPtr->ipclHeaderMuxSuperkeyBasesArr[selIdx]) break;
        }
        if (baseIdx >= numOfBases)
        {
            return GT_BAD_PARAM;
        }
        subkeyIndexPtr->ipclCommonHeaderMuxArr[selIdx] =
            subkeyCfgPtr->ipclHeaderMuxSuperkeyOffsetsArr[selIdx]
            + superkeyBasesInfoArr[baseIdx].superkeyBaseOffset;
    }

    /* MPCL */
    for (selIdx = 0; (selIdx < subkeyIndexPtr->mpclCommonHeaderMuxAmount); selIdx++)
    {
        for (baseIdx = 0; (baseIdx < numOfBases); baseIdx++)
        {
            if (superkeyBasesInfoArr[baseIdx].superkeyBaseId == subkeyCfgPtr->mpclHeaderMuxSuperkeyBasesArr[selIdx]) break;
        }
        if (baseIdx >= numOfBases)
        {
            return GT_BAD_PARAM;
        }
        subkeyIndexPtr->mpclCommonHeaderMuxArr[selIdx] =
            subkeyCfgPtr->mpclHeaderMuxSuperkeyOffsetsArr[selIdx]
            + superkeyBasesInfoArr[baseIdx].superkeyBaseOffset;
    }

    /* EPCL */
    for (selIdx = 0; (selIdx < subkeyIndexPtr->epclCommonHeaderMuxAmount); selIdx++)
    {
        for (baseIdx = 0; (baseIdx < numOfBases); baseIdx++)
        {
            if (superkeyBasesInfoArr[baseIdx].superkeyBaseId == subkeyCfgPtr->epclHeaderMuxSuperkeyBasesArr[selIdx]) break;
        }
        if (baseIdx >= numOfBases)
        {
            return GT_BAD_PARAM;
        }
        subkeyIndexPtr->epclCommonHeaderMuxArr[selIdx] =
            subkeyCfgPtr->epclHeaderMuxSuperkeyOffsetsArr[selIdx]
            + superkeyBasesInfoArr[baseIdx].superkeyBaseOffset;
    }

    return GT_OK;
}

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
)
{
    GT_U32 entryIndex;
    GT_U32 idx;
    const TGF_TCAM_CLIENTS_PCL_TCAM_PROFILE_CFG_STC          *appProfileEntryPtr;
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_PROFILE_ENTRY_STC  *dbProfileEntryPtr;

    CPSS_NULL_PTR_CHECK_MAC(appProfileCfgArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbSubkeyCfgPtr);

    dbSubkeyCfgPtr->tcamProfilesAmount = 0;
    if (appProfileCfgArrPtr == NULL)
    {
        return GT_OK;
    }

    for (entryIndex = 0;
          (appProfileCfgArrPtr[entryIndex].tcamProfileId != 0xFFFFFFFF);
          entryIndex++)
    {
        if (entryIndex >=
            PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_PROFILES_CNS)
        {
            return GT_BAD_PARAM;
        }
        appProfileEntryPtr = &(appProfileCfgArrPtr[entryIndex]);
        dbProfileEntryPtr  = &(dbSubkeyCfgPtr->tcamProfilesArr[entryIndex]);

        dbProfileEntryPtr->pclLookup = appProfileEntryPtr->pclLookup;
        dbProfileEntryPtr->tcamProfileId =
            (GT_U8)appProfileEntryPtr->tcamProfileId;
        for (idx = 0; (idx < 4); idx++)
        {
            PRV_CPSS_DXCH_TCAM_CONVERT_RULE_SIZE_TO_KEY_SIZE_VAL_MAC(
                dbProfileEntryPtr->tcamSubkeySizesArr[idx],
                appProfileEntryPtr->tcamProfileCfg.subKeySizes[idx]);
            dbProfileEntryPtr->tcamMuxTableLineIndexesArr[idx] =
                appProfileEntryPtr->tcamProfileCfg.subKeyMuxTableLineIndexes[idx];
            dbProfileEntryPtr->tcamMuxTableLineOffsetsArr[idx] =
                appProfileEntryPtr->tcamProfileCfg.subKeyMuxTableLineOffsets[idx];
        }
        dbProfileEntryPtr->tcamMuxTableLineIndexesArr[4] =
            appProfileEntryPtr->tcamProfileCfg.sharedMuxTableLineIndex;
        dbProfileEntryPtr->tcamMuxTableLineOffsetsArr[4] =
            appProfileEntryPtr->tcamProfileCfg.sharedMuxTableLineOffset;
    }
    dbSubkeyCfgPtr->tcamProfilesAmount = entryIndex;
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32 entryIndex;
    const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_SUBFIELDS_CFG_STC  *appMuxingEntryPtr;
    const TGF_TCAM_CLIENTS_PCL_TCAM_MUXING_TABLE_FIELD_ENTRY_CFG_STC *muxedSubfieldEntryPtr;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_SUBKEY_INDEXES_STC        *subkeyIndexPtr;
    PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC         *dbTcamMuxLinesPtr;
    GT_U32 muxSlotsBitmap;
    GT_U32 muxLineUsedSlotsBitmap;
    GT_U32 muxKeyUsedSlotsBitmap;
    GT_U32 muxKeyFreeSlotsBitmap;
    GT_U32 ii, jj;
    GT_U32 numOfHdrMuxes;
    const GT_U8  *hdrMuxesArrPtr;
    GT_U32 mask;
    GT_U32 bitsStart;
    GT_U32 bitsRemainder;
    GT_U32 bitsLength;
    TGF_TCAM_CLIENTS_PCL_RULE_FIELD_ENT fieldId;
    GT_U32 superkeyBitOffset;
    GT_U32 superkeyBitLength;
    GT_U8  muxSlotData;
    GT_U32 muxSlotIdx;

    CPSS_NULL_PTR_CHECK_MAC(internalDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbSubkeyCfgPtr);

    subkeyIndexPtr = &(internalDbPtr->subkeyIndexes);

    dbSubkeyCfgPtr->tcamMuxLinesAmount = 0;
    if (appMuxingCfgArrPtr == NULL)
    {
        return GT_OK;
    }

    for (entryIndex = 0;
         (appMuxingCfgArrPtr[entryIndex].subKeyMuxTableLineIndex != 0xFF);
          entryIndex++)
    {
        if (entryIndex >=
            PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_MUX_LINES_CNS)
        {
            return GT_BAD_PARAM;
        }
        appMuxingEntryPtr = &(appMuxingCfgArrPtr[entryIndex]);
        dbTcamMuxLinesPtr = &(dbSubkeyCfgPtr->tcamMuxLinesArr[entryIndex]);
        dbTcamMuxLinesPtr->tcamMuxLineIndex =
            appMuxingEntryPtr->subKeyMuxTableLineIndex;
        dbTcamMuxLinesPtr->pclLookup = appMuxingEntryPtr->pclLookup;

        /* cleanup output */
        cpssOsMemSet(
            &(dbTcamMuxLinesPtr->tcamSuperkeyBytePairSelectArr),
            0, sizeof(dbTcamMuxLinesPtr->tcamSuperkeyBytePairSelectArr));

        muxLineUsedSlotsBitmap = 0;
        /* Header */
        if (appMuxingEntryPtr->pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E)
        {
            numOfHdrMuxes  = subkeyIndexPtr->epclCommonHeaderMuxAmount;
            hdrMuxesArrPtr = subkeyIndexPtr->epclCommonHeaderMuxArr;
        }
        else if (appMuxingEntryPtr->pclLookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E)
        {
            numOfHdrMuxes  = subkeyIndexPtr->mpclCommonHeaderMuxAmount;
            hdrMuxesArrPtr = subkeyIndexPtr->mpclCommonHeaderMuxArr;
        }
        else
        {
            numOfHdrMuxes  = subkeyIndexPtr->ipclCommonHeaderMuxAmount;
            hdrMuxesArrPtr = subkeyIndexPtr->ipclCommonHeaderMuxArr;
        }
        if (numOfHdrMuxes > 5)
        {
            /* cannot occur now - max header size defined as 4 */
            return GT_FAIL;
        }
        mask = (1 << numOfHdrMuxes) - 1;
        for (ii = 0; (ii < 6); ii++)
        {
            /* 6 words, each for 5 byte-pairs */
            if (appMuxingEntryPtr->muxWordWithCommonHeaderIndexesBmp & (1 << ii))
            {
                muxLineUsedSlotsBitmap |= (mask << (ii * 5));
                for (jj = 0; (jj < numOfHdrMuxes); jj++)
                {
                    dbTcamMuxLinesPtr->tcamSuperkeyBytePairSelectArr[(ii * 5) + jj]
                        = hdrMuxesArrPtr[jj];
                }
            }
        }

        /* sub-fields */
        for (muxedSubfieldEntryPtr = appMuxingEntryPtr->muxedSubfieldEntryArrPtr;
             (muxedSubfieldEntryPtr->subField.fieldId !=
              TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E);
              muxedSubfieldEntryPtr++)
        {
            muxSlotsBitmap = 0;
            for (ii = 0; (ii < 6); ii++)
            {
                /* 6 words, each for 5 byte-pairs */
                if (muxedSubfieldEntryPtr->muxWordIndexesBmp & (1 << ii))
                {
                    muxSlotsBitmap |= (0x1F << (ii * 5));
                }
            }
            muxKeyUsedSlotsBitmap = muxSlotsBitmap & muxLineUsedSlotsBitmap;
            muxKeyFreeSlotsBitmap = muxSlotsBitmap & (~ muxLineUsedSlotsBitmap);

            fieldId       = muxedSubfieldEntryPtr->subField.fieldId;
            bitsStart     = muxedSubfieldEntryPtr->subField.bitOffset;
            bitsRemainder = muxedSubfieldEntryPtr->subField.bitLength;

            while (1)
            {
                rc = prvTgfTcamClientsSuperkeySubfieldLayoutGet(
                    internalDbPtr, appMuxingEntryPtr->pktTypeGroupIndex,
                    appMuxingEntryPtr->pclLookup,
                    fieldId, bitsStart,
                    &superkeyBitOffset, &superkeyBitLength);
                if (rc != GT_OK)
                {
                    return rc;
                }
                muxSlotData = (GT_U8)(superkeyBitOffset / 16);
                /* look for offset in already allocated */
                for (muxSlotIdx = 0; (muxSlotIdx < 30); muxSlotIdx++)
                {
                    if ((muxKeyUsedSlotsBitmap & (1 << muxSlotIdx)) == 0) continue;
                    if (dbTcamMuxLinesPtr->tcamSuperkeyBytePairSelectArr[muxSlotIdx]
                        == muxSlotData)
                    {
                        /* the field part already added to the subkey */
                        break;
                    }
                }
                if (muxSlotIdx >= 30)
                {
                    /* not found - look for free slot */
                    for (muxSlotIdx = 0; (muxSlotIdx < 30); muxSlotIdx++)
                    {
                        if (muxKeyFreeSlotsBitmap & (1 << muxSlotIdx))
                        {
                            muxLineUsedSlotsBitmap |= (1 << muxSlotIdx);
                            muxKeyUsedSlotsBitmap  |= (1 << muxSlotIdx);
                            muxKeyFreeSlotsBitmap  &= (~ (1 << muxSlotIdx));
                            dbTcamMuxLinesPtr->tcamSuperkeyBytePairSelectArr[muxSlotIdx]
                                = muxSlotData;
                            break;
                        }
                    }
                }
                if (muxSlotIdx >= 30)
                {
                    /* no free slots */
                    return GT_NO_RESOURCE;
                }

                bitsLength = (16 - (superkeyBitOffset % 16));
                if (bitsLength > superkeyBitLength)
                {
                    bitsLength = superkeyBitLength;
                }
                if (bitsLength >= bitsRemainder) break;
                bitsStart     += bitsLength;
                bitsRemainder -= bitsLength;
            }
        }

        /* Amount */
        dbSubkeyCfgPtr->tcamMuxLinesAmount = entryIndex + 1;
    }
    /* redundant - added for Klockwork */
    if (dbSubkeyCfgPtr->tcamMuxLinesAmount > PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_MUX_LINES_CNS)
    {
        dbSubkeyCfgPtr->tcamMuxLinesAmount = PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_MUX_LINES_CNS;
    }

    return GT_OK;
}

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
)
{
    GT_U32 ndx;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT pclLookup;
    GT_U32 tcamMuxLinesAmount;
    GT_U8  id;


    CPSS_NULL_PTR_CHECK_MAC(internalDbPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbSubkeyIdxPtr);

    cpssOsMemSet(
        &(dbSubkeyIdxPtr->tcamProfileCfgIndexArr),
        0xFF, sizeof(dbSubkeyIdxPtr->tcamProfileCfgIndexArr));
    cpssOsMemSet(
        &(dbSubkeyIdxPtr->tcamMuxLinexIndexArr),
        0xFF, sizeof(dbSubkeyIdxPtr->tcamMuxLinexIndexArr));

    for (ndx = 0; (ndx < internalDbPtr->tcamSubkeysCfg.tcamProfilesAmount); ndx++)
    {
        pclLookup = internalDbPtr->tcamSubkeysCfg.tcamProfilesArr[ndx].pclLookup;
        if (pclLookup >= TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E)
        {
            return GT_BAD_PARAM;
        }
        id        = internalDbPtr->tcamSubkeysCfg.tcamProfilesArr[ndx].tcamProfileId;
        if (id >= PRV_TGF_TCAM_CLIENTS_TCAM_PROFILE_IDS_MAX_CNS)
        {
            return GT_BAD_PARAM;
        }
        dbSubkeyIdxPtr->tcamProfileCfgIndexArr[pclLookup][id] = ndx;
    }

    tcamMuxLinesAmount = internalDbPtr->tcamSubkeysCfg.tcamMuxLinesAmount;

    /* redundant - added for Klockwork */
    if (tcamMuxLinesAmount > PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_TCAM_MUX_LINES_CNS)
    {
        return GT_BAD_PARAM;
    }

    for (ndx = 0; (ndx < tcamMuxLinesAmount); ndx++)
    {
        pclLookup = internalDbPtr->tcamSubkeysCfg.tcamMuxLinesArr[ndx].pclLookup;
        if (pclLookup >= TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E)
        {
            return GT_BAD_PARAM;
        }
        id        = internalDbPtr->tcamSubkeysCfg.tcamMuxLinesArr[ndx].tcamMuxLineIndex;
        if (id >= PRV_TGF_TCAM_CLIENTS_TCAM_MUX_LINES_MAX_CNS)
        {
            return GT_BAD_PARAM;
        }
        dbSubkeyIdxPtr->tcamMuxLinexIndexArr[pclLookup][id] = ndx;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(appCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(workMemoPtr);
    CPSS_NULL_PTR_CHECK_MAC(internalDbPtr);

    cpssOsMemSet(
        internalDbPtr, 0,
        sizeof(PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_STC));

    if (appCfgPtr->numOfPktTypeGroups
        >= TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS)
    {
        return GT_BAD_PARAM;
    }

    /* UDB Packet Type Groups */
    internalDbPtr->numOfPktTypeGroups = appCfgPtr->numOfPktTypeGroups;
    cpssOsMemCpy(
        internalDbPtr->udbPktTypesBmpArr,
        appCfgPtr->udbPktTypesBmpArr,
        sizeof(internalDbPtr->udbPktTypesBmpArr));

    /* suprkey related configuration */

    /* ingress UDB fields */
    rc = prvTgfTcamClientsIngressUdbConfigurationAllocate(
        &(appCfgPtr->ingressSuperKeyFieldsSet.udbCfgAllFields),
        appCfgPtr->numOfPktTypeGroups,
        workMemoPtr,
        &(internalDbPtr->superkeyData));
    if (rc != GT_OK)
    {
        return rc;
    }
    /* egress UDB fields */
    rc = prvTgfTcamClientsEgressUdbConfigurationAllocate(
        &(appCfgPtr->egressSuperKeyFieldsSet),
        appCfgPtr->numOfPktTypeGroups,
        workMemoPtr,
        &(internalDbPtr->superkeyData));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* common Header UDB fields */
    rc = prvTgfTcamClientsCommonHeaderUdbToInternalDb(
        appCfgPtr->subkeyCommonHeaderPtr,
        appCfgPtr->numOfPktTypeGroups,
        &(internalDbPtr->superkeyData));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Action Modified fields (IPCL and MPCL) */
    rc = prvTgfTcamClientsActionModifiedFieldsToInternalDb(
        appCfgPtr->ingressSuperKeyFieldsSet.actionModifiedSubfieldsArrPtr,
        workMemoPtr,
        &(internalDbPtr->superkeyData));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* build indexes for fast access to superkey related internal DB data */
    rc = prvTgfTcamClientsBiuldInternalDbSuperkeyIndex(
        &(internalDbPtr->superkeyData),
        appCfgPtr->numOfPktTypeGroups,
        workMemoPtr,
        &(internalDbPtr->superkeyIndexes));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Common Header Muxing table related data to DB */
    rc = prvTgfTcamClientsCommonHeaderMuxToInternalDb(
        appCfgPtr->subkeyCommonHeaderPtr,
        &(internalDbPtr->tcamSubkeysCfg));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Common Header Muxing table related data index build */

    rc = prvTgfTcamClientsCommonHeaderMuxBuildInternalDbIndex(
        &(internalDbPtr->tcamSubkeysCfg),
        &(internalDbPtr->subkeyIndexes));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* profile Id mapping */
    rc = prvTgfTcamClientsProfileIdMappingToInternalDb(
        &(appCfgPtr->cfgToTcamProfileIdMap),
        appCfgPtr->numOfPktTypeGroups,
        &(internalDbPtr->pclCfgToTcamProfileId));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* profile configurations */
    rc = prvTgfTcamClientsProfileCfgToInternalDb(
        appCfgPtr->profileSubkeyCfgArrPtr,
        &(internalDbPtr->tcamSubkeysCfg));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Muxind tables configurations */
    rc = prvTgfTcamClientsMuxingTableCfgToInternalDb(
        appCfgPtr->muxingTableSubfieldsCfgArrPtr,
        internalDbPtr,
        &(internalDbPtr->tcamSubkeysCfg));
    if (rc != GT_OK)
    {
        return rc;
    }

    /* TCAM profiles and muxings index */
    rc = prvTgfTcamClientsProfilesAndMuxingCfgToSubkeyIndex(
        internalDbPtr,
        &(internalDbPtr->subkeyIndexes));
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC *dbPtr;

    CPSS_NULL_PTR_CHECK_MAC(appCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(dbHandlePtr);

    dbPtr = (PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC *)cpssOsMalloc(
        sizeof(PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC));
    if (dbPtr == NULL)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    rc = prvTgfTcamClientsAppConfigurationToInternalDb(
        appCfgPtr, &(dbPtr->workMemo), &(dbPtr->db));
    if (rc != GT_OK)
    {
        cpssOsFree(dbPtr);
        return rc;
    }

    *dbHandlePtr = dbPtr;
    return GT_OK;
}

/**
* @internal tgfTcamClientsConfigurationDbDelete function
* @endinternal
*
* @brief   Delete Configuration DB Configuration
*
* @param[in] dbHandle     - (pointer to) DB Handler
*
* @retval GT_OK                - on OK
* @retval GT_BAD_PTR           - on null pointer
*
*/
GT_STATUS tgfTcamClientsConfigurationDbDelete
(
    IN      TGF_TCAM_CLIENTS_DB_HANDLE                     dbHandle
)
{
    CPSS_NULL_PTR_CHECK_MAC(dbHandle);
    cpssOsFree(dbHandle);
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC *dbPtr;
    GT_U32    udbIdx;
    GT_U32    numOfPktTypeGroups;
    GT_U32    pktTypeGroupIdx;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT lookup;
    GT_U32    pclCfgProfileId;
    GT_U32    tcamProfileId;
    GT_U8     lookupToTcamClientGroupArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E];
    GT_U32    tcamClientGroup;
    GT_BOOL   enable;
    GT_U32    idx;
    GT_U32    ii;
    CPSS_DXCH_TCAM_CLIENT_GROUP_LOOKUP_PROFILE_CFG_STC tcamProfileCfg;
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  tcamLookupMuxTableLineCfg;
    CPSS_PCL_DIRECTION_ENT direction;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == 0)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
    CPSS_NULL_PTR_CHECK_MAC(dbHandle);
    dbPtr = (PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC*)dbHandle;
    numOfPktTypeGroups = dbPtr->db.numOfPktTypeGroups;

    /* ingress user defined bytes */
    for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
    {
        for (udbIdx = 0; (udbIdx < TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS); udbIdx++)
        {
            if ((dbPtr->db.superkeyData.udbIngressAnchorArr[pktTypeGroupIdx]
                 [udbIdx] & 0xFF) == 0xFF) continue;
            for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
            {
                if ((dbPtr->db.udbPktTypesBmpArr[pktTypeGroupIdx] & (1 << packetType)) == 0) continue;
                rc = cpssDxChPclUserDefinedByteSet(
                    devNum,
                    CPSS_DXCH_PCL_RULE_FORMAT_LAST_E   /*ruleFormat - not relevant to device*/,
                    packetType,
                    CPSS_PCL_DIRECTION_INGRESS_E,
                    udbIdx,
                    dbPtr->db.superkeyData.udbIngressAnchorArr[pktTypeGroupIdx][udbIdx],
                    dbPtr->db.superkeyData.udbIngressOffsetArr[pktTypeGroupIdx][udbIdx]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    /* egress user defined bytes */
    for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
    {
        for (udbIdx = 0; (udbIdx < TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS); udbIdx++)
        {
            if ((dbPtr->db.superkeyData.udbEgressAnchorArr[pktTypeGroupIdx]
                 [udbIdx] & 0xFF) == 0xFF) continue;
            for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
            {
                if ((dbPtr->db.udbPktTypesBmpArr[pktTypeGroupIdx] & (1 << packetType)) == 0) continue;
                rc = cpssDxChPclUserDefinedByteSet(
                    devNum,
                    CPSS_DXCH_PCL_RULE_FORMAT_LAST_E   /*ruleFormat - not relevant to device*/,
                    packetType,
                    CPSS_PCL_DIRECTION_EGRESS_E,
                    udbIdx,
                    dbPtr->db.superkeyData.udbEgressAnchorArr[pktTypeGroupIdx][udbIdx],
                    dbPtr->db.superkeyData.udbEgressOffsetArr[pktTypeGroupIdx][udbIdx]);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    /*  Action Modified Fields Selection */
    for (lookup = 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E); lookup++)
    {
        switch (lookup)
        {
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:
                direction = CPSS_PCL_DIRECTION_MIDWAY_E;
                break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
                direction = CPSS_PCL_DIRECTION_EGRESS_E;
                break;
            default:
                direction = CPSS_PCL_DIRECTION_INGRESS_E;
                break;
        }
        if ((lookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E)
            && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass))

        {
            /* NO IPCL2  */
            continue;
        }
        rc = cpssDxChPclLookupActionModifiedFieldsSelectSet(
            devNum, direction,
            lookupToPclLookupNumberArr[lookup],
            &(dbPtr->db.superkeyData.actionModifiedFieldsSelectArr[lookup]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* PCL Cfg PCL_ID + Packet Type To TCAM Profile Id */
    /* IPCL */
    for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
    {
        for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
        {
            if ((dbPtr->db.udbPktTypesBmpArr[pktTypeGroupIdx] & (1 << packetType)) == 0) continue;
            for (pclCfgProfileId = 0;
                  (pclCfgProfileId < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
                  pclCfgProfileId++)
            {
                tcamProfileId =
                    dbPtr->db.pclCfgToTcamProfileId.ipclCfgProfileToTcamProfileArr
                        [pktTypeGroupIdx][pclCfgProfileId];
                if (tcamProfileId == 0) continue;
                rc = cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet(
                    devNum,CPSS_PCL_DIRECTION_INGRESS_E,
                    packetType, pclCfgProfileId, tcamProfileId);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    /* MPCL */
    for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
    {
        for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
        {
            if ((dbPtr->db.udbPktTypesBmpArr[pktTypeGroupIdx] & (1 << packetType)) == 0) continue;
            for (pclCfgProfileId = 0;
                  (pclCfgProfileId < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
                  pclCfgProfileId++)
            {
                tcamProfileId =
                    dbPtr->db.pclCfgToTcamProfileId.mpclCfgProfileToTcamProfileArr
                        [pktTypeGroupIdx][pclCfgProfileId];
                if (tcamProfileId == 0) continue;
                rc = cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet(
                    devNum,CPSS_PCL_DIRECTION_MIDWAY_E,
                    packetType, pclCfgProfileId, tcamProfileId);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    /* EPCL */
    for (pktTypeGroupIdx = 0; (pktTypeGroupIdx < numOfPktTypeGroups); pktTypeGroupIdx++)
    {
        for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
        {
            if ((dbPtr->db.udbPktTypesBmpArr[pktTypeGroupIdx] & (1 << packetType)) == 0) continue;
            for (pclCfgProfileId = 0;
                  (pclCfgProfileId < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
                  pclCfgProfileId++)
            {
                tcamProfileId =
                    dbPtr->db.pclCfgToTcamProfileId.epclCfgProfileToTcamProfileArr
                        [pktTypeGroupIdx][pclCfgProfileId];
                if (tcamProfileId == 0) continue;
                rc = cpssDxChPclMapCfgTableProfileIdToTcamProfileIdSet(
                    devNum,CPSS_PCL_DIRECTION_EGRESS_E,
                    packetType, pclCfgProfileId, tcamProfileId);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    /* Get mapping lookup to TCAM Client Group */
    for (lookup= 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E); lookup++)
    {
        if ((lookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E)
            && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass))
        {
            /* NO IPCL2  */
            lookupToTcamClientGroupArr[lookup] = 0xFF;
            continue;
        }
        rc = cpssDxChTcamPortGroupClientGroupGet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            lookupToTcamClientArr[lookup],
            &tcamClientGroup, &enable);
        if (rc != GT_OK)
        {
            return rc;
        }
        lookupToTcamClientGroupArr[lookup] =
            (enable != GT_FALSE) ? (GT_U8)tcamClientGroup : 0xFF;
    }

    /* TCAM profiles */
    for (idx = 0; (idx < dbPtr->db.tcamSubkeysCfg.tcamProfilesAmount); idx++)
    {
        cpssOsMemSet(&tcamProfileCfg, 0, sizeof(tcamProfileCfg));
        for (ii = 0; (ii < 4); ii++)
        {
            PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(
                tcamProfileCfg.subKeySizes[ii],
                dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].tcamSubkeySizesArr[ii],
                GT_BAD_PARAM);
            tcamProfileCfg.subKeyMuxTableLineIndexes[ii] =
                dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].tcamMuxTableLineIndexesArr[ii];
            tcamProfileCfg.subKeyMuxTableLineOffsets[ii] =
                dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].tcamMuxTableLineOffsetsArr[ii];
        }
        tcamProfileCfg.sharedMuxTableLineIndex =
            dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].tcamMuxTableLineIndexesArr[4];
        tcamProfileCfg.sharedMuxTableLineOffset =
            dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].tcamMuxTableLineOffsetsArr[4];

        rc = cpssDxChTcamClientGroupLookupProfileCfgSet(
            devNum,
            lookupToTcamClientGroupArr[
                dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].pclLookup],
            dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[idx].tcamProfileId,
            &tcamProfileCfg);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* TCAM Muxing Tables */
    for (idx = 0; (idx < dbPtr->db.tcamSubkeysCfg.tcamMuxLinesAmount); idx++)
    {
        for (ii = 0; (ii < 30); ii++)
        {
            tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[ii] =
                dbPtr->db.tcamSubkeysCfg.tcamMuxLinesArr[idx].tcamSuperkeyBytePairSelectArr[ii];
        }
        rc = cpssDxChTcamClientGroupLookupMuxTableLineSet(
            devNum,
            lookupToTcamClientGroupArr[
                dbPtr->db.tcamSubkeysCfg.tcamMuxLinesArr[idx].pclLookup],
            dbPtr->db.tcamSubkeysCfg.tcamMuxLinesArr[idx].tcamMuxLineIndex,
            &tcamLookupMuxTableLineCfg);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

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
)
{
    GT_U32 ndx;
    GT_U32 ii;
    GT_U32 direction;
    GT_U32 udbIdx;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType;
    const char* name;
    const CPSS_DXCH_PCL_OFFSET_TYPE_ENT *udbOffsetPtr;
    const GT_U8 *bytePtr;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT lookup;
    GT_U32 inString;
    GT_U32 profile;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_PROFILE_ENTRY_STC  *tcamProfilePtr;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC *tcamMuxLinePtr;

    CPSS_NULL_PTR_CHECK_MAC(internalDbPtr);

    cpssOsPrintf(
        "Defined %d Packet type Groups: \n",
        internalDbPtr->numOfPktTypeGroups);
    for (ndx = 0; (ndx < internalDbPtr->numOfPktTypeGroups); ndx++)
    {
        cpssOsPrintf("Group %d -", ndx);
        for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
        {
            if ((internalDbPtr->udbPktTypesBmpArr[ndx] & (1 << packetType)) == 0) continue;
            switch (packetType)
            {
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E:           name = "IPV4_TCP";   break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E:           name = "IPV4_UDP";   break;
                case CPSS_DXCH_PCL_PACKET_TYPE_MPLS_E:               name = "MPLS";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_FRAGMENT_E:      name = "IPV4_FRAG";  break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV4_OTHER_E:         name = "IPV4_OTHER"; break;
                case CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E:     name = "ETH_OTHER "; break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE_E:                name = "UDE";        break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E:               name = "IPV6";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE1_E:               name = "UDE1";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE2_E:               name = "UDE2";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE3_E:               name = "UDE3";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE4_E:               name = "UDE4";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E:           name = "IPV6_TCP";   break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E:           name = "IPV6_UDP";   break;
                case CPSS_DXCH_PCL_PACKET_TYPE_IPV6_OTHER_E:         name = "IPV6_OTHER"; break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE5_E:               name = "UDE5";       break;
                case CPSS_DXCH_PCL_PACKET_TYPE_UDE6_E:               name = "UDE6";       break;
                default: name = "unknown"; break;
            }
            cpssOsPrintf(" %s", name);
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("User Defined Bytes Configuraton \n");
    for (direction = 0; (direction < 2); direction++)
    {
        for (ndx = 0; (ndx < internalDbPtr->numOfPktTypeGroups); ndx++)
        {
            udbOffsetPtr =
                (direction == 0)
                ? internalDbPtr->superkeyData.udbIngressAnchorArr[ndx]
                : internalDbPtr->superkeyData.udbEgressAnchorArr[ndx];
            bytePtr =
                (direction == 0)
                ? internalDbPtr->superkeyData.udbIngressOffsetArr[ndx]
                : internalDbPtr->superkeyData.udbEgressOffsetArr[ndx];
            name = (direction == 0) ? "Ingress" : "Egress";
            cpssOsPrintf("%s UDB of Packet type Group %d \n", name, ndx);
            inString = 0;
            for (udbIdx = 0; (udbIdx < TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS); udbIdx++)
            {
                if ((udbOffsetPtr[udbIdx] & 0xFF) == 0xFF) continue;
                switch (udbOffsetPtr[udbIdx])
                {
                    case CPSS_DXCH_PCL_OFFSET_L2_E:                name = "L2";          break;
                    case CPSS_DXCH_PCL_OFFSET_L4_E:                name = "L4";          break;
                    case CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E:        name = "L3-2";        break;
                    case CPSS_DXCH_PCL_OFFSET_MPLS_MINUS_2_E:      name = "MPLS-2";      break;
                    case CPSS_DXCH_PCL_OFFSET_TUNNEL_L2_E:         name = "TUNNEL_L2";   break;
                    case CPSS_DXCH_PCL_OFFSET_TUNNEL_L3_MINUS_2_E: name = "TUNNEL_L3-2"; break;
                    case CPSS_DXCH_PCL_OFFSET_METADATA_E:          name = "METADATA";    break;
                    case CPSS_DXCH_PCL_OFFSET_TUNNEL_L4_E:         name = "TUNNEL_L4";   break;
                    default: name = "unknown"; break;
                }
                cpssOsPrintf("#%d %s +%d\t", udbIdx, name, bytePtr[udbIdx]);
                inString ++;
                if (inString >= 4)
                {
                    cpssOsPrintf("\n");
                    inString = 0;
                }
            }
            cpssOsPrintf("\n");
        }
    }
    cpssOsPrintf("Action Modified Fields Selection\n");
    for (lookup = 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E); lookup++)
    {
        switch (lookup)
        {
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E: name = "IPCL0"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E: name = "IPCL1"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E: name = "IPCL2"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:  name = "MPCL ";  break;
            default: name = "unknown"; break;
        }
        bytePtr =
            internalDbPtr->superkeyData.actionModifiedFieldsSelectArr[lookup].bytePairOffsets;
        cpssOsPrintf("%s: ", name);
        for (ndx = 0;
              (ndx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS);
              ndx++)
        {
            cpssOsPrintf("%d ", bytePtr[ndx]);
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("IPCL packetTypeGroup, pclCfgProfileId  => TCAM Profile Id mapping \n");
    for (ndx = 0; (ndx < internalDbPtr->numOfPktTypeGroups); ndx++)
    {
        cpssOsPrintf("Group%d:", ndx);
        for (profile = 0;
              (profile < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
              profile++)
        {
            cpssOsPrintf(
                " %d",internalDbPtr->pclCfgToTcamProfileId.ipclCfgProfileToTcamProfileArr[ndx][profile]);
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("MPCL packetTypeGroup, pclCfgProfileId  => TCAM Profile Id mapping \n");
    for (ndx = 0; (ndx < internalDbPtr->numOfPktTypeGroups); ndx++)
    {
        cpssOsPrintf("Group%d:", ndx);
        for (profile = 0;
              (profile < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
              profile++)
        {
            cpssOsPrintf(
                " %d",internalDbPtr->pclCfgToTcamProfileId.mpclCfgProfileToTcamProfileArr[ndx][profile]);
        }
        cpssOsPrintf("\n");
    }

    cpssOsPrintf("EPCL packetTypeGroup, pclCfgProfileId  => TCAM Profile Id mapping \n");
    for (ndx = 0; (ndx < internalDbPtr->numOfPktTypeGroups); ndx++)
    {
        cpssOsPrintf("Group%d:", ndx);
        for (profile = 0;
              (profile < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
              profile++)
        {
            cpssOsPrintf(
                " %d",internalDbPtr->pclCfgToTcamProfileId.epclCfgProfileToTcamProfileArr[ndx][profile]);
        }
        cpssOsPrintf("\n");
    }

    /* TCAM Profiles */
    cpssOsPrintf("TCAM Profiles \n");
    for (ndx = 0; (ndx < internalDbPtr->tcamSubkeysCfg.tcamProfilesAmount); ndx++)
    {
        tcamProfilePtr =
            &(internalDbPtr->tcamSubkeysCfg.tcamProfilesArr[ndx]);
        switch (tcamProfilePtr->pclLookup)
        {
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E: name = "IPCL0"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E: name = "IPCL1"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E: name = "IPCL2"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:  name = "MPCL ";  break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:  name = "EPCL ";  break;
            default: name = "unknown"; break;
        }
        cpssOsPrintf("Profile%d %s keys <enum-size mux-index mux-offset>:",
            tcamProfilePtr->tcamProfileId, name);
        for (ii = 0; (ii < 4); ii++)
        {
            cpssOsPrintf(
                "<%d %d %d> ",
                tcamProfilePtr->tcamSubkeySizesArr[ii],
                tcamProfilePtr->tcamMuxTableLineIndexesArr[ii],
                tcamProfilePtr->tcamMuxTableLineOffsetsArr[ii]);
        }
        cpssOsPrintf(
            "shared mux-index %d mux-offset %d",
            tcamProfilePtr->tcamMuxTableLineIndexesArr[4],
            tcamProfilePtr->tcamMuxTableLineOffsetsArr[4]);
        cpssOsPrintf("\n");
    }

    /* TCAM Mux Tables */
    cpssOsPrintf("TCAM Muxing Lines: \n");
    for (ndx = 0; (ndx < internalDbPtr->tcamSubkeysCfg.tcamMuxLinesAmount); ndx++)
    {
        tcamMuxLinePtr =
            &(internalDbPtr->tcamSubkeysCfg.tcamMuxLinesArr[ndx]);
        switch (tcamMuxLinePtr->pclLookup)
        {
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E: name = "IPCL0"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E: name = "IPCL1"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E: name = "IPCL2"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:  name = "MPCL ";  break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:  name = "EPCL ";  break;
            default: name = "unknown"; break;
        }
        cpssOsPrintf(
            "MuxLine%d %s byte_pairs_idx:",
            tcamMuxLinePtr->tcamMuxLineIndex, name);
        for (ii = 0; (ii < 30); ii++)
        {
            cpssOsPrintf(
                " %d", tcamMuxLinePtr->tcamSuperkeyBytePairSelectArr[ii]);
        }
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32    udbIdx;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType, packetType1;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT   offsetTypeArr[CPSS_DXCH_PCL_PACKET_TYPE_LAST_E];
    GT_U8                           offsetArr[CPSS_DXCH_PCL_PACKET_TYPE_LAST_E];
    GT_U32    dir;
    GT_U32    pktTypeBmp, pktTypeBmp1;
    const char* udbAnchorNameArr[] =
    {
        "L2",
        "L3",
        "L4",
        "IPV6_EXT_HDR",
        "TCP_UDP_COMPARATOR",
        "L3-2",
        "MPLS-2",
        "TUNNEL_L2",
        "TUNNEL_L3-2",
        "META",
        "TUNNEL_L4",
        "INVALID"
    };
    CPSS_DXCH_PCL_LOOKUP_ACTION_MODIFIED_FIELDS_SELECT_STC  actionModifiedFieldsSelect;
    const char* name;
    GT_BOOL   enable;
    GT_U32    tcamClientGroup;
    GT_U8     lookupToTcamClientGroupArr[TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E];
    GT_U32    tcamClientGroupBmp;
    GT_U32    ndx;
    CPSS_PCL_DIRECTION_ENT direction;
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT lookup;
    GT_U32    pclCfgProfileId;
    GT_U32    tcamProfileIdArr[CPSS_DXCH_PCL_PACKET_TYPE_LAST_E];
    GT_U32    tcamProfileId;
    CPSS_DXCH_TCAM_CLIENT_GROUP_LOOKUP_PROFILE_CFG_STC tcamProfileCfg;
    GT_U32    profileMuxRefsBmp;
    GT_U32    muxLineId;
    CPSS_DXCH_TCAM_LOOKUP_MUX_TABLE_LINE_CFG_STC  tcamLookupMuxTableLineCfg;
    GT_U32    refMuxLineBmpArr[(PRV_TGF_TCAM_CLIENTS_TCAM_MUX_LINES_MAX_CNS + 31) / 32];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == 0)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    cpssOsPrintf("UDB PKT types:\n");
    cpssOsPrintf("0:IPV4_TCP 1:IPV4_UDP 2:MPLS 3:IPV4_FRAG 4:IPV4_OTH\n");
    cpssOsPrintf("5:ETH_OTH 6:UDE 7:IPV6 8-11:UDE1-4\n");
    cpssOsPrintf("12:IPV6_TCP 13:IPV6_UDP 14:IPV6_OTH 15-16:UDE5-6\n");
    cpssOsPrintf("\n");

    /* user defined bytes */
    for (dir = 0; (dir < 2); dir++)
    {
        direction = (dir ? CPSS_PCL_DIRECTION_EGRESS_E : CPSS_PCL_DIRECTION_INGRESS_E);
        cpssOsPrintf(dir ? "Egress UDBs\n" : "Ingress UDBs\n");
         for (udbIdx = 0; (udbIdx < TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_UDB_CNS); udbIdx++)
         {
             for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
             {
                 rc = cpssDxChPclUserDefinedByteGet(
                     devNum,
                     CPSS_DXCH_PCL_RULE_FORMAT_LAST_E   /*ruleFormat - not relevant to device*/,
                     packetType, direction,
                     udbIdx, &(offsetTypeArr[packetType]), &(offsetArr[packetType]));
                 if (rc != GT_OK)
                 {
                     offsetTypeArr[packetType] = CPSS_DXCH_PCL_OFFSET_INVALID_E;
                     offsetArr[packetType]     = 0;
                 }
             }
             pktTypeBmp1 = 0;
             packetType1 = 0xFFFFFFFF; /* avoid compiler warning */
             while (1)
             {
                 pktTypeBmp = 0;
                 for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
                 {
                     if (pktTypeBmp1 & (1 << packetType)) continue;
                     if (offsetTypeArr[packetType] ==
                         CPSS_DXCH_PCL_OFFSET_INVALID_E) continue;
                     if (pktTypeBmp == 0)
                     {
                         pktTypeBmp = (1 << packetType);
                         packetType1 = packetType;
                         continue;
                     }
                     if (offsetTypeArr[packetType] != offsetTypeArr[packetType1]) continue;
                     if (offsetArr[packetType] != offsetArr[packetType1]) continue;
                     pktTypeBmp |= (1 << packetType);
                 }
                 if (pktTypeBmp == 0) break;
                 if (pktTypeBmp1 == 0)
                 {
                     cpssOsPrintf("UDB%d: ", udbIdx);
                 }
                 cpssOsPrintf(
                     "(%s +%d 0x%X) ",
                      udbAnchorNameArr[offsetTypeArr[packetType1]],
                      offsetArr[packetType1], pktTypeBmp);
                 pktTypeBmp1 |= pktTypeBmp;
             }
             if (pktTypeBmp1 != 0)
             {
                 cpssOsPrintf("\n");
             }
         }
    }

    /*  Action Modified Fields Selection */
    cpssOsPrintf("Action Modified Fields Selection\n");
    for (lookup = 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_INGRESS_NUM_OF_E); lookup++)
    {
        if ((lookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E)
            && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass))
        {
            /* NO IPCL2  */
            continue;
        }
        rc = cpssDxChPclLookupActionModifiedFieldsSelectGet(
            devNum, CPSS_PCL_DIRECTION_INGRESS_E,
            lookupToPclLookupNumberArr[lookup],
            &actionModifiedFieldsSelect);
        if (rc != GT_OK)
        {
            return rc;
        }
        switch (lookup)
        {
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E: name = "IPCL0"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E: name = "IPCL1"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E: name = "IPCL2"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:  name = "MPCL ";  break;
            default: name = "unknown"; break;
        }
        cpssOsPrintf("%s: ", name);
        for (ndx = 0;
              (ndx < CPSS_DXCH_PCL_INGRESS_ACTION_MODIFIED_DATA_SELECTION_SIZE_CNS);
              ndx++)
        {
            cpssOsPrintf("%d ", actionModifiedFieldsSelect.bytePairOffsets[ndx]);
        }
        cpssOsPrintf("\n");
    }

    /* PCL Cfg PCL_ID + Packet Type To TCAM Profile Id */
    cpssOsPrintf("PCL Cfg PCL_ID + Packet Type To TCAM Profile Id\n");
    for (dir = 0; (dir < 3); dir++)
    {
        switch (dir)
        {
            case 0:
                direction = CPSS_PCL_DIRECTION_INGRESS_E;
                cpssOsPrintf("Ingress PCL \n");
                break;
            case 1:
                direction = CPSS_PCL_DIRECTION_MIDWAY_E;
                cpssOsPrintf("Midway PCL \n");
                break;
            case 2:
                direction = CPSS_PCL_DIRECTION_EGRESS_E;
                cpssOsPrintf("Egress PCL \n");
                break;
        }
        for (pclCfgProfileId = 0;
              (pclCfgProfileId < PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS);
              pclCfgProfileId++)
        {
            for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
            {
                rc = cpssDxChPclMapCfgTableProfileIdToTcamProfileIdGet(
                    devNum, direction,
                    packetType, pclCfgProfileId, &tcamProfileIdArr[packetType]);
                if (rc != GT_OK)
                {
                    tcamProfileIdArr[packetType] = 0;
                }
            }
            pktTypeBmp1 = 0;
            packetType1 = 0xFFFFFFFF; /* avoid compiler warning */
            while (1)
            {
                pktTypeBmp = 0;
                for (packetType = 0; (packetType < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E); packetType++)
                {
                    if (pktTypeBmp1 & (1 << packetType)) continue;
                    if (tcamProfileIdArr[packetType] == 0) continue;
                    if (pktTypeBmp == 0)
                    {
                        pktTypeBmp = (1 << packetType);
                        packetType1 = packetType;
                        continue;
                    }
                    if (tcamProfileIdArr[packetType1] != tcamProfileIdArr[packetType]) continue;
                    pktTypeBmp |= (1 << packetType);
                }
                if (pktTypeBmp == 0) break;
                if (pktTypeBmp1 == 0)
                {
                    cpssOsPrintf("pclCfgProfileId%d ", pclCfgProfileId);
                }
                cpssOsPrintf("(%d 0x%X) ", tcamProfileIdArr[packetType1], pktTypeBmp);
                pktTypeBmp1 |= pktTypeBmp;
            }
            if (pktTypeBmp1 != 0)
            {
                cpssOsPrintf("\n");
            }
        }
    }

    /* Get mapping lookup to TCAM Client Group */
    tcamClientGroupBmp = 0;
    cpssOsPrintf("lookup to TCAM Client Group\n");
    for (lookup = 0; (lookup < TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E); lookup++)
    {
        if ((lookup == TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E)
            && (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass))
        {
            /* NO IPCL2  */
            lookupToTcamClientGroupArr[lookup] = 0xFF;
            continue;
        }
        rc = cpssDxChTcamPortGroupClientGroupGet(
            devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            lookupToTcamClientArr[lookup],
            &tcamClientGroup, &enable);
        if (rc != GT_OK)
        {
            return rc;
        }
        lookupToTcamClientGroupArr[lookup] =
            (enable != GT_FALSE) ? (GT_U8)tcamClientGroup : 0xFF;
        switch (lookup)
        {
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E: name = "IPCL0"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E: name = "IPCL1"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E: name = "IPCL2"; break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:  name = "MPCL ";  break;
            case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:  name = "EPCL ";  break;
            default: name = "unknown"; break;
        }
        if (lookupToTcamClientGroupArr[lookup] != 0xFF)
        {
            cpssOsPrintf("%s -> %d\n", name, lookupToTcamClientGroupArr[lookup]);
            tcamClientGroupBmp |= (1 << lookupToTcamClientGroupArr[lookup]);
        }
    }

    /* TCAM profiles and Mux tables */
    for (tcamClientGroup = 0; (tcamClientGroup < 32); tcamClientGroup++)
    {
        if ((tcamClientGroupBmp & (1 << tcamClientGroup)) == 0) continue;
        cpssOsPrintf("--- tcamClientGroup %d\n", tcamClientGroup);
        cpssOsPrintf("TCAM Profiles: \n");
        cpssOsMemSet(refMuxLineBmpArr, 0, sizeof(refMuxLineBmpArr));
        for (tcamProfileId = 1;
              (tcamProfileId < PRV_TGF_TCAM_CLIENTS_TCAM_PROFILE_IDS_MAX_CNS);
              tcamProfileId++)
        {
            rc = cpssDxChTcamClientGroupLookupProfileCfgGet(
                devNum, tcamClientGroup, tcamProfileId, &tcamProfileCfg);
            if (rc != GT_OK)
            {
                return rc;
            }
            profileMuxRefsBmp = 0;
            for (ndx = 0; (ndx < 4); ndx++)
            {
                muxLineId = tcamProfileCfg.subKeyMuxTableLineIndexes[ndx];
                if (muxLineId != 0)
                {
                    profileMuxRefsBmp |= (1 << ndx);
                    refMuxLineBmpArr[muxLineId / 32] |= (1 << (muxLineId % 32));
                }
            }
            muxLineId = tcamProfileCfg.sharedMuxTableLineIndex;
            if (muxLineId != 0)
            {
                profileMuxRefsBmp |= (1 << 4);
                refMuxLineBmpArr[muxLineId / 32] |= (1 << (muxLineId % 32));
            }
            if (profileMuxRefsBmp == 0) continue;
            cpssOsPrintf("Profile%d ", tcamProfileId);
            for (ndx = 0; (ndx < 4); ndx++)
            {
                cpssOsPrintf(
                    "(size %d Line %d Off %d) ",
                    tcamProfileCfg.subKeySizes[ndx],
                    tcamProfileCfg.subKeyMuxTableLineIndexes[ndx],
                    tcamProfileCfg.subKeyMuxTableLineOffsets[ndx]);
            }
            cpssOsPrintf(
                "(shared Line %d Off %d)\n",
                tcamProfileCfg.sharedMuxTableLineIndex,
                tcamProfileCfg.sharedMuxTableLineOffset);
        }
        cpssOsPrintf("TCAM Muxing Table: \n");
        for (muxLineId = 1;
              (muxLineId < PRV_TGF_TCAM_CLIENTS_TCAM_MUX_LINES_MAX_CNS);
              muxLineId++)
        {
            if ((refMuxLineBmpArr[muxLineId / 32] & (1 << (muxLineId % 32))) == 0) continue;
            rc = cpssDxChTcamClientGroupLookupMuxTableLineGet(
                devNum, tcamClientGroup, muxLineId, &tcamLookupMuxTableLineCfg);
            if (rc != GT_OK)
            {
                return rc;
            }
            cpssOsPrintf("Line%d ", muxLineId);
            for (ndx = 0; (ndx < 30); ndx++)
            {
                cpssOsPrintf(
                    "%d ", tcamLookupMuxTableLineCfg.superKeyTwoByteUnitsOffsets[ndx]);
            }
            cpssOsPrintf("\n");
        }
    }

    return GT_OK;
}

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
)
{
    CPSS_PCL_DIRECTION_ENT             direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT         lookupNum;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC       lookupCfg;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT keyFormat;

    switch (pclLookup)
    {
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E:
            direction = CPSS_PCL_DIRECTION_INGRESS_E;
            lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
            keyFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E:
            direction = CPSS_PCL_DIRECTION_INGRESS_E;
            lookupNum = CPSS_PCL_LOOKUP_NUMBER_1_E;
            keyFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E:
            direction = CPSS_PCL_DIRECTION_INGRESS_E;
            lookupNum = CPSS_PCL_LOOKUP_NUMBER_2_E;
            keyFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
            direction = CPSS_PCL_DIRECTION_EGRESS_E;
            lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
            keyFormat = CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:
            direction = CPSS_PCL_DIRECTION_MIDWAY_E;
            lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
            keyFormat = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
            break;
        default: return GT_BAD_PARAM;
    }

    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));
    lookupCfg.enableLookup                 = enable;
    lookupCfg.exactMatchLookupSubProfileId = pclCfgProfileId;
    lookupCfg.pclId                        = pclId;
    /* key formats not relevant to device, but should be valid */
    lookupCfg.groupKeyTypes.nonIpKey       = keyFormat;
    lookupCfg.groupKeyTypes.ipv4Key        = keyFormat;
    lookupCfg.groupKeyTypes.ipv6Key        = keyFormat;

    return cpssDxChPclCfgTblSet(
        devNum, interfaceInfoPtr, direction, lookupNum, &lookupCfg);
}

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
)
{
    GT_STATUS rc;
    PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC *dbPtr;
    GT_U32 subFieldIdx;
    GT_U32 bitOffset;
    GT_U32 bitLength;
    GT_U32 superkeyBitOffset;
    GT_U32 superkeyBitLength;
    GT_U32 *superkeyMaskPtr;
    GT_U32 *superkeyPatternPtr;
    GT_U32 mask;
    GT_U32 pattern;
    GT_U32 superkeyIdx;
    GT_U32 superkeyShift;
    GT_U32 tcamProfileId;
    GT_U32 ndx, ii, w;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_PROFILE_ENTRY_STC *tcamProfilePtr;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;
    GT_U32 muxLineIdx;
    GT_U32 muxLineOffset;
    GT_U32 sharedMuxLineIdx;
    GT_U32 sharedMuxLineOffset;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC *muxLinePtr;
    const PRV_TGF_TCAM_CLIENTS_INTERNAL_DB_TCAM_MUX_LINE_ENTRY_STC *sharedMuxLinePtr;
    GT_U32 muxNumOfBytePairs;
    GT_U32 sharedMuxNumOfBytePairs;
    GT_U32 *subkeyMaskPtr;
    GT_U32 *subkeyPatternPtr;
    GT_U32 commonHeaderBytePairs;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == 0)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
    CPSS_NULL_PTR_CHECK_MAC(dbHandle);
    CPSS_NULL_PTR_CHECK_MAC(commonKeyHeaderMaskPtr);
    CPSS_NULL_PTR_CHECK_MAC(commonKeyHeaderPatternPtr);
    CPSS_NULL_PTR_CHECK_MAC(subFieldConditionArrPtr);
    CPSS_NULL_PTR_CHECK_MAC(tcamRuleSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtrPtr);
    CPSS_NULL_PTR_CHECK_MAC(patternPtrPtr);
    if (pclLookup >= TGF_TCAM_CLIENTS_PCL_LOOKUP_TOTAL_NUM_OF_E)
    {
        return GT_BAD_PARAM;
    }
    if (pclCfgProfileId >= PRV_TGF_TCAM_CLIENTS_PCL_AMOUNT_OF_PCL_CFG_PROFILES_CNS)
    {
        return GT_BAD_PARAM;
    }
    if (tcamSubkeyIndex >= 4)
    {
        return GT_BAD_PARAM;
    }
    if (udbPacketTypeGroupIndex >= TGF_TCAM_CLIENTS_PCL_MAX_UDB_PKT_TYPE_SETS_CNS)
    {
        return GT_BAD_PARAM;
    }

    dbPtr = (PRV_TGF_TCAM_CLIENTS_INTERNAL_HANDLED_DB_STC*)dbHandle;

    /* build superkey mask and pattern */
    cpssOsMemSet(
        &(dbPtr->workMemo.buildRule.superkeyMaskArr),
        0, sizeof(dbPtr->workMemo.buildRule.superkeyMaskArr));
    cpssOsMemSet(
        &(dbPtr->workMemo.buildRule.superkeyPatternArr),
        0, sizeof(dbPtr->workMemo.buildRule.superkeyPatternArr));
    superkeyMaskPtr    = dbPtr->workMemo.buildRule.superkeyMaskArr;
    superkeyPatternPtr = dbPtr->workMemo.buildRule.superkeyPatternArr;

    for (subFieldIdx = 0;
          (subFieldConditionArrPtr[subFieldIdx].subField.fieldId !=
           TGF_TCAM_CLIENTS_PCL_RULE_FIELD_STAMP_LIST_END_E);
          subFieldIdx++)
    {
        if (subFieldConditionArrPtr[subFieldIdx].subField.bitLength > 32)
        {
            return GT_BAD_PARAM;
        }
        /* loop over not contiguos parts of subfield in superkey */
        bitOffset = 0;
        while (1)
        {
            rc = prvTgfTcamClientsSuperkeySubfieldLayoutGet(
                &(dbPtr->db), udbPacketTypeGroupIndex, pclLookup,
                subFieldConditionArrPtr[subFieldIdx].subField.fieldId,
                (subFieldConditionArrPtr[subFieldIdx].subField.bitOffset + bitOffset),
                &superkeyBitOffset, &superkeyBitLength);
            if (rc != GT_OK)
            {
                return rc;
            }
            bitLength =
                subFieldConditionArrPtr[subFieldIdx].subField.bitLength - bitOffset;
            if (bitLength > superkeyBitLength)
            {
                bitLength = superkeyBitLength;
            }

            mask    = (bitLength >= 32) ? 0xFFFFFFFF : (GT_U32)((1 << bitLength) - 1); /* recalculated below */
            pattern = (subFieldConditionArrPtr[subFieldIdx].pattern >> bitOffset) & mask;
            mask    = (subFieldConditionArrPtr[subFieldIdx].mask >> bitOffset) & mask;

            superkeyIdx   = superkeyBitOffset / 32;
            superkeyShift = superkeyBitOffset % 32;
            superkeyMaskPtr[superkeyIdx]    |= (mask << superkeyShift);
            superkeyPatternPtr[superkeyIdx] |= (pattern << superkeyShift);
            if (bitLength > (32 - superkeyShift))
            {
                superkeyMaskPtr[superkeyIdx + 1]    |= (mask >> (32 - superkeyShift));
                superkeyPatternPtr[superkeyIdx + 1] |= (pattern >> (32 - superkeyShift));
            }
            bitOffset += bitLength;
            /* all subfield parts copied to superkey buffer */
            if (subFieldConditionArrPtr[subFieldIdx].subField.bitLength <= bitOffset) break;
        }
    }

    /* TCAM Profile Id */
    switch (pclLookup)
    {
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E:
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E:
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E:
            tcamProfileId =
                dbPtr->db.pclCfgToTcamProfileId.
                    ipclCfgProfileToTcamProfileArr[udbPacketTypeGroupIndex][pclCfgProfileId];
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:
            tcamProfileId =
                dbPtr->db.pclCfgToTcamProfileId.
                    mpclCfgProfileToTcamProfileArr[udbPacketTypeGroupIndex][pclCfgProfileId];
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
            tcamProfileId =
                dbPtr->db.pclCfgToTcamProfileId.
                    epclCfgProfileToTcamProfileArr[udbPacketTypeGroupIndex][pclCfgProfileId];
           break;
        default: return GT_BAD_PARAM;
    }
    /* TCAM Profile Config */
    ndx = dbPtr->db.subkeyIndexes.tcamProfileCfgIndexArr[pclLookup][tcamProfileId];
    if ((ndx & 0xFF) == 0xFF)
    {
        return GT_BAD_PARAM;
    }
    tcamProfilePtr = &(dbPtr->db.tcamSubkeysCfg.tcamProfilesArr[ndx]);
    PRV_CPSS_DXCH_TCAM_CONVERT_KEY_SIZE_TO_RULE_SIZE_VAL_MAC_EX(
        tcamRuleSize,
        tcamProfilePtr->tcamSubkeySizesArr[tcamSubkeyIndex],
        GT_BAD_PARAM);

    muxLineIdx          = tcamProfilePtr->tcamMuxTableLineIndexesArr[tcamSubkeyIndex];
    muxLineOffset       = tcamProfilePtr->tcamMuxTableLineOffsetsArr[tcamSubkeyIndex];
    sharedMuxLineIdx    = tcamProfilePtr->tcamMuxTableLineIndexesArr[4];
    sharedMuxLineOffset = tcamProfilePtr->tcamMuxTableLineOffsetsArr[4];

    /* Mux table lines */
    ndx = dbPtr->db.subkeyIndexes.tcamMuxLinexIndexArr[pclLookup][muxLineIdx];
    if ((ndx & 0xFF) == 0xFF)
    {
        return GT_BAD_STATE;
    }
    muxLinePtr = &(dbPtr->db.tcamSubkeysCfg.tcamMuxLinesArr[ndx]);

    /* NULL vaid for subkeys shorter or equal to 60 bytes */
    sharedMuxLinePtr = NULL;
    ndx = dbPtr->db.subkeyIndexes.tcamMuxLinexIndexArr[pclLookup][sharedMuxLineIdx];
    if ((ndx & 0xFF) != 0xFF)
    {
        sharedMuxLinePtr = &(dbPtr->db.tcamSubkeysCfg.tcamMuxLinesArr[ndx]);
    }

    switch (tcamRuleSize)
    {
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
            muxNumOfBytePairs       =  5;
            sharedMuxNumOfBytePairs =  0;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
            muxNumOfBytePairs       = 10;
            sharedMuxNumOfBytePairs =  0;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
            muxNumOfBytePairs       = 15;
            sharedMuxNumOfBytePairs =  0;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
            muxNumOfBytePairs       = 20;
            sharedMuxNumOfBytePairs =  0;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
            muxNumOfBytePairs       = 25;
            sharedMuxNumOfBytePairs =  0;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
            muxNumOfBytePairs       = 30;
            sharedMuxNumOfBytePairs =  0;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
            muxNumOfBytePairs       = 30;
            sharedMuxNumOfBytePairs = 10;
            break;
        default: return GT_BAD_STATE;
    }

    if ((muxNumOfBytePairs + (5 * muxLineOffset)) > 30)
    {
        return GT_BAD_STATE;
    }
    if ((sharedMuxNumOfBytePairs + (5 * sharedMuxLineOffset)) > 30)
    {
        return GT_BAD_STATE;
    }
    if ((sharedMuxLinePtr == 0) && (sharedMuxNumOfBytePairs > 0))
    {
        return GT_BAD_STATE;
    }

    /* build superkey mask and pattern */
    cpssOsMemSet(
        &(dbPtr->workMemo.buildRule.subkeyMaskArr),
        0, sizeof(dbPtr->workMemo.buildRule.subkeyMaskArr));
    cpssOsMemSet(
        &(dbPtr->workMemo.buildRule.subkeyPatternArr),
        0, sizeof(dbPtr->workMemo.buildRule.subkeyPatternArr));
    subkeyMaskPtr    = dbPtr->workMemo.buildRule.subkeyMaskArr;
    subkeyPatternPtr = dbPtr->workMemo.buildRule.subkeyPatternArr;

    for (ndx = 0; (ndx < muxNumOfBytePairs); ndx++)
    {
        ii = muxLinePtr->
            tcamSuperkeyBytePairSelectArr[(5 * muxLineOffset) + ndx];
        w = (superkeyMaskPtr[ii / 2] >> (16 * (ii % 2))) & 0xFFFF;
        subkeyMaskPtr[ndx / 2] |= w << (16 * (ndx % 2));
        w = (superkeyPatternPtr[ii / 2] >> (16 * (ii % 2))) & 0xFFFF;
        subkeyPatternPtr[ndx / 2] |= w << (16 * (ndx % 2));
    }
    for (ndx = 0; (ndx < sharedMuxNumOfBytePairs); ndx++)
    {
        ii = sharedMuxLinePtr->
            tcamSuperkeyBytePairSelectArr[(5 * sharedMuxLineOffset) + ndx];
        w = (superkeyMaskPtr[ii / 2] >> (16 * (ii % 2))) & 0xFFFF;
        subkeyMaskPtr[(30 / 2) + (ndx / 2)] |= w << (16 * (ndx % 2));
        w = (superkeyPatternPtr[ii / 2] >> (16 * (ii % 2))) & 0xFFFF;
        subkeyPatternPtr[(30 / 2) + (ndx / 2)] |= w << (16 * (ndx % 2));
    }

    /* common header mask and pattern copied direct to result rule */
    /* origin by bitwize OR - not via superkey                     */
    switch (pclLookup)
    {
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E:
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E:
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E:
            commonHeaderBytePairs =
                dbPtr->db.subkeyIndexes.ipclCommonHeaderMuxAmount;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E:
            commonHeaderBytePairs =
                dbPtr->db.subkeyIndexes.mpclCommonHeaderMuxAmount;
            break;
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
            commonHeaderBytePairs =
                dbPtr->db.subkeyIndexes.epclCommonHeaderMuxAmount;
            break;
        default: return GT_BAD_PARAM;
    }
    for (ndx = 0; (ndx < ((commonHeaderBytePairs + 1) / 2)); ndx++)
    {
        subkeyMaskPtr[ndx] |= commonKeyHeaderMaskPtr[ndx];
        subkeyPatternPtr[ndx] |= commonKeyHeaderPatternPtr[ndx];
    }

    *tcamRuleSizePtr = tcamRuleSize;
    *maskPtrPtr      = subkeyMaskPtr;
    *patternPtrPtr   = subkeyPatternPtr;
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32 numOfBytes;
    GT_U32 *maskPtr;
    GT_U32 *patternPtr;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT pclRuleFormat;
    GT_U8 *maskBytePtr;
    GT_U8 *patternBytePtr;
    GT_U32 byteIdx;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == 0)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
    CPSS_NULL_PTR_CHECK_MAC(pclRuleFormatPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclMaskPtr);
    CPSS_NULL_PTR_CHECK_MAC(pclPatternPtr);

    rc = tgfTcamClientsPclRuleBuild(
        devNum, dbHandle,
        pclCfgProfileId, udbPacketTypeGroupIndex, pclLookup, tcamSubkeyIndex,
        commonKeyHeaderMaskPtr, commonKeyHeaderPatternPtr, subFieldConditionArrPtr,
        &tcamRuleSize, &maskPtr, &patternPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (tcamRuleSize)
    {
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
            numOfBytes    = 10;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
            numOfBytes    = 20;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
            numOfBytes    = 30;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
            numOfBytes    = 40;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
            numOfBytes    = 50;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
            numOfBytes    = 60;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
            numOfBytes    = 80;
            break;
        default: return GT_BAD_STATE;
    }

    switch (pclLookup)
    {
        case TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E:
            maskBytePtr    = &(pclMaskPtr->ruleEgrUdbOnly.udb[0]);
            patternBytePtr = &(pclPatternPtr->ruleEgrUdbOnly.udb[0]);
            switch (tcamRuleSize)
            {
                case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_20_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_30_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_40_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_50_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_60_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_80_E;
                    break;
                default: return GT_BAD_STATE;
            }
            break;
        default: /*ingress and midway */
            maskBytePtr    = &(pclMaskPtr->ruleIngrUdbOnly.udb[0]);
            patternBytePtr = &(pclPatternPtr->ruleIngrUdbOnly.udb[0]);
            switch (tcamRuleSize)
            {
                case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_30_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_50_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_60_E;
                    break;
                case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
                    pclRuleFormat =  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_80_E;
                    break;
                default: return GT_BAD_STATE;
            }
            break;
    }


    *pclRuleFormatPtr = pclRuleFormat;
    cpssOsMemSet(pclMaskPtr, 0, sizeof(*pclMaskPtr));
    cpssOsMemSet(pclPatternPtr, 0, sizeof(*pclPatternPtr));
    for (byteIdx = 0; (byteIdx < numOfBytes); byteIdx++)
    {
        maskBytePtr[byteIdx]    = (GT_U8)((maskPtr[byteIdx / 4] >> ((byteIdx % 4) * 8)) & 0xFF);
        patternBytePtr[byteIdx] = (GT_U8)((patternPtr[byteIdx / 4] >> ((byteIdx % 4) * 8)) & 0xFF);
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    GT_U32 *maskPtr;
    GT_U32 *patternPtr;
    CPSS_DXCH_TCAM_RULE_SIZE_ENT tcamRuleSize;
    CPSS_PCL_RULE_SIZE_ENT       pclRuleSize;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == 0)
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }
    CPSS_NULL_PTR_CHECK_MAC(actionPtr);

    rc = tgfTcamClientsPclRuleBuild(
        devNum, dbHandle,
        pclCfgProfileId, udbPacketTypeGroupIndex, pclLookup, tcamSubkeyIndex,
        commonKeyHeaderMaskPtr, commonKeyHeaderPatternPtr, subFieldConditionArrPtr,
        &tcamRuleSize, &maskPtr, &patternPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch (tcamRuleSize)
    {
        case CPSS_DXCH_TCAM_RULE_SIZE_10_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_10_BYTES_E;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_20_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_20_BYTES_E;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_30_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_30_BYTES_E;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_40_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_40_BYTES_E;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_50_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_50_BYTES_E;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_60_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_60_BYTES_E;
            break;
        case CPSS_DXCH_TCAM_RULE_SIZE_80_B_E:
            pclRuleSize             =  CPSS_PCL_RULE_SIZE_80_BYTES_E;
            break;
        default: return GT_BAD_STATE;
    }

    /* write rule to TCAM */
    /* invalidate before action write */
    rc = cpssDxChTcamPortGroupRuleValidStatusSet(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        ruleIndex, GT_FALSE /*valid*/);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write action */
    rc = cpssDxChPclRuleActionUpdate(
        devNum, ruleIndex, pclRuleSize, ruleIndex, actionPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* write rule */
    rc =  cpssDxChTcamPortGroupRuleWrite(
        devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
        ruleIndex, writeValidRule,
        tcamRuleSize, patternPtr, maskPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    TGF_TCAM_CLIENTS_PCL_LOOKUP_ENT   pclLookup;
    switch (direction)
    {
        default:
        case CPSS_PCL_DIRECTION_INGRESS_E:
            switch (lookupNum)
            {
                default:
                case CPSS_PCL_LOOKUP_0_E:
                case CPSS_PCL_LOOKUP_0_0_E:
                    pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL0_E;
                    break;
                case CPSS_PCL_LOOKUP_0_1_E:
                    pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL1_E;
                    break;
                case CPSS_PCL_LOOKUP_1_E:
                    pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_IPCL2_E;
                    break;
            }
            break;
        case CPSS_PCL_DIRECTION_MIDWAY_E:
            pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_MPCL_E;
            break;
        case CPSS_PCL_DIRECTION_EGRESS_E:
            pclLookup = TGF_TCAM_CLIENTS_PCL_LOOKUP_EPCL_E;
            break;
    }
    return pclLookup;
}



