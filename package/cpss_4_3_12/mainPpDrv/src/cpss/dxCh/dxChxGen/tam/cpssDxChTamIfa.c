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
* @file cpssDxChTamIfa.c
*
* @brief CPSS declarations relate to IFA (Inband Flow Analyzer) in the ingress,
* transit and egress processing in the system
*
* @version   1
********************************************************************************
*/

#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTamLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfoEnhanced.h>

#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTam.h>
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTam.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTamIfa.h>
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTamIfa.h>

/**
* @internal internal_cpssDxChIfaGlobalConfigSet function
* @endinternal
*
* @brief   Set global IFA parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Aldrin2; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in] tamParamsPtr          - pointer to IFA parameters structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaParamsPtr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChIfaGlobalConfigSet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_IFA_PARAMS_STC *ifaParamsPtr
)
{
    GT_STATUS                                         rc;
    GT_U32                                            msgData[PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_SET_MSG_LENGTH];
    GT_U32                                            msgLen = 0;
    GT_U32                                            egressPortDropCountingMode;
    GT_U32                                            egressPortDropByteMode;
    PRV_CPSS_DXCH_TAM_INFO_STC                        *tamInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(ifaParamsPtr);

    tamInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->tamInfo);
    if (tamInfoPtr->tamParams.serviceCpuId < PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS ||
        tamInfoPtr->tamParams.serviceCpuId > PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (ifaParamsPtr->samplePeriod >= BIT_8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch(ifaParamsPtr->egressPortDropCountingMode)
    {
        case CPSS_DXCH_TAM_IFA_EGRESS_PORT_BYTE_BASED_DROP_COUNTING_MODE_E:
            egressPortDropCountingMode = 0;
            break;
        case CPSS_DXCH_TAM_IFA_EGRESS_PORT_PACKET_BASED_DROP_COUNTING_MODE_E:
            egressPortDropCountingMode = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(ifaParamsPtr->egressPortDropCountingMode);
    }

    switch(ifaParamsPtr->egressPortDropByteMode)
    {
        case CPSS_DXCH_TAM_IFA_EGRESS_PORT_L2_BASED_DROP_BYTE_MODE_E:
            egressPortDropByteMode = 0;
            break;
        case CPSS_DXCH_TAM_IFA_EGRESS_PORT_L3_BASED_DROP_BYTE_MODE_E:
            egressPortDropByteMode = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(ifaParamsPtr->egressPortDropByteMode);
    }

    cpssOsMemSet(&msgData, 0, sizeof(msgData));
    /* Copy the Global IFA parameters to IPC message structure */

    msgData[0] = PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_SET_E;
    msgLen++;
    msgData[1] = ifaParamsPtr->samplePeriod;
    msgLen++;
    msgData[2] = egressPortDropCountingMode;
    msgLen++;
    msgData[3] = egressPortDropByteMode;
    msgLen++;
    msgData[4] = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
    msgLen++;

    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/* @internal cpssDxChIfaGlobalConfigSet function
* @endinternal
*
* @brief   Set Global Parameters for IFA.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Aldrin2; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in] ifaParamsPtr          - pointer to IFA parameters structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaParamsPtr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaGlobalConfigSet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_IFA_PARAMS_STC *ifaParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIfaGlobalConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ifaParamsPtr));

    rc = internal_cpssDxChIfaGlobalConfigSet(devNum, ifaParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ifaParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIfaGlobalConfigGet function
* @endinternal
*
* @brief  Get Global Parameters for IFA.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Aldrin2; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[out] ifaParamsPtr         - pointer to IFA parameters structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaParamsPtr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChIfaGlobalConfigGet
(
    IN   GT_U8                    devNum,
    OUT  CPSS_DXCH_IFA_PARAMS_STC *ifaParamsPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      msgData[PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_RETURN_MSG_LENGTH];
    GT_U32                      msgLen = 0;
    GT_U32                      egressPortDropCountingMode;
    GT_U32                      egressPortDropByteMode;
    PRV_CPSS_DXCH_TAM_INFO_STC  *tamInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(ifaParamsPtr);

    tamInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->tamInfo);
    if (tamInfoPtr->tamParams.serviceCpuId < PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS ||
        tamInfoPtr->tamParams.serviceCpuId > PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&msgData, 0, sizeof(msgData));
    msgData[msgLen]    = PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_GET_E;
    msgLen             = PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_RETURN_MSG_LENGTH;

    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if (GT_OK != rc)
    {
        return rc;
    }

    msgLen = 0;
    if (msgData[msgLen] != PRV_CPSS_DXCH_TAM_OP_CODE_GLOBAL_CONFIG_GET_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    msgLen++;

    ifaParamsPtr->samplePeriod                  = msgData[msgLen];
    msgLen++;

    egressPortDropCountingMode                  = msgData[msgLen];
    msgLen++;
    switch(egressPortDropCountingMode)
    {
        case 0:
            ifaParamsPtr->egressPortDropCountingMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_BYTE_BASED_DROP_COUNTING_MODE_E;
            break;
        case 1:
            ifaParamsPtr->egressPortDropCountingMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_PACKET_BASED_DROP_COUNTING_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(egressPortDropCountingMode);
    }

    egressPortDropByteMode                   = msgData[msgLen];
    msgLen++;
    switch(egressPortDropByteMode)
    {
        case 0:
            ifaParamsPtr->egressPortDropByteMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_L2_BASED_DROP_BYTE_MODE_E;
            break;
        case 1:
            ifaParamsPtr->egressPortDropByteMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_L3_BASED_DROP_BYTE_MODE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_ON_ENUM_MAC(egressPortDropByteMode);
    }

    return GT_OK;
}

/* @internal cpssDxChIfaGlobalConfigGet function
* @endinternal
*
* @brief  Get Global Parameters for IFA.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; Aldrin2; AC3X; Bobcat3; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[out] ifaParamsPtr         - pointer to IFA parameters structure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaParamsPtr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaGlobalConfigGet
(
    IN   GT_U8                    devNum,
    OUT  CPSS_DXCH_IFA_PARAMS_STC *ifaParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIfaGlobalConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ifaParamsPtr));

    rc = internal_cpssDxChIfaGlobalConfigGet(devNum, ifaParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ifaParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIfaEnableSet function
* @endinternal
*
* @brief Enable/Disable IFA
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum       - device number
* @param[in] ifaEnable    - GT_TRUE - enable
*                           GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on DB not found.
*/
static GT_STATUS internal_cpssDxChIfaEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ifaEnable
)
{
    GT_STATUS                    rc = GT_OK;
    GT_U32                       msgData[PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_MSG_LENGTH];
    GT_U32                       msgLen = 0;
    PRV_CPSS_DXCH_TAM_INFO_STC  *tamInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    tamInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->tamInfo);
    if (tamInfoPtr->tamParams.serviceCpuId < PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS ||
        tamInfoPtr->tamParams.serviceCpuId > PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    msgData[msgLen]    = PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_E;
    msgLen++;

    msgData[msgLen]    = ifaEnable;
    msgLen++;

    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChIfaEnableSet function
* @endinternal
*
* @brief Enable/Disable TAM IFA
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum       - device number
* @param[in] ifaEnable    - GT_TRUE - enable
*                           GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_NOT_FOUND             - on DB not found.
*/
GT_STATUS cpssDxChIfaEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_BOOL      ifaEnable
)
{
    GT_STATUS   rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIfaEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ifaEnable));

    rc = internal_cpssDxChIfaEnableSet(devNum, ifaEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ifaEnable));
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIfaEnableGet function
* @endinternal
*
* @brief Get IFA Enable/Disable status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum       - device number
* @param[out] ifaEnablePtr - (pointer to) IFA enable status
*                           GT_TRUE - enable
*                           GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - ifaEnablePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - on DB not found.
*/
static GT_STATUS internal_cpssDxChIfaEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *ifaEnablePtr
)
{
    GT_STATUS                   rc;
    GT_U32                      msgData[PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_RETURN_MSG_LENGTH];
    GT_U32                      msgLen = 0;
    PRV_CPSS_DXCH_TAM_INFO_STC  *tamInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(ifaEnablePtr);

    tamInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->tamInfo);
    if (tamInfoPtr->tamParams.serviceCpuId < PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS ||
        tamInfoPtr->tamParams.serviceCpuId > PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    msgData[msgLen]    = PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_GET_E;
    msgLen             = PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_RETURN_MSG_LENGTH;

    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if (GT_OK != rc)
    {
        return rc;
    }

    msgLen = 0;
    if (msgData[msgLen] != PRV_CPSS_DXCH_TAM_OP_CODE_ENABLE_GET_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    msgLen++;
    *ifaEnablePtr = msgData[msgLen];

    return GT_OK;
}

/**
* @internal cpssDxChIfaEnableGet function
* @endinternal
*
* @brief Get IFA Enable/Disable status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in]  devNum       - device number
* @param[out] ifaEnablePtr - (pointer to) IFA enable status
*                            GT_TRUE - enable
*                            GT_FALSE - disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - ifaEnablePtr is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_FOUND             - on DB not found.
*/
GT_STATUS cpssDxChIfaEnableGet
(
    IN  GT_U8        devNum,
    OUT GT_BOOL      *ifaEnablePtr
)
{
    GT_STATUS   rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIfaEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, ifaEnablePtr));

    rc = internal_cpssDxChIfaEnableGet(devNum, ifaEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, ifaEnablePtr));
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* @internal internal_cpssDxChIfaPortConfigSet function
* @endinternal
*
* @brief   Set IFA port parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in] portCfgArraySize      - count of physical ports array indexes.
* @param[in] portCfgArr            - list of per port related configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - portCfgArr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChIfaPortConfigSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     portCfgArraySize,
    IN  CPSS_DXCH_IFA_PORT_CFG_STC portCfgArr[] /*arraySize=portCfgArraySize*/
)
{
    GT_U32                                      iter;
    GT_U32                                      portMacNum;
    GT_STATUS                                   rc;
    GT_U32                                      msgData[PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_SET_MSG_LENGTH];
    GT_U32                                      msgLen = 0;
    GT_BOOL                                     isCpuPort;
    CPSS_DXCH_DETAILED_PORT_MAP_STC             portMap;
    GT_U32                                      numberOfQueues;
    PRV_CPSS_DXCH_TAM_INFO_STC                 *tamInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    CPSS_NULL_PTR_CHECK_MAC(portCfgArr);

    tamInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->tamInfo);
    if (tamInfoPtr->tamParams.serviceCpuId < PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS ||
        tamInfoPtr->tamParams.serviceCpuId > PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&msgData, 0, sizeof(msgData));
    msgData[msgLen] = PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_SET_E;
    msgLen++;

    msgData[msgLen] = portCfgArraySize;
    msgLen++;

    /* Copy Per Port IFA configuration parameters to IFA DB to be pushed to
     * SCPU */
    for(iter = 0; iter < portCfgArraySize; iter++)
    {
        /* Convert physical port to Global MAC */
        /* CPU Port/ Invalid Port/ Remote Physical Port to be excluded */
        PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum, portCfgArr[iter].portNum);
        rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portCfgArr[iter].portNum, &isCpuPort);
        if(rc != GT_OK || isCpuPort == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"physical port is a CPU port");
        }
        if(prvCpssDxChPortRemotePortCheck(devNum, portCfgArr[iter].portNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        /* Get Global MAC from physical port using port mapping DB */
        PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portCfgArr[iter].portNum,portMacNum);
        msgData[msgLen] = portMacNum;
        msgLen++;

        /* Get actual port speed from port info based on global MAC */
        if(portCfgArr[iter].speed > CPSS_PORT_SPEED_NA_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        msgData[msgLen] = portCfgArr[iter].speed;
        msgLen++;

        /* Get CNC Block Number for physical port */
        if(portCfgArr[iter].cncBlockNum >= PRV_CPSS_DXCH_MAX_IFA_CNC_BLOCK_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        msgData[msgLen] = portCfgArr[iter].cncBlockNum;
        msgLen++;

        /* Get CNC Base Entry for physical port */
        if(portCfgArr[iter].cncBaseEntryNum >= PRV_CPSS_DXCH_MAX_IFA_CNC_COUNTER_INDEX_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        msgData[msgLen] = portCfgArr[iter].cncBaseEntryNum;
        msgLen++;

        /* Get number of CNC counters required for physical port */
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portCfgArr[iter].portNum, &portMap);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        numberOfQueues = portMap.extPortMap.txqInfo.numberOfQueues;
        if(portCfgArr[iter].numOfCncCounter > numberOfQueues)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Number of CNC counters per port can't be greater than 0x%x\n", numberOfQueues);
        }
        msgData[msgLen] = portCfgArr[iter].numOfCncCounter;
        msgLen++;
    }

    msgLen = PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_SET_MSG_LENGTH;
    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/* @internal cpssDxChIfaPortConfigSet function
* @endinternal
*
* @brief   Set IFA port parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in] portCfgArraySize      - count of physical ports array indexes.
* @param[in] portCfgArr            - list of per port related configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - portCfgArr is NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaPortConfigSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     portCfgArraySize,
    IN  CPSS_DXCH_IFA_PORT_CFG_STC portCfgArr[] /*arraySize=portCfgArraySize*/
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIfaPortConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portCfgArraySize, portCfgArr));

    rc = internal_cpssDxChIfaPortConfigSet(devNum, portCfgArraySize, portCfgArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portCfgArraySize, portCfgArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChIfaGlobalConfigGet function
* @endinternal
*
* @brief  Get IFA port parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in]  portNum              - physical port number.
* @param[out] portCfgPtr           - (pointer to) port related configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChIfaPortConfigGet
(
    IN   GT_U8                      devNum,
    IN   GT_PHYSICAL_PORT_NUM       portNum,
    OUT  CPSS_DXCH_IFA_PORT_CFG_STC *portCfgPtr
)
{
    GT_U32                                      msgData[PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_RETURN_MSG_LENGTH];
    GT_U32                                      msgLen = 0;
    GT_U32                                      portMacNum;
    GT_STATUS                                   rc;
    GT_BOOL                                     isCpuPort;
    PRV_CPSS_DXCH_TAM_INFO_STC                 *tamInfoPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_IRONMAN_E | CPSS_AAS_E);
    PRV_CPSS_DXCH_PHY_PORT_NUM_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(portCfgPtr);

    tamInfoPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->tamInfo);
    if (tamInfoPtr->tamParams.serviceCpuId < PRV_CPSS_DXCH_TAM_SERVICE_CPU_MIN_CNS ||
        tamInfoPtr->tamParams.serviceCpuId > PRV_CPSS_DXCH_TAM_SERVICE_CPU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&msgData, 0, sizeof(msgData));
    msgData[msgLen]    = PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_GET_E;
    msgLen++;

    /* Convert physical port to Global MAC */
    /* CPU Port/ Invalid Port/ Remote Physical Port to be excluded */
    rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpuPort);
    if(rc != GT_OK || isCpuPort == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"physical port is a CPU port");
    }
    if(prvCpssDxChPortRemotePortCheck(devNum, portNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get Global MAC from physical port using port mapping DB */
    PRV_CPSS_DXCH_PORT_NUM_OR_CPU_PORT_CHECK_AND_MAC_NUM_GET_MAC(devNum,portNum,portMacNum);
    msgData[msgLen] = portMacNum;
    msgLen          = PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_RETURN_MSG_LENGTH;

    rc = prvCpssDxChTamMessageSend(devNum, &msgData[0], &msgLen);
    if (GT_OK != rc)
    {
        return rc;
    }

    msgLen = 0;
    if (msgData[msgLen] != PRV_CPSS_DXCH_TAM_OP_CODE_PORT_CONFIG_GET_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    msgLen++;

    portCfgPtr->portNum = portNum;
    msgLen++;

    portCfgPtr->speed = msgData[msgLen];
    msgLen++;

    portCfgPtr->cncBlockNum = msgData[msgLen];
    msgLen++;

    portCfgPtr->cncBaseEntryNum = msgData[msgLen];
    msgLen++;

    portCfgPtr->numOfCncCounter = msgData[msgLen];
    msgLen++;

    return GT_OK;
}

/* @internal cpssDxChIfaPortConfigGet function
* @endinternal
*
* @brief  Get IFA port parameters.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Ironman.
*
* @param[in] devNum                - physical device number.
* @param[in]  portNum              - physical port number.
* @param[out] portCfgPtr           - (pointer to) port related configuration parameters.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChIfaPortConfigGet
(
    IN   GT_U8                      devNum,
    IN   GT_PHYSICAL_PORT_NUM       portNum,
    OUT  CPSS_DXCH_IFA_PORT_CFG_STC *portCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIfaPortConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, portCfgPtr));

    rc = internal_cpssDxChIfaPortConfigGet(devNum, portNum, portCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, portCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

