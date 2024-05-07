/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsIronmanDevInit.c
*
* DESCRIPTION:
*     Ironman specific HW Services init
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIpcApis.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsIronmanPortAnp.h>

#include <cpss/generic/labservices/port/gop/silicon/ac5p/mvHwsAc5pPortIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ironman/mvHwsIronmanPortIf.h>

#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs10If.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>
#if !defined (IRONMAN_DEV_SUPPORT)
#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#endif

extern  GT_BOOL hwsPpHwTraceFlag;


extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanMtipMac10GBrUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanMtipMac10GUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanMtipMacGlobalExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanMtipMacPortExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanLpcsGeneralUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanLpcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanBaseRPcsUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanPcsFcFecUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanPcsExtUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanAnpUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC anUnitsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanSdwUnitsDb[];
extern const MV_HWS_SERDES_TXRX_TUNE_PARAMS alleycat5TxRxTuneParams[];

/************************* definition *****************************************************/

/************************* Globals *******************************************************/

extern MV_HWS_PORT_INIT_PARAMS *hwsPortsIronmanParams[];
extern const MV_HWS_PORT_INIT_PARAMS *hwsPortsIronman_ParamsSupModesMap[];
extern GT_STATUS hwsIronmanPortElementsDbInit(GT_U8 devNum);

/* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};

const MV_HWS_COMPHY_REG_FIELD_STC mvHwsComphyC12GP41P2VSip6_30_PinToRegMap[] =  /* based on SDW */
{
  /*  C12GP41P2V_PIN_RESET                = 0,  */ {IRONMAN_SDW_COMMON_GENERAL_CONTROL_0_ASYNC_RESET_E  , 0                     ,1},
  /*  C12GP41P2V_PIN_ISOLATION_ENB        = 1,  */ {MV_HWS_SERDES_REG_NA                                , 0                     ,1},
  /*  C12GP41P2V_PIN_BG_RDY               = 2,  */ {MV_HWS_SERDES_REG_NA                                , 0                     ,1},
  /*  C12GP41P2V_PIN_SIF_SEL              = 3,  */ {IRONMAN_SDW_COMMON_GENERAL_CONTROL_0_SIF_SEL_E      , 0                     ,1},
  /*  C12GP41P2V_PIN_PHY_MODE             = 4,  */ {MV_HWS_SERDES_TIED_PIN                              , 4/*<-default value*/  ,0},
  /*  C12GP41P2V_PIN_REFCLK_SEL           = 5,  */ {IRONMAN_SDW_LANE_CONTROL_0_REFCLK_SEL_E             , 0                     ,1},
  /*  C12GP41P2V_PIN_REF_FREF_SEL         = 6,  */ {IRONMAN_SDW_LANE_CONTROL_0_REF_FREF_SEL_E           , 0                     ,1},
  /*  C12GP41P2V_PIN_PHY_GEN_TX0          = 7,  */ {IRONMAN_SDW_LANE_CONTROL_0_PHY_GEN_TX_E             , 0                     ,1},
  /*  C12GP41P2V_PIN_PHY_GEN_RX0          = 8,  */ {IRONMAN_SDW_LANE_CONTROL_0_PHY_GEN_RX_E             , 0                     ,1},
  /*  C12GP41P2V_PIN_DFE_EN0              = 9,  */ {IRONMAN_SDW_LANE_CONTROL_0_DFE_EN_E                 , 0                     ,1},
  /*  C12GP41P2V_PIN_DFE_UPDATE_DIS0      = 10, */ {IRONMAN_SDW_LANE_CONTROL_0_DFE_UPDATE_DIS_E         , 0                     ,1},
  /*  C12GP41P2V_PIN_PU_PLL0              = 11, */ {IRONMAN_SDW_LANE_CONTROL_0_PU_PLL_E                 , 0                     ,1},
  /*  C12GP41P2V_PIN_PU_RX0               = 12, */ {IRONMAN_SDW_LANE_CONTROL_0_PU_RX_E                  , 0                     ,1},
  /*  C12GP41P2V_PIN_PU_TX0               = 13, */ {IRONMAN_SDW_LANE_CONTROL_0_PU_TX_E                  , 0                     ,1},
  /*  C12GP41P2V_PIN_TX_IDLE0             = 14, */ {IRONMAN_SDW_LANE_CONTROL_0_TX_IDLE_E                , 0                     ,1},
  /*  C12GP41P2V_PIN_PU_IVREF             = 15, */ {IRONMAN_SDW_COMMON_GENERAL_CONTROL_0_PU_IVREF_E     , 0                     ,1},
  /*  C12GP41P2V_PIN_RX_TRAIN_ENABLE0     = 16, */ {IRONMAN_SDW_LANE_CONTROL_1_RX_TRAIN_ENABLE_E        , 0                     ,1},
  /*  C12GP41P2V_PIN_RX_TRAIN_COMPLETE0   = 17, */ {IRONMAN_SDW_LANE_STATUS_0_RX_TRAIN_COMPLETE_E       , 0                     ,1},
  /*  C12GP41P2V_PIN_RX_TRAIN_FAILED0     = 18, */ {IRONMAN_SDW_LANE_STATUS_0_RX_TRAIN_FAILED_E         , 0                     ,1},
  /*  C12GP41P2V_PIN_TX_TRAIN_ENABLE0     = 19, */ {IRONMAN_SDW_LANE_CONTROL_1_TX_TRAIN_ENABLE_E        , 0                     ,1},
  /*  C12GP41P2V_PIN_TX_TRAIN_COMPLETE0   = 20, */ {IRONMAN_SDW_LANE_STATUS_0_TX_TRAIN_COMPLETE_E       , 0                     ,1},
  /*  C12GP41P2V_PIN_TX_TRAIN_FAILED0     = 21, */ {IRONMAN_SDW_LANE_STATUS_0_TX_TRAIN_FAILED_E         , 0                     ,1},
  /*  C12GP41P2V_PIN_SQ_DETECTED_LPF0     = 22, */ {IRONMAN_SDW_LANE_STATUS_0_SQ_DETECTED_LPF_E         , 0                     ,1},
  /*  C12GP41P2V_PIN_RX_INIT0             = 23, */ {IRONMAN_SDW_LANE_CONTROL_0_RX_INIT_E                , 0                     ,1},
  /*  C12GP41P2V_PIN_RX_INIT_DONE0        = 24, */ {IRONMAN_SDW_LANE_STATUS_0_RX_INIT_DONE_E            , 0                     ,1},
  /*  C12GP41P2V_PIN_DFE_PAT_DIS0         = 25, */ {IRONMAN_SDW_LANE_CONTROL_0_DFE_PAT_DIS_E            , 0                     ,1},
  /*  C12GP41P2V_PIN_PLL_READY_TX         = 26, */ {IRONMAN_SDW_LANE_STATUS_0_PLL_READY_TX_E            , 0                     ,1},
  /*  C12GP41P2V_PIN_PLL_READY_RX         = 27  */ {IRONMAN_SDW_LANE_STATUS_0_PLL_READY_RX_E            , 0                     ,1}
};
#define NA      NA_8BIT
#define NA_S    NA_16BIT_SIGNED
/** IM_COMPHY_12nm_Serdes_Init_rev0.4 */
const MV_HWS_SERDES_TXRX_TUNE_PARAMS ironmanTxRxTuneParams[] =
#ifndef WIN32
{  /* SerDes speed                                   PRE  PEAK POST                      RES_SEL RES_SHIFT CAP_SEL  SELMUFI SELMUFF SELMUPI SELMUPF MIN_CAP,  MAX_CAP,  MIN_RES,  MAX_RES, SQLCH*/
    {_1_25G,                   {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={0,       6,      0,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_1_25G_100FX,             {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={0,       0,     15,         3,      3,      5,      5,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_1_25G_SR_LR,             {.txComphyC12GP41P2V={ 0,  13,   0}}, {.rxComphyC12GP41P2V={0,       6,      0,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_3_125G,                  {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={4,       9,      1,         3,      3,      3,      3,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_2_578125,                {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={4,       9,      5,         3,      3,      3,      3,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_5G,                      {.txComphyC12GP41P2V={ 0,   9,   2}}, {.rxComphyC12GP41P2V={5,       1,     10,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_5_15625G,                {.txComphyC12GP41P2V={ 4,   0,   0}}, {.rxComphyC12GP41P2V={5,       1,     10,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,  10}}},
    {_10G,                     {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={4,       2,      6,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,   8}}},
    {_10_3125G,                {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={4,       2,      6,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,   8}}},
    {_10_3125G_SR_LR,          {.txComphyC12GP41P2V={ 0,   1,  10}}, {.rxComphyC12GP41P2V={4,       2,      6,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,   7}}},
    {_12_375G,                 {.txComphyC12GP41P2V={ 0,   0,   0}}, {.rxComphyC12GP41P2V={6,       3,     11,         3,      3,      4,      4,   NA_8BIT,  NA_8BIT,  NA_8BIT,  NA_8BIT,   8}}},

    {LAST_MV_HWS_SERDES_SPEED, {.txComphyC12GP41P2V={NA_8BIT, NA_8BIT, NA_8BIT}}, {.rxComphyC12GP41P2V={NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_8BIT, NA_16BIT_SIGNED}}}
};
#else
{
    {0}
};
#endif

HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes0Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes1Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes2Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes3Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes4Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes5Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes6Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes7Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes8Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes9Data  = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes10Data = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes11Data = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA ironmanSerdes12Data = {mvHwsComphyC12GP41P2VSip6_30_PinToRegMap, NULL, ironmanTxRxTuneParams, NULL};

HOST_CONST MV_HWS_PER_SERDES_INFO_STC ironmanSerdesDb[]=
{
    {NULL, 0,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes0Data}},
    {NULL, 1,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes1Data}},
    {NULL, 2,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes2Data}},
    {NULL, 3,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes3Data}},
    {NULL, 4,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes4Data}},
    {NULL, 5,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes5Data}},
    {NULL, 6,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes6Data}},
    {NULL, 7,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes7Data}},
    {NULL, 8,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes8Data}},
    {NULL, 9,  0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes9Data}},
    {NULL, 10, 0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes10Data}},
    {NULL, 11, 0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes11Data}},
    {NULL, 12, 0, COM_PHY_C12GP41P2V_SIP6_30, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&ironmanSerdes12Data}},
};
/************************* pre-declaration ***********************************************/


/*****************************************************************************************/

/*******************************************************************************
* hwsPcsIfInit
*
* DESCRIPTION:
*       Init all supported PCS types relevant for devices.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS pcsIfInit(GT_U8 devNum)
{
    MV_HWS_PCS_FUNC_PTRS **hwsPcsFuncsPtr;

    CHECK_STATUS(hwsPcsGetFuncPtr(devNum, &hwsPcsFuncsPtr));

    CHECK_STATUS(mvHwsMtiPcs10IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs10LpcsIfInit(hwsPcsFuncsPtr));

    return GT_OK;
}

/*******************************************************************************
* hwsMacIfInit
*
* DESCRIPTION:
*       Init all supported MAC types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS macIfInit(GT_U8 devNum)
{
    MV_HWS_MAC_FUNC_PTRS **hwsMacFuncsPtr;

    CHECK_STATUS(hwsMacGetFuncPtr(devNum, &hwsMacFuncsPtr));

    CHECK_STATUS(mvHwsMtiMac10Br_SIP6_30_IfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMtiMac10_SIP6_30_IfInit(hwsMacFuncsPtr));

    return GT_OK;
}

/*******************************************************************************
* hwsIronmanSerdesIfInit
*
* DESCRIPTION:
*       Init all supported Serdes types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static GT_STATUS hwsIronmanSerdesIfInit
(
    GT_U8 devNum,
    MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock,
    GT_BOOL callbackOnly
)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    GT_U32 sdIndex;
    serdesRefClock = serdesRefClock;

    if(GT_FALSE==callbackOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {
            CHECK_STATUS(mvHwsIronmanAnpSerdesSdwMuxSet(devNum, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, GT_FALSE));

            /* Enable SWD clk_4x_enable for 100FX port mode */
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_1_TX_CLK_4X_ENABLE_E, 1, NULL));
            CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, SERDES_UNIT, IRONMAN_SDW_LANE_CONTROL_1_RX_CLK_4X_ENABLE_E, 1, NULL));
        }
    }

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    CHECK_STATUS(mvHwsComphyIfInit(devNum,COM_PHY_C12GP41P2V_SIP6_30,hwsSerdesFuncsPtr,MV_HWS_COMPHY_FIRMWARE_NR,callbackOnly));
#ifndef MV_HWS_REDUCED_BUILD
    if(GT_FALSE==callbackOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {
            CHECK_STATUS(mvHwsSerdesBandGapOptimization(devNum, 0, sdIndex));
        }
    }
#endif
    return GT_OK;
}

GT_STATUS hwsIronmanRegDbInit
(
    GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = 0;
    rc += mvHwsRegDbInit(devNum, MTI_10G_MAC_BR_UNIT,              mvHwsIronmanMtipMac10GBrUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_MAC_NON_BR_UNIT,          mvHwsIronmanMtipMac10GUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_GLOBAL_EXT_UNIT,          mvHwsIronmanMtipMacGlobalExtUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_PORT_EXT_UNIT,            mvHwsIronmanMtipMacPortExtUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_PCS_LPCS_GENERAL_UNIT,    mvHwsIronmanLpcsGeneralUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_PCS_LPCS_PORT_UNIT,       mvHwsIronmanLpcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_PCS_BASER_UNIT,           mvHwsIronmanBaseRPcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_PCS_FCFEC_UNIT,           mvHwsIronmanPcsFcFecUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_10G_PCS_EXT_UNIT,             mvHwsIronmanPcsExtUnitsDb);
    rc += mvHwsRegDbInit(devNum, ANP_USX_UNIT,                     mvHwsIronmanAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, ANP_USX_O_UNIT,                   mvHwsIronmanAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, ANP_CPU_UNIT,                     mvHwsIronmanAnpUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_USX_UNIT,                      anUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_USX_O_UNIT,                    anUnitsDb);
    rc += mvHwsRegDbInit(devNum, AN_CPU_UNIT,                      anUnitsDb);
    rc += mvHwsRegDbInit(devNum, SERDES_UNIT,                      mvHwsIronmanSdwUnitsDb);

    return rc;
}


/**
* @internal hwsIronmanIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsIronmanIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
#ifndef MV_HWS_FREE_RTOS
    GT_U32 devId, revNum;
#endif
    GT_U8 i;

    if (funcPtr == NULL)
        return GT_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) ||
        (funcPtr->serdesRegSetAccess == NULL) ||(funcPtr->serdesRegGetAccess == NULL) ||
        (funcPtr->serverRegSetAccess == NULL) || (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL) || (funcPtr->deviceTemperatureGet == NULL)
#ifndef MV_HWS_FREE_RTOS
        || (funcPtr->timerGet == NULL)
#endif
        )
    {
        return GT_BAD_PTR;
    }

    hwsOsExactDelayPtr = funcPtr->osExactDelayPtr;
    hwsOsTimerWkFuncPtr = funcPtr->osTimerWkPtr;
    hwsOsMemSetFuncPtr = funcPtr->osMemSetPtr;
    hwsOsFreeFuncPtr = funcPtr->osFreePtr;
    hwsOsMallocFuncPtr = funcPtr->osMallocPtr;
    hwsSerdesRegSetFuncPtr = funcPtr->serdesRegSetAccess;
    hwsSerdesRegGetFuncPtr = funcPtr->serdesRegGetAccess;
    hwsDeviceTemperatureGetFuncPtr = funcPtr->deviceTemperatureGet;
    hwsServerRegSetFuncPtr = funcPtr->serverRegSetAccess;
    hwsServerRegGetFuncPtr = funcPtr->serverRegGetAccess;
    hwsOsMemCopyFuncPtr = funcPtr->osMemCopyPtr;
    hwsServerRegFieldSetFuncPtr = funcPtr->serverRegFieldSetAccess;
    hwsServerRegFieldGetFuncPtr = funcPtr->serverRegFieldGetAccess;
#ifndef MV_HWS_FREE_RTOS
    hwsTimerGetFuncPtr = funcPtr->timerGet;
#endif
    hwsRegisterSetFuncPtr = funcPtr->registerSetAccess;
    hwsRegisterGetFuncPtr = funcPtr->registerGetAccess;

    hwsDeviceSpecInfo[devNum].devType = Ironman;

    /* Default Serdes mode values (used in QSGMII/QXGMII modes) */
    for (i = 0; i < 3; i++)
    {
        hwsDeviceSpecInfo[devNum].gopMuxMode[i] = 1;
    }

#ifndef MV_HWS_FREE_RTOS
    /* Serdes mode values override, per flavor */
    funcPtr->sysDeviceInfo(devNum,&devId, &revNum);

    if (devId == CPSS_98DX2556_CNS  ||
        devId == CPSS_98DX2556M_CNS ||
        devId == CPSS_98DXC254_CNS)
    {
        for (i = 0; i < 3; i++)
        {
            hwsDeviceSpecInfo[devNum].gopMuxMode[i] = 0;
        }
    }
    else
    {
        if (devId == CPSS_98DX2558_CNS  ||
            devId == CPSS_98DX2558M_CNS ||
            devId == CPSS_98DX2568_CNS  ||
            devId == CPSS_98DX2568M_CNS)
        {
            hwsDeviceSpecInfo[devNum].gopMuxMode[2] = 0;
        }
    }

    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;
#endif
    return hwsIronmanRegDbInit(devNum);
}

static GT_STATUS hwsIronmanSerdesGetClockAndInit
(
    GT_U8 devNum
)
{
    MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock;
#ifndef MV_HWS_REDUCED_BUILD
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

#ifndef IRONMAN_DEV_SUPPORT
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc != NULL)
    {
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc(devNum, &serdesRefClock));
    }
#else
    serdesRefClock = MHz_25;
#endif

#ifndef MV_HWS_REDUCED_BUILD
    cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
         (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
         (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E))
    {
        /* skip HW write */
        CHECK_STATUS(hwsIronmanSerdesIfInit(devNum,serdesRefClock,GT_TRUE));
    }
    else
    {
        /* Init SERDES unit relevant for current device */
        CHECK_STATUS(hwsIronmanSerdesIfInit(devNum,serdesRefClock,GT_FALSE));
    }
#else
        CHECK_STATUS(hwsIronmanSerdesIfInit(devNum,serdesRefClock,GT_TRUE));
#endif
    return GT_OK;
}

#ifndef IRONMAN_DEV_SUPPORT
GT_STATUS hwsIronmanPortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsIronman_ParamsSupModesMap;

    return GT_OK;
}
#endif

GT_VOID hwsIronmanIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    MV_HWS_DEV_FUNC_PTRS      *hwsDevFunc;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if ( funcPtr )
    {
        if ( hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL )
        {
            hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
        }
        if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc == NULL)
        {
            hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc = (MV_HWS_SERDES_REF_CLOCK_GET_FUNC_PTR)(funcPtr->serdesRefClockGet);
        }

#ifndef MV_HWS_FREE_RTOS
        if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc == NULL)
        {
            hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc = funcPtr->coreClockGetPtr;
        }
#endif
    }
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = NULL;

    /* Configures port init / reset functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsHawkPortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAnpStartFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsHawkPortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortGeneralAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
#if !defined MV_HWS_REDUCED_BUILD
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = hwsIronmanPortParamsSet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = hwsIronmanPortParamsGetLanes;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc = mvHwsIronmanExtMacClockEnableGet;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devUnitFieldFindAddrFunc = original_genUnitFindAddressPrv;
}


/*******************************************************************************
* hwsIronmanIfInit
*
* DESCRIPTION:
*       Init all supported units needed for port initialization.
*       Must be called per device.
*
* INPUTS:
*       funcPtr - pointer to structure that hold the "os"
*                 functions needed be bound to HWS.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
GT_STATUS hwsIronmanIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_U32                    data, version;
    MV_HWS_DEV_TYPE           devType;
    MV_HWS_DEV_FUNC_PTRS      *hwsDevFunc;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32                    sdIndex;
    GT_BOOL                   skipWritingHW = GT_FALSE;
#endif

    if (hwsInitDone[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifndef MV_HWS_REDUCED_BUILD
    if (funcPtr == NULL)
    {
        skipWritingHW = GT_TRUE;
    }
#endif

#ifdef MV_HWS_REDUCED_BUILD
    CHECK_STATUS(hwsIronmanIfPreInit(devNum, funcPtr));
#endif

    hwsDeviceSpecInfo[devNum].devType = Ironman;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;

    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_C12GP41P2V_SIP6_30; /* most common SD type in the device */
    hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(ironmanSerdesDb) / sizeof(ironmanSerdesDb[0]);


#ifndef MV_HWS_REDUCED_BUILD
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
    for (sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
    {
        CHECK_STATUS(mvHwsComphySerdesAddToDatabase(devNum, sdIndex, &ironmanSerdesDb[sdIndex]));
    }
#else
    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = ironmanSerdesDb;
#endif

#if (!defined MV_HWS_REDUCED_BUILD)
    CHECK_STATUS(hwsIronmanPortsParamsSupModesMapSet(devNum));

#endif
    hwsDeviceSpecInfo[devNum].portsNum = 55;

    devType = hwsDeviceSpecInfo[devNum].devType;

    hwsIronmanIfInitHwsDevFunc(devNum,funcPtr);

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);

    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;
#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Ironman ifInit start ******\n");
    }
#endif

    /* Init HWS Firmware */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwInit(devNum, 0, 0, NULL));

    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */
    if(hwsInitDone[devNum] == GT_FALSE)
    {
#ifndef MV_HWS_REDUCED_BUILD
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
#else
        CHECK_STATUS(hwsIronmanPortsElementsCfg(devNum, 0));
#endif
    }

    CHECK_STATUS(hwsIronmanSerdesGetClockAndInit(devNum));

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));

    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));
    /* Init all ANP units relevant for current device */
#ifndef MV_HWS_REDUCED_BUILD
    CHECK_STATUS(mvHwsIronmanAnpInit(devNum,skipWritingHW /* false */));
#endif

    hwsInitDone[devNum] = GT_TRUE;

    /* Update HWS After Firmware load and init */
    if (mvHwsServiceCpuEnableGet(devNum))
        CHECK_STATUS(mvHwsServiceCpuFwPostInit(devNum, devNum));

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** IronmanifInit end ********\n");
    }
#endif
    return GT_OK;
}

#ifdef SHARED_MEMORY

GT_STATUS hwsIronmanIfReload
(
    GT_U8 devNum
)
{
    MV_HWS_DEV_FUNC_PTRS      *hwsDevFunc;
    MV_HWS_REF_CLOCK_SUP_VAL  serdesRefClock;

   hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    CHECK_STATUS(macIfInit(devNum));

    /* By default - give control over the SerDes to the register-file instead of the ANP */
    if ((hwsDevFunc!= NULL)&&(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc != NULL))
    {
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc(devNum, &serdesRefClock));

        /* Init SERDES unit relevant for current device */
        CHECK_STATUS(hwsIronmanSerdesIfInit(devNum,serdesRefClock,GT_TRUE));
    }


    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    return GT_OK;
}
#endif

#ifndef MV_HWS_REDUCED_BUILD
/*******************************************************************************
* hwsSerdesIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported serdes types.
*
* INPUTS:
*       GT_U8 devNum
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void serdesIfClose(GT_U8 devNum)
{
#ifndef IRONMAN_DEV_SUPPORT
    CPSS_TBD_BOOKMARK_IRONMAN
#endif

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsPcsIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported PCS types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void pcsIfClose
(
    GT_U8 devNum
)
{
#ifndef IRONMAN_DEV_SUPPORT
    CPSS_TBD_BOOKMARK_IRONMAN
#endif

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsMacIfClose
*
* DESCRIPTION:
*       Free all resources allocated by supported MAC types.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
static void macIfClose
(
    GT_U8 devNum
)
{
#ifndef IRONMAN_DEV_SUPPORT
    CPSS_TBD_BOOKMARK_IRONMAN
#endif

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsIronmanIfClose
*
* DESCRIPTION:
*       Free all resource allocated for ports initialization.
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       0  - on success
*       1  - on error
*
*******************************************************************************/
void hwsIronmanIfClose(GT_U8 devNum)
{
    if (hwsInitDone[devNum])
    {
        hwsInitDone[devNum] = GT_FALSE;
    }

    serdesIfClose(devNum);
    pcsIfClose(devNum);
    macIfClose(devNum);

    hwsSerdesIfClose(devNum);
    hwsPcsIfClose(devNum);
    hwsMacIfClose(devNum);

    /*mvHwsAnpClose(devNum);*/

    hwsPortsElementsClose(devNum);
}

#endif




