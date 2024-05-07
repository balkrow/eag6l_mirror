/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvAppIpfixFwDbg.c
*
* @brief Application Code Debug APIs for IPFIX Manager
*
* @version   1
*********************************************************************************
**/

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
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManagerSamples.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <ipFix/prvAppIpfixFw.h>
#include <ipFix/prvAppIpfixFwDbg.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixFw.h>

extern PRV_APP_IPFIX_FW_DB_STC *appIpfixFwDbPtr;

extern void dxChNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
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
)
{
    if ((appIpfixFwDbPtr != NULL) &&
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))
    {
        cpssOsPrintf("Debug Counters:\n");
    }

    /*__IPFIX_FW_DBG_COUNTER_PRINT(pktsMirroredToHost);*/
    __IPFIX_FW_DBG_COUNTER_PRINT(firstPktsFetchedByHost);
    __IPFIX_FW_DBG_COUNTER_PRINT(firstPktsFilteredByHost);
    __IPFIX_FW_DBG_COUNTER_PRINT(pktsDrops);
    __IPFIX_FW_DBG_COUNTER_PRINT(activeFlows);
    if ((appIpfixFwDbPtr != NULL) &&
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))
    {
        cpssOsPrintf("    activeKeysDbNodes               : %d\n",
                     appIpfixFwDbPtr->mgrDb.activeKeysDbNodes);
    }
    __IPFIX_FW_DBG_COUNTER_PRINT(pendingTermination);
    __IPFIX_FW_DBG_COUNTER_PRINT(elephantFlows);
    __IPFIX_FW_DBG_COUNTER_PRINT(dataPktsFetchedByHost);
    __IPFIX_FW_DBG_COUNTER_PRINT(dataPktsDrops);
    __IPFIX_FW_DBG_COUNTER_PRINT(synPktsFetchedByHost);
    __IPFIX_FW_DBG_COUNTER_PRINT(synPktsFilteredByHost);
    __IPFIX_FW_DBG_COUNTER_PRINT(synPktsDrops);
    __IPFIX_FW_DBG_COUNTER_PRINT(emEntryAddFails);

    /* Dump IPC counters */
    __IPFIX_FW_DBG_COUNTER_PRINT(ipcEvents);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(CONFIG_GET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(CONFIG_ERROR);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(GLOBAL_CONFIG_SET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(GLOBAL_CONFIG_RETURN);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(PORT_GROUP_CONFIG_SET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(PORT_GROUP_CONFIG_RETURN);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(EXPORT_CONFIG_SET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(EXPORT_CONFIG_RETURN);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ELEPHANT_CONFIG_SET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ELEPHANT_CONFIG_RETURN);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENABLE_SET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENTRY_ADD);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENTRY_REMOVED);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENTRY_INVALIDATE);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENTRY_DELETE);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENTRY_DELETE_ALL);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(DELETE_ALL_COMPLETION);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(DATA_GET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(DATA_GET_ALL);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(EXPORT_COMPLETION);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ELEPHANT_SET);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(DATA_CLEAR);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(DATA_GET_ALL_STOP);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(DATA_CLEAR_ALL_COMPLETION);
    __IPFIX_FW_DBG_IPC_MSG_COUNTER_PRINT(ENTRY_LEARNED);

    if ((appIpfixFwDbPtr != NULL) &&
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))
    {
        cpssOsPrintf("\n");
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwDebugCountersReset function
 *
 * @brief  Reset debug counters
 *
 */
GT_STATUS appDemoIpfixFwDebugCountersReset
(
    GT_VOID
)
{
    GT_U32  activeFlows = 0;
    GT_U32  activeKeysDbNodes = 0;

    activeFlows = appIpfixFwDbPtr->dbg.dbgCounters.activeFlows;
    activeKeysDbNodes = appIpfixFwDbPtr->mgrDb.activeKeysDbNodes;
    cpssOsMemSet(&appIpfixFwDbPtr->dbg.dbgCounters, 0,
                 sizeof(appIpfixFwDbPtr->dbg.dbgCounters));

    appIpfixFwDbPtr->dbg.dbgCounters.activeFlows = activeFlows;
    appIpfixFwDbPtr->mgrDb.activeKeysDbNodes = activeKeysDbNodes;
    return GT_OK;
}

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
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("appIpfixFwDbPtr is not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    cpssOsMemCpy(dbgCountersPtr, &appIpfixFwDbPtr->dbg.dbgCounters,
                 sizeof(PRV_APP_IPFIX_FW_DBG_COUNTERS_STC));
    return GT_OK;
}

/**
 * @internal appDemoIpfixFwFlowInfoGet function
 *
 * @brief Get IPFIX flow information for a given flowId
 *
 * @param[in] devNum       - device number
 * @parampin] flowId       - flowId
 * @param[out] flowInfoPtr  - flow info pointer
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwFlowInfoGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 flowId,
    OUT PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC *flowInfoPtr
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Application DB not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    PRV_APP_IPFIX_FW_FLOW_ID_CHECK_MAC(devNum, flowId);

    cpssOsMemCpy(flowInfoPtr, &appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum,flowId)],
                 sizeof(*flowInfoPtr));

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwNextActiveFlowGet function
 *
 * @brief Get IPFIX flow information for a given flowId
 *
 * @param[in] devNum       - device number
 * @parampin] flowId        - flowId
 * @param[out] flowInfoPtr  - flow info pointer
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwNextActiveFlowGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 flowId,
    OUT PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC *flowInfoPtr
)
{
    GT_U32 i = 0;
    GT_BOOL isFound = GT_FALSE;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Application DB not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    PRV_APP_IPFIX_FW_FLOW_ID_CHECK_MAC(devNum, flowId);

    for (i = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId); i < PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum); i++)
    {
        if (appIpfixFwDbPtr->flowsDb[i].info.isActive == GT_TRUE)
        {
            cpssOsMemCpy(flowInfoPtr, &appIpfixFwDbPtr->flowsDb[i],
                         sizeof(*flowInfoPtr));
            isFound = GT_TRUE;
            break;
        }
    }

    if (isFound == GT_FALSE)
    {
        return GT_NOT_FOUND;
    }

    return GT_OK;
}

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
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("IPFIX Manager not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    PRV_APP_IPFIX_FW_FLOW_ID_CHECK_MAC(devNum, flowId);

    cpssOsMemCpy(&appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)], flowInfoPtr,
                 sizeof(PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC));

    return GT_OK;
}

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
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("IPFIX Manager not initialized\n");
        return GT_NOT_INITIALIZED;
    }
    appIpfixFwDbPtr->dbg.cpuRxDumpEnable = enable;

    /* allow to 'see' the packets that we get (with the DSA) */
    dxChNetIfRxPacketParse_DebugDumpEnable(enable);


    return GT_OK;
}

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
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("IPFIX Manager not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    appIpfixFwDbPtr->dbg.debugPrintsEnable = enable;
    return GT_OK;
}

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
)
{
    GT_U32      i = 0;
    GT_U32      k = 0;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC *flowPtr;
    PRV_APP_IPFIX_FW_DATA_RECORD_STC *dataRecordPtr;
    GT_U32      usedKeyLength = 14;
    GT_BOOL     printHeading = GT_TRUE;
    GT_U32      flowCount = 0;
    GT_U32      flowMax = PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum);
    GT_CHAR     *flowTypeStringArr[] = {"ALL ACTIVE FLOWS", "LASTLY ACTIVE FLOWS",
                                        "ELEPHANT FLOWS", "LASTLY ACTIVE ELEPHANT FLOWS"};

    if ((appIpfixFwDbPtr != NULL) &&
        (appIpfixFwDbPtr->dbg.debugPrintsEnable == GT_TRUE))
    {
        cpssOsPrintf("Flow Statistics: %s\n", flowTypeStringArr[flowType]);

        for (i = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, firstFlowId); i < flowMax; i++)
        {
            flowPtr = &(appIpfixFwDbPtr->flowsDb[i]);

            if (flowType == CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E)
            {
                if (flowPtr->info.isActive == GT_FALSE)
                {
                    continue;
                }
            }
            else if (flowType == CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ELEPHANTS_E)
            {
                if ((flowPtr->info.isActive == GT_FALSE) || (flowPtr->info.isElephant == GT_FALSE))
                {
                    continue;
                }
            }
            else
            {
                /* [TBD] Last active and last elephant flows */
            }

            dataRecordPtr = &flowPtr->dataRecord;

            if (printHeading == GT_TRUE)
            {
                cpssOsPrintf("FlowId, Type,           %d byte key, firstTs, lastTs, pktCnt(MSB,LSB), dropCnt(MSB,LSB), byteCnt(MSB,LSB)\n", usedKeyLength);
                printHeading = GT_FALSE;
            }
            cpssOsPrintf("%4X ", dataRecordPtr->flowId);
            cpssOsPrintf("%2d ", flowPtr->pktType);
            for (k = 0; k < usedKeyLength; k++)
            {
                cpssOsPrintf("%02X", flowPtr->key.pattern[k]);
            }

            cpssOsPrintf("  %4X    ", dataRecordPtr->firstTs);
            cpssOsPrintf("%4X   ", dataRecordPtr->lastTs);
            cpssOsPrintf("(%5d,%6d) ", dataRecordPtr->packetCount[1], dataRecordPtr->packetCount[0]);
            cpssOsPrintf("(%5d,%6d) ", dataRecordPtr->dropCount[1], dataRecordPtr->dropCount[0]);
            cpssOsPrintf("(%5d,%8d) ", dataRecordPtr->byteCount[1], dataRecordPtr->byteCount[0]);
            cpssOsPrintf("\n");

            flowCount++;
            if (flowCount >= numFlows)
            {
                break;
            }
        }

        cpssOsPrintf("\n");
    }
    else if (appIpfixFwDbPtr != NULL)
    {
        cpssOsPrintf("Error: appIpfixFwDb no created\n");
    }
    return GT_OK;
}

GT_VOID  appDemoIpfixFwDataRecordDump
(
    PRV_APP_IPFIX_FW_DATA_PKT_STC   *dataPktPtr
)
{
    GT_U32 i;

    if (dataPktPtr == NULL)
    {
        cpssOsPrintf("dataPktPtr cannot be NULL\n");
        return;
    }

    cpssOsPrintf("Header:\n");
    cpssOsPrintf("    PacketType      : %X \n", dataPktPtr->header.packetType);
    cpssOsPrintf("    Sequence number : %X \n", dataPktPtr->header.seqNum);
    cpssOsPrintf("    No. Exported    : %X \n", dataPktPtr->header.numExported);
    cpssOsPrintf("    First FlowId    : %X \n", dataPktPtr->header.firstFlowId);
    cpssOsPrintf("    Last  FlowId    : %X \n", dataPktPtr->header.lastFlowId);
    cpssOsPrintf("    Time of the Day : %06X %08X \n", dataPktPtr->header.tod[1],
                                               dataPktPtr->header.tod[0]);

    for (i=0; i<dataPktPtr->header.numExported; i++)
    {
        cpssOsPrintf("Record Number: %d\n", i);
        cpssOsPrintf("    FlowId          : %04X\n", dataPktPtr->dataRecord[i].flowId);
        cpssOsPrintf("    First Time Stamp: %05X\n", dataPktPtr->dataRecord[i].firstTs);
        cpssOsPrintf("    Last Time Stamp : %05X\n", dataPktPtr->dataRecord[i].lastTs);
        cpssOsPrintf("    Packet Count    : %X %X\n", dataPktPtr->dataRecord[i].packetCount[1],
                                                    dataPktPtr->dataRecord[i].packetCount[0]);
        cpssOsPrintf("    Drop Count      : %X %X\n", dataPktPtr->dataRecord[i].dropCount[1],
                                                    dataPktPtr->dataRecord[i].dropCount[0]);
        cpssOsPrintf("    Byte Count      : %X %X\n", dataPktPtr->dataRecord[i].byteCount[1],
                                                    dataPktPtr->dataRecord[i].byteCount[0]);
        cpssOsPrintf("    Entry Status    : %X\n", dataPktPtr->dataRecord[i].entryStatus);
    }
}
#if 0
GT_STATUS appDemoIpfixFwAgingEnable
(
    IN GT_BOOL     enable
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("IPFIX Manager not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    appIpfixFwDbPtr->agingEnable = enable;

    return GT_OK;
}
#endif

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
)
{
    PRV_APP_IPFIX_FW_DATA_RECORD_STC   *dbDataRecordPtr;

    if (appIpfixFwDbPtr == NULL)
    {
        return GT_NOT_INITIALIZED;
    }

    PRV_APP_IPFIX_FW_FLOW_ID_CHECK_MAC(devNum, flowId);

    dbDataRecordPtr = &appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].dataRecord;
    cpssOsMemSet(dbDataRecordPtr, 0, sizeof(PRV_APP_IPFIX_FW_DATA_RECORD_STC));
    dbDataRecordPtr->flowId = PRV_APP_IPFIX_FW_DB_ID_2_FLOW_ID_CONVERT(devNum, flowId);

    return GT_OK;
}

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
)
{
    GT_U32          flowDbId;
    PRV_APP_IPFIX_FW_DATA_RECORD_STC   *dbDataRecordPtr;

    if (appIpfixFwDbPtr == NULL)
    {
        return GT_NOT_INITIALIZED;
    }

    for (flowDbId = 0; flowDbId < PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum); flowDbId++)
    {
        if (appIpfixFwDbPtr->flowsDb[flowDbId].info.isActive == GT_TRUE)
        {
            dbDataRecordPtr = &appIpfixFwDbPtr->flowsDb[flowDbId].dataRecord;
            cpssOsMemSet(dbDataRecordPtr, 0, sizeof(PRV_APP_IPFIX_FW_DATA_RECORD_STC));
            dbDataRecordPtr->flowId = PRV_APP_IPFIX_FW_DB_ID_2_FLOW_ID_CONVERT(devNum, flowDbId);;
        }
    }

    return GT_OK;
}

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
)
{
    appIpfixFwDbPtr->mgrDb.keysLookupAlgo = keysLookupAlgo;
    return GT_OK;
}

GT_STATUS appDemoIpfixFwDbgDisableFlagsSet
(
    GT_U32      disableBitMap
)
{
    appIpfixFwDbPtr->mgrDb.tmpDbgBmp = disableBitMap;
    return GT_OK;
}

GT_STATUS appDemoIpfixFwDbgPrintKey
(
    GT_U32 size,
    GT_U8 *keyPtr
)
{
    GT_U32 i;
    cpssOsPrintf("[DBG] KeyMask: ");
    for(i=0; i<size; i++)
    {
        cpssOsPrintf("%02X ", keyPtr[i]);
    }
    cpssOsPrintf("\n");
    return GT_OK;
}