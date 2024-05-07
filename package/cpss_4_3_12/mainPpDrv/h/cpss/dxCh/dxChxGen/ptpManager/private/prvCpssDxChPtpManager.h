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
* @file prvCpssDxChPtpManager.h
*
* @brief CPSS DXCH private PTP Manager debug API implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPtpManagerh
#define __prvCpssDxChPtpManagerh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManagerCommonTypes.h>


/**
* @struct CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC
*
* @brief Structure for PTP delay values.
*/
typedef struct{

    /** @brief ptp delay values */
    CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC ptpDelayValues;

    /** @brief Egress pipe delay of channel n time-stamping frames,
     *         represented as unsigned 30-bit nanosecond value.
     *        (APPLICABLE RANGES: 0..10^9-1.)
     */
    GT_32  egressPipeDelayCtsu;

} PRV_CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC;

/**
* @struct PRV_CPSS_DXCH_PTP_MANAGER_PTP_TIMESTAMPING_OPERATION_MODE_STC
*
* @brief Structure for PTP timestamping opreation mode.
*        (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*/
typedef struct{

    /** @brief ptp enhanced timestamping mode. */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT tsMode;

    /** @brief ptp operation modes. */
    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT  operationMode;

} PRV_CPSS_DXCH_PTP_MANAGER_PTP_TIMESTAMPING_OPERATION_MODE_STC;

/********* lib API DB *********/
/**
* @internal prvCpssDxChPtpManagerConvertFloatToBinary function
* @endinternal
*
* @brief   convert fractional nanosecond value from Float to Binary.
*
* @param[in]  fUpdateValue             - fractional nanosecond value in float format
*
* @param[out] bUpdateValuePtr          - (pointer to) fractional nanosecond value in binary format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
*/
GT_STATUS prvCpssDxChPtpManagerConvertFloatToBinary
(
    IN GT_FLOAT64                             fUpdateValue,
    OUT GT_U32                                *bUpdateValuePtr
);

/**
* @internal prvCpssDxChPtpManagerConvertBinaryToFloat function
* @endinternal
*
* @brief   convert fractional nanosecond value from Binary to Float.
*
* @param[in]  bUpdateValue             - fractional nanosecond value in binary format
*
* @param[out] fUpdateValuePtr          - (pointer to) fractional nanosecond value in float format
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChPtpManagerConvertBinaryToFloat
(
    IN  GT_U32                                 bUpdateValue,
    OUT GT_FLOAT64                             *fUpdateValuePtr
);

/**
* @internal prvCpssDxChPtpManagerDelayOffsetInDbGet function
* @endinternal
*
* @brief   Get PTP delay offset in DB for specific physical port.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @param[out] foundOffsetPtr       - (pointer to) wether offset was found according to port parameters.
* @param[out] offsetPtr            - (pointer to) offset of delay params in DB if foundOffsetPtr == GT_TRUE
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - on out-of-range parameter
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*
*/
GT_STATUS prvCpssDxChPtpManagerDelayOffsetInDbGet
(
    IN  GT_SW_DEV_NUM                        devNum,
    IN  GT_PHYSICAL_PORT_NUM                 portNum,
    OUT GT_BOOL                              *foundOffsetPtr,
    OUT GT_U32                               *offsetPtr
);

/**
* @internal prvCpssDxChPtpManagerPortTimestampingModeDefaultConfig function
* @endinternal
*
* @brief   Configure default Timestamping Mode per Egress port.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2; xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChPtpManagerPortTimestampingModeDefaultConfig
(
    IN  GT_SW_DEV_NUM                        devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPtpManagerh */
