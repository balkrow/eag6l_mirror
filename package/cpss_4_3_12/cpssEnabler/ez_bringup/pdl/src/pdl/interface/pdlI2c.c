/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlI2c.c
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
 * @brief Platform driver layer - Button related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <pdl/parser/pdlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlI2CDebug.h>
#include <pdl/interface/pdlGpioDebug.h>
#ifdef LINUX_HW
#include    <linux/i2c.h>
#include    <linux/i2c-dev.h>
#include    <fcntl.h>
#include    <sys/ioctl.h>
/*#include    <stropts.h>*/
#include    <unistd.h>
extern int usleep(unsigned int);
#endif
#include <pdlib/lib/pdlLib.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl i2c database */
static PRV_PDLIB_DB_TYP           prvPdlI2cDb;
/** @brief   The pdl i2c bus fd arr[ pdl i2c bus identifier max] */
#ifdef LINUX_HW
static int                      pdlI2cBusFdArr[PDL_I2C_BUS_ID_MAX];
#endif

/**
 * @brief   Zero-based index of the 2c access size enum to name[ pdl interface i 2 c access size
 *          count]
 */

/** @brief   True if pdl I2C bus identifier is used[ pdl I2C bus identifier max] */
static BOOLEAN                  pdlI2cBusIdIsUsed[PDL_I2C_BUS_ID_MAX+1] = {0};
#ifdef LINUX_HW
/** @brief   The pdl I2C number of retries */
static int                      pdlI2cNumOfRetries = 20;
#endif /*LINUX_HW*/

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/
/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cTransmit
*
* DESCRIPTION:   transmit i2c value
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cTransmit ( IN UINT_8 slave_address, IN UINT_32 buffer_size, IN UINT_8 bus_id, IN UINT_8 * buffer )
 *
 * @brief   Prv pdl i2c transmit
 *
 * @param           slave_address   The slave address.
 * @param           buffer_size     Size of the buffer.
 * @param           bus_id          Identifier for the bus.
 * @param [in,out]  buffer          If non-null, the buffer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cTransmit(
    IN UINT_8     slave_address,   /* the target device slave address on I2C bus */
    IN UINT_32    buffer_size,     /* buffer length */
    IN UINT_8     bus_id,          /* the I2C bus id if only one bus then bus_id=0*/
    IN UINT_8   * buffer           /* transmited buffer */
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
#ifdef LINUX_HW
    int try, ret;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg;
    unsigned char buf[PDL_I2C_MAX_MSG_LEN];
    PDL_STATUS              pdlStatus;
#endif
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (buffer_size > PDL_I2C_MAX_MSG_LEN -1) {
        return PDL_I2C_ERROR;
    }
    memcpy(&buf, buffer, buffer_size);
    /*
        * write operation
        */
    i2c_data.msgs = &msg;
    i2c_data.nmsgs = 1;                     /* use one message structure */

    i2c_data.msgs[0].addr = slave_address;
    i2c_data.msgs[0].flags = 0;             /* don't need flags */
    i2c_data.msgs[0].len = buffer_size;
    i2c_data.msgs[0].buf = (__u8 *)buf;

    /*
        * ioctl() processes read & write.
        * Operation is determined by flags field of i2c_msg
        */
    try = 0;
    do {
        ret = ioctl(pdlI2cBusFdArr[bus_id], I2C_RDWR, &i2c_data);
        pdlStatus = ret < 0 ? PDL_I2C_ERROR : PDL_OK;
        if (pdlStatus==PDL_OK || try==(pdlI2cNumOfRetries-1)) break;
        try++;
        usleep(20000);
    }
    while (1);
    if (pdlStatus!=PDL_OK) {
        prvPdlI2cResultHandler(pdlStatus, slave_address, bus_id, buffer[0], TRUE);
    }
    return pdlStatus;

#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlI2cTransmit */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cReceive
*
* DESCRIPTION:   receive i2c value
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cReceive ( IN UINT_8 slave_address, IN UINT_32 buffer_size, IN UINT_8 bus_id, OUT UINT_8 * buffer )
 *
 * @brief   Prv pdl i 2c receive
 *
 * @param           slave_address   The slave address.
 * @param           buffer_size     Size of the buffer.
 * @param           bus_id          Identifier for the bus.
 * @param [in,out]  buffer          If non-null, the buffer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cReceive(
    IN  UINT_8     slave_address,   /* the target device slave address on I2C bus */
    IN  UINT_32    buffer_size,     /* buffer length */
    IN  UINT_8     bus_id,          /* the I2C bus id */
    OUT UINT_8   * buffer           /* received buffer */
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
#ifdef LINUX_HW
    int try, ret;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[1];
    PDL_STATUS              pdlStatus;
#endif
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    i2c_data.msgs = msg;
    i2c_data.nmsgs = 1;    /* two i2c_msg */

    i2c_data.msgs[0].addr = slave_address;
    i2c_data.msgs[0].flags = I2C_M_RD;      /* read command */
    i2c_data.msgs[0].len = buffer_size;
    i2c_data.msgs[0].buf = (__u8 *)buffer;


    try = 0;
    do {
        ret = ioctl(pdlI2cBusFdArr[bus_id], I2C_RDWR, &i2c_data);
        pdlStatus = ret < 0 ? PDL_I2C_ERROR : PDL_OK;
        if (pdlStatus==PDL_OK || try==(pdlI2cNumOfRetries-1)) break;
        try++;
        usleep(20000);
    }
    while (1);
    if (pdlStatus!=PDL_OK) {
        prvPdlI2cResultHandler(pdlStatus, slave_address, bus_id, buffer[0], FALSE);
    }
    return pdlStatus;
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlI2cReceive */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cReceiveWithOffset
*
* DESCRIPTION:   receive i2c value with offset
*
* PARAMETERS:
*
*
*****************************************************************************/

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
#ifdef LINUX_HW
    int try, ret, msg_index = 0, offset_len = 1;
    struct i2c_rdwr_ioctl_data i2c_data;
    struct i2c_msg msg[2];
    char tmp[2];
    PDL_STATUS              pdlStatus;
    UINT_8                  offsetByte;
#endif
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
#ifdef LINUX_HW
    if (transaction_type == PDL_I2C_TRANSACTION_TYPE_OFFSET_AND_DATA_E) {
        /* If offset type is 8 or 16 bits,
        * two messages are required within this transaction.
        * First determine offset address
        * Second - receive data.
        */
        i2c_data.msgs = msg;

        i2c_data.msgs[msg_index].addr = slave_address;
        i2c_data.nmsgs = 1;                   /* single i2c_msg */

        switch (offset_type) {
            case 0:
                break;
            case PDL_I2C_OFFSET_2_BYTE_SIZE:
                /* "Write" part of the transaction */
                tmp[1] = (char)((offset >> 8) & 0xFF);
                offset_len = 2;
            /* fall through */
            case PDL_I2C_OFFSET_1_BYTE_SIZE:
                i2c_data.nmsgs = 2;                   /* two i2c_msg */
                i2c_data.msgs[msg_index].flags = 0;   /* No flags for write command */
                i2c_data.msgs[msg_index].len = offset_len;   /* only one byte */
                i2c_data.msgs[msg_index++].buf = (__u8 *)tmp;
                tmp[0] = (char)(offset & 0xFF);
                break;
            default:
                return PDL_BAD_PARAM;
        }
        /*
            * "Read" part of the transaction
            */
        i2c_data.msgs[msg_index].addr = slave_address;
        i2c_data.msgs[msg_index].flags = I2C_M_RD;  /* read command */
        i2c_data.msgs[msg_index].len = buffer_size;
        i2c_data.msgs[msg_index].buf = (__u8 *)buffer;
        try = 0;
        do {
            ret = ioctl(pdlI2cBusFdArr[bus_id], I2C_RDWR, &i2c_data);
            pdlStatus = ret < 0 ? PDL_I2C_ERROR : PDL_OK;
            if (pdlStatus==PDL_OK || try==(pdlI2cNumOfRetries-1)) break;
            try++;
            usleep(20000);
        }
        while (1);
        if (pdlStatus!=PDL_OK) {
            prvPdlI2cResultHandler(pdlStatus, slave_address, bus_id, (UINT_8)offset, FALSE);
        }
    }
    else {
        offsetByte = (UINT_8) offset;
        /* prvPdlI2cTransmit() already handles failure case */
        pdlStatus = prvPdlI2cTransmit(slave_address, offset_type, bus_id, &offsetByte);
        PDL_CHECK_STATUS(pdlStatus);
        /* prvPdlI2cReceive() already handles failure case */
        pdlStatus = prvPdlI2cReceive(slave_address, buffer_size, bus_id, buffer);
    }
    return pdlStatus;
#else
    return PDL_OK;
#endif
}

/*$ END OF prvPdlI2cReceiveWithOffset */


/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CConfigureMuxes
*
* DESCRIPTION:   configure all muxes in I2C interface
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CConfigureMuxes ( IN PDL_INTERFACE_PRV_I2C_DB_STC * i2cPtr )
 *
 * @brief   Prv pdl i 2 c configure muxes
 *
 * @param [in,out]  i2cPtr  If non-null, zero-based index of the 2c pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConfigureMuxes(
    IN  PRV_PDLIB_DB_TYP      mux_list_handle
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN                                     muxConfigured = FALSE;
    PDL_MUX_LIST_PARAMS_STC                   * muxInfoPtr;
    PDL_MUX_LIST_KEYS_STC                       muxKey;
    UINT_8                                      offset_and_data_buf[20];
    UINT_8                                    * offset_and_data_buf_PTR;
    PDL_STATUS                                  pdlStatus;
    UINT_32                                     oldData = 0, writeValue, writeSize = 0;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* configure all muxes */
    pdlStatus = prvPdlibDbGetFirst(mux_list_handle, (void **)&muxInfoPtr);
    while (pdlStatus == PDL_OK) {
        muxConfigured = TRUE;
        PDL_CHECK_STATUS(pdlStatus);
        if (muxInfoPtr->muxType.type == PDL_MUX_TYPE_MUX_I2C_E) {
            writeValue = muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CValue;
            /* read old data in offset if mask is not max value */
            if (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CValue_mask != MAX_UINT_8) {
                switch (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAccess) {
                    case PDL_I2C_ACCESS_TYPE_ONE_BYTE_E:
                        writeSize = 1;
                        break;
                    case PDL_I2C_ACCESS_TYPE_TWO_BYTES_E:
                        writeSize = 2;
                        break;
                    case PDL_I2C_ACCESS_TYPE_FOUR_BYTES_E:
                        writeSize = 4;
                        break;
                    default:
                        return PDL_BAD_VALUE;
                }

#ifdef LINUX_HW
                /* Must put a delay here to ensure previous transaction had a STOP condition */
                usleep(20000);
#endif

                pdlStatus = prvPdlI2cReceiveWithOffset(muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                                       writeSize,
                                                       muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                                       PDL_I2C_OFFSET_SIZE,
                                                       muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset,
                                                       muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CTransactionType,
                                                       (UINT_8*) & oldData);
                PDL_CHECK_STATUS(pdlStatus);
#ifdef LINUX_HW
                if (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CDisableValue_mask != MAX_UINT_8) {
                    /* Mux value should have been cleared after previous i2c transaction - make sure it is */
                    if ((muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask & oldData) != 
                        muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CDisableValue) {
                        PDL_LIB_DEBUG_MAC(prvPdlI2CInterfaceMuxDebugFlag)(__FUNCTION__, __LINE__, "ERROR: Previous I2C transaction was NOT de-muxed on address[0x%x] busId[%d] offset[0x%x] data[0x%x]\n",
                                                                          muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                                                          muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                                                          muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset,
                                                                          oldData);
                    }
                }
#endif
                writeValue = (~muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask & oldData) |
                             (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask & muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CValue);
            }

            /* point to head of buffer */
            offset_and_data_buf_PTR = &offset_and_data_buf[0];
            /* copy offset */
            memcpy(offset_and_data_buf_PTR, &muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset, PDL_I2C_OFFSET_SIZE);
            /* point past the offset in the buffer */
            offset_and_data_buf_PTR += PDL_I2C_OFFSET_SIZE;
            /* copy data */
            memcpy(offset_and_data_buf_PTR, &writeValue, writeSize);
            pdlStatus = prvPdlI2cTransmit(muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                          writeSize + PDL_I2C_OFFSET_SIZE,
                                          (UINT_8)muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                          (UINT_8*)&offset_and_data_buf[0]);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlI2CInterfaceMuxDebugFlag)(__FUNCTION__, __LINE__, "Set I2C Mux address[0x%x] busId[%d] offset[0x%x] data[0x%x]",
                                                              muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                                              muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                                              muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset,
                                                              writeValue);
        }
        else if (muxInfoPtr->muxType.type == PDL_MUX_TYPE_MUX_GPIO_E) {
            prvPdlGpioPinHwSetValue((GT_U8)muxInfoPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioDeviceNumber,
                                    muxInfoPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioWritePinNumber,
                                    PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E,
                                    muxInfoPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioPinValue);
        }

        muxKey = muxInfoPtr->list_keys;
        pdlStatus  = prvPdlibDbGetNext(mux_list_handle, (void*) &muxKey, (void**) &muxInfoPtr);
    }
    if (muxConfigured) {
        prvPdlLibSleep(PDL_I2C_MUX_SLEEP_MSEC);
    }
    return PDL_OK;
}

/*$ END OF prvPdlI2CConfigureMuxes */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CDisableMuxes
*
* DESCRIPTION:   disable all muxes in I2C interface (irrelevant for GPIOs muxes)
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CDisableMuxes ( IN PDL_INTERFACE_PRV_I2C_DB_STC * i2cPtr )
 *
 * @brief   Prv pdl i 2 c disable muxes
 *
 * @param [in,out]  i2cPtr  If non-null, zero-based index of the 2c pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CDisableMuxes(
    IN  PRV_PDLIB_DB_TYP      mux_list_handle
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN                                     muxConfigured = FALSE;
    PDL_MUX_LIST_PARAMS_STC                   * muxInfoPtr;
    PDL_MUX_LIST_KEYS_STC                       muxKey;
    UINT_8                                      offset_and_data_buf[20];
    UINT_8                                    * offset_and_data_buf_PTR;
    PDL_STATUS                                  pdlStatus;
    UINT_32                                     oldData = 0, writeValue, writeSize = 0;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* go over all muxes */
    pdlStatus = prvPdlibDbGetFirst(mux_list_handle, (void **)&muxInfoPtr);
    while (pdlStatus == PDL_OK) {
        muxConfigured = TRUE;
        PDL_CHECK_STATUS(pdlStatus);
        if (muxInfoPtr->muxType.type == PDL_MUX_TYPE_MUX_I2C_E) {
            /* check if de-mux is actually needed for this specific mux */
            if (pdlIsFieldHasValue(muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.isMuxI2CDisableUsed_mask) == FALSE ||
                (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.isMuxI2CDisableUsed == TRUE)) {                              
                /* read old data in offset */
                switch (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAccess) {
                    case PDL_I2C_ACCESS_TYPE_ONE_BYTE_E:
                        writeSize = 1;
                        break;
                    case PDL_I2C_ACCESS_TYPE_TWO_BYTES_E:
                        writeSize = 2;
                        break;
                    case PDL_I2C_ACCESS_TYPE_FOUR_BYTES_E:
                        writeSize = 4;
                        break;
                    default:
                        return PDL_BAD_VALUE;
                }

#ifdef LINUX_HW
                /* Must put a delay here to ensure previous transaction had a STOP condition */
                usleep(20000);
#endif

                pdlStatus = prvPdlI2cReceiveWithOffset(muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                                       writeSize,
                                                       muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                                       PDL_I2C_OFFSET_SIZE,
                                                       muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset,
                                                       muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CTransactionType,
                                                       (UINT_8*) & oldData);
                PDL_CHECK_STATUS(pdlStatus);
#ifdef LINUX_HW
                /* Mux value should NOT have been cleared after previous i2c transaction - make sure it is */
                if ((muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask & oldData) == 
                    muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CDisableValue) {
                    PDL_LIB_DEBUG_MAC(prvPdlI2CInterfaceMuxDebugFlag)(__FUNCTION__, __LINE__, "ERROR: Previous I2C transaction already de-muxed on address[0x%x] busId[%d] offset[0x%x] data[0x%x]\n",
                                                                      muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                                                      muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                                                      muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset,
                                                                      oldData);
                }
#endif
                writeValue = (~muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask & oldData) |
                             (muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask & muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CDisableValue);

                /* point to head of buffer */
                offset_and_data_buf_PTR = &offset_and_data_buf[0];
                /* copy offset */
                memcpy(offset_and_data_buf_PTR, &muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset, PDL_I2C_OFFSET_SIZE);
                /* point past the offset in the buffer */
                offset_and_data_buf_PTR += PDL_I2C_OFFSET_SIZE;
                /* copy data */
                memcpy(offset_and_data_buf_PTR, &writeValue, writeSize);
                pdlStatus = prvPdlI2cTransmit(muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                              writeSize + PDL_I2C_OFFSET_SIZE,
                                              (UINT_8)muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                              (UINT_8*)&offset_and_data_buf[0]);
                PDL_CHECK_STATUS(pdlStatus);
                PDL_LIB_DEBUG_MAC(prvPdlI2CInterfaceMuxDebugFlag)(__FUNCTION__, __LINE__, "Disable I2C Mux address[0x%x] busId[%d] offset[0x%x] data[0x%x]",
                                                                  muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress,
                                                                  muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId,
                                                                  muxInfoPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset,
                                                                  writeValue);
            }
        }
        else if (muxInfoPtr->muxType.type == PDL_MUX_TYPE_MUX_GPIO_E) {
            /* not supported - irrelevant */
            /* return PDL_OK; */
        }

        muxKey = muxInfoPtr->list_keys;
        pdlStatus  = prvPdlibDbGetNext(mux_list_handle, (void*) &muxKey, (void**) &muxInfoPtr);
    }
    if (muxConfigured) {
        prvPdlLibSleep(PDL_I2C_MUX_SLEEP_MSEC);
    }
    return PDL_OK;
}

/*$ END OF prvPdlI2CDisableMuxes */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CFindNthBitInData
*
* DESCRIPTION:   Finds the nth bit set in data if none, return FALSE
*
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CFindNthBitInData ( IN UINT_32 data, IN UINT_32 bit, OUT UINT_32 * bitNumPtr )
 *
 * @brief   Prv pdl i 2 c find nth bit in data
 *
 * @param           data        The data.
 * @param           bit         The bit.
 * @param [in,out]  bitNumPtr   If non-null, the bit number pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CFindNthBitInData(
    IN  UINT_32                                 data,
    IN  UINT_32                                 bit,
    OUT UINT_32                               * bitNumPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                 i;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (bitNumPtr == NULL) {
        return PDL_BAD_PTR;
    }
    bit++;
    for (i = 0; i < sizeof(data) * 8; i++) {
        if (data & 1) {
            bit--;
            if (bit == 0) {
                break;
            }
        }
        data = data >> 1;
    }
    if (bit == 0) {
        *bitNumPtr = i;
        return PDL_OK;
    }

    return PDL_NOT_FOUND;
}

/*$ END OF prvPdlI2CFindNthBitInData */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CConvertRel2Abs
*
* DESCRIPTION:   convert relative data to absolute data using i2c interface mask
*                examples: mask is 0XC0 and data is 3 - absData = 0xC0
*                          mask is 0xC0 and data is 2 - absData = 0x80
*                          mask is 0xC0 and data is 1 - absData = 0x40
*                          mask is 0xC0 and data is 0 - absData = 0
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CConvertRel2Abs ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 relData, OUT UINT_32 * absDataPtr )
 *
 * @brief   Prv pdl i 2 c convert relative 2 abs
 *
 * @param           interfaceId Identifier for the interface.
 * @param           relData     Information describing the relative.
 * @param [in,out]  absDataPtr  If non-null, the abs data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertRel2Abs(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 relData,
    OUT UINT_32                               * absDataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, temp = 0;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (absDataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    for (currentBit = 0; currentBit < sizeof(relData) * 8; currentBit++) {
        if (relData & (1 << currentBit)) {
            /* find the matching bit in mask */
            pdlStatus = prvPdlI2CFindNthBitInData((UINT_32)i2cPtr->publicInfo.mask, currentBit, &temp);
            if (pdlStatus == PDL_OK) {
                out |= 1 << temp;
            }
            else {
                return pdlStatus;
            }
        }
    }
    *absDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertRel2Abs */

/**
 * @fn  PDL_STATUS prvPdlI2CConvertRel2AbsWithMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 relData,IN  UINT_32 mask ,OUT UINT_32 * absDataPtr )
 *
 * @brief  brief   Prv pdl i 2 c convert relative 2 abs according to mask
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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, temp;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (absDataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);


    for (currentBit = 0; currentBit < sizeof(relData) * 8; currentBit++) {
        if (relData & (1 << currentBit)) {
            /* find the matching bit in mask */
            pdlStatus = prvPdlI2CFindNthBitInData(mask, currentBit, &temp);
            if (pdlStatus == PDL_OK) {
                out |= 1 << temp;
            }
            else {
                return pdlStatus;
            }
        }
    }
    *absDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertRel2Abs */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CConvertAbs2Rel
*
* DESCRIPTION:   convert absolute data to relative data using i2c interface mask
*                examples: mask is 0XC0 and value is 0xC0 - 3 is returned
*                          mask is 0xC0 and value is 0x80 - 2 is returned
*                          mask is 0xC0 and value is 0x40 - 1 is returned
*
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2CConvertAbs2Rel ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 absData, OUT UINT_32 * relDataPtr )
 *
 * @brief   Prv pdl i 2 c convert abs 2 relative
 *
 * @param           interfaceId Identifier for the interface.
 * @param           absData     Information describing the abs.
 * @param [in,out]  relDataPtr  If non-null, the relative data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2CConvertAbs2Rel(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 absData,
    OUT UINT_32                               * relDataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, count = 0;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (relDataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* absData doesn't match absMask */
    if ((absData & ~i2cPtr->publicInfo.mask) != 0) {
        return PDL_OUT_OF_RANGE;
    }
    for (currentBit = 0; currentBit < sizeof(absData) * 8; currentBit++) {
        if (absData & (1 << currentBit)) {
            /* find the matching bit in mask */
            out |= 1 << count;
        }
        if (i2cPtr->publicInfo.mask & (1 << currentBit)) {
            count++;
        }
    }
    *relDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertAbs2Rel */

/**
 * @fn  PDL_STATUS prvPdlI2CConvertAbs2RelWithMask ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 absData,IN  UINT_32 mask ,OUT UINT_32 * relDataPtr )
 *
 * @brief   Prv pdl i 2 c convert abs 2 relative according to mask
 *
 * @param           interfaceId Identifier for the interface.
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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     out = 0, currentBit, count = 0;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (relDataPtr == NULL) {
        return PDL_BAD_PTR;
    }
    /* absData doesn't match absMask */
    if ((absData & ~mask) != 0) {
        return PDL_OUT_OF_RANGE;
    }
    for (currentBit = 0; currentBit < sizeof(absData) * 8; currentBit++) {
        if (absData & (1 << currentBit)) {
            /* find the matching bit in mask */
            out |= 1 << count;
        }
        if (mask & (1 << currentBit)) {
            count++;
        }
    }
    *relDataPtr = out;
    return PDL_OK;
}

/*$ END OF prvPdlI2CConvertAbs2Rel */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2CHwGetValue
*
* DESCRIPTION:   get i2c value
*
* PARAMETERS:
*
*
*****************************************************************************/

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* perform i2c read */
    pdlStatus = prvPdlI2cReceiveWithOffset(i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.transactionType, (UINT_8*) dataPtr);
    if (pdlStatus != PDL_OK) {
        /* Restore Mux to default settings */
        (void)prvPdlI2CDisableMuxes(i2cPtr->muxDb);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    *dataPtr &= i2cPtr->publicInfo.mask;

    /* disable all muxes */
    pdlStatus = prvPdlI2CDisableMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "Reading i2c value from address[0x%x] busId[%d] offset[0x%x] mask[0x%x] data[0x%x]\n", i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.mask, *dataPtr);
    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return PDL_OK;
}
/*$ END OF prvPdlI2CHwGetValue */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* perform i2c read */
    pdlStatus = prvPdlI2cReceiveWithOffset(i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, offset, i2cPtr->publicInfo.transactionType, (UINT_8*) dataPtr);
    if (pdlStatus != PDL_OK) {
        /* Restore Mux to default settings */
        (void)prvPdlI2CDisableMuxes(i2cPtr->muxDb);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);
    *dataPtr &= mask;

    /* disable all muxes */
    pdlStatus = prvPdlI2CDisableMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "Reading i2c value from address[0x%x] busId[%d] offset[0x%x] mask[0x%x] data[0x%x]\n", i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, offset, mask, *dataPtr);
    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return PDL_OK;
}

/*$ END OF pdlI2CHwGetValueWithOffsetMask */


/* ***************************************************************************
* FUNCTION NAME: pdlI2CHwGetBufferWithOffset
*
* DESCRIPTION:   get i2c buffer the size of length with given offset (used for SFP EEPROM read)
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlI2CHwGetBufferWithOffset ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_16 offset, IN UINT_32 length, OUT void * dataPtr )
 *
 * @brief   Pdl i 2 c hardware get buffer with offset
 *
 * @param           interfaceId Identifier for the interface.
 * @param           i2cAddress  I2c address to read from can only be 0x50 or 0x51 or 0x56
 * @param           offset      The offset.
 * @param           length      The length.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CHwGetBufferWithOffset(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_8                                  i2cAddress,
    IN  UINT_16                                 offset,
    IN  UINT_32                                 length,
    OUT void                                  * dataPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_8                                    * dataWritePtr;
    UINT_16                                     relOffset;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (i2cAddress != 0x50 && i2cAddress != 0x51 && i2cAddress != 0x56) {
        return PDL_BAD_PARAM;
    }

    if (dataPtr == NULL) {
        return PDL_BAD_PTR;
    }

    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);
    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* perform i2c read */
    relOffset = offset;
    dataWritePtr = (UINT_8 *)dataPtr;

    /*
    * Read from I2C the whole required length.
    * Basically, this code is called for devices which supports such reads (SFP) or
    * others (CPLD, FAN Controller, RPS) which read only one byte from specific offset.
    */
    pdlStatus = prvPdlI2cReceiveWithOffset(i2cAddress, length, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, relOffset, i2cPtr->publicInfo.transactionType, (UINT_8*) dataWritePtr);
    if (pdlStatus != PDL_OK) {
    	/* Restore Mux to default settings */
        (void)prvPdlI2CDisableMuxes(i2cPtr->muxDb);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* disable all muxes */
    pdlStatus = prvPdlI2CDisableMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return PDL_OK;
}
/*$ END OF pdlI2CHwGetBufferWithOffset */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     oldData = 0, writeValue;
    UINT_8                                      offsetAndDataBuf[20];
    UINT_8                                    * offsetAndDataBufPtr;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlI2cReceiveWithOffset(i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.transactionType, (UINT_8*) & oldData);
    if (pdlStatus != PDL_OK) {
        /* Restore Mux to default settings */
        (void)prvPdlI2CDisableMuxes(i2cPtr->muxDb);
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }

    PDL_CHECK_STATUS(pdlStatus);
    writeValue = (~i2cPtr->publicInfo.mask & oldData) | (i2cPtr->publicInfo.mask & data);

    /* perform i2c write */
    /* point to head of buffer */
    offsetAndDataBufPtr = &offsetAndDataBuf[0];
    /* copy offset */
    memcpy(offsetAndDataBufPtr, &i2cPtr->publicInfo.offset, PDL_I2C_OFFSET_SIZE);
    /* point past the offset in the buffer */
    offsetAndDataBufPtr += PDL_I2C_OFFSET_SIZE;
    /* copy data */
    memcpy(offsetAndDataBufPtr, &writeValue, i2cPtr->publicInfo.accessSize);
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "Writing i2c value[0x%x] to address[0x%x] busId[%d] offset[0x%x] mask[0x%x]\n", i2cPtr->publicInfo.mask & data, i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.busId, i2cPtr->publicInfo.offset, i2cPtr->publicInfo.mask);
    pdlStatus = prvPdlI2cTransmit(i2cPtr->publicInfo.i2cAddress, i2cPtr->publicInfo.accessSize + PDL_I2C_OFFSET_SIZE, (UINT_8)i2cPtr->publicInfo.busId, (UINT_8*)&offsetAndDataBuf[0]);

    /* disable all muxes */
    pdlStatus = prvPdlI2CDisableMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return pdlStatus;
}
/*$ END OF prvPdlI2CHwSetValue */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     oldData = 0, writeValue, fullMaskValue = 0;
    UINT_8                                      offsetAndDataBuf[20];
    UINT_8                                    * offsetAndDataBufPtr, accessSize;
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* configure all muxes */
    prvPdlLock(PDL_OS_LOCK_TYPE_I2C_E);
    pdlStatus = prvPdlI2CConfigureMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }

    switch (i2cPtr->publicInfo.accessSize) {
        case PDL_I2C_ACCESS_TYPE_ONE_BYTE_E:
            accessSize = 1;
            fullMaskValue = MAX_UINT_8;
            break;
        case PDL_I2C_ACCESS_TYPE_TWO_BYTES_E:
            accessSize = 2;
            fullMaskValue = MAX_UINT_16;
            break;
        case PDL_I2C_ACCESS_TYPE_FOUR_BYTES_E:
            accessSize = 4;
            fullMaskValue = MAX_UINT_32;
            break;
        default:
            prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
            return PDL_BAD_VALUE;
    }

    if (mask != fullMaskValue) {
		pdlStatus = prvPdlI2cReceiveWithOffset(i2cAddress, accessSize, (UINT_8)i2cPtr->publicInfo.busId, PDL_I2C_OFFSET_SIZE, offset, i2cPtr->publicInfo.transactionType, (UINT_8*) & oldData);
		if (pdlStatus != PDL_OK) {
			/* Restore Mux to default settings */
			(void)prvPdlI2CDisableMuxes(i2cPtr->muxDb);
			prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
		}
		PDL_CHECK_STATUS(pdlStatus);
    }

    writeValue = (~mask & oldData) | (mask & data);

    /* perform i2c write */
    /* point to head of buffer */
    offsetAndDataBufPtr = &offsetAndDataBuf[0];
    /* copy offset */
    memcpy(offsetAndDataBufPtr, &offset, PDL_I2C_OFFSET_SIZE);
    /* point past the offset in the buffer */
    offsetAndDataBufPtr += PDL_I2C_OFFSET_SIZE;
    /* copy data */
    memcpy(offsetAndDataBufPtr, &writeValue, accessSize);
    PDL_LIB_DEBUG_MAC(prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "Writing i2c value[0x%x] to address[0x%x] busId[%d] offset[0x%x] mask[0x%x]\n", mask & data, i2cAddress, i2cPtr->publicInfo.busId, offset, mask);
    pdlStatus = prvPdlI2cTransmit(i2cAddress, accessSize + PDL_I2C_OFFSET_SIZE, (UINT_8)i2cPtr->publicInfo.busId, (UINT_8*)&offsetAndDataBuf[0]);

    /* disable all muxes */
    pdlStatus = prvPdlI2CDisableMuxes(i2cPtr->muxDb);
    if (pdlStatus != PDL_OK) {
        prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    }
    PDL_CHECK_STATUS(pdlStatus);

    prvPdlUnlock(PDL_OS_LOCK_TYPE_I2C_E);
    return pdlStatus;
}
/*$ END OF pdlI2CHwSetValueWithOffsetMask */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);
	pdlStatus = pdlI2CHwSetValueWithAddressOffsetMask(interfaceId, i2cPtr->publicInfo.i2cAddress, offset, mask, data);

    return pdlStatus;
}
/*$ END OF pdlI2CHwSetValueWithOffsetMask */

/**
 * @fn  PDL_STATUS PdlI2CDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_I2C_STC * attributesPtr )
 *
 * @brief   Pdl i 2 c debug database get attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CDebugDbGetAttributes(
    IN  PDL_INTERFACE_TYP                    interfaceId,
    OUT PDL_INTERFACE_I2C_STC              * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    memcpy(attributesPtr, &i2cPtr->publicInfo, sizeof(PDL_INTERFACE_I2C_STC));

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  i2cPtr->publicInfo.accessSize);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, i2cPtr->publicInfo.busId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8,  i2cPtr->publicInfo.i2cAddress);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, i2cPtr->publicInfo.mask);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, i2cPtr->publicInfo.numberOfMuxes);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_16, i2cPtr->publicInfo.offset);

    return PDL_OK;
}
/*$ END OF pdlI2CDebugDbGetAttributes */

/* ***************************************************************************
* FUNCTION NAME: pdlI2CDebugDbSetAttributes
*
* DESCRIPTION:  set I2C attributes
*               update I2C relevant information for given interfaceId
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlI2CDebugDbSetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_I2C_STC * attributesPtr )
 *
 * @brief   Pdl i 2 c debug database set attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CDebugDbSetAttributes(
    IN  PDL_INTERFACE_TYP                    interfaceId,
    IN  UINT_32                              attributesMask,
    OUT PDL_INTERFACE_I2C_STC              * attributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (attributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_BUS_ID) {
        i2cPtr->publicInfo.busId = attributesPtr->busId;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ADDRESS) {
        i2cPtr->publicInfo.i2cAddress = attributesPtr->i2cAddress;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ACCESS_SIZE) {
        i2cPtr->publicInfo.accessSize = attributesPtr->accessSize;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_OFFSET) {
        i2cPtr->publicInfo.offset = attributesPtr->offset;
    }
    if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_MASK) {
        i2cPtr->publicInfo.mask = attributesPtr->mask;
    }
    return PDL_OK;
}
/*$ END OF pdlI2CDebugDbSetAttributes */

/* ***************************************************************************
* FUNCTION NAME: pdlI2CDebugDbGetMuxAttributes
*
* DESCRIPTION:  get I2C Mux attributes
*               based on information from XML
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlI2CDebugDbGetMuxAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 muxNumber, OUT PDL_INTERFACE_MUX_STC * muxAttributesPtr )
 *
 * @brief   Pdl i 2 c debug database get multiplexer attributes
 *
 * @param           interfaceId         Identifier for the interface.
 * @param           muxNumber           The multiplexer number.
 * @param [in,out]  muxAttributesPtr    If non-null, the multiplexer attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CDebugDbGetMuxAttributes(
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             muxNumber,
    OUT PDL_INTERFACE_MUX_STC             * muxAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    PDL_MUX_LIST_PARAMS_STC                   * muxPtr;
    PDL_MUX_LIST_KEYS_STC                       muxKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (muxAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    muxKey.muxNumber = muxNumber;
    pdlStatus  = prvPdlibDbFind(i2cPtr->muxDb, (void*) &muxKey, (void**) &muxPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (muxPtr->muxType.type == PDL_MUX_TYPE_MUX_GPIO_E) {
        muxAttributesPtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_GPIO_E;
        muxAttributesPtr->interfaceInfo.gpioInterface.dev = muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioDeviceNumber;
        muxAttributesPtr->interfaceInfo.gpioInterface.pushValue = muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioPinValue;
        muxAttributesPtr->interfaceInfo.gpioInterface.pinNumber = muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioWritePinNumber;
    }
    else if (muxPtr->muxType.type == PDL_MUX_TYPE_MUX_I2C_E) {
        muxAttributesPtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_I2C_E;
        muxAttributesPtr->interfaceInfo.i2cMux.accessSize = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAccess;
        muxAttributesPtr->interfaceInfo.i2cMux.baseAddress = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress;
        muxAttributesPtr->interfaceInfo.i2cMux.busId = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId;
        muxAttributesPtr->interfaceInfo.i2cMux.mask = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask;
        muxAttributesPtr->interfaceInfo.i2cMux.offset = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset;
        muxAttributesPtr->interfaceInfo.i2cMux.transactionType = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CTransactionType;
        muxAttributesPtr->interfaceInfo.i2cMux.value = muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CValue;
    }
    else {
        return PDL_BAD_STATE;
    }

    return PDL_OK;
}
/*$ END OF pdlI2CDebugDbGetMuxAttributes */

/* ***************************************************************************
* FUNCTION NAME: pdlI2CDebugDbSetMuxAttributes
*
* DESCRIPTION:  set I2C Mux attributes
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlI2CDebugDbSetMuxAttributes ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 muxNumber, IN PDL_INTERFACE_MUX_STC * muxAttributesPtr )
 *
 * @brief   Pdl i 2 c debug database set multiplexer attributes
 *
 * @param           interfaceId         Identifier for the interface.
 * @param           muxNumber           The multiplexer number.
 * @param [in,out]  muxAttributesPtr    If non-null, the multiplexer attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlI2CDebugDbSetMuxAttributes(
    IN  PDL_INTERFACE_TYP                   interfaceId,
    IN  UINT_32                             muxNumber,
    IN  UINT_32                             attributesMask,
    IN  PDL_INTERFACE_MUX_STC             * muxAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cPtr;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    PDL_MUX_LIST_PARAMS_STC                   * muxPtr;
    PDL_MUX_LIST_KEYS_STC                       muxKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    i2cKey.interfaceId = interfaceId;
    pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (muxAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    muxKey.muxNumber = muxNumber;
    pdlStatus  = prvPdlibDbFind(i2cPtr->muxDb, (void*) &muxKey, (void**) &muxPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (muxAttributesPtr->interfaceType == PDL_INTERFACE_EXTENDED_TYPE_GPIO_E) {
        if (muxPtr->muxType.type != PDL_MUX_TYPE_MUX_GPIO_E) {
            return PDL_BAD_PARAM;
        }

        if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_DEVICE) {
            muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioDeviceNumber = muxAttributesPtr->interfaceInfo.gpioInterface.dev;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioDeviceNumber_mask);
        }
        if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_PIN_NUMBER) {
            muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioWritePinNumber = muxAttributesPtr->interfaceInfo.gpioInterface.pinNumber;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioWritePinNumber_mask);
        }
        if (attributesMask & PDL_GPIO_SET_ATTRIBUTE_MASK_VALUE) {
            muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioPinValue = muxAttributesPtr->interfaceInfo.gpioInterface.pushValue;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxGpio.muxGpioInfo.gpioMuxGroupType.muxGpioPinValue_mask);
        }
    }
    else if (muxAttributesPtr->interfaceType == PDL_INTERFACE_EXTENDED_TYPE_I2C_E) {
        if (muxPtr->muxType.type != PDL_MUX_TYPE_MUX_I2C_E) {
            return PDL_BAD_PARAM;
        }

        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ADDRESS) {
            muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress = muxAttributesPtr->interfaceInfo.i2cMux.baseAddress;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAddress_mask);
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_OFFSET) {
            muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset = muxAttributesPtr->interfaceInfo.i2cMux.offset;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2COffset_mask);
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_MASK) {
            muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask = muxAttributesPtr->interfaceInfo.i2cMux.mask;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CMask_mask);
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_VALUE) {
            muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CValue = muxAttributesPtr->interfaceInfo.i2cMux.value;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CValue_mask);
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_BUS_ID) {
            muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId = muxAttributesPtr->interfaceInfo.i2cMux.busId;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CBusId_mask);
        }
        if (attributesMask & PDL_I2C_SET_ATTRIBUTE_MASK_ACCESS_SIZE) {
            muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAccess = muxAttributesPtr->interfaceInfo.i2cMux.accessSize;
            prvPdlSetFieldHasValue(&muxPtr->muxType.data.muxI2C.muxI2CInfo.i2CMuxGroupType.muxI2CAccess_mask);
        }
    }
    else {
        return PDL_BAD_STATE;
    }

    return PDL_OK;
}
/*$ END OF pdlI2CDebugDbSetMuxAttributes */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (busId > PDL_I2C_BUS_ID_MAX) {
        return PDL_OUT_OF_RANGE;
    }
    pdlI2cBusIdIsUsed [busId] = isUsed;
    return PDL_OK;
}
/*$ END OF prvPdlI2cBusSetUsed */

/* ***************************************************************************
* FUNCTION NAME: prvPdlI2cBusIsUsed
*
* DESCRIPTION:  return if bus_id is being used
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  BOOLEAN prvPdlI2cBusIsUsed ( IN UINT_32 busId )
 *
 * @brief   Prv pdl i 2c bus is used
 *
 * @param   busId   Identifier for the bus.
 *
 * @return  True if it succeeds, false if it fails.
 */

BOOLEAN prvPdlI2cBusIsUsed(
    IN  UINT_32          busId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (busId > PDL_I2C_BUS_ID_MAX) {
        return PDL_OUT_OF_RANGE;
    }
    return pdlI2cBusIdIsUsed[busId];
}
/*$ END OF prvPdlI2cBusIsUsed */

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

    return prvPdlibDbGetNumOfEntries(prvPdlI2cDb, countPtr);
}

/**
 * @fn  PDL_STATUS prvPdlI2cInterfaceRegister ( IN UINT_32  busId, IN UINT_8 i2cAddress, IN PDL_I2C_ACCESS_TYPE_ENT accessSize, IN UINT_16 offset, IN UINT_32 mask, IN PRV_PDLIB_DB_TYP muxDb, IN BOOLEAN isChannelUsed, IN UINT_32 channelId, IN UINT_32 baseAddress, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register i2c interface usage
 *
 * @param [in]  busId           bus id
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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus = PDL_OK;
    PDL_INTERFACE_PRV_I2C_DB_STC              * i2cEntryPtr, i2cEntry;
    PDL_INTERFACE_PRV_KEY_STC                   i2cKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    memset(&i2cEntry, 0, sizeof(i2cEntry));

    pdlStatus = prvPdlibDbGetNumOfEntries(prvPdlI2cDb, &i2cKey.interfaceId);
    PDL_CHECK_STATUS(pdlStatus);
    *interfaceIdPtr = i2cKey.interfaceId;

    if (isChannelUsed == FALSE) {
        i2cEntry.publicInfo.busId = busId;
    }
    else {
        i2cEntry.publicInfo.i2cChannelRequired = TRUE;
        i2cEntry.publicInfo.i2cBaseAddress = baseAddress;
        i2cEntry.publicInfo.i2cChannelId = channelId;
    }

    i2cEntry.publicInfo.accessSize = accessSize;
    i2cEntry.publicInfo.mask = mask;
    i2cEntry.publicInfo.offset = offset;
    i2cEntry.muxDb = muxDb;
    i2cEntry.publicInfo.i2cAddress = i2cAddress;
    i2cEntry.publicInfo.transactionType = transactionType;


    i2cEntry.publicInfo.numberOfMuxes = 0;
    if (i2cEntry.muxDb) {
        pdlStatus = prvPdlibDbGetNumOfEntries(i2cEntry.muxDb, &i2cEntry.publicInfo.numberOfMuxes);
        PDL_CHECK_STATUS(pdlStatus);
    }

    pdlStatus = prvPdlibDbAdd(prvPdlI2cDb, (void *)&i2cKey, (void **)&i2cEntry, (void **)&i2cEntryPtr);

    prvPdlI2cBusSetUsed(i2cEntry.publicInfo.busId, TRUE);

    return pdlStatus;
}
/*$ END OF prvPdlI2cInterfaceRegister */

/****************************************************************************
* FUNCTION NAME: prvPdlI2cBusFdInit
*
* DESCRIPTION:   Initialize I2C Bus file descriptors
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlI2cBusFdInit ( IN void )
 *
 * @brief   Prv pdl i 2c bus fd initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cBusFdInit(
    IN void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_INTERFACE_PRV_I2C_DB_STC      * i2cPtr = NULL;
    PDL_INTERFACE_PRV_KEY_STC           i2cKey;
#ifdef LINUX_HW
    UINT_32                           i;
    char                              i2cDevName[20];
#endif
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* Go over all i2c interfaces and mark busId that are used. Convert channels to busIds */
    pdlStatus = prvPdlibDbGetNextKey(prvPdlI2cDb, NULL, (void*) &i2cKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlibDbFind(prvPdlI2cDb, (void*) &i2cKey, (void**) &i2cPtr);
        PDL_CHECK_STATUS(pdlStatus);
        /* if channel i2c mode is required, convert base address & channel id to i2c-dev-id and store as the busId */
        if (i2cPtr->publicInfo.i2cChannelRequired == TRUE) {
            pdlStatus = prvPdlI2cChannelToDevIdConvert(i2cPtr->publicInfo.i2cBaseAddress, i2cPtr->publicInfo.i2cChannelId, &i2cPtr->publicInfo.busId);
            PDL_DEBUG_ERROR(pdlStatus, prvPdlInterfaceDebugFlag)(__FUNCTION__, __LINE__, "i2c base address [%d], channel [%d] convert to i2c_dev_id FAILED", i2cPtr->publicInfo.i2cBaseAddress, i2cPtr->publicInfo.i2cChannelId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        prvPdlI2cBusSetUsed(i2cPtr->publicInfo.busId, TRUE);
        pdlStatus = prvPdlibDbGetNextKey(prvPdlI2cDb, (void*) &i2cKey, (void*) &i2cKey);
    }

#ifdef LINUX_HW
    for (i = 0; i < PDL_I2C_BUS_ID_MAX; i++) {
        if (prvPdlI2cBusIsUsed(i) == TRUE) {
            sprintf(i2cDevName, "%s-%d", PDL_I2C_PREFIX_DEV_NAME, i);
            pdlI2cBusFdArr[i] = open(i2cDevName, O_RDWR);
            if (pdlI2cBusFdArr[i] == -1) {
                return PDL_I2C_ERROR;
            }
        }
    }
#endif
    return PDL_OK;
}
/*$ END OF prvPdlI2cBusFdInit */

/**
 * @fn  PDL_STATUS prvPdlI2cBusFdDestroy ( IN void )
 *
 * @brief   Prv pdl i 2c bus fd initialize (+convert channels to real bus ids)
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlI2cBusFdDestroy(
    IN void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* destroy i2c interface db */
    pdlStatus = prvPdlibDbDestroy(prvPdlI2cDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlI2cBusFdDestroy */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC       dbAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_INTERFACE_PRV_I2C_DB_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlI2cDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* destroy i2c interface db */
    pdlStatus = prvPdlibDbDestroy(prvPdlI2cDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlI2cDestroy */
