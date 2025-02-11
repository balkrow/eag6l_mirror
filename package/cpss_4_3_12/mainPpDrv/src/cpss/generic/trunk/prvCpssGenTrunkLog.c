/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenTrunkLog.c
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
#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/generic/trunk/cpssGenTrunkTypes.h>
#include <cpss/generic/trunk/private/prvCpssGenTrunkLog.h>


/********* enums *********/

const PRV_CPSS_ENUM_STRING_VALUE_PAIR_STC prvCpssLogEnum_map_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT[]  =
{PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_NATIVE_WEIGHTED_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORBID_ALL_E),
PRV_CPSS_LOG_ENUM_NAME_AND_VALUE_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_FORCE_ALL_E)
};
PRV_CPSS_LOG_STC_ENUM_MAP_ARRAY_SIZE_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT[]  =
{
    "CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_BY_CPSS_E",
    "CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_BY_APPLICATION_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_TRUNK_TYPE_ENT[]  =
{
    "CPSS_TRUNK_TYPE_FREE_E",
    "CPSS_TRUNK_TYPE_REGULAR_E",
    "CPSS_TRUNK_TYPE_CASCADE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TRUNK_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_TRUNK_LOAD_BALANCE_POLICY_TYPE_ENT[]  =
{
    "CPSS_TRUNK_LOAD_BALANCE_POLICY_REWRITE_E",
    "CPSS_TRUNK_LOAD_BALANCE_POLICY_RESILIENT_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_TRUNK_LOAD_BALANCE_POLICY_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_TRUNK_POLICY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TRUNK_POLICY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, policy, CPSS_TRUNK_LOAD_BALANCE_POLICY_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TRUNK_SHARED_PORT_INFO_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, manipulateTrunkIdPerPort);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, manipulateUcDesignatedTable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, manipulateMcDesignatedTable);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_TRUNK_WEIGHTED_MEMBER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_TRUNK_WEIGHTED_MEMBER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, member, CPSS_TRUNK_MEMBER_STC);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, weight);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, paramVal, CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAP_MAC(namePtr, *paramVal, CPSS_TRUNK_DESIGNATED_TABLE_MC_PORT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_TRUNK_POLICY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TRUNK_POLICY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TRUNK_POLICY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_TRUNK_PORT_TRUNK_ID_MANAGEMENT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_TRUNK_SHARED_PORT_INFO_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_TRUNK_SHARED_PORT_INFO_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_TRUNK_TYPE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_TRUNK_TYPE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_TRUNK_TYPE_ENT);
}

