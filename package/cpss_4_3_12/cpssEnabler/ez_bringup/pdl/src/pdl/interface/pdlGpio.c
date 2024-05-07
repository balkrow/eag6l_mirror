/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlGpio.c
 * @copyright
 *    (c), Copyright (C) 2023, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 *
 * @brief Platform driver layer - Gpio related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/common/pdlTypes.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdl/parser/pdlParser.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlGpioDebug.h>
#include <pdl/cpu/pdlCpu.h>
#ifdef LINUX_HW
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/*! Zero array helper */
#define PDL_PP_INIT_REGS_ZERO_ARRAY       {{0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0},{0, 0, 0, 0, 0, 0}}
/*! Zero array helper */
#define PDL_PP_TYPE_REGS_ZERO_ARRAY       {{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0},{0, 0}}
/** @brief   The pdl gpio database */
static PRV_PDLIB_DB_TYP           prvPdlGpioDb;
/** @brief   The pdl gpio memory CPU registers */
static UINTPTR                  pdlGpioMemCpuRegisters = 0;
#ifdef LINUX_HW
static UINTPTR                  pdlGpioMemCpuBase = 0;
static UINTPTR                  pdlGpioMemMppRegisters = 0;
static UINTPTR                  pdlGpioMemMppBase = 0;
#endif
/** @brief   Type of the pdl gpio CPU */
static PDL_CPU_FAMILY_TYPE_ENT  pdlGpioCpuType;
/** @brief   The pdl gpio mpp mask[ 8] */
UINT_32                         pdlGpioMppMask[8] = {0xF, 0xF0, 0xF00, 0xF000, 0xF0000, 0xF00000, 0xF000000, 0xF0000000};
BOOLEAN                         pdlMppCpuInitialized = FALSE;
BOOLEAN                         pdlMppInitialized = FALSE;
UINT_32                         pdlGpioPpMppInitbaseAddr[PDL_MAX_PP_NUM] = { 0x0 };
UINT_32                         pdlGpioPpGpioInitbaseAddr[PDL_MAX_PP_NUM] = { 0x0 };
UINT_32                         pdlGpioCpuRegMppbaseAddr = 0x0;
UINT_32                         pdlGpioCpuRegGpiobaseAddr = 0x0;
SIZE_T                          pdlGpioCpuRegMppSize = 0;
SIZE_T                          pdlGpioCpuRegCpuSize = 0;

/** @brief   The pdl gpio offset string to enum pairs[] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlGpioOffsetStrToEnumPairs[] = {
    {"output", PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E},
    {"type", PDL_INTERFACE_GPIO_OFFSET_TYPE_E},
    {"blinking", PDL_INTERFACE_GPIO_OFFSET_BLINKING_E},
    {"input", PDL_INTERFACE_GPIO_OFFSET_INPUT_E},
};
/** @brief   The pdl gpio offset string to enum */
static PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlGpioOffsetStrToEnum = {pdlGpioOffsetStrToEnumPairs, sizeof(pdlGpioOffsetStrToEnumPairs) / sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

#ifdef LINUX_HW
static INT_32                          prvPdlGpioMemfd = -1;
static INT_32                          prvPdlGpioMppMemfd = -1;
#endif

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/
/**
 * @fn  PDL_STATUS prvPdlGpioHwGetValue ( IN UINT_32 dev, IN UINT_32 pinNumber, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl gpio hardware get value
 *
 * @param   dev                 The device number.
 * @param   pinNumber           The pin number.
 * @param   offset              The offset.
 * @param   dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlGpioPinHwGetValue(
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    OUT UINT_32                               * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char                              * offsetStr;
    UINT_32                             gpioOutputRegister, data, pinNormalized;
    PDL_STATUS                          pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlibLibEnumToStrConvert(&pdlGpioOffsetStrToEnum, offset, &offsetStr);

    if (dev == PDL_GPIO_CPU_DEV) {
        if (pdlGpioMemCpuRegisters == 0) {
#ifdef LINUX_HW
            return PDL_NOT_IMPLEMENTED;
#else
            return PDL_OK;
#endif
        }
        PDL_GPIO_GET_BASE_REG_MAC(pinNumber, gpioOutputRegister);
        if (gpioOutputRegister == 0) {
            return PDL_ERROR;
        }

        gpioOutputRegister += offset;
        prvPdlLock(PDL_OS_LOCK_TYPE_CPU_GPIO_E);
        data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + gpioOutputRegister));
        prvPdlUnlock(PDL_OS_LOCK_TYPE_CPU_GPIO_E);
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "reading CPU MPP#%d from address [0x%x] value is [0x%x] offset type [%s]\n", pinNumber, (pdlGpioMemCpuRegisters + gpioOutputRegister), data, offsetStr);
    }
    /* packet processor GPIO */
    else if (dev < PDL_MAX_PP_NUM) {
        PDL_GPIO_GET_BASE_REG_MAC(pinNumber, gpioOutputRegister);
        if (gpioOutputRegister == 0) {
            return PDL_ERROR;
        }

        gpioOutputRegister += offset;
        pdlStatus = prvPdlCmRegRead((GT_U8)dev, (pdlGpioPpGpioInitbaseAddr[dev] + gpioOutputRegister), PDL_MASK_32_BIT, &data);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "reading dev#%d MPP#%d from address [0x%x] value is [0x%x] offset type [%s]\n", dev, pinNumber, gpioOutputRegister, data, offsetStr);
    }
    else {
        return PDL_OUT_OF_RANGE;
    }
    /* apply the pin number mask */
    pinNormalized = pinNumber % 32;
    *dataPtr = (data & (1 << pinNormalized)) >> pinNormalized;
    return PDL_OK;
}

/*$ END OF prvPdlGpioPinHwGetValue */

/**
 * @fn  PDL_STATUS prvPdlGpioHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl gpio hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      The offset.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlGpioHwGetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    OUT UINT_32                               * dataPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    UINT_8                                      pinNumber, dev;
    PDL_INTERFACE_GPIO_STC                    * gpioPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    gpioKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlGpioDb, (void*) &gpioKey, (void**) &gpioPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* init MPP before access if needed */
    if (gpioPtr->dev == PDL_GPIO_CPU_DEV) {
        if (pdlMppCpuInitialized == FALSE) {
            pdlStatus = pdlGpioCpuMppInit();
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    else {
        if (pdlMppInitialized == FALSE) {
            pdlStatus = pdlGpioMppInit();
            PDL_CHECK_STATUS(pdlStatus);
        }
    }

    dev = gpioPtr->dev;
    pinNumber = gpioPtr->pinNumber;
    return prvPdlGpioPinHwGetValue(dev, pinNumber, offset, dataPtr);
}

/*$ END OF prvPdlGpioHwGetValue */

/**
 * @fn  PDL_STATUS prvPdlGpioPinHwSetValue ( IN UINT_32 dev, IN UINT_32 pinNumber, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, IN UINT_32 data )
 *
 * @brief   Prv pdl gpio hardware set value
 *
 * @param   dev         The development.
 * @param   pinNumber   The pin number.
 * @param   offset      The offset.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlGpioPinHwSetValue(
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data can be 0 or 1*/
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     value = 0, gpioOutputRegister, pinNormalized;
    char                                      * offsetStr;
    PDL_STATUS                                  pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (data > 1) {
        return PDL_BAD_VALUE;
    }

    pinNormalized = pinNumber % 32;
    prvPdlibLibEnumToStrConvert(&pdlGpioOffsetStrToEnum, offset, &offsetStr);

    if (dev == PDL_GPIO_CPU_DEV) {
        if (pdlGpioMemCpuRegisters == 0) {
#ifdef LINUX_HW
            return PDL_NOT_IMPLEMENTED;
#else
            return PDL_OK;
#endif
        }
        PDL_GPIO_GET_BASE_REG_MAC(pinNumber, gpioOutputRegister);
        if (gpioOutputRegister == 0) {
            return PDL_ERROR;
        }
        gpioOutputRegister += offset;

        prvPdlLock(PDL_OS_LOCK_TYPE_CPU_GPIO_E);
        value |= *((volatile UINT_32*)(pdlGpioMemCpuRegisters + gpioOutputRegister)) & ~(1 << pinNormalized);
        value |= data << pinNormalized;
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "writing CPU MPP#%d value=[%d] to address [0x%x] offset type [%s]\n", pinNumber, data, (pdlGpioMemCpuRegisters + gpioOutputRegister), offsetStr);
        *((volatile UINT_32*)(pdlGpioMemCpuRegisters + gpioOutputRegister)) = value;
        prvPdlUnlock(PDL_OS_LOCK_TYPE_CPU_GPIO_E);
    }
    /* packet processor GPIO */
    else {
        PDL_GPIO_GET_BASE_REG_MAC(pinNumber, gpioOutputRegister);
        if (gpioOutputRegister == 0) {
            return PDL_ERROR;
        }
        if (dev >= PDL_MAX_PP_NUM) {
        	PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "dev [%d] out of bounds\n",dev);
        	return PDL_OUT_OF_RANGE;
        }
        gpioOutputRegister += offset;
        value = data << pinNormalized;
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "writing PP#%d MPP#%d value=[0%d] to address [0x%x] offset type [%s]\n", dev, pinNumber, data, (pdlGpioPpGpioInitbaseAddr[dev] + gpioOutputRegister), offsetStr);
        pdlStatus = prvPdlCmRegWrite((GT_U8)dev, (pdlGpioPpGpioInitbaseAddr[dev] + gpioOutputRegister), 1 << pinNormalized, value);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlGpioPinHwSetValue */

/**
 * @fn  PDL_STATUS prvPdlGpioHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, IN UINT_32 data )
 *
 * @brief   Pdl gpio hardware set value
 *
 * @param   interfaceId Identifier for the interface.
 * @param   offset      The offset.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlGpioHwSetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data will can be 0 or 1*/
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    UINT_32                                     dev, pin;
    PDL_INTERFACE_GPIO_STC                    * gpioPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    gpioKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlGpioDb, (void*) &gpioKey, (void**) &gpioPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* init MPP before access if needed */
    if (gpioPtr->dev == PDL_GPIO_CPU_DEV) {
        if (pdlMppCpuInitialized == FALSE) {
            pdlStatus = pdlGpioCpuMppInit();
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    else {
        if (pdlMppInitialized == FALSE) {
            pdlStatus = pdlGpioMppInit();
            PDL_CHECK_STATUS(pdlStatus);
        }
    }

    dev = gpioPtr->dev;
    pin = gpioPtr->pinNumber;

    return prvPdlGpioPinHwSetValue(dev, pin, offset, data);
}
/*$ END OF prvPdlGpioHwSetValue */

/**
 * @fn  PDL_STATUS pdlGpioDebugHwSetValue ( IN UINT_32 dev, IN UINT_32 pinNumber, IN  PDL_INTERFACE_GPIO_OFFSET_ENT offset, IN UINT_32 data )
 *
 * @brief   Wrap pdl gpio hardware set value
 *
 * @param   dev         The development.
 * @param   pinNumber   The pin number.
 * @param   offset      The gpio type.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlGpioDebugHwSetValue(
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data can be 0 or 1*/
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    return prvPdlGpioPinHwSetValue(dev, pinNumber, offset, data);
}
/*$ END OF pdlGpioDebugHwSetValue */


/**
 * @fn  PDL_STATUS PdlGpioDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_GPIO_STC * attributesPtr )
 *
 * @brief   Pdl gpio debug database get attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlGpioDebugDbGetAttributes(
    IN  PDL_INTERFACE_TYP                             interfaceId,
    OUT PDL_INTERFACE_GPIO_STC                      * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
    PDL_INTERFACE_GPIO_STC                     *gpioEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    gpioKey.interfaceId = interfaceId;
    pdlStatus = prvPdlibDbFind(prvPdlGpioDb, (void *)&gpioKey, (void **)&gpioEntryPtr);
    if (pdlStatus == PDL_OK) {
        memcpy(attributesPtr, gpioEntryPtr, sizeof(PDL_INTERFACE_GPIO_STC));
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioEntryPtr->dev);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioEntryPtr->initialValue);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioEntryPtr->pinNumber);
        IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, gpioEntryPtr->pushValue);
    }

    return pdlStatus;
}
/*$ END OF PdlGpioDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: PdlGpioDebugDbSetAttributes
*
* DESCRIPTION:  set GPIO attributes
*               update GPIO relevant information for given interfaceId
*               based on information from XML
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS PdlGpioDebugDbSetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_GPIO_STC * attributesPtr )
 *
 * @brief   Pdl gpio debug database set attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlGpioDebugDbSetAttributes(
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_GPIO_STC             * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
    PDL_INTERFACE_GPIO_STC                     *gpioEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    gpioKey.interfaceId = interfaceId;
    pdlStatus = prvPdlibDbFind(prvPdlGpioDb, (void *)&gpioKey, (void **)&gpioEntryPtr);
    if (pdlStatus != PDL_OK) {
        return pdlStatus;
    }
    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_DEVICE) {
        gpioEntryPtr->dev = attributesPtr->dev;
    }
    if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_PIN_NUMBER) {
        gpioEntryPtr->pinNumber = attributesPtr->pinNumber;
    }
    if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_TYPE) {
        gpioEntryPtr->type = attributesPtr->type;
    }

    return pdlStatus;
}
/*$ END OF PdlGpioDebugDbSetAttributes */

/**
 * @fn  PDL_STATUS prvPdlGpioInterfaceRegister ( IN UINT_8 devNum, IN UINT_8 pinNum, IN UINT_8 pushValue, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register gpio interface usage
 *
 * @param [in]  devNum          device number
 * @param [in]  pinNum          pin number
 * @param [in]  pushValue       push value if exists
 * @param [out] interfaceIdPtr  Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioInterfaceRegister(
    IN UINT_8               devNum,
    IN UINT_8               pinNum,
    IN UINT_8               pushValue,
    OUT PDL_INTERFACE_TYP * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
    PDL_INTERFACE_GPIO_STC                      gpioEntry, *gpioEntryPtr;

    PDL_MPP_GPIO_PINS_PARAMS_STC                params;
    PDL_MPP_DEVICE_LIST_PARAMS_STC            * mppDevPtr;
    PDL_MPP_DEVICE_LIST_KEYS_STC                mppDevKey;
    PDL_PIN_LIST_PARAMS_STC                   * mppPinPtr;
    PDL_PIN_LIST_KEYS_STC                       mppPinKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    memset(&gpioEntry, 0, sizeof(gpioEntry));

    if (pdlProjectParamsGet(PDL_FEATURE_ID_MPP_GPIO_PINS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params) == PDL_OK) {
        mppDevKey.mppDeviceNumber = devNum;
        pdlStatus = prvPdlibDbFind(params.mppDeviceList_PTR, (void*)&mppDevKey, (void **)&mppDevPtr);
        if (pdlStatus == PDL_OK) {
            mppPinKey.pinNumber = pinNum;
            pdlStatus = prvPdlibDbFind(mppDevPtr->pinList_PTR, (void *)&mppPinKey, (void **)&mppPinPtr);
            if (pdlStatus == PDL_OK) {
                gpioEntry.initialValue = mppPinPtr->pinInitialValue;
                gpioEntry.type = mppPinPtr->pinMode;
            }
        }
    }

    pdlStatus = prvPdlibDbGetNumOfEntries(prvPdlGpioDb, &gpioKey.interfaceId);
    PDL_CHECK_STATUS(pdlStatus);
    *interfaceIdPtr = gpioKey.interfaceId;

    gpioEntry.dev = devNum;
    gpioEntry.pinNumber = pinNum;
    gpioEntry.pushValue = pushValue;

    pdlStatus = prvPdlibDbAdd(prvPdlGpioDb, (void *)&gpioKey, (void **)&gpioEntry, (void **)&gpioEntryPtr);
    return pdlStatus;
}
/*$ END OF prvPdlGpioInterfaceRegister */

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioMmapRegion
*
* DESCRIPTION:   Memory Map the given address using /dev/mem
*
* PARAMETERS:
*
*
*****************************************************************************/
#ifdef LINUX_HW
/**
 * @fn  static PDL_STATUS prvPdlGpioMmapRegion ( IN void )
 *
 * @brief   Prv pdl gpio CPU type initialize
 *
 * @param [in]  regaddr       register address
 * @param [in]  size          size to mmap
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlGpioMmapRegion(
    IN  UINT_32          regaddr,
    IN  SIZE_T              size
)
{
    UINT_32 pageSz, pageOffset, szPageMap;
    UINT_32 addrOffset;

    /* memory map CPU GPIO */
    prvPdlGpioMemfd = open(PDL_MEM_DEV_MEM_NAME, O_RDWR | O_SYNC);

    if (prvPdlGpioMemfd < 0) {
        return PDL_MEM_INIT_ERROR;
    }

    pageSz = sysconf(_SC_PAGESIZE);
    pageOffset = regaddr & (pageSz - 1);

    if (pageSz >= (pageOffset + size)) {
        szPageMap = pageSz;
    }
    else {
        szPageMap = pageSz * ((size % pageSz) + ((float)size / (float)pageSz ? 1 : 0));
    }

    addrOffset = regaddr & ~(off_t)(pageSz - 1);
    pdlGpioMemCpuBase = (UINTPTR) mmap((void*) NULL                /* Addr */,
                                       szPageMap    /* length */,
                                       PROT_READ | PROT_WRITE           /* protection */,
                                       MAP_SHARED                       /* flags */,
                                       prvPdlGpioMemfd                  /* file descriptor */,
                                       (off_t)addrOffset                         /* offset */);

    if (pdlGpioMemCpuBase == (UINTPTR)MAP_FAILED) {
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "mmap failed for addr %x size %u !!\n", regaddr, size);
        return PDL_ERROR;
    }
    pdlGpioMemCpuRegisters = pdlGpioMemCpuBase + pageOffset;
    pdlGpioCpuRegCpuSize = szPageMap;

    return PDL_OK;
}
#endif

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioMppMmapRegion
*
* DESCRIPTION:   Memory Map the given address using /dev/mem
*
* PARAMETERS:
*
*
*****************************************************************************/

#ifdef LINUX_HW
/**
 * @fn  static PDL_STATUS prvPdlGpioMppMmapRegion ( IN void )
 *
 * @brief   Prv pdl gpio CPU type initialize
 *
 * @param [in]  regaddr       register address
 * @param [in]  size          size to mmap
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlGpioMppMmapRegion(
    IN  UINT_32          regaddr,
    IN  SIZE_T              size
)
{
    UINT_32 pageSz, pageOffset, szPageMap;
    UINT_32 addrOffset;

    /* memory map CPU GPIO */
    prvPdlGpioMppMemfd = open(PDL_MEM_DEV_MEM_NAME, O_RDWR | O_SYNC);

    if (prvPdlGpioMppMemfd < 0) {
        return PDL_MEM_INIT_ERROR;
    }

    pageSz = sysconf(_SC_PAGESIZE);
    pageOffset = regaddr & (pageSz - 1);

    if (pageSz >= (pageOffset + size)) {
        szPageMap = pageSz;
    }
    else {
        szPageMap = pageSz * (size % pageSz + ((float)size / (float)pageSz ? 1 : 0));
    }

    addrOffset = regaddr & ~(off_t)(pageSz - 1);
    pdlGpioMemMppBase = (UINTPTR) mmap((void*) NULL                /* Addr */,
                                       szPageMap    /* length */,
                                       PROT_READ | PROT_WRITE           /* protection */,
                                       MAP_SHARED                       /* flags */,
                                       prvPdlGpioMppMemfd                  /* file descriptor */,
                                       (off_t)addrOffset                         /* offset */);

    if (pdlGpioMemMppBase == (UINTPTR)MAP_FAILED) {
        PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "mmap failed for addr %x size %u !!\n", regaddr, size);
        return PDL_ERROR;
    }
    pdlGpioMemMppRegisters = pdlGpioMemMppBase + pageOffset;
    pdlGpioCpuRegMppSize = szPageMap;

    return PDL_OK;
}
#endif

/* ***************************************************************************
* FUNCTION NAME: prvPdlGpioCpuTypeInit
*
* DESCRIPTION:   Initialize CPU type
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  static PDL_STATUS prvPdlGpioCpuTypeInit ( IN void )
 *
 * @brief   Prv pdl gpio CPU type initialize
/ *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlGpioCpuTypeInit(
    IN          void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
#ifdef LINUX_HW
    UINT_32 len, i;
    char buf[2000], cpu_part[100]={0}, *tmp_PTR;
    char cpu_info_path[] = "/proc/cpuinfo";
    FILE* fd = NULL;
    char cpu_msys_HW_identifier[] = "Marvell Msys";
    char cpu_axp_HW_identifier[]  = "Marvell Armada XP";
    char cpu_a38x_HW_identifier[] = "Marvell Armada 38";
    char cpu_A55_cpupart_identifier[] = "0xd05";
#endif
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#define MIN(a,b) (((a)<(b))?(a):(b))
#ifdef LINUX_HW
    fd = fopen(cpu_info_path, "r");
    if (NULL == fd) {
        pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
    }
    else {
        len = fread(buf, 1, sizeof(buf), fd);
        buf[MIN(len, sizeof(buf)-1)] = '\0';
        tmp_PTR = strstr (buf, "CPU part");
        if (tmp_PTR != NULL) {
            for (i=0; tmp_PTR[i] && tmp_PTR[i]!='\n'; i++) {
                cpu_part[i] = tmp_PTR[i];
            }
        }
        if (strstr(buf, cpu_msys_HW_identifier) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
        }
        else if (strstr(buf, cpu_axp_HW_identifier) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_AXP_E;
        }
        else if (strstr(buf, cpu_a38x_HW_identifier) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_A38X_E;
        }
        else if (strstr(cpu_part, cpu_A55_cpupart_identifier) != NULL) {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_ARMv8_E;
        }
        else {
            pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
        }
        fclose(fd);
    }
#else
    pdlGpioCpuType = PDL_CPU_FAMILY_TYPE_MSYS_E;
#endif
    return PDL_OK;
}

/*$ END OF prvPdlGpioCpuTypeInit */

/* ***************************************************************************
* FUNCTION NAME: pdlGpioCpuMppInit
*
* DESCRIPTION:   Init CPU only MPPs
*                CPU MPPs can be initialized in earlier init stages (they're only memory accesses) by calling this API
*                can be called by application after pdlInit has been called (otherwise PDL_NOT_INITIALIZED is returned)
*                in case application doesn't explicitly call this API. it will be called on the first access to a CPU GPIO
*                PP MPPs can't be initialized in early stages - they need CPSS to complete initialization
*                in case no external call is done to initialize CPU only MPPs, they'll initialized together with PP MPPs
*
* PARAMETERS:
*
*****************************************************************************/


PDL_STATUS pdlGpioCpuMppInit(
    IN  void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
#ifdef LINUX_HW
    UINT_32                                     i, data;
    PDL_INTERFACE_GPIO_STC                    * gpioEntryPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
#endif
    UINT_8                                      devNum;
    UINT_32                                     pinNumber, relPinNumber, mppWord, gpioWord;
    UINT_32                                     cpuMppMask[PDL_GPIO_MPP_NUM_OF_INIT_REGS] = {0}, cpuInoutWord[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}; 
    UINT_32                                     cpuInterruptMask[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, cpuInPolarityData[PDL_GPIO_NUM_OF_TYPE_REGS] = {0};
    UINT_32                                     cpuInitMask[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, cpuOutputWord[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, cpuHwreadMask[PDL_GPIO_NUM_OF_TYPE_REGS] = {0}, cpuOutputMask[PDL_GPIO_NUM_OF_TYPE_REGS] = {0};
    PDL_STATUS                                  pdlStatus, list_status2;
    PDL_MPP_GPIO_PINS_PARAMS_STC                params;
    PDL_MPP_DEVICE_LIST_PARAMS_STC            * mppDevPtr;
    PDL_MPP_DEVICE_LIST_KEYS_STC                mppDevKey;
    PDL_PIN_LIST_PARAMS_STC                   * mppPinPtr;
    PDL_PIN_LIST_KEYS_STC                       mppPinKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlMppCpuInitialized == TRUE) {
        return PDL_ALREADY_EXIST;
    }

    if (pdlProjectParamsGet(PDL_FEATURE_ID_MPP_GPIO_PINS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params) == PDL_OK) {
        mppDevKey.mppDeviceNumber = PDL_GPIO_CPU_DEV;
        devNum = mppDevKey.mppDeviceNumber;
        pdlStatus = prvPdlibDbFind(params.mppDeviceList_PTR, (void*)&mppDevKey, (void **)&mppDevPtr);
        if (pdlStatus == PDL_OK) {
            if (mppDevPtr->cpuGpioRegistersRegionBaseAddress_mask) {
                pdlGpioCpuRegGpiobaseAddr = mppDevPtr->cpuGpioRegistersRegionBaseAddress;
            }
            else {
                pdlGpioCpuRegGpiobaseAddr = PDL_CPU_REGISTERS_REGION;
            }

            if (mppDevPtr->cpuMppRegistersRegionBaseAddress_mask) {
                pdlGpioCpuRegMppbaseAddr = mppDevPtr->cpuMppRegistersRegionBaseAddress;
            }
            else {
                pdlGpioCpuRegMppbaseAddr = PDL_CPU_REGISTERS_REGION;
            }

#ifdef LINUX_HW
            pdlStatus = prvPdlGpioMppMmapRegion(pdlGpioCpuRegMppbaseAddr, PDL_CPU_REGISTERS_REGION_SIZE);
            if (pdlStatus != PDL_OK) {
                return pdlStatus;
            }

            if (pdlGpioCpuRegGpiobaseAddr != pdlGpioCpuRegMppbaseAddr) {
                pdlStatus = prvPdlGpioMmapRegion(pdlGpioCpuRegGpiobaseAddr, PDL_CPU_REGISTERS_REGION_SIZE);
                if (pdlStatus != PDL_OK) {
                    return pdlStatus;
                }
            }
            else {
                /* Base address is the same can be accessed in the same page*/
                pdlGpioMemCpuRegisters = pdlGpioMemMppRegisters;
                pdlGpioCpuRegCpuSize = pdlGpioCpuRegMppSize;
            }
#endif
            for (list_status2 = prvPdlibDbGetFirst(mppDevPtr->pinList_PTR, (void **)&mppPinPtr);
                list_status2 == PDL_OK;
                list_status2 = prvPdlibDbGetNext(mppDevPtr->pinList_PTR, (void *)&mppPinKey, (void **)&mppPinPtr)) {

                mppPinKey = mppPinPtr->list_keys;
                pinNumber = mppPinPtr->list_keys.pinNumber;
                relPinNumber = pinNumber % 32;
                mppWord = pinNumber / 8;
                gpioWord = pinNumber / 32;
                if (mppWord >= PDL_GPIO_MPP_NUM_OF_INIT_REGS || gpioWord >= PDL_GPIO_NUM_OF_TYPE_REGS ||
                    (devNum != PDL_GPIO_CPU_DEV && devNum >= PDL_MAX_PP_NUM)) {
                    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "bad size mppWord=[%d] gpioWord=[%d] dev=[%d] !!! MPP AREN'T INITIALIZED !!!\n",
                                                                mppWord, gpioWord, devNum);
                    return PDL_BAD_SIZE;
                }
                cpuMppMask[mppWord] |= PDL_GPIO_MPP_GET_INIT_MASK_VALUE(pinNumber);
                /* input gpios bit should be set to 1 */
                if (mppPinPtr->pinMode == PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_PIN_E) {
                    cpuInoutWord[gpioWord] |= 1 << relPinNumber;
                }
                else if (mppPinPtr->pinMode == PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_INTERRUPT_PIN_E){
                    /* set the pin as INPUT only */
                    cpuInoutWord[gpioWord] |= 1 << relPinNumber;
                    /* unmask the pin for clear the INT value and unmask the INT */
                    cpuInterruptMask[gpioWord] |= 1 << relPinNumber;
                    /* Input polarity */
                    if (mppPinPtr->pinPolaritySwap == TRUE){
                        cpuInPolarityData[gpioWord] |= 1 << relPinNumber;
                    }
                }
                else {
                    switch (mppPinPtr->pinInitialValue) {
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_0_E:
                            /*cpuOutputWord[gpioWord] &= ~(0 << relPinNumber);*/
                            cpuInitMask[gpioWord] |= 1 << relPinNumber;
                            break;
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_1_E:
                            cpuOutputWord[gpioWord] |= 1 << relPinNumber;
                            cpuInitMask[gpioWord] |= 1 << relPinNumber;
                            break;
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_HW_E:
                            cpuHwreadMask[gpioWord] |= 1 << relPinNumber;
                            /*cpuInitMask[gpioWord] |= 1 << relPinNumber;*/
                            break;
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_LAST_E:
                            return PDL_BAD_PARAM;
                    }
                    cpuOutputMask[gpioWord] |= 1 << relPinNumber;
                }
            }
#ifdef LINUX_HW
            /* write MPP configuration */
            if (pdlGpioMemMppRegisters == 0) {
                return PDL_NOT_IMPLEMENTED;
            }
            /* configure MPP registers (set 4 bits to 0 in case GPIO is used) */

            for (i = 0; i < PDL_GPIO_MPP_NUM_OF_INIT_REGS; i++) {
                /* each 4 bits represent MPP. write 0 with mask F to set as MPP */
                /* write only on Valid Mask */
                if (cpuMppMask[i]) {
                    data = 0;
                    data |= *((volatile UINT_32*)(pdlGpioMemMppRegisters + i * 4)) & ~cpuMppMask[i];
                    *((volatile UINT_32*)(pdlGpioMemMppRegisters + i * 4)) = data;
                }
            }

            if (pdlGpioMemCpuRegisters == 0) {
                return PDL_NOT_IMPLEMENTED;
            }
            /* set i/o type */
            data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_0_31));
            /* First, raise Input pins */
            data |= cpuInoutWord[0];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_0_31)) = data;
            /* Then, clear Output pins */
            data &= ~(cpuOutputMask[0]);
            /* Finally, update */
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_0_31)) = data;

            data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_32_63));
            /* First, raise Input pins */
            data |= cpuInoutWord[1];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_32_63)) = data;
            /* Then, clear Output pins */
            data &= ~(cpuOutputMask[1]);
            /* Finally, update */
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_SET_TYPE_MPP_32_63)) = data;

            /* write default output value */
            data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_0_31)) & ~cpuInitMask[0];
            data |= cpuOutputWord[0] & cpuInitMask[0];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_0_31)) = data;

            data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_32_63)) & ~cpuInitMask[1];
            data |= cpuOutputWord[1] & cpuInitMask[1];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_OUT_MPP_32_63)) = data;
        

            /* clear INT-Cause bit */
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_INT_CAUSE_MPP_0_31)) &= ~(cpuInterruptMask[0]);
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_INT_CAUSE_MPP_32_63)) &= ~(cpuInterruptMask[1]);
        
            /* set input-polarity swap */
            data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_POLARITY_MPP_0_31)) & cpuInterruptMask[0];
            data |= cpuInPolarityData[0];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_POLARITY_MPP_0_31)) |= data;

            data = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_POLARITY_MPP_32_63)) & cpuInterruptMask[1];
            data |= cpuInPolarityData[1];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_POLARITY_MPP_32_63)) |= data;

            /* unmask interrupts */
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_INT_MASK_MPP_0_31)) |= cpuInterruptMask[0];
            *((volatile UINT_32*)(pdlGpioMemCpuRegisters + PDL_GPIO_INT_MASK_MPP_32_63)) |= cpuInterruptMask[1];

            /* update initial values for GPIO interfaces */
            pdlStatus = prvPdlibDbGetNextKey(prvPdlGpioDb, NULL, (void*) &gpioKey);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlibDbFind(prvPdlGpioDb, (void*) &gpioKey, (void**) &gpioEntryPtr);
                PDL_CHECK_STATUS(pdlStatus);
                if (gpioEntryPtr->dev != PDL_GPIO_CPU_DEV) {
                    /* only handle CPU GPIOs here */
                    pdlStatus = prvPdlibDbGetNextKey(prvPdlGpioDb, (void*) &gpioKey, (void*) &gpioKey);
                    continue;
                }
                if (gpioEntryPtr->type == PDL_MPP_PIN_MODE_TYPE_GPIO_WRITABLE_PIN_E || gpioEntryPtr->type == PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_WRITABLE_PIN_E) {
                    switch (gpioEntryPtr->initialValue) {
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_0_E:
                            gpioEntryPtr->initialValue = 0;
                            break;
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_1_E:
                            gpioEntryPtr->initialValue = 1;
                            break;
                        case PDL_MPP_PIN_INIT_VALUE_TYPE_HW_E:
                            pdlStatus = prvPdlGpioPinHwGetValue(gpioEntryPtr->dev, gpioEntryPtr->pinNumber, PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, &data);
                            PDL_CHECK_STATUS(pdlStatus);
                            gpioEntryPtr->initialValue = (UINT_8)data;
                            break;
                        default:
                            return PDL_BAD_PARAM;
                    }
                }
                else {
                    gpioEntryPtr->initialValue = 0;
                }
                pdlStatus = prvPdlibDbGetNextKey(prvPdlGpioDb, (void*) &gpioKey, (void*) &gpioKey);
            }
#endif /*LINUX_HW*/
        }
        pdlMppCpuInitialized = TRUE;
        return PDL_OK;
    }
    return PDL_NOT_INITIALIZED;
}
/*$ END OF pdlGpioCpuMppInit */

/* ***************************************************************************
* FUNCTION NAME: pdlGpioMppInit
*
* DESCRIPTION:   Init MPPs
*                SHOULD be called after data on all GPIO interfaces has been collected to DB
*                Writes 0 in case MPP is used to the MPP init register
*                and write I/O to the gpio input/output register
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlGpioMppInit ( IN void )
 *
 * @brief   Public pdl gpio mpp initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlGpioMppInit(
    IN  void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
#ifdef LINUX_HW
    UINT_32                                     data, i, j;
    PDL_INTERFACE_GPIO_STC                    * gpioEntryPtr;
    PDL_INTERFACE_PRV_KEY_STC                   gpioKey;
#endif
    PDL_STATUS                                  pdlStatus;
    UINT_32                                     maxDev = 0, pinNumber, relPinNumber, gpioWord, ppInoutWord[PDL_MAX_PP_NUM][PDL_GPIO_NUM_OF_TYPE_REGS] = PDL_PP_TYPE_REGS_ZERO_ARRAY;
    UINT_32                                     mppWord, ppMppMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY; /*mpp word has to be set to 0xF for each GPIO that is being used  */
    UINT_32                                     ppOutputWord[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     ppHwreadMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     ppOutputMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     ppInitMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     ppInterruptMask[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    UINT_32                                     ppInPolarityData[PDL_MAX_PP_NUM][PDL_GPIO_MPP_NUM_OF_INIT_REGS] = PDL_PP_INIT_REGS_ZERO_ARRAY;
    PDL_STATUS                                  list_status1, list_status2;
    PDL_MPP_GPIO_PINS_PARAMS_STC                params;
    PDL_MPP_DEVICE_LIST_PARAMS_STC            * mppDevPtr;
    PDL_MPP_DEVICE_LIST_KEYS_STC                mppDevKey;
    PDL_PIN_LIST_PARAMS_STC                   * mppPinPtr;
    PDL_PIN_LIST_KEYS_STC                       mppPinKey;
    UINT_8                                      devNum;
#ifdef LINUX_HW
    BOOLEAN                                     ppMppsExist[PDL_MAX_PP_NUM] = {0};
#endif
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (pdlMppInitialized == TRUE) {
        return PDL_ALREADY_EXIST;
    }

    if (pdlMppCpuInitialized == FALSE) {
        pdlStatus = pdlGpioCpuMppInit();
        PDL_CHECK_STATUS(pdlStatus);
    }
    if (pdlProjectParamsGet(PDL_FEATURE_ID_MPP_GPIO_PINS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params) == PDL_OK) {
        for (list_status1 = prvPdlibDbGetFirst(params.mppDeviceList_PTR, (void **)&mppDevPtr);
             list_status1 == PDL_OK;
             list_status1 = prvPdlibDbGetNext(params.mppDeviceList_PTR, (void *)&mppDevKey, (void **)&mppDevPtr)) {

            mppDevKey = mppDevPtr->list_keys;
            devNum = mppDevKey.mppDeviceNumber;

            if (devNum == PDL_GPIO_CPU_DEV) {
                /* cpu mpp init is already done in prvPdlGpioCpuMppInit */
                continue;
            }
            else {
                if (devNum >= PDL_MAX_PP_NUM) {
                    return PDL_OUT_OF_RANGE;
                }
                /* uses PP API to set/get registers */
                if (mppDevPtr->ppMppInitBaseAddress_mask) {
                    pdlGpioPpMppInitbaseAddr[devNum] = mppDevPtr->ppMppInitBaseAddress;
                }
                else {
                    pdlGpioPpMppInitbaseAddr[devNum] = PDL_GPIO_PP_MPP_INIT_BASE_OFF;
                }

                if (mppDevPtr->ppGpioInitBaseAddress_mask) {
                    pdlGpioPpGpioInitbaseAddr[devNum] = mppDevPtr->ppGpioInitBaseAddress;
                }
                else {
                    pdlGpioPpGpioInitbaseAddr[devNum] =  PDL_GPIO_PP_GPIO_REG_BASE_OFF;
                }
            }

            for (list_status2 = prvPdlibDbGetFirst(mppDevPtr->pinList_PTR, (void **)&mppPinPtr);
                 list_status2 == PDL_OK;
                 list_status2 = prvPdlibDbGetNext(mppDevPtr->pinList_PTR, (void *)&mppPinKey, (void **)&mppPinPtr)) {

                mppPinKey = mppPinPtr->list_keys;

                pinNumber = mppPinPtr->list_keys.pinNumber;
                relPinNumber = pinNumber % 32;
                mppWord = pinNumber / 8;
                gpioWord = pinNumber / 32;
                if (mppWord >= PDL_GPIO_MPP_NUM_OF_INIT_REGS || gpioWord >= PDL_GPIO_NUM_OF_TYPE_REGS ||
                    (devNum != PDL_GPIO_CPU_DEV && devNum >= PDL_MAX_PP_NUM)) {
                    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "bad size mppWord=[%d] gpioWord=[%d] dev=[%d] !!! MPP AREN'T INITIALIZED !!!\n",
                                                                mppWord, gpioWord, devNum);
                    return PDL_BAD_SIZE;
                }
                if (devNum == PDL_GPIO_CPU_DEV) {
                    /* cpu mpp init is already done in prvPdlGpioCpuMppInit */
                    continue;
                }
                else {
                    if (devNum > maxDev) {
                        maxDev = devNum;
                    }
                    ppMppMask[devNum][mppWord] |= PDL_GPIO_MPP_GET_INIT_MASK_VALUE(pinNumber);
#ifdef LINUX_HW
                    ppMppsExist[devNum] = TRUE;
#endif
                    /* input gpios bit should be set to 1 */
                    if (mppPinPtr->pinMode == PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_PIN_E) {
                        ppInoutWord[devNum][gpioWord] |= 1 << relPinNumber;
                    }
                    else if (mppPinPtr->pinMode == PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_INTERRUPT_PIN_E){
                        /* set the pin as INPUT only */
                        ppInoutWord[devNum][gpioWord] |= 1 << relPinNumber;
                        /* unmask the pin for clear the INT value and unmask the INT */
                        ppInterruptMask[devNum][gpioWord] |= 1 << relPinNumber;
                        /* Input polarity */
                        if (mppPinPtr->pinPolaritySwap == TRUE){
                            ppInPolarityData[devNum][gpioWord] |= 1 << relPinNumber;
                        }
                    }
                    /* set default value for output GPIOs as the not pushed value */
                    else {
                        switch (mppPinPtr->pinInitialValue) {
                            case PDL_MPP_PIN_INIT_VALUE_TYPE_0_E:
                                /*ppOutputWord[devNum][gpioWord] &= ~(0 << relPinNumber);*/
                                ppInitMask[devNum][gpioWord] |= 1 << relPinNumber;
                                break;
                            case PDL_MPP_PIN_INIT_VALUE_TYPE_1_E:
                                ppOutputWord[devNum][gpioWord] |= 1 << relPinNumber;
                                ppInitMask[devNum][gpioWord] |= 1 << relPinNumber;
                                break;
                            case PDL_MPP_PIN_INIT_VALUE_TYPE_HW_E:
                                ppHwreadMask[devNum][gpioWord] |= 1 << relPinNumber;
                                /*ppInitMask[devNum][gpioWord] |= 1 << relPinNumber;*/
                                break;
                            case PDL_MPP_PIN_INIT_VALUE_TYPE_LAST_E:
                                return PDL_BAD_PARAM;
                        }
                        ppOutputMask[devNum][gpioWord] |= 1 << relPinNumber;
                    }
                }
            }
        }
    }

#ifdef LINUX_HW
    /* write MPP configuration */
    /* PP MPP */
    for (i = 0; i <= maxDev && i < PDL_MAX_PP_NUM; i++) {
        if (ppMppsExist[i] == FALSE) {
            /* No MPPs configured on this PP - skip */
            continue;
        }
        /* configure MPP registers (set 4 bits to 0 in case GPIO is used) */
        for (j = 0; j < PDL_GPIO_MPP_NUM_OF_INIT_REGS; j++) {
            /* each 4 bits represent MPP. write 0 with mask F to set as MPP */
            /* write only on valid mask */
            if (ppMppMask[i][j]) {
                pdlStatus = prvPdlCmRegWrite((GT_U8)i, pdlGpioPpMppInitbaseAddr[i] + j * 4, ppMppMask[i][j], 0);
                PDL_CHECK_STATUS(pdlStatus);
            }
        }
        /* set i/o type */
        pdlStatus = prvPdlCmRegRead((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_SET_TYPE_MPP_0_31), PDL_MASK_32_BIT, &data);
        PDL_CHECK_STATUS(pdlStatus);
        /* First, raise Input pins */
        data |= ppInoutWord[i][0];
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_SET_TYPE_MPP_0_31), PDL_MASK_32_BIT, data);
        PDL_CHECK_STATUS(pdlStatus);
        /* Then, clear Output pins */
        data &= ~(ppOutputMask[i][0]);
        /* Finally, update */
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_SET_TYPE_MPP_0_31), PDL_MASK_32_BIT, data);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlCmRegRead((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_SET_TYPE_MPP_32_63), PDL_MASK_32_BIT, &data);
        PDL_CHECK_STATUS(pdlStatus);
        /* First, raise Input pins */
        data |= ppInoutWord[i][1];
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_SET_TYPE_MPP_32_63), PDL_MASK_32_BIT, data);
        PDL_CHECK_STATUS(pdlStatus);
        /* Then, clear Output pins */
        data &= ~(ppOutputMask[i][1]);
        /* Finally, update */
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_SET_TYPE_MPP_32_63), PDL_MASK_32_BIT, data);
        PDL_CHECK_STATUS(pdlStatus);
                
        /* write default output value */
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_OUT_MPP_0_31), ppOutputMask[i][0], ppOutputWord[i][0]);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_OUT_MPP_32_63), ppOutputMask[i][1], ppOutputWord[i][1]);
        PDL_CHECK_STATUS(pdlStatus);
        
        /* clear INT-Cause bit */
        pdlStatus = prvPdlCmRegRead((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_INT_CAUSE_MPP_0_31), ppInterruptMask[i][0], &data);
        PDL_CHECK_STATUS(pdlStatus);
        data &= ~ppInterruptMask[i][0];
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_INT_CAUSE_MPP_0_31), ppInterruptMask[i][0], data);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlCmRegRead((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_INT_CAUSE_MPP_32_63), ppInterruptMask[i][1], &data);
        PDL_CHECK_STATUS(pdlStatus);
        data &= ~ppInterruptMask[i][1];
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_INT_CAUSE_MPP_32_63), ppInterruptMask[i][1], data);
        PDL_CHECK_STATUS(pdlStatus);

        /* set input-polarity swap */
        pdlStatus = prvPdlCmRegRead((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_POLARITY_MPP_0_31), ppInterruptMask[i][0], &data);
        PDL_CHECK_STATUS(pdlStatus);
        data |= ppInPolarityData[i][0];
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_POLARITY_MPP_0_31), ppInterruptMask[i][0], data);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlCmRegRead((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_POLARITY_MPP_32_63), ppInterruptMask[i][1], &data);
        PDL_CHECK_STATUS(pdlStatus);
        data |= ppInPolarityData[i][1];
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_POLARITY_MPP_32_63), ppInterruptMask[i][1], data);
        PDL_CHECK_STATUS(pdlStatus);

        /* unmask interrupts */
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_INT_MASK_MPP_0_31), ppInterruptMask[i][0], ppInterruptMask[i][0]);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlCmRegWrite((GT_U8)i, (pdlGpioPpGpioInitbaseAddr[i] + PDL_GPIO_INT_MASK_MPP_32_63), ppInterruptMask[i][1], ppInterruptMask[i][1]);
        PDL_CHECK_STATUS(pdlStatus);
        
    }

    /* update initial values for GPIO interfaces */
    pdlStatus = prvPdlibDbGetNextKey(prvPdlGpioDb, NULL, (void*) &gpioKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbFind(prvPdlGpioDb, (void*) &gpioKey, (void**) &gpioEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (gpioEntryPtr->dev == PDL_GPIO_CPU_DEV) {
            /* cpu mpp init is already done in prvPdlGpioCpuMppInit */
            pdlStatus = prvPdlibDbGetNextKey(prvPdlGpioDb, (void*) &gpioKey, (void*) &gpioKey);
            continue;
        }
        if (gpioEntryPtr->type == PDL_MPP_PIN_MODE_TYPE_GPIO_WRITABLE_PIN_E || gpioEntryPtr->type == PDL_MPP_PIN_MODE_TYPE_GPIO_READABLE_WRITABLE_PIN_E) {
            switch (gpioEntryPtr->initialValue) {
                case PDL_MPP_PIN_INIT_VALUE_TYPE_0_E:
                    gpioEntryPtr->initialValue = 0;
                    break;
                case PDL_MPP_PIN_INIT_VALUE_TYPE_1_E:
                    gpioEntryPtr->initialValue = 1;
                    break;
                case PDL_MPP_PIN_INIT_VALUE_TYPE_HW_E:
                    pdlStatus = prvPdlGpioPinHwGetValue(gpioEntryPtr->dev, gpioEntryPtr->pinNumber, PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, &data);
                    PDL_CHECK_STATUS(pdlStatus);
                    gpioEntryPtr->initialValue = (UINT_8)data;
                    break;
                default:
                    return PDL_BAD_PARAM;
            }
        }
        else {
            gpioEntryPtr->initialValue = 0;
        }
        pdlStatus = prvPdlibDbGetNextKey(prvPdlGpioDb, (void*) &gpioKey, (void*) &gpioKey);
    }

#endif /*LINUX_HW*/
    pdlMppInitialized = TRUE;
    return PDL_OK;
}

/*$ END OF pdlGpioMppInit */

/**
 * @fn  PDL_STATUS prvPdlGpioRegisterSet ( IN UINT_32      dev, IN UINT_32      regOffset, IN UINT_32      mask, IN UINT_32      data)
 *
 * @brief   Set value for a single Gpio register (used by pdlLed to set blinking registers)
 *
 * @param   dev [in]                 device (255 for CPU)
 * @param   regOffset [in]           offset for register that has to be set
 * @param   mask [in]                mask bitmap for the write operation
 * @param   data [in]                data that will be written to register
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioRegisterSet(
    IN UINT_32      dev, 
    IN UINT_32      regOffset, 
    IN UINT_32      mask,
    IN UINT_32      data
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    UINT_32                                     oldData, writeValue;
/****************************************************************************/
/*                     F U N C T I O N   L O G I C                          */
/****************************************************************************/
    if (dev == PDL_GPIO_CPU_DEV) {
        if (pdlGpioMemCpuRegisters == 0) {
#ifdef LINUX_HW
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "Cpu GPIO not implemented yet");
            return PDL_NOT_IMPLEMENTED;
#else
            return PDL_OK;
#endif
        }
        prvPdlLock(PDL_OS_LOCK_TYPE_CPU_GPIO_E);
        oldData = *((volatile UINT_32*)(pdlGpioMemCpuRegisters + regOffset));
        writeValue = (~mask & oldData) | (mask & data);
        *((volatile UINT_32*)(pdlGpioMemCpuRegisters + regOffset)) = writeValue;
        prvPdlUnlock(PDL_OS_LOCK_TYPE_CPU_GPIO_E);
    }
    else if (dev < PDL_MAX_PP_NUM) {
        if (pdlGpioPpGpioInitbaseAddr[dev] == 0) {
#ifdef LINUX_HW
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "Pp %d GPIO not implemented yet", dev);
            return PDL_NOT_IMPLEMENTED;
#else
            return PDL_OK;
#endif
        }
        else {
            pdlStatus = prvPdlCmRegWrite((GT_U8)dev, (pdlGpioPpGpioInitbaseAddr[dev] + regOffset), mask, data);
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    else {
        return PDL_OUT_OF_RANGE;
    }
    return PDL_OK;

}

/*$ END OF prvPdlGpioRegisterSet */


/**
 * @fn  PDL_STATUS prvPdlGpioCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of GPIO interfaces
 *
 * @param[out] countPtr Number of gpio interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlGpioCountGet(
    OUT UINT_32 * countPtr
)
{
    /****************************************************************************/
    /*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /****************************************************************************/

    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    if (countPtr == NULL) {
        return PDL_BAD_PTR;
    }

    return prvPdlibDbGetNumOfEntries(prvPdlGpioDb, countPtr);
}

/**
 * @fn  PDL_STATUS prvPdlGpioInit ( IN PDL_OS_INIT_TYPE_ENT initType )
 *
 * @brief   Init GPIO module
 *
 * @param   initType    type of init performed (on FULL init - CPU memory space is mapped)
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlGpioInit(
    IN PDL_OS_INIT_TYPE_ENT         initType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC                   dbAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (initType == PDL_OS_INIT_TYPE_FULL_E) {
        prvPdlGpioCpuTypeInit();
    }

    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_GPIO_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlGpioDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlGpioInit */

/**
 * @fn  PDL_STATUS prvPdlGpioDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    /* unmap CPU GPIO memory, skip if the base address is same as MPP */
    if (pdlGpioMemCpuRegisters && (pdlGpioMemCpuRegisters != pdlGpioMemMppRegisters)) {
        munmap((void *)pdlGpioMemCpuBase, pdlGpioCpuRegCpuSize /* length */);
    }

    if (pdlGpioMemMppRegisters) {
        munmap((void *)pdlGpioMemMppBase, pdlGpioCpuRegMppSize /* length */);
    }

    if (prvPdlGpioMemfd != -1) {
        close(prvPdlGpioMemfd);
    }
    if (prvPdlGpioMppMemfd != -1) {
        close(prvPdlGpioMppMemfd);
    }
#endif

    pdlStatus = prvPdlibDbDestroy(prvPdlGpioDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlGpioDestroy */
