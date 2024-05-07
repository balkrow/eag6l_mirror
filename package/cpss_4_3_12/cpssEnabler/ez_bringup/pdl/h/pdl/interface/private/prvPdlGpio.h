/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\interface\private\prvpdlgpio.h.
 *
 * @brief   Declares the prvpdlgpio class
 */

#ifndef __prvPdlGpioh

#define __prvPdlGpioh
/**
********************************************************************************
 * @file prvPdlGpio.h
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
 * @brief Platform driver layer - Private GPIO related API (internal)
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/init/pdlInit.h>

/**
* @addtogroup Interface
* @{
*/

/**
* @addtogroup GPIO
* @{
*/

/**
 * @defgroup GPIO_private GPIO Private
 * @{defines private structures and function for GPIO
*/

/*! Memory file descriptor name */
#define PDL_MEM_DEV_MEM_NAME                        "/dev/mem"

/*! Memory CPU region size */
#define PDL_CPU_REGISTERS_REGION_SIZE               (0x200)

/*! Memory CPU region address */
#define PDL_CPU_REGISTERS_REGION                0xF1018000

/*! this value is used to represent GPIOs on the CPU */
#define PDL_GPIO_CPU_DEV                            255

/*! GPIO_0 out reg */
#define PDL_GPIO_OUT_MPP_0_31                       0x100

/*! GPIO_0 out ena */
#define PDL_GPIO_SET_TYPE_MPP_0_31                  0x104

/*! GPIO_0 out blink */
#define PDL_GPIO_BLINKING_MPP_0_31                  0x108

/*! GPIO_0 data in polarity */
#define PDL_GPIO_POLARITY_MPP_0_31                  0x10C

/*! GPIO_0 data in */
#define PDL_GPIO_INPUT_MPP_0_31                     0x110

/*! GPIO_0 interrupt cause */
#define PDL_GPIO_INT_CAUSE_MPP_0_31                 0x114

/*! GPIO_0 interrupt mask */
#define PDL_GPIO_INT_MASK_MPP_0_31                  0x118


/*! GPIO_1 out reg */
#define PDL_GPIO_OUT_MPP_32_63                      0x140

/*! GPIO_1 out ena */
#define PDL_GPIO_SET_TYPE_MPP_32_63                 0x144

/*! GPIO_1 out blink */
#define PDL_GPIO_BLINKING_MPP_32_63                 0x148

/*! GPIO_1 data in polarity */
#define PDL_GPIO_POLARITY_MPP_32_63                 0x14C

/*! GPIO_1 data in */
#define PDL_GPIO_INPUT_MPP_32_63                    0x150

/*! GPIO_1 interrupt cause */
#define PDL_GPIO_INT_CAUSE_MPP_32_63                0x154

/*! GPIO_1 interrupt mask */
#define PDL_GPIO_INT_MASK_MPP_32_63                 0x158


/*! GPIO Base offset of registers */
#define PDL_GPIO_PP_GPIO_REG_BASE_OFF               0x18000

/*! MPP initialization */
#define PDL_GPIO_MPP_INIT_0_7                       0x00

/*! MPP initialization */
#define PDL_GPIO_MPP_INIT_8_15                      0x04

/*! MPP initialization */
#define PDL_GPIO_MPP_INIT_16_23                     0x08

/*! MPP initialization */
#define PDL_GPIO_MPP_INIT_24_31                     0x0C

/*! MPP initialization */
#define PDL_GPIO_MPP_INIT_32_39                     0x10

/*! MPP initialization */
#define PDL_GPIO_MPP_INIT_40_47                     0x14

/*! MPP initialization Base offset of registers */
#define PDL_GPIO_PP_MPP_INIT_BASE_OFF               0x18000

/*! Max number of GPIO memory mapped registers */
#define PDL_GPIO_MPP_NUM_OF_INIT_REGS               8

/*! Max number of register types */
#define PDL_GPIO_NUM_OF_TYPE_REGS                   2


/**
*  @def PDL_GPIO_GET_BASE_REG_MAC(__pin_number, reg)
*  @brief get base register based on pin number
*/
#define PDL_GPIO_GET_BASE_REG_MAC(__pin_number, reg)                                                                   \
                                 if (/*(__pin_number >= 0) && */  (__pin_number <= 31))  reg = PDL_GPIO_OUT_MPP_0_31;  \
                                 else if ((__pin_number >= 32) && (__pin_number <= 63))  reg = PDL_GPIO_OUT_MPP_32_63; \
                                 else reg = 0;
/** @brief  mask per quad */
extern UINT_32                  pdlGpioMppMask[8];

/**
* @def PDL_GPIO_MPP_GET_INIT_MASK_VALUE(__pin_number)
* get the mask corresponding to __pin_number quad
*/
#define PDL_GPIO_MPP_GET_INIT_MASK_VALUE(__pin_number) pdlGpioMppMask[__pin_number % 8]


/*! main tag - GPIO group */
#define PDL_GPIO_TAG_NAME           "gpio-group"

/*! tag for pin number */
#define PDL_GPIO_PIN_READ_NUMBER_NAME    "gpio-read-pin-number"
#define PDL_GPIO_PIN_WRITE_NUMBER_NAME   "gpio-write-pin-number"

/*! tag for device number */
#define PDL_GPIO_PIN_READ_DEV_NAME       "gpio-read-device-number"
#define PDL_GPIO_PIN_WRITE_DEV_NAME      "gpio-write-device-number"

/*! tag for push value ?? @todo what is this? */
#define PDL_GPIO_VALUE_NAME                 "push-value"

#define PDL_GPIO_TAG_NAME                   "gpio-group"

#define PDL_GPIO_READ_ONLY_TAG_NAME         "gpio-read-address"

#define PDL_GPIO_WRITE_TAG_NAME             "gpio-write-address"

#define PDL_GPIO_PIN_NUMBER_NAME            "pin-number"

#define PDL_GPIO_PIN_DEV_NAME               "device-number"

#define PDL_GPIO_PIN_VALUE_NAME             "pin-value"

#define PDL_GPIO_MUX_PIN_DEV_NAME       "mux-gpio-device-number"
#define PDL_GPIO_MUX_PIN_NUMBER_NAME    "mux-gpio-write-pin-number"
#define PDL_GPIO_MUX_PIN_VALUE_NAME     "mux-gpio-pin-value"


/**
* @internal prvPdlGpioPinHwSetValue
* @endinternal
* @brief  set gpio value, used also in I2C for GPIO muxes
*
* @returns PDL_STATUS
*
* @param[in]  dev               -   device number (255 = CPU gpio)
* @param[in] pinNumber          -   pin number
* @param[in] offset             -   offset (output/ type / polarity /blink)
* @param[in] data               -   data that will be written
*/

PDL_STATUS prvPdlGpioPinHwSetValue(
    IN  UINT_32                                 dev,
    IN  UINT_32                                 pinNumber,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data             /* input data can be 0 or 1*/

    /**
     * @property    )
     *
     * @brief   Gets or sets the )
     *
     * @value   .
     */

);

/**
 * @fn  PDL_STATUS prvPdlGpioMppInit ( );
 *
 * @brief   Init MPPs SHOULD be called after data on all GPIO interfaces has been collected to DB
 *          Writes 0 in case MPP is used to the MPP init register and write I/O to the gpio
 *          input/output register
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlGpioMppInit(
);

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
);


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
);


/**
 * @fn  PDL_STATUS prvPdlGpioHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN PDL_INTERFACE_GPIO_OFFSET_ENT offset, IN UINT_32 data );
 *
 * @brief   set SMI value offset&amp;mask and value given will be written to HW using the
 *          information relevant to the GPIO interface based on information from HW access
 *
 * @param [in]  interfaceId - Interface identifier.
 * @param [in]  offset      - GPIO offset.
 * @param       data        The data.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlGpioHwSetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  PDL_INTERFACE_GPIO_OFFSET_ENT           offset,
    IN  UINT_32                                 data
);

/**
 * @fn  PDL_STATUS prvPdlGpioDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlGpioDestroy(
    void
);


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
);

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
);
/*$ END OF prvPdlGpioInterfaceRegister */

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
);
/*$ END OF prvPdlGpioInterfaceRegister */

/* @}*/
/* @}*/
/* @}*/

#endif
