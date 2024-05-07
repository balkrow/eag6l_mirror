/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file appDemoCfgMisc.c
*
* @brief Includes misc functions implementations to be used by the application
* demo configuration functions.
*
* @version   26
********************************************************************************
*/

#include <appDemo/sysHwConfig/gtAppDemoSmiConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>

#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/hwDriver/cpssHwDriverAPI.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if defined CHX_FAMILY
    #include <cpss/generic/cpssHwInit/private/prvCpssHwInit.h>
    #include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
    #include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>
    #include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
    #include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIf.h>
    #include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>

#endif/*CHX_FAMILY*/
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <cpss/common/smi/cpssGenSmi.h>

/*******************************************************************************
 * Internal variables
 ******************************************************************************/

/* This variable will be set to GT_TRUE  by function appDemoRxBuffsCacheableSet in case of cacheable RX buffers */
static GT_BOOL rxBuffersInCachedMem = GT_FALSE;

#ifdef IMPL_PP
static GT_STATUS coreGetRxDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
);
#endif /* IMPL_PP */


GT_STATUS appDemoGetTxDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
);

#ifdef IMPL_PP
static GT_STATUS coreGetAuDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
);
#endif /* IMPL_PP */

#ifdef CHX_FAMILY
static CPSS_REG_VALUE_INFO_STC   dummyRegValInfoList[]       = GT_DUMMY_REG_VAL_INFO_LIST;
#else /*CHX_FAMILY*/
static CPSS_REG_VALUE_INFO_STC   dummyRegValInfoList[]       = {{0}};
#endif /*CHX_FAMILY*/

/**
* @internal appDemoRxBuffsCacheableSet function
* @endinternal
*
* @brief   This function set status of RX buffers - cacheable or not.
*         Should be called one time BEFORE cpssInitSystem
* @param[in] buffersCacheable         - Should be GT_TRUE if RX buffers are in cacheable memory.
*
* @retval GT_OK                    - always,
*/
GT_STATUS appDemoRxBuffsCacheableSet
(
    IN GT_BOOL buffersCacheable
)
{
    rxBuffersInCachedMem = buffersCacheable;

    return GT_OK;
}

/**
* @internal appDemoRawSocketModeSet function
* @endinternal
*
* @brief   Instruct the system to use raw packets in MII mode.
*
* @retval GT_OK                    - always,
*/
GT_STATUS appDemoRawSocketModeSet
(
  IN GT_VOID
)
{
  cpssExtDrvEthRawSocketModeSet(GT_TRUE);
  return GT_OK;
}

/**
* @internal appDemoLinuxModeSet function
* @endinternal
*
* @brief   Set port <portNum> to Linux Mode (applicable for Linux Only)
*         INPUTS:
*
* @retval GT_OK                    - always,
*/

GT_STATUS appDemoLinuxModeSet
(
 IN GT_U32  portNum,
 IN GT_U32 ip1,
 IN GT_U32 ip2,
 IN GT_U32 ip3,
 IN GT_U32 ip4
)
{
  return cpssExtDrvLinuxModeSet(portNum, ip1, ip2, ip3, ip4);
}

/**
* @internal appDemoLinuxModeGet function
* @endinternal
*
* @brief   Get port <portNum> Linux Mode indication (Linux Only)
*
* @param[in] portNum                  - The port number to be defined for Linux mode
*                                       GT_TRUE if Linux mode, or
*                                       GT_FALSE otherwise.
*/

GT_BOOL appDemoLinuxModeGet
(
 IN GT_U32  portNum
)
{
  return cpssExtDrvLinuxModeGet(portNum);
}

/* next info not saved in CPSS , so can not be retrieved from CPSS. (lack of support)

   so we need to store the DMAs allocated from the first time
*/
typedef struct{
    GT_BOOL                    useMultiNetIfSdma;
    CPSS_MULTI_NET_IF_CFG_STC  multiNetIfCfg;
}APP_DEMO_DB_gtShutdownAndCoreRestart_STC;


GT_VOID* appDemoMemCacheDmaMalloc
(
    IN GT_U8  devNum,
    IN GT_U32 size
)
{
    if(devNum == 0)
    {
        /* legacy API */
        return osCacheDmaMalloc(size);
    }
    else
    {
        /* this function also supports 'devNum = 0' ,
           but lets keep using osCacheDmaMalloc() for the devNum == 0 */
        return osCacheDmaMallocByDevice(devNum,size);
    }
}

/**
* @internal appDemoAllocateDmaMem function
* @endinternal
*
* @brief   This function allocates memory for phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
* @param[in] devType                  - The Pp device type to allocate the memory for.
* @param[in] rxDescNum                - Number of Rx descriptors (and buffers) to allocate.
* @param[in] rxBufSize                - Size of each Rx Buffer to allocate.
* @param[in] rxBufAllign              - Ammount of allignment required on the Rx buffers.
* @param[in] txDescNum                - Number of Tx descriptors to allocate.
* @param[in] auDescNum                - Number of address update descriptors to allocate.
* @param[in,out] ppPhase2Params           - The device's Phase2 parameters.
* @param[in,out] ppPhase2Params           - The device's Phase2 parameters including the required
*                                      allocations.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoAllocateDmaMem
(
    IN      CPSS_PP_DEVICE_TYPE         devType,
    IN      GT_U32                      rxDescNum,
    IN      GT_U32                      rxBufSize,
    IN      GT_U32                      rxBufAllign,
    IN      GT_U32                      txDescNum,
    IN      GT_U32                      auDescNum,
    INOUT   CPSS_PP_PHASE2_INIT_PARAMS  *ppPhase2Params
)
{
#ifdef IMPL_PP
    GT_STATUS                   rc;
    GT_U32                      tmpData, rxDescSize, ii;
    GT_U32                      *tmpPtr;
    GT_U32                      fuDescNum; /* number of FU descriptors ... allow to be diff then auDescNum */
    GT_CHAR                     txGenQueueParamStr[40];
    GT_BOOL                     txGenQueue[NUM_OF_SDMA_QUEUES];/* Enable Tx queue to work in generator mode */
    GT_U32                      txGenQueueBuffSize[NUM_OF_SDMA_QUEUES];/* in SDMA generator mode , the buffers size in the queue */
    GT_U32                      txGenNumOfDescBuffArr[NUM_OF_SDMA_QUEUES];/* in SDMA generator mode , the number of descriptors in the queue */
    GT_U32                      txGenNumOfDescBuff; /* Number of descriptors and buffers per Tx queue */
    GT_U32                      txGenBuffSize;      /* Size of buffer per Tx queue */
    GT_U32                      txGenDescSize;      /* Size of descriptor per Tx queue */
    GT_U32                      tmp_txGenBuffSize;  /* Size of buffer per Tx queue */
    GT_U32                      txQue,rxQue;              /* queue number */
    GT_U32                      useIpfixFlowManager = 0; /* 0 is used for disabling Ipfix Flow manager feature */
    GT_U8                       disableTxSdma = 6; /* Default tx queue used by fw in gtAppDemoIpFix.c file */
    GT_U8                       disableRxSdma = 6; /* Default rx queue used by fw in gtAppDemoIpFix.c file */
    GT_U8   devNum;
    char dbEntryName[64];
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    /* rxSdmaDesciptor amount adjust data */
    GT_U32 rxDescrNumArr[CPSS_MAX_SDMA_CPU_PORTS_CNS][8];
    GT_U32 cmdIdx;
    GT_U32 sdmaBase;
    GT_U32 sdmaBmp;
    GT_U32 queuesBmp;
    GT_U32 value;
    GT_U8 dmaWindow;/* support allocation of DMA from the specific DMA window (needed for SMMU systems)*/
    GT_U32  toCpu_numOfQueues,fromCpu_numOfQueues;
    GT_U32  globalQueue;

    GT_CHAR                     txSramParamStr[40];
    GT_BOOL                     txSramQueue[NUM_OF_SDMA_QUEUES];/* Enable Tx queue to send packets from SRAM */
    GT_U32                      txSramNumOfDesc[NUM_OF_SDMA_QUEUES];/* Number of TX descriptor in SRAM */

    if (appDemoCpssCurrentDevIndex >= APP_DEMO_PP_CONFIG_SIZE_CNS)
    {
        return GT_FAIL;
    }
    devNum = appDemoPpConfigList[appDemoCpssCurrentDevIndex].devNum;
#if defined(CONFIG_V2_DMA)
    /*
    * dmaWindow variable is used to pass parameter to appDemoMemCacheDmaMalloc
    * At the time of removing V1 DMA code, replace dmaWindow with devNum
    */
    dmaWindow = devNum;
#else
    dmaWindow = appDemoPpConfigList[appDemoCpssCurrentDevIndex].dmaWindow;
#endif
    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    toCpu_numOfQueues   = 8;
    fromCpu_numOfQueues = 8;

    if(CPSS_IS_DXCH_FAMILY_MAC(appDemoPpConfigList[devNum].devFamily))
    {
#ifdef CHX_FAMILY
        rc = prvCpssDxChNetIfNumQueuesGet(devNum,
            &toCpu_numOfQueues,
            &fromCpu_numOfQueues);
        if(rc != GT_OK)
        {
            cpssOsPrintf("prvCpssDxChNetIfNumQueuesGet : device[%d] did HW reset but the prvCpssPpConfigDevDbInfoGet(...) not failed \n",
                devNum);
            return rc;
        }
#endif /*CHX_FAMILY*/
    }

    if(appDemoDbEntryGet("auDescNum", &tmpData) == GT_OK)
    {
        /* allow to update the default of the auDescNum */
        auDescNum = tmpData;
    }

    /* default and possible override */
    fuDescNum = auDescNum;
    if(appDemoDbEntryGet("fuDescNum", &tmpData) == GT_OK)
    {
        /* allow to update the default of the auDescNum */
        fuDescNum = tmpData;
    }

    if(appDemoDbEntryGet("fuqUseSeparate", &tmpData) == GT_OK)
    {
        ppPhase2Params->fuqUseSeparate = (CPSS_PP_PHASE2_UPLOAD_QUEUE_TYPE_ENT)tmpData;
    }

    if(appDemoDbEntryGet("rxDescNum", &tmpData) == GT_OK)
    {
        /* allow to update the default of the rxDescNum */
        rxDescNum = tmpData;
    }

    if(appDemoDbEntryGet("txDescNum", &tmpData) == GT_OK)
    {
        /* allow to update the default of the txDescNum */
        txDescNum = tmpData;
    }

    if(appDemoDbEntryGet("useDoubleAuq", &tmpData) == GT_OK)
    {
        /* allow to work with double AUQ */
        ppPhase2Params->useDoubleAuq = tmpData;
    }

    if(appDemoDbEntryGet("useSecondaryAuq", &tmpData) == GT_OK)
    {
        /* allow to work with secondary AUQ */
        ppPhase2Params->useSecondaryAuq = tmpData;
    }

    if(appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_OK)
    {
        /* allow to work in Tx queue generator mode */
        ppPhase2Params->useMultiNetIfSdma = tmpData;
    }

    if(appDemoDbEntryGet("dma_reduce_factor", &tmpData) == GT_OK && tmpData)
    {
        osPrintf("Reduce the DMA allocations by factor[%d] (to support multiple devices/tiles) \n",tmpData);
        rxDescNum /= tmpData;
        txDescNum /= tmpData;
        auDescNum /= tmpData;
    }


    for(txQue = 0; txQue < NUM_OF_SDMA_QUEUES; txQue++)
    {
        txGenQueue[txQue] = GT_FALSE;
        txGenQueueBuffSize[txQue] = 0;
        txGenNumOfDescBuffArr[txQue] = 0;
        osSprintf(txGenQueueParamStr, "txGenQueue_%d", txQue);
        if(appDemoDbEntryGet(txGenQueueParamStr, &tmpData) == GT_OK)
        {
            /* Enable Tx queue to work in Tx queue generator mode */
            txGenQueue[txQue] = GT_TRUE;


            osSprintf(txGenQueueParamStr, "txGenBuffSize_%d", txQue);
            if(appDemoDbEntryGet(txGenQueueParamStr, &tmpData) == GT_OK)
            {
                /* explicit size for the buffers in this queue  */
                txGenQueueBuffSize[txQue] = tmpData;
            }

            osSprintf(txGenQueueParamStr, "txGenNumOfDescBuff_%d", txQue);
            if(appDemoDbEntryGet(txGenQueueParamStr, &tmpData) == GT_OK)
            {
                /* explicit number of descriptors for this queue  */
                txGenNumOfDescBuffArr[txQue] = tmpData;
            }
        }

        txSramQueue[txQue] = GT_FALSE;
        txSramNumOfDesc[txQue] = 0;

        if(appDemoPpConfigList[appDemoCpssCurrentDevIndex].devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E ||
            appDemoPpConfigList[appDemoCpssCurrentDevIndex].devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E ||
            appDemoPpConfigList[appDemoCpssCurrentDevIndex].devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
            osSprintf(txSramParamStr, "txSramQueue_%d", txQue);
            if(appDemoDbEntryGet(txSramParamStr, &tmpData) == GT_OK)
            {
                /* Enable Tx queue to send packets from SRAM */
                txSramQueue[txQue] = GT_TRUE;

                osSprintf(txSramParamStr, "txSramNumOfDesc_%d", txQue);
                if(appDemoDbEntryGet(txSramParamStr, &tmpData) == GT_OK)
                {
                    /* Number of TX descriptor in SRAM */
                    txSramNumOfDesc[txQue] = tmpData;
                }
            }
        }
    }

    if(appDemoDbEntryGet("txGenNumOfDescBuff", &tmpData) == GT_OK)
    {
        /* Number of descriptors and buffers */
        txGenNumOfDescBuff = tmpData;
    }
    else
    {
        /* Default number of descriptors and buffers */
        txGenNumOfDescBuff = 512;
    }

    if(appDemoDbEntryGet("txGenBuffSize", &tmpData) == GT_OK)
    {
        /* Number of descriptors and buffers */
        txGenBuffSize = tmpData;
    }
    else
    {
        /* Default number of descriptors and buffers */
        txGenBuffSize = 128 + 16; /* 16 bytes of eDsa tag */
    }
    osSprintf(dbEntryName,"noSdmaRxTxPP%d", devNum);
    if((appDemoDbEntryGet(dbEntryName, &tmpData) == GT_OK) && (tmpData != 0))
    {
        /* Disable Rx/Tx SDMA for selected devNum */
        rxDescNum = 0;
        txDescNum = 0;
        rxBufSize = 0;
        txGenNumOfDescBuff = 0;
        txGenBuffSize = 0;
        ppPhase2Params->useMultiNetIfSdma = GT_FALSE;
        ppPhase2Params->netIfCfg.rxBufInfo.allocMethod = CPSS_RX_BUFF_NO_ALLOC_E;
        appDemoPpConfigList[appDemoCpssCurrentDevIndex].cpuPortMode = CPSS_NET_CPU_PORT_MODE_NONE_E;
        for(txQue = 0; txQue < NUM_OF_SDMA_QUEUES; txQue++)
            txGenQueue[txQue] = GT_FALSE;
    }

    if(appDemoDbEntryGet("useIpfixFlowManager", &tmpData) == GT_OK)
    {
        /* allow to work in Ipfix Flow manager mode */
        useIpfixFlowManager = tmpData;
    }

    /* Au block size calc & malloc  */
    coreGetAuDescSize(devType,&tmpData);

    if (auDescNum == 0)
    {
        ppPhase2Params->auqCfg.auDescBlock = 0;
        ppPhase2Params->auqCfg.auDescBlockSize = 0;
    }
    else
    {
        ppPhase2Params->auqCfg.auDescBlockSize = tmpData * auDescNum;

        ppPhase2Params->auqCfg.auDescBlock =
            appDemoMemCacheDmaMalloc(dmaWindow,ppPhase2Params->auqCfg.auDescBlockSize +
                            tmpData);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
        if(ppPhase2Params->auqCfg.auDescBlock == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }
        if(((GT_UINTPTR)ppPhase2Params->auqCfg.auDescBlock) % tmpData)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            ppPhase2Params->auqCfg.auDescBlockSize += tmpData;
        }
        appDemoPpConfigList[devNum].allocDmaInfo.auqCfg.auDescBlock=ppPhase2Params->auqCfg.auDescBlock;
    }

    if (fuDescNum == 0)
    {
        ppPhase2Params->fuqCfg.auDescBlock = 0;
        ppPhase2Params->fuqCfg.auDescBlockSize = 0;
    }
    else
    {
        /* Fu block size calc & malloc  */
        coreGetAuDescSize(devType,&tmpData);
        ppPhase2Params->fuqCfg.auDescBlockSize = tmpData * fuDescNum;

        ppPhase2Params->fuqCfg.auDescBlock =
            appDemoMemCacheDmaMalloc(dmaWindow,ppPhase2Params->fuqCfg.auDescBlockSize +
                            tmpData);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
        if(ppPhase2Params->fuqCfg.auDescBlock == NULL)
        {
            return GT_OUT_OF_CPU_MEM;
        }

        if(((GT_UINTPTR)ppPhase2Params->fuqCfg.auDescBlock) % tmpData)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            ppPhase2Params->fuqCfg.auDescBlockSize += tmpData;
        }
         appDemoPpConfigList[devNum].allocDmaInfo.fuqCfg.auDescBlock=ppPhase2Params->fuqCfg.auDescBlock;
    }

    /* Tx block size calc & malloc  */
    if(ppPhase2Params->useMultiNetIfSdma == GT_FALSE)
    {
        appDemoGetTxDescSize(devType,&tmpData);
        ppPhase2Params->netIfCfg.txDescBlockSize = tmpData * txDescNum;
        if (ppPhase2Params->netIfCfg.txDescBlockSize != 0)
        {
            ppPhase2Params->netIfCfg.txDescBlock =
                appDemoMemCacheDmaMalloc(dmaWindow,ppPhase2Params->netIfCfg.txDescBlockSize);
            if(ppPhase2Params->netIfCfg.txDescBlock == NULL)
            {
                osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                return GT_OUT_OF_CPU_MEM;
            }
        }
        if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
           ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
              (system_recovery.systemRecoveryMode.continuousTx == GT_FALSE)) )
        {
            if (ppPhase2Params->netIfCfg.txDescBlockSize != 0)
            {
                osMemSet(ppPhase2Params->netIfCfg.txDescBlock, 0,
                         ppPhase2Params->netIfCfg.txDescBlockSize);
            }
        }

        /* Rx block size calc & malloc  */
        coreGetRxDescSize(devType,&tmpData);

        ppPhase2Params->netIfCfg.rxDescBlockSize = tmpData * rxDescNum;
        if (ppPhase2Params->netIfCfg.rxDescBlockSize != 0)
        {
            ppPhase2Params->netIfCfg.rxDescBlock =
                appDemoMemCacheDmaMalloc(dmaWindow,ppPhase2Params->netIfCfg.rxDescBlockSize);
            if(ppPhase2Params->netIfCfg.rxDescBlock == NULL)
            {
                osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                osCacheDmaFree(ppPhase2Params->netIfCfg.txDescBlock);
                return GT_OUT_OF_CPU_MEM;
            }
           appDemoPpConfigList[devNum].allocDmaInfo.netIfCfg.rxDescBlock=ppPhase2Params->netIfCfg.rxDescBlock;
        }
        if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
           ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
              (system_recovery.systemRecoveryMode.continuousRx == GT_FALSE)) )
        {
            if (ppPhase2Params->netIfCfg.rxDescBlockSize != 0)
            {
                osMemSet(ppPhase2Params->netIfCfg.rxDescBlock,0,
                         ppPhase2Params->netIfCfg.rxDescBlockSize);
            }
        }

        /* init the Rx buffer allocation method */
        /* Set the system's Rx buffer size.     */
        if((rxBufSize % rxBufAllign) != 0)
        {
            rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
        }

        if (ppPhase2Params->netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
        {
            ppPhase2Params->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize =
            rxBufSize * rxDescNum;

            /* set status of RX buffers - cacheable or not */
            ppPhase2Params->netIfCfg.rxBufInfo.buffersInCachedMem = rxBuffersInCachedMem;

            if ((rxBufSize * rxDescNum) != 0)
            {
                /* If RX buffers should be cachable - allocate it from regular memory */
                if (GT_TRUE == rxBuffersInCachedMem)
                {
                    tmpPtr = osMalloc(((rxBufSize * rxDescNum) + rxBufAllign-1));
                }
                else
                {
                    tmpPtr = appDemoMemCacheDmaMalloc(dmaWindow,((rxBufSize * rxDescNum) + rxBufAllign-1));
                }

                if(tmpPtr == NULL)
                {
                    osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                    osCacheDmaFree(ppPhase2Params->netIfCfg.txDescBlock);
                    osCacheDmaFree(ppPhase2Params->netIfCfg.rxDescBlock);
                    return GT_OUT_OF_CPU_MEM;
                }
            }
            else
            {
                tmpPtr = NULL;
            }

            if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
            {
                tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                   (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
            }
            ppPhase2Params->netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = tmpPtr;
        }
        else if (ppPhase2Params->netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
        {
            /* do not allocate rx buffers*/
        }
        else
        {
            /* dynamic RX buffer allocation currently is not supported by appDemo*/
            osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
            osCacheDmaFree(ppPhase2Params->netIfCfg.txDescBlock);
            osCacheDmaFree(ppPhase2Params->netIfCfg.rxDescBlock);
            return GT_NOT_SUPPORTED;
        }
    }

    if(ppPhase2Params->useMultiNetIfSdma)
    {
        CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC  * sdmaQueuesConfigPtr;
        CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC  * sdmaRxQueuesConfigPtr;
        /* Tx block size calc & malloc  */
        appDemoGetTxDescSize(devType,&txGenDescSize);

        for(globalQueue = 0; globalQueue < fromCpu_numOfQueues; globalQueue++)
        {
            txQue = globalQueue % 8;
            ii    = globalQueue / 8;

            if(txSramQueue[(ii*8) + txQue] == GT_TRUE)
                continue;

            sdmaQueuesConfigPtr = &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
            /* need to make this array per mg per q:*/
            if(txGenQueue[(ii*8) + txQue] == GT_FALSE)
            {
                /* Tx block size calc & malloc  */
                sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E;
                sdmaQueuesConfigPtr->numOfTxBuff = 0; /*not relevant in non traffic generator mode*/
                sdmaQueuesConfigPtr->buffSize = 0; /*not relevant in non traffic generator mode*/
                /* Block TxSdmaQueue for flow manager feature */
                if( (useIpfixFlowManager == 1) && ( ii == 0)  && (disableTxSdma == txQue) )
                {
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                    continue;
                }
                sdmaQueuesConfigPtr->numOfTxDesc = txDescNum ? (txDescNum/8 ) : 0; /*125 */
                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize = (sdmaQueuesConfigPtr->numOfTxDesc * txGenDescSize);
                if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                {
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                        appDemoMemCacheDmaMalloc(dmaWindow,sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                    if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                    {
                        osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                        return GT_OUT_OF_CPU_MEM;
                    }

                    appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr =
                                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr;
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                }
                if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
                   ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                     (system_recovery.systemRecoveryMode.continuousTx == GT_FALSE)) )
                {
                    osMemSet(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr,0,
                             sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                }
            }
            else
            {/* Generator mode */
                tmp_txGenBuffSize = (txGenBuffSize == 0) || (txGenQueueBuffSize[(ii*8) + txQue] == 0) ?
                    txGenBuffSize :
                    txGenQueueBuffSize[(ii*8) + txQue] + 16;

                sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                if(txGenNumOfDescBuffArr[(ii*8) + txQue])
                {
                    sdmaQueuesConfigPtr->numOfTxDesc = txGenNumOfDescBuffArr[(ii*8) + txQue];
                }
                else
                {
                    sdmaQueuesConfigPtr->numOfTxDesc = txGenNumOfDescBuff;
                }

                sdmaQueuesConfigPtr->numOfTxDesc /= 2;

                sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E;
                sdmaQueuesConfigPtr->numOfTxBuff = sdmaQueuesConfigPtr->numOfTxDesc;

                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize =
                    (sdmaQueuesConfigPtr->numOfTxDesc + 1) * (txGenDescSize + tmp_txGenBuffSize);
                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
                    appDemoMemCacheDmaMalloc(dmaWindow,sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                {
                    osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                    return GT_OUT_OF_CPU_MEM;
                }
                appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr =
                                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr;
                sdmaQueuesConfigPtr->buffSize = tmp_txGenBuffSize;
            }
        }

        for(globalQueue = 0; globalQueue < fromCpu_numOfQueues; globalQueue++)
        {
            txQue = globalQueue % 8;
            ii    = globalQueue / 8;

            sdmaQueuesConfigPtr = &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
            /* need to make this array per mg per q:*/
            if(txSramQueue[(ii*8) + txQue] == GT_TRUE)
            {
                /* Tx block size calc & malloc  */
                sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E;
                sdmaQueuesConfigPtr->numOfTxBuff = 0; /*not relevant in non traffic generator mode*/
                sdmaQueuesConfigPtr->buffSize = 0; /*not relevant in non traffic generator mode*/
                /* Block TxSdmaQueue for flow manager feature */
                if( (useIpfixFlowManager == 1) && ( ii == 0)  && (disableTxSdma == txQue) )
                {
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                    continue;
                }
                sdmaQueuesConfigPtr->numOfTxDesc = txSramNumOfDesc[(ii*8) + txQue];
                sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize = (sdmaQueuesConfigPtr->numOfTxDesc * txGenDescSize);
                if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                {
                    /* CPSS will find free space in SRAM for TX desc and buffers */
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr = NULL;

                    appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr =
                                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr;
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                }
            }
        }

        /* Rx block size calc & malloc  */

        /* Debug environment commands to reorder RX SDMA descripters between SDMA/Ques */
        for(globalQueue = 0; globalQueue < toCpu_numOfQueues; globalQueue++)
        {
            rxQue = globalQueue % 8;
            ii    = globalQueue / 8;
            rxDescrNumArr[ii][rxQue] = rxDescNum/8;
        }

        for (cmdIdx = 0; (1); cmdIdx++)
        {
            osSprintf(dbEntryName,"rxSdmaDescrFix_%d_Add", cmdIdx);
            if ((appDemoDbEntryGet(dbEntryName, &tmpData) != GT_OK)) break;
            sdmaBase        = (tmpData >> 24) & 0xFF;
            sdmaBmp         = (tmpData >> 16) & 0xFF;
            queuesBmp       = (tmpData >>  8) & 0xFF;
            value           = tmpData & 0xFF;
            for (ii = sdmaBase; (ii < (sdmaBase + 8)); ii++)
            {
                if (ii >= CPSS_MAX_SDMA_CPU_PORTS_CNS) break;
                if ((sdmaBmp & (1 << (ii - sdmaBase))) == 0) continue;
                for (rxQue = 0; (rxQue < 8); rxQue++)
                {
                    if ((queuesBmp & (1 << rxQue)) == 0) continue;
                    rxDescrNumArr[ii][rxQue] += value;
                }
            }
        }
        for (cmdIdx = 0; (1); cmdIdx++)
        {
            osSprintf(dbEntryName,"rxSdmaDescrFix_%d_Sub", cmdIdx);
            if ((appDemoDbEntryGet(dbEntryName, &tmpData) != GT_OK)) break;
            sdmaBase        = (tmpData >> 24) & 0xFF;
            sdmaBmp         = (tmpData >> 16) & 0xFF;
            queuesBmp       = (tmpData >>  8) & 0xFF;
            value           = tmpData & 0xFF;
            for (ii = sdmaBase; (ii < (sdmaBase + 8)); ii++)
            {
                if (ii >= CPSS_MAX_SDMA_CPU_PORTS_CNS) break;
                if ((sdmaBmp & (1 << (ii - sdmaBase))) == 0) continue;
                for (rxQue = 0; (rxQue < 8); rxQue++)
                {
                    if ((queuesBmp & (1 << rxQue)) == 0) continue;
                    if (rxDescrNumArr[ii][rxQue] >= value)
                    {
                        rxDescrNumArr[ii][rxQue] -= value;
                    }
                    else
                    {
                        rxDescrNumArr[ii][rxQue] = 0;
                    }
                }
            }
        }

        coreGetRxDescSize(devType,&rxDescSize);

        for(globalQueue = 0; globalQueue < toCpu_numOfQueues; globalQueue++)
        {
            rxQue = globalQueue % 8;
            ii    = globalQueue / 8;

            sdmaRxQueuesConfigPtr = &ppPhase2Params->multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue];
            sdmaRxQueuesConfigPtr->buffAllocMethod = CPSS_RX_BUFF_STATIC_ALLOC_E;
            sdmaRxQueuesConfigPtr->buffersInCachedMem = rxBuffersInCachedMem;
            sdmaRxQueuesConfigPtr->numOfRxDesc = rxDescrNumArr[ii][rxQue];
            /* Block RxSdmaQueue for flow manager feature */
            if( (useIpfixFlowManager == 1) && ( ii == 0) && (disableRxSdma == rxQue) )
            {
                continue;
            }
            sdmaRxQueuesConfigPtr->descMemSize = sdmaRxQueuesConfigPtr->numOfRxDesc * rxDescSize;
            sdmaRxQueuesConfigPtr->descMemPtr = appDemoMemCacheDmaMalloc(dmaWindow,sdmaRxQueuesConfigPtr->descMemSize);
            if(sdmaRxQueuesConfigPtr->descMemPtr == NULL)
            {
                osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                for(txQue = 0; txQue < 8; txQue++)
                {
                    sdmaQueuesConfigPtr = &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                    if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                    {
                        osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                    }
                }

                return GT_OUT_OF_CPU_MEM;
            }
            appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].descMemPtr =
                                    sdmaRxQueuesConfigPtr->descMemPtr;
            /* init the Rx buffer allocation method */
            /* Set the system's Rx buffer size.     */
            if((rxBufSize % rxBufAllign) != 0)
            {
                rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
            }


            sdmaRxQueuesConfigPtr->buffHeaderOffset = ppPhase2Params->headerOffset; /* give the same offset to all queues*/
            sdmaRxQueuesConfigPtr->buffSize = rxBufSize;
            sdmaRxQueuesConfigPtr->numOfRxBuff = rxDescrNumArr[ii][rxQue]; /* by default the number of buffers equel the number of descriptors*/
            sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize = (rxBufSize * sdmaRxQueuesConfigPtr->numOfRxBuff);
            if (sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize != 0)
            {
                /* If RX buffers should be cachable - allocate it from regular memory */
                if (GT_TRUE == rxBuffersInCachedMem)
                {
                    tmpPtr = osMalloc((sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
                }
                else
                {
                    tmpPtr = appDemoMemCacheDmaMalloc(dmaWindow,(sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
                }

                if(tmpPtr == NULL)
                {
                    osCacheDmaFree(ppPhase2Params->auqCfg.auDescBlock);
                    for(txQue = 0; txQue < 8; txQue++)
                    {
                        sdmaQueuesConfigPtr = &ppPhase2Params->multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                        if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                        {
                            osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                        }
                    }
                    osCacheDmaFree(sdmaRxQueuesConfigPtr->descMemPtr);
                    return GT_OUT_OF_CPU_MEM;
                }
            }
            else
            {
                tmpPtr = NULL;
            }

            if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
            {
                tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                   (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
            }
            sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr = tmpPtr;
           appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].memData.staticAlloc.buffMemPtr=sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr;

            if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
               ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                 (system_recovery.systemRecoveryMode.continuousRx == GT_FALSE)) )
            {
                osMemSet(sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr,0,
                     sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize);
            }
        }
    }

#endif

    return GT_OK;
}

/**
* @internal appDemoGetPpRegCfgList function
* @endinternal
*
* @brief   This function returns the list of registers to be configured to a given
*         device before and after the startInit operation. (Passed to
*         corePpHwStartInit() ).
*
* @param[out] regCfgList               - A pointer to the register list.
* @param[out] regCfgListSize           - Number of elements in regListPtr.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoGetPpRegCfgList
(
    IN  CPSS_PP_DEVICE_TYPE         devType,
    IN  GT_BOOL                     isB2bSystem,
    OUT CPSS_REG_VALUE_INFO_STC     **regCfgList,
    OUT GT_U32                      *regCfgListSize
)
{
    GT_UNUSED_PARAM(devType);
    GT_UNUSED_PARAM(isB2bSystem);

    *regCfgList     = dummyRegValInfoList;
    *regCfgListSize = (sizeof(dummyRegValInfoList) /
                       sizeof(CPSS_REG_VALUE_INFO_STC));
    return GT_OK;
}

#ifdef IMPL_PP
/**
* @internal coreGetRxDescSize function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Rx descriptor,
*         for a given device.
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] descSize                 The descrptor's size (in bytes).
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
static GT_STATUS coreGetRxDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
)
{
#if defined CHX_FAMILY
    return cpssDxChHwRxDescSizeGet(devType, descSize);
#else
    GT_UNUSED_PARAM(devType);
    *descSize = 0;
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}
#endif /* IMPL_PP */


/**
* @internal appDemoGetTxDescSize function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Tx descriptor,
*         for a given device.
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] descSize                 The descrptor's size (in bytes).
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
extern GT_STATUS appDemoGetTxDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
)
{
#if defined CHX_FAMILY
    return cpssDxChHwTxDescSizeGet(devType, descSize);
#else
    GT_UNUSED_PARAM(devType);
    *descSize = 0;
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

#ifdef IMPL_PP
/**
* @internal oreGetAuDescSize function
* @endinternal
*
* @brief   This function returns the size in bytes of a single Address Update
*         descriptor, for a given device.
* @param[in] devType                  - The PP's device type to return the descriptor's size for.
*
* @param[out] descSize                 The descrptor's size (in bytes).
*                                       GT_OK on success,
*                                       GT_FAIL otherwise.
*/
static GT_STATUS coreGetAuDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
)
{
#if defined CHX_FAMILY
    return cpssDxChHwAuDescSizeGet(devType, descSize);
#else
    GT_UNUSED_PARAM(devType);
    *descSize = 0;
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}
#endif /* IMPL_PP */

/**
* @internal appDemoSetPortPhyAddr function
* @endinternal
*
* @brief   Configure the port's default phy address, this function should be used
*         to change the default port's phy address.
* @param[in] devNum                   - The Pp device number.
* @param[in] portNum                  - The port number to update the phy address for.
* @param[in] phyAddr                  - The new phy address, only the lower 5 bits of this param are
*                                      relevant.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This function should be called after corePpHwPhase1Init(), and before
*       gtPortSmiInit() (in Tapi level).
*
*/
GT_STATUS appDemoSetPortPhyAddr
(
    IN  GT_U8   devNum,
    IN  GT_U8   portNum,
    IN  GT_U8   phyAddr
)
{
#ifndef IMPL_PP
    return GT_OK;
#else /*IMPL_PP*/
    GT_STATUS rc = GT_NOT_IMPLEMENTED;

    /* check if device is active */
    if(appDemoPpConfigList[devNum].valid == GT_FALSE)
    {
        return GT_FAIL;
    }

    if(CPSS_IS_DXCH_FAMILY_MAC(appDemoPpConfigList[devNum].devFamily))
    {
#if (defined CHX_FAMILY)
        rc = cpssDxChPhyPortAddrSet(devNum,portNum,phyAddr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortAddrSet", rc);
        return rc;
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portNum);
    GT_UNUSED_PARAM(phyAddr);
#endif /*(defined CHX_FAMILY)*/
    }

    return rc;

#endif /*IMPL_PP*/
}

/**
* @internal appDemoGetRegisterWriteTime function
* @endinternal
*
* @brief   Get the time needed for register write consecutive operations
*         as stated in the request.
* @param[in] devNum                   - the device number to write to.
* @param[in] regAddr1                 - first register address to write to.
* @param[in] regAddr2                 - second register address to write to.
* @param[in] regAddr3                 - third register address to write to.
* @param[in] regData                  - the data to write to the register.
* @param[in] numOfWrites              - the number of times to perform the register write operation.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoGetRegisterWriteTime
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr1,
    IN GT_U32 regAddr2,
    IN GT_U32 regAddr3,
    IN GT_U32 regData,
    IN GT_U32 numOfWrites
)
{
    GT_STATUS rc;
    GT_U32 writeTime;
    GT_U32 i;

    writeTime = osTime();
    for( i = 0 ; i< numOfWrites ; i++)
    {
        rc = cpssDrvPpHwRegisterWrite(devNum,
                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      regAddr1,
                                      regData);
        if( GT_OK != rc )
            return rc;

        rc = cpssDrvPpHwRegisterWrite(devNum,
                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      regAddr2,
                                      regData);
        if( GT_OK != rc )
            return rc;

        rc = cpssDrvPpHwRegisterWrite(devNum,
                                      CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                      regAddr3,
                                      regData);
        if( GT_OK != rc )
            return rc;
    }
    writeTime = osTime() - writeTime;

    osPrintf("%d register write operations took %d Sec.\n", 3*numOfWrites, writeTime);

    return GT_OK;
}

/**
* @internal appDemoGetRegisterReadTime function
* @endinternal
*
* @brief   Get the time needed for register read consecutive operations
*         as stated in the request.
* @param[in] devNum                   - the device number to read from.
* @param[in] regAddr1                 - first register address to read.
* @param[in] regAddr2                 - second register address to read.
* @param[in] regAddr3                 - third register address to read.
* @param[in] numOfReads               - the number of times to perform the register read operation.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoGetRegisterReadTime
(
    IN GT_U8 devNum,
    IN GT_U32 regAddr1,
    IN GT_U32 regAddr2,
    IN GT_U32 regAddr3,
    IN GT_U32 numOfReads
)
{
    GT_STATUS rc;
    GT_U32 regData;
    GT_U32 readTime;
    GT_U32 i;

    readTime = osTime();
    for( i = 0 ; i< numOfReads ; i++)
    {
        rc = cpssDrvPpHwRegisterRead(devNum,
                                     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                     regAddr1,
                                     &regData);
        if( GT_OK != rc )
            return rc;

        rc = cpssDrvPpHwRegisterRead(devNum,
                                     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                     regAddr2,
                                     &regData);
        if( GT_OK != rc )
            return rc;

        rc = cpssDrvPpHwRegisterRead(devNum,
                                     CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                     regAddr3,
                                     &regData);
        if( GT_OK != rc )
            return rc;
    }
    readTime = osTime() - readTime;

    osPrintf("%d register read operations took %d Sec.\n", 3*numOfReads, readTime);

    return GT_OK;
}

#define BAD_VALUE (GT_U32)(~0)

typedef struct
{
    GT_U32              speedMbps;
    CPSS_PORT_SPEED_ENT speedEnm;
}APPDEMO_SPEED_MBPS_2_CPSS_SPEED_STC;
static APPDEMO_SPEED_MBPS_2_CPSS_SPEED_STC prv_speed2enm[] =
{
     {        10  ,  CPSS_PORT_SPEED_10_E      }
    ,{       100  ,  CPSS_PORT_SPEED_100_E     }
    ,{      1000  ,  CPSS_PORT_SPEED_1000_E    }
    ,{      2500  ,  CPSS_PORT_SPEED_2500_E    }
    ,{      5000  ,  CPSS_PORT_SPEED_5000_E    }
    ,{     10000  ,  CPSS_PORT_SPEED_10000_E   }
    ,{     11800  ,  CPSS_PORT_SPEED_11800_E   }
    ,{     12000  ,  CPSS_PORT_SPEED_12000_E   }
    ,{     12500  ,  CPSS_PORT_SPEED_12500_E   }
    ,{     13600  ,  CPSS_PORT_SPEED_13600_E   }
    ,{     15000  ,  CPSS_PORT_SPEED_15000_E   }
    ,{     16000  ,  CPSS_PORT_SPEED_16000_E   }
    ,{     20000  ,  CPSS_PORT_SPEED_20000_E   }
    ,{     23600  ,  CPSS_PORT_SPEED_23600_E   }
    ,{     24000  ,  CPSS_PORT_SPEED_23600_E   }
    ,{     25000  ,  CPSS_PORT_SPEED_25000_E   }
    ,{     40000  ,  CPSS_PORT_SPEED_40000_E   }
    ,{     47200  ,  CPSS_PORT_SPEED_47200_E   }
    ,{     48000  ,  CPSS_PORT_SPEED_47200_E   }
    ,{     50000  ,  CPSS_PORT_SPEED_50000_E   }
    ,{     52500  ,  CPSS_PORT_SPEED_52500_E   }
    ,{     75000  ,  CPSS_PORT_SPEED_75000_E   }
    ,{    100000  ,  CPSS_PORT_SPEED_100G_E    }
    ,{    140000  ,  CPSS_PORT_SPEED_140G_E    }
    ,{     26700  ,  CPSS_PORT_SPEED_26700_E   }
    ,{ BAD_VALUE  ,  CPSS_PORT_SPEED_NA_E      }
};

CPSS_PORT_SPEED_ENT CPSS_SPEED_Mbps_2_ENM
(
    GT_U32 speedMbps
)
{
    GT_U32 i;
    for (i = 0 ; prv_speed2enm[i].speedEnm != CPSS_PORT_SPEED_NA_E; i++)
    {
        if (prv_speed2enm[i].speedMbps == speedMbps)
        {
            return prv_speed2enm[i].speedEnm;
        }
    }
    return CPSS_PORT_SPEED_NA_E;
}

 GT_U32 CPSS_SPEED_ENM_2_Mbps
(
    CPSS_PORT_SPEED_ENT speedEnm
)
{
    GT_U32 i;
    for (i = 0 ; prv_speed2enm[i].speedEnm != CPSS_PORT_SPEED_NA_E; i++)
    {
        if (prv_speed2enm[i].speedEnm == speedEnm)
        {
            return prv_speed2enm[i].speedMbps;
        }
    }
    return 0;
}

/*******************************************************************************
* u32_2_STR
*
* DESCRIPTION:
*       conver integer to string according to table
*
* APPLICABLE DEVICES:
*
* NOT APPLICABLE DEVICES:
*
* INPUTS:
*       u32_Num          - number
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       converter string , or string represtantion of integer value if not found
*
* COMMENTS:
*
*******************************************************************************/
GT_CHAR * u32_2_STR
(
    IN GT_U32 u32_Num,
    IN APPDEMO_GT_U32_2_STR_STC * tbl
)
{
    GT_U32 i;
    static GT_CHAR badFmt[10];
    for (i = 0 ; tbl[i].u32_Num != BAD_VALUE; i++)
    {
        if (tbl[i].u32_Num == u32_Num)
        {
            return tbl[i].u32_Str;
        }
    }
    cpssOsSprintf(badFmt,"-%d",u32_Num);
    return &badFmt[0];
}

GT_CHAR * RXDMA_IfWidth_2_STR
(
    IN GT_U32 RXDMA_IfWidth
)
{
    static APPDEMO_GT_U32_2_STR_STC prv_mappingTypeStr[] =
    {
             { 0,           "64b" }
            ,{ 2,           "256b" }
            ,{ 3,           "512b" }
            ,{ BAD_VALUE,   (GT_CHAR *)NULL }
    };
    return u32_2_STR(RXDMA_IfWidth,&prv_mappingTypeStr[0]);
}

GT_CHAR * TX_FIFO_IfWidth_2_STR
(
    IN GT_U32 TX_FIFO_IfWidth
)
{
    static APPDEMO_GT_U32_2_STR_STC prv_mappingTypeStr[] =
    {
         { 0,           "1B" }
        ,{ 3,           "8B" }
        ,{ 5,           "32B" }
        ,{ 6,           "64B" }
        ,{ BAD_VALUE,   (GT_CHAR *)NULL }
    };
    return u32_2_STR(TX_FIFO_IfWidth,&prv_mappingTypeStr[0]);
}

#define DIV_0_32   0x00000000
#define DIV_1_32   0x80000000
#define DIV_2_32   0x80008000
#define DIV_3_32   0x80200400
#define DIV_4_32   0x80808080
#define DIV_5_32   0x82081040
#define DIV_6_32   0x84208420
#define DIV_7_32   0x88442210
#define DIV_8_32   0x88888888
#define DIV_9_32   0x91224488
#define DIV_10_32  0x92489248
#define DIV_11_32  0xa4924924
#define DIV_12_32  0xa4a4a4a4
#define DIV_13_32  0xa94a5294
#define DIV_14_32  0xaa54aa54
#define DIV_15_32  0xaaaa5554
#define DIV_16_32  0xaaaaaaaa
#define DIV_17_32  0xd555aaaa
#define DIV_18_32  0xd5aad5aa
#define DIV_19_32  0xd6b5ad6a
#define DIV_20_32  0xdadadada
#define DIV_21_32  0xdb6db6da
#define DIV_22_32  0xedb6edb6
#define DIV_23_32  0xeeddbb76
#define DIV_24_32  0xeeeeeeee
#define DIV_25_32  0xf7bbddee
#define DIV_26_32  0xfbdefbde
#define DIV_27_32  0xfdf7efbe
#define DIV_28_32  0xfefefefe
#define DIV_29_32  0xffdffbfe
#define DIV_30_32  0xfffefffe
#define DIV_31_32  0xfffffffe

GT_CHAR * TXDMA_RateLimitResidueValue_2_STR
(
    IN GT_U32 TXDMA_residueValue
)
{
    static APPDEMO_GT_U32_2_STR_STC prv_mappingTypeStr[] =
    {
         {  DIV_0_32  , " 0/32" }
        ,{  DIV_1_32  , " 1/32" }
        ,{  DIV_2_32  , " 2/32" }
        ,{  DIV_3_32  , " 3/32" }
        ,{  DIV_4_32  , " 4/32" }
        ,{  DIV_5_32  , " 5/32" }
        ,{  DIV_6_32  , " 6/32" }
        ,{  DIV_7_32  , " 7/32" }
        ,{  DIV_8_32  , " 8/32" }
        ,{  DIV_9_32  , " 9/32" }
        ,{  DIV_10_32 , "10/32" }
        ,{  DIV_11_32 , "11/32" }
        ,{  DIV_12_32 , "12/32" }
        ,{  DIV_13_32 , "13/32" }
        ,{  DIV_14_32 , "14/32" }
        ,{  DIV_15_32 , "15/32" }
        ,{  DIV_16_32 , "16/32" }
        ,{  DIV_17_32 , "17/32" }
        ,{  DIV_18_32 , "18/32" }
        ,{  DIV_19_32 , "19/32" }
        ,{  DIV_20_32 , "20/32" }
        ,{  DIV_21_32 , "21/32" }
        ,{  DIV_22_32 , "22/32" }
        ,{  DIV_23_32 , "23/32" }
        ,{  DIV_24_32 , "24/32" }
        ,{  DIV_25_32 , "25/32" }
        ,{  DIV_26_32 , "26/32" }
        ,{  DIV_27_32 , "27/32" }
        ,{  DIV_28_32 , "28/32" }
        ,{  DIV_29_32 , "29/32" }
        ,{  DIV_30_32 , "30/32" }
        ,{  DIV_31_32 , "31/32" }
        ,{ BAD_VALUE,   (GT_CHAR *)NULL }
    };
    return u32_2_STR(TXDMA_residueValue,&prv_mappingTypeStr[0]);
}

#define PHY_1680M_NUM_OF_PORTS_CNS 8

APP_DEMO_QUAD_PHY_CFG_STC  macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_Array[] =
{
    { AD_ALL_PORTS,  22, 0x0000 } /* RW u1 P0-7 R22 H0000 */ /* ensure we on page 0 */
   ,{ AD_ALL_PORTS,   4, 0x01E1 } /* RW u1 P0-3 R4 H01E1 - restore default of register #4
                                     that may be overridden by BC2 B0 during power-up */
   /* ---------------------------------------*/
   /* Link indication config                 */
   /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,  22, 0x0003 } /* RW u1 P0-7 R22 h0003 */
   ,{ AD_ALL_PORTS,  16, 0x1771 } /* RW u1 P0-7 R16 h1771 */
   /* ---------------------------------------*/
   /* MACSec and PTP disable                 */
   /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,  22, 0x0012 } /* RW u1 P0-7 R22 h0012 */
   ,{ AD_ALL_PORTS,  27, 0x0000 } /* RW u1 P0-7 R27 h0000 */
   /* ---------------------------------------*/
   /* QSGMII Tx Amp change                   */
   /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,  22, 0x00FD } /* RW u1 P0-7 R22  h00FD */
   ,{ AD_ALL_PORTS,   8, 0x0B53 } /* RW u1 P0-7 R8   h0B53 */
   ,{ AD_ALL_PORTS,   7, 0x200D } /* RW u1 P0-7 R7   h200D */
#if 0     /* currently not relevenat for MACSEC disable */
    /*---------------------------------------*/
    /* PHY EEE Initialization                */
    /*---------------------------------------*/
   ,{ AD_ALL_PORTS,  22, 0x00FF } /* RW u1 P0-7 R22 h00FF */
   ,{ AD_ALL_PORTS,  17, 0xB030 } /* RW u1 P0-7 R17 hB030 */
   ,{ AD_ALL_PORTS,  16, 0x215C } /* RW u1 P0-7 R16 h215C */
#endif
   /* ---------------------------------------*/
   /* Power Down disable & Soft-Reset        */
   /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,  22, 0x0000 } /* RW u1 P0-7 R22 H0000 */
   ,{ AD_ALL_PORTS,  16, 0x3070 } /* RW u1 P0-7 R16 H3070 */ /* PHY power up (reg 0x10_0.2=0 and reg 0x0_0.11=0) */
   ,{ AD_ALL_PORTS,   0, 0x9140 } /* RW u1 P0-7 R0  H9140 */ /* Soft Reset */
};
GT_U32 macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_ArraySize =
    sizeof(macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_Array) / sizeof(macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_Array[0]);

static APP_DEMO_QUAD_PHY_CFG_STC  macSecPHY_88E1548P_RevA0_Init_Array_phase_1[] =
{
    /* for all/first port, regaddr, regvalue, delay if need in ms */
    { AD_ALL_PORTS,           22, 0x0000 } /* RW u1 P0-3 R22 H0000 */
   ,{ AD_ALL_PORTS,           4,  0x01E1 } /* RW u1 P0-3 R4 H01E1 - restore default of register #4
                                              that may be overridden by BC2 B0 during power-up */
    /*---------------------------------------*/
    /* MACSEC disable                        */
    /*---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x0012 } /* RW u1 P0-3 R22 H0012 */
   ,{ AD_ALL_PORTS,           27, 0x0000 } /* RW u1 P0-3 R27 H0000 */
#if 0     /* currently not relevenat for MACSEC disable */
    /*---------------------------------------*/
    /* PHY EEE Initialization                */
    /*---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x00FF } /* RW u1 P0-3 R22 h00FF */
   ,{ AD_ALL_PORTS,           17, 0x2148 } /* RW u1 P0-3 R17 h2148 */
   ,{ AD_ALL_PORTS,           16, 0x2144 } /* RW u1 P0-3 R16 h2144 */
   ,{ AD_ALL_PORTS,           17, 0xDC0C } /* RW u1 P0-3 R17 hDC0C */
   ,{ AD_ALL_PORTS,           16, 0x2159 } /* RW u1 P0-3 R16 h2159 */
#endif
    /* ---------------------------------------*/
    /* QSGMII Amp                             */
    /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x00FD }  /* RW u1 P0-3 R22 h00FD */
   ,{ AD_ALL_PORTS,           11, 0x1D70 }  /* RW u1 P0-3 R11 h1D70 */
    /* ---------------------------------------*/
    /* QSGMII block power down/up WA - ?      */
    /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x0004 }  /* RW u1 P0-3 R22 h0004 */
   ,{ AD_ALL_PORTS,           26, 0x3802 }  /* RW u1 P0-3 R26 h3802 , Wait 300ms */
   ,{  AD_BAD_PORT,            0, 0x0000 }  /* end seq */
};

static APP_DEMO_QUAD_PHY_CFG_STC  macSecPHY_88E1548P_RevA0_Init_Array_phase_2[] =
{
    { AD_ALL_PORTS,           26, 0x3002 }  /* RW u1 P0-3 R26 h3002 , Wait 200ms */
   ,{  AD_BAD_PORT,            0, 0x0000 }  /* end seq */
};

static APP_DEMO_QUAD_PHY_CFG_STC  macSecPHY_88E1548P_RevA0_Init_Array_phase_3[] =
{
    /*--------------------------------------------------*/
    /* Incorrect Q_ANEG configuration errata workaround */
    /*--------------------------------------------------*/
    {  AD_ALL_PORTS,           22, 0x0004 } /* RW u1 P0-3 R22 h0004 */
   ,{  AD_ALL_PORTS,            0, 0x9140 } /* RW u1 P0-3 R0  h9140 */
    /* ---------------------------------------*/
    /* Soft-Reset                             */
    /* ---------------------------------------*/
   ,{ AD_ALL_PORTS,           22, 0x0000 }/* RW u1 P0-3 R22 H0000 */
   ,{ AD_ALL_PORTS,           16, 0x3060 }/* RW u1 P0-7 R16 H3060 */ /* Disable Energy Detect - bits8:9 = 0 to make VCT working */
   ,{ AD_ALL_PORTS,            0, 0x9140 }/* RW u1 P0-3 R0  H9140 */
   ,{  AD_BAD_PORT,            0, 0x0000 }  /* end seq */
};

APP_DEMO_QUAD_PHY_CFG_STC_PHASE_STC macSecPHY_88E1548P_RevA0_Init_Array[] =
{
     { &macSecPHY_88E1548P_RevA0_Init_Array_phase_1[0], 300 }
    ,{ &macSecPHY_88E1548P_RevA0_Init_Array_phase_2[0], 200 }
    ,{ &macSecPHY_88E1548P_RevA0_Init_Array_phase_3[0],   0 }
    ,{ NULL                                           ,   0 }
};

/**
* @internal appDemoBoardExternalPhyConfig function
* @endinternal
*
* @brief   External PHY configuration.
*
* @param[in] devNum                   - device number
* @param[in] phyType                  - PHY type
* @param[in] smiInterface             - SMI interface
* @param[in] smiStartAddr             - address of SMI device to be configured
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad phyType or smiInterface
* @retval GT_NOT_READY             - SMI is busy
* @retval GT_HW_ERROR              - HW error
*/
GT_STATUS appDemoBoardExternalPhyConfig
(
    IN  GT_U8                        devNum,
    IN  GT_U32                       phyType,
    IN  CPSS_PHY_SMI_INTERFACE_ENT   smiInterface,
    IN  GT_U32                       smiStartAddr
)
{
    GT_STATUS rc;           /* return code */
    GT_U32 i, j, phaseId;   /* loop iterators */
    GT_U32 numOfConfigs;    /* number of PHY init sequence configurations */
    GT_U32 phyRegAddr;      /* PHY address */
    GT_U16 phyRegData;      /* PHY data */
    APP_DEMO_QUAD_PHY_CFG_STC *currentArrayPtr;
    APP_DEMO_QUAD_PHY_CFG_STC_PHASE_STC * phasePtr;


    if(phyType == 0)/* PHY1680M */
    {
        numOfConfigs = macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_ArraySize;

        for(i = 0; i < PHY_1680M_NUM_OF_PORTS_CNS; i++)
        {
            for(j = 0; j < numOfConfigs; j++)
            {
                phyRegAddr = macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_Array[j].phyRegAddr;
                phyRegData = macSec_PTP_disable_PHY_88E1680M_RevA0_RD_MSI_Init_Array[j].phyRegData;

                cpssOsPrintf("SMI_[%d]  SMI_addr_[%d]  PHY_REG=0x%04x  PHY_DATA=0x%04x\r\n", smiInterface, (smiStartAddr + i), phyRegAddr, phyRegData);

                rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, smiInterface, (smiStartAddr + i), phyRegAddr, phyRegData);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("\r\nEXT_PHY_ERROR: error in SMI write for SMI_[%d]  SMI_addr_[%d]  PHY_REG 0x%x \r\n", smiInterface, (smiStartAddr + i), phyRegAddr);
                    return rc;
                }
            }

            cpssOsPrintf("\r\n\r\n");
        }
    }
    else if(phyType == 1)/* PHY1548P */
    {
        for (phaseId = 1, phasePtr = &macSecPHY_88E1548P_RevA0_Init_Array[0]; phasePtr->seqPtr != NULL; phaseId++, phasePtr++)
        {
            currentArrayPtr = phasePtr->seqPtr;
            for (i = 0; currentArrayPtr->allPorts != AD_BAD_PORT; i++, currentArrayPtr++)  /* loop over commands */
            {
                for (j = 0; j < PHY_1548M_NUM_OF_PORTS_CNS; j++)
                {
                    phyRegAddr = (GT_U8)currentArrayPtr->phyRegAddr;
                    phyRegData = currentArrayPtr->phyRegData;

                    cpssOsPrintf("SMI_[%d]  SMI_addr_[%d]  PHY_REG=0x%04x  PHY_DATA=0x%04x\r\n", smiInterface, (smiStartAddr + j), phyRegAddr, phyRegData);

                    rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, smiInterface, (smiStartAddr + j), phyRegAddr, phyRegData);
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPhyPortSmiRegisterWrite", rc);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintf("\n-->ERROR : bobkExternalBoardPhyConfig() failure: phase = %2d PP port[%d] Reg[%d] Data[%04X]\n", phaseId,i, phyRegAddr, phyRegData);
                        return rc;
                    }
                }
            }
            cpssOsPrintf("\r\n\r\n");

            /* whether there is a need for a delay */
            if (phasePtr->sleepTimeAfter_ms > 0)
            {
                cpssOsTimerWkAfter(phasePtr->sleepTimeAfter_ms);
            }
        }
    }
    else
    {
        cpssOsPrintf("\r\nEXT_PHY_ERROR: illegal PHY type - %d\r\n", phyType);
        return GT_BAD_PARAM;
    }

    return GT_OK;
}


GT_STATUS appDemoHwDriverRead(const char *path,GT_U32 as,GT_U32 reg)
{
    CPSS_HW_DRIVER_STC *drv = cpssHwDriverLookup(path);
    GT_U32  data;
    GT_U32  rc;
    if (!drv)
    {
        osPrintf("driver not found\n");
        return GT_FAIL;
    }
    rc = drv->read(drv, as, reg, &data, 1);
    if (rc == GT_OK)
    {
        osPrintf("data=0x%x\n",data);
    }
    return rc;
}

GT_STATUS appDemoHwDriverWrite(const char *path,GT_U32 as,GT_U32 reg,GT_U32 data)
{
    CPSS_HW_DRIVER_STC *drv = cpssHwDriverLookup(path);
    if (!drv)
    {
        osPrintf("driver not found\n");
        return GT_FAIL;
    }
    return drv->writeMask(drv, as, reg, &data, 1, 0xffffffff);
}

GT_STATUS appDemoSmiScan(const char *path)
{
    GT_U32 data, phy;
    CPSS_HW_DRIVER_STC *smi = cpssHwDriverLookup(path);
    if (!smi)
    {
        osPrintf("SMI driver not found\n");
        return GT_FAIL;
    }
    for (phy = 0; phy < 0x20; phy++)
    {
        if (smi->read(smi, phy, 2, &data, 1) != GT_OK)
            continue;
        data &= 0xffff;
        if (data == 0xffff)
            continue;
        osPrintf("found: phy=%d id0=0x%x", phy, data);
        if (data == 0x0141)
        {
            if (smi->read(smi, phy, 3, &data, 1) == GT_OK)
            {
                osPrintf(" id1=0x%x", data & 0xffff);
            }
        }
        osPrintf("\n");
    }
    return GT_OK;
}


/**
* @internal appDemoDeAllocateDmaMem function
* @endinternal
*
* @param[in] devNum                - device number.
* @brief   This function deallocates memory of phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoDeAllocateDmaMem
(
    IN      GT_U8  devNum
)
{
    GT_U32  ii;
    GT_U32  txQue,rxQue;              /* queue number */
    for(ii = 0; ii < CPSS_MAX_SDMA_CPU_PORTS_CNS; ii++)
    {

        if( ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) )
        {
            continue;
        }
        for(txQue = 0; txQue < 8; txQue++)
        {

        osCacheDmaFree(appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr);
        appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr=
                        NULL;
        }
    }
    for(ii = 0; ii < CPSS_MAX_SDMA_CPU_PORTS_CNS; ii++)
    {

        if( ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) )
        {
            continue;
        }
        for(rxQue = 0; rxQue < 8; rxQue++)
        {
            osCacheDmaFree(appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].descMemPtr);
            appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].descMemPtr=NULL;

            osCacheDmaFree(appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].memData.staticAlloc.buffMemPtr);
            appDemoPpConfigList[devNum].allocDmaInfo.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].memData.staticAlloc.buffMemPtr=NULL;
        }
    }
    osCacheDmaFree(appDemoPpConfigList[devNum].allocDmaInfo.auqCfg.auDescBlock);
    appDemoPpConfigList[devNum].allocDmaInfo.auqCfg.auDescBlock= NULL;

    osCacheDmaFree(appDemoPpConfigList[devNum].allocDmaInfo.fuqCfg.auDescBlock);
   appDemoPpConfigList[devNum].allocDmaInfo.fuqCfg.auDescBlock= NULL;


    osCacheDmaFree(appDemoPpConfigList[devNum].allocDmaInfo.netIfCfg.rxDescBlock);
    appDemoPpConfigList[devNum].allocDmaInfo.netIfCfg.rxDescBlock= NULL;

    osMemSet(&appDemoPpConfigList[devNum].allocDmaInfo,0,sizeof(APP_DEMO_CPSS_DMA_INIT_INFO));

    return GT_OK;
}

#if defined CHX_FAMILY

/**
* @internal appDemoDxChPortPhysicalPortEntryIndexGet function
* @endinternal
*
* @brief   searcs for entry for the given physical port in the array,
*
* @param[in] portMapArraySize         - Number of ports to map, array size
* @param[in] portMapArrayPtr          - pointer to array of mappings
*
* @retval the found entry index or 0xFFFFFFFF if not found
*
*/
static GT_U32 appDemoDxChPortPhysicalPortEntryIndexGet
(
    IN  GT_U32                      portMapArraySize,
    IN  CPSS_DXCH_PORT_MAP_STC      portMapArrayPtr[],
    IN  GT_U32                      port
)
{
    GT_U32 i;

    for (i = 0; (i < portMapArraySize); i++)
    {
        if (portMapArrayPtr[i].physicalPortNumber == port)
        {
            return i;
        }
    }
    return 0xFFFFFFFF;
}

/**
* @internal appDemoDxChPortPhysicalPortUpdatedMapCreate function
* @endinternal
*
* @brief   Create ports mapping map updated by appDemo environment commands
*          Updated array must be freed by cpssOsFree function;
*
* @param[in] portMapArraySize             - Number of ports to map, array size
* @param[in] portMapArrayPtr              - pointer to array of mappings
* @param[out] portMapUpdatedArraySizePtr  - (pointer to)Number of ports to map updated array size
*                                           0 if array not updated
* @param[out] portMapArrayPtr             - (pointer to)pointer to updated array of mappings
*                                           NULL if array not updated
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - memory allocation failed
*
* @note Command list, # at the command name end - number 0,1...
* @command portMapEditMaxIndex  - maximal index used in commands below, if not specified the feature is disable
* @command portMapEditDelete#   - delete entry (port number is data)
* @command portMapEditAddPort#  - add entry (word0 is data)
*        word0 0x00000FFF - physicalPortNumber
*        word0 0x0000F000 - mappingType (0 - ethernet, 1 - SDMA, 2 - ILKN, 3 - REMOTE)
*        word0 0x00FF0000 - portGroup
* @command portMapEditAddInterface#  - add entry (word1 is data) - should have the same index(#) as portMapEditAddPort
*        - mandatory pair for each portMapEditAddPort#
*        word0 0x0000FFFF - interfaceNum
*        word1 0xFFFF0000 - txqPortNumber
* @command portMapEditAddTm#  - add entry (word2 is data) - should have the same index(#) as portMapEditAddPort
*        - optional pair for each portMapEditAddPort#
*        word2 0x000FFFFF - tmPortInd
*        word2 0x80000000 - tmEnable
*/
GT_STATUS appDemoDxChPortPhysicalPortUpdatedMapCreate
(
    IN   GT_U32                      portMapArraySize,
    IN   CPSS_DXCH_PORT_MAP_STC      portMapArrayPtr[],
    OUT  GT_U32                      *portMapUpdatedArraySizePtr,
    OUT  CPSS_DXCH_PORT_MAP_STC      **portMapUpdatedArrayPtrPtr
)
{
    GT_U32                      portMapNewArrayMaxSize;
    GT_U32                      portMapNewArraySize;
    CPSS_DXCH_PORT_MAP_STC      *portMapNewArrayPtr;
    GT_U32                      tmpData;
    GT_U32                      maxCmdIndex;
    GT_U32                      word0, word1, word2;
    GT_U32                      port;
    GT_U32                      index;
    GT_U32                      entryIdx;
    GT_CHAR                     name[64];


    if (appDemoDbEntryGet("portMapEditMaxIndex", &tmpData) != GT_OK)
    {
        /* portMapEdit feature disable */
        *portMapUpdatedArraySizePtr = 0;
        *portMapUpdatedArrayPtrPtr  = NULL;
        return GT_OK;
    }
    maxCmdIndex = tmpData;
    portMapNewArrayMaxSize = portMapArraySize;
    portMapNewArraySize    = portMapArraySize;

    for (index = 0; (index <= maxCmdIndex); index++)
    {
        cpssOsSprintf(name,"portMapEditAddPort%d", index);
        if (appDemoDbEntryGet(name, &tmpData) != GT_OK) continue;
        portMapNewArrayMaxSize ++;
    }

    portMapNewArrayPtr = (CPSS_DXCH_PORT_MAP_STC*)cpssOsMalloc(portMapNewArrayMaxSize * sizeof(CPSS_DXCH_PORT_MAP_STC));
    if (portMapNewArrayPtr == NULL)
    {
        cpssOsPrintf("appDemoDxChPortPhysicalPortUpdatedMapSet memory allocation failed");
        return GT_OUT_OF_CPU_MEM;
    }
    cpssOsMemCpy(portMapNewArrayPtr, portMapArrayPtr, (portMapArraySize * sizeof(CPSS_DXCH_PORT_MAP_STC)));

    for (index = 0; (index <= maxCmdIndex); index++)
    {
        cpssOsSprintf(name, "portMapEditDelete%d", index);
        if (appDemoDbEntryGet(name, &port) != GT_OK) continue;
        entryIdx = appDemoDxChPortPhysicalPortEntryIndexGet(portMapNewArraySize, portMapNewArrayPtr, port);
        if (entryIdx == 0xFFFFFFFF)
        {
            cpssOsPrintf(name,"appDemoDxChPortPhysicalPortUpdatedMapSet deleted port %d not found", port);
            continue;
        }
        for (; ((entryIdx + 1) < portMapNewArraySize); entryIdx++)
        {
            cpssOsMemCpy(&(portMapNewArrayPtr[entryIdx]), &(portMapNewArrayPtr[entryIdx + 1]), sizeof(CPSS_DXCH_PORT_MAP_STC));
        }
        portMapNewArraySize --;
    }


    for (index = 0; (index <= maxCmdIndex); index++)
    {
        cpssOsSprintf(name,"portMapEditAddPort%d", index);
        if (appDemoDbEntryGet(name, &word0) != GT_OK) continue;
        port = word0 & 0xFFF;
        cpssOsSprintf(name,"portMapEditAddInterface%d", index);
        if (appDemoDbEntryGet(name, &word1) != GT_OK)
        {
            cpssOsPrintf("appDemoDxChPortPhysicalPortUpdatedMapSet portMapEditAddInterface for port %d not found\n", port);
            continue;
        }
        word2 = 0; /*default - command is optional*/
        cpssOsSprintf(name,"portMapEditAddTm%d", index);
        appDemoDbEntryGet(name, &word2);
        entryIdx = appDemoDxChPortPhysicalPortEntryIndexGet(portMapNewArraySize, portMapNewArrayPtr, port);
        if (entryIdx != 0xFFFFFFFF)
        {
            cpssOsPrintf(name,"appDemoDxChPortPhysicalPortUpdatedMapSet added port %d found", port);
            continue;
        }
        /* index bounds not checked: tha allcocated size should be enough */
        portMapNewArrayPtr[portMapNewArraySize].physicalPortNumber = port;
        portMapNewArrayPtr[portMapNewArraySize].mappingType        = ((word0 >> 12) & 0xF);
        portMapNewArrayPtr[portMapNewArraySize].portGroup          = ((word0 >> 16) & 0xFF);
        portMapNewArrayPtr[portMapNewArraySize].interfaceNum       = (word1 & 0xFFFF);
        portMapNewArrayPtr[portMapNewArraySize].txqPortNumber      = ((word1 >> 16) & 0xFFFF);
        portMapNewArrayPtr[portMapNewArraySize].tmPortInd          = (word2 & 0xFFFF);
        portMapNewArrayPtr[portMapNewArraySize].tmEnable           = ((word2 >> 31) & 0x1);
        portMapNewArraySize ++;
    }

    *portMapUpdatedArraySizePtr = portMapNewArraySize;
    *portMapUpdatedArrayPtrPtr  = portMapNewArrayPtr;

    return GT_OK;
}
#endif /*CHX_FAMILY*/

GT_STATUS cpssEnablerUtilsTasksInfoGet
(
    GT_VOID
)
{
    static const GT_U32 size = 4096; /* Hope this is enough */
    GT_CHAR *buff = cpssOsMalloc(size);
    GT_STATUS rc = GT_OK;

    if (osTasksInfo(buff, size) != GT_OK) {
        rc = GT_FAIL;
        goto out;
    }

    cpssOsPrintf("%s\n", buff);

out:
    cpssOsFree(buff);

    return rc;
}
