/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenFlexeLog.c
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

/* disable deprecation warnings (if one) */
#ifdef __GNUC__
#if  (__GNUC__*100+__GNUC_MINOR__) >= 406
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/flexe/cpssFlexeTypes.h>
#include <cpss/generic/flexe/private/prvCpssGenFlexeLog.h>
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/log/prvCpssGenDbLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_FLEXE_GROUP_BANDWIDTH_ENT[]  =
{
    "CPSS_FLEXE_GROUP_BANDWIDTH_50G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_100G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_150G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_200G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_250G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_300G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_350G_E",
    "CPSS_FLEXE_GROUP_BANDWIDTH_400G_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_FLEXE_GROUP_BANDWIDTH_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_FLEXE_GROUP_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_FLEXE_GROUP_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, bandwidth, CPSS_FLEXE_GROUP_BANDWIDTH_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numPhys);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numInstances);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, instanceArr, CPSS_FLEXE_GROUP_INSTANCES_MAX_CNS, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_FLEXE_GROUP_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_FLEXE_GROUP_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_FLEXE_GROUP_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_FLEXE_GROUP_INFO_STC_PTR_groupInfoPtr = {
     "groupInfoPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_FLEXE_GROUP_INFO_STC)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssFlexeHwInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssFlexeGroupCreate_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_groupNum,
    &DX_IN_CPSS_FLEXE_GROUP_INFO_STC_PTR_groupInfoPtr
};


/********* lib API DB *********/

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChFlexeLogLibDb[] = {
    {"cpssFlexeHwInit", 2, cpssFlexeHwInit_PARAMS, NULL},
    {"cpssFlexeGroupCreate", 3, cpssFlexeGroupCreate_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_FLEX_E(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChFlexeLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChFlexeLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

