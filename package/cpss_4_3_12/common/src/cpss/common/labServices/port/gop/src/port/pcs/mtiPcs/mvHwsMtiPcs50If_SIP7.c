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
* @file mvHwsMtiPcs50If_SIP7.c
*
* @brief MTI PCS50 interface API
*
* @version   1
********************************************************************************
*/
#ifndef MICRO_INIT
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#else
#include <cpssCommon/cpssPresteraDefs.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>


static char* mvHwsMtiPcs50TypeGetFunc(void)
{
  return "MTI_PCS50";
}

/**
* @internal mvHwsMtiPcs50Reset function
* @endinternal
*
* @brief   Set MTI PCS50 RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs50Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      pcsField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if((POWER_DOWN == action) || (RESET == action) || (PARTIAL_POWER_DOWN == action))
    {
        pcsField = AAS_PCS50_CONTROL1_RESET_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0, NULL));

        pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER25_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

        pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER5_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

        pcsField = AAS_PCS50_VENDOR_PCS_MODE_DISABLE_MLD_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));

        pcsField = AAS_PCS50_VENDOR_PCS_MODE_ENA_CLAUSE49_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));

        pcsField = AAS_PCS50_VENDOR_VL_INTVL_MARKER_COUNTER_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x3FFF, NULL));

        pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x0, NULL));

        pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_ALTER_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x0, NULL));

        pcsField = AAS_PCS50_VL0_0_VL0_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x7690, NULL));

        pcsField = AAS_PCS50_VL0_1_VL0_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x47, NULL));

        pcsField = AAS_PCS50_VL1_0_VL1_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0xC4F0, NULL));

        pcsField = AAS_PCS50_VL1_1_VL1_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0xE6, NULL));

        pcsField = AAS_PCS50_VL2_0_VL2_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x65C5, NULL));

        pcsField = AAS_PCS50_VL2_1_VL2_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x9b, NULL));

        pcsField = AAS_PCS50_VL3_0_VL3_0_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x79A2, NULL));

        pcsField = AAS_PCS50_VL3_1_VL3_1_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x3D, NULL));
    }
    else if (UNRESET == action)
    {
        pcsField = AAS_PCS50_CONTROL1_RESET_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 1, NULL));
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs50Mode function
* @endinternal
*
* @brief   Set the MTI PCS50 mode
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
static GT_STATUS mvHwsMtiPcs50Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_STATUS                   rc;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_PORT_FEC_MODE        portFecMode;
    GT_U32                      markerInterval;
    GT_U32                      pcsField;
    MV_HWS_PORT_MII_TYPE_E      miiType;

    GT_UNUSED_PARAM(attributesPtr);

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    MV_HWS_PORT_MII_TYPE_GET(portMode, miiType);
    if(miiType > _50GMII_E)
    {
        hwsOsPrintf("mvHwsMtiPcs50Mode: bad MII type for port mode %d\n", portMode);
        return GT_BAD_PARAM;
    }

    switch (miiType)
    {
        case _40GMII_E:
        case _50GMII_E:
            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER25_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER5_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_ENA_CLAUSE49_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));
            break;

        case _10GMII_E:
            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER25_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER5_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_ENA_CLAUSE49_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));
            break;


        case _5GMII_E:
            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER25_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER5_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_DISABLE_MLD_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_ENA_CLAUSE49_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));
            break;

        case _25GMII_E:
            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER25_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_HI_BER5_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_DISABLE_MLD_E;
            if(portFecMode == RS_FEC)
            {
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x0, NULL));
            }
            else
            {
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));
            }

            pcsField = AAS_PCS50_VENDOR_PCS_MODE_ENA_CLAUSE49_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x1, NULL));
            break;

        default:
            break;
    }

    switch(portMode)
    {
        case HWS_50G_R1_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
        case HWS_40G_R2_MODE_CASE:
        case HWS_40G_R4_MODE_CASE:
            pcsField = AAS_PCS50_VL0_0_VL0_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x7690, NULL));

            pcsField = AAS_PCS50_VL0_1_VL0_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x47, NULL));

            pcsField = AAS_PCS50_VL1_0_VL1_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0xc4f0, NULL));

            pcsField = AAS_PCS50_VL1_1_VL1_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0xe6, NULL));

            pcsField = AAS_PCS50_VL2_0_VL2_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x65c5, NULL));

            pcsField = AAS_PCS50_VL2_1_VL2_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x9b, NULL));

            pcsField = AAS_PCS50_VL3_0_VL3_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x79a2, NULL));

            pcsField = AAS_PCS50_VL3_1_VL3_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x3d, NULL));
            break;

        case _5GBaseR:
        case HWS_10G_MODE_CASE:
            /*do nothing*/
            break;

        case HWS_25G_MODE_CASE:
            pcsField = AAS_PCS50_VL0_0_VL0_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x68c1, NULL));

            pcsField = AAS_PCS50_VL0_1_VL0_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x21, NULL));

            pcsField = AAS_PCS50_VL1_0_VL1_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0xc4f0, NULL));

            pcsField = AAS_PCS50_VL1_1_VL1_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0xe6, NULL));

            pcsField = AAS_PCS50_VL2_0_VL2_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x65c5, NULL));

            pcsField = AAS_PCS50_VL2_1_VL2_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x9b, NULL));

            pcsField = AAS_PCS50_VL3_0_VL3_0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x79a2, NULL));

            pcsField = AAS_PCS50_VL3_1_VL3_1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, 0x3d, NULL));
            break;

        default:
            return GT_BAD_PARAM;
    }

    rc = mvHwsAasUnitMarkerIntervalCalc(devNum, portMode, portFecMode, 0, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }
    pcsField = AAS_PCS50_VENDOR_VL_INTVL_MARKER_COUNTER_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS50_UNIT, pcsField, markerInterval, NULL));


    /*m_int_manager.RS_FEC_pad()*/
    switch(portMode)
    {
        case HWS_50G_R1_MODE_CASE:
            pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x1, NULL));

            pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_ALTER_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x1, NULL));
            break;

        case HWS_40G_R2_MODE_CASE:
            if(portFecMode == RS_FEC)
            {
                pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x1, NULL));

                pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_ALTER_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x1, NULL));
            }
            break;

        case HWS_25G_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
            if(portFecMode == RS_FEC)
            {
                pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_VALUE_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x1, NULL));

                pcsField = AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_TC_PAD_ALTER_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT, pcsField, 0x0, NULL));
            }
            break;

        case _5GBaseR:
        case HWS_10G_MODE_CASE:
            /*do nothing*/
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50LoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs50LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    GT_U32                      data;
    GT_U32                      pcsField;
    GT_U32                      extField;

    GT_UNUSED_PARAM(portMode);

    pcsField = AAS_PCS50_CONTROL1_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_PCS50_UNIT, pcsField, data, NULL));

    extField = AAS_MAC_PORT_EXT_CONTROL1_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs50LoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_U32 data;
    GT_U32 pcsField;

    GT_UNUSED_PARAM(portMode);

    pcsField = AAS_PCS50_CONTROL1_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, MTI_PCS50_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MTI PCS50.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - port number
* @param[in] portMode                 - port mode
*
* @param[out] lock                     - true or false.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs50AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_U32  data;
    GT_U32  extField;

    GT_UNUSED_PARAM(portMode);

    extField = AAS_PCS800_EXT_PORT0_STATUS_P0_LINK_STATUS_E;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, MTI_PCS_EXT_UNIT, extField, &data, NULL));
    *lock = (data != 0);

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50SendFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending fault signals to partner.
*         on single lane, the result will be local-fault on the sender and remote-fault on the receiver,
*         on multi-lane there will be local-fault on both sides, and there won't be align lock
*         at either side.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs50SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               phyPortNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_U32  data;
    GT_U32  extField;

    GT_UNUSED_PARAM(portMode);

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

    extField = AAS_MAC_PORT_EXT_CONTROL_TX_REM_FAULT_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs50FecConfigGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_FEC_MODE    *portFecTypePtr
)
{
    if(portFecTypePtr == NULL)
    {
        return GT_BAD_PTR;
    }
    return  mvHwsExtFecTypeGet(devNum, portGroup, phyPortNum, portMode, portFecTypePtr);
}

/**
* @internal mvHwsMtiPcs50CheckGearBox function
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
static GT_STATUS mvHwsMtiPcs50CheckGearBox
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lockPtr
)
{
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_U32                      data;
    GT_U32                      pcsField;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* for RS_FEC modes 'block_lock' field is not relevant */
    if((curPortParams.portFecMode == RS_FEC) || (curPortParams.portFecMode == RS_FEC_544_514))
    {
        CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx));

        pcsField = AAS_PCS800_EXT_GLOBAL_AMPS_LOCK_STATUS2_AMPS_LOCK_E;

        CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, MTI_PCS_EXT_UNIT, pcsField, &data, NULL));
        data = (data >> convertIdx.ciderIndexInUnit) & ((1 << curPortParams.numOfActLanes) - 1);

        *lockPtr = ((GT_U16)data == ((1 << curPortParams.numOfActLanes) - 1)) ? GT_TRUE : GT_FALSE;
    }
    else
    {
        pcsField = AAS_PCS50_BASER_STATUS1_BLOCK_LOCK_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, MTI_PCS50_UNIT, pcsField, &data, NULL));
        *lockPtr = (data & 0x1) ? GT_TRUE : GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50SendLocalFaultSet function
* @endinternal
*
* @brief   Configure the PCS to start or stop sending local fault signals to partner.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - pcs number
* @param[in  portMode                 - port mode
* @param[in] send                     - start/ stop send faults
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs50SendLocalFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               phyPortNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_U32  data;
    GT_U32  extField;

    GT_UNUSED_PARAM(portMode);

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

    extField = AAS_MAC_PORT_EXT_CONTROL_TX_REM_FAULT_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, portGroup, phyPortNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50RxReset function
* @endinternal
*
* @brief   MTI PCS50 SD RX RESET/UNRESET action.
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
static GT_STATUS mvHwsMtiPcs50RxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    GT_STATUS                   rc;
    GT_U32                      regData = 0, dataToWrite = 0, i;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      extField;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_UNITS_ID             unitId = MTI_PCS_EXT_UNIT;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if(action == UNRESET)
    {
        for (i = 0; i < curPortParams.numOfActLanes; i++)
        {
            dataToWrite |= (1 <<(i));
        }
    }
    else
    {
        dataToWrite = 0;
    }

    extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL_GC_SD_RX_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, unitId, extField, &regData, NULL));
    U32_SET_FIELD(regData, (phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS), curPortParams.numOfActLanes, dataToWrite);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, unitId, extField, regData, NULL));

    return GT_OK;
}

/**
 * @internal mvHwsMtiPcs50RxResetGet function
 * @endinternal
 *
 * @brief   Get the PCS sd_rx_reset state.
 *
 * @param devNum
 * @param portGroup
 * @param phyPortNum
 * @param portMode
 * @param action
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsMtiPcs50RxResetGet
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            *state
)
{
    GT_STATUS                   rc;
    GT_U32                      regData = 0, mask = 0, data, i;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      extField;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_UNITS_ID             unitId = MTI_PCS_EXT_UNIT;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        mask |= (1 <<(i));
    }

    extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL_GC_SD_RX_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, unitId, extField, &regData, NULL));
    data = U32_GET_FIELD(regData, (phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS), curPortParams.numOfActLanes);

    *state = (data == mask) ? UNRESET : RESET;

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs50IfInit function
* @endinternal
*
* @brief   Init MTI PCS50 for SIP7 devices configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs50_SIP7IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_50])
    {
        funcPtrArray[MTI_PCS_50] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_50])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_50], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_50]->pcsResetFunc     = mvHwsMtiPcs50Reset;
    funcPtrArray[MTI_PCS_50]->pcsRxResetFunc   = mvHwsMtiPcs50RxReset;
    funcPtrArray[MTI_PCS_50]->pcsRxResetGetFunc = mvHwsMtiPcs50RxResetGet;
    funcPtrArray[MTI_PCS_50]->pcsModeCfgFunc   = mvHwsMtiPcs50Mode;
    funcPtrArray[MTI_PCS_50]->pcsLbCfgFunc     = mvHwsMtiPcs50LoopBack;
    funcPtrArray[MTI_PCS_50]->pcsLbCfgGetFunc = mvHwsMtiPcs50LoopBackGet;
    funcPtrArray[MTI_PCS_50]->pcsTypeGetFunc   = mvHwsMtiPcs50TypeGetFunc;
    funcPtrArray[MTI_PCS_50]->pcsFecCfgGetFunc = mvHwsMtiPcs50FecConfigGet;
    funcPtrArray[MTI_PCS_50]->pcsCheckGearBoxFunc = mvHwsMtiPcs50CheckGearBox;
    funcPtrArray[MTI_PCS_50]->pcsAlignLockGetFunc = mvHwsMtiPcs50AlignLockGet;
    funcPtrArray[MTI_PCS_50]->pcsSendFaultSetFunc = mvHwsMtiPcs50SendFaultSet;
    funcPtrArray[MTI_PCS_50]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs50SendLocalFaultSet;

    return GT_OK;
}

