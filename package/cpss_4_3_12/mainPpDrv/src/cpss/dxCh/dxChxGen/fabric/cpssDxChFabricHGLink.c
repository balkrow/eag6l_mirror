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
* @file cpssDxChFabricHGLink.c
*
* @brief CPSS DxCh Fabric Connectivity HyperG.Link interface API.
*
* @version   11
********************************************************************************
*/

/* macro needed to support the call to PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC */
/* this define must come before include files */
#define PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_SUPPORTED_FLAG_CNS
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/fabric/private/prvCpssDxChFabricLog.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/fabric/cpssDxChFabricHGLink.h>

/* get CPSS definitions for private port configurations */
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPort.h>

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

static GT_STATUS prvCpssDxChFabricHGLinkSupported
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              *supportedPtr
)
{
    return cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum,
                                                CPSS_PORT_INTERFACE_MODE_HGL_E,
                                                /* not important what speed is */
                                                CPSS_PORT_SPEED_15000_E,
                                                supportedPtr);
}

#define HGL_SUPPORT_CHECK_MAC(_dev, _port)  \
    {                                       \
        GT_BOOL     hglSupported;           \
        rc = prvCpssDxChFabricHGLinkSupported(_dev,_port,&hglSupported);    \
        if(rc != GT_OK)                     \
        {                                   \
            return rc;                      \
        }                                   \
        if(GT_FALSE == hglSupported)        \
        {                                   \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);        \
        }                                   \
    }

/**
* @internal internal_cpssDxChFabricHGLinkPcsLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/disable PCS loopback on Hyper.G Link
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  -  loopback
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note If loopback enbaled PCS Rx is connected directly to PCS Tx, bypassing
*       the SERDES Interface.
*
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsLoopbackEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */
    GT_PHYSICAL_PORT_NUM localPort; /* number of port in local core */
    MV_HWS_PORT_LB_TYPE lbType; /* port's loopback type */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);
    CPSS_COVERITY_NON_ISSUE_BOOKMARK

    lbType = (enable) ? TX_2_RX_LB : DISABLE_LB;

    return mvHwsPcsLoopbackCfg(devNum, portGroupId, localPort, HGL, HGLPCS, lbType);
}

/**
* @internal cpssDxChFabricHGLinkPcsLoopbackEnableSet function
* @endinternal
*
* @brief   Enable/disable PCS loopback on Hyper.G Link
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  -  loopback
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note If loopback enbaled PCS Rx is connected directly to PCS Tx, bypassing
*       the SERDES Interface.
*
*/
GT_STATUS cpssDxChFabricHGLinkPcsLoopbackEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsLoopbackEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChFabricHGLinkPcsLoopbackEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkPcsLoopbackEnableGet function
* @endinternal
*
* @brief   Get PCS loopback on Hyper.G Link status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - loopback enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note If loopback enbaled PCS Rx is connected directly to PCS Tx, bypassing
*       the SERDES Interface.
*
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsLoopbackEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    CPSS_TBD_BOOKMARK_LION2 /* support in hwServices needed */

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].xgGlobalConfReg1;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                1, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkPcsLoopbackEnableGet function
* @endinternal
*
* @brief   Get PCS loopback on Hyper.G Link status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - loopback enabled
*                                      GT_FALSE - disabled
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - enablePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note If loopback enbaled PCS Rx is connected directly to PCS Tx, bypassing
*       the SERDES Interface.
*
*/
GT_STATUS cpssDxChFabricHGLinkPcsLoopbackEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsLoopbackEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChFabricHGLinkPcsLoopbackEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet function
* @endinternal
*
* @brief   Enable/Disable the Idle counter in the Tx SM. When Idle counter is
*         disabled, it is the MACs responsibilty to generate idles.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] idleCntEnable            - GT_TRUE  - idle count enable
*                                      GT_FALSE - idle count disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              idleCntEnable
)
{
    GT_STATUS   rc; /* return code */
    GT_BOOL     xpcsResetState; /* current status of XPCS reset */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */
    GT_PHYSICAL_PORT_NUM localPort; /* number of port in local core */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    rc = prvCpssDxChPortXgPcsResetStateGet(devNum,portNum,&xpcsResetState);
    if(rc != GT_OK)
        return rc;

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(!xpcsResetState)
    {/* reset XPCS if needed */
        CPSS_COVERITY_NON_ISSUE_BOOKMARK

        rc = mvHwsPcsReset (devNum,portGroupId,localPort,HGL,HGLPCS,RESET);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* configure idleCntEnable */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].xgGlobalConfReg0;
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                            14, 1, BOOL2BIT_MAC(idleCntEnable));
    if(rc != GT_OK)
    {
        return rc;
    }

    if(!xpcsResetState)
    {/* unreset XPCS */
        rc = mvHwsPcsReset (devNum,portGroupId,localPort,HGL,HGLPCS,UNRESET);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet function
* @endinternal
*
* @brief   Enable/Disable the Idle counter in the Tx SM. When Idle counter is
*         disabled, it is the MACs responsibilty to generate idles.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] idleCntEnable            - GT_TRUE  - idle count enable
*                                      GT_FALSE - idle count disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              idleCntEnable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, idleCntEnable));

    rc = internal_cpssDxChFabricHGLinkPcsMaxIdleCntEnableSet(devNum, portNum, idleCntEnable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, idleCntEnable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of the Idle counter in the Tx SM. When Idle
*         counter is disabled, it is the MACs responsibilty to generate idles.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] idleCntEnablePtr         - GT_TRUE  - idle count enable
*                                      GT_FALSE - idle count disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - idleCntEnablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *idleCntEnablePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(idleCntEnablePtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].xgGlobalConfReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                14, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *idleCntEnablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet function
* @endinternal
*
* @brief   Get enable/disable status of the Idle counter in the Tx SM. When Idle
*         counter is disabled, it is the MACs responsibilty to generate idles.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] idleCntEnablePtr         - GT_TRUE  - idle count enable
*                                      GT_FALSE - idle count disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_BAD_PTR               - idleCntEnablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *idleCntEnablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, idleCntEnablePtr));

    rc = internal_cpssDxChFabricHGLinkPcsMaxIdleCntEnableGet(devNum, portNum, idleCntEnablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, idleCntEnablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkPcsMaxIdleCntSet function
* @endinternal
*
* @brief   Configure the number of cycles between ||k|| sequences.
*         ||K|| sequence must be sent every programmable
*         number of cycles in order to keep a sufficient
*         frequency of commas for each lane and maintain
*         code group alignment.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] maxIdleCnt               - number of cycles between ||k|| sequences,
*                                      (APPLICABLE RANGES: 0..32768)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - if maxIdleCnt out of range
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsMaxIdleCntSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               maxIdleCnt
)
{
    GT_STATUS   rc; /* return code */
    GT_BOOL     xpcsResetState; /* current status of XPCS reset */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */
    GT_PHYSICAL_PORT_NUM localPort; /* number of port in local core */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    if(BIT_17 <= maxIdleCnt)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    rc = prvCpssDxChPortXgPcsResetStateGet(devNum,portNum,&xpcsResetState);
    if(rc != GT_OK)
        return rc;

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    localPort = PRV_CPSS_GLOBAL_PORT_TO_LOCAL_PORT_CONVERT_MAC(devNum,portNum);

    if(!xpcsResetState)
    {
        /* reset XPCS */
        CPSS_COVERITY_NON_ISSUE_BOOKMARK

        rc = mvHwsPcsReset (devNum,portGroupId,localPort,HGL,HGLPCS,RESET);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* configure MAX idle count value */
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].xpcsGlobalMaxIdleCounter;
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                0, 16, maxIdleCnt);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(!xpcsResetState)
    {
        /* unreset XPCS */
        rc = mvHwsPcsReset(devNum,portGroupId,localPort,HGL,HGLPCS,UNRESET);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntSet function
* @endinternal
*
* @brief   Configure the number of cycles between ||k|| sequences.
*         ||K|| sequence must be sent every programmable
*         number of cycles in order to keep a sufficient
*         frequency of commas for each lane and maintain
*         code group alignment.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] maxIdleCnt               - number of cycles between ||k|| sequences,
*                                      (APPLICABLE RANGES: 0..32768)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_OUT_OF_RANGE          - if maxIdleCnt out of range
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_U32               maxIdleCnt
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsMaxIdleCntSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, maxIdleCnt));

    rc = internal_cpssDxChFabricHGLinkPcsMaxIdleCntSet(devNum, portNum, maxIdleCnt);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, maxIdleCnt));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkPcsMaxIdleCntGet function
* @endinternal
*
* @brief   Get the number of cycles between ||k|| sequences.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] maxIdleCntPtr            - number of cycles between ||k|| sequences
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if maxIdleCntPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsMaxIdleCntGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *maxIdleCntPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(maxIdleCntPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].xpcsGlobalMaxIdleCounter;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 16, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *maxIdleCntPtr = value;

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkPcsMaxIdleCntGet function
* @endinternal
*
* @brief   Get the number of cycles between ||k|| sequences.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] maxIdleCntPtr            - number of cycles between ||k|| sequences
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if maxIdleCntPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsMaxIdleCntGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *maxIdleCntPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsMaxIdleCntGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, maxIdleCntPtr));

    rc = internal_cpssDxChFabricHGLinkPcsMaxIdleCntGet(devNum, portNum, maxIdleCntPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, maxIdleCntPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkPcsRxStatusGet function
* @endinternal
*
* @brief   Get the XPCS lanes synchronization status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS] - array of statuses of lanes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if laneSyncOkArr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkPcsRxStatusGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS]
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */
    GT_U32      i; /* iterator */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(laneSyncOkArr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    for(i = 0; i < CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS; i++)
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                      macRegs.perPortRegs[portNum].laneStatus[i];

        if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                    4, 1, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        laneSyncOkArr[i] = BIT2BOOL_MAC(value);
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkPcsRxStatusGet function
* @endinternal
*
* @brief   Get the XPCS lanes synchronization status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS] - array of statuses of lanes
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if laneSyncOkArr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkPcsRxStatusGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              laneSyncOkArr[CPSS_DXCH_FABRIC_HGL_SERDES_LANES_NUM_CNS]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkPcsRxStatusGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, laneSyncOkArr));

    rc = internal_cpssDxChFabricHGLinkPcsRxStatusGet(devNum, portNum, laneSyncOkArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, laneSyncOkArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxErrorCntrGet function
* @endinternal
*
* @brief   Get the XPCS lanes synchronization status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxErrorsCntrsPtr         - array of statuses of lanes (6 in Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if rxErrorsCntrsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxErrorCntrGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC *rxErrorsCntrsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(rxErrorsCntrsPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacDroppedRxCellCntrsReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                             &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    rxErrorsCntrsPtr->reformatErrorCells = U32_GET_FIELD_MAC(value, 0, 8);
    rxErrorsCntrsPtr->badHeaderCells = U32_GET_FIELD_MAC(value, 8, 8);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacDroppedRxCellCntrsReg1;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 8, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    rxErrorsCntrsPtr->badLengthCells = value;

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxErrorCntrGet function
* @endinternal
*
* @brief   Get the XPCS lanes synchronization status
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] rxErrorsCntrsPtr         - array of statuses of lanes (6 in Lion2)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if rxErrorsCntrsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxErrorCntrGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT CPSS_DXCH_FABRIC_HGL_RX_ERROR_CNTRS_STC *rxErrorsCntrsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxErrorCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, rxErrorsCntrsPtr));

    rc = internal_cpssDxChFabricHGLinkRxErrorCntrGet(devNum, portNum, rxErrorsCntrsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, rxErrorsCntrsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkConfigEccTypeSet function
* @endinternal
*
* @brief   This routine defines how to calculate
*         the error correction on the HyperG.Link cell
*         Both Rx and Tx.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] eccType                  - the Error Correction Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number,device,eccType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkConfigEccTypeSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT eccType
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    if (eccType > CPSS_DXCH_FABRIC_HGL_ECC_CRC8_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                1, 1, eccType);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg1;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                15, 1, eccType);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkConfigEccTypeSet function
* @endinternal
*
* @brief   This routine defines how to calculate
*         the error correction on the HyperG.Link cell
*         Both Rx and Tx.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] eccType                  - the Error Correction Type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number,device,eccType
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkConfigEccTypeSet
(
    IN GT_U8                 devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT eccType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkConfigEccTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, eccType));

    rc = internal_cpssDxChFabricHGLinkConfigEccTypeSet(devNum, portNum, eccType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, eccType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkConfigEccTypeGet function
* @endinternal
*
* @brief   Get current type of the error correction on the HyperG.Link cell
*         Both Rx and Tx.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] eccTypePtr               - current Error Correction Type of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number,device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if eccTypePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkConfigEccTypeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT *eccTypePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(eccTypePtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                1, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *eccTypePtr = (CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT)value;

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkConfigEccTypeGet function
* @endinternal
*
* @brief   Get current type of the error correction on the HyperG.Link cell
*         Both Rx and Tx.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] eccTypePtr               - current Error Correction Type of port
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number,device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if eccTypePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkConfigEccTypeGet
(
    IN  GT_U8                 devNum,
    IN  GT_PHYSICAL_PORT_NUM  portNum,
    OUT CPSS_DXCH_FABRIC_HGL_ECC_TYPE_ENT *eccTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkConfigEccTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, eccTypePtr));

    rc = internal_cpssDxChFabricHGLinkConfigEccTypeGet(devNum, portNum, eccTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, eccTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable CRC check of received cells.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  -  CRC check
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxCrcCheckEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                0, 1, BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxCrcCheckEnableSet function
* @endinternal
*
* @brief   Enable/disable CRC check of received cells.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  -  CRC check
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxCrcCheckEnableSet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    IN  GT_BOOL              enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxCrcCheckEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChFabricHGLinkRxCrcCheckEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxCrcCheckEnableGet function
* @endinternal
*
* @brief   Get status of CRC check of received cells.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - enable CRC check
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - if enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxCrcCheckEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value;/* value of field */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxCrcCheckEnableGet function
* @endinternal
*
* @brief   Get status of CRC check of received cells.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - enable CRC check
*                                      GT_FALSE - disable
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - if enablePtr is NULL
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxCrcCheckEnableGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL              *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxCrcCheckEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChFabricHGLinkRxCrcCheckEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkLbiEnableSet function
* @endinternal
*
* @brief   Defines if the forwarding is affected from the Load Balancing Index (LBI)
*         field in the cell header
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - The target is defined according to the Target
*                                      Device and LBI fields in the cell header
*                                      GT_FALSE - The target is defined only according to the Target
*                                      Device field in the cell header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkLbiEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                2, 1, BOOL2BIT_MAC(enable));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkLbiEnableSet function
* @endinternal
*
* @brief   Defines if the forwarding is affected from the Load Balancing Index (LBI)
*         field in the cell header
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] enable                   - GT_TRUE  - The target is defined according to the Target
*                                      Device and LBI fields in the cell header
*                                      GT_FALSE - The target is defined only according to the Target
*                                      Device field in the cell header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkLbiEnableSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_BOOL                 enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkLbiEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enable));

    rc = internal_cpssDxChFabricHGLinkLbiEnableSet(devNum, portNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkLbiEnableGet function
* @endinternal
*
* @brief   Defines if the forwarding is affected from the Load Balancing Index (LBI)
*         field in the cell header
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - The target is defined according to the Target
*                                      Device and LBI fields in the cell header
*                                      GT_FALSE - The target is defined only according to the Target
*                                      Device field in the cell header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if enablePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkLbiEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                2, 1, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(value);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkLbiEnableGet function
* @endinternal
*
* @brief   Defines if the forwarding is affected from the Load Balancing Index (LBI)
*         field in the cell header
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] enablePtr                - GT_TRUE  - The target is defined according to the Target
*                                      Device and LBI fields in the cell header
*                                      GT_FALSE - The target is defined only according to the Target
*                                      Device field in the cell header
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if enablePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkLbiEnableGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT GT_BOOL                 *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkLbiEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, enablePtr));

    rc = internal_cpssDxChFabricHGLinkLbiEnableGet(devNum, portNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkCellsCntrTypeSet function
* @endinternal
*
* @brief   Configure the types of received/transmitted cells to be
*         counted in the received/transmitted cell counter
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
* @param[in] cellType                 - counted cells type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkCellsCntrTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    IN  CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT  cellType
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      offset; /* filed offset in register */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    if (cellDirection>CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if (cellType>CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_OTHER_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacCellCounterTypeReg;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    offset = (cellDirection == CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E) ? 0 : 4;
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                offset, 4, cellType);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkCellsCntrTypeSet function
* @endinternal
*
* @brief   Configure the types of received/transmitted cells to be
*         counted in the received/transmitted cell counter
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
* @param[in] cellType                 - counted cells type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkCellsCntrTypeSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    IN  CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT  cellType
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkCellsCntrTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cellDirection, cellType));

    rc = internal_cpssDxChFabricHGLinkCellsCntrTypeSet(devNum, portNum, cellDirection, cellType);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cellDirection, cellType));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkCellsCntrTypeGet function
* @endinternal
*
* @brief   Get the types of received/transmitted cells
*         counted in the received/transmitted cell counter
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
*
* @param[out] cellTypePtr              - current counted cells type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - cellTypePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkCellsCntrTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    OUT CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT  *cellTypePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      offset; /* field offset in register */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(cellTypePtr);

    if (cellDirection > CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacCellCounterTypeReg;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);
    offset = (cellDirection == CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E) ? 0 : 4;
    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                offset, 4, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    *cellTypePtr = (CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT)value;

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkCellsCntrTypeGet function
* @endinternal
*
* @brief   Get the types of received/transmitted cells
*         counted in the received/transmitted cell counter
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
*
* @param[out] cellTypePtr              - current counted cells type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - cellTypePtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkCellsCntrTypeGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    OUT CPSS_DXCH_FABRIC_HGL_CELL_COUNTER_TYPE_ENT  *cellTypePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkCellsCntrTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cellDirection, cellTypePtr));

    rc = internal_cpssDxChFabricHGLinkCellsCntrTypeGet(devNum, portNum, cellDirection, cellTypePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cellDirection, cellTypePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkCellsCntrGet function
* @endinternal
*
* @brief   Get the counter of received/transmitted cells
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
*
* @param[out] cellCntrPtr              - counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - cellCntrPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkCellsCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    OUT GT_U32                              *cellCntrPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;/* address of register */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(cellCntrPtr);

    if((cellDirection != CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E) &&
       (cellDirection != CPSS_DXCH_FABRIC_HGL_DIRECTION_TX_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacRxCellCounterReg;
    /* Check for valid port number*/
    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(cellDirection == CPSS_DXCH_FABRIC_HGL_DIRECTION_RX_E)
    {
        GT_U32      value; /* field value */
        GT_U32      portGroupId; /* number of ports group for multi-port-group
                                    devices */

        portGroupId =
            PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                        portNum);
        rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                    0, 16, &value);
        if(rc != GT_OK)
        {
            return rc;
        }

        *cellCntrPtr = value;
    }
    else
    {
        GT_U64 cntrValue;

        rc = cpssDxChMacCounterGet(devNum, portNum,
                                    /* offset 68 bytes in MIB counters table */
                                    CPSS_EXCESSIVE_COLLISIONS_E,
                                    &cntrValue);

        *cellCntrPtr = cntrValue.l[0];
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkCellsCntrGet function
* @endinternal
*
* @brief   Get the counter of received/transmitted cells
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] cellDirection            - cells direction
*
* @param[out] cellCntrPtr              - counter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - cellCntrPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkCellsCntrGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DIRECTION_ENT  cellDirection,
    OUT GT_U32                              *cellCntrPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkCellsCntrGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, cellDirection, cellCntrPtr));

    rc = internal_cpssDxChFabricHGLinkCellsCntrGet(devNum, portNum, cellDirection, cellCntrPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, cellDirection, cellCntrPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkFcMacSaSet function
* @endinternal
*
* @brief   Configure the MAC-SA of LL-FC and E2E constructed packets
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macPtr                   - new source mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if macPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkFcMacSaSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_ETHERADDR            *macPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      middle16Bit; /* bytes 2 and 3 of MAC SA */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacSaReg[0];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                GT_HW_MAC_HIGH16(macPtr));
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacSaReg[1];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    middle16Bit = ((macPtr)->arEther[3] | ((macPtr)->arEther[2] << 8));
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                middle16Bit);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacSaReg[2];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                GT_HW_MAC_LOW16(macPtr));
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkFcMacSaSet function
* @endinternal
*
* @brief   Configure the MAC-SA of LL-FC and E2E constructed packets
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] macPtr                   - new source mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if macPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcMacSaSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_ETHERADDR            *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkFcMacSaSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macPtr));

    rc = internal_cpssDxChFabricHGLinkFcMacSaSet(devNum, portNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkFcMacSaGet function
* @endinternal
*
* @brief   Configure the MAC-SA of LL-FC and E2E constructed packets
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macPtr                   - source mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if macPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkFcMacSaGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_ETHERADDR         *macPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(macPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacSaReg[0];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    macPtr->arEther[0] = (GT_U8)U32_GET_FIELD_MAC(value, 8, 8);
    macPtr->arEther[1] = (GT_U8)U32_GET_FIELD_MAC(value, 0, 8);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacSaReg[1];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    macPtr->arEther[2] = (GT_U8)U32_GET_FIELD_MAC(value, 8, 8);
    macPtr->arEther[3] = (GT_U8)U32_GET_FIELD_MAC(value, 0, 8);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacSaReg[2];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                               &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    macPtr->arEther[4] = (GT_U8)U32_GET_FIELD_MAC(value, 8, 8);
    macPtr->arEther[5] = (GT_U8)U32_GET_FIELD_MAC(value, 0, 8);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkFcMacSaGet function
* @endinternal
*
* @brief   Configure the MAC-SA of LL-FC and E2E constructed packets
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] macPtr                   - source mac address
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if macPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcMacSaGet
(
    IN  GT_U8                devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_ETHERADDR         *macPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkFcMacSaGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, macPtr));

    rc = internal_cpssDxChFabricHGLinkFcMacSaGet(devNum, portNum, macPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, macPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxDsaParamsSet function
* @endinternal
*
* @brief   Configure some Fabric RX DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] dsaParamsPtr             - DSA parameters for incomming cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if dsaParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note Also used for E2E-FC and LL-FC DSA Tag parameters configuration
*
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxDsaParamsSet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  CPSS_DXCH_FABRIC_HGL_RX_DSA_STC  *dsaParamsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(dsaParamsPtr);
    if ((dsaParamsPtr->vid>=BIT_12) || (dsaParamsPtr->srcId>=BIT_5))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = U32_GET_FIELD_MAC(dsaParamsPtr->vid, 6, 6);
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                10, 6, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg1;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = U32_GET_FIELD_MAC(dsaParamsPtr->vid, 0, 6) | (dsaParamsPtr->srcId << 6);
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                0, 11, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxDsaParamsSet function
* @endinternal
*
* @brief   Configure some Fabric RX DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] dsaParamsPtr             - DSA parameters for incomming cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if dsaParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*
* @note Also used for E2E-FC and LL-FC DSA Tag parameters configuration
*
*/
GT_STATUS cpssDxChFabricHGLinkRxDsaParamsSet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    IN  CPSS_DXCH_FABRIC_HGL_RX_DSA_STC  *dsaParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxDsaParamsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, dsaParamsPtr));

    rc = internal_cpssDxChFabricHGLinkRxDsaParamsSet(devNum, portNum, dsaParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, dsaParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxDsaParamsGet function
* @endinternal
*
* @brief   Get some configurable Fabric RX DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] dsaParamsPtr             - DSA parameters for incomming cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if dsaParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxDsaParamsGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_DXCH_FABRIC_HGL_RX_DSA_STC  *dsaParamsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(dsaParamsPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                10, 6, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    dsaParamsPtr->vid = value << 6;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg1;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 11, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    dsaParamsPtr->vid |= U32_GET_FIELD_MAC(value, 0, 6);
    dsaParamsPtr->srcId = U32_GET_FIELD_MAC(value, 6, 5);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxDsaParamsGet function
* @endinternal
*
* @brief   Get some configurable Fabric RX DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] dsaParamsPtr             - DSA parameters for incomming cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if dsaParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxDsaParamsGet
(
    IN  GT_U8                            devNum,
    IN  GT_PHYSICAL_PORT_NUM             portNum,
    OUT CPSS_DXCH_FABRIC_HGL_RX_DSA_STC  *dsaParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxDsaParamsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, dsaParamsPtr));

    rc = internal_cpssDxChFabricHGLinkRxDsaParamsGet(devNum, portNum, dsaParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, dsaParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkFcRxE2eParamsSet function
* @endinternal
*
* @brief   Configure some Fabric FC RX E2E header fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] e2eParamsPtr             - parameters for incomming E2E FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if e2eParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkFcRxE2eParamsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *e2eParamsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(e2eParamsPtr);
    if ((e2eParamsPtr->prioVlanId>=BIT_12) || (e2eParamsPtr->version>=BIT_4))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[0];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                e2eParamsPtr->etherType);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[1];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                e2eParamsPtr->prioVlanId);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[2];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                U32_GET_FIELD_MAC(e2eParamsPtr->cpId, 16, 16));
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[3];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                U32_GET_FIELD_MAC(e2eParamsPtr->cpId, 0, 16));
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[4];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = e2eParamsPtr->macLowByte | (e2eParamsPtr->version << 9);
    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkFcRxE2eParamsSet function
* @endinternal
*
* @brief   Configure some Fabric FC RX E2E header fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] e2eParamsPtr             - parameters for incomming E2E FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if e2eParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcRxE2eParamsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *e2eParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkFcRxE2eParamsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, e2eParamsPtr));

    rc = internal_cpssDxChFabricHGLinkFcRxE2eParamsSet(devNum, portNum, e2eParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, e2eParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkFcRxE2eParamsGet function
* @endinternal
*
* @brief   Get some Fabric FC RX E2E header fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] e2eParamsPtr             - parameters for incomming E2E FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if e2eParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkFcRxE2eParamsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *e2eParamsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(e2eParamsPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[0];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    e2eParamsPtr->etherType = value;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[1];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    e2eParamsPtr->prioVlanId = value;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[2];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    e2eParamsPtr->cpId = value << 16;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[3];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    e2eParamsPtr->cpId |= value;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacE2eConfigReg[4];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    e2eParamsPtr->macLowByte = (GT_U8)U32_GET_FIELD_MAC(value, 0, 8);
    e2eParamsPtr->version = U32_GET_FIELD_MAC(value, 9, 4);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkFcRxE2eParamsGet function
* @endinternal
*
* @brief   Get some Fabric FC RX E2E header fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] e2eParamsPtr             - parameters for incomming E2E FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if e2eParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkFcRxE2eParamsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  CPSS_DXCH_FABRIC_HGL_FC_RX_E2E_HEADER_STC *e2eParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkFcRxE2eParamsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, e2eParamsPtr));

    rc = internal_cpssDxChFabricHGLinkFcRxE2eParamsGet(devNum, portNum, e2eParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, e2eParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxLinkLayerFcParamsSet function
* @endinternal
*
* @brief   Configure some Fabric Link Layer FC RX packet and DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] llParamsPtr              - parameters for incomming Link Layer FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if llParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxLinkLayerFcParamsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *llParamsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(llParamsPtr);
  /*  PRV_CPSS_DXCH_QOS_PROFILE_ID_CHECK_MAC(devNum, llParamsPtr->qosProfile);*/
    if ((llParamsPtr->pfcOpcode >= BIT_16) || (llParamsPtr->trgDev >= BIT_5)
        ||(llParamsPtr->trgPort >= BIT_6) || (llParamsPtr->up >= BIT_3) || (llParamsPtr->qosProfile >= BIT_7))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[3];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                0, 16, llParamsPtr->etherType);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[1];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupWriteRegister(devNum, portGroupId, regAddr,
                                                llParamsPtr->pfcOpcode);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[2];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = (llParamsPtr->up << 6) | llParamsPtr->trgPort;
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                0, 9, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[0];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    value = (llParamsPtr->trgDev << 7) | llParamsPtr->qosProfile;
    rc = prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr,
                                                1, 12, value);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxLinkLayerFcParamsSet function
* @endinternal
*
* @brief   Configure some Fabric Link Layer FC RX packet and DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] llParamsPtr              - parameters for incomming Link Layer FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if llParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxLinkLayerFcParamsSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *llParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxLinkLayerFcParamsSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, llParamsPtr));

    rc = internal_cpssDxChFabricHGLinkRxLinkLayerFcParamsSet(devNum, portNum, llParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, llParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChFabricHGLinkRxLinkLayerFcParamsGet function
* @endinternal
*
* @brief   Get configurable Fabric Link Layer FC RX packet and DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] llParamsPtr              - parameters for incomming Link Layer FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if llParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkRxLinkLayerFcParamsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *llParamsPtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      value; /* field value */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);
    CPSS_NULL_PTR_CHECK_MAC(llParamsPtr);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[3];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 16, &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    llParamsPtr->etherType = value;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[1];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupReadRegister(devNum, portGroupId, regAddr,
                                                &value);
    if(rc != GT_OK)
    {
        return rc;
    }
    llParamsPtr->pfcOpcode = value;

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[2];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                0, 9, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    llParamsPtr->up = U32_GET_FIELD_MAC(value, 6, 4);
    llParamsPtr->trgPort = U32_GET_FIELD_MAC(value, 0, 6);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglMacLLFCConfigReg[0];

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr,
                                                1, 12, &value);
    if(rc != GT_OK)
    {
        return rc;
    }

    llParamsPtr->trgDev = (GT_U8)U32_GET_FIELD_MAC(value, 7, 5);
    llParamsPtr->qosProfile = U32_GET_FIELD_MAC(value, 0, 7);

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkRxLinkLayerFcParamsGet function
* @endinternal
*
* @brief   Get configurable Fabric Link Layer FC RX packet and DSA tag fields
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] llParamsPtr              - parameters for incomming Link Layer FC cells
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - if llParamsPtr is NULL
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkRxLinkLayerFcParamsGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT CPSS_DXCH_FABRIC_HGL_FC_RX_LINK_LAYER_PARAMS_STC *llParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkRxLinkLayerFcParamsGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, llParamsPtr));

    rc = internal_cpssDxChFabricHGLinkRxLinkLayerFcParamsGet(devNum, portNum, llParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, llParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChFabricHGLinkDescriptorTypeSet function
* @endinternal
*
* @brief   Configure the descriptor type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - descriptor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkDescriptorTypeSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT   type
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */
    GT_U32      regValue;/* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    regValue = (type == CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_20B_E) ? 0 : 1;

    return prvCpssDrvHwPpPortGroupSetRegField(devNum, portGroupId, regAddr, 1, 1, regValue);
}

/**
* @internal cpssDxChFabricHGLinkDescriptorTypeSet function
* @endinternal
*
* @brief   Configure the descriptor type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] type                     - descriptor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkDescriptorTypeSet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    IN  CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT   type
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkDescriptorTypeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, type));

    rc = internal_cpssDxChFabricHGLinkDescriptorTypeSet(devNum, portNum, type);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, type));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChFabricHGLinkDescriptorTypeGet function
* @endinternal
*
* @brief   Gets the descriptor type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] typePtr                  - (pointer to) descriptor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
static GT_STATUS internal_cpssDxChFabricHGLinkDescriptorTypeGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    OUT CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT   *typePtr
)
{
    GT_STATUS   rc; /* return code */
    GT_U32      regAddr;/* address of register */
    GT_U32      portGroupId; /* number of ports group for multi-port-group
                                devices */
    GT_U32      regValue;/* register value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E |
                                          CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PHY_PORT_CHECK_MAC(devNum,portNum);

    HGL_SUPPORT_CHECK_MAC(devNum, portNum);

    regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                  macRegs.perPortRegs[portNum].hglGlobalConfigReg0;

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED == regAddr)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    portGroupId =
        PRV_CPSS_PHYSICAL_GLOBAL_PORT_TO_PORT_GROUP_ID_CONVERT_MAC(devNum,
                                                                    portNum);

    rc = prvCpssDrvHwPpPortGroupGetRegField(devNum, portGroupId, regAddr, 1, 1, &regValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    *typePtr = (regValue == 0) ? CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_20B_E : CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_24B_E;

    return GT_OK;
}

/**
* @internal cpssDxChFabricHGLinkDescriptorTypeGet function
* @endinternal
*
* @brief   Gets the descriptor type.
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
*
* @param[out] typePtr                  - (pointer to) descriptor type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - wrong device family
*/
GT_STATUS cpssDxChFabricHGLinkDescriptorTypeGet
(
    IN  GT_U8                                      devNum,
    IN  GT_PHYSICAL_PORT_NUM                       portNum,
    OUT CPSS_DXCH_FABRIC_HGL_DESCRIPTOR_TYPE_ENT   *typePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChFabricHGLinkDescriptorTypeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portNum, typePtr));

    rc = internal_cpssDxChFabricHGLinkDescriptorTypeGet(devNum, portNum, typePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portNum, typePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


