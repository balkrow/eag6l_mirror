/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlInterface.c
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
 * @brief Platform driver layer - Interface related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/interface/private/prvPdlPpReg.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>
#include <pdl/parser/pdlParser.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The interface type string to enum pairs */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdlInterfaceTypeStrToEnumPairs[] = {
    {"i2c", PDL_INTERFACE_EXTENDED_TYPE_I2C_E },
    {"smi", PDL_INTERFACE_EXTENDED_TYPE_SMI_E  },
    {"xsmi", PDL_INTERFACE_EXTENDED_TYPE_XSMI_E },
    {"gpio", PDL_INTERFACE_EXTENDED_TYPE_GPIO_E },
    {"smi_cpu", PDL_INTERFACE_EXTENDED_TYPE_SMI_CPU_E },
    {"external", PDL_INTERFACE_EXTENDED_TYPE_EXTERNAL_E },
    {"ppreg", PDL_INTERFACE_EXTENDED_TYPE_PPREG_E},
    {"mpd_phy", PDL_INTERFACE_EXTENDED_TYPE_MPD_E}
};
/** @brief   The interface type string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlInterfaceTypeStrToEnum = {prvPdlInterfaceTypeStrToEnumPairs, sizeof(prvPdlInterfaceTypeStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @fn  PDL_STATUS prvPdlInterfaceReadGroupRegister ( IN PDL_GROUPING_READ_INTERFACE_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr, OUT PDL_INTERFACE_TYP  * interfaceIdPtr )
 *
 * @brief   Prv pdl interface read group register (enumerate)
 *
 * @param    [in]   interfaceInfoPtr    Pointer to interface data.
 * @param    [out]  interfaceIdPtr      Pointer to interface id.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceReadGroupRegister(
    IN  PDL_GROUPING_READ_INTERFACE_GROUP_TYPE_PARAMS_STC     * interfaceInfoPtr,
    OUT PDL_INTERFACE_TYP                                     * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                                          pdlStatus;
    PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC  * i2cIfPtr;
    PDL_GPIO_READ_ADDRESS_PARAMS_STC                                  * gpioIfPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceInfoPtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (pdlIsFieldHasValue(interfaceInfoPtr->readInterfaceSelect_mask) == FALSE) {
        return PDL_BAD_PARAM;
    }

    switch (interfaceInfoPtr->readInterfaceSelect) {
        case PDL_INTERFACE_TYPE_INTERFACE_I2C_E:
            if (pdlIsFieldHasValue(interfaceInfoPtr->readI2CInterface_mask) == FALSE) {
                return PDL_BAD_PARAM;
            }

            i2cIfPtr = &interfaceInfoPtr->readI2CInterface.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType;
            pdlStatus = prvPdlI2cInterfaceRegister(i2cIfPtr->i2CBusId, i2cIfPtr->i2CAddress, i2cIfPtr->i2CAccess, i2cIfPtr->i2COffset, i2cIfPtr->i2CMask, i2cIfPtr->muxGroupType.muxList_PTR, i2cIfPtr->i2CTransactionType, i2cIfPtr->isI2CChannelUsed, i2cIfPtr->i2CChannelId, i2cIfPtr->i2CBaseAddress, interfaceIdPtr);
            break;
        case PDL_INTERFACE_TYPE_INTERFACE_GPIO_E:
            if (pdlIsFieldHasValue(interfaceInfoPtr->readGpioInterface_mask) == FALSE) {
                return PDL_BAD_PARAM;
            }

            gpioIfPtr = &interfaceInfoPtr->readGpioInterface.gpioReadAddressGroupType.gpioReadAddress;
            pdlStatus = prvPdlGpioInterfaceRegister(gpioIfPtr->gpioReadDeviceNumber, gpioIfPtr->gpioReadPinNumber, interfaceInfoPtr->readGpioInterface.gpioReadValue, interfaceIdPtr);
            break;
        default:
            pdlStatus = PDL_BAD_STATE;
    }

    PDL_CHECK_STATUS(pdlStatus);
    return pdlStatus;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceWriteGroupRegister ( IN PDL_GROUPING_WRITE_INTERFACE_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr, OUT PDL_INTERFACE_TYP  * interfaceIdPtr )
 *
 * @brief   Prv pdl interface write group register (enumerate)
 *
 * @param    [in]   interfaceInfoPtr    Pointer to interface data.
 * @param    [out]  interfaceIdPtr      Pointer to interface id.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlInterfaceWriteGroupRegister(
    IN  PDL_GROUPING_WRITE_INTERFACE_GROUP_TYPE_PARAMS_STC    * interfaceInfoPtr,
    OUT PDL_INTERFACE_TYP                                     * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                                          pdlStatus;
    PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC  * i2cIfPtr;
    PDL_GPIO_WRITE_ADDRESS_PARAMS_STC                                 * gpioIfPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceInfoPtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (pdlIsFieldHasValue(interfaceInfoPtr->writeInterfaceSelect_mask) == FALSE) {
        return PDL_BAD_PARAM;
    }

    switch (interfaceInfoPtr->writeInterfaceSelect) {
        case PDL_INTERFACE_TYPE_INTERFACE_I2C_E:
            if (pdlIsFieldHasValue(interfaceInfoPtr->writeI2CInterface_mask) == FALSE) {
                return PDL_BAD_PARAM;
            }

            i2cIfPtr = &interfaceInfoPtr->writeI2CInterface.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType;
            pdlStatus = prvPdlI2cInterfaceRegister(i2cIfPtr->i2CBusId, i2cIfPtr->i2CAddress, i2cIfPtr->i2CAccess, i2cIfPtr->i2COffset, i2cIfPtr->i2CMask, i2cIfPtr->muxGroupType.muxList_PTR, i2cIfPtr->i2CTransactionType, i2cIfPtr->isI2CChannelUsed, i2cIfPtr->i2CChannelId, i2cIfPtr->i2CBaseAddress, interfaceIdPtr);
            break;

        case PDL_INTERFACE_TYPE_INTERFACE_GPIO_E:
            if (pdlIsFieldHasValue(interfaceInfoPtr->writeGpioInterface_mask) == FALSE) {
                return PDL_BAD_PARAM;
            }

            gpioIfPtr = &interfaceInfoPtr->writeGpioInterface.gpioWriteAddressGroupType.gpioWriteAddress;
            pdlStatus = prvPdlGpioInterfaceRegister(gpioIfPtr->gpioWriteDeviceNumber, gpioIfPtr->gpioWritePinNumber, interfaceInfoPtr->writeGpioInterface.gpioWriteValue, interfaceIdPtr);
            break;

        default:
            pdlStatus = PDL_BAD_STATE;
    }
    PDL_CHECK_STATUS(pdlStatus);
    return pdlStatus;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceGpioWriteGroupRegister ( IN PDL_GROUPING_GPIO_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr, IN  UINT_32 writeValue, OUT PDL_INTERFACE_TYP  * interfaceIdPtr )
 *
 * @brief   Prv pdl interface gpio write group register (enumerate)
 *
 * @param    [in]   interfaceInfoPtr    Pointer to interface data.
 * @param    [in]   writeValue          Value to use
 * @param    [out]  interfaceIdPtr      Pointer to interface id.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlInterfaceGpioWriteGroupRegister(
    IN  PDL_GROUPING_GPIO_WRITE_ADDRESS_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr,
    IN  UINT_32                                                 writeValue,
    OUT PDL_INTERFACE_TYP                                     * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_GPIO_WRITE_ADDRESS_PARAMS_STC                     * gpioIfPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceInfoPtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    gpioIfPtr = &interfaceInfoPtr->gpioWriteAddress;
    pdlStatus = prvPdlGpioInterfaceRegister(gpioIfPtr->gpioWriteDeviceNumber, gpioIfPtr->gpioWritePinNumber, writeValue, interfaceIdPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return pdlStatus;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceGpioReadGroupRegister ( IN PDL_GROUPING_GPIO_READ_ADDRESS_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr, IN  UINT_32 readValue, OUT PDL_INTERFACE_TYP  * interfaceIdPtr )
 *
 * @brief   Prv pdl interface gpio read group register (enumerate)
 *
 * @param    [in]   interfaceInfoPtr    Pointer to interface data.
 * @param    [in]   readValue          Value to use
 * @param    [out]  interfaceIdPtr      Pointer to interface id.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlInterfaceGpioReadGroupRegister(
    IN  PDL_GROUPING_GPIO_READ_ADDRESS_GROUP_TYPE_PARAMS_STC  * interfaceInfoPtr,
    IN  UINT_32                                                 readValue,
    OUT PDL_INTERFACE_TYP                                     * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_GPIO_READ_ADDRESS_PARAMS_STC                      * gpioIfPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceInfoPtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    gpioIfPtr = &interfaceInfoPtr->gpioReadAddress;
    pdlStatus = prvPdlGpioInterfaceRegister(gpioIfPtr->gpioReadDeviceNumber, gpioIfPtr->gpioReadPinNumber, readValue, interfaceIdPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return pdlStatus;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceI2CReadWriteFieldsGroupRegister ( IN PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr, OUT PDL_INTERFACE_TYP  * interfaceIdPtr )
 *
 * @brief   Prv pdl i2c interface read/write group register (enumerate)
 *
 * @param    [in]   interfaceInfoPtr    Pointer to interface data.
 * @param    [out]  interfaceIdPtr      Pointer to interface id.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceI2CReadWriteFieldsGroupRegister(
    IN  PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC      * interfaceInfoPtr,
    OUT PDL_INTERFACE_TYP                                                     * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                                          pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceInfoPtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlI2cInterfaceRegister(interfaceInfoPtr->i2CBusId,
                                           interfaceInfoPtr->i2CAddress,
                                           interfaceInfoPtr->i2CAccess,
                                           interfaceInfoPtr->i2COffset,
                                           interfaceInfoPtr->i2CMask,
                                           interfaceInfoPtr->muxGroupType.muxList_PTR,
                                           interfaceInfoPtr->i2CTransactionType,
                                           interfaceInfoPtr->isI2CChannelUsed,
                                           interfaceInfoPtr->i2CChannelId,
                                           interfaceInfoPtr->i2CBaseAddress,
                                           interfaceIdPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return pdlStatus;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceI2CReadWriteGroupRegister ( IN PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC * interfaceInfoPtr, OUT PDL_INTERFACE_TYP  * interfaceIdPtr )
 *
 * @brief   Prv pdl i2c interface read/write group register (enumerate)
 *
 * @param    [in]   interfaceInfoPtr    Pointer to interface data.
 * @param    [out]  interfaceIdPtr      Pointer to interface id.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceI2CReadWriteGroupRegister(
    IN  PDL_GROUPING_I2C_READ_WRITE_ADDRESS_FIELDS_GROUP_TYPE_PARAMS_STC  * interfaceInfoPtr,
    OUT PDL_INTERFACE_TYP                                                 * interfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                                          pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceInfoPtr == NULL || interfaceIdPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlI2cInterfaceRegister(interfaceInfoPtr->i2CBusId,
                                           interfaceInfoPtr->i2CAddress,
                                           interfaceInfoPtr->i2CAccess,
                                           interfaceInfoPtr->i2COffset,
                                           interfaceInfoPtr->i2CMask,
                                           interfaceInfoPtr->muxGroupType.muxList_PTR,
                                           interfaceInfoPtr->i2CTransactionType,
                                           interfaceInfoPtr->isI2CChannelUsed,
                                           interfaceInfoPtr->i2CChannelId,
                                           interfaceInfoPtr->i2CBaseAddress,
                                           interfaceIdPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return pdlStatus;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceHwGetValue ( IN PDL_INTERFACE_TYPE_ENT interfaceType, IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr )
 *
 * @brief   Prv pdl interface hardware get value
 *
 *
 * @param [in]      interfaceType   Type of the interface.
 * @param [in]      interfaceId     Identifier for the interface.
 * @param [in,out]  dataPtr         If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlInterfaceHwGetValue(
    IN  PDL_INTERFACE_TYPE_ENT              interfaceType,
    IN  PDL_INTERFACE_TYP                   interfaceId,
    OUT UINT_32                           * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    switch (interfaceType) {
        case PDL_INTERFACE_TYPE_INTERFACE_GPIO_E:
            return prvPdlGpioHwGetValue(interfaceId, PDL_INTERFACE_GPIO_OFFSET_INPUT_E, dataPtr);
        case PDL_INTERFACE_TYPE_INTERFACE_I2C_E:
            return prvPdlI2CHwGetValue(interfaceId, dataPtr);
        case PDL_INTERFACE_TYPE_INTERFACE_PPREG_E:
            return prvPdlPpRegHwGetValue(interfaceId, dataPtr);
        default:
            return PDL_ERROR;
    }

    return PDL_OK;
}
/*$ END OF prvPdlInterfaceHwGetValue */

/**
 * @fn  PDL_STATUS pdlInterfaceDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Interface debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlInterfaceDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlInterfaceDebugFlag = state;
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlI2CInterfaceMuxDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl I2C Interface Mux debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CInterfaceMuxDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlI2CInterfaceMuxDebugFlag = state;
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlInterfaceHwSetValue ( IN PDL_INTERFACE_TYPE_ENT interfaceType, IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 data );
 *
 * @brief   Prv pdl interface hardware set value
 *
 * @param [in]  interfaceType   - Type of the interface.
 * @param [in]  interfaceId     - Identifier for the interface.
 * @param [in]  data            - data that will be written to HW.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlInterfaceHwSetValue(
    IN  PDL_INTERFACE_TYPE_ENT          interfaceType,
    IN PDL_INTERFACE_TYP                interfaceId,
    IN UINT_32                          data
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    switch (interfaceType) {
        case PDL_INTERFACE_TYPE_INTERFACE_GPIO_E:
            return prvPdlGpioHwSetValue(interfaceId, PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, data);
        case PDL_INTERFACE_TYPE_INTERFACE_I2C_E:
            return prvPdlI2CHwSetValue(interfaceId, data);
        case PDL_INTERFACE_TYPE_INTERFACE_PPREG_E:
            return prvPdlPpRegHwSetValue(interfaceId, data);
        default:
            return PDL_ERROR;
    }
    return PDL_OK;
}
/*$ END OF prvPdlInterfaceHwSetValue */

/**
 * @fn  PDL_STATUS pdlInterfaceCountGet ( IN PDL_INTERFACE_EXTENDED_TYPE_ENT type, OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of interfaces
 *
 * @param [in] type      interface type
 * @param [out] countPtr Number of smi/xsmi interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlInterfaceCountGet(
    IN PDL_INTERFACE_EXTENDED_TYPE_ENT   type,
    OUT UINT_32                         *countPtr
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

    switch (type) {
        case PDL_INTERFACE_EXTENDED_TYPE_GPIO_E:
            return prvPdlGpioCountGet(countPtr);
        case PDL_INTERFACE_EXTENDED_TYPE_I2C_E:
            return prvPdlI2cCountGet(countPtr);
        case PDL_INTERFACE_EXTENDED_TYPE_SMI_E:
        case PDL_INTERFACE_EXTENDED_TYPE_XSMI_E:
            return pdlSmiXsmiCountGet(countPtr);
        case PDL_INTERFACE_EXTENDED_TYPE_SMI_CPU_E:
        default:
            return PDL_BAD_PARAM;
    }

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlInterfaceInit ( IN PDL_OS_INIT_TYPE_ENT initType )
 *
 * @brief   Init interface module
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlInterfaceInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    return PDL_OK;
}
/*$ END OF pdlInterfaceInit */

/**
 * @fn  PDL_STATUS pdlInterfaceDestroy ( void )
 *
 * @brief   release all memory allocated by this module
 *
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlInterfaceDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    return PDL_OK;
}
/*$ END OF pdlInterfaceDestroy */
