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
* @file mvHwsMtiPcs10If.c
*
* @brief MTI PCS10 interface API
*
* @version   1
********************************************************************************
*/
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs10If.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>

static char* mvHwsMtiPcs10TypeGetFunc(void)
{
  return "MTI_PCS10_BaseR";
}

/**
* @internal mvHwsMtiPcs10Reset function
* @endinternal
*
* @brief   Set MTI25 PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs10Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32                  pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((POWER_DOWN == action) || (PARTIAL_POWER_DOWN == action))
    {
        pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_RX_PREAMBLE_SYNC_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0, NULL));

        pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_MODE_XGMII_BASEX_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 1, NULL));

        if(POWER_DOWN == action)
        {
            pcsField = IRONMAN_PCS_FCFEC_SDCONFIG_TOGGLE_EN_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, 0, NULL));

            pcsField = IRONMAN_PCS_FCFEC_VL_INTVL_MARKER_COUNTER_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, 0x1004, NULL));

            pcsField = IRONMAN_PCS_FCFEC_PORTS_ENA_NUMPORTS_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, 0, NULL));

            pcsField = IRONMAN_PCS_FCFEC_FC_FEC_CONTROL_FC_ENABLE_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, 0, NULL));
        }
    }

    return GT_OK;
}

static GT_U32 mvHwsMtiPchGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_UNITS_ID unitId;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 regData, fieldMac;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    if(curPortParams.portMacType == MTI_MAC_10G)
    {
        fieldMac = IRONMAN_MTIP_MAC_10G_USX_PCH_CONTROL_PCH_ENA_E;
        unitId = MTI_10G_MAC_NON_BR_UNIT;
    }
    else if(curPortParams.portMacType == MTI_MAC_10G_BR)
    {
        fieldMac = IRONMAN_MTIP_MAC_10G_BR_USX_PCH_CONTROL_PCH_ENA_E;
        unitId = MTI_10G_MAC_BR_UNIT;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, unitId, fieldMac, &regData, NULL));

    if(regData == 0)
        return 0;

    switch(portMode)
    {
        case _5G_QUSGMII:
        case _10G_OUSGMII:
        case _2_5G_SXGMII:
        case _5G_SXGMII:
        case _10G_SXGMII:
        case _10G_QXGMII:
        case _10G_DXGMII:
        case _5G_DXGMII:
            return 1;
        default:
            break;
    }
    return 0;
}

/**
* @internal mvHwsMtiPcs10Mode function
* @endinternal
*
* @brief   Set the MtiPcs10 mode
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
static GT_STATUS mvHwsMtiPcs10Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_FEC_MODE    portFecMode;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 pch_regValue, pcsField;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _10GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPcs10Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    if((portMode == _10GBase_KR) || (portMode == _10GBase_SR_LR) || (portMode == _5GBaseR) || (portMode == _12GBaseR) ||
       (portMode == _2_5G_SXGMII) || (portMode == _5G_SXGMII) || (portMode == _10G_SXGMII))
    {
        /*reg_model.m_int_manager.ConfigureSFD(conf.port_num, 1); for BaseR always 1 */
        pch_regValue = 1;
        pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_RX_PREAMBLE_SYNC_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, pch_regValue, NULL));

        if((portMode == _2_5G_SXGMII) || (portMode == _5G_SXGMII) || (portMode == _10G_SXGMII))
        {
            /* SFD configurations - for SXGMII depending on PCH */
            pch_regValue = mvHwsMtiPchGet(devNum, portGroup,portMacNum,portMode);
            pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_RX_PREAMBLE_SYNC_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, pch_regValue, NULL));

            /* configure toggle enable for non-multiplex usx ports with replication */
            pcsField = IRONMAN_PCS_FCFEC_SDCONFIG_TOGGLE_EN_E;
            CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, 1, NULL));

            /* Configure LPCS */
            pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_MODE_XGMII_BASEX_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 1, NULL));
        }
        else
        {
            if(portFecMode == FC_FEC)
            {
                /* Enable FEC */
                pcsField = IRONMAN_PCS_FCFEC_FC_FEC_CONTROL_FC_ENABLE_E;
                CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, 1, NULL));
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsPcs10TxRemoteFaultSelectSet function
* @endinternal
*
* @brief   set port remote fault selector.
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] enable                   - enable or disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsPcs10TxRemoteFaultSelectSet
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_BOOL                  enable
)
{
    GT_U32 regData;
    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL_TX_REM_FAULT_SEL_E;

    /*
        It's a new configuration we've added in IronMan.
        it's a MUX on the tx_rem_fault (force MAC to send remote faults).
        When the configuration is '0', tx_rem_fault is driven from regfile
        (port control) as was done in all previous projects.
        When set to '1', it drives tx_rem_fault by HW whenever the
        link_status (or lpcs_link_status) is '0'.
        The main purpose was to force faults for BASE-X ports when
        link is down so traffic will be drained and not get to peer.
     */

    switch(portMode)
    {
        case _100Base_FX:
        case SGMII:
        case QSGMII:
        case _1000Base_X:
        case SGMII2_5:
        case _2500Base_X:
        case _10G_OUSGMII:
        case _5G_QUSGMII:
            regData = (enable == GT_TRUE) ? 1 : 0;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PORT_EXT_UNIT, extField, regData, NULL));
            break;
        default:
            break;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10LoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    GT_U32 pcsField;
    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Disable Remote Fault if we are enabling loopback and vice versa */
    CHECK_STATUS(mvHwsPcs10TxRemoteFaultSelectSet(devNum, phyPortNum, portMode, ((DISABLE_LB == lbType) ? GT_TRUE : GT_FALSE)));

    data = (lbType == TX_2_RX_LB) ? 1 : 0;

#if 0
#ifndef ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        /*
            Currently PCS EXT loopback does not work - JIRA MTIWRAPPER-401
            WA - using PCS Base_R loopback field for tests on emulator
        */
        pcsField = IRONMAN_BASE_R_PCS_CONTROL1_LOOPBACK_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_BASER_E, pcsField, data, NULL));
    }
    else
#endif
#endif
    pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_CONTROL_P0_LOOPBACK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_EXT_UNIT, pcsField, data, NULL));

    extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL1_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PORT_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10LoopBackGet function
* @endinternal
*
* @brief   Get the PCS loop back mode state.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10LoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_UREG_DATA    data;
    GT_U32 pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

#if 0
#ifndef ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        /*
            Currently PCS EXT loopback does not work - JIRA MTIWRAPPER-401
            WA - using PCS Base_R loopback field for tests on emulator
        */
        pcsField = IRONMAN_BASE_R_PCS_CONTROL1_LOOPBACK_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_BASER_E, pcsField, &data, NULL));
    }
    else
#endif
#endif
    pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_CONTROL_P0_LOOPBACK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_EXT_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPcs10.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA                  data;
    MV_HWS_PORT_INIT_PARAMS       curPortParams;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField = IRONMAN_MTIP_PCS_EXT_UNITS_STATUS_P0_LINK_STATUS_E;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_EXT_UNIT, extField, &data, NULL));

    *lock = (data != 0);
#ifdef ASIC_SIMULATION
    *lock = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10SendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, pcsNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#if 0
    if(curPortParams.portPcsType == MTI_PCS_10G_LPCS)
    {
        if(portMode != _10G_QXGMII)
        {
            return GT_NOT_SUPPORTED;
        }
    }
#endif
    if (send == GT_TRUE)
    {
        /*  start sending fault signals  */
        data = 1;
    }
    else
    {
        /*  stop sending fault signals */
        data = 0;
    }

    extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL_TX_REM_FAULT_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_10G_PORT_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10FecConfigGet function
* @endinternal
*
* @brief   Return the FEC mode  status on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] portFecTypePtr           - pointer to fec mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10FecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      regData, pcsField;

    if(portFecTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(curPortParams.fecSupportedModesBmp == FEC_OFF)
    {
        *portFecTypePtr = FEC_OFF;
        return GT_OK;
    }

    pcsField = IRONMAN_PCS_FCFEC_FC_FEC_CONTROL_FC_ENABLE_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, phyPortNum, MTI_10G_PCS_FCFEC_UNIT, portMode, pcsField, &regData, NULL));

    *portFecTypePtr = (regData == 1) ? FC_FEC : FEC_OFF;

    return  GT_OK;
}

/**
* @internal mvHwsMtiPcs10CheckGearBox function
* @endinternal
*
* @brief   check if gear box is locked on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] lockPtr                 - pointer lock value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10CheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    MV_HWS_IRONMAN_BASE_R_PCS_UNITS_E  pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = IRONMAN_BASE_R_PCS_BASER_STATUS1_BLOCK_LOCK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_BASER_UNIT, pcsField, &data, NULL));
    *lockPtr = (data & 0x1) ? GT_TRUE : GT_FALSE;

#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10SendLocalFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending local fault signals to partner.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10SendLocalFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_IRONMAN_MTIP_MAC_PORT_EXT_UNITS extField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, pcsNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
#if 0
    if(curPortParams.portPcsType == MTI_PCS_10G_LPCS)
    {
        if(portMode != _10G_QXGMII)
        {
            return GT_NOT_SUPPORTED;
        }
    }
#endif
    if (send == GT_TRUE)
    {
        /*  start sending fault signals  */
        data = 1;
    }
    else
    {
        /*  stop sending fault signals */
        data = 0;
    }

    extField = IRONMAN_MTIP_MAC_PORT_EXT_CONTROL_TX_LOC_FAULT_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, pcsNum, MTI_10G_PORT_EXT_UNIT, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10IfInit function
* @endinternal
*
* @brief   Init MtiPcs10 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs10IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_10G_BASE_R])
    {
        funcPtrArray[MTI_PCS_10G_BASE_R] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_10G_BASE_R])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_10G_BASE_R], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsResetFunc     = mvHwsMtiPcs10Reset;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsModeCfgFunc   = mvHwsMtiPcs10Mode;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsLbCfgFunc     = mvHwsMtiPcs10LoopBack;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsLbCfgGetFunc = mvHwsMtiPcs10LoopBackGet;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsTypeGetFunc   = mvHwsMtiPcs10TypeGetFunc;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsFecCfgGetFunc = mvHwsMtiPcs10FecConfigGet;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsCheckGearBoxFunc = mvHwsMtiPcs10CheckGearBox;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsAlignLockGetFunc = mvHwsMtiPcs10AlignLockGet;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsSendFaultSetFunc = mvHwsMtiPcs10SendFaultSet;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs10SendLocalFaultSet;
    funcPtrArray[MTI_PCS_10G_BASE_R]->pcsTxRemoteFaultSelectSetFunc = mvHwsPcs10TxRemoteFaultSelectSet;

    return GT_OK;
}

static char* mvHwsMtiPcs10LpcsTypeGetFunc(void)
{
  return "MTI_PCS10_LPCS";
}

/**
* @internal mvHwsMtiPcs10LpcsReset function
* @endinternal
*
* @brief   Set LPCS PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs10LpcsReset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 regData, pcsField;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    if(GT_OK != mvHwsGlobalMacToLocalIndexConvert(devNum, portMacNum, portMode, &convertIdx))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((POWER_DOWN == action) || (PARTIAL_POWER_DOWN == action))
    {
        /*if(POWER_DOWN == action)
        {*/
            pcsField = IRONMAN_LPCS_GENERAL_GMODE_LPCS_ENABLE_E;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, &regData, NULL));
            regData &= ~(1 << convertIdx.ciderIndexInUnit);
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, regData, NULL));
        /*}*/

        pcsField = IRONMAN_LPCS_LINK_TIMER_0_TIMER0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));
        pcsField = IRONMAN_LPCS_LINK_TIMER_0_TIMER15_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x968, NULL));
        pcsField = IRONMAN_LPCS_LINK_TIMER_1_TIMER20_16_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x13, NULL));
        pcsField = IRONMAN_LPCS_CONTROL_ANENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x1, NULL));
        pcsField = IRONMAN_LPCS_IF_MODE_SGMII_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));


        if(POWER_DOWN == action)
        {
            if((portMode == QSGMII) || (portMode == _5G_QUSGMII))
            {
                if(convertIdx.ciderIndexInUnit < 4)
                {
                    pcsField = IRONMAN_LPCS_GENERAL_GMODE_QSGMII_0_ENABLE_E;
                }
                else
                {
                    pcsField = IRONMAN_LPCS_GENERAL_GMODE_QSGMII_1_ENABLE_E;
                }
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, 0x0, NULL));
            }
        }

        if(POWER_DOWN == action)
        {
            if(portMode == _10G_OUSGMII)
            {
                pcsField = IRONMAN_LPCS_GENERAL_GMODE_USGMII8_ENABLE_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, 0x0, NULL));
                pcsField = IRONMAN_LPCS_GENERAL_GMODE_USGMII_SCRAMBLE_ENABLE_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, 0x0, NULL));
            }
        }

        if(portMode == _2500Base_X)
        {
            pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_SEQ_ENA_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));
        }

        if(portMode == _100Base_FX)
        {
            pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_PCS_100FX_100FX_RX_CONTROL2_SD0_CDR_SPEED_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_EXT_UNIT, pcsField, 0x0, NULL));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10LpcsMode function
* @endinternal
*
* @brief   Set the PCS10 LPCS mode
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
static GT_STATUS mvHwsMtiPcs10LpcsMode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  portMacNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_STATUS rc;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_MII_TYPE_E miiType;
    GT_U32 pch_regValue, regData, pcsField;
    MV_HWS_HAWK_CONVERT_STC convertIdx;

    GT_UNUSED_PARAM(attributesPtr);

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portMacNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portMacNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _10GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPcs10Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    pch_regValue = mvHwsMtiPchGet(devNum, portGroup,portMacNum,portMode);
    pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_RX_PREAMBLE_SYNC_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, pch_regValue, NULL));

    /*ext_manager::set_hd */
    regData = (attributesPtr->duplexMode == MV_HWS_PORT_HALF_DUPLEX_E) ? 1 : 0;
    pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_CONTROL_P0_HD_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_EXT_UNIT, pcsField, regData, NULL));
    pcsField = IRONMAN_LPCS_IF_MODE_SGMII_DUPLEX_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, regData, NULL));

    /*int_manager::ConfigureLpcs*/
    if((portMode == QSGMII) || (portMode == _5G_QUSGMII))
    {
        if(convertIdx.ciderIndexInUnit < 4)
        {
            pcsField = IRONMAN_LPCS_GENERAL_GMODE_QSGMII_0_ENABLE_E;
        }
        else
        {
            pcsField = IRONMAN_LPCS_GENERAL_GMODE_QSGMII_1_ENABLE_E;
        }
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, 1, NULL));
    }
    else if(portMode == _10G_OUSGMII)
    {
        pcsField = IRONMAN_LPCS_GENERAL_GMODE_USGMII8_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, 1, NULL));
        pcsField = IRONMAN_LPCS_GENERAL_GMODE_USGMII_SCRAMBLE_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, 1, NULL));
    }
    else if(portMode == _2500Base_X)
    {
        /*
            SGMII                   _SGMII
            _1000Base_X             _1000BASE_X
            SGMII2_5                _1000BASE_X_OS_2p5=SGMII2_5
            _2500Base_X             _2500BASE_X_8023CB
        */

        pcsField = IRONMAN_LPCS_IF_MODE_IFMODE_SEQ_ENA_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 1, NULL));
    }

    if(portMode != _10G_QXGMII)
    {
        pcsField = IRONMAN_LPCS_GENERAL_GMODE_LPCS_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, &regData, NULL));
        regData |= (1 << convertIdx.ciderIndexInUnit);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_GENERAL_UNIT, pcsField, regData, NULL));
    }

    pcsField = IRONMAN_LPCS_LINK_TIMER_0_TIMER0_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));
    pcsField = IRONMAN_LPCS_LINK_TIMER_0_TIMER15_1_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));
    pcsField = IRONMAN_LPCS_LINK_TIMER_1_TIMER20_16_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));
    pcsField = IRONMAN_LPCS_CONTROL_ANENABLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, 0x0, NULL));

    pcsField = IRONMAN_LPCS_IF_MODE_SGMII_ENA_E;
    if((portMode == _1000Base_X) || (portMode == SGMII2_5) || (portMode == _2500Base_X))
    {
        regData = 0;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, regData, NULL));
    }
    else if((portMode == SGMII) || (portMode == QSGMII) || (portMode == _10G_OUSGMII) || (portMode == _5G_QUSGMII))
    {
        regData = 1;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, pcsField, regData, NULL));
    }

    if(portMode == _100Base_FX)
    {
        pcsField = IRONMAN_MTIP_PCS_EXT_UNITS_PCS_100FX_100FX_RX_CONTROL2_SD0_CDR_SPEED_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_EXT_UNIT, pcsField, 0x4, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10LpcsAlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPcs10.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] portNum                  - port number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10LpcsAlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    GT_U32                      portNum,
    MV_HWS_PORT_STANDARD        portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA                data;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      extField;

    if(portMode == _10G_QXGMII)
    {
        extField = ((portNum % 8) < 4) ? IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_STATUS_ALIGN_LOCK_0_E : IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_STATUS_ALIGN_LOCK_1_E;
    }
    else if(portMode == _100Base_FX)
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_PCS_100FX_100FX_STATUS0_P0_100FX_PCS_LINK_STATUS_E;
    }
    else
    {
       /*
        * lpcs_link_status = lpcs_rx_sync & lpcs_an_done (if AN enabled).
        * To avoid align-lock lost when AN is restarted, RX_SYNC bit is
        * used instead of LINK_STATUS
        */
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_STATUS_P0_LPCS_RX_SYNC_E;
    }

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, portNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_EXT_UNIT, extField, &data, NULL));

    *lock = (data != 0);
#ifdef ASIC_SIMULATION
    *lock = GT_TRUE;
#endif

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs10LpcsCheckGearBox function
* @endinternal
*
* @brief   check if gear box is locked on PCS.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @param[out] lockPtr                 - pointer lock value
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs10LpcsCheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_UREG_DATA                data;
    GT_32                       extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(portMode == _10G_QXGMII)
    {
        extField = ((phyPortNum % 8) < 4) ? IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_STATUS_ALIGN_LOCK_0_E : IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_STATUS_ALIGN_LOCK_1_E;
    }
    else if(portMode == _100Base_FX)
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_PCS_100FX_100FX_STATUS0_P0_100FX_PCS_LINK_STATUS_E;
    }
    else
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_STATUS_P0_LPCS_LINK_STATUS_E;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, curPortParams.portPcsNumber, MTI_10G_PCS_EXT_UNIT, extField, &data, NULL));
    *lockPtr = (data & 0x1) ? GT_TRUE : GT_FALSE;

#ifdef ASIC_SIMULATION
    *lockPtr = GT_TRUE;
#endif

    return GT_OK;
}

#ifndef IRONMAN_DEV_SUPPORT
/**
 * @internal mvHwsMtiPcs10LpcsAutoNeg1GSgmii function
 * @endinternal
 *
 * @brief   Configure Auto-negotiation for SGMII/1000BaseX port
 *          modes.
 *
 *
 * @param devNum
 * @param phyPortNum
 * @param portMode
 * @param autoNegotiationPtr
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsMtiPcs10LpcsAutoNeg1GSgmii
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portMacNum,
    IN  MV_HWS_PORT_STANDARD        portMode,
    IN  MV_HWS_PCS_AUTONEG_1G_SGMII *autoNegPtr
)
{
    GT_UREG_DATA    data;
    GT_BOOL enable = (autoNegPtr->inbandAnEnable) && !(autoNegPtr->byPassEnable);

        /*
         * 1.  For both 1000Base-X and SGMII, need to enable the AN by setting:
         *      /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_CONTROL
         *      Bit[12] = 0x1.
         */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_CONTROL_ANENABLE_E, enable, NULL));

    switch (portMode)
    {
        case _10G_OUSGMII:
        case _5G_QUSGMII:
        case QSGMII:
        case SGMII:
            /*
             * 2.  For SGMII, need to configure the
             *      link timer. Each with different value. Registers:
             *       /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_1
             *       /<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_LINK_TIMER_0
             *     For SGMII the standard requires 1.6ms, which means:
             *      Link_Timer_1[4:0] = 0x3.
             *      Link_Timer_0[15:0] = 0xd40.
             */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_LINK_TIMER_0_TIMER0_E, 0, NULL));
            data = ((enable) ? (0xd40>>1) : (0x0));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_LINK_TIMER_0_TIMER15_1_E, data, NULL));
            data = ((enable) ? (0x3) : (0x0));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_LINK_TIMER_1_TIMER20_16_E, data, NULL));
            /*
             * 3.  For SGMII/QSGMII only:
             */
            if (SGMII == portMode || QSGMII == portMode)
            {
              /**
                *  We wish to let the HW set the speed automatically once AN is done.
                *  This is done by writing to following register:
                *  /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
                *  Bit[1] = 0x1.
                *  In this case, bit[3:2] are don’t care.
                */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_IF_MODE_USE_SGMII_AN_E, enable, NULL));
            }
            else
            {
               /**
                 *      For OUSGMII we must use the SW set speed flow
                 *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_IF_MODE
                 *      Bit[1] = 0x0
                 *      Set bit[3:2] in  mvHwsMtiUsxReplicationSet
                 */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_IF_MODE_USE_SGMII_AN_E, 0, NULL));
            }

            /*
             * 4.  Configure device ability:
             *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
             *      Set bit[0] to 0x1 and set all other bits to 0.
             */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_DEV_ABILITY_ABILITY_RSV05_E, enable, NULL));
            break;

        case _1000Base_X:
        case _1000Base_SX:
           /*
             * 2.  For 1000Base-X the standard requires 10ms, which means:
             *      Link_Timer_1[4:0] = 0x13.
             *      Link_Timer_0[15:0] = 0x12d0. (bit[0] is part of the value although it is RO)
             */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_LINK_TIMER_0_TIMER0_E, 0, NULL));
            data = ((enable) ? (0x12d0>>1) : (0x0));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_LINK_TIMER_0_TIMER15_1_E, data, NULL));
            data = ((enable) ? (0x13) : (0x0));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_LINK_TIMER_1_TIMER20_16_E, data, NULL));
            /*
             * 3.  Configure device ability:
             *      /<MTIP_IP>MTIP_IP/<MTIP_IP> LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
             *      Since we support only Full-duplex, set bit[5] to 0x1 and bit[6] to 0x0.
             *      Set bit[7] 'PS1' - PAUSE and bit[8] 'PS2' - ASM_DIR
             *      Set all other bits to 0
             */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_DEV_ABILITY_FD_E, 1, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_DEV_ABILITY_HD_E, 0, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_DEV_ABILITY_PS1_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlPauseAdvertiseEnable) : (0x1)), NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portMacNum, MTI_10G_PCS_LPCS_PORT_UNIT, IRONMAN_LPCS_DEV_ABILITY_PS2_E, ((enable) ? BOOL2BIT_MAC(autoNegPtr->flowCtrlAsmAdvertiseEnable) : (0x1)), NULL));

            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    return GT_OK;
}
#endif
/**
* @internal mvHwsMtiPcs10LpcsIfInit function
* @endinternal
*
* @brief   Init MtiPcs10 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs10LpcsIfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_10G_LPCS])
    {
        funcPtrArray[MTI_PCS_10G_LPCS] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_10G_LPCS])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_10G_LPCS], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsResetFunc     = mvHwsMtiPcs10LpcsReset;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsModeCfgFunc   = mvHwsMtiPcs10LpcsMode;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsLbCfgFunc     = mvHwsMtiPcs10LoopBack;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsLbCfgGetFunc  = mvHwsMtiPcs10LoopBackGet;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsTypeGetFunc   = mvHwsMtiPcs10LpcsTypeGetFunc;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsAlignLockGetFunc = mvHwsMtiPcs10LpcsAlignLockGet;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs10SendLocalFaultSet;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsSendFaultSetFunc = mvHwsMtiPcs10SendFaultSet;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsFecCfgGetFunc = mvHwsMtiPcs10FecConfigGet;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsCheckGearBoxFunc = mvHwsMtiPcs10LpcsCheckGearBox;
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsTxRemoteFaultSelectSetFunc = mvHwsPcs10TxRemoteFaultSelectSet;
#ifndef IRONMAN_DEV_SUPPORT
    funcPtrArray[MTI_PCS_10G_LPCS]->pcsAutoNeg1GSgmiiFunc = mvHwsMtiPcs10LpcsAutoNeg1GSgmii;
#endif

    return GT_OK;
}
