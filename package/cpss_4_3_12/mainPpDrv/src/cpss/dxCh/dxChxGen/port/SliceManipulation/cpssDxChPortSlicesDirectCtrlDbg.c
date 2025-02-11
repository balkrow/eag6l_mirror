/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChPortSlicesDirectCtrl.c
*
* DESCRIPTION:
*       Pizza Aribiter Units Slices Manipulation Interface
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesRxDMA.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesRxDMACTU.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxDMA.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxDMACTU.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesBM.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/private/prvCpssDxChPortSlicesTxQHWDef.h>
#include <cpss/dxCh/dxChxGen/port/SliceManipulation/cpssDxChPortSlicesDirectCtrlDbg.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

typedef GT_STATUS (*PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICES_NUM_SET_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 slicesNum,
    IN GT_BOOL enableStrictPriority4CPU
);

typedef GT_STATUS (*PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICES_NUM_GET_FUNC)
(
    IN   GT_U8  devNum,
    IN   GT_U32 portGroupId,
    OUT  GT_U32  *slicesNumPtr,
    OUT  GT_BOOL *enableStrictPriority4CPUPtr
);


typedef GT_STATUS (*PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICE_OCCUPY_FUNC)
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 sliceIdx_inGop,
    IN  GT_PHYSICAL_PORT_NUM localPort
);

typedef GT_STATUS (*PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICE_RELEASE_FUNC)
(
    IN  GT_U8  devNum,
    IN  GT_U32 portGroupId,
    IN  GT_U32 sliceIdx_inGop
);

typedef GT_STATUS (*PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICE_STATE_SET_FUNC)
(
    IN   GT_U8  devNum,
    IN   GT_U32 portGroupId,
    IN   GT_U32 sliceIdx_inGop,
    OUT  GT_BOOL isEnabled,
    OUT  GT_PHYSICAL_PORT_NUM portId
);


typedef GT_STATUS (*PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICE_STATE_GET_FUNC)
(
    IN   GT_U8  devNum,
    IN   GT_U32 portGroupId,
    IN   GT_U32 sliceIdx_inGop,
    OUT  GT_BOOL * isOccupiedPtr,
    OUT  GT_PHYSICAL_PORT_NUM * portNumPtr
);


typedef struct PRV_DXCH_PA_UNIT_IF_STCT
{
    CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT                 unit;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICES_NUM_SET_FUNC  slicesNumSetFunc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICES_NUM_GET_FUNC  slicesNumGetFunc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICE_STATE_SET_FUNC sliceStateSetFunc;
    PRV_CPSS_DXCH_PIZZA_ARBITER_UNIT_SLICE_STATE_GET_FUNC sliceStateGetFunc;
}PRV_DXCH_PA_UNIT_IF_STC;


/* #define UNUSED_PARAMETER_CNS(x) x = x */

/**
* @internal prvLion2PortPizzaArbiterTxQWrapperSlicesNumSet function
* @endinternal
*
* @brief   The wrapper function sets slice number used in DRV of TxQ unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] slicesNum2Configure      - slices number to be set in pizza arbiter
* @param[in] enableStrictPriority4CPU - strict priority to CPU enable or disable
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
*/
static GT_STATUS prvLion2PortPizzaArbiterTxQWrapperSlicesNumSet
(
    IN GT_U8  devNum,
    IN GT_U32 portGroupId,
    IN GT_U32 slicesNum2Configure,
    IN GT_BOOL enableStrictPriority4CPU
)
{
    GT_STATUS rc;

    /* UNUSED_PARAMETER(enableStrictPriority4CPU) */
    enableStrictPriority4CPU = enableStrictPriority4CPU;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    rc = prvLion2PortPizzaArbiterTxQDrvSlicesNumSet(devNum,portGroupId,slicesNum2Configure);
    return rc;
}

/**
* @internal prvLion2PortPizzaArbiterTxQWrapperSlicesNumGet function
* @endinternal
*
* @brief   The wrapper function get slice number used in Pizza Arbiter and strict priority state bit
*         for BM unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
*
* @param[out] slicesNumPtr             - place to store slices number used in pizza arbiter
* @param[out] enableStrictPriority4CPUPtr - where to store strict priority to CPU bit (dummy parameter)
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_BAD_PTR               - bad slicesNumPtr, enableStrictPriority4CPUPtr
* @retval GT_HW_ERROR              - hw error
*/
static GT_STATUS prvLion2PortPizzaArbiterTxQWrapperSlicesNumGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   portGroupId,
    OUT GT_U32  *slicesNumPtr,
    OUT GT_BOOL *enableStrictPriority4CPUPtr
)
{
    GT_STATUS rc;

    /* UNUSED_PARAMETER_CNS(enableStrictPriority4CPUPtr); */
    enableStrictPriority4CPUPtr = enableStrictPriority4CPUPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);


    if (NULL == slicesNumPtr || NULL == enableStrictPriority4CPUPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    rc = prvLion2PortPizzaArbiterTxQDrvSlicesNumGet(devNum,portGroupId,/*OUT*/slicesNumPtr);
    return rc;
}


/*--------------------------------------------------------*/
/* Slice manipalation default                             */
/*--------------------------------------------------------*/
static const PRV_DXCH_PA_UNIT_IF_STC  prv_unitSliceManipArr[] =
{
     {  CPSS_DXCH_DIAG_PA_UNIT_RXDMA_E,     prvLion2PortPizzaArbiterRxDMASlicesNumSet,
                                            prvLion2PortPizzaArbiterRxDMASlicesNumGet,
                                            prvLion2PortPizzaArbiterRxDMASliceStateSet,
                                            prvLion2PortPizzaArbiterRxDMASliceGetState
     }
    ,{  CPSS_DXCH_DIAG_PA_UNIT_RXDMA_CTU_E, prvLion2PortPizzaArbiterRxDMACTUSlicesNumSet,
                                            prvLion2PortPizzaArbiterRxDMACTUSlicesNumGet,
                                            prvLion2PortPizzaArbiterRxDMACTUSliceStateSet,
                                            prvLion2PortPizzaArbiterRxDMACTUSliceGetState
     }
    ,{  CPSS_DXCH_DIAG_PA_UNIT_TXDMA_E,     prvLion2PortPizzaArbiterTxDMASlicesNumSet,
                                            prvLion2PortPizzaArbiterTxDMASlicesNumGet,
                                            prvLion2PortPizzaArbiterTxDMASliceStateSet,
                                            prvLion2PortPizzaArbiterTxDMASliceGetState
     }
    ,{  CPSS_DXCH_DIAG_PA_UNIT_TXDMA_CTU_E, prvLion2PortPizzaArbiterTxDMACTUSlicesNumSet,
                                            prvLion2PortPizzaArbiterTxDMACTUSlicesNumGet,
                                            prvLion2PortPizzaArbiterTxDMACTUSliceStateSet,
                                            prvLion2PortPizzaArbiterTxDMACTUSliceGetState
     }
    ,{  CPSS_DXCH_DIAG_PA_UNIT_BM_E,        prvLion2PortPizzaArbiterBMSlicesNumSet,
                                            prvLion2PortPizzaArbiterBMSlicesNumGet,
                                            prvLion2PortPizzaArbiterBMSliceStateSet,
                                            prvLion2PortPizzaArbiterBMSliceGetState
     }
    ,{  CPSS_DXCH_DIAG_PA_UNIT_TXQ_E,       prvLion2PortPizzaArbiterTxQWrapperSlicesNumSet,
                                            prvLion2PortPizzaArbiterTxQWrapperSlicesNumGet,
                                            prvLion2PortPizzaArbiterTxQDrvSliceStateSet,
                                            prvLion2PortPizzaArbiterTxQDrvSliceStateGet
     }
};






/**
* @internal prvCpssDxChPAUnitIfSelect function
* @endinternal
*
* @brief   The function gets interface for specific module
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] unitId                   - unit id
*
* @param[out] ifPtrPtr                 - (pointer to ) interface to specific unit
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
static GT_STATUS prvCpssDxChPAUnitIfSelect
(
    IN  CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT  unitId,
    OUT const PRV_DXCH_PA_UNIT_IF_STC **ifPtrPtr
)
{
    if (unitId >= CPSS_DXCH_DIAG_PA_UNIT_MAX_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if (NULL == ifPtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *ifPtrPtr = &prv_unitSliceManipArr[unitId];
    if ((*ifPtrPtr)->unit != unitId)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ABORTED, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/**
* @internal internal_cpssDxChSlicesNumSet function
* @endinternal
*
* @brief   The function sets number of slices to be used in Pizza Arbiter for specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit id
* @param[in] sliceNumToConfig         - slices number to be set in pizza arbiter
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
static GT_STATUS internal_cpssDxChSlicesNumSet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    IN GT_U32                                sliceNumToConfig
)
{
    GT_STATUS rc;
    GT_U32 sliceNumConfigured;
    GT_BOOL strictPriority2Cpu;
    const PRV_DXCH_PA_UNIT_IF_STC * ifPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    rc = prvCpssDxChPAUnitIfSelect(unitId,/*OUT*/&ifPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = ifPtr->slicesNumGetFunc(devNum,portGroupId, /*OUT*/&sliceNumConfigured,/*OUT*/&strictPriority2Cpu);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = ifPtr->slicesNumSetFunc(devNum,portGroupId,sliceNumToConfig,strictPriority2Cpu);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal cpssDxChSlicesNumSet function
* @endinternal
*
* @brief   The function sets number of slices to be used in Pizza Arbiter for specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit id
* @param[in] sliceNumToConfig         - slices number to be set in pizza arbiter
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
GT_STATUS cpssDxChSlicesNumSet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    IN GT_U32                                sliceNumToConfig
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChSlicesNumSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, unitId, sliceNumToConfig));

    rc = internal_cpssDxChSlicesNumSet(devNum, portGroupId, unitId, sliceNumToConfig);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, unitId, sliceNumToConfig));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChSlicesNumGet function
* @endinternal
*
* @brief   The function gets number of slices used in Pizza Arbiter for specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit id
*
* @param[out] numOfConfiguredSlicesPtr - (the pointer to) number of slices used by pizza arbiter
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_ABORTED               - error in internal data structures
*/
static GT_STATUS internal_cpssDxChSlicesNumGet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    OUT GT_U32                              *numOfConfiguredSlicesPtr
)
{
    GT_STATUS rc;
    GT_BOOL strictPriority2Cpu;
    const PRV_DXCH_PA_UNIT_IF_STC * ifPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    if (NULL == numOfConfiguredSlicesPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChPAUnitIfSelect(unitId,/*OUT*/&ifPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = ifPtr->slicesNumGetFunc(devNum,portGroupId, /*OUT*/numOfConfiguredSlicesPtr,&strictPriority2Cpu);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;

}

/**
* @internal cpssDxChSlicesNumGet function
* @endinternal
*
* @brief   The function gets number of slices used in Pizza Arbiter for specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit id
*
* @param[out] numOfConfiguredSlicesPtr - (the pointer to) number of slices used by pizza arbiter
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_ABORTED               - error in internal data structures
*/
GT_STATUS cpssDxChSlicesNumGet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    OUT GT_U32                              *numOfConfiguredSlicesPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChSlicesNumGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, unitId, numOfConfiguredSlicesPtr));

    rc = internal_cpssDxChSlicesNumGet(devNum, portGroupId, unitId, numOfConfiguredSlicesPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, unitId, numOfConfiguredSlicesPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}



/**
* @internal internal_cpssDxChSliceStateSet function
* @endinternal
*
* @brief   The function sets slice state for specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit id
* @param[in] sliceNum                 - slice number to be set in pizza arbiter
* @param[in] localPortNum             - local port number
* @param[in] isEnable                 - is enable or disable
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
static GT_STATUS internal_cpssDxChSliceStateSet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    IN GT_U32                                sliceNum,
    IN GT_U32                                localPortNum,
    IN GT_BOOL                               isEnable
)
{
    GT_STATUS rc;
    const PRV_DXCH_PA_UNIT_IF_STC * ifPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    rc = prvCpssDxChPAUnitIfSelect(unitId,/*OUT*/&ifPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = ifPtr->sliceStateSetFunc(devNum,portGroupId, sliceNum,isEnable,localPortNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSliceStateSet function
* @endinternal
*
* @brief   The function sets slice state for specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit id
* @param[in] sliceNum                 - slice number to be set in pizza arbiter
* @param[in] localPortNum             - local port number
* @param[in] isEnable                 - is enable or disable
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
GT_STATUS cpssDxChSliceStateSet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    IN GT_U32                                sliceNum,
    IN GT_U32                                localPortNum,
    IN GT_BOOL                               isEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChSliceStateSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, unitId, sliceNum, localPortNum, isEnable));

    rc = internal_cpssDxChSliceStateSet(devNum, portGroupId, unitId, sliceNum, localPortNum, isEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, unitId, sliceNum, localPortNum, isEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChSliceStateGet function
* @endinternal
*
* @brief   The function gets state of specific slice in specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit idex
* @param[in] sliceNum                 - number of slice which state is desired to get
*
* @param[out] localPortNumPtr          - (pointer to) local port number
* @param[out] isEnablePtr              - (pointer to) whether port is enable or disable
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
static GT_STATUS internal_cpssDxChSliceStateGet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    IN GT_U32                                sliceNum,
    OUT GT_U32                               *localPortNumPtr,
    OUT GT_BOOL                              *isEnablePtr
)
{
    GT_STATUS rc;
    const PRV_DXCH_PA_UNIT_IF_STC * ifPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    rc = prvCpssDxChPAUnitIfSelect(unitId,/*OUT*/&ifPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = ifPtr->sliceStateGetFunc(devNum,portGroupId,sliceNum,/*OUT*/isEnablePtr,/*OUT*/localPortNumPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChSliceStateGet function
* @endinternal
*
* @brief   The function gets state of specific slice in specific unit
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group id
* @param[in] unitId                   - unit idex
* @param[in] sliceNum                 - number of slice which state is desired to get
*
* @param[out] localPortNumPtr          - (pointer to) local port number
* @param[out] isEnablePtr              - (pointer to) whether port is enable or disable
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - bad devNum, portGroupId
* @retval GT_HW_ERROR              - hw error
* @retval GT_OUT_OF_RANGE          - number of slice to be set is greater than available
* @retval GT_ABORTED               - error in internal data structures
*/
GT_STATUS cpssDxChSliceStateGet
(
    IN GT_U8                                 devNum,
    IN GT_U32                                portGroupId,
    IN CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitId,
    IN GT_U32                                sliceNum,
    OUT GT_U32                               *localPortNumPtr,
    OUT GT_BOOL                              *isEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChSliceStateGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupId, unitId, sliceNum, localPortNumPtr, isEnablePtr));

    rc = internal_cpssDxChSliceStateGet(devNum, portGroupId, unitId, sliceNum, localPortNumPtr, isEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupId, unitId, sliceNum, localPortNumPtr, isEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

#undef SLICE_MANIP_IF_TEST

#ifdef SLICE_MANIP_IF_TEST

    GT_STATUS prvSliceManipIfTest(void)
    {
        CPSS_DXCH_DIAG_PIZZA_ARBITER_UNIT_ENT unitIdxArr[] =
        {
             CPSS_DXCH_DIAG_PA_UNIT_RXDMA_E
            ,CPSS_DXCH_DIAG_PA_UNIT_RXDMA_CTU_E
            ,CPSS_DXCH_DIAG_PA_UNIT_TXDMA_E
            ,CPSS_DXCH_DIAG_PA_UNIT_TXDMA_CTU_E
            ,CPSS_DXCH_DIAG_PA_UNIT_BM_E
            ,CPSS_DXCH_DIAG_PA_UNIT_TXQ_E
        };

        GT_U32 i;
        GT_STATUS rc;
        GT_U32 sliceConfiguredOld;
        GT_U32 sliceConfigured;
        GT_U8 devNum = 0;
        GT_U32 pg = 0;
        GT_U32 sliceId;

        GT_BOOL isEnabledOld;
        GT_U32  assigned2portIdOld;
        GT_BOOL isEnabled;
        GT_U32  assigned2portId;

        for (i = 0 ; i < sizeof(unitIdxArr)/sizeof(unitIdxArr[0]); i++)
        {
            rc = cpssDxChSlicesNumGet(devNum,pg,unitIdxArr[i],/*OUT*/&sliceConfiguredOld);
            if (GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChSlicesNumSet(devNum,pg,unitIdxArr[i],5);
            if (GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChSlicesNumGet(devNum,pg,unitIdxArr[i],/*OUT*/&sliceConfigured);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (sliceConfigured != 5)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            rc = cpssDxChSlicesNumSet(devNum,pg,unitIdxArr[i],sliceConfiguredOld);
            if (GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChSlicesNumGet(devNum,pg,unitIdxArr[i],/*OUT*/&sliceConfigured);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (sliceConfigured != sliceConfiguredOld)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            sliceId = i;
            rc = cpssDxChSliceStateGet(devNum,pg,unitIdxArr[i],sliceId,/*OUT*/&assigned2portIdOld,&isEnabledOld);
            if (GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChSliceStateSet(devNum,pg,unitIdxArr[i],sliceId,5, GT_FALSE);
            if (GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChSliceStateGet(devNum,pg,unitIdxArr[i],sliceId,/*OUT*/&assigned2portId,&isEnabled);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (isEnabled != GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if (assigned2portId != 5)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG) ;
            }
            rc = cpssDxChSliceStateSet(devNum,pg,unitIdxArr[i],sliceId,assigned2portIdOld,isEnabledOld);
            if (GT_OK != rc)
            {
                return rc;
            }
            rc = cpssDxChSliceStateGet(devNum,pg,unitIdxArr[i],sliceId,/*OUT*/&assigned2portId,&isEnabled);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (isEnabledOld != isEnabled)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
            if (assigned2portId != assigned2portId)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG) ;
            }
        }
        return GT_OK;
     }
#endif


