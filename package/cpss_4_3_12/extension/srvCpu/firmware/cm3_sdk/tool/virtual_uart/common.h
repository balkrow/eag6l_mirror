/*******************************************************************************
*                Copyright 2016, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
*/
/**
********************************************************************************
* @file common.h
*
* @brief 
*
* @version   1.0.0
********************************************************************************
*/
#ifndef COMMON_H
#define COMMON_H

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define WORD_SIZE 4

typedef unsigned int GT_STATUS;


#define TRUE 1
#define FALSE 0

#define BOOLEAN unsigned long

/* Common definitions */
/* The following is a list of Marvell status    */
#define MV_ERROR            (-1)
#define MV_OK               (0x00)  /* Operation succeeded                   */
#define MV_FAIL             (0x01)  /* Operation failed                      */
#define MV_BAD_VALUE        (0x02)  /* Illegal value (general)               */
#define MV_OUT_OF_RANGE     (0x03)  /* The value is out of range             */
#define MV_BAD_PARAM        (0x04)  /* Illegal parameter in function called  */

#define IN
#define OUT

/* This macro returns absolute value */
#define MV_ABS(number)  (((int)(number) < 0) ? -(int)(number) : (int)(number))

/* typedefs */
typedef int8_t              INT_8;
typedef int16_t             INT_16;
typedef int32_t             INT_32;
typedef int64_t             INT_64;

typedef uint8_t             UINT8;
typedef uint8_t             UINT_8;
typedef uint16_t            UINT_16;
typedef uint32_t            UINT32;
typedef uint32_t            UINT_32;
typedef uint64_t            UINT_64;

/* Pointers typedef for arithmetic & logic operations */
typedef intptr_t            INT_ARCH_PTR;
typedef uintptr_t           UINT_ARCH_PTR;

/* sysv */
typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

/* Common definitions */
typedef unsigned char           MV_U8;
typedef unsigned short          MV_U16;

typedef unsigned int            MV_U32;
typedef long                    MV_32;
typedef void                    MV_VOID;
typedef long                    MV_BOOL;
typedef int                     MV_STATUS;
#endif
