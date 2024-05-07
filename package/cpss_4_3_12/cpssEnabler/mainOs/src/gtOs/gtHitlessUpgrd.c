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
* @file gtHitlessUpgrd.c
*
* @brief Implementation for hitless SW upgrade.
*
* @version   1.1.2.1
********************************************************************************
*/

#include <gtOs/gtHitlessUpgrd.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_GT_HITLESS_UPGRD_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtHitlessUpgrdSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_GT_HITLESS_UPGRD_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtHitlessUpgrdSrc._var)


/**
* @internal osHitlessUpgradeDetach function
* @endinternal
*
* @brief   Detaches the mainOs lib
*
* @retval GT_OK                    - operation succeeded
* @retval GT_FAIL                  - operation failed
*/
GT_STATUS osHitlessUpgradeDetach (void)
{
    PRV_SHARED_MAIN_OS_DIR_GT_HITLESS_UPGRD_SRC_GLOBAL_VAR_SET(osAttachStageDone,GT_FALSE);
    return GT_OK;
}


/**
* @internal osHitlessUpgradeAttach function
* @endinternal
*
* @brief   Attaches the mainOs lib to the stored system state.
*
* @param[in] memAllocFuncPtr          - the function to be used in memory allocation
*                                      (refer to osMemLibInit)
*
* @retval GT_OK                    - operation succeeded
* @retval GT_FAIL                  - operation failed
*/
GT_STATUS osHitlessUpgradeAttach
(
    IN   GT_MEMORY_ALLOC_FUNC    memAllocFuncPtr
)
{
    GT_STATUS rc = GT_OK;
    if (GT_FALSE == PRV_SHARED_MAIN_OS_DIR_GT_HITLESS_UPGRD_SRC_GLOBAL_VAR_GET(osAttachStageDone))
    {
        rc = osMemBindMemAllocFunc(memAllocFuncPtr);
        if (rc != GT_OK)
        {
            return rc;
        }
        PRV_SHARED_MAIN_OS_DIR_GT_HITLESS_UPGRD_SRC_GLOBAL_VAR_SET(osAttachStageDone,GT_TRUE);
    }

    return rc;
}




