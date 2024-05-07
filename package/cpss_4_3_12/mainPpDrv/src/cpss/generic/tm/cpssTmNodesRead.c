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
* @file cpssTmNodesRead.c
*
* @brief TM nodes configuration reading APIs
*
* @version   2
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <tm_nodes_read.h>
#include <core/set_hw_registers.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/***************************************************************************
 * Read Configuration
 ***************************************************************************/

#define PRV_CPSS_TM_INVALID_NODE_INDEX_CNS 0xFFFFFFFF

/**
* @internal internal_cpssTmQueueConfigurationRead function
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
static GT_STATUS internal_cpssTmQueueConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_QUEUE_PARAMS_STC        *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct tm_queue_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    ret = tm_read_queue_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    return rc;
}

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
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_QUEUE_PARAMS_STC        *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmQueueConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmQueueConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssTmAnodeConfigurationRead function
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
static GT_STATUS internal_cpssTmAnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_A_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_a_node_params prms;

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    ret = tm_read_a_node_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;
    for(i=0; i<8; i++)
        paramsPtr->schdModeArr[i] = prms.dwrr_priority[i];
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;
    return rc;
}

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
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_A_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmAnodeConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmAnodeConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssTmBnodeConfigurationRead function
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
static GT_STATUS internal_cpssTmBnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_B_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_b_node_params prms;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    ret = tm_read_b_node_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;
    for(i=0; i<8; i++)
        paramsPtr->schdModeArr[i] = prms.dwrr_priority[i];
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;
    return rc;
}

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
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_B_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmBnodeConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmBnodeConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssTmCnodeConfigurationRead function
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
static GT_STATUS internal_cpssTmCnodeConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_C_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_c_node_params prms;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    ret = tm_read_c_node_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->dropCosMap = prms.wred_cos;
    for(i=0; i<8; i++)
    {
        paramsPtr->dropProfileIndArr[i] = prms.wred_profile_ref[i];
        paramsPtr->schdModeArr[i]= prms.dwrr_priority[i];
    }
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;
    paramsPtr->shapingProfilePtr = prms.shaping_profile_ref;
    paramsPtr->quantum = prms.quantum;

    return rc;
}

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
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_C_NODE_PARAMS_STC       *paramsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmCnodeConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr));

    rc = internal_cpssTmCnodeConfigurationRead(devNum, index, paramsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssTmPortConfigurationRead function
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
static GT_STATUS internal_cpssTmPortConfigurationRead
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          index,
    OUT CPSS_TM_PORT_PARAMS_STC         *paramsPtr,
    OUT CPSS_TM_PORT_DROP_PER_COS_STC   *cosParamsPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    int         i;
    struct tm_port_params prms;
    struct tm_port_drop_per_cos cos;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(paramsPtr);

    ret = tm_read_port_configuration(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, index, &prms, &cos);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if(rc)
        return rc;

    paramsPtr->cirBw = prms.cir_bw;
    paramsPtr->eirBw = prms.eir_bw;
    paramsPtr->cbs = prms.cbs;
    paramsPtr->ebs = prms.ebs;
    for(i=0; i<8; i++) {
        paramsPtr->quantumArr[i] = prms.quantum[i];
        paramsPtr->schdModeArr[i] = prms.dwrr_priority[i];
    }
    paramsPtr->dropProfileInd = prms.wred_profile_ref;
    paramsPtr->eligiblePrioFuncId = prms.elig_prio_func_ptr;
    paramsPtr->numOfChildren = prms.num_of_children;

    cosParamsPtr->dropCosMap = cos.wred_cos;
    for(i=0; i<8; i++)
        cosParamsPtr->dropProfileIndArr[i] = cos.wred_profile_ref[i];

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmPortConfigurationRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, paramsPtr, cosParamsPtr));

    rc = internal_cpssTmPortConfigurationRead(devNum, index, paramsPtr, cosParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, paramsPtr, cosParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssTmNodeMappingInfoRead function
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
GT_STATUS internal_cpssTmNodeMappingInfoRead
(
    IN  GT_U8                devNum,
    IN  CPSS_TM_LEVEL_ENT    level,
    IN  GT_U32               currNodeIndex,
    OUT GT_U32               *parentIndexPtr,
    OUT GT_U32               *firstChildIndexPtr,
    OUT GT_U32               *lastChildIndexPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         ret = 0;
    struct queue_hw_data_t queue_hw_data;
    struct a_node_hw_data_t a_node_hw_data;
    struct b_node_hw_data_t b_node_hw_data;
    struct c_node_hw_data_t c_node_hw_data;
    struct port_hw_data_t port_hw_data;

    CPSS_NULL_PTR_CHECK_MAC(parentIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(firstChildIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(lastChildIndexPtr);

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    switch (level)
    {
    case CPSS_TM_LEVEL_Q_E:
        ret = get_hw_queue_params(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, currNodeIndex, &queue_hw_data);
        if (ret)
        {
            break;
        }
        *parentIndexPtr     = (GT_U32)queue_hw_data.parent_a_node;
        *firstChildIndexPtr = (GT_U32)PRV_CPSS_TM_INVALID_NODE_INDEX_CNS;
        *lastChildIndexPtr  = (GT_U32)PRV_CPSS_TM_INVALID_NODE_INDEX_CNS;
        break;
    case CPSS_TM_LEVEL_A_E:
        ret = get_hw_a_node_params(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, currNodeIndex, &a_node_hw_data);
        if (ret)
        {
            break;
        }
        *parentIndexPtr     = (GT_U32)a_node_hw_data.parent_b_node;
        *firstChildIndexPtr = (GT_U32)a_node_hw_data.first_child_queue;
        *lastChildIndexPtr  = (GT_U32)a_node_hw_data.last_child_queue;
        break;
    case CPSS_TM_LEVEL_B_E:
        ret =    get_hw_b_node_params(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, currNodeIndex, &b_node_hw_data);
        if (ret)
        {
            break;
        }
        *parentIndexPtr     = (GT_U32)b_node_hw_data.parent_c_node;
        *firstChildIndexPtr = (GT_U32)b_node_hw_data.first_child_a_node;
        *lastChildIndexPtr  = (GT_U32)b_node_hw_data.last_child_a_node;
        break;
    case CPSS_TM_LEVEL_C_E:
        ret = get_hw_c_node_params(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, currNodeIndex, &c_node_hw_data);
        if (ret)
        {
            break;
        }
        *parentIndexPtr     = (GT_U32)c_node_hw_data.parent_port;
        *firstChildIndexPtr = (GT_U32)c_node_hw_data.first_child_b_node;
        *lastChildIndexPtr  = (GT_U32)c_node_hw_data.last_child_b_node;
        break;
    case CPSS_TM_LEVEL_P_E:
        ret = get_hw_port_params(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, currNodeIndex, &port_hw_data);
        if (ret)
        {
            break;
        }
        *parentIndexPtr     = (GT_U32)PRV_CPSS_TM_INVALID_NODE_INDEX_CNS;
        *firstChildIndexPtr = (GT_U32)port_hw_data.first_child_c_node;
        *lastChildIndexPtr  = (GT_U32)port_hw_data.last_child_c_node;
        break;
    default:
        ret = -EFAULT;
        *parentIndexPtr     = (GT_U32)PRV_CPSS_TM_INVALID_NODE_INDEX_CNS;
        *firstChildIndexPtr = (GT_U32)PRV_CPSS_TM_INVALID_NODE_INDEX_CNS;
        *lastChildIndexPtr  = (GT_U32)PRV_CPSS_TM_INVALID_NODE_INDEX_CNS;
        break;
    }

    rc = XEL_TO_CPSS_ERR_CODE(ret);
    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmNodeMappingInfoRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, level, currNodeIndex, parentIndexPtr, firstChildIndexPtr, lastChildIndexPtr));

    rc = internal_cpssTmNodeMappingInfoRead(devNum, level, currNodeIndex, parentIndexPtr, firstChildIndexPtr, lastChildIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, level, currNodeIndex, parentIndexPtr, firstChildIndexPtr, lastChildIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}

