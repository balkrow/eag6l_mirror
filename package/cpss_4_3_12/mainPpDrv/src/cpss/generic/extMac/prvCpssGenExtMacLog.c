/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenExtMacLog.c
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
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortLog.h>
#include <cpss/generic/extMac/cpssExtMacDrv.h>
#include <cpss/generic/extMac/private/prvCpssGenExtMacLog.h>


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_MACDRV_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_MACDRV_OBJ_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSpeedSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSpeedGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacDuplexANSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacDuplexANGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFlowCntlANSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFlowCntlANGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSpeedANSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSpeedANGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFlowCntlSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFlowCntlGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPeriodFlowCntlSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPeriodFlowCntlGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacBackPrSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacBackPrGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortlinkGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacDuplexSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacDuplexGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortEnableSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortEnableGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacExcessiveCollisionDropSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacExcessiveCollisionDropGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPaddingEnableSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPaddingEnableGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPreambleLengthSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPreambleLengthGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacCRCCheckSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacCRCCheckGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacMRUSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacMRUGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacAutonegSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortPowerDownSetFunc);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, macCounters, CPSS_MACDRV_COUNTERS_OBJ_STC);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacTypeGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortLoopbackSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortLoopbackGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortForceLinkDownSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortForceLinkDownGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortForceLinkPassEnableSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortForceLinkPassEnableGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacHwSmiRegisterSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacHwSmiRegisterGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacHw10GSmiRegisterSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacHw10GSmiRegisterGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortAttributesGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacCscdPortTypeSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacCscdPortTypeGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortInterfaceSpeedSupportGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSaLsbSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSaLsbGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacSaBaseSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortInterfaceModeGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortTpidProfileSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacStatusGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortDefaultUPSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacCnFcTimerSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacCnFcTimerGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFcModeSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFcModeGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFcParamsSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacFcParamsGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacVctLengthOffsetSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacVctLengthOffsetGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacEeeConfigSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacEeeConfigGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacEeeStatusGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacLedGlobalConfigSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacLedGlobalConfigGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacLedPerPortConfigSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacLedPerPortConfigGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacGpioConfigSetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacGpioConfigGetFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacGpioDataReadFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacGpioDataWriteFunc);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, objStubFunc);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, phyMacType, CPSS_DXCH_CFG_REMOTE_PHY_MAC_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_MACDRV_COUNTERS_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_MACDRV_COUNTERS_OBJ_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCounterGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersOnPortGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCounterCaptureGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersCaptureOnPortGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersCaptureTriggerSet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersCaptureTriggerGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersEnable);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersEnableGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersClearOnReadSet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersClearOnReadGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersRxHistogramEnable);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersTxHistogramEnable);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersHistogramEnableGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacOversizedPacketsCounterModeSet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacOversizedPacketsCounterModeGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCounterOnPhySideGet);
    PRV_CPSS_LOG_STC_PTR_MAC(valPtr, macDrvMacPortMacCountersOnPhySidePortGet);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_MACDRV_OBJ_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_MACDRV_OBJ_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_MACDRV_OBJ_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}

