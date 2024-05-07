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
* @file mvComphyC112GX4SerdesIf.c \
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

#ifdef C112GX4
#include "mcesdC112GX4_Defs.h"
#include "mcesdC112GX4_API.h"
#include "mcesdC112GX4_DeviceInit.h"
#include "mcesdC112GX4_HwCntl.h"
#include "mcesdC112GX4_RegRW.h"
#include "mcesdC112GX4_FwDownload.h"
#ifndef MICRO_INIT
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_main.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_SERDES_REF25MHz_SPDCHG0_LANE.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_SERDES_REF156MHz_SPDCHG0_LANE.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/comphy_fw/C112GX4_SERDES_CMN.h>
#endif

#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
GT_U32 *C112GX4_main_Ptr;
GT_U32 C112GX4_main_Size;

GT_U32 *C112GX4_SERDES_CMN_Ptr;
GT_U32 C112GX4_SERDES_CMN_Size;

GT_U32 *C112GX4_SERDES_REF156MHz_SPDCHG0_LANE_Ptr;
GT_U32 C112GX4_SERDES_REF156MHz_SPDCHG0_LANE_Size;
#endif

#define HWS_PMD_LOG_ARRAY_DATA_SIZE 23

#ifndef MV_HWS_REDUCED_BUILD
#define ESTIMATE_EYE_DIM_COUNT 4
#endif

static GT_STATUS mvHwsComphyC112GX4SerdesRxAutoTuneStart
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL training
);

static GT_STATUS mvHwsComphyC112GX4SerdesManualRxConfig
(
    IN GT_U8                               devNum,
    IN GT_UOPT                             portGroup,
    IN GT_UOPT                             serdesNum,
    IN MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
);

static GT_STATUS mvHwsComphyC112GX4SerdesDbRxConfigGet
(
    IN  GT_U8                               devNum,
    IN  GT_UOPT                             portGroup,
    IN  GT_UOPT                             serdesNum,
    IN  MV_HWS_SERDES_SPEED                 baudRate,
    IN  MV_HWS_SERDES_DB_TYPE               dbType,
    OUT MV_HWS_SERDES_RX_CONFIG_DATA_UNT    *rxConfigPtr
);

static GT_STATUS mvHwsComphyC112GX4SerdesCtleLimitsWrite
(
    GT_U8                                  devNum,
    GT_U32                                 portGroup,
    GT_U32                                 serdesNum,
    GT_U8                                  val,
    GT_BOOL                                isCap1,
    GT_BOOL                                isMax
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    GT_U8 paramType /*res1/cap1*/, limitType /*min/max*/;
    GT_U32 regVal;
    MCESD_FIELD reservedOutputTx  = F_C112GX4R1P0_RESERVED_OUTPUT_TX;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /*      3.  RESERVED_INPUT_RX_LANE = register 0x2108[25:10]
            4.  RESERVED_INPUT_TX_LANE = register 0x200C[22:7]
            5.  Example Limit=0x18
            for ctle_cap1_min  =>  [0x18 | 0x0<<8 | 0x0<<9 | 0x1<<12 | 0x1<<14] <<10  = 0x1406000
              •   0x1406000 | (0x1<<2) | 0x3 = 0x1406007   RESERVED_INPUT_RX_LANE = 0x140 RESERVED_INPUT_TX_LANE = 0x6007
              •   0x1406000 | (0x1<<2) | 0x1 = 0x1406005   RESERVED_INPUT_RX_LANE = 0x140 RESERVED_INPUT_TX_LANE = 0x6005
            for ctle_cap1_max  =>  [0x18 | 0x0<<8 | 0x0<<9 | 0x2<<12 | 0x1<<14] <<10  = 0x1806000
              •   0x1406000 | (0x1<<2) | 0x3 = 0x1406007   RESERVED_INPUT_RX_LANE = 0x180 RESERVED_INPUT_TX_LANE = 0x6007
              •   0x1406000 | (0x1<<2) | 0x1 = 0x1406005   RESERVED_INPUT_RX_LANE = 0x180 RESERVED_INPUT_TX_LANE = 0x6005
            for ctle_Res1_min  =>  [0x18 | 0x0<<8 | 0x0<<9 | 0x1<<12 | 0x0<<14] <<10  = 0x406000
              •   0x1406000 | (0x1<<2) | 0x3 = 0x1406007   RESERVED_INPUT_RX_LANE = 0x40 RESERVED_INPUT_TX_LANE = 0x6007
              •   0x1406000 | (0x1<<2) | 0x1 = 0x1406005   RESERVED_INPUT_RX_LANE = 0x40 RESERVED_INPUT_TX_LANE = 0x6005
            for ctle_Res1_max  =>  [0x18 | 0x0<<8 | 0x0<<9 | 0x2<<12 | 0x0<<14] <<10  = 0x806000
              •   0x1406000 | (0x1<<2) | 0x3 = 0x1406007   RESERVED_INPUT_RX_LANE = 0x80 RESERVED_INPUT_TX_LANE = 0x6007
              •   0x1406000 | (0x1<<2) | 0x1 = 0x1406005   RESERVED_INPUT_RX_LANE = 0x80 RESERVED_INPUT_TX_LANE = 0x6005  */

    paramType = (isCap1) ? 0x1 : 0x0;
    limitType = (isMax) ? 0x2 : 0x1;

    regVal = (val | (limitType << 12) | (paramType << 14))<<10 ;

    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2108, 10, 16, (regVal>>16))); /*RESERVED_INPUT_RX_LANE = register 0x2108[25:10]*/
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x200c, 7, 16,  ((regVal&0xFFFF)|0x7) )); /*RESERVED_INPUT_TX_LANE = register 0x200C[22:7]*/
    MCESD_ATTEMPT(API_C112GX4_PollField(sDev, serdesInfo->internalLane, &reservedOutputTx, 1, 500));
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x200c, 7, 16,  ((regVal&0xFFFF)|0x5) )); /*RESERVED_INPUT_TX_LANE = register 0x200C[22:7]*/
    MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));

    return GT_OK;
}

static GT_STATUS mvHwsComphyC112GX4SerdesResetDfe
(
    GT_U8                                  devNum,
    GT_U32                                 portGroup,
    GT_U32                                 serdesNum
)
{
    MCESD_DEV_PTR sDev;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,NULL,NULL,&sDev));

    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x241c,(0x1<<2),(0x1<<2)));
    MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, DFE_RESET_DELAY));
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x241c,(0x1<<2),0));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesGrayCodeEnableSet
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
static GT_STATUS mvHwsComphyC112GX4SerdesGrayCodeEnableSet
(
    IN GT_U8    devNum,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  enable
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_SetGrayCode(sDev, serdesInfo->internalLane,
                                                   (enable) ? C112GX4_GRAY_CODE_ENABLE : C112GX4_GRAY_CODE_DISABLE,
                                                   (enable) ? C112GX4_GRAY_CODE_ENABLE : C112GX4_GRAY_CODE_DISABLE));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesGrayCodeEnableGet
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
static GT_STATUS mvHwsComphyC112GX4SerdesGrayCodeEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_UOPT  serdesNum,
    OUT GT_BOOL  *enablePtr
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    E_C112GX4_GRAY_CODE txGrayCode, rxGrayCode;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetGrayCode(sDev, serdesInfo->internalLane, &txGrayCode, &rxGrayCode));
    *enablePtr = (txGrayCode && rxGrayCode) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPreCodeEnableSet
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
static GT_STATUS mvHwsComphyC112GX4SerdesPreCodeEnableSet
(
    IN GT_U8    devNum,
    IN GT_UOPT  serdesNum,
    IN GT_BOOL  enable
)
{
    MCESD_DEV_PTR   sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_SetPreCode(sDev, serdesInfo->internalLane,
                                                   (enable) ? MCESD_TRUE : MCESD_FALSE,
                                                   (enable) ? MCESD_TRUE : MCESD_FALSE));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPreCodeEnableGet
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
static GT_STATUS mvHwsComphyC112GX4SerdesPreCodeEnableGet
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

    MCESD_CHECK_STATUS(API_C112GX4_GetPreCode(sDev, serdesInfo->internalLane, &txState, &rxState));
    *enablePtr = (txState && rxState) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

static GT_STATUS mvHwsComphyC112GX4SerdesOperation
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
    GT_U32          ctleLimitMinCap1 = 0xff, ctleLimitMaxCap1 = 0xff, ctleLimitMinRes1 = 0xff, ctleLimitMaxRes1 = 0xff;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MV_HWS_SERDES_RX_CONFIG_DATA_UNT   rxParameters;
    GT_STATUS rc;
    MCESD_U32                   cap1, cap2, res1, align90;
    MCESD_16                    trainingResult;

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
           CHECK_STATUS(mvHwsComphyC112GX4SerdesResetDfe(devNum, portGroup, serdesNum));
           break;

       case MV_HWS_PORT_SERDES_OPERATION_CHECK_CLAMPING_TRIGGERED_E:

           /* read dtl_clamping_triggered_lane */
           MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,15,1,resultPtr));
           break;

        case MV_HWS_PORT_SERDES_OPERATION_DEBUG_MODE_E:
        /* data[0] : 0 = NO_DEBUG_STATE, 1 = DEBUG_STATE */
        CHECK_STATUS(mvHwsComphyC112GX4SerdesGrayCodeEnableSet(devNum, serdesNum, (dataPtr[0] == 0) ? GT_TRUE : GT_FALSE));
        CHECK_STATUS(mvHwsComphyC112GX4SerdesPreCodeEnableSet(devNum, serdesNum, (dataPtr[0] == 1) ? GT_TRUE : GT_FALSE));
        break;

       case MV_HWS_PORT_SERDES_OPERATION_RES_CAP_SEL_TUNING_CDR_E:

           /* read cap1 and cap2*/
           MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL, &cap1));
           MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL, &cap2));
           if ((cap1 != 0) && (cap2 != 0))
           {
               cap1 = cap1 / 2;       /* cap1 will be 0x1F ,0xF , 0x7 ,0x3 ,0x1, 0,0*/
               cap2 = (2 * cap2) / 3; /* cap2 will be 0x0F ,0xA , 0x6 ,0x4 ,0x2, 1,0*/
               MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL, cap1));
               MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL, cap2));
           }
           else
           {
               /* write back default values of cap1 and cap2 */
               MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL, 0x1F));
               MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL, 0xF));

               /* tune res1 and back to default */
               MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RES1_SEL, &res1));
               res1 = (res1 < 0xF) ? (res1 + 1) : 0x5;
               MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RES1_SEL, res1));

               /* tune align90 from 60 to -180 in step of 60 */
               MCESD_CHECK_STATUS(API_C112GX4_GetAlign90(sDev, serdesInfo->internalLane, &align90, &trainingResult));
               trainingResult = (trainingResult > -180) ? (trainingResult - 60) : 60;  /* 60, 0, -60, -120 */
               MCESD_CHECK_STATUS(API_C112GX4_SetAlign90(sDev, serdesInfo->internalLane, trainingResult));
           }
           /* at this stage the training is only for DFE - start and stop training - might not complete but should not be a problem */
            CHECK_STATUS(mvHwsComphyC112GX4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE));
           break;

       case MV_HWS_PORT_SERDES_OPERATION_PRE_RX_TRAINING_E:

            rc = mvHwsComphyC112GX4SerdesDbRxConfigGet(devNum, 0, serdesNum, (MV_HWS_SERDES_SPEED) dataPtr[0] /*serdes speed*/, SERDES_DB_OVERRIDE_ONLY, &rxParameters);

            if (rc != GT_OK)
            {
                if (rc == GT_NOT_FOUND)/*No override params - set limits*/
                {
                    ctleLimitMinCap1 =  0;
                    ctleLimitMaxCap1 =  63;
                    ctleLimitMinRes1 =  0;
                    ctleLimitMaxRes1 =  15;
                }
                else
                {
                    return rc;/*Fail to get override parameters*/
                }
            }
            else
            {
                /*If min/max res1/cap1 set, and with valid range - config them; else set limits*/
                ctleLimitMinCap1 =  (rxParameters.rxComphyC112G.minCap1 < 64) ? rxParameters.rxComphyC112G.minCap1 : 0;
                ctleLimitMaxCap1 =  (rxParameters.rxComphyC112G.maxCap1 < 64) ? rxParameters.rxComphyC112G.maxCap1 : 63;
                ctleLimitMinRes1 =  (rxParameters.rxComphyC112G.minRes1 < 16) ? rxParameters.rxComphyC112G.minRes1 : 0;
                ctleLimitMaxRes1 =  (rxParameters.rxComphyC112G.maxRes1 < 16) ? rxParameters.rxComphyC112G.maxRes1 : 15;
            }

            /* 1.  Set RESERVED_INPUT_TX_FM_REG_LANE = 1 register 0x200C[6]
               2.  Set RESERVED_INPUT_TX_FM_REG_LANE = 1 register 0x2108[9]*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x200c, 6, 1, 1));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2108, 9, 1, 1));

            /*min cap 1*/
            CHECK_STATUS(mvHwsComphyC112GX4SerdesCtleLimitsWrite(devNum, portGroup, serdesNum, ctleLimitMinCap1, GT_TRUE, GT_FALSE));
            /*max cap 1*/
            CHECK_STATUS(mvHwsComphyC112GX4SerdesCtleLimitsWrite(devNum, portGroup, serdesNum, ctleLimitMaxCap1, GT_TRUE, GT_TRUE));
            /*min res 1*/
            CHECK_STATUS(mvHwsComphyC112GX4SerdesCtleLimitsWrite(devNum, portGroup, serdesNum, ctleLimitMinRes1, GT_FALSE, GT_FALSE));
            /*max res 1*/
            CHECK_STATUS(mvHwsComphyC112GX4SerdesCtleLimitsWrite(devNum, portGroup, serdesNum, ctleLimitMaxRes1, GT_FALSE, GT_TRUE));

            /* before training we want to set the rx serdes params values to default */
            CHECK_STATUS(mvHwsComphyC112GX4SerdesDbRxConfigGet(devNum, 0, serdesNum, (MV_HWS_SERDES_SPEED) dataPtr[0] /*serdes speed*/, SERDES_DB_OVERRIDE_AND_DEFAULT, &rxParameters));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesManualRxConfig(devNum, 0, serdesNum, &rxParameters));
            /* align90 back to default value */
            MCESD_CHECK_STATUS(API_C112GX4_SetAlign90(sDev, serdesInfo->internalLane, 0));
            /* reset DFE */
            CHECK_STATUS(mvHwsComphyC112GX4SerdesResetDfe(devNum, portGroup, serdesNum));
            break;

       default:
           return GT_OK;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesEncodingTypeGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesEncodingTypeGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           serdesNum,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *txEncodingPtr,
    OUT MV_HWS_SERDES_ENCODING_TYPE     *rxEncodingPtr
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_FIELD txPAM2En = F_C112GX4R1P0_TX_PAM2_EN;
    MCESD_FIELD rxPAM2En = F_C112GX4R1P0_RX_PAM2_EN;
    MCESD_U32 data;
    MCESD_BOOL txReady, rxReady;

    if( NULL == txEncodingPtr || NULL == rxEncodingPtr )
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* check if serdes powered-up */
    MCESD_ATTEMPT(API_C112GX4_GetTxRxReady(sDev, serdesInfo->internalLane, &txReady, &rxReady));

    if ( txReady == MCESD_FALSE )
    {
        *txEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(sDev, serdesInfo->internalLane, &txPAM2En, &data));
        *txEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    if ( rxReady == MCESD_FALSE )
    {
        *rxEncodingPtr = SERDES_ENCODING_NA;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_ReadField(sDev, serdesInfo->internalLane, &rxPAM2En, &data));
        *rxEncodingPtr = (1 == data) ? SERDES_ENCODING_NRZ : SERDES_ENCODING_PAM4;
    }

    return GT_OK;
}

#if !defined(AC5P_DEV_SUPPORT)
/**
* @internal mvHwsComphyC112GX4GetDefaultParameters function
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
GT_STATUS mvHwsComphyC112GX4GetDefaultParameters
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
        hwsOsMemCopyFuncPtr(&txDefaultParameters->txComphyC112G,
                            &tuneParams[baudRateIterator].txParams.txComphyC112G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemCopyFuncPtr(&rxDefaultParameters->rxComphyC112G,
                            &tuneParams[baudRateIterator].rxParams.rxComphyC112G,
                            sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
    }
#else
    GT_UNUSED_PARAM(serdesInfo);
    GT_UNUSED_PARAM(serdesData);
    GT_UNUSED_PARAM(baudRateIterator);

    if(txDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&txDefaultParameters->txComphyC112G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
    }

    if(rxDefaultParameters != NULL)
    {
        hwsOsMemSetFuncPtr(&rxDefaultParameters->rxComphyC112G, 0, sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
    }

#endif
    return GT_OK;
}
#endif

/**
* @internal mvHwsComphyC112GX4SerdesDbTxConfigGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesDbTxConfigGet
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
                hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC112G),
                                    &(serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC112G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
            }
            break;
        case SERDES_DB_OVERRIDE_AND_DEFAULT:
            if (!overrideExist)/*override param not found - set default param*/
            {
                hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC112G),
                                    &(tuneParams[defaultParamIterator].txParams.txComphyC112G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
            }
            else/*override param exist - go over tx params and set override value if exist or default if not exist*/
            {
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC112G.pre2,  tuneParams[defaultParamIterator].txParams.txComphyC112G.pre2, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC112G.pre2);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC112G.pre,   tuneParams[defaultParamIterator].txParams.txComphyC112G.pre,  serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC112G.pre);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC112G.main,  tuneParams[defaultParamIterator].txParams.txComphyC112G.main, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC112G.main);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_SIGNED_VAL(txConfigPtr->txComphyC112G.post,  tuneParams[defaultParamIterator].txParams.txComphyC112G.post, serdesData->tuneOverrideParams[speedIdx].txParams.txComphyC112G.post);
            }
            break;
        case SERDES_DB_DEFAULT_ONLY:
            hwsOsMemCopyFuncPtr(&(txConfigPtr->txComphyC112G),
                                    &(tuneParams[defaultParamIterator].txParams.txComphyC112G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_TX_CONFIG_DATA));
            break;
        default:
            rc = GT_NOT_INITIALIZED;
            break;
    }

    return rc;
}

/**
* @internal mvHwsComphyC112GX4SerdesDbRxConfigGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesDbRxConfigGet
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
                hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC112G),
                                    &(serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
            }
            break;
        case SERDES_DB_OVERRIDE_AND_DEFAULT:
            if (!overrideExist) /*override param not found - set default param*/
            {
                hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC112G),
                                    &(tuneParams[defaultParamIterator].rxParams.rxComphyC112G),
                                    sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
            }
            else/*override param exist - go over rx params and set override value if exist or default if not exist*/
            {
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.current1Sel,       tuneParams[defaultParamIterator].rxParams.rxComphyC112G.current1Sel,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.current1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rl1Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rl1Sel,            serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rl1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rl1Extra,          tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rl1Extra,          serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rl1Extra);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.res1Sel,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.res1Sel,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.res1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.cap1Sel,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.cap1Sel,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.cap1Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.cl1Ctrl,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.cl1Ctrl,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.cl1Ctrl);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.enMidFreq,         tuneParams[defaultParamIterator].rxParams.rxComphyC112G.enMidFreq,         serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.enMidFreq);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.cs1Mid,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.cs1Mid,            serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.cs1Mid);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rs1Mid,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rs1Mid,            serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rs1Mid);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rfCtrl,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rfCtrl,            serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rfCtrl);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rl1TiaSel,         tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rl1TiaSel,         serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rl1TiaSel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rl1TiaExtra,       tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rl1TiaExtra,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rl1TiaExtra);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.hpfRSel1st,        tuneParams[defaultParamIterator].rxParams.rxComphyC112G.hpfRSel1st,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.hpfRSel1st);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.current1TiaSel,    tuneParams[defaultParamIterator].rxParams.rxComphyC112G.current1TiaSel,    serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.current1TiaSel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rl2Tune,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rl2Tune,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rl2Tune);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rl2Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rl2Sel,            serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rl2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.rs2Sel,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.rs2Sel,            serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.rs2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.current2Sel,       tuneParams[defaultParamIterator].rxParams.rxComphyC112G.current2Sel,       serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.current2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.cap2Sel,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.cap2Sel,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.cap2Sel);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.hpfRsel2nd,        tuneParams[defaultParamIterator].rxParams.rxComphyC112G.hpfRsel2nd,        serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.hpfRsel2nd);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.selmufi,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.selmufi,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.selmufi);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.selmuff,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.selmuff,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.selmuff);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.selmupi,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.selmupi,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.selmupi);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.selmupf,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.selmupf,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.selmupf);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.minCap1,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.minCap1,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.minCap1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.maxCap1,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.maxCap1,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.maxCap1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.minRes1,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.minRes1,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.minRes1);
                HWS_COMPHY_CHECK_AND_SET_DB_16BIT_VAL(rxConfigPtr->rxComphyC112G.maxRes1,           tuneParams[defaultParamIterator].rxParams.rxComphyC112G.maxRes1,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.maxRes1);
                HWS_COMPHY_CHECK_AND_SET_DB_8BIT_VAL(rxConfigPtr->rxComphyC112G.squelch,            tuneParams[defaultParamIterator].rxParams.rxComphyC112G.squelch,           serdesData->tuneOverrideParams[speedIdx].rxParams.rxComphyC112G.squelch);
            }
            break;
        case SERDES_DB_DEFAULT_ONLY:
            hwsOsMemCopyFuncPtr(&(rxConfigPtr->rxComphyC112G),
                                &(tuneParams[defaultParamIterator].rxParams.rxComphyC112G),
                                sizeof(MV_HWS_MAN_TUNE_COMPHY_C112G_RX_CONFIG_DATA));
            break;
        default:
            rc = GT_NOT_INITIALIZED;
            break;
    }

    return rc;
}

static MCESD_STATUS API_C112GX4_SetSignedTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    IN MCESD_32 paramValue
)
{
    E_C112GX4_POLARITY polarity = C112GX4_POLARITY_INVERTED;

    if ( param != C112GX4_TXEQ_EM_MAIN_CTRL )
    {
        polarity =  (paramValue < 0) ? C112GX4_POLARITY_INVERTED: C112GX4_POLARITY_NORMAL;
    }
    MCESD_CHECK_STATUS(API_C112GX4_SetTxEqPolarity(devPtr, lane, param, polarity));
    MCESD_ATTEMPT(API_C112GX4_SetTxEqParam(devPtr, lane, param, ABS(paramValue)));
    return MCESD_OK;
}

static MCESD_STATUS API_C112GX4_GetSignedTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_C112GX4_TXEQ_PARAM param,
    OUT MCESD_32 *paramValue
)
{
    MCESD_U32 val;
    E_C112GX4_POLARITY inverted;

    MCESD_ATTEMPT(API_C112GX4_GetTxEqParam(devPtr, lane, param, &val));
    if ( param == C112GX4_TXEQ_EM_MAIN_CTRL )
    {
        *paramValue = (MCESD_32)val;
    }
    else
    {
        MCESD_ATTEMPT(API_C112GX4_GetTxEqPolarity(devPtr, lane, param, &inverted));
        *paramValue = inverted ? (((MCESD_32)val)*(-1)): ((MCESD_32)val);
    }
    return MCESD_OK;
}

static MCESD_STATUS API_C112GX4_SetForceTxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
)
{
    MCESD_FIELD txFir  = F_C112GX4R1P0_TX_FIR_TAP_POL_F;

    MCESD_ATTEMPT(API_C112GX4_WriteField(devPtr, lane, &txFir, state));

    return MCESD_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesManualTxConfig function
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
static GT_STATUS mvHwsComphyC112GX4SerdesManualTxConfig
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

    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.pre2)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE2_CTRL, (MCESD_32)txConfigPtr->txComphyC112G.pre2));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.pre)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE_CTRL,  (MCESD_32)txConfigPtr->txComphyC112G.pre));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.post)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_POST_CTRL, (MCESD_32)txConfigPtr->txComphyC112G.post));
    if(NA_8BIT_SIGNED != txConfigPtr->txComphyC112G.main)
        MCESD_CHECK_STATUS(API_C112GX4_SetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_MAIN_CTRL, (MCESD_32)txConfigPtr->txComphyC112G.main));
    /** Static configurations */

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesManualRxConfig function
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
static GT_STATUS mvHwsComphyC112GX4SerdesManualRxConfig
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
    /* 1st stage GM Main */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.current1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC112G.current1Sel));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_SEL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1Sel        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1Extra)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_EXTRA,        (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1Extra      ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.res1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RES1_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.res1Sel       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cap1Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.cap1Sel       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cl1Ctrl)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CL1_CTRL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.cl1Ctrl       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.enMidFreq)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_EN_MID_FREQ,      (MCESD_U32)rxConfigPtr->rxComphyC112G.enMidFreq     ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cs1Mid)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CS1_MID,          (MCESD_U32)rxConfigPtr->rxComphyC112G.cs1Mid        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rs1Mid)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS1_MID,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rs1Mid        ));
    /* 1st stage TIA */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rfCtrl)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RF_CTRL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rfCtrl        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1TiaSel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_SEL,      (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1TiaSel     ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl1TiaExtra)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_EXTRA,    (MCESD_U32)rxConfigPtr->rxComphyC112G.rl1TiaExtra   ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.hpfRSel1st)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_1ST,     (MCESD_U32)rxConfigPtr->rxComphyC112G.hpfRSel1st    ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.current1TiaSel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_TIA_SEL, (MCESD_U32)rxConfigPtr->rxComphyC112G.current1TiaSel));
    /* 2nd Stage */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl2Tune)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_TUNE,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rl2Tune       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rl2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.rl2Sel        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.rs2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS2_SEL,          (MCESD_U32)rxConfigPtr->rxComphyC112G.rs2Sel        ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.current2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT2_SEL,     (MCESD_U32)rxConfigPtr->rxComphyC112G.current2Sel   ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.cap2Sel)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL,         (MCESD_U32)rxConfigPtr->rxComphyC112G.cap2Sel       ));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.hpfRsel2nd)
        MCESD_CHECK_STATUS(API_C112GX4_SetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_2ND,     (MCESD_U32)rxConfigPtr->rxComphyC112G.hpfRsel2nd    ));

    /* Advanced (CDR) */
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmufi)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFI, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmufi));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmuff)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFF, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmuff));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmupi)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPI, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmupi));
    if(NA_16BIT != rxConfigPtr->rxComphyC112G.selmupf)
        MCESD_CHECK_STATUS(API_C112GX4_SetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPF, (MCESD_U32)rxConfigPtr->rxComphyC112G.selmupf));

    if(NA_8BIT != rxConfigPtr->rxComphyC112G.squelch)
        MCESD_CHECK_STATUS(API_C112GX4_SetSquelchThreshold(sDev, serdesInfo->internalLane, (MCESD_U8)rxConfigPtr->rxComphyC112G.squelch));

    MCESD_CHECK_STATUS(API_C112GX4_ExecuteCDS(sDev, serdesInfo->internalLane));

    /** Static configurations */

    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x2170, (0xFFFF << 16), 0x3FF << 16));

    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum, 0x2170, (0x1 << 12), 0x1   << 12));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesReset function
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
static GT_STATUS mvHwsComphyC112GX4SerdesReset
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
* @internal mvHwsComphyC112GX4SerdesCoreReset function
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
static GT_STATUS mvHwsComphyC112GX4SerdesCoreReset
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
* @internal mvHwsComphyC112GX4SerdesArrayPowerCtrl function
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
static GT_STATUS mvHwsComphyC112GX4SerdesArrayPowerCtrl
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
    E_C112GX4_SERDES_SPEED      speed = 0;
    E_C112GX4_REFFREQ           refFreq = 0;
    E_C112GX4_REFCLK_SEL        refClkSel = 0;
    E_C112GX4_DATABUS_WIDTH     dataBusWidth = 0;
    S_C112GX4_PowerOn           powerOnConfig;
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
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_SERDES_SPEED,     (GT_U32)serdesConfigPtr->baudRate,       &mcesdEnum));
            speed = (E_C112GX4_SERDES_SPEED)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_FREQ,   (GT_U32)serdesConfigPtr->refClock,       &mcesdEnum));
            refFreq = (E_C112GX4_REFFREQ)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_REF_CLOCK_SOURCE, (GT_U32)serdesConfigPtr->refClockSource, &mcesdEnum));
            refClkSel = (E_C112GX4_REFCLK_SEL)mcesdEnum;
            CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_DATA_BUS_WIDTH,   (GT_U32)serdesConfigPtr->busWidth,       &mcesdEnum));
            dataBusWidth = (E_C112GX4_DATABUS_WIDTH)mcesdEnum;
            comphyPrintf("--[%s]--[%d]-serdesNum[%d]-speed[%d]-refFreq[%d]-refClkSel[%d]-dataBusWidth[%d]-\n",
                         __func__,__LINE__,serdesArr[serdesIterator],speed,refFreq,refClkSel,dataBusWidth);

            CHECK_STATUS(mvHwsComphyC112GX4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_TRUE, GT_TRUE));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesCoreReset(devNum, portGroup, serdesArr[serdesIterator], UNRESET));

            powerOnConfig.powerLaneMask = 1 << serdesInfo->internalLane;
            powerOnConfig.initTx        = MCESD_TRUE;
            powerOnConfig.initRx        = MCESD_TRUE;
            powerOnConfig.txOutputEn    = MCESD_FALSE;
            powerOnConfig.dataPath      = C112GX4_PATH_EXTERNAL;
            powerOnConfig.downloadFw    = MCESD_FALSE;

            powerOnConfig.refClkSel     = refClkSel;
            powerOnConfig.dataBusWidth  = dataBusWidth;
            powerOnConfig.speed         = speed;
            powerOnConfig.refFreq       = refFreq;

            MCESD_CHECK_STATUS(API_C112GX4_PowerOnSeq(sDev, powerOnConfig));
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
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
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
            CHECK_STATUS(mvHwsComphyC112GX4GetDefaultParameters(devNum, serdesArr[serdesIterator], baudRate, &txDefaultParameters, &rxDefaultParameters));
#endif
            CHECK_STATUS(mvHwsComphyC112GX4SerdesDbTxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &txParameters));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesDbRxConfigGet(devNum, 0, serdesArr[serdesIterator], baudRate, SERDES_DB_OVERRIDE_AND_DEFAULT, &rxParameters));

            CHECK_STATUS(mvHwsComphyC112GX4SerdesManualTxConfig(devNum, 0, serdesArr[serdesIterator], &txParameters));
            CHECK_STATUS(mvHwsComphyC112GX4SerdesManualRxConfig(devNum, 0, serdesArr[serdesIterator], &rxParameters));

            CHECK_STATUS(mvHwsComphyC112GX4SerdesReset(devNum, portGroup, serdesArr[serdesIterator], GT_FALSE, GT_FALSE, GT_FALSE));
            MCESD_CHECK_STATUS(API_C112GX4_SetTxOutputEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));
        }
    }
    else /* Power-Down */
    {
        for(serdesIterator = 0 ; serdesIterator < numOfSer ; serdesIterator++)
        {
            CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesArr[serdesIterator],&serdesInfo,NULL,&sDev));
            MCESD_CHECK_STATUS(API_C112GX4_SetForceTxPolarity(sDev, serdesInfo->internalLane, MCESD_FALSE));

            /* if user enabled PRBS and deleted the port (SerDes is powered-down), need to restore to normal mode,
               in order to enable port to reach link up in next power-up */
            MCESD_CHECK_STATUS(API_C112GX4_StopPhyTest(sDev, serdesInfo->internalLane));

            MCESD_CHECK_STATUS(API_C112GX4_PowerOffLane(sDev, serdesInfo->internalLane));
            /* freq offset read request */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<16),(0x0<<16)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 2));
            /* freq offset reset*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<19),(0x1<<19)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, PLL_INIT_POLLING_DELAY));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<19),0));
            /* freq offset disable*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesArr[serdesIterator],0x2160,(0x1<<20),(0x0<<20)));
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPowerCtrl function
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
static GT_STATUS mvHwsComphyC112GX4SerdesPowerCtrl
(
    IN  GT_U8                       devNum,
    IN  GT_UOPT                     portGroup,
    IN  GT_UOPT                     serdesNum,
    IN  GT_BOOL                     powerUp,
    IN  MV_HWS_SERDES_CONFIG_STC    *serdesConfigPtr
)
{
    return mvHwsComphyC112GX4SerdesArrayPowerCtrl(devNum, portGroup, 1, &serdesNum, powerUp, serdesConfigPtr);
}

/**
* @internal mvHwsComphyC112GX4SerdesRxAutoTuneStart function
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
static GT_STATUS mvHwsComphyC112GX4SerdesRxAutoTuneStart
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
        MCESD_CHECK_STATUS(API_C112GX4_StartTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C112GX4_StopTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxAutoTuneStart function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTxAutoTuneStart
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
        MCESD_CHECK_STATUS(API_C112GX4_StartTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_TRX));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C112GX4_StopTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_TRX));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxAutoTuneStop function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTxAutoTuneStop

(
    GT_U8    devNum,
    GT_UOPT  portGroup,
    GT_UOPT  serdesNum
)
{
    return mvHwsComphyC112GX4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_FALSE);
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneStart function
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
static GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneStart
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
        CHECK_STATUS(mvHwsComphyC112GX4SerdesRxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    if(txTraining)
    {
        CHECK_STATUS(mvHwsComphyC112GX4SerdesTxAutoTuneStart(devNum,portGroup,serdesNum,GT_TRUE));
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneStatus function
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
static GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneStatus
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
        MCESD_CHECK_STATUS(API_C112GX4_CheckTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_RX, &completed, &failed));
        *rxStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C112GX4_CheckTraining(sDev, serdesInfo->internalLane, C112GX4_TRAINING_TRX, &completed, &failed));
        *txStatus = completed? (failed ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneStatusShort function
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
static GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneStatusShort
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
        MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_RX_TRAIN_COMPLETE0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_RX_TRAIN_FAILED0 + serdesInfo->internalLane, &failedData));
        }
        *rxStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    if(NULL != txStatus)
    {
        MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_TX_TRAIN_COMPLETE0 + serdesInfo->internalLane, &completeData));
        if (1 == completeData)
        {
            MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_TX_TRAIN_FAILED0 + serdesInfo->internalLane, &failedData));
        }
        *txStatus = completeData? (failedData ? TUNE_FAIL : TUNE_PASS) : TUNE_NOT_COMPLITED;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesAutoTuneResult function
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
static GT_STATUS mvHwsComphyC112GX4SerdesAutoTuneResult
(
    GT_U8                                devNum,
    GT_UOPT                              portGroup,
    GT_UOPT                              serdesNum,
    MV_HWS_SERDES_AUTO_TUNE_RESULTS_UNT *tuneResults
)
{
    MV_HWS_PER_SERDES_INFO_PTR              serdesInfo;
    MCESD_DEV_PTR                           sDev;
    MV_HWS_COMPHY_C112G_AUTO_TUNE_RESULTS  *results;
    E_C112GX4_DATA_ACQ_RATE acqRate;

    MCESD_U32 val;
    MCESD_32  vals;
    MCESD_U8  val8;
    MCESD_16  vals16;
    MCESD_U32 tapsI;
    MCESD_32 txParam;

    GT_UNUSED_PARAM(portGroup);

    if(NULL == tuneResults)
    {
        return GT_BAD_PARAM;
    }
    results = &tuneResults->comphyC112GResults;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE2_CTRL, &txParam));
    results->txComphyC112G.pre2 = (GT_8)txParam;
    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_PRE_CTRL,  &txParam));
    results->txComphyC112G.pre  = (GT_8)txParam;
    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_MAIN_CTRL, &txParam));
    results->txComphyC112G.main = (GT_8)txParam;
    MCESD_CHECK_STATUS(API_C112GX4_GetSignedTxEqParam(sDev, serdesInfo->internalLane, C112GX4_TXEQ_EM_POST_CTRL, &txParam));
    results->txComphyC112G.post = (GT_8)txParam;

    /* Basic (CTLE) */
    /* 1st stage GM Main */
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_SEL, &val));
    results->rxComphyC112G.current1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_SEL,      &val));
    results->rxComphyC112G.rl1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_EXTRA,    &val));
    results->rxComphyC112G.rl1Extra= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RES1_SEL,     &val));
    results->rxComphyC112G.res1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP1_SEL,     &val));
    results->rxComphyC112G.cap1Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CL1_CTRL,     &val));
    results->rxComphyC112G.cl1Ctrl= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_EN_MID_FREQ,  &val));
    results->rxComphyC112G.enMidFreq= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CS1_MID,      &val));
    results->rxComphyC112G.cs1Mid= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS1_MID,      &val));
    results->rxComphyC112G.rs1Mid= (GT_U32)val;
    /* 1st stage TIA */
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RF_CTRL,          &val));
    results->rxComphyC112G.rfCtrl= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_SEL,      &val));
    results->rxComphyC112G.rl1TiaSel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL1_TIA_EXTRA,    &val));
    results->rxComphyC112G.rl1TiaExtra= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_1ST,     &val));
    results->rxComphyC112G.hpfRSel1st= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT1_TIA_SEL, &val));
    results->rxComphyC112G.current1TiaSel= (GT_U32)val;
    /* 2nd Stage */
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_SEL,      &val));
    results->rxComphyC112G.rl2Tune= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RL2_TUNE,     &val));
    results->rxComphyC112G.rl2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_RS2_SEL,      &val));
    results->rxComphyC112G.rs2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CURRENT2_SEL, &val));
    results->rxComphyC112G.current2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_CAP2_SEL,     &val));
    results->rxComphyC112G.cap2Sel= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCTLEParam(sDev, serdesInfo->internalLane, C112GX4_CTLE_HPF_RSEL_2ND, &val));
    results->rxComphyC112G.hpfRsel2nd= (GT_U32)val;

    /* Advanced (CDR) */
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFI,  &val));
    results->rxComphyC112G.selmufi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUFF,  &val));
    results->rxComphyC112G.selmuff= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPI,  &val));
    results->rxComphyC112G.selmupi= (GT_U32)val;
    MCESD_CHECK_STATUS(API_C112GX4_GetCDRParam(sDev, serdesInfo->internalLane, C112GX4_CDR_SELMUPF,  &val));
    results->rxComphyC112G.selmupf= (GT_U32)val;

    MCESD_CHECK_STATUS(API_C112GX4_GetSquelchThreshold(sDev, serdesInfo->internalLane, &val8));
    results->rxComphyC112G.squelch= (MCESD_U8)val8;

    MCESD_CHECK_STATUS(API_C112GX4_GetAlign90(sDev, serdesInfo->internalLane, &val, &vals16));
    results->align90AnaReg = (GT_U32)val;
    results->align90 = (GT_16)vals16;

    results->eo = 0;
    results->sampler = 0;
    results->slewRateCtrl0 = 0;
    results->slewRateCtrl1 = 0;

    /* Get Data Acquisition Rate */
    MCESD_ATTEMPT(API_C112GX4_GetDataAcquisitionRate(sDev, serdesInfo->internalLane, &acqRate));

    for(tapsI = 0 ; tapsI < C112GX4_DFE_FF7 ; tapsI++)
    {
        /* when we are in full rate, DFE8-15 are not supported */
        if (acqRate == C112GX4_RATE_FULL)
        {
            if ((tapsI >= C112GX4_DFE_F8) && (tapsI <= C112GX4_DFE_F15))
            {
                results->dfe[tapsI] = 0;
                continue;
            }
        }
        if (MCESD_OK == API_C112GX4_GetDfeTap(sDev, serdesInfo->internalLane, C112GX4_EYE_MID, (E_C112GX4_DFE_TAP)tapsI, &vals))
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
* @internal mvHwsComphyC112GX4SerdesDigitalReset function
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
static GT_STATUS mvHwsComphyC112GX4SerdesDigitalReset
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
* @internal mvHwsComphyC112GX4SerdesLoopback function
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
static GT_STATUS mvHwsComphyC112GX4SerdesLoopback
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C112GX4_DATAPATH          path;
    GT_U32                      mcesdEnum;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)lbMode, &mcesdEnum));
    path = (E_C112GX4_DATAPATH)mcesdEnum;

    MCESD_CHECK_STATUS(API_C112GX4_SetDataPath(sDev, serdesInfo->internalLane, path));

    switch ( lbMode )
    {
    case SERDES_LP_AN_TX_RX:
        /* Masks sq_detected indication. If set, SQ detect is always zero (signal detected). Masks sq_detected indication */
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (1 << 25), (1 << 25)));
        break;
    default:
        CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, portGroup, EXTERNAL_REG, serdesNum, SDW_LANE_CONTROL_0, (0 << 25), (1 << 25)));
        break;
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesLoopbackGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesLoopbackGet
(
    GT_U8               devNum,
    GT_UOPT             portGroup,
    GT_UOPT             serdesNum,
    MV_HWS_SERDES_LB    *lbMode
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    E_C112GX4_DATAPATH          path;
    GT_U32                      tempLbMode;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetDataPath(sDev, serdesInfo->internalLane, &path));
    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_LOOPBACK, (GT_U32)path, &tempLbMode));
    *lbMode = (MV_HWS_SERDES_LB)tempLbMode;
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTestGen function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTestGen
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
    E_C112GX4_PATTERN             pattern;
    const char                    *userPattern;
    const char                    patternEmpty[] = {'\0'};
    const char                    pattern1T[]  = {0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,0xA,'\0'};
    const char                    pattern2T[]  = {0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,0xC,'\0'};
    const char                    pattern5T[]  = {0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,0x7,0xC,0x1,0xF,'\0'};
    const char                    pattern10T[] = {0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,0x3,0xF,0xF,'\0'};
    GT_U32                        mcesdPattern;
    MV_HWS_SERDES_ENCODING_TYPE   txEncoding, rxEncoding;
    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    CHECK_STATUS(mvHwsComphyConvertHwsToMcesdType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U8)txPattern, &mcesdPattern));
    pattern = (E_C112GX4_PATTERN)mcesdPattern;

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
    MCESD_CHECK_STATUS(API_C112GX4_SetTxRxPattern(sDev, serdesInfo->internalLane, pattern, pattern, userPattern));

    /* PT_PRBS_INV_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x2304, 23, 1, 0x0));

    /* PT_PRBS_INV_RX_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, 0, serdesNum, 0x2304, 21, 1, 0x0));

    CHECK_STATUS(mvHwsComphyC112GX4SerdesEncodingTypeGet(devNum,serdesNum,&txEncoding,&rxEncoding));

    if(SERDES_TEST == mode)
    {
        MCESD_CHECK_STATUS(API_C112GX4_ResetComparatorStats(sDev, serdesInfo->internalLane));
        MCESD_CHECK_STATUS(API_C112GX4_StartPhyTest(sDev, serdesInfo->internalLane));
    }
    else
    {
        MCESD_CHECK_STATUS(API_C112GX4_StopPhyTest(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTestGenGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTestGenGet
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
    E_C112GX4_PATTERN                 txPattern;
    E_C112GX4_PATTERN                 rxPattern;
    char                              userPattern[21];
    MCESD_FIELD                       ptEn = F_C112GX4R1P0_PT_EN;
    MCESD_U32                         enable;
    GT_U32                            tempTxPattern;
    GT_UNUSED_PARAM(portGroup);

    if((NULL == txPatternPtr) || (NULL == modePtr))
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    MCESD_CHECK_STATUS(API_C112GX4_GetTxRxPattern(sDev, serdesInfo->internalLane, &txPattern, &rxPattern, userPattern));
    MCESD_CHECK_STATUS(API_C112GX4_ReadField(sDev, serdesInfo->internalLane, &ptEn, &enable));

    CHECK_STATUS(mvHwsComphyConvertMcesdToHwsType(devNum, COM_PHY_C112GX4, MV_HWS_COMPHY_MCESD_ENUM_PATTERN, (GT_U32)txPattern, &tempTxPattern));

    *txPatternPtr   = (MV_HWS_SERDES_TX_PATTERN)tempTxPattern;
    *modePtr        = enable ? SERDES_TEST : SERDES_NORMAL;

    return GT_OK;
}
/**
* @internal mvHwsComphyC112GX4SerdesTestGenStatus
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
static GT_STATUS mvHwsComphyC112GX4SerdesTestGenStatus
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
    S_C112GX4_PATTERN_STATISTICS     statistics;
    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(txPattern);

    if(NULL == status)
    {
        return GT_BAD_PTR;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    C112GX4_WRITE_FIELD(sDev, F_C112GX4R1P0_PT_PRBS_LOAD, serdesInfo->internalLane, 1);
    MCESD_ATTEMPT(API_C112GX4_Wait(sDev, 1));
    MCESD_CHECK_STATUS(API_C112GX4_GetComparatorStats(sDev, serdesInfo->internalLane, &statistics));
    C112GX4_WRITE_FIELD(sDev, F_C112GX4R1P0_PT_PRBS_LOAD, serdesInfo->internalLane, 0);

    status->lockStatus   = (GT_U32)statistics.lock;
    status->errorsCntr.l[0]   = (GT_U32) (statistics.totalErrorBits & 0x00000000FFFFFFFFUL);
    status->errorsCntr.l[1]   = (GT_U32)((statistics.totalErrorBits & 0xFFFFFFFF00000000UL) >> 32);
    status->txFramesCntr.l[0] = (GT_U32) (statistics.totalBits & 0x00000000FFFFFFFFUL);
    status->txFramesCntr.l[1] = (GT_U32)((statistics.totalBits & 0xFFFFFFFF00000000UL) >> 32);
    if(!counterAccumulateMode)
    {
        MCESD_CHECK_STATUS(API_C112GX4_ResetComparatorStats(sDev, serdesInfo->internalLane));
    }

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPolarityConfig function
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
static GT_STATUS mvHwsComphyC112GX4SerdesPolarityConfig
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

    MCESD_CHECK_STATUS(API_C112GX4_SetTxRxPolarity(sDev, serdesInfo->internalLane, invertTx ? C112GX4_POLARITY_INVERTED : C112GX4_POLARITY_NORMAL,
                                                        invertRx ? C112GX4_POLARITY_INVERTED : C112GX4_POLARITY_NORMAL));
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesPolarityConfigGet
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
static GT_STATUS mvHwsComphyC112GX4SerdesPolarityConfigGet
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
    E_C112GX4_POLARITY         txPolarity, rxPolarity;

    GT_UNUSED_PARAM(portGroup);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetTxRxPolarity(sDev, serdesInfo->internalLane, &txPolarity, &rxPolarity));

    *invertTx = (txPolarity == C112GX4_POLARITY_INVERTED);
    *invertRx = (rxPolarity == C112GX4_POLARITY_INVERTED);

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxEnbale function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTxEnbale
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

    MCESD_CHECK_STATUS(API_C112GX4_SetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)txEnable));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTxEnbale function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTxEnbaleGet
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

    MCESD_CHECK_STATUS(API_C112GX4_GetTxOutputEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL*)txEnable));

    return GT_OK;
}


/**
* @internal mvHwsComphyC112GX4SerdesSignalDetectGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesSignalDetectGet
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

    MCESD_CHECK_STATUS(API_C112GX4_GetSquelchDetect(sDev, serdesInfo->internalLane, &squelched));
    *enable = squelched ? GT_FALSE : GT_TRUE;

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesCdrLockStatusGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesCdrLockStatusGet
(
    GT_U8                   devNum,
    GT_UOPT                 portGroup,
    GT_UOPT                 serdesNum,
    GT_BOOL                 *lock
)
{
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    MCESD_DEV_PTR               sDev;
    GT_U32                      data = 0, freqReady = 0, dtlClamping, countConsecutive = 0, startSec, startNanoSec, currentSec, currentNanoSec;
    GT_16                       dataSigned = 0,dataSigned2 = 0, diff = 0;
    MCESD_U32                   completed;
    GT_BOOL                     timeout = GT_FALSE, state = GT_FALSE, updateStateAndCount = GT_FALSE;
    MV_HWS_SERDES_LB            lbMode;

    *lock = GT_FALSE;
    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum, serdesNum, &serdesInfo, NULL, &sDev));

    CHECK_STATUS(hwsTimerGetFuncPtr(&startSec, &startNanoSec));

    MCESD_CHECK_STATUS(mvHwsComphyC112GX4SerdesSignalDetectGet(devNum, portGroup, serdesNum, lock));
    if (*lock == GT_FALSE)
    {
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphyC112GX4SerdesLoopbackGet(devNum, portGroup, serdesNum, &lbMode));
    if (lbMode != SERDES_LP_DISABLE)
    {
        *lock = GT_TRUE;
        return GT_OK;
    }

    /* check if we're in a middle of rx train */
    MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, C112GX4_PIN_RX_TRAIN_ENABLE0, &completed));
    if (completed == 1)
    {
        *lock = GT_TRUE;
        return GT_OK;
    }

    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x2160,13, 1, 0x0)); /* DTL_SQ_DET_EN_LANE */
    MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));

    /* RX TRAINING ONLY DFE = 1, the next training will be DFE training */
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x6038,23, 1, 0x1));

    /* check 4 consecutive same result during MV_HWS_SERDES_CDR_LOCK_TIMEOUT */
    while (!timeout)
    {
        CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,15, 1, &dtlClamping)); /* dtl_clamping_triggered_lane */
        if (dtlClamping == 0x1)
        {
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x2160,12, 1, 0x1));  /* clear dtl_clamping */
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));
            CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x2160,12, 1, 0x0));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, DTL_CLAMPING_DELAY));
            CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,15, 1, &dtlClamping)); /* dtl_clamping_triggered_lane */
        }

        /* read freq offset ready */
        MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,31,1,&freqReady));
        if (freqReady == 0)
        {
            /* RX_FOFFSET_EXTRACTION_EN_LANE */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<20),(0x1<<20)));
            /* RX_FOFFSET_EXTRACTION_EN_LANE */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<19),(0x1<<19)));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<19),0));
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));
            /* freq offset read request */
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, portGroup, serdesNum,0x2160,(0x1<<16),(0x1<<16)));
            /* read freq offset ready*/
            MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,31,1,&freqReady));
        }
        else
        {
            /* read freq value twice and check if changed*/
            MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,18,13,&data)); /* RX_FOFFSET_RD_LANE */
            dataSigned = data;
            /* if freq is negative then convert to negative number */
            if ( dataSigned & 0x1000 )
            {
                dataSigned |= 0xe000;
            }
            MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 2));
            MCESD_ATTEMPT(mvHwsComphySerdesRegisterReadField(devNum,portGroup,serdesNum,0x2168,18,13,&data));
            dataSigned2 = data;
            /* if freq is negative then convert to negative number */
            if ( dataSigned2 & 0x1000 )
            {
                dataSigned2 |= 0xe000;
            }
            /* allow +/- 60ppm tolerance */
            diff = (dataSigned > dataSigned2) ? dataSigned - dataSigned2 : dataSigned2 - dataSigned;
            if (diff <= MV_HWS_COMPHY_PPM_TOLERANCE_FOR_112G)
            {
                if ( (ABS(dataSigned) < MV_HWS_COMPHY_FREQUENCY_TOLERANCE) && (dtlClamping == 0) )
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
        MCESD_CHECK_STATUS(API_C112GX4_Wait(sDev, 1));
        continue;
    }
    CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum,0x2160,13, 1, 0x1)); /* DTL_SQ_DET_EN_LANE */
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
 * mvHwsComphyC112GX4InitProgData
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
static GT_STATUS mvHwsComphyC112GX4InitProgData
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
        xMemData->dataMaxSize = C112GX4_XDATA_MAX_SIZE;
        xMemData->dataBaseAddr = C112GX4_XDATA_CMN_BASE_ADDR;
        xMemData->regAddrExpectCheckSum = 0xA3A4;
        xMemData->regAddrActualCheckSum = 0xA3A8;
        xMemData->regAddrCSPass = 0xA3AC;
        break;
    case MV_HWS_COMPHY_LANE_DATA:
        xMemData->dataMaxSize = C112GX4_XDATA_MAX_SIZE;
        xMemData->dataBaseAddr = C112GX4_XDATA_LANE_BASE_ADDR;
        xMemData->regAddrExpectCheckSum = 0x22E4;
        xMemData->regAddrActualCheckSum = 0x22E8;
        xMemData->regAddrCSPass = 0x2294;
        break;
    case MV_HWS_COMPHY_FW_DATA:
        xMemData->dataMaxSize = C112GX4_FW_MAX_SIZE;
        xMemData->dataBaseAddr = C112GX4_FW_BASE_ADDR;
        xMemData->regAddrExpectCheckSum = 0xA224;
        xMemData->regAddrActualCheckSum = 0xA228;
        break;
    default:
        return GT_FAIL;
    }

    /* Calculate CS incrementally */
    for (index = 0; index < codeSize; index++)
    {
        xMemData->expectCheckSum += mvHwsComphyCodeGet(index,codePtr);
    }
    MCESD_ATTEMPT(API_C112GX4_WriteReg(devPtr, xMemData->lane, xMemData->regAddrExpectCheckSum, xMemData->expectCheckSum));

    return GT_OK;
}

/**
 * @internal mvHwsComphyC112GX4UpdateRamCode
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
static GT_STATUS mvHwsComphyC112GX4UpdateRamCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
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
        MCESD_ATTEMPT(API_C112GX4_WriteReg(devPtr, lane, address, mvHwsComphyCodeGet(index,code)));
        address += 4;
    }

    *errCode = 0;
    return GT_OK;
}

/**
 * @internal mvHwsComphyC112GX4ProgRam
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
static GT_STATUS mvHwsComphyC112GX4ProgRam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 codePtr[],
    IN MCESD_U32 codeSize,
    IN MV_HWS_COMPHY_RAM_DATA_TYPE xType,
    INOUT MV_HWS_COMPHY_PROG_RAM_DATA *progData
)
{
    MCESD_U32 actualChecksum, checksumPass;

    CHECK_STATUS(mvHwsComphyC112GX4InitProgData(devPtr, codePtr, codeSize, xType, progData));

    /* Write XDATA to MCU */
    CHECK_STATUS(mvHwsComphyC112GX4UpdateRamCode(devPtr, progData->lane, progData->pDataPtr, progData->dataSizeDW, progData->dataMaxSize, progData->dataBaseAddr, &(progData->errCode)));

    MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, progData->lane, progData->regAddrActualCheckSum, &actualChecksum));

    if (MV_HWS_COMPHY_FW_DATA != progData->xType)
    {
        MCESD_ATTEMPT(API_C112GX4_ReadReg(devPtr, progData->lane, progData->regAddrCSPass, &checksumPass));
        if ((checksumPass & (1 << ((progData->xType == MV_HWS_COMPHY_CMN_DATA) ? 1 : 29))) && (actualChecksum == progData->expectCheckSum))
        {
            progData->errCode = 0;
            return GT_OK;
        }

        MCESD_DBG_ERROR("mvHwsComphyC112GX4ProgRam Failed: expectedChecksum = 0x%08x, actualChecksum = 0x%08x\n",progData->expectCheckSum ,actualChecksum);
        progData->errCode = MCESD_IO_ERROR;
        return GT_FAIL;
    }

    return GT_OK;
}

/**
 * @internal mvHwsComphyC112GX4DownloadFirmwareProgXData
 *
 * @brief Download C28GP4X1 FW Data
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
static GT_STATUS mvHwsComphyC112GX4DownloadFirmwareProgXData
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

    if (API_C112GX4_SetMcuBroadcast(devPtr, GT_TRUE) == GT_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_SetMcuBroadcast: Failed\n");
        return GT_FAIL;
    }

    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DIRECT_ACCESS_EN, 1));
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PRAM_SOC_EN, 1));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 10));

    /* Download MCU Firmware */
    if (mvHwsComphyC112GX4ProgRam(devPtr, fwCodePtr, fwCodeSizeDW, MV_HWS_COMPHY_FW_DATA, &xMemData) == GT_FAIL)
    {
        MCESD_DBG_ERROR("mvHwsComphyC112GX4ProgRam: Failed firmware download with error code: 0x%X\n", xMemData.errCode);
        return GT_FAIL;
    }

    /* Download common XDATA */
    if (mvHwsComphyC112GX4ProgRam(devPtr, cmnXDataPtr, cmnXDataSizeDW, MV_HWS_COMPHY_CMN_DATA, &xMemData) == GT_FAIL)
    {
        MCESD_DBG_ERROR("mvHwsComphyC112GX4ProgRam: Failed common XDATA download with error code: 0x%X\n", xMemData.errCode);
        return GT_FAIL;
    }

    /* Download lane XDATA - will be broadcasted to all lanes */
    if (mvHwsComphyC112GX4ProgRam(devPtr, laneXDataPtr, laneXDataSizeDW, MV_HWS_COMPHY_LANE_DATA, &xMemData) == GT_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_PowerOnSeq: Failed lane XDATA download with error code: 0x%X\n", xMemData.errCode);
        return GT_FAIL;
    }

    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_PRAM_SOC_EN, 0));
    MCESD_ATTEMPT(API_C112GX4_HwSetPinCfg(devPtr, C112GX4_PIN_DIRECT_ACCESS_EN, 0));
    MCESD_ATTEMPT(API_C112GX4_Wait(devPtr, 10));

    if (API_C112GX4_SetMcuBroadcast(devPtr, GT_FALSE) == GT_FAIL)
    {
        MCESD_DBG_ERROR("API_C112GX4_SetMcuBroadcast: Failed\n");
        return GT_FAIL;
    }

    /* Enable MCU */
    MCESD_ATTEMPT(API_C112GX4_SetLaneEnable(devPtr, 255, GT_TRUE));
    MCESD_ATTEMPT(API_C112GX4_SetMcuEnable(devPtr, 255, GT_TRUE));
    return GT_OK;
}
#endif

/**
* @internal mvHwsComphyC112GX4SerdesFirmwareDownloadIter
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
static GT_STATUS mvHwsComphyC112GX4SerdesFirmwareDownloadIter
(
    GT_U8                 devNum,
    GT_UOPT               *serdesArr,
    GT_U8                 numOfSerdeses,
    MCESD_U32             *fwCodePtr,
    MCESD_U32             fwCodeSize,
    MCESD_U32             *cmnXDataPtr,
    MCESD_U32             cmnXDataSize,
    MCESD_U32             *laneXDataPtr,
    MCESD_U32             laneXDataSize,
    E_C112GX4_REFCLK_SEL  refClkSel,
    E_C112GX4_REFFREQ     refFreq
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
        MCESD_CHECK_STATUS(API_C112GX4_SetPhyMode(sDev, C112GX4_PHYMODE_SERDES));
        /* Configure Reference Frequency and reference clock source group */
        MCESD_CHECK_STATUS(API_C112GX4_SetRefFreq(sDev, refFreq, refClkSel));
        /* Power up current and voltage reference */
        MCESD_CHECK_STATUS(API_C112GX4_SetPowerIvRef(sDev, MCESD_TRUE));
        /* Download MCU Firmware */
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
        MCESD_CHECK_STATUS(API_C112GX4_DownloadFirmwareProgXData(sDev,
                                          fwCodePtr,    fwCodeSize,
                                          cmnXDataPtr,  cmnXDataSize,
                                          laneXDataPtr, laneXDataSize));
#else
        CHECK_STATUS(mvHwsComphyC112GX4DownloadFirmwareProgXData(sDev,
                                          fwCodePtr,    fwCodeSize,
                                          cmnXDataPtr,  cmnXDataSize,
                                          laneXDataPtr, laneXDataSize));
#endif
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesFirmwareDownload function
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
GT_STATUS mvHwsComphyC112GX4SerdesFirmwareDownload
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

    E_C112GX4_REFCLK_SEL   refClkSel = C112GX4_REFCLK_SEL_GROUP1;
    E_C112GX4_REFFREQ      refFreq = C112GX4_REFFREQ_156P25MHZ;
    GT_BOOL                freeNeeded = GT_FALSE;

    MCESD_U32 *fwCodePtr = NULL;
    MCESD_U32 fwCodeSize = 0;

    MCESD_U32 *cmnXDataPtr = NULL;
    MCESD_U32 cmnXDataSize = 0;

    MCESD_U32 *laneXDataPtr = NULL;
    MCESD_U32 laneXDataSize = 0;

    GT_UNUSED_PARAM(portGroup);
    switch(firmwareSelector)
    {
        case MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE:
        case MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE:
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            fwCodePtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C112GX4_FW_MAX_SIZE);
            if ( fwCodePtr == NULL )
            {
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//main.dat",
                                        fwCodePtr,       C112GX4_FW_MAX_SIZE,         &fwCodeSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }

            cmnXDataPtr  = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C112GX4_XDATA_MAX_SIZE);
            if ( cmnXDataPtr == NULL )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                return GT_FAIL;
            }
            if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//cmn.dat",
                                        cmnXDataPtr,     C112GX4_XDATA_MAX_SIZE,  &cmnXDataSize, &errCode) != MCESD_OK )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                hwsOsFreeFuncPtr(cmnXDataPtr);
                return GT_FAIL;
            }

            laneXDataPtr = (MCESD_U32*)hwsOsMallocFuncPtr(sizeof(MCESD_U32)*C112GX4_XDATA_MAX_SIZE);
            if ( laneXDataPtr == NULL )
            {
                hwsOsFreeFuncPtr(fwCodePtr);
                hwsOsFreeFuncPtr(cmnXDataPtr);
                return GT_FAIL;
            }
            if(firmwareSelector == MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE)
            {
                refFreq = C112GX4_REFFREQ_156P25MHZ;
                if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//lane_156.dat",
                                            laneXDataPtr,    C112GX4_XDATA_MAX_SIZE, &laneXDataSize, &errCode) != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(fwCodePtr);
                    hwsOsFreeFuncPtr(cmnXDataPtr);
                    hwsOsFreeFuncPtr(laneXDataPtr);
                    return GT_FAIL;
                }
            }
            else
            {
                refFreq = C112GX4_REFFREQ_25MHZ;
                if ( LoadFwDataFileToBuffer("serdes_fw//C112GX4//lane_25.dat",
                                            laneXDataPtr,    C112GX4_XDATA_MAX_SIZE, &laneXDataSize, &errCode) != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(fwCodePtr);
                    hwsOsFreeFuncPtr(cmnXDataPtr);
                    hwsOsFreeFuncPtr(laneXDataPtr);
                    return GT_FAIL;
                }
            }
#endif
            freeNeeded = GT_TRUE;
            break;
        case MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE:
#ifndef MICRO_INIT
            fwCodePtr     = (MCESD_U32*)C112GX4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_C112GX4_MAIN_DATA_IMAGE_SIZE;

            cmnXDataPtr   = (MCESD_U32*)C112GX4_SERDES_CMN_Data;
            cmnXDataSize  = (MCESD_U32)COMPHY_C112GX4_SERDES_CMN_DATA_IMAGE_SIZE;

            laneXDataPtr  = (MCESD_U32*)C112GX4_SERDES_REF25MHz_SPDCHG0_LANE_Data;
            laneXDataSize = (MCESD_U32)COMPHY_C112GX4_SERDES_REF25MHZ_SPDCHG0_LANE_DATA_IMAGE_SIZE;

            refFreq       = C112GX4_REFFREQ_25MHZ;
#endif
            break;
        case MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE:
#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
            fwCodePtr     = (MCESD_U32*)C112GX4_main_Data;
            fwCodeSize    = (MCESD_U32)COMPHY_C112GX4_MAIN_DATA_IMAGE_SIZE;

            cmnXDataPtr   = (MCESD_U32*)C112GX4_SERDES_CMN_Data;
            cmnXDataSize  = (MCESD_U32)COMPHY_C112GX4_SERDES_CMN_DATA_IMAGE_SIZE;

            laneXDataPtr  = (MCESD_U32*)C112GX4_SERDES_REF156MHz_SPDCHG0_LANE_Data;
            laneXDataSize = (MCESD_U32)COMPHY_C112GX4_SERDES_REF156MHZ_SPDCHG0_LANE_DATA_IMAGE_SIZE;
#else
            fwCodePtr     = (MCESD_U32*)C112GX4_main_Ptr;
            fwCodeSize    = (MCESD_U32)C112GX4_main_Size;

            cmnXDataPtr   = (MCESD_U32*)C112GX4_SERDES_CMN_Ptr;
            cmnXDataSize  = (MCESD_U32)C112GX4_SERDES_CMN_Size;

            laneXDataPtr  = (MCESD_U32*)C112GX4_SERDES_REF156MHz_SPDCHG0_LANE_Ptr;
            laneXDataSize = (MCESD_U32)C112GX4_SERDES_REF156MHz_SPDCHG0_LANE_Size;
#endif

            refFreq       = C112GX4_REFFREQ_156P25MHZ;
            break;
        default:
            return GT_NOT_SUPPORTED;
    }

#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3
    comphyPrintf("--fwCodeSize[%d]-cmnXDataSize[%d]-laneXDataSize[%d]--\n",fwCodeSize, cmnXDataSize,laneXDataSize);
    if ( freeNeeded )
    {
        hwsOsPrintf("Using SerDes FW from file: fwCodeSize=[%d], cmnXDataSize=[%d], laneXDataSize=[%d]\n",fwCodeSize, cmnXDataSize,laneXDataSize);
    }
#endif
    rc = mvHwsComphyC112GX4SerdesFirmwareDownloadIter(devNum, serdesArr, numOfSerdeses,
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
* @internal mvHwsComphyC112GX4AnpPowerUp function
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
static GT_STATUS mvHwsComphyC112GX4AnpPowerUp
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

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(serdesConfigPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_SetLaneEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)powerUp));
    MCESD_CHECK_STATUS(API_C112GX4_SetDataBusWidth(sDev, serdesInfo->internalLane, C112GX4_DATABUS_40BIT, C112GX4_DATABUS_40BIT));

    /* SQ_LPF_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2170, (0xFFFF << 16), 0x3FF << 16));
    /* SQ_LPF_EN_LANE */
    CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0, serdesNum, 0x2170, (1<<12), (regData<<12)));

    MCESD_CHECK_STATUS(API_C112GX4_SetMcuEnable(sDev, serdesInfo->internalLane, (MCESD_BOOL)powerUp));

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesTemperatureGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesTemperatureGet
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

    MCESD_CHECK_STATUS(API_C112GX4_GetTemperature(sDev, &temp));
    *serdesTemperature = (GT_32)temp;

    return GT_OK;
}

#ifndef MICRO_INIT
/**
* @internal mvHwsComphyC112GX4SerdesPMDLogGet function
* @endinternal
*
* @brief   Display's Tx Training Log
*
* @param[in] devNum                   - physical device number
* @param[in] serdesNum                - serdes number
*
* @param[out] validEntriesPtr         - (pointer to) number of
*                                        valid entries
* @param[out] pmdLogStr               - (pointer to) struct
*                                       aligned to string
* @param[in] pmdLogStrSize            - max size of pmdLogStr
*
* @retval 0 - on success
* @retval 1 - on error
*/
GT_STATUS mvHwsComphyC112GX4SerdesPMDLogGet
(
    IN  GT_U8                  devNum,
    IN  GT_UOPT                serdesNum,
    OUT GT_U32                 *validEntriesPtr,
    OUT GT_CHAR                *pmdLogStr,
    IN  GT_U32                 pmdLogStrSize
)
{
    MV_HWS_PER_SERDES_INFO_PTR      serdesInfo;
    MCESD_DEV_PTR                   sDev;
    S_C112GX4_TRX_TRAINING_LOGENTRY logArrayData[HWS_PMD_LOG_ARRAY_DATA_SIZE];
    MCESD_U32                       logArraySizeEntries = HWS_PMD_LOG_ARRAY_DATA_SIZE;
    GT_U32                          i, shift = 0, offset = 0;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    if(validEntriesPtr == NULL)
    {
        return GT_BAD_PARAM;
    }

    hwsOsMemSetFuncPtr(logArrayData, 0, sizeof(S_C112GX4_TRX_TRAINING_LOGENTRY) * logArraySizeEntries);

    MCESD_CHECK_STATUS(API_C112GX4_DisplayTrainingLog(sDev, serdesInfo->internalLane, logArrayData, logArraySizeEntries, validEntriesPtr));

    if ( logArrayData[0].isPAM2 )
    {
        shift  = hwsSprintf(&pmdLogStr[offset], "PAM2\n====\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "Entry # | patternType | txPresetIndex | localCtrlG1 | localCtrlG0 | localCtrlGN1 | remoteStatusG1 | remoteStatusG0 | remoteStatusGN1 | errorCode\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "-------------------------------------------------------------------------------------------------------------------------------------------------\n");
        offset += shift;
    }
    else
    {
        shift  = hwsSprintf(&pmdLogStr[offset], "PAM4\n====\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "Entry # | txPresetIndex | localCtrlPat | PRE2 | PRE | MAIN | POST | remoteStatusAck | remoteStatusG\n");
        offset += shift;
        shift = hwsSprintf(&pmdLogStr[offset], "----------------------------------------------------------------------------------------------------\n");
        offset += shift;
    }

    for (i = 0; i < *validEntriesPtr; i++)
    {
        if ( offset >  pmdLogStrSize - 200 )
        {
            hwsSprintf(&pmdLogStr[offset - 1], "out of space for next entry\n\0");
            return GT_OK;
        }
        shift = hwsSprintf(&pmdLogStr[offset], (i < 10) ? "%d       |" : "%d      |", i);
        offset += shift;
        if ( logArrayData[i].isPAM2 )
        {
            shift = hwsSprintf(&pmdLogStr[offset], (logArrayData[i].pam2.patternType) ? "PAT0         |" : "PAT1         |");
            offset += shift;

            switch ( logArrayData[i].pam2.txPresetIndex )
            {
            case C112GX4_TRX_LOG_TX_PRESET_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA             |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET1:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET1        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET2:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET2        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET3:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET3        |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.localCtrlG1 )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD         |");
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                shift = hwsSprintf(&pmdLogStr[offset], "INC          |");
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                shift = hwsSprintf(&pmdLogStr[offset], "DEC          |");
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA           |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.localCtrlG0 )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD         |");
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                shift = hwsSprintf(&pmdLogStr[offset], "INC          |");
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                shift = hwsSprintf(&pmdLogStr[offset], "DEC          |");
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA           |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.localCtrlGN1 )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD          |");
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                shift = hwsSprintf(&pmdLogStr[offset], "INC           |");
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                shift = hwsSprintf(&pmdLogStr[offset], "DEC           |");
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA            |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.remoteStatusG1 )
            {
            case C112GX4_TRX_LOG_PAM2_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD            |");
                break;
            case C112GX4_TRX_LOG_PAM2_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED         |");
                break;
            case C112GX4_TRX_LOG_PAM2_MIN:
                shift = hwsSprintf(&pmdLogStr[offset], "MIN             |");
                break;
            case C112GX4_TRX_LOG_PAM2_MAX:
                shift = hwsSprintf(&pmdLogStr[offset], "MAX             |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.remoteStatusG0 )
            {
            case C112GX4_TRX_LOG_PAM2_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD            |");
                break;
            case C112GX4_TRX_LOG_PAM2_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED         |");
                break;
            case C112GX4_TRX_LOG_PAM2_MIN:
                shift = hwsSprintf(&pmdLogStr[offset], "MIN             |");
                break;
            case C112GX4_TRX_LOG_PAM2_MAX:
                shift = hwsSprintf(&pmdLogStr[offset], "MAX             |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam2.remoteStatusGN1 )
            {
            case C112GX4_TRX_LOG_PAM2_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD             |");
                break;
            case C112GX4_TRX_LOG_PAM2_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED          |");
                break;
            case C112GX4_TRX_LOG_PAM2_MIN:
                shift = hwsSprintf(&pmdLogStr[offset], "MIN              |");
                break;
            case C112GX4_TRX_LOG_PAM2_MAX:
                shift = hwsSprintf(&pmdLogStr[offset], "MAX              |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], "%d\n", logArrayData[i].pam2.errorCode);
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], "-------------------------------------------------------------------------------------------------------------------------------------------------\n");
            offset += shift;
        } /* logArrayData[i].isPAM2 */
        else /* PAM4*/
        {
            GT_CHAR temp[5];

            switch ( logArrayData[i].pam4.txPresetIndex )
            {
            case C112GX4_TRX_LOG_TX_PRESET_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA             |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET1:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET1        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET2:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET2        |");
                break;
            case C112GX4_TRX_LOG_TX_PRESET3:
                shift = hwsSprintf(&pmdLogStr[offset], "PRESET3        |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam4.localCtrlPat )
            {
            case C112GX4_TRX_LOG_PAT_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA            |");
                break;
            case C112GX4_TRX_LOG_PAT_GRAY_CODE:
                shift = hwsSprintf(&pmdLogStr[offset], "GRAY_CODE     |");
                break;
            case C112GX4_TRX_LOG_PAT_PRE_CODE:
                shift = hwsSprintf(&pmdLogStr[offset], "PRE_CODE     |");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            switch ( logArrayData[i].pam4.localCtrlG )
            {
            case C112GX4_TRX_LOG_REQ_HOLD:
                hwsOsMemCopyFuncPtr(temp, "HOLD\0", 5 * sizeof(GT_CHAR));
                break;
            case C112GX4_TRX_LOG_REQ_INC:
                hwsOsMemCopyFuncPtr(temp, "INC \0", 5 * sizeof(GT_CHAR));
                break;
            case C112GX4_TRX_LOG_REQ_DEC:
                hwsOsMemCopyFuncPtr(temp, "DEC \0", 5 * sizeof(GT_CHAR));
                break;
            case C112GX4_TRX_LOG_REQ_NA:
                hwsOsMemCopyFuncPtr(temp, "NA  \0", 5 * sizeof(GT_CHAR));
                break;
            default:
                return GT_BAD_PARAM;
            }
            switch ( logArrayData[i].pam4.localCtrlSel )
            {
            case C112GX4_TRX_LOG_SEL_MAIN:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD  |HOLD |%s  |HOLD  |", temp);
                break;
            case C112GX4_TRX_LOG_SEL_POST:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD  |HOLD |HOLD  |%s  |", temp);
                break;
            case C112GX4_TRX_LOG_SEL_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA    |NA   |NA    |NA    |");
                break;
            case C112GX4_TRX_LOG_SEL_PRE2:
                shift = hwsSprintf(&pmdLogStr[offset], "%s  |HOLD |HOLD  |HOLD  |", temp);
                break;
            case C112GX4_TRX_LOG_SEL_PRE1:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD  |%s |HOLD  |HOLD  |", temp);
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], (logArrayData[i].pam4.remoteStatusAck == C112GX4_TRX_LOG_ACK_HOLD) ? "HOLD             |" : "UPDATED          |");
            offset += shift;
            switch ( logArrayData[i].pam4.remoteStatusG )
            {
            case C112GX4_TRX_LOG_PAM4_HOLD:
                shift = hwsSprintf(&pmdLogStr[offset], "HOLD          |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_UPDATED:
                shift = hwsSprintf(&pmdLogStr[offset], "UPDATED       |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_HIT_LIMIT:
                shift = hwsSprintf(&pmdLogStr[offset], "HIT_LIMIT     |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_NOTSUPPORT:
                shift = hwsSprintf(&pmdLogStr[offset], "NOTSUPPORT    |\n");
                break;
            case C112GX4_TRX_LOG_PAM4_NA:
                shift = hwsSprintf(&pmdLogStr[offset], "NA            |\n");
                break;
            default:
                return GT_BAD_PARAM;
            }
            offset += shift;
            shift = hwsSprintf(&pmdLogStr[offset], "----------------------------------------------------------------------------------------------------\n");
            offset += shift;
        } /* PAM4*/
    }/* end for */

    hwsSprintf(&pmdLogStr[offset], "\0");

    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesDumpInfo function
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
static GT_STATUS mvHwsComphyC112GX4SerdesDumpInfo
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
        "C112GX4_PIN_RESET",
        "C112GX4_PIN_ISOLATION_ENB",
        "C112GX4_PIN_BG_RDY",
        "C112GX4_PIN_SIF_SEL",
        "C112GX4_PIN_MCU_CLK",
        "C112GX4_PIN_DIRECT_ACCESS_EN",
        "C112GX4_PIN_PRAM_FORCE_RESET",
        "C112GX4_PIN_PRAM_RESET",
        "C112GX4_PIN_PRAM_SOC_EN",
        "C112GX4_PIN_PRAM_SIF_SEL",
        "C112GX4_PIN_PHY_MODE",
        "C112GX4_PIN_REFCLK_SEL",
        "C112GX4_PIN_REF_FREF_SEL",
        "C112GX4_PIN_PHY_GEN_TX0",
        "C112GX4_PIN_PHY_GEN_TX1",
        "C112GX4_PIN_PHY_GEN_TX2",
        "C112GX4_PIN_PHY_GEN_TX3",
        "C112GX4_PIN_PHY_GEN_RX0",
        "C112GX4_PIN_PHY_GEN_RX1",
        "C112GX4_PIN_PHY_GEN_RX2",
        "C112GX4_PIN_PHY_GEN_RX3",
        "C112GX4_PIN_DFE_EN0",
        "C112GX4_PIN_DFE_EN1",
        "C112GX4_PIN_DFE_EN2",
        "C112GX4_PIN_DFE_EN3",
        "C112GX4_PIN_DFE_UPDATE_DIS0",
        "C112GX4_PIN_DFE_UPDATE_DIS1",
        "C112GX4_PIN_DFE_UPDATE_DIS2",
        "C112GX4_PIN_DFE_UPDATE_DIS3",
        "C112GX4_PIN_PU_PLL0",
        "C112GX4_PIN_PU_PLL1",
        "C112GX4_PIN_PU_PLL2",
        "C112GX4_PIN_PU_PLL3",
        "C112GX4_PIN_PU_RX0",
        "C112GX4_PIN_PU_RX1",
        "C112GX4_PIN_PU_RX2",
        "C112GX4_PIN_PU_RX3",
        "C112GX4_PIN_PU_TX0",
        "C112GX4_PIN_PU_TX1",
        "C112GX4_PIN_PU_TX2",
        "C112GX4_PIN_PU_TX3",
        "C112GX4_PIN_TX_IDLE0",
        "C112GX4_PIN_TX_IDLE1",
        "C112GX4_PIN_TX_IDLE2",
        "C112GX4_PIN_TX_IDLE3",
        "C112GX4_PIN_PU_IVREF",
        "C112GX4_PIN_RX_TRAIN_ENABLE0",
        "C112GX4_PIN_RX_TRAIN_ENABLE1",
        "C112GX4_PIN_RX_TRAIN_ENABLE2",
        "C112GX4_PIN_RX_TRAIN_ENABLE3",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_COMPLET",
        "C112GX4_PIN_RX_TRAIN_FAILED0",
        "C112GX4_PIN_RX_TRAIN_FAILED1",
        "C112GX4_PIN_RX_TRAIN_FAILED2",
        "C112GX4_PIN_RX_TRAIN_FAILED3",
        "C112GX4_PIN_TX_TRAIN_ENABLE0",
        "C112GX4_PIN_TX_TRAIN_ENABLE1",
        "C112GX4_PIN_TX_TRAIN_ENABLE2",
        "C112GX4_PIN_TX_TRAIN_ENABLE3",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_COMPLET",
        "C112GX4_PIN_TX_TRAIN_FAILED0",
        "C112GX4_PIN_TX_TRAIN_FAILED1",
        "C112GX4_PIN_TX_TRAIN_FAILED2",
        "C112GX4_PIN_TX_TRAIN_FAILED3",
        "C112GX4_PIN_SQ_DETECTED_LPF0",
        "C112GX4_PIN_SQ_DETECTED_LPF1",
        "C112GX4_PIN_SQ_DETECTED_LPF2",
        "C112GX4_PIN_SQ_DETECTED_LPF3",
        "C112GX4_PIN_RX_INIT0",
        "C112GX4_PIN_RX_INIT1",
        "C112GX4_PIN_RX_INIT2",
        "C112GX4_PIN_RX_INIT3",
        "C112GX4_PIN_RX_INIT_DONE0",
        "C112GX4_PIN_RX_INIT_DONE1",
        "C112GX4_PIN_RX_INIT_DONE2",
        "C112GX4_PIN_RX_INIT_DONE3",
        "C112GX4_PIN_DFE_PAT_DIS0",
        "C112GX4_PIN_DFE_PAT_DIS1",
        "C112GX4_PIN_DFE_PAT_DIS2",
        "C112GX4_PIN_DFE_PAT_DIS3"
    };

    GT_UNUSED_PARAM(portGroup);
    GT_UNUSED_PARAM(outputPtr);

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));
    osPrintfIf(printBool,"Dump SerDes Number[%d] information\n");
    osPrintfIf(printBool,"SerDes Type: COM_PHY_C112GX4\n");

    switch(dumpType)
    {
        case SERDES_DUMP_INTERNAL_REGISTERS:
            osPrintfIf(printBool,"Dump type - Internal SerDes registers:\n");
            for(i = 0 ; i <= 0xE68C ; i = i+0x4)
            {
                CHECK_STATUS(mvHwsComphySerdesRegisterRead(devNum, portGroup,serdesNum,i,0xffffffff,&regVal));
                osPrintfIf(printBool,"Address: 0x%04x, Data: 0x%08x\n", i, regVal);
                if(NULL != outputPtr) outputPtr[i] = regVal;
            }
            break;
        case SERDES_DUMP_PINS:
            osPrintfIf(printBool,"Dump type - SerDes external pins:\n");
            for(i = 0 ; i <= (GT_U32)C112GX4_PIN_DFE_PAT_DIS3 ; i++)
            {
                mvHwsComphySerdesPinGet(devNum, portGroup, serdesNum, (E_C112GX4_PIN)i, &pinVal);
                osPrintfIf(printBool,"(%3d) %#-40s %3d\n", i, pinsList[i],pinVal);
                if(NULL != outputPtr) outputPtr[i] = pinVal;
            }
            break;
        default:
            return GT_BAD_PARAM;
    }
    return GT_OK;
}

/**
* @internal mvHwsComphyC112GX4SerdesRxPllLockGet function
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
GT_STATUS mvHwsComphyC112GX4SerdesRxPllLockGet
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
#endif

/**
* @internal mvHwsComphyC112GX4SerdesDroGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesDroGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          serdesNum,
    OUT GT_U16                          *dro
)
{
    MCESD_DEV_PTR sDev;
    MV_HWS_PER_SERDES_INFO_PTR  serdesInfo;
    S_C112GX4_DRO_DATA freq;

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    MCESD_CHECK_STATUS(API_C112GX4_GetDRO(sDev,&freq));
    hwsOsMemCopyFuncPtr(dro, freq.dro, sizeof(S_C112GX4_DRO_DATA));

    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsComphyC112GX4SerdesEomMatrixGet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesEomMatrixGet
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
    S_C112GX4_EYE_RAW *mcesdEyeRawData;
    MV_HWS_SERDES_ENCODING_TYPE txEncoding, rxEncoding;
    GT_U32 i;
    S_C112GX4_EYE_BUFFER *eyeBufferData;
    S_EOM_STATS_EYE_DIM estimateEyeDim[ESTIMATE_EYE_DIM_COUNT];
    S_EOM_STATS_EYE_AMP amplitudeStats;
    MCESD_U32 widthmUI, heightuV;
    MCESD_U32 width, heightUpper, heightLower;
    MCESD_BOOL dfeState;

    if ( NULL == eomInParams|| NULL == eomOutParams)
    {
        return GT_BAD_PTR;
    }

    if (eomInParams->comphyC112GEomInParams.phaseStepSize   < 1 || eomInParams->comphyC112GEomInParams.phaseStepSize   > 16 ||
        eomInParams->comphyC112GEomInParams.voltageStepSize < 1 || eomInParams->comphyC112GEomInParams.voltageStepSize > 16)
    {
        return GT_OUT_OF_RANGE;
    }

    CHECK_STATUS(mvHwsComphySerdesIndexInfoSDevGet(devNum,serdesNum,&serdesInfo,NULL,&sDev));

    /* need to enable DFE for EOM (disabled for low baud rates during power-up) */
    MCESD_CHECK_STATUS(API_C112GX4_GetDfeEnable(sDev, serdesInfo->internalLane, &dfeState));
    if (dfeState == MCESD_FALSE)
    {
        MCESD_CHECK_STATUS(API_C112GX4_SetDfeEnable(sDev, serdesInfo->internalLane, MCESD_TRUE));
    }

    mcesdEyeRawData = (S_C112GX4_EYE_RAW*)hwsOsMallocFuncPtr(sizeof(S_C112GX4_EYE_RAW));
    if ( NULL == mcesdEyeRawData )
    {
        return GT_NO_RESOURCE;
    }

    eyeBufferData = (S_C112GX4_EYE_BUFFER *)hwsOsMallocFuncPtr(sizeof(S_C112GX4_EYE_BUFFER));
    if (NULL == eyeBufferData)
    {
        return GT_NO_RESOURCE;
    }

    /* NRZ/PAM4 */
    rc = mvHwsComphyC112GX4SerdesEncodingTypeGet(devNum,serdesNum,&txEncoding,&rxEncoding);
    if ( rc != GT_OK)
    {
        hwsOsFreeFuncPtr(eyeBufferData);
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return rc;
    }

    if ( rxEncoding == SERDES_ENCODING_NRZ )    /* for NRZ speeds, only mid eye exists */
    {
        mcesdSt = API_C112GX4_EOMGetEyeData(sDev, serdesInfo->internalLane, C112GX4_EYE_MID, (MCESD_U32)eomInParams->comphyC112GEomInParams.minSamples,
                                                (MCESD_U32)eomInParams->comphyC112GEomInParams.berThreshold, (MCESD_BOOL)eomInParams->comphyC112GEomInParams.eomStatsMode, mcesdEyeRawData);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(eyeBufferData);
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }

        /* retrieve height and width in code word and convert to mV and pSec */
        mcesdSt = API_C112GX4_EyeDataDimensions(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC112GEomInParams.berThreshold, &width, &heightUpper, &heightLower);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(eyeBufferData);
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
        mcesdSt = API_C112GX4_EOMConvertWidthHeight(sDev, serdesInfo->internalLane, width, heightUpper, heightLower, &widthmUI, &heightuV);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(eyeBufferData);
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
        eomOutParams->comphyC112GEomOutParams.width_mUI[C112GX4_EYE_MID]   = (GT_U32)widthmUI;
        eomOutParams->comphyC112GEomOutParams.height_mV[C112GX4_EYE_MID]   = (GT_U32)heightuV / 1000;

        hwsOsMemSetFuncPtr(&amplitudeStats, 0, sizeof(amplitudeStats));
        hwsOsMemSetFuncPtr(estimateEyeDim, 0, sizeof(estimateEyeDim));

        /* retrieve EOM statistics */
        estimateEyeDim[0].ber = 1e-7;
        estimateEyeDim[1].ber = 1e-10;
        estimateEyeDim[2].ber = 1e-12;
        estimateEyeDim[3].ber = 1e-15;

        mcesdSt = API_C112GX4_EOMGetStats(mcesdEyeRawData, eyeBufferData, (S_EOM_STATS_EYE_DIM *)estimateEyeDim, ESTIMATE_EYE_DIM_COUNT, &amplitudeStats);
        if ( mcesdSt != MCESD_OK )
        {
            hwsOsFreeFuncPtr(eyeBufferData);
            hwsOsFreeFuncPtr(mcesdEyeRawData);
            return GT_FAIL;
        }
        eomOutParams->comphyC112GEomOutParams.Q[C112GX4_EYE_MID]           = amplitudeStats.Q;
        eomOutParams->comphyC112GEomOutParams.SNR[C112GX4_EYE_MID]         = amplitudeStats.SNR;
        eomOutParams->comphyC112GEomOutParams.upperMean[C112GX4_EYE_MID]   = amplitudeStats.upperMean;
        eomOutParams->comphyC112GEomOutParams.lowerMean[C112GX4_EYE_MID]   = amplitudeStats.lowerMean;
        eomOutParams->comphyC112GEomOutParams.upperStdDev[C112GX4_EYE_MID] = amplitudeStats.upperStdDev;
        eomOutParams->comphyC112GEomOutParams.lowerStdDev[C112GX4_EYE_MID] = amplitudeStats.lowerStdDev;

        if ( GT_TRUE == printEnable )
        {
            GT_U32 j;

            mcesdSt = API_C112GX4_EOMPlotEyeData(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC112GEomInParams.phaseStepSize,
                                                 (MCESD_U32)eomInParams->comphyC112GEomInParams.voltageStepSize,
                                                 (MCESD_U32) eomInParams->comphyC112GEomInParams.berThreshold,
                                                 (MCESD_U32)eomInParams->comphyC112GEomInParams.berThresholdMax);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(eyeBufferData);
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }

            hwsOsPrintf("Estimated eye statistics per BER target:\n");
            for (j = 0; j < ESTIMATE_EYE_DIM_COUNT; j++)
            {
                hwsOsPrintf("BER target : %E - Width = %f, Height Upper = %f, Height Lower = %f\n", estimateEyeDim[j].ber, estimateEyeDim[j].width, estimateEyeDim[j].upperHeight, estimateEyeDim[j].lowerHeight);
            }
        }
        hwsOsFreeFuncPtr(eyeBufferData);
        hwsOsFreeFuncPtr(mcesdEyeRawData);
    }
    else if ( rxEncoding == SERDES_ENCODING_PAM4 )
    {
        for (i = 0; i < MAX_EYE_NUM; i++) /* PAM4 speeds have 3 eyes */
        {
            mcesdSt = API_C112GX4_EOMGetEyeData(sDev, serdesInfo->internalLane, i /*TMB*/, (MCESD_U32)eomInParams->comphyC112GEomInParams.minSamples,
                                                    (MCESD_U32)eomInParams->comphyC112GEomInParams.berThreshold, (MCESD_BOOL)eomInParams->comphyC112GEomInParams.eomStatsMode, mcesdEyeRawData);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(eyeBufferData);
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }

            /* retrieve height and width in code word and convert to mV and pSec */
            mcesdSt = API_C112GX4_EyeDataDimensions(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC112GEomInParams.berThreshold, &width, &heightUpper, &heightLower);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(eyeBufferData);
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
            mcesdSt = API_C112GX4_EOMConvertWidthHeight(sDev, serdesInfo->internalLane, width, heightUpper, heightLower, &widthmUI, &heightuV);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(eyeBufferData);
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
            eomOutParams->comphyC112GEomOutParams.width_mUI[i /*TMB*/]   = (GT_U32)widthmUI;
            eomOutParams->comphyC112GEomOutParams.height_mV[i /*TMB*/]   = (GT_U32)heightuV / 1000;

            hwsOsMemSetFuncPtr(&amplitudeStats, 0, sizeof(amplitudeStats));
            hwsOsMemSetFuncPtr(estimateEyeDim, 0, sizeof(estimateEyeDim));

            /* retrieve EOM statistics */
            estimateEyeDim[0].ber = 1e-7;
            estimateEyeDim[1].ber = 1e-10;
            estimateEyeDim[2].ber = 1e-12;
            estimateEyeDim[3].ber = 1e-15;

            mcesdSt = API_C112GX4_EOMGetStats(mcesdEyeRawData, eyeBufferData, (S_EOM_STATS_EYE_DIM *)estimateEyeDim, ESTIMATE_EYE_DIM_COUNT, &amplitudeStats);
            if ( mcesdSt != MCESD_OK )
            {
                hwsOsFreeFuncPtr(eyeBufferData);
                hwsOsFreeFuncPtr(mcesdEyeRawData);
                return GT_FAIL;
            }
            eomOutParams->comphyC112GEomOutParams.Q[i /*TMB*/]           = amplitudeStats.Q;
            eomOutParams->comphyC112GEomOutParams.SNR[i /*TMB*/]         = amplitudeStats.SNR;
            eomOutParams->comphyC112GEomOutParams.upperMean[i /*TMB*/]   = amplitudeStats.upperMean;
            eomOutParams->comphyC112GEomOutParams.lowerMean[i /*TMB*/]   = amplitudeStats.lowerMean;
            eomOutParams->comphyC112GEomOutParams.upperStdDev[i /*TMB*/] = amplitudeStats.upperStdDev;
            eomOutParams->comphyC112GEomOutParams.lowerStdDev[i /*TMB*/] = amplitudeStats.lowerStdDev;

            if ( GT_TRUE == printEnable )
            {
                GT_U32 j;

                mcesdSt = API_C112GX4_EOMPlotEyeData(mcesdEyeRawData, (MCESD_U32)eomInParams->comphyC112GEomInParams.phaseStepSize,
                                                     (MCESD_U32)eomInParams->comphyC112GEomInParams.voltageStepSize,
                                                     (MCESD_U32)eomInParams->comphyC112GEomInParams.berThreshold,
                                                     (MCESD_U32)eomInParams->comphyC112GEomInParams.berThresholdMax);
                if ( mcesdSt != MCESD_OK )
                {
                    hwsOsFreeFuncPtr(eyeBufferData);
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
        hwsOsFreeFuncPtr(eyeBufferData);
        hwsOsFreeFuncPtr(mcesdEyeRawData);
    }
    else
    {
        hwsOsFreeFuncPtr(eyeBufferData);
        hwsOsFreeFuncPtr(mcesdEyeRawData);
        return GT_BAD_VALUE;
    }

    /* restore DFE */
    MCESD_CHECK_STATUS(API_C112GX4_SetDfeEnable(sDev, serdesInfo->internalLane, dfeState));

    return GT_OK;
}
#endif /* MV_HWS_REDUCED_BUILD */

static GT_STATUS mvHwsComphyC112GX4SerdesRxInit
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
    MCESD_CHECK_STATUS(mvHwsComphyC112GX4SerdesSignalDetectGet(devNum, portGroup, serdesNum, &sigDetect));
    if (sigDetect == GT_TRUE)
    {
        MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, (C112GX4_PIN_RX_TRAIN_ENABLE0 + serdesInfo->internalLane), &completed));
        MCESD_CHECK_STATUS(API_C112GX4_HwGetPinCfg(sDev, (C112GX4_PIN_RX_INIT_DONE0 + serdesInfo->internalLane), &rxInitDone));
        if ((completed == 1) || (rxInitDone == 1))
            return GT_OK;

        switch (rxInitMode)
        {
        case RX_INIT_BLOCKING:
            MCESD_CHECK_STATUS(API_C112GX4_RxInit(sDev, serdesInfo->internalLane));
            break;
        default:
            return GT_NOT_SUPPORTED;
        }

    }
    return GT_OK;
}

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsComphyC112GX4SerdesSignalDetectMaskSet function
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
static GT_STATUS mvHwsComphyC112GX4SerdesSignalDetectMaskSet
(
    GT_U8   devNum,
    GT_UOPT portGroup,
    GT_UOPT serdesNum,
    GT_BOOL maskEnable
)
{
 devNum      =  devNum      ;
 portGroup   =  portGroup   ;
 serdesNum   =  serdesNum   ;
 maskEnable   =  maskEnable ;
    /*MCESD_CHECK_STATUS(mvHwsComphySerdesRegisterWriteField(devNum, portGroup, serdesNum, 0x2110, 9, 1, (maskEnable==GT_TRUE?1:0)));*/
    return GT_OK;
}

#endif /* MV_HWS_REDUCED_BUILD */

/**
* @internal mvHwsComphyC112GX4IfInit function
* @endinternal
*
* @brief   Init Comphy Serdes IF functions.
*
* @param[in] funcPtrArray             - array for function registration
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsComphyC112GX4IfInit
(
    IN MV_HWS_SERDES_FUNC_PTRS **funcPtrArray
)
{
    if(funcPtrArray == NULL)
    {
        return GT_BAD_PARAM;
    }

    if(!funcPtrArray[COM_PHY_C112GX4])
    {
        funcPtrArray[COM_PHY_C112GX4] = (MV_HWS_SERDES_FUNC_PTRS*)hwsOsMallocFuncPtr(sizeof(MV_HWS_SERDES_FUNC_PTRS));
        if(!funcPtrArray[COM_PHY_C112GX4])
        {
            return GT_NO_RESOURCE;
        }
        hwsOsMemSetFuncPtr(funcPtrArray[COM_PHY_C112GX4], 0, sizeof(MV_HWS_SERDES_FUNC_PTRS));
    }

    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesArrayPowerCntrlFunc             ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesArrayPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPowerCntrlFunc                  ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPowerCtrl);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualTxCfgFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesManualTxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesManualRxCfgFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesManualRxConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxAutoTuneStartFunc             ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesRxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxAutoTuneStartFunc             ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStartFunc               ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneStart);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneResultFunc              ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneResult);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusFunc              ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStatusShortFunc         ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesAutoTuneStatusShort);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesResetFunc                       ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDigitalReset                    ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDigitalReset);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackCfgFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesLoopback);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesLoopbackGetFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesLoopbackGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenFunc                     ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTestGen);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenGetFunc                  ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTestGenGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTestGenStatusFunc               ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTestGenStatus);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPolarityConfig);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPolarityCfgGetFunc              ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPolarityConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableFunc                    ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxEnbale);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTxEnableGetFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxEnbaleGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesAutoTuneStopFunc                ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTxAutoTuneStop);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSignalDetectGetFunc             ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesSignalDetectGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesFirmwareDownloadFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesFirmwareDownload);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesTemperatureFunc                 ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesTemperatureGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbTxCfgGetFunc                  ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDbTxConfigGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDbRxCfgGetFunc                  ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDbRxConfigGet);
#ifndef MICRO_INIT
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDumpInfoFunc                    ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDumpInfo);
#endif
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesApPowerCntrlFunc                ,COM_PHY_C112GX4, mvHwsComphyC112GX4AnpPowerUp);
#ifndef MICRO_INIT
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPMDLogGetFunc                   ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPMDLogGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEncodingTypeGetFunc             ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesEncodingTypeGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxPllLockGetFunc                ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesRxPllLockGet);
#endif
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesDroGetFunc                      ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesDroGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesOperFunc                        ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesOperation);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesCdrLockStatusGetFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesCdrLockStatusGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesGrayCodeEnableSetFunc           ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesGrayCodeEnableSet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesGrayCodeEnableGetFunc           ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesGrayCodeEnableGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPreCodeEnableSetFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPreCodeEnableSet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesPreCodeEnableGetFunc            ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesPreCodeEnableGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesRxInitFunc                      ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesRxInit);
#ifndef MV_HWS_REDUCED_BUILD
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesEomMatrixGetFunc                ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesEomMatrixGet);
    HWS_COMPHY_BIND_SERDES_FUNC(funcPtrArray, serdesSigDetectMaskFunc               ,COM_PHY_C112GX4, mvHwsComphyC112GX4SerdesSignalDetectMaskSet);
#endif

    return GT_OK;
}
#endif /* C112GX4 */

