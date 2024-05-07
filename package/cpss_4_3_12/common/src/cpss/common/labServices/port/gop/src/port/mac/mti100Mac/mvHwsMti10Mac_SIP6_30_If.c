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
* mvHwsMti10Mac_SIP6_30_If.c
*
* DESCRIPTION: MTI10G MAC
*
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
******************************************************************************/
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#endif
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>
#include <cpss/common/labServices/port/gop/common/os/mvSemaphore.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

static char* mvHwsMacTypeGet(void)
{
  return "MTI10G_MAC";
}

/**
* @internal mvHwsMtiMac10_SIP6_30_ModeCfg function
* @endinternal
*
* @brief   Set the MTI100 MAC in GOP.
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
static GT_STATUS mvHwsMtiMac10_SIP6_30_ModeCfg
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE    portFecMode;
    GT_STATUS  rc;
    GT_U32 markerInterval;
    MV_HWS_IRONMAN_MTIP_MAC_10G_UNITS macField;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId = MTI_10G_MAC_NON_BR_UNIT;
    GT_U32 txFifoFullThreshold;
    GT_U32 txFifoEmptyThreshold;
    GT_U32 duplexMode = 0;

    if(!HWS_DEV_SIP_6_30_CHECK_MAC(devNum))
    {
        return GT_NOT_APPLICABLE_DEVICE;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _10GMII_E)
    {
        hwsOsPrintf("mvHwsMtiMac10_SIP6_30_ModeCfg: bad MII type for portNum[%d], port mode[%d]\n", portMacNum, portMode);
        return GT_BAD_PARAM;
    }

    if ( attributesPtr != NULL )
    {
        duplexMode = ((attributesPtr->duplexMode==MV_HWS_PORT_HALF_DUPLEX_E) ? 1 : 0);
    }
    rc = mvHwsMarkerIntervalCalc(devNum, portMode, portFecMode, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Common COMPENSATION */
    macField = IRONMAN_MTIP_MAC_10G_TX_IPG_LENGTH_COMPENSATION_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, markerInterval, NULL));

    if(mvHwsUsxModeCheck(devNum, portMacNum, portMode) &&
                         ((markerInterval != 0x4FFF) && (markerInterval != 0x0)))
    {
        regValue = 1;
    }
    else
    {
        regValue = 0;
    }
    macField = IRONMAN_MTIP_MAC_10G_TX_IPG_LENGTH_CMP_ALLOW_SHORT_IPG_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    macField = IRONMAN_MTIP_MAC_10G_TX_IPG_LENGTH_TXIPG_RESERVED2_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    /* Common TXIPG */
    macField = IRONMAN_MTIP_MAC_10G_TX_IPG_LENGTH_TXIPG_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    /* CRC */
    macField = IRONMAN_MTIP_MAC_10G_CRC_MODE_DIS_RX_CRC_CHK_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    /* EMAC CNTL_FRAME_ENA */
    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_CNTL_FRAME_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    /* Common TX_PAD_EN */
    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_TX_PAD_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 1, NULL));

    /* Common CRC_FWD */
    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_CRC_FWD_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0, NULL));

    CHECK_STATUS(mvHwsMtipMacTxSectionFullCalc(devNum, portMacNum,portMode,attributesPtr,&txFifoFullThreshold));
    CHECK_STATUS(mvHwsMtipMacTxSectionEmptyCalc(devNum, portMacNum,portMode,attributesPtr,&txFifoEmptyThreshold));

    /* TX_SECTION_FULL */
    macField = IRONMAN_MTIP_MAC_10G_TX_FIFO_SECTIONS_TX_SECTION_FULL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, txFifoFullThreshold, NULL));
    macField = IRONMAN_MTIP_MAC_10G_TX_FIFO_SECTIONS_TX_SECTION_EMPTY_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, txFifoEmptyThreshold, NULL));


    /* XIF_MODE_ONESTEP_ENA configuration will be handled by PTP manager */
#if 0
    /* PTP related configuration - for port modes that should work with PHY may be skipped to
       improve latency */
    if(!((attributesPtr->lowLatencyEnabled == GT_TRUE) && (portMode == _10G_OUSGMII)))
    {
        /* Common ONESTEPENA */
        macField = IRONMAN_MTIP_MAC_10G_XIF_MODE_ONESTEP_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));
    }
#endif

    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_RX_SFD_ANY_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

   /* Common HD_ENA */
    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_HD_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, duplexMode, NULL));

    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_FLT_TX_STOP_DIS_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x1, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiMac10_SIP6_30_Reset function
* @endinternal
*
* @brief   Set MTI100 MAC RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiMac10_SIP6_30_Reset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_UNITS_ID unitId = MTI_10G_MAC_NON_BR_UNIT;
    MV_HWS_IRONMAN_MTIP_MAC_10G_UNITS macField;

    portMode = portMode;

    if (POWER_DOWN == action)
    {
        macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_TX_PAD_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, portMacNum, unitId, macField, 0x0, NULL));

        macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_RX_SFD_ANY_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x0, NULL));

        macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_HD_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x0, NULL));

        macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_FLT_TX_STOP_DIS_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, 0x0, NULL));
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
* @internal mvHwsMtiMac10_SIP6_30_LinkStatus function
* @endinternal
*
* @brief   Get MTI100 MAC link status.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiMac10_SIP6_30_LinkStatus
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *linkStatus
)
{
    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    portMode = portMode;
    portGroup = portGroup;

    extField = IRONMAN_MTIP_MAC_PORT_EXT_STATUS_LINK_OK_E;
    unitId = MTI_10G_PORT_EXT_UNIT;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, extField, &regValue, NULL));
    *linkStatus = (regValue == 1) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsMtiMac10_SIP6_30_PortEnable function
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
static GT_STATUS mvHwsMtiMac10_SIP6_30_PortEnable
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 enable
)
{
    MV_HWS_IRONMAN_MTIP_MAC_10G_UNITS macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    unitId = MTI_10G_MAC_NON_BR_UNIT;

    regValue = (enable == GT_TRUE) ? 1 : 0;

    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));
    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_RX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, unitId, macField, regValue, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiMac10_SIP6_30_PortEnableGet function
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
static GT_STATUS mvHwsMtiMac10_SIP6_30_PortEnableGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_BOOL                 *enablePtr
)
{
    MV_HWS_IRONMAN_MTIP_MAC_10G_UNITS macField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    unitId = MTI_10G_MAC_NON_BR_UNIT;
    macField = IRONMAN_MTIP_MAC_10G_COMMAND_CONFIG_TX_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, macField, &regValue, NULL));

    *enablePtr = (regValue) ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/**
* @internal mvHwsMtiMac10_SIP6_30_LoopbackStatusGet function
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
static GT_STATUS mvHwsMtiMac10_SIP6_30_LoopbackStatusGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField;
    GT_U32 regValue;
    MV_HWS_UNITS_ID unitId;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL_LOOP_ENA_E;
    unitId = MTI_10G_PORT_EXT_UNIT;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, unitId, extField, &regValue, NULL));

    *lbType = (regValue == 1) ? RX_2_TX_LB : DISABLE_LB;

    return GT_OK;
}

/**
* @internal mvHwsMtiMac10_SIP6_30_LoopbackSet function
* @endinternal
*
* @brief   Set MAC loopback .
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] macNum                   - MAC number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - supported loopback type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiMac10_SIP6_30_LoopbackSet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  portMacNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_PORT_LB_TYPE     lbType
)
{

    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField;
    GT_U32 regValue;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

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

    extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL_LOOP_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PORT_EXT_UNIT, extField, regValue, NULL));
    extField = IRONMAN_MTIP_MAC_PORT_EXT_PMAC_CONTROL_PREEMPTION_LOOP_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PORT_EXT_UNIT, extField, regValue, NULL));

    /*
        Please pay attention that
        bit_10(MAC100_UNITS_COMMAND_CONFIG_P0_LOOPBACK_EN_E)
        is not connected to any logic - don't use it
    */

    /*
       In order to disable regular traffic to continue to the next units in
       pipe (MIF) it is needed to configure <loop_rx_block_out> bit, but
       default value already do it
    */

    return GT_OK;
}


/**
* @internal mvHwsMtiMac10_SIP6_30_IfInit function
* @endinternal
 *
* @param[in] devNum                   - system device number
* @param[in] funcPtrArray             - (pointer to) func ptr array
*/
GT_STATUS mvHwsMtiMac10_SIP6_30_IfInit(MV_HWS_MAC_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_MAC_10G])
    {
        funcPtrArray[MTI_MAC_10G] = (MV_HWS_MAC_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_MAC_FUNC_PTRS));
        if(!funcPtrArray[MTI_MAC_10G])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_MAC_10G], 0, sizeof(MV_HWS_MAC_FUNC_PTRS));
    }
    funcPtrArray[MTI_MAC_10G]->macRestFunc    = mvHwsMtiMac10_SIP6_30_Reset;
    funcPtrArray[MTI_MAC_10G]->macModeCfgFunc = mvHwsMtiMac10_SIP6_30_ModeCfg;
    funcPtrArray[MTI_MAC_10G]->macLinkGetFunc = mvHwsMtiMac10_SIP6_30_LinkStatus;
    funcPtrArray[MTI_MAC_10G]->macLbCfgFunc   = mvHwsMtiMac10_SIP6_30_LoopbackSet;
#ifndef CO_CPU_RUN
    funcPtrArray[MTI_MAC_10G]->macLbStatusGetFunc = mvHwsMtiMac10_SIP6_30_LoopbackStatusGet;
#endif
    funcPtrArray[MTI_MAC_10G]->macTypeGetFunc = mvHwsMacTypeGet;
    funcPtrArray[MTI_MAC_10G]->macPortEnableFunc = mvHwsMtiMac10_SIP6_30_PortEnable;
    funcPtrArray[MTI_MAC_10G]->macPortEnableGetFunc = mvHwsMtiMac10_SIP6_30_PortEnableGet;

    return GT_OK;
}

