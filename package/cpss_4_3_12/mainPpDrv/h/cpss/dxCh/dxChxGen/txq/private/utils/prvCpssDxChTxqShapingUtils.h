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
* @file prvCpssDxChTxqShapingUtils.h
*
* @brief CPSS SIP6 TXQ  shaping operation functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqShapingUtils
#define __prvCpssDxChTxqShapingUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssSip6TxqUtilsShaperPermissiveModeEnableSet function
* @endinternal
*
* @brief Set operation mode of egress shaper.
*  If enable equal GT_TRUE then  shaper rate accuracy error is ignored,
*  Otherwise  GT_SHAPER_GRANULARITY_OUT_OF_RANGE will be returned in
*  case accuracy error exceed threshold.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier;Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - physical device number
* @param[in] enable               -if equals GT_TRUE shaper permissive mode activated,GT_FALSE otherwise.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqUtilsShaperPermissiveModeEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 enable
);

/**
* @internal prvCpssSip6TxqUtilsShaperPermissiveModeEnableGet function
* @endinternal
*
* @brief Get operation mode of egress shaper.
*  If enable equal GT_TRUE then  shaper rate accuracy error is ignored,
*  Otherwise  GT_SHAPER_GRANULARITY_OUT_OF_RANGE will be returned in
*  case accuracy error exceed threshold.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier;Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum               -physical device number
* @param[out] enablePtr            -(pointer to) shaper permissive mode activation.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqUtilsShaperPermissiveModeEnableGet
(
    IN  GT_U8                        devNum,
    OUT GT_BOOL                      *enablePtr
);

/**
* @internal prvCpssFalconTxqUtilsShapingParametersSet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and write shaping parameters to specific node
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] burstSize                - burst size in units of 4K bytes
*                                       max value is 1K-1 which results in 4M-1 burst size
* @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
* @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsShapingParametersSet
(
    IN    GT_U8                               devNum,
    IN    GT_U32                              physicalPortNum,
    IN    GT_U32                              queueNumber,
    IN    GT_BOOL                             perQueue,
    IN    PRV_QUEUE_SHAPING_ACTION_ENT        rateType,
    IN    GT_U16                              burstSize,
    INOUT GT_U32                              *maxRatePtr
);
/**
* @internal prvCpssFalconTxqUtilsShapingParametersGet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and read shaping parameters from specific node(SW)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          - physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[in] burstSize                - burst size in units of 4K bytes
*                                       max value is 1K-1 which results in 4M-1 burst size
* @param[out] maxRatePtr               -(pointer to) the requested shaping Rate value in Kbps.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssFalconTxqUtilsShapingParametersGet
(
    IN    GT_U8                               devNum,
    IN    GT_U32                              physicalPortNum,
    IN    GT_U32                              queueNumber,
    IN    GT_BOOL                             perQueue,
    IN    PRV_QUEUE_SHAPING_ACTION_ENT        rateType,
    OUT    GT_U16                             *burstSizePtr,
    OUT GT_U32                                *maxRatePtr
);
/**
* @internal prvCpssSip6TxqUtilsShapingEnableSet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
* @param[in] enable                   - GT_TRUE,  Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssSip6TxqUtilsShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN    GT_BOOL    enable
);
/**
* @internal prvCpssSip6TxqUtilsShapingEnableGet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and get Enable/Disable Token Bucket rate shaping on specified port or queue of specified device.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
* @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
*
* @param[out] enablePtr                - GT_TRUE, enable Shaping
*                                      GT_FALSE, disable Shaping
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/

GT_STATUS prvCpssSip6TxqUtilsShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN  GT_U32   queueNumber,
    IN  GT_BOOL  perQueue,
    IN  GT_BOOL  *  enablePtr
);


/**
* @internal prvCpssFalconTxqUtilsMinimalBwEnableSet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
*
* @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port
* @param[in] enable                   - GT_TRUE,  Set lowest  priority once token bucket is empty
*                                      GT_FALSE, otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      physicalPortNum,
    IN  GT_U32      queueNumber,
    IN  GT_BOOL     enable,
    IN  GT_BOOL     keepPriorityGroup
);
/**
* @internal prvCpssFalconTxqUtilsMinimalBwEnableGet function
* @endinternal
*
* @brief   Convert physicalPortNum into scheduler node and get
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
*
* @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - Device number.
* @param[in] physicalPortNum          -physical port number        -
* @param[in] queueNumber              - Queue offset from base queue mapped to the current port
* @param[in] enablePtr                   - (pointer to)GT_TRUE,  Set lowest  priority once token bucket is empty
*                                                       GT_FALSE, otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableGet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       queueNumber,
    OUT GT_BOOL     *enablePtr,
    OUT  GT_BOOL    *keepPriorityGroupPtr
);


/**
* @internal prvCpssSip6TxqUtilsShapingCreditAccumulationEnableSet function
* @endinternal
*
* @brief   This function also handle preemptive channel
                Enable/Disable credit accumulation. Required for 802.1.Q 8.6.8.2 (previously 802.1Qav)
*              A frame is only allowed to transmit when the corresponding credit is not negative.
*              If credit accumulation is disabled ,then positive credit is set to zero if there is no frame of the corresponding class.
*              Note : Default behavior is enabling credit accumulation
*
* @note   APPLICABLE DEVICES:          Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] queueOffset              - Egress queue offset
* @param[in] enable                   -  GT_TRUE,  Credit is accumulated until max burst size.
*                                                          GT_FALSE, Credit is not accumulated .Positive credit is set to zero if there is no frame of the corresponding class.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqUtilsShapingCreditAccumulationEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPortNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  enable
);
/**
* @internal prvCpssSip6TxqUtilsShapingCreditAccumulationEnableGet function
* @endinternal
*
* @brief
                Get Enable/Disable credit accumulation. Required for 802.1.Q 8.6.8.2 (previously 802.1Qav)
*              A frame is only allowed to transmit when the corresponding credit is not negative.
*              If credit accumulation is disabled ,then positive credit is set to zero if there is no frame of the corresponding class.
*              Note : Default behavior is enabling credit accumulation
*
* @note   APPLICABLE DEVICES:          Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] queueOffset              - Egress queue offset
* @param[in] enable                   -  GT_TRUE,  Credit is accumulated until max burst size.
*                                                          GT_FALSE, Credit is not accumulated .Positive credit is set to zero if there is no frame of the corresponding class.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqUtilsShapingCreditAccumulationEnableGet
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPortNum,
    IN GT_U32   queueNumber,
    OUT GT_BOOL  *enablePtr
);

GT_STATUS prvCpssSip7TxqUtilsServiceShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_BOOL enable
);

GT_STATUS prvCpssSip7TxqUtilsServiceShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_BOOL *enablePtr
);

GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_U32 subServiceId,
    IN GT_BOOL enable
);

GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_U32 subServiceId,
    OUT GT_BOOL *enablePtr
);

GT_STATUS prvCpssSip7TxqUtilsServiceShapingParametersSet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U16       burstSize,
    INOUT GT_U32    *maxRatePtr
);

GT_STATUS prvCpssSip7TxqUtilsServiceShapingParametersGet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U16       *burstSizePtr,
    OUT GT_U32    *maxRatePtr
);

GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingParametersSet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U32       subServiceId,
    IN GT_U16       burstSize,
    INOUT GT_U32    *maxRatePtr
);

GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingParametersGet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U32       subServiceId,
    OUT GT_U16      *burstSizePtr,
    OUT GT_U32      *maxRatePtr
);



GT_STATUS prvCpssSip7TxqUtilsPnodeShapingParametersSet
(
    IN GT_U8        devNum,
    IN GT_U32       tileIndex,
    IN GT_U32       pNodeIndex,
    IN GT_U16       burstSize,
    INOUT GT_U32    *maxRatePtr
);

GT_STATUS prvCpssSip7TxqUtilsPnodeShapingParametersGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileIndex,
    IN GT_U32       pNodeIndex,
    OUT GT_U16      *burstSizePtr,
    OUT GT_U32      *maxRatePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqMemory */

