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
* @file cpssAppReferenceInit.c
*
* @brief This file includes functions to be called on cpss app platform refernce
* initialization,
*
* @version   47
********************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    #include <gtStack/gtOsSocket.h>
    #include <gtOs/gtOsTask.h>
    #include <gtOs/gtOsTimer.h>
    #include <gtUtil/gtBmPool.h>
    #include <gtOs/gtOsGen.h>
    #include <gtOs/gtOsMem.h>
    #include <gtOs/gtOsIo.h>
#ifdef __cplusplus
}
#endif /* __cplusplus */

#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/shell/cmdMain.h>

#include <cpssAppPlatformBoardConfig.h>

#ifndef CPSS_APP_REF_OSMEM_DEFAULT_SIZE_CNS
    /* Default memory size */
    #define CPSS_APP_REF_OSMEM_DEFAULT_SIZE_CNS (2048*1024)
#endif

/**
* @internal cpssAppRefOsFuncBind function
* @endinternal
*
* @brief   This function call CPSS to do initial initialization.
*
* @retval GT_OK      - on success.
* @retval GT_FAIL    - on failure.
*
* @note This function must be called before phase 1 init.
*
*/
GT_STATUS cpssAppRefOsFuncBind
(
    GT_VOID
)
{
    GT_STATUS             rc = GT_OK;
    CPSS_OS_FUNC_BIND_STC osFuncBind;

#if defined(SHARED_MEMORY)
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E, &osFuncBind);
#else
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E, &osFuncBind);
#endif
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssExtServicesBind(NULL, &osFuncBind, NULL);
    if(rc != GT_OK)
    {
        return rc;
    }

    return rc;
} /* cpssAppRefOsFuncBind */

/***
* @internal cpssAppPlatformWelcome function
* #endinternal
*
* @brief This function is assigned to cmdAppShowBoardsList
*
* @retval GT_OK                    - on success, Always sucess
**/
GT_STATUS cpssAppPlatRefWelcome(GT_VOID)
{
    osPrintf("\nCPSS Application Platform Reference:\n");
    return GT_OK;
}

#define BIND_APP_LEVEL_FUNC(infoPtr,level,funcName)     \
        infoPtr->level.funcName = funcName

