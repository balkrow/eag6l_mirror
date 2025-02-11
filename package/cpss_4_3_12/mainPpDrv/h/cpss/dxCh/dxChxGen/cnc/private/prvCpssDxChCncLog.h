/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChCncLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChCncLogh
#define __prvCpssDxChCncLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* manually implemented declarations *********/

void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_CLIENT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_COUNTER_FORMAT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_EGRESS_PACKET_TYPE_CLIENT_INDEX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_TM_INDEX_MODE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_CNC_BLOCK_SHARED_CLIENTS_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);
void prvCpssLogParamFuncStc_CPSS_DXCH_CNC_COUNTER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_CNC_BLOCK_SHARED_CLIENTS_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_CLIENT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_PACKET_TYPE_CLIENT_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_PACKET_TYPE_CLIENT_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_indexNumPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_INOUT_GT_U32_PTR_numOfCounterValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DMA_QUEUE_CFG_STC_PTR_cncCfgPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_BLOCK_SHARED_CLIENTS_CONFIG_STC_PTR_clientsConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_countMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_client;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_CLIENT_ENT_vlanClient;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_format;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_ENT_cncUnit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_EGRESS_PACKET_TYPE_CLIENT_INDEX_MODE_ENT_indexMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_mode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_toCpuMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_indexMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_indexMode;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_updateEnable;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_blockNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_clientIndex;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_cncUnitId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_portLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueBase;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_queueLimit;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_HEX_PTR_indexRangesBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_BLOCK_SHARED_CLIENTS_CONFIG_STC_PTR_clientsConfigPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_BYTE_COUNT_MODE_ENT_PTR_countModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_COUNTER_FORMAT_ENT_PTR_formatPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_COUNTER_STC_PTR_counterValuesPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_EGRESS_DROP_COUNT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_EGRESS_PACKET_TYPE_CLIENT_INDEX_MODE_ENT_PTR_indexModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_EGRESS_QUEUE_CLIENT_MODE_ENT_PTR_modePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_PACKET_TYPE_PASS_DROP_TO_CPU_MODE_ENT_PTR_toCpuModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_TM_INDEX_MODE_ENT_PTR_indexModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_CNC_VLAN_INDEX_MODE_ENT_PTR_indexModePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_updateEnablePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_inProcessBlocksBmpPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portGroupIdArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_portLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueBasePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_queueLimitPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_HEX_PTR_indexRangesBmpPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockClientEnableSet_E = (CPSS_LOG_LIB_CNC_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockClientEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockClientRangesSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockClientRangesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortClientEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortClientEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncIngressVlanPassDropFromCpuCountEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncIngressVlanPassDropFromCpuCountEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncVlanClientIndexModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncVlanClientIndexModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPacketTypePassDropToCpuModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPacketTypePassDropToCpuModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncTmClientIndexModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncTmClientIndexModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncClientByteCountModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncClientByteCountModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncEgressVlanDropCountModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncEgressVlanDropCountModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterClearByReadEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterClearByReadEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterClearByReadValueSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterClearByReadValueGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterWraparoundEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterWraparoundEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterWraparoundIndexesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockUploadTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockUploadInProcessGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncUploadedBlockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCountingEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCountingEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterFormatSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCounterFormatGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncEgressQueueClientModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncEgressQueueClientModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCpuAccessStrictPriorityEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncCpuAccessStrictPriorityEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupBlockClientEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupBlockClientEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupBlockClientRangesSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupBlockClientRangesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupCounterWraparoundIndexesGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupCounterSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupCounterGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupBlockUploadTrigger_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupBlockUploadInProcessGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupUploadedBlockGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupCounterFormatSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortGroupCounterFormatGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncOffsetForNatClientSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncOffsetForNatClientGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncQueueStatusLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncQueueStatusLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortStatusLimitSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortStatusLimitGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncIngressPacketTypeClientHashModeEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncIngressPacketTypeClientHashModeEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortHashClientEnableSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortHashClientEnableGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortQueueGroupBaseSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncPortQueueGroupBaseGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockSharedClientConfigSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncBlockSharedClientConfigGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncEgressPacketTypePassDropClientIndexModeSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncEgressPacketTypePassDropClientIndexModeGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChCncUploadInit_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChCncLogh */
