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
* @file mvHwsSiliconIf.c
*
* @brief General silicon related HW Services API
*
* @version   17
********************************************************************************
*/

#ifndef  MV_HWS_FREE_RTOS
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#endif
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortCtrlApLogDefs.h>
#include <cpss/common/labServices/port/gop/port/mvHwsServiceCpuFwIf.h>
#include <cpss/common/labServices/port/gop/port/sse/mvHwsSseInvokerIf.h>
#include <cpss/common/labServices/port/gop/port/mvPortModeElements.h>

#ifndef  MV_HWS_FREE_RTOS
#define PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc._var)

#define PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_SET(_var,_val)\
    PRV_SHARED_GLOBAL_VAR_SET(commonMod.labServicesDir.mvHwsSiliconIfSrc._var,_val)

#define PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(_var)\
    PRV_NON_SHARED_GLOBAL_VAR_GET(commonMod.labServicesDir.mvHwsSiliconIfSrc._var)

#define PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_SET(_var,_val)\
    PRV_NON_SHARED_GLOBAL_VAR_SET(commonMod.labServicesDir.mvHwsSiliconIfSrc._var,_val)

#else
MV_HWS_REG_ADDR_FIELD_STC *mvHwsRegDb[LAST_UNIT] = {NULL};
#define PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(_var) _var


#define PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_SET(_var,_val)\
    _var= _val


#define PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(_var)\
    _var

#define PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_SET(_var,_val)\
    _var= _val

#endif


#define CALC_MASK_MAC(fieldLen, fieldOffset, mask)     \
            if(((fieldLen) + (fieldOffset)) >= 32)     \
             (mask) = (GT_U32)(0 - (1<< (fieldOffset)));    \
            else                                   \
          (mask) = (((1<<((fieldLen) + (fieldOffset)))) - (1 << (fieldOffset)))

#ifdef MV_HWS_REDUCED_BUILD
MV_OS_EXACT_DELAY_FUNC          hwsOsExactDelayPtr = NULL;
MV_OS_MICRO_DELAY_FUNC          hwsOsMicroDelayPtr = NULL;
MV_OS_TIME_WK_AFTER_FUNC        hwsOsTimerWkFuncPtr = NULL;
MV_OS_MEM_SET_FUNC              hwsOsMemSetFuncPtr = NULL;
MV_OS_FREE_FUNC                 hwsOsFreeFuncPtr = NULL;
MV_OS_MALLOC_FUNC               hwsOsMallocFuncPtr = NULL;
MV_SERDES_REG_ACCESS_SET        hwsSerdesRegSetFuncPtr = NULL;
MV_SERDES_REG_ACCESS_GET        hwsSerdesRegGetFuncPtr = NULL;
MV_DEVICE_TEMPERATURE_GET       hwsDeviceTemperatureGetFuncPtr = NULL;
MV_SERVER_REG_ACCESS_SET        hwsServerRegSetFuncPtr = NULL;
MV_SERVER_REG_ACCESS_GET        hwsServerRegGetFuncPtr = NULL;
MV_OS_MEM_COPY_FUNC             hwsOsMemCopyFuncPtr = NULL;
MV_OS_STR_CAT_FUNC              hwsOsStrCatFuncPtr = NULL;
MV_SERVER_REG_FIELD_ACCESS_SET  hwsServerRegFieldSetFuncPtr = NULL;
MV_SERVER_REG_FIELD_ACCESS_GET  hwsServerRegFieldGetFuncPtr = NULL;
MV_TIMER_GET                    hwsTimerGetFuncPtr = NULL;
MV_REG_ACCESS_SET               hwsRegisterSetFuncPtr = NULL;
MV_REG_ACCESS_GET               hwsRegisterGetFuncPtr = NULL;
#if defined (AC5P_DEV_SUPPORT) || defined (HARRIER_DEV_SUPPORT)
MV_SPIFLASH_ACCESS_SET          hwsSFlashSetFuncPtr = NULL;
MV_SPIFLASH_ACCESS_GET          hwsSFlashGetFuncPtr = NULL;
#endif
#endif

#if defined(BC2_DEV_SUPPORT)
/* Pointer to a function which returns the ILKN registers DB */
MV_SIL_ILKN_REG_DB_GET hwsIlknRegDbGetFuncPtr = NULL;
#endif

#ifdef MV_HWS_REDUCED_BUILD
/* Contains related data for specific device */
HWS_DEVICE_INFO hwsDeviceSpecInfo[HWS_MAX_DEVICE_NUM];
#endif

#ifdef MV_HWS_FREE_RTOS
/* store base address and unit index per unit per device type */
static HWS_UNIT_INFO   hwsDeviceSpecUnitInfo[1][LAST_UNIT];
/* device specific functions pointers */
static MV_HWS_DEV_FUNC_PTRS hwsDevFunc[1];
#endif /*MV_HWS_FREE_RTOS*/

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
GT_STATUS genInterlakenRegSet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 data, GT_U32 mask);
GT_STATUS genInterlakenRegGet(GT_U8 devNum, GT_U32 portGroup, GT_U32 address, GT_U32 *data, GT_U32 mask);
#endif
/* HWS log message buffer size */
#ifdef  MV_HWS_FREE_RTOS
#define HWS_LOG_MSG_BUFFER_SIZE  MV_FW_HWS_LOG_STRING_SIZE
#else
#define HWS_LOG_MSG_BUFFER_SIZE  MV_FW_HWS_LOG_STRING_SIZE * 10
#endif

#ifdef FALCON_DEV_SUPPORT
#define D2D_EAGLE_ADDR                  0xd7f0000
#define D2D_RAVEN_ADDR                  0x680000
#define MAC_100_ADDR                    0x444000
#define MSDB_CHANNEL_TX_CONTROL_ADDR    0x508000
#define TSU_INTERRUPT_CAUSE_ADDR        0x405000
#define MTIP_GLOBAL_PMA_CONTROL_ADDR    0x478000
#define PAU_CONTROL_ADDR                0x406000
#define PCS100_UNITS_CONTROL_ADDR       0x464000
#define PCS50_UNITS_CONTROL_ADDR        0x465000
#define DFX_CLIENT_UNITS_ADDR           0x700000
#define LPCS_UNITS_PORT_CONTROL_ADDR    0x474000
#define PORTS_IN_RAVEN                  8 /*???*/
#define D2D_OFFSET_ADDR                 0x80000
#define PORTS_OFFSET_ADDR               0x1000
#endif

#ifdef CHX_FAMILY
extern void hwsFalconIfClose(GT_U8 devNum);
/* PATCH for Falcon that currently uses bobcat3 general code */
/* but only in limited places needs 'Falcon awareness'       */
static GT_U32  falconAsBobcat3_numOfTiles = 0;
/* indicate the HWS that the Bobcat3 device is actually Falcon + number of tiles */
/* this function is called by CPSS (for Falcon) prior to bobcat3 initialization */
void hwsFalconAsBobcat3Set(GT_U32 numOfTiles)
{
    falconAsBobcat3_numOfTiles = numOfTiles ? numOfTiles : 1;
}
/* get indication if the Bobcat3 device is actually Falcon */
/* function actually return the number of Tiles (1,2,4)*/
GT_U32  hwsFalconAsBobcat3Check(void)
{
    return falconAsBobcat3_numOfTiles;
}

static GT_U32  aasNumOfTiles = 0;

void hwsAasNumOfTilesSet(GT_U32 numOfTiles)
{
    aasNumOfTiles = numOfTiles;
}
/* get indication if the Harrier device is actually AAS */
/* function actually return the number of Tiles (1 or 2)*/
GT_U32  hwsAasNumOfTilesGet(void)
{
    return aasNumOfTiles;
}

/* Falcon type pass CPSS to HWS */

static HWS_FALCON_DEV_TYPE_ENT hwsFalconType[HWS_MAX_DEVICE_NUM] = {HWS_FALCON_DEV_NONE_E};


/**
* @internal hwsFalconTypeSet function
* @endinternal
*
* @brief   indicate the HWS that the Falcon device is actually device
*          tof the specified type.
*
* @param[in] devNum             - device number.
* @param[in] type               - type of Falcon device.
*
*
* @return None
*/
void hwsFalconTypeSet
(
    IN GT_U8 devNum,
    IN HWS_FALCON_DEV_TYPE_ENT type
)
{
    hwsFalconType[devNum] = type;
}

/**
* @internal hwsFalconTypeGet function
* @endinternal
*
* @brief   used by HWS to determinate that the Falcon device is actually Armstrong2 device
*
* @param[in] devNum             - device number.
*
* @return Falcon device type
*/
HWS_FALCON_DEV_TYPE_ENT hwsFalconTypeGet
(
    IN GT_U8 devNum
)
{
    return hwsFalconType[devNum];
}
#endif /*#ifdef CHX_FAMILY*/

#if !defined (MV_HWS_FREE_RTOS)
/* Get Cider index for a given port when using mac port*/
static GT_STATUS mvHwsPhoenixGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    devNum   = devNum;
    portMode = portMode;

    if(portNum >= HWS_CORE_PORTS_NUM(devNum))
    {
        return /*not error for the error log */ GT_NO_SUCH;
    }

    ciderIndexPtr->expressChannelId        = portNum;
    if(portNum < MV_HWS_AC5X_GOP_PORT_CPU_0_CNS)
    {
        ciderIndexPtr->ciderUnit        = portNum / 8; /* 8 ports in 'gop' */
        ciderIndexPtr->ciderIndexInUnit = portNum % 8; /*0..7*/
    }
    else if(portNum <= MV_HWS_AC5X_GOP_PORT_CPU_1_CNS)/*48,49*/
    {
        if (mvHwsUsxModeCheck(devNum, portNum, portMode))
        {
            return GT_NO_SUCH;
        }
        ciderIndexPtr->ciderUnit        = portNum - MV_HWS_AC5X_GOP_PORT_CPU_0_CNS;
        ciderIndexPtr->ciderIndexInUnit = 0;
    }
    else /*50,51,52,53*/
    {
        if (mvHwsUsxModeCheck(devNum, portNum, portMode))
        {
            return GT_NO_SUCH;
        }
        ciderIndexPtr->ciderUnit        = 0;
        ciderIndexPtr->ciderIndexInUnit = (portNum - 50);/*0..3*/
    }

    return GT_OK;
}
#endif

GT_STATUS hwsRegisterSetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
)
{
    GT_U32 mask;
    GT_U32 regValue;

    if(hwsRegisterSetFuncPtr == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    regValue = ((fieldData << fieldOffset) & mask);

    return hwsRegisterSetFuncPtr(devNum, portGroup, regAddr, regValue, mask);
}

GT_STATUS hwsRegisterGetFieldFunc
(
    GT_U8   devNum,
    GT_U32  portGroup,
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldDataPtr
)
{
    GT_STATUS rc;
    GT_U32 mask;
    GT_U32 regValue;

    if(hwsRegisterGetFuncPtr == NULL)
    {
        return GT_NOT_IMPLEMENTED;
    }

    rc = hwsRegisterGetFuncPtr(devNum, portGroup, regAddr, &regValue, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    CALC_MASK_MAC(fieldLength, fieldOffset, mask);
    *fieldDataPtr = (GT_U32)((regValue & mask) >> fieldOffset);
    return rc;
}

/**
* @internal hwsOsLocalMicroDelay function
* @endinternal
*
* @brief   This API is used in case micro sec counter is not supported
*         It will convert micro to msec and round up in case needed
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - Port Group
* @param[in] microSec                 - Delay in micro sec
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS hwsOsLocalMicroDelay
(
    IN GT_U8  devNum,
    IN GT_U32 portGroup,
    IN GT_U32 microSec
)
{
    GT_U32 msec  = (microSec / 1000) + ((microSec % 1000) ? 1 : 0);

    hwsOsExactDelayPtr(devNum, portGroup, msec);

    return GT_OK;
}



#if defined (RAVEN_DEV_SUPPORT) || defined (HARRIER_DEV_SUPPORT) || defined (AC5P_DEV_SUPPORT) || defined (IRONMAN_DEV_SUPPORT)
/**
* @internal ravenUnitBaseAddrCalc function
* @endinternal
*
* @brief   function for HWS raven (to avoid call to cpss) to use for Calculate the base address
*         of a raven port (0 ..15), for next units :
*         mac, pcs, serdes
* @param[in] unitId                   - the unit : mac, pcs, serdes
* @param[in] unitNum                  - the raven port num (MAC
*       number)
*                                     - The address of the port in the unit
*/
static GT_U32 ravenUnitBaseAddrCalc
(
    IN  GT_U8           devNum,
    IN  MV_HWS_UNITS_ID unitId,
    IN  GT_U32          unitNum
)
{
#ifdef IRONMAN_DEV_SUPPORT
    GT_U32 relativeAddr = 0xFFFFFFFB;
    GT_U8 index, indexInUnit;
    GT_U32 anpRelativeAddrOffsetArr[]= {0x1A00000, 0x6A00000, 0x7A00000};

    switch (unitId)
    {
        case ANP_CPU_UNIT:
        case AN_CPU_UNIT:
            if(unitNum >= 48)
            {
               index = unitNum - 48;   /* CPU: 0,1,2,3,4,5,6*/

               if ( index < 4 )
               {
                   relativeAddr = 0x91E00000 + (0x100000 * index);
               }
               else
               {
                   relativeAddr = 0x91E00000 + anpRelativeAddrOffsetArr[(index % 3)] + (0x100000 * 4);
               }

               if (unitId == AN_CPU_UNIT)
               {
                   relativeAddr += 0x4000;
               }
            }
            else
            {
                return relativeAddr;
            }
            break;

        case AN_USX_UNIT:
        case ANP_USX_UNIT:
            if(unitNum < 48)
            {
                index = ((unitNum / 8) * 2) + ((unitNum % 8) / 4);   /* USX: 0-11 */
                if ((index == 0) || (index == 1) || (index == 6) || (index == 7))
                {
                    relativeAddr = 0x91E00000 + anpRelativeAddrOffsetArr[1] + (0x100000 * ((index % 4)));
                }
                else if ( (index == 2) || (index == 3) || (index == 8) || (index == 9) )
                {
                    relativeAddr = 0x91E00000 + anpRelativeAddrOffsetArr[2] + (0x100000 * ((index / 3)));
                }
                else /*4,5,10,11*/
                {
                    relativeAddr = 0x91E00000 + anpRelativeAddrOffsetArr[0] + (0x100000 * ((index % 4)));
                }

                if (unitId == AN_USX_UNIT)
                {
                    relativeAddr += 0x4000;
                }
            }
            else
            {
                return relativeAddr;
            }
            break;

       case MTI_10G_MAC_BR_UNIT:
            if(unitNum < 24)
            {
                index       = unitNum / 8;
                indexInUnit = unitNum % 8;

                relativeAddr = 0x93000000;
                if (index < 2)
                {
                    relativeAddr += 0x5000000 + (0x1000000 * index);
                }
            }
            else if(unitNum < 48)
            {
                return relativeAddr;
            }
            else if(unitNum < 52)
            {
                index = (unitNum - 48);
                indexInUnit = 0;

                relativeAddr = 0x91600000 + (0x100000 * index);
            }
            else
            {
                return relativeAddr;

            }
            relativeAddr += 0x3000 + (0x1000 * indexInUnit);
            break;

        case MTI_10G_MAC_NON_BR_UNIT:
            if(unitNum < 24)
            {
                return relativeAddr;
            }
            else if(unitNum < 48)
            {
                index       = (unitNum % 24) / 8;
                indexInUnit = unitNum % 8;

                relativeAddr = 0x93100000;
                if (index < 2)
                {
                    relativeAddr += 0x5000000 + (0x1000000 * index);
                }
            }
            else if(unitNum < 52)
            {
                return relativeAddr;
            }
            else
            {
                index       = unitNum - 52;
                indexInUnit = 0;

                relativeAddr = 0x93200000;
                if (index < 2)
                {
                    relativeAddr += 0x5000000 + (0x1000000 * index);
                }
            }
            relativeAddr += 0x3000 + (0x1000 * indexInUnit);
            break;


        case MTI_10G_GLOBAL_EXT_UNIT:
        case MTI_10G_PORT_EXT_UNIT:
            if(unitNum < 24) /* BR */
            {
                index       = unitNum / 8;
                indexInUnit = unitNum % 8;

                relativeAddr = 0x93000000;
                if (index < 2)
                {
                    relativeAddr += 0x5000000 + (0x1000000 * index);
                }
            }
            else if(unitNum < 48) /* NON-BR */
            {
                index       = (unitNum % 24) / 8;
                indexInUnit = unitNum % 8;

                relativeAddr = 0x93100000;
                if (index < 2)
                {
                    relativeAddr += 0x5000000 + (0x1000000 * index);
                }
            }
            else if(unitNum < 52) /* BR */
            {
                index       = unitNum - 48;
                indexInUnit = 0;

                relativeAddr = 0x91600000 + (0x100000 * index);
            }
            else  /* NON-BR */
            {
                index       = unitNum - 52;
                indexInUnit = 0;

                relativeAddr = 0x93200000;
                if (index < 2)
                {
                    relativeAddr += 0x5000000 + (0x1000000 * index);
                }
            }

            if ( MTI_10G_PORT_EXT_UNIT )
            {
                relativeAddr += 0x100 + (0x100 * indexInUnit);
            }
            break;

        case MTI_10G_PCS_LPCS_GENERAL_UNIT:
        case MTI_10G_PCS_LPCS_PORT_UNIT:
        case MTI_10G_PCS_BASER_UNIT:
        case MTI_10G_PCS_FCFEC_UNIT:
        case MTI_10G_PCS_EXT_UNIT:
            if(unitNum < 48)
            {
                index       = unitNum / 8;
                indexInUnit = unitNum % 8;

                relativeAddr = 0x93300000; /*ports 8-16, 32-39*/
                if ((index == 0) || (index == 3)) /*ports 0-7, 24-31*/
                {
                    relativeAddr += 0x5000000;
                }
                else if ((index == 1) || (index == 4)) /*ports 8-15, 32-39*/
                {
                    relativeAddr += 0x6000000;
                }
                /*else: ports 16-23, 40-47 - no additional offset*/

                if (unitNum > 23)
                {
                    relativeAddr += 0x100000;
                }
            }
            else
            {
                indexInUnit = 0;

                if (unitNum < 52)
                {
                    index = (unitNum - 48);

                    relativeAddr = 0x91A00000;
                    relativeAddr +=  (0x100000 * index);
                }
                else
                {
                    index = (unitNum - 52);

                    relativeAddr = 0x91A00000 + 0x1B00000;
                    if (index < 2)
                    {
                        relativeAddr += 0x5000000 + (0x1000000 * index);
                    }

                }
            }

            if (unitId == MTI_10G_PCS_LPCS_GENERAL_UNIT)
            {
                relativeAddr += 0x1000;
            }
            else if (unitId == MTI_10G_PCS_LPCS_PORT_UNIT)
            {
                relativeAddr += 0x1000 + (0x80 * indexInUnit);
            }
            else if (unitId == MTI_10G_PCS_BASER_UNIT)
            {
                relativeAddr += 0x2000 + (0x100 * indexInUnit);
            }
            else if (unitId == MTI_10G_PCS_FCFEC_UNIT)
            {
                relativeAddr += 0x3000 + (0x1000 * (indexInUnit / 4));/*SD_NUM: 0..1*/;
            }

            break;
        case SERDES_UNIT:
            if (unitNum <= 6)
            {
                relativeAddr = 0x91300000 + 0x8A00000 + ((unitNum % 2) * 0x80000) + ((unitNum / 2) * 0x100000);
            }
            else if (unitNum <= 12)
            {
                relativeAddr = 0x91300000 + (((unitNum - 7) % 2) * 0x80000) + (((unitNum - 7) / 2) * 0x100000);
            }
            else
            {
                return relativeAddr;
            }
        default:
            break;
    }
    return relativeAddr;
#else
#if defined(HARRIER_DEV_SUPPORT) || defined(AC5P_DEV_SUPPORT)
    GT_U32 uNum_tmp = unitNum;
#endif
    GT_U32 relativeAddr = 0xFFFFFFFB;
#if defined (RAVEN_DEV_SUPPORT)
    GT_U32 localPortIndex;

    if(unitNum > RAVEN_PORTS_NUM) /*we check also reduce port*/
    {
        return relativeAddr;
    }

    localPortIndex = unitNum  % 16;
    relativeAddr = 0;
#endif

#if defined AC5P_DEV_SUPPORT
    unitNum = (unitNum >= 104) ? 1: (unitNum/26);
#endif

    switch (unitId)
    {
        case D2D_RAVEN_UNIT:
             relativeAddr   = 0x00680000 + (unitNum%2) * 0x00010000; /* offset of D2D_1 relating to D2D_0 */
            break;

        case RAVEN_BASE_UNIT:
            break;

#if defined(HARRIER_DEV_SUPPORT) || defined(AC5P_DEV_SUPPORT)
        case ANP_400_UNIT:
#ifdef HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr   = 0x0D800000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#else
            relativeAddr   = 0x1F000000 + unitNum*0x400000;
#endif
            break;

        case AN_400_UNIT:
            unitNum /= 16;
            relativeAddr   = 0x0D804000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000 + 0x1000 * ((uNum_tmp/2)%8);
            break;

        case MIF_400_UNIT:
        case MIF_400_BR_UNIT:
        case MIF_400_SEG_UNIT:
#ifdef HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr   = 0x0D480000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#else
            relativeAddr   = 0x1B400000 + unitNum*0x1000000;
#endif
            break;
#endif

        case MTI_MAC400_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x00440000 + 0x00080000 * (unitNum/8) + 0x1000 * ((unitNum%8)/4); /* start address of MAC400 unit */
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D602000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000 + 0x1000 * ((uNum_tmp%16)/8);
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1B802000 + unitNum*0x1000000 + 0x1000 * ((uNum_tmp%26)/8);
#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_MAC100_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x00444000 + 0x00080000 * (unitNum/8) + 0x1000 * (unitNum%8); /* start address of MAC100 unit */
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D604000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000 + 0x1000 * ((uNum_tmp/2)%8);
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1B804000 + unitNum*0x1000000 + (((uNum_tmp%26)%4) + ((uNum_tmp%26)/4)) *0x1000;
#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_CPU_MAC_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x00518000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1A802000;
#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_CPU_PCS_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x00519000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1AA00000;
#else
            return 0xFFFFFFFB;
#endif
            break;

#ifdef AC5P_DEV_SUPPORT
        case MTI_CPU_RSFEC_UNIT:
            relativeAddr = 0x1AA01000;
            break;
#endif

        case MTI_CPU_EXT_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x0051C000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1A800000;
#else
            return 0xFFFFFFFB;
#endif
            break;

#ifdef AC5P_DEV_SUPPORT
        case MTI_CPU_SGPCS_UNIT:
            relativeAddr = 0x1AA02000;
            break;
#endif

#ifdef RAVEN_DEV_SUPPORT
        case RAVEN_MTI_CPU_MPFS_UNIT:
            relativeAddr = 0x00516000;
            break;
#endif

#ifdef RAVEN_DEV_SUPPORT
        case MTI_PCS50_UNIT:
            if(localPortIndex >= 8)
                relativeAddr = 0x00080000;
            relativeAddr += 0x00465000 + ((localPortIndex % 8) -1) * 0x1000;
            break;

        case MTI_PCS100_UNIT:
            if(localPortIndex >= 8)
                relativeAddr = 0x00080000;
            relativeAddr += 0x00464000 + (localPortIndex % 8) * 0x1000;
            break;

#elif defined (HARRIER_DEV_SUPPORT) || defined (AC5P_DEV_SUPPORT)
        case MTI_PCS50_UNIT:
        case MTI_PCS100_UNIT:
#if defined (HARRIER_DEV_SUPPORT)
            unitNum /= 16;
            relativeAddr = 0x0D902000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000 + (((uNum_tmp/4)%4)*2 + ((uNum_tmp/2)%2)) * 0x1000;
#elif defined (AC5P_DEV_SUPPORT)
            relativeAddr = 0x1F202000 + (unitNum)*0x400000 + (((uNum_tmp%26)%4) + ((uNum_tmp%26)/4)) *0x1000;
#else
            return 0xFFFFFFFB;
#endif
            break;
#endif
        case MTI_PCS200_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            if(localPortIndex >= 8)
                relativeAddr = 0x00080000;
            relativeAddr += 0x00461000;
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D901000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1F201000 + (unitNum)*0x400000;

#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_PCS400_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            if(localPortIndex >= 8)
                relativeAddr = 0x00080000;
            relativeAddr += 0x00460000; /* start address of PCS400 unit */
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D900000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1F200000 + (unitNum)*0x400000;

#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_LOW_SP_PCS_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            if(localPortIndex >= 8)
                relativeAddr = 0x00080000;
            relativeAddr += 0x00474000; /* start address of Low Speed PCS unit */
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D90B000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1F20b000 + (unitNum)*0x400000;
#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_EXT_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x00478000 + 0x80000 * ((unitNum  % 16) / 8);
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D600000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1B800000 + unitNum*0x1000000;
#else
            return 0xFFFFFFFB;
#endif
            break;

        case MTI_RSFEC_UNIT:
#ifdef RAVEN_DEV_SUPPORT
            relativeAddr = 0x00470000 + 0x80000 * ((unitNum  % 16) / 8);
#elif defined HARRIER_DEV_SUPPORT
            unitNum /= 16;
            relativeAddr = 0x0D90A000 + (unitNum%2)*0x2000000 - (unitNum/2)*0xC400000;
#elif defined AC5P_DEV_SUPPORT
            relativeAddr = 0x1F20a000 + (unitNum)*0x400000;
#else
            return 0xFFFFFFFB;
#endif
            break;
#ifdef RAVEN_DEV_SUPPORT
        case RAVEN_STATISTICS_UNIT:
            relativeAddr = 0x00450000 + 0x80000 * (unitNum / 8);
            break;

        case RAVEN_DFX_UNIT:
            relativeAddr = 0x700000;
            break;

        case RAVEN_MPFS_UNIT:
            relativeAddr = 0x406000 + 0x80000 * ((unitNum  % 16) / 8) + 0x4000 * (unitNum  % 8);
            break;

        case RAVEN_TSU_UNIT:
            relativeAddr = 0x405000 + 0x80000 * ((unitNum  % 16) / 8) + 0x4000 * (unitNum  % 8);
            break;

        case RAVEN_MSDB_UNIT:
            relativeAddr = 0x508000 + 0x1000 * (unitNum/8);
            break;
#endif
        case SERDES_UNIT:
            /* SERDES unit */
            relativeAddr = 0x00340000 + unitNum * 0x1000;
            break;
#if defined AC5P_DEV_SUPPORT
        case MIF_CPU_UNIT:
            relativeAddr   = 0x26600000;
            break;
        case MIF_USX_UNIT:
            relativeAddr   = 0x24400000 + unitNum*0x2000000;
            break;
        case ANP_USX_UNIT:
            relativeAddr   = 0x2A000000 + (((uNum_tmp%26) - 2)/4) * 0x200000 + unitNum*0x2000000;
            break;
        case ANP_CPU_UNIT:
            relativeAddr   = 0x1A600000;
            break;
#endif
        default:
            /* to prevent bug if miss some unit */
            return 0xFFFFFFFB;
    }
    /* indicate error */
    return relativeAddr;
#endif
}
#elif ((! defined  MV_HWS_REDUCED_BUILD_EXT_CM3) && (defined CHX_FAMILY)) || defined (FALCON_DEV_SUPPORT)
/**
* @internal ravenUnitBaseAddrCalc function
* @endinternal
*
* @brief   function for HWS raven (to avoid call to cpss) to use for Calculate the base address
*         of a raven port (0 ..15), for next units :
*         mac, pcs, serdes
* @param[in] unitId                   - the unit : mac, pcs, serdes
* @param[in] unitNum                  - the raven port num (MAC
*       number)
*                                     - The address of the port in the unit
*/
static GT_U32 ravenUnitBaseAddrCalc
(
    IN  GT_U8           devNum,
    IN  MV_HWS_UNITS_ID unitId,
    IN  GT_U32          unitNum
)
{
    GT_U32  baseAddr;
    HWS_UNIT_BASE_ADDR_TYPE_ENT cpssAddrType;

#ifdef FALCON_DEV_SUPPORT
    GT_U8 secondHalf = 0;
    GT_U8 raven;
    GT_U8 pipe;
    GT_U8 tile;
    if ((unitId != D2D_EAGLE_UNIT) && (unitId != D2D_RAVEN_UNIT))
    {
        raven = unitNum / 16 ;
        unitNum = unitNum % 16 ;
    }
    else
    {
        raven = (unitNum % 8) / 2 ;
    }
#else
    GT_U32  relativeAddr;
#endif

    switch (unitId)
    {
#ifdef GM_USED  /* the GM of Falcon keeps emulate of BC3 units */
        case GEMAC_UNIT:
        case XLGMAC_UNIT:
        case MMPCS_UNIT:
        case CG_UNIT:
        case CGPCS_UNIT:
        case CG_RS_FEC_UNIT:
            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_GOP_E;
            break;
#endif  /*GM_USED*/


        case PTP_UNIT:                  cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_GOP_E;               break;
        case D2D_EAGLE_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E;         break;
        case D2D_RAVEN_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E;         break;
        case RAVEN_BASE_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_BASE_RAVEN_E;        break;
        case MTI_MAC400_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC400_E;        break;
        case MTI_MAC100_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E;        break;
        case MTI_CPU_MAC_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_MAC_E;       break;
        case MTI_CPU_PCS_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_PCS_E;       break;
        case MTI_CPU_EXT_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_EXT_E;       break;
        case RAVEN_MTI_CPU_MPFS_UNIT:   cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_CPU_MPFS_E;break;
        case MTI_PCS25_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E;         break;
        case MTI_PCS50_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E;         break;
        case MTI_PCS100_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E;        break;
        case MTI_PCS200_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS200_E;        break;
        case MTI_PCS400_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E;        break;
        case MTI_LOW_SP_PCS_UNIT:       cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E;     break;
        case MTI_EXT_UNIT:              cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E;           break;
        case MTI_RSFEC_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_RSFEC_E;     break;
        case MTI_CPU_SGPCS_UNIT:        cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_GSPCS_E;     break;
        case MTI_CPU_RSFEC_UNIT:        cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_E;     break;
        case MTI_CPU_RSFEC_STATISTICS_UNIT:        cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_CPU_RSFEC_STATISTICS_E; break;
        case RAVEN_MPFS_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E;    break;
        case RAVEN_TSU_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E;         break;
        case RAVEN_MSDB_UNIT:           cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E;        break;
        case RAVEN_STATISTICS_UNIT:     cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC_STATISTICS_E;break;
        case RAVEN_DFX_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_RAVEN_DFX_E;         break;
        case SERDES_UNIT:               cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_SERDES_E;            break;
        case MTI_RSFEC_STATISTICS_UNIT:     cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_RSFEC_STATISTICS_E; break;

        case MIF_400_UNIT:
        case MIF_400_SEG_UNIT:
            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MIF_400_E;
            break;
        case MIF_USX_UNIT:              cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MIF_USX_E;           break;
        case MIF_CPU_UNIT:              cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MIF_CPU_E;           break;

        case ANP_400_UNIT:              cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_ANP_400_E;           break;
        case ANP_USX_UNIT:              cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_E;           break;
        case ANP_USX_O_UNIT:            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_ANP_USX_O_E;         break;
        case ANP_CPU_UNIT:              cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_ANP_CPU_E;           break;
        case AN_400_UNIT:               cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_AN_400_E;            break;
        case AN_USX_UNIT:               cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_AN_USX_E;            break;
        case AN_USX_O_UNIT:             cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_AN_USX_O_E;          break;
        case AN_CPU_UNIT:               cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_AN_CPU_E;            break;
        case MTI_USX_MAC_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MAC_E;       break;
        case MTI_USX_EXT_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_EXT_E;       break;
        case MTI_USX_PCS_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_E;       break;
        case MTI_USX_LPCS_UNIT:         cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_PCS_LSPCS_E; break;
        case MTI_USX_RSFEC_UNIT:        cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_E;     break;
        case MTI_USX_MULTIPLEXER_UNIT:  cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_MULTIPLEXER_E;break;
        case MTI_USX_RSFEC_STATISTICS_UNIT: cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_USX_RSFEC_STATISTICS_E; break;

        case MTI_10G_MAC_BR_UNIT:          cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_MAC_BR_E;     break;
        case MTI_10G_MAC_NON_BR_UNIT:      cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_MAC_NON_BR_E; break;
        case MTI_10G_GLOBAL_EXT_UNIT:      cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_GLOBAL_EXT_E; break;
        case MTI_10G_PORT_EXT_UNIT:        cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_PORT_EXT_E;   break;
        case MTI_10G_PCS_LPCS_GENERAL_UNIT:cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_PCS_LPCS_GENERAL_E; break;
        case MTI_10G_PCS_LPCS_PORT_UNIT:   cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_PCS_LPCS_PORT_E;break;
        case MTI_10G_PCS_BASER_UNIT:       cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_PCS_BASER_E;  break;
        case MTI_10G_PCS_FCFEC_UNIT:       cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_PCS_FCFEC_E;  break;
        case MTI_10G_PCS_EXT_UNIT:         cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE_MTI_10G_PCS_EXT_E;    break;


        default:
            cpssAddrType = HWS_UNIT_BASE_ADDR_TYPE__MUST_BE_LAST__E;
            break;
    }
#if !defined (FALCON_DEV_SUPPORT) && !defined (HARRIER_DEV_SUPPORT) && !defined (AC5P_DEV_SUPPORT)
    if(cpssAddrType != HWS_UNIT_BASE_ADDR_TYPE__MUST_BE_LAST__E)
    {
        relativeAddr = 0;
        hwsFalconAddrCalc(devNum,cpssAddrType, unitNum/*portNum*/,relativeAddr/*regAddr*/,&baseAddr);
    }
    else
    {
        baseAddr = 0x11111111;
    }
#else

    if(unitNum > 7)
        secondHalf = 1 ;
    pipe = (((raven * 2) + secondHalf) / 4 ) % 2;
    tile = raven / 4 ;

    /* eagle */
    if(cpssAddrType == HWS_UNIT_BASE_ADDR_TYPE_D2D_EAGLE_E ) /* 4 */
    {
        tile = unitNum / 8 ;
        if (unitNum % 8 > 3)
            pipe = 1;
        else
            pipe = 0;

        if (tile%2 == 0 )
            baseAddr = D2D_EAGLE_ADDR + (0x10000 * (unitNum%4)) + (0x8000000 * pipe) + 0x20000000*tile ;
        else
            baseAddr = D2D_EAGLE_ADDR + (0x30000 - (0x10000 * (unitNum%4))) + (0x8000000 * !pipe) + 0x20000000*tile;
    }

    /* raven */
    else {
            switch(cpssAddrType) {

                case HWS_UNIT_BASE_ADDR_TYPE_D2D_RAVEN_E: /* 5 */
                    raven = (unitNum % 8) / 2 ;
                    tile = unitNum / 8 ;
                    baseAddr = D2D_RAVEN_ADDR + (0x10000 * (unitNum%2));
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_MAC100_E: /* 6 */
                    baseAddr =  MAC_100_ADDR + (D2D_OFFSET_ADDR * secondHalf) + (PORTS_OFFSET_ADDR * (unitNum % PORTS_IN_RAVEN)) ;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MSDB_E: /* 9 */
                    baseAddr =  MSDB_CHANNEL_TX_CONTROL_ADDR + (PORTS_OFFSET_ADDR * secondHalf);
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_TSU_E: /* 10 */
                    baseAddr = TSU_INTERRUPT_CAUSE_ADDR + (D2D_OFFSET_ADDR * secondHalf) + (0x4000 * (unitNum % PORTS_IN_RAVEN)) ;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_EXT_E: /* 11 */
                    baseAddr = MTIP_GLOBAL_PMA_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf)  ;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_MTI_MPFS_E: /* 12 */
                    baseAddr = PAU_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf) + (0x4000 * (unitNum % PORTS_IN_RAVEN)) ;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_RAVEN_DFX_E: /* 20 */
                    baseAddr = DFX_CLIENT_UNITS_ADDR;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS25_E: /* 21 */
                    baseAddr = PCS50_UNITS_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf) +(PORTS_OFFSET_ADDR * (unitNum % PORTS_IN_RAVEN)) - PORTS_OFFSET_ADDR ;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS50_E: /* 22 */
                    baseAddr = PCS100_UNITS_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf) + (PORTS_OFFSET_ADDR * (unitNum % PORTS_IN_RAVEN));
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS100_E: /* 23 */
                    baseAddr = PCS100_UNITS_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf) +(PORTS_OFFSET_ADDR * (unitNum % PORTS_IN_RAVEN));
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS400_E: /* 25 */
                    baseAddr = LPCS_UNITS_PORT_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf) ;
                    break;

                case HWS_UNIT_BASE_ADDR_TYPE_MTI_PCS_LSPCS_E: /* 26 */
                    baseAddr = LPCS_UNITS_PORT_CONTROL_ADDR + (D2D_OFFSET_ADDR * secondHalf);
                    break;

                default:
                    return 0x11111111;

            }

            if (tile%2 == 0)
                baseAddr = baseAddr + ((raven%4) * 0x1000000) + tile*0x20000000  ;
            else
                baseAddr = baseAddr + 0x3000000 - ((raven%4) * 0x1000000) + tile*0x20000000 ;
    }

#endif
    return baseAddr;
}
#endif

/**
* @internal mvUnitExtInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit based on
*         unit index.
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] unitNum                  - unit index
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
* @param[out] localUnitNumPtr          - (pointer to) local unit number (support for Bobcat3 multi-pipe)
*                                       None
*/
GT_STATUS  mvUnitExtInfoGet
(
    IN  GT_U8           devNum,
    IN  MV_HWS_UNITS_ID unitId,
    IN  GT_U32          unitNum,
    OUT GT_U32          *baseAddr,
    OUT GT_U32          *unitIndex,
    OUT GT_U32          *localUnitNumPtr
)
{
    if ((baseAddr == NULL) || (unitIndex == NULL) || (localUnitNumPtr == NULL))
    {
        return GT_BAD_PTR;
    }
    if (unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_FREE_RTOS)
    *baseAddr = PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[hwsDeviceSpecInfo[devNum].devType][unitId].baseAddr;
    *unitIndex = PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[hwsDeviceSpecInfo[devNum].devType][unitId].regOffset;
#else
    *baseAddr = hwsDeviceSpecUnitInfo[0][unitId].baseAddr;
    *unitIndex = hwsDeviceSpecUnitInfo[0][unitId].regOffset;
#endif /*MV_HWS_FREE_RTOS*/

    if(localUnitNumPtr)
    {
        *localUnitNumPtr = unitNum;
    }
#ifndef IRONMAN_DEV_SUPPORT
    if ( BobcatA0 == HWS_DEV_SILICON_TYPE(devNum) || BobK == HWS_DEV_SILICON_TYPE(devNum) )
    {
        /* fixed units base address */
        switch (unitId)
        {
        case GEMAC_UNIT:
        case XLGMAC_UNIT:
        case MMPCS_UNIT:
        case XPCS_UNIT:
        case PTP_UNIT:
            if (unitNum >= 56)
            {
                /* Unit calculation is: Base + 0x200000 + 0x1000*(K-56): where K (56-71) represents Port
                   in addition, since in register access level we also add INDEX*K (INDEX here is 0x1000),
                   we need to remove here INDEX*56 so total calculation will be
                   INDEX*K - INDEX*56 = INDEX*(K-56) */
                *baseAddr += (0x200000 - ((*unitIndex) * 56));
            }
            break;
        case SERDES_UNIT:
            if ((hwsDeviceSpecInfo[devNum].devType == BobK) && (unitNum >= 20))
            {
                /* Unit calc is: Base + 0x40000 + 0x1000*S: where S (20-35) represents Serdes */
                *baseAddr += 0x40000;
            }
            break;
        default:
            break;
        }
    }
#endif
/* this strange !defined(MV_HWS_REDUCED_BUILD_EXT_CM3) I added because noone of xxx_DEV_SUPPORT
    defined in simulation
*/
#if defined(CHX_FAMILY) || defined(RAVEN_DEV_SUPPORT) || defined(FALCON_DEV_SUPPORT) || defined(HARRIER_DEV_SUPPORT) || defined(AC5P_DEV_SUPPORT) || defined(IRONMAN_DEV_SUPPORT)
/*#ifndef MV_HWS_REDUCED_BUILD_EXT_CM3*/
    if (HWS_DEV_SIP_6_CHECK_MAC(devNum))
    {
        *unitIndex = 0;
        *localUnitNumPtr = 0;/* the caller uses this parameter for additional
            address calculations ... we can not allow them to do it. because
            formula can be very complex inside :
            hwsAldrin2GopAddrCalc(...) / hwsAldrin2SerdesAddrCalc(...) */
        *baseAddr =  ravenUnitBaseAddrCalc(devNum, unitId, unitNum);
        if(*baseAddr == MV_HWS_SW_PTR_ENTRY_UNUSED)
        {
            return GT_NOT_INITIALIZED;
        }
        else
        {
            return GT_OK;
        }
    }
/*#endif  #ifndef MV_HWS_REDUCED_BUILD_EXT_CM3  in comment for raven CM3*/
#endif /* #ifdef CHX_FAMILY */
#if defined BC3_DEV_SUPPORT || defined(CHX_FAMILY)
    if (hwsDeviceSpecInfo[devNum].devType == Bobcat3)
    {
        GT_U32  offset=0;
        /* fixed units base address */
        switch (unitId)
        {
            case GEMAC_UNIT:
            case XLGMAC_UNIT:
            case MMPCS_UNIT:
                /* baseAddr + (0x1000 * a) + 0x400000 * t: where a (36-36) represents CPU Port, where t (0-1) represents Pipe
                   baseAddr + (0x1000 * a) + 0x400000 * t: where a (0-35)  represents Port_num, where t (0-1) represents Pipe */
            case CG_UNIT:
            case CGPCS_UNIT:
            case CG_RS_FEC_UNIT:
            case PTP_UNIT:
                /* baseAddr + 0x1000 * a + 0x400000 * t: where a (0-35 in steps of 4) represents Port, where t (0-1) represents Pipe */
                offset = 0x400000;
                break;
            case SERDES_UNIT:
                /* 0x13000000 + 0x1000 * a + 0x80000 * t: where a (0-36) represents SERDES_Num, where t (0-1) represents Pipe */
                offset = 0x80000;
                break;
            default:
                /* to prevent bug if miss some unit */
                                return GT_NOT_INITIALIZED;
        }

        if (unitNum <= 71)
        {
            *baseAddr |= (unitNum / 36) * offset;
            *localUnitNumPtr = (unitNum % 36);
        }
        else if ((unitNum == 72) || (unitNum == 73))
        {
            *baseAddr |= (unitNum % 2) * offset;
            *localUnitNumPtr = 36;
        }
    }
#endif

#if defined(ALDRIN2_DEV_SUPPORT) || defined(CHX_FAMILY)
    if(hwsDeviceSpecInfo[devNum].devType == Aldrin2)
    {
        GT_U32  relativeAddr = (*baseAddr) & 0x00FFFFFF;

        switch (unitId)
        {
            case GEMAC_UNIT:
            case XLGMAC_UNIT:
            case MMPCS_UNIT:
            case CG_UNIT:
            case CGPCS_UNIT:
            case CG_RS_FEC_UNIT:
            case PTP_UNIT:
                /* GOP units */
                hwsAldrin2GopAddrCalc(unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            case SERDES_UNIT:
                /* SERDES unit */
                hwsAldrin2SerdesAddrCalc(unitNum/*portNum*/,relativeAddr/*regAddr*/,baseAddr);
                break;
            default:
                /* to prevent bug if miss some unit */
                return GT_NOT_INITIALIZED;
        }

        *localUnitNumPtr = 0;/* the caller uses this parameter for additional
            address calculations ... we can not allow them to do it. because
            formula can be very complex inside :
            hwsAldrin2GopAddrCalc(...) / hwsAldrin2SerdesAddrCalc(...) */
        return GT_OK;
    }
#endif
    return GT_OK;
}

/**
* @internal mvUnitInfoGet function
* @endinternal
*
* @brief   Return silicon specific base address and index for specified unit
*
* @param[in] devNum                   - Device Number
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @param[out] baseAddr                 - unit base address in device
* @param[out] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGet
(
    IN  GT_U8           devNum,
    IN  MV_HWS_UNITS_ID unitId,
    OUT GT_U32          *baseAddr,
    OUT GT_U32          *unitIndex
)
{
    if ((baseAddr == NULL) || (unitIndex == NULL))
    {
        return GT_BAD_PTR;
    }
    if (unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_FREE_RTOS)
    *baseAddr = PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[hwsDeviceSpecInfo[devNum].devType][unitId].baseAddr;
    *unitIndex = PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[hwsDeviceSpecInfo[devNum].devType][unitId].regOffset;
#else
    *baseAddr = hwsDeviceSpecUnitInfo[0][unitId].baseAddr;
    *unitIndex = hwsDeviceSpecUnitInfo[0][unitId].regOffset;
#endif /*MV_HWS_FREE_RTOS*/

    return GT_OK;
}

/**
* @internal mvUnitInfoSet function
* @endinternal
*
* @brief   Init silicon specific base address and index for specified unit
*
* @param[in] devType                  - Device type
* @param[in] unitId                   - unit ID (MAC, PCS, SERDES)
* @param[in] baseAddr                 - unit base address in device
* @param[in] unitIndex                - unit index in device
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoSet
(
    IN MV_HWS_DEV_TYPE devType,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32          baseAddr,
    IN GT_U32          unitIndex
)
{
    if ((devType >= LAST_SIL_TYPE) || (unitId >= LAST_UNIT))
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_FREE_RTOS)
    PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[devType][unitId].baseAddr = baseAddr;
    PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[devType][unitId].regOffset = unitIndex;
#else
    hwsDeviceSpecUnitInfo[0][unitId].baseAddr = baseAddr;
    hwsDeviceSpecUnitInfo[0][unitId].regOffset = unitIndex;
#endif /*MV_HWS_FREE_RTOS*/

    return GT_OK;
}

/**
* @internal mvUnitInfoGetByAddr function
* @endinternal
*
* @brief   Return unit ID by unit address in device
*
* @param[in] devNum                   - Device Number
* @param[in] baseAddr                 - unit base address in device
*
* @param[out] unitId                   - unit ID (MAC, PCS, SERDES)
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS  mvUnitInfoGetByAddr
(
    IN  GT_U8           devNum,
    IN  GT_U32          baseAddr,
    OUT MV_HWS_UNITS_ID *unitId
)
{
    GT_U32 i;

    if (unitId == NULL)
    {
        return GT_BAD_PTR;
    }
    *unitId = LAST_UNIT;
    for (i = 0; i < LAST_UNIT; i++)
    {
#if !defined (MV_HWS_FREE_RTOS)
        if (baseAddr == PRV_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDeviceSpecUnitInfo)[hwsDeviceSpecInfo[devNum].devType][i].baseAddr)
#else
    if (baseAddr == hwsDeviceSpecUnitInfo[0][i].baseAddr)
#endif
        {
            *unitId = i;
            break;
        }
    }

    return GT_OK;
}

/**
* @internal mvHwsRedundancyVectorGet function
* @endinternal
*
* @brief   Get SD vector.
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
*
* @param[out] sdVector                - SD vector
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsRedundancyVectorGet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    OUT GT_U32                  *sdVector
)
{
#ifdef  MV_HWS_FREE_RTOS
#ifndef RAVEN_DEV_SUPPORT
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc == NULL)
    {
                *sdVector = 0;
                return GT_OK;
    }

    return hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc(devNum, portGroup, sdVector);
#else
    *sdVector = 0;
    return GT_OK;
#endif
#else
    if (PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc == NULL)
    {
                *sdVector = 0;
                return GT_OK;
    }

    return PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[HWS_DEV_SILICON_INDEX(devNum)].redundVectorGetFunc(devNum, portGroup, sdVector);
#endif
}

#ifndef RAVEN_DEV_SUPPORT
/**
* @internal mvHwsClockSelectorConfig function
* @endinternal
*
* @brief   Configures the DP/Core Clock Selector on port
*
* @param[in] devNum                   - system device number
* @param[in] portGroup                - port group (core) number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port standard metric
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsClockSelectorConfig
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
)
{
#ifdef MV_HWS_FREE_RTOS
    if (hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc == NULL)
    {
                return GT_OK;
    }


    CHECK_STATUS_EXT(hwsDevFunc[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc(devNum, portGroup, phyPortNum, portMode),
                     LOG_ARG_MAC_IDX_MAC(phyPortNum));
#else
    if (PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc == NULL)
    {
                return GT_OK;
    }


    CHECK_STATUS_EXT(PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[HWS_DEV_SILICON_INDEX(devNum)].clkSelCfgGetFunc(devNum, portGroup, phyPortNum, portMode),
                     LOG_ARG_MAC_IDX_MAC(phyPortNum));
#endif
    return GT_OK;
}
#endif

/**
* @internal hwsDeviceSpecGetFuncPtr function
* @endinternal
*
* @brief   Get function structure pointer.
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
void hwsDeviceSpecGetFuncPtr(MV_HWS_DEV_FUNC_PTRS **hwsFuncsPtr)
{
#ifdef  MV_HWS_FREE_RTOS
    static GT_BOOL devFuncInitDone = GT_FALSE;
#endif
        if(PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(devFuncInitDone)== GT_FALSE)
        {
#ifdef  MV_HWS_FREE_RTOS
                hwsOsMemSetFuncPtr(hwsDevFunc, 0, sizeof(MV_HWS_DEV_FUNC_PTRS));
#else
                hwsOsMemSetFuncPtr(PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc), 0, sizeof(MV_HWS_DEV_FUNC_PTRS) * LAST_SIL_TYPE);
#endif
                PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_SET(devFuncInitDone,GT_TRUE);
        }
#ifdef  MV_HWS_FREE_RTOS
        *hwsFuncsPtr = &hwsDevFunc[0];
#else
        *hwsFuncsPtr = &(PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[0]);
#endif
}

/**
* @internal genUnitRegisterSet function
* @endinternal
*
* @brief   Implement write access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] unitId                   -  unit ID
* @param[in] unitNum                  -  unit Num
* @param[in] regOffset                -  Reg Offset
* @param[in] data                     -  to write
* @param[in] mask                     -  Mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterSet
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_UOPT         unitNum,
    IN GT_UREG_DATA    regOffset,
    IN GT_UREG_DATA    data,
    IN GT_UREG_DATA    mask
)
{
    GT_U32 address;
    GT_U32 unitAddr = 0;
    GT_U32 unitIndex = 0;

    /* get unit base address and unit index for current device.
       In Bobcat3: the return value of unitNum is localUnitNumPtr (relative value in pipe 0,1) */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, unitNum, &unitAddr, &unitIndex, &unitNum /* localUnitNumPtr */));

    if(unitAddr == 0)
    {
        return GT_BAD_PARAM;
    }
    address = unitAddr + unitIndex * unitNum + regOffset;

    if ((unitId == INTLKN_RF_UNIT) || (unitId == ETI_ILKN_RF_UNIT))
    {
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        CHECK_STATUS(genInterlakenRegSet(devNum, portGroup, address, data, mask));
#endif
    }
    else
    {
        CHECK_STATUS(hwsRegisterSetFuncPtr(devNum, portGroup, address, data, mask));
    }

    return GT_OK;
}

/**
* @internal genUnitRegisterGet function
* @endinternal
*
* @brief   Read access to device registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] unitId                   -  unit ID
* @param[in] unitNum                  -  unit Num
* @param[in] regOffset                -  Reg Offset
* @param[OUT] data                    -  to read
* @param[in] mask                     -  Mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterGet
(
    IN GT_U8           devNum,
    IN GT_UOPT         portGroup,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_UOPT         unitNum,
    IN GT_UREG_DATA    regOffset,
    OUT GT_UREG_DATA    *data,
    IN GT_UREG_DATA    mask
)
{
    GT_U32 address;
    GT_U32 unitAddr = 0;
    GT_U32 unitIndex = 0;

    /* get unit base address and unit index for current device */
    CHECK_STATUS(mvUnitExtInfoGet(devNum, unitId, unitNum, &unitAddr, &unitIndex,&unitNum));
    if (unitAddr == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    address = unitAddr + unitIndex * unitNum + regOffset;

    if ((unitId == INTLKN_RF_UNIT) || (unitId == ETI_ILKN_RF_UNIT))
    {
#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
        CHECK_STATUS(genInterlakenRegGet(devNum, portGroup, address, data, mask));
#endif
    }
    else
    {
        CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, data, mask));
    }

    return GT_OK;
}

#if defined(BC2_DEV_SUPPORT) || !defined(MV_HWS_REDUCED_BUILD_EXT_CM3)
/**
* @internal genInterlakenRegSet function
* @endinternal
*
* @brief   Implement write access to INERLAKEN IP registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
* @param[in] data                     -  to write
* @param[in] mask                     -  mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegSet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    IN GT_U32 data,
    IN GT_U32 mask
)
{
    MV_INTLKN_REG *dbArray;
    GT_U32 regData;
    GT_U32 offset;

        if(hwsIlknRegDbGetFuncPtr == NULL)
        {
                return GT_NOT_INITIALIZED;
        }

    dbArray = hwsIlknRegDbGetFuncPtr(devNum, portGroup, address);

        if(dbArray == NULL)
        {
                return GT_NOT_SUPPORTED;
        }

        /*hwsOsPrintf("genInterlakenRegSet address 0x%x\n", address);*/

    if (mask == 0)
    {
        /* store the register data */
        regData = data;
    }
    else
    {
                CHECK_STATUS(genInterlakenRegGet(devNum, portGroup, address, &regData, 0));

                /*hwsOsPrintf("Data: 0x%x", regData);*/

                /* Update the relevant bits at the register data */
        regData = (regData & ~mask) | (data & mask);

                /*hwsOsPrintf("\tNew Data: 0x%x\n", regData);*/
    }

        offset = address & 0x7FF;

        /*hwsOsPrintf("(Set) offset 0x%x\n", offset);*/

    /* store new value in shadow */
    switch (offset)
    {
    case 0:
        dbArray->ilkn0MacCfg0 = regData;
        break;
    case 4:
        dbArray->ilkn0ChFcCfg0 = regData;
        break;
    case 0xC:
        dbArray->ilkn0MacCfg2 = regData;
        break;
    case 0x10:
        dbArray->ilkn0MacCfg3 = regData;
        break;
    case 0x14:
        dbArray->ilkn0MacCfg4 = regData;
        break;
    case 0x1C:
        dbArray->ilkn0MacCfg6 = regData;
        break;
    case 0x60:
        dbArray->ilkn0ChFcCfg1 = regData;
        break;
    case 0x200:
        dbArray->ilkn0PcsCfg0 = regData;
        break;
    case 0x204:
        dbArray->ilkn0PcsCfg1 = regData;
        break;
    case 0x20C:
        dbArray->ilkn0En = regData;
        break;
    case 0x238:
        dbArray->ilkn0StatEn = regData;
        break;
    default:
        return GT_NOT_SUPPORTED;
    }

    return hwsRegisterSetFuncPtr(devNum, portGroup, address, regData, 0);
}

/**
* @internal genInterlakenRegGet function
* @endinternal
*
* @brief   Implement read access to INERLAKEN IP registers.
*
* @param[in] devNum                   - Device Number
* @param[in] portGroup                - port group (core) number
* @param[in] address                  -  to access
*
* @param[out] data                     - read data
* @param[in] mask                     -  mask
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genInterlakenRegGet
(
    IN GT_U8 devNum,
    IN GT_U32 portGroup,
    IN GT_U32 address,
    OUT GT_U32 *data,
    IN GT_U32 mask
)
{
    MV_INTLKN_REG *dbArray;
    GT_U32 regData;
    GT_U32 offset;

    devNum = devNum;
    portGroup = portGroup;

    if (data == NULL)
    {
        return GT_BAD_PARAM;
    }

        if(hwsIlknRegDbGetFuncPtr == NULL)
        {
                return GT_NOT_INITIALIZED;
        }

        /*hwsOsPrintf("genInterlakenRegGet address 0x%x\n", address);*/

        dbArray = hwsIlknRegDbGetFuncPtr(devNum, portGroup, address);

        if(dbArray == NULL)
        {
                return GT_NOT_SUPPORTED;
        }

        offset = address & 0x7FF;

        /*hwsOsPrintf("(Set) offset 0x%x\n", offset);*/

        switch (offset)
    {
    case 0:      /*ILKN_0_MAC_CFG_0 = 0,*/
        regData = dbArray->ilkn0MacCfg0;
        break;
    case 4:
        regData = dbArray->ilkn0ChFcCfg0;
        break;
    case 0xC:
        regData = dbArray->ilkn0MacCfg2;
        break;
    case 0x10:
        regData = dbArray->ilkn0MacCfg3;
        break;
    case 0x14:
        regData = dbArray->ilkn0MacCfg4;
        break;
    case 0x1C:
        regData = dbArray->ilkn0MacCfg6;
        break;
    case 0x60:
        regData = dbArray->ilkn0ChFcCfg1;
        break;
    case 0x200:
        regData = dbArray->ilkn0PcsCfg0;
        break;
    case 0x204:
        regData = dbArray->ilkn0PcsCfg1;
        break;
    case 0x20C:
        regData = dbArray->ilkn0En;
        break;
    case 0x238:
        regData = dbArray->ilkn0StatEn;
        break;
    case 0x10C:
                /* these registers offsets are readable */
                CHECK_STATUS(hwsRegisterGetFuncPtr(devNum, portGroup, address, &regData, 0));
        break;
    default:
        *data = 0;
        return GT_NOT_SUPPORTED;
    }

    if (mask == 0)
    {
        *data = regData;
    }
    else
    {
        /* Retrieve the relevant bits from the register's data and shift left */
        *data = (regData & mask);
    }
    return GT_OK;
}
#endif /*#ifdef BC2_DEV_SUPPORT*/

#ifndef MV_HWS_REDUCED_BUILD
/**
* @internal mvHwsReNumberDevNum function
* @endinternal
*
* @brief   Replace the ID of a device from old device number
*         to a new device number
* @param[in] oldDevNum                - old device num
* @param[in] newDevNum                - new device num
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsReNumberDevNum
(
    IN GT_U8   oldDevNum,
    IN GT_U8   newDevNum
)
{
    MV_HWS_DEV_TYPE  devType;
    GT_U32           portNum;
    GT_U32           numOfSupModes;
    GT_U32           portMode;

    if ((newDevNum >= HWS_MAX_DEVICE_NUM) || (hwsDeviceSpecInfo[oldDevNum].devNum != oldDevNum))
    {
        return GT_BAD_PARAM;
    }

    devType = hwsDeviceSpecInfo[oldDevNum].devType;

    /* Init device */
    if ((HWS_DEV_SILICON_TYPE(oldDevNum) == Bobcat3) || (HWS_DEV_SILICON_TYPE(oldDevNum) == Aldrin2) ||
        (HWS_DEV_SILICON_TYPE(oldDevNum) == Pipe))
    {
        hwsOsMemCopyFuncPtr(hwsDeviceSpecInfo[newDevNum].avagoSerdesInfo.cpllInitDoneStatusArr,
                            hwsDeviceSpecInfo[oldDevNum].avagoSerdesInfo.cpllInitDoneStatusArr,sizeof(GT_BOOL)*MV_HWS_MAX_CPLL_NUMBER);

        hwsOsMemCopyFuncPtr(hwsDeviceSpecInfo[newDevNum].avagoSerdesInfo.cpllCurrentOutFreqArr,
                            hwsDeviceSpecInfo[oldDevNum].avagoSerdesInfo.cpllCurrentOutFreqArr,sizeof(MV_HWS_CPLL_OUTPUT_FREQUENCY)*MV_HWS_MAX_CPLL_NUMBER);
    }

    /* init new dev Num */
    CHECK_STATUS(mvHwsDeviceInit(newDevNum, devType, NULL));

    /* copy fec from old elements array */
    for (portNum = 0; portNum < hwsDeviceSpecInfo[oldDevNum].portsNum; portNum++)
    {
        if (hwsDevicesPortsElementsArray[newDevNum][portNum].supModesCatalog == NULL)
        {
            continue;
        }

        hwsPortsElementsFindMode(hwsDevicesPortsElementsArray[newDevNum][portNum].supModesCatalog,
                                 NON_SUP_MODE, &numOfSupModes);

        for (portMode = 0; portMode < numOfSupModes; portMode++)
        { /* for each portMode, copy the FEC value from old device DB */
            hwsDevicesPortsElementsArray[newDevNum][portNum].perModeFecList[portMode] =
            hwsDevicesPortsElementsArray[oldDevNum][portNum].perModeFecList[portMode];
        }
    }

    /* Close old device */
    mvHwsDeviceClose(oldDevNum);

    hwsOsMemSetFuncPtr(&hwsDeviceSpecInfo[oldDevNum], 0, sizeof (HWS_DEVICE_INFO));

    return GT_OK;
}

/**
* @internal mvHwsDeviceInit function
* @endinternal
*
* @brief   Init device,
*         according to device type call function to init all software related DB:
*         DevInfo, Port (Port modes, MAC, PCS and SERDES) and address mapping.
* @param[in] devNum                  - Device Number
* @param[in] devType                  - enum of the device type
* @param[in] funcPtr                  - pointer to structure that hold the "os"
*                                      functions needed be bound to HWS.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceInit
(
    IN GT_U8             devNum,
    IN MV_HWS_DEV_TYPE   devType,
    IN HWS_OS_FUNC_PTR   *funcPtr
)
{
    GT_STATUS rc = GT_OK;
    switch (devType)
    {
#ifdef CHX_FAMILY
    case Lion2A0:
    case Lion2B0:
        rc = hwsLion2IfInit(devNum, funcPtr);
        break;
    case HooperA0:
        rc = hwsHooperIfInit(devNum, funcPtr);
        break;
    case BobcatA0:
        rc = hwsBobcat2IfInit(devNum, funcPtr);
        break;
    case Alleycat3A0:
        rc = mvHwsAlleycat3IfInit(devNum, funcPtr);
        break;
    case AC5:
        rc = mvHwsAlleycat5IfInit(devNum, funcPtr);
        break;
    case BobK:
        rc = hwsBobKIfInit(devNum, funcPtr);
        break;
    case Aldrin:
        rc = hwsAldrinIfInit(devNum, funcPtr);
        break;
    case Bobcat3:
        rc = hwsBobcat3IfInit(devNum, funcPtr);
        break;
    case Aldrin2:
        rc = hwsAldrin2IfInit(devNum, funcPtr);
        break;
    case Falcon:
        rc = hwsFalconIfInit(devNum, funcPtr);
        break;
    case AC5P:
        rc = hwsHawkIfInit(devNum, funcPtr);
        break;
    case AC5X:
        rc = hwsPhoenixIfInit(devNum, funcPtr);
        break;
    case Harrier:
        rc = hwsHarrierIfInit(devNum, funcPtr);
        break;
    case Ironman:
        rc = hwsIronmanIfInit(devNum, funcPtr);
        break;
    case AAS:
        rc = hwsAasIfInit(devNum, funcPtr);
        break;


#endif /*CHX_FAMILY*/
#ifdef PX_FAMILY
    case Pipe:
        rc = hwsPipeIfInit(devNum, funcPtr);
        break;
#endif /*PX_FAMILY*/
    default:
        break;
    }

    return rc;
}

/**
* @internal mvHwsDeviceClose function
* @endinternal
*
* @brief   Close device,
*         according to device type call function to free all resource allocated for ports initialization.
*
* @param[in] devNum                  - Device Number
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsDeviceClose
(
    IN GT_U8   devNum
)
{
    switch (hwsDeviceSpecInfo[devNum].devType)
    {
#ifdef CHX_FAMILY
        case Lion2A0:
        case Lion2B0:
            hwsLion2IfClose(devNum);
            break;
        case HooperA0:
            hwsHooperIfClose(devNum);
            break;
        case BobcatA0:
            hwsBobcat2IfClose(devNum);
            break;
        case Alleycat3A0:
            hwsAlleycat3IfClose(devNum);
            break;
        case AC5:
            hwsAlleycat5IfClose(devNum);
            break;
        case BobK:
            hwsBobKIfClose(devNum);
            break;
        case Aldrin:
            hwsAldrinIfClose(devNum);
            break;
        case Bobcat3:
            hwsBobcat3IfClose(devNum);
            break;
        case Aldrin2:
            hwsAldrin2IfClose(devNum);
            break;
        case Falcon:
            hwsFalconIfClose(devNum);
            break;
        case AC5P:
            hwsHawkIfClose(devNum);
            break;
        case AC5X:
            hwsPhoenixIfClose(devNum);
            break;
        case Harrier:
            hwsHarrierIfClose(devNum);
            break;
        case Ironman:
            hwsIronmanIfClose(devNum);
            break;
        case AAS:
            hwsAasIfClose(devNum);
            break;

#endif /*CHX_FAMILY*/
#ifdef PX_FAMILY
        case Pipe:
            hwsPipeIfClose(devNum);
            break;
#endif
        default:
            break;
    }

    #if defined (SSE_ENABLED) && defined (CHX_FAMILY)
        mvHwsSseInvokerDestroy();
    #endif

    return GT_OK;
}

#endif

/**
* @internal mvHwsGeneralLog function
* @endinternal
*
* @brief   Hws log message builder and logger functionn for HOST/FW
*
* @param[in] funcNamePtr              - function name pointer
* @param[in] fileNamePtr              - file name pointer
* @param[in] lineNum                  - line number
* @param[in] returnCode               - function return code
*                                      ...            - argument list
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Arguments: fileNamePtr, lineNum, returnCode are used only in host side.
*
*/
GT_STATUS mvHwsGeneralLog
(
    IN const char      *funcNamePtr,
#ifndef  MV_HWS_FREE_RTOS
    IN char            *fileNamePtr,
    IN GT_U32          lineNum,
    IN GT_STATUS       returnCode,
#endif
    ...
)
{
    char buffer[HWS_LOG_MSG_BUFFER_SIZE]; /* String message bufer */
    char unknownStr[] = "[unknown]";
    unsigned int bufferOffset=0;   /* Offset in buffer */
    va_list args;   /* parameter for iterating the arguments */
    GT_U32 val; /* current value read from argument list */
    GT_U8 stop=0; /* stop iterating the arguments */
    GT_U8 iteration=0;  /* iteration number */
    char *msgArgStringPtr;    /* string argument */
    GT_U32 msgArgStringPtrLen=0; /* string argument length */
#ifdef  MV_HWS_FREE_RTOS
    GT_U32 funcNameLength=0; /* function name string length */
#endif

#ifndef  MV_HWS_FREE_RTOS
    if (fileNamePtr==NULL)
    {
        fileNamePtr = unknownStr;
        /*return GT_BAD_PTR;*/
    }
#endif

    if (funcNamePtr==NULL)
    {
        funcNamePtr = unknownStr;
        /*return GT_BAD_PTR;*/
    }

#ifdef  HWS_NO_LOG
    /* Prevent 'unused-parameter' compilation error in case of running in host without log enabled */
    lineNum = lineNum;
    returnCode = returnCode;
#endif

    /* Initializing the buffer */
    hwsOsMemSetFuncPtr(buffer, '\0', HWS_LOG_MSG_BUFFER_SIZE);

/* Only in FW side the function name and "Fail" stringshould be a part of the message.
   In cpss, function name and return code is taken as different parameters */
#ifdef  MV_HWS_FREE_RTOS
    /* Maximal function name to copy to buffer, with enought space for '\0' and " Fail " */
    funcNameLength = hwsStrLen(funcNamePtr);
    funcNameLength = ( funcNameLength < (HWS_LOG_MSG_BUFFER_SIZE-7) ) ? funcNameLength : (HWS_LOG_MSG_BUFFER_SIZE-7);
    /* Copying function name */
    hwsOsMemCopyFuncPtr(buffer, funcNamePtr, funcNameLength);
    bufferOffset += funcNameLength;
    /* Copy "fail" */
    hwsOsMemCopyFuncPtr(buffer + bufferOffset, " Fail ", 6);
    bufferOffset += 6;
#endif

#ifdef  MV_HWS_FREE_RTOS
    va_start(args, funcNamePtr);
#else
    va_start(args, returnCode);
#endif
    val = va_arg(args, unsigned int);
    /* While the current argument is not the last argument
       and there is enought space in the buffer */
    while (val!=LOG_ARG_NONE_MAC && stop==0)
    {
        if (iteration>0)
        {
            hwsOsMemCopyFuncPtr(buffer + bufferOffset, ",", 1);
            bufferOffset++;
        }
        else
        {
            iteration++;
        }
        /* Decoding argument */
        switch(val)
        {
            case LOG_ARG_SERDES_FLAG:
                /* Serdes argument */
                /* space check */
                if (bufferOffset+1+8 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " sd: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=8;
                break;

            case LOG_ARG_MAC_FLAG:
                /* Mac argument */
                /* space check */
                if (bufferOffset+1+8 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " mc: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=8;
                break;

            case LOG_ARG_PCS_FLAG:
                /* Pcs argument */
                /* space check */
                if (bufferOffset+1+9 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " pcs: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=9;
                break;

            case LOG_ARG_PORT_FLAG:
                /* Port argument */
                /* space check */
                if (bufferOffset+1+7 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                /* space check */
                hwsSprintf(buffer+bufferOffset, " p: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=7;
                break;

            case LOG_ARG_REG_ADDR_FLAG:
                /* Register address argument */
                /* space check */
                if (bufferOffset+1+15 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " rg: 0x%08x", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=15;
                break;

            case LOG_ARG_STRING_FLAG:
                /* String argument */
                msgArgStringPtr = va_arg(args, char*);
                msgArgStringPtrLen = hwsStrLen(msgArgStringPtr);
                /* space check */
                if (bufferOffset+1+msgArgStringPtrLen >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    /* Get at least some of the string message */
                    if ( hwsMemCpy(buffer+bufferOffset, msgArgStringPtr, HWS_LOG_MSG_BUFFER_SIZE-(bufferOffset+1)) == NULL )
                    {
                        stop=1;
                        break;
                    }
                    /* That's it, buffer is full */
                    bufferOffset += (HWS_LOG_MSG_BUFFER_SIZE-(bufferOffset+1));
                    stop=1;
                    break;
                }
                /* space check */
                hwsSprintf(buffer+bufferOffset, msgArgStringPtr);
                bufferOffset+=msgArgStringPtrLen;
                break;

            case LOG_ARG_GEN_PARAM_FLAG:
                /* Gen param address argument */
                /* space check */
                if (bufferOffset+1+9 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, " prm: %03d", ( 0xFF & (va_arg(args, unsigned int)) ) );
                bufferOffset+=9;
                break;

            default:
                /* Unrecognized argument */
                /* space check */
                if (bufferOffset+1+7 >= HWS_LOG_MSG_BUFFER_SIZE)
                {
                    stop=1;
                    break;
                }
                hwsSprintf(buffer+bufferOffset, "bad_arg", 7);
                bufferOffset+=7;
                break;
        }

        if (stop==0)
        {
            val = va_arg(args, unsigned int);
        }
    }
    va_end(args);

    /* FW log */
    FW_LOG_ERROR(buffer);
    /* Host (cpss) log */
#ifdef  MV_HWS_FREE_RTOS
    HWS_TO_CPSS_LOG_ERROR_AND_RETURN_MAC(funcNamePtr, " "/*fileNamePtr*/, 0/*lineNum*/, 1/*returnCode*/, buffer);
#else
    if (returnCode != GT_OK)
    {
         /* hwsOsPrintf("%s", buffer); */
        HWS_TO_CPSS_LOG_ERROR_AND_RETURN_MAC(funcNamePtr, fileNamePtr, lineNum, returnCode, buffer);
    }
    else
    {
        /*  hwsOsPrintf("%s", buffer); */
       HWS_TO_CPSS_LOG_INFORMATION_MAC(funcNamePtr, fileNamePtr, lineNum, buffer);
    }
    return GT_OK;
#endif

}

/**
* @internal mvHwsGeneralLogStrMsgWrapper function
* @endinternal
*
* @brief   Hws log message builder wrapper function for a log
*          message that is already evaluated by the caller.
*
* @param[in] ffuncNamePtr    - function name pointer
* @param[in] fileNamePtr    - file name pointer
* @param[in] lineNum        - line number
* @param[in] returnCode     - function return code
* @param[in] strMsgPtr      - string message to log
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
unsigned int mvHwsGeneralLogStrMsgWrapper
(
    IN const char      *funcNamePtr,
#ifndef  MV_HWS_FREE_RTOS
    IN char            *fileNamePtr,
    IN unsigned int    lineNum,
    IN int             returnCode,
#endif
    IN char            *strMsgPtr
)
{
#ifndef  MV_HWS_FREE_RTOS
    return (mvHwsGeneralLog(funcNamePtr, fileNamePtr, lineNum, returnCode, LOG_ARG_STRING_FLAG, strMsgPtr, LOG_ARG_NONE_FLAG));
#else
    return (mvHwsGeneralLog(funcNamePtr, LOG_ARG_STRING_FLAG, strMsgPtr, LOG_ARG_NONE_FLAG));
#endif
}

#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (AC5P_DEV_SUPPORT) || defined (HARRIER_DEV_SUPPORT) || defined (IRONMAN_DEV_SUPPORT)

/**
* @internal genUnitRegDbEntryGet function
* @endinternal
*
* @brief   get DB entry.
*
* @param[in] devNum                  - system device number
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
* @param[out] convertIdxPtr          - (pointer to) structure with unit data
*
* @retval address                        - on success
* @retval MV_HWS_SW_PTR_ENTRY_UNUSED     - on error
*/
GT_STATUS genUnitRegDbEntryGet
(
    IN GT_U8           devNum,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr,
    OUT MV_HWS_HAWK_CONVERT_STC *convertIdxPtr
)
{
    GT_U32 size = 0;
    const MV_HWS_REG_ADDR_FIELD_STC *regDbPtr = NULL;
    GT_STATUS rc;

    if(fieldRegOutPtr == NULL)
    {
        return GT_BAD_PTR;
    }

    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, phyPortNum, portMode, convertIdxPtr);
    if(rc != GT_OK)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    regDbPtr = PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(mvHwsRegDb)[unitId];
    if(regDbPtr == NULL)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    switch (unitId)
    {
        case MIF_CPU_UNIT:
            if(HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                convertIdxPtr->ciderIndexInUnit = convertIdxPtr->ciderUnit;
                convertIdxPtr->ciderUnit = 0;
            }
            size = MIF_REGISTER_LAST_E;
            break;

        case MIF_400_UNIT:
        case MIF_400_SEG_UNIT:
            if(unitId == MIF_400_SEG_UNIT)
            {
                convertIdxPtr->ciderIndexInUnit /= 4;
            }
            size = MIF_REGISTER_LAST_E;
            break;

        case MIF_USX_UNIT:
            /* calling mvHwsGlobalMacToLocalIndexConvert return
                    in hawk we have 2 units with 24 port each.
                    in phoenix we have 6 units with 8 port each but we need 3 units with 16 port each */
            if(HWS_DEV_SILICON_TYPE(devNum) == AC5X)
            {
                convertIdxPtr->ciderIndexInUnit = (convertIdxPtr->ciderUnit% 2) * 8 + convertIdxPtr->ciderIndexInUnit;
                convertIdxPtr->ciderUnit /= 2;
            }
            size = MIF_REGISTER_LAST_E;
            break;

        case MTI_MAC400_UNIT:
            convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            if (hwsDeviceSpecInfo[devNum].devType == AAS)
            {
                size = AAS_MAC400_REGISTER_LAST_E;
            }
            else
            {
                size = MAC400_REGISTER_LAST_E;
            }
            break;

        case MTI_USX_MAC_UNIT:
            size = MAC100_REGISTER_LAST_E;
            convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            break;

        case MTI_MAC100_UNIT:
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                size = MAC100_BR_REGISTER_LAST_E;
                convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AAS)
            {
                size = AAS_MAC100_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AAS)
            {
                size = AAS_MAC100_REGISTER_LAST_E;
            }
            else
            {
                size = MAC100_REGISTER_LAST_E;
                convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            }
            break;

        case MTI_EXT_UNIT:
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                size = HAWK_MTIP_EXT_BR_UNITS_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                size = PHOENIX_MTIP_EXT_UNITS_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AAS)
            {
                size = AAS_MAC_PORT_EXT_REGISTER_LAST_E;
            }
            else
            {
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
            }
            break;

        case MTI_USX_EXT_UNIT:
            size = MTIP_USX_EXT_UNITS_REGISTER_LAST_E;
            convertIdxPtr->ciderIndexInUnit %= 8;
            break;

        case MTI_RSFEC_UNIT:
            if(HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum))
            {
                size = RSFEC_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                size = PHOENIX_RSFEC_REGISTER_LAST_E;
            }
            else
            {
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
            }
            break;

        case MTI_CPU_RSFEC_UNIT:
            size = CPU_PCS_RSFEC_UNITS_RSFEC_REGISTER_LAST_E;
            break;

        case MTI_CPU_PCS_UNIT:
            size = CPU_PCS_UNITS_REGISTER_LAST_E;
            break;

        case MTI_CPU_EXT_UNIT:
            if (hwsDeviceSpecInfo[devNum].devType == AAS)
            {
                size = AAS_MAC_PORT_EXT_REGISTER_LAST_E;
            }
            else
            {
                size = MTIP_CPU_EXT_UNITS_REGISTER_LAST_E;
            }
            break;

        case MTI_CPU_MAC_UNIT:
            if (hwsDeviceSpecInfo[devNum].devType == AAS)
            {
                size = AAS_MAC100_REGISTER_LAST_E;
            }
            else
            {
                size = CPU_MAC_REGISTER_LAST_E;
            }
            break;

        case MTI_PCS25_UNIT:
            size = PCS25_UNITS_REGISTER_LAST_E;
            convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            break;

        case MTI_PCS50_UNIT:
            size = PCS50_REGISTER_LAST_E;
            convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            break;

        case MTI_PCS400_UNIT:
            convertIdxPtr->ciderIndexInUnit = 0;
            size = PCS400_UNITS_REGISTER_LAST_E;
            break;

        case MTI_PCS200_UNIT:
            convertIdxPtr->ciderIndexInUnit = 0;
            size = PCS200_UNITS_REGISTER_LAST_E;
            break;

        case MTI_PCS100_UNIT:
            size = PCS100_REGISTER_LAST_E;
            convertIdxPtr->ciderIndexInUnit = 0;    /* port offset got from CPSS */
            break;

        case MTI_LOW_SP_PCS_UNIT:
        case MTI_USX_LPCS_UNIT:
            size = LPCS_UNITS_REGISTER_LAST_E;
            if(unitId == MTI_USX_LPCS_UNIT)
            {
                /* USX_MAC CIDER defined with portions of 8 x 3different base addresses,
                   therefore it is needed to normalize it to %8 and add appropriate base address via
                   mvUnitExtInfoGet()*/
                convertIdxPtr->ciderIndexInUnit %= 8;
            }
            break;

        case MTI_USX_PCS_UNIT:
            size = USX_PCS_UNITS_REGISTER_LAST_E;
            /* USX_MAC CIDER defined with portions of 8 x 3different base addresses,
               therefore it is needed to normalize it to %8 and add appropriate base address via
               mvUnitExtInfoGet()*/
            convertIdxPtr->ciderIndexInUnit %= 8;
            break;

        case ANP_USX_UNIT:
            if (hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                convertIdxPtr->ciderIndexInUnit = 0;
            }
            else
            {
                convertIdxPtr->ciderIndexInUnit %= 8;
            }

            GT_ATTR_FALLTHROUGH;
        case ANP_400_UNIT:
        case ANP_USX_O_UNIT:
            size = HAWK_ANP_UNITS_REGISTER_LAST_E;
            if (hwsDeviceSpecInfo[devNum].devType == AC5P)
            {
                size = HAWK_ANP_UNITS_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == Harrier)
            {
                if((unitId == ANP_USX_O_UNIT) || (unitId == ANP_USX_UNIT))
                {
                    return MV_HWS_SW_PTR_ENTRY_UNUSED;
                }
                else
                {
                    size = HARRIER_ANP_UNITS_REGISTER_LAST_E;
                }
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                size = PHOENIX_ANP_UNITS_REGISTER_LAST_E;
                if(unitId == ANP_USX_O_UNIT)
                {
                    convertIdxPtr->ciderIndexInUnit = 0;
                }
            }
            else if (hwsDeviceSpecInfo[devNum].devType == Ironman)
            {
                size = IRONMAN_ANP_UNIT_REGISTER_LAST_E;
            }
            else
            {
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
            }
            break;

        case ANP_CPU_UNIT:
            if (hwsDeviceSpecInfo[devNum].devType == AC5P)
            {
                size = HAWK_ANP_UNITS_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == AC5X)
            {
                size = PHOENIX_ANP_UNITS_REGISTER_LAST_E;
            }
            else if (hwsDeviceSpecInfo[devNum].devType == Ironman)
            {
                size = IRONMAN_ANP_UNIT_REGISTER_LAST_E;
            }
            else
            {
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
            }
            break;

        case AN_USX_UNIT:
            convertIdxPtr->ciderIndexInUnit = 0;
            GT_ATTR_FALLTHROUGH;
        case AN_400_UNIT:
        case AN_CPU_UNIT:
            size = AN_UNITS_REGISTER_LAST_E;
            break;

        case MTI_USX_RSFEC_UNIT:
        case MTI_USX_MULTIPLEXER_UNIT:
            if (unitId == MTI_USX_MULTIPLEXER_UNIT)
            {
                size = USXM_UNITS_REGISTER_LAST_E;
            }
            else
            {
                size = USX_RSFEC_UNITS_REGISTER_LAST_E;
            }
            if (HWS_USX_O_MODE_CHECK(portMode))
            {
                convertIdxPtr->ciderIndexInUnit = 0;
            }
            else
            {
                /* USX_MAC CIDER defined with portions of 8 x 3different base addresses,
                   therefore it is needed to normalize it to %8 and add appropriate base address via
                   mvUnitExtInfoGet()*/
                convertIdxPtr->ciderIndexInUnit %= 8;
                convertIdxPtr->ciderIndexInUnit /= 4;
            }
            break;
        case  MTI_CPU_SGPCS_UNIT:
            size = CPU_SGPCS_REGISTER_LAST_E;
            break;
        case MTI_USX_RSFEC_STATISTICS_UNIT:
            size = RSFEC_STATISTICS_REGISTER_LAST_E;
            if (HWS_USX_O_MODE_CHECK(portMode))
            {
                convertIdxPtr->ciderIndexInUnit = 0;
            }
            else
            {
                /* USX_MAC CIDER defined with portions of 8 x 3different base addresses,
                   therefore it is needed to normalize it to %8 and add appropriate base address via
                   mvUnitExtInfoGet()*/
                convertIdxPtr->ciderIndexInUnit %= 8;
                convertIdxPtr->ciderIndexInUnit /= 4;
            }
            break;
        case MTI_RSFEC_STATISTICS_UNIT:
            size = HWS_DEV_IS_AC5P_BASED_DEV_MAC(devNum) ? RSFEC_STATISTICS_REGISTER_LAST_E : PHOENIX_RSFEC_STATISTICS_REGISTER_LAST_E;
            break;

        case MTI_CPU_RSFEC_STATISTICS_UNIT:
            size = RSFEC_STATISTICS_REGISTER_LAST_E;
            break;

        case MTI_10G_PCS_FCFEC_UNIT:
            size = IRONMAN_PCS_FCFEC_REGISTER_LAST_E;
            convertIdxPtr->ciderIndexInUnit %= 8;
            convertIdxPtr->ciderIndexInUnit /= 4;
            break;

        case MTI_10G_PORT_EXT_UNIT:
            size = IRONMAN_MTIP_MAC_PORT_EXT_REGISTER_LAST_E;
            break;

        case MTI_10G_GLOBAL_EXT_UNIT:
            size = IRONMAN_MTIP_MAC_GLOBAL_EXT_REGISTER_LAST_E;
            break;

        case MTI_10G_MAC_NON_BR_UNIT:
            size = IRONMAN_MTIP_MAC_10G_REGISTER_LAST_E;
            break;

        case MTI_10G_MAC_BR_UNIT:
            size = IRONMAN_MTIP_MAC_10G_BR_REGISTER_LAST_E;
            break;

        case MTI_10G_PCS_LPCS_GENERAL_UNIT:
            size = IRONMAN_LPCS_GENERAL_UNITS_REGISTER_LAST_E;
            break;
        case MTI_10G_PCS_LPCS_PORT_UNIT:
            size = IRONMAN_LPCS_UNITS_REGISTER_LAST_E;
            break;
        case MTI_10G_PCS_BASER_UNIT:
            size = IRONMAN_BASE_R_PCS_REGISTER_LAST_E;
            break;
        case MTI_10G_PCS_EXT_UNIT:
            size = IRONMAN_MTIP_PCS_EXT_UNITS_REGISTER_LAST_E;
            break;
        case SERDES_UNIT:
            size = IRONMAN_SDW_LANE_REGISTER_LAST_E;
            break;
        case MTI_GLOBAL_EXT_UNIT:
        case MTI_CPU_GLOBAL_EXT_UNIT:
            size = AAS_MAC_GLOBAL_EXT_GLOBAL_REGISTER_LAST_E;
            break;
        case MTI_PCS_EXT_UNIT:
            size = AAS_PCS800_EXT_REGISTER_LAST_E;
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
* @internal genUnitFindAddressPrv function
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
GT_U32 genUnitFindAddressPrv
(
    IN GT_U8           devNum,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
)
{
    if (PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[HWS_DEV_SILICON_INDEX(devNum)].devUnitFieldFindAddrFunc == NULL)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }
    else
    {
        return PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(hwsDevFunc)[HWS_DEV_SILICON_INDEX(devNum)].devUnitFieldFindAddrFunc(devNum, phyPortNum, unitId, portMode, fieldName, fieldRegOutPtr);
    }
}

/**
* @internal original_genUnitFindAddressPrv function
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
GT_U32 original_genUnitFindAddressPrv
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
    GT_U32 unitIndex = 0;
    GT_U32 localUnitNum = 0;
    GT_U32 address = MV_HWS_SW_PTR_ENTRY_UNUSED;
    GT_U8  preemptionAccessForMif = 0;
    GT_U32 mask = 0xFFFFF000;

    if(hwsDeviceSpecInfo[devNum].devType == AAS)
    {
        mask = 0xFFFF0000;
    }

    /* if MIF BR access requested - get regular MIF address and add 8 to "ciderIndexInUnit" */
    if(unitId == MIF_400_BR_UNIT)
    {
        preemptionAccessForMif = 1;
        unitId = MIF_400_UNIT;
    }

    if (portMode == NON_SUP_MODE)
    {
        switch (unitId) {
            case MIF_400_UNIT:
            case MTI_MAC100_UNIT:
            case MTI_EXT_UNIT:
            case MTI_RSFEC_UNIT:
            case MTI_PCS25_UNIT:
            case MTI_PCS50_UNIT:
            case MTI_PCS100_UNIT:
            case MTI_LOW_SP_PCS_UNIT:
            case ANP_400_UNIT:
            case AN_400_UNIT:
            case MIF_CPU_UNIT:
            case MTI_CPU_RSFEC_UNIT:
            case MTI_CPU_PCS_UNIT:
            case MTI_CPU_EXT_UNIT:
            case MTI_CPU_MAC_UNIT:
            case MTI_CPU_SGPCS_UNIT:
            case MTI_RSFEC_STATISTICS_UNIT:
            case MTI_CPU_RSFEC_STATISTICS_UNIT:
            case MTI_10G_PORT_EXT_UNIT:
            case MTI_10G_GLOBAL_EXT_UNIT:
            case MTI_10G_MAC_BR_UNIT:
            case MTI_10G_MAC_NON_BR_UNIT:
            case MTI_10G_PCS_LPCS_GENERAL_UNIT:
            case MTI_10G_PCS_LPCS_PORT_UNIT:
            case MTI_10G_PCS_BASER_UNIT:
            case MTI_10G_PCS_FCFEC_UNIT:
            case MTI_10G_PCS_EXT_UNIT:
            case SERDES_UNIT:
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

            case MIF_USX_UNIT:
            case MTI_USX_MAC_UNIT:
            case MTI_USX_EXT_UNIT:
            case MTI_USX_LPCS_UNIT:
            case MTI_USX_PCS_UNIT:
            case MTI_USX_RSFEC_UNIT:
            case ANP_USX_UNIT:
            case ANP_USX_O_UNIT:
            case AN_USX_UNIT:
            case MTI_USX_MULTIPLEXER_UNIT:
            case MTI_USX_RSFEC_STATISTICS_UNIT:
                portMode = _10G_OUSGMII;
                break;

            case ANP_CPU_UNIT:
            case AN_CPU_UNIT:
                portMode = _10GBase_KR;
                break;

            default:
                return MV_HWS_SW_PTR_ENTRY_UNUSED;
        }
    }

    /* TBD_ALEX: should be impemented per devType by calling to generic callback */
    rc = genUnitRegDbEntryGet(devNum, phyPortNum, unitId, portMode, fieldName, fieldRegOutPtr, &convertIdx);
    if(rc != GT_OK)
    {
        return MV_HWS_SW_PTR_ENTRY_UNUSED;
    }

    rc = mvUnitExtInfoGet(devNum, unitId, phyPortNum, &unitAddr, &unitIndex, &localUnitNum);

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
    if(unitId == MTI_EXT_UNIT)
    {
        if(HWS_IS_PORT_MULTI_SEGMENT(portMode) && (fieldRegOutPtr->offsetFormula != 0))
        {
             convertIdx.ciderIndexInUnit /= 4;
        }
    }

    if((hwsDeviceSpecInfo[devNum].devType == AC5P) || (hwsDeviceSpecInfo[devNum].devType == Ironman))
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

    if(hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        if((unitId == MTI_RSFEC_UNIT) &&
           (convertIdx.ciderIndexInUnit != 0))
        {
            /*
                Treatment for special CIDER order:
                    example for 'p0_RSFEC_STATUS' register:

                    p0 = 0x4
                    p1_p3 = 0x104 + 0x20*(n-1)   while n=1..3

            */
            switch(fieldRegOutPtr->regOffset)
            {
                case 0x0:   /*p0_RSFEC_CONTROL*/
                case 0x4:   /*p0_RSFEC_STATUS*/
                case 0x8:   /*p0_RSFEC_CCW_LO*/
                case 0xc:   /*p0_RSFEC_CCW_HI*/
                case 0x10:  /*p0_RSFEC_NCCW_LO*/
                case 0x14:  /*p0_RSFEC_NCCW_HI*/
                case 0x18:  /*p0_RSFEC_LANE_MAP*/
                    fieldRegOutPtr->offsetFormula = 0x0;
                    fieldRegOutPtr->regOffset += 0x100 + 0x20 * (convertIdx.ciderIndexInUnit - 1);
                    break;

                default:
                    break;
            }
        }
    }

    if (hwsDeviceSpecInfo[devNum].devType == Ironman)
    {
        if(unitId == MTI_10G_PORT_EXT_UNIT)
        {
            mask = 0xFFFFFF00;
        }

        /* 100BaseX registers are not per MAC port, but per SD lane [0..1] */
        if(unitId == MTI_10G_PCS_EXT_UNIT)
        {
            switch(fieldRegOutPtr->regOffset)
            {
                case 0xC0:
                case 0xC4:
                case 0xC8:
                case 0xCC:
                case 0xD0:
                case 0xD4:
                case 0xD8:
                case 0xDC:
                case 0xE0:
                    convertIdx.ciderIndexInUnit /= 4;
                    break;

                default:
                    break;
            }
        }
    }

    address = (unitAddr & mask) +  fieldRegOutPtr->regOffset + fieldRegOutPtr->offsetFormula * (convertIdx.ciderIndexInUnit + 8 * preemptionAccessForMif);
    return address;
}

/**
* @internal genUnitPortModeRegisterFieldSet function
* @endinternal
*
* @brief   set field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] portMode                - port mode used
* @param[in] fieldName               - register field to be configured
* @param[in] fieldData               - register field data to be written
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitPortModeRegisterFieldSet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    IN GT_UREG_DATA    fieldData,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
)
{
    GT_U32 address;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg = {0, 0, 0, 0};

    address = genUnitFindAddressPrv(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg);
    if(address == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        return GT_BAD_PARAM;
    }

    if(fieldRegOutPtr != NULL)
    {
        hwsMemCpy(fieldRegOutPtr, &fieldReg, sizeof(MV_HWS_REG_ADDR_FIELD_STC));
    }

    CHECK_STATUS(hwsRegisterSetFieldFunc(devNum, portGroup, address, fieldReg.fieldStart, fieldReg.fieldLen, fieldData));

    return GT_OK;
}

/**
* @internal genUnitRegisterFieldSet function
* @endinternal
*
* @brief   set field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[in] fieldData               - register field data to be written
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterFieldSet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32          fieldName,
    IN GT_UREG_DATA    fieldData,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr
)
{
    /* the following units must call to genUnitPortModeRegisterFieldSet with relevant portMode !! */
    if((MTI_USX_RSFEC_UNIT == unitId) || (MTI_USX_MULTIPLEXER_UNIT == unitId) || (MTI_USX_RSFEC_STATISTICS_UNIT == unitId))
    {
        return GT_NOT_SUPPORTED;
    }

    if(AAS == HWS_DEV_SILICON_TYPE(devNum))
    {
        if(MTI_EXT_UNIT == unitId)
        {
            return GT_NOT_SUPPORTED;
        }
    }

    CHECK_STATUS(genUnitPortModeRegisterFieldSet(devNum, portGroup, phyPortNum, unitId,
                                                 NON_SUP_MODE, fieldName,
                                                 fieldData, fieldRegOutPtr));

    return GT_OK;
}

/**
* @internal genUnitPortModeRegisterFieldGet function
* @endinternal
*
* @brief   get field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] portMode                - port mode used
* @param[in] fieldName               - register field to be configured
* @param[out] fieldDataPtr           - (pointer to) register field data
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitPortModeRegisterFieldGet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN MV_HWS_PORT_STANDARD portMode,
    IN GT_U32          fieldName,
    OUT GT_UREG_DATA   *fieldDataPtr,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr

)
{
    GT_U32 address;
    MV_HWS_REG_ADDR_FIELD_STC fieldReg = {0, 0, 0, 0};

    address = genUnitFindAddressPrv(devNum, phyPortNum, unitId, portMode, fieldName, &fieldReg);
    if(address == MV_HWS_SW_PTR_ENTRY_UNUSED)
    {
        return GT_BAD_PARAM;
    }

    if(fieldRegOutPtr != NULL)
    {
        hwsMemCpy(fieldRegOutPtr, &fieldReg, sizeof(MV_HWS_REG_ADDR_FIELD_STC));
    }

    CHECK_STATUS(hwsRegisterGetFieldFunc(devNum, portGroup, address, fieldReg.fieldStart, fieldReg.fieldLen, fieldDataPtr));

    return GT_OK;
}

/**
* @internal genUnitRegisterFieldGet function
* @endinternal
*
* @brief   get field in register function.
*
* @param[in] devNum                  - system device number
* @param[in] portGroup               - port group
* @param[in] phyPortNum              - physical port number
* @param[in] unitId                  - unit to be configured
* @param[in] fieldName               - register field to be configured
* @param[out] fieldDataPtr           - (pointer to) register field data
* @param[out] fieldRegOutPtr         - (pointer to) field structure that was configured
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS genUnitRegisterFieldGet
(
    IN GT_U8           devNum,
    IN GT_U32          portGroup,
    IN GT_UOPT         phyPortNum,
    IN MV_HWS_UNITS_ID unitId,
    IN GT_U32          fieldName,
    OUT GT_UREG_DATA   *fieldDataPtr,
    OUT MV_HWS_REG_ADDR_FIELD_STC *fieldRegOutPtr

)
{
    /* the following units must call to genUnitPortModeRegisterFieldSet with relevant portMode !! */
    if((MTI_USX_RSFEC_UNIT == unitId) || (MTI_USX_MULTIPLEXER_UNIT == unitId) || (MTI_USX_RSFEC_STATISTICS_UNIT == unitId))
    {
        return GT_NOT_SUPPORTED;
    }

    if(AAS == HWS_DEV_SILICON_TYPE(devNum))
    {
        if(MTI_EXT_UNIT == unitId)
        {
            return GT_NOT_SUPPORTED;
        }
    }

    CHECK_STATUS(genUnitPortModeRegisterFieldGet(devNum, portGroup, phyPortNum, unitId,
                                                 NON_SUP_MODE, fieldName,
                                                 fieldDataPtr, fieldRegOutPtr));

    return GT_OK;
}
#endif

#if !defined (MV_HWS_FREE_RTOS) || defined (AC5P_DEV_SUPPORT)
/* Get Cider index for a given port when using mac 400 port*/
static GT_STATUS mvHwsHawkGlobalMac400ToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    devNum = devNum;

    ciderIndexPtr->ciderUnit = portNum / MV_HWS_AC5P_GOP_PORT_NUM_CNS;
    ciderIndexPtr->expressChannelId = portNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS;

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        switch(portNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS)
        {
            case 0:
                ciderIndexPtr->ciderIndexInUnit = 0;
                break;
            case 10:
                /*
                    Please pay attention:
                        actually CIDER index = 1, but for most of use cases
                        local group port index is needed (0, 4). In rare cases of
                        (0, 1) division by 4 should be applied.
                */
                ciderIndexPtr->ciderIndexInUnit = 4;
                break;
            default:
                return GT_NO_SUCH;
        }
    }
    else
    {
        if(portNum == MV_HWS_AC5P_GOP_PORT_CPU_CNS)/*105*/
        {
            ciderIndexPtr->ciderUnit = 0;
            ciderIndexPtr->ciderIndexInUnit = 0;
            ciderIndexPtr->expressChannelId = 26;
        }
        else if(portNum == (MV_HWS_AC5P_GOP_PORT_CPU_CNS - 1))/*104*/
        {
            return GT_NO_SUCH;
        }
        else
        {
            switch(portNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS)
            {
                case 0:
                    ciderIndexPtr->ciderIndexInUnit = 0;
                    ciderIndexPtr->preemptionChannelId = 5;
                    break;
                case 1:
                    ciderIndexPtr->ciderIndexInUnit = 1;
                    ciderIndexPtr->preemptionChannelId = 9;
                    break;
                case 2:
                    ciderIndexPtr->ciderIndexInUnit = 2;
                    ciderIndexPtr->preemptionChannelId = 3;
                    break;
                case 6:
                    ciderIndexPtr->ciderIndexInUnit = 3;
                    ciderIndexPtr->preemptionChannelId = 7;
                    break;
                case 10:
                    ciderIndexPtr->ciderIndexInUnit = 4;
                    ciderIndexPtr->preemptionChannelId = 11;
                    break;
                case 14:
                    ciderIndexPtr->ciderIndexInUnit = 5;
                    ciderIndexPtr->preemptionChannelId = 15;
                    break;
                case 18:
                    ciderIndexPtr->ciderIndexInUnit = 6;
                    ciderIndexPtr->preemptionChannelId = 19;
                    break;
                case 22:
                    ciderIndexPtr->ciderIndexInUnit = 7;
                    ciderIndexPtr->preemptionChannelId = 23;
                    break;
                default:
                    return GT_NO_SUCH;
            }
        }
    }

    return GT_OK;
}
#endif

#if !defined (MV_HWS_FREE_RTOS) || defined (HARRIER_DEV_SUPPORT)
/* Get Cider index for a given port when using mac 400 port*/
static GT_STATUS mvHwsHarrierGlobalMac400ToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    devNum = devNum;

    if((portNum & 0x1) || portNum >= 40)
    {
        /* the MACs are all even numbers */
        return GT_NO_SUCH;
    }

    if(HWS_IS_PORT_MULTI_SEGMENT(portMode))
    {
        ciderIndexPtr->ciderUnit = portNum / MV_HWS_HARRIER_GOP_PORT_NUM_CNS;

        switch (portNum % MV_HWS_HARRIER_GOP_PORT_NUM_CNS)
        {
        case 0:
                ciderIndexPtr->expressChannelId     = 0;
                ciderIndexPtr->ciderIndexInUnit     = 0;
                break;
        case 8:
                if (ciderIndexPtr->ciderUnit == 2) /*ciderUnit 2 have only channel id 0 */
                {
                    return GT_NO_SUCH;
                }

                ciderIndexPtr->expressChannelId     = 8;
                ciderIndexPtr->ciderIndexInUnit     = 4;
                break;
        default:
                return GT_NO_SUCH;
        }
    }
    else
    {
        ciderIndexPtr->ciderUnit            = portNum / MV_HWS_HARRIER_GOP_PORT_NUM_CNS;
        ciderIndexPtr->expressChannelId     = portNum % MV_HWS_HARRIER_GOP_PORT_NUM_CNS;
        ciderIndexPtr->preemptionChannelId  = ciderIndexPtr->expressChannelId + 1;
        ciderIndexPtr->ciderIndexInUnit     = ciderIndexPtr->expressChannelId / 2;
    }

    return GT_OK;
}
#endif

#if !defined (MV_HWS_FREE_RTOS) || defined (AC5P_DEV_SUPPORT)
/* Get Cider index for a given port when using mac USX port*/
static GT_STATUS mvHwsHawkGlobalMacUsxToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    GT_U32 portIdx;
    devNum = devNum;
    portMode = portMode;

    if (((portNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS) < 2) || (portNum >= MV_HWS_AC5P_GOP_PORT_NUM_CNS * 2))
    {
        return GT_NO_SUCH;
    }
    else
    {
        ciderIndexPtr->expressChannelId = portNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS;
        portIdx = (portNum % MV_HWS_AC5P_GOP_PORT_NUM_CNS) - 2;
        ciderIndexPtr->ciderUnit = portNum / MV_HWS_AC5P_GOP_PORT_NUM_CNS;
        switch(portIdx % 8)
        {
            case 0:
                ciderIndexPtr->ciderIndexInUnit = 0;
                break;
            case 1:
                ciderIndexPtr->ciderIndexInUnit = 2;
                break;
            case 2:
                ciderIndexPtr->ciderIndexInUnit = 1;
                break;
            case 3:
                ciderIndexPtr->ciderIndexInUnit = 3;
                break;
            case 4:
                ciderIndexPtr->ciderIndexInUnit = 4;
                break;
            case 5:
                ciderIndexPtr->ciderIndexInUnit = 6;
                break;
            case 6:
                ciderIndexPtr->ciderIndexInUnit = 5;
                break;
            case 7:
                ciderIndexPtr->ciderIndexInUnit = 7;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"portNum[%d] is not implemented",
                    portNum);
        }
        ciderIndexPtr->ciderIndexInUnit += (portIdx / 8) * 8;
    }

    return GT_OK;
}

static GT_STATUS mvHwsHawkGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return mvHwsHawkGlobalMacUsxToLocalIndexConvert(devNum, portNum, portMode, ciderIndexPtr);
    }
    else
    {
        return mvHwsHawkGlobalMac400ToLocalIndexConvert(devNum, portNum, portMode, ciderIndexPtr);
    }
}
#endif

#if !defined (MV_HWS_FREE_RTOS) || defined (HARRIER_DEV_SUPPORT)
static GT_STATUS mvHwsHarrierGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
#ifndef HARRIER_DEV_SUPPORT
    if (mvHwsUsxModeCheck(devNum, portNum, portMode))
    {
        return GT_NO_SUCH;
    }
    else
#endif
    {
        return mvHwsHarrierGlobalMac400ToLocalIndexConvert(devNum, portNum, portMode, ciderIndexPtr);
    }
}
#endif

#if !defined (MV_HWS_FREE_RTOS) || defined (IRONMAN_DEV_SUPPORT)
static GT_STATUS mvHwsIronmanGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    devNum = devNum;
    portMode = portMode;

    if(portNum < 24) /* MAC 10G BR unit */
    {
        ciderIndexPtr->ciderUnit        = portNum / 8;          /* BR: 0,1,2 */
        ciderIndexPtr->ciderIndexInUnit = portNum % 8;
    }
    else if(portNum < 48) /* MAC 10G unit */
    {
        ciderIndexPtr->ciderUnit        = (portNum % 24) / 8;   /* NON-BR: 0,1,2*/
        ciderIndexPtr->ciderIndexInUnit = portNum % 8;
    }
    else if(portNum < 52)
    {
        ciderIndexPtr->ciderUnit        = 3 + (portNum - 48);   /* BR: 3,4,5,6 */
        ciderIndexPtr->ciderIndexInUnit = 0;
    }
    else
    {
        ciderIndexPtr->ciderUnit        = 3 + (portNum - 52);   /* NON-BR: 3,4,5 */
        ciderIndexPtr->ciderIndexInUnit = 0;
    }

    return GT_OK;
}
#endif

#if !defined (MV_HWS_FREE_RTOS)
static GT_STATUS mvHwsAasGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    devNum = devNum;
    portMode = portMode;

    /* not needed for AAS (was needed for MIF chanel configuration
       that is now out of L1 scope) */
    ciderIndexPtr->preemptionChannelId     = 0x3F;

    if (mvHwsMtipIsReducedPort(devNum, portNum))
    {
        ciderIndexPtr->ciderIndexInUnit = 0;
        ciderIndexPtr->ciderUnit = (portNum / MV_HWS_AAS_GOP_PORT_NUM_CNS) * 4 + (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS - 16);

#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
        if((hwsDeviceSpecInfo[devNum].numOfTiles == 1) && (portNum == MV_HWS_AAS_GOP_PORT_1TILE_CPU_0_CNS))
        {
            ciderIndexPtr->ciderUnit = 16;
        }
        else  if((hwsDeviceSpecInfo[devNum].numOfTiles == 2) && (portNum == MV_HWS_AAS_GOP_PORT_2TILE_CPU_0_CNS))
        {
            ciderIndexPtr->ciderUnit = 32;
        }
        else  if((hwsDeviceSpecInfo[devNum].numOfTiles == 2) && (portNum == MV_HWS_AAS_GOP_PORT_2TILE_CPU_1_CNS))
        {
            ciderIndexPtr->ciderUnit = 33;
        }
#endif
        return GT_OK;
    }


#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3)
    if(portNum >= MV_HWS_AAS_GOP_PORT_NUM_CNS * 4 * hwsDeviceSpecInfo[devNum].numOfTiles)
    {
        return GT_NO_SUCH;
    }
#endif

    if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) < 16)
    {
#if 0
        if(HWS_IS_PORT_MODE_400G(portMode) || HWS_IS_PORT_MODE_800G(portMode))
        {
            if((portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8 == 0)
            {
                ciderIndexPtr->ciderIndexInUnit = 8;
            }
            else
            {
                return GT_NO_SUCH;
            }
        }
#endif

        ciderIndexPtr->ciderIndexInUnit = (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) % 8;
        ciderIndexPtr->ciderUnit        = (portNum / MV_HWS_AAS_GOP_PORT_NUM_CNS) * 2 + (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS) / 8;
    }
#if 0
    else
    {
        if(HWS_IS_PORT_MODE_400G_R8(portMode)||
           HWS_IS_PORT_MODE_400G_R4(portMode)||
           HWS_IS_PORT_MODE_800G_R8(portMode))
        {
            return GT_NO_SUCH;
        }
        ciderIndexPtr->ciderUnit        = (portNum % MV_HWS_AAS_GOP_PORT_NUM_CNS - 16) + 4 * (portNum / MV_HWS_AAS_GOP_PORT_NUM_CNS);
        ciderIndexPtr->ciderIndexInUnit = 0;
    }
#endif

    return GT_OK;
}
#endif


GT_STATUS mvHwsGlobalMacToLocalIndexConvert
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT MV_HWS_HAWK_CONVERT_STC *ciderIndexPtr
)
{
    ciderIndexPtr->preemptionChannelId = 0x3F; /* max value that indicates - no preemptive channel */
#if !defined (MV_HWS_FREE_RTOS)
    if (hwsDeviceSpecInfo[devNum].devType == AC5P)
    {
        return mvHwsHawkGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
    }
    else if (hwsDeviceSpecInfo[devNum].devType == AC5X)
    {
        return mvHwsPhoenixGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
    }
    else if(hwsDeviceSpecInfo[devNum].devType == Harrier)
    {
        return mvHwsHarrierGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
    }
    else if(hwsDeviceSpecInfo[devNum].devType == Ironman)
    {
        return mvHwsIronmanGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
    }
    else if(hwsDeviceSpecInfo[devNum].devType == AAS)
    {
        return mvHwsAasGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }
#elif defined (HARRIER_DEV_SUPPORT)
    return mvHwsHarrierGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
#elif defined (IRONMAN_DEV_SUPPORT)
    return mvHwsIronmanGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
#elif defined (AC5P_DEV_SUPPORT)
    return mvHwsHawkGlobalMacToLocalIndexConvert(devNum,portNum,portMode,ciderIndexPtr);
#else
    return GT_NOT_SUPPORTED;
#endif
}


#ifndef RAVEN_DEV_SUPPORT

/**
* @internal mvHwsUsxExtSerdesGroupParamsGet function
* @endinternal
*
* @brief  USX Serdes group parameters get
*
* @param[in] devNum                - system device number
* @param[in] portNum               - physical port number
* @param[in] portMode              - port mode
* @param[out] *firstPortInGroupPtr - (pointer to) first port in USX SD group
* @param[out] *nextPortStepPtr     - (pointer to) next port step
* @param[out] *numOfPortsPtr       - (pointer to) number of ports in USX SD group
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsUsxExtSerdesGroupParamsGet
(
    IN GT_U8                    devNum,
    IN GT_U32                   portNum,
    IN MV_HWS_PORT_STANDARD     portMode,
    OUT GT_U32                  *firstPortInGroupPtr,
    OUT GT_U32                  *nextPortStepPtr,
    OUT GT_U32                  *numOfPortsPtr
)
{
    GT_STATUS rc;
    GT_U32 firstPortShift = 0;
    MV_HWS_HAWK_CONVERT_STC convertIdx;
    GT_U32 usxStep = 1;
    GT_U32 startPort = portNum;
    GT_U32 numOfPortsInGroup;

    if((firstPortInGroupPtr == NULL) ||
       (nextPortStepPtr == NULL) ||
       (numOfPortsPtr == NULL))
    {
        return GT_BAD_PTR;
    }
#ifndef HARRIER_DEV_SUPPORT
    if (mvHwsUsxModeCheck(devNum, portNum, portMode) == GT_FALSE)
    {
        return GT_BAD_STATE;
    }
#endif
    rc = mvHwsGlobalMacToLocalIndexConvert(devNum, portNum, portMode, &convertIdx);
    CHECK_STATUS(rc);
#if !defined (MV_HWS_FREE_RTOS)
    if(AC5P == HWS_DEV_SILICON_TYPE(devNum))
    {
        /* 2 Cider units with 24 ciderIndexInUnit ports */
        firstPortShift = 2 + convertIdx.ciderUnit *  MV_HWS_AC5P_GOP_PORT_NUM_CNS;
    }
    else if(AC5X == HWS_DEV_SILICON_TYPE(devNum))
    {
        /* 6 Cider units with 8 ciderIndexInUnit ports */
        firstPortShift = convertIdx.ciderUnit *  8;
    }
    else if(Ironman == HWS_DEV_SILICON_TYPE(devNum))
    {
        if(portNum < 24)
        {
            firstPortShift = convertIdx.ciderUnit *  8;
        }
        else if(portNum < 48)
        {
            firstPortShift = 24 + convertIdx.ciderUnit *  8;
        }
        else
        {
            firstPortShift = portNum;
        }
    }
    else if(Harrier == HWS_DEV_SILICON_TYPE(devNum))
    {
        firstPortShift = convertIdx.ciderUnit *  MV_HWS_HARRIER_GOP_PORT_NUM_CNS;
    }
    else
    {
        return GT_NOT_SUPPORTED;
    }
#elif defined (HARRIER_DEV_SUPPORT)
    firstPortShift = convertIdx.ciderUnit *  MV_HWS_HARRIER_GOP_PORT_NUM_CNS;
#elif defined (IRONMAN_DEV_SUPPORT)
    if(portNum < 24)
    {
        firstPortShift = convertIdx.ciderUnit *  8;
    }
    else if(portNum < 48)
    {
       firstPortShift = 24 + convertIdx.ciderUnit *  8;
    }
    else
    {
        firstPortShift = portNum;
    }
#elif defined (AC5P_DEV_SUPPORT)
    /* 2 Cider units with 24 ciderIndexInUnit ports */
    firstPortShift = 2 + convertIdx.ciderUnit *  MV_HWS_AC5P_GOP_PORT_NUM_CNS;
#else
    return GT_NOT_SUPPORTED;
#endif

    switch(portMode)
    {
        case _5G_DXGMII:
        case _10G_DXGMII:
        case _20G_DXGMII:
            numOfPortsInGroup = 2;
            if(AC5P == HWS_DEV_SILICON_TYPE(devNum))
            {
                usxStep = 2;
            }
            break;

        case  QSGMII:
        case _5G_QUSGMII:
        case _10G_QXGMII:
        case _20G_QXGMII:
            numOfPortsInGroup = 4;
            break;

        case _10G_OUSGMII:
        case _20G_OXGMII:
            numOfPortsInGroup = 8;
            break;

        default:
            /* no special treatment for single USX modes */
            numOfPortsInGroup = 1;
            break;
    }
    /* Zeroing of 1/2/3(depending on "numOfPortsInGroup") least significant bits */
    startPort = (convertIdx.ciderIndexInUnit / numOfPortsInGroup) * numOfPortsInGroup  + firstPortShift;

    *firstPortInGroupPtr = startPort;
    *nextPortStepPtr = usxStep;
    *numOfPortsPtr = numOfPortsInGroup;

    return GT_OK;
}
#endif

/**
* @internal mvHwsRegDbInit function
* @endinternal
*
* @brief   DB init.
*
* @param[in] devNum                  - system device number
* @param[in] unitId                  - unit to be configured
* @param[in] entryPtr                - entry
*
* @retval GT_OK                      - on success
* @retval not GT_OK                  - on error
*/
GT_STATUS mvHwsRegDbInit
(
    IN GT_U8 devNum,
    IN MV_HWS_UNITS_ID unitId,
    IN const MV_HWS_REG_ADDR_FIELD_STC *entryPtr
)
{
    devNum = devNum;

    if(entryPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if(unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (HARRIER_DEV_SUPPORT) || defined (AC5P_DEV_SUPPORT) || defined (IRONMAN_DEV_SUPPORT)
    PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(mvHwsRegDb)[unitId] = (MV_HWS_REG_ADDR_FIELD_STC*)entryPtr;
#endif
    return GT_OK;
}

/**
* @internal mvHwsRegDbEntryGet function
* @endinternal
*
* @brief   DB entry get.
*
* @param[in] devNum                  - system device number
* @param[in] unitId                  - unit to be configured
* @param[out] entryPtr               - (pointer to) requested DB entry
*
* @retval GT_OK                      - on success
* @retval not GT_OK                  - on error
*/
GT_STATUS mvHwsRegDbEntryGet
(
    IN GT_U8 devNum,
    IN MV_HWS_UNITS_ID unitId,
    OUT MV_HWS_REG_ADDR_FIELD_STC **entryPtr
)
{
    devNum = devNum;

    if(entryPtr == NULL)
    {
        return GT_BAD_PTR;
    }
    if(unitId >= LAST_UNIT)
    {
        return GT_BAD_PARAM;
    }

#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined (HARRIER_DEV_SUPPORT) || defined (AC5P_DEV_SUPPORT) || defined (IRONMAN_DEV_SUPPORT)
    *entryPtr = (PRV_NON_SHARED_LAB_SERVICES_DIR_SILICON_IF_SRC_GLOBAL_VAR_GET(mvHwsRegDb)[unitId]);
#endif
    return GT_OK;
}

/**
* @internal mvHwsHWAccessLock function
* @endinternal
*
* @brief   Protection Definition
*         =====================
*         some registers cannot be accessed by more than one
*         client concurrently Concurrent access might result in
*         invalid data read/write.
*         Multi-Processor Environment This case is protected by
*         HW Semaphore HW Semaphore is defined based in MSYS /
*         CM3 resources In case customer does not use MSYS / CM3
*         resources, the customer will need to implement its own
*         HW Semaphore This protection is relevant ONLY in case
*         Service CPU Firmware is loaded to CM3
*
* @param[in] devNum                   - system device number
*
*          */
void mvHwsHWAccessLock
(
    IN GT_U8 devNum,
#if !defined (MV_HWS_REDUCED_BUILD_EXT_CM3) || defined(FALCON_DEV_SUPPORT)
    IN GT_U8 chipIndex,
#endif
    IN GT_U8 hwSem
)
{
    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
    if (hwSem < MV_SEMA_LAST ) {
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
        /* When running on any Service CPU, HW semaphore always used */
#ifndef  FALCON_DEV_SUPPORT
        mvSemaLock(devNum, hwSem);
#else
        mvSemaLock(devNum, chipIndex, hwSem);
#endif
#else
        /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
        if ( mvHwsServiceCpuEnableGet(devNum) ) {
           mvSemaLock(devNum, chipIndex, hwSem);
        }
#endif
    }
}

/**
* @internal mvHwsHWAccessUnlock function
* @endinternal
*
* @brief   See description in mvHwsAvagoAccessLock API
* @param[in] devNum                   - system device number
*/
void mvHwsHWAccessUnlock
(
    IN GT_U8 devNum,
#if !defined MV_HWS_REDUCED_BUILD_EXT_CM3 || defined (FALCON_DEV_SUPPORT)
    IN GT_U8 chipIndex,
#endif
    IN GT_U8 hwSem
)
{
    /*
    ** HW Semaphore Protection Section
    ** ===============================
    */
    if (hwSem < MV_SEMA_LAST ) {
#ifdef MV_HWS_REDUCED_BUILD_EXT_CM3
        /* When running on any Service CPU, HW semaphore always used */
#ifdef  FALCON_DEV_SUPPORT
        mvSemaUnlock(devNum,chipIndex,hwSem);
#else
        mvSemaUnlock(devNum,hwSem);
#endif
#else
        /* When running on Host CPU, HW semaphore always used only when a service CPU is present */
        if ( mvHwsServiceCpuEnableGet(devNum) ) {
           mvSemaUnlock(devNum, chipIndex, hwSem);
        }
#endif
    }
}

#ifndef MV_HWS_REDUCED_BUILD
const MV_HWS_PORT_INIT_PARAMS   * hwsDevicePortsElementsSupModesCatalogGet
(
    IN GT_U8       devNum,
    GT_U32         portNum
)
{
    /* Get device's specific data from hwsDeviceSpecInfo Db*/

    const MV_HWS_PORT_INIT_PARAMS   **hwsPortsSupModes = PRV_PORTS_PARAMS_SUP_MODE_MAP(devNum);

    return hwsPortsSupModes[portNum];

}
#endif

GT_VOID hwsAvagoSerdesFreeInfoMapDb
(
    GT_U8 devNum
)
{
    if(NULL!=hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr)
    {
      hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr);
      hwsDeviceSpecInfo[devNum].avagoSerdesInfo.serdesToAvagoMapPtr = NULL;
    }

    if(NULL!=hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr)
    {
      hwsOsFreeFuncPtr(hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr);
      hwsDeviceSpecInfo[devNum].avagoSerdesInfo.avagoToSerdesMapPtr = NULL;
    }
}




