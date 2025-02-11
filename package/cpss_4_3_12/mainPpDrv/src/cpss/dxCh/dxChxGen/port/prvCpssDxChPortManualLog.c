/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPortManualLog.c
*       Manually implemented CPSS Log type wrappers
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/cpssDxChPortPizzaArbiter.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/port/private/prvCpssGenPortLog.h>
#include <cpss/common/port/cpssPortManager.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/********* enums *********/
/********* structure fields log functions *********/
void prvCpssLogParamFunc_CPSS_DXCH_PORT_MAP_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_PORT_MAP_STC*, paramVal);
    prvCpssLogArrayOfParamsHandle(contextLib,logType,namePtr,(void*) paramVal, sizeof(CPSS_DXCH_PORT_MAP_STC), inOutParamInfoPtr,
                              prvCpssLogParamFuncStc_CPSS_DXCH_PORT_MAP_STC_PTR);
}
/********* parameters log functions *********/
void prvCpssLogParamFuncStc_CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC_PTR
(
    IN    CPSS_LOG_LIB_ENT                        contextLib,
    IN    CPSS_LOG_TYPE_ENT                       logType,
    IN    GT_CHAR_PTR                             namePtr,
    IN    void                                  * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_DEV_PIZZA_ARBITER_STATE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, devType, CPSS_PP_FAMILY_TYPE_ENT);
    switch (valPtr->devType)
    {
    case CPSS_PP_FAMILY_DXCH_LION2_E:
        PRV_CPSS_LOG_STC_STC_MAC((&valPtr->devState), lion2, CPSS_DXCH_LION2_DEV_PIZZA_ARBITER_STATE_STC);
        break;
    case CPSS_PP_FAMILY_DXCH_BOBCAT2_E:
        PRV_CPSS_LOG_STC_STC_MAC((&valPtr->devState), bc2, CPSS_DXCH_BOBCAT2_DEV_PIZZA_ARBITER_STATE_STC);
        break;
    default:
        PRV_CPSS_LOG_STC_STC_MAC((&valPtr->devState), bobK, CPSS_DXCH_BOBK_DEV_PIZZA_ARBITER_STATE_STC);
        break;
    }
    prvCpssLogStcLogEnd(contextLib, logType);
}

void prvCpssLogParamFuncStc_CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_PORT_MANAGER_LOOPBACK_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, loopbackType, CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);
    switch (valPtr->loopbackType)
    {
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_MAC_E:
            PRV_CPSS_LOG_STC_BOOL_MAC((&valPtr->loopbackMode), macLoopbackEnable);
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_SERDES_E:
            PRV_CPSS_LOG_STC_ENUM_MAC((&valPtr->loopbackMode), serdesLoopbackMode, CPSS_PORT_SERDES_LOOPBACK_MODE_ENT);
            break;
        case CPSS_PORT_MANAGER_LOOPBACK_TYPE_LAST_E:
            /* nothing to add here. the attributeType 'LAST' was logged before the 'switch' scope at
               PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, loopbackType, CPSS_PORT_MANAGER_LOOPBACK_TYPE_ENT);*/
            break;
    }
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, enableRegularTraffic);
    prvCpssLogStcLogEnd(contextLib, logType);
}

void cpssDxChPortPhysicalPortMapGet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* firstPhysicalPortNumber */
    va_arg(args, GT_PHYSICAL_PORT_NUM);
    /* portMapArraySize */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}
void cpssDxChPortPhysicalPortMapSet_preLogic
(
    IN va_list  args,
    OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr
)
{
    /* devNum */
    va_arg(args, GT_U32);
    /* portMapArraySize */
    paramDataPtr->paramKey.paramKeyArr[2] = (GT_U32)va_arg(args, GT_U32);
}


