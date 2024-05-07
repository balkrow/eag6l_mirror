/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file cpssDxChIpfixFw.c
*
* @brief CPSS APIs implementation for IPFIX Firmware APIs.
*
* @version   1
*********************************************************************************
**/

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixFw.h>
#include <cpss/common/srvCpu/prvCpssGenericSrvCpuLoad.h>

/* Check service cpu number */
#define PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(_devNum, _cpuNum)                   \
    if(PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)  \
    {                                                                        \
        if (_cpuNum >= PRV_CPSS_DXCH_IPFIX_FALCON_FW_SCPU_MAX_CNS)           \
        {                                                                    \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);   \
        }                                                                    \
    }                                                                        \
    else                                                                     \
    {                                                                        \
        if (_cpuNum >= PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS)            \
        {                                                                    \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);   \
        }                                                                    \
    }

/* Flow Id check */
#define PRV_CPSS_DXCH_IPFIX_FW_FLOWID_CHECK_MAC(_devNum, flowId)                         \
    if(((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E) &&    \
        ((flowId & 0xFFF) >= PRV_CPSS_DXCH_IPFIX_FW_FLOWS_PER_PORT_GROUP_MAX_CNS))    || \
       ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)   &&    \
        (flowId >= PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5X_MAX_CNS))                         || \
       ((PRV_CPSS_PP_MAC(_devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)   &&    \
        (flowId >= PRV_CPSS_DXCH_IPFIX_FW_FLOWS_AC5P_MAX_CNS)))                           \
    {                                                                               \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);                                                     \
    }

/* device support check */
#define PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum)                                     \
PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E |               \
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E |   \
    CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E |     \
    CPSS_AAS_E)

#define PRV_CPSS_DXCH_DEV_SUPPORT_FALCON_CHECK_MAC(devNum)                                     \
PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E |               \
    CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E |   \
    CPSS_AC5X_E | CPSS_AC5P_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E |     \
    CPSS_AAS_E)

/**
 * @internal internal_cpssDxChIpfixFwInit function
 * @endinternal
 *
 * @brief Load/Initialize FW channel
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS   internal_cpssDxChIpfixFwInit
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum
)
{
    GT_STATUS                           rc  = GT_OK;
    GT_UINTPTR                          fwChannel;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);

    /* Reset the specified service CPU and load IPFIX Assistant FW image into its SRAM */
    rc = prvCpssIpfixFwToSrvCpuLoad(devNum, sCpuNum, &fwChannel);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, "IPFIX Firmware load failed");
    }

    return GT_OK;
}

/**
 * @internal cpssDxChIpfixFwInit function
 * @endinternal
 *
 * @brief Load/Initialize FW channel
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS   cpssDxChIpfixFwInit
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwInit(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwRestore function
 * @endinternal
 *
 * @brief Restore FW channel
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS   internal_cpssDxChIpfixFwRestore
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum
)
{
    GT_STATUS                           rc  = GT_OK;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);

    /* Reset the specified service CPU and load IPFIX Assistant FW image into its SRAM */
    rc = prvCpssGenericSrvCpuRemove(devNum, sCpuNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "IPFIX Firmware Remove failed");
    }

    return GT_OK;
}

/**
 * @internal cpssDxChIpfixFwRestore function
 * @endinternal
 *
 * @brief Remove FW channel
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS   cpssDxChIpfixFwRestore
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwRestore(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwEnableSet function
 * @endinternal
 *
 * @brief Start/stop the full running of IPFIX Assistant FW; i.e., modify its
 *        admin state between enabled/disabled
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum,
    IN  GT_BOOL      ipfixEnable
)
{
    GT_STATUS                           rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);


    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.enableSet.opcode        = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENABLE_SET_E;
    ipc_msg.enableSet.ipfixEnable   = ipfixEnable;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwEnableSet function
 * @endinternal
 *
 * @brief Start/stop the full running of IPFIX Assistant FW; i.e., modify its
 *        admin state between enabled/disabled
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum       - device number
 * @param[in] sCpuNum      - service CPU number
 * @param[in] ipfixEnable  - GT_TRUE - enable
 *                           GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEnableSet
(
    IN  GT_U8        devNum,
    IN  GT_U32       sCpuNum,
    IN  GT_BOOL      ipfixEnable
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, ipfixEnable));

    rc = internal_cpssDxChIpfixFwEnableSet(devNum, sCpuNum, ipfixEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, ipfixEnable));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwGlobalConfigSet function
 * @endinternal
 *
 * @brief  Set the global IPFIX settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum            - device number
 * @param[in] sCpuNum           - service CPU number
 * @param[in] globalCfgPtr      - (pointer to) ipfix global configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    *globalCfgPtr
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    CPSS_NULL_PTR_CHECK_MAC(globalCfgPtr);

    /* Global config params check */
    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
        (globalCfgPtr->firstPolicerStage > CPSS_DXCH_POLICER_STAGE_EGRESS_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "firstPolicerStage out of range");
    }
    if (globalCfgPtr->hwAutoLearnEnable == GT_TRUE)
    {
        if(globalCfgPtr->maxInTransitIndex > PRV_CPSS_DXCH_IPFIX_FW_MAX_INTRANSIT_INDEX_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "maxInTransitIndex out of range");
        }

        if(globalCfgPtr->maxFwIndex > PRV_CPSS_DXCH_IPFIX_FW_MAX_IPFIX_INDEX_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "maxFwIndex out of range");
        }

        if(globalCfgPtr->maxAppIndex > PRV_CPSS_DXCH_IPFIX_FW_MAX_IPFIX_INDEX_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "maxAppIndex out of range");
        }

        if (globalCfgPtr->maxFwIndex <= globalCfgPtr->maxInTransitIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "maxFwIndex must be greater then maxInTransitIndex");
        }
        if (globalCfgPtr->maxAppIndex < globalCfgPtr->maxFwIndex)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "maxAppIndex must be greater or equal to maxFwIndex");
        }
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        if (globalCfgPtr->secondPolicerStage > CPSS_DXCH_POLICER_STAGE_EGRESS_E)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "secondPolicerStage out of range");
        }
        if (globalCfgPtr->secondPolicerMaxIpfixIndex > PRV_CPSS_DXCH_IPFIX_FW_POLICER_MAX_IPFIX_INDEX_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "secondPolicerMaxIpfixIndex out of range");
        }
    }

    if (globalCfgPtr->idleAgingOffload == GT_TRUE)
    {
        if (globalCfgPtr->timeStampsExtensionOffload == GT_TRUE)
        {
            if (globalCfgPtr->idleTimeout > PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                              "idleTimeout out of range. With timestamp extension enabled, Max = %d",
                                              PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_CNS);
            }
        }
        else
        {
            if (globalCfgPtr->idleTimeout > PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_WITHOUT_EXTENSION_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                              "idleTimeout out of range. With timestamp extension disabled, Max = %d",
                                              PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_WITHOUT_EXTENSION_CNS);
            }
        }
    }

    if (globalCfgPtr->longAgingOffload == GT_TRUE)
    {
        if (globalCfgPtr->timeStampsExtensionOffload == GT_TRUE)
        {
            if (globalCfgPtr->activeTimeout > PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                              "activeTimeout out of range. With timestamp extension enabled, Max = %d",
                                              PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_CNS);
            }
        }
        else
        {
            if (globalCfgPtr->activeTimeout > PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_WITHOUT_EXTENSION_CNS)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                              "activeTimeout out of range. With timestamp extension disabled, Max = %d",
                                              PRV_CPSS_DXCH_IPFIX_FW_MAX_TIMEOUT_WITHOUT_EXTENSION_CNS);
            }
        }
    }

    if ((globalCfgPtr->scanCycleInterval < PRV_CPSS_DXCH_IPFIX_FW_SCAN_CYCLE_INTERVAL_MIN_CNS) ||
        (globalCfgPtr->scanCycleInterval > PRV_CPSS_DXCH_IPFIX_FW_SCAN_CYCLE_INTERVAL_MAX_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                      "ScanCycleInterval out of range (%d). Range [%d - %d]",
                                      globalCfgPtr->scanCycleInterval,
                                      PRV_CPSS_DXCH_IPFIX_FW_SCAN_CYCLE_INTERVAL_MIN_CNS,
                                      PRV_CPSS_DXCH_IPFIX_FW_SCAN_CYCLE_INTERVAL_MAX_CNS);
    }

    if ((globalCfgPtr->scanLoopItems < PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_MIN_CNS) ||
        (globalCfgPtr->scanLoopItems > PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_MAX_CNS(devNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                                      "ScanLoopItems out of range (%d). Range [%d - %d]",
                                      globalCfgPtr->scanLoopItems,
                                      PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_MIN_CNS,
                                      PRV_CPSS_DXCH_IPFIX_FW_SCAN_LOOP_ITEMS_MAX_CNS(devNum));
    }

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_FALCON_E) &&
        ((globalCfgPtr->firstPolicerMaxIpfixIndex > PRV_CPSS_DXCH_IPFIX_FW_POLICER_MAX_IPFIX_INDEX_CNS(devNum)) ||
         (globalCfgPtr->secondPolicerMaxIpfixIndex > PRV_CPSS_DXCH_IPFIX_FW_POLICER_MAX_IPFIX_INDEX_CNS(devNum))))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "firstPolicerMaxIpfixIndex out of range");
    }

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.globalConfigSet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_GLOBAL_CONFIG_SET_E;
    cpssOsMemCpy(&ipc_msg.globalConfigSet.globalCfg, globalCfgPtr, sizeof(*globalCfgPtr));
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwGlobalConfigSet function
 * @endinternal
 *
 * @brief  Set the global IPFIX settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum            - device number
 * @param[in] sCpuNum           - service CPU number
 * @param[in] globalCfgPtr      - (pointer to) ipfix global configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC    *globalCfgPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwGlobalConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, globalCfgPtr));

    rc = internal_cpssDxChIpfixFwGlobalConfigSet(devNum, sCpuNum, globalCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, globalCfgPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwGlobalConfigGet function
 * @endinternal
 *
 * @brief  Request that all global IPFIX settings be returned via
 *         a GlobalConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum           - device number
 * @param[in] sCpuNum          - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwGlobalConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);


    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.configGet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E;
    ipc_msg.configGet.configtype = CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_GLOBAL_E;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);
    return rc;
}

/**
 * @internal cpssDxChIpfixFwGlobalConfigGet function
 * @endinternal
 *
 * @brief  Request that all elephant IPFIX settings be returned via
 *         a GlobalConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in] devNum           - device number
 * @param[in] sCpuNum          - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwGlobalConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwGlobalConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwGlobalConfigGet(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwPortGroupConfigSet function
 * @endinternal
 *
 * @brief  Set per tile IPFIX settings
 *         It is used to bind one or two policers to IPFIX. Per Tile, a
 *           consecutive range of FlowIDs is allocated to IPFIX, from
 *           index #1 upward. The range is split between the two policers
 *         After calling this API, it is required to call the PortGroupConfigGet
 *           API followed by IcpMsgFetch until a PortGroupConfigReturn (or a
 *           ConfigError) IPC message is fetched, and check that the configuration
 *           has been properly recorded by FW.
 *         This API can be invoked only when no active IPFIX flow.
 *
 * @note  APPLICABLE_DEVICES: Falcon.
 *
 * @param[in] devNum            - device number
 * @param[in] sCpuNum           - service CPU number
 * @param[in] globalCfgPtr      - (pointer to) ipfix global configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS internal_cpssDxChIpfixFwPortGroupConfigSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC *portGroupCfgPtr
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;
    GT_U32                              i = 0;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_FALCON_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    CPSS_NULL_PTR_CHECK_MAC(portGroupCfgPtr);

    /* Global config params check */
    if (portGroupCfgPtr->firstPolicerStage > CPSS_DXCH_POLICER_STAGE_EGRESS_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "firstPolicerStage out of range");
    }
    if (portGroupCfgPtr->secondPolicerStage > CPSS_DXCH_POLICER_STAGE_EGRESS_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "firstPolicerStage out of range");
    }
    for (i=0; i<PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles; i++)
    {
        if (portGroupCfgPtr->firstPolicerMaxIpfixIndex[i] > PRV_CPSS_DXCH_IPFIX_FW_POLICER_MAX_IPFIX_INDEX_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                "firstPolicerMaxIpfixIndex (tile=%d) out of range", i);
        }
        if (portGroupCfgPtr->secondPolicerMaxIpfixIndex[i] > PRV_CPSS_DXCH_IPFIX_FW_POLICER_MAX_IPFIX_INDEX_CNS(devNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                "secondPolicerMaxIpfixIndex (tileId=%d) out of range", i);
        }
        if (portGroupCfgPtr->secondPolicerMaxIpfixIndex[i] < portGroupCfgPtr->firstPolicerMaxIpfixIndex[i])
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
                 "secondPolicerMaxIpfixIndex (tileId=%d) must be less than firstPolicerMaxIpfixIndex", i);
        }
    }

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.portGroupConfigSet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_PORT_GROUP_CONFIG_SET_E;
    cpssOsMemCpy(&ipc_msg.portGroupConfigSet.portGroupCfg, portGroupCfgPtr, sizeof(*portGroupCfgPtr));
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);
    return rc;
}

/**
 * @internal cpssDxChIpfixFwPortGroupConfigSet function
 * @endinternal
 *
 * @brief  Set per tile IPFIX settings
 *         It is used to bind one or two policers to IPFIX. Per Tile, a
 *           consecutive range of FlowIDs is allocated to IPFIX, from
 *           index #1 upward. The range is split between the two policers
 *         After calling this API, it is required to call the PortGroupConfigGet
 *           API followed by IcpMsgFetch until a PortGroupConfigReturn (or a
 *           ConfigError) IPC message is fetched, and check that the configuration
 *           has been properly recorded by FW.
 *         This API can be invoked only when no active IPFIX flow.
 *
 * @note  APPLICABLE_DEVICES: Falcon.
 *
 * @param[in] devNum            - device number
 * @param[in] sCpuNum           - service CPU number
 * @param[in] globalCfgPtr      - (pointer to) ipfix global configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwPortGroupConfigSet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_PORT_GROUP_CONFIG_STC *portGroupCfgPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwPortGroupConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, portGroupCfgPtr));

    rc = internal_cpssDxChIpfixFwPortGroupConfigSet(devNum, sCpuNum, portGroupCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, portGroupCfgPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwPortGroupConfigGet function
 * @endinternal
 *
 * @brief  Request that all per tile IPFIX settings be returned via
 *         a PortGroupConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum           - device number
 * @param[in] sCpuNum          - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS internal_cpssDxChIpfixFwPortGroupConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_FALCON_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);


    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.configGet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E;
    ipc_msg.configGet.configtype = CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_PORT_GROUP_E;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);
    return rc;
}

/**
 * @internal cpssDxChIpfixFwPortGroupConfigGet function
 * @endinternal
 *
 * @brief  Request that all per tile IPFIX settings be returned via
 *         a PortGroupConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: Falcon
 *
 * @param[in] devNum           - device number
 * @param[in] sCpuNum          - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwPortGroupConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwGlobalConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwPortGroupConfigGet(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwEntryAdd function
 * @endinternal
 *
 * @brief   Manually learn a new IPFIX flow, specifying the EM entry index
 *          at which the flow was already learned into EM Table
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum  - device number
 * @param[in]  sCpuNum - service CPU number
 * @param[in]  flowId  - flow id. When hwAutoLearnEnable is enabled,
 *                       range must be maxFirmwareIndex+1 and maxIpfixIndex (included).
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 * @param[in]  entryParamsPtr - (pointer to) IPFIX entry input parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwEntryAdd
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  GT_U32      flowId,
    IN  CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC  *entryParamsPtr
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    PRV_CPSS_DXCH_IPFIX_FW_FLOWID_CHECK_MAC(devNum, flowId);
    CPSS_NULL_PTR_CHECK_MAC(entryParamsPtr);

    if (entryParamsPtr->emIndex >= PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_MAX_CNS(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "emIndex %d, exceeds the limit %d",
                                      entryParamsPtr->emIndex, PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_MAX_CNS(devNum));
    }

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.entryAdd.opcode  = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_ADD_E;
    ipc_msg.entryAdd.flowId  = flowId;
    cpssOsMemCpy(&ipc_msg.entryAdd.entryParams, entryParamsPtr, sizeof(CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC));
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwEntryAdd function
 * @endinternal
 *
 * @brief   Manually learn a new IPFIX flow, specifying the EM entry index
 *          at which the flow was already learned into EM Table
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum  - device number
 * @param[in]  sCpuNum - service CPU number
 * @param[in]  flowId  - flow id. When hwAutoLearnEnable is enabled,
 *                       range must be maxFirmwareIndex+1 and maxIpfixIndex (included).
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 * @param[in]  entryParamsPtr - (pointer to) IPFIX entry input parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEntryAdd
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  GT_U32      flowId,
    IN  CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC  *entryParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwEntryAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, flowId, entryParamsPtr));

    rc = internal_cpssDxChIpfixFwEntryAdd(devNum, sCpuNum, flowId, entryParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, flowId, entryParamsPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChIpfixFwEntryDelete function
 * @endinternal
 *
 * @brief   Request to delete an IPFIX entry
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum   - device number
 * @param[in]  sCpuNum  - service CPU number
 * @param[in]  flowId   - flow identifier
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwEntryDelete
(
    IN  GT_U8         devNum,
    IN  GT_U32        sCpuNum,
    IN  GT_U32        flowId
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    PRV_CPSS_DXCH_IPFIX_FW_FLOWID_CHECK_MAC(devNum, flowId);


    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.entryDelete.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_E;
    ipc_msg.entryDelete.flowId = flowId;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwEntryDelete function
 * @endinternal
 *
 * @brief   Request to delete an IPFIX entry
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum   - device number
 * @param[in]  sCpuNum  - service CPU number
 * @param[in]  flowId   - flow identifier
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEntryDelete
(
    IN  GT_U8         devNum,
    IN  GT_U32        sCpuNum,
    IN  GT_U32        flowId
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwEntryDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, flowId));

    rc = internal_cpssDxChIpfixFwEntryDelete(devNum, sCpuNum, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, flowId));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwEntryDeleteAll function
 * @endinternal
 *
 * @brief   Request to delete ALL IPFIX entries in the device
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum          - device number
 * @param[in]  sCpuNum         - service CPU number
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwEntryDeleteAll
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);


    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.entryDeleteAll.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ENTRY_DELETE_ALL_E;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwEntryDeleteAll function
 * @endinternal
 *
 * @brief   Request to delete ALL IPFIX entries in the device
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum          - device number
 * @param[in]  sCpuNum         - service CPU number
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwEntryDeleteAll
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwEntryDeleteAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwEntryDeleteAll(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwDataGet function
 * @endinternal
 *
 * @brief   Request to export the IPFIX data for a single entry.
 *          After calling this API, it is required to call the cpssDxChNetIfSdmaRxPacketGet
 *          API until the IPFIX data packet returned by FW is fetched.
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum         - device number
 * @param[in]  sCpuNum        - service CPU number
 * @param[in]  portGroupsBmp  - port groups bitmap
 * @param[in]  flowId         - flow identifier
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwDataGet
(
    IN  GT_U8         devNum,
    IN  GT_U32        sCpuNum,
    IN  GT_U32        flowId
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    PRV_CPSS_DXCH_IPFIX_FW_FLOWID_CHECK_MAC(devNum, flowId);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.dataGet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_E;
    ipc_msg.dataGet.flowId = flowId;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwDataGet function
 * @endinternal
 *
 * @brief   Request to export the IPFIX data for a single entry.
 *          After calling this API, it is required to call the cpssDxChNetIfSdmaRxPacketGet
 *          API until the IPFIX data packet returned by FW is fetched.
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum         - device number
 * @param[in]  sCpuNum        - service CPU number
 * @param[in]  portGroupsBmp  - port groups bitmap
 * @param[in]  flowId         - flow identifier
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataGet
(
    IN  GT_U8         devNum,
    IN  GT_U32        sCpuNum,
    IN  GT_U32        flowId
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwDataGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, flowId));

    rc = internal_cpssDxChIpfixFwDataGet(devNum, sCpuNum, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, flowId));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwDataGetAll function
 * @endinternal
 *
 * @brief  Request to export the IPFIX data for a group of entries either once or periodically.
 *         It cancels the effect of any DataGetAllStop API invoked before.
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum           - device number
 * @param[in]  sCpuNum          - service CPU number
 * @param[in]  flowType         - flow type. Type of entries that will be
 *                                exported by FW either once or periodically.
 * @param[in]  maxNumOfFlows    - maximum number of entries exported by FW.
 *                                When set to 0, there is no export limit.
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwDataGetAll
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT    flowType,
    IN  CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT    maxNumOfFlows
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    if (flowType > CPSS_DXCH_IPFIX_FW_FLOW_TYPE_LASTLY_ACTIVE_ELEPHANTS_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid flowType");
    }

    if (maxNumOfFlows >= CPSS_DXCH_IPFIX_FW_MAX_FLOWS_LAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Invalid maxNumOfFlows");
    }

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.dataGetAll.opcode        = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E;
    ipc_msg.dataGetAll.flowType      = flowType;
    ipc_msg.dataGetAll.maxNumOfFlows = maxNumOfFlows;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwDataGetAll function
 * @endinternal
 *
 * @brief  Request to export the IPFIX data for a group of entries either once or periodically.
 *         It cancels the effect of any DataGetAllStop API invoked before.
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum           - device number
 * @param[in]  sCpuNum          - service CPU number
 * @param[in]  flowType         - flow type. Type of entries that will be
 *                                exported by FW either once or periodically.
 * @param[in]  maxNumOfFlows    - maximum number of entries exported by FW.
 *                                When set to 0, there is no export limit.
 *
 * @retval GT_OK       - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataGetAll
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT    flowType,
    IN  CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT    maxNumOfFlows
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwDataGetAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, flowType, maxNumOfFlows));

    rc = internal_cpssDxChIpfixFwDataGetAll(devNum, sCpuNum, flowType, maxNumOfFlows);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, flowType, maxNumOfFlows));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwIpcMsgFetch function
 * @endinternal
 *
 * @brief  Fetch, format and return several pending Tx IPC messages
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum              - device number
 * @param[in]  sCpuNum             - service CPU number
 * @param[in]  numOfMsgs           - number of messages to be fetched.
 * @param[out] numOfMsgsFetchedPtr - (pointer to) number of messages fetched.
 * @param[out] pendingMsgBuffSizePtr - (pointer to) Size in Bytes of the number of remaining messages
 *                                   in Tx IPC queue, which are pending to be fetched
 * @param[out] ipcMsgArrPtr        - (pointer to) fetched messages array.
 *                                   Application needs to provide enough memory
 *                                   i.e. (numOfMsgs * CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT)
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwIpcMsgFetch
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum,
    IN  GT_U32    numOfMsgs,
    OUT GT_U32    *numOfMsgsFetchedPtr,
    OUT GT_U32    *pendingMsgBuffSizePtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT   *ipcMsgArrPtr
)
{
    GT_STATUS                          rc  = GT_OK;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    CPSS_NULL_PTR_CHECK_MAC(numOfMsgsFetchedPtr);
    CPSS_NULL_PTR_CHECK_MAC(pendingMsgBuffSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(ipcMsgArrPtr);
    if (numOfMsgs > PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "numOfMsgs: %d exceeds the limit %d",
                                      numOfMsgs, PRV_CPSS_DXCH_IPFIX_FW_IPC_MSGS_FETCH_MAX_NUM_CNS);
    }

    rc = prvCpssDxChIpfixFwIpcMsgFetchAndParse(devNum, sCpuNum, numOfMsgs, numOfMsgsFetchedPtr,
                                               pendingMsgBuffSizePtr, ipcMsgArrPtr);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwIpcMsgFetch function
 * @endinternal
 *
 * @brief  Fetch, format and return several pending Tx IPC messages
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum              - device number
 * @param[in]  sCpuNum             - service CPU number
 * @param[in]  numOfMsgs           - number of messages to be fetched.
 * @param[out] numOfMsgsFetchedPtr - (pointer to) number of messages fetched.
 * @param[out] pendingMsgBuffSizePtr - (pointer to) Size in Bytes of the number of remaining messages
 *                                   in Tx IPC queue, which are pending to be fetched
 * @param[out] ipcMsgArrPtr        - (pointer to) fetched messages array.
 *                                   Application needs to provide enough memory
 *                                   i.e. (numOfMsgs * CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT)
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwIpcMsgFetch
(
    IN  GT_U8     devNum,
    IN  GT_U32    sCpuNum,
    IN  GT_U32    numOfMsgs,
    OUT GT_U32    *numOfMsgsFetchedPtr,
    OUT GT_U32    *pendingMsgBuffSizePtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT   *ipcMsgArrPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwIpcMsgFetch);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, numOfMsgs, numOfMsgsFetchedPtr, pendingMsgBuffSizePtr, ipcMsgArrPtr));

    rc = internal_cpssDxChIpfixFwIpcMsgFetch(devNum, sCpuNum, numOfMsgs, numOfMsgsFetchedPtr, pendingMsgBuffSizePtr, ipcMsgArrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, numOfMsgs, numOfMsgsFetchedPtr, pendingMsgBuffSizePtr, ipcMsgArrPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwExportConfigSet function
 * @endInternal
 *
 * @brief Set the IPFIX Data Export settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  exportCfgPtr - (pointer to) data export configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwExportConfigSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC  *exportCfgPtr
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    CPSS_NULL_PTR_CHECK_MAC(exportCfgPtr);

    if (exportCfgPtr->dataPktMtu > PRV_CPSS_DXCH_IPFIX_FW_DATA_PKTS_MTU_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "dataPktMtu out of range");
    }
    if (exportCfgPtr->mgNum > PRV_CPSS_DXCH_PP_HW_INFO_MG_MAC(devNum).sip6MgNumOfUnits)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "mgNum out of range");
    }
    if (exportCfgPtr->periodicExportInterval > PRV_CPSS_DXCH_IPFIX_FW_PERIODIC_EXPORT_INTERVAL_MAX_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "periodicExportInterval out of range");
    }
    if (exportCfgPtr->localQueueNum > PRV_CPSS_DXCH_IPFIX_FW_SDMA_MAX_QUEUE_NUM_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "localQueueNum out of range");
    }

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.exportConfigSet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_CONFIG_SET_E;
    cpssOsMemCpy(&ipc_msg.exportConfigSet.exportCfg, exportCfgPtr, sizeof(*exportCfgPtr));
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwExportConfigSet function
 * @endInternal
 *
 * @brief Set the IPFIX Data Export settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  exportCfgPtr - (pointer to) data export configuration settings.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwExportConfigSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC  *exportCfgPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwExportConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, exportCfgPtr));

    rc = internal_cpssDxChIpfixFwExportConfigSet(devNum, sCpuNum, exportCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, exportCfgPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwExportConfigGet function
 * @endInternal
 *
 * @brief Request that IPFIX Data Export settings be returned via an ExportConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwExportConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.configGet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E;
    ipc_msg.configGet.configtype = CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_EXPORT_E;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwExportConfigGet function
 * @endInternal
 *
 * @brief Request that IPFIX Data Export settings be returned via an ExportConfigReturn IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwExportConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwExportConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwExportConfigGet(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwElephantConfigSet function
 * @endInternal
 *
 * @brief Set the Elephant Detection anf Report settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  elephantCfgPtr - (pointer to) elephant detection settings structure.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwElephantConfigSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  *elephantCfgPtr
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    CPSS_NULL_PTR_CHECK_MAC(elephantCfgPtr);

    /* MSB is 16 bit value */
    if (elephantCfgPtr->pktCntThreshold[1] >= (1<<16))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "pktCntThreshold out of range");
    }
    /* MSB is 16 bit value */
    if (elephantCfgPtr->byteCntThreshold[1] >= (1<<16))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "byteCntThreshold out of range");
    }
    /* 8bit value */
    if (elephantCfgPtr->crossCntThresholdLow >= (1<<8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "crossCntThresholdLow out of range");
    }
    /* 8bit value */
    if (elephantCfgPtr->crossCntThresholdHigh >= (1<<8))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "crossCntThresholdHigh out of range");
    }
    /* 16bit value */
    if (elephantCfgPtr->startThreshold >= (1<<16))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "startThreshold out of range");
    }

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.elephantConfigSet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_ELEPHANT_CONFIG_SET_E;
    cpssOsMemCpy(&ipc_msg.elephantConfigSet.elephantCfg, elephantCfgPtr, sizeof(*elephantCfgPtr));
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwElephantConfigSet function
 * @endInternal
 *
 * @brief Set the Elephant Detection anf Report settings
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  elephantCfgPtr - (pointer to) elephant detection settings structure.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_BAD_PTR               - on NULL pointer.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwElephantConfigSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC  *elephantCfgPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwElephantConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, elephantCfgPtr));

    rc = internal_cpssDxChIpfixFwElephantConfigSet(devNum, sCpuNum, elephantCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, elephantCfgPtr));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwElephantConfigGet function
 * @endInternal
 *
 * @brief Request that Elephant Detection settings be returned via an ElephantConfigReturn
 *       IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwElephantConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.configGet.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_GET_E;
    ipc_msg.configGet.configtype = CPSS_DXCH_IPFIX_FW_CONFIG_GET_TYPE_ELEPHANT_E;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwElephantConfigGet function
 * @endInternal
 *
 * @brief Request that Elephant Detection settings be returned via an ElephantConfigReturn
 *       IPC message
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwElephantConfigGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwElephantConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwElephantConfigGet(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwDataGetAllStop function
 * @endInternal
 *
 * @brief Request to stop the periodic IPFIX Data Export
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwDataGetAllStop
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.dataGetAllStop.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_STOP_E;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwDataGetAllStop function
 * @endInternal
 *
 * @brief Request to stop the periodic IPFIX Data Export
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataGetAllStop
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwDataGetAllStop);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwDataGetAllStop(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwDataClear function
 * @endInternal
 *
 * @brief Clear IPFIX counters of an entry, both in HW IPFIX Table and in counters extension in SRAM
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  flowId       - flow identifier.
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwDataClear
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  GT_U32      flowId
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);
    PRV_CPSS_DXCH_IPFIX_FW_FLOWID_CHECK_MAC(devNum, flowId);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.dataClear.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_E;
    ipc_msg.dataClear.flowId = flowId;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwDataClear function
 * @endInternal
 *
 * @brief Clear IPFIX counters of an entry, both in HW IPFIX Table and in counters extension in SRAM
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 * @param[in]  flowId       - flow identifier.
 *                       For Falcon, flowId[14:12] represents Port Group Id.
 *                           Tile #0: port group 0, 1 for CP #0, CP #1, respectively
 *                           Tile #1: port group 2, 3 for CP #0, CP #1, respectively
 *                           Tile #2: port group 4, 5 for CP #0, CP #1, respectively
 *                           Tile #3: port group 6, 7 for CP #0, CP #1, respectively
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataClear
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum,
    IN  GT_U32      flowId
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwDataClear);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum, flowId));

    rc = internal_cpssDxChIpfixFwDataClear(devNum, sCpuNum, flowId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum, flowId));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChIpfixFwDataClearAll function
 * @endInternal
 *
 * @brief Clear all IPFIX counters of all IPFIX entries that were allocated to IPFIX
 *        (whether they are active or not), both in HW IPFIX Table and in counters extension in SRAM
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
static GT_STATUS internal_cpssDxChIpfixFwDataClearAll
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS                          rc  = GT_OK;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_SEND_UNT ipc_msg;

    /* check device */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_DEV_SUPPORT_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_SCPU_NUM_CHECK_MAC(devNum, sCpuNum);

    cpssOsMemSet(&ipc_msg, 0, sizeof(ipc_msg));
    ipc_msg.dataClear.opcode = CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_CLEAR_E;
    ipc_msg.dataClear.flowId = 0;
    rc = prvCpssDxChIpfixFwIpcMessageSend(devNum, sCpuNum, &ipc_msg);

    return rc;
}

/**
 * @internal cpssDxChIpfixFwDataClearAll function
 * @endInternal
 *
 * @brief Clear all IPFIX counters of all IPFIX entries that were allocated to IPFIX
 *        (whether they are active or not), both in HW IPFIX Table and in counters extension in SRAM
 *
 * @note  APPLICABLE_DEVICES: AC5X, AC5P, Falcon.
 *
 * @param[in]  devNum       - device number
 * @param[in]  sCpuNum      - service CPU number
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error.
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssDxChIpfixFwDataClearAll
(
    IN  GT_U8       devNum,
    IN  GT_U32      sCpuNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChIpfixFwDataClearAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, sCpuNum));

    rc = internal_cpssDxChIpfixFwDataClearAll(devNum, sCpuNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, sCpuNum));

    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_IPFIX_MANAGER_CNS);

    return rc;
}
