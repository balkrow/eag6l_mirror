/*******************************************************************************
*           Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsIronmanPortAnp.c
*
* @brief This file contains API for ANPp port configuartion
*
* @version   1
********************************************************************************
*/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIronmanPortAnp.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>


/**************************** Globals ****************************************************/
#ifndef IRONMAN_DEV_SUPPORT
extern  GT_BOOL hwsPpHwTraceFlag;
#endif

#ifdef IRONMAN_DEV_SUPPORT
#define IRONMAN_DEBUG 0
#else
#define IRONMAN_DEBUG 1
#endif

/**************************** Definition *************************************************/

GT_STATUS mvHwsHawkLocalIndexToAnpIndexConvert
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    IN GT_U32                   localIdx,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_U32                  *anpIdxPtr
);

extern GT_STATUS mvHwsExtIfLastInPortGroupCheck
(
    IN GT_U8                       devNum,
    IN GT_U32                      portNum,
    IN MV_HWS_PORT_STANDARD        portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT   *isLastPtr
);

extern GT_STATUS mvHwsExtIfFirstInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
);

extern GT_STATUS mvHwsExtIfLastInSerdesGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isLastPtr
);

/**************************** Declaration ********************************************/

/**
* @internal mvHwsIronmanAnpPortPcsSdTxRxClockGate function
* @endinternal
*
* @brief   Gate/Un-gate PCS sd TX/RX clock
*          TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortPcsSdTxRxClockGate
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             gate
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (gate == GT_FALSE)?1:0;

    /* Set PCS sd tx & rx clock to gated */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E, data, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortPcsSdTxRxReset function
* @endinternal
*
* @brief   Reset/Un-Reset PCS sd TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortPcsSdTxRxReset
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             reset
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (reset == GT_FALSE)?1:0;

    /* Set PCS sd tx & rx to reset */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E, data, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortAnArbStateGet function
* @endinternal
*
* @brief   get AN ARB SM
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnArbStateGet
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    MV_HWS_AP_SM_STATE      *anSmState
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = AN_USX_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  portNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_AP_ANEG_STATE_S10_0_E, &data, NULL));
    *anSmState = (MV_HWS_AP_SM_STATE)data;

    return GT_OK;
}
/**
* @internal mvHwsIronmanAnpPortAnTxRxClockGate function
* @endinternal
*
* @brief   Gate/Un-gate AN TX/RX clock
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnTxRxClockGate
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 gate,
    MV_HWS_ANP_TXRX_MODE    txrx
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (gate == GT_FALSE)?1:0;

    switch ( txrx )
    {
    case MV_HWS_ANP_TX_ONLY:
        /* Un/gate AN TX clock */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_VAL_E, data, NULL));
        break;
    case MV_HWS_ANP_RX_ONLY:
        /* Un/gate AN RX clock */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_VAL_E, data, NULL));
        break;
    case MV_HWS_ANP_TXRX:
        /* Un/gate AN TX clock */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_VAL_E, data, NULL));
        /* Un/gate AN RX clock */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_VAL_E, data, NULL));
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanIronmanAnpPortAnTxRxReset function
* @endinternal
*
* @brief   Reset/Un-Reset AN TX/RX
*          TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnTxRxReset
(
    GT_U8                   devNum,
    GT_U32                  portNum,
    GT_BOOL                 reset,
    MV_HWS_ANP_TXRX_MODE    txrx

)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (reset == GT_TRUE)?1:0;

    switch ( txrx )
    {
    case MV_HWS_ANP_TX_ONLY:
        /* Un/Reset AN TX */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_TX_OW_VAL_E, data, NULL));
        break;
    case MV_HWS_ANP_RX_ONLY:
        /* Un/Reset AN RX */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_RX_OW_VAL_E, data, NULL));
        break;
    case MV_HWS_ANP_TXRX:
        /* Un/Reset AN TX */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_TX_OW_VAL_E, data, NULL));
        /* Un/Reset AN RX */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_RX_OW_VAL_E, data, NULL));
        break;
    default:
        return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortPcsTxRxReset function
* @endinternal
*
* @brief   Reset/Un-Reset PCS TX/RX
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortPcsTxRxReset
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             reset
)
{
    GT_U8 data, sdIndex;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_IRONMAN_MTIP_PCS_EXT_UNITS_E extField;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    data = (reset == GT_FALSE)?1:0;

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, _10GBase_KR, &convertIdx));
    sdIndex = convertIdx.ciderIndexInUnit / 4;

    if(sdIndex == 0)
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_TX_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, data, NULL));
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD0_RX_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, data, NULL));
    }
    else
    {
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_TX_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, data, NULL));
        extField = IRONMAN_MTIP_PCS_EXT_UNITS_GLOBAL_PCS_CLOCK_AND_RESET_CONTROL_SD1_RX_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_10G_PCS_EXT_UNIT, extField, data, NULL));
    }
    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortAnPcsMuxSet function
* @endinternal
*
* @brief   Set AN/PCS mux
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnPcsMuxSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             pcs
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (pcs == GT_TRUE)?1:0;

    /* Gate sd_tx_clk back to COMPHY */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PM_CLOCKOUT_GATER_OW_VAL_E, 0, NULL));
    /* Set AN/PCS mux to AN=0/PCS=1 */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_AN_PCS_CLKOUT_SEL_OW_VAL_E, data, NULL));
    /* Un-gate sd_tx_clk back to COMPHY */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PM_CLOCKOUT_GATER_OW_VAL_E, 1, NULL));

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortANPLinkStatusSet function
* @endinternal
*
* @brief   Overwrite link_status value in ANP (assume
*          IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_LINK_STATUS_OW_E
*          is set on init seq
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortANPLinkStatusSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    GT_BOOL             linkUp
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (linkUp == GT_TRUE)?1:0;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_LINK_STATUS_OW_VAL_E, data, NULL));

    return GT_OK;
}


/**
* @internal mvHwsIronmanAnpPortAnSet function
* @endinternal
*
* @brief   Enable/Disable/Restart AN
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortAnSet
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_ANP_AN_OPER  oper
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 data;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = AN_USX_UNIT;
    }

    if ( oper ==  MV_HWS_ANP_AN_OPER_RESTART)
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_RESTART_802_3AP_AUTO_NEGOTIATION_E, 1, NULL));
    }
    else
    {
         data = (oper == MV_HWS_ANP_AN_OPER_ENABLE)?1:0;
         CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_FIELD_802_3AP_AUTO_NEGOTIATION_ENABLE_E, data, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortHCDGet function
* @endinternal
*
* @brief   return HCD results
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

GT_STATUS mvHwsIronmanAnpPortHCDGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    GT_U32                  *apHcdStatus
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 val;
    MV_HWA_AP_PORT_MODE hcdType = MODE_NOT_SUPPORTED;
    GT_U8 fecType = AP_ST_HCD_FEC_RES_NONE;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(phyPortNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS2_P0_AP_AG_HCD_RESOLVED_S_E, &val, NULL));
    if ( val == 0 )
    {
        AP_ST_HCD_FOUND_SET(*apHcdStatus, 0);
        return GT_OK;
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS1_P0_AP_PWRUP_10GKR_S_E, &val, NULL));
    if(val == 1 )
    {
        hcdType = Port_10GBase_R;
    }
    else
    {
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS1_P0_AP_PWRUP_G_S_E, &val, NULL));
        if(val == 1 )
        {
            hcdType = Port_1000Base_KX;
        }
        else
        {
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS1_P0_AP_PWRUP_2P5G_S_E, &val, NULL));
            if(val == 1 )
            {
                hcdType = Port_2500Base_KX;
            }
            else
            {
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS1_P0_AP_PWRUP_100GCR10_S_E, &val, NULL));
                if(val == 1 )
                {
                    hcdType = Port_100GBASE_CR10;
                }
            }
        }
    }

    if ( hcdType == MODE_NOT_SUPPORTED )
    {
        AP_ST_HCD_FOUND_SET(*apHcdStatus, 0);
        return GT_OK;
    }

    AP_ST_HCD_FOUND_SET(*apHcdStatus, 1);
    AP_ST_HCD_TYPE_SET(*apHcdStatus, hcdType);

    AP_ST_HCD_FC_RX_RES_SET(*apHcdStatus, 0);
    AP_ST_HCD_FC_TX_RES_SET(*apHcdStatus, 0);
    AP_ST_HCD_FEC_RES_SET(*apHcdStatus, 0);

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS2_P0_AP_AG_RX_PAUSE_ENABLE_S_E, &val, NULL));
    if( val )
    {
        AP_ST_HCD_FC_RX_RES_SET(*apHcdStatus, 1);
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS2_P0_AP_AG_TX_PAUSE_ENABLE_S_E, &val, NULL));
    if( val )
    {
        AP_ST_HCD_FC_TX_RES_SET(*apHcdStatus, 1);
    }

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_STATUS2_P0_AP_FEC_ENABLE_S_E, &val, NULL));
    if ( val )
    {
        fecType = AP_ST_HCD_FEC_RES_FC;
        AP_ST_HCD_FEC_RES_SET(*apHcdStatus, fecType);
    }

    return GT_OK;
}

/**
* @internal mvHwsAnpIronmanInit function
* @endinternal
*
* @brief  ANP unit init
*
* @param[in] devNum                - system device number
* @param[in] skipWritingToHW       - skip writing to HW
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIronmanAnpInit
(
    IN GT_U8   devNum,
    IN GT_BOOL skipWritingToHW
)
{
    GT_U32 portNum;
    MV_HWS_UNITS_ID unitId;
    GT_U32 regValue/*field*/;

#ifndef  IRONMAN_DEV_SUPPORT
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Start of mvHwsIronmanAnpInit ******\n");
    }
#endif

#ifndef IRONMAN_DEV_SUPPORT
    if (skipWritingToHW == GT_TRUE)
    {
        return GT_OK;
    }

    /*skip hw write in case of HA or HS */
    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
    if (!(((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
          (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
          (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)))
#endif
    {
        for(portNum = 0; portNum < hwsDeviceSpecInfo[devNum].portsNum; portNum++)
        {
            if(mvHwsMtipIsReducedPort(devNum, portNum))
            {
                unitId = ANP_CPU_UNIT;
            }
            else
            {
                unitId = ANP_USX_UNIT;
                if(portNum % 4)
                {
                    continue;
                }
            }


            /*********************/
            /* ANP init sequence */
            /*********************/
            /* Ensure PWM clock is enabled, and soft reset is de-asserted */
            /**************************************************************/
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E, 1, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_PWM_CLK_EN_E, 1, NULL));

            /* Ow_val signals */
            /******************/
            /* AN + TFIFO TX - reset */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_TX_OW_VAL_E, 1, NULL));
            /* AN + TFIFO TX - clock gate */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_VAL_E, 0, NULL));
            /* AN + TFIFO RX - reset */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_RX_OW_VAL_E, 1, NULL));
            /* AN + TFIFO RX - clock gate */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_VAL_E, 0, NULL));

            /* AN/PCS data & "sd_tx_clk towards COMPHY" mux control (set to PCS)*/
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_AN_PCS_CLKOUT_SEL_OW_VAL_E, 0, NULL));/*common (same value at init)*/
            /* Link_status towards AN */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_LINK_STATUS_OW_VAL_E, 0, NULL));
            /* Sd_tx un-reset towards PCS */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E, 1, NULL));/*common - set correct value for non-AN*/
            /* Un-gate PCS sd tx clock */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E, 1, NULL));/*common - set correct value for non-AN*/
            /* Sd_rx un-reset towards PCS */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E, 1, NULL));/*common - set correct value for non-AN*/
            /* Un-gate PCS sd rx clock */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E, 1, NULL));/*common - set correct value for non-AN*/

            /* AN enable (set to 0) towards PWM */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_VAL_E, 0, NULL));
            /* Dsp_lock towards AN */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_DSP_LOCK_OW_VAL_E, 1, NULL));

            /* Sd_tx_clk towards COMPHY gater */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PM_CLOCKOUT_GATER_OW_VAL_E, 1, NULL));/*common  - set correct value for non-AN*/
            /* Sd_sw_resetn */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_SD_SW_RESETN_OW_VAL_E, 1, NULL));


            /* Ow signals (only after these are set, the values are taken from Ow_val signals, and not from HW)*/
            /***************************************************************************************************/
            /* AN + TFIFO TX - reset */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_TX_OW_E, 1, NULL));
            /* AN + TFIFO TX - clock gate */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_E, 1, NULL));
            /* AN + TFIFO RX - reset */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_RX_OW_E, 1, NULL));
            /* AN + TFIFO RX - clock gate */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_E, 1, NULL));

            /* AN/PCS data & "sd_tx_clk towards COMPHY" mux control (set to PCS)*/
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_AN_PCS_CLKOUT_SEL_OW_E, 1, NULL));
            /* Link_status towards AN */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_LINK_STATUS_OW_E, 1, NULL));
            /* Sd_tx reset towards PCS */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_E, 1, NULL));
            /* Sd_tx clock enable towards PCS */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_E, 1, NULL));
            /* Sd_rx reset towards PCS */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_E, 1, NULL));
            /* Sd_rx clock enable towards PCS */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E, 1, NULL));

            /* AN enable (set to 0) towards PWM */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_E, 1, NULL));
            /* Dsp_lock towards AN */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_DSP_LOCK_OW_E, 1, NULL));

            /* Sd_tx_clk towards COMPHY gater */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PM_CLOCKOUT_GATER_OW_E, 1, NULL));
            /* Sd_sw_resetn */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_SD_SW_RESETN_OW_E, 1, NULL));

             /* other configuration*/
            /***************************************************************************************************/
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_REG_SIGDET_MODE_E, 0, NULL));/*common (same value)*/
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_TIED_IN_P0_RING_OSC_A_E, 1, NULL));

            /***************************************************************************/
            /* default SD_MUX configuration is:
             * port0  = SD_0
             * port4  = SD_1
             * port24 = SD_2
             * port28 = SD_3(not connected)
             * port52 = SD_4(not connected) */
            /***************************************************************************/
            if((portNum >= 48) && (portNum <= 51))
            {
                /* do nothing*/
            }
            else
            {
                if(portNum > 51) /* 52..54 */
                {
                    regValue = 0x4;
                    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_CPU_UNIT, IRONMAN_ANP_UNIT_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, regValue, NULL));
                }
                else
                {
                    if((portNum % 8) == 0) /*0,8,16,24,32,40*/
                    {
                        /* 0,8,16 mapped to SD_0
                           24,32,40 mapped to SD_2*/
                        regValue = 0x0 + (portNum / 24) * 2;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_USX_UNIT, IRONMAN_ANP_UNIT_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, regValue, NULL));
                    }
                    else if((portNum % 8) == 4)
                    {
                        /* 4,12,20 mapped to SD_1
                           28,36,44 mapped to non-existing SD_3*/
                        regValue = 0x1 + (portNum / 24) * 2;
                        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, ANP_USX_UNIT, IRONMAN_ANP_UNIT_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E, regValue, NULL));
                    }
                }
            }
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsIronmanAnpInit ******\n");
    }
#endif
     return GT_OK;
}

/**
* @internal mvHwsIronmanAnpSerdesSdwMuxSet function
* @endinternal
*
* @brief  ANP unit enable SerDes control
*
* @param[in] devNum    - system device number
* @param[in] serdesNum - system SerDes number
* @param[in] anpEnable - true  - SerDes is controlled by ANP
*                      - false - SerDes is controlled by regs
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIronmanAnpSerdesSdwMuxSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               serdesNum,
    IN  GT_BOOL              anpEnable
)
{
    MV_HWS_REG_ADDR_FIELD_STC fieldReg = {0, 0, 0, 0};
    GT_U32                    fieldName;
    GT_UREG_DATA              regAddr;

    GT_UNUSED_PARAM(anpEnable);

    /* lane Mux Control */
    fieldName = IRONMAN_SDW_LANE_MUX_CONTROL_PU_PLL_SELECT_E;
    regAddr = genUnitFindAddressPrv(devNum, serdesNum, SERDES_UNIT, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0x7FFFF, 0x7FFFF));

    /* General_Mux_Control */
    fieldName = IRONMAN_SDW_COMMON_GENERAL_MUX_CONTROL_PU_IVREF_SELECT_E;
    regAddr = genUnitFindAddressPrv(devNum, serdesNum, SERDES_UNIT, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, regAddr, 0xF, 0xF));

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortEnable function
* @endinternal
*
* @brief  ANP unit port enable
*
* @param[in] devNum                - system device number
* @param[in] portNum               - system port number
* @param[in] portMode              - configured port mode
* @param[in] apEnable              - apEnable flag
* @param[in] enable                - True = enable/False =
*       disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIronmanAnpPortEnable
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_PORT_STANDARD portMode,
    GT_BOOL              apEnable,
    GT_BOOL              enable
)
{
    GT_U32 hwSdMuxValue, regValue, regValue1, startArrayIndex, i;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    MV_HWS_IRONMAN_ANP_UNITS anpRegName;
    GT_STATUS       rc;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_UNITS_ID swapUnitId = ANP_USX_UNIT;
    GT_U32 swapPortNum = 0;
    MV_HWS_PORT_IN_GROUP_ENT isFirst, isLast;
    MV_HWS_PORT_INIT_PARAMS  curPortParams;
    GT_U8  gopNum;
    static const GT_U32 anp2portConvert[] = {0,4,24,28,52, 8,12,32,36,53, 16,20,40,44,54};

    GT_UNUSED_PARAM(apEnable);

#ifdef GM_USED
    return GT_OK;
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsIronmanAnpPortEnable ******\n");
    }
#endif

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        unitId = ANP_CPU_UNIT;
    }
    else if(mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        unitId = ANP_USX_UNIT;
    }
    else
    {
        return GT_BAD_PARAM;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    if (rc != GT_OK)
    {
        return GT_BAD_PARAM;
    }

    if (enable == GT_FALSE)
    {
        return GT_OK;
    }

    if (apEnable)
    {
        /* AN + TFIFO TX - reset */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_TX_OW_VAL_E, 1, NULL));
        /* AN + TFIFO TX - clock gate */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FT_OW_VAL_E, 0, NULL));
        /* AN + TFIFO RX - reset */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_AP_RESET_RX_OW_VAL_E, 1, NULL));
        /* AN + TFIFO RX - clock gate */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PM_ENCLK_AP_FR_OW_VAL_E, 0, NULL));

        /* AN/PCS data & "sd_tx_clk towards COMPHY" mux control (set to PCS)*/
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_AN_PCS_CLKOUT_SEL_OW_VAL_E, 0, NULL));
        /* Link_status towards AN */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_LINK_STATUS_OW_VAL_E, 0, NULL));
        /* Sd_tx un-reset towards PCS */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E, 0, NULL));
        /* Un-gate PCS sd tx clock */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E, 0, NULL));
        /* Sd_rx un-reset towards PCS */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E, 0, NULL));
        /* Un-gate PCS sd rx clock */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E, 0, NULL));

        /* AN enable (set to 0) towards PWM */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_VAL_E, 0, NULL));
        /* Dsp_lock towards AN */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_DSP_LOCK_OW_VAL_E, 1, NULL));

        /* Sd_tx_clk towards COMPHY gater */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PM_CLOCKOUT_GATER_OW_VAL_E, 0, NULL));
        /* Sd_sw_resetn */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_SD_SW_RESETN_OW_VAL_E, 1, NULL));
    }
    else
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_RX_RESETN_OW_VAL_E,  1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_SD_TX_RESETN_OW_VAL_E,  1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_VAL_E,  1, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_TX_CLK_ENA_OW_VAL_E,  1, NULL));
        /* AN/PCS data & "sd_tx_clk towards COMPHY" mux control (set to PCS)*/
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_AN_PCS_CLKOUT_SEL_OW_VAL_E, 0, NULL));
        /* Sd_tx_clk towards COMPHY gater */
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PM_CLOCKOUT_GATER_OW_VAL_E, 1, NULL));
    }


    if(mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        CHECK_STATUS(mvHwsExtIfFirstInSerdesGroupCheck(devNum,portNum,portMode, &isFirst));
        CHECK_STATUS(mvHwsExtIfLastInSerdesGroupCheck(devNum,portNum,portMode, &isLast));
        if((isFirst != MV_HWS_PORT_IN_GROUP_FIRST_E) && (isLast != MV_HWS_PORT_IN_GROUP_LAST_E))
        {
            return GT_OK;
        }
    }

    /* No SerDes swap on indexes 9-12 */
    if((portNum > 47) && (portNum < 52))
    {
        return GT_OK;
    }

    rc = hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch(portMode)
    {
        case _5GBaseR:
        case _10GBase_KR:
        case _10GBase_SR_LR:
        case _100Base_FX:
        case SGMII:
        case _1000Base_X:
        case SGMII2_5:
        case _2500Base_X:
        case _10G_SXGMII:
        case _5G_SXGMII:
        case _2_5G_SXGMII:
            if(portNum < 48)
            {
                if((portNum % 8) == 0)
                {
                    hwSdMuxValue = 0;
                }
                else /*if((portNum % 8) == 4))*/
                {
                    hwSdMuxValue = 1;
                }
            }
            else
            {
                hwSdMuxValue = 2;
            }
            break;

        case _10G_OUSGMII:
            if(portNum < 24)
            {
                hwSdMuxValue = 0;
            }
            else
            {
                hwSdMuxValue = 1;
            }
            break;

        case QSGMII:
        case _5G_QUSGMII:
        case _10G_QXGMII:
            if((portNum % 8) < 4)
            {
                hwSdMuxValue = 1;
            }
            else
            {
                /* check sd mux mode */
                gopNum = curPortParams.firstLaneNum / 3; /* each GOP 3 SerDeses (SerDes 0-2, SerDes 3-5, SerDes 6-8) */

                if (gopNum > 2)
                {
                    return GT_OUT_OF_RANGE;
                }

                if (hwsDeviceSpecInfo[devNum].gopMuxMode[gopNum] == 1)
                {
                    hwSdMuxValue = 2;
                }
                else if (hwsDeviceSpecInfo[devNum].gopMuxMode[gopNum] == 0)
                {
                    hwSdMuxValue = 0;
                }
                else
                {
                    return GT_BAD_VALUE;
                }

            }
            break;

        default:
            return GT_BAD_PARAM;
    }

    anpRegName = IRONMAN_ANP_UNIT_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, anpRegName,  &regValue, NULL));

    if(regValue == hwSdMuxValue)
    {
        /* nothing to be done */
        return GT_OK;
    }

    startArrayIndex = (portNum < 48) ? (convertIdx.ciderUnit * 5) : ((convertIdx.ciderUnit - 3) * 5);

    for(i = startArrayIndex; i < (startArrayIndex+5); i++)
    {
        swapPortNum = anp2portConvert[i];
        if(swapPortNum == portNum)
        {
            continue;
        }
        if(mvHwsMtipIsReducedPort(devNum, swapPortNum))
        {
            swapUnitId = ANP_CPU_UNIT;
        }
        else if (mvHwsUsxModeCheck(devNum, swapPortNum, _10GBase_KR))
        {
            swapUnitId = ANP_USX_UNIT;
        }
        else
        {
            return GT_BAD_PARAM;
        }
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, swapPortNum, swapUnitId, anpRegName,  &regValue1, NULL));

        if(regValue1 == hwSdMuxValue)
        {
            break;
        }
    }
    if(i == (startArrayIndex + 5))
    {
        return GT_BAD_STATE;
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum,     unitId,     anpRegName,  regValue1, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, swapPortNum, swapUnitId, anpRegName,  regValue, NULL));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsIronmanAnpPortEnable ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortReset function
* @endinternal
*
* @brief   reset anp machine
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] reset              - reset parameter
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortReset
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_BOOL              reset
)
{
    GT_U32 data;
    MV_HWS_UNITS_ID unitId;

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    data = (reset == GT_FALSE)?1:0;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_P0_AN_HW_SOFT_RESET__E,  0, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_P0_AN_SW_SOFT_RESET__E,  0, NULL));

    if ( reset == GT_FALSE) {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_P0_AN_HW_SOFT_RESET__E,  data, NULL));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_P0_AN_SW_SOFT_RESET__E,  data, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortCapabilitiesReg1Set function
* @endinternal
*
* @brief   set anp capability first register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] options            - other advertisment
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsIronmanAnpPortCapabilitiesReg1Set
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    GT_U32              options
)
{
    GT_U32          fieldName;

    if (AP_CTRL_FC_PAUSE_GET(options)) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_PAUSE_CAPABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_FC_ASM_GET(options)) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_ASYMMETRIC_PAUSE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortCapabilitiesReg2Set function
* @endinternal
*
* @brief   set anp capability second register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] capability         - capabilitis
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsIronmanAnpPortCapabilitiesReg2Set
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    GT_U32              capability
)
{
    GT_U32          fieldName;
    GT_U32          enable;


    /* in case of 1000BaseX the default wrong so we need to unset*/
    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_1000BASE_KX__E;
    enable = (AP_CTRL_1000Base_KX_GET(capability))? 1:0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, enable, NULL));

    /* in case of 10GBaseKR the default wrong so we need to unset*/
    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_10GBASE_KR_E;
    enable = (AP_CTRL_10GBase_KR_GET(capability))? 1:0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, enable, NULL));

    /*12G KR uses 100GBASE_CR10 bit for advertisment - marvell proprietary*/
    if (AP_CTRL_100GBASE_CR10_GET(capability))
    {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_100GBASE_CR10_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortCapabilitiesReg3Set function
* @endinternal
*
* @brief   set anp capability third register
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitId             - unit id
* @param[in] options            - other advertisment
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsIronmanAnpPortCapabilitiesReg3Set
(
    GT_U8               devNum,
    GT_U32              portNum,
    MV_HWS_UNITS_ID     unitId,
    GT_U32              options,
    GT_U32              capability
)
{
    GT_U32          fieldName;


    if (AP_CTRL_2500Base_KX_GET(capability)) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_2_5G_KX_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    if (AP_CTRL_FEC_ABIL_GET(options)) {

        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_FEC_ABILITY_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }
    if (AP_CTRL_FEC_REQ_GET(options)) {
        fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_REQUESTING_FEC_ENABLE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, fieldName, 1, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortCapabilities function
* @endinternal
*
* @brief   set anp capabilities
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
* @param[in] unitIdAn           - AN Unit
* @param[in] capability         - capabilitis
* @param[in] options            - other advertisment
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS mvHwsIronmanAnpPortCapabilities
(
    GT_U8                devNum,
    GT_U32               portNum,
    MV_HWS_UNITS_ID      unitIdAn,
    MV_HWS_UNITS_ID      unitIdAnp,
    GT_U32               capability,
    GT_U32               options
)
{
    GT_U32          fieldName, data;
    unitIdAnp = unitIdAnp;

    if (AP_CTRL_LB_EN_GET(options))
    {
        /*m_RAL.m_RAL_AN[n].an_units_RegFile.ANEG_LANE_0_CONTROL_REGISTER_1.ow_as_nonce_match_s.set(1);
          m_RAL.m_RAL_AN[n].an_units_RegFile.ANEG_LANE_0_CONTROL_REGISTER_1.rg_as_nonce_match_s.set(0); */
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_AS_NONCE_MATCH_S_E;
        data = 0;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName,  data, NULL));
        fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_OW_AS_NONCE_MATCH_S_E;
        data = 1;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName,  data, NULL));
    }

    /* Writing to the Base page registers = 3 registers of 16 bit each.
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3.write(status, bp[47:32]);
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2.write(status, bp[31:16]);
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1.write(status, bp[15:0]); */
    mvHwsIronmanAnpPortCapabilitiesReg3Set(devNum, portNum, unitIdAn, options, capability);
    mvHwsIronmanAnpPortCapabilitiesReg2Set(devNum, portNum, unitIdAn, capability);
    mvHwsIronmanAnpPortCapabilitiesReg1Set(devNum, portNum, unitIdAn, options);


    /* Write ENABLE AN
    m_RAL.m_RAL_AN[n].an_units_RegFile.REG_802_3AP_AUTO_NEGOTIATION_CONTROL.field_802_3ap_auto_negotiation_enable.set(1);*/
    fieldName = AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_FIELD_802_3AP_AUTO_NEGOTIATION_ENABLE_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAn, fieldName, 1, NULL));

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortLinkTimerSet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortLinkTimerSet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  timeInMsec
)
{
    GT_U32  value;
    MV_HWS_UNITS_ID unitId;

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
    }
    else
    {
        if(phyPortNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = AN_USX_UNIT;
    }

    if ( HWS_PAM4_MODE_CHECK(portMode) )
    {
        /*  Def_3150ms is set according to the configuration "rg_link_fail_timer_sel_s[1:0]":
            2'b00: 3150ms. 2'b01: 6s. 2'b10: 9s. 2'b11: 12s.
            Default is 2'b00.
            In order to change, set desired value in bits[10:9] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1 */
        if ( timeInMsec <= 3200) {
            value = 0;
        }
        else if ( timeInMsec <= 6000 ) {
            value = 1;
        }
        else if ( timeInMsec <= 9000 ) {
            value = 2;
        }
        else /*if ( timeInMsec < 6000 ) */{
            value = 3;
        }
        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E;*/

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E, value, NULL));
    }
    else if ( portMode ==_1000Base_X)
    {
        /*Def_50ms is set according to the configuration "rg_link_fail_timer_sel_s[5:4]":
            2'b00: 50ms. 2'b01: 100ms. 2'b10: 200ms. 2'b11: 500ms. Default is 2'b00.
            In order to change, set desired value in bits[14:13] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/
        if ( timeInMsec <= 50) {
            value = 0;
        }
        else if ( timeInMsec <= 100 ) {
            value = 1;
        }
        else if ( timeInMsec <= 200 ) {
            value = 2;
        }
        else /*if ( timeInMsec <= 500 ) */{
            value = 3;
        }
        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E;*/

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E, value, NULL));
    }
    else
    {
        /*Def_500ms is set according to the configuration "rg_link_fail_timer_sel_s[3:2]":
            2'b00: 500ms. 2'b01: 1s. 2'b10: 2s. 2'b11: 5s. Default is 2'b00.
            In order to change, set desired value in bits[12:11] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/
        if ( timeInMsec <= 500) {
            value = 0;
        }
        else if ( timeInMsec <= 1000 ) {
            value = 1;
        }
        else if ( timeInMsec <= 2000 ) {
            value = 2;
        }
        else /*if ( timeInMsec <= 5000 ) */{
            value = 3;
        }
        /*fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E;*/

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E, value, NULL));
    }

    return GT_OK;
}


/**
* @internal mvHwsronmanAnpPortLinkTimerGet function
* @endinternal
*
* @brief   set link up timer.
*
* @param[in] devNum          - system device number
* @param[in] portNum         - Physical Port Number
* @param[in] portMode        - required port mode
* @param[in] timeInMsec      - link up timeout.
*   supported timeout: slow speed - 50ms,100ms,200ms,500ms.
*                      nrz- 500ms, 1s, 2s, 5s.
*                      pam4- 3150ms, 6s, 9s, 12s.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortLinkTimerGet
(
    GT_U8                   devNum,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    GT_U32                  *timeInMsecPtr
)
{
    GT_U32          value;
    MV_HWS_UNITS_ID unitId;

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
    }
    else
    {
        if(phyPortNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = AN_USX_UNIT;
    }

    if ( HWS_PAM4_MODE_CHECK(portMode))
    {
        /*  Def_3150ms is set according to the configuration "rg_link_fail_timer_sel_s[1:0]":
            2'b00: 3150ms. 2'b01: 6s. 2'b10: 9s. 2'b11: 12s.
            Default is 2'b00.
            In order to change, set desired value in bits[10:9] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1 */

        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL1500_S_E, &value, NULL));

        switch ( value) {
        case 1:
            *timeInMsecPtr = 6000; break;
        case 2:
            *timeInMsecPtr = 9000; break;
        case 3:
            *timeInMsecPtr = 12000; break;
        case 0:
        default:
            *timeInMsecPtr = 3200; break;
        }
    }
    else if ( portMode ==_1000Base_X)
    {
        /*Def_50ms is set according to the configuration "rg_link_fail_timer_sel_s[5:4]":
            2'b00: 50ms. 2'b01: 100ms. 2'b10: 200ms. 2'b11: 500ms. Default is 2'b00.
            In order to change, set desired value in bits[14:13] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/

        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL50_S_E, &value, NULL));

        switch ( value) {
        case 1:
            *timeInMsecPtr = 100; break;
        case 2:
            *timeInMsecPtr = 200; break;
        case 3:
            *timeInMsecPtr = 500; break;
        case 0:
        default:
            *timeInMsecPtr = 50; break;
        }
    }
    else
    {
        /*Def_500ms is set according to the configuration "rg_link_fail_timer_sel_s[3:2]":
            2'b00: 500ms. 2'b01: 1s. 2'b10: 2s. 2'b11: 5s. Default is 2'b00.
            In order to change, set desired value in bits[12:11] of following AN register:
            /Cider/External IP/Soft IP/ANP/AN/AN 1.0 (hawk1)/AN {RTLF 8.1 200120.0}/AN/AN Units/ANEG_LANE_0_CONTROL_1*/

        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0,  phyPortNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_1_RG_LINK_FAIL_TIMER_SEL500_S_E, &value, NULL));

        switch ( value) {
        case 1:
            *timeInMsecPtr = 1000; break;
        case 2:
            *timeInMsecPtr = 2000; break;
        case 3:
            *timeInMsecPtr = 5000; break;
        case 0:
        default:
            *timeInMsecPtr = 500; break;
        }
    }
    return GT_OK;
}

#if 0
/**
* @internal mvHwsHawkLocalIndexToAnpIndexConvert function
* @endinternal
*
* @brief  ANP convert fuction
*
*
************************************************************************************************************************
* portNum      | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |20 |21 |22 |23 |24 |25 |
************************************************************************************************************************
* convert portNum ==> localIdx  according to portMode
************************************************************************************************************************
* localIdx 400 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | -----------------------------------------------------------------------
* localIdx USX | - | - | 0 | 2 | 1 | 3 | 4 | 6 | 5 | 7 | 8 |10 | 9 |11 |12 |14 |13 |15 |16 |18 |17 |19 |20 |22 |21 |23 |
************************************************************************************************************************
* convert localIdx ==> anpIdx  according to portMode
************************************************************************************************************************
* anpIdx 400   | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | -----------------------------------------------------------------------
* anpIdx USX   | - | - | 8 | 8 | 8 | 8 | 9 | 9 | 9 | 9 |10 |10 |10 |10 |11 |11 |11 |11 |12 |12 |12 |12 |13 |13 |13 |13 |
* anpIdx USX_O | - | - | 8 | 8 | 8 | 8 | 8 | 8 | 8 | 8 |10 |10 |10 |10 |10 |10 |10 |10 |12 |12 |12 |12 |12 |12 |12 |12 |
************************************************************************************************************************
*/
GT_STATUS mvHwsHawkLocalIndexToAnpIndexConvert /*TBD*/
(
    GT_U8                       devNum,
    GT_U32                      portNum,
    IN GT_U32                   localIdx,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_U32                  *anpIdxPtr
)
{
#ifndef HARRIER_DEV_SUPPORT
    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        if (localIdx >=24)
        {
            return GT_NO_SUCH;
        }
        if (HWS_USX_O_MODE_CHECK(portMode))
        {
            *anpIdxPtr = (8 + (localIdx/8)*2);
        }
        else
        {
            *anpIdxPtr = (8 + localIdx/4);
        }
    }
    else
#endif
    {
        if (localIdx >=8)
        {
            return GT_NO_SUCH;
        }
        *anpIdxPtr = localIdx;
    }
    return GT_OK;
}
#endif

GT_STATUS mvHwsIronmanAnpSetDefaultLinkTimer
(
    GT_U8  devNum,
    GT_U32 port,
    GT_U32 capability
)
{
    GT_U32 timer = 0;

    if ((AP_CTRL_1000Base_KX_GET(capability)) || (AP_CTRL_2500Base_KX_GET(capability)))
    {
        timer = 500;
        mvHwsIronmanAnpPortLinkTimerSet(devNum,port,_1000Base_X, timer);
    }
    if (AP_CTRL_10GBase_KR_GET(capability) || (AP_CTRL_100GBASE_CR10_GET(capability)))
    {
        timer = 1000;
        mvHwsIronmanAnpPortLinkTimerSet(devNum,port,_10GBase_KR, timer);
    }

    return GT_OK;
}

GT_STATUS mvHwsIronmanAnpSetIntropLinkTimer
(
    GT_U8  devNum,
    GT_U32 port,
    GT_U32 capability,
    GT_U32 pdTimer,
    GT_U32 apTimer

)
{
    if ((AP_CTRL_1000Base_KX_GET(capability)) || (AP_CTRL_2500Base_KX_GET(capability)))
    {
        mvHwsIronmanAnpPortLinkTimerSet(devNum,port,_1000Base_X, pdTimer);
    }
    if (AP_CTRL_10GBase_KR_GET(capability) || (AP_CTRL_100GBASE_CR10_GET(capability)))
    {
        mvHwsIronmanAnpPortLinkTimerSet(devNum,port,_10GBase_KR, apTimer);
    }

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortStopAn function
* @endinternal
*
* @brief   Port anp stop autoneg
*
* @param[in] devNum             - system device number
* @param[in] phyPortNum         - Physical Port Number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortStopAn
(
    GT_U8                devNum,
    GT_U32               phyPortNum
)
{
    CHECK_STATUS(mvHwsIronmanAnpPortReset(devNum, phyPortNum, GT_TRUE));
    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortStart function
* @endinternal
*
* @brief   Port anp start (set capabilities and start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanAnpPortStart
(
    GT_U8                devNum,
    GT_U32               portNum,
    GT_U32               capability,
    GT_U32               options
)
{

    MV_HWS_UNITS_ID unitId,unitIdAnp;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (portNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE )
    {
        unitId = AN_CPU_UNIT;
        unitIdAnp = ANP_CPU_UNIT;
    }
    else
    {
        if(portNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = AN_USX_UNIT;
        unitIdAnp = ANP_USX_UNIT;
    }

    /* "Fix" for SD MUX + init*/
    CHECK_STATUS(mvHwsIronmanAnpPortEnable(devNum, portNum, _10GBase_KR, GT_TRUE, GT_TRUE));

    CHECK_STATUS(mvHwsIronmanAnpPortReset(devNum, portNum, GT_FALSE));

    /* Set AN RX clock & TX clock to gate */
    CHECK_STATUS(mvHwsIronmanAnpPortAnTxRxClockGate(devNum, portNum, GT_TRUE, MV_HWS_ANP_TXRX));
    /* Set AN RX reset & TX reset to Reset*/
    CHECK_STATUS(mvHwsIronmanAnpPortAnTxRxReset(devNum, portNum, GT_TRUE, MV_HWS_ANP_TXRX));

    /* AN enable (set to 0) towards PWM */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitIdAnp, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_VAL_E, 0, NULL));

    /* Set AN/PCS mux to AN*/
    CHECK_STATUS(mvHwsIronmanAnpPortAnPcsMuxSet(devNum, portNum, GT_TRUE));

    /*Set PCS sd tx & rx to reset*/
    CHECK_STATUS(mvHwsIronmanAnpPortPcsSdTxRxReset(devNum, portNum, GT_TRUE));
    /* Set PCS sd tx & rx clock to gated */
    CHECK_STATUS(mvHwsIronmanAnpPortPcsSdTxRxClockGate(devNum, portNum, GT_TRUE));

    /*CHECK_STATUS(mvHwsIronmanAnpPortPcsTxRxReset(0, portNum, GT_TRUE));*/

#if 0
    /* MB - add this to make sure AN ARB SM is in normal opertion/read mode*/
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  portNum, unitId, AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_OVERRIDE_CTRL_S_E, 0, NULL));
#endif

    mvHwsIronmanAnpSetDefaultLinkTimer(devNum, portNum, capability);/*TBD - change to introp timer after bringup*/

    CHECK_STATUS(mvHwsIronmanAnpPortCapabilities(devNum, portNum, unitId, unitIdAnp, capability, options));

    return GT_OK;
}

/**
* @internal mvHwsIronmanAnpPortSignalDetectMask function
* @endinternal
*
* @brief   Mask signal detect.
*
* @param[in] devNum                - system device number
* @param[in] phyPortNum            - physical port
* @param[in] startSeq              - start or end seq
*
* @retval 0                        - on success
* @retval 1                        - on error
*
* • For link up the following settings should be made:
* (sq_detected_lpf_ow) Port<%n> Control4[6] to 1
* (sq_detected_lpf_ow_val) Port<%n> Control4[7] to 0 (default)
*
* • For link down the following settings should be made:*
* (sq_detected_lpf_ow) Port<%n> Control4[6] to 0
*
*/
GT_STATUS mvHwsIronmanAnpPortSignalDetectHwMask
(
    IN  GT_U8                    devNum,
    IN  GT_U32                   phyPortNum,
    IN  GT_BOOL                  mask
)
{
    MV_HWS_UNITS_ID unitId;

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
    }
    else
    {
        if(phyPortNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
    }

    if (mask)
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_SQ_DETECTED_LPF_OW_VAL_E, 0, NULL));
    }

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0,  phyPortNum, unitId, IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_SQ_DETECTED_LPF_OW_E, mask, NULL));

    return GT_OK;
}


GT_STATUS mvHwsIronmanAnpPortRegDump
(
    GT_U8                    devNum,
    GT_U32                   phyPortNum
)
{
#if IRONMAN_DEBUG
    GT_U32          fieldName;
    MV_HWS_UNITS_ID unitId, unitIdAn;
    GT_U32  regAddr, regData;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg = {0, 0, 0, 0};

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if ((devNum >= HWS_MAX_DEVICE_NUM) || (phyPortNum >= HWS_CORE_PORTS_NUM(devNum)))
    {
        return GT_BAD_PARAM;
    }

    if (mvHwsMtipIsReducedPort(devNum, phyPortNum) == GT_TRUE )
    {
        unitId = ANP_CPU_UNIT;
        unitIdAn = AN_CPU_UNIT;
    }
    else
    {
        if(phyPortNum % 4)/*TBD */
        {
            return GT_BAD_PARAM;
        }
        unitId = ANP_USX_UNIT;
        unitIdAn = AN_USX_UNIT;
    }

     /***********ANP**************/
    hwsOsPrintf("------ ANP registers ------\n");
    fieldName = IRONMAN_ANP_UNIT_GLOBAL_CONTROL_AN_AP_TRAIN_TYPE_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_CONTROL(0x%x) :0x%x\n",phyPortNum, regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_GLOBAL_CLOCK_AND_RESET_PWM_SOFT_RESET__E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_CLOCK_AND_RESET(0x%x) :0x%x\n",phyPortNum, regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_GLOBAL_SD_MUX_CONTROL_SD_MUX_CONTROL0_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_SD_MUX_CONTROL(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_GLOBAL_SD_MUX_CONTROL1_SD_MUX_CONTROL4_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_SD_MUX_CONTROL1(0x%x) :0x%x\n",phyPortNum, regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_GLOBAL_CONTROL2_REG_PU_TX_CONF_DELAY_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_CONTROL2(0x%x) :0x%x\n", phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_GLOBAL_CONTROL3_REG_NO_PRE_SELECTOR_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_CONTROL3(0x%x) :0x%x\n", phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_GLOBAL_CONTROL4_REG_SD_DFE_UPDATE_DIS_SAMP_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_GLOBAL_CONTROL4(0x%x) :0x%x\n",  phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_INTERRUPT_CAUSE_P0_PORT_INT_SUM_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_INTERRUPT(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_SERDES_TX_SM_CONTROL_P0_SD_TX_SM_OVERRIDE_CTRL_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_SERDES_TX_SM(0x%x) :0x%x\n",phyPortNum, regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_AN_CONTROL_P0_PHY_GEN_AP_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_AN_CONTROL(0x%x) :0x%x\n",phyPortNum, regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_AN_TIED_IN_P0_PM_AP_ANEG_REMOTE_READY_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_AN_TIED_IN(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_AN_HANG_OUT_P0_AP_IRQ_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_AN_HANG_OUT(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_AN_STATUS1_P0_AP_PWRUP_4X_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_AN_STATUS1(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_AN_STATUS2_P0_AP_AA_CLEAR_HCD_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_AN_STATUS2(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL1_P0_RG_MODE_50GR_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL1(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL2_P0_PM_SD_PHY_GEN_TX_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL2(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL3_P0_PM_PCS_COUPLE_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL3(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL4_P0_PM_AP_EN_S_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL4(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL5_P0_PM_RX_TRAIN_ENABLE_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL5(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL6_P0_PM_PCS_RX_CLK_ENA_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL6(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL7_P0_PLL_READY_RX_CLEAN_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL7(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL8_P0_PM_TRAIN_TYPE_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL8(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL9_P0_INT_ENABLED_OW_VAL_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL9(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL10_P0_PM_PU_RX_REQ_S_OW_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL10(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_CONTROL11_P0_PU_RX_BOTH_IN_IDLE_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_CONTROL11(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_STATUS_P0_REG_TX_TRAIN_COMPLETE_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_STATUS(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_STATUS1_P0_STAT_DSP_RXSTR_REQ_MX_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_STATUS1(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_STATUS2_P0_STAT_PM_MODE_50GR2_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_STATUS2(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_STATUS3_P0_STAT_PM_SD_PU_TX_S_E;
   regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_STATUS3(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_STATUS4_P0_STAT_PM_SD_TX_IDLE_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_STATUS4(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = IRONMAN_ANP_UNIT_PORT0_STATUS5_P0_STAT_PM_CMEM_ADDR_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitId, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port %d: ANP_UNITS_PORT0_STATUS5(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    /***********AN**************/
    hwsOsPrintf("------ AN registers ------\n");
    fieldName = AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0_OVERRIDE_CTRL_S_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: AN_UNITS_ANEG_LANE_0_CONTROL_REGISTER_0(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS_LINK_PARTNER_AUTO_NEGOTIATION_ABILITY_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_STATUS(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_CONTROL_RESERVED_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: AN_UNITS_REG_802_3AP_AUTO_NEGOTIATION_CONTROL(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_1_LD_NEXT_PAGE_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_1(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_2_LD_25GBASE_KR_OR_25GBASE_CR_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_2(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_REGISTER_3_LD_LINK_PARTNER_REQUESTING_FEC_ENABLE_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_3(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_1_LP_NEXT_PAGE_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_1(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_2_LP_25GBASE_KR_OR_25GBASE_CR_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_2(0x%x) :0x%x\n",phyPortNum,regAddr,regData);

    fieldName = AN_UNITS_REG_802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_REGISTER_3_LP_LINK_PARTNER_FEC_ABILITY_E;
    regAddr = genUnitFindAddressPrv(devNum, phyPortNum, unitIdAn, NON_SUP_MODE, fieldName, &fieldReg);
    if(regAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("regAddr is MV_HWS_SW_PTR_ENTRY_UNUSED"));
    }
    CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, 0, regAddr, &regData, 0xFFFFFFFF));
    hwsOsPrintf("port%d: 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_3(0x%x) :0x%x\n",phyPortNum,regAddr,regData);
#else
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(phyPortNum);
#endif

    return GT_OK;
}



