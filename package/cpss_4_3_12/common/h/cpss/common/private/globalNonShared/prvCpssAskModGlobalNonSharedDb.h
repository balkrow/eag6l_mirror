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
* @file prvCpssAskModGlobalNonSharedDb.h
*
* @brief This file define common/cpssAppPlatform module shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssAskModGlobalNonSharedDb
#define __prvCpssAskModGlobalNonSharedDb

#ifdef CPSS_APP_PLATFORM_REFERENCE

#include <cpssAppPlatformLogLib.h>


/**
* @struct PRV_CPSS_COMMON_APP_REF_DIR_SHARED_PCI_CONFIG_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are meant to be shared
* between processes (when CPSS is linked as shared object)
* in appRef module , common directory,cpssAppPlatformProfileMgr.c source file.
*/
typedef struct
{
    CPSS_APP_PLATFORM_LOG_LEVEL_ENT capLogDb [CPSS_APP_PLATFORM_LOG_MODULE_LAST_E];


} PRV_ASK_COMMON_NON_SHARED_LOG_SRC_GLOBAL_DB;



#endif /*CPSS_APP_PLATFORM_REFERENCE*/
#endif  /* __prvCpssAskModGlobalNonSharedDb */
