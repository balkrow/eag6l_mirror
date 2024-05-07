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
* @file gtOsEnablerModGlobalNonSharedDb.h
*
* @brief This file define cpssEnabler module non-shared variables defenitions.
*
* @version   1
********************************************************************************
*/

#ifndef __gtOsEnablerModGlobalNonSharedDb
#define __gtOsEnablerModGlobalNonSharedDb

#ifdef LINUX
#include <pthread.h>
#endif

#include <gtOs/gtOsIo.h>
#include <gtUtil/gtTimeMeasure.h>
#include <gtUtil/gtMemoryUsageTrace.h>

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_OS_LINUX_IO_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / osLinixIo file
*/
typedef struct
{
    /*add here global non shared variables used in source files*/
    OS_BIND_STDOUT_FUNC_PTR writeFunctionPtr;
    /* Redirect Stdio to network connection: */
    GT_BOOL redirectNetStdio;
} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_OS_LINUX_IO_SRC_GLOBAL_DB;


/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / osLinixMem file
*/
typedef struct
{
   /*add here global non shared variables used in source files*/
   GT_32    allocCounter;
   GT_32    reallocCounter;
   GT_32    freeCounter;
   GT_32    tempAllocCounter;
   GT_32    maxAllocCounter;
#ifdef CONFIG_V2_DMA
   GT_U32   osVirt2Phy_trace_enable;
#endif

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_OS_LINUX_MEM_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GT_TIME_MEASURE_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / gtTimeMeasure.c file
*/
typedef struct
{
    /*add here global non shared variables used in source files*/
    GT_TIME_EXACT_NODE *firstPrintNodePtr; /* first node in print chain */
    GT_TIME_EXACT_NODE *prevPrintNodePtr; /* previous node in print chain */
    GT_TIME_EXACT_NODE *prevProcessedNodePtr; /* previous processed node */

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GT_TIME_MEASURE_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir directory / gtMemoryUsageTrace.c file
*/
typedef struct
{
    /*add here global non shared variables used in source files*/
    GT_BOOL gtMemoryUsageTraceEnable; /* needed for fast skipping of tracing */
    char anchorKeyMemoArr[MAX_ACHOR_KEYS_MEMO_CNS]; /* anchor keys - ancor enabled if one of keys is it's name substring substing */
    GT_U32 anchorKeyMemoArrUsed;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module /mainOsDir
*/
typedef struct
{
    /*add here source files*/
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_OS_LINUX_MEM_SRC_GLOBAL_DB osLinuxMemSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_OS_LINUX_IO_SRC_GLOBAL_DB  osLinuxIoSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GT_TIME_MEASURE_SRC_GLOBAL_DB  gtTimeMeasureSrc;
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_DB  gtMemoryUsageTraceSrc;

} PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GLOBAL_DB;

/**
* @struct PRV_CPSS_ENABLER_MOD_SHARED_GLOBAL_DB
*
* @brief  Structure contain global variables that are not ment to be shared
* between processes (when CPSS is linked as shared object)
* in cpssEnabler module
*/
typedef struct
{
    /*add here directories*/
    PRV_CPSS_ENABLER_MOD_MAIN_OS_DIR_NON_SHARED_GLOBAL_DB mainOsDir;

} PRV_CPSS_ENABLER_MOD_NON_SHARED_GLOBAL_DB;

#endif /* __gtOsEnablerModGlobalNonSharedDb */


