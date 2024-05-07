/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChIpfixLog.c
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
#include <cpss/dxCh/dxChxGen/exactMatch/private/prvCpssDxChExactMatchLog.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixLog.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicerLog.h>
#include <cpss/generic/log/prvCpssGenLog.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfLog.h>


/********* enums *********/

const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E",
    "CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E",
    "CPSS_DXCH_IPFIX_FW_FLOW_TYPE_LASTLY_ACTIVE_E",
    "CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ELEPHANTS_E",
    "CPSS_DXCH_IPFIX_FW_FLOW_TYPE_LASTLY_ACTIVE_ELEPHANTS_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT[]  =
{
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_UNLIMITED_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_128_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_256_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_512_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_1K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_2K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_4K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_8K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_16K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_32K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_64K_E",
    "CPSS_DXCH_IPFIX_FW_MAX_FLOWS_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_RESET_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E",
    "CPSS_DXCH_IPFIX_RESET_MODE_LEGACY_E",
    "CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_E",
    "CPSS_DXCH_IPFIX_RESET_MODE_IPFIX_DISABLED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_ENT[]  =
{
    "CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E",
    "CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_ONE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_PHA_METADATA_MODE_DISABLE_E",
    "CPSS_DXCH_IPFIX_PHA_METADATA_MODE_COUNTER_E",
    "CPSS_DXCH_IPFIX_PHA_METADATA_MODE_RANDOM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT[]  =
{
    "CPSS_DXCH_IPFIX_SAMPLING_ACTION_ALARM_E",
    "CPSS_DXCH_IPFIX_SAMPLING_ACTION_MIRROR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT[]  =
{
    "CPSS_DXCH_IPFIX_SAMPLING_DIST_DETERMINISTIC_E",
    "CPSS_DXCH_IPFIX_SAMPLING_DIST_RANDOM_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_DISABLE_E",
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_PACKET_E",
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_BYTE_E",
    "CPSS_DXCH_IPFIX_SAMPLING_MODE_TIME_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ABSOLUTE_E",
    "CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_INCREMENTAL_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT[]  =
{
    "CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_NONE_E",
    "CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_FAIL_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_OUT_OF_RANGE_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_BAD_PARAM_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NO_RESOURCE_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_INIT_ERROR_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NOT_SUPPORTED_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_SCAN_CYCLE_ERROR_E",
    "CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_NONE_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_FW_END_REASON_ENT[]  =
{
    "CPSS_DXCH_IPFIX_FW_END_REASON_IDLE_E",
    "CPSS_DXCH_IPFIX_FW_END_REASON_LONG_LASTING_E",
    "CPSS_DXCH_IPFIX_FW_END_REASON_RESERVED_E",
    "CPSS_DXCH_IPFIX_FW_END_REASON_APP_REMOVED_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_FW_END_REASON_ENT);
const char * const prvCpssLogEnum_CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT[]  =
{
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_NONE_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_ADD_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_SET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_SET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_SET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_SET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENABLE_SET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_REMOVED_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_SET_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DELETE_ALL_COMPLETION_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_STOP_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_ALL_COMPLETION_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_LEARNED_E",
    "CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_LAST_E"
};
PRV_CPSS_LOG_STC_ENUM_ARRAY_SIZE_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_ENTRY_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, timeStamp);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetCount);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteCount, GT_U64);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, randomOffset);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, lastSampledValue, GT_U64);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, samplingWindow, GT_U64);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, samplingAction, CPSS_DXCH_IPFIX_SAMPLING_ACTION_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, logSamplingRange);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, randomFlag, CPSS_DXCH_IPFIX_SAMPLING_DIST_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, samplingMode, CPSS_DXCH_IPFIX_SAMPLING_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, cpuSubCode);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstPacketsCounter);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numberOfFirstPacketsToMirror);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, phaMetadataMode, CPSS_DXCH_IPFIX_PHA_METADATA_MODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstTimestamp);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, firstTimestampValid);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lastPacketCommand, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, lastCpuOrDropCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, packetCmd, CPSS_PACKET_CMD_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, cpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, elephantDetectionOffload);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, reportMiceEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, reportEmIndexEnable);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, pktCntThreshold, 2, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, byteCntThreshold, 2, GT_U32);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, crossCntThresholdHigh);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, crossCntThresholdLow);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, startThreshold);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, emIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstTs);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, firstTsValid);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dataPktMtu);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, txDsaTag, 4, GT_U32);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, localQueueNum);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, mgNum);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, periodicExportEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, periodicExportInterval);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, exportLastCpuCode);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, exportLastPktCmd);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, deltaMode);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, ipfixEnable);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, idleAgingOffload);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, longAgingOffload);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, countersExtensionOffload);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, timeStampsExtensionOffload);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, firstPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, idleTimeout);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, activeTimeout);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanCycleInterval);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, scanLoopItems);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, secondPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, reportNewFlows);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, hwAutoLearnEnable);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxInTransitIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxFwIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, maxAppIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstPolicerMaxIpfixIndex);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, secondPolicerMaxIpfixIndex);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, ipfixEntriesPerFlow, CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, opcodeGet, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, globalConfigReturn, CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, exportCompletion, CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_COMPLETION_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, exportConfigReturn, CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, elephantConfigReturn, CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, elephantSet, CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_SET_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, entryRemoved, CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_REMOVED_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, entryInvalidate, CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_INVALIDATE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, deleteAllCompletion, CPSS_DXCH_IPFIX_FW_IPC_MSG_DELETE_ALL_COMPLETION_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, dataClearAllCompletion, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, configError, CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_ERROR_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, entryLearned, CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_LEARNED_STC);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, portGroupConfigReturn, CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, firstPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, secondPolicerStage, CPSS_DXCH_POLICER_STAGE_TYPE_ENT);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, firstPolicerMaxIpfixIndex, CPSS_DXCH_IPFIX_FW_FALCON_TILES_MAX_CNS, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, secondPolicerMaxIpfixIndex, CPSS_DXCH_IPFIX_FW_FALCON_TILES_MAX_CNS, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMER_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_TIMER_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, nanoSecondTimer);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, secondTimer, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, uploadMode, CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_MODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, timer, CPSS_DXCH_IPFIX_TIMER_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, action, CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, dropThreshold);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, packetThreshold);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, byteThreshold, GT_U64);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_ERROR_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_CONFIG_ERROR_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, errType, CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_DELETE_ALL_COMPLETION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_DELETE_ALL_COMPLETION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfDeleted);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, elephantCfg, CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_SET_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_ELEPHANT_SET_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_BOOL_MAC(valPtr, state);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, emIndex);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_INVALIDATE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_INVALIDATE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, endReason, CPSS_DXCH_IPFIX_FW_END_REASON_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_LEARNED_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_LEARNED_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, keySize, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, emIndex);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, flowKey, 12, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_REMOVED_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_ENTRY_REMOVED_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, endReason, CPSS_DXCH_IPFIX_FW_END_REASON_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, flowId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, firstTs);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastTs);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, packetCount, 2, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, dropCount, 2, GT_U32);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, byteCount, 2, GT_U32);
    PRV_CPSS_LOG_STC_ENUM_MAP_MAC(valPtr, lastCpuCode, CPSS_NET_RX_CPU_CODE_ENT);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, lastPktCmd, CPSS_PACKET_CMD_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_COMPLETION_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_COMPLETION_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, numOfExported);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastExportedFlowId);
    PRV_CPSS_LOG_STC_NUMBER_MAC(valPtr, lastPktNum);
    PRV_CPSS_LOG_STC_TYPE_ARRAY_MAC(valPtr, lastTod, 2, GT_U32);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_EXPORT_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, exportCfg, CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_GLOBAL_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, globalCfg, CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    prvCpssLogStcLogEnd(contextLib, logType);
}
void prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_FIELD_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_PORT_GROUP_CONFIG_STC *, valPtr);
    PRV_CPSS_LOG_UNUSED_MAC(inOutParamInfoPtr);
    PRV_CPSS_LOG_STC_ENUM_MAC(valPtr, opcode, CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENT);
    PRV_CPSS_LOG_STC_STC_MAC(valPtr, portGroupCfg, CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC);
    prvCpssLogStcLogEnd(contextLib, logType);
}


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT*, paramVal);
    if (paramVal == NULL)
    {
        PRV_CPSS_LOG_AND_HISTORY_PARAM_MAC(contextLib, logType, "%s = NULL\n", namePtr);
        return;
    }
    PRV_CPSS_LOG_ENUM_MAC(namePtr, *paramVal, CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_ENTRY_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_ENTRY_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_RESET_MODE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_SET_PARAM_VAL_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT, paramVal);
    PRV_CPSS_LOG_ENUM_MAC(namePtr, paramVal, CPSS_DXCH_IPFIX_RESET_MODE_ENT);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_TIMER_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_TIMER_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMER_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}
void prvCpssLogParamFunc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
)
{
    PRV_CPSS_LOG_START_PARAM_STC_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC*, paramVal);
    prvCpssLogParamFuncStc_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR(contextLib, logType, namePtr, paramVal, inOutParamInfoPtr);
}


/********* API fields DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_mode = {
     "mode", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr = {
     "ipfixEntryPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr = {
     "firstPacketCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC_PTR_elephantCfgPtr = {
     "elephantCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC_PTR_entryParamsPtr = {
     "entryParamsPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC_PTR_exportCfgPtr = {
     "exportCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT_flowType = {
     "flowType", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC_PTR_globalCfgPtr = {
     "globalCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT_maxNumOfFlows = {
     "maxNumOfFlows", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC_PTR_portGroupCfgPtr = {
     "portGroupCfgPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset = {
     "reset", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(CPSS_DXCH_IPFIX_RESET_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr = {
     "uploadPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr = {
     "confPtr", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_endIndex = {
     "endIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_ipfixIndex = {
     "ipfixIndex", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_numOfMsgs = {
     "numOfMsgs", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_sCpuNum = {
     "sCpuNum", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U64_phaCounterMask = {
     "phaCounterMask", PRV_CPSS_LOG_PARAM_IN_E,  PRV_CPSS_LOG_FUNC_TYPE_MAC(GT_U64)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR_modePtr = {
     "modePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr = {
     "ipfixEntryPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_ENTRY_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr = {
     "firstPacketCfgPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT_PTR_ipcMsgArrPtr = {
     "ipcMsgArrPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_TIMER_STC_PTR_timerPtr = {
     "timerPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_TIMER_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr = {
     "uploadPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr = {
     "confPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_uploadStatusPtr = {
     "uploadStatusPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_BOOL)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_bmpPtr = {
     "bmpPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eventsArr = {
     "eventsArr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_eventsNumPtr = {
     "eventsNumPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_numOfMsgsFetchedPtr = {
     "numOfMsgsFetchedPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_pendingMsgBuffSizePtr = {
     "pendingMsgBuffSizePtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U32)
};
const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U64_PTR_phaCounterMaskPtr = {
     "phaCounterMaskPtr", PRV_CPSS_LOG_PARAM_OUT_E,  PRV_CPSS_LOG_FUNC_TYPE_PTR_MAC(GT_U64)
};


/********* API prototypes DB *********/

const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixDropCountModeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_mode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimestampUploadSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixWraparoundConfSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixCpuCodeSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_CPSS_NET_RX_CPU_CODE_ENT_cpuCode
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixBaseFlowIdSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_baseFlowId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset,
    &DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketsMirrorEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_ipfixIndex,
    &DX_IN_GT_BOOL_enable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketsMirrorEnableGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_ipfixIndex,
    &DX_OUT_GT_BOOL_PTR_enablePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixWraparoundStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_endIndex,
    &DX_IN_GT_BOOL_reset,
    &DX_OUT_GT_U32_PTR_bmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPhaMetadataCounterMaskSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U64_phaCounterMask
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixDropCountModeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ENT_PTR_modePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFirstPacketConfigGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC_PTR_firstPacketCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimerGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_TIMER_STC_PTR_timerPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimestampUploadGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_TIMESTAMP_UPLOAD_CONF_STC_PTR_uploadPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixWraparoundConfGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC_PTR_confPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixCpuCodeGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_CPSS_NET_RX_CPU_CODE_ENT_PTR_cpuCodePtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixBaseFlowIdGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_GT_U32_PTR_baseFlowIdPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPhaMetadataCounterMaskGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_OUT_GT_U64_PTR_phaCounterMaskPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupEntrySet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupEntryGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_entryIndex,
    &DX_IN_CPSS_DXCH_IPFIX_RESET_MODE_ENT_reset,
    &DX_OUT_CPSS_DXCH_IPFIX_ENTRY_STC_PTR_ipfixEntryPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupWraparoundStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_PORT_GROUPS_BMP_portGroupsBmp,
    &DX_IN_CPSS_DXCH_POLICER_STAGE_TYPE_ENT_stage,
    &DX_IN_GT_U32_startIndex,
    &DX_IN_GT_U32_endIndex,
    &DX_IN_GT_BOOL_reset,
    &DX_OUT_GT_U32_PTR_bmpPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwInit_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwElephantConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC_PTR_elephantCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwExportConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC_PTR_exportCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwDataGetAll_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT_flowType,
    &DX_IN_CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT_maxNumOfFlows
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwGlobalConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC_PTR_globalCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwPortGroupConfigSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC_PTR_portGroupCfgPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwEnableSet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_GT_BOOL_ipfixEnable
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwEntryDelete_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_GT_U32_flowId
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwEntryAdd_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_GT_U32_flowId,
    &DX_IN_CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC_PTR_entryParamsPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixFwIpcMsgFetch_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_IN_GT_U32_sCpuNum,
    &DX_IN_GT_U32_numOfMsgs,
    &DX_OUT_GT_U32_PTR_numOfMsgsFetchedPtr,
    &DX_OUT_GT_U32_PTR_pendingMsgBuffSizePtr,
    &DX_OUT_CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT_PTR_ipcMsgArrPtr
};
const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixTimestampUploadStatusGet_PARAMS[] =  {
    &DX_IN_GT_U8_devNum,
    &DX_OUT_GT_BOOL_PTR_uploadStatusPtr
};
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixAlarmEventsGet_PARAMS[];
extern const PRV_CPSS_LOG_FUNC_PARAM_STC * const cpssDxChIpfixPortGroupAlarmEventsGet_PARAMS[];


/********* lib API DB *********/

extern void cpssDxChIpfixAlarmEventsGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);
extern void cpssDxChIpfixPortGroupAlarmEventsGet_preLogic(IN va_list  args, OUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC * paramDataPtr);

static const PRV_CPSS_LOG_FUNC_ENTRY_STC prvCpssDxChIpfixLogLibDb[] = {
    {"cpssDxChIpfixEntrySet", 4, cpssDxChIpfixEntrySet_PARAMS, NULL},
    {"cpssDxChIpfixEntryGet", 5, cpssDxChIpfixEntryGet_PARAMS, NULL},
    {"cpssDxChIpfixDropCountModeSet", 3, cpssDxChIpfixDropCountModeSet_PARAMS, NULL},
    {"cpssDxChIpfixDropCountModeGet", 3, cpssDxChIpfixDropCountModeGet_PARAMS, NULL},
    {"cpssDxChIpfixCpuCodeSet", 3, cpssDxChIpfixCpuCodeSet_PARAMS, NULL},
    {"cpssDxChIpfixCpuCodeGet", 3, cpssDxChIpfixCpuCodeGet_PARAMS, NULL},
    {"cpssDxChIpfixAgingEnableSet", 3, prvCpssLogGenDevNumStageEnable_PARAMS, NULL},
    {"cpssDxChIpfixAgingEnableGet", 3, prvCpssLogGenDevNumStageEnablePtr_PARAMS, NULL},
    {"cpssDxChIpfixWraparoundConfSet", 3, cpssDxChIpfixWraparoundConfSet_PARAMS, NULL},
    {"cpssDxChIpfixWraparoundConfGet", 3, cpssDxChIpfixWraparoundConfGet_PARAMS, NULL},
    {"cpssDxChIpfixWraparoundStatusGet", 6, cpssDxChIpfixWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixAgingStatusGet", 6, cpssDxChIpfixWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadSet", 3, cpssDxChIpfixTimestampUploadSet_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadGet", 3, cpssDxChIpfixTimestampUploadGet_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadTrigger", 1, prvCpssLogGenDevNum_PARAMS, NULL},
    {"cpssDxChIpfixTimestampUploadStatusGet", 2, cpssDxChIpfixTimestampUploadStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixTimerGet", 3, cpssDxChIpfixTimerGet_PARAMS, NULL},
    {"cpssDxChIpfixAlarmEventsGet", 4, cpssDxChIpfixAlarmEventsGet_PARAMS, cpssDxChIpfixAlarmEventsGet_preLogic},
    {"cpssDxChIpfixPortGroupEntrySet", 5, cpssDxChIpfixPortGroupEntrySet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupEntryGet", 6, cpssDxChIpfixPortGroupEntryGet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupWraparoundStatusGet", 7, cpssDxChIpfixPortGroupWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupAgingStatusGet", 7, cpssDxChIpfixPortGroupWraparoundStatusGet_PARAMS, NULL},
    {"cpssDxChIpfixPortGroupAlarmEventsGet", 5, cpssDxChIpfixPortGroupAlarmEventsGet_PARAMS, cpssDxChIpfixPortGroupAlarmEventsGet_preLogic},
    {"cpssDxChIpfixTimestampToCpuEnableSet", 2, prvCpssLogGenDevNumEnable_PARAMS, NULL},
    {"cpssDxChIpfixTimestampToCpuEnableGet", 2, prvCpssLogGenDevNumEnablePtr_PARAMS, NULL},
    {"cpssDxChIpfixBaseFlowIdSet", 3, cpssDxChIpfixBaseFlowIdSet_PARAMS, NULL},
    {"cpssDxChIpfixBaseFlowIdGet", 3, cpssDxChIpfixBaseFlowIdGet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketConfigSet", 3, cpssDxChIpfixFirstPacketConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketConfigGet", 3, cpssDxChIpfixFirstPacketConfigGet_PARAMS, NULL},
    {"cpssDxChIpfixPhaMetadataCounterMaskSet", 3, cpssDxChIpfixPhaMetadataCounterMaskSet_PARAMS, NULL},
    {"cpssDxChIpfixPhaMetadataCounterMaskGet", 3, cpssDxChIpfixPhaMetadataCounterMaskGet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketsMirrorEnableSet", 4, cpssDxChIpfixFirstPacketsMirrorEnableSet_PARAMS, NULL},
    {"cpssDxChIpfixFirstPacketsMirrorEnableGet", 4, cpssDxChIpfixFirstPacketsMirrorEnableGet_PARAMS, NULL},
    {"cpssDxChIpfixFwInit", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwRestore", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwEnableSet", 3, cpssDxChIpfixFwEnableSet_PARAMS, NULL},
    {"cpssDxChIpfixFwGlobalConfigSet", 3, cpssDxChIpfixFwGlobalConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixFwGlobalConfigGet", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwPortGroupConfigSet", 3, cpssDxChIpfixFwPortGroupConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixFwPortGroupConfigGet", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwEntryAdd", 4, cpssDxChIpfixFwEntryAdd_PARAMS, NULL},
    {"cpssDxChIpfixFwEntryDelete", 3, cpssDxChIpfixFwEntryDelete_PARAMS, NULL},
    {"cpssDxChIpfixFwEntryDeleteAll", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwDataGet", 3, cpssDxChIpfixFwEntryDelete_PARAMS, NULL},
    {"cpssDxChIpfixFwDataGetAll", 4, cpssDxChIpfixFwDataGetAll_PARAMS, NULL},
    {"cpssDxChIpfixFwIpcMsgFetch", 6, cpssDxChIpfixFwIpcMsgFetch_PARAMS, NULL},
    {"cpssDxChIpfixFwExportConfigSet", 3, cpssDxChIpfixFwExportConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixFwExportConfigGet", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwElephantConfigSet", 3, cpssDxChIpfixFwElephantConfigSet_PARAMS, NULL},
    {"cpssDxChIpfixFwElephantConfigGet", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwDataGetAllStop", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
    {"cpssDxChIpfixFwDataClear", 3, cpssDxChIpfixFwEntryDelete_PARAMS, NULL},
    {"cpssDxChIpfixFwDataClearAll", 2, cpssDxChIpfixFwInit_PARAMS, NULL},
};

/******** DB Access Function ********/
void prvCpssLogParamLibDbGet_DX_CPSS_LOG_LIB_IPFIX(
    OUT const PRV_CPSS_LOG_FUNC_ENTRY_STC ** logFuncDbPtrPtr,
    OUT GT_U32 * logFuncDbSizePtr
)
{
    *logFuncDbPtrPtr = prvCpssDxChIpfixLogLibDb;
    *logFuncDbSizePtr = sizeof(prvCpssDxChIpfixLogLibDb) / sizeof(PRV_CPSS_LOG_FUNC_ENTRY_STC);
}

