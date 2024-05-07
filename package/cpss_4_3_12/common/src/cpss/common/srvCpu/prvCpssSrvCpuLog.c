/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssSrvCpuLog.c
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
#include <cpss/common/srvCpu/private/prvCpssGenSrvCpuLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_bufBmp = {
     "bufBmp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_bufNum = {
     "bufNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_bufSize = {
     "bufSize", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_fw_id = {
     "fw_id", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_PTR_bufPtr = {
     "bufPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_resetOp = {
     "resetOp", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_scpuId = {
     "scpuId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_srvCpuId = {
     "srvCpuId", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U8)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC IN_const_char = {
     "char", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(const)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_status = {
     "status", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC _IN_GT_CHAR = {
     "GT_CHAR", ,  PRV_CPSS_LOG_FUNC_TYPE_MAC(IN)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericSrvCpuFWUpgrade_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U32_bufBmp,
    &IN_GT_U32_bufNum,
    &IN_GT_U32_bufSize,
    &IN_GT_U8_PTR_bufPtr,
    &IN_GT_U8_scpuId,
    &OUT_GT_U32_PTR_status
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssGenericSrvCpuResetUnreset_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U8_scpuId,
    &IN_GT_U8_resetOp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssSrvCpuSetOptionalFW_PARAMS[] =  {
    &IN_GT_U8_devNum,
    &IN_GT_U8_srvCpuId,
    &IN_GT_U32_fw_id,
    &_IN_GT_CHAR
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssSrvCpuSetUsrPath_PARAMS[] =  {
    &IN_const_char
};


/********* lib API DB *********/


#ifdef CHX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonDXSrvCpuLogLibDb[] = {
    {"cpssSrvCpuSetOptionalFW", 4, cpssSrvCpuSetOptionalFW_PARAMS, NULL},
    {"cpssSrvCpuSetUsrPath", 1, cpssSrvCpuSetUsrPath_PARAMS, NULL},
    {"cpssGenericSrvCpuResetUnreset", 3, cpssGenericSrvCpuResetUnreset_PARAMS, NULL},
    {"cpssGenericSrvCpuFWUpgrade", 7, cpssGenericSrvCpuFWUpgrade_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_SRV_CPU(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonDXSrvCpuLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonDXSrvCpuLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

#ifdef PX_FAMILY
static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssCommonPXSrvCpuLogLibDb[] = {
    {"cpssSrvCpuSetOptionalFW", 4, cpssSrvCpuSetOptionalFW_PARAMS, NULL},
    {"cpssSrvCpuSetUsrPath", 1, cpssSrvCpuSetUsrPath_PARAMS, NULL},
    {"cpssGenericSrvCpuResetUnreset", 3, cpssGenericSrvCpuResetUnreset_PARAMS, NULL},
    {"cpssGenericSrvCpuFWUpgrade", 7, cpssGenericSrvCpuFWUpgrade_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_PX_CPSS_LOG_LIB_SRV_CPU(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssCommonPXSrvCpuLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssCommonPXSrvCpuLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}
#endif

