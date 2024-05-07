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

#ifdef CPSS_APP_PLATFORM


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
static GT_VOID cpssGlobalSharedDBAskModAppUtilsSrcDataSectionInit
(
    GT_VOID
)
{

#ifdef GM_USED
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssAppUtilsCommonSrc.useSinglePipeSingleDp) = 1;
#else /*!GM_USED*/
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssAppUtilsCommonSrc.useSinglePipeSingleDp) = 0;
#endif /*!GM_USED*/
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssAppUtilsCommonSrc.appRefPortMgrMode) = GT_FALSE;

#ifdef ASIC_SIMULATION
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPortInitSrc.appRefPortManagerTaskSleepTime) = 10;/*100 times in sec*/
#else
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPortInitSrc.appRefPortManagerTaskSleepTime) = 100;/*10 times in sec */
#endif

    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPpUtilsSrc.applicationPlatformTtiTcamUseOffset) = GT_TRUE ;
    PRV_SHARED_GLOBAL_VAR_GET(appRefMod.cpssPpUtilsSrc.applicationPlatformPclTcamUseIndexConversion) = GT_TRUE ;

}

/**
 * @internal cpssGlobalSharedDbMainTmDrvModDataSectionInit function
 * @endinternal
 *
 * @brief  Initialize global shared variables used in : module:mainTmDrv
 *
 */
GT_VOID cpssGlobalSharedDbAskModDataSectionInit
(
    GT_VOID
)
{
    cpssGlobalSharedDBAskModAppUtilsSrcDataSectionInit();
}
#endif
