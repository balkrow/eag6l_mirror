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
* @file prvCpssDxChMacSecDriver.h
*
* @brief Holds internal DDK API, which need to be called from CPSS.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChMacSecDriverh
#define __prvCpssDxChMacSecDriverh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChMacSecStatisticsSaClear function
* @endinternal
*
* @brief   Reset SA statistics.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number.
* @param[in] unitBmp               - bitmap of Data Paths (APPLICABLE VALUES: 1; 2.)
*                                    - 1: select DP0, 2: select DP1.
*                                    - if a bit of non valid data path is set then API returns GT_BAD_PARAM.
*                                    - for non multi data paths device this parameter is IGNORED.
* @param[in] direction             - select Egress or Ingress MACSec transformer
* @param[in] saHandle              - SA handle of the SA for which the statistics need to be cleared.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_FAIL                  - operation failed
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_INITIALIZED       - device was not initialized
*/
GT_STATUS prvCpssDxChMacSecStatisticsSaClear
(
    IN   GT_U8                              devNum,
    IN   GT_MACSEC_UNIT_BMP                 unitBmp,
    IN   CPSS_DXCH_MACSEC_DIRECTION_ENT     direction,
    IN   CPSS_DXCH_MACSEC_SECY_SA_HANDLE    saHandle
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChMacSecDriverh */

