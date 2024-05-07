/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\fan\private\prvPdlFanNCT7802.h.
 *
 * @brief   Declares the prvpdlfan class
 */

#ifndef __prvPdlFanNCT7802h
#define __prvPdlFanNCT7802h
/**
********************************************************************************
 * @file prvPdlFanNCT7802.h
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
 * @brief Platform driver layer - Fan Tc654 private declarations and APIs
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdlib/xml/private/prvXmlParser.h>

/**
* @addtogroup FanNCT7802
* @{
*/

/**
* @addtogroup prvFanNCT7802
* @{
*/

extern PDL_FAN_PRV_CALLBACK_FUNC_STC   pdlFanCallbacksNCT7802;

typedef enum {
    PDL_FAN_NCT7802_SENSOR_ID_LOCAL_E = 0,
    PDL_FAN_NCT7802_SENSOR_ID_REMOTE_1_E,
    PDL_FAN_NCT7802_SENSOR_ID_REMOTE_2_E,
    PDL_FAN_NCT7802_SENSOR_ID_REMOTE_3_E,
    PDL_FAN_NCT7802_SENSOR_ID_LAST_E,
} PDL_FAN_NCT7802_SENSOR_ID_ENT;


/**
 * @defgroup Fan_private Fan Private
 * @{Fan private definitions and declarations including:
*/

#define PDL_FAN_NCT7802_STATUS_REG_OFFSET_CNS               0x1A
#define PDL_FAN_NCT7802_STATUS_REG_FANS_Fault_BIT_MASK_CNS  0x7
#define PDL_FAN_NCT7802_STATUS_REG_FAN1_Fault_BIT_MASK_CNS  0x1
#define PDL_FAN_NCT7802_STATUS_REG_FAN2_Fault_BIT_MASK_CNS  0x2
#define PDL_FAN_NCT7802_STATUS_REG_FAN3_Fault_BIT_MASK_CNS  0x4

#define PDL_FAN_NCT7802_DUTY_CYCLE_FAN1_REG_OFFSET_CNS      0x60
#define PDL_FAN_NCT7802_DUTY_CYCLE_FAN2_REG_OFFSET_CNS      0x61
#define PDL_FAN_NCT7802_DUTY_CYCLE_FAN3_REG_OFFSET_CNS      0x62

#define PDL_FAN_NCT7802_FAN1_COUNT_MSB_REG_OFFSET_CNS       0x10
#define PDL_FAN_NCT7802_FAN2_COUNT_MSB_REG_OFFSET_CNS       0x11
#define PDL_FAN_NCT7802_FAN3_COUNT_MSB_REG_OFFSET_CNS       0x12
#define PDL_FAN_NCT7802_FANs_COUNT_LSB_REG_OFFSET_CNS       0x13

#define PDL_FAN_NCT7802_RPM_BASE_VALUE_CNS                  1350000

/**
 * @fn  PDL_STATUS prvPdlFanNCT7802DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802DbInit(
    IN  void
);

/**
 * @fn  PDL_STATUS prvPdlFanNCT7802HwGetRpm ()
 *
 * @brief   get rpm for NCT 7802
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanNumber           - fan number
 * @param [out] rpmPtr              - rpm for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802HwGetRpm(
    IN  UINT_32                           fanControllerId,
    IN  UINT_32                           fanNumber,
    OUT UINT_32                         * rpmPtr
);

/**
 * @fn  PDL_STATUS prvPdlFanNCT7802HwGetTemperature ()
 *
 * @brief   get Temperature for NCT 7802
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanSensorId         - fan sensor identifier.
 * @param [out] tempPtr             - Temperature for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802HwGetTemperature(
    IN  UINT_32                           fanControllerId,
    IN  UINT_32                           fanSensorId,
    OUT INT_32                          * tempPtr
);

/**
 * @fn  PDL_STATUS prvPdlFanNCT7802HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802HwStatusGet(
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
);


/**
 * @fn  PDL_STATUS prvPdlFanNCT7802HwDutyCycleGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT UINT_32 *speed_val_PTR, OUT UINT_32 *speedPercent_PTR )
 *
 * @brief   Pdl fan hardware duty cycle (speed) get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Fan number.
 * @param [out]     speed_val_PTR           speed value read from HW
 * @param [out]     speedPercent_PTR        speed percentage value from HW max speed
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802HwDutyCycleGet(
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    OUT UINT_32     *speed_val_PTR,
    OUT UINT_32     *speedPercent_PTR
);

/**
 * @fn  PDL_STATUS prvPdlFanControllerNCT7802HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speedRelativeVal )
 *
 * @brief   Pdl fan hardware duty cycle set
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 * @param [in]  speedRelativeVal        The speed relative value.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanControllerNCT7802HwDutyCycleSet(
    IN UINT_32      fanControllerId,
    IN UINT_32      speedAbsoulteVal
);


/**
 * @fn  PDL_STATUS prvPdlFanNCT7802HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802HwInit(
    IN UINT_32                  fanControllerId
);

/* @}*/
/* @}*/
/* @}*/

#endif
