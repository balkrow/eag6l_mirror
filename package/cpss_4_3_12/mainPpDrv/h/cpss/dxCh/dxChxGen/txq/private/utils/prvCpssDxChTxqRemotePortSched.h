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
* @file prvCpssDxChTxqRemotePortSched.h
*
* @brief CPSS SIP6 TXQ remote port scheduling utilities.
*
* @version   1
********************************************************************************


*/

#ifndef __prvCpssDxChTxqRemotePortSched
#define __prvCpssDxChTxqRemotePortSched

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssDxChTxqRemotePortDwrrWeightSet  function
* @endinternal
*
* @brief   Assign WRR weight to physical remote port.
*     This weight will be considered in case of port level DWRR.
*     The  weight will be  used in DWRR between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum -         physical port number
* @param[in] wrrWeight           -    wrr weight[Applicable range : 1..256]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortDwrrWeightSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               wrrWeight
);
/**
* @internal prvCpssDxChTxqRemotePortDwrrWeightGet  function
* @endinternal
*
* @brief   Get assigned WRR weight of physical remote port.
*     This weight will be considered in case of port level DWRR.
*     The  weight will be  used in DWRR between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum -         physical port number
* @param[out] wrrWeightPtr           -    (pointer to)wrr weigh[Applicable range : 1..256]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortDwrrWeightGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *wrrWeightPtr
);
/**
* @internal prvCpssDxChTxqRemotePortPrioritySet function
* @endinternal
*
* @brief   Assign priority group to physical port. This group will be considered in case of port level scheduling.
* The priority group will be used in scheduling  between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum  -         physical port number
* @param[in] priorityGroup -    scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortPrioritySet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               priorityGroupIndex
);
/**
* @internal prvCpssDxChTxqRemotePortPriorityGet function
* @endinternal
*
* @brief  Get  assigned priority group to physical port. This group will be considered in case of port level scheduling.
* The priority group will be used in scheduling  between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum  -         physical port number
* @param[out] priorityGroupPtr -    scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortPriorityGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *priorityGroupPtr
);
/**
* @internal prvCpssDxChTxqSubServicePrioritySet function
* @endinternal
*
* @brief   Assign priority group to sub service . This sub service priority  will be considered in case of service  level scheduling.
* The priority group will be used in scheduling  between subservices  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum            -physical device number
* @param[in] portNum           -physical port number (should be cascade port)
* @param[in] serviceId         -local service id
* @param[in] subServiceId      -local sub service id
* @param[in] priorityGroup      -scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND -sub service not found
*/

GT_STATUS prvCpssDxChTxqSubServicePrioritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               priorityGroup
);
/**
* @internal prvCpssDxChTxqSubServicePriorityGet function
* @endinternal
*
* @brief   Get asssigned  priority group to sub service . This sub service priority  will be considered in case of service  level scheduling.
* The priority group will be used in scheduling  between subservices  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[out] priorityGroupPtr-(pointer to)scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -sub service not found
*/

GT_STATUS prvCpssDxChTxqSubServicePriorityGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    OUT GT_U32               *priorityGroupPtr
);

/**
* @internal prvCpssDxChTxqServicePrioritySet function
* @endinternal
*
* @brief   Assign priority group to  service . This service priority  will be considered in case of cascade port  level scheduling.
* The priority group will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum            -physical device number
* @param[in] portNum           -physical port number (should be cascade port)
* @param[in] serviceId         -local service id
* @param[in] priorityGroup      -scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND -sub service not found
*/
GT_STATUS prvCpssDxChTxqServicePrioritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               priorityGroup
);

/**
* @internal prvCpssDxChTxqServicePriorityGet function
* @endinternal
*
* @brief   Get asssigned  priority group to service . This service priority  will be considered in case of cascade port  level scheduling.
* The priority group will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[out] priorityGroupPtr-(pointer to)scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -sub service not found
*/

GT_STATUS prvCpssDxChTxqServicePriorityGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               *priorityGroupPtr
);

/**
* @internal prvCpssDxChTxqServiceDwrrWeightSet function
* @endinternal
*
* @brief   Set weight  to service . This weight  will be considered in case of service  level DWRR.
* The weight will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] wrrWeight        -wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -service not found
*/
GT_STATUS prvCpssDxChTxqServiceDwrrWeightSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               wrrWeight
);
/**
* @internal prvCpssDxChTxqServiceDwrrWeightGet function
* @endinternal
*
* @brief   Get weight  assigned to service . This weight  will be considered in case of service  level DWRR.
* The weight will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[out] wrrWeight       -(pointer to)wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - service not found
*/
GT_STATUS prvCpssDxChTxqServiceDwrrWeightGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               *wrrWeightPtr
);
/**
* @internal prvCpssDxChTxqSubServiceDwrrWeightSet function
* @endinternal
*
* @brief   Set weight  to sub service . This weight  will be considered in case of suub service  level DWRR.
* The weight will be used in scheduling  between sub services  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[in] wrrWeight        -wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -sub service not found
*/
GT_STATUS prvCpssDxChTxqSubServiceDwrrWeightSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               wrrWeight
);
/**
* @internal prvCpssDxChTxqSubServiceDwrrWeightGet function
* @endinternal
*
* @brief   Get weight  assigned to sub service . This weight  will be considered in case of service  level DWRR.
* The weight will be used in scheduling  between sub services  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[out] wrrWeight       -(pointer to)wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - sub service not found
*/
GT_STATUS prvCpssDxChTxqSubServiceDwrrWeightGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               *wrrWeightPtr
);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqRemotePortSched */

