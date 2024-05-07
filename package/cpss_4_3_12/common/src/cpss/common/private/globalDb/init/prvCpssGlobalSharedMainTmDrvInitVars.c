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
* @file prvCpssGlobalSharedMainTmDrvInitVars.c
*
* @brief This file  Initialize global non shared variables used in  module:mainTmDrv
*
* @version   1
********************************************************************************
*/

#if defined (INCLUDE_TM)

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
 * @internal
 *           cpssGlobalSharedDBMainTMDrvModPlatformDirDataSectionInit
 *           function
 * @endinternal
 *
 * @brief   Initialize global shared variables used in :
 *          module:mainTmDrv
 *          dir:platform
 *
 */
static GT_VOID cpssGlobalSharedDBMainTMDrvModPlatformDirDataSectionInit
(
    GT_VOID
)
{
    PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(isInitialized) = GT_FALSE;
    PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_log_write_requests) = 0;
    PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_log_read_requests) = 0;
    PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(tm_pAliasingDummy) = NULL;
}

/**
 * @internal cpssGlobalSharedDbMainTmDrvModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global shared variables used in : module:mainTmDrv
 *
 */
GT_VOID cpssGlobalSharedDbMainTmDrvModDataSectionInit
(
    GT_VOID
)
{
    cpssGlobalSharedDBMainTMDrvModPlatformDirDataSectionInit();
}
#endif
