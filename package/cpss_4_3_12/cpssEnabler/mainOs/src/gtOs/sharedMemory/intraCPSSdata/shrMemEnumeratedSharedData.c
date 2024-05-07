/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#include <gtOs/gtEnvDep.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSem.h>

#ifdef SHARED_MEMORY

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtOsShmemEnumShdataSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtOsShmemEnumShdataSrc._var)

/**
* @internal cpssMultiProcGetSharedData function
* @endinternal
*
* @brief   return enumerated shared data
*
* @param[in] id                       - data identifier
*                                       data (integer or pointer)
*/
uintptr_t cpssMultiProcGetSharedData
(
    IN CPSS_SHARED_DATA_TYPE id
)
{
    if (id >= CPSS_SHARED_DATA_MAX_E)
    {
        return 0;
    }
    return PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(sharedDataArray[id]);
}

/**
* @internal cpssMultiProcSetSharedData function
* @endinternal
*
* @brief   Set enumerated shared data
*
* @param[in] id                       - data identifier
*                                       GT_OK
*                                       GT_FAIL
*/
GT_STATUS cpssMultiProcSetSharedData
(
    IN CPSS_SHARED_DATA_TYPE id,
    IN uintptr_t data
)
{
    if (id >= CPSS_SHARED_DATA_MAX_E)
    {
        return GT_BAD_PARAM;
    }
    PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_SET(sharedDataArray[id],data);
    return GT_OK;
}


/**
* @internal cpssMultiProcSharedDataLock function
* @endinternal
*
* @brief   Lock shared data for critical operations (data alloc)
*/
GT_STATUS cpssMultiProcSharedDataLock(void)
{
    if (!PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(semMtx))
    {
        /* not initialized yet */
        if (osSemBinCreate("MP_EnumeratedMtx", OS_SEMB_FULL, &PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(semMtx)) != GT_OK)
        {
            return GT_FAIL;
        }
    }
    return osSemWait(PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(semMtx), OS_WAIT_FOREVER);
}

/**
* @internal cpssMultiProcSharedDataUnlock function
* @endinternal
*
* @brief   Unlock shared data
*/
GT_STATUS cpssMultiProcSharedDataUnlock(void)
{
    if (!PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(semMtx))
    {
        return GT_FAIL;
    }
    return osSemSignal(PRV_SHARED_MAIN_OS_DIR_SHMEM_ENUM_SHDATA_SRC_GLOBAL_VAR_GET(semMtx));
}

#endif




