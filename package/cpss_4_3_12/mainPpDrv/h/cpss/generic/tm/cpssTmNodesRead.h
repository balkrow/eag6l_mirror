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
* @file cpssTmNodesRead.h
*
* @brief TM nodes configuration reading APIs
*
* @version   1
********************************************************************************
*/

#ifndef __cpssTmNodesReadh
#define __cpssTmNodesReadh

#include <cpss/generic/tm/cpssTmPublicDefs.h>


/**
* @internal cpssTmQueueConfigurationRead function
* @endinternal
*
* @brief   Read queue software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Queue index.
*
* @param[out] paramsPtr                - (pointer to) Queue parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmQueueConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_QUEUE_PARAMS_STC    *paramsPtr
);


/**
* @internal cpssTmAnodeConfigurationRead function
* @endinternal
*
* @brief   Read A-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) A-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmAnodeConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_A_NODE_PARAMS_STC   *paramsPtr
);


/**
* @internal cpssTmBnodeConfigurationRead function
* @endinternal
*
* @brief   Read B-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) B-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmBnodeConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_B_NODE_PARAMS_STC   *paramsPtr
);


/**
* @internal cpssTmCnodeConfigurationRead function
* @endinternal
*
* @brief   Read C-node software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Node index.
*
* @param[out] paramsPtr                - (pointer to) C-Node parameters structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCnodeConfigurationRead
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      index,
    OUT CPSS_TM_C_NODE_PARAMS_STC   *paramsPtr
);


/**
* @internal cpssTmPortConfigurationRead function
* @endinternal
*
* @brief   Read port software configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] index                    - Port index.
*
* @param[out] paramsPtr                - (pointer to) Port parameters structure.
* @param[out] cosParamsPtr             - (pointer to) Port Drop per Cos structure.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmPortConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_PORT_PARAMS_STC         *paramsPtr,
    OUT CPSS_TM_PORT_DROP_PER_COS_STC   *cosParamsPtr
);

/**
 * @internal cpssTmNodeMappingInfoRead function
 * @endinternal
 *
 * @brief  Read the mapping information from hardware and get
 *         the indexes for the parent node and first child node
 *         and last child node corresponding to any given
 *         scheduler node
 *
 * @param[in] devNum          - device number
 * @param[in] level           - one of the Q,A,B,C,P levels
 * @param[in] currNodeIndex   - index to the current node of interest
 * @param[in] parentIndexPtr     - (pointer to)index to the parent node. It is invalid for P level
 * @param[in] firstChildIndexPtr - (pointer to)index to the first child. It is invalid for Q level
 * @param[in] lastChildIndexPtr  - (pointer to)index to the last child. It is invalid for Q level
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_OUT_OF_RANGE          - on index out of range
*/
GT_STATUS cpssTmNodeMappingInfoRead
(
    IN  GT_U8                devNum,
    IN  CPSS_TM_LEVEL_ENT    level,
    IN  GT_U32               currNodeIndex,
    OUT GT_U32               *parentIndexPtr,
    OUT GT_U32               *firstChildIndexPtr,
    OUT GT_U32               *lastChildIndexPtr
);

#endif      /* __cpssTmNodesReadh */


