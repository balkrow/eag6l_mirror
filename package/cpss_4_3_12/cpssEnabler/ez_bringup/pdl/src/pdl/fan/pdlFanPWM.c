/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlFanPWM.c
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
 * @brief Platform driver layer - Fan related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/fan/pdlFan.h>
#include <pdl/fan/private/prvPdlFanPWM.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/fan/pdlFanDebug.h>



/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksPWM = {
    &prvPdlFanPWMHwStatusGet,           /* fanHwStatusGetFun */
    NULL,                               /* fanHwStateSetFun */
    &prvPdlFanPWMHwDutyCycleGet,        /* fanHwDutyCycleGetFun */
    NULL,                               /* fanHwDutyCycleSetFun */
    NULL,                               /* fanHwRotationDirectionGetFun */
    NULL,                               /* fanHwRotationDirectionSetFun */
    NULL,                               /* fanHwDutyCycleMethodSetFun */
    NULL,                               /* fanHwFaultSetFun */
    NULL,                               /* fanHwPulsesPerRorationSetFun */
    NULL,                               /* fanHwThresholdSetFun */
    &prvPdlFanPWMHwInit,                /* fanHwInitFun */
    &prvPdlFanPWMDbInit,                /* fanDbInitFun */
    &prvPdlFanPWMDbDestroy,             /* fanDbDeInitFun */
    &prvPdlFanPWMHwGetRpm,              /* fanRpmGetFun */
    NULL                                /* fanTemperatureGetFun */

};


/** @brief   The pdl fan PWM database */
static PRV_PDLIB_DB_TYP pdlFanPWMDb;

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlFanDebugFlag)

#undef PDL_ASSERT_TRUE
#define PDL_ASSERT_TRUE(_condition) PDL_ASSERT_TRUE_GEN(_condition, prvPdlFanDebugFlag)
/**
 * @fn  PDL_STATUS pdlFanDbNumberOfFansGet ( OUT UINT_32 * numOfFansPtr )
 *
 * @brief   Pdl fan database number of fans get
 *
 * @param [in,out]  numOfFansPtr    If non-null, number of fans pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanPWMDbNumberOfFansGet(
    OUT  UINT_32                 *  numOfFansPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/

    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    return prvPdlibDbGetNumOfEntries(&pdlFanPWMDb, numOfFansPtr);
}
/*$ END OF pdlFanDbNumberOfFansGet */

/**
 * @fn  PDL_STATUS prvPdlFanPWMHwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr )
 *
 * @brief   Pdl fan hardware state get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               Identifier for the fan.
 * @param [in,out]  statePtr                If non-null, the state pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwStatusGet(
    IN  UINT_32                     fanControllerId,
    IN  UINT_32                     fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT    * statePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             statusRegData, maskBit, fanMaskBit;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (statePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "statePtr NULL, FAN Number: %d", fanNumber);
        return PDL_BAD_PTR;
    }

    *statePtr = PDL_FAN_STATE_INFO_OK_E;

    maskBit = PDL_FAN_PWM_STATUS_REG_FANs_Fault_BIT_MASK_CNS;

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_PWM_STATUS_REG_OFFSET_CNS, maskBit, &statusRegData);
    PDL_CHECK_STATUS(pdlStatus);

    PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "FAN ID: %d, status reg %#x", fanNumber, statusRegData);

    /* mask the error bit in each specific status register according to the input fan */
    switch (fanNumber) {
        case 1:
            fanMaskBit = PDL_FAN_PWM_STATUS_REG_FAN1_Fault_BIT_MASK_CNS;
            break;

        case 2:
            fanMaskBit = PDL_FAN_PWM_STATUS_REG_FAN2_Fault_BIT_MASK_CNS;
            break;

        case 3:
            fanMaskBit = PDL_FAN_PWM_STATUS_REG_FAN3_Fault_BIT_MASK_CNS;
            break;

        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "Illegal fan ID %d", fanNumber);
            return PDL_BAD_VALUE;
    }

    /* Fault fan status? */
    if (statusRegData & fanMaskBit) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "FAN Number: %d, fault, mask=%#x, data = %#x", fanNumber, fanMaskBit, statusRegData);
        *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
    }

    return PDL_OK;
}

/*$ END OF pdlFanPWMHwStateGet */

/**
 * @fn  PDL_STATUS prvPdlFanPWMHwDutyCycleGet ( IN UINT_32 fanControllerId, IN UINT_32 fanId, OUT UINT_32 *speed_val_PTR, OUT UINT_32 *speedPercent_PTR )
 *
 * @brief   Pdl fan hardware duty cycle (speed) get
 *
 * @param           fanControllerId         Identifier for the fan controller.
 * @param           fanNumber               fan number.
 * @param [out]     speed_val_PTR           speed value read from HW
 * @param [out]     speedPercent_PTR        speed percentage value from HW max speed
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwDutyCycleGet(
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    OUT UINT_32     *speed_val_PTR,
    OUT UINT_32     *speedPercent_PTR
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_FAN_PWM_PRV_DB_STC        * fanPtr;
    PDL_FAN_PWM_PRV_KEY_STC         fanKey;
    PDL_STATUS                      pdlStatus;
    PDL_FAN_INTERFACE_STC           fanInterface;
    UINT_32                         Offset, maskBit, RegData;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    fanKey.controllerId = fanControllerId;
    pdlStatus = prvPdlibDbFind(pdlFanPWMDb, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    maskBit = PDL_FAN_PWM_DUTY_CYCLE_BIT_MASK_CNS;
    Offset = PDL_FAN_PWM_DUTY_CYCLE_REGISTER_OFFSET_MAC(fanNumber);

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, Offset, maskBit, &RegData);
    PDL_CHECK_STATUS(pdlStatus);


    if (fanPtr->fanData.config_state[fanNumber] == PDL_FAN_STATE_CONTROL_SHUTDOWN_E) {
        *speed_val_PTR = 0;
        *speedPercent_PTR = 0;
    }
    else {
        *speed_val_PTR = RegData;
        *speedPercent_PTR = PDL_FAN_PWM_DUTY_CYCLE_PERCENT_MAC(RegData);
    }

    PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "FAN ID: %d, data reg %#x, speedPercent=%d%", fanNumber, RegData, *speedPercent_PTR);

    return PDL_OK;
}

/*$ END OF prvPdlFanPWMHwDutyCycleGet */

/**
 * @fn  PDL_STATUS prvPdlFanPWMHwGetRpm ()
 *
 * @brief   get rpm for PWM
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanNumber           - fan number
 * @param [out] rpmPtr              - rpm for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwGetRpm(
    IN  UINT_32                           fanControllerId,
    IN  UINT_32                           fanNumber,
    OUT UINT_32                         * rpmPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                         fanSpeed, speedPercent;
    PDL_STATUS                      pdlStatus;

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (rpmPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlFanPWMHwDutyCycleGet(fanControllerId, fanNumber, &fanSpeed, &speedPercent);
    PDL_CHECK_STATUS(pdlStatus);

    /*
    * Count Frequency is 1KHz= 1ms
    * Count Period =400 cycle=400*1ms=400ms
    * R.P.M value = read value x 2.5*60/2
    * 2.5= 1000ms/400ms
    * 60 = 1min =60s
    * 2 = 1 rotation of fan has two pulses.
    */
    *rpmPtr = fanSpeed * 75;
    return PDL_OK;
}

/*$ END OF prvPdlFanPWMHwGetRpm */


/**
 * @fn  PDL_STATUS prvPdlFanPWMHwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanPWMHwInit(
    IN UINT_32                  fanControllerId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    UINT_32                         pdl_fan_id, pdl_fan_num = 0xFF, pdl_fan_controller;
    PDL_FAN_STATE_INFO_ENT          state;
    PDL_FAN_PWM_PRV_DB_STC          fanDbStruct;
    PDL_FAN_PWM_PRV_KEY_STC       * fanControllerPtr = NULL;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    fanDbStruct.fanKey.controllerId = fanControllerId;
    memset(&fanDbStruct, 0, sizeof(fanDbStruct));
    /*add to db*/
    pdlStatus = prvPdlibDbAdd(pdlFanPWMDb, (void*) &fanDbStruct.fanKey, (void*) &fanDbStruct, (void**) &fanControllerPtr);
    PDL_CHECK_STATUS(pdlStatus);

    /* read the status register one time in order to clear it */
    pdlStatus = pdlFanDbGetFirst(&pdl_fan_id);
    while (pdlStatus == PDL_OK) {
        pdlStatus = pdlFanIdToFanControllerAndNumber(pdl_fan_id, &pdl_fan_controller, &pdl_fan_num);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_ASSERT_TRUE(pdl_fan_num == 1 || pdl_fan_num == 2);
        if (pdl_fan_controller == fanControllerId) {
            break;
        }

        pdlStatus = pdlFanDbGetNext(pdl_fan_id, &pdl_fan_id);
        PDL_CHECK_STATUS(pdlStatus);
    }

    pdlStatus = prvPdlFanPWMHwStatusGet(fanControllerId, pdl_fan_num, &state);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlFanPWMHwInit */

/**
 * @fn  PDL_STATUS pdlFanDebugInterfaceGet ( IN UINT_32 fanId, OUT PDL_FAN_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl fan debug interface get
 *
 * @param           fanId           Identifier for the fan.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanPWMDebugInterfaceGet(
    IN  UINT_32                         fanId,
    OUT PDL_FAN_PWM_INTERFACE_STC   * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_PWM_PRV_DB_STC            * fanPtr;
    PDL_FAN_PWM_PRV_KEY_STC             fanKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "interfacePtr NULL, FAN ID: %d", fanId);
        return PDL_BAD_PTR;
    }
    fanKey.controllerId = fanId;
    pdlStatus = prvPdlibDbFind(pdlFanPWMDb, (void*) &fanKey, (void**) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(interfacePtr, &fanPtr->fanInterface, sizeof(fanPtr->fanInterface));
    return PDL_OK;

}

/*$ END OF pdlFanDebugInterfaceGet */

/**
 * @fn  PDL_STATUS prvPdlFanPWMDbInit ( void )
 *
 * @brief   Pdl fan initialize
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanPWMDbInit(
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
    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_PWM_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_PWM_PRV_KEY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &pdlFanPWMDb);
    PDL_CHECK_STATUS(pdlStatus);

    pdlFanDebugSet(TRUE);

    return PDL_OK;
}
/*$ END OF prvPdlFanPWMDbInit */

/**
 * @fn  PDL_STATUS prvPdlFanPWMDbDestroy ( void )
 *
 * @brief   Pdl fan deinitialize
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanPWMDbDestroy(
    IN  void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    pdlStatus = prvPdlibDbDestroy(pdlFanPWMDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlFanPWMDbInit */

