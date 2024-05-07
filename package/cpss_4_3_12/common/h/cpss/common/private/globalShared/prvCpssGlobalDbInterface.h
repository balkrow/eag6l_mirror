/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file prvCpssGlobalDbInterface.h
*
* @brief This file provide interface to  global variables data base.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssGlobalDbInterface
#define __prvCpssGlobalDbInterface

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*macro for setting and getting shared global variables*/
#define PRV_SHARED_GLOBAL_VAR_GET(_var) cpssSharedGlobalVarsPtr->_var
#define PRV_SHARED_GLOBAL_VAR_SET(_var,_value) cpssSharedGlobalVarsPtr->_var = _value

/*macro for setting and getting non-shared global variables*/
#define PRV_NON_SHARED_GLOBAL_VAR_GET(_var) cpssNonSharedGlobalVarsPtr->_var
#define PRV_NON_SHARED_GLOBAL_VAR_SET(_var,_value) cpssNonSharedGlobalVarsPtr->_var = _value

#define PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(_var) cpssNonSharedGlobalVars._var
#define PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(_var,_value) cpssNonSharedGlobalVars._var = _value



#define PRV_NON_SHARED_GLOBAL_VAR_LOG_GET() \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(nonVolatileDb.cpssLogDb.prvCpssLogEnabled)

#define PRV_NON_SHARED_GLOBAL_VAR_LOG_SET(_value) \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_SET(nonVolatileDb.cpssLogDb.prvCpssLogEnabled,_value)

#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_DEBUG \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.systemRecoveryDebugDb)

#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_INFO \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.systemRecoveryInfo)

#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_ERROR_INJECTION_DB \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.errorInjection)

#define PRV_NON_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_GENERAL_VAR \
        PRV_NON_SHARED_GLOBAL_VAR_DIRECT_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc)

#define PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_SET(_var,_value)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(nonVolatileDb._var,_value)

#define PRV_NON_SHARED_NON_VOLATILE_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonVolatileDb._var)


#define PRV_SHARED_GLOBAL_VAR_SYSTEM_RECOVERY_MANAGERS_DB \
        PRV_SHARED_GLOBAL_VAR_GET(commonMod.systemRecoveryDir.\
        genSystemRecoverySrc.systemRecoveryManagersDb)

#define PRV_SHARED_GLOBAL_VAR_DB_CHECK() \
    do \
    { \
        if(NULL==cpssSharedGlobalVarsPtr) \
        { \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, \
                                    "cpssSharedGlobalVarsPtr ==NULL\n");\
        } \
    } \
    while(0)

#ifdef  WIN32
     #define  globalDbFuncName __FUNCTION__
#else
     #define  globalDbFuncName __func__
#endif


#define PRV_CPSS_SHM_PRINT(fmt, ...) \
        do \
        { \
            if(cpssNonSharedGlobalVars.nonVolatileDb.generalVars.verboseMode)\
                cpssOsPrintf("[CPSS_SHM_DBG] %s[%d] " fmt,  globalDbFuncName, __LINE__, ## __VA_ARGS__);\
        }while(0)


#define PRV_CPSS_SHM_LOG_ENTER    PRV_CPSS_SHM_PRINT("Enter %s\n",globalDbFuncName);
#define PRV_CPSS_SHM_LOG_EXIT     PRV_CPSS_SHM_PRINT("Exit %s\n",globalDbFuncName);

/** globals defines to replace full path of global variables */

/* shared vaiables */
#define hwsDeviceSpecInfo (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceSpecInfoGlobal))
#define hwsDevicesPortsElementsArray (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.commonSrc.hwsDevicesPortsElementsArrayGlobal))
#define hwsOsMicroDelayPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMicroDelayGlobalPtr))
#define addedRsfecResult (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.rsfecSrc.addedRsfecResultGlobal))
/* For debugging: cpssSharedGlobalVarsPtr->commonMod.labServicesDir.rsfecSrc.addedRsfecResultGlobal */

#define hwsIlknRegDbGetFuncPtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsIlknRegDbGetFuncGlobalPtr))
#define hwsPpHwTraceEnablePtr (PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsPpHwTraceEnableGlobalPtr))

/*non shared*/
#define hwsServerRegFieldSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegFieldSetFuncGlobalPtr))
#define hwsServerRegFieldGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegFieldGetFuncGlobalPtr))
#define hwsServerRegSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegSetFuncGlobalPtr))
#define hwsServerRegGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsServerRegGetFuncGlobalPtr))
#define hwsOsExactDelayPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsExactDelayGlobalPtr))
#define hwsSerdesRegSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegSetFuncGlobalPtr))
#define hwsSerdesRegGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsSerdesRegGetFuncGlobalPtr))
#define hwsDeviceTemperatureGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsDeviceTemperatureGetFuncGlobalPtr))
#define hwsOsMemCopyFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemCopyFuncGlobalPtr))
#define hwsOsTimerWkFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsTimerWkFuncGlobalPtr))
#define hwsOsMemSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMemSetFuncGlobalPtr))
#define hwsOsFreeFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsFreeFuncGlobalPtr))
#define hwsOsMallocFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsMallocFuncGlobalPtr))
#define hwsOsStrCatFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsOsStrCatFuncGlobalPtr))
#define hwsTimerGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsTimerGetFuncGlobalPtr))
#define hwsRegisterSetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterSetFuncGlobalPtr))
#define hwsRegisterGetFuncPtr (PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc.hwsRegisterGetFuncGlobalPtr))



#define DUPLICATED_ADDR_GETFUNC(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].duplicatedAddrGetFunc)


#define PRV_HW_REG_TO_UNIT_ID(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].hwInfoCallbacks.\
                                                    prvCpssHwRegAddrToUnitIdConvertFunc)

#define PRV_DMA_DP_LOCAL_TO_GLOBAL_NUM_CONVERT_GET_NEXT(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].hwInfoCallbacks.\
                                                    prvCpssDmaLocalNumInDpToGlobalNumConvertGetNextFunc)

#define PRV_OFFSET_FROM_FIRST_INSTANCE_GET(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].hwInfoCallbacks.\
                                                    prvCpssOffsetFromFirstInstanceGetFunc)

#define PRV_CPSS_UNIT_ID_SIZE_IN_BYTE_GET(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].hwInfoCallbacks.\
                                                    prvCpssUnitIdSizeInByteGetFunc)


#define PRV_CPSS_ACCESS_TABLE_INFO_PTR(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].accessTableInfoPtr)





#define DMA_GLOBALNUM_TO_LOCAL_NUM_IN_DP_CONVERT_FUNC(_devNum) \
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].prvCpssDmaGlobalNumToLocalNumInDpConvertFunc)

#define DMA_LOCALNUM_TO_GLOBAL_NUM_IN_DP_CONVERT_FUNC(_devNum) \
    PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].prvCpssDmaLocalNumInDpToGlobalNumConvertFunc)


#define PORT_OBJ_FUNC(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].portObjFunc)

#define PRV_TABLE_FORMAT_INFO(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].tableFormatInfo)

#define PRV_PORTS_PARAMS_SUP_MODE_MAP(_devNum) PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].portsParamsSupModesMap)

#define PRV_INTERRUPT_FUNC_GET(_devNum,_func)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum]._func)

#define PRV_INTERRUPT_CTRL_GET(_devNum)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].intCtrlNonShared)

#define PRV_HWS_NUMBER_OF_RAVENS PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.falconDevinitSrc.hwsFalconNumOfRavens)
#define PRV_HWS_SDFW_DOWNLOAD_DONE PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.falconDevinitSrc.hwsSerdesFwDownloadDone)
#define PRV_AAPL_SERDES_DB ((Aapl_t **)(cpssSharedGlobalVarsPtr->commonMod.labServicesDir.mvAvagoSerdesIfSrc.aaplSerdesDb))
#define PRV_AAPL_SERDES_VOID_DB cpssSharedGlobalVarsPtr->commonMod.labServicesDir.mvAvagoSerdesIfSrc.aaplSerdesDb

#define PRV_MPD_SHARED_DB           cpssSharedGlobalVarsPtr->mainPpDrvMod.phyDir.mpdGlobalSharedDb
#define PRV_MPD_NON_SHARED_DB       cpssNonSharedGlobalVars.mainPpDrvMod.phyDir.mpdGlobalNonSharedDb

#define PRV_ERRATA_STC_FIELD_GET(_devNum,_field)  PRV_NON_SHARED_GLOBAL_VAR_GET(nonSharedDeviceSpecificDb[_devNum].errataCallbacks._field)

#define PRV_PER_PROCESS_SWITCH_BASE_GET(_devNum)  cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_devNum].ppMapping.switching.start
#define PRV_PER_PROCESS_CNM_BASE_GET(_devNum)     cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_devNum].ppMapping.cnm.start
#define PRV_PER_PROCESS_DFX_BASE_GET(_devNum)     cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_devNum].ppMapping.resetAndInitController.start
#define PRV_PER_PROCESS_SRAM_BASE_GET(_devNum)    cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_devNum].ppMapping.sram.start

/* macro to free global pointer (given by the 'path') */
#define FREE_GLOBAL_PTR_MAC(path) FREE_PTR_MAC(PRV_SHARED_GLOBAL_VAR_GET(path))
#define FREE_GLOBAL_NON_SHARED_PTR_MAC(path) \
do \
{ \
    if(cpssNonSharedGlobalVarsPtr) \
    {\
        FREE_PTR_MAC(PRV_NON_SHARED_GLOBAL_VAR_GET(path)); \
    }\
}while(0)

/*

Callback ID decoding (32 bit ID):
====================
Bits 31-28 : Unit ID
Bits  27-0  :  Callback function id

*/

#define PRV_UNIT_CB_FUNC_OFFSET  28
#define PRV_CB_FUNC_MASK  ((1<<PRV_UNIT_CB_FUNC_OFFSET)-1)



#ifdef SHARED_MEMORY

#define PRV_PER_PROCESS_BASE_GET(_drv) _drv->common.adressSpaceTypeValid?((_drv->common.adressSpaceType==CPSS_HW_DRIVER_AS_CNM_E)?PRV_PER_PROCESS_CNM_BASE_GET(_drv->common.devNum):\
                                                         (_drv->common.adressSpaceType==CPSS_HW_DRIVER_AS_SWITCHING_E)?PRV_PER_PROCESS_SWITCH_BASE_GET(_drv->common.devNum):\
                                                         (_drv->common.adressSpaceType==CPSS_HW_DRIVER_AS_DFX_E)?PRV_PER_PROCESS_DFX_BASE_GET(_drv->common.devNum):\
                                                         (_drv->common.adressSpaceType==CPSS_HW_DRIVER_AS_DRAM_E)?PRV_PER_PROCESS_SRAM_BASE_GET(_drv->common.devNum):\
                                                         _drv->base):_drv->base

/*
This macro should be used when updating per process /resource  adress space
Another way to update this data base is through cpssGlobalNonSharedDbPpMappingAdd.
The difference is that cpssGlobalNonSharedDbPpMappingAdd update all adress spaces CnM/Swithc/DFX at once.
*/
#define PRV_PER_PROCESS_BASE_SET(_drv) \
    do\
    {\
       switch(_drv->common.adressSpaceType)\
       {\
            case CPSS_HW_DRIVER_AS_CNM_E:\
                cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_drv->common.devNum].ppMapping.cnm.start = _drv->base;\
                break;\
            case CPSS_HW_DRIVER_AS_SWITCHING_E:\
                cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_drv->common.devNum].ppMapping.switching.start = _drv->base;\
                break;\
            case CPSS_HW_DRIVER_AS_DFX_E:\
                cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_drv->common.devNum].ppMapping.resetAndInitController.start = _drv->base;\
                break;\
           case CPSS_HW_DRIVER_AS_DRAM_E:\
                cpssNonSharedGlobalVars.nonSharedDeviceSpecificDb[_drv->common.devNum].ppMapping.sram.start = _drv->base;\
                break;\
           default:\
                break;\
       }\
    }while(0)

#else
#define PRV_PER_PROCESS_BASE_GET(_drv)_drv->base
#define PRV_PER_PROCESS_BASE_SET(_drv)

#endif

#ifdef CPSS_APP_PLATFORM
#define PRV_APP_REF_PORT_MGR_MODE PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssAppUtilsCommonSrc.appRefPortMgrMode)
#define PRV_APP_REF_PP_UTILS_VAR(_var) PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPpUtilsSrc._var)
#define PRV_APP_REF_PP_CONFIG_VAR(_var) PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPpConfigSrc._var)
#define PRV_APP_REF_RUN_TIME_CONFIG_VAR(_var) PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssRunTimeConfigSrc._var)
#endif

GT_U32  prvCpssBmpIsZero
(
    GT_VOID * portsBmpPtr
);

/*NOTE: PRV_CPSS_PORTS_BMP_PORT_SET_ALL_MAC and PRV_CPSS_PORTS_BMP_IS_ZERO_MAC  \
        should be NOT used by  application. This is for CPSS internal use use only*/


#define PRV_CPSS_PORTS_BMP_PORT_SET_ALL_MAC(portsBmpPtr)     \
    cpssOsMemSet(portsBmpPtr,0xFF,sizeof(CPSS_PORTS_BMP_STC))


#define PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(portsBmpPtr)     \
    cpssOsMemSet(portsBmpPtr,0,sizeof(CPSS_PORTS_BMP_STC))

/* all ii 0.. max --> portsBmpTargetPtr[ii] == 0 */
#define PRV_CPSS_PORTS_BMP_IS_ZERO_MAC(portsBmpPtr)     \
    prvCpssBmpIsZero(portsBmpPtr)

/* check if ports bmp in portsBmpPtr1 == portsBmpPtr2
    return 1 - equal
    return 0 - NOT equal
*/
#define PRV_CPSS_PORTS_BMP_ARE_EQUAL_MAC(portsBmpPtr1,portsBmpPtr2)     \
    (cpssOsMemCmp(portsBmpPtr1,portsBmpPtr2,sizeof(CPSS_PORTS_BMP_STC)) ? 0 : 1)

#define PRV_SHARED_PA_DB_VAR(_var)\
      PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChPortDir.portPaSrc._var)

#define PRV_NON_SHARED_PA_DB_VAR(_var)\
      PRV_NON_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.portDir.portPaSrc._var)


#define PRV_CPSS_NON_SHARED_FAMILY_INFO(_family) \
    PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir.prvCpssFamilyInfoArray[_family])


/* non-shared variables */


/**
* @internal cpssGlobalDbInit function
* @endinternal
*
* @brief   Initialize CPSS global variables data base.
*          Global variables data base is allocated.
*          In case useSharedMem equal GT_TRUE then also shared memory is allocated for shared variables.
*          After memory is allocated ,the global variables are being initialized to init values.
* @param[in] aslrSupport - whether shared memory should be used for shared variables.
*                           in case equal GT_FALSE dynamic memory allocation is used for shared variables,
*                           otherwise shared memory is used s used for shared variables.
* @param[in] forceInit - whether to reset  shared data base to default values.
*                           in case equal GT_FALSE shared data base  is initialized only if allocated,
*                           otherwise shared data base  is initialized even if the data base is allocated.
*                           Required for PP reset.
* @param[in] hwInfoPtr - information regarding the resources that already mapped by operating system. In case equal 0 there is no device mapped.
* @param[in] numberOfDevices - information regarding the number of  resources that already mapped by operating system.
*                                                          In case equal 0 there is no device mapped.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

GT_STATUS cpssGlobalDbInit
(
    IN GT_BOOL             aslrSupport,
    IN GT_BOOL             forceInit,
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U32               numberOfDevices
);
/**
* @internal cpssGlobalDbDestroy function
* @endinternal
*
* @brief   UnInitialize CPSS global variables data base.
*          Global variables data base is deallocated.
*          In case  shared memory is used then shared memory is unlinked here.
*
* @param[in] unlink   - whether shared memory should be unlinked.
* @param[in] resetPointers   - whether to reset data base pointers
* @param[out] unlinkDonePtr   -GT_TRUE if this is the last process and unlink is performed
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS cpssGlobalDbDestroy
(
    IN GT_BOOL unlink,
    IN GT_BOOL resetPointers,
    OUT GT_BOOL *unlinkDonePtr
);


/**
* @internal prvCpssGlobalDbExistGet function
* @endinternal
*
* @brief   Check if  gllobal variables data base was allocated.
*
* @retval GT_TRUE                    global DB exist
* @retval GT_FAIL                  - global DB does not exist
*/
GT_BOOL prvCpssGlobalDbExistGet
(
    GT_VOID
);

/**
 * @internal cpssGlobalNonSharedDbExtDrvFuncInit function
 * @endinternal
 *
 * @brief  Initialize global non shared function pointers
 *
 */
GT_VOID cpssGlobalNonSharedDbExtDrvFuncInit
(
    GT_VOID
);

/**
 * @internal cpssGlobalNonSharedDbOsFuncInit function
 * @endinternal
 *
 * @brief  Initialize global non shared OS function pointers
 *
 */
GT_VOID cpssGlobalNonSharedDbOsFuncInit
(
    GT_VOID
);

/**
* @internal osGlobalSharedDbAddProcess function
* @endinternal
*
* @brief add process to pid list(debug)
*
*/
GT_BOOL osGlobalSharedDbAddProcess
(
    GT_VOID
);

/**
* @internal cpssGlobalNonSharedDbPpMappingSyncSecondaryProcess function
* @endinternal
*
* @brief This function update non shared DB with unique PP mapping.
*              Should be called for secondary process.
*
* @param[in] hwInfoPtr - information regarding the devices that already mapped by operating system. In case equal 0 there is no device mapped.
* @param[in] numberOfDevices - information regarding the number of  devices that already mapped by operating system.
*                                                          In case equal 0 there is no device mapped.
*
*
*/
GT_VOID cpssGlobalNonSharedDbPpMappingSyncSecondaryProcess
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U32               numberOfDevices
);

/**
* @internal cpssGlobalNonSharedDbPpMappingSyncPrimaryProcess function
* @endinternal
*
* @brief  This function add PP mapping to  non shared DB.
*               Should be called for master  process.
*               Master process will call this function when creating a driver.
* @param[in] hwInfoPtr - information regarding the device that already mapped .
* @param[in] devNum - device number
*
*
*/
GT_VOID cpssGlobalNonSharedDbPpMappingSyncPrimaryProcess
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U8                devNum
);

#ifdef SHARED_MEMORY
#define PRV_CALLBACK_GET(_id,_casting)\
   ( (_casting *)cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[_id>>PRV_UNIT_CB_FUNC_OFFSET])[_id&PRV_CB_FUNC_MASK]

/**
 * @internal prvCpssCallbackFuncIdGet function
 * @endinternal
 *
 * @brief  Get the pointer to compare function
 *
 * @note   APPLICABLE DEVICES:      All devices
 *
 *
 * @param[in] compareFunc     -compare function pointer
 *
 * @retval ID of compare function.In case not found 0 is returned
 */
GT_U32  prvCpssCallbackFuncIdGet
(
    GT_VOID *  func
);

#define PRV_CALLBACK_ID_GET(_func) prvCpssCallbackFuncIdGet(_func)
/**
* @internal prvCpssDeviceNumberByHwInfoGet function
* @endinternal
*
* @brief  This function find  device number  by hwInfo.
*         Should be called for secondary   process in order to understand which CPSS device number was mapped to
*         specific hwInfo.
*
* @param[in]  hwInfoPtr - information regarding the device that already mapped .
* @param[out] devNumPtr - (pointer to) device number
*
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_FOUND             - such device was not found
*/
GT_STATUS prvCpssDeviceNumberByHwInfoGet
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    OUT GT_U8                *devNumPtr
);


#else
#define PRV_CALLBACK_GET(_func,_casting) _func
#define PRV_CALLBACK_ID_GET(_func) _func

#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGlobalMutexh */

