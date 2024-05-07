/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\fan\private\prvpdlfan.h.
 *
 * @brief   Declares the prvpdlfan class
 */

#ifndef __prvPdlFanh

#define __prvPdlFanh
/**
********************************************************************************
 * @file prvPdlFan.h
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
 * @brief Platform driver layer - Fan private declarations and APIs
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdl/fan/pdlFan.h>
#include <pdl/lib/private/prvPdlLib.h>
/**
* @addtogroup Fan
* @{
*/

/**
* @addtogroup prvFan
* @{
*/

#define PDL_FAN_TAG_NAME                            "fans"

/*! Tag of one fan !*/
#define PDL_FAN_UNIT_TAG_NAME                       "fan"

/*! Tag for fan number !*/
#define PDL_FAN_NUMBER_TAG_NAME                     "fan-number"

typedef struct {
    PDL_INTERFACE_EXTENDED_TYPE_ENT         interfaceType;
    PDL_INTERFACE_TYP                       interfaceId;
} PDL_FAN_INTERFACE_STC;


typedef struct {
    PDL_FAN_CONTROLLER_TYPE_ENT                     fanType;
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP            externalDriverId;
} PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC;

/**
 * @struct  PDL_FAN_ADT7476_PRV_KEY_STC
 *
 * @brief   defines structure for fan key in hash
 */

typedef struct {
    UINT_32                     fanControllerId;
} PDL_FAN_CONTROLLER_PRV_KEY_STC;


/**
 * @struct  PDL_FAN_CONTROLLER_PRV_DB_STC
 *
 * @brief   defines structure stored for fan controller in hash
 */
typedef struct {
    PDL_FAN_CONTROLLER_PRV_KEY_STC          key;
    PDL_FAN_CONTROLLER_TYPE_PRV_KEY_STC     fanTypeKey;
    PDL_FAN_INTERFACE_STC                   fanInterface;
} PDL_FAN_CONTROLLER_PRV_DB_STC;

/**
 * @struct  PDL_FAN_ADT7476_PRV_KEY_STC
 *
 * @brief   defines structure for fan key in hash
 */

typedef struct {
    /** @brief   Identifier for the fan */
    UINT_32                        fanNumber;
} PDL_FAN_CONNECTED_FANS_PRV_KEY_STC;


/**
 * @struct  PDL_FAN_CONNECTED_FANS_PRV_DB_STC
 *
 * @brief   defines structure stored for fan connected fans in hash
 */
typedef struct {
    PDL_FAN_CONNECTED_FANS_PRV_KEY_STC      connectedFanKey;
    UINT_32                                 fanNumber;
    UINT_32                                 fanUniqueId;
    PDL_FAN_ROLE_ENT                        fanRole;

} PDL_FAN_CONNECTED_FANS_PRV_DB_STC;

typedef struct {
    UINT_32                        fanId;
} PDL_FAN_ID_PRV_KEY_STC;

typedef struct {
    UINT_32                        fanId;
} PDL_FAN_ID_PRV_DB_STC;

typedef struct {
    UINT_32                         thresholdId;
} PDL_FAN_THRESHOLD_PRV_KEY_STC;


typedef struct {
    PDL_FAN_THRESHOLD_PRV_KEY_STC   thresholdKey;
    PDL_SW_FAN_THRESHOLD_INFO_STC   publicInfo;
    PRV_PDLIB_DB_TYP                  sensorDb;
} PDL_FAN_THRESHOLD_PRV_INFO_STC;

typedef struct {
    pdlFanHwStatusGet_FUN                       * fanHwStatusGetFun;
    pdlFanHwStateSet_FUN                        * fanHwStateSetFun;
    pdlFanHwDutyCycleGet_FUN                    * fanHwDutyCycleGetFun;
    pdlFanControllerHwDutyCycleSet_FUN          * fanControllerHwDutyCycleSetFun;
    pdlFanControllerHwRotationDirectionGet_FUN  * fanControllerHwRotationDirectionGetFun;
    pdlFanControllerHwRotationDirectionSet_FUN  * fanControllerHwRotationDirectionSetFun;
    pdlFanControllerHwDutyCycleMethodSet_FUN    * fanControllerHwDutyCycleMethodSetFun;
    pdlFanControllerHwFaultSet_FUN              * fanControllerHwFaultSetFun;
    pdlFanHwPulsesPerRotationSet_FUN            * fanHwPulsesPerRorationSetFun;
    pdlFanHwThresholdSet_FUN                    * fanHwThresholdSetFun;
    pdlFanControllerHwInit_FUN                  * fanControllerHwInitFun;
    pdlFanDbInit_FUN                            * fanDbInitFun;
    pdlFanDbDeInit_FUN                          * fanDbDeInitFun;
    pdlFanHwRpmGet_FUN                          * fanRpmGetFun;
    pdlFanHwTemperatureGet_FUN                  * fanTemperatureGetFun;
} PDL_FAN_PRV_CALLBACK_FUNC_STC;


typedef struct {
    PDL_FAN_CALLBACK_INFO_STC                     externalCallbackInfo;
} PDL_FAN_PRV_CALLBACK_INFO_STC;


/* ***************************************************************************
* FUNCTION NAME: prvPdlFanGetInterface
*
* DESCRIPTION:  return FAN interface information
*
* PARAMETERS:
*
* RETURNES:     PDL_OK                      -       SUCCESS
*               PDL_BAD_PTR                 -       NULL pointers as param
*
*****************************************************************************/

PDL_STATUS prvPdlFanGetInterface(
    IN  UINT_32                           fanControllerId,
    OUT PDL_FAN_INTERFACE_STC           * fanInterfacePtr
);

/*$ END OF prvPdlFanGetInterface */

/**
 * @fn  PDL_STATUS pdlFanDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Fan debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanDebugSet(
    IN  BOOLEAN             state
);


/**
 * @fn  PDL_STATUS prvPdlFanBindInternalFanDriver
 *
 * @brief   binds new internal fan driver, function that aren't implemented by specific fan driver
 *          should be NULL pointers
 *
 *
 * @param [in] callbacksInfoPtr        - all callbacks relevant to this specific fan
 * @param [in] fanType                 - type of the fan
 *
 * @return
 */

PDL_STATUS prvPdlFanBindInternalFanDriver(
    IN PDL_FAN_PRV_CALLBACK_FUNC_STC           * callbacksInfoPtr,
    IN PDL_FAN_CONTROLLER_TYPE_ENT               fanType
);

/* ***************************************************************************
* FUNCTION NAME: prvPdlFanVerifyExternalDrivers
*
* DESCRIPTION:   verify all external drivers have been binded
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlFanVerifyExternalDrivers(
    IN void
);

/**
 * @fn  PDL_STATUS pdlFanInit ( void );
 *
 * @brief   Init fan modules
 *          call all registered fan init functions
 *          create local DB for all fans
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlFanInit(
    void
);

/**
 * @fn  PDL_STATUS prvPdlFanDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanDestroy(
    void
);

/* @}*/
/* @}*/
/* @}*/
#endif
