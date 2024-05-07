/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* mvHwsAasDevInit.c
*
* DESCRIPTION:
*     Aas specific HW Services init
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
#include <cpss/common/labServices/port/gop/port/mvHwsPortAnp.h>
#include <cpss/common/labServices/port/gop/port/mvHwsAasPortAnp.h>

#include <cpss/generic/labservices/port/gop/silicon/aas/mvHwsAasPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralPortIf.h>
#include <cpss/common/labServices/port/gop/port/silicon/general/mvHwsGeneralCpll.h>

#include <cpss/common/labServices/port/gop/port/mac/mvHwsMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mtiCpuMac/mvHwsMtiCpuMacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti400Mac/mvHwsMti400MacIf.h>
#include <cpss/common/labServices/port/gop/port/mac/mti100Mac/mvHwsMti100MacIf.h>

#include <cpss/common/labServices/port/gop/port/pcs/mvHwsPcsIf.h>
#include <cpss/common/labServices/port/gop/port/pcs/mtiPcs/mvHwsMtiPcs800If.h>

#include <cpss/common/labServices/port/gop/port/serdes/mvHwsSerdesPrvIf.h>
#include <cpss/common/labServices/port/gop/port/serdes/comPhy/mvComphyIf.h>

#include <cpssCommon/private/prvCpssEmulatorMode.h>
#include <cpss/generic/hwDriver/cpssHwDriverGeneric.h>
#ifndef MICRO_INIT
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#endif
extern  GT_BOOL hwsPpHwTraceFlag;

GT_STATUS mvHwsAasRegisterMisconfigurationSet
(
    IN GT_U8    devNum
);

/************************* definition *****************************************************/
const MV_HWS_COMPHY_REG_FIELD_STC mvHwsAasComphyN5XC56GP5X4PinToRegMapSdw[] = {{0,0,0}};
#define NA      NA_16BIT
#define NA_S    NA_16BIT_SIGNED

#if 0
const MV_HWS_SERDES_TXRX_TUNE_PARAMS aasTxRxTuneParams[] =
{
    {0}
};
#endif

HOST_CONST MV_HWS_COMPHY_SERDES_DATA aasSerdes0Data  = {mvHwsAasComphyN5XC56GP5X4PinToRegMapSdw, NULL, NULL, NULL};
HOST_CONST MV_HWS_COMPHY_SERDES_DATA aasSerdes8Data  = {mvHwsAasComphyN5XC56GP5X4PinToRegMapSdw, NULL, NULL, NULL};

HOST_CONST MV_HWS_PER_SERDES_INFO_STC aasSerdesDb[]=
{
    {NULL, 0,  0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 1,  1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 2,  2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 3,  3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 4,  0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 5,  1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 6,  2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 7,  3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 8,  0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 9,  1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 10, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 11, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 12, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 13, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 14, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 15, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 16, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 17, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 18, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 19, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 20, 0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 21, 1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 22, 2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 23, 3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 24, 0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 25, 1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 26, 2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 27, 3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 28, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 29, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 30, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 31, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 32, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 33, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 34, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 35, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 36, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 37, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 38, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 39, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 40, 0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 41, 1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 42, 2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 43, 3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 44, 0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 45, 1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 46, 2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 47, 3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 48, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 49, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 50, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 51, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 52, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 53, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 54, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 55, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 56, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 57, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 58, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 59, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 60, 0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 61, 1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 62, 2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 63, 3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 64, 0, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 65, 1, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 66, 2, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 67, 3, COM_PHY_C112GX4,     {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 68, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 69, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 70, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 71, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 72, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 73, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 74, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 75, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 76, 0, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 77, 1, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 78, 2, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 79, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
/*CPU*/
    {NULL, 80, 3, COM_PHY_N5XC56GP5X4, {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},

};

#define AAS_SD_DATA(_dp,_sd) \
    HOST_CONST MV_HWS_COMPHY_SERDES_DATA dp##_dp##_aasSerdes##_sd##Data = {mvHwsAasComphyN5XC56GP5X4PinToRegMapSdw, NULL, NULL, NULL}

AAS_SD_DATA(0,0);
AAS_SD_DATA(0,1);
AAS_SD_DATA(0,2);
AAS_SD_DATA(0,3);
AAS_SD_DATA(0,4);
AAS_SD_DATA(0,5);
AAS_SD_DATA(0,6);
AAS_SD_DATA(0,7);
AAS_SD_DATA(0,8);
AAS_SD_DATA(0,9);
AAS_SD_DATA(0,10);
AAS_SD_DATA(0,11);
AAS_SD_DATA(0,12);
AAS_SD_DATA(0,13);
AAS_SD_DATA(0,14);
AAS_SD_DATA(0,15);
AAS_SD_DATA(0,16);
AAS_SD_DATA(0,17);
AAS_SD_DATA(0,18);
AAS_SD_DATA(0,19);
AAS_SD_DATA(1,0);
AAS_SD_DATA(1,1);
AAS_SD_DATA(1,2);
AAS_SD_DATA(1,3);
AAS_SD_DATA(1,4);
AAS_SD_DATA(1,5);
AAS_SD_DATA(1,6);
AAS_SD_DATA(1,7);
AAS_SD_DATA(1,8);
AAS_SD_DATA(1,9);
AAS_SD_DATA(1,10);
AAS_SD_DATA(1,11);
AAS_SD_DATA(1,12);
AAS_SD_DATA(1,13);
AAS_SD_DATA(1,14);
AAS_SD_DATA(1,15);
AAS_SD_DATA(1,16);
AAS_SD_DATA(1,17);
AAS_SD_DATA(1,18);
AAS_SD_DATA(1,19);
AAS_SD_DATA(2,0);
AAS_SD_DATA(2,1);
AAS_SD_DATA(2,2);
AAS_SD_DATA(2,3);
AAS_SD_DATA(2,4);
AAS_SD_DATA(2,5);
AAS_SD_DATA(2,6);
AAS_SD_DATA(2,7);
AAS_SD_DATA(2,8);
AAS_SD_DATA(2,9);
AAS_SD_DATA(2,10);
AAS_SD_DATA(2,11);
AAS_SD_DATA(2,12);
AAS_SD_DATA(2,13);
AAS_SD_DATA(2,14);
AAS_SD_DATA(2,15);
AAS_SD_DATA(2,16);
AAS_SD_DATA(2,17);
AAS_SD_DATA(2,18);
AAS_SD_DATA(2,19);
AAS_SD_DATA(3,0);
AAS_SD_DATA(3,1);
AAS_SD_DATA(3,2);
AAS_SD_DATA(3,3);
AAS_SD_DATA(3,4);
AAS_SD_DATA(3,5);
AAS_SD_DATA(3,6);
AAS_SD_DATA(3,7);
AAS_SD_DATA(3,8);
AAS_SD_DATA(3,9);
AAS_SD_DATA(3,10);
AAS_SD_DATA(3,11);
AAS_SD_DATA(3,12);
AAS_SD_DATA(3,13);
AAS_SD_DATA(3,14);
AAS_SD_DATA(3,15);
AAS_SD_DATA(3,16);
AAS_SD_DATA(3,17);
AAS_SD_DATA(3,18);
AAS_SD_DATA(3,19);
AAS_SD_DATA(0,_cpu_);

#define  init_values_MAC {NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM, NA_NUM}


HOST_CONST MV_HWS_PER_SERDES_INFO_STC aas_2_tiles_SerdesDb[]=
{
    /*Tile 0*/
    {NULL, 0,  0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 1,  1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 2,  2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 3,  3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 4,  0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 5,  1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 6,  2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 7,  3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 8,  0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 9,  1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 10, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 11, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 12, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 13, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 14, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 15, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 16, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 17, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 18, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 19, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 20, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 21, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 22, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 23, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 24, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 25, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 26, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 27, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 28, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 29, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 30, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 31, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 32, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 33, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 34, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 35, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 36, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 37, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 38, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 39, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 40, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 41, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 42, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 43, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 44, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 45, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 46, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 47, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 48, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 49, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 50, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 51, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 52, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 53, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 54, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 55, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 56, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 57, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 58, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 59, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 60, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 61, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 62, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 63, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 64, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 65, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 66, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 67, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 68, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 69, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 70, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 71, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 72, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 73, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 74, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 75, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 76, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 77, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 78, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 79, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    /*Tile 1*/
    {NULL, 80, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 81, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 82, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 83, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 84, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 85, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 86, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 87, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 88, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 89, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 90, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 91, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 92, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 93, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 94, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 95, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 96, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 97, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 98, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 99, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 100, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 101, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 102, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 103, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 104, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 105, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 106, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 107, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 108, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 109, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 110, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 111, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 112, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 113, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 114, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 115, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 116, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 117, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 118, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 119, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 120, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 121, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 122, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 123, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 124, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 125, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 126, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 127, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 128, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 129, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 130, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 131, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 132, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 133, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 134, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 135, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 136, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 137, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 138, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 139, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 140, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 141, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 142, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 143, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 144, 0, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 145, 1, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 146, 2, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 147, 3, COM_PHY_C112GX4,     init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes0Data }},
    {NULL, 148, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 149, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data }},
    {NULL, 150, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 151, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 152, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 153, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 154, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 155, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 156, 0, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 157, 1, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 158, 2, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    {NULL, 159, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    /*CPU Tile 0*/
    {NULL, 160, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
    /*CPU Tile 1*/
    {NULL, 161, 3, COM_PHY_N5XC56GP5X4, init_values_MAC, {(MV_HWS_COMPHY_SERDES_DATA*)&aasSerdes8Data}},
};


AAS_SD_DATA(4,0);
AAS_SD_DATA(4,1);
AAS_SD_DATA(4,2);
AAS_SD_DATA(4,3);
AAS_SD_DATA(4,4);
AAS_SD_DATA(4,5);
AAS_SD_DATA(4,6);
AAS_SD_DATA(4,7);
AAS_SD_DATA(4,8);
AAS_SD_DATA(4,9);
AAS_SD_DATA(4,10);
AAS_SD_DATA(4,11);
AAS_SD_DATA(4,12);
AAS_SD_DATA(4,13);
AAS_SD_DATA(4,14);
AAS_SD_DATA(4,15);
AAS_SD_DATA(4,16);
AAS_SD_DATA(4,17);
AAS_SD_DATA(4,18);
AAS_SD_DATA(4,19);
AAS_SD_DATA(5,0);
AAS_SD_DATA(5,1);
AAS_SD_DATA(5,2);
AAS_SD_DATA(5,3);
AAS_SD_DATA(5,4);
AAS_SD_DATA(5,5);
AAS_SD_DATA(5,6);
AAS_SD_DATA(5,7);
AAS_SD_DATA(5,8);
AAS_SD_DATA(5,9);
AAS_SD_DATA(5,10);
AAS_SD_DATA(5,11);
AAS_SD_DATA(5,12);
AAS_SD_DATA(5,13);
AAS_SD_DATA(5,14);
AAS_SD_DATA(5,15);
AAS_SD_DATA(5,16);
AAS_SD_DATA(5,17);
AAS_SD_DATA(5,18);
AAS_SD_DATA(5,19);
AAS_SD_DATA(6,0);
AAS_SD_DATA(6,1);
AAS_SD_DATA(6,2);
AAS_SD_DATA(6,3);
AAS_SD_DATA(6,4);
AAS_SD_DATA(6,5);
AAS_SD_DATA(6,6);
AAS_SD_DATA(6,7);
AAS_SD_DATA(6,8);
AAS_SD_DATA(6,9);
AAS_SD_DATA(6,10);
AAS_SD_DATA(6,11);
AAS_SD_DATA(6,12);
AAS_SD_DATA(6,13);
AAS_SD_DATA(6,14);
AAS_SD_DATA(6,15);
AAS_SD_DATA(6,16);
AAS_SD_DATA(6,17);
AAS_SD_DATA(6,18);
AAS_SD_DATA(6,19);
AAS_SD_DATA(7,0);
AAS_SD_DATA(7,1);
AAS_SD_DATA(7,2);
AAS_SD_DATA(7,3);
AAS_SD_DATA(7,4);
AAS_SD_DATA(7,5);
AAS_SD_DATA(7,6);
AAS_SD_DATA(7,7);
AAS_SD_DATA(7,8);
AAS_SD_DATA(7,9);
AAS_SD_DATA(7,10);
AAS_SD_DATA(7,11);
AAS_SD_DATA(7,12);
AAS_SD_DATA(7,13);
AAS_SD_DATA(7,14);
AAS_SD_DATA(7,15);
AAS_SD_DATA(7,16);
AAS_SD_DATA(7,17);
AAS_SD_DATA(7,18);
AAS_SD_DATA(7,19);
AAS_SD_DATA(4,_cpu_);

/************************* Globals *******************************************************/

extern const MV_HWS_PORT_INIT_PARAMS **hwsPortsAasParamsSupModesMap;
extern GT_VOID mvHwsAasPortElementsDbInit(GT_U8 devNum);


#ifdef MV_HWS_REDUCED_BUILD
 /* init per device */
static GT_BOOL hwsInitDone[HWS_MAX_DEVICE_NUM] = {0};
static GT_BOOL aasDbInitDone = GT_FALSE;
#define PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    _var
#else
#define PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.labServicesDir.aasDevInitSrc._var)
#endif

/************************* pre-declaration ***********************************************/


/*****************************************************************************************/

typedef enum {

 /*0*/   MV_HWS_AAS_UNIT_MAC_800G_0_E ,         /* 0..7 */
 /*1*/   MV_HWS_AAS_UNIT_MAC_800G_1_E ,         /* 8..15 */
 /*2*/   MV_HWS_AAS_UNIT_MAC_800G_2_E ,         /* 20..27 */
 /*3*/   MV_HWS_AAS_UNIT_MAC_800G_3_E ,         /* 28..35 */
 /*4*/   MV_HWS_AAS_UNIT_MAC_800G_4_E ,         /* 40..47 */
 /*5*/   MV_HWS_AAS_UNIT_MAC_800G_5_E ,         /* 48..55 */
 /*6*/   MV_HWS_AAS_UNIT_MAC_800G_6_E ,         /* 60..67 */
 /*7*/   MV_HWS_AAS_UNIT_MAC_800G_7_E ,         /* 68..75 */
 /*8*/   MV_HWS_AAS_UNIT_MAC_800G_8_E ,         /* 80..87 */
 /*9*/   MV_HWS_AAS_UNIT_MAC_800G_9_E ,         /* 88..95 */
 /*10*/   MV_HWS_AAS_UNIT_MAC_800G_10_E,        /* 100..107 */
 /*11*/   MV_HWS_AAS_UNIT_MAC_800G_11_E,        /* 108..115 */
 /*12*/   MV_HWS_AAS_UNIT_MAC_800G_12_E,        /* 120..127 */
 /*13*/   MV_HWS_AAS_UNIT_MAC_800G_13_E,        /* 128..135 */
 /*14*/   MV_HWS_AAS_UNIT_MAC_800G_14_E,        /* 140..147 */
 /*15*/   MV_HWS_AAS_UNIT_MAC_800G_15_E,        /* 148..155 */

 /*16*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_0_E ,        /* 16 */
 /*17*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_1_E ,        /* 17 */
 /*18*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_2_E ,        /* 18 */
 /*19*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_3_E ,        /* 19 */
 /*20*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_4_E ,        /* 36 */
 /*21*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_5_E ,        /* 37 */
 /*22*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_6_E ,        /* 38 */
 /*23*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_7_E ,        /* 39 */
 /*24*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_8_E ,        /* 56 */
 /*25*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_9_E ,        /* 57 */
 /*26*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_10_E,        /* 58 */
 /*27*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_11_E,        /* 59 */
 /*28*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_12_E,        /* 76 */
 /*29*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_13_E,        /* 77 */
 /*30*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_14_E,        /* 78 */
 /*31*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_15_E,        /* 79 */
 /*32*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_16_E,        /* 96 */
 /*33*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_17_E,        /* 97 */
 /*34*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_18_E,        /* 98 */
 /*35*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_19_E,        /* 99 */
 /*36*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_20_E,        /* 116 */
 /*37*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_21_E,        /* 117 */
 /*38*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_22_E,        /* 118 */
 /*39*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_23_E,        /* 119 */
 /*40*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_24_E,        /* 136 */
 /*41*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_25_E,        /* 137 */
 /*42*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_26_E,        /* 138 */
 /*43*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_27_E,        /* 139 */
 /*44*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_28_E,        /* 156 */
 /*45*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_29_E,        /* 157 */
 /*46*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_30_E,        /* 158 */
 /*47*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_31_E,        /* 159 */
 /*48*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_32_E,        /* */
 /*49*/   MV_HWS_AAS_UNIT_CPU_MAC_100G_33_E,        /* */

 /*50*/   MV_HWS_AAS_UNIT_CL0_ANP0_E,           /* 0..7 */
 /*51*/   MV_HWS_AAS_UNIT_CL0_ANP1_E,           /* 8..15 */
 /*52*/   MV_HWS_AAS_UNIT_CL1_ANP0_E,           /* 20..27 */
 /*53*/   MV_HWS_AAS_UNIT_CL1_ANP1_E,           /* 28..35 */
 /*54*/   MV_HWS_AAS_UNIT_CL2_ANP0_E,           /* 40..47 */
 /*55*/   MV_HWS_AAS_UNIT_CL2_ANP1_E,           /* 48..55 */
 /*56*/   MV_HWS_AAS_UNIT_CL3_ANP0_E,           /* 60..67 */
 /*57*/   MV_HWS_AAS_UNIT_CL3_ANP1_E,           /* 68..75 */
 /*58*/   MV_HWS_AAS_UNIT_CL4_ANP0_E,           /* 80..87 */
 /*59*/   MV_HWS_AAS_UNIT_CL4_ANP1_E,           /* 88..95 */
 /*60*/   MV_HWS_AAS_UNIT_CL5_ANP0_E,           /* 100..107 */
 /*61*/   MV_HWS_AAS_UNIT_CL5_ANP1_E,           /* 108..115 */
 /*62*/   MV_HWS_AAS_UNIT_CL6_ANP0_E,           /* 120..127 */
 /*63*/   MV_HWS_AAS_UNIT_CL6_ANP1_E,           /* 128..135 */
 /*64*/   MV_HWS_AAS_UNIT_CL7_ANP0_E,           /* 140..147 */
 /*65*/   MV_HWS_AAS_UNIT_CL7_ANP1_E,           /* 148..155 */

 /*66*/   MV_HWS_AAS_UNIT_CL0_ANP2_E,           /* 16 */
 /*67*/   MV_HWS_AAS_UNIT_CL0_ANP3_E,           /* 17 */
 /*68*/   MV_HWS_AAS_UNIT_CL0_ANP4_E,           /* 18 */
 /*69*/   MV_HWS_AAS_UNIT_CL0_ANP5_E,           /* 19 */
 /*70*/   MV_HWS_AAS_UNIT_CL1_ANP2_E,           /* 36 */
 /*71*/   MV_HWS_AAS_UNIT_CL1_ANP3_E,           /* 37 */
 /*72*/   MV_HWS_AAS_UNIT_CL1_ANP4_E,           /* 38 */
 /*73*/   MV_HWS_AAS_UNIT_CL1_ANP5_E,           /* 39 */
 /*74*/   MV_HWS_AAS_UNIT_CL2_ANP2_E,           /* 56 */
 /*75*/   MV_HWS_AAS_UNIT_CL2_ANP3_E,           /* 57 */
 /*76*/   MV_HWS_AAS_UNIT_CL2_ANP4_E,           /* 58 */
 /*77*/   MV_HWS_AAS_UNIT_CL2_ANP5_E,           /* 59 */
 /*78*/   MV_HWS_AAS_UNIT_CL3_ANP2_E,           /* 76 */
 /*79*/   MV_HWS_AAS_UNIT_CL3_ANP3_E,           /* 77 */
 /*80*/   MV_HWS_AAS_UNIT_CL3_ANP4_E,           /* 78 */
 /*81*/   MV_HWS_AAS_UNIT_CL3_ANP5_E,           /* 79 */
 /*82*/   MV_HWS_AAS_UNIT_CL4_ANP2_E,           /* 96 */
 /*83*/   MV_HWS_AAS_UNIT_CL4_ANP3_E,           /* 97 */
 /*84*/   MV_HWS_AAS_UNIT_CL4_ANP4_E,           /* 98 */
 /*85*/   MV_HWS_AAS_UNIT_CL4_ANP5_E,           /* 99 */
 /*86*/   MV_HWS_AAS_UNIT_CL5_ANP2_E,           /* 116 */
 /*87*/   MV_HWS_AAS_UNIT_CL5_ANP3_E,           /* 117 */
 /*88*/   MV_HWS_AAS_UNIT_CL5_ANP4_E,           /* 118 */
 /*89*/   MV_HWS_AAS_UNIT_CL5_ANP5_E,           /* 119 */
 /*90*/   MV_HWS_AAS_UNIT_CL6_ANP2_E,           /* 136 */
 /*91*/   MV_HWS_AAS_UNIT_CL6_ANP3_E,           /* 137 */
 /*92*/   MV_HWS_AAS_UNIT_CL6_ANP4_E,           /* 138 */
 /*93*/   MV_HWS_AAS_UNIT_CL6_ANP5_E,           /* 139 */
 /*94*/   MV_HWS_AAS_UNIT_CL7_ANP2_E,           /* 156 */
 /*95*/   MV_HWS_AAS_UNIT_CL7_ANP3_E,           /* 157 */
 /*96*/   MV_HWS_AAS_UNIT_CL7_ANP4_E,           /* 158 */
 /*97*/   MV_HWS_AAS_UNIT_CL7_ANP5_E,           /* 159 */

 /*98*/   MV_HWS_AAS_UNIT_ANP_CPU0_E,
 /*99*/   MV_HWS_AAS_UNIT_ANP_CPU1_E,

 /*100*/  MV_HWS_AAS_UNIT_PCS_800G_0_E,          /*  0..15 */
 /*101*/  MV_HWS_AAS_UNIT_PCS_800G_1_E,          /* 20..25 */
 /*102*/  MV_HWS_AAS_UNIT_PCS_800G_2_E,          /* 40..55 */
 /*103*/  MV_HWS_AAS_UNIT_PCS_800G_3_E,          /* 60..75 */
 /*104*/  MV_HWS_AAS_UNIT_PCS_800G_4_E,          /* 80..95 */
 /*105*/  MV_HWS_AAS_UNIT_PCS_800G_5_E,          /* 100..115 */
 /*106*/  MV_HWS_AAS_UNIT_PCS_800G_6_E,          /* 120..135 */
 /*107*/  MV_HWS_AAS_UNIT_PCS_800G_7_E,          /* 140..155 */

 /*108*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_0_E ,        /* 16 */
 /*109*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_1_E ,        /* 17 */
 /*110*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_2_E ,        /* 18 */
 /*111*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_3_E ,        /* 19 */
 /*112*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_4_E ,        /* 36 */
 /*113*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_5_E ,        /* 37 */
 /*114*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_6_E ,        /* 38 */
 /*115*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_7_E ,        /* 39 */
 /*116*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_8_E ,        /* 56 */
 /*117*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_9_E ,        /* 57 */
 /*118*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_10_E,        /* 58 */
 /*119*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_11_E,        /* 59 */
 /*120*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_12_E,        /* 76 */
 /*121*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_13_E,        /* 77 */
 /*122*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_14_E,        /* 78 */
 /*123*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_15_E,        /* 79 */
 /*124*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_16_E,        /* 96 */
 /*125*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_17_E,        /* 97 */
 /*126*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_18_E,        /* 98 */
 /*127*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_19_E,        /* 99 */
 /*128*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_20_E,        /* 116 */
 /*129*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_21_E,        /* 117 */
 /*130*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_22_E,        /* 118 */
 /*131*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_23_E,        /* 119 */
 /*132*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_24_E,        /* 136 */
 /*133*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_25_E,        /* 137 */
 /*134*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_26_E,        /* 138 */
 /*135*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_27_E,        /* 139 */
 /*136*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_28_E,        /* 156 */
 /*137*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_29_E,        /* 157 */
 /*138*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_30_E,        /* 158 */
 /*139*/   MV_HWS_AAS_UNIT_CPU_PCS_50G_31_E,        /* 159 */
 /*140*/   MV_HWS_AAS_UNIT_PCS_CPU0_E,           /* 160 */
 /*141*/   MV_HWS_AAS_UNIT_PCS_CPU1_E,           /* 161 */

 /*142*/   MV_HWS_AAS_UNIT_SDW112G_0_E,           /* 0..3 */
 /*143*/   MV_HWS_AAS_UNIT_SDW112G_1_E,           /* 4..7 */
 /*144*/   MV_HWS_AAS_UNIT_SDW112G_2_E,           /* 20..23 */
 /*145*/   MV_HWS_AAS_UNIT_SDW112G_3_E,           /* 24..27 */
 /*146*/   MV_HWS_AAS_UNIT_SDW112G_4_E,           /* 40..43 */
 /*147*/   MV_HWS_AAS_UNIT_SDW112G_5_E,           /* 44..47 */
 /*148*/   MV_HWS_AAS_UNIT_SDW112G_6_E,           /* 60..63 */
 /*149*/   MV_HWS_AAS_UNIT_SDW112G_7_E,           /* 64..67 */

 /*150*/   MV_HWS_AAS_UNIT_SDW56G_0_E,            /* 8..11  */
 /*151*/   MV_HWS_AAS_UNIT_SDW56G_1_E,            /* 12..15 */
 /*152*/   MV_HWS_AAS_UNIT_SDW56G_2_E,            /* 28..31 */
 /*153*/   MV_HWS_AAS_UNIT_SDW56G_3_E,            /* 32..35 */
 /*154*/   MV_HWS_AAS_UNIT_SDW56G_4_E,            /* 48..51 */
 /*155*/   MV_HWS_AAS_UNIT_SDW56G_5_E,            /* 52..55 */
 /*156*/   MV_HWS_AAS_UNIT_SDW56G_6_E,            /* 68..71 */
 /*157*/   MV_HWS_AAS_UNIT_SDW56G_7_E,            /* 72..75 */

 /*158*/   MV_HWS_AAS_UNIT_CPU_SDW56G_0_E,        /* 16..19 */
 /*159*/   MV_HWS_AAS_UNIT_CPU_SDW56G_1_E,        /* 36..39 */
 /*160*/   MV_HWS_AAS_UNIT_CPU_SDW56G_2_E,        /* 56..59 */
 /*162*/   MV_HWS_AAS_UNIT_CPU_SDW56G_3_E,        /* 76..79 */
 /*163*/   MV_HWS_AAS_UNIT_CPU_SDW56G_4_E,        /* CPU */


    MV_HWS_AAS_UNIT_LAST
} MV_HWS_AAS_BASE_ADDR_OFFSET_ENT;


GT_U32 unitBaseAddrArr_singleTile[MV_HWS_AAS_UNIT_LAST] =
{
 /*0 - MV_HWS_AAS_UNIT_MAC_800G_0_E*/       0x00E00000,     /* 0..7 */
 /*1 - MV_HWS_AAS_UNIT_MAC_800G_1_E*/       0x00E40000,     /* 8..15 */
 /*2 - MV_HWS_AAS_UNIT_MAC_800G_2_E*/       0x01E00000,     /* 20..27 */
 /*3 - MV_HWS_AAS_UNIT_MAC_800G_3_E*/       0x01E40000,     /* 28..35 */
 /*4 - MV_HWS_AAS_UNIT_MAC_800G_4_E*/       0x02E00000,     /* 40..47 */
 /*5 - MV_HWS_AAS_UNIT_MAC_800G_5_E*/       0x02E40000,     /* 48..55 */
 /*6 - MV_HWS_AAS_UNIT_MAC_800G_6_E*/       0x03E00000,     /* 60..67 */
 /*7 - MV_HWS_AAS_UNIT_MAC_800G_7_E*/       0x03E40000,     /* 68..75 */
 /*8 - MV_HWS_AAS_UNIT_MAC_800G_8_E*/       0xFFFFFFFB,     /* 80..87 */
 /*9 - MV_HWS_AAS_UNIT_MAC_800G_9_E*/       0xFFFFFFFB,     /* 88..95 */
 /*10- MV_HWS_AAS_UNIT_MAC_800G_10_E*/      0xFFFFFFFB,     /* 100..107 */
 /*11- MV_HWS_AAS_UNIT_MAC_800G_11_E*/      0xFFFFFFFB,     /* 108..115 */
 /*12- MV_HWS_AAS_UNIT_MAC_800G_12_E*/      0xFFFFFFFB,     /* 120..127 */
 /*13- MV_HWS_AAS_UNIT_MAC_800G_13_E*/      0xFFFFFFFB,     /* 128..135 */
 /*14- MV_HWS_AAS_UNIT_MAC_800G_14_E*/      0xFFFFFFFB,     /* 140..147 */
 /*15- MV_HWS_AAS_UNIT_MAC_800G_15_E*/      0xFFFFFFFB,     /* 148..155 */
 /*16- MV_HWS_AAS_UNIT_CPU_MAC_100G_0_E*/   0x00380000,     /* 16 */
 /*17- MV_HWS_AAS_UNIT_CPU_MAC_100G_1_E*/   0x00390000,     /* 17 */
 /*18- MV_HWS_AAS_UNIT_CPU_MAC_100G_2_E*/   0x003A0000,     /* 18 */
 /*19- MV_HWS_AAS_UNIT_CPU_MAC_100G_3_E*/   0x003B0000,     /* 19 */
 /*20- MV_HWS_AAS_UNIT_CPU_MAC_100G_4_E*/   0x01380000,     /* 36 */
 /*21- MV_HWS_AAS_UNIT_CPU_MAC_100G_5_E*/   0x01390000,     /* 37 */
 /*22- MV_HWS_AAS_UNIT_CPU_MAC_100G_6_E*/   0x013A0000,     /* 38 */
 /*23- MV_HWS_AAS_UNIT_CPU_MAC_100G_7_E*/   0x013B0000,     /* 39 */
 /*24- MV_HWS_AAS_UNIT_CPU_MAC_100G_8_E*/   0x02380000,     /* 56 */
 /*25- MV_HWS_AAS_UNIT_CPU_MAC_100G_9_E*/   0x02390000,     /* 57 */
 /*26- MV_HWS_AAS_UNIT_CPU_MAC_100G_10_E*/  0x023A0000,     /* 58 */
 /*27- MV_HWS_AAS_UNIT_CPU_MAC_100G_11_E*/  0x023B0000,     /* 59 */
 /*28- MV_HWS_AAS_UNIT_CPU_MAC_100G_12_E*/  0x03380000,     /* 76 */
 /*29- MV_HWS_AAS_UNIT_CPU_MAC_100G_13_E*/  0x03390000,     /* 77 */
 /*30- MV_HWS_AAS_UNIT_CPU_MAC_100G_14_E*/  0x033A0000,     /* 78 */
 /*31- MV_HWS_AAS_UNIT_CPU_MAC_100G_15_E*/  0x033B0000,     /* 79 */
 /*32- MV_HWS_AAS_UNIT_CPU_MAC_100G_16_E*/  0xFFFFFFFB,     /* 96 */
 /*33- MV_HWS_AAS_UNIT_CPU_MAC_100G_17_E*/  0xFFFFFFFB,     /* 97 */
 /*34- MV_HWS_AAS_UNIT_CPU_MAC_100G_18_E*/  0xFFFFFFFB,     /* 98 */
 /*35- MV_HWS_AAS_UNIT_CPU_MAC_100G_19_E*/  0xFFFFFFFB,     /* 99 */
 /*36- MV_HWS_AAS_UNIT_CPU_MAC_100G_20_E*/  0xFFFFFFFB,     /* 116 */
 /*37- MV_HWS_AAS_UNIT_CPU_MAC_100G_21_E*/  0xFFFFFFFB,     /* 117 */
 /*38- MV_HWS_AAS_UNIT_CPU_MAC_100G_22_E*/  0xFFFFFFFB,     /* 118 */
 /*39- MV_HWS_AAS_UNIT_CPU_MAC_100G_23_E*/  0xFFFFFFFB,     /* 119 */
 /*40- MV_HWS_AAS_UNIT_CPU_MAC_100G_24_E*/  0xFFFFFFFB,     /* 136 */
 /*41- MV_HWS_AAS_UNIT_CPU_MAC_100G_25_E*/  0xFFFFFFFB,     /* 137 */
 /*42- MV_HWS_AAS_UNIT_CPU_MAC_100G_26_E*/  0xFFFFFFFB,     /* 138 */
 /*43- MV_HWS_AAS_UNIT_CPU_MAC_100G_27_E*/  0xFFFFFFFB,     /* 139 */
 /*44- MV_HWS_AAS_UNIT_CPU_MAC_100G_28_E*/  0xFFFFFFFB,     /* 156 */
 /*45- MV_HWS_AAS_UNIT_CPU_MAC_100G_29_E*/  0xFFFFFFFB,     /* 157 */
 /*46- MV_HWS_AAS_UNIT_CPU_MAC_100G_30_E*/  0xFFFFFFFB,     /* 158 */
 /*47- MV_HWS_AAS_UNIT_CPU_MAC_100G_31_E*/  0xFFFFFFFB,     /* 159 */
 /*48- MV_HWS_AAS_UNIT_CPU_MAC_100G_32_E*/  0x01C20000,     /* CPU0 */
 /*49- MV_HWS_AAS_UNIT_CPU_MAC_100G_33_E*/  0xFFFFFFFB,     /* CPU1 */

 /*50 - MV_HWS_AAS_UNIT_CL0_ANP0_E*/        0x00A80000,     /* 0..7 */
 /*51 - MV_HWS_AAS_UNIT_CL0_ANP1_E*/        0x00AC0000,     /* 8..15 */
 /*52 - MV_HWS_AAS_UNIT_CL1_ANP0_E*/        0x01A80000,     /* 20..27 */
 /*53 - MV_HWS_AAS_UNIT_CL1_ANP1_E*/        0x01AC0000,     /* 28..35 */
 /*54 - MV_HWS_AAS_UNIT_CL2_ANP0_E*/        0x02A80000,     /* 40..47 */
 /*55 - MV_HWS_AAS_UNIT_CL2_ANP1_E*/        0x02AC0000,     /* 48..55 */
 /*56 - MV_HWS_AAS_UNIT_CL3_ANP0_E*/        0x03A80000,     /* 60..67 */
 /*57 - MV_HWS_AAS_UNIT_CL3_ANP1_E*/        0x03AC0000,     /* 68..75 */
 /*58 - MV_HWS_AAS_UNIT_CL4_ANP0_E*/        0xFFFFFFFB,     /* 80..87 */
 /*59 - MV_HWS_AAS_UNIT_CL4_ANP1_E*/        0xFFFFFFFB,     /* 88..95 */
 /*60 - MV_HWS_AAS_UNIT_CL5_ANP0_E*/        0xFFFFFFFB,     /* 100..107 */
 /*61 - MV_HWS_AAS_UNIT_CL5_ANP1_E*/        0xFFFFFFFB,     /* 108..115 */
 /*62 - MV_HWS_AAS_UNIT_CL6_ANP0_E*/        0xFFFFFFFB,     /* 120..127 */
 /*63 - MV_HWS_AAS_UNIT_CL6_ANP1_E*/        0xFFFFFFFB,     /* 128..135 */
 /*64 - MV_HWS_AAS_UNIT_CL7_ANP0_E*/        0xFFFFFFFB,     /* 140..147 */
 /*65 - MV_HWS_AAS_UNIT_CL7_ANP1_E*/        0xFFFFFFFB,     /* 148..155 */
 /*66 - MV_HWS_AAS_UNIT_CL0_ANP2_E*/        0x00200000,     /* 16 */
 /*67 - MV_HWS_AAS_UNIT_CL0_ANP3_E*/        0x00240000,     /* 17 */
 /*68 - MV_HWS_AAS_UNIT_CL0_ANP4_E*/        0x00280000,     /* 18 */
 /*69 - MV_HWS_AAS_UNIT_CL0_ANP5_E*/        0x002C0000,     /* 19 */
 /*70 - MV_HWS_AAS_UNIT_CL1_ANP2_E*/        0x01200000,     /* 36 */
 /*71 - MV_HWS_AAS_UNIT_CL1_ANP3_E*/        0x01240000,     /* 37 */
 /*72 - MV_HWS_AAS_UNIT_CL1_ANP4_E*/        0x01280000,     /* 38 */
 /*73 - MV_HWS_AAS_UNIT_CL1_ANP5_E*/        0x012C0000,     /* 39 */
 /*74 - MV_HWS_AAS_UNIT_CL2_ANP2_E*/        0x02200000,     /* 56 */
 /*75 - MV_HWS_AAS_UNIT_CL2_ANP3_E*/        0x02240000,     /* 57 */
 /*76 - MV_HWS_AAS_UNIT_CL2_ANP4_E*/        0x02280000,     /* 58 */
 /*77 - MV_HWS_AAS_UNIT_CL2_ANP5_E*/        0x022C0000,     /* 59 */
 /*78 - MV_HWS_AAS_UNIT_CL3_ANP2_E*/        0x03200000,     /* 76 */
 /*79 - MV_HWS_AAS_UNIT_CL3_ANP3_E*/        0x03240000,     /* 77 */
 /*80 - MV_HWS_AAS_UNIT_CL3_ANP4_E*/        0x03280000,     /* 78 */
 /*81 - MV_HWS_AAS_UNIT_CL3_ANP5_E*/        0x032C0000,     /* 79 */
 /*82 - MV_HWS_AAS_UNIT_CL0_ANP2_E*/        0xFFFFFFFB,     /* 96 */
 /*83 - MV_HWS_AAS_UNIT_CL0_ANP3_E*/        0xFFFFFFFB,     /* 97 */
 /*84 - MV_HWS_AAS_UNIT_CL0_ANP4_E*/        0xFFFFFFFB,     /* 98 */
 /*85 - MV_HWS_AAS_UNIT_CL0_ANP5_E*/        0xFFFFFFFB,     /* 99 */
 /*86 - MV_HWS_AAS_UNIT_CL1_ANP2_E*/        0xFFFFFFFB,     /* 116 */
 /*87 - MV_HWS_AAS_UNIT_CL1_ANP3_E*/        0xFFFFFFFB,     /* 117 */
 /*88 - MV_HWS_AAS_UNIT_CL1_ANP4_E*/        0xFFFFFFFB,     /* 118 */
 /*89 - MV_HWS_AAS_UNIT_CL1_ANP5_E*/        0xFFFFFFFB,     /* 119 */
 /*90 - MV_HWS_AAS_UNIT_CL2_ANP2_E*/        0xFFFFFFFB,     /* 136 */
 /*91 - MV_HWS_AAS_UNIT_CL2_ANP3_E*/        0xFFFFFFFB,     /* 137 */
 /*92 - MV_HWS_AAS_UNIT_CL2_ANP4_E*/        0xFFFFFFFB,     /* 138 */
 /*93 - MV_HWS_AAS_UNIT_CL2_ANP5_E*/        0xFFFFFFFB,     /* 139 */
 /*94 - MV_HWS_AAS_UNIT_CL3_ANP2_E*/        0xFFFFFFFB,     /* 156 */
 /*95 - MV_HWS_AAS_UNIT_CL3_ANP3_E*/        0xFFFFFFFB,     /* 157 */
 /*96 - MV_HWS_AAS_UNIT_CL3_ANP4_E*/        0xFFFFFFFB,     /* 158 */
 /*97 - MV_HWS_AAS_UNIT_CL3_ANP5_E*/        0xFFFFFFFB,     /* 159 */
 /*98 - MV_HWS_AAS_UNIT_ANP_CPU0_E*/        0x01B80000,
 /*99 - MV_HWS_AAS_UNIT_ANP_CPU1_E*/        0xFFFFFFFB,

 /*100- MV_HWS_AAS_UNIT_PCS_800G_0_E*/      0x00660000,     /*  0..15 */
 /*101- MV_HWS_AAS_UNIT_PCS_800G_1_E*/      0x01660000,     /* 20..25 */
 /*102- MV_HWS_AAS_UNIT_PCS_800G_2_E*/      0x02660000,     /* 40..55 */
 /*103- MV_HWS_AAS_UNIT_PCS_800G_3_E*/      0x03660000,     /* 60..75 */
 /*104- MV_HWS_AAS_UNIT_PCS_800G_4_E*/      0xFFFFFFFB,     /* 80..95 */
 /*105- MV_HWS_AAS_UNIT_PCS_800G_5_E*/      0xFFFFFFFB,     /* 100..115 */
 /*106- MV_HWS_AAS_UNIT_PCS_800G_6_E*/      0xFFFFFFFB,     /* 120..135 */
 /*107- MV_HWS_AAS_UNIT_PCS_800G_7_E*/      0xFFFFFFFB,     /* 140..155 */

 /*108- MV_HWS_AAS_UNIT_CPU_PCS_50G_0_E*/   0x00300000,     /* 16 */
 /*109- MV_HWS_AAS_UNIT_CPU_PCS_50G_1_E*/   0x00320000,     /* 17 */
 /*110- MV_HWS_AAS_UNIT_CPU_PCS_50G_2_E*/   0x00340000,     /* 18 */
 /*111- MV_HWS_AAS_UNIT_CPU_PCS_50G_3_E*/   0x00360000,     /* 19 */
 /*112- MV_HWS_AAS_UNIT_CPU_PCS_50G_4_E*/   0x01300000,     /* 36 */
 /*113- MV_HWS_AAS_UNIT_CPU_PCS_50G_5_E*/   0x01320000,     /* 37 */
 /*114- MV_HWS_AAS_UNIT_CPU_PCS_50G_6_E*/   0x01340000,     /* 38 */
 /*115- MV_HWS_AAS_UNIT_CPU_PCS_50G_7_E*/   0x01360000,     /* 39 */
 /*116- MV_HWS_AAS_UNIT_CPU_PCS_50G_8_E*/   0x02300000,     /* 56 */
 /*117- MV_HWS_AAS_UNIT_CPU_PCS_50G_9_E*/   0x02320000,     /* 57 */
 /*118- MV_HWS_AAS_UNIT_CPU_PCS_50G_10_E*/  0x02340000,     /* 58 */
 /*119- MV_HWS_AAS_UNIT_CPU_PCS_50G_11_E*/  0x02360000,     /* 59 */
 /*120- MV_HWS_AAS_UNIT_CPU_PCS_50G_12_E*/  0x03300000,     /* 76 */
 /*121- MV_HWS_AAS_UNIT_CPU_PCS_50G_13_E*/  0x03320000,     /* 77 */
 /*122- MV_HWS_AAS_UNIT_CPU_PCS_50G_14_E*/  0x03340000,     /* 78 */
 /*123- MV_HWS_AAS_UNIT_CPU_PCS_50G_15_E*/  0x03360000,     /* 79 */
 /*124- MV_HWS_AAS_UNIT_CPU_PCS_50G_16_E*/  0xFFFFFFFB,     /* 96 */
 /*125- MV_HWS_AAS_UNIT_CPU_PCS_50G_17_E*/  0xFFFFFFFB,     /* 97 */
 /*126- MV_HWS_AAS_UNIT_CPU_PCS_50G_18_E*/  0xFFFFFFFB,     /* 98 */
 /*127- MV_HWS_AAS_UNIT_CPU_PCS_50G_19_E*/  0xFFFFFFFB,     /* 99 */
 /*128- MV_HWS_AAS_UNIT_CPU_PCS_50G_20_E*/  0xFFFFFFFB,     /* 116 */
 /*129- MV_HWS_AAS_UNIT_CPU_PCS_50G_21_E*/  0xFFFFFFFB,     /* 117 */
 /*130- MV_HWS_AAS_UNIT_CPU_PCS_50G_22_E*/  0xFFFFFFFB,     /* 118 */
 /*131- MV_HWS_AAS_UNIT_CPU_PCS_50G_23_E*/  0xFFFFFFFB,     /* 119 */
 /*132- MV_HWS_AAS_UNIT_CPU_PCS_50G_24_E*/  0xFFFFFFFB,     /* 136 */
 /*133- MV_HWS_AAS_UNIT_CPU_PCS_50G_25_E*/  0xFFFFFFFB,     /* 137 */
 /*134- MV_HWS_AAS_UNIT_CPU_PCS_50G_26_E*/  0xFFFFFFFB,     /* 138 */
 /*135- MV_HWS_AAS_UNIT_CPU_PCS_50G_27_E*/  0xFFFFFFFB,     /* 139 */
 /*136- MV_HWS_AAS_UNIT_CPU_PCS_50G_28_E*/  0xFFFFFFFB,     /* 156 */
 /*137- MV_HWS_AAS_UNIT_CPU_PCS_50G_29_E*/  0xFFFFFFFB,     /* 157 */
 /*138- MV_HWS_AAS_UNIT_CPU_PCS_50G_30_E*/  0xFFFFFFFB,     /* 158 */
 /*139- MV_HWS_AAS_UNIT_CPU_PCS_50G_31_E*/  0xFFFFFFFB,     /* 159 */
 /*140- MV_HWS_AAS_UNIT_CPU_PCS_50G_32_E*/  0x01BF0000,     /* CPU */
 /*141- MV_HWS_AAS_UNIT_CPU_PCS_50G_33_E*/  0xFFFFFFFB,     /* CPU */

 /*142- MV_HWS_AAS_UNIT_SDW112G_0_E*/       0x00980000,     /* SD: 0..3 */
 /*143- MV_HWS_AAS_UNIT_SDW112G_1_E*/       0x00A00000,     /* SD: 4..7 */
 /*144- MV_HWS_AAS_UNIT_SDW112G_2_E*/       0x01980000,     /* SD: 20..23 */
 /*145- MV_HWS_AAS_UNIT_SDW112G_3_E*/       0x01A00000,     /* SD: 24..27 */
 /*146- MV_HWS_AAS_UNIT_SDW112G_4_E*/       0x02980000,     /* SD: 40..43 */
 /*147- MV_HWS_AAS_UNIT_SDW112G_5_E*/       0x02A00000,     /* SD: 44..47 */
 /*148- MV_HWS_AAS_UNIT_SDW112G_6_E*/       0x03980000,     /* SD: 60..63 */
 /*149- MV_HWS_AAS_UNIT_SDW112G_7_E*/       0x03A00000,     /* SD: 64..67 */
 /*150- MV_HWS_AAS_UNIT_SDW56G_0_E*/        0x00800000,     /* SD: 8..11  */
 /*151- MV_HWS_AAS_UNIT_SDW56G_1_E*/        0x00880000,     /* SD: 12..15 */
 /*152- MV_HWS_AAS_UNIT_SDW56G_2_E*/        0x01800000,     /* SD: 28..31 */
 /*153- MV_HWS_AAS_UNIT_SDW56G_3_E*/        0x01880000,     /* SD: 32..35 */
 /*154- MV_HWS_AAS_UNIT_SDW56G_4_E*/        0x02800000,     /* SD: 48..51 */
 /*155- MV_HWS_AAS_UNIT_SDW56G_5_E*/        0x02880000,     /* SD: 52..55 */
 /*156- MV_HWS_AAS_UNIT_SDW56G_6_E*/        0x03800000,     /* SD: 68..71 */
 /*157- MV_HWS_AAS_UNIT_SDW56G_7_E*/        0x03880000,     /* SD: 72..75 */

 /*158- MV_HWS_AAS_UNIT_CPU_SDW56G_0_E*/    0x00900000,     /* SD: 16..19 */
 /*159- MV_HWS_AAS_UNIT_CPU_SDW56G_1_E*/    0x01900000,     /* SD: 36..39 */
 /*160- MV_HWS_AAS_UNIT_CPU_SDW56G_2_E*/    0x02900000,     /* SD: 56..59 */
 /*161- MV_HWS_AAS_UNIT_CPU_SDW56G_3_E*/    0x03900000,     /* SD: 76..79 */
 /*162- MV_HWS_AAS_UNIT_CPU_SDW56G_4_E*/    0x01B00000,     /* SD: CPU */

};

GT_U32 unitBaseAddrArr_dualTile[MV_HWS_AAS_UNIT_LAST] =
{
 /*0 - MV_HWS_AAS_UNIT_MAC_800G_0_E*/    0x23E80000,     /* 0..7 */
};

GT_STATUS mvHwsAasUnitBaseAddrCalc
(
    IN GT_U8                    devNum,
    IN MV_HWS_UNITS_ID          unitId,
    IN MV_HWS_PORT_STANDARD     portMode,
    IN GT_U32                   phyPortNum,
    OUT GT_U32                 *baseAddrPtr
)
{
    MV_HWS_HAWK_CONVERT_STC     convertIdx;
    GT_U32                      baseAddr;
    GT_U32                      *currUnitBaseArrPtr;
    GT_U32                      localPortNum;

    if (baseAddrPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if (unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

    if(hwsDeviceSpecInfo[devNum].numOfTiles == 1)
    {
        currUnitBaseArrPtr = unitBaseAddrArr_singleTile;
    }
    else
    {
        currUnitBaseArrPtr = unitBaseAddrArr_dualTile;
    }

    *baseAddrPtr = 0xFFFFFFFB;

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, &convertIdx));

    localPortNum = phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS;

    switch(unitId)
    {
        case MTI_GLOBAL_EXT_UNIT:
        case MTI_EXT_UNIT:
        case MTI_MAC100_UNIT:
        case MTI_MAC400_UNIT:
            if (localPortNum < 16)
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_MAC_800G_0_E + convertIdx.ciderUnit];
            }
            else
            {
                return GT_BAD_PARAM;
#if 0

                if(unitId == MTI_MAC400_UNIT)
                {
                    return GT_BAD_PARAM;
                }
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_MAC_100G_0_E + convertIdx.ciderUnit];
#endif
            }
            break;

        case MTI_CPU_GLOBAL_EXT_UNIT:
        case MTI_CPU_EXT_UNIT:
        case MTI_CPU_MAC_UNIT:
            if (mvHwsMtipIsReducedPort(devNum, phyPortNum))
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_CPU_MAC_100G_0_E + convertIdx.ciderUnit];
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;

        case MTI_PCS_EXT_UNIT:
        case MTI_LOW_SP_PCS_UNIT:
        case MTI_RSFEC_UNIT:
            if (localPortNum < 16)
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_PCS_800G_0_E + (convertIdx.ciderUnit/2)];
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;

        case MTI_PCS50_UNIT:
            if (localPortNum < 16)
            {
                if(localPortNum % 2 == 0)
                {
                    return GT_BAD_PARAM;
                }
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_PCS_800G_0_E + (convertIdx.ciderUnit/2)];
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;
        case MTI_PCS100_UNIT:
            if (localPortNum < 16)
            {
                if(localPortNum % 2 == 1)
                {
                    return GT_BAD_PARAM;
                }
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_PCS_800G_0_E + (convertIdx.ciderUnit/2)];
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;
        case MTI_PCS200_UNIT:
            if (localPortNum < 16)
            {
                if(localPortNum == 4 || localPortNum == 12)
                {
                    baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_PCS_800G_0_E + (convertIdx.ciderUnit/2)];
                }
                else
                {
                    return GT_BAD_PARAM;
                }
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;
        case MTI_PCS400_UNIT:
            if (localPortNum < 16)
            {
                if(localPortNum == 0 || localPortNum == 8)
                {
                    baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_PCS_800G_0_E + (convertIdx.ciderUnit/2)];
                }
                else
                {
                    return GT_BAD_PARAM;
                }
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;

        case ANP_400_UNIT:
        case AN_400_UNIT:
            if ((phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_CL0_ANP0_E + convertIdx.ciderUnit];
            }
            else
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_CL0_ANP2_E + convertIdx.ciderUnit];
            }
            if(unitId == AN_400_UNIT)
            {
                baseAddr += 0x20000;
            }
            break;

        case MTI_CPU_PCS_EXT_UNIT:
        case MTI_CPU_RSFEC_UNIT:
        case MTI_CPU_PCS_UNIT:
        case MTI_CPU_SGPCS_UNIT:
            if (mvHwsMtipIsReducedPort(devNum, phyPortNum))
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_CPU_PCS_50G_0_E + convertIdx.ciderUnit];
            }
            else
            {
                return GT_BAD_PARAM;
            }
            break;

        case SERDES_UNIT:
            if (phyPortNum == 80)
            {
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_CPU_SDW56G_4_E];

            }
            if(phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS > 15)
            {
                GT_U32 sdUnitIndex;

                /*phyPortNum variable represents SD number*/
                sdUnitIndex = (phyPortNum / MV_HWS_AAS_GOP_PORT_NUM_CNS);
                baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_CPU_SDW56G_0_E + sdUnitIndex];
            }
            else
            {
                GT_U32 sdUnitIndex;

                /*phyPortNum variable represents SD number*/

                if(phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS < 8)
                {
                    sdUnitIndex = (phyPortNum / MV_HWS_AAS_GOP_PORT_NUM_CNS) * 2 + (phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 4;
                    baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_SDW112G_0_E + sdUnitIndex];
                }
                else
                {
                    sdUnitIndex = (phyPortNum / MV_HWS_AAS_GOP_PORT_NUM_CNS) * 2 + (phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 12;
                    baseAddr = currUnitBaseArrPtr[MV_HWS_AAS_UNIT_SDW56G_0_E + sdUnitIndex];
                }
            }
            break;


        default:
            return GT_BAD_PARAM;
    }
    *baseAddrPtr = baseAddr;
    return GT_OK;
}

/**
* @internal mvHwsAasUnitRegDbEntryGet function
* @endinternal
*
* @brief   get DB entry.
*
* @param[in] devNum                  - system device number
* @param[in] portNum                 - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
* @param[out] convertIdxPtr          - (pointer to) structure with unit data
*
* @retval address                        - on success
* @retval MV_HWS_SW_PTR_ENTRY_UNUSED     - on error
*/
GT_STATUS mvHwsAasUnitRegDbEntryGet
(
    IN GT_U8           devNum,
    IN GT_UOPT         portNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr,
    OUT MV_HWS_HAWK_CONVERT_STC *convertIdxPtr
)
{
    GT_U32 size = 0;
    MV_HWS_REG_ADDR_FIELD_STC *regDbPtr = NULL;
    GT_STATUS rc;

    if(fieldRegOutPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, convertIdxPtr);
    if(rc != GT_OK)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    rc = mvHwsRegDbEntryGet(devNum, unitId, &regDbPtr);
    if(regDbPtr == NULL)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    switch (unitId)
    {
        case MTI_MAC400_UNIT:
            if(HWS_IS_PORT_MODE_400G_R8(portMode) ||
               HWS_IS_PORT_MODE_400G_R4(portMode) ||
               HWS_IS_PORT_MODE_800G_R8(portMode))
            {
                if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8 != 0)
                {
                    return MV_HWS_SW_PTR_ENTRY_UNUSED;
                }
            }
            convertIdxPtr->ciderIndexInUnit = 0;
            size = AAS_MAC400_REGISTER_LAST_E;
            break;

        case MTI_MAC100_UNIT:
        case MTI_CPU_MAC_UNIT:
            size = AAS_MAC100_REGISTER_LAST_E;
            break;

        case MTI_EXT_UNIT:
            if(HWS_IS_PORT_MODE_400G_R8(portMode)||
               HWS_IS_PORT_MODE_400G_R4(portMode)||
               HWS_IS_PORT_MODE_800G_R8(portMode))
            {
                if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8 == 0)
                {
                    convertIdxPtr->ciderIndexInUnit = 8;
                }
                else
                {
                    return MV_HWS_SW_PTR_ENTRY_UNUSED;
                }
            }
            size = AAS_MAC_PORT_EXT_REGISTER_LAST_E;
            break;

        case MTI_CPU_EXT_UNIT:
            size = AAS_MAC_PORT_EXT_REGISTER_LAST_E;
            break;

        case MTI_GLOBAL_EXT_UNIT:
        case MTI_CPU_GLOBAL_EXT_UNIT:
            size = AAS_MAC_GLOBAL_EXT_GLOBAL_REGISTER_LAST_E;
            break;

        case MTI_PCS_EXT_UNIT:
            size = AAS_PCS800_EXT_REGISTER_LAST_E;
            if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
            {
                convertIdxPtr->ciderIndexInUnit += 8*(convertIdxPtr->ciderUnit % 2);    /* EXT_PCS800 serves 16 ports */
            }
            break;

        case MTI_PCS50_UNIT:
            size = AAS_PCS50_REGISTER_LAST_E;
            if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
            {
                convertIdxPtr->ciderIndexInUnit = (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 2;
            }
            else
            {
                /*TBD*/
            }
            break;
        case MTI_PCS100_UNIT:
            size = AAS_PCS100_REGISTER_LAST_E;
            if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
            {
                convertIdxPtr->ciderIndexInUnit = (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 2;
            }
            else
            {
                /*TBD*/
            }
            break;
        case MTI_PCS200_UNIT:
            size = AAS_PCS200_REGISTER_LAST_E;
            if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
            {
                convertIdxPtr->ciderIndexInUnit = (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 8; /* local port 4 and 12 mapped to indexes 0,1 */
            }
            else
            {
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
            }
            break;
        case MTI_PCS400_UNIT:
            size = AAS_PCS400_REGISTER_LAST_E;
            if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
            {
                convertIdxPtr->ciderIndexInUnit = (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 8; /* local port 0 and 8 mapped to indexes 0,1 */
            }
            else
            {
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
            }
            break;
        case MTI_LOW_SP_PCS_UNIT:
        case MTI_CPU_SGPCS_UNIT:
            size = AAS_LPCS_REGISTER_LAST_E;
            break;
        case MTI_RSFEC_UNIT:
            size = AAS_RSFEC_UNIT_REGISTER_LAST_E;
            break;

        case ANP_400_UNIT:
            size = AAS_ANP_UNIT_REGISTER_LAST_E;
            break;
        case AN_400_UNIT:
            size = AAS_AN_UNIT_REGISTER_LAST_E;
            break;
        case MTI_CPU_PCS_EXT_UNIT:
            size = AAS_CPU_PCS_EXT_REGISTER_LAST_E;
            break;
        case MTI_CPU_PCS_UNIT:
            size = AAS_CPU_PCS50_REGISTER_LAST_E;
            break;
        case MTI_CPU_RSFEC_UNIT:
            size = AAS_CPU_RSFEC_UNIT_REGISTER_LAST_E;
            break;
        default:
            return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    if(fieldName >= size)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    hwsMemCpy(fieldRegOutPtr, &regDbPtr[fieldName], sizeof(MV_HWS_REG_ADDR_FIELD_STC));

    return GT_OK;
}

/**
* @internal mvHwsAasUnitFieldAddrFind function
* @endinternal
*
* @brief   get register address.
*
* @param[in] devNum                  - system device number
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval address                        - on success
* @retval MV_HWS_SW_PTR_ENTRY_UNUSED     - on error
*/
GT_U32 mvHwsAasUnitFieldAddrFind
(
    IN GT_U8           devNum,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
)
{
    GT_STATUS rc;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 unitAddr = 0;
    GT_U32 address = MV_HWS_SW_PTR_ENTRY_UNUSED;

    if (portMode == NON_SUP_MODE)
    {
        switch (unitId) {
            case MTI_MAC100_UNIT:
            case MTI_EXT_UNIT:
            case MTI_RSFEC_UNIT:
            case MTI_PCS25_UNIT:
            case MTI_PCS50_UNIT:
            case MTI_PCS100_UNIT:
            case MTI_LOW_SP_PCS_UNIT:
            case ANP_400_UNIT:
            case AN_400_UNIT:
            case MTI_CPU_RSFEC_UNIT:
            case MTI_CPU_PCS_UNIT:
            case MTI_CPU_EXT_UNIT:
            case MTI_CPU_MAC_UNIT:
            case MTI_CPU_SGPCS_UNIT:
            case MTI_RSFEC_STATISTICS_UNIT:
            case MTI_CPU_RSFEC_STATISTICS_UNIT:
            case SERDES_UNIT:
            case MTI_PCS_EXT_UNIT:
            case MTI_GLOBAL_EXT_UNIT:
                portMode = _10GBase_KR;
                break;

            case MTI_PCS200_UNIT:
                portMode = _200GBase_KR8;
                break;

            case MIF_400_SEG_UNIT:
            case MTI_MAC400_UNIT:
            case MTI_PCS400_UNIT:
                portMode = _400GBase_KR8;
                break;

            default:
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
        }
    }

    rc = mvHwsAasUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, fieldRegOutPtr, &convertIdx);
    if(rc != GT_OK)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    rc = mvHwsAasUnitBaseAddrCalc(devNum, unitId, portMode, phyPortNum, &unitAddr);
    if((unitAddr == 0) || (rc != GT_OK))
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    /*
        In order to access to the following registers in MTI EXT unit
        it is needed to update ciderIndexInUnit:
            Seg Port<%n> Control
            Seg Port<%n> Status
            Seg Port<%n> Pause and Err Stat
    */
#if 0
    if(unitId == MTI_EXT_UNIT)
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode) && (fieldRegOutPtr->offsetFormula != 0))
        {
             convertIdx.ciderIndexInUnit /= 4;
        }
    }
#endif


#if 0
    if((hwsDeviceSpecInfo[devNum].devType == Ironman))
    {
        if((unitId == ANP_USX_UNIT) || (unitId == ANP_USX_O_UNIT))
        {
            convertIdx.ciderIndexInUnit = 0;
        }
        if(unitId == MTI_USX_RSFEC_STATISTICS_UNIT)
        {
            fieldRegOutPtr->offsetFormula = 0x3000;
        }
    }
#endif

    if(((phyPortNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 8) == 1)
    {
        if((unitId == MTI_RSFEC_UNIT) ||
           (unitId == MTI_LOW_SP_PCS_UNIT))
        {
            /*
                Ports 0..7 use RSFEC unit 0 with base address 0x11000
                Ports 8..15 use RSFEC unit 1 with base address 0x12000
            */
            /*
                Ports 0..7 use LPCS unit 0 with base address 0x13000
                Ports 8..15 use LPCS unit 1 with base address 0x14000
            */
            fieldRegOutPtr->regOffset += 0x1000;
        }
    }

    if ((MTI_RSFEC_STATISTICS_UNIT     == unitId) || (MTI_CPU_RSFEC_STATISTICS_UNIT == unitId))
    {
        switch(fieldRegOutPtr->regOffset)
        {
            case 0x70:
            case 0x74:
            case 0x78:
            case 0x7C:
            case 0x80:
            case 0x84:
            case 0x88:
            case 0x8C:
            case 0x90:
            case 0x94:
            case 0x98:
            case 0x9C:
            case 0xA0:
            case 0xA4:
            case 0xA8:
            case 0xAC:
            case 0xB0:
            case 0xB4:
            case 0xB8:
            case 0xBC:
                fieldRegOutPtr->offsetFormula = 0x50;/*kalex: need to check once again*/
            break;
            default:
            break;
        }
    }

    address = (unitAddr) +  fieldRegOutPtr->regOffset + fieldRegOutPtr->offsetFormula * convertIdx.ciderIndexInUnit;
    return address;
}

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

    CHECK_STATUS(mvHwsMtiPcs50_SIP7IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs100_SIP7IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs400_SIP7IfInit(hwsPcsFuncsPtr));
#if 0
    CHECK_STATUS(mvHwsMtiPcs50Rev2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiPcs100Rev2IfInit(hwsPcsFuncsPtr));
    /*CHECK_STATUS(mvHwsMtiPcs200Rev2IfInit(hwsPcsFuncsPtr)); use 400 pcs */
    CHECK_STATUS(mvHwsMtiPcs400Rev2IfInit(hwsPcsFuncsPtr));
    CHECK_STATUS(mvHwsMtiLowSpeedPcsRev2IfInit(hwsPcsFuncsPtr));
#endif

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

    CHECK_STATUS(mvHwsMti100Mac_SIP7IfInit(hwsMacFuncsPtr));
    CHECK_STATUS(mvHwsMti400Mac_SIP7IfInit(hwsMacFuncsPtr));

    return GT_OK;
}

/*******************************************************************************
* hwsAasSerdesIfInit
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
static GT_STATUS hwsAasSerdesIfInit(GT_U8 devNum , MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock,GT_BOOL callbackInitOnly)
{
    MV_HWS_SERDES_FUNC_PTRS **hwsSerdesFuncsPtr;
    GT_U32 sdIndex;

    GT_UNUSED_PARAM(hwsSerdesFuncsPtr);
    GT_UNUSED_PARAM(sdIndex);
    GT_UNUSED_PARAM(devNum);
    GT_UNUSED_PARAM(serdesRefClock);
    GT_UNUSED_PARAM(callbackInitOnly);
#if 0
    if(GT_FALSE==callbackInitOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {

            CHECK_STATUS(mvHwsAnpSerdesSdwMuxSet(devNum, hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum, GT_TRUE));
            /* Enable SWD rx_clk_4x_enable, tx_clk_4x_enable , reset rf_reset phase fifo */
            CHECK_STATUS(hwsSerdesRegSetFuncPtr(devNum, 0, EXTERNAL_REG,
                                                hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                SDW_LANE_CONTROL_1,
                                                0xC000010,
                                                0xC000010));
        }
    }

    CHECK_STATUS(hwsSerdesGetFuncPtr(devNum, &hwsSerdesFuncsPtr));

    if (serdesRefClock == MHz_25)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_N5XC56GP5X4, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_25MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_25MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
    }
    else if (serdesRefClock == MHz_156)
    {
        CHECK_STATUS(mvHwsComphyIfInit(devNum, COM_PHY_N5XC56GP5X4, hwsSerdesFuncsPtr,
                                       (PRV_SHARED_SERDES_DIR_SERDES_IF_SRC_GLOBAL_VAR_GET(serdesExternalFirmware)) ? MV_HWS_COMPHY_156MHZ_EXTERNAL_FILE_FIRMWARE : MV_HWS_COMPHY_156MHZ_REF_CLOCK_FIRMWARE,
                                       callbackInitOnly));
    }
    else
    {
        return GT_BAD_STATE;
    }

    if(GT_FALSE==callbackInitOnly)
    {
        for(sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {
            {
                /* Enable SD(internal memory) TX: TXDCLK_4X_EN_LANE */
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    0x5534,
                                                    0x1,
                                                    0x1));
                /* Enable SD(internal memory) RX: RXDCLK_4X_EN_LANE */
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    0x5630,
                                                    0x40,
                                                    0x40));
                /*Select new command interface in FW*/
                CHECK_STATUS(mvHwsComphySerdesRegisterWrite(devNum, 0,
                                                    hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb[sdIndex].serdesNum,
                                                    0xe774,
                                                    0xFFFFFFF,
                                                    0x1));
            }
        }
    }
#endif

    return GT_OK;
}

extern const MV_HWS_REG_ADDR_FIELD_STC aasMacGlobalExtDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasMacPortExtDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasMac100Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasMac400Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasPcs800ExtDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasAnpDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasAnDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasRsfecDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasLpcsDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasPcs400Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasPcs200Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasPcs100Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasPcs50Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasCpuPcsExtDb[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasCpuPcs50Db[];
extern const MV_HWS_REG_ADDR_FIELD_STC aasCpuRsfecDb[];


/**
* @internal hwsAasRegDbInit function
* @endinternal
 *
*/
GT_STATUS hwsAasRegDbInit
(
    GT_U8 devNum
)
{
    GT_STATUS rc=GT_OK;

    rc  = mvHwsRegDbInit(devNum, MTI_GLOBAL_EXT_UNIT, aasMacGlobalExtDb);
    rc += mvHwsRegDbInit(devNum, MTI_EXT_UNIT, aasMacPortExtDb);
    rc += mvHwsRegDbInit(devNum, MTI_MAC100_UNIT, aasMac100Db);
    rc += mvHwsRegDbInit(devNum, MTI_MAC400_UNIT, aasMac400Db);

    rc += mvHwsRegDbInit(devNum, MTI_PCS_EXT_UNIT, aasPcs800ExtDb);
    rc += mvHwsRegDbInit(devNum, MTI_LOW_SP_PCS_UNIT, aasLpcsDb);
    rc += mvHwsRegDbInit(devNum, MTI_RSFEC_UNIT, aasRsfecDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS400_UNIT, aasPcs400Db);
    rc += mvHwsRegDbInit(devNum, MTI_PCS200_UNIT, aasPcs200Db);
    rc += mvHwsRegDbInit(devNum, MTI_PCS100_UNIT, aasPcs100Db);
    rc += mvHwsRegDbInit(devNum, MTI_PCS50_UNIT, aasPcs50Db);

    rc += mvHwsRegDbInit(devNum, ANP_400_UNIT, aasAnpDb);
    rc += mvHwsRegDbInit(devNum, AN_400_UNIT, aasAnDb);

    rc += mvHwsRegDbInit(devNum, MTI_CPU_GLOBAL_EXT_UNIT, aasMacGlobalExtDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_EXT_UNIT, aasMacPortExtDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_MAC_UNIT, aasMac100Db);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_PCS_EXT_UNIT, aasCpuPcsExtDb);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_SGPCS_UNIT, aasLpcsDb); /*LPCS*/
    rc += mvHwsRegDbInit(devNum, MTI_CPU_PCS_UNIT, aasCpuPcs50Db);
    rc += mvHwsRegDbInit(devNum, MTI_CPU_RSFEC_UNIT, aasCpuRsfecDb);



#if 0
    rc += mvHwsRegDbInit(devNum, MIF_400_UNIT, hawkMifRegistersDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS400_UNIT, hawkMtiPcs400UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS200_UNIT, hawkMtiPcs200UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS100_UNIT, hawkMtiPcs100UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_PCS50_UNIT, hawkMtiPcs50UnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_RSFEC_UNIT, hawkMtiRsFecUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_LOW_SP_PCS_UNIT, hawkLpcsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
    rc += mvHwsRegDbInit(devNum, MTI_USX_RSFEC_STATISTICS_UNIT, hawkRsfecStatisticsUnitsDb);
#endif

    return rc;
}

/**
* @internal hwsAasIfPreInit function
* @endinternal
 *
*/
GT_STATUS hwsAasIfPreInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    GT_STATUS rc;
    GT_U32 i;

    if (funcPtr == NULL)
        return GT_FAIL;

    if ((funcPtr->osTimerWkPtr == NULL) || (funcPtr->osMemSetPtr == NULL) ||
        (funcPtr->osFreePtr == NULL) || (funcPtr->osMallocPtr == NULL) ||
        (funcPtr->osExactDelayPtr == NULL) || (funcPtr->sysDeviceInfo == NULL) ||
        (funcPtr->osMemCopyPtr == NULL) ||
        (funcPtr->serdesRegSetAccess == NULL) ||(funcPtr->serdesRegGetAccess == NULL) ||
        (funcPtr->serverRegSetAccess == NULL) || (funcPtr->serverRegGetAccess == NULL) ||
        (funcPtr->registerSetAccess == NULL) || (funcPtr->registerGetAccess == NULL)
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

#if 0
    hwsSFlashSetFuncPtr = funcPtr->spiFlashSetAccess;
    hwsSFlashGetFuncPtr = funcPtr->spiFlashGetAccess;
#endif

    hwsDeviceSpecInfo[devNum].devType = AAS;
#ifndef MV_HWS_FREE_RTOS
    hwsPpHwTraceEnablePtr = ((HWS_EXT_FUNC_STC_PTR*)funcPtr->extFunctionStcPtr)->ppHwTraceEnable;
#endif
    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }

    rc = hwsAasRegDbInit(devNum);

    return rc;
}

static GT_STATUS hwsAasSerdesGetClockAndInit
(
    GT_U8 devNum,
    GT_BOOL callbackInitOnly
)
{
    MV_HWS_REF_CLOCK_SUP_VAL serdesRefClock;

    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc != NULL)
    {
        CHECK_STATUS(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc(devNum, &serdesRefClock));
        /* Init SERDES unit relevant for current device */
        CHECK_STATUS(hwsAasSerdesIfInit(devNum,serdesRefClock,callbackInitOnly));
    }

    return GT_OK;
}




#ifdef SHARED_MEMORY

GT_STATUS hwsAasIfReIoad
(
    GT_U8 devNum
)
{
    CHECK_STATUS(macIfInit(devNum));

    CHECK_STATUS(hwsAasSerdesGetClockAndInit(devNum,GT_TRUE));
    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    return GT_OK;
}
#endif

GT_STATUS hwsAasPortsParamsSupModesMapSet
(
    GT_U8 devNum
)
{
    mvHwsAasPortElementsDbInit(devNum);
#ifndef MV_HWS_REDUCED_BUILD
    PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum) = hwsPortsAasParamsSupModesMap;
#endif
    return GT_OK;
}



GT_VOID hwsAasIfInitHwsDevFunc
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{
    MV_HWS_DEV_FUNC_PTRS *hwsDevFunc;
    GT_U32  data, version;

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

    if ( funcPtr != NULL )
    {
        if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc == NULL)
        {
            hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc = funcPtr->sysDeviceInfo;
        }

        if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc == NULL)
        {
            hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].serdesRefClockGetFunc = funcPtr->serdesRefClockGet;
        }

    #ifndef MV_HWS_FREE_RTOS
        if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc == NULL)
        {
            hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].coreClockGetFunc = funcPtr->coreClockGetPtr;
        }
    #endif
    }

    /* get device id and version */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].deviceInfoFunc(devNum, &data, &version);


    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].pcsMarkModeFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc = NULL;

    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc = NULL;

    /* Configures port init / reset functions */
#if (!defined MV_HWS_REDUCED_BUILD) || defined(MICRO_INIT)
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortinitFunc = mvHwsAasPortInit;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAnpStartFunc = mvHwsAasPortAnpStart;
#endif
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devApPortinitFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortResetFunc = mvHwsAasPortReset;

    /* Configures  device handler functions */
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetExtFunc = mvHwsPortGeneralAutoTuneSetExt;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgFunc     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portExtModeCfgGetFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsIndexGetFunc = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portLbSetWaFunc        = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneStopFunc   = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portAutoTuneSetWaFunc  = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portFixAlign90Func     = NULL;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsSetFunc      = mvHwsAasPortParamsSet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].portParamsGetLanesFunc = mvHwsAasPortParamsGetLanes;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devPortAccessCheckFunc = mvHwsAasExtMacClockEnableGet;
    hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].devUnitFieldFindAddrFunc = mvHwsAasUnitFieldAddrFind;
}

/*******************************************************************************
* hwsAasIfInit
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
GT_STATUS hwsAasIfInit
(
    GT_U8 devNum,
    HWS_OS_FUNC_PTR *funcPtr
)
{

    MV_HWS_DEV_TYPE               devType;
    MV_HWS_DEV_FUNC_PTRS          *hwsDevFunc;
    GT_BOOL                       skipWritingHW = GT_FALSE;
#ifndef MV_HWS_REDUCED_BUILD
    GT_U32                        sdIndex;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
#endif

    hwsDeviceSpecInfo[devNum].numOfTiles = hwsAasNumOfTilesGet();

    if (PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        return GT_ALREADY_EXIST;
    }

#ifndef MV_HWS_REDUCED_BUILD
    CHECK_STATUS(cpssSystemRecoveryStateGet(&tempSystemRecovery_Info));
#endif

    if (funcPtr == NULL)
    {
        skipWritingHW = GT_TRUE;
    }

#ifdef MV_HWS_REDUCED_BUILD
    if(aasDbInitDone == GT_FALSE)
    {
        CHECK_STATUS(hwsAasIfPreInit(devNum, funcPtr));
    }
#endif
    hwsDeviceSpecInfo[devNum].devType = AAS;
    hwsDeviceSpecInfo[devNum].devNum = devNum;
    hwsDeviceSpecInfo[devNum].lastSupPortMode = LAST_PORT_MODE;
    hwsDeviceSpecInfo[devNum].serdesType = COM_PHY_N5XC56GP5X4;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesNumber = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.spicoNumber = 0;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr = NULL;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesVosOverride = GT_FALSE;
    hwsDeviceSpecInfo[devNum].avagoSerdesInfo.sbusAddrToSerdesFuncPtr = NULL;

    {
        HOST_CONST MV_HWS_PER_SERDES_INFO_STC     *serdesInfoPtr;

        if(hwsDeviceSpecInfo[devNum].numOfTiles == 1)
        {
            serdesInfoPtr = aasSerdesDb;
            hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(aasSerdesDb) / sizeof(aasSerdesDb[0]);
        }
        else /*2 tiles*/
        {
            serdesInfoPtr = aas_2_tiles_SerdesDb;
            hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses = sizeof(aas_2_tiles_SerdesDb) / sizeof(aas_2_tiles_SerdesDb[0]);
        }

        hwsDeviceSpecInfo[devNum].serdesInfo.serdesesDb = NULL;
        for (sdIndex = 0; sdIndex < hwsDeviceSpecInfo[devNum].serdesInfo.numOfSerdeses; sdIndex++)
        {
            CHECK_STATUS(mvHwsComphySerdesAddToDatabase(devNum, sdIndex, &serdesInfoPtr[sdIndex]));
        }
    }

    hwsAasPortsParamsSupModesMapSet(devNum);

    hwsDeviceSpecInfo[devNum].portsNum = hwsDeviceSpecInfo[devNum].numOfTiles*(80 + 1);
    devType = hwsDeviceSpecInfo[devNum].devType;
    hwsDeviceSpecInfo[devNum].ipcConnType = HOST2HWS_LEGACY_MODE;
    hwsDeviceSpecInfo[devNum].gopRev = GOP_16NM_REV1;

    hwsAasIfInitHwsDevFunc(devNum,funcPtr);

    hwsDeviceSpecGetFuncPtr(&hwsDevFunc);

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_TRUE);
        hwsOsPrintf("****** Aas ifInit start ******\n");
    }
#endif
    hwsInitPortsModesParam(devNum,NULL);
    devType = devType; /* avoid warning */

    if(PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] == GT_FALSE)
    {
        CHECK_STATUS(hwsDevicePortsElementsCfg(devNum));
    }

    if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E) && (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)) ||
          (skipWritingHW == GT_TRUE))
    {
        /*no HW write */
        CHECK_STATUS(hwsAasSerdesGetClockAndInit(devNum,GT_TRUE));
    }
    else

    /* Init all MAC units relevant for current device */
    CHECK_STATUS(macIfInit(devNum));
    /* Init all PCS units relevant for current device */
    CHECK_STATUS(pcsIfInit(devNum));

    /* Init all ANP units relevant for current device */
    CHECK_STATUS(mvHwsAasAnpInit(devNum,skipWritingHW /*False*/));
    /* Init configuration sequence executer */

    PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_TRUE;

    if (skipWritingHW == GT_FALSE)
    {
#ifndef  MV_HWS_FREE_RTOS
        if (!(((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
              (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E)) &&
              (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E)))
#endif
        {
            CHECK_STATUS(mvHwsAasRegisterMisconfigurationSet(devNum));
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsPpHwTraceEnablePtr(devNum, 1, GT_FALSE);
        hwsOsPrintf("******** Aas ifInit end ********\n");
    }
#endif

    return GT_OK;
}


#if !defined (MV_HWS_REDUCED_BUILD)
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
    mvHwsComphySerdesDestroyDatabase(devNum);

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
    CPSS_TBD_BOOKMARK_AAS

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
    CPSS_TBD_BOOKMARK_AAS

    devNum = devNum;

    return;
}

/*******************************************************************************
* hwsAasIfClose
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
void hwsAasIfClose(GT_U8 devNum)
{
    GT_U32  i;

    if (PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum])
    {
        PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[devNum] = GT_FALSE;
    }
    /* if there is no active device remove DB */
    for (i = 0; i < HWS_MAX_DEVICE_NUM; i++)
    {
        if (PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(hwsInitDone)[i])
        {
            break;
        }
    }
    if (i == HWS_MAX_DEVICE_NUM)
    {
        PRV_SHARED_LAB_SERVICES_DIR_AAS_DEV_INIT_SRC_GLOBAL_VAR(aasDbInitDone) = GT_FALSE;
    }

    serdesIfClose(devNum);
    pcsIfClose(devNum);
    macIfClose(devNum);
    mvHwsAnpClose(devNum);
    hwsSerdesIfClose(devNum);
    hwsPcsIfClose(devNum);
    hwsMacIfClose(devNum);

    for(i = 0; i < MV_HWS_MAX_CPLL_NUMBER; i++)
    {
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllInitDoneStatusArr[i] = GT_FALSE;
        hwsDeviceSpecInfo[devNum].avagoSerdesInfo.cpllCurrentOutFreqArr[i] = MV_HWS_MAX_OUTPUT_FREQUENCY;
    }
    hwsPortsElementsClose(devNum);
}

GT_STATUS mvHwsAasRegisterMisconfigurationSet
(
    IN GT_U8    devNum
)
{
    GT_U32          extField;
    MV_HWS_UNITS_ID unitId = MTI_PCS_EXT_UNIT;
    GT_U32          portNum;

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** Start of mvHwsAasRegisterMisconfigurationSet ******\n");
    }

#endif

    /* temp code - needed in CC-minus model, may be removed later then HW value will be fixed */
    for (portNum = 0; portNum < (hwsDeviceSpecInfo[devNum].portsNum - 1); portNum++)
    {
        if(mvHwsMtipIsReducedPort(devNum,portNum) == GT_TRUE)
        {

        }
        else
        {
            if(portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS == 0)
            {
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN0_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN2_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN4_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN6_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN8_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN10_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN12_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
                extField = AAS_PCS800_EXT_GLOBAL_CHANNEL_CONTROL_2_GC_FEC91_1LANE_IN14_E;
                CHECK_STATUS(genUnitRegisterFieldSet(devNum, 0, portNum, unitId, extField, 1, NULL));
            }
        }
    }

#ifndef  MV_HWS_FREE_RTOS
    if (hwsPpHwTraceFlag && hwsPpHwTraceEnablePtr != NULL) {
        hwsOsPrintf("****** End of mvHwsAasRegisterMisconfigurationSet ******\n");
    }
#endif

    return GT_OK;
}

GT_STATUS dbgAlex
()
{
    GT_U32 portIndex, address;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg;

    for(portIndex = 0; portIndex < (hwsDeviceSpecInfo[0].portsNum - 1); portIndex++)
    {
        if (mvHwsMtipIsReducedPort(0, portIndex))
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_GLOBAL_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
            hwsOsPrintf("CPU_GLOBAL_EXT[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_GLOBAL_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
            hwsOsPrintf("GLOBAL_EXT[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    portIndex = MV_HWS_AAS_GOP_PORT_1TILE_CPU_0_CNS;
    address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_GLOBAL_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
    hwsOsPrintf("CPU_GLOBAL_EXT[port_%d] = 0x%x\n", portIndex, address);
    hwsOsPrintf("\n");

    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        if (mvHwsMtipIsReducedPort(0, portIndex))
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
            hwsOsPrintf("CPU_GLOBAL_EXT[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_EXT_UNIT, _10GBase_KR, AAS_MAC_PORT_EXT_CONTROL_TX_LOC_FAULT_E, &fieldReg);
            hwsOsPrintf("PORT_EXT[port_%d] = 0x%x\n", portIndex, address);
            address = genUnitFindAddressPrv(0, portIndex, MTI_EXT_UNIT, _400GBase_KR8, AAS_MAC_PORT_EXT_CONTROL_TX_LOC_FAULT_E, &fieldReg);
            hwsOsPrintf("PORT_EXT[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    portIndex = MV_HWS_AAS_GOP_PORT_1TILE_CPU_0_CNS;
    address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
    hwsOsPrintf("CPU_GLOBAL_EXT[port_%d] = 0x%x\n", portIndex, address);
    hwsOsPrintf("\n");

    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        if (mvHwsMtipIsReducedPort(0, portIndex))
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_MAC_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
            hwsOsPrintf("CPU_MAC_100[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_MAC100_UNIT, _10GBase_KR, AAS_MAC100_REVISION_CORE_REVISION_E, &fieldReg);
            hwsOsPrintf("MAC_100[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    portIndex = MV_HWS_AAS_GOP_PORT_1TILE_CPU_0_CNS;
    address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_MAC_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
    hwsOsPrintf("CPU_MAC_100[port_%d] = 0x%x\n", portIndex, address);
    hwsOsPrintf("\n");

    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, MTI_MAC400_UNIT, _400GBase_KR8, AAS_MAC400_REVISION_CORE_REVISION_E, &fieldReg);
        hwsOsPrintf("MAC_400[port_%d] = 0x%x\n", portIndex, address);
    }
    hwsOsPrintf("\n");


    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        if ((portIndex % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_PCS_EXT_UNIT, _400GBase_KR8, AAS_PCS800_EXT_GLOBAL_PMA_CONTROL_GC_SD_8X_E, &fieldReg);
            hwsOsPrintf("PCS800_EXT[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_PCS_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
            hwsOsPrintf("MTI_CPU_PCS_EXT_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    hwsOsPrintf("\n");
    portIndex = MV_HWS_AAS_GOP_PORT_1TILE_CPU_0_CNS;
    address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_PCS_EXT_UNIT, _10GBase_KR, AAS_MAC_GLOBAL_EXT_GLOBAL_MAC_CLOCK_AND_RESET_CONTROL_CMN_MAC_RESET__E, &fieldReg);
    hwsOsPrintf("MTI_CPU_PCS_EXT_UNIT[port_%d] = 0x%x\n", portIndex, address);
    hwsOsPrintf("\n");


    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, ANP_400_UNIT, _10GBase_KR, AAS_ANP_UNIT_CONTROL_1_AN_AP_TRAIN_TYPE_E, &fieldReg);
        hwsOsPrintf("ANP_400_UNIT[port_%d] = 0x%x\n", portIndex, address);
    }

    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, AN_400_UNIT, _10GBase_KR, AAS_AN_UNIT_REG_802_3AP_AUTO_NEGOTIATION_CONTROL_CONTROL_RESERVED_E, &fieldReg);
        hwsOsPrintf("AN_400_UNIT[port_%d] = 0x%x\n", portIndex, address);
    }

    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, MTI_PCS_EXT_UNIT, _10GBase_KR, AAS_PCS800_EXT_PORT0_CONTROL_P0_SXGMII_MODE_E, &fieldReg);
        hwsOsPrintf("PCS_EXT_UNIT[port_%d] = 0x%x\n", portIndex, address);
    }
    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, MTI_PCS400_UNIT, _400GBase_KR8, AAS_PCS400_CONTROL1_SPEED_SELECTION_E, &fieldReg);
        hwsOsPrintf("MTI_PCS400_UNIT[port_%d] = 0x%x\n", portIndex, address);
    }
    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, MTI_PCS200_UNIT, _200GBase_KR8, AAS_PCS200_CONTROL1_SPEED_SELECTION_E, &fieldReg);
        hwsOsPrintf("MTI_PCS200_UNIT[port_%d] = 0x%x\n", portIndex, address);
    }
    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        address = genUnitFindAddressPrv(0, portIndex, MTI_PCS100_UNIT, _10GBase_KR, AAS_PCS100_CONTROL1_SPEED_SELECTION_E, &fieldReg);
        hwsOsPrintf("MTI_PCS100_UNIT[port_%d] = 0x%x\n", portIndex, address);
    }
    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        if (mvHwsMtipIsReducedPort(0, portIndex))
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_PCS_UNIT, _10GBase_KR, AAS_CPU_PCS50_CONTROL1_SPEED_SELECTION_E, &fieldReg);
            hwsOsPrintf("MTI_CPU_PCS_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_PCS50_UNIT, _10GBase_KR, AAS_PCS50_CONTROL1_SPEED_SELECTION_E, &fieldReg);
            hwsOsPrintf("MTI_PCS50_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        if (mvHwsMtipIsReducedPort(0, portIndex))
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_SGPCS_UNIT, SGMII, AAS_LPCS_CONTROL_P0_SPEED_6_E, &fieldReg);
            hwsOsPrintf("MTI_CPU_SGPCS_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_LOW_SP_PCS_UNIT, SGMII, AAS_LPCS_CONTROL_P0_SPEED_6_E, &fieldReg);
            hwsOsPrintf("MTI_LOW_SP_PCS_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    for(portIndex = 0; portIndex < hwsDeviceSpecInfo[0].portsNum - 1; portIndex++)
    {
        if (mvHwsMtipIsReducedPort(0, portIndex))
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_CPU_RSFEC_UNIT, _50GBase_KR, AAS_CPU_RSFEC_UNIT_RSFEC_CONTROL_P0_BYPASS_CORRECTION_E, &fieldReg);
            hwsOsPrintf("MTI_CPU_RSFEC_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
        else
        {
            address = genUnitFindAddressPrv(0, portIndex, MTI_RSFEC_UNIT, _50GBase_KR, AAS_RSFEC_UNIT_RSFEC_CONTROL_P0_BYPASS_CORRECTION_E, &fieldReg);
            hwsOsPrintf("MTI_RSFEC_UNIT[port_%d] = 0x%x\n", portIndex, address);
        }
    }
    return GT_OK;
}

#endif


