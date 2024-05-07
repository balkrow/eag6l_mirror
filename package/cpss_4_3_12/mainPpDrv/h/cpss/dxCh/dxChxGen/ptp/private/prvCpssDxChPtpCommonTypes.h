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
* @file prvCpssDxChPtpCommonTypes.h
*
* @brief CPSS DXCH private PTP common types implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPtpCommonTypesh
#define __prvCpssDxChPtpCommonTypesh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct PRV_CPSS_DXCH_PTP_PLL_CONFIG_STC
 *
 * @brief Structure for PLL configuration.
*/
typedef struct{

    GT_U32 K;
    GT_U32 M;
    GT_U32 N;
    GT_U32 vcoBand;
    GT_U32 lpfCtrl;

} PRV_CPSS_DXCH_PTP_PLL_CONFIG_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPtpCommonTypesh */
