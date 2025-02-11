/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChOamLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChOamLogh
#define __prvCpssDxChOamLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_OPCODE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_STAGE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_EXCEPTION_STATUS_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_LM_COUNTING_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_SOURCE_INTERFACE_CHECK_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_SRV_FRAME_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_OAM_SRV_MP_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_EXCEPTION_COMMAND_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_DM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_LB_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_LM_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_1731_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_DM_BASIC_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_LM_BASIC_COUNTERS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_OAM_SRV_TRANSMIT_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_EXCEPTION_COMMAND_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_OPCODE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_DM_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_LB_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_LM_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_OAM_STAGE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_EXCEPTION_COMMAND_CONFIG_STC_PTR_exceptionCommandPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC_PTR_exceptionConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_EXCEPTION_TYPE_ENT_exceptionType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_OPCODE_TYPE_ENT_opcodeType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_SRV_DM_CONFIG_STC_PTR_dmConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_SRV_LB_CONFIG_STC_PTR_lbConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_SRV_LM_CONFIG_STC_PTR_lmConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC_PTR_frameParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_SRV_SYS_PARAMS_STC_PTR_sysParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_OAM_STAGE_TYPE_ENT_stage;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_NET_RX_CPU_CODE_ENT_baseCpuCode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_clearOnRead;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_clearOnReadEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_profileEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_ETHERADDR_PTR_macAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_PTR_cookiePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_channelTypeId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cpuCodeLsBits;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cpuCodeOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hashFirstBit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_hashLastBit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_macAddIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_opcodeIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_opcodeValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_agingPeriodValue;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_AGING_BITMAP_UPDATE_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_ENTRY_STC_PTR_entryPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_EXCEPTION_COMMAND_CONFIG_STC_PTR_exceptionCommandPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_EXCEPTION_CONFIG_STC_PTR_exceptionConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_DM_CONFIG_STC_PTR_dmConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_DM_COUNTERS_STC_PTR_dmCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_LB_CONFIG_STC_PTR_lbConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_LB_COUNTERS_STC_PTR_lbCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_LM_CONFIG_STC_PTR_lmConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_LM_COUNTERS_STC_PTR_lmCountersPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_OAM_SRV_RX_FLOW_FRAME_PARAMS_STC_PTR_frameParamsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_baseCpuCodePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_profileEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_ETHERADDR_PTR_macAddrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_channelTypeIdPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cpuCodeLsBitsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_cpuCodeOffsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_entryBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_groupStatusArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashFirstBitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_hashLastBitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_macAddIndexPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_opcodeValuePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_agingPeriodValuePtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChOamEntrySet_E = (CPSS_LOG_LIB_OAM_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChOamPortGroupEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamPortGroupEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamTableBaseFlowIdSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamTableBaseFlowIdGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamOpcodeProfileDedicatedMcProfileEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamOpcodeProfileDedicatedMcProfileEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamPortGroupExceptionGroupStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionGroupStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamOpcodeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamOpcodeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamAgingPeriodEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamAgingPeriodEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamOpcodeProfilePacketCommandEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamOpcodeProfilePacketCommandEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamTimeStampEtherTypeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamTimeStampEtherTypeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamTimeStampParsingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamTimeStampParsingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamLmOffsetTableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamLmOffsetTableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamCpuCodeBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamCpuCodeBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamDualEndedLmPacketCommandSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamDualEndedLmPacketCommandGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamPortGroupExceptionStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamAgingDaemonEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamAgingDaemonEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamPduCpuCodeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamPduCpuCodeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamMplsCwChannelTypeProfileSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamMplsCwChannelTypeProfileGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamAgingBitmapUpdateModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamAgingBitmapUpdateModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamHashBitSelectionSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamHashBitSelectionGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamKeepaliveForPacketCommandEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamKeepaliveForPacketCommandEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamLmStampingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamLmStampingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionRecoveryConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionRecoveryConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionSuppressConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamExceptionSuppressConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLoopbackConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLoopbackConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLoopbackStatisticalCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLoopbackStatisticalCountersClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvDmConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvDmConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvDmStatisticalCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvDmStatisticalCountersClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLmConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLmConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLmStatisticalCountersGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLmStatisticalCountersClear_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLocalMacAddressSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLocalMacAddressGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLocalInterfaceMacIndexSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvLocalInterfaceMacIndexGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvRxFlowEntrySet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvRxFlowEntryGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChOamSrvSystemInit_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChOamLogh */
