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
* @file appDemoBoardConfig2.c
*
* @brief file #2 : Includes board specific initialization definitions and data-structures.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>
#endif /*CHX_FAMILY*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
/* implement STUB */
GT_STATUS gtAppDemoXPhyFwDownload
(
    IN  GT_U8   devNum
)
{
    GT_UNUSED_PARAM(devNum);
    return GT_NOT_IMPLEMENTED;
}
GT_STATUS mtdParallelEraseFlashImage
(
    IN void* contextPtr,
    IN GT_U16 ports[],
    IN GT_U32 appSize,
    IN GT_U8 slaveData[],
    IN GT_U32 slaveSize,
    IN GT_U16 numPorts,
    OUT GT_U16 erroredPorts[],
    OUT GT_U16 *errCode
)
{
    GT_UNUSED_PARAM(contextPtr);
    GT_UNUSED_PARAM(ports);
    GT_UNUSED_PARAM(appSize);
    GT_UNUSED_PARAM(slaveData);
    GT_UNUSED_PARAM(slaveSize);
    GT_UNUSED_PARAM(numPorts);
    GT_UNUSED_PARAM(erroredPorts);
    GT_UNUSED_PARAM(errCode);
    return GT_NOT_IMPLEMENTED;
}
#endif

#if (!(defined ASIC_SIMULATION_ENV_FORBIDDEN) && defined ASIC_SIMULATION)
    /* need to run with simulation lib exists */
    #define WM_IMPLEMENTED
#endif


appDemo_cpssDxChTrunkMemberRemove_CB_FUNC appDemo_cpssDxChTrunkMemberRemove = NULL;
appDemo_cpssDxChTrunkMemberAdd_CB_FUNC    appDemo_cpssDxChTrunkMemberAdd    = NULL;
appDemo_cpssDxChTrunkMembersSet_CB_FUNC   appDemo_cpssDxChTrunkMembersSet   = NULL;

#ifdef CHX_FAMILY
/*number of DQ ports in each of the 6 DQ units in TXQ */
#define BC3_NUM_PORTS_PER_DQ_IN_TXQ_CNS     SIP_5_20_DQ_NUM_PORTS_CNS
/*macro to convert local port and data path index to TXQ port */
#define BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(localPort , dpIndex) \
    (localPort) + ((dpIndex) * BC3_NUM_PORTS_PER_DQ_IN_TXQ_CNS)
/* build TXQ_port from global mac port */
#define BC3_TXQ_PORT(globalMacPort)    BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC((globalMacPort)%12,(globalMacPort)/12)

static GT_U32   bc3ConvertDmaNumToTxqNum(IN GT_U32 dmaNum)
{
    GT_U32  txqNum;
    if(dmaNum < 72)
    {
        txqNum = BC3_TXQ_PORT(dmaNum);
    }
    else
    if(dmaNum < 78)
    {
        GT_U32  dpCpuPorts[6] = {2,5,0,1,3,4};

        txqNum = BC3_LOCAL_PORT_IN_DP_TO_TXQ_PORT_MAC(12 , dpCpuPorts[dmaNum-72]);
    }
    else
    {
        txqNum = GT_NA;
    }

    return txqNum;
}

/**
* @internal appDemoDxChFillDbForCpssPortMappingInfo function
* @endinternal
*
* @brief   This function converts 'appDemo port mapping' style to
*           'cpss DXCH port mapping' style .
*
* @param[in] devNum                   - The device number.
* @param[in] appDemoInfoPtr           - array of port mapping in AppDemo style
* @param[in]  appDemoInfoPtr          - array of port mapping in CPSS style
*                                       array allocated array by the caller !!!
* @param[out] appDemoInfoPtr          - (filled) array of port mapping in CPSS style
* @param[out] numOfMappedPortsPtr     - (pointer to) the number of entries filled in appDemoInfoPtr[]
* @param[out] numCpuSdmasPtr          - (pointer to) the number of CPU SDMAs
*                                       can be NULL
*
* @retval GT_OK      - on success
* @retval GT_FULL    - asking for too many physical ports.
* @retval GT_NOT_IMPLEMENTED - for device that code not implemented
*
*/
GT_STATUS appDemoDxChFillDbForCpssPortMappingInfo(
    IN GT_SW_DEV_NUM                devNum,
    IN APP_DEMO_PORT_MAP_STC        *appDemoInfoPtr,
    INOUT CPSS_DXCH_PORT_MAP_STC    *cpssInfoPtr,
    OUT   GT_U32                    *numOfMappedPortsPtr,
    OUT   GT_U32                    *numCpuSdmasPtr
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    GT_U32  dmaPortOffset;
    GT_U32 maxPhyPorts;
    GT_U32 numOfMappedPorts = 0;
    GT_U32 numCpuSdmas = 0;
    GT_U32 bc3used = 0;

    /*************************************************************/
    /* implementation based on fillDbForCpssPortMappingInfo(...) */
    /*************************************************************/

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    numOfMappedPorts = 0;

    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
    {
        bc3used = 1;
    }
    else
    {
        return GT_NOT_IMPLEMENTED;
    }

    for (ii = 0 ; appDemoInfoPtr->startPhysicalPortNumber != GT_NA ; ii++ , appDemoInfoPtr++)
    {
        if ((appDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)&&
            (numOfMappedPorts < maxPhyPorts))
        {
            /* first generate line that map between the mac number to physical port in the device.
            for remote ports the physical port number ( cascade number) is in field of "dma step"*/
            cpssInfoPtr->physicalPortNumber = appDemoInfoPtr->jumpDmaPorts;
            cpssInfoPtr->mappingType        = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            cpssInfoPtr->interfaceNum       = appDemoInfoPtr->startGlobalDmaNumber;

            /* the cascade port not need TXQ queue */
            cpssInfoPtr->txqPortNumber  = bc3used ? GT_NA : 0;

            numOfMappedPorts++;
            cpssInfoPtr++;
            /* second, generate lines that map between remote physical port to the mac number*/
        }

        dmaPortOffset = 0;

        for(jj = 0 ; jj < appDemoInfoPtr->numOfPorts; jj++ , cpssInfoPtr++)
        {
            if(numOfMappedPorts >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_falcon_defaultMap is FULL, maxPhy port is %d ", maxPhyPorts);
                return rc;
            }

            cpssInfoPtr->physicalPortNumber = appDemoInfoPtr->startPhysicalPortNumber + jj;
            cpssInfoPtr->mappingType        = appDemoInfoPtr->mappingType;
            cpssInfoPtr->interfaceNum       = appDemoInfoPtr->startGlobalDmaNumber + dmaPortOffset;

            if(appDemoInfoPtr->startTxqNumber == GT_NA)
            {
                cpssInfoPtr->txqPortNumber  = GT_NA;
            }
            else
            if(appDemoInfoPtr->startTxqNumber == TXQ_BY_DMA_CNS)
            {
                /* 'use startGlobalDmaNumber' for the definition */
                cpssInfoPtr->txqPortNumber  = bc3used ? bc3ConvertDmaNumToTxqNum(cpssInfoPtr->interfaceNum) : 0;
            }
            else
            {
                /* explicit value */
                cpssInfoPtr->txqPortNumber  = appDemoInfoPtr->startTxqNumber + jj;
            }

            if(appDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                numCpuSdmas++;
            }

            numOfMappedPorts++;

            if((appDemoInfoPtr->jumpDmaPorts != DMA_NO_STEP)&&(appDemoInfoPtr->mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E))
            {
                if(appDemoInfoPtr->jumpDmaPorts >= 2)
                {
                    dmaPortOffset += appDemoInfoPtr->jumpDmaPorts;
                }
                else
                {
                    dmaPortOffset ++;
                }
            }
        }
    }

    *numOfMappedPortsPtr = numOfMappedPorts;
    if(numCpuSdmasPtr)
    {
        *numCpuSdmasPtr       = numCpuSdmas;
    }

    return GT_OK;
}

#endif /*CHX_FAMILY*/

extern GT_STATUS prvNoKmDrv_configure_dma_internal_cpu(
    IN GT_U8    devNum
);
extern GT_STATUS prvNoKmDrv_configure_dma_over_the_pci(
    IN GT_U8    devNum
);
extern GT_STATUS prvNoKmDrv_configure_dma_per_devNum(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

extern GT_STATUS prvNoKmDrv_set_pci_info_by_devNum(
    IN GT_U8    devNum,
    IN GT_U8    pciDomainNum,
    IN GT_U8    pciBusNum,
    IN GT_U8    pciDevNum,
    IN GT_U8    pciFunNum
);

extern GT_STATUS prvNoKmDrv_get_dma_buf_by_devNum(
    IN GT_U8    devNum
);

extern GT_STATUS prvNoKmDrv_configure_pre_sip6_dma_per_devNum(
    IN GT_U8            devNum,
    IN GT_U16           busNo,
    IN GT_U8            devSel,
    IN GT_U8            funcNo,
    IN CPSS_HW_INFO_STC *hwInfoPtr
);

extern GT_STATUS prvNoKmDrv_get_dma_window_config_by_devNum(
    IN  GT_U8       devNum,
    OUT GT_U64_BIT  *dmaScatterBufStart,
    OUT GT_U64_BIT  *dmaScatterBufEnd,
    OUT GT_U32      *dmaScatterBufSize
);

/* this function version uses a new API */
GT_STATUS   appDemo_configure_dma_per_devNum(
    IN GT_U8    devNum
)
{
#ifdef CHX_FAMILY
    GT_STATUS rc = GT_OK;
#if defined(CONFIG_V2_DMA)
    GT_STATUS rc1 = GT_OK;

    /*
     * Store current devNum for later use by DMA driver
     */
    PRV_OS_DMA_CURR_DEVICE_SET(devNum);

    rc = prvNoKmDrv_set_pci_info_by_devNum(CAST_SW_DEVNUM(devNum),
        ((appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciBusNum >> 8) & 0xFF),
        (appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciBusNum & 0xFF),
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciIdSel,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.funcNo);
    if(rc != GT_OK) {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_set_info_per_devNum", rc);
        return rc;
    }

    if((!PRV_CPSS_SIP_6_CHECK_MAC(devNum)) &&
        (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        rc = prvNoKmDrv_configure_pre_sip6_dma_per_devNum(devNum,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciBusNum,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciIdSel,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.funcNo,
        &appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].hwInfo);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_configure_pre_sip6_dma_per_devNum", rc);
        return rc;
    } else {  /* AC5 or SIP6 */
        rc = prvNoKmDrv_get_dma_buf_by_devNum(devNum);
        if(rc != GT_OK) {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_get_dma_buf_by_devNum", rc);
            return rc;
        }
    }

    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) || /* AC5 or SIP6 */
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)) {
        CPSS_DXCH_HW_PP_DMA_CONFIGURE_STC dmaCfg;

        do {
            GT_U64_BIT  dmaScatterBufStart = 0;
            GT_U64_BIT  dmaScatterBufEnd = 0;
            GT_U32      dmaScatterBufSize = 0;

            rc = prvNoKmDrv_get_dma_window_config_by_devNum(devNum, &dmaScatterBufStart, &dmaScatterBufEnd, &dmaScatterBufSize);
            if(rc == GT_FAIL) {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_get_dma_window_config_by_devNum", rc);
                return rc;
            }

            if ((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) && (dmaScatterBufSize < _8M))
            {
                CPSS_TBD_BOOKMARK
                /* temporary fix for Falcon boards those may allocate more than 6M for DMA.
                   but dmaScatterBufSize detects 4M. Need to guaranty that window will be sufficient. */
                dmaScatterBufSize = _8M;
            }

            osPrintf("Configuring DMA Window with Start Address = 0x%llX and Size = 0x%X\n",
                dmaScatterBufStart, dmaScatterBufSize);

            dmaCfg.dmaBaseAdress.l[0] = (dmaScatterBufStart & 0xFFFFFFFF);
            dmaCfg.dmaBaseAdress.l[1] = ((dmaScatterBufStart >> 32) & 0xFFFFFFFF);
            dmaCfg.dmaRangeSize  = dmaScatterBufSize;
            dmaCfg.dmaMemoryType = CPSS_DXCH_DMA_MEM_TYPE_DRAM_E;

            rc1 = cpssDxChHwPpDmaConfigSet(devNum, &dmaCfg);
            if(rc1 != GT_OK) {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpDmaConfigSet", rc1);
                return rc1;
            }
        } while(rc != GT_NO_MORE);

        {
            GT_U32  txSramQueue = 0;
            appDemoDbEntryGet("txSramQueue", &txSramQueue);
            /*
            * Configure DMA window with CnM SRAM address and size information.
            *
            * dmaBaseAdress and dmaRangeSize is set to 0 since CPSS internally
            * retreives the SRAM base address and size from internal database
            *
            * We only need to let CPSS know about the dma memory type in case of SRAM
            */
            if(txSramQueue)
            {
                dmaCfg.dmaBaseAdress.l[0] = 0x0;
                dmaCfg.dmaBaseAdress.l[1] = 0x0;
                dmaCfg.dmaRangeSize  = 0x0;
                dmaCfg.dmaMemoryType = CPSS_DXCH_DMA_MEM_TYPE_CNM_SRAM_0_E;

                rc1 = cpssDxChHwPpDmaConfigSet(devNum, &dmaCfg);
                if(rc1 != GT_OK)
                {
                    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpDmaConfigSet", rc1);
                    return rc1;
                }
            }
        }
    }
    return GT_OK;
#else /* CONFIG_V2_DMA */
    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) || /* AC5 or SIP6 */
        (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)) {
        CPSS_DXCH_HW_PP_DMA_CONFIGURE_STC dmaCfg;

        /* CPSS keeps the bus type in DB                                 */
        /* It knows that for MBUS dmaInsidePp is TRUE, otherwise - FALSE */

        /* the code copied from extDrvGetDmaBase and extDrvGetDmaSize functions */
        dmaCfg.dmaBaseAdress =
            PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64);
        dmaCfg.dmaRangeSize  =
            PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen);
        dmaCfg.dmaMemoryType = CPSS_DXCH_DMA_MEM_TYPE_DRAM_E;

        rc = cpssDxChHwPpDmaConfigSet(devNum, &dmaCfg);
        if(rc != GT_OK) {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpDmaConfigSet", rc);
            return rc;
        }

    }
    return rc;
#endif /* CONFIG_V2_DMA */
#else /*CHX_FAMILY*/
#if defined(CONFIG_V2_DMA)
    GT_STATUS rc = GT_OK;

    /*
     * Store current devNum for later use by DMA driver
     */
    PRV_OS_DMA_CURR_DEVICE_SET(devNum);

    rc = prvNoKmDrv_set_pci_info_by_devNum(CAST_SW_DEVNUM(devNum),
        ((appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciBusNum >> 8) & 0xFF),
        (appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciBusNum & 0xFF),
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciIdSel,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.funcNo);
    if(rc != GT_OK) {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_set_info_per_devNum", rc);
        return rc;
    }

    rc = prvNoKmDrv_configure_pre_sip6_dma_per_devNum(devNum,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciBusNum,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.pciIdSel,
        appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].pciInfo.funcNo,
        &appDemoPpConfigList[CAST_SW_DEVNUM(devNum)].hwInfo);
    if(rc != GT_OK) {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvNoKmDrv_configure_pre_sip6_dma_per_devNum", rc);
        return rc;
    }

    return GT_OK;
#else
    devNum = devNum; /* avoid unused param warning */
    return GT_NOT_IMPLEMENTED;
#endif
#endif /*CHX_FAMILY*/
}


#ifdef CHX_FAMILY
/**
* @internal appDemoOamUpMepWAInit function
* @endinternal
*
*@brief  This function initialized the OAM Up-MEP WA.
*
* @param[in] devNum                - the SW devNum
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS appDemoOamUpMepWAInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS   rc;
    GT_U32      additionalInfoBmpArr[1];
    GT_U32      waIndex = 1;
    static CPSS_DXCH_IMPLEMENT_WA_ENT OamUpMepWaList[] =
    {
        /* Contains only the OAM Up-MEP WA enum */
        CPSS_DXCH_IMPLEMENT_WA_OAM_UP_MEP_INIT_E
    };

    /* OAM UP-MEP WA: Assign a reserved tail-drop profile */
    additionalInfoBmpArr[0] = CPSS_PORT_TX_DROP_PROFILE_16_E;
    rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum), waIndex, OamUpMepWaList, &(additionalInfoBmpArr[0]));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);

    return rc;
}
#endif /*CHX_FAMILY*/
