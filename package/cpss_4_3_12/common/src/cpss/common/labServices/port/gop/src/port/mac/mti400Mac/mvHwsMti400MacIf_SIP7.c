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
* mvHwsMti400MacIf_SIP7.c
*
* DESCRIPTION: MTI 400G MAC
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
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>


static char* mvHwsMacTypeGet(void)
{
  return "MTI400_MAC";
}

/**
* @internal mvHwsMti400Mac_SIP7ModeCfg function
* @endinternal
*
* @brief   Set the MTI400 MAC in GOP.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] attributesPtr            - port attributes
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti400Mac_SIP7ModeCfg
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_U32 macField;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(attributesPtr);

    macField = AAS_MAC400_COMMAND_CONFIG_CMD_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = AAS_MAC400_COMMAND_CONFIG_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = AAS_MAC400_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0, NULL));

    macField = AAS_MAC400_COMMAND_CONFIG_PROMIS_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

    macField = AAS_MAC400_TX_FIFO_SECTIONS_TX_SECTION_AVAIL_THRESHOLD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x8, NULL));

    macField = AAS_MAC400_RX_FIFO_SECTIONS_RX_SECTION_AVAIL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

#if 0
    macField = MAC400_UNITS_FRM_LENGTH_P0_FRM_LENGTH_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x3178, NULL));
#endif

    return GT_OK;
}

/**
* @internal mvHwsMti400Mac_SIP7Reset function
* @endinternal
*
* @brief   Set MTI400 MAC RESET/UNRESET or FULL_RESET action.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portMacNum               - MAC number
* @param[in] portMode                 - port mode
* @param[in] action                   - RESET/UNRESET or FULL_RESET
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti400Mac_SIP7Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    GT_U32 macField;
    portGroup = portGroup;
    portMode = portMode;

    if (POWER_DOWN == action)
    {
        macField = AAS_MAC400_COMMAND_CONFIG_CMD_FRAME_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x0, NULL));

        macField = AAS_MAC400_COMMAND_CONFIG_TX_PAD_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x1, NULL));

        macField = AAS_MAC400_COMMAND_CONFIG_PROMIS_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x0, NULL));

        macField = AAS_MAC400_TX_FIFO_SECTIONS_TX_SECTION_AVAIL_THRESHOLD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, 0x8, NULL));
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
* @internal mvHwsMti400Mac_SIP7LinkStatus function
* @endinternal
*
* @brief   Get MIB MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMti400Mac_SIP7LinkStatus
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
* @internal mvHwsMti400Mac_SIP7PortEnable function
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
static GT_STATUS mvHwsMti400Mac_SIP7PortEnable
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
    macField = AAS_MAC400_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, regValue, NULL));
    macField = AAS_MAC400_COMMAND_CONFIG_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMti400Mac_SIP7PortEnableGet function
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
static GT_STATUS mvHwsMti400Mac_SIP7PortEnableGet
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

    macField = AAS_MAC400_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMti400Mac_SIP7LoopbackStatusGet function
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
static GT_STATUS mvHwsMti400Mac_SIP7LoopbackStatusGet
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
* @internal mvHwsMti400Mac_SIP7LoopbackSet function
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
static GT_STATUS mvHwsMti400Mac_SIP7LoopbackSet
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

    macField = AAS_MAC400_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_MAC400_UNIT, macField, regValue, NULL));
    extField = AAS_MAC_PORT_EXT_CONTROL_LOOP_ENA_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_EXT_UNIT, portMode, extField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMti
* 400Mac_SIP7IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMti400Mac_SIP7IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_MAC_400])
    {
        funcPtrArray[MTI_MAC_400] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_MAC_400])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_MAC_400], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_MAC_400]->macRestFunc    = mvHwsMti400Mac_SIP7Reset;
    funcPtrArray[MTI_MAC_400]->macModeCfgFunc = mvHwsMti400Mac_SIP7ModeCfg;
    funcPtrArray[MTI_MAC_400]->macLinkGetFunc = mvHwsMti400Mac_SIP7LinkStatus;
    funcPtrArray[MTI_MAC_400]->macLbCfgFunc   = mvHwsMti400Mac_SIP7LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_400]->macLbStatusGetFunc = mvHwsMti400Mac_SIP7LoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_400]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_400]->macPortEnableFunc = mvHwsMti400Mac_SIP7PortEnable;
    funcPtrArray[MTI_MAC_400]->macPortEnableGetFunc = mvHwsMti400Mac_SIP7PortEnableGet;

    return GT_OK;
}


