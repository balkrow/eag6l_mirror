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
* @file gtMemoryUsageTrace.c
*
* @brief Tool for tracing memory usage.
* Based on osMemGetHeapBytesAllocated function;
*
* @version   1
********************************************************************************
*/

/********* include ************************************************************/

#include <gtUtil/gtMemoryUsageTrace.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.gtMemoryUsageTraceSrc._var,_value)

#define PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_NON_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.gtMemoryUsageTraceSrc._var)


#define MAX_ACHOR_KEYS_CNS  32

/**
* @internal gtMemoryUsageTraceEnableSet function
* @endinternal
*
* @brief   Enables/Disales Memory Usage Tracing
*
* @param[in] enable                   - GT_TRUE - enable, GT_FALSE - disable
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceEnableSet
(
    IN      GT_BOOL enable
)
{
    PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_SET(gtMemoryUsageTraceEnable,enable);
}

/**
* @internal gtMemoryUsageTraceAnchorKeysReset function
* @endinternal
*
* @brief   Resets anchor keys;
*/
GT_VOID  gtMemoryUsageTraceAnchorKeysReset
(
    IN   GT_VOID
)
{
    PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_SET(anchorKeyMemoArrUsed ,0);
}

/**
* @internal gtMemoryUsageTraceAnchorKeyAdd function
* @endinternal
*
* @brief   Add anchor keys.
*
* @param[in] key                      - anchor  to be addeed.
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceAnchorKeyAdd
(
    IN      char* key
)
{
    GT_U32 keySize;

    keySize = cpssOsStrlen(key) + 1;

    if ((PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArrUsed) + keySize) >= MAX_ACHOR_KEYS_MEMO_CNS) return;

    cpssOsStrCpy(&(PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArr[PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArrUsed)])), key);
    PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArrUsed) += keySize;
}

/**
* @internal gtMemoryUsageTraceAnchorReset function
* @endinternal
*
* @brief   Reset Anchor.
*
* @param[in] anchorPtr                - (pointer to) anchor.
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceAnchorReset
(
    IN      GT_MEMORY_USAGE_TRACE_ANCHOR_STC *anchorPtr
)
{
    anchorPtr->allocatedBytes = osMemGetHeapBytesAllocated();
}

/**
* @internal gtMemoryUsageTraceNode function
* @endinternal
*
* @brief   Trace node.
*
* @param[in] anchorPtr                - (pointer to) anchor.
* @param[in] nodeName                 - name of checked node.
*                                       None.
*/
GT_VOID  gtMemoryUsageTraceNode
(
    IN      GT_MEMORY_USAGE_TRACE_ANCHOR_STC *anchorPtr,
    IN      const char*                      nodeName
)
{
    GT_U32  allocatedBytes;
    char*  keyPtr;
    GT_BOOL enable;

    if (GT_FALSE == PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(gtMemoryUsageTraceEnable)) return;

    /* check if anchor is enabled by key */
    enable = GT_FALSE;
    for (keyPtr = &(PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArr[0]));
          ((GT_U32)(keyPtr - &(PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArr[0]))) < PRV_NON_SHARED_MAIN_OS_DIR_GT_MEMORY_USAGE_TRACE_SRC_GLOBAL_VAR_GET(anchorKeyMemoArrUsed));
          keyPtr += (cpssOsStrlen(keyPtr) + 1))
    {
        if (cpssOsStrStr(anchorPtr->anchorName, keyPtr) != NULL)
        {
            /* key found as substring in anchor name */
            enable = GT_TRUE;
            break;
        }
    }
    if (enable == GT_FALSE) return;

    allocatedBytes = osMemGetHeapBytesAllocated();

    if (anchorPtr->allocatedBytes == 0)
    {
        /* first anchor using - reset */
        anchorPtr->allocatedBytes = allocatedBytes;
        return;
    }

    if (anchorPtr->allocatedBytes == allocatedBytes)
    {
        /* no changes */
        return;
    }

    cpssOsPrintf(
        "total allocated bytes: old %d new %d at %s\n",
        anchorPtr->allocatedBytes, allocatedBytes, nodeName);
    if (anchorPtr->allocatedBytes < allocatedBytes)
    {
        cpssOsPrintf(
            "added allocated bytes  %d \n",
            (allocatedBytes - anchorPtr->allocatedBytes));
    }
    else
    {
        cpssOsPrintf(
            "freed bytes  %d \n",
            (anchorPtr->allocatedBytes - allocatedBytes));
    }

    /* update anchor */
    anchorPtr->allocatedBytes = allocatedBytes;
}


