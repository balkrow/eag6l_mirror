/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsIronmanPortModeElements.c
*
* DESCRIPTION:
*
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>
#include <cpss/common/labServices/port/gop/port/private/mvHwsPortPrvIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/generic/labservices/port/gop/silicon/ac5x/mvHwsAc5xPortIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCfgIf.h>

/************************* definition *****************************************************/
/************************* Globals ********************************************************/



/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsIronmanPort0SupModes[] = {
    /* port mode,    MAC Type       Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { _5GBaseR,      MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF | FC_FEC},
    { _10G_OUSGMII,  MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { QSGMII,        MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5G_QUSGMII,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { SGMII,         MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _100Base_FX,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _1_25G_100FX,    NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10G_SXGMII,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_SXGMII,    MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2_5G_SXGMII,  MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _2_578125,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { NON_SUP_MODE,  MAC_NA,         NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsIronmanPort1SupModes[] = {
    /* port mode,    MAC Type       Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { _10G_OUSGMII,  MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS     , 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { QSGMII,        MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5G_QUSGMII,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { NON_SUP_MODE,  MAC_NA,         NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsIronmanPort24SupModes[] = {
    /* port mode,    MAC Type       Mac Num   PCS Type           PCS Num  FEC             Speed          FirstLane  LanesList       LanesNum    Media          10Bit     FecSupported */
    { _10G_OUSGMII,  MTI_MAC_10G,      0,     MTI_PCS_10G_LPCS     , 0,   FEC_OFF,        _10G,            NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _10G_QXGMII,   MTI_MAC_10G,      0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { QSGMII,        MTI_MAC_10G,      0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _5G_QUSGMII,   MTI_MAC_10G,      0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _5G,             NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { NON_SUP_MODE,  MAC_NA,         NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsIronmanPort48SupModes[] = {
    /* port mode,    MAC Type       Mac Num   PCS Type           PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { _5GBaseR,      MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF | FC_FEC},
    { _12GBaseR,     MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _12_375G,        NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF | FC_FEC},
    { SGMII,         MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _100Base_FX,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_LPCS,      0,   FEC_OFF,        _1_25G_100FX,    NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10G_SXGMII,   MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_SXGMII,    MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2_5G_SXGMII,  MTI_MAC_10G_BR,   0,     MTI_PCS_10G_BASE_R,    0,   FEC_OFF,        _2_578125,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { NON_SUP_MODE,  MAC_NA,         NA_NUM,  PCS_NA,           NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsIronmanPort52SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { _5GBaseR,      MTI_MAC_10G,       0,     MTI_PCS_10G_BASE_R,   0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_KR,   MTI_MAC_10G,       0,     MTI_PCS_10G_BASE_R,   0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { _10GBase_SR_LR,MTI_MAC_10G,       0,     MTI_PCS_10G_BASE_R,   0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF|FC_FEC},
    { SGMII,         MTI_MAC_10G,       0,     MTI_PCS_10G_LPCS,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _1000Base_X,   MTI_MAC_10G,       0,     MTI_PCS_10G_LPCS,     0,   FEC_OFF,        _1_25G,          NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2500Base_X,   MTI_MAC_10G,       0,     MTI_PCS_10G_LPCS,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { SGMII2_5,      MTI_MAC_10G,       0,     MTI_PCS_10G_LPCS,     0,   FEC_OFF,        _3_125G,         NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _100Base_FX,   MTI_MAC_10G,       0,     MTI_PCS_10G_LPCS,     0,   FEC_OFF,        _1_25G_100FX,    NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _10G_SXGMII,   MTI_MAC_10G,       0,     MTI_PCS_10G_BASE_R,   0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _40BIT_ON, FEC_OFF   },
    { _5G_SXGMII,    MTI_MAC_10G,       0,     MTI_PCS_10G_BASE_R,   0,   FEC_OFF,        _5_15625G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { _2_5G_SXGMII,  MTI_MAC_10G,       0,     MTI_PCS_10G_BASE_R,   0,   FEC_OFF,        _2_578125,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _20BIT_ON, FEC_OFF   },
    { NON_SUP_MODE,  MAC_NA,          NA_NUM,  PCS_NA,          NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* the 55 MACs in the GOP of the Ironman */
#define IRONMAN_P0_P3_SUP_MODES    hwsIronmanPort0SupModes /* 0*/,hwsIronmanPort1SupModes /* 1*/, hwsIronmanPort1SupModes /* 2*/, hwsIronmanPort1SupModes /* 3*/
#define IRONMAN_P4_P7_SUP_MODES    hwsIronmanPort0SupModes /* 4*/,hwsIronmanPort1SupModes /* 5*/, hwsIronmanPort1SupModes /* 6*/, hwsIronmanPort1SupModes /* 7*/
#define IRONMAN_P8_P15_SUP_MODES   IRONMAN_P0_P3_SUP_MODES, IRONMAN_P4_P7_SUP_MODES
#define IRONMAN_P16_P23_SUP_MODES  IRONMAN_P8_P15_SUP_MODES
#define IRONMAN_P24_P31_SUP_MODES  hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes
#define IRONMAN_P32_P39_SUP_MODES  hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes
#define IRONMAN_P40_P47_SUP_MODES  hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes, hwsIronmanPort24SupModes
#define IRONMAN_P48_51_SUP_MODES   hwsIronmanPort48SupModes /*48*/, hwsIronmanPort48SupModes /*49*/, hwsIronmanPort48SupModes /*50*/, hwsIronmanPort48SupModes /*51*/
#define IRONMAN_P52_54_SUP_MODES   hwsIronmanPort52SupModes /*52*/, hwsIronmanPort52SupModes /*53*/, hwsIronmanPort52SupModes /*54*/

/* map the right supported modes table for each port */
const MV_HWS_PORT_INIT_PARAMS* const hwsPortsIronman_ParamsSupModesMap[] =
{
     IRONMAN_P0_P3_SUP_MODES
    ,IRONMAN_P4_P7_SUP_MODES
    ,IRONMAN_P8_P15_SUP_MODES
    ,IRONMAN_P16_P23_SUP_MODES
    ,IRONMAN_P24_P31_SUP_MODES
    ,IRONMAN_P32_P39_SUP_MODES
    ,IRONMAN_P40_P47_SUP_MODES
    ,IRONMAN_P48_51_SUP_MODES
    ,IRONMAN_P52_54_SUP_MODES
};

#if (!defined(MV_HWS_REDUCED_BUILD_EXT_CM3))
GT_STATUS hwsIronmanPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
    /*GT_U16    laneCtnt = 0;*/
    /*GT_U16    *activeLanes = NULL;*/
    /*GT_U16    i = 0;*/
    MV_HWS_HAWK_CONVERT_STC         convertIdx;
    GT_U8 gopNum = 0;

      /* SerDes-Muxing related variables */
    /*MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;
    GT_U32                          phyPortNumTmp;
    GT_U16                          idx;*/
    /* avoid warnings */
    portGroup = portGroup;

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    /*Check which GoP this port belongs to (used for SD muxing, valid only for QSGMII/QXGMII modes, ports 48-51 are NOT relevant )*/
    if ((phyPortNum <= 7) || (phyPortNum >= 24 && phyPortNum <= 31))
    {
        gopNum = 0;
    }
    else if ((phyPortNum <= 15) || (phyPortNum >= 32 && phyPortNum <= 39))
    {
        gopNum = 1;
    }
    else if ((phyPortNum <= 23) || (phyPortNum >= 40 && phyPortNum <= 47))
    {
        gopNum = 2;
    }

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portParams->portStandard, &convertIdx));

    if(mvHwsMtipIsReducedPort(devNum,phyPortNum))
    {
        if(phyPortNum == 52)
        {
            portParams->activeLanesList[0] = 2;
        }
        else if(phyPortNum == 53)
        {
            portParams->activeLanesList[0] = 5;
        }
        else if(phyPortNum == 54)
        {
            portParams->activeLanesList[0] = 8;
        }
        else    /* MACs: 48..51 => SDs: 9..12 */
        {
            portParams->activeLanesList[0] = 9 + (phyPortNum - 48);
        }

        portParams->firstLaneNum = portParams->activeLanesList[0];
    }
    else
    {
        if ((portParams->portStandard == _5GBaseR)    ||
            (portParams->portStandard == _10GBase_KR)    ||
            (portParams->portStandard == _10GBase_SR_LR) ||
            (portParams->portStandard == SGMII)          ||
            (portParams->portStandard == _100Base_FX)    ||
            (portParams->portStandard == _2500Base_X)    ||
            (portParams->portStandard == SGMII2_5)       ||
            (portParams->portStandard == _1000Base_X)    ||
            (portParams->portStandard == _2_5G_SXGMII) ||
            (portParams->portStandard == _5G_SXGMII) ||
            (portParams->portStandard == _10G_SXGMII))
        {
            if(phyPortNum == 0)
            {
                portParams->activeLanesList[0] = 0;
            }
            else if(phyPortNum == 4)
            {
                portParams->activeLanesList[0] = 1;
            }
            else if(phyPortNum == 8)
            {
                portParams->activeLanesList[0] = 3;
            }
            else if(phyPortNum == 12)
            {
                portParams->activeLanesList[0] = 4;
            }
            else if(phyPortNum == 16)
            {
                portParams->activeLanesList[0] = 6;
            }
            else if(phyPortNum == 20)
            {
                portParams->activeLanesList[0] = 7;
            }
            else
            {
                return GT_NOT_SUPPORTED;
            }
        }
        else
        {
            if(HWS_USX_O_MODE_CHECK(portParams->portStandard))
            {
                if(phyPortNum < 24)
                {
                    portParams->activeLanesList[0]  = (GT_U16)((convertIdx.ciderUnit)*3);
                }
                else
                {
                    portParams->activeLanesList[0]  = (GT_U16)((convertIdx.ciderUnit)*3 + 1);
                }
            }
            else
            {
                if(hwsDeviceSpecInfo[devNum].gopMuxMode[gopNum] == 1) /* mode 1 */
                {
                    if((phyPortNum % 8) < 4)
                    {
                        portParams->activeLanesList[0]  = (GT_U16)((convertIdx.ciderUnit)*3 + 1);
                    }
                    else
                    {
                        portParams->activeLanesList[0]  = (GT_U16)((convertIdx.ciderUnit)*3 + 2);
                    }

                }
                else /* mode 0 */
                {

                    if((phyPortNum % 8) < 4)
                    {
                        portParams->activeLanesList[0]  = (GT_U16)((convertIdx.ciderUnit)*3 + 1);
                    }
                    else
                    {
                        portParams->activeLanesList[0]  = (GT_U16)((convertIdx.ciderUnit)*3 + 0);
                    }

                }
            }
        }

        portParams->firstLaneNum = portParams->activeLanesList[0];

    }

    return GT_OK;
}

GT_STATUS hwsIronmanPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    const MV_HWS_PORT_INIT_PARAMS   *newPortParams;
    GT_U32                          portModeIndex;
    GT_U8                           i;
    GT_U32                          serdesIndex;
    MV_HWS_PER_SERDES_INFO_PTR      serdesInfo;

    if (phyPortNum >= hwsDeviceSpecInfo[devNum].portsNum)
    {
        return GT_OUT_OF_RANGE;
    }

    curPortParams = &curDevicePorts[phyPortNum].curPortParams;
    if (NON_SUP_MODE == portMode) /* Reset port */
    {
        /* initialize entry with default port params */
        hwsOsMemCopyFuncPtr(curPortParams, &hwsSingleEntryInitParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
        curPortParams->portMacNumber = (GT_U16)phyPortNum;
        curPortParams->portPcsNumber = (GT_U16)phyPortNum;
        curPortParams->firstLaneNum  = (GT_U16)phyPortNum;

        if (mvHwsUsxModeCheck(devNum,phyPortNum,curPortParams->portStandard))
        {
            CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,curPortParams->activeLanesList[0],&serdesIndex,&serdesInfo));
            for (i = 0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
            {
                /* clear only this port in the bundle */
                if(serdesInfo->serdesToMac[i] == phyPortNum)
                {
                    serdesInfo->serdesToMac[i] = NA_NUM;
                }
            }
        }
        else
        {
            for (i = 0; i < curPortParams->numOfActLanes; i++)
            {
                CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,curPortParams->activeLanesList[i],&serdesIndex,&serdesInfo));
                serdesInfo->serdesToMac[0] = NA_NUM;
            }
        }

        return GT_OK;
    }

    newPortParams = hwsPortsElementsFindMode(hwsDevicePortsElementsSupModesCatalogGet(devNum,phyPortNum),portMode,&portModeIndex);
    if ( (NULL == newPortParams) || (newPortParams->numOfActLanes == 0))
    {
        return GT_NOT_SUPPORTED;
    }
    /* Copy the right port mode line from the port's specific supported modes list*/
    hwsOsMemCopyFuncPtr(curPortParams, newPortParams, sizeof(MV_HWS_PORT_INIT_PARAMS));
    curPortParams->portMacNumber = (GT_U16)phyPortNum;
    curPortParams->portPcsNumber = (GT_U16)phyPortNum;
    curPortParams->portFecMode   = curDevicePorts[phyPortNum].perModeFecList[portModeIndex];

    /* build active lane list */
    CHECK_STATUS(hwsPortParamsGetLanes(devNum,portGroup,phyPortNum,curPortParams));

        /* save the mac port for the serdes num in the database*/
    if (mvHwsUsxModeCheck(devNum,phyPortNum,curPortParams->portStandard))
    {
        GT_BOOL alreadyExist = GT_FALSE;
        CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,curPortParams->activeLanesList[0],&serdesIndex,&serdesInfo));
        for (i = 0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
        {
            /* check if already exists, if so, no need to save is mac port to serdes*/
            if(serdesInfo->serdesToMac[i] == phyPortNum)
            {
                alreadyExist = GT_TRUE;
                break;
            }
        }
        if (alreadyExist == GT_FALSE)
        {
            for (i = 0; i < MV_HWS_MAX_LANES_NUM_PER_PORT; i++)
            {
                /* not exist, find the first empty place and save mac port to serdes */
                if(serdesInfo->serdesToMac[i] == NA_NUM)
                {
                    serdesInfo->serdesToMac[i] = phyPortNum;
                    break;
                }
            }
        }

    }
    else
    {
        for (i = 0 ; i < curPortParams->numOfActLanes ; i++)
        {
            CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,curPortParams->activeLanesList[i],&serdesIndex,&serdesInfo));
            serdesInfo->serdesToMac[0] = phyPortNum;
        }
    }

    return GT_OK;

}
#endif

