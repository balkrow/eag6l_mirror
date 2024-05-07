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
* @file mvComphyC28GP4X4SerdesIf.c \
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#ifdef C28GP4X4
#include "mcesdC28GP4X4_Defs.h"
#include "mcesdC28GP4X4_API.h"
#include "mcesdC28GP4X4_DeviceInit.h"
#include "mcesdC28GP4X4_HwCntl.h"
#include "mcesdC28GP4X4_RegRW.h"
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C28GP4X4_main.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C28GP4X4_SERDES_REF25MHz_CMN.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C28GP4X4_SERDES_REF156MHz_CMN.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C28GP4X4_SERDES_SPDCHG2_LANE.h>

#define MCESD_SERDES_SPEED_20G_CHECK_MAC(_speed) ( (C28GP4X4_SERDES_20P625G_RING == _speed) || (C28GP4X4_SERDES_20P625G_LC == _speed) || (C28GP4X4_SERDES_25P78125G == _speed) ||   \
                                                   (C28GP4X4_SERDES_26P5625G == _speed) || (C28GP4X4_SERDES_27P5G == _speed) || (C28GP4X4_SERDES_28P125G == _speed) )

GT_STATUS mvHwsComphyC28GP4X4SerdesDbRxConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_SPEED                 baudRate,
    IN  MV_HWS_SERDES_DB_TYPE               dbType,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
);

GT_STATUS mvHwsComphyC28GP4X4SerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
);

static GT_STATUS mvHwsComphyC28GP4X4SerdesResetDfe
(
   GT_U8                                  devNum,
   GT_U32                                 portGroup,
   GT_U32                                 serdesNum
)
{
    MCESD_DEV_PTR sDev;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    /* Reset For Analog DFE Registers */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2410, 2, 1, 0x1));
    MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, DFE_RESET_DELAY));
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2410, 2, 1, 0x0));

   return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesCtleGraduallyTune function
* @endinternal
*
* @brief   tune gradually ctle limits (min/max)
*
* @param[in] devPtr                   - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
* @param[in] lane                     - lane number 0, 1, etc.
* @param[in] param                    - ctle enum
* @param[in] startVal                 - serdes number
* @param[in] endVal                   - serdes number
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyC28GP4X4SerdesCtleGraduallyTune
(
    MCESD_DEV_PTR           devPtr,
    MCESD_U8                lane,
    E_C28GP4X4_CTLE_PARAM   param,
    MCESD_U32               startVal,
    MCESD_U32               endVal
)
{
    /* even if values are equal need to apply force */
    if (startVal == endVal)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(devPtr, lane, param, startVal));
    }
    while (startVal != endVal)
    {
        if ( startVal < endVal )
            startVal++; /* need to tune ascendly from current value to requested min value */
        else
            startVal--; /* need to tune descendly from current value to requested max value */

        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(devPtr, lane, param, startVal));
        MCESD_CHECK_STATUS(API_C28GP4X4_Wait(devPtr, 1));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesManualRxConfig function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C28GP4X4_SERDES_SPEED     txSpeed, rxSpeed;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.dataRate)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_DATA_RATE, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.dataRate ));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.res1Sel)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL,  (MCESD_U32)rxConfigPtr->rxComphyC28GP4.res1Sel  ));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.res2Sel)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES2_SEL,  (MCESD_U32)rxConfigPtr->rxComphyC28GP4.res2Sel  ));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.cap1Sel)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP1_SEL,  (MCESD_U32)rxConfigPtr->rxComphyC28GP4.cap1Sel  ));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.cap2Sel)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP2_SEL,  (MCESD_U32)rxConfigPtr->rxComphyC28GP4.cap2Sel  ));

    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.selmufi)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.selmufi));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.selmuff)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.selmuff));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.selmupi)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.selmupi));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.selmupf)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.selmupf));

    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointLargeThresKLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x6050, (0xF << 28), rxConfigPtr->rxComphyC28GP4.midpointLargeThresKLane << 28));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointSmallThresKLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x6050, (0xF << 20), rxConfigPtr->rxComphyC28GP4.midpointSmallThresKLane << 20));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointLargeThresCLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x6050, (0xF << 24), rxConfigPtr->rxComphyC28GP4.midpointLargeThresCLane << 24));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointSmallThresCLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x6050, (0xF << 16), rxConfigPtr->rxComphyC28GP4.midpointSmallThresCLane << 16));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.inxSumfMidpointAdatptiveEnLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x60D8, (0x1 << 26), rxConfigPtr->rxComphyC28GP4.inxSumfMidpointAdatptiveEnLane << 26));

    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.dfeResF0aHighThresInitLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x602C, (0xFF << 24),rxConfigPtr->rxComphyC28GP4.dfeResF0aHighThresInitLane << 24));
    if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.dfeResF0aHighThresEndLane)
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x606C, (0xFF << 0), rxConfigPtr->rxComphyC28GP4.dfeResF0aHighThresEndLane  << 0));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxRxBitRate(sDev, serdesInfo->internalLane, &txSpeed, &rxSpeed));
    if (MCESD_SERDES_SPEED_20G_CHECK_MAC(rxSpeed))
    {
        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.minCap)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60E4, 28, 4, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.minCap));

        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.minCapN)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60E4, 20, 4, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.minCapN));

        if (NA_16BIT != rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC0)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60C0, 24, 8, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC0));

        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC1)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60C0, 8, 8, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC1));

        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC2)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60C4, 24, 8, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC2));

        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointPhaseOs0)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60C0, 16, 8, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.midpointPhaseOs0));

        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointPhaseOs1)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60C0, 0, 8, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.midpointPhaseOs1));

        if(NA_16BIT != rxConfigPtr->rxComphyC28GP4.midpointPhaseOs2)
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x60C4, 16, 8, (MCESD_U32)rxConfigPtr->rxComphyC28GP4.midpointPhaseOs2));
    }

    if(NA_16BIT_SIGNED != rxConfigPtr->rxComphyC28GP4.squelch)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetSquelchThreshold(sDev, serdesInfo->internalLane, (MCESD_16)rxConfigPtr->rxComphyC28GP4.squelch));

    /** Static configurations */
    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x2170, (0xFFFF << 16), 0x3FF << 16));

    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x2170, (0x1 << 12), 0x1 << 12));

    /* VTH_RXIMPCAL */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x8228, 0x1C, 0x8));

    return GT_OK;
}


static GT_STATUS mvHwsComphyC28GP4X4SerdesOperation
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
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR   sDev;
    MCESD_U32       cap1, res1, ctleLimitMinCap1, ctleLimitMaxCap1, ctleLimitMinRes1, ctleLimitMaxRes1;
    MCESD_U16       align90;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT   rxParameters;
    GT_STATUS rc;
    GT_U32                      regVal;
    GT_32 stepSize;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(portMode);

    /* application must not pass NULL pointers */
    if((resultPtr == NULL) || (dataPtr == NULL))
    {
        return GT_BAD_PARAM;
    }

    *resultPtr = 0;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    switch ( operation )
    {
    case MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E:
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesResetDfe(devNum, portGroup, serdesNum));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_CHECK_CLAMPING_TRIGGERED_E:
        /* clear_dtl_clamping */
        MCESD_ATTEMPT(mvHwsComphySerdesRegisterWriteField(devNum,portGroup,serdesNum,0x2160,12,1,1));
        MCESD_ATTEMPT(mvHwsComphySerdesRegisterWriteField(devNum,portGroup,serdesNum,0x2160,12,1,0));
        MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 1));
        /* read dtl_clamping_triggered_lane */

        MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,17,1,resultPtr));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_CDR_E:

        /* get res1, cap1, align90 */

        MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL, &res1));
        MCESD_CHECK_STATUS(API_C28GP4X4_GetAlign90(sDev, serdesInfo->internalLane, &align90));

        /* tune values */
        if (res1 == 0)
        {
            CHECK_STATUS(mvHwsComphySerdesRegisterRead(devNum, portGroup,serdesNum,0x230c,0xffffffff,&regVal));
            stepSize = regVal;
            align90 = (align90 + stepSize);
            if (align90 > 63)
            {
                stepSize = -4;
                align90 = 63;
            }
            else if (align90 <= 0)
            {
                stepSize = 4;
                align90 = 0;
            }
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x230c, 0xFFFFFFFF, stepSize));

            MCESD_CHECK_STATUS(API_C28GP4X4_SetAlign90(sDev, serdesInfo->internalLane, align90));
        }
        res1 = (res1 == 0) ? 4  : (res1 - 1);

        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL, res1));
        /* at this stage the training is only for DFE - start and stop training - might not complete but should not be a problem */
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
        MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 1));
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_SET_CTLE_LIMITS_E:
        MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL,   &res1));
        MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP1_SEL,   &cap1));
        rc = mvHwsComphyC28GP4X4SerdesDbRxConfigGet(devNum, 0, serdesNum, (MV_HWS_SERDES_SPEED) dataPtr[0] /*serdes speed*/, SERDES_DB_OVERRIDE_ONLY, &rxParameters);

        if ( rc != GT_OK)
        {
            if ( rc == GT_NOT_FOUND )/*No override params - no need to continue*/
            {
                return GT_OK;
            }
            return rc;/*Fail to get override parameters*/
        }

        ctleLimitMinCap1 = (MCESD_U32)rxParameters.rxComphyC28GP4.minCap1;
        ctleLimitMaxCap1 = (MCESD_U32)rxParameters.rxComphyC28GP4.maxCap1;
        ctleLimitMinRes1 = (MCESD_U32)rxParameters.rxComphyC28GP4.minRes1;
        ctleLimitMaxRes1 = (MCESD_U32)rxParameters.rxComphyC28GP4.maxRes1;

        if ((ctleLimitMaxCap1 < NA_16BIT) && (cap1 >= ctleLimitMaxCap1))
        {
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesCtleGraduallyTune(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP1_SEL, cap1, ctleLimitMaxCap1));
        }
        else if ((ctleLimitMinCap1 < NA_16BIT) && (cap1 <= ctleLimitMinCap1))
        {
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesCtleGraduallyTune(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP1_SEL, cap1, ctleLimitMinCap1));
        }
        if ((ctleLimitMaxRes1 < NA_16BIT) && (res1 >= ctleLimitMaxRes1))
        {
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesCtleGraduallyTune(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL, res1, ctleLimitMaxRes1));
        }
        else if ((ctleLimitMinRes1 < NA_16BIT) && (res1 <= ctleLimitMinRes1))
        {
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesCtleGraduallyTune(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL, res1, ctleLimitMinRes1));
        }
        break;

    case MV_HWS_PORT_SERDES_OPERATION_PRE_RX_TRAINING_E:
        /* before training we want to set the rx serdes params values to default */
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesDbRxConfigGet(devNum, serdesInfo->internalLane, serdesNum, (MV_HWS_SERDES_SPEED) dataPtr[0] /*serdes speed*/, SERDES_DB_OVERRIDE_AND_DEFAULT, &rxParameters));
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesManualRxConfig(devNum, serdesInfo->internalLane, serdesNum, &rxParameters));
        /* align90 back to default value */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x0038, 0x2, 0x6, 30));
        /* Reset For Analog DFE Registers */
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesResetDfe(devNum, portGroup, serdesNum));

        break;
    default:
        return GT_OK;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4GetDefaultParameters function
* @endinternal
*
* @brief   Power up SERDES list.
*
* @param[in] devNum     - system device number
* @param[in] serdesNum  - SerDes number
* @param[in] baudRate   - SerDes baud rate
* @param[out] txDefaultParameters - The default parameters for
*       the given baud rate
* @param[out] rxDefaultParameters - he default parameters for
*       the given baud rate
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4GetDefaultParameters
(
    IN  GT_U8                              devNum,
    IN  GT_UOPT                            serdesNum,
    IN  MV_HWS_SERDES_SPEED                baudRate,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT   *txDefaultParameters,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT   *rxDefaultParameters
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    MV_HWS_SERDES_SPEED         baudRateIterator;
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;

    if((txDefaultParameters == NULL) && (rxDefaultParameters == NULL))
    {
        return GT_BAD_PARAM;
    }
#ifndef WIN32

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
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

    for(baudRateIterator = (MV_HWS_SERDES_SPEED)0 ; tuneParams[baudRateIterator].serdesSpeed != LAST_MV_HWS_SERDES_SPEED ; baudRateIterator++)
    {
        if(tuneParams[baudRateIterator].serdesSpeed == baudRate) break;
    }
    if(tuneParams[baudRateIterator].serdesSpeed == LAST_MV_HWS_SERDES_SPEED)
    {
        return GT_NOT_FOUND;
    }

    if(txDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&txDefaultParameters->txComphyC28GP4,
                            &tuneParams[baudRateIterator].txParams.txComphyC28GP4,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&rxDefaultParameters->rxComphyC28GP4,
                            &tuneParams[baudRateIterator].rxParams.rxComphyC28GP4,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA));
    }
#else
    GT_UNUSED_PARAM(serdesInfo);
    GT_UNUSED_PARAM(serdesData);
    GT_UNUSED_PARAM(baudRateIterator);

    if(txDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&txDefaultParameters->txComphyC28GP4, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&rxDefaultParameters->rxComphyC28GP4, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA));
    }

#endif
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesDbTxConfigGet function
* @endinternal
*
* @brief   Get config SERDES electrical parameters from DB
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[out] txConfigPtr - pointer to Serdes Tx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesDbTxConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_SPEED                 baudRate,
    IN  MV_HWS_SERDES_DB_TYPE               dbType,
    OUT MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       defaultParamIterator = 0;
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;
    GT_U8 sdSpeed, speedIdx=0;
    GT_BOOL overrideExist = GT_FALSE;
#ifdef _WIN32
    MV_HWS_SERDES_TXRX_TUNE_PARAMS  winTuneParams[1];
#endif


    GT_UNUSED_PARAM(portGroup);

    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);

#ifndef _WIN32
#if defined  SHARED_MEMORY
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
#else
    /*Init default params*/
    hwsOsMemSetFuncPtr(winTuneParams, 0, sizeof(winTuneParams));
    defaultParamIterator = 0;
    winTuneParams[defaultParamIterator].serdesSpeed = baudRate;

    tuneParams = winTuneParams;
#endif


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

    switch ( dbType )
    {
        case SERDES_DB_OVERRIDE_ONLY:
            if (!overrideExist)/*override param not found*/
            {
                return GT_NOT_FOUND;
            }
            else/*override param exist*/
            {
                hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC28GP4),
                                    &(serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC28GP4),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA));
            }
            break;
        case SERDES_DB_OVERRIDE_AND_DEFAULT:
            if (!overrideExist)/*override param not found - set default param*/
            {
                hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC28GP4),
                                    &(tuneParams[defaultParamIterator].txParams.txComphyC28GP4),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA));
            }
            else/*override param exist - go over tx params and set override value if exist or default if not exist*/
            {
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(txConfigPtr->txComphyC28GP4.pre,    tuneParams[defaultParamIterator].txParams.txComphyC28GP4.pre,  serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC28GP4.pre);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(txConfigPtr->txComphyC28GP4.peak,   tuneParams[defaultParamIterator].txParams.txComphyC28GP4.peak, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC28GP4.peak);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(txConfigPtr->txComphyC28GP4.post,   tuneParams[defaultParamIterator].txParams.txComphyC28GP4.post, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC28GP4.post);
            }
            break;
        case SERDES_DB_DEFAULT_ONLY:
            hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC28GP4),
                                    &(tuneParams[defaultParamIterator].txParams.txComphyC28GP4),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C28G_TX_CONFIG_DATA));
            break;
        default:
            rc = GT_NOT_INITIALIZED;
            break;
    }

    return rc;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesDbRxConfigGet function
* @endinternal
*
* @brief   Get config SERDES electrical parameters from DB
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[out] txConfigPtr - pointer to Serdes Tx params struct
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesDbRxConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_SPEED                 baudRate,
    IN  MV_HWS_SERDES_DB_TYPE               dbType,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    GT_STATUS rc = GT_OK;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_COMPHY_SERDES_DATA   *serdesData;
    GT_U8                       defaultParamIterator = 0;
    const MV_HWS_SERDES_TXRX_TUNE_PARAMS      *tuneParams;
    GT_U8 sdSpeed, speedIdx=0;
    GT_BOOL overrideExist = GT_FALSE;
#ifdef _WIN32
    MV_HWS_SERDES_TXRX_TUNE_PARAMS  winTuneParams[1];
#endif

    GT_UNUSED_PARAM(portGroup);

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,NULL));
    serdesData = HWS_COMPHY_GET_SERDES_DATA_FROM_SERDES_INFO(serdesInfo);

#ifndef _WIN32
#if defined  SHARED_MEMORY
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
#else
    /*Init default params*/
    hwsOsMemSetFuncPtr(winTuneParams, 0, sizeof(winTuneParams));
    defaultParamIterator = 0;
    winTuneParams[defaultParamIterator].serdesSpeed = baudRate;

    tuneParams = winTuneParams;
#endif


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

    switch ( dbType )
    {
        case SERDES_DB_OVERRIDE_ONLY:
            if (!overrideExist) /*override param not found*/
            {
                return GT_NOT_FOUND;
            }
            else/*override param exist*/
            {
                hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC28GP4),
                                    &(serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA));
            }
            break;
        case SERDES_DB_OVERRIDE_AND_DEFAULT:
            if (!overrideExist) /*override param not found - set default param*/
            {
                hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC28GP4),
                                    &(tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA));
            }
            else/*override param exist - go over rx params and set override value if exist or default if not exist*/
            {
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.dataRate,                         tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.dataRate,                      serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.dataRate);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.res1Sel,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.res1Sel,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.res1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.res2Sel,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.res2Sel,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.res2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.cap1Sel,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.cap1Sel,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.cap1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.cap2Sel,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.cap2Sel,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.cap2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.minCap,                        tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.minCap,                     serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.minCap);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.minCapN,                       tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.minCapN,                    serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.minCapN);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.selmufi,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.selmufi,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.selmufi);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.selmuff,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.selmuff,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.selmuff);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.selmupi,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.selmupi,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.selmupi);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.selmupf,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.selmupf,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.selmupf);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointLargeThresKLane,          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointLargeThresKLane,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointLargeThresKLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointSmallThresKLane,          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointSmallThresKLane,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointSmallThresKLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointLargeThresCLane,          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointLargeThresCLane,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointLargeThresCLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointSmallThresCLane,          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointSmallThresCLane,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointSmallThresCLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.inxSumfMidpointAdatptiveEnLane,   tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.inxSumfMidpointAdatptiveEnLane,serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.inxSumfMidpointAdatptiveEnLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.dfeResF0aHighThresInitLane,       tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.dfeResF0aHighThresInitLane,    serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.dfeResF0aHighThresInitLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.dfeResF0aHighThresEndLane,        tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.dfeResF0aHighThresEndLane,     serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.dfeResF0aHighThresEndLane);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.minCap,                           tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.minCap,                        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.minCap);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.minCapN,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.minCapN,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.minCapN);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC0,                tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.sumfBoostTargetC0,             serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.sumfBoostTargetC0);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC1,                tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.sumfBoostTargetC1,             serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.sumfBoostTargetC1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.sumfBoostTargetC2,                tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.sumfBoostTargetC2,             serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.sumfBoostTargetC2);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointPhaseOs0,                 tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointPhaseOs0,              serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointPhaseOs0);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointPhaseOs1,                 tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointPhaseOs1,              serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointPhaseOs1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.midpointPhaseOs2,                 tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.midpointPhaseOs2,              serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.midpointPhaseOs2);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.minCap1,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.minCap1,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.minCap1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.maxCap1,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.maxCap1,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.maxCap1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.minRes1,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.minRes1,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.minRes1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC28GP4.maxRes1,                          tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.maxRes1,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.maxRes1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_SIGNED_VAL(rxConfigPtr->rxComphyC28GP4.squelch,                   tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4.squelch,                       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC28GP4.squelch);
            }
            break;
        case SERDES_DB_DEFAULT_ONLY:
            hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC28GP4),
                                    &(tuneParams[defaultParamIterator].rxParams.rxComphyC28GP4),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C28GP4_RX_CONFIG_DATA));
            break;
        default:
            rc = GT_NOT_INITIALIZED;
            break;
    }

    return rc;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesManualTxConfig function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesManualTxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_TX_CONFIG_DATA_UNT    *txConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(txConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(NA_8BIT != txConfigPtr->txComphyC28GP4.pre)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetTxEqParam(sDev, serdesInfo->internalLane, C28GP4X4_TXEQ_EM_PRE_CTRL , (MCESD_U32)txConfigPtr->txComphyC28GP4.pre));
    if(NA_8BIT != txConfigPtr->txComphyC28GP4.peak)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetTxEqParam(sDev, serdesInfo->internalLane, C28GP4X4_TXEQ_EM_PEAK_CTRL, (MCESD_U32)txConfigPtr->txComphyC28GP4.peak));
    if(NA_8BIT != txConfigPtr->txComphyC28GP4.post)
        MCESD_CHECK_STATUS(API_C28GP4X4_SetTxEqParam(sDev, serdesInfo->internalLane, C28GP4X4_TXEQ_EM_POST_CTRL, (MCESD_U32)txConfigPtr->txComphyC28GP4.post));

    /** Static configurations */
    /* VTH_TXIMPCAL */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x8204, 0x1C, 0x8));

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesReset function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesReset
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

    /* SERDES SD RESET/UNRESET init */
    data = (analogReset == GT_TRUE) ? 0 : 1;
    /*CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18)));*/

    /* SERDES RF RESET/UNRESET init */
    data = (digitalReset == GT_TRUE) ? 1 : 0;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18)));

    /* SERDES SYNCE RESET init */
    data = (syncEReset == GT_TRUE) ? 0 : 1;
    /*CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18));*/

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesCoreReset function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_UREG_DATA data;

    data = (coreReset == UNRESET) ? 0 : 1;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 17), (1 << 17)));

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesArrayPowerCtrl function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesArrayPowerCtrl
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
    GT_U16                             retries = PLL_INIT_POLLING_RETRIES;
    GT_U32                             data;
    GT_U32                             mcesdEnum;
    GT_BOOL                            allStable;

    MCESD_DEV_PTR                sDev;
    E_C28GP4X4_SERDES_SPEED      speed = 0;
    E_C28GP4X4_REFFREQ           refFreq = 0;
    E_C28GP4X4_REFCLK_SEL        refClkSel = 0;
    E_C28GP4X4_DATABUS_WIDTH     dataBusWidth = 0;
    S_C28GP4X4_PowerOn           powerOnConfig;
    MV_HWS_SERDES_SPEED          baudRate = serdesConfigPtr->baudRate;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);
    if(powerUp)
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Power-on lane */
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_C28GP4X4_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_C28GP4X4_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_C28GP4X4_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_C28GP4X4_DATABUS_WIDTH)mcesdEnum;
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);

            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], UNRESET));

            powerOnConfig.powerLaneMask = 1 << serdesInfo->internalLane;
            powerOnConfig.initTx        = MCESD_TRUE;
            powerOnConfig.initRx        = MCESD_TRUE;
            powerOnConfig.txOutputEn    = MCESD_FALSE;
            powerOnConfig.dataPath      = C28GP4X4_PATH_EXTERNAL;
            powerOnConfig.downloadFw    = MCESD_FALSE;

            powerOnConfig.refClkSel     = refClkSel;
            powerOnConfig.dataBusWidth  = dataBusWidth;
            powerOnConfig.speed         = speed;
            powerOnConfig.refFreq       = refFreq;

            MCESD_CHECK_STATUS(API_C28GP4X4_PowerOnSeq(sDev, powerOnConfig));
        }

        /* Wait for stable PLLs */
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[0],NULL,NULL,&sDev));
        do
        {
            allStable = GT_TRUE;
            for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
            {
                CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SDW_LANE_STATUS_0, &data, PLL_INIT_POLLING_DONE));
                if (data != PLL_INIT_POLLING_DONE)
                {
                    allStable = GT_FALSE;
                    break;
                }
            }
            MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            retries--;
        } while ((retries != 0) && (allStable == GT_FALSE));
        if(allStable == GT_FALSE)
        {
            return GT_TIMEOUT;
        }

        for(serdesIterator = 0; serdesIterator < numOfSer; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Config Tx/Rx parameters */
            MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(serdesConfigPtr->opticalMode, baudRate);
#if 0
            CHECK_STATUS(mvHwsComphyC28GP4X4GetDefaultParameters(devNum, serdesArr[serdesIterator], baudRate, &txDefaultParameters, &rxDefaultParameters));
#endif
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesDbTxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &txParameters));
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesDbRxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &rxParameters));

            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txParameters));
            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxParameters));
            /* set step size for the destractive CDR */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator], 0x230c, 0xFFFFFFFF, 4));
            /* align90 back to default value */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator], 0x0038, 0x2, 0x6, 30));
            /*******************************************************/
            /* WA - need to remove after CE integrate it to the FW */
            /* update icp_ring_lane according refClock (only for 11G speed and below) */
            if (( serdesConfigPtr->baudRate >= _1_25G && serdesConfigPtr->baudRate <= _11_5625G ) || ( serdesConfigPtr->baudRate == _5_625G ) || ( serdesConfigPtr->baudRate == _5_15625G ) ||
                ( serdesConfigPtr->baudRate == _10G ) || ( serdesConfigPtr->baudRate == _2_578125 ) || ( serdesConfigPtr->baudRate == _1_25G_SR_LR ))
            {
                if ( _25Mhz == serdesConfigPtr->refClock )
                {
                    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesArr[serdesIterator], 0x1008, 0x4, 0x4, 0xF));
                }
                else if ( _156dot25Mhz == serdesConfigPtr->refClock )
                {
                    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesArr[serdesIterator], 0x1008, 0x4, 0x4, 0xC));
                }
            }
            /*******************************************************/

            CHECK_STATUS(mvHwsComphyC28GP4X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));
            MCESD_CHECK_STATUS(API_C28GP4X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));
        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* if user enabled PRBS and deleted the port (SerDes is powered-down), need to restore to normal mode,
               in order to enable port to reach link up in next power-up */
            MCESD_CHECK_STATUS(API_C28GP4X4_StopPhyTest(sDev, serdesInfo->internalLane));

            MCESD_CHECK_STATUS(API_C28GP4X4_PowerOffLane(sDev, serdesInfo->internalLane));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesPowerCtrl function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyC28GP4X4SerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyC28GP4X4SerdesRxAutoTuneStart function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_StartTraining(sDev, serdesInfo->internalLane, C28GP4X4_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_StopTraining(sDev, serdesInfo->internalLane, C28GP4X4_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTxAutoTuneStart function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(training)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_StartTraining(sDev, serdesInfo->internalLane, C28GP4X4_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_StopTraining(sDev, serdesInfo->internalLane, C28GP4X4_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTxAutoTuneStop function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyC28GP4X4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyC28GP4X4SerdesAutoTuneStart function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesAutoTuneStart
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
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyC28GP4X4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesAutoTuneStatus function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesAutoTuneStatus
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
    MCESD_BOOL                  completed;
    MCESD_BOOL                  failed;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(NULL != rxStatus)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_CheckTraining(sDev, serdesInfo->internalLane, C28GP4X4_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_CheckTraining(sDev, serdesInfo->internalLane, C28GP4X4_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesAutoTuneStatusShort
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
GT_STATUS mvHwsComphyC28GP4X4SerdesAutoTuneStatusShort
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
        MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, C28GP4X4_PIN_RX_TRAIN_COMPLETE0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, C28GP4X4_PIN_RX_TRAIN_FAILED0 + serdesInfo->internalLane, &failedData));
        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, C28GP4X4_PIN_TX_TRAIN_COMPLETE0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, C28GP4X4_PIN_TX_TRAIN_FAILED0 + serdesInfo->internalLane, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesAutoTuneResult function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MV_HWS_PER_SERDES_INFO_PTR              serdesInfo;
    MCESD_DEV_PTR                           sDev;
    MV_HWS_COMPHY_C28GP4_AUTO_TUNE_RESULTS  *results;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U16 val16;
    MCESD_16  vals16;
    MCESD_U32 tapsI;
    GT_U32 gtVal;
    E_C28GP4X4_SERDES_SPEED txSpeed, rxSpeed;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC28GP4Results;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxEqParam(sDev, serdesInfo->internalLane, C28GP4X4_TXEQ_EM_PRE_CTRL,  &val));
    results->txComphyC28GP4.pre  = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxEqParam(sDev, serdesInfo->internalLane, C28GP4X4_TXEQ_EM_PEAK_CTRL, &val));
    results->txComphyC28GP4.peak = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxEqParam(sDev, serdesInfo->internalLane, C28GP4X4_TXEQ_EM_POST_CTRL, &val));
    results->txComphyC28GP4.post = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_DATA_RATE,  &val));
    results->rxComphyC28GP4.dataRate   = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES1_SEL,   &val));
    results->rxComphyC28GP4.res1Sel   = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_RES2_SEL,   &val));
    results->rxComphyC28GP4.res2Sel   = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP1_SEL,   &val));
    results->rxComphyC28GP4.cap1Sel   = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEParam(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_CAP2_SEL,   &val));
    results->rxComphyC28GP4.cap2Sel   = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetSquelchThreshold(sDev, serdesInfo->internalLane, &vals16));
    results->rxComphyC28GP4.squelch = (GT_16)vals16;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUFI, &val));
    results->rxComphyC28GP4.selmufi = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUFF, &val));
    results->rxComphyC28GP4.selmuff = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUPI, &val));
    results->rxComphyC28GP4.selmupi = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetCDRParam(sDev, serdesInfo->internalLane, C28GP4X4_CDR_SELMUPF, &val));
    results->rxComphyC28GP4.selmupf = (GT_U32)val;

    results->eo = 0;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetAlign90(sDev, serdesInfo->internalLane, &val16));
    results->align90 = (GT_U16)val16;

    results->sampler = 0;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetSlewRateParam(sDev, serdesInfo->internalLane, C28GP4X4_SR_CTRL0, &val));
    results->slewRateCtrl0 = (GT_U32)val;
    MCESD_CHECK_STATUS(API_C28GP4X4_GetSlewRateParam(sDev, serdesInfo->internalLane, C28GP4X4_SR_CTRL1, &val));
    results->slewRateCtrl1 = (GT_U32)val;

    for(tapsI = 0 ; tapsI < C28GP4X4_DFE_FF5 ; tapsI++)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_GetDfeTap(sDev, serdesInfo->internalLane, (E_C28GP4X4_DFE_TAP)tapsI, &vals));
        results->dfe[tapsI] = (GT_32)vals;
    }

    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x6050, 28, 4, &gtVal));
    results->rxComphyC28GP4.midpointLargeThresKLane = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x6050, 20, 4, &gtVal));
    results->rxComphyC28GP4.midpointSmallThresKLane = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x6050, 24, 4, &gtVal));
    results->rxComphyC28GP4.midpointLargeThresCLane = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x6050, 16, 4, &gtVal));
    results->rxComphyC28GP4.midpointSmallThresCLane = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60D8, 26, 1, &gtVal));
    results->rxComphyC28GP4.inxSumfMidpointAdatptiveEnLane = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x602C, 24, 8 ,&gtVal));
    results->rxComphyC28GP4.dfeResF0aHighThresInitLane = gtVal;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x606C,  0, 8, &gtVal));
    results->rxComphyC28GP4.dfeResF0aHighThresEndLane = gtVal;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxRxBitRate(sDev, serdesInfo->internalLane, &txSpeed, &rxSpeed));
    if (MCESD_SERDES_SPEED_20G_CHECK_MAC(rxSpeed))
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60E4, 28, 4, &gtVal));
        results->rxComphyC28GP4.minCap = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60E4, 20, 4, &gtVal));
        results->rxComphyC28GP4.minCapN = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60C0, 24, 8, &gtVal));
        results->rxComphyC28GP4.sumfBoostTargetC0 = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60C0, 8, 8, &gtVal));
        results->rxComphyC28GP4.sumfBoostTargetC1 = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60C4, 24, 8, &gtVal));
        results->rxComphyC28GP4.sumfBoostTargetC2 = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60C0, 16, 8, &gtVal));
        results->rxComphyC28GP4.midpointPhaseOs0 = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60C0, 0, 8, &gtVal));
        results->rxComphyC28GP4.midpointPhaseOs1 = gtVal;
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x60C4, 16, 8, &gtVal));
        results->rxComphyC28GP4.midpointPhaseOs2 = gtVal;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesDigitalReset function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesDigitalReset
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
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18)));

    return GT_OK;
}
/**
* @internal mvHwsComphyC28GP4X4SerdesLoopback function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C28GP4X4_DATAPATH         path;
    GT_U32                      mcesdEnum;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_C28GP4X4_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_C28GP4X4_SetDataPath(sDev, serdesInfo->internalLane, path));

    switch ( lbMode )
    {
    case SERDES_LP_AN_TX_RX:
        /* Masks sq_detected indication. If set, SQ detect is always zero (signal detected). Masks sq_detected indication */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (1 << 25), (1 << 25)));
        break;
    case SERDES_LP_DIG_RX_TX:
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum ,0xA008, 24, 2, serdesInfo->internalLane)); /* modify rx2tx_foffset_lane_sel value */
        MCESD_CHECK_STATUS(API_C28GP4X4_StopPhyTest(sDev, serdesInfo->internalLane));
        break;
    default:
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (0 << 25), (1 << 25)));
        break;
    }

    if (lbMode != SERDES_LP_DIG_RX_TX)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum ,0xA008, 24, 2, 0)); /* set rx2tx_foffset_lane_sel value to default */
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesLoopbackGet function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C28GP4X4_DATAPATH         path;
    GT_U32                      tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetDataPath(sDev, serdesInfo->internalLane, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTestGen function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTestGen
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  MV_HWS_SERDES_TX_PATTERN    txPattern,
    IN  MV_HWS_SERDES_TEST_GEN_MODE mode
)
{
    MV_HWS_PER_SERDES_INFO_PTR    serdesInfo;
    MCESD_DEV_PTR                 sDev;
    S_C28GP4X4_PATTERN            pattern;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &mcesdPattern));
    pattern.pattern  = mcesdPattern;
    pattern.enc8B10B = C28GP4X4_ENC_8B10B_DISABLE;

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
    MCESD_CHECK_STATUS(API_C28GP4X4_SetTxRxPattern(sDev, serdesInfo->internalLane, &pattern, &pattern, C28GP4X4_SATA_NOT_USED, C28GP4X4_DISPARITY_NOT_USED, userPattern, 0));
    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_ResetComparatorStats(sDev, serdesInfo->internalLane));
        MCESD_CHECK_STATUS(API_C28GP4X4_StartPhyTest(sDev, serdesInfo->internalLane));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_StopPhyTest(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTestGenGet function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTestGenGet
(
    GT_U8                        devNum,
    GT_UOPT                      portGroup,
    GT_UOPT                      serdesNum,
    MV_HWS_SERDES_TX_PATTERN     *txPatternPtr,
    MV_HWS_SERDES_TEST_GEN_MODE  *modePtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR        serdesInfo;
    MCESD_DEV_PTR                     sDev;
    S_C28GP4X4_PATTERN                txPattern;
    S_C28GP4X4_PATTERN                rxPattern;
    E_C28GP4X4_SATA_LONGSHORT         sataLongShort;
    E_C28GP4X4_SATA_INITIAL_DISPARITY sataInitialDisparity;
    char                              userPattern[21];
    MCESD_U8                          userK;
    MCESD_FIELD                       ptEn = F_C28GP4X4R1P0_PT_EN;
    MCESD_U32                         enable;
    GT_U32                            tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxRxPattern(sDev, serdesInfo->internalLane, &txPattern, &rxPattern, &sataLongShort,&sataInitialDisparity, userPattern, &userK));
    MCESD_CHECK_STATUS(API_C28GP4X4_ReadField(sDev, serdesInfo->internalLane, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C28GP4X4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern.pattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyC28GP4X4SerdesTestGenStatus
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTestGenStatus
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          serdesNum,
    IN MV_HWS_SERDES_TX_PATTERN        txPattern,
    IN GT_BOOL                         counterAccumulateMode,
    OUT MV_HWS_SERDES_TEST_GEN_STATUS   *status
)
{
    MV_HWS_PER_SERDES_INFO_PTR        serdesInfo;
    MCESD_DEV_PTR                     sDev;
    S_C28GP4X4_PATTERN_STATISTICS     statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C28GP4X4_GetComparatorStats(sDev, serdesInfo->internalLane, &statistics));

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr.l[0]   = (GT_U32) (statistics.totalErrorBits & 0x00000000FFFFFFFFUL);
    status->errorsCntr.l[1]   = (GT_U32)((statistics.totalErrorBits & 0xFFFFFFFF00000000UL) >> 32);
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_ResetComparatorStats(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesPolarityConfig function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesPolarityConfig
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL invertTx,
    GT_BOOL invertRx
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_SetTxRxPolarity(sDev, serdesInfo->internalLane, invertTx ? C28GP4X4_POLARITY_INVERTED : C28GP4X4_POLARITY_NORMAL,
                                                        invertRx ? C28GP4X4_POLARITY_INVERTED : C28GP4X4_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesPolarityConfigGet
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
GT_STATUS mvHwsComphyC28GP4X4SerdesPolarityConfigGet
(
  GT_U8             devNum,
  GT_UOPT           portGroup,
  GT_UOPT           serdesNum,
  GT_BOOL           *invertTx,
  GT_BOOL           *invertRx
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C28GP4X4_POLARITY         txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxRxPolarity(sDev, serdesInfo->internalLane, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == C28GP4X4_POLARITY_INVERTED);
    *invertRx = (rxPolarity == C28GP4X4_POLARITY_INVERTED);

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTxEnbale function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTxEnbale
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL txEnable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTxEnbale function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesTxEnbaleGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *txEnable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL*)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesSignalDetectGet function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesSignalDetectGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *enable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    MCESD_BOOL                  squelched;
    GT_U32                      data = 0;

    GT_UNUSED_PARAM(portGroup);

    /* in serdes loopback analog tx2rx, we forced signal detect in mvHwsComphyC112GX4SerdesLoopback */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, &data, (1 << 25)));
    if ( data != 0 )
    {
        *enable = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C28GP4X4_GetSquelchDetect(sDev, serdesInfo->internalLane, &squelched));

    *enable = squelched ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesCdrLockStatusGet function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *lock
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      data = 0, freqReady = 0, dtlClamping, cdrLock = 0, countConsecutive = 0, startSec, startNanoSec, currentSec, currentNanoSec;
    GT_16                       dataSigned = 0,dataSigned2 = 0, diff = 0;
    MCESD_U32                   completed;
    GT_BOOL                     timeout = GT_FALSE, state = GT_FALSE, updateStateAndCount = GT_FALSE;
    MV_HWS_SERDES_LB            lbMode;

    *lock = GT_FALSE;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

    CHECK_STATUS(hwsTimerGetFuncPtr(&startSec, &startNanoSec));

    MCESD_CHECK_STATUS(mvHwsComphyC28GP4X4SerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    if (*lock == GT_FALSE)
    {
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphyC28GP4X4SerdesLoopbackGet(devNum, portGroup, serdesNum, &lbMode));
    if (lbMode != SERDES_LP_DISABLE)
    {
        *lock = GT_TRUE;
        return GT_OK;
    }

    /* check if we're in a middle of rx train */
    MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, C28GP4X4_PIN_RX_TRAIN_ENABLE0, &completed));
    if (completed == 1)
    {
        *lock = GT_TRUE;
        return GT_OK;
    }
    /* RX TRAINING ONLY DFE = 1, the next training will be DFE training */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x6038,23, 1, 0x1));

    /* check 4 consecutive same result during MV_HWS_SERDES_CDR_LOCK_TIMEOUT */
    while (!timeout)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,17, 1, &dtlClamping)); /* dtl_clamping_triggered_lane */
        if (dtlClamping == 0x1)
        {
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x2160,12, 1, 0x1));  /* clear dtl_clamping */
            MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 1));
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x2160,12, 1, 0x0));
            MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, DTL_CLAMPING_DELAY));
            CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,17, 1, &dtlClamping)); /* dtl_clamping_triggered_lane */
        }

        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x213C,30, 1, &cdrLock)); /* CDR_LOCK_LANE */

        /* read freq offset ready */
        MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,31,1,&freqReady));
        if (freqReady == 0)
        {
            /* freq offset enable */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<20),(0x1<<20)));
            /* freq offset reset */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<19),(0x1<<19)));
            MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 1));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<19),0));
            MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 1));
            /* freq offset read request */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<16),(0x1<<16)));
            /* read freq offset ready */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,31,1,&freqReady));
        }
        else
        {
            /* read freq value twice and check if changed */
            MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,21,10,&data)); /* RX_FOFFSET_RD_LANE */
            dataSigned = data ;
            /* if freq is negative then convert to negative number */
            if ( dataSigned & 0x200 )
            {
                dataSigned |= 0xfc00;
            }
            MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 2));
            MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,21,10,&data));
            dataSigned2 = data ;
            /* if freq is negative then convert to negative number */
            if ( dataSigned2 & 0x200 )
            {
                dataSigned2 |= 0xfc00;
            }
            /* allow +/-2 tap change - 60ppm tolerance */
            diff = (dataSigned > dataSigned2) ? dataSigned - dataSigned2 : dataSigned2 - dataSigned;
            if (diff <= MV_HWS_COMPHY_PPM_TOLERANCE_FOR_28G)
            {
                if ( (30 * ABS(dataSigned) < MV_HWS_COMPHY_FREQUENCY_TOLERANCE) && (dtlClamping == 0) && (cdrLock == 0x1) )
                {
                    /* *lock = GT_TRUE;*/
                    updateStateAndCount = GT_TRUE;
                }
                else
                {
                    /* *lock = GT_FALSE;*/
                    updateStateAndCount = GT_FALSE;
                }
            }
            else
            {
                /* *lock = GT_FALSE;*/
                updateStateAndCount = GT_FALSE;
            }

            /* update state and counter */
            if (updateStateAndCount == GT_TRUE)
            {
                if (state == GT_TRUE)
                    countConsecutive++;
                else
                {
                    countConsecutive = 1;
                    state = GT_TRUE;
                }
            }
            else /* updateStateAndCount == GT_FALSE */
            {
                if (state == GT_FALSE)
                    countConsecutive++;
                else
                {
                    countConsecutive = 1;
                    state = GT_FALSE;
                }
            }

            /* check consecutive results or timeout */
            if (countConsecutive == MV_HWS_SERDES_CDR_LOCK_COUNT_CONSECUTIVE)
            {
                break;
            }
        }
        CHECK_STATUS(hwsTimerGetFuncPtr(&currentSec, &currentNanoSec));
        if (MV_HWS_TIME_DIFF(startSec, startNanoSec, currentSec, currentNanoSec) > MV_HWS_SERDES_CDR_LOCK_TIMEOUT)
        {
            timeout = GT_TRUE;
            break;
        }
        MCESD_CHECK_STATUS(API_C28GP4X4_Wait(sDev, 1));
        continue;
    }
    *lock = ( (state == GT_TRUE) && (countConsecutive == MV_HWS_SERDES_CDR_LOCK_COUNT_CONSECUTIVE) ) ? GT_TRUE : GT_FALSE;
    if (*lock)
    {
        /* RX TRAINING ONLY DFE = 0, the next training will be regular */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x6038,23, 1, 0x0));
    }
    return GT_OK;
}

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
/**
 * mvHwsComphyC28GP4X4InitProgData
 *
 * @brief prepare Prog Data for download
 *
 * @param devPtr
 * @param codePtr
 * @param codeSize
 * @param xType
 * @param xMemData
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsComphyC28GP4X4InitProgData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 codePtr[],
    IN MCESD_U32 codeSize,
    IN MV_HWS_COMPHY_RAM_DATA_TYPE xType,
    OUT MV_HWS_COMPHY_PROG_RAM_DATA *xMemData
)
{
    GT_U32 index;

    hwsOsMemSetFuncPtr(xMemData,0,sizeof(MV_HWS_COMPHY_PROG_RAM_DATA));

    xMemData->xType = xType;
    xMemData->lane = 255;
    xMemData->pDataPtr = codePtr;
    xMemData->dataSizeDW = codeSize;

    switch (xType)
    {
    case MV_HWS_COMPHY_CMN_DATA:
        xMemData->dataMaxSize = C28GP4X4_XDATA_CMN_MAX_SIZE;
        xMemData->dataBaseAddr = C28GP4X4_XDATA_CMN_BASE_ADDR;
        xMemData->regAddrExpectCheckSum = 0xA358;
        xMemData->regAddrActualCheckSum = 0xA35C;
        xMemData->regAddrCSPass = 0xA360;
        break;
    case MV_HWS_COMPHY_LANE_DATA:
        xMemData->lane = 0;
        xMemData->dataMaxSize = C28GP4X4_XDATA_LANE_MAX_SIZE;
        xMemData->dataBaseAddr = C28GP4X4_XDATA_LANE_BASE_ADDR;
        xMemData->regAddrExpectCheckSum = 0x22F8;
        xMemData->regAddrActualCheckSum = 0x22FC;
        xMemData->regAddrCSPass = 0x2294;
        break;
    case MV_HWS_COMPHY_FW_DATA:
        xMemData->dataMaxSize = C28GP4X4_FW_MAX_SIZE;
        xMemData->dataBaseAddr = C28GP4X4_FW_BASE_ADDR;
        xMemData->regAddrExpectCheckSum = 0xA224;
        xMemData->regAddrActualCheckSum = 0xA228;
        xMemData->regAddrCSPass = 0xA22C;
        break;
    default:
        return GT_FAIL;
    }

    /* Calculate CS incrementally */
    for (index = 0; index < codeSize; index++)
    {
        xMemData->expectCheckSum += mvHwsComphyCodeGet(index,codePtr);
    }
    MCESD_ATTEMPT(API_C28GP4X4_WriteReg(devPtr, 255, xMemData->regAddrExpectCheckSum, xMemData->expectCheckSum));

    return GT_OK;
}

/**
 * @internal mvHwsComphyC28GP4X4UpdateRamCode
 *
 * @brief program MCU memory
 *
 * @param devPtr
 * @param lane
 * @param code
 * @param codeSize
 * @param memSize
 * @param address
 * @param errCode
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsComphyC28GP4X4UpdateRamCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 code[],
    IN MCESD_U32 codeSize,
    IN MCESD_U32 memSize,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
)
{
    GT_U32 index;

    /*******************************************************************************
       Check if the code about to be downloaded can fit into the device's memory
    *******************************************************************************/
    if (codeSize > memSize)
    {
        *errCode = MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD;
        return GT_FAIL;
    }

    /* Write to program memory incrementally */
    for (index = 0; index < codeSize; index++)
    {
        MCESD_ATTEMPT(API_C28GP4X4_WriteReg(devPtr, 255, address, mvHwsComphyCodeGet(index,code)));
        address += 4;
    }

    *errCode = 0;
    return GT_OK;
}

/**
 * mvHwsComphyC28GP4X4ProgRam
 *
 * @brief download Prog Data
 *
 * @param devPtr
 * @param codePtr
 * @param codeSize
 * @param xType
 * @param progData
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsComphyC28GP4X4ProgRam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 codePtr[],
    IN MCESD_U32 codeSize,
    IN MV_HWS_COMPHY_RAM_DATA_TYPE xType,
    INOUT MV_HWS_COMPHY_PROG_RAM_DATA *progData
)
{
    MCESD_U32 actualChecksum, checksumPass;
    MCESD_FIELD ethernetCfg = FIELD_DEFINE(0xE604, 2, 0);
    MCESD_U8 retries =  10;

    CHECK_STATUS(mvHwsComphyC28GP4X4InitProgData(devPtr, codePtr, codeSize, xType, progData));

    do
    {
        MCESD_FIELD resetChecksum = FIELD_DEFINE(progData->regAddrCSPass, 0, 0);
        if (progData->xType == MV_HWS_COMPHY_FW_DATA)
        {
            /* Select MCU program memory */
            MCESD_FIELD fieldProgRamSel = F_C28GP4X4R1P0_PROG_RAM_SEL;
            MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, progData->lane, &fieldProgRamSel, 0));
        }
        /* Write XDATA to MCU */
        CHECK_STATUS(mvHwsComphyC28GP4X4UpdateRamCode(devPtr, progData->pDataPtr, progData->dataSizeDW, progData->dataMaxSize, progData->dataBaseAddr, &(progData->errCode)));

        MCESD_ATTEMPT(API_C28GP4X4_ReadReg(devPtr, progData->lane, progData->regAddrActualCheckSum, &actualChecksum));
        MCESD_ATTEMPT(API_C28GP4X4_ReadReg(devPtr, progData->lane, progData->regAddrCSPass, &checksumPass));
        if ((checksumPass & (1 << ((progData->xType == MV_HWS_COMPHY_FW_DATA || progData->xType == MV_HWS_COMPHY_CMN_DATA) ? 1 : 29))) && (actualChecksum == progData->expectCheckSum))
        {
            if (progData->xType == MV_HWS_COMPHY_LANE_DATA)
            {
                MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, progData->lane, &ethernetCfg, 0x4));
            }

            progData->errCode = 0;
            return GT_OK;
        }

        MCESD_ATTEMPT(API_C28GP4X4_WriteField(devPtr, progData->lane, &resetChecksum, 0));
        retries--;

    }while (retries != 0);

    MCESD_DBG_ERROR("mvHwsComphyC28GP4X4ProgRam Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",progData->expectCheckSum ,actualChecksum);
    progData->errCode = MCESD_IO_ERROR;
    return GT_FAIL;
}

/**
 * mvHwsComphyC28GP4X4DownloadFirmwareProgXData
 *
 * @brief Download C28GP4X4 FW Data
 *
 * @param devPtr
 * @param fwCodePtr
 * @param fwCodeSizeDW
 * @param cmnXDataPtr
 * @param cmnXDataSizeDW
 * @param laneXDataPtr
 * @param laneXDataSizeDW
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsComphyC28GP4X4DownloadFirmwareProgXData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32 *cmnXDataPtr,
    IN MCESD_U32 cmnXDataSizeDW,
    IN MCESD_U32 *laneXDataPtr,
    IN MCESD_U32 laneXDataSizeDW
)
{
    MV_HWS_COMPHY_PROG_RAM_DATA xMemData;

    if (API_C28GP4X4_SetMcuBroadcast(devPtr, GT_TRUE) == GT_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X4_SetMcuBroadcast: Failed\n");
        return GT_FAIL;
    }

    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DIRECT_ACCESS_EN, 1));
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PRAM_SOC_EN, 1));
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 10));

    /* Download MCU Firmware */
    if (mvHwsComphyC28GP4X4ProgRam(devPtr, fwCodePtr, fwCodeSizeDW, MV_HWS_COMPHY_FW_DATA, &xMemData) == GT_FAIL)
    {
        MCESD_DBG_ERROR("mvHwsComphyC28GP4X4ProgRam: Failed firmware download with error code: 0x%X\n", xMemData.errCode);
        return GT_FAIL;
    }

    /* Download common XDATA */
    if (mvHwsComphyC28GP4X4ProgRam(devPtr, cmnXDataPtr, cmnXDataSizeDW, MV_HWS_COMPHY_CMN_DATA, &xMemData) == GT_FAIL)
    {
        MCESD_DBG_ERROR("mvHwsComphyC28GP4X4ProgRam: Failed common XDATA download with error code: 0x%X\n", xMemData.errCode);
        return GT_FAIL;
    }

    /* Download lane XDATA - will be broadcasted to all lanes */
    if (mvHwsComphyC28GP4X4ProgRam(devPtr, laneXDataPtr, laneXDataSizeDW, MV_HWS_COMPHY_LANE_DATA, &xMemData) == GT_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X4_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", xMemData.errCode);
        return GT_FAIL;
    }

    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_PRAM_SOC_EN, 0));
    MCESD_ATTEMPT(API_C28GP4X4_HwSetPinCfg(devPtr, C28GP4X4_PIN_DIRECT_ACCESS_EN, 0));
    MCESD_ATTEMPT(API_C28GP4X4_Wait(devPtr, 10));

    if (API_C28GP4X4_SetMcuBroadcast(devPtr, GT_FALSE) == GT_FAIL)
    {
        MCESD_DBG_ERROR("API_C28GP4X4_SetMcuBroadcast: Failed\n");
        return GT_FAIL;
    }

    /* Enable MCU */
    MCESD_ATTEMPT(API_C28GP4X4_SetLaneEnable(devPtr, 255, GT_TRUE));
    MCESD_ATTEMPT(API_C28GP4X4_SetMcuEnable(devPtr, 255, GT_TRUE));
    return GT_OK;
}
#endif

/**
* @internal mvHwsComphyC28GP4X4SerdesFirmwareDownloadIter
*           function
* @endinternal
*
* @brief   Download FW
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - num of serdeses in serdesArr
* @param[in] fwCodePtr         - firmware buffer
* @param[in] fwCodeSize        - firmware size
* @param[in] cmnXDataPtr       - cmnXData bufer
* @param[in] cmnXDataSize      - cmnXData size
* @param[in] laneXDataPtr      - laneXData buffer
* @param[in] laneXDataSize     - laneXData size
* @param[in] refClkSel         - Serdes reference clock select
* @param[in] refFreq           - SerDes reference clock frquency
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesFirmwareDownloadIter
(
    GT_U8                    devNum,
    GT_UOPT                  *serdesArr,
    GT_U8                    numOfSerdeses,
    MCESD_U32                *fwCodePtr,
    MCESD_U32                fwCodeSize,
    MCESD_U32                *cmnXDataPtr,
    MCESD_U32                cmnXDataSize,
    MCESD_U32                *laneXDataPtr,
    MCESD_U32                laneXDataSize,
    E_C28GP4X4_REFCLK_SEL    refClkSel,
    E_C28GP4X4_REFFREQ       refFreq
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U8 serdesIterator;

    for(serdesIterator = 0; serdesIterator < numOfSerdeses; serdesIterator++)
    {
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesArr[serdesIterator], &serdesInfo, NULL, &sDev));
        if(serdesInfo->internalLane != 0)
        {
            return GT_BAD_PARAM;
        }
        /* Set SERDES Mode */
        MCESD_CHECK_STATUS(API_C28GP4X4_SetPhyMode(sDev, C28GP4X4_PHYMODE_SERDES));
        /* Configure Reference Frequency and reference clock source group */
        MCESD_CHECK_STATUS(API_C28GP4X4_SetRefFreq(sDev, refFreq, refClkSel));
        /* Power up current and voltage reference */
        MCESD_CHECK_STATUS(API_C28GP4X4_SetPowerIvRef(sDev, MCESD_TRUE));
        /* Download MCU Firmware */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        MCESD_CHECK_STATUS(API_C28GP4X4_DownloadFirmwareProgXData(sDev,
                                          fwCodePtr,    fwCodeSize,
                                          cmnXDataPtr,  cmnXDataSize,
                                          laneXDataPtr, laneXDataSize));
#else
        MCESD_CHECK_STATUS(mvHwsComphyC28GP4X4DownloadFirmwareProgXData(sDev,
                                          fwCodePtr,    fwCodeSize,
                                          cmnXDataPtr,  cmnXDataSize,
                                          laneXDataPtr, laneXDataSize));
#endif
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesFirmwareDownload function
* @endinternal
*
* @brief   Download FW
*
* @param[in] devNum            - Device Number
* @param[in] portGroup         - Port Group
* @param[in] serdesArr         - array of serdes numbers
* @param[in] numOfSerdeses     - num of serdeses in serdesArr
* @param[in] firmwareSelector  - FW to download
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesFirmwareDownload
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     *serdesArr,
    GT_U8       numOfSerdeses,
    GT_U8       firmwareSelector
)
{
    MCESD_U16               errCode;
    GT_STATUS               rc = GT_OK;

    E_C28GP4X4_REFCLK_SEL   refClkSel = C28GP4X4_REFCLK_SEL_GROUP1;
    E_C28GP4X4_REFFREQ      refFreq;
    GT_BOOL                 freeNeeded = GT_FALSE;

    MCESD_U32 *fwCodePtr;
    MCESD_U32 fwCodeSize;

    MCESD_U32 *cmnXDataPtr;
    MCESD_U32 cmnXDataSize;

    MCESD_U32 *laneXDataPtr;
    MCESD_U32 laneXDataSize;

    GT_UNUSED_PARAM(portGroup);
    switch(firmwareSelector)
    {
        case MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE:
        case MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE:
            fwCodePtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C28GP4X4_FW_MAX_SIZE);
            if ( fwCodePtr == NULL )
            {
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//C28GP4X4//main.dat",
                                        fwCodePtr,       C28GP4X4_FW_MAX_SIZE,         &fwCodeSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }

            cmnXDataPtr  = (MCESD_U32 *)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C28GP4X4_XDATA_CMN_MAX_SIZE);
            if ( cmnXDataPtr == NULL )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }
            if(firmwareSelector == MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE)
            {
                refFreq = C28GP4X4_REFFREQ_156P25MHZ;
                if ( LoadFwDataFileToBuffer("serdes_fw//C28GP4X4//cmn_156.dat",
                                            cmnXDataPtr,     C28GP4X4_XDATA_CMN_MAX_SIZE,  &cmnXDataSize, &errCode) != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(fwCodePtr);
                    hwsOsFreeFuncPtr(cmnXDataPtr);
                    return GT_FAIL;
                }
            }
            else
            {
                refFreq = C28GP4X4_REFFREQ_25MHZ;
                if ( LoadFwDataFileToBuffer("serdes_fw//C28GP4X4//cmn_25.dat",
                                            cmnXDataPtr,     C28GP4X4_XDATA_CMN_MAX_SIZE,  &cmnXDataSize, &errCode) != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(fwCodePtr);
                    hwsOsFreeFuncPtr(cmnXDataPtr);
                    return GT_FAIL;
                }
            }

            laneXDataPtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C28GP4X4_XDATA_LANE_MAX_SIZE);
            if ( laneXDataPtr == NULL )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                hwsOsFreeFuncPtr(cmnXDataPtr);
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//C28GP4X4//lane.dat",
                                        laneXDataPtr,    C28GP4X4_XDATA_LANE_MAX_SIZE, &laneXDataSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                hwsOsFreeFuncPtr(cmnXDataPtr);
                hwsOsFreeFuncPtr(laneXDataPtr);
                return GT_FAIL;
            }

            freeNeeded = GT_TRUE;
            break;
        case MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE:
            fwCodePtr     = (MCESD_U32*)C28GP4X4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_C28GP4X4_MAIN_DATA_IMAGE_SIZE;

            cmnXDataPtr   = (MCESD_U32*)C28GP4X4_SERDES_REF25MHz_CMN_Data;
            cmnXDataSize  = (MCESD_U32)COMPHY_C28GP4X4_SERDES_REF25MHZ_CMN_DATA_IMAGE_SIZE;

            laneXDataPtr  = (MCESD_U32*)C28GP4X4_SERDES_SPDCHG2_LANE_Data;
            laneXDataSize = (MCESD_U32)COMPHY_C28GP4X4_SERDES_SPDCHG2_LANE_DATA_IMAGE_SIZE;

            refFreq       = C28GP4X4_REFFREQ_25MHZ;
            break;
        case MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE:
            fwCodePtr     = (MCESD_U32*)C28GP4X4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_C28GP4X4_MAIN_DATA_IMAGE_SIZE;

            cmnXDataPtr   = (MCESD_U32*)C28GP4X4_SERDES_REF156MHz_CMN_Data;
            cmnXDataSize  = (MCESD_U32)COMPHY_C28GP4X4_SERDES_REF156MHZ_CMN_DATA_IMAGE_SIZE;

            laneXDataPtr  = (MCESD_U32*)C28GP4X4_SERDES_SPDCHG2_LANE_Data;
            laneXDataSize = (MCESD_U32)COMPHY_C28GP4X4_SERDES_SPDCHG2_LANE_DATA_IMAGE_SIZE;

            refFreq       = C28GP4X4_REFFREQ_156P25MHZ;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

    comphyPrintf("--fwCodeSize[%d]-cmnXDataSize[%d]-laneXDataSize[%d]--\n",fwCodeSize, cmnXDataSize,laneXDataSize);
    if ( freeNeeded )
    {
        hwsOsPrintf("Using SerDes FW from file: fwCodeSize=[%d], cmnXDataSize=[%d], laneXDataSize=[%d]\n",fwCodeSize, cmnXDataSize,laneXDataSize);
    }
    rc = mvHwsComphyC28GP4X4SerdesFirmwareDownloadIter(devNum, serdesArr, numOfSerdeses,
                                                       fwCodePtr, fwCodeSize,
                                                       cmnXDataPtr, cmnXDataSize,
                                                       laneXDataPtr, laneXDataSize,
                                                       refClkSel, refFreq);

    if ( freeNeeded )
    {
        hwsOsFreeFuncPtr(fwCodePtr);
        hwsOsFreeFuncPtr(cmnXDataPtr);
        hwsOsFreeFuncPtr(laneXDataPtr);

    }

    return rc;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesTemperatureGet function
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
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesTemperatureGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  serdesNum,
    IN  GT_UOPT  portGroup,
    OUT GT_32    *serdesTemperature
)
{
    MCESD_DEV_PTR   sDev;
    MCESD_32        temp;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTemperature(sDev, &temp));
    *serdesTemperature = (GT_32)temp;

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4AnpPowerUp function
* @endinternal
*
* @brief   Prepare Comphy Serdes for ANP operation.
*
* @param[in] devNum    - system device number
* @param[in] portGroup - port group (core) number
* @param[in] serdesNum - SerDes number to power up/down
* @param[in] powerUp   - power up status
* @param[in] serdesConfigPtr - pointer to Serdes params struct
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC28GP4X4AnpPowerUp
(
    IN GT_U8    devNum,
    IN GT_UOPT  portGroup,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  powerUp,
    IN MV_HWS_SERDES_CONFIG_STC *serdesConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR   serdesInfo;
    MCESD_DEV_PTR                sDev;
    MCESD_U8                     lane;
    GT_U32 regData = (powerUp == GT_TRUE) ? 1 : 0;

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesConfigPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_SetLaneEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)powerUp));
    MCESD_CHECK_STATUS(API_C28GP4X4_SetDataBusWidth(sDev, serdesInfo->internalLane, C28GP4X4_DATABUS_40BIT, C28GP4X4_DATABUS_40BIT));

    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2170, (0xFFFF << 16), 0x3FF << 16));
    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2170, (1<<12), (regData<<12)));

    /* Enable MCU on all lanes */
    for (lane = 0; lane < C28GP4X4_TOTAL_LANES; lane++)
    {
        MCESD_ATTEMPT(API_C28GP4X4_SetMcuEnable(sDev, lane, MCESD_TRUE));
    }

    return GT_OK;
}

extern GT_U32 mvHwsComphyC28GPSelectedFields0[MV_HWS_COMPHY_C28G_SELECTED_FIELDS_ARR_SIZE][3];
/**
* @internal mvHwsComphyC28GP4X4SerdesDumpInfo function
* @endinternal
*
* @brief   Dump SerDes info
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - SerDes number to power up/down
* @param[in]  dumpType  - dump type
* @param[in]  printBool - print to terminal
* @param[out] outputPtr - generic pointer to output data
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesDumpInfo
(
    IN  GT_U8                           devNum,
    IN  GT_UOPT                         portGroup,
    IN  GT_UOPT                         serdesNum,
    IN  MV_HWS_SERDES_DUMP_TYPE_ENT     dumpType,
    IN  GT_BOOL                         printBool,
    OUT GT_U32                          *outputPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      i;
    GT_U32                      pinVal;
    GT_U32                      regVal;
    const GT_CHAR        *pinsList[] =
    {
        "C28GP4X4_PIN_RESET",
        "C28GP4X4_PIN_ISOLATION_ENB",
        "C28GP4X4_PIN_BG_RDY",
        "C28GP4X4_PIN_SIF_SEL",
        "C28GP4X4_PIN_MCU_CLK",
        "C28GP4X4_PIN_DIRECT_ACCESS_EN",
        "C28GP4X4_PIN_PHY_MODE",
        "C28GP4X4_PIN_REFCLK_SEL",
        "C28GP4X4_PIN_REF_FREF_SEL",
        "C28GP4X4_PIN_PHY_GEN_TX0",
        "C28GP4X4_PIN_PHY_GEN_TX1",
        "C28GP4X4_PIN_PHY_GEN_TX2",
        "C28GP4X4_PIN_PHY_GEN_TX3",
        "C28GP4X4_PIN_PHY_GEN_RX0",
        "C28GP4X4_PIN_PHY_GEN_RX1",
        "C28GP4X4_PIN_PHY_GEN_RX2",
        "C28GP4X4_PIN_PHY_GEN_RX3",
        "C28GP4X4_PIN_DFE_EN0",
        "C28GP4X4_PIN_DFE_EN1",
        "C28GP4X4_PIN_DFE_EN2",
        "C28GP4X4_PIN_DFE_EN3",
        "C28GP4X4_PIN_PU_PLL0",
        "C28GP4X4_PIN_PU_PLL1",
        "C28GP4X4_PIN_PU_PLL2",
        "C28GP4X4_PIN_PU_PLL3",
        "C28GP4X4_PIN_PU_RX0",
        "C28GP4X4_PIN_PU_RX1",
        "C28GP4X4_PIN_PU_RX2",
        "C28GP4X4_PIN_PU_RX3",
        "C28GP4X4_PIN_PU_TX0",
        "C28GP4X4_PIN_PU_TX1",
        "C28GP4X4_PIN_PU_TX2",
        "C28GP4X4_PIN_PU_TX3",
        "C28GP4X4_PIN_TX_IDLE0",
        "C28GP4X4_PIN_TX_IDLE1",
        "C28GP4X4_PIN_TX_IDLE2",
        "C28GP4X4_PIN_TX_IDLE3",
        "C28GP4X4_PIN_PU_IVREF",
        "C28GP4X4_PIN_RX_TRAIN_ENABLE0",
        "C28GP4X4_PIN_RX_TRAIN_ENABLE1",
        "C28GP4X4_PIN_RX_TRAIN_ENABLE2",
        "C28GP4X4_PIN_RX_TRAIN_ENABLE3",
        "C28GP4X4_PIN_RX_TRAIN_COMPLETE0",
        "C28GP4X4_PIN_RX_TRAIN_COMPLETE1",
        "C28GP4X4_PIN_RX_TRAIN_COMPLETE2",
        "C28GP4X4_PIN_RX_TRAIN_COMPLETE3",
        "C28GP4X4_PIN_RX_TRAIN_FAILED0",
        "C28GP4X4_PIN_RX_TRAIN_FAILED1",
        "C28GP4X4_PIN_RX_TRAIN_FAILED2",
        "C28GP4X4_PIN_RX_TRAIN_FAILED3",
        "C28GP4X4_PIN_TX_TRAIN_ENABLE0",
        "C28GP4X4_PIN_TX_TRAIN_ENABLE1",
        "C28GP4X4_PIN_TX_TRAIN_ENABLE2",
        "C28GP4X4_PIN_TX_TRAIN_ENABLE3",
        "C28GP4X4_PIN_TX_TRAIN_COMPLETE0",
        "C28GP4X4_PIN_TX_TRAIN_COMPLETE1",
        "C28GP4X4_PIN_TX_TRAIN_COMPLETE2",
        "C28GP4X4_PIN_TX_TRAIN_COMPLETE3",
        "C28GP4X4_PIN_TX_TRAIN_FAILED0",
        "C28GP4X4_PIN_TX_TRAIN_FAILED1",
        "C28GP4X4_PIN_TX_TRAIN_FAILED2",
        "C28GP4X4_PIN_TX_TRAIN_FAILED3",
        "C28GP4X4_PIN_SQ_DETECTED_LPF0",
        "C28GP4X4_PIN_SQ_DETECTED_LPF1",
        "C28GP4X4_PIN_SQ_DETECTED_LPF2",
        "C28GP4X4_PIN_SQ_DETECTED_LPF3",
        "C28GP4X4_PIN_RX_INIT0",
        "C28GP4X4_PIN_RX_INIT1",
        "C28GP4X4_PIN_RX_INIT2",
        "C28GP4X4_PIN_RX_INIT3",
        "C28GP4X4_PIN_RX_INIT_DONE0",
        "C28GP4X4_PIN_RX_INIT_DONE1",
        "C28GP4X4_PIN_RX_INIT_DONE2",
        "C28GP4X4_PIN_RX_INIT_DONE3",
        "C28GP4X4_PIN_PRAM_SOC_EN",
        "C28GP4X4_PIN_DFE_PAT_DIS0",
        "C28GP4X4_PIN_DFE_PAT_DIS1",
        "C28GP4X4_PIN_DFE_PAT_DIS2",
        "C28GP4X4_PIN_DFE_PAT_DIS3"
    };

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(outputPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    osPrintfIf(printBool,"Dump SerDes Number[%d] information\n");
    osPrintfIf(printBool,"SerDes Type: COM_PHY_C28GP4X4\n");

    switch(dumpType)
    {
        case SERDES_DUMP_INTERNAL_REGISTERS:
            osPrintfIf(printBool,"Dump type - Internal SerDes registers:\n");
            for(i = 0 ; i <= 0xE6EC ; i = i+0x4)
            {
                CHECK_STATUS(mvHwsComphySerdesRegisterRead(devNum, portGroup,serdesNum,i,0xffffffff,&regVal));
                osPrintfIf(printBool,"Address: 0x%04x, Data: 0x%08x\n", i, regVal);
                if(NULL != outputPtr) outputPtr[i] = regVal;
            }
            break;
        case SERDES_DUMP_PINS:
            osPrintfIf(printBool,"Dump type - SerDes external pins:\n");
            for(i = 0 ; i <= (GT_U32)C28GP4X4_PIN_DFE_PAT_DIS3 ; i++)
            {
                mvHwsComphySerdesPinGet(devNum, portGroup, serdesNum, (E_C28GP4X4_PIN)i, &pinVal);
                osPrintfIf(printBool,"(%3d) %#-40s %3d\n", i, pinsList[i],pinVal);
                if(NULL != outputPtr) outputPtr[i] = pinVal;
            }
            break;
        case SERDES_DUMP_SELECTED_FIELDS_0:
            osPrintfIf(printBool,"Dump type - SerDes fields dumped into struct only\n");
            for(i = 0 ; i < MV_HWS_COMPHY_C28G_SELECTED_FIELDS_ARR_SIZE ; i++)
            {
                CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum,
                                                                mvHwsComphyC28GPSelectedFields0[i][0],
                                                                mvHwsComphyC28GPSelectedFields0[i][1],
                                                                mvHwsComphyC28GPSelectedFields0[i][2], &regVal));
                if(NULL != outputPtr) outputPtr[i] = regVal;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesRxPllLockGet function
* @endinternal
*
* @brief   Returns whether or not the RX PLL is frequency locked
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] laneNum                  - lane number
*
* @param[out] lockPtr                 - (pointer to) lock state
*                                        GT_TRUE =  locked
*                                        GT_FALSE = not locked
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesRxPllLockGet
(
    GT_U8                 devNum,
    GT_U32                serdesNum,
    GT_BOOL               *lockPtr
)
{
    GT_U32 data;

    if (NULL == lockPtr)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, 0, EXTERNAL_REG, serdesNum, SDW_LANE_STATUS_0, &data, PLL_INIT_POLLING_DONE));

    if (data == PLL_INIT_POLLING_DONE)
    {
        *lockPtr = GT_TRUE;
    }
    else
    {
         *lockPtr = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesDroGet function
* @endinternal
*
* @brief   Get DRO (Device Ring Oscillator).
*           Indicates performance of device
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - SerDes number
*
* @param[out] dro                     - (pointer to)dro
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesDroGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          serdesNum,
    OUT GT_U16                          *dro
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    S_C28GP4X4_DRO_DATA freq;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetDRO(sDev,&freq));
    hwsOsMemCopyFuncPtr(dro, freq.dro, sizeof(S_C28GP4X4_DRO_DATA));

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsComphyC28GP4X4EyeMonitor function
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
GT_STATUS mvHwsComphyC28GP4X4SerdesEomMatrixGet
(
    IN   GT_U8                              devNum,
    IN   GT_U32                             serdesNum,
    IN   GT_BOOL                            printEnable,
    IN   MV_HWS_SERDES_EOM_IN_PARAMS_UNT    *eomInParams,
    OUT  MV_HWS_SERDES_EOM_OUT_PARAMS_UNT   *eomOutParams
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR serdesInfo;
    MCESD_STATUS    mcesdSt;
    S_C28GP4X4_EYE_RAW *mcesdEyeRawData;
    MCESD_U16 widthLeft, widthRight, heightUpper, heightLower;
    MCESD_U32 sampleCount, widthmUI, heightmVUpper, heightmVLower;
    MCESD_BOOL dfeState;

    if ( NULL == eomInParams|| NULL == eomOutParams)
    {
        return GT_BAD_PTR;
    }

    if (eomInParams->comphyC28GP4EomInParams.phaseLevels  < 1 || eomInParams->comphyC28GP4EomInParams.phaseLevels  > 16 ||
        eomInParams->comphyC28GP4EomInParams.voltageSteps < 1 || eomInParams->comphyC28GP4EomInParams.voltageSteps > 16)
    {
        return GT_OUT_OF_RANGE;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* for low baud rates, need to enable DFE */
    MCESD_CHECK_STATUS(API_C28GP4X4_GetDfeEnable(sDev, serdesInfo->internalLane, &dfeState));
    if (dfeState == MCESD_FALSE)
    {
        API_C28GP4X4_SetDfeEnable(sDev, serdesInfo->internalLane, MCESD_TRUE);
    }

    mcesdEyeRawData = (S_C28GP4X4_EYE_RAW*)hwsOsMallocFuncPtr(sizeof(S_C28GP4X4_EYE_RAW));
    if ( NULL == mcesdEyeRawData )
    {
        return GT_NO_RESOURCE;
    }

    mcesdSt = API_C28GP4X4_EOMGetEyeData(sDev, serdesInfo->internalLane, (MCESD_U32) eomInParams->comphyC28GP4EomInParams.minSamples, (MCESD_U32)eomInParams->comphyC28GP4EomInParams.berThreshold, mcesdEyeRawData);
    if ( mcesdSt != MCESD_OK )
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_FAIL;
    }

    /* retrieve height and width in code word and convert to mV and pSec */
    mcesdSt = API_C28GP4X4_EOMGetWidthHeight(sDev, serdesInfo->internalLane, (MCESD_U32)eomInParams->comphyC28GP4EomInParams.minSamples, (MCESD_U32)eomInParams->comphyC28GP4EomInParams.berThreshold,
                                             &widthLeft, &widthRight, &heightUpper, &heightLower, &sampleCount);
    if ( mcesdSt != MCESD_OK )
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_FAIL;
    }
    mcesdSt = API_C28GP4X4_EOMConvertWidthHeight(sDev, serdesInfo->internalLane, (widthLeft + widthRight), heightUpper, heightLower, &widthmUI, &heightmVUpper, &heightmVLower);
    if ( mcesdSt != MCESD_OK )
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_FAIL;
    }

    eomOutParams->comphyC28GP4EomOutParams.width_mUI   = (GT_U32)widthmUI;
    eomOutParams->comphyC28GP4EomOutParams.height_mV   = (GT_U32)(heightmVUpper + heightmVLower);
    eomOutParams->comphyC28GP4EomOutParams.sampleCount = (GT_U32)sampleCount;

    if ( GT_TRUE == printEnable )
    {
        mcesdSt = API_C28GP4X4_EOMPlotEyeData(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC28GP4EomInParams.phaseLevels, (MCESD_U32)eomInParams->comphyC28GP4EomInParams.voltageSteps,
                                              (MCESD_U32)eomInParams->comphyC28GP4EomInParams.berThreshold, (MCESD_U32)eomInParams->comphyC28GP4EomInParams.berThresholdMax);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
    }

    /* restore DFE for low baud rates */
    if (dfeState == MCESD_FALSE)
    {
        API_C28GP4X4_SetDfeEnable(sDev, serdesInfo->internalLane, MCESD_FALSE);
    }

    hwsOsFreeFuncPtr(mcesdEyeRawData);

    return GT_OK;
}

GT_STATUS mvHwsComphyC28GP4X4SerdesRxInit
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
    MCESD_U32                   completed, rxInitDone;
    GT_U32                      data = 0;

    *rxInitStatus = TUNE_PASS;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

    /* skip if serdes loopback analog tx2rx */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, &data, (1 << 25)));
    if ( data != 0 )
    {
        return GT_OK;
    }

    /* check signal */
    MCESD_CHECK_STATUS(mvHwsComphyC28GP4X4SerdesSignalDetectGet(devNum, portGroup, serdesNum, &sigDetect));
    if (sigDetect == GT_TRUE)
    {
        MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, (C28GP4X4_PIN_RX_TRAIN_ENABLE0 + serdesInfo->internalLane), &completed));
        MCESD_CHECK_STATUS(API_C28GP4X4_HwGetPinCfg(sDev, (C28GP4X4_PIN_RX_INIT_DONE0 + serdesInfo->internalLane), &rxInitDone));
        if ((completed == 1) || (rxInitDone == 1))
            return GT_OK;

        switch (rxInitMode)
        {
        case RX_INIT_BLOCKING:
            MCESD_CHECK_STATUS(API_C28GP4X4_RxInit(sDev, serdesInfo->internalLane));
            break;
        default:
            return GT_NOT_SUPPORTED;
        }
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4SerdesSignalDetectMaskSet function
* @endinternal
*
* @brief   Set signal detect mask
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number
* @param[in]  maskEnable  - if true - enable Tx
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyC28GP4X4SerdesSignalDetectMaskSet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL maskEnable
)
{
    MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2110, 9, 1, (maskEnable==GT_TRUE?1:0)));
    return GT_OK;
}

#endif /* MV_HWS_REDUCED_BUILD */

/**
* @internal
*           mvHwsComphyC28GP4X4SerdesPresetOverrideSet function
* @endinternal
*
* @brief   Debug hooks APIs : SetTxLocalPreset, SetRemotePreset, SetCTLEPreset
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] serdesPresetOverride     - override preset default paramters
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesPresetOverrideSet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    IN  MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverride
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR   sDev;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.txLocalPreset )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetTxLocalPreset(sDev, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.txLocalPreset));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.remotePresetRequestType )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetRemotePreset(sDev, serdesInfo->internalLane, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.remotePresetRequestType));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetDefault )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_DEFAULT, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetDefault));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes1 )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_RES1, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes1));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap1 )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_CAP1, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap1));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes2_O )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_RES2_O, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes2_O));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes2_E )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_RES2_E, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes2_E));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap2_O )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_CAP2_O, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap2_O));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap2_E )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_CAP2_E, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap2_E));

    if ( NA_16BIT != serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetPhase )
        MCESD_CHECK_STATUS(API_C28GP4X4_SetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_PHASE, (MCESD_U32)serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetPhase));

    return GT_OK;
}

/**
* @internal
*           mvHwsComphyC28GP4X4SerdesPresetOverrideGet function
* @endinternal
*
* @brief   Debug hooks APIs : GetTxLocalPreset, GetRemotePreset, GetCTLEPreset
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] operation                - debug API to use
* @param[in] ctlePresetParam          - relevant for CTLE_PRESET
*
* @param[out] val                     - value to set
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC28GP4X4SerdesPresetOverrideGet
(
    IN  GT_U8                                devNum,
    IN  GT_U32                               serdesNum,
    OUT MV_HWS_SERDES_PRESET_OVERRIDE_UNT    *serdesPresetOverride
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR   sDev;
    MCESD_U32 val;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C28GP4X4_GetTxLocalPreset(sDev, &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.txLocalPreset = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetRemotePreset(sDev, serdesInfo->internalLane, &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.remotePresetRequestType = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_DEFAULT, &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetDefault  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_RES1   , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes1  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_CAP1   , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap1  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_RES2_O , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes2_O  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_RES2_E , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetRes2_E  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_CAP2_O , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap2_O  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_CAP2_E , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetCap2_E  = (GT_U32)val;

    MCESD_CHECK_STATUS(API_C28GP4X4_GetCTLEPreset(sDev, serdesInfo->internalLane, C28GP4X4_CTLE_PRESET_PHASE  , &val));
    serdesPresetOverride->comphyC28GP4PresetOverride.localCtlePresetPhase  = (GT_U32)val;

    return GT_OK;
}

/**
* @internal mvHwsComphyC28GP4X4IfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC28GP4X4IfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_C28GP4X4])
    {
        funcPtrArray[COM_PHY_C28GP4X4] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_C28GP4X4])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_C28GP4X4], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }

    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc             ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc                  ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc             ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc             ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc               ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc              ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc              ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc         ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc                       ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset                    ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc                     ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc                  ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc               ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc              ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc                    ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc                ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc             ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesFirmwareDownloadFunc            ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesFirmwareDownload);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTemperatureFunc                 ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesTemperatureGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc                  ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc                  ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesDbRxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDumpInfoFunc                    ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesDumpInfo);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc                ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4AnpPowerUp);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxPllLockGetFunc                ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesRxPllLockGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDroGetFunc                      ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesDroGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPresetOverrideSetFunc           ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesPresetOverrideSet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPresetOverrideGetFunc           ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesPresetOverrideGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc            ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesCdrLockStatusGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                        ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesOperation);
#ifndef MV_HWS_REDUCED_BUILD
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEomMatrixGetFunc                ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesEomMatrixGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxInitFunc                      ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesRxInit);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSigDetectMaskFunc               ,COM_PHY_C28GP4X4, mvHwsComphyC28GP4X4SerdesSignalDetectMaskSet);
#endif

    return GT_OK;
}

#endif /* C28GP4X4 */

