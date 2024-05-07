/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenCnMLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssGenCnMLogh
#define __prvCpssGenCnMLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC_PTR_allocBlockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_numOfAllocBlocksPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_numOfFreeBlocksPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC INOUT_GT_U32_PTR_numOfScpusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT_memType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_BOOL_mpuEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_BOOL_reinitEpb;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_scpuNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U64_memOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC_PTR_allocBlocksPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC_PTR_allocBlockPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_reqNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_scpusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_allocBlockArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_allocBlockSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_blockOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_blockSizePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_freeBlockArray;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U64_PTR_freeBlockSizePtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrCreate_E = (CPSS_LOG_LIB_CNM_E << 16),
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrDelete_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrAllocBlock_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrClient2BlocksGetAll_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrBlock2ClientsGetAll_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrClientRemove_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrFreeMemTypeAll_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrMemTypeAllocMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrMemTypeAllocSizeGet_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrMemTypeFreeMapGet_E,
    PRV_CPSS_LOG_FUNC_cpssGenericCnMShMemMgrMemTypeFreeSizeGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssGenCnMLogh */
