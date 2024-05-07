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
* @file mvHwsMtiPcs10If.h
*
* @brief MTI25 PCS interface API
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMtiPcs10If_H
#define __mvHwsMtiPcs10If_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsMtiPcs10IfInit function
* @endinternal
*
* @brief   Init MtiPcs10 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs10IfInit
(
    MV_HWS_PCS_FUNC_PTRS **funcPtrArray
);

/**
* @internal mvHwsMtiPcs10LpcsIfInit function
* @endinternal
*
* @brief   Init MtiPcs10 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs10LpcsIfInit
(
    MV_HWS_PCS_FUNC_PTRS **funcPtrArray
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsMtiPcs10If_H */


