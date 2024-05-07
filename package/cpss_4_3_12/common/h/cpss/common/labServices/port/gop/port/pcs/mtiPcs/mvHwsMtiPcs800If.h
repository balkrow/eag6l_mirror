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
* @file mvHwsMtiPcs800If.h
*
* @brief MTI800 PCS interface API
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsMtiPcs800If_H
#define __mvHwsMtiPcs800If_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

/**
* @internal mvHwsMtiPcs50_SIP7IfInit function
* @endinternal
*
* @brief   Init MTI PCS400 for SIP7 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs50_SIP7IfInit
(
    MV_HWS_PCS_FUNC_PTRS **funcPtrArray
);


/**
* @internal mvHwsMtiPcs100_SIP7IfInit function
* @endinternal
*
* @brief   Init MTI PCS400 for SIP7 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs100_SIP7IfInit
(
    MV_HWS_PCS_FUNC_PTRS **funcPtrArray
);


/**
* @internal mvHwsMtiPcs400_SIP7IfInit function
* @endinternal
*
* @brief   Init MTI PCS400 for SIP7 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs400_SIP7IfInit
(
    MV_HWS_PCS_FUNC_PTRS **funcPtrArray
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvHwsMtiPcs800If_H */


