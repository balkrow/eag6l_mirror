/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *\
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssMainTmDrvModGlobalSharedDb.h
*
* @brief This file define mainTmDrv module shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssMainTmDrvModGlobalSharedDb
#define __prvCpssMainTmDrvModGlobalSharedDb

#if defined (INCLUDE_TM)

#include <platform/tm_regs.h>

#define PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(mainTmDrvMod.platformDir._var)

/**
* @struct PRV_CPSS_MAIN_TM_DRV_MOD_PLATFORM_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in mainTmDrv module , Platform directory
*/
typedef struct
{
    /** is TM alias initialized */
    GT_BOOL isInitialized;
    struct tm_alias TM;

    int     tm_log_write_requests;
    int     tm_log_read_requests;
    void    *tm_pAliasingDummy;
    GT_U64  convert_tm_write_dummy;
    GT_U64  convert_tm_read_dummy;

} PRV_CPSS_MAIN_TM_DRV_MOD_PLATFORM_DIR_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_MAIN_TM_DRV_MOD_SHARED_GLOBAL_DB
 *
 * @brief  Structure contain global variables that are meant to be shared
 * between processes (when CPSS is linked as shared object)
 *   in mainTmDrv module
*/
typedef struct
{
    PRV_CPSS_MAIN_TM_DRV_MOD_PLATFORM_DIR_SHARED_GLOBAL_DB  platformDir;
} PRV_CPSS_MAIN_TM_DRV_MOD_SHARED_GLOBAL_DB;

#endif
#endif  /* __prvCpssMainTmDrvModGlobalSharedDb */
