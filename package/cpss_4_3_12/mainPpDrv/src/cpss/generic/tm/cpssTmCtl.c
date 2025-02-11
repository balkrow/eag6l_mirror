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
* @file cpssTmCtl.c
*
* @brief TM Configuration Library control interface APIs
*
* @version   3
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/generic/tm/private/prvCpssGenTmLog.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/extServices/os/gtOs/gtGenTypes.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <tm_ctl.h>
#include <tm_nodes_read.h>
#include <platform/tm_regs.h>
#include <tm_errors.h>

#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlue.h>
#include <platform/cpss_tm_rw_registers_proc.h>

#include <cpssDriver/pp/hardware/prvCpssDrvHwCntl.h>
#include <cpssDriver/pp/hardware/cpssDriverPpHw.h>
#include <cpss/generic/cpssHwInit/private/prvCpssHwRegisters.h>

#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/prvCpssTmCtl.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiagDataIntegrityMainMappingDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* Min num Of Lads By CPSS Tm Device */
const GT_U8 prvCpssMinDramInfArr[PRV_CPSS_TM_DEV_LAST_E+1] =
{
    2, /* PRV_CPSS_TM_DEV_BC2_E */
    1, /* PRV_CPSS_TM_DEV_BOBK_CETUS_E  */
    1, /* PRV_CPSS_TM_DEV_BOBK_CAELUM_E */
    0
};

/* Max num Of Lads By CPSS Tm Device */
const GT_U8 prvCpssMaxDramInfArr[PRV_CPSS_TM_DEV_LAST_E+1] =
{
    5, /* PRV_CPSS_TM_DEV_BC2_E */
    1, /* PRV_CPSS_TM_DEV_BOBK_CETUS_E  */
    3, /* PRV_CPSS_TM_DEV_BOBK_CAELUM_E */
    0
};

#ifdef ASIC_SIMULATION
    #define WAIT_FOR_DEVICE_MAC(mili)
#else /*ASIC_SIMULATION*/
    #define WAIT_FOR_DEVICE_MAC(mili)   \
        cpssOsTimerWkAfter(mili)
#endif /*ASIC_SIMULATION*/


/**
* @internal prvCpssTmCtlGetTmHwParams function
* @endinternal
*
* @brief   Get TM HW params per device family type.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] tmHwParamsPtr            - pointer to PRV_CPSS_TM_HW_PARAMS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS prvCpssTmCtlGetTmHwParams
(
    IN GT_U8 devNum,
    OUT PRV_CPSS_TM_HW_PARAMS_STC **tmHwParamsPtr
)
{
    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(tmHwParamsPtr);

    if ((PRV_CPSS_PP_MAC(devNum)->devFamily ==  CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
        (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E))
    {
        *tmHwParamsPtr = &PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmHwParamsBobk);
    }
    else
    {
        *tmHwParamsPtr = &PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmHwParamsBc2);
    }

    return rc;
}

/**
* @internal internal_cpssTmCtlWriteRegister function
* @endinternal
*
* @brief   Write register USED before libInit for cpssTmCtlHwInit when no tm_hndl exist.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddrPtr               - Pointer to register address.
* @param[in] dataPtr                  - Pointer to write data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmCtlWriteRegister
(
    IN GT_U8                   devNum,
    IN GT_U64                  *regAddrPtr,
    IN GT_U64                  *dataPtr
)
{
    GT_STATUS   rc;
    GT_U32      data;
    GT_U8       burstMode;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    /*check whether we are in burst or non burst mode */
    rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum, 0x000F828C ,&data);
    if (GT_OK != rc)   return rc;

    burstMode = ((data & 0x10)==0);
    /* temporary   : burst mode is suppressed */
    burstMode = 0;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = tm_write_register_proc(devNum, burstMode, *regAddrPtr,  dataPtr);

    return rc;
}

/**
* @internal cpssTmCtlWriteRegister function
* @endinternal
*
* @brief   Write register USED before libInit for cpssTmCtlHwInit when no tm_hndl exist.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddrPtr               - POinter to register address.
* @param[in] dataPtr                  - Pointer to write data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCtlWriteRegister
(
    IN GT_U8                   devNum,
    IN GT_U64                  *regAddrPtr,
    IN GT_U64                  *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmCtlWriteRegister);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddrPtr, dataPtr));

    rc = internal_cpssTmCtlWriteRegister(devNum, regAddrPtr, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddrPtr, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssTmCtlReadRegister function
* @endinternal
*
* @brief   Read register USED before libInit for cpssTmCtlHwInit when no tm_hndl exist.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddrPtr               - Pointer to register address.
* @param[in] dataPtr                  - Pointer to write data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmCtlReadRegister
(
    IN GT_U8                   devNum,
    IN GT_U64                  *regAddrPtr,
    IN GT_U64                  *dataPtr
)
{
    GT_STATUS   rc;
    GT_U32      data;
    GT_U8       burstMode;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);
    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    /*check whether we are in burst or non burst mode */
    rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum, 0x000F828C ,&data);
    if (GT_OK != rc)
        return rc;

    burstMode =  ((data & 0x10)==0);
    /* temporary   : burst mode is suppressed */
    burstMode = 0;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    return  tm_read_register_proc(devNum, burstMode, *regAddrPtr,  dataPtr);
}

/**
* @internal cpssTmCtlReadRegister function
* @endinternal
*
* @brief   Read register USED before libInit for cpssTmCtlHwInit when no tm_hndl exist.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] regAddrPtr               - Pointer to register address.
* @param[in] dataPtr                  - Pointer to write data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmCtlReadRegister
(
    IN GT_U8                   devNum,
    IN GT_U64                  *regAddrPtr,
    IN GT_U64                  *dataPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmCtlReadRegister);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, regAddrPtr, dataPtr));

    rc = internal_cpssTmCtlReadRegister(devNum, regAddrPtr, dataPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, regAddrPtr, dataPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}

/**
* @internal internal_cpssTmInit function
* @endinternal
*
* @brief   Initialize the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] tmLibInitParamsPtr       (pointer of) CPSS_TM_LIB_INIT_PARAMS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmInit
(
    IN GT_U8 devNum,
    IN CPSS_TM_LIB_INIT_PARAMS_STC *tmLibInitParamsPtr
)
{
    GT_STATUS                   rc = GT_OK;
    struct tm_lib_init_params   tm_lib_init_params = {0};
    struct tm_tree_structure    tm_tree_params;
    PRV_CPSS_TM_HW_PARAMS_STC   *tmHwParamsPtr;
    /*
    PRV_CPSS_GEN_TM_DB_STC      *tmInfoPtr;
    */
    int                         ret = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle != NULL)
    {
        TM_DBG_INFO(("---- cpssTmInit:tm hndl already exist and tm_lib was invoked, ignoring ...\n"));
        return GT_OK;
    }

    rc = prvCpssTmCtlDevInit(devNum); /* verify tm device information is initialized */

    if (rc != GT_OK)
    {
        /* TM is Not Supported */
        cpssOsPrintf("TmCtl: devNum: %d, TM is Not Supported, rc: %d\n", devNum, rc);
        return rc;
    }

    CPSS_NULL_PTR_CHECK_MAC(tmLibInitParamsPtr);

    cpssTmToCpssErrCodesInit();

    tm_lib_init_params.tm_mtu = tmLibInitParamsPtr->tmMtu;
    /* more init params here */

    rc = prvCpssTmCtlGetTmHwParams(devNum, &tmHwParamsPtr);
    if (rc != GT_OK)
    {
        TM_DBG_INFO(("prvCpssTmCtlGetTmHwParams has failed\n"));
        return rc;
    }

    tm_tree_params.numOfQueues = tmHwParamsPtr->maxQueues;
    tm_tree_params.numOfAnodes = tmHwParamsPtr->maxAnodes;
    tm_tree_params.numOfBnodes = tmHwParamsPtr->maxBnodes;
    tm_tree_params.numOfCnodes = tmHwParamsPtr->maxCnodes;
    tm_tree_params.numOfPorts  = tmHwParamsPtr->maxPorts;

    tm_tree_params.queuesToAnode = tmHwParamsPtr->queuesToAnode;
    tm_tree_params.aNodesToBnode = tmHwParamsPtr->aNodesToBnode;
    tm_tree_params.bNodesToCnode = tmHwParamsPtr->bNodesToCnode;
    tm_tree_params.cNodesToPort  = tmHwParamsPtr->cNodesToPort;
    tm_tree_params.installedQueuesPerPort = tmHwParamsPtr->installedQueuesPerPort;

    /* TM Initialization */
    ret = tm_lib_open_ext(devNum, &tm_tree_params,  &tm_lib_init_params, &PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle);
    PRV_CPSS_PP_MAC(devNum)->tmInfo.tmLibIsInitilized = GT_TRUE;
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    /* Increase number of ready TM devices */
    if (rc == GT_OK)
    {
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmCurrentNum)++;
        rc = cpssDxChTmGlueInit(devNum);
    }
    else
    {
        PRV_CPSS_PP_MAC(devNum)->tmInfo.tmLibIsInitilized = GT_FALSE;
    }

    return rc;
}

/**
* @internal cpssTmInit function
* @endinternal
*
* @brief   Initialize the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;
    CPSS_TM_LIB_INIT_PARAMS_STC tmLibInitParams;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmInit);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    cpssOsMemSet(&tmLibInitParams, 0, sizeof(tmLibInitParams));
    rc = internal_cpssTmInit(devNum, &tmLibInitParams);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    return rc;
}
/**
* @internal cpssTmInitExt function
* @endinternal
*
* @brief   Customize Initialize of the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] tmLibInitParamsPtr       (pointer of) TM LIB initialize parameters.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmInitExt
(
    IN GT_U8 devNum,
    IN CPSS_TM_LIB_INIT_PARAMS_STC *tmLibInitParamsPtr
)
{
        GT_STATUS rc;
        CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmInitExt);

        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
        CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmLibInitParamsPtr));

        rc = internal_cpssTmInit(devNum, tmLibInitParamsPtr);

        CPSS_LOG_API_EXIT_MAC(funcId, rc);
        CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    return rc;
}

/**
* @internal internal_cpssTmClose function
* @endinternal
*
* @brief   Close the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmClose
(
    IN GT_U8 devNum
)
{
    int         ret = 0;
    GT_STATUS   rc = GT_OK;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle == NULL)
    {
        TM_DBG_INFO(("---- cpssTmCtlLibClose: tm_hndl is NULL ... "));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /** Close TM configuration library and free parameters */
    ret = tm_lib_close(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle);
    rc = XEL_TO_CPSS_ERR_CODE(ret);
    if (rc != GT_OK)
        return rc;

    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmCurrentNum)--;

    if (PRV_CPSS_PP_MAC(devNum)->tmInfo.pfcTmGluePfcDevInfoPtr)
    {
        cpssOsFree(PRV_CPSS_PP_MAC(devNum)->tmInfo.pfcTmGluePfcDevInfoPtr);
    }

    cpssOsMemSet(&PRV_CPSS_PP_MAC(devNum)->tmInfo, 0, sizeof(PRV_CPSS_GEN_TM_DB_STC));

    return rc;
}

/**
* @internal cpssTmClose function
* @endinternal
*
* @brief   Close the TM configuration library.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmClose
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmClose);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssTmClose(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssTmTreeParamsGet function
* @endinternal
*
* @brief   Get TM Tree Hw properties.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] tmTreeParamsPtr          - pointer to CPSS_TM_TREE_PARAMS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmTreeParamsGet
(
    IN GT_U8 devNum,
    OUT CPSS_TM_TREE_PARAMS_STC *tmTreeParamsPtr
)
{
    GT_STATUS   rc = GT_OK;
    PRV_CPSS_TM_HW_PARAMS_STC *tmHwParamsPtr;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(tmTreeParamsPtr);

    rc = prvCpssTmCtlGetTmHwParams(devNum, &tmHwParamsPtr);
    if (rc != GT_OK)
        return rc;

    tmTreeParamsPtr->maxQueues = tmHwParamsPtr->maxQueues;
    tmTreeParamsPtr->maxAnodes = tmHwParamsPtr->maxAnodes;
    tmTreeParamsPtr->maxBnodes = tmHwParamsPtr->maxBnodes;
    tmTreeParamsPtr->maxCnodes = tmHwParamsPtr->maxCnodes;
    tmTreeParamsPtr->maxPorts  = tmHwParamsPtr->maxPorts;

    return rc;
}

/**
* @internal cpssTmTreeParamsGet function
* @endinternal
*
* @brief   Get TM Tree Hw properties.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] tmTreeParamsPtr          - pointer to CPSS_TM_TREE_PARAMS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmTreeParamsGet
(
    IN GT_U8 devNum,
    OUT CPSS_TM_TREE_PARAMS_STC *tmTreeParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmTreeParamsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, tmTreeParamsPtr));

    rc = internal_cpssTmTreeParamsGet(devNum, tmTreeParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, tmTreeParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}

/**
* @internal prvCpssTmCtlGlobalParamsInit function
* @endinternal
*
* @brief   Initialize the TM global params HW configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmCtlGlobalParamsInit
(
    IN GT_U8                   devNum
)
{
    GT_U64      writeData;
    GT_U64      regAddr;
    GT_U32      regData;
    GT_U32      j;
    GT_U32      deviceCtrl15RegAddr;
    PRV_CPSS_GEN_TM_DB_STC  *tmInfoPtr;
    GT_STATUS   rc = GT_OK;

    /*cpssOsPrintf("\n ***********GLOBAL*********** \n ");*/

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    /* check if we are in non burst mode*/
    deviceCtrl15RegAddr = PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                          DFXServerUnitsDeviceSpecificRegs.deviceCtrl15;

    rc = cpssDrvHwPpResetAndInitControllerReadReg(devNum, deviceCtrl15RegAddr, &regData);
    if (rc)
    {
        return rc;
    }

#if 0
    #ifdef GM_USED
    regData = 0x10;
    #endif /*GT_USED*/

    if (!(regData & 0x10))
    {
        rc = GT_BAD_STATE;
    }

    if (rc)
    {
        return rc;
    }
#endif
    /*Write_QMR_offset_Phase_0
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0xC0FFFF8,0xFFFFFFFF ,0x8000);
    if (rc)
    {
        return rc;
    }

    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0xC0FFFFC,0xFFFFFFFF ,0x0000123);
    if (rc)
    {
        return rc;
    }

    Write__to_OOR_Phase_4()
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C001020,0xFFFFFFFF ,3);
    if (rc)
    {
        return rc;
    }
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C001024,0xFFFFFFFF ,0x123);
    if (rc)
    {
        return rc;
    }
    */

    writeData.l[1] = 0x0;
    writeData.l[0] = 0x00000003;
    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.OORPkgPanicThresh;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }

    /*
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C001028,0xFFFFFFFF ,3);
    if (rc)
    {
        return rc;
    }
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C00102C,0xFFFFFFFF ,0x00000123);
    if (rc)
    {
        return rc;
    }
    */

    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.OORPagePanicThresh;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }

    /*
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C001010,0xFFFFFFFF ,3);
    if (rc)
    {
        return rc;
    }
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0, 0x0C001014,0xFFFFFFFF ,0x00000123);
    if (rc)
    {
        return rc;
    }
    */

    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.OORPkgDropThresh;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }

    /*
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C001018,0xFFFFFFFF , 3);
    if (rc)
    {
        return rc;
    }
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C00101C,0xFFFFFFFF ,0x00000123);
    if (rc)
    {
        return rc;
    }
    */

    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.OORPageDropThresh;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }


    /*Write__to_QMR_exception_mask_Phase_6
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C000020,0xFFFFFFFF ,0x2E4FF83F);
    if (rc)
    {
        return rc;
    }
    */

    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.ExcMask;
    writeData.l[0] = 0x2E4FF83F;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }

    /* ???
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0x0C000024,0xFFFFFFFF ,0xA0088931);
    if (rc)
    {
        return rc;
    }
    */

    /*BAP_DRAM_Read_Control_FIFOs_update
    The BP thresholds currently are Set=8, Clr=7. The correct values are Set=6, Clr=5.
    bit 0-4 clr back pressure threshold, bits 32-36 set back pressure threshold*/
    /*write to all BAPs*/
    if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
    {
        tmInfoPtr = &PRV_CPSS_PP_MAC(devNum)->tmInfo;
        CPSS_NULL_PTR_CHECK_MAC(tmInfoPtr);

        for (j = 0; j < CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS; j++)
        {
            if (tmInfoPtr->BapMapping[j] == 0xFF)
                continue;

            writeData.l[0]=0x00000005;
            writeData.l[1]=0x6;
            regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].DramCtlRdFIFOsBp;
            rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
            if (rc)
                return rc;
        }

        /*
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x20;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x28;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x30;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x38;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x40;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x48;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);

        if (rc)
        {
            return rc;
        }
        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x50;

        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }
        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x58;
        rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x60;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }

        writeData.l[0]=0x00000005;
        writeData.l[1]=0x6;
        regAddr.l[0]= 0x00000318;
        regAddr.l[1]= 0x68;
        rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
        if (rc)
        {
            return rc;
        }
        */
    }

    /*Write_QMR_offset_Phase_0
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0xC0FFFF8,0xFFFFFFFF ,0x8000);
    if (rc)
    {
        return rc;
    }
    rc= cpssDrvPpHwRegBitMaskWrite (devNum,0,0xC0FFFFC,0xFFFFFFFF ,0x0000123);
    if (rc)
    {
        return rc;
    }

    Write amount of ports - 192
    writeData.l[0]=0xC0;
    writeData.l[1]=0;
    regAddr.l[0]= 0x00003000;
    regAddr.l[1]= 0x8;
    rc = cpssTmCtlWriteRegister( devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }
    */

    writeData.l[0]=0xC0;
    writeData.l[1]=0;
    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.HWInitPorts;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &writeData);
    if (rc)
    {
        return rc;
    }

    /*Internal_init___control_reg15_bit_0___0___Phase_7*/
    rc= cpssDrvHwPpResetAndInitControllerWriteReg (devNum, deviceCtrl15RegAddr, 0x19F0);
    if (rc)
    {
        return rc;
    }

    /* Soft reset including tables init is active for 2000 core clock cycles (6uS).
       Waiting 1 millisecond should be enough. */
    cpssOsTimerWkAfter(1);

    /* set bit tm_start_init back to be 1 to enable it after Soft Reset */
    rc = prvCpssDrvHwPpResetAndInitControllerSetRegField(devNum, deviceCtrl15RegAddr, 0, 1, 1);
    if (rc)
    {
        return rc;
    }

    /*Synchronize all TM related cpss shadow tables with values from HW.*/
    rc = prvCpssDxChDiagDataIntegrityShadowAndHwSynchTm(devNum);

    return rc;
}

/**
* @internal prvCpssTmCtlLadInit function
* @endinternal
*
* @brief   configure the number of lads in TM HW.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] numOfLad                 - Nubmer of lads.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
static GT_STATUS prvCpssTmCtlLadInit
(
    IN GT_U8                   devNum,
    IN GT_U8                   numOfLad
)
{
    GT_U64 regData;
    GT_U64 writeReg;
    GT_U64      writeData;
    GT_U32      i;
    GT_STATUS   rc = GT_OK;

    /*cpssOsPrintf("\n ***********LAD*********** %d \n ",numOfLad);*/
    PRV_CPSS_TM_CTL_CHECK_DRAM_INF_NUM_MAC(devNum, numOfLad);

    writeData.l[1]=0;

    /*number of lads*/
    /*read*/
    rc = cpssTmCtlReadRegister(devNum, &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.NumOfLADs), &regData);
    if (rc) {
        return rc;
    }

    writeData.l[0]= numOfLad;
    /*write*/
    rc = cpssTmCtlWriteRegister( devNum, &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.NumOfLADs), &writeData);
    if (rc) {
        return rc;
    }

    /* set crc enable */
    writeData.l[0]= 1;
    rc = cpssTmCtlWriteRegister( devNum, &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.CRC16En), &writeData);
    if (rc) {
        return rc;
    }

    /* write minPkgSize to HW */
    writeData.l[0] = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLad].minPkgSize;
    rc = cpssTmCtlWriteRegister(devNum, &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.MinPkgSize), &writeData);
    if (rc) {
        return rc;
    }

    /* write numOfPagesPerBank to HW */
    writeData.l[0] = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLad].pagesPerBank;

    rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.NumOfPagesPerBank), &writeData);
    if (rc) {
        return rc;
    }

    /* write numOfPkgesPerBank to HW */
    writeData.l[0] = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLad].pkgesPerBank;
    rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.NumOfPkgsPerBank), &writeData);
    if (rc) {
        return rc;
    }

    /* write PortChunksEmitPerSel to HW */
    writeData.l[0] = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLad].portChunksEmitPerSel;
    for (i=0; i< 192; i++)
        {
        writeReg.l[0] = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.PortChunksEmitPerSel.l[0] + (i*8);
        writeReg.l[1] = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.PortChunksEmitPerSel.l[1];

        rc = cpssTmCtlWriteRegister( devNum,  &writeReg, &writeData);
        if (rc)
                {
            return rc;
        }
    }

    return rc;
}



/*******************************************************************************
* prvCpssTmCtlDramFrequencyInit
*
* DESCRIPTION:
*       configure the frequency in BAP registers.
*
* APPLICABLE DEVICES:
*       Bobcat2; Caelum.
*
* NOT APPLICABLE DEVICES:
*       xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* INPUTS:
*       devNum            - Device number.
*       numOfLad          - number of active LAD'd
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK                       - on success.
*       GT_FAIL                     - on hardware error.
*       GT_NO_RESOURCE              - on out of memory space.
*       GT_NOT_APPLICABLE_DEVICE    - on not applicable device.
*       GT_BAD_STATE                - on burst mode or init doesnt succeed
*
* COMMENTS:
*       None.
*******************************************************************************/
        /* macro for writing data to register  - useful in following function*/
        #define WRITE_DATA_TO_REGISTER(register_address, mask, value)        \
                        /*read register content */\
/*                if (freqInitSkipParam==i++) continue; */\
                        rc = cpssTmCtlReadRegister( devNum,  &register_address, &regData); \
                        if (rc) return rc; \
                        writeData.l[0]=(regData.l[0] & (~mask)) + (value & mask);\
                        /*write to register */\
                        rc = cpssTmCtlWriteRegister( devNum,  &register_address, &writeData); \
                        if (rc) return rc;\
                        /* test for write  - uncomment if necessary \
                        rc = cpssTmCtlReadRegister( devNum,  &register_address, &regData); \
                        if (rc) return rc; \
                        if ((regData.l[0]!= writeData.l[0]) || (regData.l[1]!= writeData.l[1] ) )\
                        {\
                                cpssOsPrintf("\n %s  %d :\n write data test failed for register 0x%08X  %08X\n", __FILE__, __LINE__ , register_address.l[1],register_address.l[0]);\
cpssOsPrintf("write data : 0x%08X  %08X \n", writeData.l[1], writeData.l[0]);\
cpssOsPrintf("read  data : 0x%08X  %08X \n", regData.l[1], regData.l[0]);\
                               return GT_BAD_PARAM;\
                        }\
                        */
        /* macro for read register content         - for debug purposes */
        #define READ_REGISTER_CONTENT(register_address, mask, value)        \
                        rc = cpssTmCtlReadRegister( devNum,  &register_address, &regData); \
                        if (rc)  \
                        {\
                                cpssOsPrintf("\n %s  %d :  read failed for register %x\n ", __FILE__, __LINE__ , register_address);\
                                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);\
                        }\
                        cpssOsPrintf("\n read register : "#value" =%d (0x%x)\n ", regData.l[0], regData.l[0]);\


GT_STATUS prvCpssTmCtlDramFrequencyInit
(
    IN GT_U8                   devNum,
        IN GT_U8                   numOfLads

)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      uTRC;
    GT_U32      uTFAW;
    GT_U32      uTRRD;
    GT_U32      uTRD_AP_TO_ACT;
    GT_U32      uTWR_AP_TO_ACT;
    GT_U32      uRD_2_WR_NOPS;
    GT_U32      uWR_2_RD_NOPS;
    GT_U32      uAFTER_REFR_NOPS;
    GT_U32      uREF_INT;
    CPSS_DRAM_FREQUENCY_ENT frequency;

    GT_U64 regData;
    GT_U64 writeData;
    int     j;
    GT_U32  tmFreq;

    PRV_CPSS_GEN_TM_DB_STC  *tmInfoPtr;

    /* Read TM frequency */
    rc = prvCpssDxChTMFreqGet(devNum,&tmFreq);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(tmFreq)
    {
        case 2:
            frequency=CPSS_DRAM_FREQ_933_MHZ_E;
            break;
        case 3:
            frequency=CPSS_DRAM_FREQ_667_MHZ_E;
            break;
        case 1:
            frequency=CPSS_DRAM_FREQ_800_MHZ_E;
            break;
        case 0:
        default:
            cpssOsPrintf("DRam frequency: failed to define system frequency, exiting...\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    /*cpssOsPrintf("DRam frequency: %d (read from registry)\n ", frequency);*/

    switch (frequency)
    {

        case CPSS_DRAM_FREQ_667_MHZ_E:
        {
            uTRC  = 33;
            uTFAW = 30;
            uTRRD =  5;
            uTRD_AP_TO_ACT  = 22;
            uTWR_AP_TO_ACT  = 38;
            uRD_2_WR_NOPS   = 2;
            uWR_2_RD_NOPS   = 4;
            uAFTER_REFR_NOPS= 50;
            uREF_INT        = 2600;
            break;
        }
        case CPSS_DRAM_FREQ_800_MHZ_E:
        {
            uTRC  = 39;
            uTFAW = 32;
            uTRRD =  6;
            uTRD_AP_TO_ACT   = 27;
            uTWR_AP_TO_ACT   = 45;
            uRD_2_WR_NOPS    = 3;
            uWR_2_RD_NOPS    = 5;
            uAFTER_REFR_NOPS = 52;
            uREF_INT         = 3120;
            break;
        }
        case CPSS_DRAM_FREQ_933_MHZ_E:
        {
            uTRC  = 46;
            uTFAW = 33;
            uTRRD =  6;
            uTRD_AP_TO_ACT  = 33;
            uTWR_AP_TO_ACT  = 53;
            uRD_2_WR_NOPS   = 3;
            uWR_2_RD_NOPS   = 6;
            uAFTER_REFR_NOPS= 61;
            uREF_INT        = 3645;
            break;
        }
        default:
            cpssOsPrintf("\n undefined system frequency for initializing lads\n ");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        break;
    }

    tmInfoPtr = &PRV_CPSS_PP_MAC(devNum)->tmInfo;

    /* write registers*/
    writeData.l[1]=0;
    for (j = 0; j < CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS; j++)
    {
        if (tmInfoPtr->BapMapping[j] == 0xFF)
            continue;

        /*cpssOsPrintf("bap: %d, bap id: %d\n", j, tmInfoPtr->BapMapping[j]); */
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].tRC,            0x3F,  uTRC)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].tFAW,           0x7F,  uTFAW)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].tRRD,           0x7,   uTRRD)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].RdApToActDelay, 0x3F,  uTRD_AP_TO_ACT)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].WrApToActDelay, 0x3F,  uTWR_AP_TO_ACT)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].Read2WriteNOPs, 0xF,   uRD_2_WR_NOPS)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].Write2ReadNOPs, 0xF,   uWR_2_RD_NOPS)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].AfterRefNOPs,   0x7F,  uAFTER_REFR_NOPS)
        WRITE_DATA_TO_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].RefInterval,    0x3FFF,uREF_INT)
    }

    /*cpssOsPrintf("\n ***********Update default MAxRead/Write/Panic BAP parameters...\n ");*/
    for (j = 0; j < CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS; j++)
    {
        if (tmInfoPtr->BapMapping[j] == 0xFF)
            continue;

        writeData.l[1]=0;
        /* Max Reads */
        writeData.l[0]=80;
        rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].MaxReads), &writeData);
        if (rc) return rc;

        /* Max Reads (Write ) Panic*/
        writeData.l[0]=80;
        rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].MaxReadsPanic), &writeData);
        if (rc) return rc;

        /* Max Writes */
        writeData.l[0]=96;
        rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].MaxWrites), &writeData);
        if (rc) return rc;

        /* Max Writes(write) Panic */
        writeData.l[0]=96;
        rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].MaxWritesPanic), &writeData);
        if (rc) return rc;

        /* Write (Data FIFOs) Panic Threshold */
        writeData.l[0]=8;
        rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].WritePanicThresh), &writeData);
        if (rc) return rc;

        /* DRAM Request Shaping Parameters */
        if (numOfLads ==1)
        {
            writeData.l[0]=0;       /* special case  for 1 lad  N,K,L =0 */
        }
        else
        {
            writeData.l[0]=0xC6;       /* N parameter = 0xc6 , bits 0::13  */
            writeData.l[0]+=(1 << 16); /* K parameter = 1,     bits 16::29 */
            writeData.l[1]=0;          /* L parameter = 0,     bits 32::45 */
        }
        rc = cpssTmCtlWriteRegister( devNum,  &(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[j].ReqShpPrms), &writeData);
        if (rc) return rc;

    }

    return rc;
}

#include <mvHwsDdr3Bap.h>

/**
* @internal prvCpssTmBapDramAccessTestSetParams function
* @endinternal
*
* @brief   customize Bap Dram Access Test.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] enable                   - Whether BAP Dram Access Test is run during TM init.
* @param[in] baseAddress              - One Dram Address To test.
* @param[in] highAddress              - Another Dram Address To test.
* @param[in] retries                  - number of  in case of fail - read data is different from write data.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
*/
GT_STATUS prvCpssTmBapDramAccessTestSetParams
(
    IN GT_BOOL  enable,
    IN GT_U32   baseAddress,
    IN GT_U32   highAddress,
    GT_U32      retries
)
{
    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTest) = enable;
    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramBaseAddress) = baseAddress;
    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramHighAddress) = highAddress;
    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries) = retries;
    cpssOsPrintf("bapDramAccessTest: enable: %d, baseAddress: 0x%0x, highAddress: 0x%0x, Retries: %d\n",
                 PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTest),
                 PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramBaseAddress),
                 PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramHighAddress),
                 PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries));

    return GT_OK;
}

/**
* @internal prvCpssTmBapDramAccessTestBapRun function
* @endinternal
*
* @brief   Run Bap Dram Access Test Per Bap.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bapNum                   - BAP Device Number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
*/
GT_STATUS prvCpssTmBapDramAccessTestBapRun
(
    IN GT_U8    devNum,
    IN GT_U32   bapNum
)
{
    GT_STATUS rc = GT_OK;
    GT_STATUS ret = GT_OK;
    GT_U32 j,k;
    GT_U64 regData;
    GT_U64 regAddr;
    GT_U32 dataSet[4] = {0x1010,0x3030,0x5555,0x7007};
    GT_U32 dataGet[4] = {0,0,0,0};
    GT_U32 addrArr[2];
    GT_U32 bapDramAddr;

    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[bapNum].ID;
    /* Get Bap Identity */
    rc = cpssTmCtlReadRegister(devNum, &regAddr, &regData);
    if (rc)
    {
        cpssOsPrintf("%s %d  ERROR : rc = %d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }

    regData.l[1] = U32_GET_FIELD_MAC(regData.l[0], 8, 8);/* SUID 8..15 */
    regData.l[0] = U32_GET_FIELD_MAC(regData.l[0], 0, 8);/* UID  0..7  */
    cpssOsPrintf("Bap %d Identity: UID: 0x%0x, SUID: 0x%0x\n", bapNum, regData.l[0], regData.l[1]);

    /*  Verify Request Access Mode is Normal */
    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).BAP[bapNum].ReqAcssMode;
    regData.l[0] = 0;
    rc = cpssTmCtlReadRegister(devNum, &regAddr, &regData);
    if (rc)
    {
        cpssOsPrintf("%s %d  ERROR : rc = %d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }

    if (regData.l[0] != 2)
    {
        regData.l[0] = 2;
        regData.l[1] = 0;
        /*set Request Access Mode to Normal */
        rc = cpssTmCtlWriteRegister(devNum, &regAddr, &regData);
        if (rc)
        {
            cpssOsPrintf("%s %d  ERROR : rc = %d\n ",__FILE__, __LINE__ , rc);
            return rc;
        }
        cpssOsPrintf("Set Request Mode To: %d\n", regData.l[0]);
    }

    /* write phase */
    addrArr[0] = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramBaseAddress);
    addrArr[1] = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramHighAddress);
    for (j = 0; j < 2; j++)
    {
        dataSet[0] = 0x1010; dataSet[1] = 0x3030; dataSet[2] = 0x5555; dataSet[3] = 0x7007;
        for (k = 0; k < PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries); k++)
        {
            bapDramAddr = addrArr[j];
            if (k == 0)
                cpssOsPrintf("Addr: 0x%0x ", bapDramAddr);

            rc = mvHwsDdr3BapMemWrite(devNum, bapNum, bapDramAddr, dataSet);
            if (rc)
            {
                if (k == PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries) - 1)
                {
                    cpssOsPrintf("\n");
                    cpssOsPrintf("mvHwsDdr3BapMemWrite: %s %d  ERROR : rc = %d, bapDramAddr: 0x%0x\n ",__FILE__, __LINE__ , rc, bapDramAddr);
                }
                ret = GT_FAIL;
            }

            rc = mvHwsDdr3BapMemRead(devNum, bapNum, bapDramAddr, dataGet);
            if (rc)
            {
                if (k == PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries) - 1)
                {
                    cpssOsPrintf("\n");
                    cpssOsPrintf("mvHwsDdr3BapMemRead: %s %d ERROR : rc = %d, bapDramAddr: 0x%0x\n ",__FILE__, __LINE__ , rc, bapDramAddr);
                }
                ret = GT_FAIL;
            }
            else if (cpssOsMemCmp(dataSet, dataGet, sizeof(dataSet)))
            {
                if (k == PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries) - 1)
                {
                    cpssOsPrintf("\n");
                    cpssOsPrintf("write data is different from read data\n");
                    cpssOsPrintf("write: 0x%0x, 0x%0x, 0x%0x, 0x%0x\n",
                                 dataSet[0],dataSet[1],dataSet[2],dataSet[3]);
                    cpssOsPrintf("read: 0x%0x, 0x%0x, 0x%0x, 0x%0x\n",
                                 dataGet[0],dataGet[1],dataGet[2],dataGet[3]);
                }
                ret = GT_FAIL;
            }
            else
            {
                if (k == PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries) - 1)
                    cpssOsPrintf("OK\n");
            }

            cpssOsMemSet(dataGet, 0, sizeof(dataGet));
            dataSet[0] += 0x1010; dataSet[1] += 0x1010; dataSet[2] += 0x1111; dataSet[3] += 0x1001;
        }  /* for (k = 0; k < PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTestRetries); k++) */
    } /* for (j = 0; j < 2; j++)  */

    return ret;
}

/**
* @internal prvCpssTmBapDramAccessTestRun function
* @endinternal
*
* @brief   Run Bap Dram Access Test On All BAPs.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] numOfLad                 - number of LADs/Dram Interfaces to test.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS prvCpssTmBapDramAccessTestRun
(
    IN GT_U8    devNum,
    IN GT_U32   numOfLad
)
{
    GT_STATUS rc = GT_OK;
    /*GT_STATUS ret = GT_OK;*/
    GT_U32 i, lad, bap;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);


    cpssOsPrintf("\tbapDramAccessTest numOfLads: %d\n", numOfLad);

    for (lad = 1; lad <= MIN(numOfLad, CPSS_TM_CTL_MAX_NUM_OF_LADS_CNS); lad++)
    {
        for (i = 0; i < 2; i++)
        {
            bap = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[lad].bapIDs[i];
            cpssOsPrintf("lad %d, bap %d\n", lad, bap);
            rc = prvCpssTmBapDramAccessTestBapRun(devNum, bap);
            if (rc != GT_OK)
                cpssOsPrintf("lad %d, bap %d Fail\n", lad, bap);
            else
                cpssOsPrintf("lad %d, bap %d Pass\n", lad, bap);
        }
    }
    rc = GT_OK;
    return rc;
}

/**
* @internal prvCpssTmCtlDevInit function
* @endinternal
*
* @brief   Initialize TM Device Info.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*
* @note Called from cpssDxChTmGlueDramInit, prvCpssTmCtlHwInit or from cpssTmInit
*       depends on initSystem type or tests running order.
*
*/
GT_STATUS prvCpssTmCtlDevInit
(
    IN GT_U8    devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  i;

    PRV_CPSS_GEN_TM_DB_STC  *tmInfoPtr;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
     CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    /* init tm PP tm info */
    tmInfoPtr = &PRV_CPSS_PP_MAC(devNum)->tmInfo;

    if (tmInfoPtr->tmDevIsInitilized == GT_TRUE)
        return rc;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.TmSupported == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(tmInfoPtr, 0, sizeof(PRV_CPSS_GEN_TM_DB_STC));

    /* convert from BC2 devFamily/SubFamily to internal TM Devs Ids */
    tmInfoPtr->prvCpssTmDevId = prvCpssTmGetInternalDevId(devNum);

    if (PRV_CPSS_PP_MAC(devNum)->tmInfo.prvCpssTmDevId == PRV_CPSS_TM_DEV_LAST_E)
    {
        rc = GT_NOT_SUPPORTED;
        cpssOsPrintf("%s %d  ERROR : rc=%d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }

    /* fill BAPs mapping by device Type and update tmLadParamsArr */
    for (i = 0; i < CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS ; i++)
    {
        tmInfoPtr->BapMapping[i] = 0xFF;
    }

    /*       BAP#            LAD#           BAP#            LAD# */
    tmInfoPtr->BapMapping[0] = 1; tmInfoPtr->BapMapping[5] = 1; /* BC2, Bobk-Caelum, Bobk-Cetus */
    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[1].bapIDs[0] = 0;
    PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[1].bapIDs[1] = 5;

    if (tmInfoPtr->prvCpssTmDevId == PRV_CPSS_TM_DEV_BOBK_CAELUM_E ||
        tmInfoPtr->prvCpssTmDevId == PRV_CPSS_TM_DEV_BC2_E)
    {

        tmInfoPtr->BapMapping[1] = 2; tmInfoPtr->BapMapping[6] = 2;
        tmInfoPtr->BapMapping[2] = 3; tmInfoPtr->BapMapping[7] = 3;
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[2].bapIDs[0] = 1;
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[2].bapIDs[1] = 6;
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[3].bapIDs[0] = 2;
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[3].bapIDs[1] = 7;

        if (tmInfoPtr->prvCpssTmDevId == PRV_CPSS_TM_DEV_BC2_E)
        {
            /*       BAP#            LAD#           BAP#            LAD# */
            tmInfoPtr->BapMapping[3] = 4; tmInfoPtr->BapMapping[8] = 4;
            tmInfoPtr->BapMapping[4] = 5; tmInfoPtr->BapMapping[9] = 5;
            PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[4].bapIDs[0] = 3;
            PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[4].bapIDs[1] = 8;
            PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[5].bapIDs[0] = 4;
            PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[5].bapIDs[1] = 9;
        }
    }

    /* init tm registers */
    init_tm_alias_struct();

    tmInfoPtr->tmDevIsInitilized = GT_TRUE;

    return rc;
}

/**
* @internal prvCpssTmCtlHwInit function
* @endinternal
*
* @brief   Initialize the TM HW configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] numOfLad                 - Nubmer of lads.
*                                      frequency            - support 667 or 933 MHZ.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_NO_RESOURCE           - on out of memory space.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
*/
GT_STATUS prvCpssTmCtlHwInit
(
    IN GT_U8                   devNum,
    IN GT_U8                   numOfLad
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U64      regData;
    GT_U64      regAddr;

    rc = prvCpssTmCtlDevInit(devNum);
    if (rc)
    {
        cpssOsPrintf("%s %d  ERROR :rc=%d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }

    rc= prvCpssTmCtlDramFrequencyInit(devNum, numOfLad);
    if (rc)
    {
        cpssOsPrintf("%s %d  ERROR :rc=%d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }


    rc= prvCpssTmCtlLadInit(devNum, numOfLad);
    if (rc)
    {
        cpssOsPrintf("%s %d  ERROR : rc=%d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }

    rc= prvCpssTmCtlGlobalParamsInit(devNum);
    if (rc)
    {
        cpssOsPrintf("%s %d  ERROR : rc=%d\n ",__FILE__, __LINE__ , rc);
        return rc;
    }

    CPSS_LOG_INFORMATION_MAC("\n ***********HW INIT*********** \n ");

    WAIT_FOR_DEVICE_MAC(10*SLEEP_MS);

    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.InitSnapshot;
    regData.l[1]= 0;
    rc = cpssTmCtlReadRegister(devNum, &regAddr, &regData);
    if (rc)
    {
        return rc;
    }
    /*cpssOsPrintf("PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.InitSnapshot: 0x%0x 0x%0x\n ",regData.l[0], regData.l[1]);*/
    CPSS_LOG_INFORMATION_MAC("PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.InitSnapshot: 0x%0x 0x%0x\n ",regData.l[0], regData.l[1]);

    if (PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTest) == GT_TRUE)
    {
        rc = prvCpssTmBapDramAccessTestRun(devNum, numOfLad);
        if (rc)
        {
            return rc;
        }
    }

    regData.l[0]= 1;
    regData.l[1]= 0;
    regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.StartTMExtMemInit;
    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &regData);
    if (rc)
    {
        return rc;
    }

   /*read_external_init_done_Phase_10*/

   regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.TMExtMemInitDone;

   for (i = 0; i < WAIT_LOOP; i++)
   {
        WAIT_FOR_DEVICE_MAC(SLEEP_MS);

        rc = cpssTmCtlReadRegister(devNum, &regAddr, &regData);
        if (rc)
        {
            return rc;
        }

        #ifdef ASIC_SIMULATION
            /* no need more loops */
            regData.l[0] |= 0x1;
        #endif /*ASIC_SIMULATION*/

        if (regData.l[0] & 0x1)
        {
            break;
        }
    }

   if (i == WAIT_LOOP)
   {
       rc = GT_BAD_STATE;
       if (PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTest) == GT_TRUE)
       {
           /*cpssOsPrintf("%s %d  rc=%d, skip tm external has not initDone!\n ",__FILE__, __LINE__ ,  rc);*/
                   CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"tm external init failed!");
       }
   }
   else
   {
       /*cpssOsPrintf("Tm external initDone successfully\n");*/
       CPSS_LOG_INFORMATION_MAC("Tm external initDone successfully\n");
   }


   if (PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(bapDramAccessTest) == GT_TRUE)
   {
       rc = prvCpssTmBapDramAccessTestRun(devNum, numOfLad);
       if (rc)
       {
           return rc;
       }
   }

   regAddr = PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).QMgr.InitSnapshot;
   regData.l[1]= 0;
   rc = cpssTmCtlReadRegister(devNum, &regAddr, &regData);
   if (rc)
   {
       return rc;
   }
   /*cpssOsPrintf("TM.QMgr.InitSnapshot: 0x%0x 0x%0x\n ",regData.l[0], regData.l[1]);*/
   CPSS_LOG_INFORMATION_MAC("TM.QMgr.InitSnapshot: 0x%0x 0x%0x\n ",regData.l[0], regData.l[1]);

   return rc;
}


/**
* @internal cpssTmCtlLadParamsSet function
* @endinternal
*
* @brief   Set TM LAD parameters to its DB when neccessary to overwrite
*         its default configuration, parameters are used from DB
*         at cpssTmCtlLadInit.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] numOfLads                - number of LADs.
* @param[in] ladParamsPtr             - (pointer of) CPSS_TM_CTL_LAD_INF_PARAM_STC.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssTmCtlLadParamsSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           numOfLads,
    IN  CPSS_TM_CTL_LAD_INF_PARAM_STC   *ladParamsPtr
)
{
    GT_STATUS rc = GT_OK;
    (void)devNum;
    /*
    function is relevant before cpssInitSystem, next checks are ignored:

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_CTL_CHECK_DRAM_INF_NUM_MAC(devNum, numOfLads);
    */

    CPSS_NULL_PTR_CHECK_MAC(ladParamsPtr);

    if (numOfLads > CPSS_TM_CTL_MAX_NUM_OF_LADS_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    if (ladParamsPtr->minPkgSize != 0)
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLads].minPkgSize = ladParamsPtr->minPkgSize;

    if (ladParamsPtr->pagesPerBank != 0)
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLads].pagesPerBank = ladParamsPtr->pagesPerBank;

    if (ladParamsPtr->pkgesPerBank != 0)
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLads].pkgesPerBank = ladParamsPtr->pkgesPerBank;

    if (ladParamsPtr->portChunksEmitPerSel != 0)
        PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLads].portChunksEmitPerSel = ladParamsPtr->portChunksEmitPerSel;

    return rc;
}


/**
* @internal cpssTmCtlLadParamsGet function
* @endinternal
*
* @brief   Get TM LAD parameters from its DB.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number.
* @param[in] numOfLads                - number of LADs.
* @param[in] ladParamsPtr             - (pointer of) CPSS_TM_CTL_LAD_INF_PARAM_STC.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong device or configuration parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssTmCtlLadParamsGet
(
    IN  GT_U8                         devNum,
    IN  GT_U8                         numOfLads,
    OUT CPSS_TM_CTL_LAD_INF_PARAM_STC *ladParamsPtr
)
{
    GT_STATUS   rc = GT_OK;
    (void)devNum;

    /*
    function is relevant before cpssInitSystem, next checks are ignored:

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
    CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    PRV_CPSS_TM_CTL_CHECK_DRAM_INF_NUM_MAC(devNum, numOfLads);
    */

    CPSS_NULL_PTR_CHECK_MAC(ladParamsPtr);

    if (numOfLads > CPSS_TM_CTL_MAX_NUM_OF_LADS_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);

    *ladParamsPtr = PRV_SHARED_GENERIC_TM_DIR_GLOBAL_VAR_GET(tmLadParamsArr)[numOfLads];
    return rc;
}


/**
* @internal internal_cpssTmUnitsErrorStatusGet function
* @endinternal
*
* @brief   Get TM Units Global Error Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] unitsErrorPtr            - pointer to CPSS_TM_UNITS_ERROR_STATUS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssTmUnitsErrorStatusGet
(
    IN GT_U8 devNum,
    OUT CPSS_TM_UNITS_ERROR_STATUS_STC *unitsErrorPtr
)
{
    GT_STATUS                           rc = GT_OK;
    GT_U32                              i;
    GT_U64 errStatusRegs[4];
    GT_U64 errStatusData[4];

    cpssOsMemSet(errStatusRegs, 0, sizeof(errStatusRegs));
    cpssOsMemSet(errStatusData, 0, sizeof(errStatusData));

    errStatusRegs[0].l[1] = 0x00000008; /* Qmgr  */
    errStatusRegs[1].l[1] = 0x00000010; /* Drop  */
    errStatusRegs[2].l[1] = 0x00000080; /* Sched */
    errStatusRegs[3].l[1] = 0x00000018; /* RCB   */

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(unitsErrorPtr);

    /*
    below remarked code works only if TM lib has ininialized
        struct tm_units_error_status_info   tmUnitsErrorinfo;
        int                                 ret;
        ret = tm_units_get_error_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, &tmUnitsErrorinfo);
        rc = XEL_TO_CPSS_ERR_CODE(ret);
        if (rc != GT_OK)
            return rc;

        unitsErrorPtr->qmngrStatus = tmUnitsErrorinfo.qmgr_status;
        unitsErrorPtr->dropStatus  = tmUnitsErrorinfo.drop_status;
        unitsErrorPtr->schedStatus = tmUnitsErrorinfo.sched_status;
        unitsErrorPtr->rcbStatus   = tmUnitsErrorinfo.rcb_status;

    */

    for (i = 0; i < 4; i++)
    {

        rc = cpssTmCtlReadRegister(devNum, &(errStatusRegs[i]), &errStatusData[i]);
        if (GT_OK != rc)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    unitsErrorPtr->qmngrStatus = errStatusData[0].l[0]; /* Qmgr  */
    unitsErrorPtr->dropStatus  = errStatusData[1].l[0]; /* Drop  */
    unitsErrorPtr->schedStatus = errStatusData[2].l[0]; /* Sched */
    unitsErrorPtr->rcbStatus   = errStatusData[3].l[0]; /* RCB   */

    return rc;
}

/**
* @internal cpssTmUnitsErrorStatusGet function
* @endinternal
*
* @brief   Get TM Units Global Error Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
*
* @param[out] unitsErrorPtr            - pointer to CPSS_TM_UNITS_ERROR_STATUS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssTmUnitsErrorStatusGet
(
    IN GT_U8 devNum,
    OUT CPSS_TM_UNITS_ERROR_STATUS_STC *unitsErrorPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmUnitsErrorStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, unitsErrorPtr));

    rc = internal_cpssTmUnitsErrorStatusGet(devNum, unitsErrorPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, unitsErrorPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}


/**
* @internal internal_cpssTmBapUnitErrorStatusGet function
* @endinternal
*
* @brief   Get TM BAP Unit Global Error Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bapNum                   - BAP interface number. (APPLICABLE RANGES: 0..CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS).
*
* @param[out] bapErrorsPtr             - pointer to CPSS_TM_BAP_UNIT_ERROR_STATUS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NO_RESOURCE           - on not exist BAP interface.
*/
static GT_STATUS internal_cpssTmBapUnitErrorStatusGet
(
    IN GT_U8 devNum,
    IN GT_U8 bapNum,
    OUT CPSS_TM_BAP_UNIT_ERROR_STATUS_STC *bapErrorsPtr
)
{
    GT_STATUS                               rc = GT_OK;
    PRV_CPSS_GEN_TM_DB_STC                  *tmInfoPtr;
    GT_U64 errStatusReg;
    GT_U64 errStatusData;

    PRV_CPSS_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    CPSS_NULL_PTR_CHECK_MAC(bapErrorsPtr);

    if (bapNum >= CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tmInfoPtr = &PRV_CPSS_PP_MAC(devNum)->tmInfo;
    CPSS_NULL_PTR_CHECK_MAC(tmInfoPtr);

    bapErrorsPtr->bapNum = bapNum;

    if (tmInfoPtr->BapMapping[bapNum] == 0xFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /*
    below remarked code works only if TM lib has ininialized

        struct tm_bap_unit_error_status_info    tmBapUnitErrorinfo;
        int                         ret;

        ret = tm_bap_unit_get_error_status(PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle, bapNum, &tmBapUnitErrorinfo);
        rc = XEL_TO_CPSS_ERR_CODE(ret);
        if (rc != GT_OK)
            return rc;

        bapErrorsPtr->bapStatus = tmBapUnitErrorinfo.bap_status;
    */

    cpssOsMemSet(&errStatusReg, 0, sizeof(errStatusReg));
    cpssOsMemSet(&errStatusData, 0, sizeof(errStatusData));

    errStatusReg.l[1] = 0x00000020 + (bapNum * 0x00000008);

    rc = cpssTmCtlReadRegister(devNum, &errStatusReg, &errStatusData);
    if (GT_OK != rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    bapErrorsPtr->bapStatus = errStatusData.l[0];

    return rc;
}

/**
* @internal cpssTmBapUnitErrorStatusGet function
* @endinternal
*
* @brief   Get TM BAP Unit Global Error Status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - Device number.
* @param[in] bapNum                   - BAP interface number. (APPLICABLE RANGES: 0..CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS).
*
* @param[out] bapErrorsPtr             - pointer to CPSS_TM_BAP_UNIT_ERROR_STATUS_STC.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error.
* @retval GT_BAD_PARAM             - on wrong device number or wrong parameter value.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_NO_RESOURCE           - on not exist BAP interface.
*/
GT_STATUS cpssTmBapUnitErrorStatusGet
(
    IN GT_U8 devNum,
    IN GT_U8 bapNum,
    OUT CPSS_TM_BAP_UNIT_ERROR_STATUS_STC *bapErrorsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssTmBapUnitErrorStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, bapNum, bapErrorsPtr));

    rc = internal_cpssTmBapUnitErrorStatusGet(devNum, bapNum, bapErrorsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, bapNum, bapErrorsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_PER_DEVICE_TRAFFIC_MANAGER_CNS);

    return rc;
}

