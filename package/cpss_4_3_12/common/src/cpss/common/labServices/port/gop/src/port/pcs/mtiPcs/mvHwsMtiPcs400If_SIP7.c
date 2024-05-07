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
* @file mvHwsMtiPcs400If_SIP7.c
*
* @brief MTI PCS400 interface API
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
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcsDb.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs400If.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>


static char* mvHwsMtiPcs400TypeGetFunc(void)
{
  return "MTI_PCS400";
}

/**
* @internal mvHwsMtiPcs400Reset function
* @endinternal
*
* @brief   Set MTI400 PCS RESET/UNRESET or FULL_RESET action.
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
static GT_STATUS mvHwsMtiPcs400Reset
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_RESET            action
)
{
    GT_U32                              pcsResetField, pcsField1;
    MV_HWS_UNITS_ID                     unitId;

    if ((phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8 == 0)
    {
        pcsResetField = AAS_PCS400_CONTROL1_RESET_E;
        unitId = MTI_PCS400_UNIT;
    }
    else
    {
        pcsResetField = AAS_PCS200_CONTROL1_RESET_E;
        unitId = MTI_PCS200_UNIT;
    }
    if ((POWER_DOWN == action) || (RESET == action) || (PARTIAL_POWER_DOWN == action))
    {

        pcsField1 = AAS_PCS400_CONTROL1_SPEED_SELECTION_E;
        switch(portMode)
        {
            case HWS_800G_R8_MODE_CASE:
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField1, 0x2, NULL));
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_PCS400_UNIT, pcsField1, 0x2, NULL));


                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x1, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x5a, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x3e, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x86, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x2a, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x12, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x42, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0xd6, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL8_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0xe1, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL9_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x71, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL10_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x95, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL11_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x22, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL12_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0xa2, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL13_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0x31, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL14_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0xca, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL15_0_UM0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField1, 0xa6, NULL));

                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x71, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL0_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xf3, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xde, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL1_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x7e, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xf3, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL2_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x56, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x80, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL3_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xd0, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x51, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL4_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xf2, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x4f, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL5_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xd1, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x9c, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL6_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xa1, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x76, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL7_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x5b, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL8_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x73, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL8_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x75, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL9_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xc4, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL9_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x3c, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL10_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xeb, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL10_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xd8, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL11_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x66, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL11_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x38, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL12_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xf6, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL12_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x95, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL13_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x97, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL13_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xc3, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL14_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xfb, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL14_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xa6, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL15_0_UM1_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0xba, NULL));
                pcsField1 = AAS_RSFEC_UNIT_VENDOR_VL15_1_UM2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField1, 0x79, NULL));

                break;

            case HWS_200G_R2_MODE_CASE:
            case HWS_200G_R4_MODE_CASE:
            case HWS_400G_R4_MODE_CASE:
            case HWS_400G_R8_MODE_CASE:
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField1, 0x2, NULL));
                break;

            default:
                break;
        }


#if 0
        pcsField1 = AAS_PCS400_VENDOR_VL_INTVL_VL_INTVL_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField1, 0x2000, NULL));
        if(HWS_IS_PORT_MODE_800G_R8(portMode))
        {
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_PCS400_UNIT, pcsField1, 0x2000, NULL));
        }
#endif

        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, unitId, pcsResetField, 0, NULL));
        switch(portMode)
        {
            case HWS_800G_R8_MODE_CASE:
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, unitId, pcsResetField, 0, NULL));
                break;
            default:
                break;
        }

    }
    else if (UNRESET == action)
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, portGroup, phyPortNum, unitId, pcsResetField, 1, NULL));
        switch(portMode)
        {
            case HWS_800G_R8_MODE_CASE:
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, unitId, pcsResetField, 1, NULL));
                break;
            default:
                break;
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "undefined reset action resetMode = %d \n", action);
    }

    return GT_OK;
}


/**
* @internal mvHwsMtiPcs400Mode function
* @endinternal
*
* @brief   Set the MtiPCS400 mode
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
static GT_STATUS mvHwsMtiPcs400Mode
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS *attributesPtr
)
{
    GT_STATUS                           rc;
    MV_HWS_PORT_INIT_PARAMS             curPortParams;
    MV_HWS_PORT_FEC_MODE                portFecMode;
    GT_U32                              markerInterval, pcsField;
    /*MV_HWS_PORT_MII_TYPE_E              miiType;*/
    GT_U32                              regValue;

    GT_UNUSED_PARAM(attributesPtr);

    /*
        m_int_manager.pcs400_speed_select();
        m_int_manager.pcs400_marker_value();
        m_int_manager.pcs400_marker_interval();
    */
    pcsField = AAS_PCS400_CONTROL1_SPEED_SELECTION_E;
    /*
        speed selection [5:2]:
            bits [5:4] = always 0x2
            bit[3] = not of bit[2]
            bit[2] = '1' - 200G
                     '0' - 400G

            it means that setting 0x0 and 0x2 to bits[3:2] will give that same configuration.
    */
    switch(portMode)
    {
        case HWS_800G_R8_MODE_CASE:
            regValue = 0;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_PCS400_UNIT, pcsField, regValue, NULL));
            break;

        case HWS_400G_R4_MODE_CASE:
        case HWS_400G_R8_MODE_CASE:
            regValue = 0;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));
            break;


        case HWS_200G_R2_MODE_CASE:
        case HWS_200G_R4_MODE_CASE:
            regValue = 1;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, regValue, NULL));
            break;

        default:
            hwsOsPrintf("mvHwsMtiPcs400Mode: check mvHwsMtiPcs400Mode speed_select implementation for portMode = %d\n", portMode);
            return GT_BAD_PARAM;
    }
    /*******************************************************************************************************************************************/
    switch(portMode)
    {
        case HWS_200G_R2_MODE_CASE:
        case HWS_200G_R4_MODE_CASE:

            pcsField = AAS_PCS400_VENDOR_AM_0_CM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, 0x9a, NULL));
            pcsField = AAS_PCS400_VENDOR_AM_0_CM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, 0x4a, NULL));
            pcsField = AAS_PCS400_VENDOR_AM_1_CM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, 0x26, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xb3, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xc0, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x8c, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x5a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xde, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x7e, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x3e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xf3, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x56, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x86, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x80, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd0, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x2a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x51, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xf2, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x12, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x4f, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd1, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x42, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x9c, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xa1, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd6, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x76, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x5b, NULL));

            break;

        case HWS_400G_R4_MODE_CASE:
        case HWS_400G_R8_MODE_CASE:

            pcsField = AAS_PCS400_VENDOR_AM_0_CM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, 0x9a, NULL));
            pcsField = AAS_PCS400_VENDOR_AM_0_CM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, 0x4a, NULL));
            pcsField = AAS_PCS400_VENDOR_AM_1_CM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, 0x26, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x01, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x71, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xf3, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x5a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xde, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x7e, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x3e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xf3, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x56, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x86, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x80, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd0, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x2a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x51, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xf2, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x12, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x4f, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd1, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x42, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x9c, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xa1, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd6, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x76, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x5b, NULL));

            break;

        case HWS_800G_R8_MODE_CASE:

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xfe, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xa5, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xc1, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x79, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xd5, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xed, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xbd, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x29, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL8_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x1e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL9_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x8e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL10_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x6a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL11_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xdd, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL12_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x5d, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL13_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0xce, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL14_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x35, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL15_0_UM0_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_RSFEC_UNIT,  pcsField, 0x59, NULL));

            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x8e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL0_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x0c, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x21, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL1_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x81, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x0c, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL2_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0xa9, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x7f, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL3_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x2f, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0xae, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL4_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x0d, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0xb0, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL5_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x2e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x63, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL6_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x5e, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x89, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL7_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0xa4, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL8_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x8c, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL8_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x8a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL9_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x3b, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL9_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0xc3, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL10_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x14, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL10_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x27, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL11_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x99, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL11_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0xc7, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL12_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x09, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL12_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x6a, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL13_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x68, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL13_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x3c, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL14_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x04, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL14_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x59, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL15_0_UM1_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x45, NULL));
            pcsField = AAS_RSFEC_UNIT_VENDOR_VL15_1_UM2_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_RSFEC_UNIT,  pcsField, 0x86, NULL));

            break;

        default:
            hwsOsPrintf("mvHwsMtiPcs400Mode: check mvHwsMtiPcs400Mode marker_value implementation for portMode = %d\n", portMode);
            return GT_BAD_PARAM;
    }

    /*******************************************************************************************************************************************/
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    portFecMode = curPortParams.portFecMode;

    rc = mvHwsAasUnitMarkerIntervalCalc(devNum, portMode, portFecMode, 0, &markerInterval);
    if(rc != GT_OK)
    {
        return rc;
    }
    pcsField = AAS_PCS400_VENDOR_VL_INTVL_VL_INTVL_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS400_UNIT, pcsField, markerInterval, NULL));
    if(HWS_IS_PORT_MODE_800G_R8(portMode))
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum+8, MTI_PCS400_UNIT, pcsField, markerInterval, NULL));
    }

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400LoopBack function
* @endinternal
*
* @brief   Set PCS loop back.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] pcsNum                   - physical PCS number
*                                      pcsType   - PCS type
* @param[in] lbType                   - loop back type
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsMtiPcs400LoopBack
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  MV_HWS_PORT_LB_TYPE     lbType
)
{
    MV_HWS_PORT_INIT_PARAMS                 curPortParams;
    GT_UREG_DATA                            data;
    MV_HWS_AAS_PCS400_UNIT_FIELDS_ENT       pcsField;
    MV_HWS_AAS_MAC_PORT_EXT_UNIT_FIELDS_ENT extField;

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    pcsField = AAS_PCS400_CONTROL1_LOOPBACK_E;
    data = (lbType == TX_2_RX_LB) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_PCS400_UNIT, pcsField, data, NULL));

    if(HWS_IS_PORT_MODE_800G_R8(portMode))
    {
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, (curPortParams.portPcsNumber+8), MTI_PCS400_UNIT, pcsField, data, NULL));
    }

    extField = AAS_MAC_PORT_EXT_CONTROL1_FORCE_LINK_OK_EN_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, curPortParams.portPcsNumber, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400LoopBackGet function
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
static GT_STATUS mvHwsMtiPcs400LoopBackGet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portGroup,
    IN  GT_U32                  phyPortNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT MV_HWS_PORT_LB_TYPE     *lbType
)
{
    GT_UREG_DATA                        data;
    MV_HWS_AAS_PCS400_UNIT_FIELDS_ENT   pcsField;

    GT_UNUSED_PARAM(portMode);

    pcsField = AAS_PCS400_CONTROL1_LOOPBACK_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, portGroup, phyPortNum, MTI_PCS400_UNIT, pcsField, &data, NULL));

    *lbType = (data != 0) ? TX_2_RX_LB : DISABLE_LB;
    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400AlignLockGet function
* @endinternal
*
* @brief   Read align lock status of given MtiPCS400.
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
static GT_STATUS mvHwsMtiPcs400AlignLockGet
(
    IN  GT_U8                   devNum,
    IN  GT_UOPT                 portGroup,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    OUT GT_BOOL                 *lock
)
{
    GT_UREG_DATA                  data;
    MV_HWS_AAS_PCS800_EXT_UNIT_FIELDS_ENT extField = AAS_PCS800_EXT_PORT0_STATUS_P0_LINK_STATUS_E;

    GT_UNUSED_PARAM(portMode);

    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, portGroup, portNum, MTI_PCS_EXT_UNIT, portMode, extField, &data, NULL));

    *lock = (data != 0);

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400SendFaultSet function
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
static GT_STATUS mvHwsMtiPcs400SendFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA                            data;
    MV_HWS_AAS_MAC_PORT_EXT_UNIT_FIELDS_ENT extField;

    portGroup = portGroup;
    portMode = portMode;

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
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400SendLocalFaultSet function
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
static GT_STATUS mvHwsMtiPcs400SendLocalFaultSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               portGroup,
    IN  GT_U32               pcsNum,
    IN  MV_HWS_PORT_STANDARD portMode,
    IN  GT_BOOL              send
)
{
    GT_UREG_DATA    data;
    MV_HWS_AAS_MAC_PORT_EXT_UNIT_FIELDS_ENT extField;

    portGroup = portGroup;
    portMode = portMode;

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

    extField = AAS_MAC_PORT_EXT_CONTROL_TX_LOC_FAULT_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, pcsNum, MTI_EXT_UNIT, portMode, extField, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400FecConfigGet function
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
static GT_STATUS mvHwsMtiPcs400FecConfigGet
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

    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(phyPortNum);
    GT_UNUSED_PARAM(portMode);

    *portFecTypePtr = RS_FEC_544_514;

    return  GT_OK;
}

/**
* @internal mvHwsMtiPcs400Rex2RxReset function
* @endinternal
*
* @brief   PCS 400 SD RX RESET/UNRESET action.
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
static GT_STATUS mvHwsMtiPcs400RxReset
(
    GT_U8                   devNum,
    GT_U32                  portGroup,
    GT_U32                  phyPortNum,
    MV_HWS_PORT_STANDARD    portMode,
    MV_HWS_RESET            action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i, regData = 0, dataToWrite = 0;
    MV_HWS_AAS_PCS800_EXT_UNIT_FIELDS_ENT   extField;

    GT_UNUSED_PARAM(portGroup);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    switch (action)
    {
        case RESET:
            dataToWrite = 0;
            break;
        case UNRESET:
            for (i = 0; i < curPortParams.numOfActLanes; i++)
            {
                dataToWrite |= (1 << (i));
            }
            break;
        default:
            return GT_OK;
    }

    extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL_GC_SD_RX_RESET__E;

    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, MTI_PCS_EXT_UNIT, extField, &regData, NULL));
    U32_SET_FIELD(regData, (phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS), curPortParams.numOfActLanes, dataToWrite);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, MTI_PCS_EXT_UNIT, extField, regData, NULL));

    return GT_OK;
}

/**
* @internal mvHwsMtiPcs400IfInit function
* @endinternal
*
* @brief   Init MTI PCS400 for SIP7 configuration sequences and IF
*          functions.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsMtiPcs400_SIP7IfInit(MV_HWS_PCS_FUNC_PTRS **funcPtrArray)
{
    if(!funcPtrArray[MTI_PCS_400])
    {
        funcPtrArray[MTI_PCS_400] = (MV_HWS_PCS_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_PCS_FUNC_PTRS));
        if(!funcPtrArray[MTI_PCS_400])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[MTI_PCS_400], 0, sizeof(MV_HWS_PCS_FUNC_PTRS));
    }
    funcPtrArray[MTI_PCS_400]->pcsResetFunc         = mvHwsMtiPcs400Reset;
    funcPtrArray[MTI_PCS_400]->pcsRxResetFunc       = mvHwsMtiPcs400RxReset;
    funcPtrArray[MTI_PCS_400]->pcsModeCfgFunc       = mvHwsMtiPcs400Mode;
    funcPtrArray[MTI_PCS_400]->pcsLbCfgFunc         = mvHwsMtiPcs400LoopBack;
    funcPtrArray[MTI_PCS_400]->pcsLbCfgGetFunc      = mvHwsMtiPcs400LoopBackGet;
    funcPtrArray[MTI_PCS_400]->pcsTypeGetFunc       = mvHwsMtiPcs400TypeGetFunc;
    funcPtrArray[MTI_PCS_400]->pcsFecCfgGetFunc     = mvHwsMtiPcs400FecConfigGet;
    funcPtrArray[MTI_PCS_400]->pcsAlignLockGetFunc      = mvHwsMtiPcs400AlignLockGet;
    funcPtrArray[MTI_PCS_400]->pcsSendFaultSetFunc      = mvHwsMtiPcs400SendFaultSet;
    funcPtrArray[MTI_PCS_400]->pcsSendLocalFaultSetFunc = mvHwsMtiPcs400SendLocalFaultSet;

    return GT_OK;
}

