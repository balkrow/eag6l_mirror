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
* @file gtAppIpfixFw.c
*
* @brief Application Code for IPFIX Manager
*
* @version   1
*********************************************************************************
**/

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManagerSamples.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfix.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <ipFix/prvAppIpfixFw.h>
#include <ipFix/prvAppIpfixFwDbg.h>

PRV_APP_IPFIX_FW_DB_STC *appIpfixFwDbPtr;

extern void dxChNetIfRxPacketParse_DebugDumpEnable
(
    IN GT_BOOL  enableDumpRxPacket
);

/**
 * @internal prvAppDemoIpfixFwTxInitSdmaQ function
 *
 * @brief Initialize SDMA queue
 *
 * @param[in] devNum    - device number
 * @param[in] cpuCode   - cpu code
 * @param[in] queueNum  - queue number
 *
 * @return GT_OK - on success
 */
static GT_STATUS prvAppDemoIpfixFwTxInitSdmaQ
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN  GT_U32                   queueNum
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC cpuCodeEntry;
    CPSS_UNI_EV_CAUSE_ENT                  evCause;

    cpssOsBzero((GT_CHAR*)&cpuCodeEntry, sizeof(cpuCodeEntry));

    evCause = CPSS_PP_RX_BUFFER_QUEUE0_E + (queueNum & 0x7);
    rc = cpssEventDeviceMaskSet(devNum, evCause, CPSS_EVENT_MASK_E);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssEventDeviceMaskSet failed: rc=%d\n", rc);
        return rc;
    }

    evCause = CPSS_PP_RX_ERR_QUEUE0_E + (queueNum & 0x7);
    rc = cpssEventDeviceMaskSet(devNum, evCause, CPSS_EVENT_MASK_E);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssEventDeviceMaskSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Trancated packets to CPU */
    rc = cpssDxChNetIfCpuCodeTableGet(devNum, cpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChNetIfCpuCodeTableGet failed: rc=%d\n", rc);
        return rc;
    }

    cpuCodeEntry.tc = queueNum & 0x7;
    rc = cpssDxChNetIfCpuCodeTableSet(devNum, cpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChNetIfCpuCodeTableSet failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixFwTxRestoreSdmaQ function
 *
 * @brief Restore SDMA queue
 *
 * @param[in] devNum    - device number
 * @param[in] cpuCode   - cpu code
 * @param[in] queueNum  - queue number
 *
 * @return GT_OK - on success
 */
static GT_STATUS prvAppDemoIpfixFwTxRestoreSdmaQ
(
    IN  GT_U8                    devNum,
    IN  CPSS_NET_RX_CPU_CODE_ENT cpuCode,
    IN  GT_U32                   queueNum
)
{
    GT_STATUS                              rc = GT_OK;
    CPSS_DXCH_NET_CPU_CODE_TABLE_ENTRY_STC cpuCodeEntry;
    CPSS_UNI_EV_CAUSE_ENT                  evCause;

    cpssOsBzero((GT_CHAR*)&cpuCodeEntry, sizeof(cpuCodeEntry));

    evCause = CPSS_PP_RX_BUFFER_QUEUE0_E + (queueNum & 0x7);
    rc = cpssEventDeviceMaskSet(devNum, evCause, CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssEventDeviceMaskSet failed: rc=%d\n", rc);
        return rc;
    }

    evCause = CPSS_PP_RX_ERR_QUEUE0_E + (queueNum & 0x7);
    rc = cpssEventDeviceMaskSet(devNum, evCause, CPSS_EVENT_UNMASK_E);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssEventDeviceMaskSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Trancated packets to CPU */
    rc = cpssDxChNetIfCpuCodeTableGet(devNum, cpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChNetIfCpuCodeTableGet failed: rc=%d\n", rc);
        return rc;
    }

    cpuCodeEntry.tc = 0;
    rc = cpssDxChNetIfCpuCodeTableSet(devNum, cpuCode, &cpuCodeEntry);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChNetIfCpuCodeTableSet failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwTxInit function
 * @endinternal
 *
 * @brief  Initialize/Configure TX queue configuration
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - (pointer to) init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwTxInit
(
    IN      GT_U8                               devNum,
    IN      PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC *paramsPtr
)
{
    GT_STATUS                              rc = GT_OK;

    if (paramsPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: NULL Pointer Received\n");
        return GT_BAD_PTR;
    }

    /* Init Data packets queue */
    rc = prvAppDemoIpfixFwTxInitSdmaQ(devNum, paramsPtr->dataPktsCpuCode, paramsPtr->dataPktsQueueNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwTxInitSdmaQ failed: rc=%d\n", rc);
        return rc;
    }

    /* Init New flows packet queue */
    rc = prvAppDemoIpfixFwTxInitSdmaQ(devNum, paramsPtr->newFlowsCpuCode, paramsPtr->newFlowsQueueNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwTxInitSdmaQ failed: rc=%d\n", rc);
        return rc;
    }

    /* Init syn packets queue */
    rc = prvAppDemoIpfixFwTxInitSdmaQ(devNum, paramsPtr->synPktsCpuCode, paramsPtr->synPktsQueueNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwTxInitSdmaQ failed: rc=%d\n", rc);
        return rc;
    }

    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.txInitParams, paramsPtr, sizeof(*paramsPtr));
    return GT_OK;
}

/**
 * @internal appDemoIpfixFwDefaultTxInit function
 * @endinternal
 *
 * @brief  Initialize/Configure TX queue configuration
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - (pointer to) init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwDefaultTxInit
(
    IN      GT_U8                               devNum
)
{
    GT_STATUS                              rc = GT_OK;
    PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC    txInitParams;

    __IPFIX_FW_NULL_PTR_CHECK(appIpfixFwDbPtr);

    cpssOsMemSet(&txInitParams, 0, sizeof(txInitParams));

    /* TX Init configuration */
    txInitParams.dataPktsCpuCode  = PRV_APP_IPFIX_FW_DATA_PKTS_CPU_CODE;
    txInitParams.dataPktsQueueNum = PRV_APP_IPFIX_FW_DATA_PKTS_QUEUE_NUM_CNS;
    txInitParams.newFlowsCpuCode  = PRV_APP_IPFIX_FW_FIRST_PKTS_CPU_CODE;
    txInitParams.newFlowsQueueNum = PRV_APP_IPFIX_FW_FIRST_PKTS_QUEUE_NUM_CNS;
    txInitParams.synPktsCpuCode   = PRV_APP_IPFIX_FW_SYN_PKTS_CPU_CODE;
    txInitParams.synPktsQueueNum  = PRV_APP_IPFIX_FW_SYN_PKTS_QUEUE_NUM_CNS;
    rc = appDemoIpfixFwTxInit(devNum, &txInitParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwTxInit failed: rc=%d\n", rc);
        return rc;
    }

    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.txInitParams, &txInitParams, sizeof(txInitParams));

    return GT_OK;
}

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
    IN      GT_U8                               devNum
)
{
    GT_STATUS                              rc = GT_OK;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    /* Restore Data packets queue */
    rc = prvAppDemoIpfixFwTxRestoreSdmaQ(devNum,
                                         appIpfixFwDbPtr->initCfg.txInitParams.dataPktsCpuCode,
                                         appIpfixFwDbPtr->initCfg.txInitParams.dataPktsQueueNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwTxRestoreSdmaQ failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore New flows packet queue */
    rc = prvAppDemoIpfixFwTxRestoreSdmaQ(devNum,
                                         appIpfixFwDbPtr->initCfg.txInitParams.newFlowsCpuCode,
                                         appIpfixFwDbPtr->initCfg.txInitParams.newFlowsQueueNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwTxRestoreSdmaQ failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore syn packets queue */
    rc = prvAppDemoIpfixFwTxRestoreSdmaQ(devNum,
                                         appIpfixFwDbPtr->initCfg.txInitParams.synPktsCpuCode,
                                         appIpfixFwDbPtr->initCfg.txInitParams.synPktsQueueNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwTxRestoreSdmaQ failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwPolicerInit function
 * @endinternal
 *
 * @brief  Initialize policer engine for IPFIX manager
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - (pointer to) init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPolicerInit
(
    IN  GT_U8                            devNum,
    IN  PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC *paramsPtr
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_POLICER_STAGE_TYPE_ENT        policerStage;
    GT_U32                                  ipfixEntryIndex = 0;
    CPSS_DXCH_IPFIX_ENTRY_STC               ipfixEntry;

    if (paramsPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: NULL Pointer Received\n");
        return GT_BAD_PTR;
    }

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    policerStage = paramsPtr->policerStage;
    if (policerStage > CPSS_DXCH_POLICER_STAGE_EGRESS_E)
    {
        __IPFIX_FW_LOG("policer stage invalid\n");
        return GT_BAD_PARAM;
    }

    /* Set metering mode and counting mode */
    rc = cpssDxChPolicerStageMeterModeSet( devNum, policerStage,
                    CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxChPolicerStageMeterModeSet failed: rc=%d\n", rc);
        return rc;
    }
    rc = cpssDxChPolicerCountingModeSet( devNum, policerStage,
                    CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxChPolicerCountingModeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Get & Set packet size mode */
    rc = cpssDxCh3PolicerPacketSizeModeGet( devNum, policerStage,
                    &appIpfixFwDbPtr->restoreCfg.policerPktSizeMode);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxCh3PolicerPacketSizeModeGet failed: rc=%d\n", rc);
        return rc;
    }
    rc = cpssDxCh3PolicerPacketSizeModeSet( devNum, policerStage,
                    CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxCh3PolicerPacketSizeModeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Get and Set Memory Size*/
    rc = cpssDxChPolicerMemorySizeGet(devNum, &appIpfixFwDbPtr->restoreCfg.policerMemoryCfg);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPolicerMemorySizeGet failed: rc=%d\n", rc);
        return rc;
    }
    rc = cpssDxChPolicerMemorySizeSet(devNum,&paramsPtr->memoryCfg);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPolicerMemorySizeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Set Drop count mode */
    rc = cpssDxChIpfixDropCountModeSet(devNum, policerStage,
                                       CPSS_DXCH_IPFIX_DROP_COUNT_MODE_ALL_E);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixDropCountModeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Get and Set wrap around config */
    rc = cpssDxChIpfixWraparoundConfGet(devNum, policerStage,
                                        &appIpfixFwDbPtr->restoreCfg.policerWaConfig);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxChIpfixWraparoundConfGet failed: rc=%d\n", rc);
        return rc;
    }
    rc = cpssDxChIpfixWraparoundConfSet(devNum, policerStage, &paramsPtr->waConfig);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxChIpfixWraparoundConfSet failed: rc=%d\n", rc);
        return rc;
    }

    /* WBC flush to ensure the mode is set to billing, even in scaled environment */
    rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum, policerStage);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPolicerCountingWriteBackCacheFlush failed: rc=%d\n", rc);
        return rc;
    }

    /* Set timestamp in To CPU dsaTag */
    rc = cpssDxChIpfixTimestampToCpuEnableSet(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixTimestampToCpuEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Get and Set base flow Id */
        rc = cpssDxChIpfixBaseFlowIdGet(devNum, policerStage,
                                        &appIpfixFwDbPtr->restoreCfg.policerBaseFlowId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixBaseFlowIdGet failed: rc=%d\n", rc);
            return rc;
        }
        rc = cpssDxChIpfixBaseFlowIdSet(devNum, policerStage, paramsPtr->baseFlowId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixBaseFlowIdSet failed: rc=%d\n", rc);
            return rc;
        }
        /* Get and Set first packet config */
        rc = cpssDxChIpfixFirstPacketConfigGet(devNum, policerStage,
                                               &appIpfixFwDbPtr->restoreCfg.policerFirstPacketCfgSet);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFirstPacketConfigGet failed: rc=%d\n", rc);
        }
        rc = cpssDxChIpfixFirstPacketConfigSet(devNum, policerStage, &paramsPtr->firstPacketCfgSet);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFirstPacketConfigSet failed: rc=%d\n", rc);
        }
    }

    /* configure ipfix entries */
    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    ipfixEntry.lastCpuOrDropCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ipfixEntry.lastPacketCommand            = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    for (ipfixEntryIndex = paramsPtr->baseFlowId; ipfixEntryIndex < paramsPtr->maxPolicerEntries; ipfixEntryIndex++)
    {
#ifndef ASIC_SIMULATION
        if ((ipfixEntryIndex - paramsPtr->baseFlowId) < PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_CNS(devNum))
        {
            ipfixEntry.numberOfFirstPacketsToMirror = 0;
        }
        else
#endif
        {
            ipfixEntry.numberOfFirstPacketsToMirror = 1;
        }
        if (ipfixEntryIndex >= PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_CNS(devNum))
        {
            ipfixEntry.firstTimestampValid  = GT_TRUE;
        }

        rc = cpssDxChIpfixEntrySet(devNum,
                                   paramsPtr->policerStage,
                                   ipfixEntryIndex,
                                   &ipfixEntry);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixEntrySet failed: rc=%d\n", rc);
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            rc = cpssDxChIpfixFirstPacketsMirrorEnableSet(devNum, policerStage, ipfixEntryIndex, GT_TRUE);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChIpfixFirstPacketsMirrorEnableSet failed: rc=%d\n", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

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
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              ipfixEntryIndex = 0;
    CPSS_DXCH_IPFIX_ENTRY_STC           ipfixEntry;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    /* restore ipfix entries */
    cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
    ipfixEntry.lastCpuOrDropCode            = CPSS_NET_FIRST_USER_DEFINED_E;
    ipfixEntry.lastPacketCommand            = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    for (ipfixEntryIndex = appIpfixFwDbPtr->initCfg.policerInitParams.baseFlowId;
          ipfixEntryIndex < appIpfixFwDbPtr->initCfg.policerInitParams.maxPolicerEntries;
          ipfixEntryIndex++)
    {
        rc = cpssDxChIpfixEntrySet(devNum,
                                   appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                                   ipfixEntryIndex,
                                   &ipfixEntry);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixEntrySet failed: rc=%d\n", rc);
            return rc;
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            rc = cpssDxChIpfixFirstPacketsMirrorEnableSet(devNum,
                                                          appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                                                          ipfixEntryIndex, GT_FALSE);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChIpfixFirstPacketsMirrorEnableSet failed: rc=%d\n", rc);
                return rc;
            }
        }
    }

    /* Restore metering mode and counting mode */
    rc = cpssDxChPolicerStageMeterModeSet( devNum, appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                    CPSS_DXCH_POLICER_STAGE_METER_MODE_PORT_E);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxChPolicerStageMeterModeSet failed: rc=%d\n", rc);
        return rc;
    }
    rc = cpssDxChPolicerCountingModeSet( devNum, appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                    CPSS_DXCH_POLICER_COUNTING_DISABLE_E);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPolicerCountingModeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore packet size mode */
    rc = cpssDxCh3PolicerPacketSizeModeSet( devNum, appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                    appIpfixFwDbPtr->restoreCfg.policerPktSizeMode);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxCh3PolicerPacketSizeModeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore memory config */
    rc = cpssDxChPolicerMemorySizeSet(devNum, &appIpfixFwDbPtr->restoreCfg.policerMemoryCfg);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPolicerMemorySizeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore Drop count mode */
    rc = cpssDxChIpfixDropCountModeSet(devNum,
                                       appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                                       CPSS_DXCH_IPFIX_DROP_COUNT_MODE_METER_ONLY_E);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixDropCountModeSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore wrap around config */
    rc = cpssDxChIpfixWraparoundConfSet(devNum,
                                        appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                                        &appIpfixFwDbPtr->restoreCfg.policerWaConfig);
    if(rc != GT_OK )
    {
        __IPFIX_FW_LOG("cpssDxChIpfixWraparoundConfSet failed: rc=%d\n", rc);
        return rc;
    }

    /* WBC flush to ensure the mode is set to billing, even in scaled environment */
    rc = cpssDxChPolicerCountingWriteBackCacheFlush(devNum,
                                                    appIpfixFwDbPtr->initCfg.policerInitParams.policerStage);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPolicerCountingWriteBackCacheFlush failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore timestamp in To CPU dsaTag */
    rc = cpssDxChIpfixTimestampToCpuEnableSet(devNum, GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixTimestampToCpuEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Restore base flow Id */
        rc = cpssDxChIpfixBaseFlowIdSet(devNum,
                                        appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                                        appIpfixFwDbPtr->restoreCfg.policerBaseFlowId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixBaseFlowIdSet failed: rc=%d\n", rc);
            return rc;
        }

        /* Restore first packet config */
        rc = cpssDxChIpfixFirstPacketConfigGet(devNum,
                                               appIpfixFwDbPtr->initCfg.policerInitParams.policerStage,
                                               &appIpfixFwDbPtr->restoreCfg.policerFirstPacketCfgSet);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFirstPacketConfigGet failed: rc=%d\n", rc);
        }
    }
    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixFwIpv4UdpFlowConfig function
 * @endinternal
 *
 * @brief  Configure UDB Key for IPv4 UDP key
 *
 * @param[in] devNum - device number
 * @param[in] pclDirection - enumerator for ingress/egress pcl direction
 * @param[in] lookupnum    - lookup number
 *
 * @retval GT_OK - on success
 */
/* Function to be used to enable all key fields for Ipv4-UDP pkt type */
static GT_STATUS prvAppDemoIpfixFwIpv4FlowConfig
(
    IN  GT_U8                       devNum,
    IN  PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC *paramsPtr
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              udbIndex = 0;
    GT_U8                               offset = 0; /* UDB offset from anchor */
    CPSS_DXCH_PCL_UDB_SELECT_STC        udbSelectStruct;
    CPSS_PCL_DIRECTION_ENT              pclDirection;
    CPSS_PCL_LOOKUP_NUMBER_ENT          lookupNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  ruleFormatType;
    GT_U32                              itr = 0;

    pclDirection   = paramsPtr->direction;
    lookupNum      = paramsPtr->lookupNum;
    ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;

    for (itr=0; itr<2; itr++) {

        packetType = (itr==0) ? CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E : CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;

        cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

        /* UDB[0-3] offset for SIPv4 */
        for(udbIndex = 0, offset = 14 ; udbIndex < 4; udbIndex++,offset++)
        {
            rc = cpssDxChPclUserDefinedByteSet(devNum,
                                               ruleFormatType,
                                               packetType,
                                               pclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
                return rc;
            }
        }

        /* UDB[5-8] offset for DIPv4 */
        for(udbIndex = 5, offset = 18 ; udbIndex < 9; udbIndex++,offset++)
        {
            rc = cpssDxChPclUserDefinedByteSet(devNum,
                                               ruleFormatType,
                                               packetType,
                                               pclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
                return rc;
            }
        }

        /* UDB[9] TCP/UDP Protocol */
        udbIndex = 9;
        offset = 11;
        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                           offset);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }

        /* UDB[10-13] offset for UDP Src Port and Dst Port */
        for(udbIndex = 10, offset = 0 ; udbIndex < 14; udbIndex++,offset++)
        {
            rc = cpssDxChPclUserDefinedByteSet(devNum,
                                               ruleFormatType,
                                               packetType,
                                               pclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L4_E,
                                               offset);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
                return rc;
            }
        }

        /* UDB[40] offset for Applicable Flow Sub-template */
        udbIndex = 40;
        offset = 91;

        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                           offset /* Applicable Flow Sub-template */);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }

        /* it's a must to enable UDB's */
        udbSelectStruct.udbSelectArr[0]      = 0; /* SIPv4 */
        udbSelectStruct.udbSelectArr[1]      = 1; /* SIPv4 */
        udbSelectStruct.udbSelectArr[2]      = 2; /* SIPv4 */
        udbSelectStruct.udbSelectArr[3]      = 3; /* SIPv4 */
        if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow !=
            CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
        {
            udbSelectStruct.ingrUdbReplaceArr[4] = GT_TRUE; /* Source ePort */
        }
        udbSelectStruct.udbSelectArr[5]      = 5; /* DIPv4 */
        udbSelectStruct.udbSelectArr[6]      = 6; /* DIPv4 */
        udbSelectStruct.udbSelectArr[7]      = 7; /* DIPv4 */
        udbSelectStruct.udbSelectArr[8]      = 8; /* DIPv4 */
        udbSelectStruct.udbSelectArr[9]      = 9; /* Protocol TCP/UDP */
        udbSelectStruct.udbSelectArr[10]     = 10; /* UDP Src Port */
        udbSelectStruct.udbSelectArr[11]     = 11; /* UDP Src Port */
        udbSelectStruct.udbSelectArr[12]     = 12; /* UDP Dst Port */
        udbSelectStruct.udbSelectArr[13]     = 13; /* UDP Dst Port */
        udbSelectStruct.udbSelectArr[14]     = 40; /* Applicable Flow Sub-template */

        rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                                  ruleFormatType,
                                                  packetType,
                                                  lookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedBytesSelectSet failed: rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixFwIpv6FlowConfig function
 * @endinternal
 *
 * @brief  Configure TCP/UDB Key for IPv6 TCP/UDP key
 *
 * @param[in] devNum - device number
 * @param[in] pclDirection - enumerator for ingress/egress pcl direction
 * @param[in] lookupnum    - lookup number
 *
 * @retval GT_OK - on success
 */
/* Function to be used to enable all key fields for Ipv4-UDP pkt type */
static GT_STATUS prvAppDemoIpfixFwIpv6FlowConfig
(
    IN  GT_U8                       devNum,
    IN  PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC *paramsPtr
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              udbIndex = 0;
    GT_U8                               offset = 0; /* UDB offset from anchor */
    CPSS_DXCH_PCL_UDB_SELECT_STC        udbSelectStruct;
    CPSS_PCL_DIRECTION_ENT              pclDirection;
    CPSS_PCL_LOOKUP_NUMBER_ENT          lookupNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT       packetType;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  ruleFormatType;
    GT_U32                              i;
    GT_U32                              itr = 0;

    pclDirection   = paramsPtr->direction;
    lookupNum      = paramsPtr->lookupNum;
    ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E*/;

    for (itr=0; itr<2; itr++) {

        packetType = (itr==0) ? CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E : CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E;

        cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

        udbIndex = 0;
        offset = 10;

        /* SIP and DIP IPv6 */
        for(i = 0 ; i < 32; i++)
        {
            rc = cpssDxChPclUserDefinedByteSet(devNum,
                                               ruleFormatType,
                                               packetType,
                                               pclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                               offset);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
                return rc;
            }
            udbIndex++;
            offset++;
        }

        /* UDB[32] TCP/UDP Protocol */
        offset = 8;
        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E,
                                           offset);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }
        udbIndex++;

        offset = 0;
        /* UDB[33-36] offset for UDP Src Port and Dst Port */
        for(i = 0 ; i < 4; i++)
        {
            rc = cpssDxChPclUserDefinedByteSet(devNum,
                                               ruleFormatType,
                                               packetType,
                                               pclDirection,
                                               udbIndex,
                                               CPSS_DXCH_PCL_OFFSET_L4_E,
                                               offset);
            if (GT_OK != rc)
            {
                __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
                return rc;
            }
            udbIndex++;
            offset++;
        }

        /* UDB[40] offset for Applicable Flow Sub-template */
        udbIndex = 40;
        offset = 91;

        rc = cpssDxChPclUserDefinedByteSet(devNum,
                                           ruleFormatType,
                                           packetType,
                                           pclDirection,
                                           udbIndex,
                                           CPSS_DXCH_PCL_OFFSET_METADATA_E,
                                           offset /* Applicable Flow Sub-template */);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteSet failed: rc=%d\n", rc);
            return rc;
        }

        /* it's a must to enable UDB's */
        udbSelectStruct.udbSelectArr[0]      = 33; /* UDP Src Port */
        udbSelectStruct.udbSelectArr[1]      = 34; /* UDP Src Port */
        udbSelectStruct.udbSelectArr[2]      = 35; /* UDP Dst Port */
        udbSelectStruct.udbSelectArr[3]      = 36; /* UDP Dst Port */
        if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow !=
            CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
        {
            udbSelectStruct.ingrUdbReplaceArr[4] = GT_TRUE; /* Source ePort */
        }
        udbSelectStruct.udbSelectArr[5]      = 32;/* Protocol TCP/UDP */
        /*udbSelectStruct.udbSelectArr[6]      = 40;*/ /* Applicable Flow Sub-template */ /* DIPv4 */

         /* IPv6  SIP, DIP  */
        for (i=0; i<32; i++) {
            udbSelectStruct.udbSelectArr[6+i] = i;
        }

        rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                                  ruleFormatType,
                                                  packetType,
                                                  lookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedBytesSelectSet failed: rc=%d\n", rc);
            return rc;
        }
    }
    return GT_OK;
}

/**
 * @internal appDemoIpfixFwPclInit function
 * @endinternal
 *
 * @brief  Initialize PCL and Configure UDB configuration
 *          for PCL Keys
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - (pointer to) init params
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPclInit
(
    IN  GT_U8           devNum,
    IN  PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC  *paramsPtr
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          subLookupNum = 0;
    GT_U32                          portTxqNum = 0;
    GT_PORT_NUM                     portNum = 0;
    CPSS_INTERFACE_INFO_STC         interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC    lookupCfg;
    GT_U32                          pclId;
    GT_U32                          maxPorts;
    CPSS_PCL_DIRECTION_ENT          direction;
    CPSS_PCL_LOOKUP_NUMBER_ENT      lookupNum;

    cpssOsMemSet(&interfaceInfo, 0, sizeof(interfaceInfo));
    cpssOsMemSet(&lookupCfg, 0, sizeof(lookupCfg));

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Application DB not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    if (paramsPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: NULL Pointer Received\n");
        return GT_BAD_PTR;
    }

    direction = paramsPtr->direction;
    lookupNum = paramsPtr->lookupNum;

    /* PCL init */
    rc = cpssDxChPclInit(devNum);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChPclInit failed, rc = %d\n", rc);
        return rc;
    }

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        /* Enable Ingress Pcl */
        rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPclIngressPolicyEnable failed, rc = %d\n", rc);
            return rc;
        }
    }
    else if (direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* Enable Egress Pcl */
        rc = cpssDxCh2PclEgressPolicyEnable(devNum, GT_TRUE);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxCh2PclEgressPolicyEnable failed, rc = %d\n", rc);
            return rc;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    maxPorts = PRV_CPSS_PP_MAC(devNum)->numOfPorts;

    subLookupNum = 0;
    for (portNum = 0; portNum < maxPorts; portNum++)
    {
        rc = prvCpssDxChPortPhysicalPortMapCheckAndConvert(devNum, portNum,
                                       PRV_CPSS_DXCH_PORT_TYPE_TXQ_E, &portTxqNum);
        if( rc != GT_OK)
        {
            continue;
        }

        if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_TRUE);
            if(rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChPclPortIngressPolicyEnable failed, rc = %d\n", rc);
                return rc;
            }
        }

        rc = cpssDxChPclPortLookupCfgTabAccessModeSet(
                        devNum, portNum, direction,
                        lookupNum, subLookupNum,
                        CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
        if(rc != GT_OK )
        {
            __IPFIX_FW_LOG("cpssDxChPclPortLookupCfgTabAccessModeSet failed, rc = %d\n", rc);
            return rc;
        }

        /* Get and Set lookup config */
        interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.hwDevNum   = devNum;
        interfaceInfo.devPort.portNum    = portNum;
        rc = cpssDxChPclCfgTblGet(devNum, &interfaceInfo, paramsPtr->direction,
                                  paramsPtr->lookupNum, &appIpfixFwDbPtr->restoreCfg.pclLookupCfg);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPclCfgTblGet failed, rc = %d\n", rc);
            return rc;
        }
        pclId = PRV_APP_IPFIX_FW_PCL_DEFAULT_ID_MAC(paramsPtr->direction,
                                                    paramsPtr->lookupNum,
                                                    portNum);
        lookupCfg.enableLookup           = GT_TRUE;
        lookupCfg.pclId                  = pclId;
        lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        lookupCfg.groupKeyTypes.ipv4Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
        lookupCfg.groupKeyTypes.ipv6Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E/*CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_40_E*/;
        lookupCfg.udbKeyBitmapEnable     = GT_FALSE;
        lookupCfg.tcamSegmentMode        = CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_1_TCAM_E;
        rc = cpssDxChPclCfgTblSet(devNum, &interfaceInfo, paramsPtr->direction,
                                  paramsPtr->lookupNum, &lookupCfg);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPclCfgTblSet failed, rc = %d\n", rc);
            return rc;
        }
    }

    /* Enable IPv4 TCP/UDP Flows */
    rc = prvAppDemoIpfixFwIpv4FlowConfig(devNum, paramsPtr);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwIpv4UdpFlowConfig failed: rc=%d\n", rc);
        return rc;
    }

    /* Enable IPv6 TCP/UDP Flows */
    rc = prvAppDemoIpfixFwIpv6FlowConfig(devNum, paramsPtr);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("prvAppDemoIpfixFwIpv4UdpFlowConfig failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}

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
)
{
    GT_STATUS                      rc = GT_OK;
    CPSS_DXCH_PCL_UDB_SELECT_STC   udbSelectStruct;
    GT_PORT_NUM                    portNum = 0;
    GT_U32                         maxPorts;
    CPSS_INTERFACE_INFO_STC        interfaceInfo;
    PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC *paramsPtr;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT  packetType[4] = {CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                                    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                                    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E,
                                                    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E};
    GT_U32                          i;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    paramsPtr = &appIpfixFwDbPtr->initCfg.pclInitParams;
    maxPorts = PRV_CPSS_PP_MAC(devNum)->numOfPorts;
    for (portNum = 0; portNum < maxPorts; portNum++)
    {
        if (paramsPtr->direction == CPSS_PCL_DIRECTION_INGRESS_E)
        {
            rc = cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_FALSE);
            if(rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChPclPortIngressPolicyEnable failed, rc = %d\n", rc);
                return rc;
            }
        }

        interfaceInfo.type               = CPSS_INTERFACE_PORT_E;
        interfaceInfo.devPort.hwDevNum   = devNum;
        interfaceInfo.devPort.portNum    = portNum;
        rc = cpssDxChPclCfgTblSet(devNum, &interfaceInfo, paramsPtr->direction,
                                  paramsPtr->lookupNum, &appIpfixFwDbPtr->restoreCfg.pclLookupCfg);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPclCfgTblSet failed, rc = %d\n", rc);
            return rc;
        }
    }

    if (paramsPtr->direction == CPSS_PCL_DIRECTION_INGRESS_E)
    {
        /* Disable Ingress Pcl */
        rc = cpssDxChPclIngressPolicyEnable(devNum, GT_FALSE);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPclIngressPolicyEnable failed, rc = %d\n", rc);
            return rc;
        }
    }
    else if (paramsPtr->direction == CPSS_PCL_DIRECTION_EGRESS_E)
    {
        /* Disable Egress Pcl */
        rc = cpssDxCh2PclEgressPolicyEnable(devNum, GT_FALSE);
        if(rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxCh2PclEgressPolicyEnable failed, rc = %d\n", rc);
            return rc;
        }
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));
    for (i=0; i<4; i++)
    {

        /* Clearing UDB config */
        rc = cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                                  packetType[i],
                                                  paramsPtr->lookupNum,
                                                  &udbSelectStruct);
        if (GT_OK != rc)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedBytesSelectSet failed: rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwEmInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match for IPFIX FW
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - (pointer to) init parameters structure
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmInit
(
    IN GT_U8           devNum,
    IN PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC *paramsPtr
)
{
    GT_STATUS                                   rc=GT_OK;
    PRV_APP_IPFIX_FW_PKT_TYPE_ENT               packetType;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT               pclPacketType;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    /* AUTODOC: set exactMatchActivityEnable */
    rc =  cpssDxChExactMatchActivityBitEnableSet(devNum, paramsPtr->emUnitNum,
                                                 paramsPtr->emLookupNum,
                                                 GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchActivityBitEnableSet FAILED: %d\n", rc);
        return rc;
    }

    /* get and set lookup client type */
    rc = cpssDxChExactMatchClientLookupGet(devNum, paramsPtr->emUnitNum,
                                           paramsPtr->emLookupNum,
                                           &appIpfixFwDbPtr->restoreCfg.emClientType);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchClientLookupGet FAILED: %d\n", rc);
        return rc;
    }
    rc = cpssDxChExactMatchClientLookupSet(devNum, paramsPtr->emUnitNum,
                                           paramsPtr->emLookupNum,
                                           paramsPtr->emClientType);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchClientLookupSet FAILED: %d\n", rc);
        return rc;
    }

    for (packetType = PRV_APP_IPFIX_FW_PKT_TYPE_IPV4_UDP_E;
         packetType <= PRV_APP_IPFIX_FW_PKT_TYPE_IPV6_TCP_E; packetType++)
    {
        switch (packetType)
        {
        case PRV_APP_IPFIX_FW_PKT_TYPE_IPV4_UDP_E:
            pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
            break;
        case PRV_APP_IPFIX_FW_PKT_TYPE_IPV4_TCP_E:
            pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
            break;
        case PRV_APP_IPFIX_FW_PKT_TYPE_IPV6_UDP_E:
            pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E;
            break;
        case PRV_APP_IPFIX_FW_PKT_TYPE_IPV6_TCP_E:
            pclPacketType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E;
            break;
        default:
            continue;
            break;
        }
        rc = cpssDxChExactMatchPclProfileIdSet(devNum, paramsPtr->emUnitNum,
                                               paramsPtr->pclDirection,
                                               pclPacketType,
                                               paramsPtr->pclSubProfileId,
                                               paramsPtr->emLookupNum,
                                               paramsPtr->pclEnableEmLookup,
                                               paramsPtr->pclProfileId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchPclProfileIdSet FAILED: %d\n", rc);
            return rc;
        }
    }
    /* Set Expanded Action for PCL lookup */
    rc = cpssDxChExactMatchExpandedActionSet(devNum, paramsPtr->emUnitNum,
                                             paramsPtr->expandedActionIndex,
                                             paramsPtr->emActionType,
                                             &paramsPtr->emActionData,
                                             &paramsPtr->expandedActionOrigin);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchExpandedActionSet FAILED: %d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvAppDemoIpfixFwExactMatchActionSet function
* @endinternal
*
* @brief   This routine set default values to an exact match
*          Action
*
* @param[in] type                - action type (TTI/PCL/EPCL)
* @param[in] actionPtr           - (pointer to) action
* @param[in] flowId              - flow identifier
* @param[in] packetCmd           - packet command
*/
static GT_STATUS prvAppDemoIpfixFwExactMatchActionSet
(
    IN GT_U8                                    devNum,
    IN GT_PORT_NUM                              egressPort,
    IN CPSS_DXCH_EXACT_MATCH_ACTION_TYPE_ENT    actionType,
    OUT CPSS_DXCH_EXACT_MATCH_ACTION_UNT        *actionPtr,
    IN GT_U32                                   flowId,
    IN CPSS_PACKET_CMD_ENT                      packetCmd
)
{
    GT_STATUS rc;
    GT_HW_DEV_NUM      hwDevNum = 0;
    cpssOsBzero((GT_VOID*) actionPtr, sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChCfgHwDevNumGet failed: rc=%d\n", rc);
        return rc;
    }

    switch(actionType)
    {
    case CPSS_DXCH_EXACT_MATCH_ACTION_TTI_E:
        actionPtr->ttiAction.ttPassengerPacketType              = CPSS_DXCH_TTI_PASSENGER_IPV4V6_E;
        actionPtr->ttiAction.tsPassengerPacketType              = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
        actionPtr->ttiAction.passengerParsingOfTransitNonMplsTransitTunnelEnable = GT_TRUE;
        actionPtr->ttiAction.command                            = CPSS_PACKET_CMD_FORWARD_E;

        actionPtr->ttiAction.redirectCommand                    = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;
        actionPtr->ttiAction.egressInterface.type               = CPSS_INTERFACE_PORT_E;
        actionPtr->ttiAction.egressInterface.devPort.hwDevNum   = hwDevNum;
        actionPtr->ttiAction.egressInterface.devPort.portNum    = egressPort;
        actionPtr->ttiAction.egressInterface.trunkId            = egressPort;

        actionPtr->ttiAction.pcl0OverrideConfigIndex            = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;
        actionPtr->ttiAction.pcl0_1OverrideConfigIndex          = CPSS_DXCH_PCL_LOOKUP_CONFIG_INDEX_OVERRIDE_E;

        actionPtr->ttiAction.iPclUdbConfigTableEnable           = GT_FALSE;
        actionPtr->ttiAction.iPclUdbConfigTableIndex            = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;

        actionPtr->ttiAction.mirrorToIngressAnalyzerEnable      = GT_FALSE;
        actionPtr->ttiAction.mirrorToIngressAnalyzerIndex       = 0;
        actionPtr->ttiAction.userDefinedCpuCode                 = CPSS_NET_FIRST_USER_DEFINED_E + 1;

        break;
    case CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E:
    case CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E:

        actionPtr->pclAction.pktCmd       = packetCmd;
        actionPtr->pclAction.mirror.cpuCode = appIpfixFwDbPtr->initCfg.txInitParams.newFlowsCpuCode;
        actionPtr->pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_NONE_E;

        actionPtr->pclAction.redirect.data.outIf.outInterface.type   = CPSS_INTERFACE_PORT_E;
        actionPtr->pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum   = hwDevNum;
        actionPtr->pclAction.redirect.data.outIf.outInterface.devPort.portNum    = egressPort;
        actionPtr->pclAction.vlan.ingress.vlanId     = PRV_APP_IPFIX_FW_VLAN_ID_CNS;
        actionPtr->pclAction.bypassBridge = GT_TRUE;
        actionPtr->pclAction.bypassIngressPipe = GT_TRUE;

        if(actionType==CPSS_DXCH_EXACT_MATCH_ACTION_EPCL_E)
        {
            actionPtr->pclAction.egressPolicy = CPSS_PCL_DIRECTION_EGRESS_E;
        }
        else
        {
            actionPtr->pclAction.egressPolicy = CPSS_PCL_DIRECTION_INGRESS_E;
        }

        actionPtr->pclAction.flowId = flowId;

        /*actionPtr->pclAction.policer.policerEnable  = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        actionPtr->pclAction.policer.policerId      = flowId;*/

        break;
    default:
        break;
    }
    return GT_OK;
}

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
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_CAPACITY_STC            emmCapacity;
    CPSS_DXCH_EXACT_MATCH_MANAGER_LOOKUP_STC              emmLookup;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ATTRIBUTES_STC    emmEntryAttr;
    CPSS_DXCH_EXACT_MATCH_MANAGER_AGING_STC               emmAging;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[1];
    GT_U32                                                numOfPairs = 1;
    CPSS_DXCH_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT      expandedActionOriginData;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                      actionData;
    GT_U32                                                i = 0;

    if ((paramsPtr->emmProfileEntryIndex >= CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_PROFILES_NUM_CNS) ||
        (paramsPtr->lookupNum >= CPSS_DXCH_EXACT_MATCH_LOOKUP_LAST_E))
    {
        return GT_OUT_OF_RANGE;
    }
    cpssOsMemSet(&emmEntryAttr, 0, sizeof(emmEntryAttr));
    cpssOsMemSet(&emmLookup, 0, sizeof(emmLookup));
    cpssOsMemSet(&emmCapacity, 0, sizeof(emmCapacity));
    cpssOsMemSet(&emmAging, 0, sizeof(emmAging));
    cpssOsMemSet(&pairListArr, 0, sizeof(pairListArr[1]));
    cpssOsMemSet(&expandedActionOriginData,0, sizeof(expandedActionOriginData));
    cpssOsMemSet(&actionData, 0, sizeof(actionData));

    /* Create IPFIX manager */
    emmCapacity.hwCapacity.numOfHwIndexes = PRV_APP_IPFIX_FW_HW_INDEXES_MAX_CNS;
    emmCapacity.hwCapacity.numOfHashes    = PRV_APP_IPFIX_FW_HW_HASHES_MAX_CNS;

     /*for key size CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E, it is counted as 2 entries*/
    switch(paramsPtr->emIpv4KeySize)
    {
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_5B_E:
        emmCapacity.maxTotalEntries           = PRV_APP_IPFIX_FW_EM_INDEX_MAX_CNS(devNum);
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E:
        emmCapacity.maxTotalEntries           = PRV_APP_IPFIX_FW_EM_INDEX_MAX_CNS(devNum) * 2;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_33B_E:
        emmCapacity.maxTotalEntries           = PRV_APP_IPFIX_FW_EM_INDEX_MAX_CNS(devNum) * 3;
        break;
    case CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E:
        emmCapacity.maxTotalEntries           = PRV_APP_IPFIX_FW_EM_INDEX_MAX_CNS(devNum) * 4;
        break;
    default:
        break;
    }

    emmCapacity.maxEntriesPerAgingScan    = PRV_APP_IPFIX_FW_MAX_NUM_ENTRIES_STEPS_CNS;
    emmCapacity.maxEntriesPerDeleteScan   = PRV_APP_IPFIX_FW_MAX_NUM_ENTRIES_STEPS_CNS;

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        emmCapacity.actionMatchWaEnable = GT_TRUE;
    }

    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupEnable=GT_TRUE;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClient=paramsPtr->lookupClient;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsNum=4;

    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[0].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[0].pclMappingElem.subProfileId = 0;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[0].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[0].pclMappingElem.profileId = paramsPtr->emmProfileEntryIndex;

    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[1].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[1].pclMappingElem.subProfileId = 0;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[1].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[1].pclMappingElem.profileId = paramsPtr->emmProfileEntryIndex;

    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[2].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[2].pclMappingElem.subProfileId = 0;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[2].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[2].pclMappingElem.profileId = paramsPtr->emmProfileEntryIndex/* + 1 */;

    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[3].pclMappingElem.packetType=CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[3].pclMappingElem.subProfileId = 0;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[3].pclMappingElem.enableExactMatchLookup=GT_TRUE;
    emmLookup.lookupsArray[paramsPtr->lookupNum].lookupClientMappingsArray[3].pclMappingElem.profileId = paramsPtr->emmProfileEntryIndex/* + 1 */;

    emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].isValidProfileId = GT_TRUE;
    emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].keyParams.keySize  = paramsPtr->emIpv4KeySize;
    emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].keyParams.keyStart = 0;
    emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].defaultActionType = paramsPtr->actionType;
    emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].defaultActionEn = GT_TRUE;

    prvAppDemoIpfixFwExactMatchActionSet(devNum,
                                         appIpfixFwDbPtr->testParams.egressPort,
                                         paramsPtr->actionType,
                                         &emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].defaultAction,
                                         0 /* flowId */, CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    cpssOsMemCpy(emmLookup.profileEntryParamsArray[paramsPtr->emmProfileEntryIndex].keyParams.mask,
                 paramsPtr->emIpv4KeyMask, sizeof(paramsPtr->emIpv4KeyMask));

#if 0
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].isValidProfileId = GT_TRUE;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].keyParams.keySize  = paramsPtr->emIpv6KeySize;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].keyParams.keyStart = 0;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].defaultActionType = paramsPtr->actionType;
    emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].defaultActionEn = GT_TRUE;

    prvAppDemoIpfixFwExactMatchActionSet(devNum,
                                         appIpfixFwDbPtr->testParams.egressPort,
                                         paramsPtr->actionType,
                                         &emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].defaultAction,
                                         0 /* flowId */, CPSS_PACKET_CMD_MIRROR_TO_CPU_E);
    cpssOsMemCpy(emmLookup.profileEntryParamsArray[emmProfileEntryIndex+1].keyParams.mask,
                 paramsPtr->emIpv6KeyMask, sizeof(paramsPtr->emIpv6KeyMask));
#endif
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionCutThroughTerminateId=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPktCmd=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionSkipFdbSaLookup=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionRedirectCommand=GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerIndex = GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionPolicerEnable = GT_TRUE;
    expandedActionOriginData.pclExpandedActionOrigin.overwriteExpandedActionFlowId = GT_TRUE;
    prvAppDemoIpfixFwExactMatchActionSet(devNum,
                                              appIpfixFwDbPtr->testParams.egressPort,
                                              paramsPtr->actionType,
                                              &actionData,
                                              0 /* flowId */, CPSS_PACKET_CMD_FORWARD_E);

    emmEntryAttr.expandedArray[paramsPtr->expandedActionIndex].exactMatchExpandedEntryValid=GT_TRUE;
    emmEntryAttr.expandedArray[paramsPtr->expandedActionIndex].expandedActionType=paramsPtr->actionType;
    cpssOsMemCpy(&emmEntryAttr.expandedArray[paramsPtr->expandedActionIndex].expandedAction,&actionData,sizeof(actionData));
    cpssOsMemCpy(&emmEntryAttr.expandedArray[paramsPtr->expandedActionIndex].expandedActionOrigin.pclExpandedActionOrigin,
                 &expandedActionOriginData, sizeof(expandedActionOriginData));

    cpssOsMemCpy(&paramsPtr->action,
                 &actionData,
                 sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

    emmAging.agingRefreshEnable = GT_FALSE;

    for (i=0; i< PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; i++)
    {
        rc = cpssDxChExactMatchManagerCreate(appIpfixFwDbPtr->mgrDb.exactMatchManagerId[i],
                                             &emmCapacity, &emmLookup,
                                             &emmEntryAttr, &emmAging);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchManagerCreate failed: rc=%d\n", rc);
            return rc;
        }

        /* Add device list */
        pairListArr[0].devNum        = devNum;
        pairListArr[0].portGroupsBmp = (1 << (i*2));
        numOfPairs = 1;
        rc = cpssDxChExactMatchManagerDevListAdd(appIpfixFwDbPtr->mgrDb.exactMatchManagerId[i],
                                                 pairListArr, numOfPairs);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchManagerDevListAdd failed: rc=%d\n", rc);
            return rc;
        }
    }

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_DXCH_EXACT_MATCH_MANAGER_DEV_PORT_GROUP_PAIR_STC pairListArr[1];
    GT_U32    numOfPairs = 1;
    GT_U32    i;


    for (i=0; i< PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; i++)
    {
        /* Add device list */
        pairListArr[0].devNum        = devNum;
        pairListArr[0].portGroupsBmp = (1 << (i*2));
        numOfPairs = 1;
        rc = cpssDxChExactMatchManagerDevListRemove(appIpfixFwDbPtr->mgrDb.exactMatchManagerId[i],
                                                    pairListArr, numOfPairs);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchManagerDevListRemove failed: rc=%d\n", rc);
            return rc;
        }

        rc = cpssDxChExactMatchManagerDelete(appIpfixFwDbPtr->mgrDb.exactMatchManagerId[i]);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchManagerDelete failed: rc=%d\n", rc);
            return rc;
        }
    }

    return GT_OK;
}

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
)
{
    GT_STATUS           rc = GT_OK;
    PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC *paramsPtr;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT  packetType[4] = {CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E,
                                                    CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E,
                                                    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E,
                                                    CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E};
    GT_U32                          i;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    paramsPtr = &appIpfixFwDbPtr->initCfg.emInitParams;

    /* Restore Expanded Action for PCL lookup */
    rc = cpssDxChExactMatchExpandedActionSet(devNum, paramsPtr->emUnitNum,
                                             paramsPtr->expandedActionIndex,
                                             paramsPtr->emActionType,
                                             &appIpfixFwDbPtr->restoreCfg.emAction,
                                             &appIpfixFwDbPtr->restoreCfg.emExpandedActionOrigin);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchExpandedActionSet FAILED: %d", rc);
        return rc;
    }

    /* Restore profileId for PCL first lookup */
    for(i=0; i<4; i++)
    {
        rc = cpssDxChExactMatchPclProfileIdSet(devNum, paramsPtr->emUnitNum,
                                               paramsPtr->pclDirection,
                                               packetType[i],
                                               paramsPtr->pclSubProfileId,
                                               paramsPtr->emLookupNum,
                                               appIpfixFwDbPtr->restoreCfg.emEnableExactMatchLookup,
                                               appIpfixFwDbPtr->restoreCfg.emProfileId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchPclProfileIdSet FAILED: %d\n", rc);
            return rc;
        }
    }

    /* Restore lookup client type */
    rc = cpssDxChExactMatchClientLookupSet(devNum, paramsPtr->emUnitNum,
                                           paramsPtr->emLookupNum,
                                           appIpfixFwDbPtr->restoreCfg.emClientType);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchClientLookupSet FAILED: %d\n", rc);
        return rc;
    }

    /* Restore exactMatchActivityEnable */
    rc =  cpssDxChExactMatchActivityBitEnableSet(devNum, paramsPtr->emUnitNum,
                                                 paramsPtr->emLookupNum,
                                                 GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchActivityBitEnableSet FAILED: %d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwEmAutoLearnInit function
 * @endinternal
 *
 * @brief  Initialize Exact Match Auto learn for IPFIX FW
 *
 * @param[in] devNum - device number
 * @param[in] paramsPtr - (pointer to) init parameters
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwEmAutoLearnInit
(
    IN GT_U8           devNum,
    IN PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    if (paramsPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: NULL Pointer Received\n");
        return GT_BAD_PTR;
    }


    /* Get and Set Exact Match Auto Learn Lookup Num */
    rc = cpssDxChExactMatchAutoLearnLookupGet(devNum, paramsPtr->emUnitNum,
                                              &appIpfixFwDbPtr->restoreCfg.emAutoLearnLookupNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnLookupGet FAILED: %d\n", rc);
        return rc;
    }
    rc = cpssDxChExactMatchAutoLearnLookupSet(devNum, paramsPtr->emUnitNum, paramsPtr->emLookupNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnLookupSet FAILED: %d\n", rc);
        return rc;
    }

    /* Application MUST disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum, paramsPtr->emUnitNum,
                                                                  paramsPtr->autoLearnProfileId,
                                                                  GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d\n", rc);
        return rc;
    }
    /* Get and Set auto learn flow ID config */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet(devNum,
                                                            paramsPtr->emUnitNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &appIpfixFwDbPtr->restoreCfg.emAutoLearnFlowIdConfig);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchPortGroupAutoLearnFlowIdParamGet FAILED: %d\n", rc);
        return rc;
    }
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,
                                                            paramsPtr->emUnitNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &paramsPtr->autoLearnFlowIdConfig);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet FAILED: %d\n", rc);
        return rc;
    }

    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum, paramsPtr->emUnitNum,
                                                                  paramsPtr->autoLearnProfileId,
                                                                  GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d\n", rc);
        return rc;
    }

    /* Get and Set Profile Key Params */
    rc =  cpssDxChExactMatchProfileKeyParamsGet(devNum, paramsPtr->emUnitNum,
                                                paramsPtr->emProfileId,
                                                &appIpfixFwDbPtr->restoreCfg.emAutoLearnProfileKeyParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchProfileKeyParamsGet FAILED: %d\n", rc);
        return rc;
    }
    rc =  cpssDxChExactMatchProfileKeyParamsSet(devNum, paramsPtr->emUnitNum,
                                                paramsPtr->emProfileId,
                                                &paramsPtr->emProfileKeyParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchProfileKeyParamsSet FAILED: %d\n", rc);
        return rc;
    }

    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum, paramsPtr->emUnitNum,
                                                            paramsPtr->autoLearnProfileId,
                                                            paramsPtr->expandedActionIndex,
                                                            paramsPtr->autoLearnActionType,
                                                            &paramsPtr->autoLearnDefaulActionData,
                                                            GT_TRUE/*actionEn*/ );
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnProfileDefaultActionSet FAILED: %d\n", rc);
        return rc;
    }


    return GT_OK;
}

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
)
{
    GT_STATUS           rc = GT_OK;
    PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC *paramsPtr;
    GT_U32              flowDbId = 0;

    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Error: Application database not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    paramsPtr = &appIpfixFwDbPtr->initCfg.emAutoLearnInitParams;

    for (flowDbId=0; flowDbId < PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum); flowDbId++)
    {
        if (appIpfixFwDbPtr->flowsDb[flowDbId].info.isActive == GT_TRUE)
        {
            rc = cpssDxChExactMatchPortGroupEntryInvalidate(devNum, paramsPtr->emUnitNum,
                                                            paramsPtr->emLookupNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            appIpfixFwDbPtr->flowsDb[flowDbId].info.emIndex);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChExactMatchPortGroupEntryInvalidate FAILED: FlowId=%d, emIndex=%d, rc=%d\n",
                                PRV_APP_IPFIX_FW_DB_ID_2_FLOW_ID_CONVERT(devNum, flowDbId),
                                appIpfixFwDbPtr->flowsDb[flowDbId].info.emIndex, rc);
                return rc;
            }
        }
    }
#if 0
    /* Restore default action for Auto Learn entry */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionSet(devNum, paramsPtr->emUnitNum,
                                                            paramsPtr->autoLearnProfileId,
                                                            paramsPtr->expandedActionIndex,
                                                            paramsPtr->autoLearnActionType,
                                                            &paramsPtr->autoLearnDefaulActionData,
                                                            GT_FALSE/*actionEn*/ );
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnProfileDefaultActionSet FAILED: %d\n", rc);
        return rc;
    }
#endif
    /* Restore Profile Key Params */
    rc =  cpssDxChExactMatchProfileKeyParamsSet(devNum, paramsPtr->emUnitNum,
                                                paramsPtr->emProfileId,
                                                &appIpfixFwDbPtr->restoreCfg.emAutoLearnProfileKeyParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchProfileKeyParamsSet FAILED: %d\n", rc);
        return rc;
    }

    /* Application MUST disable Exact Match Auto Learning per profile ID before changing Flow ID allocation configuration */
    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum, paramsPtr->emUnitNum,
                                                                  paramsPtr->autoLearnProfileId,
                                                                  GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d\n", rc);
        return rc;
    }
    /* Restore auto learn flow ID config */
    rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet(devNum,
                                                            paramsPtr->emUnitNum,
                                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                                            &appIpfixFwDbPtr->restoreCfg.emAutoLearnFlowIdConfig);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchPortGroupAutoLearnFlowIdParamSet FAILED: %d\n", rc);
        return rc;
    }

    rc = cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet(devNum, paramsPtr->emUnitNum,
                                                                  paramsPtr->autoLearnProfileId,
                                                                  GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnProfileDefaultActionEnableSet FAILED: %d\n", rc);
        return rc;
    }

    /* Restore Exact Match Auto Learn Lookup Num */
    rc = cpssDxChExactMatchAutoLearnLookupSet(devNum, paramsPtr->emUnitNum,
                                              appIpfixFwDbPtr->restoreCfg.emAutoLearnLookupNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChExactMatchAutoLearnLookupSet FAILED: %d\n", rc);
        return rc;
    }

    return GT_OK;
}

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
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Application DB not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    if (paramsPtr == NULL)
    {
        __IPFIX_FW_LOG("testParamsPtr cannot be null\n");
        return GT_BAD_PTR;
    }

    cpssOsMemCpy(&appIpfixFwDbPtr->testParams, paramsPtr, sizeof(*paramsPtr));
    return GT_OK;
}

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
)
{
    if (appIpfixFwDbPtr == NULL)
    {
        __IPFIX_FW_LOG("Application DB not initialized\n");
        return GT_NOT_INITIALIZED;
    }

    if (testParamsPtr == NULL)
    {
        __IPFIX_FW_LOG("testParamsPtr cannot be null\n");
        return GT_BAD_PTR;
    }

    cpssOsMemCpy(testParamsPtr,
               &appIpfixFwDbPtr->testParams,
               sizeof(PRV_APP_IPFIX_FW_TEST_PARAMS_STC));

    return GT_OK;
}

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
    IN GT_U8    devNum
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_APP_IPFIX_FW_TEST_PARAMS_STC    defaultTestParams;
    GT_U32                              i;

    (void) devNum;

    /* Allocate memory for IPFIX Manager application database */
    appIpfixFwDbPtr = (PRV_APP_IPFIX_FW_DB_STC *) cpssOsMalloc(sizeof(PRV_APP_IPFIX_FW_DB_STC));
    __IPFIX_FW_NULL_PTR_CHECK(appIpfixFwDbPtr);

    /* Initialize Parameters */
    cpssOsMemSet(appIpfixFwDbPtr, 0, sizeof(PRV_APP_IPFIX_FW_DB_STC));
    cpssOsMemSet(&defaultTestParams, 0, sizeof(defaultTestParams));

    defaultTestParams.waitTimeForExportComplete = PRV_APP_IPFIX_FW_EXPORT_COMPLETE_WAIT_TIME;
    defaultTestParams.newFlowsPollInterval      = PRV_APP_IPFIX_FW_NEW_FLOWS_SLEEP_TIME_CNS;
    defaultTestParams.dataPktsPollInterval      = PRV_APP_IPFIX_FW_DATA_PKTS_SLEEP_TIME_CNS;
    defaultTestParams.ipcFetchPollInterval      = PRV_APP_IPFIX_FW_IPC_EVENTS_SLEEP_TIME_CNS;
    defaultTestParams.idleTimeout               = PRV_APP_IPFIX_FW_ENTRY_IDLE_TIMEOUT_CNS;
    defaultTestParams.activeTimeout             = PRV_APP_IPFIX_FW_ENTRY_ACTIVE_TIMEOUT_CNS;
    defaultTestParams.srcPort                   = PRV_APP_IPFIX_FW_TEST_SRC_PORT_NUM_CNS;
    defaultTestParams.egressPort                = PRV_APP_IPFIX_FW_TEST_EGRESS_PORT_NUM_CNS;
    appDemoIpfixFwTestParamsSet(&defaultTestParams);

    for (i=0; i< PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; i++)
    {
        appIpfixFwDbPtr->mgrDb.exactMatchManagerId[i] = i * 2;
    }
    appIpfixFwDbPtr->mgrDb.keysLookupAlgo = PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_SERIAL_E;
    /*appIpfixFwDbPtr->mgrDb.keysLookupAlgo = PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_HASH_E;*/
    appIpfixFwDbPtr->mgrDb.hashKeySize = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 13: 16;

    /* enable bebug counters */
    appIpfixFwDbPtr->dbg.debugCountersEnable = GT_TRUE;

    return rc;
}

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
)
{

    (void) devNum;

    if(appIpfixFwDbPtr)
    {
        cpssOsFree(appIpfixFwDbPtr);
    }
    appIpfixFwDbPtr = NULL;

    return GT_OK;
}

static GT_STATUS prvAppDemoIpfixFwPktUdbOffsetsBuild
(
    IN GT_U8        devNum
)
{
    GT_STATUS   rc = GT_OK;
    CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT ruleFormatType;
    PRV_APP_IPFIX_FW_PACKET_TYPE_ENT   appPacketType;
    CPSS_PCL_DIRECTION_ENT             pclDirection;
    CPSS_PCL_LOOKUP_NUMBER_ENT         pclLookupNum;
    CPSS_DXCH_PCL_PACKET_TYPE_ENT      packetType;
    CPSS_DXCH_PCL_UDB_SELECT_STC       udbSelectStruct;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT      offsetType[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];
    GT_U8                              offset[CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS];
    GT_U32                             i;

    ruleFormatType = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    pclDirection = appIpfixFwDbPtr->initCfg.pclInitParams.direction;
    pclLookupNum = appIpfixFwDbPtr->initCfg.pclInitParams.lookupNum;

    cpssOsMemSet(&udbSelectStruct, 0, sizeof(udbSelectStruct));

    cpssOsMemSet(appIpfixFwDbPtr->mgrDb.pktUdbOffset, 0,
                 sizeof(appIpfixFwDbPtr->mgrDb.pktUdbOffset));
    cpssOsMemSet(appIpfixFwDbPtr->mgrDb.pktUdbOffsetType, 0,
                 sizeof(appIpfixFwDbPtr->mgrDb.pktUdbOffsetType));

    for(appPacketType = PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_TCP_E;
        appPacketType < PRV_APP_IPFIX_FW_PACKET_TYPE_LAST_E;
        appPacketType ++)
    {
        switch(appPacketType)
        {
            case PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_UDP_E:
                packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_UDP_E;
                break;
            case PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_TCP_E:
                packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV4_TCP_E;
                break;
            case PRV_APP_IPFIX_FW_PACKET_TYPE_IPV6_UDP_E:
                packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_UDP_E;
                break;
            case PRV_APP_IPFIX_FW_PACKET_TYPE_IPV6_TCP_E:
                packetType = CPSS_DXCH_PCL_PACKET_TYPE_IPV6_TCP_E;
                break;
            default:
                break;
        }

        /* Read from UDB select */
        rc = cpssDxChPclUserDefinedBytesSelectGet(devNum,
                                                  ruleFormatType,
                                                  packetType,
                                                  pclLookupNum,
                                                  &udbSelectStruct);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPclUserDefinedBytesSelectGet failed: rc=%d", rc);
        }
        for(i=0; i<CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS; i++)
        {
            if (pclDirection == CPSS_PCL_DIRECTION_INGRESS_E)
            {
                if ((i < CPSS_DXCH_PCL_INGRESS_UDB_REPLACE_MAX_CNS) &&
                    (udbSelectStruct.ingrUdbReplaceArr[i] == GT_TRUE))
                {
                    continue;
                }
            }
            else
            {
                /* [TBD] pclDirection == Egress not tested*/
            }

            rc = cpssDxChPclUserDefinedByteGet(devNum,
                                               ruleFormatType,
                                               packetType,
                                               pclDirection,
                                               udbSelectStruct.udbSelectArr[i],
                                               &offsetType[i],
                                               &offset[i]);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChPclUserDefinedByteGet failed: rc=%d", rc);
            }

            if (i < CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS)
            {
                appIpfixFwDbPtr->mgrDb.pktUdbOffset[appPacketType][i] = offset[i];
                appIpfixFwDbPtr->mgrDb.pktUdbOffsetType[appPacketType][i] = offsetType[i];
            }
        }
    }
    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixFwFlowKeyGet function
 * @endinternal
 *
 * @brief This function parses the packet recieved and extract the key
 *
 * @param[in]  pktBuffsArr  -  (pointer to) packet Buffer array
 * @param[in]  rxParamsPtr  -  (pointer to) RX parameters
 * @param[out] flowPtr      -  (pointer to) flow structure
 *
 * @retval  GT_OK   - on success
 */
static GT_STATUS prvAppDemoIpfixFwFlowKeyGet
(
    IN GT_U8                       **pktBuffsArr,
    IN GT_U32                      *buffLenArr,
    IN CPSS_DXCH_NET_RX_PARAMS_STC *rxParamsPtr,
    OUT PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC   *flowPtr
)
{
    GT_U32                          l3minus2Offset = 16;
    GT_U32                          l4Offset = l3minus2Offset + 22;
    GT_U8                           *pktBuff = *pktBuffsArr;
    GT_U8                           startOffset = 0;
    GT_U32                          i;
    GT_BOOL                         isVlanTagged = GT_FALSE;
    GT_BOOL                         isIpv4 = GT_FALSE;
    GT_BOOL                         isIpv6 = GT_FALSE;
    GT_BOOL                         isTCP  = GT_FALSE;
    GT_BOOL                         isUDP  = GT_FALSE;
    GT_U32                          protocolOffset = 0;

    cpssOsMemCpy(&flowPtr->dsaParam, &rxParamsPtr->dsaParam, sizeof(rxParamsPtr->dsaParam));

    /* Packet classification */
    isVlanTagged = ((pktBuff[12] == 0x81) && (pktBuff[13] == 0)) ? GT_TRUE : GT_FALSE;
    if (isVlanTagged == GT_FALSE)
    {
        l3minus2Offset = 12;
    }
    else
    {
        l3minus2Offset = 16;
    }

    /* IPv4/IPv6 classification */
    if ((pktBuff[l3minus2Offset] == 0x08) && (pktBuff[l3minus2Offset + 1] == 0x00))
    {
        isIpv4 = GT_TRUE;
        l4Offset = l3minus2Offset + 22;
        protocolOffset = 11;
        if (appIpfixFwDbPtr->fwCfg.globalCfg.hwAutoLearnEnable == GT_TRUE)
        {
            flowPtr->key.keySize = appIpfixFwDbPtr->initCfg.emAutoLearnInitParams.emProfileKeyParams.keySize;
        }
        else
        {
            flowPtr->key.keySize = appIpfixFwDbPtr->initCfg.emmInitParams.emIpv4KeySize;
        }
    }
    else if ((pktBuff[l3minus2Offset] == 0x86) && (pktBuff[l3minus2Offset + 1] == 0xDD))
    {
        isIpv6 = GT_TRUE;
        l4Offset = l3minus2Offset + 42;
        protocolOffset = 8;
        if (appIpfixFwDbPtr->fwCfg.globalCfg.hwAutoLearnEnable == GT_TRUE)
        {
            flowPtr->key.keySize = appIpfixFwDbPtr->initCfg.emAutoLearnInitParams.emProfileKeyParams.keySize;
        }
        else
        {
            flowPtr->key.keySize = appIpfixFwDbPtr->initCfg.emmInitParams.emIpv6KeySize;
        }
        (void) isIpv6;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    /* TCP/UDP classification */
    if (pktBuff[l3minus2Offset + protocolOffset] == 6)
    {
        isTCP = GT_TRUE;
    }
    else if (pktBuff[l3minus2Offset + protocolOffset] == 17)
    {
        isUDP = GT_TRUE;
        (void) isUDP;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }

    /* Packet Type */
    flowPtr->pktType = isIpv4 ?
                 (isTCP ? PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_TCP_E : PRV_APP_IPFIX_FW_PACKET_TYPE_IPV4_UDP_E) :
                 (isTCP ? PRV_APP_IPFIX_FW_PACKET_TYPE_IPV6_TCP_E : PRV_APP_IPFIX_FW_PACKET_TYPE_IPV6_UDP_E);

    for(i=0; i<CPSS_DXCH_PCL_UDB_MAX_NUMBER_CNS; i++)
    {
        startOffset = 0;
        if (appIpfixFwDbPtr->mgrDb.pktUdbOffsetType[flowPtr->pktType][i] == CPSS_DXCH_PCL_OFFSET_L3_MINUS_2_E)
        {
            startOffset = l3minus2Offset;
        }
        else if (appIpfixFwDbPtr->mgrDb.pktUdbOffsetType[flowPtr->pktType][i] == CPSS_DXCH_PCL_OFFSET_L4_E)
        {
            startOffset = l4Offset;
        }
        else
        {
            /* [TBD] other offset types not defined yet */
        }

        /* [TBD] Handle offset Type CPSS_DXCH_PCL_OFFSET_METADATA_E */
        if ((i < CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS) &&
            (startOffset + appIpfixFwDbPtr->mgrDb.pktUdbOffset[flowPtr->pktType][i] < buffLenArr[0]))
        {
            if (isIpv4)
            {
                flowPtr->key.pattern[i] = (GT_U8)((pktBuff[startOffset + appIpfixFwDbPtr->mgrDb.pktUdbOffset[flowPtr->pktType][i]]) &
                    (appIpfixFwDbPtr->initCfg.emmInitParams.emIpv4KeyMask[i]));
            }
            else
            {
                flowPtr->key.pattern[i] = (GT_U8)((pktBuff[startOffset + appIpfixFwDbPtr->mgrDb.pktUdbOffset[flowPtr->pktType][i]]) &
                    (appIpfixFwDbPtr->initCfg.emmInitParams.emIpv6KeyMask[i]));
            }
        }
    }

    return GT_OK;
}

static GT_STATUS prvAppDemoIpfixFwSdmaRxBufFree
(
    IN GT_U8      devNum,
    IN GT_U8      sdmaQNum,
    IN GT_U32     numOfBuffs,
    IN GT_U8      *packetBuffsArrPtr[], /*arrSizeVarName=numOfBuffPtr*/
    IN GT_U32     buffLenArr[] /*arrSizeVarName=numOfBuffPtr*/
)
{
    GT_STATUS rc = GT_OK;

    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        /* now you need to free the buffers */
        if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[sdmaQNum] != CPSS_RX_BUFF_NO_ALLOC_E)
        {
            rc = cpssDxChNetIfRxBufFree(devNum,
                                        sdmaQNum,
                                        packetBuffsArrPtr,numOfBuffs);
        }
        else
        {
            /* move the first buffer pointer to the original place*/
            packetBuffsArrPtr[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffsArrPtr[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                      << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

            /* fix the last buffer's size*/
            buffLenArr[numOfBuffs-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

            rc = cpssDxChNetIfRxBufFreeWithSize(devNum,
                                                sdmaQNum,
                                                packetBuffsArrPtr,buffLenArr,numOfBuffs);
        }

    }
    else /* CPSS_NET_CPU_PORT_MODE_MII_E */
    {
        rc = cpssDxChNetIfMiiRxBufFree(devNum,
                                       sdmaQNum,
                                       packetBuffsArrPtr,numOfBuffs);
    }
    return rc;
}

static GT_VOID prvAppDemoIpfixFwKeysDbHashCalc
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroupId,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC   *keyPtr,
    OUT GT_U32                          *hashValPtr
)
{
    GT_U32  hashKeySize = appIpfixFwDbPtr->mgrDb.hashKeySize;
    GT_U32  evenByteSum = 0;
    GT_U32  oddByteSum = 0;
    GT_U32  i, hashVal;

    (void) hashVal;

    (void) devNum;
    /* Max key hash value is 16 bits */

    /* calculate sum of even bytes and odd bytes */
    for (i=0; i<CPSS_DXCH_EXACT_MATCH_MAX_KEY_SIZE_CNS; i++)
    {
        if(i%2 == 0)
        {
            evenByteSum += (GT_U32)keyPtr->pattern[i];
        }
        else
        {
            oddByteSum += (GT_U32)keyPtr->pattern[i];
        }
    }

    *hashValPtr = (((evenByteSum << 8) | (oddByteSum )) +
                   ((oddByteSum << 8) | (evenByteSum ))) & 0xFFFF;

    hashVal = *hashValPtr;
    if(portGroupId < PRV_APP_IPFIX_FW_PORT_GROUPS_MAX_CNS)
    {
        *hashValPtr = ((*hashValPtr & ((1 << hashKeySize)-1)) |
                      (portGroupId << 13)) & 0xFFFF;
    }

    /* [TBD] Remove this */
    /* cpssOsPrintf("HashValue Final: %d, HashVal interm:%d, evenByteSum:%d, oddByteSum:%d \n",
     *hashValPtr, hashVal, oddByteSum, evenByteSum); */
}

static GT_STATUS prvAppDemoIpfixFwKeysDbEntryAdd
(
    GT_U32  devNum,
    GT_U32  portGroupId,
    CPSS_DXCH_EXACT_MATCH_KEY_STC *keyPtr
)
{
    PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *nodeItrPtr = NULL;
    PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *newNodePtr = NULL;
    GT_U32  hashVal = 0;

    (void) devNum;

    prvAppDemoIpfixFwKeysDbHashCalc(devNum, portGroupId, keyPtr, &hashVal);

    if(hashVal >= PRV_APP_IPFIX_FW_KEYS_DB_MAX_CNS)
    {
        return GT_BAD_VALUE;
    }

    nodeItrPtr = appIpfixFwDbPtr->mgrDb.keysDb[hashVal];
    newNodePtr = (PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *)cpssOsMalloc(sizeof(PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC));
    appIpfixFwDbPtr->mgrDb.activeKeysDbNodes ++;
    cpssOsMemCpy(&newNodePtr->key, keyPtr, sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
    newNodePtr->nextKeyNodePtr = NULL;
    if(nodeItrPtr == NULL)
    {
        appIpfixFwDbPtr->mgrDb.keysDb[hashVal] = newNodePtr;
    }
    else
    {
        while(nodeItrPtr != NULL)
        {
            if(nodeItrPtr->nextKeyNodePtr == NULL)
            {
                nodeItrPtr->nextKeyNodePtr = newNodePtr;
                break;
            }
            nodeItrPtr = nodeItrPtr->nextKeyNodePtr;
        }
    }
    return GT_OK;
}

static GT_STATUS prvAppDemoIpfixFwKeysDbEntryDelete
(
    IN  GT_U8                         devNum,
    IN  GT_32                         portGroupId,
    IN  CPSS_DXCH_EXACT_MATCH_KEY_STC *keyPtr
)
{
    GT_32 retVal = 0;
    GT_U32  hashVal = 0;
    PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *nodeItrPtr = NULL;
    PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *prevNodePtr = NULL;
    GT_BOOL     isDeleted = GT_FALSE;
    GT_U32      maxLoopCount = 4096;
    GT_U32      loopCount = 0;

    prvAppDemoIpfixFwKeysDbHashCalc(devNum, portGroupId, keyPtr, &hashVal);

    if(hashVal >= PRV_APP_IPFIX_FW_KEYS_DB_MAX_CNS)
    {
        return GT_BAD_STATE;
    }

    nodeItrPtr = appIpfixFwDbPtr->mgrDb.keysDb[hashVal];
    prevNodePtr = nodeItrPtr;
    if(nodeItrPtr == NULL)
    {
        __IPFIX_FW_LOG("[Err] not found Hash: %d, key: %02X%02X%02X%02X%02X%02X%02X%02X...\n",
                     hashVal, keyPtr->pattern[0], keyPtr->pattern[1], keyPtr->pattern[2],
                     keyPtr->pattern[3], keyPtr->pattern[4], keyPtr->pattern[5], keyPtr->pattern[6],
                     keyPtr->pattern[7]);
        return GT_NO_SUCH;
    }

    while(nodeItrPtr != NULL)
    {
        retVal = cpssOsMemCmp(&nodeItrPtr->key, keyPtr, sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
        if(retVal == 0)
        {
            if((nodeItrPtr == prevNodePtr) && (nodeItrPtr->nextKeyNodePtr == NULL))
            {
                appIpfixFwDbPtr->mgrDb.keysDb[hashVal] = NULL;
            }
            else
            {
                prevNodePtr->nextKeyNodePtr = nodeItrPtr->nextKeyNodePtr;
            }
            cpssOsFree(nodeItrPtr);
            appIpfixFwDbPtr->mgrDb.activeKeysDbNodes --;
            isDeleted = GT_TRUE;
            break;
        }
        prevNodePtr = nodeItrPtr;
        nodeItrPtr = nodeItrPtr->nextKeyNodePtr;
        maxLoopCount ++;
        if(loopCount > maxLoopCount)
        {
            __IPFIX_FW_LOG("[Err] loopCount exceeds limit. Should never happen\n");
        }
    }

    if (isDeleted == GT_FALSE)
    {
        __IPFIX_FW_LOG("[Err] not found Hash: %d, key: %02X%02X%02X%02X%02X%02X%02X%02X...\n",
                     hashVal, keyPtr->pattern[0], keyPtr->pattern[1], keyPtr->pattern[2],
                     keyPtr->pattern[3], keyPtr->pattern[4], keyPtr->pattern[5], keyPtr->pattern[6],
                     keyPtr->pattern[7]);
        return GT_NO_SUCH;
    }

    return GT_OK;
}

/**
 * @internal prvAppDemoIpfixFwFlowKeySearch function
 * @endinternal
 *
 * @brief  Search if a key is present in the IPFIX maanger database
 *
 * @param[in]  devNum      - device number
 * @param[in]  portGroupId - port group ID
 * @param[in]  flowPtr     - (pointer to) flow structure
 * @param[out] isFoundPtr  - (pointer to) GT_TRUE - if key is found
 *                      GT_FALSE - if the key is not found.
 *
 * @retval GT_OK    -  on success
 */
static GT_STATUS prvAppDemoIpfixFwFlowKeySearch
(
    IN GT_U8                                  devNum,
    IN GT_U32                                 portGroupId,
    IN  PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC   *flowPtr,
    OUT GT_BOOL                               *isFoundPtr
)
{
    GT_U32      i = 0;
    GT_32       ret = 0;
    GT_U32      flowDbId;
    GT_U32      startIndex;
    GT_U32      hashVal;
    PRV_APP_IPFIX_FW_KEYS_DB_NODE_STC *nodePtr = NULL;
    GT_U32      nodesCount;

    *isFoundPtr = GT_FALSE;

    startIndex = portGroupId;
    if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow ==
         CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
    {
        startIndex = portGroupId & 0x6;
    }

    if (appIpfixFwDbPtr->mgrDb.keysLookupAlgo != PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_HASH_E)
    {
        startIndex = startIndex << 12;
        flowDbId = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, startIndex);
        for (i=flowDbId; i < flowDbId + PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum); i++)
        {
            if (appIpfixFwDbPtr->flowsDb[i].info.isActive == GT_FALSE)
            {
                continue;
            }
            ret = cpssOsMemCmp(&flowPtr->key,
                         &(appIpfixFwDbPtr->flowsDb[i].key),
                         sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
            if (ret == 0)
            {
                *isFoundPtr = GT_TRUE;
                break;
            }
        }
    }
    else
    {
        prvAppDemoIpfixFwKeysDbHashCalc(devNum, portGroupId, &flowPtr->key, &hashVal);

        nodePtr = appIpfixFwDbPtr->mgrDb.keysDb[hashVal];
        nodesCount = 0;
        while(nodePtr != NULL)
        {
            ret = cpssOsMemCmp(&flowPtr->key, &nodePtr->key,
                         sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
            if (ret == 0)
            {
                *isFoundPtr = GT_TRUE;
                break;
            }

            nodePtr = nodePtr->nextKeyNodePtr;
            nodesCount ++;
            if (nodesCount > 256)
            {
                /*cpssOsPrintf("Warn: nodes linked list is more than 256\n");*/
            }
        }
    }

    return GT_OK;
}

/**
 * @internal  prvAppDemoIpfixFwNewFlowsHandle funtion
 * @endinternal
 *
 * @brief  Task to handle first packets of the new flows.
 *         It extracts key and installs entry in the exact match manager
 *         and IPFUX manager
 *
 * @param[in] arg  - input arguments for the task. It includes device number
 *
 * @retval unsigned __TASKCONV
 */
static unsigned __TASKCONV prvAppDemoIpfixFwNewFlowsHandle
(
    GT_VOID *arg
)
{
    GT_STATUS                    rc = GT_OK;
    GT_U8                        devNum = 0;
    GT_U32                       numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
    GT_U8                        *packetBuffs[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    GT_U32                       buffLenArr[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC  rxParams;
    GT_U32                       newFlowsCount = 0;
    GT_U32                       flowId = 0;
    GT_U8                        sdmaQNum;

    (void) arg;
    sdmaQNum = appIpfixFwDbPtr->initCfg.txInitParams.newFlowsQueueNum;
    devNum   = appIpfixFwDbPtr->threads.newFlows.devNum;

    if (appIpfixFwDbPtr->threads.newFlows.isTaskActive == GT_TRUE)
    {
        return GT_BAD_STATE;
    }

    appIpfixFwDbPtr->threads.newFlows.taskTerminate = GT_FALSE;
    appIpfixFwDbPtr->threads.newFlows.isTaskActive = GT_TRUE;

    newFlowsCount = 0;
    while (appIpfixFwDbPtr->threads.newFlows.taskTerminate == GT_FALSE)
    {
        /* get the packet from the device */
        if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        {
            numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
            rc = cpssDxChNetIfSdmaRxPacketGet(devNum, sdmaQNum,
                                              &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr, &rxParams);
            if (rc != GT_OK)
            {
                if (rc != GT_NO_MORE)
                {
                    __IPFIX_FW_LOG("cpssDxChNetIfSdmaRxPacketGet failed: rc = %d\n", rc);
                }
                continue;
            }
        }
        else if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
        {
            rc = cpssDxChNetIfMiiRxPacketGet(devNum, sdmaQNum,
                                             &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr,&rxParams);
            if (rc != 0)
            {
                __IPFIX_FW_LOG("cpssDxChNetIfMiiRxPacketGet failed: rc = %d\n", rc);
                continue;
            }
        }
        else
        {
            __IPFIX_FW_LOG("Invalid CPU port mode\n");
            rc = GT_NOT_SUPPORTED;
            break;
        }

        __IPFIX_FW_DBG_COUNTER_INCREMENT(firstPktsFetchedByHost);

        flowId = rxParams.dsaParam.dsaInfo.toCpu.flowIdTtOffset.flowId;
        if (flowId < PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum))
        {
            if (appIpfixFwDbPtr->flowsDb[flowId].info.isActive == GT_FALSE)
            {
                __IPFIX_FW_LOG("Warn: FlowId %d is not active but updating metadata anyway\n", flowId);
            }
            /* Update flow Metadata */
            cpssOsMemCpy(&appIpfixFwDbPtr->flowsDb[flowId].dsaParam,
                         &rxParams.dsaParam, sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));
        }
        else
        {
            __IPFIX_FW_DBG_COUNTER_INCREMENT(firstPktsFilteredByHost);
        }

        rc = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc);
        }

        newFlowsCount++;
        if (newFlowsCount == appIpfixFwDbPtr->threads.newFlows.burstSize)
        {
            newFlowsCount = 0;
            /* [TBD] Stop after adding 1K new flows */
        }
    }

    appIpfixFwDbPtr->threads.newFlows.taskTerminate = GT_TRUE;
    appIpfixFwDbPtr->threads.newFlows.isTaskActive = GT_FALSE;

    /* to avoid compiler warnings */
    return 0;
}

/**
 * @internal prvAppDemoIpfixFwFreeFlowIdGet function
 *
 * @brief Get Free Flow ID
 *
 * @param[in]  devNum      -  device number
 * @param[in]  portGroupId -  port group id
 * @param[out] flowIdPtr   -  (pointer to) free flowId
 *
 * @retval GT_OK        - on success
 * @retval GT_BAD_PARAM - parameter out of range
 * @retval GT_FULL      - flowId database is full
 */
GT_STATUS prvAppDemoIpfixFwFreeFlowIdGet
(
    IN  GT_U8          devNum,
    IN  GT_U32         portGroupId,
    OUT GT_U32         *flowIdPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32                                      flowId = 0;
    GT_U32                                      flowDbId = 0;
    GT_U32                                      i = 0;
    GT_U32                                      j = 0;
    GT_U32                                      startIndex;
    GT_U32                                      maxFlowId = 0;
    GT_BOOL                                     isFound = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(flowIdPtr);

    maxFlowId = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum);

    startIndex = portGroupId;
    if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow ==
         CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
    {
        startIndex = portGroupId & 0x6;
    }
    startIndex = startIndex << 12;

    flowId = startIndex;
    flowDbId = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, startIndex);
    for (i = flowDbId/32; i < (flowDbId/32 + maxFlowId/32); i++)
    {
        if (appIpfixFwDbPtr->mgrDb.usedFlowsBmp[i] == 0xFFFFFFFF)
        {
            flowId += 32;
            continue;
        }
        for (j = 0; j < 32; j++)
        {
            if (((appIpfixFwDbPtr->mgrDb.usedFlowsBmp[i] >> j) & 0x1) == 0)
            {
                flowId += j;
                isFound = GT_TRUE;
                break;
            }
        }
        if (isFound == GT_TRUE)
        {
            break;
        }
        flowId += 32;
    }

    if (isFound == GT_FALSE)
    {
        return GT_NO_RESOURCE;
    }

    *flowIdPtr = flowId;
    appIpfixFwDbPtr->mgrDb.usedFlowsBmp[i] |= (1 << (j%32));

    return rc;
}

/**
 * @internal  prvAppDemoIpfixFwManualNewFlowsHandle funtion
 * @endinternal
 *
 * @brief  Task to handle first packets of the new flows.
 *         It extracts key and installs entry in the exact match manager
 *         and IPFUX manager
 *
 * @param[in] arg  - input arguments for the task. It includes device number
 *
 * @retval unsigned __TASKCONV
 */
static unsigned __TASKCONV prvAppDemoIpfixFwManualNewFlowsHandle
(
    GT_VOID *arg
)
{
    GT_STATUS                                             rc = GT_OK;
    GT_STATUS                                             rc2 = GT_OK;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC               emmEntry;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_ADD_PARAMS_STC    emmEntryParams;
    GT_U8                                       devNum = 0;
    GT_U32                                      sCpuNum = 0;
    GT_U32                                      numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
    GT_U8                                       *packetBuffs[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    GT_U32                                      buffLenArr[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC                 rxParams;
    GT_BOOL                                     isFound = GT_FALSE;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC         flow;
    GT_U32                                      portGroupId = 0;
    GT_U32                                      flowId = 0;
    CPSS_DXCH_DETAILED_PORT_MAP_STC             portMapShadow;
    GT_U32                                      portNum;
    GT_U8                                       sdmaQNum;
    CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC         ipfixEntryParams;

    (void) arg;
    devNum   = appIpfixFwDbPtr->threads.newFlows.devNum;
    sCpuNum  = appIpfixFwDbPtr->threads.newFlows.sCpuNum;
    sdmaQNum = appIpfixFwDbPtr->initCfg.txInitParams.newFlowsQueueNum;
    cpssOsMemSet(&ipfixEntryParams, 0, sizeof(CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC));

    if (appIpfixFwDbPtr->threads.newFlows.isTaskActive == GT_TRUE)
    {
        return GT_BAD_STATE;
    }

    appIpfixFwDbPtr->threads.newFlows.taskTerminate = GT_FALSE;
    appIpfixFwDbPtr->threads.newFlows.isTaskActive = GT_TRUE;

    rc = prvAppDemoIpfixFwPktUdbOffsetsBuild(devNum);
    if(rc != GT_OK)
    {
        __IPFIX_FW_LOG("[Error]: prvAppDemoIpfixFwPktUdbOffsetsBuild failed: rc = %d\n", rc);
    }

    while (appIpfixFwDbPtr->threads.newFlows.taskTerminate == GT_FALSE)
    {
        /* get the packet from the device */
        numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
        rc = cpssDxChNetIfSdmaRxPacketGet(devNum, sdmaQNum,
                                          &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr, &rxParams);
        if (rc != GT_OK)
        {
            if (rc != GT_NO_MORE)
            {
                __IPFIX_FW_LOG("cpssDxChNetIfSdmaRxPacketGet failed: rc = %d\n", rc);
            }
            continue;
        }
        __IPFIX_FW_DBG_COUNTER_INCREMENT(firstPktsFetchedByHost);

        if((appIpfixFwDbPtr->mgrDb.tmpDbgBmp & 0x1) == 0)
        {
            portNum = (GT_PHYSICAL_PORT_NUM)rxParams.dsaParam.dsaInfo.toCpu.interface.portNum;
            /* Get Port Group Id from port number */
            rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChPortPhysicalPortDetailedMapGet failed: rc=%d\n", rc);
                continue;
            }
            if(portMapShadow.valid)
            {
                /* global pipeId (not relative to the tileId) */
                rc = prvCpssDxChHwPpGopGlobalMacPortNumToLocalMacPortInPipeConvert(devNum,
                                portMapShadow.portMap.macNum/*global MAC port*/,
                                &portGroupId,
                                NULL);/*local MAC*/
            }

            if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow ==
                    CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
            {
                portGroupId &= 0x6;
            }
        }

        /* flow key get */
        cpssOsMemSet(&flow, 0, sizeof(flow));
        if((appIpfixFwDbPtr->mgrDb.tmpDbgBmp & 0x2) == 0)
        {
            rc = prvAppDemoIpfixFwFlowKeyGet((GT_U8 **)packetBuffs,
                                                  buffLenArr, &rxParams, &flow);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("prvAppDemoIpfixFwFlowKeyGet failed: rc = %d\n", rc);
                continue;
            }
        }
        if((appIpfixFwDbPtr->mgrDb.tmpDbgBmp & 0x4) == 0)
        {
            /* Check if the key already exists */
            rc = prvAppDemoIpfixFwFlowKeySearch(devNum, portGroupId, &flow, &isFound);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("prvAppDemoIpfixFwFlowKeySearch failed: rc = %d\n", rc);
                continue;
            }
        }

        if (isFound == GT_FALSE)
        {
            if((appIpfixFwDbPtr->mgrDb.tmpDbgBmp & 0x8) == 0)
            {
                rc = prvAppDemoIpfixFwFreeFlowIdGet(devNum, portGroupId, &flowId);
                if (rc != GT_OK)
                {
                    __IPFIX_FW_LOG("prvAppDemoIpfixFwFreeFlowIdGet failed: rc = %d\n", rc);
                    continue;
                }
                if (PRV_APP_IPFIX_FW_VALID_FLOW_ID_CONDITION(devNum, flowId))
                {
                    __IPFIX_FW_LOG("FlowId=%d for portGroupId=%d out of range. Max = %d\n",
                                   flowId, portGroupId,
                                   appIpfixFwDbPtr->fwCfg.portGroupCfg.secondPolicerMaxIpfixIndex[portGroupId/2]);

                    /* Delete ipfix manager entry */
                    rc = cpssDxChIpfixFwEntryDelete(devNum, sCpuNum, flowId);
                    if (rc != GT_OK)
                    {
                        __IPFIX_FW_LOG("cpssDxChIpfixFwEntryDelete failed: rc = %d\n", rc);
                    }

                    rc2 = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
                    if (rc2 != GT_OK)
                    {
                        __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc2);
                    }

                    continue;
                }
            }

            if((appIpfixFwDbPtr->mgrDb.tmpDbgBmp & 0x10) == 0)
            {
                /* Add EMM Rule */
                /* Enable Cuckoo algorithm in case of collision */
                cpssOsMemSet(&emmEntryParams, 0, sizeof(emmEntryParams));
                emmEntryParams.rehashEnable = GT_TRUE;

                cpssOsMemSet(&emmEntry, 0, sizeof(emmEntry));
                cpssOsMemCpy(&emmEntry.exactMatchEntry.key, &flow.key, sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
                emmEntry.exactMatchEntry.lookupNum = appIpfixFwDbPtr->initCfg.emmInitParams.lookupNum;
                emmEntry.exactMatchActionType = appIpfixFwDbPtr->initCfg.emmInitParams.actionType;
                emmEntry.expandedActionIndex = appIpfixFwDbPtr->initCfg.emmInitParams.expandedActionIndex;

                cpssOsMemCpy(&emmEntry.exactMatchAction,
                            &appIpfixFwDbPtr->initCfg.emmInitParams.action,
                             sizeof(CPSS_DXCH_EXACT_MATCH_ACTION_UNT));

                emmEntry.exactMatchAction.pclAction.flowId  = (flowId & 0xFFF);
                emmEntry.exactMatchAction.pclAction.policer.policerEnable  = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
                emmEntry.exactMatchAction.pclAction.policer.policerId = (flowId & 0xFFF);

                rc = cpssDxChExactMatchManagerEntryAdd(appIpfixFwDbPtr->mgrDb.exactMatchManagerId[portGroupId/2],
                                                       &emmEntry, &emmEntryParams);
                if (rc != GT_OK)
                {
                    __IPFIX_FW_LOG("cpssDxChExactMatchManagerEntryAdd failed: rc = %d\n", rc);

                    __IPFIX_FW_DBG_COUNTER_INCREMENT(emEntryAddFails);

                    rc2 = cpssDxChIpfixFwEntryDelete(devNum, sCpuNum, flowId);
                    if (rc2 != GT_OK)
                    {
                        __IPFIX_FW_LOG("cpssDxChIpfixFwEntryDelete failed: rc = %d\n", rc);
                    }

                    rc2 = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
                    if (rc2 != GT_OK)
                    {
                        __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc2);
                    }

                    continue;
                }
            }

            if((appIpfixFwDbPtr->mgrDb.tmpDbgBmp & 0x20) == 0)
            {
                /* Add IPFIX entry */
                ipfixEntryParams.firstTsValid = GT_TRUE;
                ipfixEntryParams.firstTs = (GT_U16) rxParams.dsaParam.dsaInfo.toCpu.timestamp;
                rc = cpssDxChIpfixFwEntryAdd(devNum, sCpuNum, flowId, &ipfixEntryParams);

                if (rc != GT_OK)
                {
                    __IPFIX_FW_LOG("cpssDxChIpfixFwEntryAdd failed: rc = %d\n", rc);

                    rc2 = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
                    if (rc2 != GT_OK)
                    {
                        __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc2);
                    }

                    continue;
                }
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_ADD_E]);
                __IPFIX_FW_DBG_COUNTER_INCREMENT(activeFlows);

                flow.info.isActive = GT_TRUE;
                flow.info.flowId = ((portGroupId & 0x7) << 12) | (flowId & 0xFFF);
                flow.dataRecord.flowId = flow.info.flowId;

                /* Update the local database */
                cpssOsMemCpy(&appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)], &flow,
                            sizeof(flow));

                if(appIpfixFwDbPtr->mgrDb.keysLookupAlgo == PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_HASH_E)
                {
                    rc = prvAppDemoIpfixFwKeysDbEntryAdd(devNum, portGroupId, &flow.key);
                    if (rc != GT_OK)
                    {
                        __IPFIX_FW_LOG("prvAppIpfixFwKeysDbEntryAdd failed: rc = %d\n", rc);

                        rc2 = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
                        if (rc2 != GT_OK)
                        {
                            __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc2);
                        }
                        continue;
                    }
                }
            }
        }
        else
        {
            __IPFIX_FW_DBG_COUNTER_INCREMENT(firstPktsFilteredByHost);
        }

        rc = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc);
        }
    }

    appIpfixFwDbPtr->threads.newFlows.taskTerminate = GT_TRUE;
    appIpfixFwDbPtr->threads.newFlows.isTaskActive = GT_FALSE;

    /* to avoid compiler warnings */
    return 0;
}

/**
 * @internal  prvAppDemoIpfixFwFlowsTerminate funtion
 * @endinternal
 *
 * @brief  Task to handle flow termination
 *
 * @param[in] arg  - input arguments for the task. It includes device number
 *
 * @retval unsigned __TASKCONV
 */
static unsigned __TASKCONV prvAppDemoIpfixFwFlowsTerminate
(
    GT_VOID *arg
)
{
    GT_STATUS                            rc = GT_OK;
    GT_U32                                devNum = 0;
    GT_U32                               sCpuNum = 0;
    GT_U32                               numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
    GT_U8                                *packetBuffs[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    GT_U32                               buffLenArr[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC          rxParams;
    GT_BOOL                              isFound;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC  flow;
    GT_U32                               flowsTermCount = 0;
    GT_U32                               flowId = 0;
    GT_U8                                sdmaQNum;

    (void) arg;
    devNum = appIpfixFwDbPtr->threads.flowsTerm.devNum;
    sCpuNum = appIpfixFwDbPtr->threads.flowsTerm.sCpuNum;
    sdmaQNum = appIpfixFwDbPtr->initCfg.txInitParams.synPktsQueueNum;

    if (appIpfixFwDbPtr->threads.flowsTerm.isTaskActive == GT_TRUE)
    {
        return GT_BAD_STATE;
    }

    appIpfixFwDbPtr->threads.flowsTerm.taskTerminate = GT_FALSE;
    appIpfixFwDbPtr->threads.flowsTerm.isTaskActive = GT_TRUE;

    flowsTermCount = 0;
    while (appIpfixFwDbPtr->threads.flowsTerm.taskTerminate == GT_FALSE)
    {
        /* get the packet from the device */
        if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
        {
            numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
            rc = cpssDxChNetIfSdmaRxPacketGet(devNum, sdmaQNum,
                                              &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr, &rxParams);
            if (rc != GT_OK)
            {
                if (rc != GT_NO_MORE)
                {
                    __IPFIX_FW_LOG("cpssDxChNetIfSdmaRxPacketGet failed: rc = %d\n", rc);
                }
                continue;
            }
        }
        else if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
        {
            rc = cpssDxChNetIfMiiRxPacketGet(devNum, sdmaQNum,
                                             &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr,&rxParams);
            if (rc != 0)
            {
                __IPFIX_FW_LOG("cpssDxChNetIfMiiRxPacketGet failed: rc = %d\n", rc);
                continue;
            }
        }
        else
        {
            __IPFIX_FW_LOG("Invalid CPU port mode\n");
            rc = GT_NOT_SUPPORTED;
            break;
        }

        __IPFIX_FW_DBG_COUNTER_INCREMENT(synPktsFetchedByHost);

        /* flow key get */
        cpssOsMemSet(&flow, 0, sizeof(flow));
        rc = prvAppDemoIpfixFwFlowKeyGet((GT_U8 **)packetBuffs,
                                              buffLenArr, &rxParams, &flow);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("prvAppDemoIpfixFwFlowKeyGet failed: rc = %d\n", rc);
            continue;
        }

        /* Check if the key already exists */
        rc = prvAppDemoIpfixFwFlowKeySearch(devNum, ((flowId >> 12) & 0x7), &flow, &isFound);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("prvAppDemoIpfixFwFlowKeySearch failed: rc = %d\n", rc);
            continue;
        }

        if (isFound == GT_TRUE)
        {
            /* Delete Ipfix Entry */
            rc = cpssDxChIpfixFwEntryDelete(devNum, sCpuNum, flowId);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChIpfixFwEntryDelete failed: rc = %d\n", rc);
                continue;
            }

            /* [TBD] Check IPC response */

            __IPFIX_FW_DBG_COUNTER_DECREMENT(activeFlows);

            appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].info.isActive = GT_FALSE;

        }
        else
        {
            /* [TBD] */
            __IPFIX_FW_DBG_COUNTER_INCREMENT(synPktsFilteredByHost);
        }

        rc = prvAppDemoIpfixFwSdmaRxBufFree(devNum, sdmaQNum, numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("RX Buff Free failed: rc=%d\n",rc);
        }

        flowsTermCount++;
        if (flowsTermCount == appIpfixFwDbPtr->threads.flowsTerm.burstSize)
        {
            flowsTermCount = 0;
            /* [TBD] Stop after adding 1K new flows */
        }
    }

    appIpfixFwDbPtr->threads.flowsTerm.taskTerminate = GT_TRUE;
    appIpfixFwDbPtr->threads.flowsTerm.isTaskActive = GT_FALSE;

    /* to avoid compiler warnings */
    return 0;
}

/**
 * @internal prvAppDemoIpfixFwBitsGet function
 * @endinternal
 *
 * @brief Get bits from input stream
 *
 * @param[in] data    - (pointer to) input data stream
 * @param[in] offset  - start bit offset
 * @param[in] numBits - size in bits to read
 *
 * @retval Extracted value from the data stream
 */
static GT_U32 prvAppDemoIpfixFwBitsGet
(
    IN  GT_U8    *data,
    IN  GT_U32   offset,
    IN  GT_U32   numBits
)
{
    GT_U32 ii, byte, bitShift, outData=0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        outData  |= ((data[byte] >> bitShift) & 1) << ii;
    }
    return outData;
}

/**
 * @internal  prvAppDemoIpfixFwDataRecordGet function
 * @endinternal
 *
 * @brief   Parse data records from the data packet
 *
 * @param[in]   pktBuffsArr   -  packet buffers array.
 * @param[in]   buffLen       -  buffer length
 * @param[in]   dataPktPtr    -  pointer to data packet
 *
 * @return GT_OK  -  on success.
 */
static GT_STATUS prvAppDemoIpfixFwDataRecordGet
(
    IN GT_U8    **pktBuffsArr,
    IN GT_U32   buffLen,
    OUT PRV_APP_IPFIX_FW_DATA_PKT_STC   *dataPktPtr
)
{
    GT_U8       *pktBuff = *pktBuffsArr;
    GT_U32      startOffset = 0;
    GT_U32      dataRecordSize = 28; /*in bytes*/
    GT_U32      l2HeaderSize = 12;
    GT_U32      headerSize   = 16; /*in bytes*/
    PRV_APP_IPFIX_FW_DATA_RECORD_STC *dataRecordPtr;
    PRV_APP_IPFIX_FW_DATA_PKT_HEADER_STC *headerPtr;
    GT_U32      i = 0 ;

    if (buffLen < 16)
    {
        return GT_OUT_OF_RANGE;
    }

    /* IPFIX Data Packet Format: DMAC(6B)|SMAC(6B)|eDSA(16B)|IPFIX Packet Header(16B)|Data(24B)
     * Header 16B
     * PacketType(1B)|TODin4msunits(7B)|SequenceNumber(2B)|ExportedEntriesNumber(2B)|FirstFlowId(2B)|LastFlowId(2B)
     * Data 24B
     * FlowId(2B)|FirstTs(20b)|LastTs(20b)|PacketCount(40b)|DropCount(40b)|ByteCount(46b)|EntryStatus(3b)|Padding(7b)
     */

    headerPtr = &dataPktPtr->header;

    /* packet type (8 bits) */
    headerPtr->packetType = (GT_U32) pktBuff[l2HeaderSize];

    /* time of the day sampled while forming the packet (7 bytes) */
    startOffset = l2HeaderSize*8 + 8;
    headerPtr->tod[0] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 32);
    headerPtr->tod[1] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset+32, 24);

    /* sequence number (16 bits) */
    startOffset += 56;
    headerPtr->seqNum = (GT_U16) prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 16);

    /* number of data records exported (16 bits) */
    startOffset += 16;
    headerPtr->numExported = (GT_U16) prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 16);

    /* flowId of the first data record exported (16 bits) */
    startOffset += 16;
    headerPtr->firstFlowId = (GT_U16) prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 16);

    /* flowId of the lat data record exported (16 bits) */
    startOffset += 16;
    headerPtr->lastFlowId = (GT_U16) prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 16);

    if (buffLen < (GT_U32)(16 + headerPtr->numExported * dataRecordSize))
    {
        return GT_OUT_OF_RANGE;
    }

    for (i=0; i<headerPtr->numExported; i++)
    {
        dataRecordPtr = &dataPktPtr->dataRecord[i];

        /* flow identifier (16 bits) */
        startOffset = 8 * (l2HeaderSize + headerSize + i*dataRecordSize);
        dataRecordPtr->flowId  = (GT_U16) prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 16);

        /* Time Stamp of the first packet in the flow (20 bits)*/
        startOffset += 16;
        dataRecordPtr->firstTs = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 20);

        /* Time stamp of the last packet in the flow (20 bits) */
        startOffset += 20;
        dataRecordPtr->lastTs  = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 20);

        /* packet count (40 bits) */
        startOffset += 20;
        dataRecordPtr->packetCount[0] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 32);
        dataRecordPtr->packetCount[1] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset + 32, 8);

        /* drop count (40 bits) */
        startOffset += 40;
        dataRecordPtr->dropCount[0] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 32);
        dataRecordPtr->dropCount[1] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset + 32, 8);

        /* byte count (46 bits)*/
        startOffset += 40;
        dataRecordPtr->byteCount[0] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 14);
        dataRecordPtr->byteCount[1] = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset + 14, 32);

        /* entry status (3 bits) */
        startOffset += 46;
        dataRecordPtr->entryStatus = prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 3);

        startOffset += 3;
        if (appIpfixFwDbPtr->fwCfg.exportCfg.exportLastCpuCode)
        {
            /* cpu/drop code (8 bits) */
            dataRecordPtr->lastCpuCode = (CPSS_NET_RX_CPU_CODE_ENT) prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 8);
            startOffset += 8;
        }

        if (appIpfixFwDbPtr->fwCfg.exportCfg.exportLastPktCmd)
        {
            /* packet command (3 bits) */
            dataRecordPtr->lastPktCmd = (CPSS_PACKET_CMD_ENT)  prvAppDemoIpfixFwBitsGet(pktBuff, startOffset, 3);
            startOffset += 3;
        }

        /* [TBD] Handle padding and variable length data packet */

    }

    return GT_OK;
}

/**
 * @internal   prvAppDemoIpfixFwFlowDbStatsUpdate function
 * @endinternal
 *
 * @brief   Update statistics of a flow in aplication flows database
 *
 * @param[in] dataPktPtr  - (pointer to) data packets pointer
 *
 * @return GT_OK   -  on success.
 */
static GT_STATUS prvAppDemoIpfixFwFlowDbStatsUpdate
(
    IN  GT_U8                                  devNum,
    IN  PRV_APP_IPFIX_FW_DATA_PKT_STC   *dataPktPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      flowId;
    PRV_APP_IPFIX_FW_DATA_RECORD_STC   *dbDataRecordPtr;
    GT_U32      tempVal;

    for (i=0; i<dataPktPtr->header.numExported; i++)
    {
        flowId = dataPktPtr->dataRecord[i].flowId;
        if (flowId >= PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum))
        {
            __IPFIX_FW_LOG("flowId: %d exceeds limit %d\n", flowId, PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum));
            return GT_OUT_OF_RANGE;
        }
        dbDataRecordPtr = &appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].dataRecord;
        dbDataRecordPtr->flowId = dataPktPtr->dataRecord[i].flowId;
        dbDataRecordPtr->firstTs = dataPktPtr->dataRecord[i].firstTs;
        dbDataRecordPtr->lastTs = dataPktPtr->dataRecord[i].lastTs;
        dbDataRecordPtr->entryStatus = dataPktPtr->dataRecord[i].entryStatus;
        dbDataRecordPtr->lastCpuCode = dataPktPtr->dataRecord[i].lastCpuCode;
        dbDataRecordPtr->lastPktCmd = dataPktPtr->dataRecord[i].lastPktCmd;

        /* Increment packet count */
        tempVal = dbDataRecordPtr->packetCount[0];
        dbDataRecordPtr->packetCount[0] += dataPktPtr->dataRecord[i].packetCount[0];
        dbDataRecordPtr->packetCount[1] += dataPktPtr->dataRecord[i].packetCount[1];
        if (tempVal > dbDataRecordPtr->packetCount[0])
        {
            dbDataRecordPtr->packetCount[1] += 1; /*Add overflow*/
        }

        /* Increment byte count */
        tempVal = dbDataRecordPtr->byteCount[0];
        dbDataRecordPtr->byteCount[0] += dataPktPtr->dataRecord[i].byteCount[0];
        dbDataRecordPtr->byteCount[1] += dataPktPtr->dataRecord[i].byteCount[1];
        if (tempVal > dbDataRecordPtr->byteCount[0])
        {
            dbDataRecordPtr->byteCount[1] += 1; /*Add overflow*/
        }

        /* Increment drop count */
        tempVal = dbDataRecordPtr->dropCount[0];
        dbDataRecordPtr->dropCount[0] += dataPktPtr->dataRecord[i].dropCount[0];
        dbDataRecordPtr->dropCount[1] += dataPktPtr->dataRecord[i].dropCount[1];
        if (tempVal > dbDataRecordPtr->dropCount[0])
        {
            dbDataRecordPtr->dropCount[1] += 1; /*Add overflow*/
        }

        __IPFIX_FW_VAL_COMPARE(flowId, dbDataRecordPtr->flowId, rc, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }

    }

    return rc;
}

#if 0
static GT_BOOL prvAppDemoIpfixFwIsTimedOut
(
    IN GT_U32   firstTsSec,
    IN GT_U32   firstTsNanoSec,
    IN GT_U32   lastTsSec,
    IN GT_U32   lastTsNanoSec,
    IN GT_U32   timeOutInSec
)
{
    GT_U32              ageInSeconds = 0;
    GT_U32              nanoSeconds = 0;

    if (lastTsSec >= firstTsSec)
    {
        ageInSeconds = lastTsSec - firstTsSec;
    }
    else
    {
        ageInSeconds = 0xFF - (firstTsSec - lastTsSec);
    }

    if(lastTsNanoSec >= firstTsNanoSec)
    {
        nanoSeconds = lastTsNanoSec - firstTsNanoSec;
    }
    else
    {
        nanoSeconds = 0xFF - (firstTsNanoSec -  lastTsNanoSec);
    }

    if ((nanoSeconds + firstTsNanoSec) > 0xFF)
    {
        ageInSeconds++;
    }

    if (ageInSeconds >= timeOutInSec)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
 * @internal   prvAppDemoIpfixFwAgedOutCheck function
 * @endinternal
 *
 * @brief   Check whether a particular flow is aged out.
 *
 * @param[in] portGroupId  - port group identifier
 * @param[in] flowId       - flow identifier
 *
 * @return GT_TRUE   -  if the flow is aged out.
 *         GT_FALSE  -  otherwise
 */
static GT_BOOL prvAppDemoIpfixFwAgedOutCheck
(
    IN  GT_U32  portGroupId,
    IN  GT_U32  flowId,
    IN  GT_U32  tod,
    OUT GT_BOOL *isIdleTimeOutPtr
)
{
    PRV_APP_IPFIX_FW_DATA_RECORD_STC *dataRecordPtr;
    GT_U32              firstTsNanoSec = 0;
    GT_U32              lastTsNanoSec = 0;
    GT_U32              firstTsSec = 0;
    GT_U32              lastTsSec = 0;
    GT_U32              timeoutInSec = 0;
    GT_BOOL             isTimedOut;

    if(appIpfixFwDbPtr->flowsDb[portGroupId][flowId].info.isActive == GT_FALSE)
    {
        __IPFIX_FW_LOG("Flow: {%d %d} is not active\n"
                           , portGroupId, flowId);
        return GT_FALSE;
    }

    dataRecordPtr  = &appIpfixFwDbPtr->flowsDb[portGroupId][flowId].data;

    if (dataRecordPtr == NULL)
    {
        __IPFIX_FW_LOG("dataRecord for portGroupId:%d and flowId: %d is NULL\n",
                            portGroupId, flowId);
        *isIdleTimeOutPtr = GT_TRUE;
        return GT_FALSE;
    }

    if((dataRecordPtr->lastTs == 0) || (dataRecordPtr->firstTs == 0) ||
       (tod == 0))
    {
        return GT_FALSE;
    }

    /* check idle timeout. TOD - LastTs >= idleTimeout */
    firstTsSec     = (dataRecordPtr->lastTs >> 8) & 0xFF;
    firstTsNanoSec = dataRecordPtr->lastTs & 0xFF;
    lastTsSec      = (tod >> 8) & 0xFF;
    lastTsNanoSec  = tod & 0xFF;
    timeoutInSec   = appIpfixFwDbPtr->testParams.idleTimeout;
    isTimedOut = prvAppDemoIpfixFwIsTimedOut(firstTsSec, firstTsNanoSec, lastTsSec,
                                     lastTsNanoSec, timeoutInSec);
    *isIdleTimeOutPtr = GT_TRUE;
    if (isTimedOut == GT_TRUE)
    {
        __IPFIX_FW_DBG_LOG("Flow: {%d, %d} IdleAgedOut. TOD: (%X,%X), LastTs: (%X,%X)\n",
                            portGroupId, flowId, lastTsSec, lastTsNanoSec,
                            firstTsSec, firstTsNanoSec);
        return GT_TRUE;
    }

    /* check active timeout. LastTs - FirstTs >= activeTimeOut */
    firstTsSec     = (dataRecordPtr->firstTs >> 8) & 0xFF;
    firstTsNanoSec = dataRecordPtr->firstTs & 0xFF;
    lastTsSec      = (dataRecordPtr->lastTs >> 8) & 0xFF;
    lastTsNanoSec  = dataRecordPtr->lastTs & 0xFF;
    timeoutInSec   = appIpfixFwDbPtr->testParams.activeTimeout;
    isTimedOut = prvAppDemoIpfixFwIsTimedOut(firstTsSec, firstTsNanoSec, lastTsSec,
                                     lastTsNanoSec, timeoutInSec);
    *isIdleTimeOutPtr = GT_FALSE;
    if (isTimedOut == GT_TRUE)
    {
        __IPFIX_FW_DBG_LOG("Flow: {%d, %d} ActiveAgedOut. LastTs: (%X,%X), FirstTs: (%X,%X)\n",
                            portGroupId, flowId, lastTsSec, lastTsNanoSec,
                            firstTsSec, firstTsNanoSec);
        return GT_TRUE;
    }

    return GT_FALSE;
}
#endif

/**
 * @internal prvAppDemoIpfixFwDataPktsCollect function
 * @endinternal
 *
 * @brief   This task periodically reads data pkts queue and updates
 *          the application IPFIX database
 *
 * @param[in] arg  - input arguments for the task.
 *                   includes device number
 *
 * @return handle to the task
 */
static unsigned __TASKCONV prvAppDemoIpfixFwDataPktsCollect
(
    GT_VOID *arg
)
{
    GT_STATUS rc = GT_OK;
    PRV_APP_IPFIX_FW_DATA_PKT_STC  dataPkt;
    GT_U32                          dataRecordCount = 0;
    GT_U8                           devNum = appIpfixFwDbPtr->threads.dataPkts.devNum;
    GT_U32                          numOfBuffs;
    GT_U8                           *packetBuffs[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    GT_U32                          buffLenArr[PRV_APP_IPFIX_FW_PKT_BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC     rxParams;
    PRV_APP_IPFIX_FW_DATA_RECORD_STC dataRecord;
    GT_U8                           sdmaQNum;
    GT_U32                          prevExportCompleteMsgs = 0;
    GT_U32                          maxFlows = CPSS_DXCH_IPFIX_FW_MAX_FLOWS_1K_E;
    GT_U32                          delayCount = 0;
    GT_BOOL                         startPeriodicMode = GT_TRUE;

    (void) arg;
    sdmaQNum = appIpfixFwDbPtr->fwCfg.exportCfg.localQueueNum;

    appIpfixFwDbPtr->threads.dataPkts.taskTerminate = GT_FALSE;
    appIpfixFwDbPtr->threads.dataPkts.isTaskActive = GT_TRUE;

    dataRecordCount = 0;
    while (appIpfixFwDbPtr->threads.dataPkts.taskTerminate == GT_FALSE)
    {
        cpssOsTimerWkAfter(appIpfixFwDbPtr->testParams.dataPktsPollInterval);
        if(appIpfixFwDbPtr->threads.dataPkts.taskTerminate == GT_TRUE)
        {
            break;
        }

        /* In Periodic mode, send a single DATA GET ALL request to start receiving periodic data packets.
           In Non-Periodic mode, send DATA_GET_ALL request regularly to fetch data packets */
        if ((appIpfixFwDbPtr->fwCfg.exportCfg.periodicExportEnable == GT_FALSE) ||
            (startPeriodicMode == GT_TRUE))
        {
            prevExportCompleteMsgs = appIpfixFwDbPtr->dbg.dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E];
            rc = cpssDxChIpfixFwDataGetAll(appIpfixFwDbPtr->threads.dataPkts.devNum,
                                           appIpfixFwDbPtr->threads.dataPkts.sCpuNum,
                                           CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E, maxFlows);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("cpssDxChIpfixFwDataGetAll failed: rc = %d\n", rc);
                continue;
            }

            __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E]);

            if (startPeriodicMode == GT_TRUE)
            {
                startPeriodicMode = GT_FALSE;
            }
        }

        /*cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_TIME_AFTER_DATA_GET_ALL_CNS);*/
        while (1) {
            /* get the packet from the device */
            if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            {
                numOfBuffs = PRV_APP_IPFIX_FW_PKT_BUFF_LEN;
                rc = cpssDxChNetIfSdmaRxPacketGet(devNum,
                                                  sdmaQNum,
                                                  &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr, &rxParams);
                if (rc != 0)
                {
                    if (rc != GT_NO_MORE)
                    {
                        __IPFIX_FW_LOG("cpssDxChNetIfSdmaRxPacketGet failed: rc = %d\n", rc);
                    }
                    else
                    {
                        rc = GT_OK;
                    }

                    break;
                }
            }
            else if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
            {
                rc = cpssDxChNetIfMiiRxPacketGet(devNum,sdmaQNum,
                                                 &numOfBuffs, (GT_U8 **)packetBuffs, buffLenArr,&rxParams);
                if (rc != 0)
                {
                    __IPFIX_FW_LOG("cpssDxChNetIfMiiRxPacketGet failed: rc = %d\n", rc);
                    break;
                }
            }
            else
            {
                __IPFIX_FW_LOG("Invalid CPU port mode\n");
                rc = GT_BAD_STATE;
                break;
            }

            __IPFIX_FW_DBG_COUNTER_INCREMENT(dataPktsFetchedByHost);

            /* flow key get */
            cpssOsMemSet(&dataRecord, 0, sizeof(dataRecord));
            rc = prvAppDemoIpfixFwDataRecordGet((GT_U8 **)packetBuffs, buffLenArr[0],
                                                     &dataPkt);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("prvAppDemoIpfixFwDataRecordGet failed: rc = %d\n", rc);

                __IPFIX_FW_DBG_COUNTER_INCREMENT(dataPktsDrops);

                break;
            }

            if (appIpfixFwDbPtr->dbg.cpuRxDumpEnable == GT_TRUE)
            {
                /* Print parsed data packet prints */
                appDemoIpfixFwDataRecordDump(&dataPkt);
            }

            /* Check if the key already exists and update the stats */
            rc = prvAppDemoIpfixFwFlowDbStatsUpdate(devNum, &dataPkt);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("prvAppDemoIpfixFwFlowDbStatsUpdate failed: rc = %d\n", rc);
                break;
            }

            if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
            {
                /* now you need to free the buffers */
                if(PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.allocMethod[sdmaQNum]!= CPSS_RX_BUFF_NO_ALLOC_E)
                {
                    rc = cpssDxChNetIfRxBufFree(devNum,sdmaQNum,
                                                (GT_U8 **)packetBuffs,numOfBuffs);
                }
                else
                {
                    /* move the first buffer pointer to the original place*/
                    packetBuffs[0] = (GT_U8*)((((GT_UINTPTR)(packetBuffs[0])) >> APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF)
                                              << APP_DEMO_RX_BUFF_ALLIGN_BITS_DEF);

                    /* fix the last buffer's size*/
                    buffLenArr[numOfBuffs-1] = APP_DEMO_RX_BUFF_SIZE_DEF;

                    rc = cpssDxChNetIfRxBufFreeWithSize(devNum,sdmaQNum,
                                                        (GT_U8 **)packetBuffs,buffLenArr,numOfBuffs);
                }
            }
            else /* CPSS_NET_CPU_PORT_MODE_MII_E */
            {
                rc = cpssDxChNetIfMiiRxBufFree(devNum,sdmaQNum,
                                               (GT_U8 **)packetBuffs,numOfBuffs);
            }

            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("RX Buff Free failed\n");
                break;
            }
        }

        delayCount = 0;
        while((prevExportCompleteMsgs ==
               appIpfixFwDbPtr->dbg.dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E]) &&
              (appIpfixFwDbPtr->fwCfg.exportCfg.periodicExportEnable == GT_FALSE))
        {
            delayCount++;
            cpssOsTimerWkAfter(1);
            if (delayCount >= PRV_APP_IPFIX_FW_WAIT_FOR_EXPORT_COMPLETE_CNS)
            {
    #ifndef ASIC_SIMULATION
                __IPFIX_FW_LOG("Warning: ExportCompleteMsg not received yet\n");
    #endif
                break;
            }
        }

        if (rc != GT_OK)
        {
            continue;
        }

        dataRecordCount ++;
        if (dataRecordCount == appIpfixFwDbPtr->threads.dataPkts.burstSize)
        {
            dataRecordCount = 0;
            /* [TBD] Stop after handling 1K data packets */
        }
    }

    appIpfixFwDbPtr->threads.dataPkts.taskTerminate = GT_TRUE;
    appIpfixFwDbPtr->threads.dataPkts.isTaskActive = GT_FALSE;

    /* to avoid compiler warnings */
    return 0;
}

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
 *                         Bit 3 - portGroupCfg
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
)
{
    GT_STATUS       rc = GT_OK;
    GT_U32          numMsgs = 0;
    GT_U32          numMsgsFetched = 0;
    GT_U32          pendingMsgBuffSize = 0;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT  ipcMsgArr[4];
    GT_U32          i;
    GT_U32          globalCfgReceived = GT_FALSE;
    GT_U32          exportCfgReceived = GT_FALSE;
    GT_U32          elephantCfgReceived = GT_FALSE;
    GT_U32          portGroupCfgReceived = GT_FALSE;
    GT_32           ret = 0;

    (void) enablePrints;
    if (configBmp & 0x1)
    {
        rc = cpssDxChIpfixFwGlobalConfigGet(devNum, sCpuNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwGlobalConfigGet failed: rc=%d\n", rc);
            return rc;
        }
        __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E]);
        numMsgs++;
    }
    if (configBmp & 0x2)
    {
        rc = cpssDxChIpfixFwExportConfigGet(devNum, sCpuNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwExportConfigGet failed: rc=%d\n", rc);
            return rc;
        }
        __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E]);
        numMsgs++;
    }
    if (configBmp & 0x4)
    {
        rc = cpssDxChIpfixFwElephantConfigGet(devNum, sCpuNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwElephantConfigGet failed: rc=%d\n", rc);
            return rc;
        }
        __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E]);
        numMsgs++;
    }
    if ((configBmp & 0x8) && (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E))
    {
        rc = cpssDxChIpfixFwPortGroupConfigGet(devNum, sCpuNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwPortGroupConfigGet failed: rc=%d\n", rc);
            return rc;
        }
        __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E]);
        numMsgs++;
    }

    cpssOsTimerWkAfter(100);

    cpssOsMemSet(ipcMsgArr, 0, sizeof(ipcMsgArr));
    for (i=0; i<numMsgs; i++)
    {
        rc = cpssDxChIpfixFwIpcMsgFetch(devNum, sCpuNum, 1, &numMsgsFetched,
                                        &pendingMsgBuffSize, &ipcMsgArr[i]);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwIpcMsgFetch failed: rc=%d\n", rc);
            return rc;
        }
    }

    numMsgsFetched = i;
    if (numMsgs != numMsgsFetched)
    {
        __IPFIX_FW_LOG("Fetched %d IPC messages but expected %d\n", numMsgsFetched, numMsgs);
    }

    if (updateOrVerify == GT_TRUE)
    {
        for (i=0; i<numMsgsFetched; i++)
        {
            /* update app data base */
            switch (ipcMsgArr[i].opcodeGet.opcode)
            {
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E:
                if (globalCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one global config return found\n");
                    continue;
                }
                cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.globalCfg, &ipcMsgArr[i].globalConfigReturn.globalCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC));
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E]);
                break;
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E:
                if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
                {
                    __IPFIX_FW_LOG("Warn: Illegal IPC message opCode:%d\n", ipcMsgArr[i].opcodeGet.opcode);
                    rc = GT_BAD_PARAM;
                    break;
                }
                if (portGroupCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one port group config return found\n");
                    continue;
                }
                cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.portGroupCfg, &ipcMsgArr[i].portGroupConfigReturn.portGroupCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC));
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E]);
                break;
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E:
                if (exportCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one export config return found\n");
                    continue;
                }
                cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.exportCfg, &ipcMsgArr[i].exportConfigReturn.exportCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC));
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E]);
                break;
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E:
                if (elephantCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one elephant config return found\n");
                    continue;
                }
                cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.elephantCfg, &ipcMsgArr[i].elephantConfigReturn.elephantCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC));
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E]);
                break;
            default:
                __IPFIX_FW_LOG("Warn: Illegal IPC message opCode:%d\n", ipcMsgArr[i].opcodeGet.opcode);
                rc = GT_BAD_PARAM;
                break;
            }
        }
    }
    else
    {
         for (i=0; i<numMsgsFetched; i++)
        {
            /* update app data base */
            switch (ipcMsgArr[0].opcodeGet.opcode)
            {
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E:
                if (globalCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one global config return found\n");
                    continue;
                }
                ret = cpssOsMemCmp(&appIpfixFwDbPtr->fwCfg.globalCfg, &ipcMsgArr[i].globalConfigReturn.globalCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC));
                if (ret != 0)
                {
                    __IPFIX_FW_LOG("Warn: Global config is not in sync\n");
                }
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E]);
                break;
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E:
                if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
                {
                    __IPFIX_FW_LOG("Warn: Illegal IPC message opCode:%d\n", ipcMsgArr[i].opcodeGet.opcode);
                    rc = GT_BAD_PARAM;
                    break;
                }
                if (portGroupCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one port group config return found\n");
                    continue;
                }
                ret = cpssOsMemCmp(&appIpfixFwDbPtr->fwCfg.portGroupCfg, &ipcMsgArr[i].portGroupConfigReturn.portGroupCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC));
                if (ret != 0)
                {
                    __IPFIX_FW_LOG("Warn: port group config is not in sync\n");
                }
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E]);
                break;
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E:
                if (exportCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one export config return found\n");
                    continue;
                }
                cpssOsMemCmp(&appIpfixFwDbPtr->fwCfg.exportCfg, &ipcMsgArr[i].exportConfigReturn.exportCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC));
                if (ret != 0)
                {
                    __IPFIX_FW_LOG("Warn: Export config is not in sync\n");
                }
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E]);
                break;
            case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E:
                if (elephantCfgReceived)
                {
                    __IPFIX_FW_LOG("Warn: More than one elephant config return found\n");
                    continue;
                }
                cpssOsMemCmp(&appIpfixFwDbPtr->fwCfg.elephantCfg, &ipcMsgArr[i].elephantConfigReturn.elephantCfg,
                             sizeof(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC));
                if (ret != 0)
                {
                    __IPFIX_FW_LOG("Warn: Elephant config is not in sync\n");
                }
                __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E]);
                break;
            default:
                __IPFIX_FW_LOG("Warn: Illegal IPC message opCode:%d\n", ipcMsgArr[i].opcodeGet.opcode);
                rc = GT_BAD_PARAM;
                break;
            }
        }
    }

    return rc;
}

/**
 * @internal appDemoIpfixFwIpcEventHandle function
 * @endinternal
 *
 * @brief  Handle a single event.
 *
 * @param[in] devNum    - device number
 * @param[in] ipcMsgPtr - (pointer to) IPC message that we handle.
 *
 * @return GT_OK    -  on success
 */
GT_STATUS appDemoIpfixFwIpcEventHandle
(
    IN  GT_U8                    devNum,
    IN CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT *ipcMsgPtr
)
{
    GT_STATUS                               rc = GT_OK;
    GT_U32                                  flowId;
    GT_U32                                  flowDbId;
    GT_U32                                  i;
    GT_U32                                  numEmBanks = 16;
    GT_U32                                  shift = 0;
    GT_U32                                  value = 0;

    if (ipcMsgPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    cpssOsMemCpy(ipcMsgPtr, ipcMsgPtr, sizeof(CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT));

    /* [TBD] Add logic for using pending termination flag*/

    switch (ipcMsgPtr->opcodeGet.opcode)
    {
    /* Entry Invalidate */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_INVALIDATE_E:
        flowId = ipcMsgPtr->entryInvalidate.flowId;
        flowDbId = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId);
        if (flowId >= PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum))
        {
            __IPFIX_FW_LOG("Invalid flowId: %d\n", flowId);
            return GT_OUT_OF_RANGE;
        }

        rc = cpssDxChIpfixFwEntryDelete(appIpfixFwDbPtr->threads.ipcFetch.devNum,
                                        appIpfixFwDbPtr->threads.ipcFetch.sCpuNum, flowId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwFlowDelete failed: rc = %d\n", rc);
            return rc;
        }
        appIpfixFwDbPtr->flowsDb[flowDbId].info.isActive = GT_FALSE;
        appIpfixFwDbPtr->flowsDb[flowDbId].info.lastEndReason = ipcMsgPtr->entryInvalidate.endReason;
        cpssOsMemSet(&appIpfixFwDbPtr->flowsDb[flowDbId].dataRecord, 0,
                     sizeof(PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC));
        break;

    /* Entry Learned */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_LEARNED_E:
        flowId = ipcMsgPtr->entryLearned.flowId;
        flowDbId = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId);
        if (flowId >= PRV_APP_IPFIX_FW_FLOWS_MAX_CNS(devNum))
        {
            __IPFIX_FW_LOG("Invalid flowId: %d\n", flowId);
            return GT_OUT_OF_RANGE;
        }
        appIpfixFwDbPtr->flowsDb[flowDbId].info.isActive = GT_TRUE;
        appIpfixFwDbPtr->flowsDb[flowDbId].key.keySize   = (CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT) ipcMsgPtr->entryLearned.keySize;
        for (i=0; i<CPSS_DXCH_EXACT_MATCH_MANAGER_MAX_KEY_SIZE_CNS; i++)
        {
            appIpfixFwDbPtr->flowsDb[flowDbId].key.pattern[i] =
                (ipcMsgPtr->entryLearned.flowKey[i/4] >> (8*(i%4)) & 0xFF);
        }
        appIpfixFwDbPtr->flowsDb[flowDbId].info.flowId = flowId;

        if (appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
        {
            if ((value == 3) || (value == 4) || (value == 8) || (value == 11))
            {
                /* [TBD] update for more values */
                numEmBanks = 16;
            }
            else if ((value == 6) || (value == 7))
            {
                /* [TBD] update for more values */
                numEmBanks = 8;
            }
            else
            {
                numEmBanks = 4;
            }
        }
        else
        {
            numEmBanks = 4;
        }

        switch (numEmBanks)
        {
        case 16:
            shift = 4;
            break;
        case 8:
            shift = 3;
            break;
        case 4:
            shift = 2;
            break;
        default:
            __IPFIX_FW_LOG("Invalid numEmBanks: %d\n", numEmBanks);
            rc = GT_BAD_VALUE;
            shift = 0;

        }
        appIpfixFwDbPtr->flowsDb[flowDbId].info.emIndex = ((ipcMsgPtr->entryLearned.emIndex) >> shift);
        __IPFIX_FW_DBG_COUNTER_INCREMENT(activeFlows);
        break;

    /* entry removed */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_REMOVED_E:
        flowId = ipcMsgPtr->entryRemoved.flowId;
        flowDbId = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId);
        cpssOsMemSet(&appIpfixFwDbPtr->flowsDb[flowDbId], 0 , sizeof(PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC));
        __IPFIX_FW_DBG_COUNTER_DECREMENT(activeFlows);
        if(appIpfixFwDbPtr->fwCfg.globalCfg.hwAutoLearnEnable == GT_FALSE)
        {
            appIpfixFwDbPtr->mgrDb.usedFlowsBmp[flowDbId/32] &= ((~(1 << flowDbId%32)) & 0xFFFFFFFF);
            __IPFIX_FW_DBG_COUNTER_DECREMENT(pendingTermination);
        }
        break;

    /* DeleteAll Completion */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DELETE_ALL_COMPLETION_E:
        appIpfixFwDbPtr->lastReturnData.numDeleted = ipcMsgPtr->deleteAllCompletion.numOfDeleted;
        break;

    /* Export Completion */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E:  /* Fall through */
    /* Data Clear All Completion */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_ALL_COMPLETION_E:
        break;

    /* global config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_RETURN_E:
        cpssOsMemCpy(&appIpfixFwDbPtr->lastReturnData.fwCfg.globalCfg, &ipcMsgPtr->globalConfigReturn.globalCfg,
                     sizeof(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC));
        break;

    /* port group config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_RETURN_E:
        cpssOsMemCpy(&appIpfixFwDbPtr->lastReturnData.fwCfg.portGroupCfg,
                     &ipcMsgPtr->portGroupConfigReturn.portGroupCfg,
                     sizeof(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC));
        break;

    /* export config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_RETURN_E:
        cpssOsMemCpy(&appIpfixFwDbPtr->lastReturnData.fwCfg.exportCfg, &ipcMsgPtr->exportConfigReturn.exportCfg,
                     sizeof(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC));
        break;

    /* elephant config return */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_RETURN_E:
        cpssOsMemCpy(&appIpfixFwDbPtr->lastReturnData.fwCfg.elephantCfg, &ipcMsgPtr->elephantConfigReturn.elephantCfg,
                     sizeof(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC));
        break;

    /* elephant state set */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_SET_E:
        flowDbId = PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, ipcMsgPtr->elephantSet.flowId);
        if((ipcMsgPtr->elephantSet.state == GT_TRUE) &&
           (appIpfixFwDbPtr->flowsDb[flowDbId].info.isElephant == GT_FALSE))
        {
            appIpfixFwDbPtr->flowsDb[flowDbId].info.isElephant = GT_TRUE;
            __IPFIX_FW_DBG_COUNTER_INCREMENT(elephantFlows);
        }
        if((ipcMsgPtr->elephantSet.state == GT_FALSE) &&
           (appIpfixFwDbPtr->flowsDb[flowDbId].info.isElephant == GT_TRUE))
        {
            if (appIpfixFwDbPtr->dbg.dbgCounters.elephantFlows > 0)
            {
                __IPFIX_FW_DBG_COUNTER_DECREMENT(elephantFlows);
            }
            appIpfixFwDbPtr->flowsDb[flowDbId].info.isElephant = GT_FALSE;
        }
        break;

    /* config error */
    case CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_E:
        appIpfixFwDbPtr->lastReturnData.configErrorValid = GT_TRUE;
        appIpfixFwDbPtr->lastReturnData.configErrorType  = ipcMsgPtr->configError.errType;
        break;

    default:
        if ((ipcMsgPtr->opcodeGet.opcode >= CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_LAST_E) ||
            (ipcMsgPtr->opcodeGet.opcode == CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_NONE_E))
        {
            __IPFIX_FW_LOG("Invalid opcode: %d\n", ipcMsgPtr->opcodeGet.opcode);
            rc = GT_BAD_VALUE;
        }
        else
        {
            __IPFIX_FW_LOG("Warn: Received illegal evel with opcode: %d\n", ipcMsgPtr->opcodeGet.opcode);
        }
        break;
    }
    if (rc == GT_OK)
    {
        __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[ipcMsgPtr->opcodeGet.opcode]);
    }

    return rc;
}

#ifndef ASIC_SIMULATION
/**
 * @internal  prvAppDemoIpfixFwIpcEventHandle function
 * @endinternal
 *
 * @brief  This function periodically scans IPC message queue and
 *         handles all IPC events
 *
 * @param[in[   arg  - input arguments
 *
 * @retval handle for the task
 */
static unsigned __TASKCONV prvAppDemoIpfixFwIpcEventHandle
(
    GT_VOID *arg
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      ipcEventCount = 0;
    GT_U32      numMsgsFetched = 0;
    GT_U32      numMsgs = 1;
    GT_U32      i;
    GT_U32      pendingMsgBuffSize = 0;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT   ipcMsgArr[PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS];

    (void) arg;
    appIpfixFwDbPtr->threads.ipcFetch.taskTerminate = GT_FALSE;
    appIpfixFwDbPtr->threads.ipcFetch.isTaskActive = GT_TRUE;

    ipcEventCount = 0;
    while (appIpfixFwDbPtr->threads.ipcFetch.taskTerminate == GT_FALSE)
    {
        rc = cpssDxChIpfixFwIpcMsgFetch(appIpfixFwDbPtr->threads.ipcFetch.devNum,
                                        appIpfixFwDbPtr->threads.ipcFetch.sCpuNum,
                                        numMsgs, &numMsgsFetched,
                                        &pendingMsgBuffSize, ipcMsgArr);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwIpcMsgFetch failed: rc=%d\n", rc);
            continue;
        }

        for (i=0; i<numMsgsFetched; i++)
        {
            rc = appDemoIpfixFwIpcEventHandle(0, &ipcMsgArr[i]);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("appDemoIpfixFwIpcEventHandle failed: rc=%d\n", rc);
                continue;
            }
            __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcEvents);
            ipcEventCount++;
        }

        cpssOsTimerWkAfter(appIpfixFwDbPtr->testParams.ipcFetchPollInterval);

        if (ipcEventCount == appIpfixFwDbPtr->threads.ipcFetch.burstSize)
        {
            ipcEventCount = 0;
            /* [TBD] Stop after handling 1K IPC messages */
        }
    }

    appIpfixFwDbPtr->threads.ipcFetch.isTaskActive = GT_FALSE;
    appIpfixFwDbPtr->threads.ipcFetch.taskTerminate = GT_TRUE;

    /* to avoid compiler warnings */
    (void) pendingMsgBuffSize;

    return 0;
}
#endif

/**
 * @internal   appDemoIpfixFwThreadsStatusDump function
 * @endinternal
 *
 * @brief Show status of the threads/taks
 *
 * @return GT_OK    - on success
 */
GT_STATUS appDemoIpfixFwThreadsStatusDump
(
    GT_VOID
)
{
    if (appIpfixFwDbPtr->threads.newFlows.isTaskActive == GT_TRUE)
    {
        __IPFIX_FW_DBG_LOG("Learning new flows task   : active\n");
    }
    else
    {
        __IPFIX_FW_DBG_LOG("Learning new flows task   : inactive\n");
    }

    if (appIpfixFwDbPtr->threads.flowsTerm.isTaskActive == GT_TRUE)
    {
        __IPFIX_FW_DBG_LOG("Flows Terminate task   : active\n");
    }
    else
    {
        __IPFIX_FW_DBG_LOG("Flows Terminate task   : inactive\n");
    }

    if (appIpfixFwDbPtr->threads.dataPkts.isTaskActive == GT_TRUE)
    {
        __IPFIX_FW_DBG_LOG("DataGetAll requests task  : active\n");
    }
    else
    {
        __IPFIX_FW_DBG_LOG("DataGetAll requests task  : inactive\n");
    }

    if (appIpfixFwDbPtr->threads.ipcFetch.isTaskActive == GT_TRUE)
    {
        __IPFIX_FW_DBG_LOG("IPC message handling task : active\n");
    }
    else
    {
        __IPFIX_FW_DBG_LOG("IPC message handling task : inactive\n");
    }

    return GT_OK;
}

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
)
{
    GT_STATUS   rc = GT_OK;

    /* allow to 'see' the packets that we get (with the DSA) */
    /* dxChNetIfRxPacketParse_DebugDumpEnable(GT_FALSE); */

    if (threadsBmp & 0x1)
    {
        appIpfixFwDbPtr->threads.newFlows.devNum  = devNum;
        appIpfixFwDbPtr->threads.newFlows.sCpuNum = sCpuNum;
        if (appIpfixFwDbPtr->fwCfg.globalCfg.hwAutoLearnEnable == GT_TRUE)
        {
            /* Flow learn thread */
            rc = cpssOsTaskCreate("flows_learn",
                              255, /* priority */
                              _8KB, /* stack */
                              prvAppDemoIpfixFwNewFlowsHandle,
                              NULL /* arg */,
                              &appIpfixFwDbPtr->threads.newFlows.taskId);
        }
        else
        {
            rc = cpssOsTaskCreate("flows_learn",
                              255, /* priority */
                              _64KB, /* stack */
                              prvAppDemoIpfixFwManualNewFlowsHandle,
                              NULL /* arg */,
                              &appIpfixFwDbPtr->threads.newFlows.taskId);
        }

        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("New Flows task failed: rc=%d\n",rc);
            return rc;
        }

        /* wait for 10ms such that the thread starts properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }

    if (threadsBmp & 0x2)
    {
        appIpfixFwDbPtr->threads.flowsTerm.devNum  = devNum;
        appIpfixFwDbPtr->threads.flowsTerm.sCpuNum = sCpuNum;

        /* Flow Termination thread */
        rc = cpssOsTaskCreate("flows_terminate",
                          255, /* priority */
                          _1KB, /* stack */
                          prvAppDemoIpfixFwFlowsTerminate,
                          NULL /* arg */,
                          &appIpfixFwDbPtr->threads.flowsTerm.taskId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("Flows terminate task failed: rc=%d\n",rc);
            return rc;
        }

        /* wait for 10ms such that the thread starts properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }

    if (threadsBmp & 0x4)
    {
        appIpfixFwDbPtr->threads.dataPkts.devNum  = devNum;
        appIpfixFwDbPtr->threads.dataPkts.sCpuNum = sCpuNum;

        /* Collecting IPFIX Data Packets thread */
        rc = cpssOsTaskCreate("data_pkts_collect",
                          255, /* priority */
                          _1KB, /* stack */
                          prvAppDemoIpfixFwDataPktsCollect,
                          NULL /* arg */,
                          &appIpfixFwDbPtr->threads.dataPkts.taskId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("Create data packets collect task failed: rc=%d\n",rc);
            return rc;
        }

        /* wait for 10ms such that the thread starts properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }

#ifndef ASIC_SIMULATION
    if (threadsBmp & 0x8)
    {
        appIpfixFwDbPtr->threads.ipcFetch.devNum  = devNum;
        appIpfixFwDbPtr->threads.ipcFetch.sCpuNum = sCpuNum;

        /* Triggering Events Reporting thread */
        rc = cpssOsTaskCreate("ipc_events_handle",
                          255, /* priority */
                          _1KB, /* stack */
                          prvAppDemoIpfixFwIpcEventHandle,
                          NULL /* arg */,
                          &appIpfixFwDbPtr->threads.ipcFetch.taskId);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("Create event handle task failed: rc=%d\n",rc);
            return rc;
        }

        /* wait for 10ms such that the thread starts properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }
#endif
    return GT_OK;
}

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
)
{
    GT_STATUS    rc = GT_OK;

    if (threadsBmp & 0x1)
    {
        appIpfixFwDbPtr->threads.newFlows.taskTerminate = GT_TRUE;
    }
    if (threadsBmp & 0x2)
    {
        appIpfixFwDbPtr->threads.flowsTerm.taskTerminate = GT_TRUE;
    }
    if (threadsBmp & 0x4)
    {
        appIpfixFwDbPtr->threads.dataPkts.taskTerminate = GT_TRUE;
    }
    if (threadsBmp & 0x8)
    {
        appIpfixFwDbPtr->threads.ipcFetch.taskTerminate = GT_TRUE;
    }

    /* Wait 2 seconds for all the threads to finish tasks */
    cpssOsTimerWkAfter(2000);

    if (threadsBmp & 0x1)
    {
        if (appIpfixFwDbPtr->threads.newFlows.isTaskActive == GT_TRUE)
        {
            /* Delete task new flows handle */
            rc = cpssOsTaskDelete(appIpfixFwDbPtr->threads.newFlows.taskId);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("Delete new flows handle task failed: rc=%d\n",rc);
                return rc;
            }
        }

        /* wait for 10ms such that the thread close properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }

    if (threadsBmp & 0x2)
    {
        if (appIpfixFwDbPtr->threads.flowsTerm.isTaskActive == GT_TRUE)
        {
            /* Delete task new flows handle */
            rc = cpssOsTaskDelete(appIpfixFwDbPtr->threads.flowsTerm.taskId);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("Delete flows terminate handle task failed: rc=%d\n",rc);
                return rc;
            }
        }

        /* wait for 10ms such that the thread close properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }

    if (threadsBmp & 0x4)
    {
        if (appIpfixFwDbPtr->threads.dataPkts.isTaskActive == GT_TRUE)
        {
            /* Delete task data packets colection */
            rc = cpssOsTaskDelete(appIpfixFwDbPtr->threads.dataPkts.taskId);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("Delete data packets collect task failed: rc=%d\n",rc);
                return rc;
            }
        }

        /* wait for 10ms such that the thread close properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }

#ifndef ASIC_SIMULATION
    if (threadsBmp & 0x8)
    {
        if (appIpfixFwDbPtr->threads.ipcFetch.isTaskActive == GT_TRUE)
        {
            /* Delete task ipcEventHandle */
            rc = cpssOsTaskDelete(appIpfixFwDbPtr->threads.ipcFetch.taskId);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("Delete event handle task failed: rc=%d\n",rc);
                return rc;
            }
        }

        /* wait for 10ms such that the thread close properly */
        cpssOsTimerWkAfter(PRV_APP_IPFIX_FW_WAIT_AFTER_TASK_CREATE_CNS);
    }
#endif
    /* allow to 'see' the packets that we get (with the DSA) */
    dxChNetIfRxPacketParse_DebugDumpEnable(GT_FALSE);

    return GT_OK;
}
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
)
{
    __IPFIX_FW_NULL_PTR_CHECK(appIpfixFwDbPtr);
    __IPFIX_FW_NULL_PTR_CHECK(txInitParamsPtr);
    __IPFIX_FW_NULL_PTR_CHECK(policerInitParamsPtr);
    __IPFIX_FW_NULL_PTR_CHECK(pclInitParamsPtr);
    __IPFIX_FW_NULL_PTR_CHECK(emInitParamsPtr);
    __IPFIX_FW_NULL_PTR_CHECK(emAutoLearnInitParamsPtr);
    __IPFIX_FW_NULL_PTR_CHECK(emmInitParamsPtr);

    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.txInitParams, txInitParamsPtr, sizeof(*txInitParamsPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.policerInitParams, policerInitParamsPtr, sizeof(*policerInitParamsPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.pclInitParams, pclInitParamsPtr, sizeof(*pclInitParamsPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.emInitParams, emInitParamsPtr, sizeof(*emInitParamsPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.emAutoLearnInitParams, emAutoLearnInitParamsPtr, sizeof(*emAutoLearnInitParamsPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->initCfg.emmInitParams, emmInitParamsPtr, sizeof(*emmInitParamsPtr));

    return GT_OK;
}

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
)
{
    __IPFIX_FW_NULL_PTR_CHECK(appIpfixFwDbPtr);
    __IPFIX_FW_NULL_PTR_CHECK(globalCfgPtr);
    __IPFIX_FW_NULL_PTR_CHECK(exportCfgPtr);
    __IPFIX_FW_NULL_PTR_CHECK(elephantCfgPtr);

    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.globalCfg, globalCfgPtr, sizeof(*globalCfgPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.exportCfg, exportCfgPtr, sizeof(*exportCfgPtr));
    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.elephantCfg, elephantCfgPtr, sizeof(*elephantCfgPtr));

    return GT_OK;
}

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
)
{
    GT_STATUS                                rc;
    GT_U32                                   emmId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  emmEntry;
    GT_U32                                   portGroupId;

    portGroupId = (flowId >> 12) & 0x7;

    if (appIpfixFwDbPtr->fwCfg.globalCfg.hwAutoLearnEnable == GT_FALSE)
    {
        emmId = appIpfixFwDbPtr->mgrDb.exactMatchManagerId[portGroupId/2];
        cpssOsMemCpy(&emmEntry.exactMatchEntry.key,
                     &appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].key,
                     sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
        emmEntry.exactMatchEntry.lookupNum = appIpfixFwDbPtr->initCfg.emmInitParams.lookupNum;
        emmEntry.exactMatchActionType = appIpfixFwDbPtr->initCfg.emmInitParams.actionType;
        rc = cpssDxChExactMatchManagerEntryDelete(emmId, &emmEntry);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChExactMatchManagerEntryDelete failed: rc=%d\n",rc);
            return rc;
        }

        if(appIpfixFwDbPtr->mgrDb.keysLookupAlgo == PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_HASH_E)
        {
            rc = prvAppDemoIpfixFwKeysDbEntryDelete(devNum, portGroupId, &emmEntry.exactMatchEntry.key);
            if (rc != GT_OK)
            {
                __IPFIX_FW_LOG("prvAppDemoIpfixFwKeysDbEntryDelete failed: rc=%d\n",rc);
                return rc;
            }
        }

        appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].info.pendingTermination = GT_TRUE;
        __IPFIX_FW_DBG_COUNTER_INCREMENT(pendingTermination);
    }

    rc = cpssDxChIpfixFwEntryDelete(devNum, sCpuNum, flowId);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwEntryDelete failed: rc=%d\n",rc);
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    prevDelAllCompleteNum = 0;
    GT_U32    delayCount = 0;
    GT_U32                                   i,j;
    GT_U32                                   flowId = 0;
    GT_U32                                   emmId;
    CPSS_DXCH_EXACT_MATCH_MANAGER_ENTRY_STC  emmEntry;
    GT_U32                                   numEmDeleted;

    if (appIpfixFwDbPtr->fwCfg.globalCfg.hwAutoLearnEnable == GT_FALSE)
    {
        numEmDeleted = 0;
        for (i=0; i<PRV_APP_IPFIX_FW_PORT_GROUPS_MAX_CNS; i++)
        {
            for (j = 0; j < PRV_APP_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS; j++)
            {
                flowId = ((i & 0x7) << 12) | (j & 0xFFF);
                if(appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].info.isActive == GT_FALSE)
                {
                    continue;
                }

                emmId = appIpfixFwDbPtr->mgrDb.exactMatchManagerId[i/2];
                cpssOsMemCpy(&emmEntry.exactMatchEntry.key,
                             &appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].key,
                             sizeof(CPSS_DXCH_EXACT_MATCH_KEY_STC));
                emmEntry.exactMatchEntry.lookupNum = appIpfixFwDbPtr->initCfg.emmInitParams.lookupNum;
                emmEntry.exactMatchActionType = appIpfixFwDbPtr->initCfg.emmInitParams.actionType;
                rc = cpssDxChExactMatchManagerEntryDelete(emmId, &emmEntry);
                if (rc != GT_OK)
                {
                    __IPFIX_FW_LOG("cpssDxChExactMatchManagerEntryDelete failed: rc=%d\n",rc);
                    /* return rc; */ /*Do not exit*/
                }

                appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].info.pendingTermination = GT_TRUE;
                __IPFIX_FW_DBG_COUNTER_INCREMENT(pendingTermination);

                if (rc == GT_OK)
                {
                    numEmDeleted++;
                }
                if(appIpfixFwDbPtr->mgrDb.keysLookupAlgo == PRV_APP_IPFIX_FW_KEYS_LOOKUP_ALGO_HASH_E)
                {
                    rc = prvAppDemoIpfixFwKeysDbEntryDelete(devNum, i,
                            &appIpfixFwDbPtr->flowsDb[PRV_APP_IPFIX_FW_FLOW_ID_2_DB_ID_CONVERT(devNum, flowId)].key);
                    if (rc != GT_OK)
                    {
                        __IPFIX_FW_LOG("prvAppDemoIpfixFwKeysDbEntryDelete failed: rc=%d\n",rc);
                        /* return rc; */ /*Do not exit*/
                    }
                }
            }
        }

        __IPFIX_FW_DBG_LOG("Deleted %d EM Entries\n", numEmDeleted);
    }

    prevDelAllCompleteNum = appIpfixFwDbPtr->dbg.dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DELETE_ALL_COMPLETION_E];
    rc = cpssDxChIpfixFwEntryDeleteAll(devNum, sCpuNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwEntryDeleteAll failed: rc=%d\n", rc);
        return rc;
    }

    while (prevDelAllCompleteNum == appIpfixFwDbPtr->dbg.dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DELETE_ALL_COMPLETION_E])
    {
        cpssOsTimerWkAfter(1);
        delayCount ++;
        if (delayCount >= PRV_APP_IPFIX_FW_WAIT_FOR_DELETE_ALL_COMPLETE_CNS)
        {
#ifndef ASIC_SIMULATION
            __IPFIX_FW_LOG("Warning: DeleteAllCompleteMsg not received yet\n");
#endif
            break;
        }
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwPreReqInit function
 * @endinternal
 *
 * @brief configure pre requisite settings
 *
 * @param[in] devNum - device number
 * @param[in] emIpv4KeySize - exactmatch key size
 * @param[in] hwAutoLearnEnable - GT_TRUE/GT_FALSE - enable/disable hardare uato Learn
 *
 * @retval GT_OK - on success
 */
GT_STATUS appDemoIpfixFwPreReqInit
(
    IN GT_U8    devNum,
    IN CPSS_DXCH_EXACT_MATCH_KEY_SIZE_ENT emIpv4KeySize,
    IN GT_BOOL  hwAutoLearnEnable
)
{
    GT_STATUS                                         rc = GT_OK;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                     mask;
    CPSS_DXCH_EXACT_MATCH_ACTION_UNT                  exactMatchAutoLearnDefaulActionData;
    GT_HW_DEV_NUM                                     hwDevNum;
    PRV_APP_IPFIX_FW_TX_INIT_PARAMS_STC            txInitParams;
    PRV_APP_IPFIX_FW_POLICER_INIT_PARAMS_STC       policerInitParams;
    PRV_APP_IPFIX_FW_PCL_INIT_PARAMS_STC           pclInitParams;
    PRV_APP_IPFIX_FW_EM_INIT_PARAMS_STC            emInitParams;
    PRV_APP_IPFIX_FW_EM_AUTO_LEARN_INIT_PARAMS_STC emAutoLearnInitParams;
    PRV_APP_IPFIX_FW_EMM_INIT_PARAMS_STC           emmInitParams;
    GT_U32                                         i;

    cpssOsMemSet(&txInitParams, 0, sizeof(txInitParams));
    cpssOsMemSet(&policerInitParams, 0, sizeof(policerInitParams));
    cpssOsMemSet(&pclInitParams, 0, sizeof(pclInitParams));
    cpssOsMemSet(&emInitParams, 0, sizeof(emInitParams));
    cpssOsMemSet(&emAutoLearnInitParams, 0, sizeof(emAutoLearnInitParams));
    cpssOsMemSet(&emmInitParams, 0, sizeof(emmInitParams));

    /* get hardware device number */
    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChCfgHwDevNumGet failed rc = %d\n", rc);
        return rc;
    }

    /* TX Init configuration */
    txInitParams.dataPktsCpuCode  = PRV_APP_IPFIX_FW_DATA_PKTS_CPU_CODE;
    txInitParams.dataPktsQueueNum = PRV_APP_IPFIX_FW_DATA_PKTS_QUEUE_NUM_CNS;
    txInitParams.newFlowsCpuCode  = PRV_APP_IPFIX_FW_FIRST_PKTS_CPU_CODE;
    txInitParams.newFlowsQueueNum = PRV_APP_IPFIX_FW_FIRST_PKTS_QUEUE_NUM_CNS;
    txInitParams.synPktsCpuCode   = PRV_APP_IPFIX_FW_SYN_PKTS_CPU_CODE;
    txInitParams.synPktsQueueNum  = PRV_APP_IPFIX_FW_SYN_PKTS_QUEUE_NUM_CNS;
    rc = appDemoIpfixFwTxInit(devNum, &txInitParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwTxInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Policer Init configuration */
    policerInitParams.policerStage                = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    policerInitParams.maxPolicerEntries           = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum);
    policerInitParams.waConfig.action             = CPSS_DXCH_IPFIX_WRAPAROUND_ACTION_CLEAR_E;
    policerInitParams.waConfig.byteThreshold.l[0] = 0xFFFFFFFF;
    policerInitParams.waConfig.byteThreshold.l[1] = 0xF;
    policerInitParams.waConfig.dropThreshold      = 0x3FFFFFFF;
    policerInitParams.waConfig.packetThreshold    = 0x3FFFFFFF;
    if (policerInitParams.policerStage == CPSS_DXCH_POLICER_STAGE_INGRESS_0_E)
    {
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] = policerInitParams.maxPolicerEntries;
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = 0;
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E]    =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum -
                    policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E];

        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum;
        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = 0;
        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E]    = 0;
    }
    else if (policerInitParams.policerStage == CPSS_DXCH_POLICER_STAGE_INGRESS_1_E)
    {
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = policerInitParams.maxPolicerEntries;
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] = 0;
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E]    =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum -
                    policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E];

        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum;
        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] = 0;
        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E]    = 0;
    }
    else
    {
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E] = policerInitParams.maxPolicerEntries;
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = 0;
        policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E]    =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.plrInfo.countingEntriesNum -
                    policerInitParams.memoryCfg.numCountingEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E];

        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_EGRESS_E] =
                    PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.policersNum;
        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_1_E] = 0;
        policerInitParams.memoryCfg.numMeteringEntries[CPSS_DXCH_POLICER_STAGE_INGRESS_0_E] = 0;
    }
    policerInitParams.baseFlowId = PRV_APP_IPFIX_FW_BASE_FLOW_ID_CNS;
    policerInitParams.firstPacketCfgSet.cpuCode = PRV_APP_IPFIX_FW_FIRST_PKTS_CPU_CODE;
    policerInitParams.firstPacketCfgSet.packetCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    rc = appDemoIpfixFwPolicerInit(devNum, &policerInitParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwPolicerInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Enables the activation of aging for IPFIX */
    rc = cpssDxChIpfixAgingEnableSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixAgingEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    /* PCL Init configuration */
    pclInitParams.direction         = CPSS_PCL_DIRECTION_INGRESS_E;
    pclInitParams.lookupNum         = CPSS_PCL_LOOKUP_0_E;
    rc = appDemoIpfixFwPclInit(devNum, &pclInitParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwPclInit failed: rc=%d\n", rc);
        return rc;
    }

    if (hwAutoLearnEnable == GT_TRUE)
    {
        /* Exact Match Init configuration */
        emInitParams.emUnitNum           = CPSS_DXCH_EXACT_MATCH_UNIT_0_E;
        emInitParams.emLookupNum         = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        emInitParams.emClientType        = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
        emInitParams.pclDirection        = CPSS_PCL_DIRECTION_INGRESS_E;
        emInitParams.pclLookupNum        = CPSS_PCL_LOOKUP_0_E;
        emInitParams.pclEnableEmLookup   = GT_TRUE;
        emInitParams.pclProfileId        = PRV_APP_IPFIX_FW_PCL_PROFILE_ID_CNS;
        emInitParams.pclSubProfileId     = 0;
        emInitParams.emActionType        = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        emInitParams.expandedActionIndex = PRV_APP_IPFIX_FW_EM_EXPANDED_ACTION_IDX_CNS;
        emInitParams.emActionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        emInitParams.emActionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        emInitParams.emActionData.pclAction.bypassBridge = GT_TRUE;
        emInitParams.emActionData.pclAction.bypassIngressPipe = GT_TRUE;
        emInitParams.emActionData.pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        emInitParams.emActionData.pclAction.redirect.data.outIf.outInterface.type = CPSS_INTERFACE_PORT_E;
        emInitParams.emActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum = hwDevNum;
        emInitParams.emActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum = PRV_APP_IPFIX_FW_TEST_PORT_NUM_CNS;
        emInitParams.emActionData.pclAction.policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;
        emInitParams.emActionData.pclAction.ipfixEnable = GT_TRUE;
        /*emInitParams.emActionData.pclAction.flowId = 5;*/
        emInitParams.emActionData.pclAction.egressPolicy = CPSS_PCL_DIRECTION_INGRESS_E;
        /* the outInterface should be taken from reduced entry */
        emInitParams.expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionEgressInterface=GT_TRUE;
        /* if auto learning is enabled; flowId is always taken from reduced action*/
        emInitParams.expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionFlowId = GT_TRUE;
        emInitParams.expandedActionOrigin.pclExpandedActionOrigin.overwriteExpandedActionUserDefinedCpuCode = GT_TRUE;
        rc = appDemoIpfixFwEmInit(devNum, &emInitParams);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwEmInit failed: rc=%d\n", rc);
            return rc;
        }

        /* Exact match Auto Learn Init Configurations */
        emAutoLearnInitParams.emUnitNum    = CPSS_DXCH_EXACT_MATCH_UNIT_0_E;
        emAutoLearnInitParams.emLookupNum  = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        emAutoLearnInitParams.emClientType = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
        emAutoLearnInitParams.emProfileId  = PRV_APP_IPFIX_FW_PCL_PROFILE_ID_CNS;

        emAutoLearnInitParams.autoLearnProfileId  = PRV_APP_IPFIX_FW_PCL_PROFILE_ID_CNS;

        emAutoLearnInitParams.autoLearnActionType = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        emAutoLearnInitParams.expandedActionIndex = PRV_APP_IPFIX_FW_EM_EXPANDED_ACTION_IDX_CNS;

        emAutoLearnInitParams.autoLearnFlowIdConfig.baseFlowId  = 0;
        emAutoLearnInitParams.autoLearnFlowIdConfig.maxNum      = PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_MAX_CNS(devNum);
        emAutoLearnInitParams.autoLearnFlowIdConfig.threshold   = PRV_APP_IPFIX_FW_AUTO_LEARN_FLOWS_MAX_CNS(devNum) + 1;

        /* Set Key Params */
        cpssOsMemSet(&mask, 0, sizeof(mask));
        emAutoLearnInitParams.emProfileKeyParams.keySize  = emIpv4KeySize;
        emAutoLearnInitParams.emProfileKeyParams.keyStart = 0;
        cpssOsMemSet(mask.ruleIngrUdbOnly.udb, 0xFF, sizeof(mask.ruleIngrUdbOnly.udb));/* full mask for first stage */
        /*mask.ruleIngrUdbOnly.udb[3] = 0x1F;
        mask.ruleIngrUdbOnly.udb[5] = 0x1F;*/
        cpssOsMemCpy((GT_VOID*)&emAutoLearnInitParams.emProfileKeyParams.mask[0],
                     mask.ruleIngrUdbOnly.udb, sizeof(emAutoLearnInitParams.emProfileKeyParams.mask));

        /* AUTODOC: Auto Learn configuration */
        cpssOsMemSet((GT_VOID*) &exactMatchAutoLearnDefaulActionData, 0, sizeof(exactMatchAutoLearnDefaulActionData));

        exactMatchAutoLearnDefaulActionData.pclAction.pktCmd = CPSS_PACKET_CMD_FORWARD_E;
        exactMatchAutoLearnDefaulActionData.pclAction.mirror.cpuCode = CPSS_NET_FIRST_USER_DEFINED_E+5;
        exactMatchAutoLearnDefaulActionData.pclAction.bypassBridge = GT_TRUE;
        exactMatchAutoLearnDefaulActionData.pclAction.bypassIngressPipe = GT_TRUE;
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.redirectCmd=CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.type=CPSS_INTERFACE_PORT_E;
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum=devNum;
        exactMatchAutoLearnDefaulActionData.pclAction.redirect.data.outIf.outInterface.devPort.portNum=PRV_APP_IPFIX_FW_TEST_PORT_NUM_CNS;
        exactMatchAutoLearnDefaulActionData.pclAction.ipfixEnable = GT_TRUE;
        exactMatchAutoLearnDefaulActionData.pclAction.flowId = 2;
        exactMatchAutoLearnDefaulActionData.pclAction.policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_COUNTER_ONLY_E;

        cpssOsMemCpy(&emAutoLearnInitParams.autoLearnDefaulActionData,
                     &exactMatchAutoLearnDefaulActionData,
                     sizeof(emAutoLearnInitParams.autoLearnDefaulActionData));

        rc = appDemoIpfixFwEmAutoLearnInit(devNum, &emAutoLearnInitParams);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwEmAutoLearnInit failed: rc=%d\n", rc);
            return rc;
        }
    }
    else
    {
        emmInitParams.expandedActionIndex = PRV_APP_IPFIX_FW_EM_EXPANDED_ACTION_IDX_CNS;
        emmInitParams.emmProfileEntryIndex = PRV_APP_IPFIX_FW_EM_PROFILE_IDX_CNS;
        emmInitParams.lookupNum           = CPSS_DXCH_EXACT_MATCH_LOOKUP_FIRST_E;
        emmInitParams.lookupClient        = CPSS_DXCH_EXACT_MATCH_CLIENT_IPCL_0_E;
        emmInitParams.actionType          = CPSS_DXCH_EXACT_MATCH_ACTION_PCL_E;
        emmInitParams.emIpv4KeySize       = emIpv4KeySize;
        emmInitParams.emIpv6KeySize       = emIpv4KeySize /* CPSS_DXCH_EXACT_MATCH_KEY_SIZE_47B_E */;
        /* Set Ipv4 Key Mask*/
        for (i=0; i<14; i++)
        {
            emmInitParams.emIpv4KeyMask[i] = 0xFF;
        }
        if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow ==
                      CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
        {
            /* Ignore port number */
            emmInitParams.emIpv4KeyMask[4] = 0x0;
        }

        /* Set Key Mask*/
        for (i=0; i<14/*39*/; i++)
        {
            emmInitParams.emIpv6KeyMask[i] = 0xFF;
        }
        if (appIpfixFwDbPtr->fwCfg.globalCfg.ipfixEntriesPerFlow ==
                      CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E)
        {
            /* Ignore port number */
            emmInitParams.emIpv6KeyMask[4] = 0x0;
        }

        prvAppDemoIpfixFwExactMatchActionSet(devNum,
                                              appIpfixFwDbPtr->testParams.egressPort,
                                              emmInitParams.actionType,
                                              &emmInitParams.action,
                                              0 /* flowId */, CPSS_PACKET_CMD_FORWARD_E);

        rc = appDemoIpfixFwEmmInit(devNum, &emmInitParams);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwEmmInit failed: rc=%d\n", rc);
            return rc;
        }
    }
    rc = appDemoIpfixFwAppInitParamsUpdate(&txInitParams, &policerInitParams, &pclInitParams,
                                           &emInitParams, &emAutoLearnInitParams, &emmInitParams);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwAppInitParamsUpdate failed: rc=%d\n", rc);
        return rc;
    }
    return GT_OK;
}

static GT_STATUS appDemoIpfixFwPreReqRestore
(
    IN  GT_U8   devNum,
    IN GT_BOOL  hwAutoLearnEnable
)
{
    GT_STATUS rc = GT_OK;

    /* Restore IPFIX aging */
    rc = cpssDxChIpfixAgingEnableSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixAgingEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    if (hwAutoLearnEnable == GT_TRUE)
    {
        /* Restore Exact match auto learn configurations */
        rc = appDemoIpfixFwEmAutoLearnRestore(devNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwEmRestore failed: rc=%d\n", rc);
            return rc;
        }

        /* Restore Exact match configurations */
        rc = appDemoIpfixFwEmRestore(devNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwEmRestore failed: rc=%d\n", rc);
            return rc;
        }

    }
    else
    {
        /* Restore Exact match manager configurations */
        rc = appDemoIpfixFwEmmRestore(devNum);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("appDemoIpfixFwEmRestore failed: rc=%d\n", rc);
            return rc;
        }
    }

    /* Restore Policer configuration*/
    rc = appDemoIpfixFwPolicerRestore(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwPolicerRestore failed: rc=%d\n", rc);
        return rc;
    }

    /* Restore PCL configuration */
    rc = appDemoIpfixFwPclRestore(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwPclRestore failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixFwTxRestore(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwTxRestore failed: rc=%d\n", rc);
        return rc;
    }

    return GT_OK;
}

/**
 * @internal appDemoIpfixFwConfigSet function
 * @endinternal
 *
 * @brief Set IPFIX configuration
 *
 * @param[in] devNum   - device number
 * @param[in] sCpuNum  - service CPU number
 * @param[in] hwAutoLearnEnable - GT_TRUE/GT_FALSE - enable/disable hardare uato Learn
 *
 * @return GT_OK - on success
 */
GT_STATUS appDemoIpfixFwConfigSet
(
    IN  GT_U8   devNum,
    IN  GT_U8   sCpuNum,
    IN GT_BOOL  hwAutoLearnEnable
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    globalCfg;
    CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC portGroupCfg;
    CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC    exportCfg;
    CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  elephantCfg;
    CPSS_DXCH_NET_DSA_PARAMS_STC            dsaInfo;
    GT_U8                                   dsaTag[PRV_APP_IPFIX_FW_TX_DSA_TAG_LEN_CNS];
    GT_HW_DEV_NUM                           hwDevNum;
    GT_U32                                  i;
    GT_U32                                  numOfPhysicalPorts;
    GT_U32                                  portNum;
    GT_BOOL                                 isCpu;
    CPSS_DXCH_DETAILED_PORT_MAP_STC         portMap;
    GT_BOOL                                 isMGExists = GT_FALSE;

    /* Set CPSS ipfix fw global configuration */
    cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
    globalCfg.ipfixEnable                = GT_FALSE;
    globalCfg.idleAgingOffload           = GT_FALSE;
    globalCfg.longAgingOffload           = GT_FALSE;
    globalCfg.countersExtensionOffload   = GT_FALSE;
    globalCfg.timeStampsExtensionOffload = GT_FALSE;
    globalCfg.firstPolicerStage          = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
    globalCfg.maxInTransitIndex          = PRV_APP_IPFIX_FW_MAX_IN_TRANSIT_ENTRIES_CNS(devNum);
    globalCfg.idleTimeout                = 0;
    globalCfg.activeTimeout              = 0;
    globalCfg.scanCycleInterval          = 10; /* 1 Seconds i.e. 10 * 100 ms*/
    globalCfg.scanLoopItems              = (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) ? 256 :PRV_APP_IPFIX_FW_SCAN_LOOP_ITEMS_CNS(devNum); /* ipfix entries scan */
    globalCfg.secondPolicerStage         = CPSS_DXCH_POLICER_STAGE_INGRESS_1_E;
    globalCfg.reportNewFlows             = GT_TRUE;
    globalCfg.hwAutoLearnEnable          = hwAutoLearnEnable;
    globalCfg.maxFwIndex                 = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1;
    globalCfg.maxAppIndex                = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1;
    globalCfg.firstPolicerMaxIpfixIndex  = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1;
    globalCfg.secondPolicerMaxIpfixIndex = PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1;
    globalCfg.ipfixEntriesPerFlow        = CPSS_DXCH_IPFIX_FW_IPFIX_ENTRIES_PER_FLOW_IS_TWO_E;
    rc = cpssDxChIpfixFwGlobalConfigSet(devNum, sCpuNum, &globalCfg);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwGlobalConfigSet failed rc = %d\n", rc);
        return rc;
    }
    __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_SET_E]);

    /* Set CPSS ipfix fw export configuration */
    /* Build Extended TO_CPU DSA tag; 4 words */
    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (GT_OK != rc)
    {
        __IPFIX_FW_LOG("cpssDxChCfgHwDevNumGet failed rc = %d\n", rc);
        return rc;
    }
    cpssOsMemSet(&dsaInfo,0,sizeof(dsaInfo));
    dsaInfo.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;
    dsaInfo.dsaType = CPSS_DXCH_NET_DSA_CMD_TO_CPU_E;
    dsaInfo.dsaInfo.toCpu.hwDevNum = (hwDevNum + 1) % 32;
    dsaInfo.dsaInfo.toCpu.cpuCode = appIpfixFwDbPtr->initCfg.txInitParams.dataPktsCpuCode;
    /* build DSA Tag buffer */
    cpssOsMemSet(dsaTag, 0, sizeof(dsaTag));
    rc = cpssDxChNetIfDsaTagBuild(devNum, &dsaInfo, &dsaTag[0]);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChNetIfDsaTagBuild failed rc = %d\n", rc);
        return rc;
    }

    cpssOsMemSet(&exportCfg, 0, sizeof(exportCfg));
    exportCfg.dataPktMtu             = PRV_APP_IPFIX_FW_DATA_PKTS_MTU_CNS;
    exportCfg.localQueueNum          = PRV_APP_IPFIX_FW_DATA_PKTS_QUEUE_NUM_CNS;
    exportCfg.mgNum                  = 0;
    exportCfg.periodicExportEnable   = GT_FALSE;
    exportCfg.periodicExportInterval = 100;
    exportCfg.exportLastCpuCode      = GT_TRUE;
    exportCfg.exportLastPktCmd       = GT_TRUE;
    for(i=0; i<PRV_APP_IPFIX_FW_TX_DSA_TAG_LEN_CNS; i++)
    {
        exportCfg.txDsaTag[i/4] |= (dsaTag[i] << (8*(i%4)));
    }

    rc = cpssDxChIpfixFwExportConfigSet(devNum, sCpuNum, &exportCfg);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwExportConfigSet failed: rc=%d\n", rc);
        return rc;
    }
    __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_SET_E]);

    numOfPhysicalPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for (portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChPortPhysicalPortMapIsCpuGet failed rc = %d\n", rc);
            return rc;
        }
        if(isCpu == GT_FALSE)
        {
            continue;
        }
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMap);
        if(portMap.extPortMap.mgUnitIndex == exportCfg.mgNum)
        {
            isMGExists = GT_TRUE;
            break;
        }
    }
    if (isMGExists == GT_FALSE)
    {
        __IPFIX_FW_LOG("MG Unit does not exist = %d\n", exportCfg.mgNum);
        return rc;
    }

    /* Set CPSS ipfix fw elephant configuration */
    cpssOsMemSet(&elephantCfg, 0, sizeof(elephantCfg));
    elephantCfg.elephantDetectionOffload = GT_TRUE;
    elephantCfg.reportMiceEnable         = GT_TRUE;
    elephantCfg.reportEmIndexEnable      = GT_TRUE;
    elephantCfg.pktCntThreshold[0]       = 100; /* change later */
    elephantCfg.pktCntThreshold[1]       = 0;
    elephantCfg.byteCntThreshold[0]      = 1000;
    elephantCfg.byteCntThreshold[1]      = 0;
    elephantCfg.crossCntThresholdHigh    = 0xE0;
    elephantCfg.crossCntThresholdLow     = 0x20;
    elephantCfg.startThreshold           = 0x100;
    rc = cpssDxChIpfixFwElephantConfigSet(devNum, sCpuNum, &elephantCfg);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwElephantConfigSet failed: rc=%d\n", rc);
        return rc;
    }
    __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_SET_E]);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        cpssOsMemSet(&portGroupCfg, 0, sizeof(portGroupCfg));
        portGroupCfg.firstPolicerStage = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
        portGroupCfg.secondPolicerStage = portGroupCfg.firstPolicerStage;
        for (i=0; i<PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; i++)
        {
                portGroupCfg.firstPolicerMaxIpfixIndex[i] = (PRV_APP_IPFIX_FW_POLICER_ENTRIES_MAX_CNS(devNum) - 1);
                portGroupCfg.secondPolicerMaxIpfixIndex[i] = portGroupCfg.firstPolicerMaxIpfixIndex[i];
        }
        rc = cpssDxChIpfixFwPortGroupConfigSet(devNum, sCpuNum,&portGroupCfg);
        if (rc != GT_OK)
        {
            __IPFIX_FW_LOG("cpssDxChIpfixFwPortGroupConfigSet failed: rc=%d\n", rc);
            return rc;
        }
        __IPFIX_FW_DBG_COUNTER_INCREMENT(ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_SET_E]);
    }

    /* Get configurations and check or update params */
#ifndef ASIC_SIMULATION
    rc = appDemoIpfixFwConfigGetAndVerify(devNum, sCpuNum, 0xF, GT_TRUE, GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwConfigGetAndVerify failed: rc=%d\n", rc);
        /*return rc;*/
    }
#else
    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.globalCfg, &globalCfg, sizeof(CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC));
    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.portGroupCfg, &portGroupCfg, sizeof(CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC));
    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.exportCfg, &exportCfg, sizeof(CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC));
    cpssOsMemCpy(&appIpfixFwDbPtr->fwCfg.elephantCfg, &elephantCfg, sizeof(CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC));
#endif

    return GT_OK;
}

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
)
{
    GT_STATUS                               rc = GT_OK;

    /* Initialize application ipfix manager */
    rc = appDemoIpfixFwDbCreate(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwDbCreate failed: rc=%d\n", rc);
        return rc;
    }

    /* Enable debug prints */
    appDemoIpfixFwDebugPrintsEnable(GT_TRUE);
    /* appDemoIpfixFwCpuRxDumpEnable(GT_TRUE); */

    rc = appDemoIpfixFwPreReqInit(devNum, CPSS_DXCH_EXACT_MATCH_KEY_SIZE_19B_E, hwAutoLearnEnable);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwPreReqInit failed: rc=%d\n", rc);
        return rc;
    }

    /* Load FW */
    rc = cpssDxChIpfixFwInit(devNum, sCpuNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwInit failed: rc=%d\n", rc);
        return rc;
    }

    /* FW config Set */
    rc = appDemoIpfixFwConfigSet(devNum, sCpuNum, hwAutoLearnEnable);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwConfigSet failed: rc=%d\n", rc);
        return rc;
    }

    /* Ipfix manager enable */
    rc = cpssDxChIpfixFwEnableSet(devNum, sCpuNum, GT_TRUE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixFwThreadsStart(devNum, sCpuNum, 0xF);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwThreadsStart failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}

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
)
{
    GT_STATUS                                      rc = GT_OK;

    /* Ipfix manager disable */
    rc = cpssDxChIpfixFwEnableSet(devNum, sCpuNum, GT_FALSE);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwEnableSet failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixFwEntryDeleteAll(devNum, sCpuNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwEntryDeleteAll failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixFwThreadsStop(0xF);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwThreadsStop failed: rc=%d\n", rc);
        return rc;
    }

    rc = appDemoIpfixFwPreReqRestore(devNum, hwAutoLearnEnable);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwTxRestore failed: rc=%d\n", rc);
        return rc;
    }

    rc = cpssDxChIpfixFwRestore(devNum, sCpuNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("cpssDxChIpfixFwRestore failed: rc=%d\n", rc);
        return rc;
    }

    /* Delete ipfix manager in applicaiton */
    rc = appDemoIpfixFwDbDelete(devNum);
    if (rc != GT_OK)
    {
        __IPFIX_FW_LOG("appDemoIpfixFwDbDelete failed: rc=%d\n", rc);
        return rc;
    }

    return rc;
}
