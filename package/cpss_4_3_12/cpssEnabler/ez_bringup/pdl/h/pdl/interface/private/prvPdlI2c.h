/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\interface\private\prvpdli2c.h.
 *
 * @brief   Declares the prvpdli 2c class
 */

#ifndef __prvPdlI2ch

#define __prvPdlI2ch
/**
********************************************************************************
 * @file prvPdlI2c.h
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
 * @brief Platform driver layer - Private I2C related API (internal)
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/xml/private/prvXmlParser.h>
/**
* @addtogroup Interface
* @{
*/

/**
* @addtogroup I2C
* @{
*/

/**
 * @defgroup I2C_private I2C Private
 * @{defines private structures and function for I2C
*/


/*! Maximum message length for I2C transaction (in bytes) */
#define PDL_I2C_MAX_MSG_LEN               128

/*! I2C file descriptor name */
#define PDL_I2C_PREFIX_DEV_NAME           "/dev/i2c"

/*! offset is UINT_16*/
#define PDL_I2C_OFFSET_SIZE               PDL_I2C_OFFSET_1_BYTE_SIZE

/*! offset is UINT_8*/
#define PDL_I2C_OFFSET_1_BYTE_SIZE        1

/*! offset is UINT_16*/
#define PDL_I2C_OFFSET_2_BYTE_SIZE        2

/* XML parser tags */


/*! I2C main tag - I2C group */
#define PDL_I2C_TAG_NAME                "i2c-read-write-address"

/*! Tag for I2C bus id */
#define PDL_I2C_BUS_ID_TAG_NAME         "bus-id"

/*! Tag for I2C adress */
#define PDL_I2C_ADDRESS_TAG_NAME        "address"

/*! Tag for I2C offset */
#define PDL_I2C_OFFSET_TAG_NAME         "offset"

/*! Tag for I2C mask */
#define PDL_I2C_MASK_TAG_NAME           "mask"

/*! Tag for I2C access size */
#define PDL_I2C_ACCESS_SIZE_TAG_NAME    "access"


/*! Tag for access size of 1 byte */
#define PDL_I2C_ACCESS_SIZE_ONE_BYTE_VALUE      "one-byte"

/*! Tag for access size of 2 bytes */
#define PDL_I2C_ACCESS_SIZE_TWO_BYTE_VALUE      "two-bytes"

/*! Tag for access size of 4 bytes */
#define PDL_I2C_ACCESS_SIZE_FOUR_BYTE_VALUE     "four-bytes"



/*! I2C MUX main tag */
#define PDL_I2C_MUX_I2C_TAG_NAME            "i2c-mux"

/*! Tag for GPIO MUX main tag */
#define PDL_I2C_MUX_GPIO_TAG_NAME           "gpio-mux"

/*! Tag for GPIO MUX device number */
#define PDL_I2C_MUX_GPIO_DEVICE_TAG_NAME    "device_number"

/*! Tag for GPIO MUX pin number */
#define PDL_I2C_MUX_GPIO_PIN_TAG_NAME       "pin"

/*! Tag for I2C MUX number */
#define PDL_I2C_MUX_I2C_NUMBER_TAG_NAME     "mux-number"
/*! Tag for I2C MUX address */
#define PDL_I2C_MUX_I2C_ADDRESS_TAG_NAME    "mux-i2c-address"
/*! Tag for I2C MUX mask */
#define PDL_I2C_MUX_I2C_MASK_TAG_NAME       "mux-i2c-mask"
/*! Tag for I2C MUX offset */
#define PDL_I2C_MUX_I2C_OFFSET_TAG_NAME     "mux-i2c-offset"
/*! Tag for I2C MUX value  */
#define PDL_I2C_MUX_I2C_VALUE_TAG_NAME      "mux-i2c-value"
/*! Tag for I2C MUX bus id */
#define PDL_I2C_MUX_I2C_BUS_ID_NAME         "mux-i2c-bus-id"
/*! Tag for I2C MUX access (size) */
#define PDL_I2C_MUX_ACCESS_NAME             "mux-i2c-access"

#define PDL_I2C_MUX_TRANSACTION_TYPE_NAME  "mux-i2c-transaction-type"

#define PDL_I2C_BUS_ID_MAX                  255
#define PDL_I2C_MUX_SLEEP_MSEC              50      /* sleep duration after MUX configuration */

/**
 * @struct  PDL_I2C_ACCESS_SIZE_ENUM_STR_STC
 *
 * @brief   used for converting between access size string and access-size enum
 */

typedef struct {
    /** @brief   The name */
    char                                       * name;
    /** @brief   Size of the access */
    PDL_I2C_ACCESS_TYPE_ENT                     accessSize;
} PDL_I2C_ACCESS_SIZE_ENUM_STR_STC;

/**
* @internal prvPdlI2cInit
* @endinternal
* @brief  Initialize I2C Bus file descriptors
*
* @returns PDL_STATUS
*
*/

/**
 * @fn  PDL_STATUS pdlI2CHwGetBufferWithOffset ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr );
 *
 * @brief   get i2c buffer the size of length with given offset (used for SFP EEPROM read)
 *
 * @param       interfaceId Identifier for the interface.
 * @param [in]      i2cAddress  I2c address to read from can only be 0x50 or 0x51
 * @param [in]  offset      -   offset to start reading from.
 * @param [in]  length      -   length to read.
 * @param [out] dataPtr     -   data read.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlI2CHwGetBufferWithOffset(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  i2cAddress,
    IN  UINT_16                                 offset,
    IN  UINT_32                                 length,
    OUT void                                  * dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2cBusFdInit ( IN void );
 *
 * @brief   Prv pdl i 2c bus fd initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cBusFdInit(
    IN void
);

/**
 * @fn  PDL_STATUS prvPdlI2cBusFdDestroy ( IN void )
 *
 * @brief   Prv pdl i 2c bus fd deinitialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlI2cBusFdDestroy(
    IN void
);
/*$ END OF prvPdlI2cBusFdDestroy */

/**
 * @fn  PDL_STATUS prvPdlI2CConvertRel2Abs ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 relData, OUT UINT_32 * absDataPtr );
 *
 * @brief   convert relative data to absolute data using i2c interface mask
 *                 examples: mask is 0XC0 and data is 3 - absData = 0xC0
 *                           mask is 0xC0 and data is 2 - absData = 0x80 mask is 0xC0 and data is
 *                           1 - absData = 0x40 mask is 0xC0 and data is 0 - absData = 0
 *
 * @param [in]  interfaceId - interface id.
 * @param [in]  relData     - relative data.
 * @param [out] absDataPtr  - absolute data according to mask.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertRel2Abs(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 relData,
    OUT UINT_32                               * absDataPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2CConvertRel2AbsWithMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 relData,IN  UINT_32 mask ,OUT UINT_32 * absDataPtr )
 *
 * @brief   Prv pdl i 2 c convert relative 2 abs according to mask
 *
 * @param           interfaceId Identifier for the interface.
 * @param           relData     Information describing the relative.
 * @param           mask        mask needed.
 * @param [in,out]  absDataPtr  If non-null, the abs data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertRel2AbsWithMask(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 relData,
    IN  UINT_32                                 mask,
    OUT UINT_32                               * absDataPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2CConvertAbs2Rel ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 absData, OUT UINT_32 * relDataPtr );
 *
 * @brief   convert absolute data to relative data using i2c interface mask
 *                 examples: mask is 0XC0 and value is 0xC0 - 3 is returned
 *                           mask is 0xC0 and value is 0x80 - 2 is returned mask is 0xC0 and
 *                           value is 0x40 - 1 is returned
 *
 * @param [in]      interfaceId - interface id.
 * @param           absData     Information describing the abs.
 * @param [in,out]  relDataPtr  If non-null, the relative data pointer.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertAbs2Rel(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 absData,
    OUT UINT_32                               * relDataPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2CConvertAbs2RelWithMask (IN UINT_32 absData,IN  UINT_32 mask ,OUT UINT_32 * relDataPtr )
 *
 * @brief   Prv pdl i 2 c convert abs 2 relative according to mask
 *
 * @param           absData     Information describing the abs.
 * @param           mask        mask needed.
 * @param [in,out]  relDataPtr  If non-null, the relative data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertAbs2RelWithMask(
    IN  UINT_32                                 absData,
    IN  UINT_32                                 mask,
    OUT UINT_32                               * relDataPtr
);


/**
 * @fn  PDL_STATUS pdlI2CHwSetValueWithAddressOffsetMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_8 i2cAddress, IN UINT_32 offset, IN UINT_32 mask, IN UINT_32 data )
 *
 * @brief   set i2c value, address & offset & mask are given as parameter
 *
 * @param   interfaceId Identifier for the interface.
 * @param   i2cAddress  address
 * @param   offset      offset
 * @param   mask        mask
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlI2CHwSetValueWithAddressOffsetMask(
    IN  PDL_INTERFACE_TYP                       interfaceId,
	IN  UINT_8                                  i2cAddress,
    IN  UINT_32                                 offset,
    IN  UINT_32                                 mask,
    IN  UINT_32                                 data
);

/**
 * @fn  PDL_STATUS pdlI2CHwSetValueWithOffsetMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 offset, IN UINT_32 mask, IN UINT_32 data )
 *
 * @brief   set i2c value, offset & mask are given as parameter (SHOULD be used only by fan drivers)
 *
 * @param   interfaceId Identifier for the interface.
 * @param   offset      offset
 * @param   mask        mask
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CHwSetValueWithOffsetMask(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 offset,
    IN  UINT_32                                 mask,
    IN  UINT_32                                 data
);

/**
 * @fn  PDL_STATUS pdlI2CHwGetValueWithOffsetMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 offset, IN UINT_32 mask, OUT UINT_32 * dataPtr )
 *
 * @brief   get i2c value, offset & mask are given as parameter (SHOULD be used only by fan drivers)
 *
 * @param           interfaceId Identifier for the interface.
 * @param           offset      offset
 * @param           mask        mask
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CHwGetValueWithOffsetMask(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 offset,
    IN  UINT_32                                 mask,
    OUT UINT_32                               * dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2cReceiveWithOffset( IN UINT_8 slave_address, IN UINT_32 buffer_size, IN UINT_8 bus_id, IN UINT_32 offset_type, IN UINT_32 offset, OUT UINT_8 * buffer )
 *
 * @brief   Prv pdl i 2c receive with offset
 *
 * @param           slave_address   The slave address.
 * @param           buffer_size     Size of the buffer.
 * @param           bus_id          Identifier for the bus.
 * @param           offset_type     Type of the offset.
 * @param           offset          The offset.
 * @param [in,out]  buffer          If non-null, the buffer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS  prvPdlI2cReceiveWithOffset(
    IN  UINT_8                                    slave_address,   /* the target device slave address on I2C bus */
    IN  UINT_32                                   buffer_size,     /* buffer length */
    IN  UINT_8                                    bus_id,          /* the I2C bus id */
    IN  UINT_32                                   offset_type,     /* receive mode */
    IN  UINT_32                                   offset,          /* internal offset to read from */
    IN  PDL_I2C_TRANSACTION_TYPE_ENT              transaction_type,
    OUT UINT_8                                  * buffer           /* received buffer */
);

/**
 * @fn  PDL_STATUS prvPdlI2CHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr )
 *
 * @brief   Pdl i 2 c hardware get value
 *
 * @param [in]   interfaceId Identifier for the interface.
 * @param [out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CHwGetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    OUT UINT_32                               * dataPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2CHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 data )
 *
 * @brief   Pdl i 2 c hardware set value
 *
 * @param   interfaceId Identifier for the interface.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlI2CHwSetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 data
);

/**
 * @fn  PDL_STATUS prvPdlI2cBusSetUsed ( IN UINT_32 busId, IN  BOOLEAN   isUsed )
 *
 * @brief   Prv pdl i 2c bus set used flag.
 *
 * @param   busId   Identifier for the bus.
 * @param   isUsed  Indicates whether bus is used
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlI2cBusSetUsed(
    IN  UINT_32         busId,
    IN  BOOLEAN         isUsed
);

/**
 * @fn  PDL_STATUS prvPdlI2cCountGet ( OUT UINT_32 * countPtr )
 *
 * @brief   Gets number of i2c interfaces
 *
 * @param [out] countPtr Number of i2c interfaces
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlI2cCountGet(
    OUT UINT_32 * countPtr
);

/**
 * @fn  PDL_STATUS prvPdlI2cInterfaceRegister ( IN UINT_32  busId, IN UINT_8 i2cAddress, IN PDL_I2C_ACCESS_TYPE_ENT accessSize, IN UINT_16 offset, IN UINT_32 mask, IN PRV_PDLIB_DB_TYP muxDb, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register i2c interface usage
 *
 * @param [in]  busId           bus id (relevant when isChannelUsed == FALSE)
 * @param [in]  i2cAddress      address
 * @param [in]  accessSize      transaction size
 * @param [in]  offset          offset to use
 * @param [in]  mask            mask to apply
 * @param [in]  muxDb           Mux db handle
 * @param [in]  transactionType i2c transactionType
 * @param [in]  isChannelUsed   whether channel id is required for access
 * @param [in]  channelId       i2c channel us id (relevant when isChannelUsed == TRUE)
 * @param [in]  baseAddress     i2c base address (relevant when isChannelUsed == TRUE)
 * @param [out] interfaceIdPtr  Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlI2cInterfaceRegister(
    IN UINT_32                    busId,
    IN UINT_8                     i2cAddress,
    IN PDL_I2C_ACCESS_TYPE_ENT    accessSize,
    IN UINT_16                    offset,
    IN UINT_32                    mask,
    IN PRV_PDLIB_DB_TYP             muxDb,
    IN PDL_I2C_TRANSACTION_TYPE_ENT transactionType,
    IN BOOLEAN                   isChannelUsed,
    IN UINT_32                   channelId,
    IN UINT_32                   baseAddress,
    OUT PDL_INTERFACE_TYP      * interfaceIdPtr
);
/*$ END OF prvPdlI2cInterfaceRegister */


/**
 * @fn  PDL_STATUS prvPdlI2cInit ( IN void )
 *
 * @brief   Pdl i2c initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlI2cInit(
    IN  void
);
/*$ END OF prvPdlI2cInit */

/**
 * @fn  PDL_STATUS prvPdlI2cDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlI2cDestroy(
    void
);
/*$ END OF prvPdlI2cDestroy */

/* @}*/
/* @}*/

#endif
