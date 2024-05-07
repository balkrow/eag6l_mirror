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
*/
/**
********************************************************************************
* @file mvHwsAasPortCfgIf.c
*
* @brief
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/generic/labservices/port/gop/silicon/aas/mvHwsAasPortIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsAasPortAnp.h>

extern  GT_BOOL hwsPpHwTraceFlag;

extern GT_STATUS mvHwsAasUnitBaseAddrCalc
(
    IN GT_U8                    devNum,
    IN MV_HWS_UNITS_ID          unitId,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   phyPortNum,
    OUT GT_U32                 *baseAddrPtr
);

typedef enum{
    MV_HWS_STATE_GROUP_MAC_TYPE_E,
    MV_HWS_STATE_GROUP_PCS_TYPE_E,
    MV_HWS_STATE_GROUP_SD_TYPE_E
}MV_HWS_STATE_GROUP_TYPE_ENT;

typedef struct{
    GT_U32 regAddr;
    GT_U32 expectedData;
    GT_U32 mask;
}MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC;


/**
* @internal mvHwsAasExtPcsClockAndResetRelease function
* @endinternal
*
* @brief  EXT unit clock & reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] stateInPcsPortGroup   - the port place in the PCS group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtPcsClockAndResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     stateInPcsPortGroup
)
{
    GT_U32 regData;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;

    /*pcs_ext_manager:pcs_clock_enable_and_reset_release*/

    GT_UNUSED_PARAM(portMode);

    regData = (enable == GT_TRUE) ? 1 : 0;

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtPcsClockAndResetRelease ******\n");
    }

    if((stateInPcsPortGroup == MV_HWS_PORT_IN_GROUP_FIRST_E) || (stateInPcsPortGroup == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = AAS_PCS800_EXT_GLOBAL_CLOCK_ENABLE_MAC_CMN_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
        extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL2_GC_REF_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtPcsClockAndResetRelease ******\n");
    }

    return GT_OK;
}

/**
* @internal mvHwsAasExtPcs400ClockAndResetRelease function
* @endinternal
*
* @brief  EXT unit clock400 & reset400 enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] stateInMacGroup - the port place in the serdes group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtPcs400ClockAndResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     stateInMacGroup
)
{
    GT_U32 regData, mask = 0;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;

    /*pcs_ext_manager:pcs400_clock*/
    /*pcs_ext_manager:pcs400_reset_set*/

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        return GT_OK;
    }

    if(HWS_IS_PORT_MODE_800G_R8(portMode))
    {
        mask = 3;
    }
    else if(HWS_IS_PORT_MODE_400G_R4(portMode) ||
            HWS_IS_PORT_MODE_400G_R8(portMode) ||
            HWS_IS_PORT_MODE_200G_R2(portMode) ||
            HWS_IS_PORT_MODE_200G_R4(portMode))
    {
        mask = 1 << ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 8);
    }
    else
    {
        /*nothing to do*/
        return GT_OK;
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtPcs400ClockAndResetRelease ******\n");
    }

    if((stateInMacGroup == MV_HWS_PORT_IN_GROUP_FIRST_E) || (stateInMacGroup == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = AAS_PCS800_EXT_GLOBAL_CLOCK_ENABLE_PCS000_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
        regData = (enable == GT_TRUE) ? (regData|mask) : (regData & ~mask);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

        extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL2_GC_PCS000_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
        regData = (enable == GT_TRUE) ? (regData|mask) : (regData & ~mask);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtPcs400ClockAndResetRelease ******\n");
    }

    return GT_OK;
}

/**
* @internal mvHwsAasExtPcs100ClockAndResetRelease function
* @endinternal
*
* @brief  EXT unit clock100 & reset100 enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] stateInMacGroup - the port place in the serdes group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtPcs100ClockAndResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     stateInMacGroup
)
{
    GT_U32 regData, mask = 0;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;

    /*pcs_ext_manager:pcs100_clock*/
    /*pcs_ext_manager:pcs100_reset_set*/

    GT_UNUSED_PARAM(stateInMacGroup);

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        return GT_OK;
    }

    /* for a while there is no first/last indication for QUAD PCS group */
    if(enable == GT_FALSE)
    {
        return GT_OK;
    }

    if (HWS_IS_PORT_MODE_800G_R8(portMode) ||
        HWS_IS_PORT_MODE_400G_R4(portMode) ||
        HWS_IS_PORT_MODE_400G_R8(portMode) ||
        HWS_IS_PORT_MODE_200G_R2(portMode) ||
        HWS_IS_PORT_MODE_200G_R4(portMode))
    {
        /*nothing to do*/
        return GT_OK;
    }

    /* Quad PCS mac clk (common per Quad) */
    mask = 1 << ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 4);

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtPcs100ClockAndResetRelease ******\n");
    }

    extField = AAS_PCS800_EXT_GLOBAL_CLOCK_ENABLE_XPCS_CLK_EN_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
    regData = (enable == GT_TRUE) ? (regData|mask) : (regData & ~mask);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

    extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL2_GC_XPCS_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
    regData = (enable == GT_TRUE) ? (regData|mask) : (regData & ~mask);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtPcs100ClockAndResetRelease ******\n");
    }

    return GT_OK;
}

/**
* @internal mvHwsAasExtRsFecClockAndResetRelease function
* @endinternal
*
* @brief  EXT unit RSFEC clock & reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] stateInMacGroup - the port place in the serdes group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtRsFecClockAndResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     stateInMacGroup
)
{
    GT_U32 regData, mask = 0;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /*pcs_ext_manager:rsfec_clock*/
    /*pcs_ext_manager:rsfec_reset_set*/

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    switch (curPortParams.portFecMode)
    {
        case FC_FEC: /*FC FEC (FEC74) needs RS FEC clock enabled ... MTI*/
        case RS_FEC:
        case RS_FEC_544_514:
        case RS_FEC_272_257:
            break;
        default:
            /*nothing to do*/
            return GT_OK;
    }

    if(HWS_IS_PORT_MODE_800G_R8(portMode))
    {
        mask = 0x3;
    }
    else
    {
        mask = 1 << ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 8);
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtRsFecClockAndResetRelease ******\n");
    }

    if((stateInMacGroup == MV_HWS_PORT_IN_GROUP_FIRST_E) || (stateInMacGroup == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = AAS_PCS800_EXT_GLOBAL_CLOCK_ENABLE_FEC91_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
        regData = (enable == GT_TRUE) ? (regData|mask) : (regData & ~mask);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

        extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL2_GC_F91_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
        regData = (enable == GT_TRUE) ? (regData|mask) : (regData & ~mask);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtRsFecClockAndResetRelease ******\n");
    }

    return GT_OK;
}

/**
* @internal mvHwsAasExtPcsPmaControlSet function
* @endinternal
*
* @brief  EXT unit clock & reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtPcsPmaControlSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable
)
{
    GT_U32 regData, regData1, localPortIndex = 0;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i, regDataSd16X = 0, regDataSd8X = 0;

    /*pcs_ext_manager:pcs_PMA_control x8*/

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        return GT_OK;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    if(enable == GT_FALSE)
    {
        curPortParams.serdes10BitStatus = _10BIT_OFF;
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtPcsPmaControlSet ******\n");
    }

    extField = AAS_PCS800_EXT_GLOBAL_PMA_CONTROL_GC_SD_16X_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
    extField = AAS_PCS800_EXT_GLOBAL_PMA_CONTROL_GC_SD_8X_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData1, NULL));

    localPortIndex = portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch (curPortParams.serdes10BitStatus)
        {
            case _128BIT_ON:
                regDataSd8X = 0;
                regDataSd16X |= (0x1 << i);
                break;
            case _64BIT_ON:
                regDataSd8X |=  (0x1 << i);
                regDataSd16X = 0;
                break;
            case _32BIT_ON:
            case _10BIT_OFF:
                break;
            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsAasExtPcsPmaControlSet hws bad width param"));
        }
    }

    U32_SET_FIELD(regData, localPortIndex, curPortParams.numOfActLanes, regDataSd16X);
    extField = AAS_PCS800_EXT_GLOBAL_PMA_CONTROL_GC_SD_16X_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

    U32_SET_FIELD(regData1, localPortIndex, curPortParams.numOfActLanes, regDataSd8X);
    extField = AAS_PCS800_EXT_GLOBAL_PMA_CONTROL_GC_SD_8X_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData1, NULL));

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtPcsPmaControlSet ******\n");
    }

    return GT_OK;
}

/**
* @internal mvHwsAasExtPcsFecControlSet function
* @endinternal
*
* @brief  EXT unit clock & reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtPcsFecControlSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable
)
{
    GT_U32 regData, regData1, regData2, localPortIndex = 0;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 i;
    MV_HWS_PORT_FEC_MODE        portFecMode;
    GT_U32 gcFec91EnaIn = 0, gcFecEna = 0, gcKpModeIn = 0;

    /*pcs_ext_manager:pcs_FEC_control x8*/

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        hwsOsPrintf("\n\n\nKALEX: IMPLEMENTATION NEEDED\n\n\n");
        return GT_OK;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }
    if(enable == GT_FALSE)
    {
        portFecMode = FEC_OFF;
    }
    else
    {
        portFecMode = curPortParams.portFecMode;
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtPcsFecControlSet ******\n");
    }

    extField = AAS_PCS800_EXT_GLOBAL_FEC_CONTROL_2_GC_FEC91_ENA_IN_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
    extField = AAS_PCS800_EXT_GLOBAL_FEC_CONTROL_2_GC_KP_MODE_IN_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData1, NULL));
    extField = AAS_PCS800_EXT_GLOBAL_FEC_CONTROL_GC_FEC_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData2, NULL));

    localPortIndex = portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS;

    for (i = 0; i < curPortParams.numOfActLanes; i++)
    {
        switch(portFecMode)
        {
            case FEC_OFF:
                break;

            case FC_FEC:
                gcFecEna |= 1 << (i);
                break;

            case RS_FEC:
                gcFec91EnaIn |= 1 << (i);
                break;

            case RS_FEC_544_514:
            case RS_FEC_544_514_INT:
                gcFec91EnaIn |= 1 << (i);
                gcKpModeIn |= 1 << (i);
                break;

            default:
                HWS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ARG_STRING_MAC("mvHwsAasExtPcsFecControlSet hws bad FEC param"));
        }
    }

    U32_SET_FIELD(regData, localPortIndex, curPortParams.numOfActLanes, gcFec91EnaIn);
    extField = AAS_PCS800_EXT_GLOBAL_FEC_CONTROL_2_GC_FEC91_ENA_IN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

    U32_SET_FIELD(regData1, localPortIndex, curPortParams.numOfActLanes, gcKpModeIn);
    extField = AAS_PCS800_EXT_GLOBAL_FEC_CONTROL_2_GC_KP_MODE_IN_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData1, NULL));

    U32_SET_FIELD(regData2, localPortIndex, curPortParams.numOfActLanes, gcFecEna);
    extField = AAS_PCS800_EXT_GLOBAL_FEC_CONTROL_GC_FEC_ENA_E;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData2, NULL));


    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtPcsFecControlSet ******\n");
    }

    return GT_OK;
}


/**
* @internal mvHwsAasExtPcsChannelControlSet function
* @endinternal
*
* @brief  EXT unit clock & reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtPcsChannelControlSet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable
)
{
    GT_U32 regData, regData1;
    GT_U32 extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 channelRegData_GC_FEC91 = 0, channelRegData_GC_RXLAUI_ENA = 0;
#if 0
    GT_U32 i;
    MV_HWS_PORT_FEC_MODE        portFecMode;
    GT_U32 gcFec91EnaIn = 0, gcFecEna = 0, gcKpModeIn = 0;
#endif

    /*pcs_ext_manager:pcs_Channel_Control*/

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        hwsOsPrintf("\n\n\nKALEX: IMPLEMENTATION NEEDED\n\n\n");
        return GT_OK;
    }

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtPcsChannelControlSet ******\n");
    }


    switch(portMode)
    {
        case HWS_40G_R4_MODE_CASE:
            regData = (enable == GT_TRUE) ? 1 : 0;
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_MODE40_ENA_IN0_E + (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS)/4;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));
            break;

        case HWS_40G_R2_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:

            switch (curPortParams.portFecMode)
            {
                case RS_FEC:
                    channelRegData_GC_FEC91 = 0;
                    channelRegData_GC_RXLAUI_ENA = 0;
                    break;
                case FEC_NA: /* when disabling port - return to default*/
                    channelRegData_GC_FEC91 = 1;
                    channelRegData_GC_RXLAUI_ENA = 0;
                    break;
                default:
                    channelRegData_GC_FEC91 = 0;
                    channelRegData_GC_RXLAUI_ENA = 1;
                    break;
            }
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN0_E + (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS)/2;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, channelRegData_GC_FEC91, NULL));
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_RXLAUI_ENA_IN0_E + (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS)/2;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, channelRegData_GC_RXLAUI_ENA, NULL));
            break;

        case HWS_200G_R2_MODE_CASE:
        case HWS_200G_R4_MODE_CASE:
      /*case HWS_150G_R2_MODE_CASE:*/
            regData = (enable == GT_TRUE) ? 0x1 : 0;
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_GC_PCS400_ENA_IN_E;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData1, NULL));
            U32_SET_FIELD(regData1, ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS)/4), 1, regData);
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData1, NULL));
            break;

        case HWS_400G_R4_MODE_CASE:
        case HWS_400G_R8_MODE_CASE:
      /*case HWS_300G_R4_MODE_CASE:*/

            regData = (enable == GT_TRUE) ? 0x3 : 0;
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_GC_PCS400_ENA_IN_E;
            CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData1, NULL));
            U32_SET_FIELD(regData1, ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS)/4), 2, regData);
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData1, NULL));
            break;

        case HWS_800G_R8_MODE_CASE:
        /*case HWS_600G_R8_MODE_CASE:*/

            regData = (enable == GT_TRUE) ? 0xF : 0;
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_GC_PCS400_ENA_IN_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

            regData = (enable == GT_TRUE) ? 0x1 : 0;
            extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_MODE_800G_E;
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

            break;

        default:
            /*nothing to do*/
            break;
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtPcsChannelControlSet ******\n");
    }

    return GT_OK;
}



/**
* @internal mvHwsAasExtReset function
* @endinternal
*
* @brief  Reset/unreset EXT SD RX/TX unit
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] reset           - true = reset/ false = unreset
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtReset
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_BOOL                  reset
)
{
    GT_STATUS                   rc;
    GT_U32                      regData = 0, dataToWrite = 0, localPortIndex = 0, i;
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      extField;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    MV_HWS_UNITS_ID             unitId = MTI_PCS_EXT_UNIT;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtReset ******\n");
    }
#endif

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    if(reset == GT_FALSE)
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

    localPortIndex = portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS;
    extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL_GC_SD_TX_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
    U32_SET_FIELD(regData, localPortIndex, curPortParams.numOfActLanes, dataToWrite);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

    hwsOsPrintf("\n -mvHwsAasExtReset: TBD - SD_RX_RESET should be moved later to port manager after rx train\n");
    extField = AAS_PCS800_EXT_GLOBAL_RESET_CONTROL_GC_SD_RX_RESET__E;
    CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, portNum, unitId, extField, &regData, NULL));
    U32_SET_FIELD(regData, localPortIndex, curPortParams.numOfActLanes, dataToWrite);
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, regData, NULL));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtReset ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAasExtMacSpeedSet function
* @endinternal
*
* @brief   Set MTI MAC speed resolution.
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical number
* @param[in] portMode              - port mode
* @param[in] enable                - enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAasExtMacSpeedSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 enable
)
{
    MV_HWS_UNITS_ID     unitId;
    GT_U32              speedRes;
    GT_U32              extField;

    /*m_ext_manager:set_res_speed */

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtMacSpeedSet ******\n");
    }
#endif

    switch (portMode) {
        case SGMII:
        case QSGMII:
        case SGMII2_5:
        case HWS_D_USX_MODE_CASE:
        case _5G_QUSGMII:
        case _10G_OUSGMII:
        case HWS_Q_USX_MODE_CASE:
        case HWS_SX_MODE_CASE:
            speedRes = 1;
            break;

        case HWS_1000BASE_MODE_CASE:
        case _2500Base_X:
            speedRes = 3;
            break;

        case _5GBaseR:
        case HWS_10G_MODE_CASE:
        case _12GBaseR:
            speedRes = 5;
            break;

        case HWS_25G_MODE_CASE:
            speedRes = 6;
            break;

        case HWS_40G_R1_MODE_CASE:
        case HWS_40G_R2_MODE_CASE:
        case HWS_40G_R4_MODE_CASE:
            speedRes = 7;
            break;

        case HWS_50G_R1_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
            speedRes = 8;
            break;

        case HWS_100G_R1_MODE_CASE:
        case HWS_100G_R2_MODE_CASE:
        case HWS_100G_R4_MODE_CASE:
            speedRes = 9;
            break;

        case HWS_200G_R2_MODE_CASE:
        case HWS_200G_R4_MODE_CASE:
        case HWS_200G_R8_MODE_CASE:
            speedRes = 11;
            break;

        case HWS_400G_R4_MODE_CASE:
        case HWS_400G_R8_MODE_CASE:
            speedRes = 10;
            break;

        case HWS_800G_R8_MODE_CASE:
            speedRes = 12;
            break;

        default:
            if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
                hwsOsPrintf("****** End of mvHwsAasExtMacSpeedSet ******\n");
            }
            return GT_BAD_PARAM;
    }

    /* restore field to default value */
    if(enable == GT_FALSE)
    {
        speedRes = 0xf;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        hwsOsPrintf("CPU port  implementation is needed\n");
        return GT_BAD_PARAM;
    }
    else
    {
        unitId = MTI_EXT_UNIT;
        extField = AAS_MAC_PORT_EXT_CONTROL1_PORT_RES_SPEED_E;
    }
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, unitId, portMode, extField, speedRes, NULL));

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtMacSpeedSet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAasExtMacDoubleFreqSet function
* @endinternal
*
* @brief   enable/disable double freq for 200G_R2/200G_R4/200G_R8/800G_R8
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical number
* @param[in] portMode              - port mode
* @param[in] enable                - enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAasExtMacDoubleFreqSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 enable
)
{
    MV_HWS_UNITS_ID     unitId;
    GT_U32              regData;
    GT_U32              extField;

    /*m_ext_manager:mac_double_frequency */

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        return GT_OK;
    }

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtMacDoubleFreqSet ******\n");
    }

    switch (portMode) {
        case HWS_200G_R2_MODE_CASE:
        case HWS_200G_R4_MODE_CASE:
        case HWS_200G_R8_MODE_CASE:
        case HWS_800G_R8_MODE_CASE:
            regData = 1;
            break;

        default:
            regData = 0;
            break;
    }

    /* restore field to default value */
    if(enable == GT_FALSE)
    {
        regData = 0;
    }

    unitId = MTI_EXT_UNIT;
    extField = AAS_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_DOUBLE_FREQUENCY_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, unitId, portMode, extField, regData, NULL));


#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtMacDoubleFreqSet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAasExtMacClockAndResetRelease function
* @endinternal
*
* @brief  EXT MAC Clock/Reset enable/disable
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] enable          - port channel enable /disable
* @param[in] serdesFullConfig- the port place in the serdes group
* @param[in] portFullConfig  - the port place in the port group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtMacClockAndResetRelease
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      enable,
    IN MV_HWS_PORT_IN_GROUP_ENT     stateInMacGroup
)
{
    GT_U32      regData;
    GT_U32      extField;

    /* mac_ext_manager::mac_clock_enable_and_reset_release */

    regData = (enable == GT_TRUE) ? 1 : 0;

    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtMacClockAndResetRelease ******\n");
    }

    if((stateInMacGroup == MV_HWS_PORT_IN_GROUP_FIRST_E) || (stateInMacGroup == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_GLOBAL_EXT_UNIT, extField, regData, NULL));

        extField = AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_APP_CLK_EN_E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_GLOBAL_EXT_UNIT, extField, regData, NULL));
    }

    extField = AAS_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_CLK_EN_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, portMode, extField, regData, NULL));

    if((stateInMacGroup == MV_HWS_PORT_IN_GROUP_FIRST_E) || (stateInMacGroup == MV_HWS_PORT_IN_GROUP_LAST_E))
    {
        extField = AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_GLOBAL_EXT_UNIT, extField, regData, NULL));

        extField = AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_APP_RESET__E;
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, MTI_GLOBAL_EXT_UNIT, extField, regData, NULL));
    }

    extField = AAS_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_RESET__E;
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, portMode, extField, regData, NULL));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtMacClockAndResetRelease ******\n");
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsAasExtMacClockEnableGet function
* @endinternal
*
* @brief  EXT unit clock enable get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - (pointer to) port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsAasExtMacClockEnableGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      *enablePtr
)
{
    GT_U32 regData;
    GT_U32 extField;

    extField = AAS_MAC_PORT_EXT_PORT_MAC_CLOCK_AND_RESET_CONTROL_MAC_CLK_EN_E;
    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, 0, portNum, MTI_EXT_UNIT, portMode, extField, &regData, NULL));

    *enablePtr = (regData != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}


/**
* @internal mvHwsAasExtMacMuxCtrlSet function
* @endinternal
*
* @brief  EXT MAC MUX Control - BR/NON_BR selector
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] isPreemptionEnabled - port preemption status
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
static GT_STATUS mvHwsAasExtMacMuxCtrlSet
(
    IN GT_U8                            devNum,
    IN GT_U32                           portNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN GT_BOOL                          isPreemptionEnabled
)
{
    GT_U32 regData;
    GT_U32 extField;

    GT_UNUSED_PARAM(portMode);

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtMacMuxCtrlSet ******\n");
    }
#endif

    switch (portMode) {

        case HWS_40G_R1_MODE_CASE:
        case HWS_40G_R2_MODE_CASE:
        case HWS_40G_R4_MODE_CASE:
        case HWS_50G_R1_MODE_CASE:
        case HWS_50G_R2_MODE_CASE:
        case HWS_100G_R1_MODE_CASE:
        case HWS_100G_R2_MODE_CASE:
        case HWS_100G_R4_MODE_CASE:
        case HWS_200G_R2_MODE_CASE:
        case HWS_200G_R4_MODE_CASE:
        case HWS_200G_R8_MODE_CASE:
        case HWS_400G_R4_MODE_CASE:
        case HWS_400G_R8_MODE_CASE:
        case HWS_800G_R8_MODE_CASE:
            /* port modes that can't work with preemption enabled */
            return GT_OK;

        default:
            break;
    }

    /* even ports can't work with preemption  */
    if(portNum %2 == 0)
    {
        return GT_OK;
    }


    /* set m_RAL[port_index].mtip_mac_port_ext_units_RegFile.External_Port_Control.mac_mux_ctrl */

    /*
        External mux ctrl.
        For even MAC units
            The mux selects between express channel of this BR mac "n" and the express channel of BR MAC "n+1"
            0: Express channel of the this BR MAC ("n")
            1: Express channel of the BR MAC "n+1"

        For odd MAC units:
            The mux selects between preemption and express channel of this BR mac
            0 - Express channel of this BR mac
            1 - Preemptive channel of this BR mac
    */
    regData = (isPreemptionEnabled == GT_TRUE) ? 1 : 0;
    extField = AAS_MAC_PORT_EXT_EXTERNAL_PORT_CONTROL_MAC_MUX_CTRL_E;

    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, (portNum - 1), MTI_EXT_UNIT, portMode, extField, regData, NULL));
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, MTI_EXT_UNIT, portMode, extField, regData, NULL));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtMacMuxCtrlSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAasExtMacFec91EnaSet function
* @endinternal
*
* @brief   Set MTI MAC FEC enable/disable.
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical number
* @param[in] portMode              - port mode
* @param[in] enable                - enable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsAasExtMacFec91EnaSet
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  portNum,
    IN  MV_HWS_PORT_STANDARD    portMode,
    IN  GT_BOOL                 enable
)
{
    MV_HWS_UNITS_ID         unitId;
    GT_U32                  regValue;
    GT_U32                  extField;
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    /*mac_ext_manager::set_fec91_ena*/

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, portNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasExtMacFec91EnaSet ******\n");
    }
#endif

    switch (curPortParams.portFecMode)
    {
        case RS_FEC:
        case RS_FEC_544_514:
        case RS_FEC_544_514_INT:
        case RS_FEC_272_257:
        case RS_FEC_272_257_INT:
            regValue = 1;
            break;

        default:
            regValue = 0;
            break;
    }

    /* restore field to default value */
    if(enable == GT_FALSE)
    {
        regValue = 0;
    }

    if(mvHwsMtipIsReducedPort(devNum, portNum) == GT_TRUE)
    {
        hwsOsPrintf("CPU port  implementation is needed\n");
        return GT_BAD_PARAM;
    }
    else
    {
        unitId = MTI_EXT_UNIT;
        extField = AAS_MAC_PORT_EXT_PORT_CONTROL2_FEC91_ENA_E;
    }
    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, 0, portNum, unitId, portMode, extField, regValue, NULL));


#ifndef MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasExtMacFec91EnaSet ******\n");
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAasReplicationSet function
* @endinternal
*
* @brief   set usx replication.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical number
* @param[in] portMode                 - port mode
* @param[in] portInitInParamPtr       - PM DB
* @param[in] linkUp                   - port link status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAasReplicationSet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          portGroup,
    IN  GT_U32                          phyPortNum,
    IN  MV_HWS_PORT_STANDARD            portMode,
    IN  MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr,
    IN  GT_BOOL                         linkUp
)
{
    MV_HWS_UNITS_ID             unitId = MTI_LOW_SP_PCS_UNIT;
    GT_U32                      fieldNameGmiiRep;
    GT_U32                      fieldNameGmii_2_5;
    GT_U32                      valGmiiRep, valGmii_2_5;
    GT_U32                      portTypeDefaultSpeed = 0, sgmiiSpeed = 2;
    MV_HWS_PORT_INIT_PARAMS     curPortParams;
    GT_FLOAT32                  replication, an_step, desired_timer = 2000000.0;
    GT_U32                      final_timer;
    MV_HWS_PORT_SPEED_ENT       portSpeed = portInitInParamPtr->portSpeed;
    GT_U32                      regData = 0;
    GT_UREG_DATA                data = 0;
    GT_BOOL                     autoNegEnabled = portInitInParamPtr->autoNegEnabled;

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasReplicationSet ******\n");
    }
#endif
    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /**
     * The function is also called  in prvCpssPortManagerLinkUpCheck
     * in case of link UP
     */
    switch (linkUp)
    {
    case GT_FALSE:
        switch (portMode)
        {
                /*low speed */
            case SGMII:
            case _1000Base_X:
            case _1000Base_SX:
            case QSGMII:
            case SGMII2_5:
            case _2500Base_X:
            case _5G_QUSGMII:
                /**
                 * For low speed, the timers must be cleard
                 * and valGmiiRep and valGmii_2_5 set to zero.
                 */
                portTypeDefaultSpeed = 0;
                desired_timer = 0;
                an_step = 12.8;
                /**
                 * For low speed, autonegotiation is configured
                 * separatly
                 */
                autoNegEnabled = GT_FALSE;
                break;

            default:
                return GT_OK;
        }

        /**
         * Calculate values for replication and timers for desierd
         * speed:
         *
         * if (Anenable)
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].Usxgmiirep.set(1);
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].p_Usxgmii2_5.set(0);
         * else
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].Usxgmiirep.set(Usxgmiirep);
         *      m_RAL.lpcs_units_RegFile.p_USXGMII_REP[portIndex].p_Usxgmii2_5.set(Usxgmii2_5);
         *
         * final_timer=desired_timer/an_step*2;
         */
        replication = (autoNegEnabled) ? (0x1) : ((GT_FLOAT32)portTypeDefaultSpeed / (GT_FLOAT32)portSpeed);
        final_timer = (GT_U32)((desired_timer / an_step)*2);
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, AAS_LPCS_CONTROL_P0_ANENABLE_E, autoNegEnabled, NULL));

        data = ((autoNegEnabled) ? (final_timer & 0xFFFF) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, AAS_LPCS_LINK_TIMER_0_P0_TIMER15_1_E, data, NULL));

        data = ((autoNegEnabled) ? ((final_timer >> 16) & 0x1F) : (0x0));
        CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, AAS_LPCS_LINK_TIMER_1_P0_TIMER20_16_E, data, NULL));

        if (GT_FALSE == autoNegEnabled)
        {
            if (portSpeed == MV_HWS_PORT_SPEED_10M_E)
            {
                sgmiiSpeed = 0;
            }
            else if (portSpeed == MV_HWS_PORT_SPEED_100M_E)
            {
                sgmiiSpeed = 1;
            }
            else
            {
                sgmiiSpeed = 2;
            }

            /* m_RAL.lpcs_units_RegFile.p_IF_MODE[portIndex].p_Sgmii_speed.set(sgmii_speed); */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, AAS_LPCS_IF_MODE_P0_SGMII_SPEED_E, sgmiiSpeed, NULL));
        }
        else
        {
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, AAS_LPCS_DEV_ABILITY_P0_LD_ABILITY_RSV05_E, 1, NULL));
        }

        break;

    case GT_TRUE:
        switch (portMode)
        {
            case _10G_QXGMII:
            case _5G_DXGMII:
            case _2_5G_SXGMII:
                portTypeDefaultSpeed = 2500;
                break;

            case _10G_DXGMII:
            case _5G_SXGMII:
                portTypeDefaultSpeed = 5000;
                break;

            case _10G_SXGMII:
                portTypeDefaultSpeed = 10000;
                break;

            case _5G_QUSGMII:
            case _10G_OUSGMII:
                /**
                 * Adjust resolution after AN process completed.
                 *
                 * In OUSGMII, the speed field is 3bits – [11:9] in
                 *
                 * <MTIP_IP>MTIP_IP/<MTIP_IP>
                 * LPCS/LPCS Units %j/PORT<%n>_DEV_ABILITY
                 *
                 * TBD for QUSGMII
                 */
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV9_E, &regData, NULL));
                data = (regData & 0x1);
                CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV10_E, &regData, NULL));
                data |= ((regData & 0x3) << 1);
                /**
                 * Set the relevant speed in
                 *
                 *       <MTIP_IP>MTIP_IP/<MTIP_IP>
                 *       LPCS/LPCS Units %j/PORT<%n>_IF_MODE
                 * In bit[3:2]
                 */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, AAS_LPCS_IF_MODE_P0_SGMII_SPEED_E, data, NULL));

                return GT_OK;

            default:
                return GT_NOT_SUPPORTED;
        }

        /**
         *  Get speed from link partner page in case of AN acknowledge
         */
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV9_E, &regData, NULL));
        data |= (regData & 0x1);
        CHECK_STATUS(genUnitRegisterFieldGet(devNum, 0, phyPortNum, unitId, IRONMAN_LPCS_PARTNER_ABILITY_PABILITY_RSV10_E, &regData, NULL));
        data |= ((regData & 0x3) << 1);
        switch (data)
        {
        case 0:
            portSpeed = MV_HWS_PORT_SPEED_10M_E;
            break;
        case 1:
            portSpeed = MV_HWS_PORT_SPEED_100M_E;
            break;
        case 2:
            portSpeed = MV_HWS_PORT_SPEED_1G_E;
            break;
        case 4:
            portSpeed = MV_HWS_PORT_SPEED_2_5G_E;
            break;
        case 5:
            portSpeed = MV_HWS_PORT_SPEED_5G_E;
            break;
        case 3:
            portSpeed = MV_HWS_PORT_SPEED_10G_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        /**
         * Calculate values for replication for confirmed speed
         */
        replication = ((GT_FLOAT32)portTypeDefaultSpeed / (GT_FLOAT32)portSpeed);
        break;

    default:
        return GT_NOT_SUPPORTED;
    }

    /**
     * The replication values must be set for both the desierd and
     * confirmed speed.
     */
    if (replication == 2.5)
    {
        valGmii_2_5 = 1;
        valGmiiRep = 0;
    }
    else
    {
        valGmii_2_5 = 0;
        valGmiiRep = (GT_U32)replication;
    }

    fieldNameGmiiRep = IRONMAN_LPCS_USXGMII_REP_USXGMIIREP_E;
    fieldNameGmii_2_5 = IRONMAN_LPCS_USXGMII_REP_USXGMII2_5_E;

    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmiiRep, valGmiiRep, NULL));
    CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, phyPortNum, unitId, fieldNameGmii_2_5, valGmii_2_5, NULL));

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasReplicationSet ******\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsExtIfFirstInGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per group type (4/8/16)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] groupType       - group type
* @param[out] *isFisrtPtr    - result if port is first or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfFirstInGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_STATE_GROUP_TYPE_ENT  groupType,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isFisrtPtr
)
{
    GT_STATUS               rc;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32                  firstPortInMacGroup, i;
    GT_BOOL                 macEnable;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        *isFisrtPtr = MV_HWS_PORT_IN_GROUP_FIRST_E;
        return GT_OK;
    }

    switch(groupType)
    {
        case MV_HWS_STATE_GROUP_MAC_TYPE_E:

            firstPortInMacGroup = portNum - ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8);
            for(i = 0; i < 8; i++)
            {
                mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup + i), _10GBase_KR, &macEnable);
                if(macEnable == GT_TRUE)
                {
                    break;
                }
                if(i == 0)
                {
                    mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _400GBase_KR8, &macEnable);
                    if(macEnable == GT_TRUE)
                    {
                        break;
                    }
                }
            }
            *isFisrtPtr = (i == 8) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;

            break;

        case MV_HWS_STATE_GROUP_PCS_TYPE_E:
            firstPortInMacGroup = portNum - (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS);
            for(i = 0; i < 16; i++)
            {
                mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _10GBase_KR, &macEnable);
                if(macEnable == GT_TRUE)
                {
                    break;
                }
                if((i == 0)||(i == 8))
                {
                    mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _400GBase_KR8, &macEnable);
                    if(macEnable == GT_TRUE)
                    {
                        break;
                    }
                }
            }
            *isFisrtPtr = (i == 16) ? MV_HWS_PORT_IN_GROUP_FIRST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            break;

        case MV_HWS_STATE_GROUP_SD_TYPE_E:
            *isFisrtPtr = MV_HWS_PORT_IN_GROUP_FIRST_E;
            break;
        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsExtIfLastInGroupCheck function
* @endinternal
*
* @brief  Shared resources validation check - per group type (4/8/16)
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[in] groupType       - group type
* @param[out] *isLastPtr    - result if port is first or not
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsExtIfLastInGroupCheck
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN MV_HWS_STATE_GROUP_TYPE_ENT  groupType,
    OUT MV_HWS_PORT_IN_GROUP_ENT    *isLastPtr
)
{
    GT_STATUS               rc;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32                  firstPortInMacGroup, i;
    GT_BOOL                 macEnable;

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);

    if(mvHwsMtipIsReducedPort(devNum, portNum))
    {
        *isLastPtr = MV_HWS_PORT_IN_GROUP_LAST_E;
        return GT_OK;
    }

    switch(groupType)
    {
        case MV_HWS_STATE_GROUP_MAC_TYPE_E:
            firstPortInMacGroup = portNum - ((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8);
            for(i = 0; i < 8; i++)
            {
                if((firstPortInMacGroup+i) == portNum)
                {
                    continue;
                }
                mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _10GBase_KR, &macEnable);
                if(macEnable == GT_TRUE)
                {
                    break;
                }
                if(i == 0)
                {
                    mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _400GBase_KR8, &macEnable);
                    if(macEnable == GT_TRUE)
                    {
                        break;
                    }
                }
            }
            *isLastPtr = (i == 8) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            break;

        case MV_HWS_STATE_GROUP_PCS_TYPE_E:
            firstPortInMacGroup = portNum - (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS);
            for(i = 0; i < 16; i++)
            {
                if((firstPortInMacGroup+i) == portNum)
                {
                    continue;
                }
                mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _10GBase_KR, &macEnable);
                if(macEnable == GT_TRUE)
                {
                    break;
                }
                if((i == 0) || (i == 8))
                {
                    mvHwsAasExtMacClockEnableGet(devNum, (firstPortInMacGroup+i), _400GBase_KR8, &macEnable);
                    if(macEnable == GT_TRUE)
                    {
                        break;
                    }
                }
            }
            *isLastPtr = (i == 16) ? MV_HWS_PORT_IN_GROUP_LAST_E : MV_HWS_PORT_IN_GROUP_EXISTING_E;
            break;

        case MV_HWS_STATE_GROUP_SD_TYPE_E:
            *isLastPtr = MV_HWS_PORT_IN_GROUP_LAST_E;
            break;

        default:
            return GT_BAD_PARAM;
    }

    return GT_OK;
}

/**
* @internal mvHwsAasEthPortPowerUp function
* @endinternal
*
* @brief  port init seq
*
* @param[in] devNum                 - system device number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] portInitInParamPtr     - port info struct
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsAasEthPortPowerUp
(
    IN GT_U8                            devNum,
    IN GT_U32                           phyPortNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS    *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    MV_HWS_PORT_IN_GROUP_ENT stateInPcsPortGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;
    MV_HWS_PORT_IN_GROUP_ENT stateInMacPortGroup = MV_HWS_PORT_IN_GROUP_FIRST_E;
    MV_HWS_PORT_ATTRIBUTES_INPUT_PARAMS portAttributes;

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("\n****** Start of mvHwsAasEthPortPowerUp for portNum=%d ******\n", phyPortNum);
    }
#endif

    HWS_NULL_PTR_CHECK_MAC(portInitInParamPtr);
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsExtIfFirstInGroupCheck(devNum, phyPortNum, portMode, MV_HWS_STATE_GROUP_MAC_TYPE_E, &stateInMacPortGroup));
    CHECK_STATUS(mvHwsExtIfFirstInGroupCheck(devNum, phyPortNum, portMode, MV_HWS_STATE_GROUP_PCS_TYPE_E, &stateInPcsPortGroup));

    /*pcs_ext_manager:pcs_clock_enable_and_reset_release*/
    /* global configuration for 16 ports - stateInPcsGroup should be used */
    CHECK_STATUS(mvHwsAasExtPcsClockAndResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInPcsPortGroup));

    /*pcs_ext_manager:pcs400_clock*/
    /*pcs_ext_manager:pcs400_reset_set*/
    /* 2-bit field (1 per 8 ports) - stateInMacGroup should be used */
    CHECK_STATUS(mvHwsAasExtPcs400ClockAndResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInMacPortGroup));

    /*pcs_ext_manager:pcs100_clock*/
    /*pcs_ext_manager:pcs100_reset_set*/
    CHECK_STATUS(mvHwsAasExtPcs100ClockAndResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInMacPortGroup));

    /*pcs_ext_manager:rsfec_clock*/
    /*pcs_ext_manager:rsfec_reset_set*/
    /* 2-bit field (1 per 8 ports) - stateInMacGroup should be used */
    CHECK_STATUS(mvHwsAasExtRsFecClockAndResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInMacPortGroup));

    /*pcs_ext_manager:change_mux_configuration x8*/
    /*KALEX: SHOULD BE IMPLEMENTED AS EXTERNAL API */

    /*pcs_ext_manager:pcs_PMA_control */
    CHECK_STATUS(mvHwsAasExtPcsPmaControlSet(devNum, phyPortNum, portMode, GT_TRUE));
    /*pcs_ext_manager:pcs_FEC_control */
    CHECK_STATUS(mvHwsAasExtPcsFecControlSet(devNum, phyPortNum, portMode, GT_TRUE));

    /*pcs_ext_manager:pcs_Channel_Control */
    CHECK_STATUS(mvHwsAasExtPcsChannelControlSet(devNum, phyPortNum, portMode, GT_TRUE));

    /*pcs_int_manager:pcs400_speed_select */
    /*pcs_int_manager:pcs400_marker_value*/
    /*pcs_int_manager:pcs400_marker_interval*/

    /*pcs_int_manager.pcs100_vendor_mode*/
    /*pcs_int_manager.pcs100_marker_value*/
    /*pcs_int_manager.pcs100_marker_interval*/
    /*pcs_int_manager.RS_FEC_pad*/
    CHECK_STATUS(mvHwsPcsModeCfg(devNum, 0, phyPortNum, portMode, &portAttributes));

    /*pcs_int_manager:pcs400_SW_reset*/
    /*pcs_int_manager.pcs100_SW_reset*/
    CHECK_STATUS(mvHwsPcsReset(devNum, 0, phyPortNum, portMode, curPortParams.portPcsType, UNRESET));
    /*pcs_ext_manager:sd_reset_set x8*/
    CHECK_STATUS(mvHwsAasExtReset(devNum, phyPortNum, portMode, GT_FALSE));


    /*m_ext_manager:set_res_speed */
    CHECK_STATUS(mvHwsAasExtMacSpeedSet(devNum, phyPortNum, portMode, GT_TRUE));

    /*m_ext_manager:mac_double_frequency */
    CHECK_STATUS(mvHwsAasExtMacDoubleFreqSet(devNum, phyPortNum, portMode, GT_TRUE));

    /*m_ext_manager:mac_clock_enable_and_reset_release */
    CHECK_STATUS(mvHwsAasExtMacClockAndResetRelease(devNum, phyPortNum, portMode, GT_TRUE, stateInMacPortGroup));

    /*m_ext_manager:set_mac_mux_ctrl */
    CHECK_STATUS(mvHwsAasExtMacMuxCtrlSet(devNum, phyPortNum, portMode, portInitInParamPtr->isPreemptionEnabled));

    /*mac_ext_manager::set_fec91_ena*/
    CHECK_STATUS(mvHwsAasExtMacFec91EnaSet(devNum, phyPortNum, portMode, GT_TRUE));

    portAttributes.preemptionEnable = portInitInParamPtr->isPreemptionEnabled;
    portAttributes.portSpeed = portInitInParamPtr->portSpeed;
    portAttributes.duplexMode = portInitInParamPtr->duplexMode;

    CHECK_STATUS(mvHwsMacModeCfg(devNum, 0, phyPortNum, portMode, &portAttributes));

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsAasEthPortPowerUp for portNum=%d, portMode = %d ******\n\n", phyPortNum, portMode);
    }
#endif

    return GT_OK;
}


/**
* @internal mvHwsAasEthPortPowerDown function
* @endinternal
*
* @brief  port power down seq
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAasEthPortPowerDown
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    MV_HWS_RESET pcsReset = POWER_DOWN;
    MV_HWS_PORT_ACTION action = PORT_POWER_DOWN;
    MV_HWS_PORT_IN_GROUP_ENT stateInMacPortGroup = MV_HWS_PORT_IN_GROUP_LAST_E;
    MV_HWS_PORT_IN_GROUP_ENT stateInPcsPortGroup = MV_HWS_PORT_IN_GROUP_LAST_E;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("******mvHwsAasEthPortPowerDown: port %d delete mode %d  port ******\n", phyPortNum, portMode);
    }
#endif

    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, 0, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsExtIfLastInGroupCheck(devNum, phyPortNum, portMode, MV_HWS_STATE_GROUP_MAC_TYPE_E, &stateInMacPortGroup));
    CHECK_STATUS(mvHwsExtIfLastInGroupCheck(devNum, phyPortNum, portMode, MV_HWS_STATE_GROUP_PCS_TYPE_E, &stateInPcsPortGroup));

    CHECK_STATUS(mvHwsAasExtMacSpeedSet(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsAasExtMacDoubleFreqSet(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsAasExtMacMuxCtrlSet(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsAasExtMacFec91EnaSet(devNum, phyPortNum, portMode, GT_FALSE));

    CHECK_STATUS(mvHwsAasExtPcsPmaControlSet(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsAasExtPcsFecControlSet(devNum, phyPortNum, portMode, GT_FALSE));
    CHECK_STATUS(mvHwsAasExtPcsChannelControlSet(devNum, phyPortNum, portMode, GT_FALSE));

    CHECK_STATUS(mvHwsAasExtReset(devNum, phyPortNum, portMode, GT_TRUE));
    CHECK_STATUS(mvHwsPortStopCfg(devNum, 0, phyPortNum, portMode, action, &(curLanesList[0]), pcsReset, POWER_DOWN));

    CHECK_STATUS(mvHwsAasExtRsFecClockAndResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInMacPortGroup));
    CHECK_STATUS(mvHwsAasExtPcs400ClockAndResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInMacPortGroup));
    CHECK_STATUS(mvHwsAasExtPcs100ClockAndResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInMacPortGroup));

    CHECK_STATUS(mvHwsAasExtPcsClockAndResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInPcsPortGroup));
    CHECK_STATUS(mvHwsAasExtMacClockAndResetRelease(devNum, phyPortNum, portMode, GT_FALSE, stateInMacPortGroup));

#ifndef RAVEN_DEV_SUPPORT
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** End of mvHwsAasEthPortPowerDown for portNum=%d, portMode = %d ******\n\n", phyPortNum, portMode);
    }
#endif

    return GT_OK;
}

/**
* @internal mvHwsAasAnpSerdesSdwMuxSet function
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
GT_STATUS mvHwsAasSerdesSdwMuxSet
(
    IN  GT_U8                devNum,
    IN  GT_U32               serdesNum,
    IN  GT_BOOL              anpEnable
)
{
    GT_STATUS       rc;
    GT_UREG_DATA    laneMuxControl0,    laneMuxControl0Mask;
    GT_UREG_DATA    laneMuxControl1,    laneMuxControl1Mask;
    GT_UREG_DATA    generalMuxControl0, generalMuxControl0Mask;
    GT_U32          unitAddr;

    if (anpEnable)
    {
        laneMuxControl0     = 0x400000;
        laneMuxControl0Mask = 0xFFFFFFF;
        laneMuxControl1     = 0x0;
        laneMuxControl1Mask = 0xFFFFF;
    }
    else
    {
        laneMuxControl0     = 0x1F000FF;
        laneMuxControl0Mask = 0xFFFFFFF;
        laneMuxControl1     = 0xF3E7F;
        laneMuxControl1Mask = 0xFFFFF;
    }
    generalMuxControl0      = 0x39F;
    generalMuxControl0Mask  = 0x39F;


    rc = mvHwsAasUnitBaseAddrCalc(devNum, SERDES_UNIT, _10GBase_KR, serdesNum, &unitAddr);
    if((unitAddr == 0) || (rc != GT_OK))
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    if(serdesNum == 80)
    {
           hwsOsPrintf("\n\n\n");
           hwsOsPrintf("mvHwsSerdesSdwMuxSet: IMPLEMENTATION NEEDED for CPU port (SD=80)");
           hwsOsPrintf("\n\n\n");
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, unitAddr + 0x528 + 0x300 * (serdesNum % 4), laneMuxControl0,    laneMuxControl0Mask));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, unitAddr + 0x52C + 0x300 * (serdesNum % 4), laneMuxControl1,    laneMuxControl1Mask));
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, 0, unitAddr + 0x010, generalMuxControl0, generalMuxControl0Mask));
    }

    return GT_OK;
}

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC aas_MAC400_400G_R8_RegDb[] =
{
    /* ANP Ingress_SD_Mux_Control + Egress_SD_Mux_Control */
    {0x00A80018, 0x83828180, 0xFFFFFFFF},
    {0x00A800F8, 0x83828180, 0xFFFFFFFF},
    {0x00A8001C, 0x87868584, 0xFFFFFFFF},
    {0x00A800FC, 0x87868584, 0xFFFFFFFF},
    {0x00A810D8, 0x0f0000ff, 0xFFFFFFFF},   /*RAL_ANP.anp_units_RegFile.Lane_Control_7*/

    /*PCS*/
    {0x0066001C, 0x00000083, 0xFFFFFFFF},   /*Global_Clock_Enable*/
    {0x00660020, 0x00ff00ff, 0xFFFFFFFF},   /*Global_Reset_Control*/
    {0x00660024, 0x00000441, 0xFFFFFFFF},   /*Global_Reset_Control2*/
    {0x00660000, 0x000000FF, 0xFFFFFFFF},   /*MTIP_Global_PMA_Control*/
    {0x00660008, 0x00ff00ff, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control_2*/
    {0x0066000c, 0x00000203, 0xFFFFFFFF},   /*MTIP_Global_Channel_Control*/
    {0x00677000, 0x00008000, 0xFFFFFFFF},   /*PCS400_RALEX.CONTROL1*/
    {0x00677820, 0x00004a9a, 0xFFFFFFFF},   /*VENDOR_AM_0*/
    {0x00677824, 0x00000026, 0xFFFFFFFF},   /*VENDOR_AM_1*/
    {0x00671180, 0x00007101, 0xFFFFFFFF},   /*VENDOR_VL0_0*/
    {0x00671184, 0x000000f3, 0xFFFFFFFF},   /*VENDOR_VL0_1*/
    {0x00671188, 0x0000de5a, 0xFFFFFFFF},   /*VENDOR_VL1_0*/
    {0x0067118c, 0x0000007e, 0xFFFFFFFF},   /*VENDOR_VL1_1*/
    {0x00671190, 0x0000f33e, 0xFFFFFFFF},   /*VENDOR_VL2_0*/
    {0x00671194, 0x00000056, 0xFFFFFFFF},   /*VENDOR_VL2_1*/
    {0x00671198, 0x00008086, 0xFFFFFFFF},   /*VENDOR_VL3_0*/
    {0x0067119c, 0x000000d0, 0xFFFFFFFF},   /*VENDOR_VL3_1*/
    {0x006711a0, 0x0000512a, 0xFFFFFFFF},   /*VENDOR_VL4_0*/
    {0x006711a4, 0x000000f2, 0xFFFFFFFF},   /*VENDOR_VL4_1*/
    {0x006711a8, 0x00004f12, 0xFFFFFFFF},   /*VENDOR_VL5_0*/
    {0x006711ac, 0x000000d1, 0xFFFFFFFF},   /*VENDOR_VL5_1*/
    {0x006711b0, 0x00009c42, 0xFFFFFFFF},   /*VENDOR_VL6_0*/
    {0x006711b4, 0x000000a1, 0xFFFFFFFF},   /*VENDOR_VL6_1*/
    {0x006711b8, 0x000076d6, 0xFFFFFFFF},   /*VENDOR_VL7_0*/
    {0x006711bc, 0x0000005b, 0xFFFFFFFF},   /*VENDOR_VL7_1*/
    {0x00677808, 0x00002000, 0xFFFFFFFF},   /*PCS400_RALEX.VENDOR_VL_INTVL*/

    /*MAC*/
    {0x00E00924, 0x0000000a, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x00E00928, 0x00000001, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_Control2*/
    {0x00E00000, 0x000f0000, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x00E0097C, 0x00000003, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/
    {0x00E00170, 0x11000100, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/
    {0x00E00008, 0x0bc207ff, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_Control*/
    {0x00E09044, 0x5000000c, 0xFFFFFFFF},   /*TX_IPG_LENGTH*/
    {0x00E09008, 0x00082810, 0xFFFFFFFF},   /*COMMAND_CONFIG*/
    {0x00E09020, 0x00000007, 0xFFFFFFFF},   /*TX_FIFO_SECTIONS*/
    {0x00E0901c, 0x00000001, 0xFFFFFFFF},   /*RX_FIFO_SECTIONS*/
    {0x00E09080, 0x00000020, 0xFFFFFFFF},   /*XIF_MODE*/

};

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC aas_MAC400_800G_R8_RegDb[] =
{
    /* ANP Ingress_SD_Mux_Control + Egress_SD_Mux_Control */
    {0x00A80018, 0x83828180, 0xFFFFFFFF},
    {0x00A800F8, 0x83828180, 0xFFFFFFFF},
    {0x00A8001C, 0x87868584, 0xFFFFFFFF},
    {0x00A800FC, 0x87868584, 0xFFFFFFFF},
    {0x00A810D8, 0x0f0000ff, 0xFFFFFFFF},   /*RAL_ANP.anp_units_RegFile.Lane_Control_7*/

    /*PCS*/
    {0x0066001C, 0x00000187, 0xFFFFFFFF},   /*Global_Clock_Enable*/
    {0x00660020, 0x00ff00ff, 0xFFFFFFFF},   /*Global_Reset_Control*/
    {0x00660024, 0x000004c3, 0xFFFFFFFF},   /*Global_Reset_Control2*/
    {0x00660000, 0x00ff0000, 0xFFFFFFFF},   /*MTIP_Global_PMA_Control*/
    {0x00660008, 0x00ff00ff, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control_2*/
    {0x0066000c, 0x0000030f, 0xFFFFFFFF},   /*MTIP_Global_Channel_Control*/
    {0x00677000, 0x0000a060, 0xFFFFFFFF},   /*PCS400_RALEX.CONTROL1*/
    {0x00679000, 0x0000a060, 0xFFFFFFFF},   /*PCS400_RALEX.CONTROL1*/

    {0x00671180, 0x000071fe, 0xFFFFFFFF},   /*VENDOR_VL0_0*/
    {0x00671188, 0x0000dea5, 0xFFFFFFFF},   /*VENDOR_VL1_0*/
    {0x00671190, 0x0000f3c1, 0xFFFFFFFF},   /*VENDOR_VL2_0*/
    {0x00671198, 0x00008079, 0xFFFFFFFF},   /*VENDOR_VL3_0*/
    {0x006711a0, 0x000051d5, 0xFFFFFFFF},   /*VENDOR_VL4_0*/
    {0x006711a8, 0x00004fed, 0xFFFFFFFF},   /*VENDOR_VL5_0*/
    {0x006711b0, 0x00009cbd, 0xFFFFFFFF},   /*VENDOR_VL6_0*/
    {0x006711b8, 0x00007629, 0xFFFFFFFF},   /*VENDOR_VL7_0*/
    {0x006711c0, 0x0000731e, 0xFFFFFFFF},   /*VENDOR_VL0_0*/
    {0x006711c8, 0x0000c48e, 0xFFFFFFFF},   /*VENDOR_VL1_0*/
    {0x006711d0, 0x0000eb6a, 0xFFFFFFFF},   /*VENDOR_VL2_0*/
    {0x006711d8, 0x000066dd, 0xFFFFFFFF},   /*VENDOR_VL3_0*/
    {0x006711e0, 0x0000f65d, 0xFFFFFFFF},   /*VENDOR_VL4_0*/
    {0x006711e8, 0x000097ce, 0xFFFFFFFF},   /*VENDOR_VL5_0*/
    {0x006711f0, 0x0000fb35, 0xFFFFFFFF},   /*VENDOR_VL6_0*/
    {0x006711f8, 0x0000ba59, 0xFFFFFFFF},   /*VENDOR_VL7_0*/

    {0x00672180, 0x00008e01, 0xFFFFFFFF},   /*VENDOR_VL0_0*/
    {0x00672184, 0x0000000c, 0xFFFFFFFF},   /*VENDOR_VL0_1*/
    {0x00672188, 0x0000215a, 0xFFFFFFFF},   /*VENDOR_VL1_0*/
    {0x0067218c, 0x00000081, 0xFFFFFFFF},   /*VENDOR_VL1_1*/
    {0x00672190, 0x00000c3e, 0xFFFFFFFF},   /*VENDOR_VL2_0*/
    {0x00672194, 0x000000a9, 0xFFFFFFFF},   /*VENDOR_VL2_1*/
    {0x00672198, 0x00007f86, 0xFFFFFFFF},   /*VENDOR_VL3_0*/
    {0x0067219c, 0x0000002f, 0xFFFFFFFF},   /*VENDOR_VL3_1*/
    {0x006721a0, 0x0000ae2a, 0xFFFFFFFF},   /*VENDOR_VL4_0*/
    {0x006721a4, 0x0000000d, 0xFFFFFFFF},   /*VENDOR_VL4_1*/
    {0x006721a8, 0x0000b012, 0xFFFFFFFF},   /*VENDOR_VL5_0*/
    {0x006721ac, 0x0000002e, 0xFFFFFFFF},   /*VENDOR_VL5_1*/
    {0x006721b0, 0x00006342, 0xFFFFFFFF},   /*VENDOR_VL6_0*/
    {0x006721b4, 0x0000005e, 0xFFFFFFFF},   /*VENDOR_VL6_1*/
    {0x006721b8, 0x000089d6, 0xFFFFFFFF},   /*VENDOR_VL7_0*/
    {0x006721bc, 0x000000a4, 0xFFFFFFFF},   /*VENDOR_VL7_1*/
    {0x006721c0, 0x00008ce1, 0xFFFFFFFF},   /*VENDOR_VL8_0*/
    {0x006721c4, 0x0000008a, 0xFFFFFFFF},   /*VENDOR_VL8_1*/
    {0x006721c8, 0x00003b71, 0xFFFFFFFF},   /*VENDOR_VL9_0*/
    {0x006721cc, 0x000000c3, 0xFFFFFFFF},   /*VENDOR_VL9_1*/
    {0x006721d0, 0x00001495, 0xFFFFFFFF},   /*VENDOR_VL10_0*/
    {0x006721d4, 0x00000027, 0xFFFFFFFF},   /*VENDOR_VL10_1*/
    {0x006721d8, 0x00009922, 0xFFFFFFFF},   /*VENDOR_VL11_0*/
    {0x006721dc, 0x000000c7, 0xFFFFFFFF},   /*VENDOR_VL11_1*/
    {0x006721e0, 0x000009a2, 0xFFFFFFFF},   /*VENDOR_VL12_0*/
    {0x006721e4, 0x0000006a, 0xFFFFFFFF},   /*VENDOR_VL12_1*/
    {0x006721e8, 0x00006831, 0xFFFFFFFF},   /*VENDOR_VL13_0*/
    {0x006721ec, 0x0000003c, 0xFFFFFFFF},   /*VENDOR_VL13_1*/
    {0x006721f0, 0x000004ca, 0xFFFFFFFF},   /*VENDOR_VL14_0*/
    {0x006721f4, 0x00000059, 0xFFFFFFFF},   /*VENDOR_VL14_1*/
    {0x006721f8, 0x000045a6, 0xFFFFFFFF},   /*VENDOR_VL15_0*/
    {0x006721fc, 0x00000086, 0xFFFFFFFF},   /*VENDOR_VL15_1*/

    {0x00677808, 0x00002000, 0xFFFFFFFF},   /*PCS400_RALEX.VENDOR_VL_INTVL*/
    {0x00679808, 0x00002000, 0xFFFFFFFF},   /*PCS400_RALEX.VENDOR_VL_INTVL*/

    /*MAC*/
    {0x00E00924, 0x0000000c, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x00E00928, 0x00000001, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_Control2*/
    {0x00E00000, 0x000f0000, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x00E0097C, 0x00000007, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/
    {0x00E00170, 0x11000100, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/
    {0x00E00008, 0x0bc207ff, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_Control*/
    {0x00E09044, 0x5000000c, 0xFFFFFFFF},   /*TX_IPG_LENGTH*/
    {0x00E09008, 0x00082813, 0xFFFFFFFF},   /*COMMAND_CONFIG*/
    {0x00E09020, 0x00000007, 0xFFFFFFFF},   /*TX_FIFO_SECTIONS*/
    {0x00E0901c, 0x00000001, 0xFFFFFFFF},   /*RX_FIFO_SECTIONS*/

};

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC aas_MAC100_50G_R1_RegDb[] =
{
    /* ANP Ingress_SD_Mux_Control + Egress_SD_Mux_Control */
    {0x00A80018, 0x00000080, 0xFFFFFFFF},
    {0x00A800F8, 0x00000080, 0xFFFFFFFF},
    {0x00A8001C, 0x00000000, 0xFFFFFFFF},
    {0x00A800FC, 0x00000000, 0xFFFFFFFF},
    {0x00A810D8, 0x0f0000ff, 0xFFFFFFFF},   /*RAL_ANP.anp_units_RegFile.Lane_Control_7*/

    /*PCS*/
    {0x0066001C, 0x0000000b, 0xFFFFFFFF},   /*Global_Clock_Enable*/
    {0x00660020, 0x00010001, 0xFFFFFFFF},   /*Global_Reset_Control*/
    {0x00660024, 0x00000444, 0xFFFFFFFF},   /*Global_Reset_Control2*/
    {0x00660000, 0x00000001, 0xFFFFFFFF},   /*MTIP_Global_PMA_Control*/
    {0x00660004, 0x00000000, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control*/
    {0x00660008, 0x00010001, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control_2*/
    {0x0066000c, 0x00000200, 0xFFFFFFFF},   /*MTIP_Global_Channel_Control*/

    {0x00661840, 0x00000000, 0xFFFFFFFF},   /*PCS100_RALEX.VENDOR_PCS_MODE*/
    {0x00661900, 0x00007690, 0x0000FFFF},
    {0x00661904, 0x00000047, 0x000000FF},
    {0x00661908, 0x0000C4F0, 0x0000FFFF},
    {0x0066190C, 0x000000E6, 0x000000FF},
    {0x00661910, 0x000065C5, 0x0000FFFF},
    {0x00661914, 0x0000009B, 0x000000FF},
    {0x00661918, 0x000079A2, 0x0000FFFF},
    {0x0066191C, 0x0000003D, 0x000000FF},
    {0x00661808, 0x00004FFF, 0x0000FFFF},
    {0x00671000, 0x00000600, 0xFFFFFFFF},   /*RSFEC_RALEX.rsfec_RegFile.p_RSFEC_CONTROL[0]*/

    {0x00661000, 0x0000a040, 0x0000FFFF},   /*PCS100_RALEX.CONTROL1*/

    /*MAC*/
    {0x00E00000, 0x000f0000, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x00E00008, 0x0bc207ff, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_Control*/

    {0x00E00100, 0x0013ff10, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_Control*/
    {0x00E00124, 0x00000008, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x00E00128, 0x00000001, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_Control2*/
    {0x00E0017C, 0x00000003, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/
    {0x00E00170, 0x11000100, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/

    {0x00E01044, 0x4fff000c, 0xFFFFFFFF},   /*TX_IPG_LENGTH*/
    {0x00E01048, 0x00000000, 0xFFFFFFFF},   /**/
    {0x00E01008, 0x02002803, 0xFFFFFFFF},   /*COMMAND_CONFIG*/
    {0x00E01108, 0x00080003, 0xFFFFFFFF},   /*EMAC_COMMAND_CONFIG*/
    {0x00E010a8, 0x02780a00, 0xFFFFFFFF},   /*BR_CONTROL*/
    {0x00E01080, 0x00000100, 0xFFFFFFFF},   /*XIF_MODE, Onestepena=0*/
#if 0
    {0x00E01020, 0x0000000b, 0xFFFFFFFF},   /*TX_FIFO_SECTIONS*/
#endif
    {0x00E01120, 0x0000000b, 0xFFFFFFFF},   /*EMAC_TX_FIFO_SECTIONS*/

};

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC aas_MAC100_25G_R1_RS_FEC_RegDb[] =
{
    /* ANP Ingress_SD_Mux_Control + Egress_SD_Mux_Control */
    {0x00A80018, 0x00000080, 0xFFFFFFFF},
    {0x00A800F8, 0x00000080, 0xFFFFFFFF},
    {0x00A8001C, 0x00000000, 0xFFFFFFFF},
    {0x00A800FC, 0x00000000, 0xFFFFFFFF},
    {0x00A810D8, 0x0f0000ff, 0xFFFFFFFF},   /*RAL_ANP.anp_units_RegFile.Lane_Control_7*/

    /*PCS*/
    {0x0066001C, 0x0000000b, 0xFFFFFFFF},   /*Global_Clock_Enable*/
    {0x00660020, 0x00010001, 0xFFFFFFFF},   /*Global_Reset_Control*/
    {0x00660024, 0x00000444, 0xFFFFFFFF},   /*Global_Reset_Control2*/
    {0x00660000, 0x00000000, 0xFFFFFFFF},   /*MTIP_Global_PMA_Control*/
    {0x00660004, 0x00000000, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control*/
    {0x00660008, 0x00000001, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control_2*/
    {0x0066000c, 0x00000200, 0xFFFFFFFF},   /*MTIP_Global_Channel_Control*/
    {0x00660010, 0x00001000, 0x00001000},   /*MTIP_Global_Channel_Control_2*/

    {0x00661840, 0x00000005, 0xFFFFFFFF},   /*PCS100_RALEX.VENDOR_PCS_MODE*/
    {0x00661900, 0x000068c1, 0x0000FFFF},
    {0x00661904, 0x00000021, 0x000000FF},
    {0x00661908, 0x0000C4F0, 0x0000FFFF},
    {0x0066190C, 0x000000E6, 0x000000FF},
    {0x00661910, 0x000065C5, 0x0000FFFF},
    {0x00661914, 0x0000009B, 0x000000FF},
    {0x00661918, 0x000079A2, 0x0000FFFF},
    {0x0066191C, 0x0000003D, 0x000000FF},
    {0x00661808, 0x00004FFF, 0x0000FFFF},
    {0x00671000, 0x00000200, 0xFFFFFFFF},   /*RSFEC_RALEX.rsfec_RegFile.p_RSFEC_CONTROL[0]*/

    {0x00661000, 0x0000a040, 0x0000FFFF},   /*PCS100_RALEX.CONTROL1*/

    /*MAC*/
    {0x00E00124, 0x00000006, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x00E00128, 0x00000001, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_Control2*/
    {0x00E00000, 0x000f0000, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x00E0017C, 0x00000003, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/
    {0x00E00170, 0x01000100, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/
    {0x00E00008, 0x1bc207ff, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_Control*/
    {0x00E01044, 0x4fff000c, 0xFFFFFFFF},   /*TX_IPG_LENGTH*/
    {0x00E01008, 0x02002803, 0xFFFFFFFF},   /*COMMAND_CONFIG*/
    {0x00E01108, 0x00080003, 0xFFFFFFFF},   /*EMAC_COMMAND_CONFIG*/
    {0x00E010a8, 0x03780a00, 0xFFFFFFFF},   /*BR_CONTROL*/
    {0x00E01080, 0x00000101, 0xFFFFFFFF},   /*XIF_MODE, Onestepena=0*/
#if 0
    {0x00E01020, 0x0000000b, 0xFFFFFFFF},   /*TX_FIFO_SECTIONS*/
#endif
    {0x00E01120, 0x0000000b, 0xFFFFFFFF},   /*EMAC_TX_FIFO_SECTIONS*/

};

const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC aas_MAC100_10G_R1_NO_FEC_RegDb[] =
{
    /* ANP Ingress_SD_Mux_Control + Egress_SD_Mux_Control */
    {0x00A80018, 0x00000080, 0xFFFFFFFF},
    {0x00A800F8, 0x00000080, 0xFFFFFFFF},
    {0x00A8001C, 0x00000000, 0xFFFFFFFF},
    {0x00A800FC, 0x00000000, 0xFFFFFFFF},
    {0x00A810D8, 0x0f0000ff, 0xFFFFFFFF},   /*RAL_ANP.anp_units_RegFile.Lane_Control_7*/

    /*PCS*/
    {0x0066001C, 0x00000009, 0xFFFFFFFF},   /*Global_Clock_Enable*/
    {0x00660020, 0x00010001, 0xFFFFFFFF},   /*Global_Reset_Control*/
    {0x00660024, 0x00000404, 0xFFFFFFFF},   /*Global_Reset_Control2*/
    {0x00660000, 0x00000000, 0xFFFFFFFF},   /*MTIP_Global_PMA_Control*/
    {0x00660004, 0x00000000, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control*/
    {0x00660008, 0x00000000, 0xFFFFFFFF},   /*MTIP_Global_FEC_Control_2*/
    {0x0066000c, 0x00000200, 0xFFFFFFFF},   /*MTIP_Global_Channel_Control*/
    {0x00660010, 0x00001000, 0x00001000},   /*MTIP_Global_Channel_Control_2*/

    {0x00661840, 0x00000003, 0x0000000F},   /*PCS100_RALEX.VENDOR_PCS_MODE*/
    {0x00661808, 0x00000000, 0x0000FFFF},
    {0x00671000, 0x00000000, 0xFFFFFFFF},   /*RSFEC_RALEX.rsfec_RegFile.p_RSFEC_CONTROL[0]*/

    {0x00661000, 0x0000a040, 0x0000FFFF},   /*PCS100_RALEX.CONTROL1*/

    /*MAC*/
    {0x00E00124, 0x00000005, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_Control1*/
    {0x00E00128, 0x00000000, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_Control2*/
    {0x00E00000, 0x000f0000, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_MAC_Clock_and_Reset_Control*/
    {0x00E0017C, 0x00000003, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.Port_MAC_Clock_and_Reset_Control*/
    {0x00E00170, 0x01000100, 0xFFFFFFFF},   /*mac_seahawk_port_ext_units_RegFile.MTIP_Port_PMAC_Control*/
    {0x00E00008, 0x1bc207ff, 0xFFFFFFFF},   /*mac_seahawk_global_ext_units_RegFile.Global_Control*/
    {0x00E01044, 0x0000000c, 0xFFFFFFFF},   /*TX_IPG_LENGTH*/
    {0x00E01008, 0x02002803, 0xFFFFFFFF},   /*COMMAND_CONFIG*/
    {0x00E01108, 0x00080003, 0xFFFFFFFF},   /*EMAC_COMMAND_CONFIG*/
    {0x00E010a8, 0x03780a00, 0xFFFFFFFF},   /*BR_CONTROL*/
    {0x00E01080, 0x00000101, 0xFFFFFFFF},   /*XIF_MODE, Onestepena=0*/
#if 0
    {0x00E01020, 0x0000000b, 0xFFFFFFFF},   /*TX_FIFO_SECTIONS*/
#endif
    {0x00E01120, 0x0000000b, 0xFFFFFFFF},   /*EMAC_TX_FIFO_SECTIONS*/

};

GT_STATUS mvHwsAasPunktCheck(GT_U32 portType)
{
    GT_U32 i;
    GT_U32 iterNum;
    GT_U32 regData;
    const MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC *regPtr;

    if(portType == 0)
    {
        hwsOsPrintf(" 1 - MTI400 - 400G_R8       \n");
        hwsOsPrintf(" 2 - MTI400 - 800G_R8       \n");
        hwsOsPrintf(" 3 - MTI100 -  50G_R1       \n");
        hwsOsPrintf(" 4 - MTI100 -  25G_R1_RS_FEC\n");
        hwsOsPrintf(" 5 -     \n");
        hwsOsPrintf(" 6 -     \n");
        hwsOsPrintf(" 7 -     \n");
        hwsOsPrintf(" 8 -     \n");
        hwsOsPrintf(" 9 -     \n");
        hwsOsPrintf(" 10-     \n");
        hwsOsPrintf(" 11-     \n");
        hwsOsPrintf(" 12-     \n");
        hwsOsPrintf(" 13-     \n");
        hwsOsPrintf("\n");
        return GT_OK;
    }
    else if(portType == 1)
    {
        iterNum = sizeof(aas_MAC400_400G_R8_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = aas_MAC400_400G_R8_RegDb;
    }
    else if(portType == 2)
    {
        iterNum = sizeof(aas_MAC400_800G_R8_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = aas_MAC400_800G_R8_RegDb;
    }
    else if(portType == 3)
    {
        iterNum = sizeof(aas_MAC100_50G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = aas_MAC100_50G_R1_RegDb;
    }
    else if(portType == 4)
    {
        iterNum = sizeof(aas_MAC100_25G_R1_RS_FEC_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = aas_MAC100_25G_R1_RS_FEC_RegDb;
    }
    else if(portType == 5)
    {
        iterNum = sizeof(aas_MAC100_10G_R1_NO_FEC_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);
        regPtr = aas_MAC100_10G_R1_NO_FEC_RegDb;
    }
    else if(portType == 6)
    {
/*      iterNum = sizeof(phoenixMti100_50G_R2_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_50G_R2_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 7)
    {
/*      iterNum = sizeof(phoenixMti100_100G_R4_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_100G_R4_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 8)
    {
/*      iterNum = sizeof(phoenixMti100_SGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_SGMII_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 9) /* MTIUSX SGMII*/
    {
/*      iterNum = sizeof(phoenixMtiUSX_SGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMtiUSX_SGMII_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 10) /* MTI100 1000BaseX*/
    {
/*      iterNum = sizeof(phoenixMti100_1000BaseX_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMti100_1000BaseX_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 11) /* MTI100 10G_R1*/
    {
/*      iterNum = sizeof(ironman_MAC10G_BR_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = ironman_MAC10G_BR_10G_R1_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 12) /* MTI USX 10G OUSGMII */
    {
/*      iterNum = sizeof(phoenixMtiUsx_10G_OUSGMII_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMtiUsx_10G_OUSGMII_RegDb;                                                              */
        return GT_FAIL;
    }
    else if(portType == 13) /* USX 10G port*/
    {
/*      iterNum = sizeof(phoenixMtiUsx_10G_R1_RegDb) / sizeof(MV_HWS_PRV_PUNKT_EXPECTED_RESULT_DB_STC);   */
/*      regPtr = phoenixMtiUsx_10G_R1_RegDb;                                                              */
        return GT_FAIL;
    }
    else
    {
        hwsOsPrintf("ILLEGAL PORT TYPE\n");
        return GT_FAIL;
    }
    hwsOsPrintf("iterNum = %d\n\n", iterNum);

    for(i = 0; i < iterNum; i++)
    {
        CHECK_STATUS(hwsRegisterGetFuncPtr(0, 0, regPtr[i].regAddr, &regData, 0xFFFFFFFF));
        if ((regData & regPtr[i].mask) != (regPtr[i].expectedData & regPtr[i].mask))
        {
            hwsOsPrintf("Found mismatch: iter = %d, regAddr = 0x%x, data = 0x%x, expectedData = 0x%x\n", i, regPtr[i].regAddr, (regData& regPtr[i].mask), (regPtr[i].expectedData& regPtr[i].mask));
        }
    }

    return GT_OK;
}

GT_STATUS mvHwsAasPunktUnitBaseGet
(
    GT_U32                  macPortNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    GT_STATUS rc;
    MV_HWS_UNITS_ID unitId;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32          unitAddr = 0;
    GT_U32          localUnitNum = 0;
    GT_U32          unitIndex = 0;
    GT_U32 i;

    rc = mvHwsGlobalMacToLocalIndexConvert(0, macPortNum, portMode, &convertIdx);
    if(rc != GT_OK)
    {
        return rc;
    }
    unitId = MTI_10G_MAC_BR_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_MAC_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_MAC_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");


    unitId = MTI_10G_MAC_NON_BR_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_MAC_NON_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_MAC_NON_BR_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");

    unitId = MTI_10G_GLOBAL_EXT_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_GLOBAL_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_GLOBAL_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");

    unitId = MTI_10G_PORT_EXT_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("MTI_10G_PORT_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("MTI_10G_PORT_EXT_E: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");


    unitId = ANP_USX_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("ANP_USX_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("ANP_USX_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");

    unitId = ANP_CPU_UNIT;

    for(i = 0; i < 55; i++)
    {
        rc = mvUnitExtInfoGet(0, unitId, i, &unitAddr, &unitIndex, &localUnitNum);
        if((unitAddr == 0) || (rc != GT_OK))
        {
            hwsOsPrintf("ANP_CPU_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, MV_HWS_SW_PTR_ENTRY_UNUSED);
        }
        else
        {
            hwsOsPrintf("ANP_CPU_UNIT: MAC UNIT base addr for MAC_PORT_%d is 0x%x\n", i, unitAddr);
        }
    }
    hwsOsPrintf("\n");


    return GT_OK;
}

/**
* @internal mvHwsAasPortInit function
* @endinternal
*
* @brief   AAS port init
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portInitInParamPtr - Input parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAasPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;
    GT_U32 curLanesList[HWS_MAX_SERDES_NUM];
    /*GT_BOOL serdesInit = GT_TRUE;*/
    MV_HWS_REF_CLOCK_SUP_VAL  refClock;
    MV_HWS_REF_CLOCK_SOURCE   refClockSource;
    GT_U32 sdIndex;

    if (NULL == portInitInParamPtr)
    {
        return GT_BAD_PTR;
    }
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** mvHwsAasPortInit: port %d create mode %d ******\n", phyPortNum, portMode);
    }
#endif
    if (GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsAasAnpPortEnable(devNum, phyPortNum, portMode, portInitInParamPtr->apEnabled, GT_TRUE));

    refClock = portInitInParamPtr->refClock;
    refClockSource = portInitInParamPtr->refClockSource;

    if ( portInitInParamPtr->apEnabled == GT_FALSE )
    {
        /* rebuild active lanes list according to current configuration (redundancy) */
        CHECK_STATUS(mvHwsRebuildActiveLaneList(devNum, 0, phyPortNum, portMode, curLanesList));

        for(sdIndex = 0; sdIndex < curPortParams.numOfActLanes; sdIndex++)
        {
#ifndef ASIC_SIMULATION

            if(!cpssDeviceRunCheck_onEmulator())
            {
                /* CPSS_TBD_BOOKMACK_AAS kalex - need to change to AAS configuration */
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, curLanesList[sdIndex], SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_0_TX_IDLE_E, 1, NULL));
            }
#endif
            CHECK_STATUS(mvHwsAasSerdesSdwMuxSet(devNum, curLanesList[sdIndex], GT_FALSE));
        }

        if(!cpssDeviceRunCheck_onEmulator())
        {
            /* power up the serdes */
            CHECK_STATUS(mvHwsPortSerdesPowerUp(devNum, portGroup, phyPortNum, portMode, refClock, refClockSource, &(curLanesList[0])));
            CHECK_STATUS(mvHwsSerdesDigitalRfResetToggleSet(devNum, portGroup, phyPortNum, portMode, 10));
        }
    }
    /* init all modules as in ap port (msdb,mpf,mtipExt) */
    CHECK_STATUS(mvHwsAasEthPortPowerUp(devNum, phyPortNum, portMode, portInitInParamPtr));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL)
    {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("****** mvHwsAasPortInit: End ******\n");
    }
#endif
    return GT_OK;
}


/**
* @internal mvHwsAasPortReset function
* @endinternal
*
* @brief   Clears the port mode and release all its resources according to selected.
*         Does not verify that the selected mode/port number is valid at the core
*         level and actual terminated mode.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
* @param[in] action                   - Power down or reset
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsAasPortReset
(
    IN GT_U8   devNum,
    IN GT_U32  portGroup,
    IN GT_U32  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode,
    IN MV_HWS_PORT_ACTION  action
)
{
    MV_HWS_PORT_INIT_PARAMS curPortParams;

    action = action;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** port %d delete mode %d ******\n", phyPortNum, portMode);
    }
#endif
    /* validation */
    CHECK_STATUS(mvHwsPortValidate(devNum, portGroup, phyPortNum, portMode));

    if(GT_OK != hwsPortModeParamsGetToBuffer(devNum, portGroup, phyPortNum, portMode, &curPortParams))
    {
       return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsAasAnpPortEnable(devNum, phyPortNum, portMode, GT_FALSE/*don't care*/, GT_FALSE));

    /* should be done before mvHwsExtMacClockEnable() that disables clocks */

    CHECK_STATUS(mvHwsAasEthPortPowerDown(devNum, phyPortNum, portMode));


#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("**********************\n");
    }
#endif
    return GT_OK;
}

/**
* @internal mvHwsAasPortAnpStart function
* @endinternal
*
* @brief   Hawk port anp start (start resolution)
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] apCfgPtr           - Ap parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsAasPortAnpStart
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_AP_CFG                  *apCfgPtr
)
{
    GT_UNUSED_PARAM(portGroup);
    HWS_NULL_PTR_CHECK_MAC(apCfgPtr);

    CHECK_STATUS(mvHwsAnpPortStart(devNum,phyPortNum,apCfgPtr));
    return GT_OK;
}

GT_STATUS mvHwsAasPortDebugUnitDump
(
    GT_U32                  portNum,
    MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_UNITS_ID unitId;
    GT_U32 regData;
    GT_U32 i;

    /******************/
    /* Global EXT MAC */
    /******************/
    unitId = MTI_GLOBAL_EXT_UNIT;
    for(i = 0; i < AAS_MAC_GLOBAL_EXT_GLOBAL_REGISTER_LAST_E; i++)
    {
        CHECK_STATUS(genUnitRegisterFieldGet(0, 0, portNum, unitId, i, &regData, NULL));
        if(i % 8 == 0)
        {
            hwsOsPrintf("\nGLOBAL_MAC_EXT[%3d]:  ", i);
        }
        hwsOsPrintf("0x%08x  ", regData);
    }
    hwsOsPrintf("\n\n");

    /********************/
    /* Per port EXT MAC */
    /********************/
    unitId = MTI_EXT_UNIT;
    for(i = 0; i < AAS_MAC_PORT_EXT_REGISTER_LAST_E; i++)
    {
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
        if(i % 8 == 0)
        {
            hwsOsPrintf("\nPORT_MAC_EXT[%3d]:  ", i);
        }
        hwsOsPrintf("0x%08x  ", regData);
    }
    hwsOsPrintf("\n\n");

    /*******************/
    /* MAC100 / MAC400 */
    /*******************/
    if(HWS_IS_PORT_MODE_400G_R8(portMode) || HWS_IS_PORT_MODE_800G_R8(portMode) )
    {
        unitId = MTI_MAC400_UNIT;
        for(i = 0; i < AAS_MAC400_REGISTER_LAST_E; i++)
        {
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
            if(i % 8 == 0)
            {
                hwsOsPrintf("\nMAC400[%3d]:  ", i);
            }
            hwsOsPrintf("0x%08x  ", regData);
        }
        hwsOsPrintf("\n\n");
    }
    else
    {
        unitId = MTI_MAC100_UNIT;
        for(i = 0; i < AAS_MAC100_REGISTER_LAST_E; i++)
        {
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
            if(i % 8 == 0)
            {
                hwsOsPrintf("\nMAC100[%3d]:  ", i);
            }
            hwsOsPrintf("0x%08x  ", regData);
        }
        hwsOsPrintf("\n\n");
    }

    /*PCS EXT*/
    unitId = MTI_PCS_EXT_UNIT;
    for(i = 0; i < AAS_PCS800_EXT_REGISTER_LAST_E; i++)
    {
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
        if(i % 8 == 0)
        {
            hwsOsPrintf("\nPCS_EXT[%3d]:  ", i);
        }
        hwsOsPrintf("0x%08x  ", regData);
    }
    hwsOsPrintf("\n\n");

    /*PCS400*/
    if(HWS_IS_PORT_MODE_800G_R8(portMode) || HWS_IS_PORT_MODE_400G_R4(portMode) || HWS_IS_PORT_MODE_400G_R8(portMode))
    {
        unitId = MTI_PCS400_UNIT;
        for(i = 0; i < AAS_PCS400_REGISTER_LAST_E; i++)
        {
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
            if(i % 8 == 0)
            {
                hwsOsPrintf("\nPCS400[%3d]:  ", i);
            }
            hwsOsPrintf("0x%08x  ", regData);
        }
        hwsOsPrintf("\n");
        if(HWS_IS_PORT_MODE_800G_R8(portMode))
        {
            unitId = MTI_PCS400_UNIT;
            for(i = 0; i < AAS_PCS400_REGISTER_LAST_E; i++)
            {
                CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum+8, unitId, portMode, i, &regData, NULL));
                if(i % 8 == 0)
                {
                    hwsOsPrintf("\nPCS400[%3d]:  ", i);
                }
                hwsOsPrintf("0x%08x  ", regData);
            }
            hwsOsPrintf("\n");
        }
        hwsOsPrintf("\n");
    }
    else
    {
        if(portNum % 2 == 0)
        {
            unitId = MTI_PCS100_UNIT;
            for(i = 0; i < AAS_PCS100_REGISTER_LAST_E; i++)
            {
                CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
                if(i % 8 == 0)
                {
                    hwsOsPrintf("\nPCS100[%3d]:  ", i);
                }
                hwsOsPrintf("0x%08x  ", regData);
            }
            hwsOsPrintf("\n");
        }
        else
        {
            unitId = MTI_PCS50_UNIT;
            for(i = 0; i < AAS_PCS50_REGISTER_LAST_E; i++)
            {
                CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
                if(i % 8 == 0)
                {
                    hwsOsPrintf("\nPCS50[%3d]:  ", i);
                }
                hwsOsPrintf("0x%08x  ", regData);
            }
            hwsOsPrintf("\n");
        }
    }

    /*PCS400*/
    unitId = MTI_RSFEC_UNIT;
    for(i = 0; i < AAS_RSFEC_UNIT_REGISTER_LAST_E; i++)
    {
        CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum, unitId, portMode, i, &regData, NULL));
        if(i % 8 == 0)
        {
            hwsOsPrintf("\nPCS_RSFEC[%3d]:  ", i);
        }
        hwsOsPrintf("0x%08x  ", regData);
    }
    hwsOsPrintf("\n");
    if(HWS_IS_PORT_MODE_800G_R8(portMode))
    {
        unitId = MTI_RSFEC_UNIT;
        for(i = 0; i < AAS_RSFEC_UNIT_REGISTER_LAST_E; i++)
        {
            CHECK_STATUS(genUnitPortModeRegisterFieldGet(0, 0, portNum+8, unitId, portMode, i, &regData, NULL));
            if(i % 8 == 0)
            {
                hwsOsPrintf("\nPCS_RSFEC[%3d]:  ", i);
            }
            hwsOsPrintf("0x%08x  ", regData);
        }
        hwsOsPrintf("\n");
    }
    hwsOsPrintf("\n");

    /*ANP*/
    unitId = ANP_400_UNIT;
    for(i = 0; i < AAS_ANP_UNIT_REGISTER_LAST_E; i++)
    {
        CHECK_STATUS(genUnitRegisterFieldGet(0, 0, portNum, unitId, i, &regData, NULL));
        if(i % 8 == 0)
        {
            hwsOsPrintf("\nANP[%3d]:  ", i);
        }
        hwsOsPrintf("0x%08x  ", regData);
    }
    hwsOsPrintf("\n\n");

    return GT_OK;
}

