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
* @file prvCpssDxChTxqTailDropUtils.h
*
* @brief CPSS SIP6 TXQ  tail drop functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqTailDropUtils
#define __prvCpssDxChTxqTailDropUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
* @internal prvCpssSip6TxqUtilsPbSizeGet function
* @endinternal
*
* @brief  Get PB size.
*
* @note   APPLICABLE DEVICES:          Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -                  physical device number
* @param[out] maxPbSizePtr                   -     size of PB for device
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssSip6TxqUtilsPbSizeGet
(
    IN GT_U8                                devNum,
    OUT GT_U32                              *maxPbSizePtr
);

/**
* @internal prvCpssFalconTxqUtilsInitTailDrop function
* @endinternal
*
* @brief  Initialize PREQ tail drop parameters to default.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   -         `                          physical device number
* @param[in] configureMulticast                   -         `        if equal GT_TRUE then default multicast threshold configured ,
*                                                                                               else no multicast default limits configured
*
* @retval GT_OK                    -           on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqUtilsInitTailDrop
(
    IN GT_U8                                devNum,
    IN GT_BOOL                              configureMulticast
);


/**
 * @internal prvCpssSip6TxQUtilsPoolValidateErrataConstraint function
 * @endinternal
 *
 * @brief  Validates that pool limit is within range and autoconfigure other pool if required
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X;Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[in] poolNum                                          index of the pool
  * @param[in] limit                                               pool limit
  * @param[in] autoComputeOtherPool               - The second shared pool size is automaticly configured to be   2PB size -[poolNum] size
*                                      NOT APPLICABLE DEVICES:
*                                      Lion2, Bobcat2, Caelum, Aldrin, AC3X; Bobcat3; Aldrin2;Ironman
*                                      APPLICABLE DEVICES:Falcon; AC5P; AC5X; Harrier;
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxQUtilsPoolValidateErrataConstraint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      poolNum,
    IN  GT_U32                      limit,
    IN  GT_BOOL                     autoComputeOtherPool
);

GT_U32 prvCpssTxqUtilsPbSizeInBuffersGet
(
    IN GT_U8 devNum
);

/**
 * @internal prvDxChSip7PortTxGlobalPbLimitSet function
 * @endinternal
 *
 * @brief  Set Tail drop limit for global buffers usage. When the fill level exceeds the limit, packets will be dropped.
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[in] limit                 - total descriptor  value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPbLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    IN  GT_U32  limit
);

/**
 * @internal prvDxChSip7PortTxGlobalPbLimitGet function
 * @endinternal
 *
 * @brief  Get Tail drop limit for global buffers usage. When the fill level exceeds the limit, packets will be dropped.
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[out] limitPtr                 - (pointer to) total descriptor value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPbLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    OUT  GT_U32  *limitPtr
);

/**
 * @internal prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitSet function
 * @endinternal
 *
 * @brief  Set minimal number of availeble PDX buffers .In case number of free buffers is less then packet is dropped.
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[in] limit                 - low limit of free  buffers
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    IN  GT_U32  limit
);
/**
 * @internal prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitGet
 * @endinternal
 *
 * @brief  Get minimal number of availeble PDX buffers .In case number of free buffers is less then packet is dropped.
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[out] limitPtr             - (pointer to) low limit of free  buffers

 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    OUT  GT_U32  *limitPtr
);

GT_STATUS  prvCpssSip7TxqUtilsTailDropBindQueueToProfileSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              queueOffset,
    IN  GT_U32                              profileSet
);

GT_STATUS  prvCpssSip7TxqUtilsTailDropBindQueueToProfileGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              queueOffset,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
);

GT_STATUS  prvCpssSip6TxqUtilsTailDropCreateStaticProfile
(
    IN GT_U8 devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT     profileSet,
    IN GT_U32                                guaranteedLimit
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

