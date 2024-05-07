/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
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
* @file prvCpssTmDbgUtils.h
*
* @brief CPSS BobK TM  debug utilities.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssTmDbgUtilsh
#define __prvCpssTmDbgUtilsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GT_STATUS prvCpssTmDbgSwDbStore
(
    IN  GT_U8       devNum,
    OUT GT_VOID_PTR *swdbPtr
);

GT_STATUS prvCpssTmDbgSwDbCmp
(
    IN  GT_U8       devNum,
    IN  GT_VOID_PTR *swdbPtr
);

GT_STATUS prvCpssTmDbgSwDbFree
(
    IN  GT_U8       devNum,
    IN  GT_VOID_PTR *swdbPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*_prvCpssTmDbgUtilsh*/
