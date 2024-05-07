/* ******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/* *******************************************************************
This file contains functions to define hardware control operations to
the selected Serdes code.

Including Serdes Read/Write and Set/Get Pin Config, etc.
******************************************************************* */
#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdApiRegs.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdHwSerdesCntl.h"
#include <mpdTypes.h>
#include "mtdUtils.h"

#if MTD_PKG_CE_SERDES
#include "mcesdTop.h"
#include "mcesdInitialization.h"

#if MTD_CE_SERDES28X2
#include "mcesdC28GP4X2_Defs.h"
#include "mcesdC28GP4X2_RegRW.h"
#include "mcesdC28GP4X2_API.h"

/* Forward declaration of static callback functions */
MTD_STATIC MCESD_STATUS mtdSerdes28X2MDIORead(MCESD_DEV_PTR pSerdesDev, MCESD_U32 apbAddress, MCESD_U32 *value);
MTD_STATIC MCESD_STATUS mtdSerdes28X2MDIOWrite(MCESD_DEV_PTR pSerdesDev, MCESD_U32 apbAddress, MCESD_U32 value);
MTD_STATIC MCESD_STATUS mtdSerdes28X2SetPinCfg(MCESD_DEV_PTR pSerdesDev, MCESD_U16 pin, MCESD_U32 pinValue);
MTD_STATIC MCESD_STATUS mtdSerdes28X2GetPinCfg(MCESD_DEV_PTR pSerdesDev, MCESD_U16 pin, MCESD_U32 *pinValue);
#endif  /* MTD_CE_SERDES28X2 */

#if MTD_CE_SERDES32X1X4
#include "mcesdN5C32GP5X4_Defs.h"
#include "mcesdN5C32GP5X4_RegRW.h"
#include "mcesdN5C32GP5X4_API.h"

/* Forward declaration of static callback functions */
MTD_STATIC MCESD_STATUS mtdSerdes32X4MDIORead(MCESD_DEV_PTR pSerdesDev, MCESD_U32 apbAddress, MCESD_U32 *value);
MTD_STATIC MCESD_STATUS mtdSerdes32X4MDIOWrite(MCESD_DEV_PTR pSerdesDev, MCESD_U32 apbAddress, MCESD_U32 value);
MTD_STATIC MCESD_STATUS mtdSerdes32X4SetPinCfg(MCESD_DEV_PTR pSerdesDev, MCESD_U16 pin, MCESD_U32 pinValue);
MTD_STATIC MCESD_STATUS mtdSerdes32X4GetPinCfg(MCESD_DEV_PTR pSerdesDev, MCESD_U16 pin, MCESD_U32 *pinValue);
#endif  /* MTD_CE_SERDES32X1X4 */


MTD_STATIC MCESD_STATUS mtdSerdesWait(MCESD_DEV_PTR pSerdesDev, MCESD_U32 ms);

/******************************************************************************
 API Functions shared among different devices and Serdes
*******************************************************************************/

/******************************************************************************
 MTD_STATUS mtdInitSerdesDev
*******************************************************************************/
MTD_STATUS mtdInitSerdesDev
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 anyPort
)
{
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MCESD_DEV_PTR pSerdesDev;

        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, anyPort, &pSerdesDev));

#if MTD_CE_SERDES28X2
        MTD_ATTEMPT(mcesdLoadDriver(MTD_COMPHY_MajorRev, MTD_COMPHY_MinorRev,
                                    &mtdSerdes28X2MDIORead, &mtdSerdes28X2MDIOWrite,
                                    &mtdSerdes28X2SetPinCfg, &mtdSerdes28X2GetPinCfg, &mtdSerdesWait,
                                    (MCESD_PVOID)devPtr, pSerdesDev));
#else
        MTD_DBG_ERROR("mtdInitSerdesDev: Serdes 28G X2 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
    }   
    else if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
        MCESD_DEV_PTR pSerdesDev;

        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, anyPort, &pSerdesDev));

#if MTD_CE_SERDES32X1X4
        MTD_ATTEMPT(mcesdLoadDriver(MTD_COMPHY_MajorRev, MTD_COMPHY_MinorRev,
                                    &mtdSerdes32X4MDIORead, &mtdSerdes32X4MDIOWrite,
                                    &mtdSerdes32X4SetPinCfg, &mtdSerdes32X4GetPinCfg, &mtdSerdesWait,
                                    (MCESD_PVOID)devPtr, pSerdesDev));
#else
        MTD_DBG_ERROR("mtdInitSerdesDev: Serdes 32G X1/X4 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X4 */
    }     
    else
    {
        MTD_DBG_ERROR("mtdInitSerdesDev: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdUnloadSerdesDev
*******************************************************************************/
MTD_STATUS mtdUnloadSerdesDev /* work-in-progress */
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 anyPort
)
{
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr) || MTD_IS_X3610_DEVICE(devPtr) || MTD_IS_X3610_DEVICE(devPtr))
    {
        /* MCESD_DEV_PTR pSerdesDev; */

        /* MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, anyPort, &pSerdesDev)); */

#if MTD_PKG_CE_SERDES
        /* MTD_ATTEMPT(mcesdUnloadDriver(pSerdesDev)); */
#else
        MTD_DBG_ERROR("mtdUnloadSerdesDev: Serdes 28G X2/32G X1/32G X4 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
    }
    else
    {
        MTD_DBG_ERROR("mtdUnloadSerdesDev: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }
    MPD_UNUSED_PARAM(anyPort);
    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSetSerdesPortGetDevPtr
    Get a pointer to the Serdes device control structure
    from given MTD device pointer and set mapped Serdes Port by input mdioPort
*******************************************************************************/
MTD_STATUS mtdSetSerdesPortGetDevPtr
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    OUT MCESD_DEV_PTR *pSerdesDev
)
{
    MTD_ATTEMPT(mtdGetSerdesDevPtr(devPtr, pSerdesDev));
    MTD_ATTEMPT(mtdSetSerdesPort(devPtr, mdioPort));

    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdGetSerdesDevPtr
    Get a pointer to the Serdes device control structure
    from given MTD device pointer
*******************************************************************************/
MTD_STATUS mtdGetSerdesDevPtr
(
    IN MTD_DEV_PTR devPtr,
    OUT MCESD_DEV_PTR *pSerdesDev
)
{
    *pSerdesDev = NULL;

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
#if MTD_CE_SERDES28X2
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.c28gX2;
        *pSerdesDev = &serdesCtrlPtr->serdesDev;
#else
        MTD_DBG_ERROR("mtdGetSerdesDevPtr: Serdes 28G X2 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
    }
    else if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
#if MTD_CE_SERDES32X1X4
    PMTD_SERDES_CTRL serdesCtrlPtr;
    
    serdesCtrlPtr = &devPtr->serdesCtrl.n5c32gX1X4;
    *pSerdesDev = &serdesCtrlPtr->serdesDev;
#else
    MTD_DBG_ERROR("mtdGetSerdesDevPtr: Serdes 32G X1/X4 code not selected.\n");
    return MTD_FAIL;
#endif /* MTD_CE_SERDES32X1X4 */
    }    
    else
    {
        MTD_DBG_ERROR("mtdGetSerdesDevPtr: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSetSerdesPort
    Set mapped Serdes Port that used at given MDIO port
*******************************************************************************/
MTD_STATUS mtdSetSerdesPort
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
)
{
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
#if MTD_CE_SERDES28X2
        PMTD_SERDES_CTRL serdesCtrlPtr;
        serdesCtrlPtr = &devPtr->serdesCtrl.c28gX2;

        serdesCtrlPtr->serdesPort = (mdioPort/2) * 2;
#else
        MTD_DBG_ERROR("mtdSetSerdesPort: Serdes 28G X2 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
    }
    else if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
#if MTD_CE_SERDES32X1X4
        PMTD_SERDES_CTRL serdesCtrlPtr;
 
        serdesCtrlPtr = &devPtr->serdesCtrl.n5c32gX1X4;
        serdesCtrlPtr->serdesPort = (MTD_IS_X3680_DEVICE(devPtr)) ? ((mdioPort/4) * 4) : mdioPort;

#else
        MTD_DBG_ERROR("mtdSetSerdesPort: Serdes 32G X1/X4 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES32X1X4 */
    }     
    else
    {
        MTD_DBG_ERROR("mtdSetSerdesPort: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdHwSerdesRead
    Serdes Register Read API
*******************************************************************************/
MTD_STATUS mtdHwSerdesRead
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U32 reg,
    OUT MTD_U32 *value
)
{
    MCESD_DEV_PTR pSerdesDev;

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
#if MTD_CE_SERDES28X2
        MCESD_U8 lane;
        lane = mdioPort % 2;

        MTD_ATTEMPT(API_C28GP4X2_ReadReg(pSerdesDev, lane, reg, value));
#else
        MTD_DBG_ERROR("mtdHwSerdesRead: Serdes 28G X2 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
    }
    else if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
#if MTD_CE_SERDES32X1X4
            MCESD_U8 lane;
            lane = (MTD_IS_X3680_DEVICE(devPtr)) ? (mdioPort % 4) : 0;
    
            MTD_ATTEMPT(API_N5C32GP5X4_ReadReg(pSerdesDev, lane, reg, value));
#else
            MTD_DBG_ERROR("mtdHwSerdesRead: Serdes 32G X1/X4 code not selected.\n");
            return MTD_FAIL;
#endif  /* MTD_CE_SERDES32X1X4 */
    }    
    else
    {
        MTD_DBG_ERROR("mtdHwSerdesRead: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdHwSerdesWrite
    Serdes Register Write API
*******************************************************************************/
MTD_STATUS mtdHwSerdesWrite
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U32 reg,
    IN MTD_U32 value
)
{
    MCESD_DEV_PTR pSerdesDev;

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
#if MTD_CE_SERDES28X2
        MCESD_U8 lane;
        lane = mdioPort % 2;

        MTD_ATTEMPT(API_C28GP4X2_WriteReg(pSerdesDev, lane, reg, value));
#else
        MTD_DBG_ERROR("mtdHwSerdesWrite: Serdes 28G X2 code not selected.\n");
        return MTD_FAIL;
#endif  /* MTD_CE_SERDES28X2 */
    }
    else if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
#if MTD_CE_SERDES32X1X4
            MCESD_U8 lane;
            lane = (MTD_IS_X3680_DEVICE(devPtr)) ? (mdioPort % 4) : 0;
    
            MTD_ATTEMPT(API_N5C32GP5X4_WriteReg(pSerdesDev, lane, reg, value));
#else
            MTD_DBG_ERROR("mtdHwSerdesRead: Serdes 32G X1/X4 code not selected.\n");
            return MTD_FAIL;
#endif  /* MTD_CE_SERDES32X1X4 */
    }     
    else
    {
        MTD_DBG_ERROR("mtdHwSerdesWrite: Device doesn't support any of the COMPHY Serdes.\n");
        return MTD_FAIL;
    }

    return MTD_OK;
}

/******************************************************************************
 API Functions for specific devices and Serdes
*******************************************************************************/

#if MTD_CE_SERDES28X2
/******************************************************************************
 MTD_STATUS mtdSerdes28GX2GetFirmwareVersion
    Calls serdes API to read the firmware version
*******************************************************************************/
MTD_STATUS mtdSerdes28GX2GetFirmwareVersion
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort, 
    OUT MTD_U8 *major, 
    OUT MTD_U8 *minor, 
    OUT MTD_U8 *patch, 
    OUT MTD_U8 *build
)
{
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MCESD_DEV_PTR pSerdesDev;

        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

        MTD_ATTEMPT(API_C28GP4X2_GetFirmwareRev(pSerdesDev,major,minor,patch,build));
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes28GX2GetFirmwareVersion: Device doesn't support 28GX2 Serdes\n");
        return MTD_FAIL;
    }


    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes28GX2EyeRawData
    Calls serdes API to collect raw eye data
*******************************************************************************/
MTD_STATUS mtdSerdes28GX2EyeRawData
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort,
    IN MTD_U8 serdesLane,
    OUT PMTD_SERDES_28GX2_EYE_RAW pEyeRawData
)
{
    MTD_16 voltage;
    MTD_32 phase;
    MTD_16 startPt, tmp, offset = 10;
    MTD_16 phaseIdx, voltIdx;
    MTD_16 leftPhase, rightPhase, maxVoltageStep;
    S_C28GP4X2_EOM_DATA eomData;
    MCESD_DEV_PTR pSerdesDev;

    if (!pEyeRawData)
    {
        MTD_DBG_ERROR("mtdSerdes28GX2EyeRawData: pEyeRawData point is NULL.\n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdSerdes28GX2EyeRawData: Device does not support 28GX2 serdes.\n");
        return MTD_FAIL;
    }

    MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

    maxVoltageStep = MTD_EYE_MAX_VOLT_STEPS;
    leftPhase = MTD_EYE_MAX_PHASE_LEVEL;
    rightPhase = -(MTD_EYE_MAX_PHASE_LEVEL);

    MTD_ATTEMPT(API_C28GP4X2_EOMInit(pSerdesDev, serdesLane));

    phaseIdx = 0; /* initialized the array */
    for (phase = leftPhase; phase >= rightPhase; phase--)
    {
        voltIdx = 0;
        for (voltage = maxVoltageStep; voltage >= -maxVoltageStep; voltage--)
        {
            pEyeRawData->eyeRawData[phaseIdx][voltIdx] = -1;
            voltIdx++;
        }
        phaseIdx++;
    }

    /* decreasing phase moves to the right */
    startPt = 0;
    phaseIdx = 0;
    voltage = 0;
	/*KW*/
    mtdMemSet(&eomData,0,sizeof(eomData));
	/*End KW*/
    for (phase = leftPhase; phase >= rightPhase; phase--)
    {
        voltage = ((startPt + offset) > maxVoltageStep) ? maxVoltageStep : (startPt + offset); /* voltage, eye height not centered */
        while (voltage >= 0)
        {
            MTD_ATTEMPT(API_C28GP4X2_EOMMeasPoint(pSerdesDev, serdesLane, phase, (MTD_U8)voltage, &eomData));

            pEyeRawData->eyeRawData[phaseIdx][maxVoltageStep - voltage] = eomData.upperBitErrorCount;
            pEyeRawData->eyeRawData[phaseIdx][maxVoltageStep + voltage] = eomData.lowerBitErrorCount;

            if ((eomData.upperBitErrorCount == 0) && (eomData.lowerBitErrorCount == 0))
            {
                startPt = voltage;
                break;
            }
            voltage--;
        }

        /* patch zeros in between */
        if (voltage > 0)
        {
            for (tmp = voltage - 1; tmp >= 0; tmp--)
            {
                pEyeRawData->eyeRawData[phaseIdx][maxVoltageStep - tmp] = 0;
                pEyeRawData->eyeRawData[phaseIdx][maxVoltageStep + tmp] = 0;
            }
        }
        phaseIdx++;
    }

    /* fill untouched outer area with maxBit (error) */
    for (voltIdx = maxVoltageStep * 2; voltIdx >= 0; voltIdx--)
    {
        for (phaseIdx = 0; phaseIdx < leftPhase - rightPhase + 1; phaseIdx++)
        {
            if (pEyeRawData->eyeRawData[phaseIdx][voltIdx] == -1)
                pEyeRawData->eyeRawData[phaseIdx][voltIdx] = 0xFFFFFF;
        }
    }

    MTD_ATTEMPT(API_C28GP4X2_EOMFinalize(pSerdesDev, serdesLane));

    return MTD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes28GX2EyePlotStats
    Calls serdes API to plot eye data
*******************************************************************************/
MTD_STATUS mtdSerdes28GX2EyePlotStats
(
    IN MTD_DEV_PTR devPtr,
    IN PMTD_SERDES_28GX2_EYE_RAW pEyeRawData,
    OUT MTD_U16 *eyeWidth,
    OUT MTD_U16 *eyeHeight,
    OUT MTD_U16 *eyeArea
)
{
    MTD_U16 phaseIndex, voltageIndex;
    MTD_U16 phaseInterval, timeUnit;

    if (!pEyeRawData)
    {
        MTD_DBG_ERROR("mtdSerdes28GX2EyePlotStats: pEyeRawData point is NULL.\n");
        return MTD_FAIL;
    }

    if (!MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_DBG_ERROR("mtdSerdes28GX2EyePlotStats: Unsupported device.\n");
        return MTD_FAIL;
    }

    phaseInterval = (MTD_EYE_MAX_PHASE_LEVEL*2)+1;
    timeUnit = (MTD_EYE_MAX_VOLT_STEPS*2)+1;

    *eyeArea = *eyeWidth = *eyeHeight = 0;

    for (voltageIndex = 0; voltageIndex < timeUnit; voltageIndex++)  /* through all amplitudes */
    {
        for (phaseIndex = 0; phaseIndex < phaseInterval; phaseIndex++) /* through all phases */
        {
            if (phaseIndex+1 >= phaseInterval)
            {
                MTD_DBG_INFO("\n");
            }
            else if (voltageIndex == MTD_EYE_MAX_VOLT_STEPS+1)
            {
               MTD_DBG_INFO("-");
               if (pEyeRawData->eyeRawData[phaseIndex][voltageIndex] == 0)
               {
                   (*eyeWidth)++;
                   (*eyeArea)++;
               }
            }
            else if (phaseIndex == MTD_EYE_MAX_PHASE_LEVEL+1)
            {
               MTD_DBG_INFO("|");
               if (pEyeRawData->eyeRawData[phaseIndex][voltageIndex] == 0)
               {
                   (*eyeHeight)++;
                   (*eyeArea)++;
               }
            }
            else if (pEyeRawData->eyeRawData[phaseIndex][voltageIndex] == 0)
            {
               MTD_DBG_INFO(" ");
               (*eyeArea)++;
            }
            else if (pEyeRawData->eyeRawData[phaseIndex][voltageIndex] < 1000)
            {
               MTD_DBG_INFO(".");
            }
            else
            {
                MTD_DBG_INFO("X");
            }
        }
    }

    MTD_DBG_INFO("mtdSerdes28GX2EyePlotStats: eyeWidth:%d  eyeHeight:%d eyeArea:%d\n",
                 *eyeWidth, *eyeHeight, *eyeArea);

    return MTD_OK;
}

#endif  /* MTD_CE_SERDES28X2 */

#if MTD_CE_SERDES32X1X4
/******************************************************************************
 MTD_STATUS mtdSerdes32GX4GetFirmwareVersion
    Calls serdes API to read the firmware version
*******************************************************************************/
MTD_STATUS mtdSerdes32GX4GetFirmwareVersion
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort, 
    OUT MTD_U8 *major, 
    OUT MTD_U8 *minor, 
    OUT MTD_U8 *patch, 
    OUT MTD_U8 *build
)
{
    if (MTD_IS_X36X0_DEVICE(devPtr))
    {
        MCESD_DEV_PTR pSerdesDev;

        MTD_ATTEMPT(mtdSetSerdesPortGetDevPtr(devPtr, mdioPort, &pSerdesDev));

        MTD_ATTEMPT(API_N5C32GP5X4_GetFirmwareRev(pSerdesDev,major,minor,patch,build));
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes32GX4GetFirmwareVersion: Device doesn't support 32GX4 Serdes\n");
        return MTD_FAIL;
    }


    return MTD_OK;
}
#endif  /* MTD_CE_SERDES32X1X4 */

/******************************************************************************
 Callback functions that shared among different devices and Serdes
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdesWait
(
    MCESD_DEV_PTR pSerdesDev,
    MCESD_U32 ms
)
{
    MTD_DEV_PTR devPtr;

    devPtr = pSerdesDev->appData;

    MCESD_ATTEMPT(mtdWait(devPtr, (MTD_UINT)ms));

    return MCESD_OK;
}

/******************************************************************************
 Callback functions for specific devices and Serdes
*******************************************************************************/

#if MTD_CE_SERDES28X2
/******************************************************************************
 MTD_STATUS mtdSerdes28X2MDIORead
    Internally used for the 28G X2 SerDes MDIO read
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes28X2MDIORead
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U32 apbAddress,
    OUT MCESD_U32 *value
)
{
    MTD_U16 tmp;
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.c28gX2;
        mdioPort = serdesCtrlPtr->serdesPort;
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes28X2MDIORead: Device doesn't support Serdes 28G X2.\n");
        return MCESD_FAIL;
    }

    apbAddress >>= 2; /* Lowest 2 bits not used*/
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_CTRL, MTD_COMPHY_READ_CMD));
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_ADDR_REG, (MTD_U16)apbAddress));

    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioRead(devPtr, mdioPort, MTD_COMPHY_DATA_MSB, &tmp));
    *value = (tmp << 16) & 0xFFFF0000;
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioRead(devPtr, mdioPort, MTD_COMPHY_DATA_LSB, &tmp));
    *value |= tmp & 0xFFFF;

    return MCESD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes28X2MDIOWrite
    Internally used for the 28G X2 SerDes MDIO write
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes28X2MDIOWrite
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U32 apbAddress,
    IN MCESD_U32 value
)
{
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.c28gX2;
        mdioPort = serdesCtrlPtr->serdesPort;
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes28X2MDIOWrite: Device doesn't support Serdes 28G X2.\n");
        return MCESD_FAIL;
    }

    apbAddress >>= 2; /* Lowest 2 bits not used*/
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_CTRL, MTD_COMPHY_WRITE_CMD));
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_ADDR_REG, (MTD_U16)apbAddress));

    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_DATA_MSB, (value & 0xFFFF0000) >> 16));
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_DATA_LSB, value & 0xFFFF));

    return MCESD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes28X2SetPinCfg
    Internally used to set Pin Config for 28G X2 SerDes
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes28X2SetPinCfg
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U16 pin,
    IN MCESD_U32 pinValue
)
{
    MTD_U16 i;
    MTD_U16 mask = 0;
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_U16 ctrlReg = 0xFFFF;
        MTD_U8 overWriteBit = 16;
        MTD_U8 ctrlFiledOffset = 0;
        MTD_U8 ctrlFieldLength = 1;
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.c28gX2;
        mdioPort = serdesCtrlPtr->serdesPort;

        switch (pin)
        {
            case C28GP4X2_PIN_RESET:
                ctrlReg = 0xF08B;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_ISOLATION_ENB:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_ISOLATION_ENB is tied to high and cannot be modified.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_BG_RDY:
                ctrlReg = 0xF089;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                break;

            case C28GP4X2_PIN_SIF_SEL:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_SIF_SEL is tied to low and cannot be modified.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_MCU_CLK:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_MCU_CLK is set to 312Mhz and cannot be modified.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_DIRECT_ACCESS_EN:
                ctrlReg = 0xF08B;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                break;

            case C28GP4X2_PIN_PHY_MODE:
                ctrlReg = 0xF089;
                overWriteBit = 5;
                ctrlFiledOffset = 2;
                ctrlFieldLength = 3;
                break;

            case C28GP4X2_PIN_REFCLK_SEL:
                ctrlReg = 0xF089;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                break;

            case C28GP4X2_PIN_REF_FREF_SEL:
                ctrlReg = 0xF089;
                overWriteBit = 11;
                ctrlFiledOffset = 6;
                ctrlFieldLength = 5;
                break;

            case C28GP4X2_PIN_PHY_GEN_TX0:
                ctrlReg = 0xF081;
                overWriteBit = 11;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                break;

            case C28GP4X2_PIN_PHY_GEN_TX1:
                ctrlReg = 0xF081;
                overWriteBit = 11;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PHY_GEN_RX0:
                ctrlReg = 0xF081;
                overWriteBit = 11;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                break;

            case C28GP4X2_PIN_PHY_GEN_RX1:
                ctrlReg = 0xF081;
                overWriteBit = 11;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_DFE_EN0:
                ctrlReg = 0xF083;
                overWriteBit = 11;
                ctrlFiledOffset = 10;
                break;

            case C28GP4X2_PIN_DFE_EN1:
                ctrlReg = 0xF083;
                overWriteBit = 11;
                ctrlFiledOffset = 10;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_PLL0:
                ctrlReg = 0xF080;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                break;

            case C28GP4X2_PIN_PU_PLL1:
                ctrlReg = 0xF080;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_RX0:
                ctrlReg = 0xF080;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                break;

            case C28GP4X2_PIN_PU_RX1:
                ctrlReg = 0xF080;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_TX0:
                ctrlReg = 0xF080;
                overWriteBit = 11;
                ctrlFiledOffset = 10;
                break;

            case C28GP4X2_PIN_PU_TX1:
                ctrlReg = 0xF080;
                overWriteBit = 11;
                ctrlFiledOffset = 10;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_IDLE0:
                ctrlReg = 0xF082;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_TX_IDLE1:
                ctrlReg = 0xF082;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_IVREF:
                ctrlReg = 0xF080;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_RX_TRAIN_ENABLE0:
                ctrlReg = 0xF083;
                overWriteBit = 7;
                ctrlFiledOffset = 6;
                break;

            case C28GP4X2_PIN_RX_TRAIN_ENABLE1:
                ctrlReg = 0xF083;
                overWriteBit = 7;
                ctrlFiledOffset = 6;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_TRAIN_COMPLETE0:
                ctrlReg = 0xF083;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                break;

            case C28GP4X2_PIN_RX_TRAIN_COMPLETE1:
                ctrlReg = 0xF083;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_TRAIN_FAILED0:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_RX_TRAIN_FAILED0 is read-only latched status.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_RX_TRAIN_FAILED1:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_RX_TRAIN_FAILED1 is read-only latched status.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_TX_TRAIN_ENABLE0:
                ctrlReg = 0xF084;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                break;

            case C28GP4X2_PIN_TX_TRAIN_ENABLE1:
                ctrlReg = 0xF084;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_TRAIN_COMPLETE0:
                ctrlReg = 0xF084;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_TX_TRAIN_COMPLETE1:
                ctrlReg = 0xF084;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_TRAIN_FAILED0:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_TX_TRAIN_FAILED0 is read-only latched status.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_TX_TRAIN_FAILED1:
                MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: PIN_TX_TRAIN_FAILED1 is read-only latched status.\n");
                return MCESD_FAIL;

            case C28GP4X2_PIN_SQ_DETECTED_LPF0:
                ctrlReg = 0xF078;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                break;

            case C28GP4X2_PIN_SQ_DETECTED_LPF1:
                ctrlReg = 0xF078;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_INIT0:
                ctrlReg = 0xF080;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                break;

            case C28GP4X2_PIN_RX_INIT1:
                ctrlReg = 0xF080;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_INIT_DONE0:
                ctrlReg = 0xF078;
                overWriteBit = 7;
                ctrlFiledOffset = 6;
                break;

            case C28GP4X2_PIN_RX_INIT_DONE1:
                ctrlReg = 0xF078;
                overWriteBit = 7;
                ctrlFiledOffset = 6;
                mdioPort += 1;
                break;
        }

        for (i = 0; i < ctrlFieldLength; i++)
        {
            mask |= (0x1 << i);
        }

        /* only lower 16-bit is used in pinValue. Need update to handle upper 16-bit if needed */
        pinValue = pinValue & mask;

        if (overWriteBit < 16)
        {
            MCESD_ATTEMPT((MCESD_STATUS)mtdHwSetPhyRegField(devPtr, mdioPort, MTD_H_UNIT, ctrlReg, overWriteBit, 1, 0x1));
        }

        MCESD_ATTEMPT((MCESD_STATUS)mtdHwSetPhyRegField(devPtr, mdioPort, MTD_H_UNIT, ctrlReg, ctrlFiledOffset, ctrlFieldLength, (MTD_U16)pinValue));
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes28X2SetPinCfg: Device doesn't support Serdes 28G X2.\n");
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes28X2GetPinCfg
    Internally used to get Pin Config for 28G X2 SerDes
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes28X2GetPinCfg
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U16 pin,
    OUT MCESD_U32 *pinValue
)
{
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */
    MTD_U16 pinValue16Bit;   

    *pinValue = 0;

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X35X0_E2540_DEVICE(devPtr))
    {
        MTD_U16 ctrlReg = 0xFFFF;
        MTD_U8 ctrlFiledOffset = 0;
        MTD_U8 ctrlFieldLength = 1;
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.c28gX2;
        mdioPort = serdesCtrlPtr->serdesPort;

        switch (pin)
        {
            case C28GP4X2_PIN_RESET:
                ctrlReg = 0xF08B;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_ISOLATION_ENB:
                /* PIN_ISOLATION_ENB is tied to high and cannot be modified */
                *pinValue = 0x1;
                return MCESD_OK;

            case C28GP4X2_PIN_BG_RDY:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 14;
                break;

            case C28GP4X2_PIN_SIF_SEL:
                /* PIN_SIF_SEL is tied to low and cannot be modified */
                *pinValue = 0x0;
                return MCESD_OK;

            case C28GP4X2_PIN_MCU_CLK:
                /* PIN_MCU_CLK is set to 312Mhz and cannot be modified */
                *pinValue = 0x0;
                return MCESD_OK;

            case C28GP4X2_PIN_DIRECT_ACCESS_EN:
                ctrlReg = 0xF08B;
                ctrlFiledOffset = 4;
                break;

            case C28GP4X2_PIN_PHY_MODE:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 2;
                ctrlFieldLength = 3;
                break;

            case C28GP4X2_PIN_REFCLK_SEL:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 12;
                break;

            case C28GP4X2_PIN_REF_FREF_SEL:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 6;
                ctrlFieldLength = 5;
                break;

            case C28GP4X2_PIN_PHY_GEN_TX0:
                ctrlReg = 0xF081;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                break;

            case C28GP4X2_PIN_PHY_GEN_TX1:
                ctrlReg = 0xF081;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PHY_GEN_RX0:
                ctrlReg = 0xF081;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                break;

            case C28GP4X2_PIN_PHY_GEN_RX1:
                ctrlReg = 0xF081;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_DFE_EN0:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 10;
                break;

            case C28GP4X2_PIN_DFE_EN1:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 10;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_PLL0:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 14;
                break;

            case C28GP4X2_PIN_PU_PLL1:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 14;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_RX0:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 12;
                break;

            case C28GP4X2_PIN_PU_RX1:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 12;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_TX0:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 10;
                break;

            case C28GP4X2_PIN_PU_TX1:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 10;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_IDLE0:
                ctrlReg = 0xF082;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_TX_IDLE1:
                ctrlReg = 0xF082;
                ctrlFiledOffset = 8;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_PU_IVREF:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_RX_TRAIN_ENABLE0:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 6;
                break;

            case C28GP4X2_PIN_RX_TRAIN_ENABLE1:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 6;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_TRAIN_COMPLETE0:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 4;
                break;

            case C28GP4X2_PIN_RX_TRAIN_COMPLETE1:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_TRAIN_FAILED0:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 1;
                break;

            case C28GP4X2_PIN_RX_TRAIN_FAILED1:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 1;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_TRAIN_ENABLE0:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 14;
                break;

            case C28GP4X2_PIN_TX_TRAIN_ENABLE1:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 14;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_TRAIN_COMPLETE0:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 8;
                break;

            case C28GP4X2_PIN_TX_TRAIN_COMPLETE1:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 8;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_TX_TRAIN_FAILED0:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 5;
                break;

            case C28GP4X2_PIN_TX_TRAIN_FAILED1:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 5;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_SQ_DETECTED_LPF0:
                ctrlReg = 0xF078;
                ctrlFiledOffset = 12;
                break;

            case C28GP4X2_PIN_SQ_DETECTED_LPF1:
                ctrlReg = 0xF078;
                ctrlFiledOffset = 12;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_INIT0:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 4;
                break;

            case C28GP4X2_PIN_RX_INIT1:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 4;
                mdioPort += 1;
                break;

            case C28GP4X2_PIN_RX_INIT_DONE0:
                ctrlReg = 0xF078;
                ctrlFiledOffset = 6;
                break;

            case C28GP4X2_PIN_RX_INIT_DONE1:
                ctrlReg = 0xF078;
                ctrlFiledOffset = 6;
                mdioPort += 1;
                break;
        }

        MCESD_ATTEMPT((MCESD_STATUS)mtdHwGetPhyRegField(devPtr, mdioPort, MTD_H_UNIT, ctrlReg, ctrlFiledOffset, ctrlFieldLength, &pinValue16Bit));
        *pinValue |= pinValue16Bit & 0xFFFF;
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes28X2GetPinCfg: Device doesn't support Serdes 28G X2.\n");
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

#endif  /* MTD_CE_SERDES28X2 */

#if MTD_CE_SERDES32X1X4
/******************************************************************************
 MTD_STATUS mtdSerdes32X4MDIORead
    Internally used for the 32G X4 SerDes MDIO read
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes32X4MDIORead
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U32 apbAddress,
    OUT MCESD_U32 *value
)
{
    MTD_U16 tmp;
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.n5c32gX1X4;
        mdioPort = serdesCtrlPtr->serdesPort;
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes32X4MDIORead: Device doesn't support Serdes 32G X1/X4.\n");
        return MCESD_FAIL;
    }

    apbAddress >>= 2; /* Lowest 2 bits not used */
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_CTRL, MTD_COMPHY_READ_CMD));
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_ADDR_REG, (MTD_U16)apbAddress));

    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioRead(devPtr, mdioPort, MTD_COMPHY_DATA_MSB, &tmp));
    *value = (tmp << 16) & 0xFFFF0000;
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioRead(devPtr, mdioPort, MTD_COMPHY_DATA_LSB, &tmp));
    *value |= tmp & 0xFFFF;

    return MCESD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes32X4MDIOWrite
    Internally used for the 32G X4 SerDes MDIO write
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes32X4MDIOWrite
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U32 apbAddress,
    IN MCESD_U32 value
)
{
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.n5c32gX1X4;
        mdioPort = serdesCtrlPtr->serdesPort;
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes32X4MDIOWrite: Device doesn't support Serdes 32G X1/X4.\n");
        return MCESD_FAIL;
    }

    apbAddress >>= 2; /* Lowest 2 bits not used */
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_CTRL, MTD_COMPHY_WRITE_CMD));
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_ACCESS_ADDR_REG, (MTD_U16)apbAddress));

    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_DATA_MSB, (value & 0xFFFF0000) >> 16));
    MCESD_ATTEMPT((MCESD_STATUS)mtdHwXmdioWrite(devPtr, mdioPort, MTD_COMPHY_DATA_LSB, value & 0xFFFF));

    return MCESD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes32X4SetPinCfg
    Internally used to set Pin Config for 32G X4 SerDes
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes32X4SetPinCfg
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U16 pin,
    IN MCESD_U32 pinValue
)
{
    MTD_U16 i;
    MTD_U16 mask = 0;
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */

    devPtr = pSerdesDev->appData;
    if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
        MTD_U16 ctrlReg = 0xFFFF;
        MTD_U8 overWriteBit = 16;
        MTD_U8 ctrlFiledOffset = 0;
        MTD_U8 ctrlFieldLength = 1;
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.n5c32gX1X4;
        mdioPort = serdesCtrlPtr->serdesPort;

        switch (pin)
        {
            case N5C32GP5X4_PIN_RESET:
                ctrlReg = 0xF08B;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                break;

            case N5C32GP5X4_PIN_ISOLATION:
                MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: N5C32GP5X4_PIN_ISOLATION is tied to high and cannot be modified.\n");
                return MCESD_FAIL;

            case N5C32GP5X4_PIN_BG_RDY:
                ctrlReg = 0xF089;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                break;

            case N5C32GP5X4_PIN_SIF_SEL:
                MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: PIN_SIF_SEL is tied to low and cannot be modified.\n");
                return MCESD_FAIL;

            case N5C32GP5X4_PIN_MCU_CLK:
                MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: PIN_MCU_CLK is set to 312Mhz and cannot be modified.\n");
                return MCESD_FAIL;

            case N5C32GP5X4_PIN_DIRECTACCES:
                ctrlReg = 0xF08B;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                break;

            case N5C32GP5X4_PIN_PHY_MODE:
                ctrlReg = 0xF089;
                overWriteBit = 5;
                ctrlFiledOffset = 2;
                ctrlFieldLength = 3;
                break;
 
            case N5C32GP5X4_PIN_REFCLK_TX0:
            case N5C32GP5X4_PIN_REFCLK_TX1:
            case N5C32GP5X4_PIN_REFCLK_TX2:
            case N5C32GP5X4_PIN_REFCLK_TX3:
                ctrlReg = 0xF089;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                if (pin == N5C32GP5X4_PIN_REFCLK_TX1) mdioPort+=1;
                if (pin == N5C32GP5X4_PIN_REFCLK_TX2) mdioPort+=2;
                if (pin == N5C32GP5X4_PIN_REFCLK_TX3) mdioPort+=3;
                break;

            case N5C32GP5X4_PIN_REFFREF_TX0:
            case N5C32GP5X4_PIN_REFFREF_TX1:
            case N5C32GP5X4_PIN_REFFREF_TX2:
            case N5C32GP5X4_PIN_REFFREF_TX3:
                ctrlReg = 0xF089;
                overWriteBit = 11;
                ctrlFiledOffset = 6;
                ctrlFieldLength = 5;
                if (pin == N5C32GP5X4_PIN_REFFREF_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_REFFREF_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_REFFREF_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PHY_GEN_TX0:
            case N5C32GP5X4_PIN_PHY_GEN_TX1:
            case N5C32GP5X4_PIN_PHY_GEN_TX2:
            case N5C32GP5X4_PIN_PHY_GEN_TX3:
                ctrlReg = 0xF081;
                overWriteBit = 11;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PHY_GEN_RX0:
            case N5C32GP5X4_PIN_PHY_GEN_RX1:
            case N5C32GP5X4_PIN_PHY_GEN_RX2:
            case N5C32GP5X4_PIN_PHY_GEN_RX3:
                ctrlReg = 0xF081;
                overWriteBit = 11;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_RX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_RX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_RX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_PLL0:
            case N5C32GP5X4_PIN_PU_PLL1:
            case N5C32GP5X4_PIN_PU_PLL2:
            case N5C32GP5X4_PIN_PU_PLL3:
                ctrlReg = 0xF080;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                if (pin == N5C32GP5X4_PIN_PU_PLL1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PU_PLL2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PU_PLL3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_RX0:
            case N5C32GP5X4_PIN_PU_RX1:
            case N5C32GP5X4_PIN_PU_RX2:
            case N5C32GP5X4_PIN_PU_RX3:
                ctrlReg = 0xF080;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                if (pin == N5C32GP5X4_PIN_PU_RX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PU_RX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PU_RX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_TX0:
            case N5C32GP5X4_PIN_PU_TX1:
            case N5C32GP5X4_PIN_PU_TX2:
            case N5C32GP5X4_PIN_PU_TX3:
                ctrlReg = 0xF080;
                overWriteBit = 11;
                ctrlFiledOffset = 10;
                if (pin == N5C32GP5X4_PIN_PU_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PU_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PU_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_TX_IDLE0:
            case N5C32GP5X4_PIN_TX_IDLE1:
            case N5C32GP5X4_PIN_TX_IDLE2:
            case N5C32GP5X4_PIN_TX_IDLE3:
                ctrlReg = 0xF082;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                if (pin == N5C32GP5X4_PIN_TX_IDLE1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_TX_IDLE2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_TX_IDLE3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_IVREF:
                ctrlReg = 0xF080;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                break;

            case N5C32GP5X4_PIN_RX_TRAINEN0:
            case N5C32GP5X4_PIN_RX_TRAINEN1:
            case N5C32GP5X4_PIN_RX_TRAINEN2:
            case N5C32GP5X4_PIN_RX_TRAINEN3:
                ctrlReg = 0xF083;
                overWriteBit = 7;
                ctrlFiledOffset = 6;
                if (pin == N5C32GP5X4_PIN_RX_TRAINEN1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_TRAINEN2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_TRAINEN3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_TRAINCO0:
            case N5C32GP5X4_PIN_RX_TRAINCO1:
            case N5C32GP5X4_PIN_RX_TRAINCO2:
            case N5C32GP5X4_PIN_RX_TRAINCO3:
                ctrlReg = 0xF083;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                if (pin == N5C32GP5X4_PIN_RX_TRAINCO1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_TRAINCO2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_TRAINCO3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_TRAINFA0:
            case N5C32GP5X4_PIN_RX_TRAINFA1:
            case N5C32GP5X4_PIN_RX_TRAINFA2:
            case N5C32GP5X4_PIN_RX_TRAINFA3:
                MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: PIN_RX_TRAIN_FAILED0 is read-only latched status.\n");
                return MCESD_FAIL;

            case N5C32GP5X4_PIN_TX_TRAINEN0:
            case N5C32GP5X4_PIN_TX_TRAINEN1:
            case N5C32GP5X4_PIN_TX_TRAINEN2:
            case N5C32GP5X4_PIN_TX_TRAINEN3:
                ctrlReg = 0xF084;
                overWriteBit = 15;
                ctrlFiledOffset = 14;
                if (pin == N5C32GP5X4_PIN_TX_TRAINEN1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_TX_TRAINEN2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_TX_TRAINEN3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_TX_TRAINCO0:
            case N5C32GP5X4_PIN_TX_TRAINCO1:
            case N5C32GP5X4_PIN_TX_TRAINCO2:
            case N5C32GP5X4_PIN_TX_TRAINCO3:
                ctrlReg = 0xF084;
                overWriteBit = 9;
                ctrlFiledOffset = 8;
                if (pin == N5C32GP5X4_PIN_TX_TRAINCO1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_TX_TRAINCO2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_TX_TRAINCO3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_TX_TRAINFA0:
            case N5C32GP5X4_PIN_TX_TRAINFA1:
            case N5C32GP5X4_PIN_TX_TRAINFA2:
            case N5C32GP5X4_PIN_TX_TRAINFA3:
                MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: PIN_TX_TRAIN_FAILED0 is read-only latched status.\n");
                return MCESD_FAIL;

            case N5C32GP5X4_PIN_SQ_DET_LPF0:
            case N5C32GP5X4_PIN_SQ_DET_LPF1:
            case N5C32GP5X4_PIN_SQ_DET_LPF2:
            case N5C32GP5X4_PIN_SQ_DET_LPF3:
                ctrlReg = 0xF078;
                overWriteBit = 13;
                ctrlFiledOffset = 12;
                if (pin == N5C32GP5X4_PIN_SQ_DET_LPF1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_SQ_DET_LPF2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_SQ_DET_LPF3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_INIT0:
            case N5C32GP5X4_PIN_RX_INIT1:
            case N5C32GP5X4_PIN_RX_INIT2:
            case N5C32GP5X4_PIN_RX_INIT3:
                ctrlReg = 0xF080;
                overWriteBit = 5;
                ctrlFiledOffset = 4;
                if (pin == N5C32GP5X4_PIN_RX_INIT1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_INIT2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_INIT3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_INITDON0:
            case N5C32GP5X4_PIN_RX_INITDON1:
            case N5C32GP5X4_PIN_RX_INITDON2:
            case N5C32GP5X4_PIN_RX_INITDON3:
                ctrlReg = 0xF078;
                overWriteBit = 7;
                ctrlFiledOffset = 6;
                if (pin == N5C32GP5X4_PIN_RX_INITDON1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_INITDON2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_INITDON3) mdioPort += 3;
                break;

            default:
                MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: switch default Pin:%u is not supported.\n", pin);
                return MCESD_FAIL;
        }

        for (i = 0; i < ctrlFieldLength; i++)
        {
            mask |= (0x1 << i);
        }

        /* only lower 16-bit is used in pinValue. Need update to handle upper 16-bit if needed */
        pinValue = pinValue & mask;

        if (overWriteBit < 16)
        {
            MCESD_ATTEMPT((MCESD_STATUS)mtdHwSetPhyRegField(devPtr, mdioPort, MTD_H_UNIT, ctrlReg, overWriteBit, 1, 0x1));
        }

        MCESD_ATTEMPT((MCESD_STATUS)mtdHwSetPhyRegField(devPtr, mdioPort, MTD_H_UNIT, ctrlReg, ctrlFiledOffset, ctrlFieldLength, (MTD_U16)pinValue));
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes32X4SetPinCfg: Pin:%u is not supported.\n", pin);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

/******************************************************************************
 MTD_STATUS mtdSerdes32X4GetPinCfg
    Internally used to get Pin Config for 32G X4 SerDes
*******************************************************************************/
MTD_STATIC MCESD_STATUS mtdSerdes32X4GetPinCfg
(
    IN MCESD_DEV_PTR pSerdesDev,
    IN MCESD_U16 pin,
    OUT MCESD_U32 *pinValue
)
{
    MTD_DEV_PTR devPtr;
    MTD_U16 mdioPort;   /* The mapped port on device to control the Serdes */
    MTD_U16 pinValue16Bit;   

    *pinValue = 0;
    
    devPtr = pSerdesDev->appData;

    if (MTD_IS_X36X0_BASE(devPtr->deviceId))
    {
        MTD_U16 ctrlReg = 0xFFFF;
        MTD_U8 ctrlFiledOffset = 0;
        MTD_U8 ctrlFieldLength = 1;
        PMTD_SERDES_CTRL serdesCtrlPtr;

        serdesCtrlPtr = &devPtr->serdesCtrl.n5c32gX1X4;
        mdioPort = serdesCtrlPtr->serdesPort;

        switch (pin)
        {
            case N5C32GP5X4_PIN_RESET:
                ctrlReg = 0xF08B;
                ctrlFiledOffset = 8;
                break;

            case N5C32GP5X4_PIN_ISOLATION:
                /* PIN_ISOLATION_ENB is tied to high and cannot be modified */
                *pinValue = 0x1;
                return MCESD_OK;

            case N5C32GP5X4_PIN_BG_RDY:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 14;
                break;

            case N5C32GP5X4_PIN_SIF_SEL:
                /* PIN_SIF_SEL is tied to low and cannot be modified */
                *pinValue = 0x0;
                return MCESD_OK;

            case N5C32GP5X4_PIN_MCU_CLK:
                /* PIN_MCU_CLK is set to 312Mhz and cannot be modified */
                *pinValue = 0x0;
                return MCESD_OK;

            case N5C32GP5X4_PIN_DIRECTACCES:
                ctrlReg = 0xF08B;
                ctrlFiledOffset = 4;
                break;

            case N5C32GP5X4_PIN_PHY_MODE:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 2;
                ctrlFieldLength = 3;
                break;

            case N5C32GP5X4_PIN_REFCLK_TX0:
            case N5C32GP5X4_PIN_REFCLK_TX1:
            case N5C32GP5X4_PIN_REFCLK_TX2:
            case N5C32GP5X4_PIN_REFCLK_TX3:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 12;
                if (pin == N5C32GP5X4_PIN_REFCLK_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_REFCLK_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_REFCLK_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_REFFREF_TX0:
            case N5C32GP5X4_PIN_REFFREF_TX1:
            case N5C32GP5X4_PIN_REFFREF_TX2:
            case N5C32GP5X4_PIN_REFFREF_TX3:
                ctrlReg = 0xF089;
                ctrlFiledOffset = 6;
                ctrlFieldLength = 5;
                if (pin == N5C32GP5X4_PIN_REFFREF_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_REFFREF_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_REFFREF_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PHY_GEN_TX0:
            case N5C32GP5X4_PIN_PHY_GEN_TX1:
            case N5C32GP5X4_PIN_PHY_GEN_TX2:
            case N5C32GP5X4_PIN_PHY_GEN_TX3:
                ctrlReg = 0xF081;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PHY_GEN_RX0:
            case N5C32GP5X4_PIN_PHY_GEN_RX1:
            case N5C32GP5X4_PIN_PHY_GEN_RX2:
            case N5C32GP5X4_PIN_PHY_GEN_RX3:
                ctrlReg = 0xF081;
                ctrlFiledOffset = 7;
                ctrlFieldLength = 4;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_RX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_RX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PHY_GEN_RX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_PLL0:
            case N5C32GP5X4_PIN_PU_PLL1:
            case N5C32GP5X4_PIN_PU_PLL2:
            case N5C32GP5X4_PIN_PU_PLL3:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 14;
                if (pin == N5C32GP5X4_PIN_PU_PLL1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PU_PLL2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PU_PLL3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_RX0:
            case N5C32GP5X4_PIN_PU_RX1:
            case N5C32GP5X4_PIN_PU_RX2:
            case N5C32GP5X4_PIN_PU_RX3:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 12;
                if (pin == N5C32GP5X4_PIN_PU_RX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PU_RX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PU_RX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_TX0:
            case N5C32GP5X4_PIN_PU_TX1:
            case N5C32GP5X4_PIN_PU_TX2:
            case N5C32GP5X4_PIN_PU_TX3:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 10;
                if (pin == N5C32GP5X4_PIN_PU_TX1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_PU_TX2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_PU_TX3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_TX_IDLE0:
            case N5C32GP5X4_PIN_TX_IDLE1:
            case N5C32GP5X4_PIN_TX_IDLE2:
            case N5C32GP5X4_PIN_TX_IDLE3:
                ctrlReg = 0xF082;
                ctrlFiledOffset = 8;
                if (pin == N5C32GP5X4_PIN_TX_IDLE1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_TX_IDLE2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_TX_IDLE3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_PU_IVREF:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 8;
                break;

            case N5C32GP5X4_PIN_RX_TRAINEN0:
            case N5C32GP5X4_PIN_RX_TRAINEN1:
            case N5C32GP5X4_PIN_RX_TRAINEN2:
            case N5C32GP5X4_PIN_RX_TRAINEN3:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 6;
                if (pin == N5C32GP5X4_PIN_RX_TRAINEN1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_TRAINEN2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_TRAINEN3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_TRAINCO0:
            case N5C32GP5X4_PIN_RX_TRAINCO1:
            case N5C32GP5X4_PIN_RX_TRAINCO2:
            case N5C32GP5X4_PIN_RX_TRAINCO3:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 4;
                if (pin == N5C32GP5X4_PIN_RX_TRAINCO1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_TRAINCO2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_TRAINCO3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_TRAINFA0:
            case N5C32GP5X4_PIN_RX_TRAINFA1:
            case N5C32GP5X4_PIN_RX_TRAINFA2:
            case N5C32GP5X4_PIN_RX_TRAINFA3:
                ctrlReg = 0xF083;
                ctrlFiledOffset = 1;
                break;

            case N5C32GP5X4_PIN_TX_TRAINEN0:
            case N5C32GP5X4_PIN_TX_TRAINEN1:
            case N5C32GP5X4_PIN_TX_TRAINEN2:
            case N5C32GP5X4_PIN_TX_TRAINEN3:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 14;
                if (pin == N5C32GP5X4_PIN_TX_TRAINEN1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_TX_TRAINEN2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_TX_TRAINEN3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_TX_TRAINCO0:
            case N5C32GP5X4_PIN_TX_TRAINCO1:
            case N5C32GP5X4_PIN_TX_TRAINCO2:
            case N5C32GP5X4_PIN_TX_TRAINCO3:
                ctrlReg = 0xF084;
                ctrlFiledOffset = 8;
                if (pin == N5C32GP5X4_PIN_TX_TRAINCO1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_TX_TRAINCO2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_TX_TRAINCO3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_SQ_DET_LPF0:
            case N5C32GP5X4_PIN_SQ_DET_LPF1:
            case N5C32GP5X4_PIN_SQ_DET_LPF2:
            case N5C32GP5X4_PIN_SQ_DET_LPF3:
                ctrlReg = 0xF078;
                ctrlFiledOffset = 12;
                if (pin == N5C32GP5X4_PIN_SQ_DET_LPF1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_SQ_DET_LPF2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_SQ_DET_LPF3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_INIT0:
            case N5C32GP5X4_PIN_RX_INIT1:
            case N5C32GP5X4_PIN_RX_INIT2:
            case N5C32GP5X4_PIN_RX_INIT3:
                ctrlReg = 0xF080;
                ctrlFiledOffset = 4;
                if (pin == N5C32GP5X4_PIN_RX_INIT1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_INIT2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_INIT3) mdioPort += 3;
                break;

            case N5C32GP5X4_PIN_RX_INITDON0:
            case N5C32GP5X4_PIN_RX_INITDON1:
            case N5C32GP5X4_PIN_RX_INITDON2:
            case N5C32GP5X4_PIN_RX_INITDON3:
                ctrlReg = 0xF078;
                ctrlFiledOffset = 6;
                if (pin == N5C32GP5X4_PIN_RX_INITDON1) mdioPort += 1;
                if (pin == N5C32GP5X4_PIN_RX_INITDON2) mdioPort += 2;
                if (pin == N5C32GP5X4_PIN_RX_INITDON3) mdioPort += 3;
                break;

            default:
                MTD_DBG_ERROR("mtdSerdes32X4GetPinCfg: switch default Pin:%u is not supported.\n", pin);
                return MCESD_FAIL;
        }

        MCESD_ATTEMPT((MCESD_STATUS)mtdHwGetPhyRegField(devPtr, mdioPort, MTD_H_UNIT, ctrlReg, ctrlFiledOffset, ctrlFieldLength, &pinValue16Bit));
        *pinValue |= pinValue16Bit & 0xFFFF;
    }
    else
    {
        MTD_DBG_ERROR("mtdSerdes32X4GetPinCfg: Pin:%u is not supported.\n", pin);
        return MCESD_FAIL;
    }

    return MCESD_OK;
}

#endif  /* MTD_CE_SERDES32X1X4 */
#endif  /* MTD_PKG_CE_SERDES */
