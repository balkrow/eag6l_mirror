/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssCnMLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/CnM/cpssGenericCnMShMem.h>
#include <cpss/common/CnM/private/prvCpssGenCnMLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT[]  =
{
    "CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E",
    "CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE1_E",
    "CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE0_E",
    "CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT[]  =
{
    "CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_MANAGER_E",
    "CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_HARD_E",
    "CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_RESERVE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT[]  =
{
    "CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_CODE_E",
    "CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_DATA_E",
    "CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_INTENSIVE_DATA_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT);
const char * const prvCpssLogEnum_CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT[]  =
{
    "CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_NONE_E",
    "CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_WT_E",
    "CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_WB_WA_E",
    "CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_WB_nWA_E",
    "CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_AUTO_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, callerId);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blockUse, CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfPartners);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, partnerArray);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reqNum);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, blockSize, GT_U64);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cacheability, CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scpuNum);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, allocType, CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, blockOffset, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, blockOffset, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, blockSize, GT_U64);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, memType, CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, blockUse, CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, cacheability, CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, blockOffset, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, blockSize, GT_U64);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, memType, CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, reqNum);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, freed);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR_allocBlockPtr = {
     "allocBlockPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_numOfAllocBlocksPtr = {
     "numOfAllocBlocksPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_numOfFreeBlocksPtr = {
     "numOfFreeBlocksPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_numOfScpusPtr = {
     "numOfScpusPtr", PRV_CPSS_LOG_PARAM_INOUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType = {
     "memType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_BOOL_mpuEnable = {
     "mpuEnable", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_BOOL_reinitEpb = {
     "reinitEpb", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_scpuNum = {
     "scpuNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U64_memOffset = {
     "memOffset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR_allocBlocksPtr = {
     "allocBlocksPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR_allocBlockPtr = {
     "allocBlockPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_reqNumPtr = {
     "reqNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_scpusPtr = {
     "scpusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_allocBlockArray = {
     "allocBlockArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_allocBlockSizePtr = {
     "allocBlockSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_blockOffsetPtr = {
     "blockOffsetPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_blockSizePtr = {
     "blockSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_freeBlockArray = {
     "freeBlockArray", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_freeBlockSizePtr = {
     "freeBlockSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrBlock2ClientsGetAll_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType,
    &IN_GT_U64_memOffset,
    &INOUT_GT_U32_PTR_numOfScpusPtr,
    &OUT_GT_U32_PTR_scpusPtr,
    &OUT_GT_U64_PTR_blockOffsetPtr,
    &OUT_GT_U64_PTR_blockSizePtr,
    &OUT_GT_U32_PTR_reqNumPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrAllocBlock_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType,
    &INOUT_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR_allocBlockPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrFreeMemTypeAll_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType,
    &INOUT_GT_U32_PTR_numOfAllocBlocksPtr,
    &OUT_GT_U64_PTR_allocBlockArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrMemTypeFreeMapGet_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType,
    &INOUT_GT_U32_PTR_numOfFreeBlocksPtr,
    &OUT_GT_U64_PTR_freeBlockArray
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrMemTypeAllocSizeGet_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType,
    &OUT_GT_U64_PTR_allocBlockSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrMemTypeFreeSizeGet_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType,
    &OUT_GT_U64_PTR_freeBlockSizePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrCreate_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_BOOL_mpuEnable,
    &IN_GT_BOOL_reinitEpb
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrClient2BlocksGetAll_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_scpuNum,
    &INOUT_GT_U32_PTR_numOfAllocBlocksPtr,
    &OUT_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR_allocBlocksPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericCnMShMemMgrClientRemove_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_scpuNum,
    &INOUT_GT_U32_PTR_numOfAllocBlocksPtr,
    &OUT_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR_allocBlockPtr
};


/********* lib API DB *********/


#ifdef CHX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXCnmLogLibDb[] = {
    {"cpssGenericCnMShMemMgrCreate", 3, cpssGenericCnMShMemMgrCreate_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrDelete", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrAllocBlock", 3, cpssGenericCnMShMemMgrAllocBlock_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrClient2BlocksGetAll", 4, cpssGenericCnMShMemMgrClient2BlocksGetAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrBlock2ClientsGetAll", 8, cpssGenericCnMShMemMgrBlock2ClientsGetAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrClientRemove", 4, cpssGenericCnMShMemMgrClientRemove_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrFreeMemTypeAll", 4, cpssGenericCnMShMemMgrFreeMemTypeAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeAllocMapGet", 4, cpssGenericCnMShMemMgrFreeMemTypeAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeAllocSizeGet", 3, cpssGenericCnMShMemMgrMemTypeAllocSizeGet_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeFreeMapGet", 4, cpssGenericCnMShMemMgrMemTypeFreeMapGet_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeFreeSizeGet", 3, cpssGenericCnMShMemMgrMemTypeFreeSizeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_CNM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXCnmLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXCnmLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXCnmLogLibDb[] = {
    {"cpssGenericCnMShMemMgrCreate", 3, cpssGenericCnMShMemMgrCreate_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrDelete", 1, prvCpssLogGenDevNum3_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrAllocBlock", 3, cpssGenericCnMShMemMgrAllocBlock_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrClient2BlocksGetAll", 4, cpssGenericCnMShMemMgrClient2BlocksGetAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrBlock2ClientsGetAll", 8, cpssGenericCnMShMemMgrBlock2ClientsGetAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrClientRemove", 4, cpssGenericCnMShMemMgrClientRemove_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrFreeMemTypeAll", 4, cpssGenericCnMShMemMgrFreeMemTypeAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeAllocMapGet", 4, cpssGenericCnMShMemMgrFreeMemTypeAll_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeAllocSizeGet", 3, cpssGenericCnMShMemMgrMemTypeAllocSizeGet_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeFreeMapGet", 4, cpssGenericCnMShMemMgrMemTypeFreeMapGet_PARAMS, NULL},
    {"cpssGenericCnMShMemMgrMemTypeFreeSizeGet", 3, cpssGenericCnMShMemMgrMemTypeFreeSizeGet_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_CNM(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXCnmLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXCnmLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

