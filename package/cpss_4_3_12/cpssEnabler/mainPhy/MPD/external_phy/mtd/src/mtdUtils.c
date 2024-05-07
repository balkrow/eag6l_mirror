/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************************
* mtdUtils.c
*
* DESCRIPTION:
*       Collection of Utility functions
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/

#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdUtils.h"

#ifdef MTD_DEBUG 
#include "stdarg.h"
#include "stdio.h"

MTD_DBG_LEVEL mtd_debug_level = MTD_DBG_ALL_LVL; /* Set default debug level */

#ifdef MTD_DEBUG_FUNC
MTD_DBG_API mtdDebugLogFunc = NULL;

#define MTD_DBG_FUNC_PRINT(debugLevel, str)   \
    if (debugLevel > MTD_DBG_OFF_LVL)         \
    {                                         \
        if (mtdDebugLogFunc)                  \
            mtdDebugLogFunc(debugLevel, str); \
}

/*******************************************************************************
* mtdSetDbgLogFunc assigns the debug logging function pointer to the global
    mtdDebugLogFunc function
*******************************************************************************/
void mtdSetDbgLogFunc(MTD_DBG_API mtdDebugFuncPtr)
{
    mtdDebugLogFunc = mtdDebugFuncPtr;
}
#endif

/*******************************************************************************
void mtdSetDbgLevel
(
    MTD_DBG_LEVEL debugLevel
);

 Inputs:
       debugLevel - pass-in debug level
           MTD_DBG_OFF_LVL
           MTD_DBG_ERR_LVL
           MTD_DBG_INF_LVL
           MTD_DBG_ALL_LVL

 Outputs:
       None

 Returns:
       None

 Description:
       This function assigns the input debug level to the global mtd_debug_level.

 Side effects:
        None

 Notes/Warnings:
        None
*******************************************************************************/
void mtdSetDbgLevel(MTD_DBG_LEVEL debugLevel)
{
    mtd_debug_level = debugLevel;
}

/*******************************************************************************
* mtdDbgPrint
*
* DESCRIPTION:
*       .
*
* INPUTS:
*       debug_level - allows control of how much is printed
*               MTD_DBG_OFF_LVL  - prints nothing
*               MTD_DBG_ERR_LVL - for error messages
*               MTD_DBG_ALL_LVL - for informational messages
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       Set global mtd_debug_level to MTD_DBG_OFF_LVL to turn off all printing, or
*       MTD_DBG_ERR_LVL to print only errors, or MTD_DBG_ALL_LVL for both errors
*       and informational messages. This can be done during run time or
*       the global default can be changed at compile time to change the
*       run-time behavior.
*
*       Remove the define MTD_DEBUG_PRINT to remove all printing code (may need some 
*       modifications on some compilers).
*
*******************************************************************************/
void mtdDbgPrint(
    FILE *stream, 
    MTD_DBG_LEVEL debug_level, 
    char* format, ...
)
{
    va_list argP;
    char dbgStr[1000] = "";

#ifdef MTD_PY_DEBUG
    if (debug_level)
    {
        if (debug_level <= mtd_debug_level)
        {
            va_start(argP, format);

            vsprintf(dbgStr, format, argP);

            tstApiDbgWrite(debug_level, dbgStr);

            va_end(argP);
        }
    }
#else
    if (debug_level)
    {
        if (debug_level <= mtd_debug_level)
        {
            va_start(argP, format);

            vsprintf(dbgStr, format, argP);

            va_end(argP);

#ifdef MTD_DEBUG_FUNC
        MTD_DBG_FUNC_PRINT(debug_level, dbgStr);
            if (!mtdDebugLogFunc)
            {
                fprintf(stream, dbgStr);
            }
#else
            fprintf(stream, dbgStr);
#endif
        }
    }
#endif /* MTD_PY_DEBUG */
}
#else 

/* All printing code removed*/

#endif /* DEBUG_MTD */


/*******************************************************************************
* mtdMemSet
*
* DESCRIPTION:
*       Set a block of memory
*
* INPUTS:
*       start  - start address of memory block for setting
*       symbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
MTD_PVOID mtdMemSet
(
    IN MTD_PVOID start,
    IN int    symbol,
    IN MTD_U32 size
)
{
    MTD_U32 i;
    char* buf;
    
    buf = (char*)start;
        
    for(i=0; i<size; i++)
    {
        *buf++ = (char)symbol;
    }

    return start;
}

/*******************************************************************************
* mtdMemCpy
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
MTD_PVOID mtdMemCpy
(
    IN MTD_PVOID destination,
    IN const MTD_PVOID source,
    IN MTD_U32 size
)
{
    MTD_U32 i;
    char* buf;
    char* src;
    
    buf = (char*)destination;
    src = (char*)source;
        
    for(i=0; i<size; i++)
    {
        *buf++ = *src++;
    }

    return destination;
}

/*******************************************************************************
* mtdMemCmp
*
* DESCRIPTION:
*       Compares given memories.
*
* INPUTS:
*       src1 - source 1
*       src2 - source 2
*       size - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       0, if equal.
*        negative number, if src1 < src2.
*        positive number, if src1 > src2.
*
* COMMENTS:
*       None
*
*******************************************************************************/
int mtdMemCmp
(
    IN char src1[],
    IN char src2[],
    IN MTD_U32 size
)
{
    MTD_U32 i;
    int value;

    for(i=0; i<size; i++)
    {
        if((value = (int)(src1[i] - src2[i])) != 0)
            return value; 
    }

    return 0;
}

/*******************************************************************************
* mtdStrlen
*
* DESCRIPTION:
*       Determine the length of a string
* INPUTS:
*       source  - string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       size    - number of characters in string, not including EOS.
*
* COMMENTS:
*       None
*
*******************************************************************************/
MTD_U32 mtdStrlen
(
    IN const MTD_PVOID source
)
{
    MTD_U32 i = 0;
    char* src;
    
    src = (char*)source;
        
    while(*src++) i++;

    return i;
}

/*******************************************************************************
* mtdIsZero
*
* DESCRIPTION:
*       Test the given block of memory for zeros.
*
* INPUTS:
*       ptr - source
*       bytes - size of memory to compare
*
* OUTPUTS:
*       None
*
* RETURNS:
*       MTD_TRUE if all zeros, otherwise returns MTD_FALSE
*
* COMMENTS:
*       None
*
*******************************************************************************/
MTD_BOOL mtdIsZero
(
    IN MTD_PVOID ptr, 
    IN MTD_U16 bytes 
)
{
    MTD_U8 *bptr = (MTD_U8*)ptr;

    while(bytes--)
    {
        if(*bptr++)
        {
            return MTD_FALSE;
        }
    }

    return MTD_TRUE;
}
