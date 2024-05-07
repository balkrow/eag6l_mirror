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
* @file mvComphyN5XC56GP5X4SerdesIf.c \
*
* @brief Comphy interface
*
* @version   1
********************************************************************************
*/

#ifndef MICRO_INIT
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#ifdef N5XC56GP5X4
#include "mcesdN5XC56GP5X4_Defs.h"
#include "mcesdN5XC56GP5X4_API.h"
#include "mcesdN5XC56GP5X4_DeviceInit.h"
#include "mcesdN5XC56GP5X4_HwCntl.h"
#include "mcesdN5XC56GP5X4_RegRW.h"
#include "mcesdN5XC56GP5X4_FwDownload.h"
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/N5XC56GP5X4_main.h>
#endif

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
GT_U32 *N5XC56GP5X4_main_DataPtr;
GT_U32  N5XC56GP5X4_Image_Size;
#endif

#ifndef MV_HWS_REDUCED_BUILD
#define ESTIMATE_EYE_DIM_COUNT 4
#endif

GT_STATUS mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           serdesNum,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
);

static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableSet
(
    IN GT_U8    devNum,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  enable
);

static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableSet
(
    IN GT_U8    devNum,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  enable
);

static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesResetDfe
(
    GT_U8                                  devNum,
    GT_U32                                 portGroup,
    GT_U32                                 serdesNum
)
{
    MCESD_DEV_PTR sDev;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x401c,(0x1<<2),(0x1<<2)));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 1));
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x401c,(0x1<<2),0));

    return GT_OK;
}

static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAdjustClampingSettings
(
    GT_U8                                  devNum,
    GT_U32                                 portGroup,
    GT_U32                                 serdesNum,
    GT_BOOL                                enable
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    E_N5XC56GP5X4_SERDES_SPEED   serdesSpeedBaudRate;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_ATTEMPT(API_N5XC56GP5X4_GetTxRxBitRate(sDev, serdesInfo->internalLane, &serdesSpeedBaudRate, &serdesSpeedBaudRate));
    if (serdesSpeedBaudRate == N5XC56GP5X4_SERDES_1P25G)
    {
        if (enable)
        {
            /* DTL_CLAMPING_DIV16_EN_LANE = 1 to enable better clamping */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x3260,9, 1, 0x1));
            /* DTL_CLK_MODE = 0*/
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x3268,14, 2, 0x0));
            /* DTL CLAMPING scale = 1 */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x3260,23, 1, 0x1));
        }
        else
        {
            /* DTL_CLAMPING_DIV16_EN_LANE = 0 to disable clamping */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x3260,9, 1, 0x0));
            /* DTL_CLK_MODE = 2*/
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x3268,14, 2, 0x2));
            /* DTL CLAMPING scale = 0 */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x3260,23, 1, 0x0));
        }
    }

    return GT_OK;
}


static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesOperation
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
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_U32                   cap1, cap2, res1, syncE;
    MCESD_U16                   trainingResult;
    MCESD_16                    align90;


    GT_UNUSED_PARAM(portMode);
    GT_UNUSED_PARAM(dataPtr);
    GT_UNUSED_PARAM(resultPtr);
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
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesResetDfe(devNum, portGroup, serdesNum));
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesAdjustClampingSettings(devNum, portGroup, serdesNum, GT_TRUE));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_CHECK_CLAMPING_TRIGGERED_E:
        /* read dtl_clamping_triggered_lane */
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x3260,29,1,resultPtr));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_DEBUG_MODE_E:
        /* data[0] : 0 = NO_DEBUG_STATE, 1 = DEBUG_STATE */
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableSet(devNum, serdesNum, (dataPtr[0] == 0) ? GT_TRUE : GT_FALSE));
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableSet(devNum, serdesNum, (dataPtr[0] == 1) ? GT_TRUE : GT_FALSE));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_CDR_E:

        /* read cap1 and cap2*/
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL, &cap1));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL, &cap2));
        if ((cap1 != 0) && (cap2 != 0))
        {
            cap1 = cap1 / 2;       /* cap1 will be 0x0F ,0x7 , 0x3 ,0x1 ,0*/
            cap2 = (2 * cap2) / 3; /* cap2 will be 0x0F ,0xA , 0x6 ,0x4 ,0x2, 1,0*/
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL, cap1));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL, cap2));
        }
        else
        {
            /* write back default values of cap1 and cap2 */
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL, 0xF));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL, 0xF));

            /* tune res1 and back to default */
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES1_SEL, &res1));
            res1 = (res1 < 0xF) ? (res1 + 1) : 0x5;
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES1_SEL, res1));

            /* tune align90 from 60 to -180 in step of 60 */
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetAlign90(sDev, serdesInfo->internalLane, (MCESD_U16*)&align90, &trainingResult));
            align90 = (align90 >= 0) ? (align90 - 60) : 240;
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetAlign90(sDev, serdesInfo->internalLane, align90));
        }

        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesResetDfe(devNum, portGroup, serdesNum));
        break;
    case MV_HWS_PORT_SERDES_OPERATION_CDR_VALUES_SET_E:
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFI, dataPtr[0]));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFF, dataPtr[1]));
        break;

    case MV_HWS_PORT_SERDES_OPERATION_SYNC_E_CHECK_STATUS_E:
        /* dataPtr[0] is represent the state of the port
           dataPtr[0] == 1 is on creating port
           dataPtr[0] == 0 is on deleting port
           ------------------------------------
           dataPtr[1] is represent if the port with RX2TX LP or not
           dataPtr[1] == 1 port with RX2TX
           dataPtr[1] == 0 port Without RX2TX
           ------------------------------------
           dataPtr[2] is represent the num of active lanes */

        /* read sync E status */
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0xE6A0,8,1,&syncE));

        /* if it's create or delete */
        if ((dataPtr[0] == 1) || (dataPtr[0] == 0))
        {
            MCESD_U32       resultsStatus = 0;
            GT_U8           i;
            GT_U32          firstSerdesInGroup;
            GT_U32          tmpData;

            firstSerdesInGroup = (serdesNum -(serdesNum % 4));

            /* checking the 3 others lanes if they are up */
            for( i = 0; i < 4; i++)
            {
                CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, (firstSerdesInGroup + i), 0x5500, 7, 1, &tmpData));
                resultsStatus += tmpData;
            }

            /* on delete, since it's count it self before it's deleteing itself, reduced the num of active lanes for the results */
            if (dataPtr[0] == 0)
            {
                resultsStatus -= dataPtr[2];
            }

            if (resultsStatus == 0) /* no lanes are up, sync e change can be happen */
            {
                /*if port is RX2TX and no other lanes cfg, set sync e according to the state*/
                if (dataPtr[1] == 1)
                {
                    if (dataPtr[0] == 1) /* create */
                    {
                        /* disable sync E */
                        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xE6A0, 8, 1, 0x0));
                    }
                    else/* delete */
                    {
                        /* enable sync E */
                        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0xE6A0, 8, 1, 0x1));
                    }
                }
            }
            else /* other lane is already configured*/
            {
                if (dataPtr[0] == 1)/* create */
                {
                    /* check that the other configured lanes are the same as this one, if not this is not valid case */
                    if (((dataPtr[1] == 1) && (syncE == 1)) || /* trying to cfg RX2TX when non-RX2TX cfg already*/
                         ((dataPtr[1] == 0) && (syncE == 0))) /* trying to cfg non RX2TX when RX2TX cfg already*/
                    {
                        *resultPtr = 1;

                        return GT_NOT_SUPPORTED;
                    }
                }
            }

            *resultPtr = 0;
        }
        else /*invalid input for the state, return error*/
        {
            *resultPtr = 1;
            return GT_BAD_PARAM;
        }

        break;

    default:
        return GT_OK;
    }

    return GT_OK;
}

#if !defined(HARRIER_DEV_SUPPORT)
/**
* @internal mvHwsComphyN5XC56GP5X4GetDefaultParameters function
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
GT_STATUS mvHwsComphyN5XC56GP5X4GetDefaultParameters
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
        hwsOsMemCopyFuncPtr(&txDefaultParameters->txComphyC56G,
                            &tuneParams[baudRateIterator].txParams.txComphyC56G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&rxDefaultParameters->rxComphyC56G,
                            &tuneParams[baudRateIterator].rxParams.rxComphyC56G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
    }
#else
    GT_UNUSED_PARAM(serdesInfo);
    GT_UNUSED_PARAM(serdesData);
    GT_UNUSED_PARAM(baudRateIterator);

    if(txDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&txDefaultParameters->txComphyC56G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&rxDefaultParameters->rxComphyC56G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
    }

#endif
    return GT_OK;
}
#endif

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet
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
            if (!overrideExist) /*override param not found*/
            {
                return GT_NOT_FOUND;
            }
            else/*override param exist*/
            {
               hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC56G),
                                    &(serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC56G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
            }
            break;
        case SERDES_DB_OVERRIDE_AND_DEFAULT:
            if (!overrideExist)/*override param not found - set default param*/
            {
                hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC56G),
                                    &(tuneParams[defaultParamIterator].txParams.txComphyC56G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
            }
            else/*override param exist - go over tx params and set override value if exist or default if not exist*/
            {
                /* osPrintf("[%s::%d]  pre2: %d pre: %d post: %d main: %d usr: %d \n",__FUNCTION__,__LINE__,serdesData->tuneOverrideParams[overrideParamIterator].txParams.txComphyC56G.pre2,
                                                                                                 serdesData->tuneOverrideParams[overrideParamIterator].txParams.txComphyC56G.pre,
                                                                                                 serdesData->tuneOverrideParams[overrideParamIterator].txParams.txComphyC56G.post,
                                                                                                 serdesData->tuneOverrideParams[overrideParamIterator].txParams.txComphyC56G.main,
                                                                                                 serdesData->tuneOverrideParams[overrideParamIterator].txParams.txComphyC56G.usr);*/

                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC56G.pre2,  tuneParams[defaultParamIterator].txParams.txComphyC56G.pre2, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC56G.pre2);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC56G.pre,   tuneParams[defaultParamIterator].txParams.txComphyC56G.pre,  serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC56G.pre);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC56G.main,  tuneParams[defaultParamIterator].txParams.txComphyC56G.main, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC56G.main);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC56G.post,  tuneParams[defaultParamIterator].txParams.txComphyC56G.post, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC56G.post);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC56G.usr,   tuneParams[defaultParamIterator].txParams.txComphyC56G.usr,  serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC56G.usr);
            }
            break;
        case SERDES_DB_DEFAULT_ONLY:
            hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC56G),
                                    &(tuneParams[defaultParamIterator].txParams.txComphyC56G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_TX_CONFIG_DATA));
            break;
        default:
            rc = GT_NOT_INITIALIZED;
            break;
    }

    return rc;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet
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
                hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC56G),
                                    &(serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
            }
            break;
        case SERDES_DB_OVERRIDE_AND_DEFAULT:
            if (!overrideExist) /*override param not found - set default param*/
            {
                hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC56G),
                                    &(tuneParams[defaultParamIterator].rxParams.rxComphyC56G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
            }
            else/*override param exist - go over rx params and set override value if exist or default if not exist*/
            {
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.cur1Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.cur1Sel,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.cur1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.rl1Sel,             tuneParams[defaultParamIterator].rxParams.rxComphyC56G.rl1Sel,         serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.rl1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.rl1Extra,           tuneParams[defaultParamIterator].rxParams.rxComphyC56G.rl1Extra,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.rl1Extra);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.res1Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.res1Sel,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.res1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.cap1Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.cap1Sel,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.cap1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.enMidfreq,          tuneParams[defaultParamIterator].rxParams.rxComphyC56G.enMidfreq,      serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.enMidfreq);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.cs1Mid,             tuneParams[defaultParamIterator].rxParams.rxComphyC56G.cs1Mid,         serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.cs1Mid);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.rs1Mid,             tuneParams[defaultParamIterator].rxParams.rxComphyC56G.rs1Mid,         serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.rs1Mid);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.cur2Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.cur2Sel,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.cur2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.rl2Sel,             tuneParams[defaultParamIterator].rxParams.rxComphyC56G.rl2Sel,         serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.rl2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.rl2TuneG,           tuneParams[defaultParamIterator].rxParams.rxComphyC56G.rl2TuneG,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.rl2TuneG);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.res2Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.res2Sel,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.res2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.cap2Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.cap2Sel,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.cap2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.selmufi,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.selmufi,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.selmufi);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.selmuff,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.selmuff,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.selmuff);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.selmupi,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.selmupi,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.selmupi);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC56G.selmupf,            tuneParams[defaultParamIterator].rxParams.rxComphyC56G.selmupf,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.selmupf);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_SIGNED_VAL(rxConfigPtr->rxComphyC56G.squelch,     tuneParams[defaultParamIterator].rxParams.rxComphyC56G.squelch,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC56G.squelch);
            }
            break;
        case SERDES_DB_DEFAULT_ONLY:
            hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC56G),
                                    &(tuneParams[defaultParamIterator].rxParams.rxComphyC56G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C56G_RX_CONFIG_DATA));
            break;
        default:
            rc = GT_NOT_INITIALIZED;
            break;
    }
    return rc;
}


/**
* @internal mvHwsComphyN5XC56GP5X4SerdesManualTxConfig function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesManualTxConfig
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

    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.pre2)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE2, -1 * (MCESD_32)txConfigPtr->txComphyC56G.pre2));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.pre)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE,  -1 * (MCESD_32)txConfigPtr->txComphyC56G.pre));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.post)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_POST, -1 * (MCESD_32)txConfigPtr->txComphyC56G.post));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.main)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_MAIN, (MCESD_32)ABS(txConfigPtr->txComphyC56G.main)));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC56G.usr)
    {
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_5, (txConfigPtr->txComphyC56G.usr << 16), (1 << 16)));
        /* Assert fields 'rx_dc_term_en_fm_reg_lane' and 'rx_dc_term_en_lane' */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x5630, 25, 2, (txConfigPtr->txComphyC56G.usr ? 0x3 : 0x0)));
    }

    /** Static configurations */

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesManualRxConfig function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(rxConfigPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    /* Basic (CTLE) */
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cur1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR1_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cur1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC56G.rl1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl1Extra)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_EXTRA,   (MCESD_U32)rxConfigPtr->rxComphyC56G.rl1Extra));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.res1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES1_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.res1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cap1Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cap1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.enMidfreq)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_EN_MIDFREQ,  (MCESD_U32)rxConfigPtr->rxComphyC56G.enMidfreq));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cs1Mid)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CS1_MID,     (MCESD_U32)rxConfigPtr->rxComphyC56G.cs1Mid));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rs1Mid)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RS1_MID,     (MCESD_U32)rxConfigPtr->rxComphyC56G.rs1Mid));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cur2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR2_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cur2Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC56G.rl2Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.rl2TuneG)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_TUNE_G,  (MCESD_U32)rxConfigPtr->rxComphyC56G.rl2TuneG));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.res2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES2_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.res2Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.cap2Sel)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL,    (MCESD_U32)rxConfigPtr->rxComphyC56G.cap2Sel));

    /* Advanced (CDR) */
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmufi)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmufi));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmuff)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmuff));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmupi)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmupi));
    if(NA_16BIT != rxConfigPtr->rxComphyC56G.selmupf)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC56G.selmupf));

    if(NA_16BIT_SIGNED != rxConfigPtr->rxComphyC56G.squelch)
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetSquelchThreshold(sDev, serdesInfo->internalLane, (MCESD_16)rxConfigPtr->rxComphyC56G.squelch));

    /** Static configurations */

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesReset function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesReset
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
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_1, (data << 4), (1 << 4)));

    /* SERDES SYNCE RESET init */
    data = (syncEReset == GT_TRUE) ? 0 : 1;
    /*CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (data << 18), (1 << 18));*/

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesCoreReset function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesCoreReset
(
    GT_U8           devNum,
    GT_UOPT         portGroup,
    GT_UOPT         serdesNum,
    MV_HWS_RESET    coreReset
)
{
    GT_UREG_DATA data;

    data = (coreReset == UNRESET) ? 0 : 3;
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_1, (data << 2), (3 << 2)));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxEnbale function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxEnbale
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

    /* need to uncomment and remove the next writing after CE updates MCESD */
    /*MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)txEnable));*/
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x3004, 31, 1, (txEnable) ? 0x0 : 0x1));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL *txEnable
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32 data;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* need to uncomment and remove the next writing after CE updates MCESD */
    /*MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL*)txEnable));*/
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x3004,31,1, &data));

    *txEnable = (0 == data) ? GT_TRUE : GT_FALSE;    /* inverted */

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl
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
    E_N5XC56GP5X4_SERDES_SPEED   speed = 0;
    E_N5XC56GP5X4_REFFREQ        refFreq = 0;
    E_N5XC56GP5X4_REFCLK_SEL     refClkSel = 0;
    E_N5XC56GP5X4_DATABUS_WIDTH  dataBusWidth = 0;
    S_N5XC56GP5X4_PowerOn        powerOnConfig;
    MV_HWS_SERDES_SPEED          baudRate = serdesConfigPtr->baudRate;
    MCESD_BOOL txReset, rxReset;
#if (!defined FW_253_6)
    GT_U32                       txRegVal, rxRegVal;
#endif
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(powerUp);
    GT_UNUSED_PARAM(serdesConfigPtr);
    if(powerUp)
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Power-on lane */
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_N5XC56GP5X4_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_N5XC56GP5X4_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_N5XC56GP5X4_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_N5XC56GP5X4_DATABUS_WIDTH)mcesdEnum;
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);
#endif
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            powerOnConfig.u.powerLaneMask = 1 << serdesInfo->internalLane;
            powerOnConfig.initTx          = MCESD_TRUE;
            powerOnConfig.initRx          = MCESD_TRUE;
            powerOnConfig.txOutputEn      = MCESD_FALSE;
            powerOnConfig.downloadFw      = MCESD_FALSE;
            powerOnConfig.dataPath        = N5XC56GP5X4_PATH_EXTERNAL;
            powerOnConfig.txRefClkSel     = refClkSel;
            powerOnConfig.rxRefClkSel     = refClkSel;
            powerOnConfig.txRefFreq       = refFreq;
            powerOnConfig.rxRefFreq       = refFreq;
            powerOnConfig.dataBusWidth    = dataBusWidth;
            powerOnConfig.txSpeed         = speed;
            powerOnConfig.rxSpeed         = speed;
            powerOnConfig.avdd            = N5XC56GP5X4_AVDD_1P2V;
            powerOnConfig.spdCfg          = N5XC56GP5X4_SPD_CFG_4_PLL;
            powerOnConfig.fwDownload      = NULL;

            /* ANA_RX_SQ_OUT_FM_REG_LANE, ANA_RX_SQ_OUT_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator],0x5674,9, 2, 0x0));

            MCESD_CHECK_STATUS(API_N5XC56GP5X4_PowerOnSeq(sDev, powerOnConfig));
        }
        /* Wait for stable PLLs */
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[0],NULL,NULL,&sDev));
        do
        {
            allStable = GT_TRUE;
            for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
            {
#if (!defined FW_253_6)
                CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesArr[serdesIterator], 0x3000,(18),1, &txRegVal));
                CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesArr[serdesIterator], 0x3200,(22),1, &rxRegVal));
                data = (txRegVal <<3) | (rxRegVal << 2);
#else
                CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SDW_LANE_STATUS_0, &data, PLL_INIT_POLLING_DONE));
#endif
                if (data != PLL_INIT_POLLING_DONE)
                {
                    allStable = GT_FALSE;
                    break;
                }
            }
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            retries--;
        } while ((retries != 0) && (allStable == GT_FALSE));
        if(allStable == GT_FALSE)
        {
            return GT_TIMEOUT;
        }
        for(serdesIterator = 0; serdesIterator < numOfSer; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesAdjustClampingSettings(devNum, portGroup, serdesArr[serdesIterator], GT_TRUE));

            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));

            /* Config Tx/Rx parameters */
            MV_HWS_CONVERT_TO_OPTICAL_BAUD_RATE_MAC(serdesConfigPtr->opticalMode, baudRate);
#if 0
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4GetDefaultParameters(devNum, serdesArr[serdesIterator], baudRate, &txDefaultParameters, &rxDefaultParameters));
#ifdef MICRO_INIT
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, &txDefaultParameters));
#endif
#endif
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &txParameters));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &rxParameters));

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txParameters));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxParameters));

            /* SQ_LPF_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesArr[serdesIterator], 0x3270, (0xFFFF << 16), 0x3FF << 16));

            /* SQ_LPF_EN_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesArr[serdesIterator], 0x3270, (0x1 << 12), 0x1   << 12));

            if(serdesConfigPtr->encoding == SERDES_ENCODING_PAM4)
            {
#if (!defined FW_253_6)
                /*  N5XC56GP5X4_WRITE_FIELD(sDev, F_N5XC56GP5X4_PHASE_F0B_HYST_HIGH_LANE, serdesArr[serdesIterator], 0x1e);*/
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesArr[serdesIterator], 0x6200, 0x1e, 0x1e));
#endif
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane, N5XC56GP5X4_GRAY_ENABLE, N5XC56GP5X4_GRAY_ENABLE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPreCode(sDev, serdesInfo->internalLane, MCESD_FALSE, MCESD_FALSE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMSBLSBSwap(sDev, serdesInfo->internalLane, N5XC56GP5X4_SWAP_PRECODER, N5XC56GP5X4_SWAP_PRECODER));
            }
            else
            {
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane, N5XC56GP5X4_GRAY_DISABLE, N5XC56GP5X4_GRAY_DISABLE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPreCode(sDev, serdesInfo->internalLane, MCESD_FALSE, MCESD_FALSE));
                MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMSBLSBSwap(sDev, serdesInfo->internalLane, N5XC56GP5X4_SWAP_DISABLE, N5XC56GP5X4_SWAP_DISABLE));
            }

            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxEnbale(devNum, portGroup, serdesArr[serdesIterator], GT_TRUE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));
        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            /* Move to default clamping values */
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesAdjustClampingSettings(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE));

            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));

            /* if user enabled PRBS and deleted the port (SerDes is powered-down), need to restore to normal mode,
               in order to enable port to reach link up in next power-up */
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopPhyTest(sDev, serdesInfo->internalLane, N5XC56GP5X4_PHYTEST_TX));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopPhyTest(sDev, serdesInfo->internalLane, N5XC56GP5X4_PHYTEST_RX));

            MCESD_CHECK_STATUS(API_N5XC56GP5X4_PowerOffLane(sDev, serdesInfo->internalLane));

            /* ANA_RX_SQ_OUT_FM_REG_LANE, ANA_RX_SQ_OUT_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator],0x5674,9, 2, 0x3));

            MCESD_CHECK_STATUS(API_N5XC56GP5X4_DisableTxFirForce(sDev, serdesInfo->internalLane));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane, N5XC56GP5X4_GRAY_DISABLE, N5XC56GP5X4_GRAY_DISABLE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMSBLSBSwap(sDev, serdesInfo->internalLane, N5XC56GP5X4_SWAP_DISABLE, N5XC56GP5X4_SWAP_DISABLE));
            CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxEnbale(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE));
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_FALSE));

            /* need to clear the HW mode coding in case of trx training (does not mater for case rx only) */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator],0x30D4,31, 1, 0x1)); /* TX_TRAIN_CODING_MODE_HW_RST_TX_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator],0x5088,21, 1, 0x1)); /* TX_TRAIN_CODING_MODE_HW_RST_RX_LANE */
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 1));
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator],0x30D4,31, 1, 0x0)); /* TX_TRAIN_CODING_MODE_HW_RST_TX_LANE */
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[serdesIterator],0x5088,21, 1, 0x0)); /* TX_TRAIN_CODING_MODE_HW_RST_RX_LANE */

            /* waiting for core reset ack before de-assreting reset */
            CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesArr[serdesIterator], SDW_LANE_MUX_CONTROL_0, &data, 0x1));
            if (data == 0x1)
            {
                CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], RESET));
                retries = 0;
                do
                {
                    MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, RESET_CORE_ACK_DELAY));
                    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetResetCoreAckTxRx(sDev, serdesInfo->internalLane, &txReset, &rxReset));
                    retries++;
                } while ((txReset != MCESD_TRUE) && (rxReset != MCESD_TRUE) && (retries < RESET_CORE_ACK_TIMEOUT));
                if(retries >= RESET_CORE_ACK_TIMEOUT)
                {
                    return GT_TIMEOUT;
                }
                CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesCoreReset(devNum,portGroup,serdesArr[serdesIterator],UNRESET));
            }
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPowerCtrl function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStop function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneStart function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneStart
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
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatus function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatus
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_CheckTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_CheckTraining(sDev, serdesInfo->internalLane, N5XC56GP5X4_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort
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
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_RX_TRAINCO0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_RX_TRAINFA0 + serdesInfo->internalLane, &failedData));        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_TX_TRAINCO0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, N5XC56GP5X4_PIN_TX_TRAINFA0 + serdesInfo->internalLane, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesAutoTuneResult function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MV_HWS_PER_SERDES_INFO_PTR              serdesInfo;
    MCESD_DEV_PTR                           sDev;
    MV_HWS_COMPHY_C56G_AUTO_TUNE_RESULTS    *results;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U16 val16;
    MCESD_16  vals16;
    MCESD_U32 tapsI;
    MCESD_U16 analogSetting;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC56GResults;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE2, &vals));
    results->txComphyC56G.pre2 = -1 * (GT_8)vals;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_PRE,  &vals));
    results->txComphyC56G.pre  = -1 * (GT_8)vals;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_MAIN, &vals));
    results->txComphyC56G.main = (GT_8)vals;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxEqParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_TXEQ_EM_POST, &vals));
    results->txComphyC56G.post = -1 * (GT_8)vals;
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_5, &val, (1 << 16)));
    results->txComphyC56G.usr = (GT_8)(val >> 16);

    /* Basic (CTLE) */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR1_SEL,   &val));
    results->rxComphyC56G.cur1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_SEL,    &val));
    results->rxComphyC56G.rl1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL1_EXTRA,  &val));
    results->rxComphyC56G.rl1Extra= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES1_SEL,   &val));
    results->rxComphyC56G.res1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP1_SEL,   &val));
    results->rxComphyC56G.cap1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_EN_MIDFREQ, &val));
    results->rxComphyC56G.enMidfreq= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CS1_MID,    &val));
    results->rxComphyC56G.cs1Mid= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RS1_MID,    &val));
    results->rxComphyC56G.rs1Mid= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CUR2_SEL,   &val));
    results->rxComphyC56G.cur2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_SEL,    &val));
    results->rxComphyC56G.rl2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RL2_TUNE_G, &val));
    results->rxComphyC56G.rl2TuneG= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_RES2_SEL,   &val));
    results->rxComphyC56G.res2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCTLEParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CTLE_CAP2_SEL,   &val));
    results->rxComphyC56G.cap2Sel= (GT_U32)val;

    /* Advanced (CDR) */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFI, &val));
    results->rxComphyC56G.selmufi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUFF, &val));
    results->rxComphyC56G.selmuff= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPI, &val));
    results->rxComphyC56G.selmupi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetCDRParam(sDev, serdesInfo->internalLane, N5XC56GP5X4_CDR_SELMUPF, &val));
    results->rxComphyC56G.selmupf= (GT_U32)val;

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSquelchThreshold(sDev, serdesInfo->internalLane, &vals16));
    results->rxComphyC56G.squelch= (GT_16)vals16;

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetAlign90(sDev, serdesInfo->internalLane, &val16, &analogSetting));
    results->align90 = (GT_16)val16;
    results->align90AnaReg = (GT_U16)analogSetting;

    results->eo = 0;
    results->sampler = 0;
    results->slewRateCtrl0 = 0;
    results->slewRateCtrl1 = 0;

    for(tapsI = 0 ; tapsI < N5XC56GP5X4_DFE_FF5 ; tapsI++)
    {
        if (MCESD_OK == API_N5XC56GP5X4_GetDfeTap(sDev, serdesInfo->internalLane, N5XC56GP5X4_EYE_MID, (E_N5XC56GP5X4_DFE_TAP)tapsI, &vals))
        {
            results->dfe[tapsI] = (GT_32)vals;
        }
        else
        {
            results->dfe[tapsI] = 0xffff;
        }

    }
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesDigitalReset function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesDigitalReset
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
    CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_1, (data << 4), (1 << 4)));

    return GT_OK;
}
/**
* @internal mvHwsComphyN5XC56GP5X4SerdesLoopback function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR          serdesInfo;
    MCESD_DEV_PTR                       sDev;
    E_N5XC56GP5X4_DATAPATH              path;
    GT_U32                              mcesdEnum, data, result;
    MV_HWS_SERDES_TX_CONFIG_DATA_UNT    txParameters;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_N5XC56GP5X4_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetDataPath(sDev, serdesInfo->internalLane, path));

    switch ( lbMode )
    {
    case SERDES_LP_AN_TX_RX:
        /*TX parameter config*/
        txParameters.txComphyC56G.main = 0x3f;
        txParameters.txComphyC56G.post = 0;
        txParameters.txComphyC56G.pre = 0;
        txParameters.txComphyC56G.pre2 = 0;
        txParameters.txComphyC56G.usr = 0;
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesManualTxConfig(devNum, portGroup, serdesNum, &txParameters));
        /* Masks sq_detected indication. If set, SQ detect is always zero (signal detected). Masks sq_detected indication */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (1 << 27), (1 << 27)));
        /* dtl_sq_det_en_lane=0*/
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x3260, 13, 1, 0x0));
        /* dfe_sq_en_lane=0*/
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x4040, 29, 1, 0x0));
        /* toggle dfe_reset*/
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesOperation(devNum, portGroup, serdesNum, _10GBase_KR, MV_HWS_PORT_SERDES_OPERATION_RESET_RXTX_E, &data, &result));
        break;
    default:
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (0 << 27), (1 << 27)));
        break;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesLoopbackGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_N5XC56GP5X4_DATAPATH          path;
    GT_U32                      tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetDataPath(sDev, serdesInfo->internalLane, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTestGen function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTestGen
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
    E_N5XC56GP5X4_PATTERN             pattern;
    E_N5XC56GP5X4_SERDES_SPEED        speed;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    MV_HWS_SERDES_ENCODING_TYPE txEncoding, rxEncoding;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U8)txPattern, &mcesdPattern));
    pattern = (E_N5XC56GP5X4_PATTERN)mcesdPattern;

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
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxRxPattern(sDev, serdesInfo->internalLane, pattern, pattern, userPattern, userPattern));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxBitRate(sDev, serdesInfo->internalLane,&speed,&speed));

    CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet(devNum,serdesNum,&txEncoding, &rxEncoding));
    if ( txEncoding ==  SERDES_ENCODING_PAM4 || rxEncoding ==  SERDES_ENCODING_PAM4 )
    {
        /* TXDATA_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3024, 5,  1, (SERDES_TEST == mode) ? 0x0 : 0x1));

        /* RXDATA_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3248, 24, 1, (SERDES_TEST == mode) ? 0x0 : 0x1));

        /* TXD_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3024, 18, 1, 0x0));

        /* RXD_MSB_LSB_SWAP_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x3248, 27, 1, 0x0));
    }

    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_ResetComparatorStats(sDev, serdesInfo->internalLane));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartPhyTest(sDev, serdesInfo->internalLane, N5XC56GP5X4_PHYTEST_TX));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StartPhyTest(sDev, serdesInfo->internalLane, N5XC56GP5X4_PHYTEST_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopPhyTest(sDev, serdesInfo->internalLane, N5XC56GP5X4_PHYTEST_TX));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_StopPhyTest(sDev, serdesInfo->internalLane, N5XC56GP5X4_PHYTEST_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTestGenGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTestGenGet
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
    E_N5XC56GP5X4_PATTERN             txPattern;
    E_N5XC56GP5X4_PATTERN             rxPattern;
    char                              userPattern[21];
    MCESD_FIELD                       ptEn = F_N5XC56GP5X4_TX_EN;
    MCESD_U32                         enable;
    GT_U32                            tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxPattern(sDev, serdesInfo->internalLane, &txPattern, &rxPattern, userPattern, userPattern));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_ReadField(sDev, serdesInfo->internalLane, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTestGenStatus
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTestGenStatus
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
    S_N5XC56GP5X4_PATTERN_STATS       statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 1));
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetComparatorStats(sDev, serdesInfo->internalLane, &statistics));

    /* PHYtest Rx Control 3 */
    if (!counterAccumulateMode)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x328C, 7, 1, 0x1));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 1));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x328C, 7, 1, 0x0));
    }

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr.l[0]   = (GT_U32) (statistics.totalErrorBits & 0x00000000FFFFFFFFUL);
    status->errorsCntr.l[1]   = (GT_U32)((statistics.totalErrorBits & 0xFFFFFFFF00000000UL) >> 32);
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_ResetComparatorStats(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesErrorInject function
* @endinternal
*
* @brief   Injects errors into the RX or TX data
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] serdesNum                - physical serdes number
* @param[in] numOfBits                - Number of error bits to inject (max=8)
* @param[in] serdesDirection          - Rx or Tx
*
* @retval 0                        - on success
* @retval 1                        - on error
*
*/
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesErrorInject
(
    IN GT_U8                        devNum,
    IN GT_UOPT                      portGroup,
    IN GT_UOPT                      serdesNum,
    IN GT_UOPT                      numOfBits,
    IN MV_HWS_SERDES_DIRECTION      serdesDirection
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;

    GT_UNUSED_PARAM(portGroup);
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

    if(serdesDirection != TX_DIRECTION)
    {
        return GT_NOT_SUPPORTED;
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_TxInjectError(sDev, serdesInfo->internalLane, (MCESD_U8)numOfBits));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPolarityConfig function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPolarityConfig
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

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetTxRxPolarity(sDev, serdesInfo->internalLane, invertTx ? N5XC56GP5X4_POLARITY_INV : N5XC56GP5X4_POLARITY_NORMAL,
                                                       invertRx ? N5XC56GP5X4_POLARITY_INV : N5XC56GP5X4_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPolarityConfigGet
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPolarityConfigGet
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
    E_N5XC56GP5X4_POLARITY      txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxPolarity(sDev, serdesInfo->internalLane, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == N5XC56GP5X4_POLARITY_INV);
    *invertRx = (rxPolarity == N5XC56GP5X4_POLARITY_INV);

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet
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

    /* in serdes loopback analog tx2rx, we forced signal detect in mvHwsComphyN5XC56GP5X4SerdesLoopback */
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, &data, (1 << 27)));
    if ( data != 0 )
    {
        *enable = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetSquelchDetect(sDev, serdesInfo->internalLane, &squelched));
    *enable = squelched ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesCdrLockStatusGet function
* @endinternal
*
* @brief   Reads cdr lock in functional mode
*
* @param[in]  devNum    - system device number
* @param[in]  portGroup - port group (core) number
* @param[in]  serdesNum - serdes number to get its signal state
* @param[out] enable    - return cdr lock value
*
* @retval 0 - on success
* @retval 1 - on error
*/
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *lock
)
{
    MCESD_DEV_PTR                 sDev;
    GT_U32                        data, i, countNot1 = 0, count1 = 0;
    MV_HWS_PER_SERDES_INFO_PTR    serdesInfo;
    E_N5XC56GP5X4_SERDES_SPEED    serdesSpeedBaudRate;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_ATTEMPT(API_N5XC56GP5X4_GetTxRxBitRate(sDev, serdesInfo->internalLane, &serdesSpeedBaudRate, &serdesSpeedBaudRate));

    MCESD_CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    if (*lock == GT_FALSE)
    {
        return GT_OK;
    }

    /* read CDR_LOCK_DETECT_INTERFACE_LANE, lock determined by out of 5 */
    for (i = 0; i < 7; i++)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x6430, 8, 8, 0x0)); /* CDR_LOCK_DETECT_INTERFACE_LANE */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 1));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x6430, 8, 8, 0xFF)); /* CDR_LOCK_DETECT_INTERFACE_LANE */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, DTL_CLAMPING_DELAY));
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum, portGroup, serdesNum, 0x6430, 8, 8, &data));

        /* update counters */
        count1 = (data == 1) ? (count1 + 1) : count1;
        countNot1 = (data != 1) ? (countNot1 + 1) : countNot1;

        if (countNot1 == 5 || count1 == 5)
        {
            break;
        }
    }
    *lock = (count1 > countNot1) ? GT_TRUE : GT_FALSE;

    if ((*lock  == GT_TRUE) && (serdesSpeedBaudRate == N5XC56GP5X4_SERDES_1P25G))
    {
        /* DTL clamping clear*/
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x3260, 12, 1, 0x1));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 1));
        CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x3260, 12, 1, 0x0));
    }


    return GT_OK;
}


static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesClampingValuesResetAndStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *clampingStatusPtr
)
{
    GT_U32 data;
    CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x3260,29,1,&data));
    if (data == 0)
    {
        *clampingStatusPtr = GT_TRUE;
        /* Move to default clamping values */
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesAdjustClampingSettings(devNum, portGroup, serdesNum, GT_FALSE));
    }
    else
    {
         *clampingStatusPtr = GT_FALSE;
    }

    return GT_OK;
}

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
static MCESD_STATUS mvHwsComphyN5XC56GP5X4DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 index;

    *errCode = MCESD_IO_ERROR;

    /* Check if the code about to be downloaded can fit into the device's memory */
    if (fwCodeSizeDW > N5XC56GP5X4_FW_MAX_SIZE)
    {
        *errCode = MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD;
        return MCESD_FAIL;
    }

    /* Write to program memory incrementally */
    for (index = 0; index < fwCodeSizeDW; index++)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_WriteReg(devPtr, 255 /* ignored */, address, mvHwsComphyCodeGet(index,fwCodePtr)));
        address += 4;
    }

    *errCode = 0;
    return MCESD_OK;
}
#endif

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesFirmwareDownloadIter
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesFirmwareDownloadIter
(
    GT_U8                     devNum,
    GT_UOPT                   *serdesArr,
    GT_U8                     numOfSerdeses,
    MCESD_U32                 *fwCodePtr,
    MCESD_U32                 fwCodeSize,
    E_N5XC56GP5X4_REFCLK_SEL  refClkSel,
    E_N5XC56GP5X4_REFFREQ     refFreq
)
{
    MCESD_DEV_PTR               sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U8 serdesIterator;
    MCESD_U16 errCode;

    for(serdesIterator = 0; serdesIterator < numOfSerdeses; serdesIterator++)
    {
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesArr[serdesIterator], &serdesInfo, NULL, &sDev));
        if(serdesInfo->internalLane != 0)
        {
            return GT_BAD_PARAM;
        }
        /* Set SERDES Mode */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPhyMode(sDev, N5XC56GP5X4_PHYMODE_SERDES));
        /* Power up current and voltage reference */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPowerIvRef(sDev, MCESD_TRUE));

        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMcuBroadcast(sDev, MCESD_TRUE));
        /* Configure Reference Frequency and reference clock source group */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetRefFreq(sDev, serdesInfo->internalLane, refFreq, refFreq, refClkSel, refClkSel));

        /* Clear N5XC56GP5X4_PIN_FW_READY */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_0, (0 << 3), (1 << 3)));
        /* Set N5XC56GP5X4_PIN_DIRECTACCES */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwSetPinCfg(sDev, N5XC56GP5X4_PIN_DIRECTACCES, 1));
        /* Set N5XC56GP5X4_PIN_PRAM_SOC_EN */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_1, (1 << 14), (1 << 14)));

        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 10));
        /* Download MCU Firmware */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_DownloadFirmware(sDev, fwCodePtr, fwCodeSize, N5XC56GP5X4_FW_BASE_ADDR, &errCode));
#else
        CHECK_STATUS(mvHwsComphyN5XC56GP5X4DownloadFirmware(sDev, fwCodePtr, fwCodeSize, N5XC56GP5X4_FW_BASE_ADDR, &errCode));
#endif
        /* Clear N5XC56GP5X4_PIN_PRAM_SOC_EN */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_1, (0 << 14), (1 << 14)));
        /* Clear N5XC56GP5X4_PIN_DIRECTACCES */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwSetPinCfg(sDev, N5XC56GP5X4_PIN_DIRECTACCES, 0));
        /* Set N5XC56GP5X4_PIN_FW_READY */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG, serdesArr[serdesIterator], SDW_GENERAL_CONTROL_0, (1 << 3), (1 << 3)));

        MCESD_CHECK_STATUS(API_N5XC56GP5X4_Wait(sDev, 10));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMcuBroadcast(sDev, MCESD_FALSE));
#if 0
        /* Enable MCU */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetLaneEnable(sDev, 255, MCESD_TRUE));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetMcuEnable(sDev, 255, MCESD_TRUE));
#endif
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesFirmwareDownload function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesFirmwareDownload
(
    GT_U8       devNum,
    GT_UOPT     portGroup,
    GT_UOPT     *serdesArr,
    GT_U8       numOfSerdeses,
    GT_U8       firmwareSelector
)
{
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    MCESD_U16              errCode;
#endif
    GT_STATUS              rc = GT_OK;

    E_N5XC56GP5X4_REFCLK_SEL   refClkSel = N5XC56GP5X4_REFCLK_SEL_G1;
    E_N5XC56GP5X4_REFFREQ      refFreq;
    GT_BOOL                    freeNeeded = GT_FALSE;

    MCESD_U32 *fwCodePtr = NULL;
    MCESD_U32 fwCodeSize = 0;
    GT_UNUSED_PARAM(portGroup);

    switch(firmwareSelector)
    {
        case MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE:
        case MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE:
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            fwCodePtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*N5XC56GP5X4_FW_MAX_SIZE);
            if ( fwCodePtr == NULL )
            {
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//N5XC56GP5X4//main.dat",
                                        fwCodePtr,       N5XC56GP5X4_FW_MAX_SIZE,         &fwCodeSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }
#endif
            freeNeeded = GT_TRUE;
            break;
            case MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE:
        case MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE:
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            fwCodePtr     = (MCESD_U32*)N5XC56GP5X4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_N5XC56GP5X4_MAIN_DATA_IMAGE_SIZE;
#else
            fwCodePtr     = (MCESD_U32*)N5XC56GP5X4_main_DataPtr;
            fwCodeSize    = (MCESD_U32)N5XC56GP5X4_Image_Size;
#endif
            break;
        default:
            return GT_NOT_SUPPORTED;
    }
    if((MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE == firmwareSelector) || (MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE == firmwareSelector))
    {
        refFreq = N5XC56GP5X4_REFFREQ_156MHZ;
    }
    else if((MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE == firmwareSelector) || (MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE == firmwareSelector))
    {
        refFreq = N5XC56GP5X4_REFFREQ_25MHZ;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    comphyPrintf("--fwCodeSize[%d]--\n",fwCodeSize);
    if ( freeNeeded )
    {
        hwsOsPrintf("Using SerDes FW from file: version=[%u.%u.%u.%u], fwCodeSize=[%d]\n",
                    (fwCodePtr[128] & 0xFF000000)>>24,
                    (fwCodePtr[128] & 0x00FF0000)>>16,
                    (fwCodePtr[128] & 0x0000FF00)>>8,
                    (fwCodePtr[128] & 0x000000FF)>>0,
                    fwCodeSize);
    }
#endif
    rc = mvHwsComphyN5XC56GP5X4SerdesFirmwareDownloadIter(devNum, serdesArr, numOfSerdeses,
                                                       fwCodePtr, fwCodeSize,
                                                       refClkSel, refFreq);

    if ( freeNeeded )
    {
        hwsOsFreeFuncPtr(fwCodePtr);
    }

    /* enable SyncE by default, since FW disabled it */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesArr[0], 0xE6A0, 8, 1, 0x1));

    return rc;
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
static GT_STATUS mvHwsComphyN5XC56GP5X4AnpPowerUp
(
    IN GT_U8    devNum,
    IN  GT_UOPT portGroup,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    MV_HWS_PER_SERDES_INFO_PTR   serdesInfo;
    MCESD_DEV_PTR                sDev;
    GT_U32 regData = (powerUp == GT_TRUE) ? 1 : 0;
    GT_U32                       mcesdEnum;
    E_N5XC56GP5X4_REFFREQ        refFreq = 0;
    E_N5XC56GP5X4_REFCLK_SEL     refClkSel = 0;
    GT_UNUSED_PARAM(portGroup);
    if ( powerUp == GT_TRUE)
    {
        CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetDataBusWidth(sDev, serdesInfo->internalLane, N5XC56GP5X4_DATABUS_40BIT, N5XC56GP5X4_DATABUS_40BIT));
        /* SQ_LPF_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x3270, (0xFFFF << 16), 0x3FF << 16));
        /* SQ_LPF_EN_LANE */
        CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x3270, (1<<12), (regData<<12)));

        CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
        refFreq = (E_N5XC56GP5X4_REFFREQ)mcesdEnum;

        CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_N5XC56GP5X4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
        refClkSel = (E_N5XC56GP5X4_REFCLK_SEL)mcesdEnum;

        /* Configure Reference Frequency and reference clock source group */
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetRefFreq(sDev, serdesInfo->internalLane, refFreq, refFreq, refClkSel, refClkSel ));

    }

    /* ANA_RX_SQ_OUT_FM_REG_LANE, ANA_RX_SQ_OUT_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum,0x5674,9, 2, 0x0));

    CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesReset(devNum, portGroup, serdesNum, GT_FALSE, GT_FALSE, GT_FALSE));
    /* DISABLE TX HIZ*/
    CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesTxEnbale(devNum, portGroup, serdesNum, GT_TRUE));
    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesTemperatureGet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesTemperatureGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  portGroup,
    IN  GT_UOPT  serdesNum,
    OUT GT_32    *serdesTemperature
)
{
    MCESD_DEV_PTR   sDev;
    MCESD_32        temp;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTemperature(sDev, &temp));
    *serdesTemperature = (GT_32)temp/10000;

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet function
* @endinternal
*
* @brief   Retrieves the Tx and Rx line encoding values
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - physical PCS number
*
* @param[out] txEncodingPtr           - NRZ/PAM4
* @param[out] rxEncodingPtr           - NRZ/PAM4
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           serdesNum,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_FIELD txPAM2En = F_N5XC56GP5X4_TX_PAM2_EN;
    MCESD_FIELD rxPAM2En = F_N5XC56GP5X4_RX_PAM2_EN;
    MCESD_U32 data;
    MCESD_BOOL txReady, rxReady;

    if( NULL == txEncodingPtr || NULL == rxEncodingPtr )
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* check if serdes powered-up */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetTxRxReady(sDev, serdesInfo->internalLane, &txReady, &rxReady));

    if ( txReady == MCESD_FALSE )
    {
        *txEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_ReadField(sDev, serdesInfo->internalLane, &txPAM2En, &data));
        *txEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    if ( rxReady == MCESD_FALSE )
    {
        *rxEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_ReadField(sDev, serdesInfo->internalLane, &rxPAM2En, &data));
        *rxEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableSet
*           function
* @endinternal
*
* @brief   enable/disable gray code
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] enable                   - GT_TRUE = enable, GT_FALSE = disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableSet
(
    IN GT_U8    devNum,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  enable
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetGrayCode(sDev, serdesInfo->internalLane,
                                                   (enable) ? N5XC56GP5X4_GRAY_ENABLE : N5XC56GP5X4_GRAY_DISABLE,
                                                   (enable) ? N5XC56GP5X4_GRAY_ENABLE : N5XC56GP5X4_GRAY_DISABLE));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableGet
*           function
* @endinternal
*
* @brief   enable/disable gray code
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] enable                   - GT_TRUE = enable, GT_FALSE = disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  serdesNum,
    OUT GT_BOOL  *enablePtr
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    E_N5XC56GP5X4_GRAY_CODE txGrayCode, rxGrayCode;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetGrayCode(sDev, serdesInfo->internalLane, &txGrayCode, &rxGrayCode));
    *enablePtr = (txGrayCode && rxGrayCode) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableSet
*           function
* @endinternal
*
* @brief   enable/disable pre code
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] enable                   - GT_TRUE = enable, GT_FALSE = disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableSet
(
    IN GT_U8    devNum,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  enable
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetPreCode(sDev, serdesInfo->internalLane,
                                                   (enable) ? MCESD_TRUE : MCESD_FALSE,
                                                   (enable) ? MCESD_TRUE : MCESD_FALSE));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableGet
*           function
* @endinternal
*
* @brief   enable/disable pre code
*
* @param[in] devNum                   - system device number
* @param[in] serdesNum                - serdes number
* @param[in] enable                   - GT_TRUE = enable, GT_FALSE = disable
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  serdesNum,
    OUT GT_BOOL  *enablePtr
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    OUT MCESD_BOOL txState, rxState;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetPreCode(sDev, serdesInfo->internalLane, &txState, &rxState));
    *enablePtr = (txState && rxState) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsComphyN5XC56GP5X4SerdesEomMatrixGet function
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
GT_STATUS mvHwsComphyN5XC56GP5X4SerdesEomMatrixGet
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
    GT_STATUS rc = GT_OK;
    S_N5XC56GP5X4_EYE_RAW *mcesdEyeRawData;
    MV_HWS_SERDES_ENCODING_TYPE txEncoding, rxEncoding;
    GT_U32 i;
    S_N5XC56GP5X4_EYE_BUFFER eyeBufferData;
    S_EOM_STATS_EYE_DIM estimateEyeDim[ESTIMATE_EYE_DIM_COUNT];
    S_EOM_STATS_EYE_AMP amplitudeStats;
    MCESD_U32 widthmUI, height100uVUpper, height100uVLower;
    MCESD_U32 width, heightUpper, heightLower;
    MCESD_BOOL dfeState;

    if ( NULL == eomInParams|| NULL == eomOutParams)
    {
        return GT_BAD_PTR;
    }

    if (eomInParams->comphyC56GEomInParams.phaseStepSize   < 1 || eomInParams->comphyC56GEomInParams.phaseStepSize   > 16 ||
        eomInParams->comphyC56GEomInParams.voltageStepSize < 1 || eomInParams->comphyC56GEomInParams.voltageStepSize > 16)
    {
        return GT_OUT_OF_RANGE;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* need to enable DFE for EOM (disabled for low baud rates during power-up) */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_GetDfeEnable(sDev, serdesInfo->internalLane, &dfeState));
    if (dfeState == MCESD_FALSE)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetDfeEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));
    }

    mcesdEyeRawData = (S_N5XC56GP5X4_EYE_RAW*)hwsOsMallocFuncPtr(sizeof(S_N5XC56GP5X4_EYE_RAW));
    if ( NULL == mcesdEyeRawData )
    {
        return GT_NO_RESOURCE;
    }

    /* NRZ/PAM4 */
    rc = mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet(devNum,serdesNum,&txEncoding,&rxEncoding);
    if ( rc != GT_OK)
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return rc;
    }

    if ( rxEncoding == SERDES_ENCODING_NRZ )    /* for NRZ speeds, only mid eye exists */
    {
        mcesdSt = API_N5XC56GP5X4_EOMGetEyeData(sDev, serdesInfo->internalLane, N5XC56GP5X4_EYE_MID, (MCESD_U32)eomInParams->comphyC56GEomInParams.minSamples,
                                                (MCESD_U32)eomInParams->comphyC56GEomInParams.berThreshold, (MCESD_BOOL)eomInParams->comphyC56GEomInParams.eomStatsMode, mcesdEyeRawData);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }

        /* retrieve height and width in code word and convert to mV and pSec */
        mcesdSt = API_N5XC56GP5X4_EyeDataDimensions(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC56GEomInParams.berThreshold, &width, &heightUpper, &heightLower);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
        mcesdSt = API_N5XC56GP5X4_EOMConvertWidthHeight(sDev, serdesInfo->internalLane, width, heightUpper, heightLower, &widthmUI, &height100uVUpper, &height100uVLower);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
        eomOutParams->comphyC56GEomOutParams.width_mUI[N5XC56GP5X4_EYE_MID]   = (GT_U32)widthmUI;
        eomOutParams->comphyC56GEomOutParams.height_mV[N5XC56GP5X4_EYE_MID]   = (GT_U32)(height100uVUpper + height100uVLower) / 100;

        hwsOsMemSetFuncPtr(&amplitudeStats, 0, sizeof(amplitudeStats));
        hwsOsMemSetFuncPtr(estimateEyeDim, 0, sizeof(estimateEyeDim));

        /* retrieve EOM statistics */
        estimateEyeDim[0].ber = 1e-7;
        estimateEyeDim[1].ber = 1e-10;
        estimateEyeDim[2].ber = 1e-12;
        estimateEyeDim[3].ber = 1e-15;

        mcesdSt = API_N5XC56GP5X4_EOMGetStats(mcesdEyeRawData, &eyeBufferData, (S_EOM_STATS_EYE_DIM *)estimateEyeDim, ESTIMATE_EYE_DIM_COUNT, &amplitudeStats);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
        eomOutParams->comphyC56GEomOutParams.Q[N5XC56GP5X4_EYE_MID]           = amplitudeStats.Q;
        eomOutParams->comphyC56GEomOutParams.SNR[N5XC56GP5X4_EYE_MID]         = amplitudeStats.SNR;
        eomOutParams->comphyC56GEomOutParams.upperMean[N5XC56GP5X4_EYE_MID]   = amplitudeStats.upperMean;
        eomOutParams->comphyC56GEomOutParams.lowerMean[N5XC56GP5X4_EYE_MID]   = amplitudeStats.lowerMean;
        eomOutParams->comphyC56GEomOutParams.upperStdDev[N5XC56GP5X4_EYE_MID] = amplitudeStats.upperStdDev;
        eomOutParams->comphyC56GEomOutParams.lowerStdDev[N5XC56GP5X4_EYE_MID] = amplitudeStats.lowerStdDev;

        if ( GT_TRUE == printEnable )
        {
            GT_U32 j;

            mcesdSt = API_N5XC56GP5X4_EOMPlotEyeData(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC56GEomInParams.phaseStepSize,
                                                 (MCESD_U32)eomInParams->comphyC56GEomInParams.voltageStepSize,
                                                 (MCESD_U32)eomInParams->comphyC56GEomInParams.berThreshold,
                                                 (MCESD_U32)eomInParams->comphyC56GEomInParams.berThresholdMax);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }

            hwsOsPrintf("Estimated eye statistics per BER target:\n");
            for (j = 0; j < ESTIMATE_EYE_DIM_COUNT; j++)
            {
                hwsOsPrintf("BER target : %E - Width = %f, Height Upper = %f, Height Lower = %f\n", estimateEyeDim[j].ber, estimateEyeDim[j].width, estimateEyeDim[j].upperHeight, estimateEyeDim[j].lowerHeight);
            }
        }

        hwsOsFreeFuncPtr(mcesdEyeRawData);
    }
    else if ( rxEncoding == SERDES_ENCODING_PAM4 )
    {
        for (i = 0; i < MAX_EYE_NUM; i++) /* PAM4 speeds have 3 eyes */
        {
            mcesdSt = API_N5XC56GP5X4_EOMGetEyeData(sDev, serdesInfo->internalLane, i /*TMB*/, (MCESD_U32)eomInParams->comphyC56GEomInParams.minSamples,
                                                    (MCESD_U32)eomInParams->comphyC56GEomInParams.berThreshold, (MCESD_BOOL)eomInParams->comphyC56GEomInParams.eomStatsMode, mcesdEyeRawData);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }

            /* retrieve height and width in code word and convert to mV and pSec */
            mcesdSt = API_N5XC56GP5X4_EyeDataDimensions(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC56GEomInParams.berThreshold, &width, &heightUpper, &heightLower);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
            mcesdSt = API_N5XC56GP5X4_EOMConvertWidthHeight(sDev, serdesInfo->internalLane, width, heightUpper, heightLower, &widthmUI, &height100uVUpper, &height100uVLower);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
            eomOutParams->comphyC56GEomOutParams.width_mUI[i /*TMB*/]   = (GT_U32)widthmUI;
            eomOutParams->comphyC56GEomOutParams.height_mV[i /*TMB*/]   = (GT_U32)(height100uVUpper + height100uVLower) / 100;

            hwsOsMemSetFuncPtr(&amplitudeStats, 0, sizeof(amplitudeStats));
            hwsOsMemSetFuncPtr(estimateEyeDim, 0, sizeof(estimateEyeDim));

            /* retrieve EOM statistics */
            estimateEyeDim[0].ber = 1e-7;
            estimateEyeDim[1].ber = 1e-10;
            estimateEyeDim[2].ber = 1e-12;
            estimateEyeDim[3].ber = 1e-15;

            mcesdSt = API_N5XC56GP5X4_EOMGetStats(mcesdEyeRawData, &eyeBufferData, (S_EOM_STATS_EYE_DIM *)estimateEyeDim, ESTIMATE_EYE_DIM_COUNT, &amplitudeStats);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
            eomOutParams->comphyC56GEomOutParams.Q[i /*TMB*/]           = amplitudeStats.Q;
            eomOutParams->comphyC56GEomOutParams.SNR[i /*TMB*/]         = amplitudeStats.SNR;
            eomOutParams->comphyC56GEomOutParams.upperMean[i /*TMB*/]   = amplitudeStats.upperMean;
            eomOutParams->comphyC56GEomOutParams.lowerMean[i /*TMB*/]   = amplitudeStats.lowerMean;
            eomOutParams->comphyC56GEomOutParams.upperStdDev[i /*TMB*/] = amplitudeStats.upperStdDev;
            eomOutParams->comphyC56GEomOutParams.lowerStdDev[i /*TMB*/] = amplitudeStats.lowerStdDev;

            if ( GT_TRUE == printEnable )
            {
                GT_U32 j;

                mcesdSt = API_N5XC56GP5X4_EOMPlotEyeData(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC56GEomInParams.phaseStepSize,
                                                     (MCESD_U32)eomInParams->comphyC56GEomInParams.voltageStepSize,
                                                     (MCESD_U32)eomInParams->comphyC56GEomInParams.berThreshold,
                                                     (MCESD_U32)eomInParams->comphyC56GEomInParams.berThresholdMax);
                if ( mcesdSt != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(mcesdEyeRawData);
                    return GT_FAIL;
                }

                hwsOsPrintf("Estimated eye statistics per BER target:\n");
                for (j = 0; j < ESTIMATE_EYE_DIM_COUNT; j++)
                {
                    hwsOsPrintf("BER target : %E - Width = %f, Height Upper = %f, Height Lower = %f\n", estimateEyeDim[j].ber, estimateEyeDim[j].width, estimateEyeDim[j].upperHeight, estimateEyeDim[j].lowerHeight);
                }

                hwsOsPrintf("===============================================================================================================\n");
            }

            hwsOsMemSetFuncPtr(mcesdEyeRawData, 0, sizeof(mcesdEyeRawData));
        }
        hwsOsFreeFuncPtr(mcesdEyeRawData);
    }
    else
    {
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_BAD_VALUE;
    }

    /* restore DFE */
    MCESD_CHECK_STATUS(API_N5XC56GP5X4_SetDfeEnable(sDev, serdesInfo->internalLane, dfeState));

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4SerdesSignalDetectMaskSet function
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
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesSignalDetectMaskSet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL maskEnable
)
{
    MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x5674, 9, 1, (maskEnable==GT_TRUE?1:0)));
    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD */

/**
 * mvHwsComphyN5XC56GP5X4SerdesRxInit
 *
 * @internal mvHwsComphyN5XC56GP5X4SerdesRxInit function
 * @endinternal
 *
 * @brief Configure RX Init pins
 *
 * @param devNum
 * @param portGroup
 * @param serdesNum
 * @param rxInitMode
 * @param rxInitStatus
 *
 * @return GT_STATUS
 */
static GT_STATUS mvHwsComphyN5XC56GP5X4SerdesRxInit
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
    CHECK_STATUS(hwsSerdesRegGetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, &data, (1 << 27)));
    if ( data != 0 )
    {
        return GT_OK;
    }

    /* check signal */
    MCESD_CHECK_STATUS(mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet(devNum, portGroup, serdesNum, &sigDetect));

    if (sigDetect == GT_TRUE)
    {
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, (N5XC56GP5X4_PIN_RX_TRAINEN0 + serdesInfo->internalLane), &completed));
        MCESD_CHECK_STATUS(API_N5XC56GP5X4_HwGetPinCfg(sDev, (N5XC56GP5X4_PIN_RX_INITDON0 + serdesInfo->internalLane), &rxInitDone));
        if ((completed == 1) || (rxInitDone == 1))
            return GT_OK;

        switch (rxInitMode)
        {
        case RX_INIT_BLOCKING:
            MCESD_CHECK_STATUS(API_N5XC56GP5X4_RxInit(sDev, serdesInfo->internalLane, 5000));
            break;
        default:
            return GT_NOT_SUPPORTED;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyN5XC56GP5X4IfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyN5XC56GP5X4IfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_N5XC56GP5X4])
    {
        funcPtrArray[COM_PHY_N5XC56GP5X4] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_N5XC56GP5X4])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_N5XC56GP5X4], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }

    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc              ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc                   ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc              ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc              ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc                ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc               ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc               ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc          ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc                        ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset                     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc                      ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc                   ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc                ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc               ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc                     ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc                 ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc              ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesFirmwareDownloadFunc             ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesFirmwareDownload);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTemperatureFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesTemperatureGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc                   ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc                   ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesDbRxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc                 ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4AnpPowerUp);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                         ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesOperation);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEncodingTypeGetFunc              ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesEncodingTypeGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesErrorInjectFunc                  ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesErrorInject);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc             ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesCdrLockStatusGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesClampingValuesResetFunc          ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesClampingValuesResetAndStatusGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesGrayCodeEnableSetFunc            ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableSet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesGrayCodeEnableGetFunc            ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesGrayCodeEnableGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPreCodeEnableSetFunc             ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableSet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPreCodeEnableGetFunc             ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesPreCodeEnableGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxInitFunc              ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesRxInit);
#ifndef MV_HWS_REDUCED_BUILD
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEomMatrixGetFunc                 ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesEomMatrixGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSigDetectMaskFunc                ,COM_PHY_N5XC56GP5X4, mvHwsComphyN5XC56GP5X4SerdesSignalDetectMaskSet);
#endif

    return GT_OK;
}

#endif /* N5XC56GP5X4 */


