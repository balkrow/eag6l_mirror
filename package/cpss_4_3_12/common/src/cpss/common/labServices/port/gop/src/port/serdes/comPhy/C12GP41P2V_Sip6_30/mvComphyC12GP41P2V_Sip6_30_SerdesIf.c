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
* @file mvComphyC12GP41P2VSerdesIf.c \
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/

#if !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsDiagnostic.h>

#ifdef C12GP41P2V_SIP6_30
#include "../C12GP41P2V/mcesdC12GP41P2V_Defs.h"
#include "mcesdC12GP41P2V_API_Sip6_30.h"
#include "mcesdC12GP41P2V_DeviceInit_Sip6_30.h"
#include "mcesdC12GP41P2V_HwCntl_Sip6_30.h"
#include "mcesdC12GP41P2V_RegRW_Sip6_30.h"

#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MIN_TEMPERATURE 30
#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_TEMPERATURE 70
#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_RES_SEL 0x7
#define MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_CAP_SEL 0xF

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
/* Flag to enable/disable 40bit WA */
GT_STATUS mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWaCtrlChangeVal(GT_U8 ctrlVal);
GT_STATUS mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWaCtrlChangeVal(GT_U8 ctrlVal)
{
    if ((ctrlVal != 0) && (ctrlVal != 1))
        return GT_BAD_PARAM;

    PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWaCtrl) = ctrlVal;

    return GT_OK;
}
#endif

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTemperatureGet function
* @endinternal
*
* @brief   Read SD temperature
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
*
* @param[out] serdesTemperature       - (pointer to) temperature
*
* @note 12G SD Does not have tsen, hence need to read device's
*       temperature.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTemperatureGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  portGroup,
    IN  GT_UOPT  serdesNum,
    OUT GT_32    *serdesTemperature
)
{
    GT_UNUSED_PARAM(serdesNum);
    GT_UNUSED_PARAM(portGroup);
    if (NULL == serdesTemperature)
    {
        return GT_BAD_PTR;
    }

    return hwsDeviceTemperatureGetFuncPtr(devNum, serdesTemperature);
}

static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesOperation
(
    GT_U8                                  devNum,
    GT_U32                                 portGroup,
    GT_U32                                 serdesNum,
    MV_HWS_PORT_STANDARD                   portMode,
    MV_HWS_PORT_SERDES_OPERATION_MODE_ENT  operation,
    GT_U32                                 *dataPtr,
    GT_U32                                 *resultPtr
)
{
    MCESD_DEV_PTR   sDev;
    GT_32     temperature = 0;

    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(operation);
    GT_UNUSED_PARAM(dataPtr);
    GT_UNUSED_PARAM(resultPtr);

    /* application must not pass NULL pointers */
    if((resultPtr == NULL) || (dataPtr == NULL))
    {
        return GT_BAD_PARAM;
    }

    *resultPtr = 0;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    switch ( operation )
    {
    case MV_HWS_PORT_SERDES_OPERATION_PRE_RX_TRAINING_E:

        /* get Temperature!! */
        CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesTemperatureGet(devNum, portGroup, serdesNum, &temperature));

        if (temperature <= MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MIN_TEMPERATURE)
        {
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x156, 0, 6, 0x0)); /* CDR_MIDPOINT_PHASE_OS_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x175, 4, 4, 0x6)); /* DFE_RES_F0A_HIGH_THRES_LANE */
        }
        else if ( temperature > MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MIN_TEMPERATURE && temperature <= MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_TEMPERATURE )
        {
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x156, 0, 6, 0x2)); /* CDR_MIDPOINT_PHASE_OS_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x175, 4, 4, 0x5)); /* DFE_RES_F0A_HIGH_THRES_LANE */
        }
        else if ( temperature > MV_HWS_COMPHY_C12GP41P2V_SERDES_INTERCONNECT_MAX_TEMPERATURE )
        {
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x156, 0, 6, 0x6)); /* CDR_MIDPOINT_PHASE_OS_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x175, 4, 4, 0x5)); /* DFE_RES_F0A_HIGH_THRES_LANE */
        }

        break;

    case MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E:

        /* Reset For Analog DFE Registers */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x14, 14, 1, 0x1));
        MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, DFE_RESET_DELAY));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x14, 14, 1, 0x0));

        break;

    case MV_HWS_PORT_SERDES_OPERATION_SET_USR_E:
            C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_FFE_SETTING_F, 0x1);
            C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RES_SEL, 0x0);
            C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_CAP_SEL, 0x0);
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x183, 12, 4,  0xF)); /* rx_ffe_test_index_init_lane */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x175,  8, 2,  0x0)); /* gain_train_init_en_lane, gain_train_en_en_lane */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x4F,   5, 2,  0x3)); /* g1_dfe_res_f0_lane */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x140, 11, 1,  0x1)); /* dfe_res_f0_force_lane */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x183,  3, 1,  0x0)); /* cdrphase_opt_en_lane */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x195,  0, 1,  0x0)); /* cdrphase_opt_en_mode0_lane */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2C,  4, 2,  0x3)); /* rx_dc_term_en_fm_reg_lane , rx_dc_term_en_lane  */

        break;

    default:
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure TX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_TX_CONFIG_DATA - pointer to Tx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    IN  MV_HWS_SERDES_DB_TYPE             dbType,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT  *configParamsPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       defaultParamIterator = 0;
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;
    GT_U8 sdSpeed, speedIdx=0;
    GT_BOOL overrideExist = GT_FALSE;


    GT_UNUSED_PARAM(dbType);
    GT_UNUSED_PARAM(portGroup);

    if(configParamsPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* array like alleycat5TxRxTuneParams is not initialized for 'WIN32' */
        return GT_OK;
    }
#endif /**/
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);
#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

            /*dynamicly get the map*/
            tuneParams = mvHwsComphyTxRxTuneParamsGet(devNum,serdesNum);

             /*if not supported yet then use shared*/
            if(NULL==tuneParams)
            {
                tuneParams = serdesData->tuneParams;
            }
#else
            tuneParams = serdesData->tuneParams;
#endif


    for(defaultParamIterator = 0 ; tuneParams[defaultParamIterator].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; defaultParamIterator++)
    {
        if(tuneParams[defaultParamIterator].serdesSpeed == baudRate) break;
    }
    if(tuneParams[defaultParamIterator].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_SUPPORTED;
    }

    if(serdesData->tuneOverrideParams != NULL)
    {
        sdSpeed = baudRate;
        speedIdx = mvHwsSerdesSpeedToIndex(devNum,sdSpeed);
        if (speedIdx < MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE)
        {
            if(serdesData->tuneOverrideParams[speedIdx].serdesSpeed != LAST_MV_HWS_SERDES_SPEED)
                overrideExist = GT_TRUE;
        }
    }

    if (!overrideExist)/*override param not found - set default param*/
    {
        hwsOsMemCopyFuncPtr(&(configParamsPtr->txComphyC12GP41P2V),
                            &(tuneParams[defaultParamIterator].txParams.txComphyC12GP41P2V),
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_TX_CONFIG_DATA));
    }
    else/*override param exist - go over tx params and set override value if exist or default if not exist*/
    {
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->txComphyC12GP41P2V.pre, tuneParams[defaultParamIterator].txParams.txComphyC12GP41P2V.pre,  serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC12GP41P2V.pre);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->txComphyC12GP41P2V.peak,tuneParams[defaultParamIterator].txParams.txComphyC12GP41P2V.peak, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC12GP41P2V.peak);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->txComphyC12GP41P2V.post,tuneParams[defaultParamIterator].txParams.txComphyC12GP41P2V.post, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC12GP41P2V.post);
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesDbRxConfigGet function
* @endinternal
*
* @brief   Per SERDES get the configure RX parameters:
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] serdesType               - serdes type
* @param[in] baudRate                 - baud rate
*
* @param[out] MV_HWS_MAN_TUNE_RX_CONFIG_DATA - pointer to Rx
*                                              parameters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesDbRxConfigGet
(
    IN  GT_U8                             devNum,
    IN  GT_UOPT                           portGroup,
    IN  GT_UOPT                           serdesNum,
    IN  MV_HWS_SERDES_SPEED               baudRate,
    IN  MV_HWS_SERDES_DB_TYPE               dbType,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT  *configParamsPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       defaultParamIterator = 0;
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;
    GT_U8 sdSpeed, speedIdx=0;
    GT_BOOL overrideExist = GT_FALSE;

    GT_UNUSED_PARAM(dbType);
    GT_UNUSED_PARAM(portGroup);

    if(configParamsPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);
#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* array like alleycat5TxRxTuneParams is not initialized for 'WIN32' */
        return GT_OK;
    }
#endif /**/
#if defined  SHARED_MEMORY && !defined MV_HWS_REDUCED_BUILD

    /*dynamicly get the map*/
    tuneParams = mvHwsComphyTxRxTuneParamsGet(devNum,serdesNum);

     /*if not supported yet then use shared*/
    if(NULL==tuneParams)
    {
        tuneParams = serdesData->tuneParams;
    }
#else
     tuneParams = serdesData->tuneParams;
#endif


    for(defaultParamIterator = 0 ; tuneParams[defaultParamIterator].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; defaultParamIterator++)
    {
        if(tuneParams[defaultParamIterator].serdesSpeed == baudRate) break;
    }
    if(tuneParams[defaultParamIterator].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_SUPPORTED;
    }

    if(serdesData->tuneOverrideParams != NULL)
    {
        sdSpeed = baudRate;
        speedIdx = mvHwsSerdesSpeedToIndex(devNum,sdSpeed);
        if (speedIdx < MV_HWS_SERDES_TXRX_OVERRIDE_TUNE_PARAMS_ARR_SIZE)
        {
            if(serdesData->tuneOverrideParams[speedIdx].serdesSpeed != LAST_MV_HWS_SERDES_SPEED)
                overrideExist = GT_TRUE;
        }
    }

    if (!overrideExist)/*override param not found - set default param*/
    {
        hwsOsMemCopyFuncPtr(&(configParamsPtr->rxComphyC12GP41P2V),
                            &(tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V),
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C12GP41P2V_RX_CONFIG_DATA));
    }
    else/*override param exist - go over rx params and set override value if exist or default if not exist*/
    {
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.resSel,     tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.resSel,   serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.resSel);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.resShift,   tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.resShift, serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.resShift);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.capSel,     tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.capSel,   serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.capSel);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmufi,    tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.selmufi,  serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.selmufi);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmuff,    tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.selmuff,  serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.selmuff);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmupi,    tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.selmupi,  serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.selmupi);
        HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(configParamsPtr->rxComphyC12GP41P2V.selmupf,    tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.selmupf,  serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.selmupf);
        HWS_COMPHY_CHECK_AND_SET_DB_16BIT_SIGNED_VAL(configParamsPtr->rxComphyC12GP41P2V.squelch,    tuneParams[defaultParamIterator].rxParams.rxComphyC12GP41P2V.squelch,  serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC12GP41P2V.squelch);
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesManualTxConfig function
* @endinternal
*
* @brief   Config SERDES electrical parameters
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] txConfigPtr - pointer to Serdes Tx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesManualTxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
)
{
    MCESD_DEV_PTR sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* we got dummy info from mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet */
        /* see comments in function mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet */
        return GT_OK;
    }
#endif /**/

    if(NA_8BIT != txConfigPtr->txComphyC12GP41P2V.pre)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PRE_CTRL , (MCESD_U32)txConfigPtr->txComphyC12GP41P2V.pre));
    if(NA_8BIT != txConfigPtr->txComphyC12GP41P2V.peak)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PEAK_CTRL, (MCESD_U32)txConfigPtr->txComphyC12GP41P2V.peak));
    if(NA_8BIT != txConfigPtr->txComphyC12GP41P2V.post)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_POST_CTRL, (MCESD_U32)txConfigPtr->txComphyC12GP41P2V.post));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesManualRxConfig function
* @endinternal
*
* @brief   Config SERDES electrical parameters
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] RxConfigPtr - pointer to Serdes Rx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    MCESD_DEV_PTR sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

#ifdef _WIN32
    if(serdesNum != 0xFFFF)
    {
        /* we got dummy info from mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet */
        /* see comments in function mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet */
        return GT_OK;
    }
#endif /**/

    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.resSel)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL,   (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.resSel  ));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.resShift)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SHIFT, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.resShift));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.capSel)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL,   (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.capSel  ));

    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmufi)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmufi));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmuff)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmuff));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmupi)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmupi));
    if(NA_8BIT != rxConfigPtr->rxComphyC12GP41P2V.selmupf)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC12GP41P2V.selmupf));

    if(NA_16BIT_SIGNED != rxConfigPtr->rxComphyC12GP41P2V.squelch)
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetSquelchThreshold(sDev, (MCESD_16)rxConfigPtr->rxComphyC12GP41P2V.squelch));

    /** Static configurations */
    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0xCA, 0xFFFF, 0x3FF));

    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0xC9, (0x1 << 12), (0x1 << 12)));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesReset function
* @endinternal
*
* @brief   Reset serdes
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] analogReset              - Analog Reset (On/Off)
* @param[in] digitalReset             - digital Reset (On/Off)
* @param[in] syncEReset               - SyncE Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesReset
(
    IN GT_U8       devNum,
    IN GT_UOPT     portGroup,
    IN GT_UOPT     serdesNum,
    IN GT_BOOL     analogReset,
    IN GT_BOOL     digitalReset,
    IN GT_BOOL     syncEReset
)
{
    GT_UREG_DATA data;
    MCESD_DEV_PTR sDev;
    GT_UNUSED_PARAM(syncEReset);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    /* SERDES SD RESET/UNRESET init */
    data = (analogReset == GT_TRUE) ? 1 : 0;    /* active high */
    MCESD_CHECK_STATUS(API_C12GP41P2V_HwSetPinCfg(sDev, C12GP41P2V_PIN_RESET, data));

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == GT_TRUE) ? 1 : 0;   /* active high */
    CHECK_STATUS(genUnitRegisterFieldSet(devNum,portGroup,serdesNum,SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_1_RF_RESET_IN_E, data, NULL));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset function
* @endinternal
*
* @brief   Core Serdes Reset
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] coreReset              - Analog Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_UREG_DATA data;
    GT_UNUSED_PARAM(portGroup);

    data = (coreReset == UNRESET) ? 0 : 1;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum,portGroup,serdesNum,SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_1_RESET_CORE_E, data, NULL));

    return GT_OK;
}

static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWA
(
    GT_U8                       devNum,
    GT_UOPT                     portGroup,
    GT_UOPT                     serdesNum,
    E_C12GP41P2V_SERDES_SPEED   speed
)
{
    MCESD_U16 txdInvData, rxdInvData, ptLockPollingTime;
    MCESD_DEV_PTR                   sDev = NULL;
    MV_HWS_PER_SERDES_INFO_PTR      serdesInfo;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /*
        In order to run PRBS in loopback mode, it's needed to
        disable board related SD polarity settings. After PRBS
        is stopped - need to restore these configurations.
    */
    C12GP41P2V_READ_FIELD(sDev, F_C12GP41P2VR2P0_TXD_INV, txdInvData);
    C12GP41P2V_READ_FIELD(sDev, F_C12GP41P2VR2P0_RXD_INV, rxdInvData);

    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TXD_INV, C12GP41P2V_POLARITY_NORMAL);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RXD_INV, C12GP41P2V_POLARITY_NORMAL);

    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_ANA_PU_LB, 0x1);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_ANA_PU_LB_DLY, 0x1);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_PRBS_ENC, 0x1);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_TX_PAT_SEL, 0x18);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_RX_PAT_SEL, 0x18);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_EN, 0x1);

    MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 5));

    MCESD_CHECK_STATUS(API_C12GP41P2V_HwSetPinCfg(sDev, C12GP41P2V_PIN_RX_INIT0, 1));
    MCESD_CHECK_STATUS(API_C12GP41P2V_PollPin(sDev, C12GP41P2V_PIN_RX_INIT_DONE0, 1, 10));
    MCESD_CHECK_STATUS(API_C12GP41P2V_HwSetPinCfg(sDev, C12GP41P2V_PIN_RX_INIT0, 0));

    MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 1));
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_PRBS_LOAD_LANE, 0x1);
    MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 1));
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_RST, 0x1);
    MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 1));
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_RST, 0x0);
    MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 5));

    /* speed > 10G -> 5ms delay
       else -> 10ms delay */
    switch (speed)
    {
    case C12GP41P2V_SERDES_10P3125G:
    case C12GP41P2V_SERDES_12P5G:
    case C12GP41P2V_SERDES_10G:
    case C12GP41P2V_SERDES_12P375G:
        ptLockPollingTime = 5;
        break;

    default:
        ptLockPollingTime = 10;
        break;
    }

    C12GP41P2V_POLL_FIELD(sDev, F_C12GP41P2VR2P0_PT_LOCK, 1, ptLockPollingTime);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_EN, 0x0);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_TX_PAT_SEL, 0x9);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_RX_PAT_SEL, 0x9);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_ANA_PU_LB_DLY, 0x0);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_ANA_PU_LB, 0x0);

    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TXD_INV, (txdInvData == 0) ? C12GP41P2V_POLARITY_NORMAL : C12GP41P2V_POLARITY_INVERTED);
    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RXD_INV, (rxdInvData == 0) ? C12GP41P2V_POLARITY_NORMAL : C12GP41P2V_POLARITY_INVERTED);

    return GT_OK;
}
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTxRxExtraConfiguraion
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_SERDES_SPEED     serdesSpeed,
    GT_BOOL                 powerUp
)
{
    MCESD_DEV_PTR                   sDev = NULL;
    MV_HWS_PER_SERDES_INFO_PTR      serdesInfo;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if (powerUp)
    {
        if (_1_25G_100FX == serdesSpeed)
        {
            /* TXDCLK_2X_EN_LANE - Output Enable */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, (0x18/4), 5, 1, 1));
            MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRFreeze(sDev, MCESD_TRUE));
        }

        if (_12_375G == serdesSpeed)
        {
            /* G1_TX_VREF_TXDRV_SEL_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x48, 0x7, 0x7));
        }
    }
    else
    {
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_FBCK_SEL_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TXDRV_SEL_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_DC_OS_NORMAL, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_MIN_BOOST_MODE, 0x1);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TX_CLK_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TX_DATA_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TX_SPDIV_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TX_VDD_CAL_F, 0x0);

        /* RX Programming */
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_SPDIV_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_DTLCLK_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_INTPI_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_INTPR_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_EOM_INTPR_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_CLK_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_DATA_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_CKPW_CNT_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_PU_A90_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_CTLE_1_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_CTLE_2_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_CTLE_3_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_VDD_CAL_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_DCC_CAL_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_RX_25M_FORCE, 0x0);

       /* PLL Programming */
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_REFDIV_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_FBDIV_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_BW_SEL_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_LPF_C2_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_ICP_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_SPEED_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_FBDIV_CAL_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TXINTPI_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TXINTPR_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_TXDTXCLK_DIV_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_REG_FORCE, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_INIT_TXFOFFS_F, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_INIT_RXFOFFS_F, 0x0);

        /* PRBS disable */
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_EN, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_TX_PAT_SEL, 0x9);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PT_RX_PAT_SEL, 0x9);

        /* looback disable */
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_ANA_PU_LB_DLY, 0x0);
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_ANA_PU_LB, 0x0);

        if (_1_25G_100FX == serdesSpeed)
        {
            /* TXDCLK_2X_EN_LANE - Output Enable */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, (0x18/4), 5, 1, 0x0));
            MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRFreeze(sDev, MCESD_FALSE));
        }

        if (_12_375G == serdesSpeed)
        {
            /* G1_TX_VREF_TXDRV_SEL_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x48, 0x7, 0x4));
        }

        /* restore CTLE, CDR, SQUELCH to default */
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL, 0x0));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SHIFT, 0x9));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL, 0x0));
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_FFE_SETTING_F, 0x0);
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUFI, 0x4));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUFF, 0x4));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUPI, 0x2));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetCDRParam(sDev, C12GP41P2V_CDR_SELMUPF, 0x2));
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetSquelchThreshold(sDev, 0x0));
        C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_SQ_THRESH_F_EN, 0x0);
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesArrayPowerCtrl function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] numOfSer  - number of SERDESes to configure
* @param[in] serdesArr - collection of SERDESes to configure
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesArrayPowerCtrl
(
    IN GT_U8                       devNum,
    IN GT_UOPT                     portGroup,
    IN GT_UOPT                     numOfSer,
    IN GT_UOPT                     *serdesArr,
    IN GT_BOOL                     powerUp,
    IN MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{

    GT_UOPT                            serdesIterator;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT   txParameters;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT   rxParameters;
    MV_HWS_PER_SERDES_INFO_PTR         serdesInfo;
    GT_U32                             mcesdEnum;
    GT_U16                             retries = PLL_INIT_POLLING_RETRIES;
    GT_U32                             data, result;
    GT_BOOL                            allStable;
    MCESD_BOOL                         txReady, rxReady;

    MCESD_DEV_PTR                  sDev = NULL;
    E_C12GP41P2V_SERDES_SPEED      speed = 0;
    E_C12GP41P2V_REFFREQ           refFreq = 0;
    E_C12GP41P2V_REFCLK_SEL        refClkSel = 0;
    E_C12GP41P2V_DATABUS_WIDTH     dataBusWidth = 0;
    S_C12GP41P2V_PowerOn           powerOnConfig;
    MV_HWS_SERDES_SPEED            baudRate = serdesConfigPtr->baudRate;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);

    if (powerUp)
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Power-on lane */
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_C12GP41P2V_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_C12GP41P2V_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_C12GP41P2V_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_C12GP41P2V_DATABUS_WIDTH)mcesdEnum;
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);

            /* need to hold SD in core reset during power-on sequence */
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], RESET));
            /* Analog Unreset */
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));

            powerOnConfig.powerLaneMask = 0x1;
            powerOnConfig.initTx        = MCESD_TRUE;
            powerOnConfig.initRx        = MCESD_TRUE;
            powerOnConfig.txOutputEn    = MCESD_FALSE;
            powerOnConfig.dataPath      = C12GP41P2V_PATH_EXTERNAL;
            powerOnConfig.refClkSel     = refClkSel;
            powerOnConfig.dataBusWidth  = dataBusWidth;
            powerOnConfig.speed         = speed;
            powerOnConfig.refFreq       = refFreq;

            MCESD_CHECK_STATUS(API_C12GP41P2V_PowerOnSeq(sDev, powerOnConfig));
           /* PLL settings update for 125MHz Refclk */
            if (refFreq == C12GP41P2V_REFFREQ_125MHZ)
            {
                switch (powerOnConfig.speed)
                {
                case C12GP41P2V_SERDES_10G:
                case C12GP41P2V_SERDES_10P3125G:
                    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_ICP_FORCE, 0x1);
                    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_G1_PLL_ICP, 0xA);
                    break;

                case C12GP41P2V_SERDES_12P375G:
                    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_PLL_ICP_FORCE, 0x1);
                    C12GP41P2V_WRITE_FIELD(sDev, F_C12GP41P2VR2P0_G1_PLL_ICP, 0xB);
                    break;

                default:
                    break;
                }
            }

            MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 1));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], UNRESET));
        }

        /* Wait for stable PLLs */
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[0],NULL,NULL,&sDev));
        do
        {
            allStable = GT_TRUE;
            for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
            {
                CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],NULL,NULL,&sDev));
                MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxReady(sDev, &txReady, &rxReady));

                if ( (txReady != MCESD_TRUE) || (rxReady != MCESD_TRUE) )
                {
                    allStable = GT_FALSE;
                    break;
                }
            }
            MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, PLL_INIT_POLLING_DELAY));
            retries--;
        } while ((retries != 0) && (allStable == GT_FALSE));
        if(allStable == GT_FALSE)
        {
            return GT_TIMEOUT;
        }

        for(serdesIterator = 0; serdesIterator < numOfSer; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));
            /*Perform 40Bit WA*/
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            if( (serdesConfigPtr->busWidth == _40BIT_ON) && (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWaCtrl) == 1) )
            {
                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWA(devNum, portGroup, serdesArr[serdesIterator], powerOnConfig.speed));
            }
#else
            if (serdesConfigPtr->busWidth == _40BIT_ON)
            {
                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_Serdes40bitWA(devNum, portGroup, serdesArr[serdesIterator], powerOnConfig.speed));
            }
#endif

            /* Config Tx/Rx parameters */
            MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(serdesConfigPtr->opticalMode, baudRate);
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &txParameters));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txParameters));

            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesDbRxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &rxParameters));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxParameters));

            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesTxRxExtraConfiguraion(devNum, portGroup, serdesArr[serdesIterator], baudRate, GT_TRUE));

            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));

            MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxOutputEnable(sDev, MCESD_TRUE));

        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],NULL,NULL,&sDev));
            MCESD_CHECK_STATUS(API_C12GP41P2V_PowerOffLane(sDev));
            CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], RESET));

            if (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesBgEnable) == GT_TRUE)
            {
                /* in case of BG optimization, we don't use sft_rst_only_reg to reset SD - in order not to delete BG configuration,
                   and keep it "alive". Hence, need to restore SD to default values */
                MCESD_CHECK_STATUS(API_C12GP41P2V_Wait(sDev, 1));
                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], UNRESET));

                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesOperation(devNum, portGroup, serdesArr[serdesIterator],
                                                           /*don't care*/_1000Base_X, MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E, &data, &result));

                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesTxRxExtraConfiguraion(devNum, portGroup, serdesArr[serdesIterator], serdesConfigPtr->baudRate, GT_FALSE));

                /* still keep analog unreset */
                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_TRUE));
            }
            else
            {
                CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
                CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesArr[serdesIterator], 0x4  , 11, 1,  0x1)); /* sft_rst_only_reg */
            }
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesPowerCtrl function
* @endinternal
*
* @brief   Power up/down SERDES list.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] powerUp   - True for PowerUP, False for PowerDown
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyC12GP41P2VSip6_30_SerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesRxAutoTuneStart function
* @endinternal
*
* @brief   Start Rx auto-tuning
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  training  - Start/Stop training
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StartTraining(sDev, C12GP41P2V_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StopTraining(sDev, C12GP41P2V_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStart function
* @endinternal
*
* @brief   Start Tx auto-tuning
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  training  - Start/Stop training
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_SERDES_SPEED speed;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxBitRate(sDev, &speed));

        if ( speed == C12GP41P2V_SERDES_10P3125G || speed == C12GP41P2V_SERDES_12P375G)
        {
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x25A , 8, 6,  0x1D); /* tx_amp_default2 */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x25A , 4, 4,  0x0);  /* tx_emph1_default2 */
            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x25A , 0, 4,  0x0);  /* tx_emph0_default2 */

            mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x258, 4, 4,  0x2);  /* local_tx_preset_index */
        }
        MCESD_CHECK_STATUS(API_C12GP41P2V_StartTraining(sDev, C12GP41P2V_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StopTraining(sDev, C12GP41P2V_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStop function
* @endinternal
*
* @brief   Stop Tx auto-tuning
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStart function
* @endinternal
*
* @brief   Start Tx/Rx auto-tuning
*
* @param[in]  devNum     - system device number
* @param[in]  portGroup  - port group (core) number
* @param[in]  serdesNum  - serdes number
* @param[in]  rxTraining - Start/Stop Rx training
* @param[in]  txTraining - Start/Stop Tx training
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL rxTraining,
    GT_BOOL txTraining
)
{
    if(rxTraining)
    {
        CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStatus function
* @endinternal
*
* @brief   Get Rx and Tx auto-tuning status
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out] rxStatus  - Rx auto-tuning status
* @param[out] txStatus  - Tx auto-tuning status
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStatus
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    MCESD_DEV_PTR              sDev;
    MCESD_BOOL                 completed;
    MCESD_BOOL                 failed;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    if(NULL != rxStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_CheckTraining(sDev, C12GP41P2V_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_CheckTraining(sDev, C12GP41P2V_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStatusShort
*           function
* @endinternal
*
* @brief   Get Rx and Tx auto-tuning status
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out] rxStatus  - Rx auto-tuning status
* @param[out] txStatus  - Tx auto-tuning status
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStatusShort
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_AUTO_TUNE_STATUS *rxStatus,
    MV_HWS_AUTO_TUNE_STATUS *txStatus
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    MCESD_U32                   completeData, failedData = 0;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(NULL != rxStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_RX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_RX_TRAIN_FAILED0, &failedData));
        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_TX_TRAIN_COMPLETE0, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_TX_TRAIN_FAILED0, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneResult function
* @endinternal
*
* @brief   Get Rx and Tx auto-tuning results
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out] results  - auto-tuning results
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MCESD_DEV_PTR                               sDev;
    MV_HWS_COMPHY_C12GP41P2V_AUTO_TUNE_RESULTS  *results;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U16 val16;
    MCESD_16  vals16;
    MCESD_U32 tapsI;
    GT_U32    gtVal;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC12GP41P2VResults;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x258, 0, 1, &gtVal));

    if ( gtVal )
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PRE_CTRL, &val));
        results->txComphyC12GP41P2V.pre = (GT_U8)val;
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_PEAK_CTRL, &val));
        results->txComphyC12GP41P2V.peak = (GT_U8)val;
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxEqParam(sDev, C12GP41P2V_TXEQ_EM_POST_CTRL, &val));
        results->txComphyC12GP41P2V.post = (GT_U8)val;
    }
    else
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x257, 4 ,4, &gtVal));
        results->txComphyC12GP41P2V.pre = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x257, 8 ,6, &gtVal));
        results->txComphyC12GP41P2V.peak = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x257, 0 ,4, &gtVal));
        results->txComphyC12GP41P2V.post = gtVal;
    }

    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xA7, 4 ,4, &gtVal));
    results->analogPre = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xA7, 0 ,4, &gtVal));
    results->analogPeak = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xA7, 8 ,4, &gtVal));
    results->analogPost = gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SHIFT, &val));
    results->rxComphyC12GP41P2V.resShift = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_RES_SEL,   &val));
    results->adaptedResSel              = (GT_U32)val;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x4E, 0, 3, &gtVal));
    results->rxComphyC12GP41P2V.resSel = (GT_U32)gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCTLEParam(sDev, C12GP41P2V_CTLE_CAP_SEL,   &val));
    results->adaptedCapSel             = (GT_U32)val;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x4D, 0, 4, &gtVal));
    results->rxComphyC12GP41P2V.capSel = (GT_U32)gtVal;

    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x4E, 12, 1, &gtVal));
    results->ffeSettingForce           = (GT_U32)gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUFI, &val));
    results->rxComphyC12GP41P2V.selmufi = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUFF, &val));
    results->rxComphyC12GP41P2V.selmuff = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUPI, &val));
    results->rxComphyC12GP41P2V.selmupi = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetCDRParam(sDev, C12GP41P2V_CDR_SELMUPF, &val));
    results->rxComphyC12GP41P2V.selmupf = (GT_U32)val;

    /* indicator to eye opening */
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x14B, 8, 6, &gtVal)); /* sav_f0d  */
    results->eo = gtVal;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetAlign90(sDev, &val16));
    results->align90 = (GT_U16)val16;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetSquelchThreshold(sDev, &vals16));
    results->rxComphyC12GP41P2V.squelch = (GT_16)vals16;

    results->sampler = 0;

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetSlewRateParam(sDev, C12GP41P2V_SR_CTRL0, &val));
    results->slewRateCtrl0 = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetSlewRateParam(sDev, C12GP41P2V_SR_CTRL1, &val));
    results->slewRateCtrl1 = (GT_U32)val;

    for(tapsI = 0 ; tapsI < C12GP41P2V_DFE_F7 ; tapsI++)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetDfeTap(sDev, (E_C12GP41P2V_DFE_TAP)tapsI, &vals));
        results->dfe[tapsI] = (GT_32)vals;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesDigitalReset function
* @endinternal
*
* @brief   Run digital reset / unreset on current SERDES.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - serdes number
* @param[in] digitalReset             - digital Reset (On/Off)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesDigitalReset
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_RESET        digitalReset
)
{
    GT_UREG_DATA data;

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == RESET) ? 1 : 0;
    CHECK_STATUS(genUnitRegisterFieldSet(devNum,portGroup,serdesNum,SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_1_RF_RESET_IN_E, data, NULL));

    return GT_OK;
}
/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesLoopback function
* @endinternal
*
* @brief   Set SerDes loopback mode
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  lbMode    - serdes looback mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;

    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_DATAPATH      path;
    GT_U32                     mcesdEnum;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    switch ( lbMode )
    {
    case SERDES_LP_AN_TX_RX:
        MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x26, 15, 1, 0x1)); /* ana_rx_sq_out_fm_reg_lane */
        break;
    default:
         MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x26, 15, 1, 0x0)); /* ana_rx_sq_out_fm_reg_lane */
        break;
    }

    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_C12GP41P2V_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_C12GP41P2V_SetDataPath(sDev, path));
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesLoopbackGet function
* @endinternal
*
* @brief   Get SerDes loopbabk mode
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out]  lbMode    - serdes looback mode
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;

    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_DATAPATH      path;
    GT_U32                     tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetDataPath(sDev, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTestGen function
* @endinternal
*
* @brief   Activates the Serdes test generator/checker.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] mode                     - test  or normal
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTestGen
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN  MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    MCESD_DEV_PTR                 sDev;
    S_C12GP41P2V_PATTERN          pattern;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &mcesdPattern));
    pattern.pattern  = mcesdPattern;
    pattern.enc8B10B = C12GP41P2V_ENC_8B10B_DISABLE;

    switch (txPattern)
    {
        case _1T:
            userPattern = pattern1T;
            break;
        case _2T:
            userPattern = pattern2T;
            break;
        case _5T:
            userPattern = pattern5T;
            break;
        case _10T:
            userPattern = pattern10T;
            break;
        default:
            userPattern = patternEmpty;
            break;
    }
    MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxRxPattern(sDev, &pattern, &pattern, C12GP41P2V_SATA_NOT_USED, C12GP41P2V_DISPARITY_NOT_USED, userPattern, 0));
    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_ResetComparatorStats(sDev));
        MCESD_CHECK_STATUS(API_C12GP41P2V_StartPhyTest(sDev));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_StopPhyTest(sDev));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTestGenGet function
* @endinternal
*
* @brief  Get configuration of the Serdes test
*         generator/checker. Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
*
* @param[out] txPatternPtr             - pattern to transmit ("Other" means any mode not
*                                      included explicitly in MV_HWS_SERDES_TX_PATTERN type)
* @param[out] modePtr                  - test mode or normal
*
* @retval GT_OK                    - on success
* @retval GT_NOT_SUPPORTED         - unexpected pattern
* @retval GT_FAIL                  - HW error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTestGenGet
(
    GT_U8                        devNum,
    GT_UOPT                      portGroup,
    GT_UOPT                      serdesNum,
    MV_HWS_SERDES_TX_PATTERN     *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE  *modePtr
)
{
    MCESD_DEV_PTR                       sDev;
    S_C12GP41P2V_PATTERN                txPattern;
    S_C12GP41P2V_PATTERN                rxPattern;
    E_C12GP41P2V_SATA_LONGSHORT         sataLongShort;
    E_C12GP41P2V_SATA_INITIAL_DISPARITY sataInitialDisparity;
    char                                userPattern[21];
    MCESD_U8                            userK;
    MCESD_FIELD                         ptEn = F_C12GP41P2VR2P0_PT_EN;
    MCESD_U16                           enable;
    GT_U32                              tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxPattern(sDev, &txPattern, &rxPattern, &sataLongShort,&sataInitialDisparity, userPattern, &userK));
    MCESD_CHECK_STATUS(API_C12GP41P2V_ReadField(sDev, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C12GP41P2V_SIP6_30, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern.pattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTestGenStatus
 function
* @endinternal
*
* @brief   Read the tested pattern receive error counters and status.
*         Can be run after create port.
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] txPattern                - pattern to transmit
* @param[in] counterAccumulateMode    - Enable/Disable reset the accumulation of error counters
*
* @param[out] status                   - test generator status
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTestGenStatus
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN        txPattern,
    IN GT_BOOL                         counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
    MCESD_DEV_PTR                       sDev;
    S_C12GP41P2V_PATTERN_STATISTICS     statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C12GP41P2V_GetComparatorStats(sDev, &statistics));

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr.l[0]   = (GT_U32) (statistics.totalErrorBits & 0x00000000FFFFFFFFUL);
    status->errorsCntr.l[1]   = (GT_U32)((statistics.totalErrorBits & 0xFFFFFFFF00000000UL) >> 32);
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_ResetComparatorStats(sDev));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesPolarityConfig function
* @endinternal
*
* @brief   Set Tx/Rx polarity invert
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  invertTx  - if true - invert Tx polarity
* @param[in]  invertRx  - if true - invert Rx polarity
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesCheckAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxRxPolarity(sDev, invertTx ? C12GP41P2V_POLARITY_INVERTED : C12GP41P2V_POLARITY_NORMAL,
                                                        invertRx ? C12GP41P2V_POLARITY_INVERTED : C12GP41P2V_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesPolarityConfigGet
*           function
* @endinternal
*
* @brief   Get Tx/Rx polarity invert
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[out]  invertTx  - if true - Tx polarity is inverted
* @param[out]  invertRx  - if true - Rx polarity is inverted
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesPolarityConfigGet
(
  GT_U8             devNum,
  GT_UOPT           portGroup,
  GT_UOPT           serdesNum,
  GT_BOOL           *invertTx,
  GT_BOOL           *invertRx
)
{
    MCESD_DEV_PTR              sDev;
    E_C12GP41P2V_POLARITY      txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphyC12GP41P2VSerdesCheckAccess(devNum,portGroup,serdesNum));

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxRxPolarity(sDev, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == C12GP41P2V_POLARITY_INVERTED);
    *invertRx = (rxPolarity == C12GP41P2V_POLARITY_INVERTED);

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTxEnbaleGet function
* @endinternal
*
* @brief   Get Tx signal enbale
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  txEnable  - if true - enable Tx
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTxEnbale
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL txEnable
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_SetTxOutputEnable(sDev, (MCESD_BOOL)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesTxEnbale function
* @endinternal
*
* @brief   Set Tx signal enbale
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  txEnable  - if true - enable Tx
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesTxEnbaleGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *txEnable
)
{
    MCESD_DEV_PTR              sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C12GP41P2V_GetTxOutputEnable(sDev, (MCESD_BOOL*)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesSignalDetectGet function
* @endinternal
*
* @brief   Check if signal is present
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number to get its signal state
* @param[out] enable    - return signal detect value
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesSignalDetectGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
)
{
    MCESD_DEV_PTR              sDev;
    MCESD_BOOL                 squelched;
    MV_HWS_SERDES_LB           lbMode;
    GT_U8 hitCounter = 0;
    GT_U32 i;

    GT_UNUSED_PARAM(portGroup);
    /* in serdes loopback analog tx2rx, we force signal detect */
    CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesLoopbackGet(devNum, portGroup, serdesNum, &lbMode));
    if ( lbMode ==  SERDES_LP_AN_TX_RX )
    {
        *enable = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    for (i = 0; i < 1200; i++)
    {
        /* no cable */
        if ( i > 100 && hitCounter == 0)
        {
            *enable = GT_FALSE;
            return GT_OK;
        }
        /* if sq == 0 meaning we have a signal */
        MCESD_CHECK_STATUS(API_C12GP41P2V_GetSquelchDetect(sDev, &squelched));
        /**enable = squelched ? GT_FALSE : GT_TRUE;*/
        if ( squelched == 0 )
        {
            hitCounter++;
            if ( hitCounter >= 20 )
            {
                *enable = GT_TRUE;
                return GT_OK;
            }
        }
    }
    *enable = GT_FALSE; /* signal is not stable */

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Check if cdr is lock
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number to get its signal state
* @param[out] enable    - return cdr lock value
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *lock
)
{
/*    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      data = 0, isPam2, clampingTrigger=0,freqReady=0, rxTrain;
    GT_8                        dataSigned = 0,dataSigned2;*/

    /* check signal */
    MCESD_CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    /*TODO add CDR SUPPORT*/
    return GT_OK;
}

extern GT_STATUS mvHwsComH28nmSerdesTxIfSelect
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    GT_UREG_DATA        serdesTxIfNum
);

extern GT_STATUS mvHwsComH28nmSerdesTxIfSelectGet
(
    GT_U8      devNum,
    GT_U32     portGroup,
    GT_U32     serdesNum,
    GT_U32     *serdesTxIfNum
);

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsComphyC12GP41P2VSip6_30_SerdesEomMatrixGet function
* @endinternal
*
* @brief   Read eye data
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] printEnable              - True = print, False = don't print
* @param[in] eomInParams              - (pointer to) EOM intput params
*
* @param[out] eomOutParams            - (pointer to) EOM output params
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesEomMatrixGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             serdesNum,
    IN   GT_BOOL                            printEnable,
    IN   MV_HWS_SERDES_EOM_IN_PARAMS_UNT    *eomInParams,
    OUT  MV_HWS_SERDES_EOM_OUT_PARAMS_UNT   *eomOutParams
)
{
    MCESD_DEV_PTR   sDev;
    MCESD_STATUS    mcesdSt;
    S_C12GP41P2V_EYE_RAW *mcesdEyeRawData;
    MCESD_U32 widthmUI, heightmVUpper, heightmVLower;
    MCESD_U32 width, heightUpper, heightLower;
    MCESD_BOOL dfeState;
    GT_U8 retries = MV_HWS_SERDES_EOM_RETRIES;

    if ( NULL == eomInParams|| NULL == eomOutParams)
    {
        return GT_BAD_PTR;
    }

    if (eomInParams->comphyC12GP41P2VEomInParams.phaseLevels  < 1 || eomInParams->comphyC12GP41P2VEomInParams.phaseLevels  > 16 ||
        eomInParams->comphyC12GP41P2VEomInParams.voltageSteps < 1 || eomInParams->comphyC12GP41P2VEomInParams.voltageSteps > 16)
    {
        return GT_OUT_OF_RANGE;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    /* for low baud rates, need to change resolution and enable DFE */
    MCESD_ATTEMPT(API_C12GP41P2V_GetDfeEnable(sDev, &dfeState));
    if (dfeState == MCESD_FALSE)
    {
        /* G1_DFE_RES_F0_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x4F, 5, 2, 0x3));
        MCESD_ATTEMPT(API_C12GP41P2V_SetDfeEnable(sDev, MCESD_TRUE));
    }

    mcesdEyeRawData = (S_C12GP41P2V_EYE_RAW*)hwsOsMallocFuncPtr(sizeof(S_C12GP41P2V_EYE_RAW));
    if ( NULL == mcesdEyeRawData )
    {
        return GT_NO_RESOURCE;
    }

    do
    {
        mcesdSt = API_C12GP41P2V_EOMGetEyeData(sDev, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.phaseLevels, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.voltageSteps, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.minSamples, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.berThreshold, mcesdEyeRawData);
        if ( mcesdSt != MCESD_OK )
        {
            retries--;
            if (retries == 0)
            {
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
        }
        else
        {
            break;
        }
    } while (retries > 0);

    /* retrieve height and width in code word and convert to mV and pSec */
    mcesdSt = API_C12GP41P2V_EyeDataDimensions(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.berThreshold,
                                               &width, &heightUpper, &heightLower);
    if ( mcesdSt != MCESD_OK )
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_FAIL;
    }
    mcesdSt = API_C12GP41P2V_EOMConvertWidthHeight(sDev, width, heightUpper, heightLower, &widthmUI, &heightmVUpper, &heightmVLower);
    if ( mcesdSt != MCESD_OK )
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_FAIL;
    }
    eomOutParams->comphyC12GP41P2VEomOutParams.width_mUI   = (GT_U32)widthmUI;
    eomOutParams->comphyC12GP41P2VEomOutParams.height_mV   = (GT_U32)(heightmVUpper + heightmVLower);

    if ( GT_TRUE == printEnable )
    {
        mcesdSt = API_C12GP41P2V_EOMPlotEyeData(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.phaseLevels, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.voltageSteps,
                                              (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.berThreshold, (MCESD_U32)eomInParams->comphyC12GP41P2VEomInParams.berThresholdMax);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
    }

    hwsOsFreeFuncPtr(mcesdEyeRawData);

    /* restore DFE for low baud rates */
    if (dfeState == MCESD_FALSE)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x4F, 5, 2, 0x0)); /* G1_DFE_RES_F0_LANE */
        MCESD_CHECK_STATUS(API_C12GP41P2V_SetDfeEnable(sDev, MCESD_FALSE));
    }
    return GT_OK;
}

static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_BandGapOptimization
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum
)
{
    GT_U32 data = 0;
    GT_STATUS rc;

    /* TBD : need to take care of OOB ports */

    /* in case we have already enabled the WA we do not want to reconfigure it again */
    rc = mvHwsComphyC12GP41P2VSerdesCheckAccess(devNum, 0, serdesNum);
    if (rc == GT_OK)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0xc0 / 4, 2, 2, &data));
        if (data == 0x3)
        {
            return GT_OK;
        }
    }

    /* Analog Unreset */
    CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesReset(devNum, portGroup, serdesNum, GT_FALSE, GT_TRUE, GT_TRUE));
    CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesCoreReset(devNum, portGroup, serdesNum, UNRESET));

    /* force band-gap active */
    if (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesBgEnable) == GT_TRUE)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xc0 / 4, 2, 2,  0x3)); /* ana_pu_bg, pu_bg_force */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x44 / 4 , 0, 8,  0xc0)); /* reg_ana_cmn_test */
    }

    if (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesBgClkBypassEnable) == GT_TRUE)
    {
        /* bg_ring_speed=0x3, bg_clkbypass_en=1 */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x224 / 4, 8, 3,  0x7));
    }

    return GT_OK;
}

#endif /* MV_HWS_REDUCED_BUILD */

static GT_STATUS mvHwsComphyC12GP41P2VSip6_30_SerdesRxInit
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    MV_HWS_RX_INIT_MODE     rxInitMode,
    MV_HWS_AUTO_TUNE_STATUS *rxInitStatus
)
{

    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_BOOL                     sigDetect;
    MV_HWS_SERDES_LB            lbMode;
    MCESD_U32                   completed, rxInitDone;

    *rxInitStatus = TUNE_PASS;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

    /* skip if serdes loopback analog tx2rx */
    CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesLoopbackGet(devNum, portGroup, serdesNum, &lbMode));
    if ( lbMode ==  SERDES_LP_AN_TX_RX )
    {
        return GT_OK;
    }

    /* check signal */
    MCESD_CHECK_STATUS(mvHwsComphyC12GP41P2VSip6_30_SerdesSignalDetectGet(devNum, portGroup, serdesNum, &sigDetect));
    if (sigDetect == GT_TRUE)
    {
        MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_RX_TRAIN_ENABLE0, &completed));
        MCESD_CHECK_STATUS(API_C12GP41P2V_HwGetPinCfg(sDev, C12GP41P2V_PIN_RX_INIT_DONE0, &rxInitDone));
        if ((completed == 1) || (rxInitDone == 1))
            return GT_OK;

        switch (rxInitMode)
        {
        case RX_INIT_BLOCKING:
            MCESD_CHECK_STATUS(API_C12GP41P2V_RxInit(sDev));
            break;
        default:
            return GT_NOT_SUPPORTED;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC12GP41P2VSip6_30_IfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC12GP41P2VSip6_30_IfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_C12GP41P2V_SIP6_30])
    {
        funcPtrArray[COM_PHY_C12GP41P2V_SIP6_30] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_C12GP41P2V_SIP6_30])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_C12GP41P2V_SIP6_30], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc             ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc                  ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc                ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc             ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc             ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc               ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc              ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc              ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc         ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc                       ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset                    ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc                     ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc                  ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc               ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc              ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc                    ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxIfSelectFunc                  ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComH28nmSerdesTxIfSelect);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxIfSelectGetFunc               ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComH28nmSerdesTxIfSelectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc                ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc             ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                        ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesOperation);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc                  ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesDbRxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc                  ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc            ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesCdrLockStatusGet);
#ifndef MV_HWS_REDUCED_BUILD
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEomMatrixGetFunc                 ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesEomMatrixGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesBandGapOptimizationFunc          ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_BandGapOptimization);
#endif
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxInitFunc                       ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesRxInit);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTemperatureFunc                  ,COM_PHY_C12GP41P2V_SIP6_30, mvHwsComphyC12GP41P2VSip6_30_SerdesTemperatureGet);

    return GT_OK;
}

#endif /* C12GP41P2V_SIP6_30 */
