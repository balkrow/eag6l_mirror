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
* @file gtOsGlobalDbInterface.h
*
* @brief This file provide interface to  global variables data base.
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsGlobalDbInterface
#define __gtOsGlobalDbInterface

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*macro for setting and getting shared global variables*/
#define PRV_HELPER_SHARED_GLOBAL_VAR_GET(_var) osSharedGlobalVarsPtr->_var
#define PRV_HELPER_SHARED_GLOBAL_VAR_SET(_var,_value) osSharedGlobalVarsPtr->_var = _value

#define PRV_SHARED_DEVICE_INFO_DB PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxMemSrc.dmaDeviceInfo)

/*macro for setting and getting non-shared global variables*/
#define PRV_HELPER_NON_SHARED_GLOBAL_VAR_GET(_var) osNonSharedGlobalVarsPtr->_var
#define PRV_HELPER_NON_SHARED_GLOBAL_VAR_SET(_var,_value) osNonSharedGlobalVarsPtr->_var = _value

#define PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(_var) osNonSharedGlobalVars._var
#define PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(_var,_value) osNonSharedGlobalVars._var = _value

/*#include <gtOs/gtOsMem.h>*/


/**
* @internal gtOsHelperGlobalDbInit function
* @endinternal
*
* @brief                       Initialize OS layer  global variables data base.
*                              Global variables data base is allocated.
*                              In case useSharedMem equal GT_TRUE then also shared memory is allocated for shared variables.
*                              After memory is allocated ,the global variables are being initialized to init values.
* @param[in] aslrSupport    - whether shared memory should be used for shared variables.
*                              in case equal GT_FALSE static  memory allocation is used for shared variables,
*                              otherwise shared memory is used s used for shared variables,
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/
GT_STATUS gtOsHelperGlobalDbInit
(
    GT_BOOL aslrSupport,
    const char * callerFuncName,
    GT_U32       callerLine
);

/**
* @internal gtOsHelperGlobalDbDestroy function
* @endinternal
*
* @brief                   De-Initialize OS layer  global variables data base.
*                          Global variables data base is deallocated.
*                          In case shared memory is used for shared variables,shared memory is unlinked.
*
* @param[in] unlink        -whether shared memory should be unlinked.
*
*
* @retval GT_OK            - on success
* @retval GT_FAIL          - on error
*/

GT_STATUS gtOsHelperGlobalDbDestroy
(
    GT_BOOL unlink
);

/**
* @internal osGlobalSharedDbLock function
* @endinternal
*
* @brief  Lock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS osGlobalSharedDbLock
(
    GT_VOID
);


/**
* @internal osGlobalSharedDbUnLock function
* @endinternal
*
* @brief  Unlock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS osGlobalSharedDbUnLock
(
    GT_VOID
);


GT_VOID osGlobalDbNonSharedDbPpMappingStageDone
(
   GT_VOID

);

GT_VOID osGlobalDbNonSharedDbPciePpMappingSet
(
   const char *fname,
   void *vaddr
);


GT_STATUS prvOsHelperGlobalDbInitParamsGet
(
    GT_U32                  *clientNumPtr,
    GT_U32                  *initPidPtr
);

GT_VOID osGlobalDbNonSharedDbMbusPpMappingSet
(
   GT_U32 res,
   void *vaddr
);


/*DMA config access*/

#define PRV_OS_DMA_CURR_WINDOW \
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigCurrentWindow)

#define PRV_OS_DMA_CURR_DEVICE \
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigCurrentDevice)

#define PRV_OS_DMA_CURR_DEVICE_CONFIGURED \
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigCurrentDeviceConfigured)


#define PRV_OS_DMA_CURR_WINDOW_SET(_win) \
    PRV_OS_DMA_CURR_WINDOW = _win

#define PRV_OS_DMA_CURR_DEVICE_SET(_device) \
    PRV_OS_DMA_CURR_DEVICE = _device;PRV_OS_DMA_CURR_DEVICE_CONFIGURED=GT_TRUE

#define PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.dmaConfig[PRV_OS_DMA_CURR_WINDOW]._var,_value)

#define PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(_var) \
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[PRV_OS_DMA_CURR_WINDOW]._var)

#define PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(_var) \
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb._var)

#ifdef  WIN32
     #define  globalDbOsFuncName __FUNCTION__
#else
     #define  globalDbOsFuncName __func__
#endif



#define SHM_PRINT(fmt, ...) \
    do \
    { \
        if(osNonSharedGlobalVars.osNonVolatileDb.verboseMode)\
            osPrintf("[SHM_DBG] %s[%d] " fmt,  globalDbOsFuncName, __LINE__, ## __VA_ARGS__);\
    }while(0)


#define PRV_OS_SHM_LOG_ENTER    SHM_PRINT("Enter %s\n",globalDbOsFuncName);
#define PRV_OS_SHM_LOG_EXIT     SHM_PRINT("Exit %s\n",globalDbOsFuncName);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtOsGlobalDbInterface */

