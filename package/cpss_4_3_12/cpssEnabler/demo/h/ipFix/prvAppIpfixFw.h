/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file prvAppIpfixFw.h
*
* @brief App demo Ipfix manager header file.
*
* @version   1
********************************************************************************/

#ifndef __prvAppIpfixFwh
#define __prvAppIpfixFwh

#include <gtOs/gtOsTask.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixFw.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**************************************************************
 *                    Definitions
 **************************************************************/

/* Falcon */
#define PRV_APP_IPFIX_FW_PORT_GROUPS_MAX_CNS 8
#define PRV_APP_IPFIX_FW_EM_INDEX_PER_TILE_FALCON_MAX_CNS (_4K)
#define PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS     (_4K)
#define PRV_APP_IPFIX_FW_FLOWS_FALCON_MAX_CNS             (_32K)

/* AC5X */
#define PRV_APP_IPFIX_FW_FLOWS_AC5X_MAX_CNS                (_16K)
/* Maximum In-Transit IPFIX Index */
#define PRV_APP_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5X_CNS      ((_16K) - 1)
/* Maximum number of exact match indexes */
#define PRV_APP_IPFIX_FW_EM_INDEX_AC5X_MAX_CNS             (_16K)
#define PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_AC5X_CNS   _1K
#define PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_AC5X_MAX_CNS     _1K


/* AC5P */
#define PRV_APP_IPFIX_FW_FLOWS_AC5P_MAX_CNS                (_64K)
/* Maximum In-Transit IPFIX Index */
#define PRV_APP_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5P_CNS      ((_64K) - 1)
/* Maximum number of exact match indexes */
#define PRV_APP_IPFIX_FW_EM_INDEX_AC5P_MAX_CNS             (_64K)
#define PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_AC5P_MAX_CNS     _1K
#define PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_AC5P_CNS   _1K

#define PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(_devNum, flowId)                \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ?     \
     ((((flowId >> 12) & 0x7) * PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS) + \
      (flowId % PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS)) : flowId)

#define PRV_APP_IPFIX_FW_DB_ID_2_FLOW_ID_CONVERT(_devNum, dbId)                   \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ?      \
     ((((dbId / PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS) & 0x7) << 12) +    \
      ((dbId % PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS) & 0xFFF)) : dbId)

#define PRV_APP_IPFIX_FW_FLOW_ID_CHECK_MAC(_devNum, flowId)                         \
    if(((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&    \
        ((flowId & 0xFFF) >= PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS))    || \
       ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)   &&    \
        (flowId >= PRV_APP_IPFIX_FW_FLOWS_AC5X_MAX_CNS))                         || \
       ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)   &&    \
        (flowId >= PRV_APP_IPFIX_FW_FLOWS_AC5P_MAX_CNS)))                           \
    {                                                                               \
        return GT_OUT_OF_RANGE;                                                     \
    }

#define PRV_APP_IPFIX_FW_VALID_FLOW_ID_CONDITION(_devNum, flowId)                         \
      (((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&    \
        ((flowId & 0xFFF) >= PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS))    || \
       ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)   &&    \
        (flowId >= PRV_APP_IPFIX_FW_FLOWS_AC5X_MAX_CNS))                         || \
       ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)   &&    \
        (flowId >= PRV_APP_IPFIX_FW_FLOWS_AC5P_MAX_CNS)))

#define PRV_APP_IPFIX_FW_MAX_INTRANSIT_INDEX_CNS(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
    PRV_APP_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5X_CNS :                          \
    PRV_APP_IPFIX_FW_MAX_INTRANSIT_INDEX_AC5P_CNS)
#define   PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_CNS(_devNum)               \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
    PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_AC5X_CNS :                       \
    PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_AC5P_CNS)
#define PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(_devNum)                              \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
     PRV_APP_IPFIX_FW_FLOWS_FALCON_MAX_CNS :                                 \
     (((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ? \
      PRV_APP_IPFIX_FW_FLOWS_AC5X_MAX_CNS :                                  \
      PRV_APP_IPFIX_FW_FLOWS_AC5P_MAX_CNS)))
#define PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
     PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS :                         \
     (((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ? \
      PRV_APP_IPFIX_FW_FLOWS_AC5X_MAX_CNS :                                  \
      PRV_APP_IPFIX_FW_FLOWS_AC5P_MAX_CNS)))
#define PRV_APP_IPFIX_FW_SCAN_LOOP_ITEMS_CNS(_devNum)                       \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ? 256 : 1024);
#define PRV_APP_IPFIX_FW_EM_INDEX_MAX_CNS(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? \
     PRV_APP_IPFIX_FW_EM_INDEX_PER_TILE_FALCON_MAX_CNS :                  \
     (((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
      PRV_APP_IPFIX_FW_EM_INDEX_AC5X_MAX_CNS :                          \
      PRV_APP_IPFIX_FW_EM_INDEX_AC5P_MAX_CNS)))
#define PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_MAX_CNS(_devNum)                    \
    ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) ?   \
    PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_AC5X_MAX_CNS :                          \
    PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_AC5X_MAX_CNS)

/* Maximum Flow DB entries */
#define PRV_APP_IPFIX_FW_FLOWS_DB_ENTRIES_MAX_CNS   (PRV_APP_IPFIX_FW_FLOWS_AC5P_MAX_CNS)
#define PRV_APP_IPFIX_FW_FLOWS_BMP_ARR_SIZE_CNS     ((PRV_APP_IPFIX_FW_FLOWS_DB_ENTRIES_MAX_CNS >> 5) + 1)
#define PRV_APP_IPFIX_FW_KEYS_DB_MAX_CNS            (_64K)

#define   PRV_APP_IPFIX_FW_SERVICE_CPU_NUM_CNS    16
#define   PRV_APP_IPFIX_FW_PKT_BUFF_LEN           8

/* Config Params */
#define   PRV_APP_IPFIX_FW_ACTIVE_TIMEOUT_CNS     30
#define   PRV_APP_IPFIX_FW_DATA_PKTS_MTU_CNS      1150
#define   PRV_APP_IPFIX_FW_IDLE_TIMEOUT_CNS       30

/* Burst sizes */
#define   PRV_APP_IPFIX_FW_NEW_FLOWS_BURST_SIZE_CNS   _1K
#define   PRV_APP_IPFIX_FW_DATA_PKTS_BURST_SIZE_CNS   _1K
#define   PRV_APP_IPFIX_FW_IPC_EVENTS_BURST_SIZE_CNS  _1K

/* sleep time in milli seconds */
#define   PRV_APP_IPFIX_FW_NEW_FLOWS_SLEEP_TIME_CNS   0
#define   PRV_APP_IPFIX_FW_DATA_PKTS_SLEEP_TIME_CNS   600
#define   PRV_APP_IPFIX_FW_IPC_EVENTS_SLEEP_TIME_CNS  0
#define   PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS 10
#define   PRV_APP_IPFIX_FW_WAIT_TIME_FOR_IPC_MSG      1
#define   PRV_APP_IPFIX_FW_WAIT_TIME_AFTER_DATA_GET_ALL_CNS 500
#define   PRV_APP_IPFIX_FW_WAIT_FOR_EXPORT_COMPLETE_CNS     3000
#define   PRV_APP_IPFIX_FW_WAIT_FOR_DELETE_ALL_COMPLETE_CNS 3000

#define   PRV_APP_IPFIX_FW_EXPORT_COMPLETE_TIME_PER_TIK 10
#define   PRV_APP_IPFIX_FW_EXPORT_COMPLETE_WAIT_TIME   5000

#define   PRV_APP_IPFIX_FW_EVENT_ARRAY_SIZE_CNS       _1K
#define   PRV_APP_IPFIX_FW_BASE_FLOW_ID_CNS           0

#define   PRV_APP_IPFIX_FW_PCL_PROFILE_ID_CNS         5

#define PRV_APP_IPFIX_FW_HW_INDEXES_MAX_CNS PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.emNum
#define PRV_APP_IPFIX_FW_HW_HASHES_MAX_CNS  PRV_CPSS_DXCH_PP_MAC(devNum)->exactMatchInfo.exactMatchNumOfBanks

#define PRV_APP_IPFIX_FW_FLOW_KEY_SIZE_MAX_CNS   47

#define PRV_APP_IPFIX_FW_DATA_RECORDS_PER_PKT_MAX_CNS  _1K

#define PRV_APP_IPFIX_FW_EM_EXPANDED_ACTION_IDX_CNS    PRV_APP_IPFIX_FW_PCL_PROFILE_ID_CNS /* 1:1 mapping */
#define PRV_APP_IPFIX_FW_EM_PROFILE_IDX_CNS            8
#define PRV_APP_IPFIX_FW_MAX_NUM_ENTRIES_STEPS_CNS 256
#define PRV_APP_IPFIX_FW_VLAN_ID_CNS               5

#define PRV_APP_IPFIX_FW_FIRST_PKTS_CPU_CODE       (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 2)
#define PRV_APP_IPFIX_FW_DATA_PKTS_CPU_CODE        (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 3)
#define PRV_APP_IPFIX_FW_SYN_PKTS_CPU_CODE         (CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 4)
#define PRV_APP_IPFIX_FW_DATA_PKTS_QUEUE_NUM_CNS    3
#define PRV_APP_IPFIX_FW_FIRST_PKTS_QUEUE_NUM_CNS   4
#define PRV_APP_IPFIX_FW_SYN_PKTS_QUEUE_NUM_CNS     5


#define PRV_APP_IPFIX_FW_MAX_PORT_NUM_CNS          512

#define PRV_APP_IPFIX_FW_TEST_PORT_NUM_CNS         28
#define PRV_APP_IPFIX_FW_TEST_SRC_PORT_NUM_CNS         32
#define PRV_APP_IPFIX_FW_TEST_EGRESS_PORT_NUM_CNS         42
#define PRV_APP_IPFIX_FW_TEST_VLAN_ID_CNS         PRV_APP_IPFIX_FW_VLAN_ID_CNS
#define PRV_APP_IPFIX_FW_TEST_PCL_ID_CNS         5

/* Age limit in seconds */
#define PRV_APP_IPFIX_FW_ENTRY_IDLE_TIMEOUT_CNS       10
#define PRV_APP_IPFIX_FW_ENTRY_ACTIVE_TIMEOUT_CNS     10

#define PRV_APP_IPFIX_FW_TX_DSA_TAG_LEN_CNS     16

/* default PCL id for PCL-ID table configuration */
#define PRV_APP_IPFIX_FW_PCL_DEFAULT_ID_MAC(_direction, _lookup, _port) \
    ((_direction * 128) + (_lookup * 64) + _port)

#define __IPFIX_FW_LOG(x,...)                              \
    if ((appIpfixFwDbPtr != NULL) &&                       \
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))   \
    {                                                           \
        cpssOsPrintf("Error: %s: %d\n  " x, __FILE__, __LINE__,##__VA_ARGS__); \
    }

#define __IPFIX_FW_DBG_LOG(x,...)                              \
    if ((appIpfixFwDbPtr != NULL) &&                       \
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))   \
    {                                                           \
        cpssOsPrintf(x, ##__VA_ARGS__); \
    }

#define __IPFIX_FW_DBG_COUNTER_INCREMENT(_counterName)       \
    if ((appIpfixFwDbPtr != NULL) &&                         \
        (appIpfixFwDbPtr->dbg.debugCountersEnable == GT_TRUE))   \
    {                                                             \
        appIpfixFwDbPtr->dbg.dbgCounters._counterName ++;        \
    }

#define __IPFIX_FW_DBG_COUNTER_DECREMENT(_counterName)       \
    if ((appIpfixFwDbPtr != NULL) &&                         \
        (appIpfixFwDbPtr->dbg.debugCountersEnable == GT_TRUE))   \
    {                                                             \
        appIpfixFwDbPtr->dbg.dbgCounters._counterName --;        \
    }

#define __IPFIX_FW_DBG_COUNTER_SET(_counterName, _val)       \
    if ((appIpfixFwDbPtr != NULL) &&                         \
        (appIpfixFwDbPtr->dbg.debugCountersEnable == GT_TRUE))   \
    {                                                             \
        appIpfixFwDbPtr->dbg.dbgCounters._counterName = _val;     \
    }

#define __IPFIX_FW_DBG_COUNTER_PRINT(_counterName)               \
    if ((appIpfixFwDbPtr != NULL) &&                             \
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))         \
    {                                                                 \
        cpssOsPrintf("    %-32s: %d\n", #_counterName,              \
                     appIpfixFwDbPtr->dbg.dbgCounters._counterName);  \
    }

#define __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(_counterName)               \
    if ((appIpfixFwDbPtr != NULL) &&                             \
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))         \
    {                                                                 \
        cpssOsPrintf("    op: %-28s: %d\n", #_counterName,              \
                     appIpfixFwDbPtr->dbg.dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_##_counterName##_E]);  \
    }

#define __IPFIX_FW_VAL_COMPARE(_val1, _val2, _rc, _print)                      \
    _rc = GT_OK;                                                                    \
    if((_print == GT_TRUE) && (_val1 == _val2))                                     \
    {                                                                               \
        cpssOsPrintf(#_val1 ": %d   " #_val2 " : %d\n", _val1, _val2);       \
    }                                                                               \
    if (_val1 != _val2)                                                             \
    {                                                                               \
        cpssOsPrintf(#_val1 ": %d   " #_val2 " : %d - Not Matching\n", _val1, _val2); \
        _rc = GT_BAD_VALUE;                                                         \
    }

#define __IPFIX_FW_NULL_PTR_CHECK(varPtr)      \
    if (!varPtr)                                      \
    {                                                 \
        return GT_BAD_PTR;                            \
    }
/**************************************************************
 *                    Enumerators
 **************************************************************/

/**
 * @enum PRV_APP_IPFIX_FW_PACKET_TYPE_ENT
 *
 * @brief enumerator for packet type
 */
typedef enum{
    /* IPv4-TCP */
    PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_TCP_E,

    /* IPv4-UDP */
    PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_UDP_E,

    /* IPv6-TCP */
    PRV_APP_IPFIX_FW_PACKET_TYPE_IPV6_TCP_E,

    /* IPv6-UDP */
    PRV_APP_IPFIX_FW_PACKET_TYPE_IPV6_UDP_E,

    /* OTHER */
    PRV_APP_IPFIX_FW_PACKET_TYPE_OTHER_E,

    /* enumeration count */
    PRV_APP_IPFIX_FW_PACKET_TYPE_LAST_E = PRV_APP_IPFIX_FW_PACKET_TYPE_OTHER_E

} PRV_APP_IPFIX_FW_PACKET_TYPE_ENT;

/**
 * @enum PRV_APP_IPFIX_FW_ENTRY_STATUS_ENT
 *
 * @brief enumerator for Flow Entry status
 */
typedef enum {
    PRV_APP_IPFIX_FW_ENTRY_STATUS_NEW_E,
    PRV_APP_IPFIX_FW_ENTRY_STATUS_LAST_ACTIVE_E,
    PRV_APP_IPFIX_FW_ENTRY_STATUS_ELEPHANT_E
} PRV_APP_IPFIX_FW_ENTRY_STATUS_ENT;

/**
 * @enum PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_ENT
 *
 * @brief enumerator to select the keys lookup alogirthm
*/
typedef enum {
    /* serial lookup for keys */
    PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_SERIAL_E,

    /* hash based lookup for keys */
    PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_HASH_E
} PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_ENT;

/**************************************************************
 *                    Structures
 **************************************************************/


/**
 * @struct PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC
 *
 * @brief TX Init Parameters
 */
typedef struct {
    CPSS_NET_RX_CPU_CODE_ENT      newFlowsCpuCode;
    GT_U32                        newFlowsQueueNum;
    CPSS_NET_RX_CPU_CODE_ENT      dataPktsCpuCode;
    GT_U32                        dataPktsQueueNum;
    CPSS_NET_RX_CPU_CODE_ENT      synPktsCpuCode;
    GT_U32                        synPktsQueueNum;
} PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC
 *
 * @brief Policer Init Parameters
 */
typedef struct {
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT         policerStage;
    GT_U32                                   maxPolicerEntries;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC       waConfig;
    CPSS_DXCH_POLICER_MEMORY_STC             memoryCfg;
    GT_U32                                   baseFlowId;
    CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC  firstPacketCfgSet;
} PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC
 *
 * @brief PCL Init Parameters
 */
typedef struct {
    CPSS_PCL_DIRECTION_ENT             direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT         lookupNum;
} PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC
 *
 * @brief Exact Match Init Parameters
 */
typedef struct {
    CPSS_DXCH_EXACT_MATCH_UNIT_NUM_ENT  emUnitNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT    emLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT    emClientType;
    CPSS_PCL_DIRECTION_ENT              pclDirection;
    CPSS_PCL_LOOKUP_NUMBER_ENT          pclLookupNum;
    GT_BOOL                             pclEnableEmLookup;
    GT_U32                              pclProfileId;
    GT_U32                              pclSubProfileId;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT      emActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT emActionType;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    GT_U32                              expandedActionIndex;
} PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC
 *
 * @brief Exact Match Auto learn Init Parameters
 */
typedef struct {
    CPSS_DXCH_EXACT_MATCH_UNIT_NUM_ENT                  emUnitNum;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                    emLookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                    emClientType;
    GT_U32                                              emProfileId;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC        emProfileKeyParams;
    GT_U32                                              autoLearnProfileId;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    autoLearnDefaulActionData;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT               autoLearnActionType;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC autoLearnFlowIdConfig;
    GT_U32                              expandedActionIndex;
} PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_DBG_COUNTERS_STC
 *
 * @brief Structure for capturing various debug counters
 */
typedef struct {

    /* Number of first packets fetched by host */
    GT_U32      firstPktsFetchedByHost;

    /* Number of first packets filtered out by host because the flow is already learned */
    GT_U32      firstPktsFilteredByHost;

    /* Number of mirrored first packets dropped by the switch because of congestion */
    GT_U32      pktsDrops;

    /* Number of active flows */
    GT_U32      activeFlows;

    /* Pending termination */
    GT_U32      pendingTermination;

    /* Number of elephant flows */
    GT_U32      elephantFlows;

    /* Number of IPFIX data packets fetched by the host */
    GT_U32      dataPktsFetchedByHost;

    /* Number of IPFIX data packets dropped by the switch because of congestion */
    GT_U32      dataPktsDrops;

    /* Sync packets received by host */
    GT_U32      synPktsFetchedByHost;

    /* Sync packets filtered by host */
    GT_U32      synPktsFilteredByHost;

    /* Number of IPFIX data packets dropped by the switch because of congestion */
    GT_U32      synPktsDrops;

    /* IPC messages count */
    GT_U32      ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_LAST_E];

    /* Total IPC events */
    GT_U32      ipcEvents;

    /* EM entry Add fails */
    GT_U32      emEntryAddFails;

} PRV_APP_IPFIX_FW_DBG_COUNTERS_STC;

typedef struct {

    /* packet type (8 bits) */
    GT_U8       packetType;

    /* sequence number (16 bits) */
    GT_U16      seqNum;

    /* number of data records exported (16 bits) */
    GT_U16      numExported;

    /* flowId of the first data record exported (16 bits) */
    GT_U16      firstFlowId;

    /* flowId of the lat data record exported (16 bits) */
    GT_U16      lastFlowId;

    /* time of the day sampled while forming the packet (7 bytes) */
    GT_U32      tod[2];

} PRV_APP_IPFIX_FW_DATA_PKT_HEADER_STC;

/**
 * @struct PRV_APP_IPFIX_FW_DATA_RECORD_STC
 *
 * @brief structure to hold IPFIX flow data record
 */
typedef struct {
    /* flow identifier (16 bits) */
    GT_U32      flowId;

    /* Time Stamp of the first packet in the flow (20 bits)*/
    GT_U32      firstTs;

    /* Time stamp of the last packet in the flow (20 bits) */
    GT_U32      lastTs;

    /* packet count (40 bits) */
    GT_U32      packetCount[2];

    /* drop count (40 bits) */
    GT_U32      dropCount[2];

    /* byte count (46 bits)*/
    GT_U32      byteCount[2];

    /* entry status (3 bits) */
    PRV_APP_IPFIX_FW_ENTRY_STATUS_ENT  entryStatus;

    /* cpu/drop code (8 bits) */
    CPSS_NET_RX_CPU_CODE_ENT    lastCpuCode;

    /* packet command (3 bits) */
    CPSS_PACKET_CMD_ENT         lastPktCmd;
} PRV_APP_IPFIX_FW_DATA_RECORD_STC;

/**
 * @struct PRV_APP_IPFIX_FW_DATA_PKT_STC
 *
 * @brief structure to hold parsed data record
 */
typedef struct {

    PRV_APP_IPFIX_FW_DATA_PKT_HEADER_STC header;

    PRV_APP_IPFIX_FW_DATA_RECORD_STC dataRecord[PRV_APP_IPFIX_FW_DATA_RECORDS_PER_PKT_MAX_CNS];

} PRV_APP_IPFIX_FW_DATA_PKT_STC;

/**
 * @struct PRV_APP_IPFIX_FW_FLOW_INFO_STC
 *
 * @brief structure for flow information.
 */
typedef struct {
    GT_BOOL         isActive;
    GT_BOOL         isElephant;
    GT_BOOL         pendingTermination;
    GT_U32          emIndex;
    GT_U32          flowId;
    CPSS_DXCH_IPFIX_FW_END_REASON_ENT lastEndReason;
} PRV_APP_IPFIX_FW_FLOW_INFO_STC;

/**
 * @struct PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC
 *
 * @brief structure for IPFIX manager flows database per control pipe
 */
typedef struct {

    /* flow state */
    PRV_APP_IPFIX_FW_ENTRY_STATUS_ENT  state;

    /* packet type */
    PRV_APP_IPFIX_FW_PACKET_TYPE_ENT pktType;

    /* flow key */
    CPSS_DXCH_EXACT_MATCH_KEY_STC    key;

    /* data Record */
    PRV_APP_IPFIX_FW_DATA_RECORD_STC dataRecord;

    /* flow information */
    PRV_APP_IPFIX_FW_FLOW_INFO_STC info;

    /* flow Metadata / dsa tag info */
    CPSS_DXCH_NET_DSA_PARAMS_STC dsaParam;
} PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC;

/**
 * @struct PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC
 *
 * @brief structure to hold Keys
*/
struct PRV_APP_IPFIX_FW_KEYS_DB_NODE_STCT{
    /* flow key */
    CPSS_DXCH_EXACT_MATCH_KEY_STC    key;

    /* next key node pointer */
    struct PRV_APP_IPFIX_FW_KEYS_DB_NODE_STCT *nextKeyNodePtr;
};

typedef struct PRV_APP_IPFIX_FW_KEYS_DB_NODE_STCT PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC;

typedef struct {
    /* task id */
    GT_TASK   taskId;

    /* terminate newFlowsHandle task */
    GT_BOOL   taskTerminate;

    /* is task active */
    GT_BOOL   isTaskActive;

    /* burst size */
    GT_U32    burstSize;

    /* device number */
    GT_U32  devNum;

    /* service cpu number */
    GT_U32  sCpuNum;
} PRV_APP_IPFIX_FW_THREAD_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_TEST_PARAMS_STC
 *
 * @brief structure for test params
 */
typedef struct {
    GT_PORT_NUM     srcPort;
    GT_PORT_NUM     egressPort;
    GT_U32          newFlowsPollInterval;
    GT_U32          flowsTermPollInterval;
    GT_U32          dataPktsPollInterval;
    GT_U32          ipcFetchPollInterval;
    GT_U32          waitTimeForExportComplete;
    GT_U32          activeTimeout;
    GT_U32          idleTimeout;
} PRV_APP_IPFIX_FW_TEST_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_EMM_INIT_PARAMS_STC structure
 *
 * @brief structure for exact match manager init parameters
 */
typedef struct {
    GT_U32                                expandedActionIndex;
    GT_U32                                emmProfileEntryIndex;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT      lookupNum;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT      lookupClient;
    CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT actionType;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT      action;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT    emIpv4KeySize;
    CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT    emIpv6KeySize;
    GT_U8                                 emIpv4KeyMask[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];
    GT_U8                                 emIpv6KeyMask[CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];
} PRV_APP_IPFIX_FW_EMM_INIT_PARAMS_STC;

/**
 * @struct PRV_APP_IPFIX_FW_INIT_INFO_STC
 *
 * @brief structure for Prerequisite init parameters
 */
typedef struct {
    PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC            txInitParams;
    PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC       policerInitParams;
    PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC           pclInitParams;
    PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC            emInitParams;
    PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC emAutoLearnInitParams;
    PRV_APP_IPFIX_FW_EMM_INIT_PARAMS_STC           emmInitParams;
} PRV_APP_IPFIX_FW_INIT_INFO_STC;

/**
 * @struct PRV_APP_IPFIX_FW_CONFIG_INFO_STC
 *
 * @brief structure for IPFIX firmware config info
 */
typedef struct {
    CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    globalCfg;
    CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC    exportCfg;
    CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  elephantCfg;
    CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC portGroupCfg;
} PRV_APP_IPFIX_FW_CONFIG_INFO_STC;

/**
 * @struct PRV_APP_IPFIX_FW_THREADS_INFO_STC
 *
 * @brief structure for IPFIX App Threads information
 */
typedef struct {
    PRV_APP_IPFIX_FW_THREAD_PARAMS_STC newFlows;
    PRV_APP_IPFIX_FW_THREAD_PARAMS_STC flowsTerm;
    PRV_APP_IPFIX_FW_THREAD_PARAMS_STC dataPkts;
    PRV_APP_IPFIX_FW_THREAD_PARAMS_STC ipcFetch;
} PRV_APP_IPFIX_FW_THREADS_INFO_STC;

typedef struct {

    GT_BOOL                                  configErrorValid;
    PRV_APP_IPFIX_FW_CONFIG_INFO_STC         fwCfg;
    CPSS_DXCH_IPFIX_FW_CONFIG_ERROR_TYPE_ENT configErrorType;
    GT_U32                                   numDeleted;
} PRV_APP_IPFIX_FW_LAST_RETURN_STC;

/**
 * @struct PRV_APP_IPFIX_FW_DEBUG_INFO_STC
 *
 * @brief structure for debug information
 */
typedef struct {
    GT_BOOL             debugPrintsEnable;
    GT_BOOL             debugCountersEnable;
    GT_BOOL             cpuRxDumpEnable;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
} PRV_APP_IPFIX_FW_DEBUG_INFO_STC;

typedef struct {
    CPSS_POLICER_PACKET_SIZE_MODE_ENT                   policerPktSizeMode;
    CPSS_DXCH_IPFIX_WRAPAROUND_CFG_STC                  policerWaConfig;
    CPSS_DXCH_POLICER_MEMORY_STC                        policerMemoryCfg;
    GT_U32                                              policerBaseFlowId;
    CPSS_DXCH_IPFIX_FIRST_PACKET_CONFIG_STC             policerFirstPacketCfgSet;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC                        pclLookupCfg;
    CPSS_DXCH_EXACT_MATCH_CLIENT_ENT                    emClientType;
    GT_BOOL                                             emEnableExactMatchLookup;
    GT_U32                                              emProfileId;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                    emAction;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT    emExpandedActionOrigin;
    CPSS_DXCH_EXACT_MATCH_LOOKUP_ENT                    emAutoLearnLookupNum;
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_PARAMS_STC emAutoLearnFlowIdConfig;
    CPSS_DXCH_EXACT_MATCH_PROFILE_KEY_PARAMS_STC        emAutoLearnProfileKeyParams;
} PRV_APP_IPFIX_FW_RESTORE_CFG_STC;

typedef struct PRV_APP_IPFIX_FW_MGR_DB_STCT {

    /* exact match manager id */
    GT_U32    exactMatchManagerId[CPSS_DXCH_TILES_MAX_CNS];

    /* used Flows Bitmap */
    GT_U32    usedFlowsBmp[PRV_APP_IPFIX_FW_FLOWS_BMP_ARR_SIZE_CNS];

    /* last flow Id */
    GT_U32    lastAllocatedFlowId;

    /* keys database */
    PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *keysDb[PRV_APP_IPFIX_FW_KEYS_DB_MAX_CNS];

    /* keys lookup algo */
    PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_ENT keysLookupAlgo;

    /* hash key size */
    GT_U32   hashKeySize;

    /* Nodes added */
    GT_U32   activeKeysDbNodes;

    /* select values values for each packet type */
    GT_U8   pktUdbOffset[PRV_APP_IPFIX_FW_PACKET_TYPE_LAST_E][CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS];

    /* offset type */
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT pktUdbOffsetType[PRV_APP_IPFIX_FW_PACKET_TYPE_LAST_E][CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];

    /* Bit[0] disable port to portGroupId */
    /* Bit[1] disable keys build */
    /* Bit[2] disable keys search */
    /* Bit[3] disable flowId get */
    /* Bit[4] disable EM add */
    /* BIT[5] disable EntryAdd IPC */
    GT_U32  tmpDbgBmp;
} PRV_APP_IPFIX_FW_MGR_DB_STC;

/**
 * @struct PRV_APP_IPFIX_FW_DB_STC
 *
 * @brief structure for IPFIX manager database
 */
typedef struct PRV_APP_IPFIX_FW_DB_STCT {

    /* device number */
    GT_U8                                   devNum;

    /* pre requisite init config */
    PRV_APP_IPFIX_FW_INIT_INFO_STC          initCfg;

    /* configurations required for firmware */
    PRV_APP_IPFIX_FW_CONFIG_INFO_STC        fwCfg;

    /* threads info */
    PRV_APP_IPFIX_FW_THREADS_INFO_STC       threads;

    /* debug info */
    PRV_APP_IPFIX_FW_DEBUG_INFO_STC         dbg;

    /* flow database */
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC     flowsDb[PRV_APP_IPFIX_FW_FLOWS_DB_ENTRIES_MAX_CNS];

    /* Last IPC return data */
    PRV_APP_IPFIX_FW_LAST_RETURN_STC        lastReturnData;

    /* test params */
    PRV_APP_IPFIX_FW_TEST_PARAMS_STC        testParams;

    /* restore configurations */
    PRV_APP_IPFIX_FW_RESTORE_CFG_STC        restoreCfg;

    /* local database */
    PRV_APP_IPFIX_FW_MGR_DB_STC             mgrDb;
} PRV_APP_IPFIX_FW_DB_STC;

/**************************************************************
 *                    Functions
 **************************************************************/

/**
 * @internal appDemoIpfixFwTestParamsSet function
 * @endinternal
 *
 * @brief  Set test params for application threads
 *
 * @param[in] paramsPtr - (pointer to) test parameters
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwTestParamsSet
(
    IN PRV_APP_IPFIX_FW_TEST_PARAMS_STC *paramsPtr
);

/**
 * @internal appDemoIpfixFwTestParamsGet function
 * @endinternal
 *
 * @brief Get test params database structure
 *
 * @param[out] testParamsPtr - (pointer to) test params structure
 *
 * @retval GT_OK    - on success
 */
GT_STATUS appDemoIpfixFwTestParamsGet
(
    OUT PRV_APP_IPFIX_FW_TEST_PARAMS_STC *testParamsPtr
);

/**
 * @internal appDemoIpfixFwDbCreate function
 * @endinternal
 *
 * @brief  Create IPFIX application database and initialize parameters
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwDbCreate
(
    GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwDbDelete function
 * @endinternal
 *
 * @brief  Delete IPFIX application database
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwDbDelete
(
    GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwTxInit function
 * @endinternal
 *
 * @brief  Initialize/Configure TX queue configuration
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwTxInit
(
    IN      GT_U8                                         devNum,
    IN      PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC *paramsPtr
);

/**
 * @internal appDemoIpfixFwTxRestore function
 * @endinternal
 *
 * @brief  Restore TX queue configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwTxRestore
(
    IN      GT_U8                                         devNum
);

/**
 * @internal appDemoIpfixFwPolicerInit function
 * @endinternal
 *
 * @brief  Initialize policer engine for IPFIX manager
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPolicerInit
(
    IN  GT_U8                            devNum,
    IN  PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC *paramsPtr
);

/**
 * @internal appDemoIpfixFwPolicerRestore function
 * @endinternal
 *
 * @brief  Restore policer configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPolicerRestore
(
    IN  GT_U8                            devNum
);

/**
 * @internal appDemoIpfixFwPclInit function
 * @endinternal
 *
 * @brief  Initialize PCL and Configure UDB configuration
 *          for PCL Keys
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPclInit
(
    IN  GT_U8           devNum,
    IN  PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC  *paramsPtr
);

/**
 * @internal appDemoIpfixFwPclRestore function
 * @endinternal
 *
 * @brief  Restore UDB and PCL configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPclRestore
(
    IN  GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwEmInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match for IPFIX FW
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - init parameters
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmInit
(
    IN GT_U8           devNum,
    IN PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC *paramsPtr
);

/**
 * @internal appDemoIpfixFwEmRestore function
 * @endinternal
 *
 * @brief  Restore Exact Match for IPFIX FW
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmRestore
(
    IN GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwEmAutoLearnInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match Auto learn for IPFIX FW
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - init parameters
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmAutoLearnInit
(
    IN GT_U8           devNum,
    IN PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC *paramsPtr
);

/**
 * @internal appDemoIpfixFwEmAutoLearnRestore function
 * @endinternal
 *
 * @brief  Restore Exact Match Auto learn for IPFIX FW
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmAutoLearnRestore
(
    IN GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwEmmInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match Maanger for IPFIX manager
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmmInit
(
    IN GT_U8                                 devNum,
    IN PRV_APP_IPFIX_FW_EMM_INIT_PARAMS_STC  *paramsPtr
);

/**
 * @internal appDemoIpfixFwEmmRestore function
 * @endinternal
 *
 * @brief  Restore Exact Match Maanger configuration
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmmRestore
(
    IN  GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwAppInitParamsUpdate function
 * @endinternal
 *
 * @brief Update Init params in App Database
 *
 * @param[in] txInitParamsPtr           - (pointer to) TX init parameters
 * @param[in] policerInitParamsPtr      - (pointer to) Policer init parameters
 * @param[in] pclInitParamsPtr          - (pointer to) PCL init parameters
 * @param[in] emInitParamsPtr           - (pointer to) Exact match init parameters
 * @param[in] emAutoLearnInitParamsPtr  - (pointer to) Exact match auto learn init parameters
 * @param[in] emmInitParamsPtr          - (pointer to) Exact match manager init parameters
 *
 * @retval GT_OK    - on success
 */
GT_STATUS appDemoIpfixFwAppInitParamsUpdate
(
    IN  PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC            *txInitParamsPtr,
    IN  PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC       *policerInitParamsPtr,
    IN  PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC           *pclInitParamsPtr,
    IN  PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC            *emInitParamsPtr,
    IN  PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC *emAutoLearnInitParamsPtr,
    IN  PRV_APP_IPFIX_FW_EMM_INIT_PARAMS_STC            *emmInitParamsPtr
);

/**
 * @internal appDemoIpfixFwAppConfigParamsUpdate function
 * @endinternal
 *
 * @brief update Application configuration
 *
 * @param[in] globalCfgPtr  - global config pointer
 * @param[in] exportCfgPtr  - export config pointer
 * @param[in] elephantCfgPtr - elephant config pointer
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwAppConfigParamsUpdate
(
    IN  CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    *globalCfgPtr,
    IN  CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC    *exportCfgPtr,
    IN  CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  *elephantCfgPtr
);

/**
 * @internal appDemoIpfixFwConfigGetAndVerify function
 * @endinternal
 *
 * @brief  Read config parameters from firmware and check
 *         they are in sync with the configuration in the application database.
 *
 * @param[in] devNum    -  device number
 * @param[in] configBmp -  config bitmap.
 *                         Bit 0 - globalCfg
 *                         Bit 1 - exportCfg
 *                         Bit 2 - elephantCfg
 * @param[in] updateOrVerify - update or verify with app database
 *                             GT_TRUE - update
 *                             GT_FALSE - verify
 * @param[in] printEnable   - enable prints
 *
 * @return GT_OK  - on success
 */
GT_STATUS appDemoIpfixFwConfigGetAndVerify
(
    GT_U8           devNum,
    GT_U32          sCpuNum,
    GT_U32          configBmp,
    GT_BOOL         updateOrVerify,
    GT_BOOL         enablePrints
);

/**
 * @internal appDemoIpfixFwIpcEventHandle function
 * @endinternal
 *
 * @brief  Handle a single event.
 *
 * @param[in] devNum    -  device number
 * @param[in] ipcMsgPtr - (pointer to) IPC message that we handle.
 *
 * @return GT_OK    -  on success
 */
GT_STATUS appDemoIpfixFwIpcEventHandle
(
    IN GT_U8                               devNum,
    IN CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT *ipcMsgPtr
);

/**
 * @internal   appDemoIpfixFwThreadsStatusDump function
 * @endinternal
 *
 * @brief Show status of the threads/tasks
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixFwThreadsStatusDump
(
    GT_VOID
);

/**
 * @internal   appDemoIpfixFwEntryDelete function
 * @endinternal
 *
 * @brief App Initiated deletion of a flow
 *        APPLICABLE DEVICES: Falcon
 *
 * @param[in]  devNum        - device number
 * @param[in]  sCpuNum    - service CPU number
 * @param[in]  flowId        - flow identifier
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixFwEntryDelete
(
    IN GT_U8  devNum,
    IN GT_U32 sCpuNum,
    IN GT_U32 flowId
);

/**
 * @internal   appDemoIpfixFwEntryDeleteAll function
 * @endinternal
 *
 * @brief Delete all IPFIX entries and wait for
 *        deleteAllComplete message.
 *
 * @param[in] devNum     - device number
 * @param[in] sCpuNum    - service CPU number
 *
 * @return GT_OK    - on success
 */
GT_STATUS   appDemoIpfixFwEntryDeleteAll
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);

/**
 * @internal appDemoIpfixFwPreReqInit function
 * @endinternal
 *
 * @brief configure pre requisite settings
 *
 * @param[in] devNum - device number
 * @param[in] emKeySize - exactmatch key size
 * @param[in] hwAutoLearnEnable - GT_TRUE/GT_FALSE - enable/disable hardare uato Learn
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPreReqInit
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT emKeySize,
    IN GT_BOOL  hwAutoLearnEnable
);

/**
 * @internal   appDemoIpfixFwThreadsStart function
 * @endinternal
 *
 * @brief Start IPFIX Manager tasks
 *
 * @param[in] devNum     - device number
 * @param[in] sCpuNum    - service CPU number
 * @param[in] threadsBmp - threads bit map
 *                         BIT 0 - New flows Learning
 *                         BIT 1 - Flow termiation
 *                         BIT 2 - Data Packet collection
 *                         BIT 4 - IPC Event handling
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixFwThreadsStart
(
    IN  GT_U8   devNum,
    IN  GT_U32  sCpuNum,
    IN  GT_U32  threadsBmp
);

/**
 * @internal appDemoIpfixFwThreadsStop
 * @endinternal
 *
 * @brief   Stop IPFIX Manager tasks
 *
 * @param[in] threadsBmp - threads bit map
 *                         BIT 0 - New flows Learning
 *                         BIT 1 - Flow termiation
 *                         BIT 2 - Data Packet collection
 *                         BIT 4 - IPC Event handling
 *
 * @retval GT_OK  - on success
 */
GT_STATUS appDemoIpfixFwThreadsStop
(
    IN  GT_U32  threadsBmp
);

/**
 * @internal    appDemoIpfixFwMain function
 * @endinternal
 *
 * @brief   Perform all initializations and start IPFIX
 *          Manager tasks
 *
 * @param[in] devNum   -   device number
 * @param[in] sCpuNum  -   service CPU Number
 * @param[in] hwAutoLearnEnable - GT_TRUE/GT_FALSE - enable/disable hardare uato Learn
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwMain
(
    IN  GT_U8   devNum,
    IN  GT_U8   sCpuNum,
    IN  GT_BOOL hwAutoLearnEnable
);

/**
 * @internal appDemoIpfixFwClear function
 * @endinternal
 *
 * @brief   Stop IPFIX Manager tasks and restore configuration
 *
 * @param[in] devNum         - device number
 * @param[in] sCpuNum        - service CPU number
 * @param[in] hwAutoLearnEnable - GT_TRUE/GT_FALSE - enable/disable hardare uato Learn
 *
 * @retval   GT_OK  - on success
 */
GT_STATUS appDemoIpfixFwClear
(
    IN  GT_U8   devNum,
    IN  GT_U32  sCpuNum,
    IN  GT_BOOL  hwAutoLearnEnable
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvAppIpfixFwh */
