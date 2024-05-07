/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlFanNCT7802.c
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
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/interface/private/prvPdlI2c.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdl/fan/pdlFanDebug.h>
#include <pdl/fan/private/prvPdlFanNCT7802.h>


/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksNCT7802 = {
    &prvPdlFanNCT7802HwStatusGet,                   /* fanHwStatusGetFun */
    NULL,                                           /* fanHwStateSetFun */
    &prvPdlFanNCT7802HwDutyCycleGet,                /* fanHwDutyCycleGetFun */
    &prvPdlFanControllerNCT7802HwDutyCycleSet,      /* fanHwDutyCycleSetFun */
    NULL,                                           /* fanHwRotationDirectionGetFun */
    NULL,                                           /* fanHwRotationDirectionSetFun */
    NULL,                                           /* fanHwDutyCycleMethodSetFun */
    NULL,                                           /* fanHwFaultSetFun */
    NULL,                                           /* fanHwPulsesPerRorationSetFun */
    NULL,                                           /* fanHwThresholdSetFun */
    &prvPdlFanNCT7802HwInit,                        /* fanHwInitFun */
    &prvPdlFanNCT7802DbInit,                        /* fanDbInitFun */
    NULL,
    &prvPdlFanNCT7802HwGetRpm,                      /* fanRpmGetFun */
    &prvPdlFanNCT7802HwGetTemperature               /* fanTemperatureGetFun */
};


/** @brief   The pdl fan NCT7802 database */
static PRV_PDLIB_DB_STC pdlFanNCT7802Db =  {0,0};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)                                                                   \
    if (__pdlStatus != PDL_OK) {                                                                        \
    if (prvPdlFanDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __pdlStatus;                                                                                 \
    }
/*macro to check the xml status and debug prints if debug flag on*/
#undef XML_CHECK_STATUS
#define XML_CHECK_STATUS(__xmlStatus)                                                                   \
    if (__xmlStatus != PDL_OK) {                                                                        \
    if (prvPdlFanDebugFlag) {                                                                           \
    prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__xmlStatus,XML_STATUS_ERROR_PRINT_IDENTIFIER);       \
    }                                                                                                   \
    return __xmlStatus;                                                                                 \
    }

/**
 * @fn  PDL_STATUS pdlFanNCT7802DbNumberOfFansGet ( OUT UINT_32 * numOfFansPtr )
 *
 * @brief   Pdl fan database number of fans get
 *
 * @param [in,out]  numOfFansPtr    If non-null, number of fans pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanNCT7802DbNumberOfFansGet(
    OUT  UINT_32                 *  numOfFansPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/

    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    return prvPdlibDbGetNumOfEntries(&pdlFanNCT7802Db, numOfFansPtr);
}
/*$ END OF pdlFanNCT7802DbNumberOfFansGet */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             statusRegData, fanMaskBit;
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

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_NCT7802_STATUS_REG_OFFSET_CNS, PDL_FAN_NCT7802_STATUS_REG_FANS_Fault_BIT_MASK_CNS, &statusRegData);
    PDL_CHECK_STATUS(pdlStatus);

    PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "FAN ID: %d, status reg %#x", fanNumber, statusRegData);

    /* mask the error bit in each specific status register according to the input fan */
    switch (fanNumber) {
        case 1:
            fanMaskBit = PDL_FAN_NCT7802_STATUS_REG_FAN1_Fault_BIT_MASK_CNS;
            break;

        case 2:
            fanMaskBit = PDL_FAN_NCT7802_STATUS_REG_FAN2_Fault_BIT_MASK_CNS;
            break;
        case 3:
            fanMaskBit = PDL_FAN_NCT7802_STATUS_REG_FAN3_Fault_BIT_MASK_CNS;
            break;

        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "Illegal fan ID %d", fanNumber);
            return PDL_BAD_VALUE;
    }

    if (statusRegData & fanMaskBit) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "FAN Number: %d, overflowed, mask=%#x, data = %#x", fanNumber, fanMaskBit, statusRegData);
        *statePtr = PDL_FAN_STATE_INFO_OVERFLOW_E;
    }
    return PDL_OK;
}

/*$ END OF pdlFanNCT7802HwStateGet */

/**
 * @fn  PDL_STATUS prvPdlFanNCT7802HwDutyCycleGet ( IN UINT_32 fanControllerId, IN UINT_32 fanId, OUT UINT_32 *speed_val_PTR, OUT UINT_32 *speedPercent_PTR )
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

PDL_STATUS prvPdlFanNCT7802HwDutyCycleGet(
    IN UINT_32      fanControllerId,
    IN UINT_32      fanNumber,
    OUT UINT_32     *speedValPtr,
    OUT UINT_32     *speedPercentPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
    UINT_32                             offset;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (speedValPtr == NULL || speedPercentPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);
    switch (fanNumber) {
        case 1:
            offset = PDL_FAN_NCT7802_DUTY_CYCLE_FAN1_REG_OFFSET_CNS;
            break;
        case 2:
            offset = PDL_FAN_NCT7802_DUTY_CYCLE_FAN2_REG_OFFSET_CNS;
            break;
        case 3:
            offset = PDL_FAN_NCT7802_DUTY_CYCLE_FAN1_REG_OFFSET_CNS;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "Illegal fan ID %d", fanNumber);
            return PDL_BAD_VALUE;
    }
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, offset, 0xFF, speedValPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *speedPercentPtr = *speedValPtr * 100 /255; /* 255 (0xff) is maximum duty cycle */
    return PDL_OK;
}

/*$ END OF prvPdlFanNCT7802HwDutyCycleGet */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    /* set all controller fan speeds to the given value */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_NCT7802_DUTY_CYCLE_FAN1_REG_OFFSET_CNS, 0xFF, speedAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_NCT7802_DUTY_CYCLE_FAN2_REG_OFFSET_CNS, 0xFF, speedAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, PDL_FAN_NCT7802_DUTY_CYCLE_FAN3_REG_OFFSET_CNS, 0xFF, speedAbsoulteVal);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlFanControllerNCT7802HwDutyCycleSet */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             msbOffset, lsbOffset, msbValue, lsbValue, val;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (rpmPtr == NULL) {
        return PDL_BAD_PTR;
    }
    switch (fanNumber) {
        case 1:
            msbOffset = PDL_FAN_NCT7802_FAN1_COUNT_MSB_REG_OFFSET_CNS;
            lsbOffset = PDL_FAN_NCT7802_FANs_COUNT_LSB_REG_OFFSET_CNS;
            break;
        case 2:
            msbOffset = PDL_FAN_NCT7802_FAN2_COUNT_MSB_REG_OFFSET_CNS;
            lsbOffset = PDL_FAN_NCT7802_FANs_COUNT_LSB_REG_OFFSET_CNS;
            break;
        case 3:
            msbOffset = PDL_FAN_NCT7802_FAN3_COUNT_MSB_REG_OFFSET_CNS;
            lsbOffset = PDL_FAN_NCT7802_FANs_COUNT_LSB_REG_OFFSET_CNS;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "Illegal fan ID %d", fanNumber);
            return PDL_BAD_VALUE;
    }
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, msbOffset, 0xFF, &msbValue);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, lsbOffset, 0xFF, &lsbValue);
    PDL_CHECK_STATUS(pdlStatus);
    val = msbValue << 4 | lsbValue >> 3;
    *rpmPtr = PDL_FAN_NCT7802_RPM_BASE_VALUE_CNS / val;
    return PDL_OK;
}

/*$ END OF prvPdlFanNCT7802HwGetRpm */

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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             val, offset;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (tempPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);
    switch ((PDL_FAN_NCT7802_SENSOR_ID_ENT)fanSensorId) {
        case PDL_FAN_NCT7802_SENSOR_ID_REMOTE_1_E:
            offset = 0x1;
            break;
        case PDL_FAN_NCT7802_SENSOR_ID_REMOTE_2_E:
            offset = 0x2;
            break;
        case PDL_FAN_NCT7802_SENSOR_ID_REMOTE_3_E:
            offset = 0x3;
            break;
        case PDL_FAN_NCT7802_SENSOR_ID_LOCAL_E:
            offset = 0x4;
            break;
        default:
            return PDL_BAD_PARAM;
    }

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, offset, 0xFF, &val);
    PDL_CHECK_STATUS(pdlStatus);
    /* check if temperature is negative */
    if (val & 0x80) {
        *tempPtr = (val & 0x7F) - 128;
    }
    else {
        *tempPtr = val;
    }

    return PDL_OK;
}

/*$ END OF prvPdlFanNCT7802HwGetTemperature */


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
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             numOfConnectedFans, temp1FanSelectValue;
    PDL_STATUS                          pdlStatus;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    /* nct7802 fan init sequence (SMART FAN IV Summary Register) */
    /* Temperature point 1 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x80, 0xFF, 0);
    PDL_CHECK_STATUS(pdlStatus);
    /* Temperature point 2 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x81, 0xFF, 0x17);
    PDL_CHECK_STATUS(pdlStatus);
    /* Temperature point 3 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x82, 0xFF, 0x23);
    PDL_CHECK_STATUS(pdlStatus);
    /* Temperature point 4 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x83, 0xFF, 0x2D);
    PDL_CHECK_STATUS(pdlStatus);
    /* Critical Temperature point */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x84, 0xFF, 0x37);
    PDL_CHECK_STATUS(pdlStatus);
    /* FANCTL point 1 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x85, 0xFF, 0x33);
    PDL_CHECK_STATUS(pdlStatus);
    /* FANCTL point 2 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x86, 0xFF, 0x62);
    PDL_CHECK_STATUS(pdlStatus);
    /* FANCTL point 3 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x87, 0xFF, 0x8E);
    PDL_CHECK_STATUS(pdlStatus);
    /* FANCTL point 4 */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x88, 0xFF, 0xD9);
    PDL_CHECK_STATUS(pdlStatus);
    /* Hysteresis value of critical and operating temperature.*/
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x74, 0xFF, 0x53);
    PDL_CHECK_STATUS(pdlStatus);
    /*Temperature source selection */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x66, 0xFF, 0);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x67, 0xFF, 0);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x68, 0xFF, 0);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x69, 0xFF, 0);
    PDL_CHECK_STATUS(pdlStatus);
    /* Turn on the fan with the specified fan duty */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x77, 0xFF, 0x30);
    PDL_CHECK_STATUS(pdlStatus);
    /* Determines the amount of time FANOUT takes to increase its value by one step.*/
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x6e, 0xFF, 0x05);
    PDL_CHECK_STATUS(pdlStatus);
    /* Determines the amount of time FANOUT takes to increase its value by one step.*/
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x6f, 0xFF, 0x05);
    PDL_CHECK_STATUS(pdlStatus);
    /* Keep the fan working when the system does not require the fan to help reduce heat
       1 indicates the FAN1 Nons top functiondisabled */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x79, 0xFF, 0x01);
    PDL_CHECK_STATUS(pdlStatus);
    /* Turn off the fan if temperature keeps lower than T1 in stop time
       Unit in 0.1sec. Ranges from 0.1sec to 25.5 sec . (0 is never) */
    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x78, 0xFF, 0xff);
    PDL_CHECK_STATUS(pdlStatus);

    /* Assign Smart_Fan1 to FANCTL1 0 = FANCTL has no relation with this
       temperature source. FANCTL is controlled by Manual mode (Default)
       1 = Applies SMART FAN IV control for SMART FAN IV on FANCTL and this temperature */
    pdlStatus = pdlFanDbGetNumOfControllerFans(fanControllerId, &numOfConnectedFans);
    PDL_CHECK_STATUS(pdlStatus);
    switch (numOfConnectedFans) {
        case 1:
            temp1FanSelectValue = 1;
            break;
        case 2:
            temp1FanSelectValue = 3;
            break;
        case 3:
            temp1FanSelectValue = 7;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "Illegal number of connected fans %d on fanController %d", numOfConnectedFans, fanControllerId);
            return PDL_BAD_VALUE;
    }

    pdlStatus =  pdlI2CHwSetValueWithOffsetMask(fanInterface.interfaceId, 0x64, 0xFF, temp1FanSelectValue);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlFanNCT7802HwInit */


/**
 * @fn  PDL_STATUS prvPdlFanNCT7802DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanNCT7802DbInit(
    IN  void
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

/*$ END OF prvPdlFanNCT7802DbInit */

