/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\interface\private\prvpdlinterface.h.
 *
 * @brief   Declares the prvpdlinterface class
 */

#ifndef __prvPdlInterfaceh

#define __prvPdlInterfaceh
/**
********************************************************************************
 * @file prvPdlInterface.h
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
 * @brief Platform driver layer - Private Interface related API (internal)
 *
 * @version   1
********************************************************************************
*/

#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/lib/private/prvPdlLib.h>

/**
* @addtogroup Interface
* @{
*/

/**
 * @defgroup Interface_private Interface Private
 * @{defines private structures used to store interface information from XML inside db
*/
#define PDL_INTERFACE_INVALID_INTERFACE_ID      0xFFFFFFFF

/* size of above enumerator */
#define PDL_INTERFACE_I2C_ACCESS_SIZE_COUNT     3

/**
 * @struct PDL_INTERFACE_OFFSET_MASK_VALUE_STC
 *
 * @brief  offset/mask/value container
 */

typedef struct {
    /** @brief   The base address */
    UINT_32 baseAddress;
    /** @brief   The offset */
    UINT_32 offset;
    /** @brief   The mask */
    UINT_32 mask;
    /** @brief   The value */
    UINT_32 value;
} PDL_INTERFACE_OFFSET_MASK_VALUE_STC;

/**
 * @typedef UINT_32 PDL_INTERFACE_TYP
 *
 * @brief   Defines an alias representing the pdl interface typ
 */

/* internal driver id is reserved from 1000 */
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN            1000
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD          1000
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_POE            1001
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_PHY            1002
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_PP             1003
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_CPU            1004
#define PDL_INTERFACE_INTERNAL_DRIVER_ID_FAN            1005


/** @brief   The interface type string to enum */
extern PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlInterfaceTypeStrToEnum;

/**
 * @struct  PDL_INTERFACE_PRV_KEY_STC
 *
 * @brief   Interface key \n this structure is the key for searching interface in DB
 */

typedef struct {
    /** @brief   Identifier for the interface */
    PDL_INTERFACE_TYP                   interfaceId;
} PDL_INTERFACE_PRV_KEY_STC;

/**
 * @struct  PDL_INTERFACE_PRV_I2C_MUX_KEY_STC
 *
 * @brief   Interface key \n this structure is the key for searching interface in DB
 */

typedef struct {
    /** @brief   The multiplexer number */
    UINT_32                   muxNum;
} PDL_INTERFACE_PRV_I2C_MUX_KEY_STC;

/**
 * @struct PDL_INTERFACE_PRV_DB_STC
 *
 * @brief  Interface db information \n this structure contains all that is required in order to
 *         identify interface.
 */

typedef struct {
    /** @brief   db key */
    PDL_INTERFACE_PRV_KEY_STC           key;
    /** @brief   Read/write value depending on interface type */
    UINT_32                             optionalValue;

} PDL_INTERFACE_PRV_DB_STC;

/**
 * @struct PDL_INTERFACE_PRV_I2C_DB_STC
 *
 * @brief  I2C db information \n this structure contains all that is required in order to perform
 *         a full (logical) I2C transaction.
 */

typedef struct {
    /** @brief   Information describing the public */
    PDL_INTERFACE_I2C_STC                publicInfo;
    /** @brief   The multiplexer database */
    PRV_PDLIB_DB_TYP                       muxDb;

} PDL_INTERFACE_PRV_I2C_DB_STC;

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
);

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
);

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
);

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
);

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
);

/**
 * @fn  PDL_STATUS pdlInterfaceInit ( void )
 *
 * @brief   Init interface module
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlInterfaceInit(
    void
);
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
);
/*$ END OF pdlInterfaceDestroy */

/* @}*/
/* @}*/

#endif
