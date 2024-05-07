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
* @brief App demo Ipfix manager debug apis header file.
*
* @version   1
********************************************************************************/

#ifndef __prvAppIpfixFwDbgh
#define __prvAppIpfixFwDbgh

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
#include <cpss/common/cpssTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @internal appDemoIpfixFwDebugCountersReset function
 *
 * @brief  Reset debug counters
 *
 */
GT_STATUS appDemoIpfixFwDebugCountersReset
(
    GT_VOID
);

/**
 * @internal appDemoIpfixFwDebugCountersDump function
 *
 * @brief  Dump debug counters
 *
 */
GT_STATUS appDemoIpfixFwDebugCountersDump
(
    GT_VOID
);

/**
 * @internal appDemoIpfixFwStatisticsDump function
 *
 * @brief  Dump flow statistics
 *
 * @param[in] devNum       - device number
 * @param[in] firstFlowId   - start of the flow ID
 * @param[in] numFlows      - number of flows
 * @param[in] flowType      - flow type
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwStatisticsDump
(
    IN  GT_U8  devNum,
    IN  GT_U32 firstFlowId,
    IN  GT_U32 numFlows,
    IN  CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT  flowType
);

/**
 * @internal appDemoIpfixFwDebugCountersGet function
 *
 * @brief Get debug counters
 *
 * @param[in]  dbgCountersPtr - (pointer to) debug counters structure
 */
GT_STATUS appDemoIpfixFwDebugCountersGet
(
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC *dbgCountersPtr
);

/**
 * @internal appDemoIpfixFwCpuRxDumpEnable function
 * @endinternal
 *
 * @brief Enable CPU Rx Dump
 *
 * @param[in] enable - GT_TRUE/GT_FALSE enable/disable
 *
 * @retval GT_OK    - on success
 */
GT_STATUS  appDemoIpfixFwCpuRxDumpEnable
(
    IN GT_BOOL enable
);

/**
 * @internal appDemoIpfixFwDebugPrintsEnable function
 * @endinternal
 *
 * @brief Enable debug prints
 *
 * @param[in] enable - GT_TRUE/GT_FALSE enable/disable
 *
 * @retval GT_OK    - on success
 */
GT_STATUS  appDemoIpfixFwDebugPrintsEnable
(
    IN GT_BOOL enable
);


GT_VOID  appDemoIpfixFwDataRecordDump
(
    PRV_APP_IPFIX_FW_DATA_PKT_STC   *dataPktPtr
);

#if 0
GT_STATUS appDemoIpfixFwAgingEnable
(
    IN GT_BOOL     enable
);
#endif

/**
 * @internal appDemoIpfixFwFlowInfoGet
 *
 * @param[in] devNum       - device number
 * @parampin] flowId       - flowId
 * @param[in] flowInfoPtr  - flow info pointer
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwFlowInfoGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      flowId,
    OUT PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC *flowInfoPtr
);

/**
 * @internal appDemoIpfixFwFlowInfoSet
 *
 * @brief  Set Flow info. This API is to used when manually adding IPFIX entry.
 *
 * @param[in] devNum       - device number
 * @parampin] flowId       - flowId
 * @param[in] flowInfoPtr  - flow info pointer
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwFlowInfoSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      flowId,
    IN  PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC *flowInfoPtr
);

/**
 * @internal appDemoIpfixFwNextActiveFlowGet function
 *
 * @brief Get IPFIX flow information for a given flowId
 *
 * @param[in] devNum       - device number
 * @parampin] flowId       - flowId
 * @param[out] flowInfoPtr  - flow info pointer
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwNextActiveFlowGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 flowId,
    OUT PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC *flowInfoPtr
);

/**
 * @internal appDemoIpfixFwFlowStatisticsClear function
 *
 * @brief Clear statistics of a specific flow
 *
 * @param[in] devNum       - device number
 * @parampin] flowId       - flowId
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwFlowStatisticsClear
(
    IN  GT_U8           devNum,
    IN  GT_U32          flowId
);

/**
 * @internal appDemoIpfixFwFlowStatisticsClearAll function
 *
 * @brief Clear statistics of all active flows
 *
 * @param[in] devNum       - device number
 * @parampin] flowId       - flowId
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwFlowStatisticsClearAll
(
    IN  GT_U8           devNum
);

/**
 * @internal appDemoIpfixFwDbgKeysLookupAlgoSet function
 * @endinternal
 *
 * @brief Set keys lookup algorithm serial or hash based
 *
 * @retval GT_OK
*/
GT_STATUS appDemoIpfixFwDbgKeysLookupAlgoSet
(
    PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_ENT keysLookupAlgo
);

GT_STATUS appDemoIpfixFwDbgPrintKey
(
    GT_U32 size,
    GT_U8 *keyPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvAppIpfixFwDbgh */
