/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*  mvHwsAasPortModeElements.c
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
#include <cpss/generic/labservices/port/gop/silicon/aas/mvHwsAasPortIf.h>

/************************* definition *****************************************************/
/************************* Globals ********************************************************/


/* Last line of each SupModes array MUST be NON_SUP_MODE */
const MV_HWS_PORT_INIT_PARAMS hwsAasPort0SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit      FecSupported */
    { _400GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _64BIT_ON,  RS_FEC_544_514 | RS_FEC_272_257},
    { _800GBase_KR8, MTI_MAC_400,   0,     MTI_PCS_400,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   8,    XAUI_MEDIA,   _128BIT_ON, RS_FEC_544_514 | RS_FEC_272_257},
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _64BIT_ON,  RS_FEC_544_514 | RS_FEC_272_257},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  RS_FEC | FC_FEC | FEC_OFF},
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  FC_FEC | FEC_OFF},

    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON,  FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsAasPort1SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _64BIT_ON, RS_FEC_544_514 | RS_FEC_272_257},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  RS_FEC | FC_FEC | FEC_OFF},
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_50,          0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  FC_FEC | FEC_OFF},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

const MV_HWS_PORT_INIT_PARAMS hwsAasPort2SupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { _50GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC_544_514, _26_5625G_PAM4,  NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _64BIT_ON, RS_FEC_544_514 | RS_FEC_272_257},
    { _25GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  RS_FEC | FC_FEC | FEC_OFF},
    { _10GBase_KR,   MTI_MAC_100,   0,     MTI_PCS_100,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  FC_FEC | FEC_OFF},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};


const MV_HWS_PORT_INIT_PARAMS hwsAasCpuPortSupModes[] = {
    /* port mode,    MAC Type    Mac Num   PCS Type         PCS Num  FEC             Speed            FirstLane  LanesList       LanesNum  Media         10Bit     FecSupported */
    { _25GBase_KR,   MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   RS_FEC,         _25_78125G,      NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _64BIT_ON, RS_FEC | FC_FEC | FEC_OFF},
    { _10GBase_KR,   MTI_CPU_MAC,   0,     MTI_CPU_PCS,         0,   FEC_OFF,        _10_3125G,       NA_NUM,   {0,0,0,0,0,0,0,0},   1,    XAUI_MEDIA,   _32BIT_ON,  FC_FEC | FEC_OFF},
    { NON_SUP_MODE,  MAC_NA,      NA_NUM,  PCS_NA,         NA_NUM,   FEC_OFF,        SPEED_NA,        NA_NUM,   {0,0,0,0,0,0,0,0},   0,    XAUI_MEDIA,   _10BIT_ON, FEC_NA}
};

/* map the right supported modes table for each port */
const MV_HWS_PORT_INIT_PARAMS **hwsPortsAasParamsSupModesMap;


#define AAS_P0_SUP_MODES        hwsAasPort0SupModes
#define AAS_P1_SUP_MODES        hwsAasPort1SupModes
#define AAS_P2_SUP_MODES        hwsAasPort2SupModes
#define AAS_P16_SUP_MODES       hwsAasCpuPortSupModes

#define CLUSTER0_AAS_SINGLE_GOP   \
 /*0*/    AAS_P0_SUP_MODES          ,   \
 /*1*/    AAS_P1_SUP_MODES          ,   \
 /*2*/    AAS_P2_SUP_MODES          ,   \
 /*3*/    AAS_P1_SUP_MODES          ,   \
 /*4*/    AAS_P2_SUP_MODES          ,   \
 /*5*/    AAS_P1_SUP_MODES          ,   \
 /*6*/    AAS_P2_SUP_MODES          ,   \
 /*7*/    AAS_P1_SUP_MODES          ,   \
 /*8*/    AAS_P0_SUP_MODES          ,   \
 /*9*/    AAS_P1_SUP_MODES          ,   \
 /*10*/   AAS_P2_SUP_MODES          ,   \
 /*11*/   AAS_P1_SUP_MODES          ,   \
 /*12*/   AAS_P2_SUP_MODES          ,   \
 /*13*/   AAS_P1_SUP_MODES          ,   \
 /*14*/   AAS_P2_SUP_MODES          ,   \
 /*15*/   AAS_P1_SUP_MODES          ,   \
 /*16*/   AAS_P16_SUP_MODES         ,   \
 /*17*/   AAS_P16_SUP_MODES         ,   \
 /*18*/   AAS_P16_SUP_MODES         ,   \
 /*19*/   AAS_P16_SUP_MODES

const MV_HWS_PORT_INIT_PARAMS *hwsPortsAas_ParamsSupModesMap[] =
{
    /* GOP0 of : 8+8+4 MACs  : index  0..19 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP1 of : 8+8+4 MACs  : index 20..39 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP2 of : 8+8+4 MACs  : index 40..59 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP3 of : 8+8+4 MACs  : index 60..79 */
    CLUSTER0_AAS_SINGLE_GOP,

    /* CPU port : index 80 */
    hwsAasCpuPortSupModes
};

const MV_HWS_PORT_INIT_PARAMS *hwsPortsAas_2_tiles_ParamsSupModesMap[] =
{
    /* GOP0 of : 8+8+4 MACs  : index  0..19 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP1 of : 8+8+4 MACs  : index 20..39 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP2 of : 8+8+4 MACs  : index 40..59 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP3 of : 8+8+4 MACs  : index 60..79 */
    CLUSTER0_AAS_SINGLE_GOP,

    /* GOP0 of : 8+8+4 MACs  : index 80..99 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP1 of : 8+8+4 MACs  : index 100..119 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP2 of : 8+8+4 MACs  : index 120..139 */
    CLUSTER0_AAS_SINGLE_GOP,
    /* GOP3 of : 8+8+4 MACs  : index 140..159 */
    CLUSTER0_AAS_SINGLE_GOP,

    /* CPU port : index 160 */
    hwsAasCpuPortSupModes
};

/************************* functions ******************************************************/

GT_VOID mvHwsAasPortElementsDbInit
(
    GT_U8 devNum
)
{
    hwsPortsAasParamsSupModesMap = (hwsDeviceSpecInfo[devNum].numOfTiles == 1) ?
        hwsPortsAas_ParamsSupModesMap :
        hwsPortsAas_2_tiles_ParamsSupModesMap;
}

GT_STATUS mvHwsAasPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
)
{
    GT_U16    i = 0, j = 0;
    MV_HWS_HAWK_CONVERT_STC         convertIdx;
    GT_U32    portOffset, firstSdInGop;
    MV_HWS_PORT_SERDES_TO_MAC_MUX   hwsMacToSerdesMuxStc;

    /* avoid warnings */
    portGroup = portGroup;

    if (portParams->portStandard == NON_SUP_MODE)
    {
        return GT_NOT_SUPPORTED;
    }

    CHECK_STATUS(mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portParams->portStandard, &convertIdx));

    if(mvHwsMtipIsReducedPort(devNum,phyPortNum))
    {
        if (phyPortNum < hwsDeviceSpecInfo[devNum].portsNum)
        {
            portParams->activeLanesList[0] = phyPortNum;
        }
        else
        {
            if (hwsDeviceSpecInfo[devNum].numOfTiles > 1)
            {
                portParams->activeLanesList[0] = (phyPortNum == MV_HWS_AAS_GOP_PORT_2TILE_CPU_0_CNS) ? 160 : 161;
            }
            else
            {
                portParams->activeLanesList[0] = 80;
            }
        }
        portParams->firstLaneNum = portParams->activeLanesList[0];
        return GT_OK;
    }


    if(GT_OK == mvHwsPortLaneMacToSerdesMuxGet(devNum, (GT_U8)portGroup, phyPortNum, &hwsMacToSerdesMuxStc))
    {
        portOffset = convertIdx.ciderIndexInUnit;
        firstSdInGop = convertIdx.ciderUnit * 8;
    }
    else
    {
        portParams->firstLaneNum = (GT_U16)phyPortNum;
        portOffset = 0;
        firstSdInGop = portParams->firstLaneNum;
    }

    for (i = 0 ; i < portParams->numOfActLanes ; i++)
    {
        j = portOffset + i; /* start from the right offset in the ports group */
        if(hwsMacToSerdesMuxStc.enableSerdesMuxing)
        {
            portParams->activeLanesList[i] = (GT_U16)firstSdInGop + (GT_U16)(hwsMacToSerdesMuxStc.serdesLanes[j]);
        }
        else
        {
            portParams->activeLanesList[i] = (GT_U16)firstSdInGop + j;
        }
    }
    portParams->firstLaneNum = portParams->activeLanesList[0];

    return GT_OK;
}

GT_STATUS mvHwsAasPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
#ifndef MV_HWS_REDUCED_BUILD
    MV_HWS_DEVICE_PORTS_ELEMENTS    curDevicePorts = hwsDevicesPortsElementsArray[devNum];
    MV_HWS_PORT_INIT_PARAMS         *curPortParams;
    const MV_HWS_PORT_INIT_PARAMS   *newPortParams;
    GT_U32                          portModeIndex;
    GT_U8                           i;
    GT_U32                          serdesIndex;
    MV_HWS_PER_SERDES_INFO_PTR      serdesInfo;

    if (phyPortNum > hwsDeviceSpecInfo[devNum].portsNum)
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
        for (i = 0 ; i < curPortParams->numOfActLanes ; i++)
        {
            CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,curPortParams->activeLanesList[i],&serdesIndex,&serdesInfo));
            serdesInfo->serdesToMac[0] = NA_NUM;
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

    for (i = 0 ; i < curPortParams->numOfActLanes ; i++)
    {
        /* save the mac port for the serdes num in the database*/
        CHECK_STATUS(mvHwsSerdesGetInfoBySerdesNum(devNum,curPortParams->activeLanesList[i],&serdesIndex,&serdesInfo));
        serdesInfo->serdesToMac[0] = phyPortNum;
    }

    return GT_OK;

#else

    return GT_OK;

#endif
}


