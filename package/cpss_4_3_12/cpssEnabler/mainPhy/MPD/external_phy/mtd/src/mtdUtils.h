/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains utility functions prototypes.
********************************************************************/
#ifndef MTD_UTIL_H
#define MTD_UTIL_H

#ifdef MTD_DEBUG
#include "stdio.h"
#include "stdarg.h"
#endif

#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

#ifdef MTD_DEBUG_FUNC
#ifndef MTD_DEBUG
#error "MTD API Error: Must define MTD_DEBUG to use debug functions."
#endif

typedef void (*MTD_DBG_API)(MTD_U16 debugLevel, char *str);
extern MTD_DBG_API mtdDebugLogFunc;

/*******************************************************************************
void mtdSetDbgLogFunc
(
    MTD_DBG_API mtdDebugFuncPtr 
);

 Inputs:
       mtdDebugFuncPtr - pass-in user defined debug logging function

 Outputs:
       None

 Returns:
       None

 Description:
       This function assigns the debug logging function pointer to the global
       mtdDebugLogFunc function. The mtdDbgPrint will pass a formatted message 
       together with the debug level to this mtdDebugLogFunc.

       A sample of the customized logging function is something similar to below 
       where the function will have a pass-in debugLevel and a string pointer. 
       This user customized function can determine the debug level before 
       printing out the log message.

       MTD_STATUS customDebugLogger
       (
           IN MTD_U16 debugLevel, 
           IN MTD_VOID *pString
       );

       Call mtdSetDbgLogFunc by passing in the customized logging function:
       mtdSetDbgLogFunc(customDebugLogger);

 Side effects:
        None

 Notes/Warnings:
       MTD_DEBUG_FUNC must be defined in the mtdApiTypes.h to use this feature
*******************************************************************************/
void mtdSetDbgLogFunc
(
    MTD_DBG_API mtdDebugFuncPtr 
);
#endif

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
    IN int  symbol,
    IN MTD_U32 size
);

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
    IN MTD_U32  size
);

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
);

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
);

/*******************************************************************************
* mtdIsZero
*
* DESCRIPTION:
*       Compares given memories.
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
MTD_BOOL mtdIsZero(
    IN MTD_PVOID ptr, 
    IN MTD_U16 bytes 
);


#if C_LINKAGE
#if defined __cplusplus 
}
#endif 
#endif

#endif /* defined MTD_UTIL_H */


