/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformPpConfig.c
*
* @brief This file contains APIs for adding and removing PPs.
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformBoardConfig.h>
#include <cpssAppPlatformRunTimeConfig.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformPciConfig.h>
#include <cpssAppPlatformPpUtils.h>
#include <cpssAppPlatformPortInit.h>
#include <extUtils/common/cpssEnablerUtils.h>
#include <cpssAppUtilsEvents.h>
#include <cpssAppUtilsCommon.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#ifdef INCLUDE_MPD
#undef MAX_UINT_8
#endif
    #include <gtOs/gtOsExc.h>
    #include <gtExtDrv/drivers/gtDmaDrv.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/diag/cpssDxChDiag.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortAp.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/private/prvCpssDxChVirtualTcamDb.h>
#include <cpss/generic/cpssHwInfo.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

#include <gtExtDrv/drivers/gtPciDrv.h>
#include <ezbringup/cpssAppPlatformEzBringupTools.h>

#include <cpssAppPlatform/cpssAppPlatformPpConfig.h>

#define CPSS_APP_PLATFORM_AC5_MAX_PORTS 28


#define GT_DUMMY_REG_VAL_INFO_LIST                                          \
{                                                                           \
    {0x00000000, 0x00000000, 0x00000000, 0},                                \
    {0x00000001, 0x00000000, 0x00000000, 0},                                \
    {0x00000002, 0x00000000, 0x00000000, 0},                                \
    {0x00000003, 0x00000000, 0x00000000, 0},                                \
    {0x00000004, 0x00000000, 0x00000000, 0},                                \
    {0x00000005, 0x00000000, 0x00000000, 0},                                \
    {0x00000006, 0x00000000, 0x00000000, 0},                                \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0},    /* Delimiter        */      \
    {0xFFFFFFFF, 0x00000000, 0x00000000, 0}     /* Delimiter        */      \
}

#define TIME_DIFF_MAC(sec1, nsec1, sec2, nsec2, sec, nsec) \
    if(nsec2 < nsec1)\
    {\
        nsec2 += 1000000000;\
        sec2  -= 1;\
    }\
    sec  = sec2 - sec1;\
    nsec = nsec2 - nsec1;\


#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS

GT_STATUS prvEventMaskSet
(
    IN GT_U8 devNum,
    IN CPSS_EVENT_MASK_SET_ENT operation
);

extern GT_VOID cpssAppPlatformBelly2BellyEnable(IN GT_BOOL enable);

extern GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);

#ifdef LINUX_NOKM
extern GT_STATUS prvNoKmDrv_configure_dma_per_devNum
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId
);

extern GT_STATUS prvNoKmDrv_configure_dma_over_the_pci(
    IN GT_U8    devNum
);

extern GT_STATUS prvNoKmDrv_configure_dma_internal_cpu(
    IN GT_U8    devNum
);

extern GT_STATUS prvNoKmDrv_get_dma_window_config_by_devNum(
    IN  GT_U8       devNum,
    OUT GT_U64_BIT  *dmaScatterBufStart,
    OUT GT_U64_BIT  *dmaScatterBufEnd,
    OUT GT_U32      *dmaScatterBufSize
);
extern GT_STATUS prvNoKmDrv_set_pci_info_by_devNum(
     IN GT_U8    devNum,
     IN GT_U8    pciDomainNum,
     IN GT_U8    pciBusNum,
     IN GT_U8    pciDevNum,
     IN GT_U8    pciFunNum
 );

GT_STATUS prvNoKmDrv_configure_pre_sip6_dma_per_devNum(
     IN GT_U8            devNum,
     IN GT_U16           busNo,
     IN GT_U8            devSel,
     IN GT_U8            funcNo,
     IN CPSS_HW_INFO_STC *hwInfoPtr
);

GT_STATUS prvNoKmDrv_get_dma_buf_by_devNum(
    IN GT_U8    devNum
);


#endif

#ifdef ASIC_SIMULATION
extern GT_STATUS cpssSimSoftResetDoneWait(void);
#endif


/*****************reInit Handling for unexpected app exit***************/

static GT_STATUS    doDeviceResetAndRemove(IN GT_U8   devNum,GT_BOOL includePex, IN GT_U32 pciBusNum, IN GT_U32 pciIdSel, IN GT_U32 funcNo)
{
    GT_STATUS   rc;

#ifndef SUPPORT_PCIe_RESCAN
    /* the environment just not supports PCIe rescan , so do not allow to reset the device with the PCIe */
    includePex = GT_FALSE;
#endif /*SUPPORT_PCIe_RESCAN*/

    /* Disable All Skip Reset options ,exclude PEX */
    /* this Enable Skip Reset for PEX */
    if( GT_FALSE == includePex )
    {
        osPrintf("doDeviceResetAndRemove : NO PEX reset !!! \n");

        rc = cpssDxChHwPpSoftResetSkipParamSet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    cpssOsPrintf("PCIe info :pciBusNum[%d],pciIdSel[%d],funcNo[%d] \n",
        pciBusNum,pciIdSel,funcNo);

    /* provide time to finish printings */
    osTimerWkAfter(100);

    rc = cpssDxChHwPpSoftResetTrigger(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef ASIC_SIMULATION
    rc = cpssSimSoftResetDoneWait();
#endif

#ifndef ASIC_SIMULATION
#ifdef SUPPORT_PCIe_RESCAN
/*SUPPORT_PCIe_RESCAN*/
    if(GT_TRUE == includePex)
    {
        CPSS_HW_INFO_STC hwInfo;
        osMemSet(&hwInfo, 0, sizeof(CPSS_HW_INFO_STC));

        rc = extDrvPexRemove(
                    pciBusNum,
                    pciIdSel,
                    funcNo);

        rc = extDrvPexRescan(
                   pciBusNum,
                   pciIdSel,
                   funcNo,
                   &hwInfo);
    }
/*SUPPORT_PCIe_RESCAN*/
#endif
#endif

    rc = cpssDxChCfgDevRemove(devNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

static GT_STATUS appPlatformdoAtomicDeviceResetAndRemove(IN GT_U8 devNum, IN GT_U32 pciBusNum, IN GT_U32 pciIdSel, IN GT_U32 funcNo)
{
    GT_STATUS   rc;
    GT_BOOL     pexReset = GT_FALSE;

    /* make 'atomic' protection on the CPSS APIs that access this device that will do reset + remove */
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(GT_TRUE == PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /*Reset PEX for SIP6 only */
        pexReset = GT_TRUE;
#ifdef ASIC_SIMULATION_ENV_FORBIDDEN
        /* WM-ASIM environment , the ASIM not supports reset with the PCIe */
        pexReset = GT_FALSE;
#endif /*ASIC_SIMULATION_ENV_FORBIDDEN*/
    }


    rc = doDeviceResetAndRemove(devNum,pexReset, pciBusNum, pciIdSel, funcNo);

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/*****************reInit Handling for unexpected app exit***************/

/**
* @internal cpssAppPlatformPhase1Init function
* @endinternal
*
* @brief   Phase1 PP configurations
*
* @param [in] *ppMapPtr          - PCI/SMI info of the device,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPhase1Init
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS                         rc = GT_OK;
#if defined(CONFIG_V2_DMA)
    GT_STATUS                         rc1 = GT_OK;
#endif /*CONFIG_V2_DMA*/
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC cpssPpPhase1 = {0};
    CPSS_PP_DEVICE_TYPE               ppDevType;
    CPSS_REG_VALUE_INFO_STC          *regCfgList;
    GT_U32                            regCfgListSize;
    GT_U32                            pciConfigFlag;
    PCI_INFO_STC                      pciInfo;
    CPSS_PP_FAMILY_TYPE_ENT           ppDevFamily;
    GT_U32                            phase1InitCounter = 0;

#ifdef CHX_FAMILY
     CPSS_REG_VALUE_INFO_STC defaultRegValInfoList[] = GT_DUMMY_REG_VAL_INFO_LIST;
#else /*CHX_FAMILY*/
    CPSS_REG_VALUE_INFO_STC defaultRegValInfoList[] = {{0}};
#endif /*CHX_FAMILY*/

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!ppMapPtr || !ppProfilePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    if (ppMapPtr->mapType == CPSS_APP_PLATFORM_PP_MAP_TYPE_LAST_E)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("mapType=%d not supported.\n", ppMapPtr->mapType);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(!ppProfilePtr->internalCpu)
    {
        /* Get the PCI info specific device on bus,dev with devNum*/
        if ((ppMapPtr->mngInterfaceAddr.pciAddr.busId == 0xFF)
                || ppMapPtr->mngInterfaceAddr.pciAddr.deviceId == 0xFF) /*To enable autoscan initialize profile with FF values for bus or dev in management interface*/
        {
            /* Get the PCI info specific device on bus,dev with devNum*/
            rc = prvCpssAppPlatformSysGetPciInfoAutoScan(ppMapPtr->devNum, &ppDevFamily, &pciInfo);
        }
        else
        {
            /* Get the PCI info specific device on bus,dev with devNum*/
            rc = prvCpssAppPlatformSysGetPciInfo(ppMapPtr->devNum,
                    ppMapPtr->mngInterfaceAddr.pciAddr.domain,
                    ppMapPtr->mngInterfaceAddr.pciAddr.busId,
                    ppMapPtr->mngInterfaceAddr.pciAddr.deviceId,
                    ppMapPtr->mngInterfaceAddr.pciAddr.functionId,
                    &ppDevFamily, &pciInfo);
        }
        if(rc != GT_OK)
        {
            if(rc == GT_NO_MORE)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("Prestera Device on pciBusNum:%d Not Present\n", ppMapPtr->mngInterfaceAddr.pciAddr.busId);
            } else {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("prvCpssAppPlatformSysGetPciInfo rc=%d\n", rc);
            }
            return rc;
        }

        switch (ppDevFamily)
        {
           case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
           case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
           case CPSS_PP_FAMILY_DXCH_AC3X_E:
              pciConfigFlag = MV_EXT_DRV_CFG_FLAG_NEW_ADDRCOMPL_E;
              break;

           case CPSS_PP_FAMILY_DXCH_FALCON_E:
           case CPSS_PP_FAMILY_DXCH_AC5X_E:
           case CPSS_PP_FAMILY_DXCH_AC5P_E:
           case CPSS_PP_FAMILY_DXCH_HARRIER_E:
           case CPSS_PP_FAMILY_DXCH_AC5_E:
           case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
              pciConfigFlag = MV_EXT_DRV_CFG_FLAG_EAGLE_E;
              break;

           default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }

        /* WA For AC5X */
        if ((ppDevFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) || (ppDevFamily == CPSS_PP_FAMILY_DXCH_AC5_E) ||
             (ppDevFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (ppDevFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E) ||
             (ppDevFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
        {
            prvCpssDrvHwPpPrePhase1NextDevFamilySet(ppDevFamily);
        }

        /* Need to use busId with domain, so that port arbiter will differentiate between the devices. */
        pciInfo.pciBus = pciInfo.pciBus + (256 * ppMapPtr->mngInterfaceAddr.pciAddr.domain);
        ppMapPtr->mngInterfaceAddr.pciAddr.busId = pciInfo.pciBus;
        rc = extDrvPexConfigure(pciInfo.pciBus, pciInfo.pciDev, pciInfo.pciFunc,
                                pciConfigFlag, &(cpssPpPhase1.hwInfo[0]));
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPexConfigure);

    }
    else
    {
        pciConfigFlag   = MV_EXT_DRV_CFG_FLAG_EAGLE_E;
        pciInfo.pciDev  = 0xFF;
        pciInfo.pciFunc = 0xFF;
        pciInfo.pciBus  = 0xFF;

        if (!cpssOsStrCmp(ppProfilePtr->ppName, "AC5X"))
        {
           ppDevFamily = CPSS_PP_FAMILY_DXCH_AC5X_E;
        }
        else if (!cpssOsStrCmp(ppProfilePtr->ppName, "AC5P"))
        {
           ppDevFamily = CPSS_PP_FAMILY_DXCH_AC5P_E;
        }
        else if (!cpssOsStrCmp(ppProfilePtr->ppName, "IM"))
        {
           ppDevFamily = CPSS_PP_FAMILY_DXCH_IRONMAN_E;
        }
        else if (!cpssOsStrCmp(ppProfilePtr->ppName, "IRONMAN"))
        {
           ppDevFamily = CPSS_PP_FAMILY_DXCH_IRONMAN_E;
        }
        else if (!cpssOsStrCmp(ppProfilePtr->ppName, "AC5"))
        {
           ppDevFamily = CPSS_PP_FAMILY_DXCH_AC5_E;
        }

        prvCpssDrvHwPpPrePhase1NextDevFamilySet(ppDevFamily);

        if (ppDevFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
        {
            pciConfigFlag |= (MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5_ID_CNS << MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS);
        }
        else
        {
            pciConfigFlag |= (MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_AC5X_ID_CNS << MV_EXT_DRV_CFG_FLAG_SPECIAL_INFO_OFFSET_CNS);
        }

        rc = extDrvPexConfigure(pciInfo.pciBus, pciInfo.pciDev, pciInfo.pciFunc, /*pciBus value to be supplied as 0xFFFF */
                                pciConfigFlag , &(cpssPpPhase1.hwInfo[0]));
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPexConfigure);
    }

    cpssPpPhase1.devNum                      = ppMapPtr->devNum;
    cpssPpPhase1.mngInterfaceType            = ppMapPtr->mngInterfaceType;

    cpssPpPhase1.coreClock                   = ppProfilePtr->coreClock;
    cpssPpPhase1.serdesRefClock              = ppProfilePtr->serdesRefClock;
    cpssPpPhase1.isrAddrCompletionRegionsBmp = ppProfilePtr->isrAddrCompletionRegionsBmp;
    cpssPpPhase1.appAddrCompletionRegionsBmp = ppProfilePtr->appAddrCompletionRegionsBmp;
    cpssPpPhase1.numOfPortGroups             = ppProfilePtr->numOfPortGroups;
    cpssPpPhase1.tcamParityCalcEnable        = ppProfilePtr->tcamParityCalcEnable;
    cpssPpPhase1.numOfDataIntegrityElements  = ppProfilePtr->numOfDataIntegrityElements;
    cpssPpPhase1.maxNumOfPhyPortsToUse       = ppProfilePtr->maxNumOfPhyPortsToUse;

    CPSS_APP_PLATFORM_LOG_DBG_MAC("ApEnabled %d\n",  ppProfilePtr->apEnable);
    rc = cpssDxChPortApEnableSet(ppMapPtr->devNum, 0x1, ppProfilePtr->apEnable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortApEnableSet);

phase_1_init_lbl:
    phase1InitCounter++;
    rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1, &ppDevType);
    if(rc == GT_HW_ERROR_NEED_RESET )
    {
        appPlatformdoAtomicDeviceResetAndRemove(CAST_SW_DEVNUM(ppMapPtr->devNum), pciInfo.pciBus, pciInfo.pciDev, pciInfo.pciFunc);
        osTimerWkAfter(100);

        if(phase1InitCounter < 5)
        {
            cpssOsPrintf("loop iteration [%d] : call again to 'phase-1' init \n",
                phase1InitCounter);
            goto  phase_1_init_lbl;
        }
        else
        {
            rc = cpssDxChHwPpPhase1Init(&cpssPpPhase1, &ppDevType);
        }
    }
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpPhase1Init);

#ifdef CPSS_APP_PLATFORM_REFERENCE
    /* Change is needed only for AC5X as per 4.3.3 CPSS appDemo */
    /* Change miscellaneous MPP configurations */
    if (CPSS_PP_FAMILY_DXCH_AC5X_E == ppDevFamily)
    {
       /* MPP 19 is used as OUT for DEV_INIT_DONE signal.
        This signal is connected to RESET input of external CPU cards on DB board.
        AC5X Device toggles DEV_INIT_DONE during Soft Reset and this resets external CPU.
        Following configuration changes MPP settings to connecte constant value '1' to MPP 19..
        And external CPU will not get reset. */

        /* set output for GPIO 19 to be 1 */
        rc = prvCpssDrvHwPpSetRegField(ppMapPtr->devNum, 0x7F018130, 19, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* enable output for GPIO 19 */
        rc = prvCpssDrvHwPpSetRegField(ppMapPtr->devNum, 0x7F01812C, 19, 1, 1);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    if (CPSS_PP_FAMILY_DXCH_AC5_E == ppDevFamily)
    {
        GT_STATUS   rc;         /* return code */
        GT_U32  jj=0;
        GT_U32 const mppListArr[][2] =
        {
            /* Set MPP 36,37,40,43,44,45 to output LED signals */
            {36,1}, {37,1}, {40,1}, {43,1}, {44,1}, {45,1}
        };
        GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));

        for (jj = 0; jj < mppListSize; jj++)
        {
            rc = cpssDxChHwMppSelectSet(ppMapPtr->devNum, mppListArr[jj][0], mppListArr[jj][1]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }


        /* in few line the register will be read to
           recognize 'APP_DEMO_AC3_BOARD_DB_CNS' */
        rc = prvCpssHwPpSetRegField(ppMapPtr->devNum, 0x7C, 0, 8, 0x30);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, "prvCpssHwPpSetRegField");

    }
    if (CPSS_PP_FAMILY_DXCH_IRONMAN_E == ppDevFamily)
    {
        GT_U32      ii;
        GT_U32 const mppListArr[][2] =
        {
            /* MPP 30 - LED_CLK, MPP 31 - LED_STB, MPP 32 - LED_DATA */
            {30,1}, {31,1}, {32,1}
        };
        GT_U32 mppListSize = (sizeof(mppListArr) / sizeof(mppListArr[0]));

        for (ii = 0; ii < mppListSize; ii++)
        {
            rc = cpssDxChHwMppSelectSet(ppMapPtr->devNum, mppListArr[ii][0], mppListArr[ii][1]);
            if (rc != GT_OK)
            {
                return rc;
            }
        }

    }
#endif /* CPSS_APP_PLATFORM_REFERENCE */

    /* Mpp Select */
    {
        GT_STATUS   rc;         /* return code */
        GT_U32      ii;
        for (ii = 0; ii < ppMapPtr->mppSelectArrSize; ii++)
        {
            rc = cpssDxChHwMppSelectSet(ppMapPtr->devNum, ppMapPtr->mppSelectPtr[ii].mppNumber,
                                        ppMapPtr->mppSelectPtr[ii].mppSelect);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }

#if defined(CONFIG_V2_DMA)
    if (ppProfilePtr->internalCpu)
    {
        rc = prvNoKmDrv_set_pci_info_by_devNum(ppMapPtr->devNum,
                ((pciInfo.pciBus >> 8) & 0xFF),
                (pciInfo.pciBus & 0xFF),
                pciInfo.pciDev,
                pciInfo.pciFunc);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_set_info_per_devNum);
    }
    else
    {
        rc = prvNoKmDrv_set_pci_info_by_devNum(ppMapPtr->devNum,
                ppMapPtr->mngInterfaceAddr.pciAddr.domain,
                ppMapPtr->mngInterfaceAddr.pciAddr.busId,
                ppMapPtr->mngInterfaceAddr.pciAddr.deviceId,
                ppMapPtr->mngInterfaceAddr.pciAddr.functionId);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_set_info_per_devNum);
    }
    if((!PRV_CPSS_SIP_6_CHECK_MAC(ppMapPtr->devNum)) &&
            (PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        rc = prvNoKmDrv_configure_pre_sip6_dma_per_devNum(ppMapPtr->devNum,
                ppMapPtr->mngInterfaceAddr.pciAddr.busId,
                ppMapPtr->mngInterfaceAddr.pciAddr.deviceId,
                ppMapPtr->mngInterfaceAddr.pciAddr.functionId,
                &(cpssPpPhase1.hwInfo[0]));
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_configure_pre_sip6_dma_per_devNum);
    } else {  /* AC5 or SIP6 */
        rc = prvNoKmDrv_get_dma_buf_by_devNum(ppMapPtr->devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_get_dma_buf_by_devNum);
    }
    if((PRV_CPSS_SIP_6_CHECK_MAC(ppMapPtr->devNum)) || /* AC5 or SIP6 */
            (PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)) {
        CPSS_DXCH_HW_PP_DMA_CONFIGURE_STC dmaCfg;

        do {
            GT_U64_BIT  dmaScatterBufStart = 0;
            GT_U64_BIT  dmaScatterBufEnd = 0;
            GT_U32      dmaScatterBufSize = 0;

            rc = prvNoKmDrv_get_dma_window_config_by_devNum(ppMapPtr->devNum, &dmaScatterBufStart, &dmaScatterBufEnd, &dmaScatterBufSize);
            if(rc == GT_FAIL) {
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNoKmDrv_get_dma_window_config_by_devNum);
            }

            if ((PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) && (dmaScatterBufSize < _8M))
            {
                CPSS_TBD_BOOKMARK
                /* temporary fix for Falcon boards those may allocate more than 6M for DMA.
                   but dmaScatterBufSize detects 4M. Need to guaranty that window will be sufficient. */
                dmaScatterBufSize = _8M;
            }

            cpssOsPrintf("Configuring DMA Window with Start Address = 0x%llX and Size = 0x%X\n",
                    dmaScatterBufStart, dmaScatterBufSize);

            dmaCfg.dmaBaseAdress.l[0] = (dmaScatterBufStart & 0xFFFFFFFF);
            dmaCfg.dmaBaseAdress.l[1] = ((dmaScatterBufStart >> 32) & 0xFFFFFFFF);
            dmaCfg.dmaRangeSize  = dmaScatterBufSize;
            dmaCfg.dmaMemoryType = CPSS_DXCH_DMA_MEM_TYPE_DRAM_E;

            rc1 = cpssDxChHwPpDmaConfigSet(ppMapPtr->devNum, &dmaCfg);
            if(rc1 != GT_OK) {
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc1, cpssDxChHwPpDmaConfigSet);
            }
        } while(rc != GT_NO_MORE);


        {
            GT_U32  txSramQueue = 0;
            appPlatformDbEntryGet("txSramQueue", &txSramQueue);
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

                rc1 = cpssDxChHwPpDmaConfigSet(ppMapPtr->devNum, &dmaCfg);
                if(rc1 != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc1, cpssDxChHwPpDmaConfigSet);
                    return rc1;
                }
            }
        }
    }

#else /*CONFIG_V2_DMA*/

    if ((PRV_CPSS_SIP_6_CHECK_MAC(ppMapPtr->devNum)) ||
            (PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        CPSS_DXCH_HW_PP_DMA_CONFIGURE_STC dmaCfg;

        /* CPSS keeps the bus type in DB                                 */
        /* It knows that for MBUS dmaInsidePp is TRUE, otherwise - FALSE */

        /* the code copied from extDrvGetDmaBase and extDrvGetDmaSize functions */
        dmaCfg.dmaBaseAdress =
            PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64);
        dmaCfg.dmaRangeSize  =
            PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen);
        dmaCfg.dmaMemoryType = CPSS_DXCH_DMA_MEM_TYPE_DRAM_E;

        rc = cpssDxChHwPpDmaConfigSet(ppMapPtr->devNum, &dmaCfg);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpDmaConfigSet);
    }
#endif /**/

    regCfgList     = defaultRegValInfoList;
    regCfgListSize = sizeof(defaultRegValInfoList) / sizeof(CPSS_REG_VALUE_INFO_STC);

    rc = cpssDxChHwPpStartInit(ppMapPtr->devNum, regCfgList, regCfgListSize);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpStartInit);

    return rc;
}

/*
* @internal cpssAppPlatformPpPortsInit function
* @endinternal
*
* @brief  Set port mapping. (after Phase1 Init)
*
* @param [in]  *ppMapPtr         - PP_MAP board profile
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpPortsInit
(
   IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr
)
{
    GT_STATUS               rc;
    GT_U32                  mapArrLen = 0;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr   = NULL;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if(ppMapPtr->portMap != NULL)
    {
        mapArrLen = ppMapPtr->portMapSize;
        mapArrPtr = ppMapPtr->portMap;
    }
    else
    {
        rc = cpssAppPlatformPpPortMapGet(ppMapPtr->devNum, &mapArrPtr, &mapArrLen);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpPortMapGet);
    }

    if (mapArrPtr && mapArrLen)
    {
        GT_U32                  i;
        GT_U32                  maxPortNum  = 0;
       /* Find Max Port Number in this mapArray and store in our per Device DB for future use */
       for (i=0; i < mapArrLen; i++)
       {
          if (mapArrPtr[i].physicalPortNumber > maxPortNum)
              maxPortNum = mapArrPtr[i].physicalPortNumber;
       }

       rc = cpssDxChPortPhysicalPortMapSet(ppMapPtr->devNum, mapArrLen, mapArrPtr);
       CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapSet);

       PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[ppMapPtr->devNum] = maxPortNum;
       CPSS_APP_PLATFORM_LOG_INFO_MAC("Max Port Number [%d] = %d.\n", ppMapPtr->devNum, maxPortNum);
    }
    if (PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E)
    {
        PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[ppMapPtr->devNum] = CPSS_APP_PLATFORM_AC5_MAX_PORTS;
    }


#if DEBUG_MODE
    /* TODO */
    /* restore OOB port configuration after systemReset */
    if ((bc2BoardResetDone == GT_TRUE) && appDemoBc2IsInternalCpuEnabled(devNum) &&  (!isBobkBoard))
    {
        MV_HWS_SERDES_CONFIG_STR    serdesConfig;

        /* configure SERDES TX interface */
        rc = mvHwsSerdesTxIfSelect(devNum, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, COM_PHY_28NM, 1);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, mvHwsSerdesTxIfSelect);

        serdesConfig.baudRate = _1_25G;
        serdesConfig.media = XAUI_MEDIA;
        serdesConfig.busWidth = _10BIT_ON;
        serdesConfig.refClock = _156dot25Mhz;
        serdesConfig.refClockSource = PRIMARY;
        serdesConfig.rxEncoding = SERDES_ENCODING_NA;
        serdesConfig.serdesType = COM_PHY_28NM;
        serdesConfig.txEncoding = SERDES_ENCODING_NA;

        /* power UP and configure SERDES */
        rc = mvHwsSerdesPowerCtrl(devNum, 0, BC2_BOARD_MSYS_OOB_PORT_SERDES_CNS, GT_TRUE, &serdesConfig);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, mvHwsSerdesPowerCtrl);
    }
#endif /* DEBUG_MODE */

    /* set the EGF to filter traffic to ports that are 'link down'.
       state that all ports are currently 'link down' (except for 'CPU PORT') */
    rc = cpssDxChPortManagerInit(ppMapPtr->devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortManagerInit);
    return rc;
}

/**
* @internal cpssAppPlatformAfterPhase2Init function
* @endinternal
*
* @brief   After phase2 PP configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS cpssAppPlatformAfterPhase2Init
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_PHYSICAL_PORT_NUM  port;   /* port number */
    GT_STATUS             rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

#ifndef ASIC_SIMULATION
    /******************************************************/
                    CPSS_TBD_BOOKMARK_BOBCAT3
    /* BC3 board got stuck - code should be checked later */
    /******************************************************/
#if DEBUG_MODE
/* TODO LATER */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssSystemRecoveryStateGet);

    if(system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_BOBCAT3_E)
        {
            rc = gtApplicationPlatformXPhyFwDownload(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, gtApplicationPlatformXPhyFwDownload);
        }
    }
#endif /* DEBUG_MODE */
#endif

    if ((PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_FALSE) &&
            (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        for(port = 0; port < CPSS_MAX_PORTS_NUM_CNS;port++)
        {
            if( !(CPSS_PORTS_BMP_IS_PORT_SET_MAC(&PRV_CPSS_PP_MAC(devNum)->existingPorts, port)))
            {
                continue;
            }

            /* split ports between MC FIFOs for Multicast arbiter */
            rc = cpssDxChPortTxMcFifoSet(devNum, port, port%2);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxMcFifoSet);
        }
    }
    /* Init LED interfaces */
    if(PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        rc = appPlatformLedInterfacesInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformLedInterfacesInit);
    }
    CPSS_APP_PLATFORM_LOG_INFO_MAC("After phase2 done...\n");
    return rc;
}

/*
* @internal cpssAppPlatformPpHwInit function
* @endinternal
*
* @brief   initialize Pp phase1 init and WAs.
*
* @param [in] *ppMapPtr          - PCI/SMI info of the device,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if devNum > 31 or device not present.
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPpHwInit
(
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr
)
{
    GT_STATUS                                       rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformPhase1Init(ppMapPtr, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPhase1Init);

    /* Wa Init */
    if(ppProfilePtr->cpssWaNum)
    {
        rc = cpssDxChHwPpImplementWaInit(ppMapPtr->devNum, ppProfilePtr->cpssWaNum, &ppProfilePtr->cpssWaList[0], NULL);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpImplementWaInit);
    }



    CPSS_APP_PLATFORM_LOG_INFO_MAC("Pp HwInit Done...\n");
    return rc;
}

/**
* @internal internal_cpssAppPlatformPpInsert function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] *ppMapPtr        - PP_MAP board profile,
* @param [in] *ppProfileInfo   - Pp Profile,
* @param [in] *systemRecovery  - System Recovery mode
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
static GT_STATUS internal_cpssAppPlatformPpInsert
(
    IN GT_U8                                     devNum,
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC            *systemRecovery
)
{
    GT_STATUS rc         = GT_OK;
    GT_U32    start_sec  = 0;
    GT_U32    start_nsec = 0;
    GT_U32    end_sec    = 0;
    GT_U32    end_nsec   = 0;
    GT_U32    diff_sec   = 0;
    GT_U32    diff_nsec  = 0;
    GT_U32    value      = 0;

    (void)systemRecovery;
    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    if (!ppMapPtr || !ppProfilePtr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    if (ppMapPtr->mapType == CPSS_APP_PLATFORM_PP_MAP_TYPE_LAST_E)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("mapType=%d not supported.\n", ppMapPtr->mapType);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Device %d Already Exists\n", ppMapPtr->devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* devNum should be 0-31 so this number will be used both as hw device number and cpss device number */
    if (devNum >= CPSS_APP_PLATFORM_MAX_PP_CNS)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input Cpss Dev Num is %d - But supported range is 0-%d\n", devNum, CPSS_APP_PLATFORM_MAX_PP_CNS);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssAppPlatformBelly2BellyEnable(ppProfilePtr->belly2belly);

    if(appPlatformDbEntryGet("serdesExternalFirmware", &value) == GT_OK)
    {
        PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_SET(serdesExternalFirmware, (GT_BOOL)value);
    }

    rc = cpssAppPlatformPpHwInit(ppMapPtr, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpHwInit);

    rc = cpssAppPlatformPpPortsInit(ppMapPtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpPortsInit);

    rc = cpssAppPlatformPhase2Init(ppMapPtr->devNum, ppMapPtr, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPhase2Init);

    rc = cpssAppPlatformAfterPhase2Init(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformAfterPhase2Init);

    rc = cpssAppPlatformPpLogicalInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpLogicalInit);

    rc = cpssAppPlatformPpLibInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpLibInit);

    rc = cpssAppPlatformPpGeneralInit(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpGeneralInit);

    rc = cpssAppPlatformAfterInitConfig(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformAfterInitConfig);

    if(NULL != cpssAppUtilsEventHandlerPreInitCb)
    {
        rc = cpssAppUtilsEventHandlerPreInitCb(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppUtilsEventHandlerPreInit);
    }

    /* unmask user events for the device */
    rc = prvEventMaskSet(devNum, CPSS_EVENT_UNMASK_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvEventMaskSet);

    rc = appPlatformDbEntryAdd("portMgr",1);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformDbEntryAdd);

    if(! PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        rc = cpssDxChCscdPortTypeSet(devNum, CPU_PORT, CPSS_PORT_DIRECTION_BOTH_E, CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCscdPortTypeSet);
    }

    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    TIME_DIFF_MAC(start_sec, start_nsec, end_sec, end_nsec, diff_sec, diff_nsec);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppPlatformPpInsert Time is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}

/**
* @internal cpssAppPlatformPpInsert function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] *ppMapPtr        - PP_MAP board profile,
* @param [in] *ppProfileInfo   - Pp Profile,
* @param [in] *systemRecovery  - System Recovery mode
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPpInsert
(
    IN GT_U8                                     devNum,
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC         *ppProfilePtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC            *systemRecovery
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssAppPlatformPpInsert);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssAppPlatformPpInsert(devNum, ppMapPtr, ppProfilePtr, systemRecovery);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssAppPlatformPpRemove function
* @endinternal
*
* @brief   Remove packet processor.
*
* @param[in] devNum       - Device number.
* @param[in] removalType  - Removal type: 0 - Managed Removal,
*                                         1 - Unmanaged Removal,
*                                         2 - Managed Reset
*
* @retval GT_OK           - on success,
* @retval GT_FAIL         - otherwise.
*/
static GT_STATUS internal_cpssAppPlatformPpRemove
(
    IN GT_SW_DEV_NUM                         devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT removalType
)
{
    GT_STATUS                           rc = GT_FAIL;
    GT_PHYSICAL_PORT_NUM                portNum;
    GT_U32                              maxPortNumber = 0;
#ifdef CHX_FAMILY
    GT_U32                              regAddr;
    GT_U32                              vTcamMngItr;
    PRV_CPSS_DXCH_VIRTUAL_TCAM_MNG_STC *vTcamMng;
    GT_U8                               vTcamDev;
    GT_UINTPTR                          slItr;
    GT_U32                              lpmDb = 0;
    GT_U32                              trunkId;
    CPSS_TRUNK_MEMBER_STC               trunkMembersArray[CPSS_MAX_PORTS_NUM_CNS];
    GT_U32                              numOfMembers;
    GT_U32                              trunkMemberItr;
#endif
    GT_BOOL                             isLast     = GT_FALSE;
    GT_BOOL                             isPci      = GT_FALSE;
    GT_BOOL                             isSmi      = GT_FALSE;
    GT_BOOL                             isPipe     = GT_FALSE;
    GT_U32                              start_sec  = 0;
    GT_U32                              start_nsec = 0;
    GT_U32                              end_sec    = 0;
    GT_U32                              end_nsec   = 0;
    GT_U32                              diff_sec   = 0;
    GT_U32                              diff_nsec  = 0;
    CPSS_TRUNK_TYPE_ENT                 trunkType;
    CPSS_PORT_MANAGER_STC               portEventStc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssOsTimeRT(&start_sec, &start_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    if (0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("PP [%d] Not Initialized\n",devNum);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    maxPortNumber = PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[devNum]; /*PRV_CPSS_PP_MAC(devNum)->numOfPorts;*/

    PRV_IS_LAST_DEVICE_MAC(devNum, isLast);

    switch (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->mngInterfaceType)
    {
        case CPSS_CHANNEL_PEX_MBUS_E:
        case CPSS_CHANNEL_PEX_EAGLE_E:
        case CPSS_CHANNEL_PEX_FALCON_Z_E:
            isPci = GT_TRUE;
            break;
        case CPSS_CHANNEL_SMI_E:
            isSmi = GT_TRUE;
            break;
        default:
            break;
    }

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PX_FAMILY_PIPE_E)
    {
        isPipe = GT_TRUE;
    }

    if (isPci)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("%s Removal of PP on PCI Bus %d  Dev %d device \n",
                (removalType == CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)? "Unmanaged" : "Managed",
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].hwAddr.busNo,
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].hwAddr.devSel
                );
    }
    else if (isSmi)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("%s Removal of PP of SMI slave Dev %d device \n",
                (removalType == CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)? "Unmanaged" : "Managed",
                PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].hwAddr.devSel
                );
    }

    /*Kill PM task*/
    rc = cpssAppPlatformPmTaskDelete();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPmTaskDelete);

    /* TBD : TODO If system recovery porcess is fastBoot then skip HW disable steps
       if(appDemoPpConfigList[devNum].systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E)
       {
       removalType = CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E;
       }
     */

    /* HW disable steps - Applicable for Managed Removal/Restart*/
    if (removalType != CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E)
    {
        /* Disable interrupts : interrupt bits masked int cause registers*/
        /* this call causing crash in AC5x with internal cpu */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5X_E) &&
                (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5P_E))
        {
            rc = cpssPpInterruptsDisable(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,cpssPpInterruptsDisable);
        }

        if (isPipe != GT_TRUE)
        {
#ifdef CHX_FAMILY
            /* Link down the ports */
            for (portNum = 0; portNum < maxPortNumber; portNum++)
            {
                CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                /* Link down ports */
                portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_FORCE_LINK_DOWN_E;
                rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortManagerEventSet);

#ifndef ASIC_SIMULATION
                /*Make Link partner down*/
                rc= cpssDxChPortSerdesTxEnableSet(devNum, portNum, GT_FALSE);
                if (rc != GT_OK && rc != GT_NOT_SUPPORTED)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortSerdesTxEnableSet ret=%d", rc);
                    return rc;
                }
#endif
            }
            /* Disable CPU traffic */
            for (portNum = 0; portNum < maxPortNumber; portNum++)
            {
                CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
                /* Disable PP-CPU Traffic-egress */
                rc = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortEgressFrwFilterSet);

                /* Disable PP-CPU Traffic-ingress */
                rc = cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortIngressFrwFilterSet);
            }

            if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
            {
                regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
                rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 1);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,prvCpssHwPpSetRegField);
            }

            for (portNum = 0 ; portNum < CPSS_MAX_PORTS_NUM_CNS; portNum++)
            {
                rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, portNum, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaToCpuPerPortSet);
            }
#endif
        }

        /* Masks unified events specific to device*/
        rc = prvEventMaskSet(devNum, CPSS_EVENT_MASK_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvEventMaskSet);

    }

    /* SW cleanup - Applicable for all removal Types*/
    if (isPipe != GT_TRUE)
    {
#ifdef CHX_FAMILY
        GT_U8    devListArr[1];
        /* Remove device from LPM DB*/
        devListArr[0] = CAST_SW_DEVNUM(devNum);
        slItr = 0;
        while ((rc = prvCpssDxChIpLpmDbIdGetNext(&lpmDb, &slItr)) == GT_OK)
        {
            rc = cpssDxChIpLpmDBDevsListRemove(lpmDb, devListArr, 1);
            if (rc != GT_OK && rc != GT_NOT_FOUND)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpLpmDBDevsListRemove ret=%d", rc);
                return rc;
            }
        }

        /* Delete LPM DB while removing last device*/
        if (isLast == GT_TRUE)
        {
            rc = prvCpssAppPlatformIpLpmLibReset();
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformIpLpmLibReset);
        }

        /* Remove Trunks from device */
        for (trunkId = 0; trunkId < (PRV_CPSS_DEV_TRUNK_INFO_MAC(devNum)->numberOfTrunks); trunkId++)
        {
            if (prvCpssGenericTrunkDbIsValid(devNum, trunkId) == GT_OK)
            {

                rc = cpssDxChTrunkDbTrunkTypeGet(devNum,trunkId,&trunkType);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,cpssDxChTrunkDbTrunkTypeGet);

                if(CPSS_TRUNK_TYPE_CASCADE_E==trunkType)
                {
                   continue;
                }
                else
                {
                    numOfMembers = CPSS_MAX_PORTS_NUM_CNS;
                    rc = cpssDxChTrunkDbEnabledMembersGet(devNum, trunkId, &numOfMembers, trunkMembersArray);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkDbEnabledMembersGet);
                }

                /* Remove enabled trunk members */

                for (trunkMemberItr = 0; trunkMemberItr < numOfMembers; trunkMemberItr++)
                {
                    rc = cpssDxChTrunkMemberRemove(devNum, trunkId, &trunkMembersArray[trunkMemberItr]);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc,cpssDxChTrunkMemberRemove);
                }
                /* Remove disabled trunk members */
                numOfMembers = CPSS_MAX_PORTS_NUM_CNS;
                rc = cpssDxChTrunkDbDisabledMembersGet(devNum, trunkId, &numOfMembers, trunkMembersArray);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkDbDisabledMembersGet);

                for (trunkMemberItr = 0; trunkMemberItr < numOfMembers; trunkMemberItr++)
                {
                    rc = cpssDxChTrunkMemberRemove(devNum, trunkId, &trunkMembersArray[trunkMemberItr]);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkMemberRemove);
                }
            }
        }

        for (vTcamMngItr = 0; vTcamMngItr < CPSS_DXCH_VIRTUAL_TCAM_MNG_MAX_ID_CNS; vTcamMngItr++)
        {
            vTcamMng = prvCpssDxChVirtualTcamDbVTcamMngGet(vTcamMngItr);
            if (vTcamMng != NULL)
            {
                rc = prvCpssDxChVirtualTcamDbDeviceIteratorGetFirst(vTcamMng, &vTcamDev);
                if (rc == GT_NO_MORE)
                {
                    continue;
                }
                else if (rc != GT_OK)
                {
                    return rc;
                }
                if (devNum == vTcamDev)
                {
                    /* Remove device from default vTcam manager */
                    rc = cpssDxChVirtualTcamManagerDevListRemove(vTcamMngItr, &vTcamDev, 1);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChVirtualTcamManagerDevListRemove);
                }
            }
        }
#endif /*CHX_FAMILY*/
    }

    /* Delete User Event handlers */
    rc =  cpssAppPlatformEventHandlerReset(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEventHandlerReset);

    /* Deallocate FUQ, AUQ, SDMA Tx/Rx Bescriptors & Buffers */
    if (isPci)
    {
#ifdef CHX_FAMILY
#ifndef ASIC_SIMULATION
        rc = prvCpssAppPlatformDeAllocateDmaMem(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformDeAllocateDmaMem);
#else
        rc = osCacheDmaFreeAll();
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFreeAll);
#endif
#endif
    }

    if(!(removalType == CPSS_APP_PLATFORM_PP_REMOVAL_UNMANAGED_E || removalType == CPSS_APP_PLATFORM_PP_REMOVAL_NORESET_E))
    {
        if (isPipe != GT_TRUE)
        {
#ifdef CHX_FAMILY
            /* Disable All Skip Reset options ,exclude PEX */
            /* this Enable Skip Reset for PEX */
            if (PRV_CPSS_HW_IF_PCI_COMPATIBLE_MAC(devNum))
            {
                rc = cpssDxChHwPpSoftResetSkipParamSet(devNum, CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpSoftResetSkipParamSet);
            }

            rc = cpssDxChHwPpSoftResetTrigger(devNum);
            if (GT_OK != rc)
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChHwPpSoftResetTrigger ret=%d", rc);
#endif
        }
    }

    rc = prvCpssAppPlatformPpPhase1ConfigClear(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformPpPhase1ConfigClear);

    if (isPipe != GT_TRUE)
    {
#ifdef CHX_FAMILY
        rc = cpssDxChCfgDevRemove(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevRemove);
#endif
    }

    rc = cpssAppPlatformProfileDbDelete(devNum, CPSS_APP_PLATFORM_PROFILE_DB_ENTRY_TYPE_PP_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileDbDelete);

    if (isLast)
    {
        rc = cpssAppPlatformProfileDbReset();
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileDbReset);
    }

    if (rc != GT_OK)
    {
        return rc;
    }

#ifdef ASIC_SIMULATION
    cpssSimSoftResetDoneWait();
#endif

    rc = cpssOsTimeRT(&end_sec, &end_nsec);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssOsTimeRT);

    TIME_DIFF_MAC(start_sec, start_nsec, end_sec, end_nsec, diff_sec, diff_nsec);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("cpssAppPlatformPpRemove Time is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    return rc;
}

/**
* @internal cpssAppPlatformPpRemove function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] ppRemovalType    - Removal type: 0 - Managed Removal,
*                                              1 - Unmanaged Removal,
*                                              2 - Managed Reset
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPpRemove
(
    IN GT_SW_DEV_NUM                         devNum,
    IN CPSS_APP_PLATFORM_PP_REMOVAL_TYPE_ENT ppRemovalType
)
{
    GT_STATUS rc = GT_OK;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssAppPlatformPpRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = internal_cpssAppPlatformPpRemove(devNum, ppRemovalType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* CPSS Applicaiton Platform - PP configuration APIs */
/**
* @internal cpssAppPlatformPhyPortOperGet function
* @endinternal
*
* @brief   Initialize a specific PP based on profile.
*
* @param [in] devNum           - CPSS device Number,
* @param [in] portNum          - Removal type: 0 - Managed Removal,
*                                              1 - Unmanaged Removal,
*                                              2 - Managed Reset
* @param [out] enable          - CPSS device Number,
* @param [out] speed           - CPSS device Number,
* @param [out] duplex          - CPSS device Number,
*
* @retval GT_OK                - on success,
* @retval GT_BAD_PARAM         - if devNum > 31 or device not present.
* @retval GT_FAIL              - otherwise.
*/
GT_STATUS cpssAppPlatformPhyPortOperGet
(
   IN  GT_SW_DEV_NUM            devNum,
   IN  GT_U32                   portNum,
   OUT GT_BOOL                  *enable,
   OUT GT_U32                   *speed,
   OUT GT_BOOL                  *duplex
)
{   devNum = devNum;
    portNum=portNum;
    enable = enable;
    speed = speed;
    duplex = duplex;

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}

