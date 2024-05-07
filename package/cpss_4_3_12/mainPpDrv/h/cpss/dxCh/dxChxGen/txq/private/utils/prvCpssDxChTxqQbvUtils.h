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
* @file prvCpssDxChTxqQbvUtils.h
*
* @brief CPSS SIP6 TXQ  QBV related functions
*
* @version   1
********************************************************************************

*/

#ifndef __prvCpssDxChTxqQbvUtils
#define __prvCpssDxChTxqQbvUtils

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
* @internal prvCpssSip6_30TxqSdqLocalPortQbvEnableSet function
* @endinternal
*
* @brief   Enable/disable  QVB for  physical  port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               -physical port number
* @param[in] enable                - QVB enable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqPortQbvEnableSet
(
 IN GT_U8   devNum,
 IN GT_PHYSICAL_PORT_NUM portNum,
 IN GT_BOOL enable
);


/**
* @internal prvCpssSip6_30TxqSdqLocalPortQbvEnableGet function
* @endinternal
*
* @brief   Get enable/disable  QVB for  physical  port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               -physical port number
* @param[out] enable                - QVB enable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqPortQvbEnableGet
(
 IN GT_U8   devNum,
 IN GT_PHYSICAL_PORT_NUM portNum,
 OUT GT_BOOL *enablePtr
);



/**
* @internal prvCpssSip6_30TxqPortQueueGateSet function
* @endinternal
*
* @brief   Sets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[in] gate                       - gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqPortQueueGateSet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    IN GT_U32 gate
);


/**
* @internal prvCpssSip6_30TxqPortQueueGateGet function
* @endinternal
*
* @brief   Gets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[out] gatePtr                       - gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqPortQueueGateGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    OUT GT_U32 *gatePtr
);

/**
* @internal prvCpssSip6_30TxqQbvBindPortToTableSet function
* @endinternal
*
* @brief   Bind port to table set
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -    physical port number

* @param[in] bindEntryPtr            (pointer to ) bind parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvBindPortToTableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
);

/**
* @internal prvCpssSip6_30TxqQbvBindPortToTableGet function
* @endinternal
*
* @brief   Get  port to tableset binding info
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -    physical port number
* @param[out] tableSetPtr            (pointer to ) tablseset index
* @param[out] bindEntryPtr            (pointer to ) bind parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvBindPortToTableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               * tableSetPtr,
    OUT CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
);


/**
* @internal prvCpssSip6_30TxqQbvConfigureTableSet function
* @endinternal
*
* @brief  Configure tableset
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tableSet                    -   tableset index
* @param[in] slotsNum                    -   number of slots to fill
* @param[in] timeSlots           array of slots configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvConfigureTableSet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    IN CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlots
);

/**
* @internal prvCpssSip6_30TxqQbvConfigureTableGet function
* @endinternal
*
* @brief  Get tableset entries
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tableSet                    -   tableset index
* @param[in] slotsNum                    -   number of slots to fill
* @param[out] timeSlots           array of slots configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvConfigureTableGet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    OUT CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlots
);


/**
* @internal prvCpssSip6_30TxqQbvChangeReqCompleteGet function
* @endinternal
*
* @brief  Polls on status of configuration pending bit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT - after max number of retries checking if pending status bit is cleared
*
*/
GT_STATUS prvCpssSip6_30TxqQbvChangeReqCompleteGet
(
    IN GT_U8  devNum
);


GT_STATUS prvCpssSip6_30TxqPortQbvCapabilityGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL             *qbvCapablePtr,
    OUT GT_U32              *errCodePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqQbvUtils */

