/*******************************************************************************
*            Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* mvHwsMti100MacIf_SIP7.c
*
* DESCRIPTION: MTI 100G MAC
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#ifndef MICRO_INIT
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#else
#include <cpssCommon/cpssPresteraDefs.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>


static char* mvHwsMacTypeGet(void)
{
  return "MTI100_MAC";
}

/**
* @internal mvHwsMti100Mac_SIP7ModeCfg function
* @endinternal
*
* @brief   Set the MTI100 MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7ModeCfg
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_STATUS                   rc;
    GT_U32                      markerInterval;
    GT_U32                      macField;
    GT_U32                      regValue;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_PORT_FEC_MODE        portFecMode;
    MV_HWS_PORT_MII_TYPE_E      miiType;
    GT_BOOL                     isPreemptionEnabled = GT_FALSE;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    rc = mvHwsAasUnitMarkerIntervalCalc(devNum, portMode, portFecMode, 1, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }
    if (attributesPtr != NULL)
    {
        isPreemptionEnabled = attributesPtr->preemptionEnable;
    }

    macField = AAS_MAC100_TX_IPG_LENGTH_COMPENSATION_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, markerInterval, NULL));

    macField = AAS_MAC100_EMAC_COMMAND_CONFIG_CNTL_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

    macField = AAS_MAC100_COMMAND_CONFIG_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

    macField = AAS_MAC100_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));

    /* EMAC TX_SECTION_FULL */
    macField = AAS_MAC100_EMAC_TX_FIFO_SECTIONS_TX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 11, NULL));

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _100GMII_E)
    {
        hwsOsPrintf("mvHwsMti100Mac_SIP7ModeCfg: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    /* Common XGMII */
    regValue = ((miiType == _GMII_E) || (miiType == _5GMII_E) || (miiType == _10GMII_E) || (miiType == _25GMII_E)) ? 1 : 0;
    macField = AAS_MAC100_XIF_MODE_XGMII_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, regValue, NULL));

    /* If preemption MAC feature is enabled */
    if(isPreemptionEnabled == GT_TRUE)
    {
        /* PMAC CNTL_FRAME_ENA */
        macField = AAS_MAC100_COMMAND_CONFIG_CNTL_FRAME_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 1, NULL));

        /* PMAC TX_SECTION_FULL */
        macField = AAS_MAC100_PMAC_TX_FIFO_SECTIONS_TX_SECTION_FULL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 11, NULL));

        /* Preemption MAC enable */
        macField = AAS_MAC100_BR_CONTROL_TX_PREEMPT_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

        macField = AAS_MAC100_BR_CONTROL_BR_RX_SMD_DIS_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));

        macField = AAS_MAC100_BR_CONTROL_RX_STRICT_PREAMBLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

        /* When set to 0, PMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = AAS_MAC100_BR_CONTROL_TX_ALLOW_PMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

        /* When set to 0, EMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = AAS_MAC100_BR_CONTROL_TX_ALLOW_EMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));
    }
    else
    {
        /* Preemption MAC disable */
        macField = AAS_MAC100_BR_CONTROL_TX_PREEMPT_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));

        macField = AAS_MAC100_BR_CONTROL_BR_RX_SMD_DIS_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

        macField = AAS_MAC100_BR_CONTROL_RX_STRICT_PREAMBLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));

        /* When set to 0, PMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = AAS_MAC100_BR_CONTROL_TX_ALLOW_PMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));

        /* When set to 0, EMAC frames are not transmitted unless preemption verification has succeeded. */
        macField = AAS_MAC100_BR_CONTROL_TX_ALLOW_EMAC_IF_NVERIF_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7Reset function
* @endinternal
*
* @brief   Set MTI100 MAC RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    GT_U32 macField;
    portMode = portMode;

    if (POWER_DOWN == action)
    {
        macField = AAS_MAC100_COMMAND_CONFIG_CNTL_FRAME_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_EMAC_COMMAND_CONFIG_CNTL_FRAME_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_COMMAND_CONFIG_TX_PAD_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x1, NULL));
        macField = AAS_MAC100_BR_CONTROL_RX_STRICT_PREAMBLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_BR_CONTROL_TX_PREEMPT_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_EMAC_TX_FIFO_SECTIONS_TX_SECTION_FULL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x7, NULL));
        macField = AAS_MAC100_PMAC_TX_FIFO_SECTIONS_TX_SECTION_FULL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x7, NULL));
        macField = AAS_MAC100_BR_CONTROL_BR_RX_SMD_DIS_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_TX_IPG_LENGTH_COMPENSATION_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_COMMAND_CONFIG_TX_PAD_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));
        macField = AAS_MAC100_XIF_MODE_XGMII_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_MAC100_UNIT, macField, 0x0, NULL));

    }
    else if (RESET == action)
    {
        /*
            DO NOTHING:

            1. MAC reset sequence is implemented in mvHwsExtMacResetRelease.
        */
    }
    else if (UNRESET == action)
    {
        /*
            DO NOTHING:

            1. MAC unreset sequence is implemented in mvHwsExtMacResetRelease.
        */
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7LinkStatus function
* @endinternal
*
* @brief   Get MIB MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7LinkStatus
(
    IN  GT_U8       devNum,
    IN  GT_U32      portGroup,
    IN  GT_U32      portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL     *linkStatus
)
{
    GT_U32 extField;
    GT_U32 regValue;

    portGroup = portGroup;

    extField = AAS_MAC_PORT_EXT_STATUS_LINK_OK_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, &regValue, NULL));

    *linkStatus = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7PortEnable function
* @endinternal
*
* @brief   Perform port enable on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7PortEnable
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 enable
)
{
    GT_U32 macField;
    GT_U32 regValue;

    portGroup = portGroup;
    portMode = portMode;

    regValue = (enable == GT_TRUE) ? 1 : 0;
    macField = AAS_MAC100_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC100_UNIT, macField, regValue, NULL));
    macField = AAS_MAC100_COMMAND_CONFIG_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC100_UNIT, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7PortEnableGet function
* @endinternal
*
* @brief   Get port enable status on the a port MAC.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[out] enablePtr               - port enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7PortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    GT_U32 macField;
    GT_U32 regValue;
    portGroup = portGroup;
    portMode = portMode;

    macField = AAS_MAC100_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, MTI_MAC100_UNIT, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7LoopbackStatusGet function
* @endinternal
*
* @brief   Retrive MAC loopback status.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7LoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 extField;
    GT_U32 regValue;

    portGroup = portGroup;

    extField = AAS_MAC_PORT_EXT_CONTROL_LOOP_ENA_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, &regValue, NULL));

    *lbType = (regValue == 1) ? RX_2_TX_LB : DISABLE_LB;

    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7LoopbackSet function
* @endinternal
*
* @brief   Set MAC loopback .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[out] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti100Mac_SIP7LoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32       macField;
    GT_U32 extField;
    GT_U32 regValue;

    portGroup = portGroup;

    if(lbType == RX_2_TX_LB)
    {
        regValue = 1;
    }
    else if(lbType == DISABLE_LB)
    {
        regValue = 0;
    }
    else
    {
        return GT_BAD_STATE;
    }

    macField = AAS_MAC100_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC100_UNIT, macField, regValue, NULL));
    extField = AAS_MAC_PORT_EXT_CONTROL_LOOP_ENA_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMti100Mac_SIP7IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti100Mac_SIP7IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_MAC_100])
    {
        funcPtrArray[MTI_MAC_100] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_MAC_100])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_MAC_100], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_MAC_100]->macRestFunc    = mvHwsMti100Mac_SIP7Reset;
    funcPtrArray[MTI_MAC_100]->macModeCfgFunc = mvHwsMti100Mac_SIP7ModeCfg;
    funcPtrArray[MTI_MAC_100]->macLinkGetFunc = mvHwsMti100Mac_SIP7LinkStatus;
    funcPtrArray[MTI_MAC_100]->macLbCfgFunc   = mvHwsMti100Mac_SIP7LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_100]->macLbStatusGetFunc = mvHwsMti100Mac_SIP7LoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_100]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_100]->macPortEnableFunc = mvHwsMti100Mac_SIP7PortEnable;
    funcPtrArray[MTI_MAC_100]->macPortEnableGetFunc = mvHwsMti100Mac_SIP7PortEnableGet;

    return GT_OK;
}


