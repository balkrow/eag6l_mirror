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
* @file cpssAppPlatformPciConfig.c
*
* @brief Prestera Devices pci initialization & detection module.
*
* @version   1
********************************************************************************
*/

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformPciConfig.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef ASIC_SIMULATION
    #include <asicSimulation/SCIB/scib.h>
#endif /* ASIC_SIMULATION */

#define PRV_APP_REF_PCI_CONFIG_VAR(_var) PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPciConfigSrc._var)


#define END_OF_TABLE    0xFFFFFFFF
/* DB to hold the device types that the CPSS support */
extern const struct {
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;
    GT_U32                     numOfPorts;
    const CPSS_PP_DEVICE_TYPE *devTypeArray;
    const CPSS_PORTS_BMP_STC  *defaultPortsBmpPtr;
    const GT_VOID_PTR         *coresInfoPtr;
    const GT_U32              *activeCoresBmpPtr;
}cpssSupportedTypes[];

/**
* @internal getDeviceFamily function
* @endinternal
*
* @brief   Gets the device family from vendor Id and device Id (read from PCI bus)
*
* @param[in] pciDevVendorIdPtr     - pointer to PCI/PEX device identification data.
*
* @param[out] devFamilyPtr         - pointer to CPSS PP device family.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getDeviceFamily
(
    IN PCI_DEV_VENDOR_ID_STC    *pciDevVendorIdPtr,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamilyPtr
)
{
    CPSS_PP_DEVICE_TYPE deviceType = pciDevVendorIdPtr->devId << 16 | pciDevVendorIdPtr->vendorId;
    GT_U32  i = 0;
    GT_U32  j = 0;
    GT_BOOL found = GT_FALSE;

    for (i=0; cpssSupportedTypes[i].devFamily != END_OF_TABLE; i++)
    {
        for (j=0; cpssSupportedTypes[i].devTypeArray[j] != END_OF_TABLE; j++)
        {
            if (deviceType == cpssSupportedTypes[i].devTypeArray[j])
            {
                found = GT_TRUE;
                break;
            }
        }
        if(found == GT_TRUE)
            break;
    }

    if (GT_TRUE == found)
    {
        *devFamilyPtr = cpssSupportedTypes[i].devFamily;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/************ PCI Auto Scan Support *************/

#define MAX_DEV_IDS_CNS  300

#define IS_DEVICE_CAN_BE_VB_MUX_DEVICE_MAC(devId)    \
    ((((devId) >> 12) == 0xD) ? 1 :    /*dxCh devices*/ \
     (((devId) >> 8)  == 0x0D) ? 1 :   /*dxSal devices*/\
     0)

#define IS_XCAT_DEV_CPU_ENABLED_MAC(devId)    \
        ((((devId) & 0x2) == 0) ? GT_TRUE : GT_FALSE)



/* init the device[] array                                                    */
/* NOTES :                                                                    */
/* 1.This function will arange the Dx devices as last                         */
/* devices to support VB with ExMx/ExMxPm device with the Dx                  */
/* device(s)                                                                  */
/* 2.The XCAT devices with CPU enabled will be last.                          */
/* This is done to support existing tests for B2B XCAT boards                 */
static GT_STATUS initDevArray(PCI_DEV_VENDOR_ID_STC *devPtr)
{
    GT_U32 ii,jj,kk;
    GT_U32  devId;

    if(PRV_APP_REF_PCI_CONFIG_VAR(initWasDone)== GT_FALSE)
    {
        PRV_APP_REF_PCI_CONFIG_VAR(initWasDone) = GT_TRUE;
    }
    else
    {
        return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
    }

    /* loop on devices that are 'Non CPSS' devices , and we need to look for the end of them */
    for(ii = 0 ; ii < MAX_DEV_IDS_CNS;ii++)
    {
        if(devPtr[ii].vendorId == 0)
        {
            break;
        }
    }

    if(ii == MAX_DEV_IDS_CNS)
    {
        PRV_APP_REF_PCI_CONFIG_VAR(firstStatus) = GT_FULL;
        return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
    }

    /* now ii is the index to start the adding to device[] of the cpss devices */


    /***********************************************************************/
    /*on first loop add only devices that can't be use as MUX device in VB */
    /***********************************************************************/

    jj=0;
    while(cpssSupportedTypes[jj].devFamily != END_OF_TABLE)
    {
        kk = 0;
        while(cpssSupportedTypes[jj].devTypeArray[kk] != END_OF_TABLE)
        {
            devId = (cpssSupportedTypes[jj].devTypeArray[kk] >> 16);

            if(IS_DEVICE_CAN_BE_VB_MUX_DEVICE_MAC(devId))
            {
                /* the device can be mux so we skip it on the first loop of devices */
                /* we will add those devices on the 'Second loop' */
                /* XCAT devices can be mux */
                kk++;

                /* don't increment ii here */

                continue;
            }

            devPtr[ii].vendorId = (GT_U16)(cpssSupportedTypes[jj].devTypeArray[kk] & 0xFFFF);
            devPtr[ii].devId    = (GT_U16) devId;

            ii++;
            if(ii == MAX_DEV_IDS_CNS)
            {
                PRV_APP_REF_PCI_CONFIG_VAR(firstStatus) = GT_FULL;
                return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
            }

            kk++;
        }
        jj++;
    }

    /**********************************************************************/
    /*on the second loop add only devices that CAN be use as MUX device in VB */
    /**********************************************************************/
    jj=0;
    while(cpssSupportedTypes[jj].devFamily != END_OF_TABLE)
    {
        kk = 0;
        while(cpssSupportedTypes[jj].devTypeArray[kk] != END_OF_TABLE)
        {
            devId = (cpssSupportedTypes[jj].devTypeArray[kk] >> 16);
            if(!IS_DEVICE_CAN_BE_VB_MUX_DEVICE_MAC(devId) ||
                    (cpssSupportedTypes[jj].devFamily == CPSS_PP_FAMILY_DXCH_XCAT_E &&
                     IS_XCAT_DEV_CPU_ENABLED_MAC(devId)))

            {
                /* the device CAN'T be mux or XCAT dev with CPU enabled,*/
                /* so we skip it on the second loop of devices */
                kk++;

                /* don't increment ii here */

                continue;
            }
            devPtr[ii].vendorId = (GT_U16)(cpssSupportedTypes[jj].devTypeArray[kk] & 0xFFFF);
            devPtr[ii].devId    = (GT_U16) devId;

            ii++;
            if(ii == MAX_DEV_IDS_CNS)
            {
                PRV_APP_REF_PCI_CONFIG_VAR(firstStatus) = GT_FULL;
                return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
            }

            kk++;
        }
        jj++;
    }

    /**********************************************************************/
    /*on the third loop add XCAT devices with CPU enabled                     */
    /**********************************************************************/
    jj=0;
    while(cpssSupportedTypes[jj].devFamily != END_OF_TABLE)
    {
        kk = 0;
        if (cpssSupportedTypes[jj].devFamily != CPSS_PP_FAMILY_DXCH_XCAT_E)
        {
            jj++;
            continue;
        }
        while(cpssSupportedTypes[jj].devTypeArray[kk] != END_OF_TABLE)
        {
            devId = (cpssSupportedTypes[jj].devTypeArray[kk] >> 16);
            if (!IS_XCAT_DEV_CPU_ENABLED_MAC(devId))
            {
                kk++;
                continue;
            }
            devPtr[ii].vendorId = (GT_U16)(cpssSupportedTypes[jj].devTypeArray[kk] & 0xFFFF);
            devPtr[ii].devId    = (GT_U16) devId;

            ii++;
            if(ii == MAX_DEV_IDS_CNS)
            {
                PRV_APP_REF_PCI_CONFIG_VAR(firstStatus) = GT_FULL;
                return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
            }

            kk++;
        }
        jj++;
    }

    /* put here ONLY devices that are 'unknown' to the CPSS */
    devPtr[ii].vendorId     = 0x11AB;
    devPtr[ii++].devId      = 0xF950;

    if(ii == MAX_DEV_IDS_CNS)
    {
        PRV_APP_REF_PCI_CONFIG_VAR(firstStatus) = GT_FULL;
        return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
    }

    devPtr[ii].vendorId = 0;/* set new 'end of array' */

    PRV_APP_REF_PCI_CONFIG_VAR(firstStatus) = GT_OK;
    return PRV_APP_REF_PCI_CONFIG_VAR(firstStatus);
}

/**
* @internal prvCpssAppPlatformGetPciDevAutoScan function
* @endinternal
*
* @brief   This routine search for Prestera Devices Over the PCI.
*
* @param[in] first                 - whether to bring the first device, if GT_FALSE return the next
*                                    device.
*
* @param[out] pciInfo              - the next device PCI info.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
* @retval GT_NO_MORE               - no more prestera devices.
*/
GT_STATUS prvCpssAppPlatformGetPciDevAutoScan
(
    IN  GT_BOOL         first,
    OUT PCI_INFO_STC    *pciInfo
)
{
    GT_U32      busNo;                   /* pci bus number */
    GT_U32      deviceNo;                /* PCI device number */
    GT_U32      funcNo;                  /* PCI function number */
    GT_U8       i;
    GT_STATUS   rc;                      /* function return value */

    PCI_DEV_VENDOR_ID_STC device[MAX_DEV_IDS_CNS] =
    {
        {0,0}/* --> must be last 'Non CPSS' device */


        /************************************************************/
        /* entries here will be filled in runtime from the CPSS DB  */
        /* see function initDevArray()                              */
        /* NOTE : this function will arrange the Dx devices as last */
        /* devices to support VB with ExMx/ExMxPm device with the Dx*/
        /* device(s)                                                */
        /***********************************************************/

    };

    /* check parameters */
    if(pciInfo == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* check whether it is the first call */
    if(first == GT_TRUE)
    {
        PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx) = 0;
        PRV_APP_REF_PCI_CONFIG_VAR(instance) = 0;
        PRV_APP_REF_PCI_CONFIG_VAR(numOfDevices) = 0;

        rc = initDevArray(device);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, initDevArray);
    }

#ifdef ASIC_SIMULATION
    /* optimize the number of times calling the device to get PCI/PEX info */
    {
        GT_U32  regData;
        for(/*continue*/;PRV_APP_REF_PCI_CONFIG_VAR(instance) < 256; PRV_APP_REF_PCI_CONFIG_VAR(instance)++)
        {
            scibPciRegRead(PRV_APP_REF_PCI_CONFIG_VAR(instance), 0, 1, &regData);
            if(regData == 0xFFFFFFFF)
            {
                /* the 'PRV_APP_REF_PCI_CONFIG_VAR(instance)' is not exists on the PCI/PEX bus */
                continue;
            }

            for(PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx) = 0; device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)].vendorId; PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)++)
            {
                /* check if the device is 'known device' */
                if(regData == (device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)].vendorId | (GT_U32)(device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)].devId << 16)))
                {
                    busNo = 0;/* not used by asic simulation */
                    deviceNo = PRV_APP_REF_PCI_CONFIG_VAR(instance);
                    funcNo = 0;/* not used by asic simulation */

                    /* found device , update instance for next iteration*/
                    PRV_APP_REF_PCI_CONFIG_VAR(instance)++;

                    goto foundInstanceOnPex_lbl;
                }
            }
        }
        /* get here only when not doing 'goto foundInstanceOnPex_lbl' */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);

    }
#endif /*ASIC_SIMULATION*/


    /*call the BSP PCI facility to get all Prestera devices */
    for(;device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)].vendorId != 0;)
    {
        if(extDrvPciFindDev(device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)].vendorId, device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)].devId,
                    PRV_APP_REF_PCI_CONFIG_VAR(instance)++, &busNo, &deviceNo, &funcNo) != GT_OK)
        {
            PRV_APP_REF_PCI_CONFIG_VAR(instance) = 0; /* first PRV_APP_REF_PCI_CONFIG_VAR(instance) for that device type */
            PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)++;
            continue;
        }

#ifdef ASIC_SIMULATION
foundInstanceOnPex_lbl:
#endif /*ASIC_SIMULATION*/

        PRV_APP_REF_PCI_CONFIG_VAR(numOfDevices)++;

        pciInfo->pciDevVendorId = device[PRV_APP_REF_PCI_CONFIG_VAR(deviceIdx)];
        pciInfo->pciDev         = deviceNo;
        pciInfo->pciBus         = busNo;
        pciInfo->pciFunc        = funcNo;

        /* Get the Pci header info  */
        for(i = 0; i < 64; i += 4)
        {
            rc = extDrvPciConfigReadReg(busNo,deviceNo,funcNo,i,
                    &(pciInfo->pciHeaderInfo[i / 4]));
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPciGetDev);
        }

        return GT_OK;
    }
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
}

/**
* @internal prvCpssAppPlatformSysGetPciInfoAutoScan function
* @endinternal
*
* @brief  Gets the Pci device info through pci scan
* IN devIdx           -   device number
* OUT devFamily       -   Device family type
* OUT pciInfo         -   PCI info of the device
*
* @retval GT_OK       - on success,
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS prvCpssAppPlatformSysGetPciInfoAutoScan
(
    IN GT_U8 devIdx,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamily,
    OUT PCI_INFO_STC *pciInfo
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    devId = 0;
    (void)devIdx;

    rc = prvCpssAppPlatformGetPciDevAutoScan(GT_TRUE, pciInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformGetPciDevAutoScan);

    rc = extDrvPciConfigDev(pciInfo->pciBus, pciInfo->pciDev, pciInfo->pciFunc);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPciConfigDev);

    rc = getDeviceFamily(&pciInfo->pciDevVendorId, devFamily);
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("getDeviceFamily rc=%d ret=%d", rc, GT_FAIL);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    devId = (pciInfo->pciDevVendorId.devId << 16) | (pciInfo->pciDevVendorId.vendorId);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Found device:[0x%8.8x] on pciBus %d pciDev %d\n",devId, pciInfo->pciBus, pciInfo->pciDev);

    return rc;
}

/************ PCI Auto Scan Support *************/


/**
* @internal prvCpssAppPlatformGetPciDev function
* @endinternal
*
* @brief  Gets the Pci device info.
* IN pciBus           -   PCI Bus number
* IN pciDev           -   PCI device number
* IN pciFunc          -   PCI function number
*
* OUT vendorId        -   PCI vendor id
* OUT deviceId        -   PCI device Id
*
* @retval GT_OK       - on success.
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS prvCpssAppPlatformGetPciDev
(
    IN  GT_U8  pciDomain,
    IN  GT_U8  pciBus,
    IN  GT_U8  pciDev,
    IN  GT_U8  pciFunc,
    OUT GT_U16 *vendorId,
    OUT GT_U16 *deviceId
)
{
    GT_STATUS rc;

    rc = extDrvPciGetDev(pciDomain, pciBus, pciDev, pciFunc, vendorId, deviceId);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPciGetDev);

    return rc;
}

/**
* @internal prvCpssAppPlatformSysGetPciInfo function
* @endinternal
*
* @brief  Gets the Pci device info.
* IN devIdx           -   device number
* IN pciBus           -   PCI Bus number
* IN pciDev           -   PCI device number
* OUT devFamily       -   Device family type
* OUT pciInfo         -   PCI info of the device
*
* @retval GT_OK       - on success,
* @retval GT_FAIL     - otherwise.
*/
GT_STATUS prvCpssAppPlatformSysGetPciInfo
(
    IN GT_U8 devIdx,
    IN GT_U8 pciDomain,
    IN GT_U8 pciBus,
    IN GT_U8 pciDev,
    IN GT_U8 pciFunc,
    OUT CPSS_PP_FAMILY_TYPE_ENT *devFamily,
    OUT PCI_INFO_STC *pciInfo
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    devId = 0;
    (void)devIdx;

    rc = prvCpssAppPlatformGetPciDev(pciDomain, pciBus, pciDev, pciFunc, &(pciInfo->pciDevVendorId.vendorId),
            &(pciInfo->pciDevVendorId.devId));
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssAppPlatformGetPciDev);

    pciBus = pciBus + (256 * pciDomain);
    rc = extDrvPciConfigDev(pciBus, pciDev, 0);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPciConfigDev);

    rc = getDeviceFamily(&pciInfo->pciDevVendorId, devFamily);
    if(rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("getDeviceFamily rc=%d ret=%d", rc, GT_FAIL);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    pciInfo->pciDev  = pciDev;
    pciInfo->pciBus  = pciBus;
    pciInfo->pciFunc = pciFunc;

    devId = (pciInfo->pciDevVendorId.devId << 16) | (pciInfo->pciDevVendorId.vendorId);

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Found device:[0x%8.8x] on pciBus %d pciDev %d\n",devId, pciInfo->pciBus, pciInfo->pciDev);

    return rc;
}
