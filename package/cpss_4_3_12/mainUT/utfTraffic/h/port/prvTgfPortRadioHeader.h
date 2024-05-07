/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvTgfPortRadioHeader.h
*
* @brief Port Radio Header addition test
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfRadioHeaderh
#define __prvTgfRadioHeaderh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* internal tgfPortRadioHeaderConfigSet function
* @endinternal
*
* @brief   Radio Header use case configurations
*/
GT_VOID tgfPortRadioHeaderConfigSet
(
    GT_VOID
);

/**
* @internal tgfPortRadioHeaderTest function
* @endinternal
*
* @brief   Radio Header use case test.
*/
GT_VOID tgfPortRadioHeaderTest
(
    GT_VOID
);

/**
* @internal tgfPortRadioHeaderConfigRestore function
* @endinternal
*
* @brief   Radio Header use case configurations restore.
* @note    1. Restore Forwarding
*          2. Restore PHA Configuration
*          3. Restore Base Configuration
*
*/
GT_VOID tgfPortRadioHeaderConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfRadioHeaderh */




