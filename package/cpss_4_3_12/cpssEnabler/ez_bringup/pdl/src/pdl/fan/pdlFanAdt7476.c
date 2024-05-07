/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlFanController.c
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
 * @brief Platform driver layer - Sensor related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/fan/private/prvPdlFanAdt7476.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/xmlParser.h>
#include <pdl/fan/private/prvPdlFan.h>
#include <pdl/fan/pdlFanDebug.h>
#include <pdl/interface/private/prvPdlI2c.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/
/** @brief   The pdl fan adt7476 database */
static PRV_PDLIB_DB_TYP pdlFanAdt7476Db;
/*callbacks struct*/
PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksAdt7476 = {
    &prvPdlFanAdt7476HwStatusGet,                   /* fanHwStatusGetFun */
    &prvPdlFanAdt7476HwStateSet,                    /* fanHwStateSetFun */
    &prvPdlFanAdt7476HwDutyCycleGet,                /* fanHwDutyCycleGetFun */
    &prvPdlFanAdt7476HwDutyCycleSet,                /* fanHwDutyCycleSetFun */
    NULL,                                           /* fanHwRotationDirectionGetFun */
    NULL,                                           /* fanHwRotationDirectionSetFun */
    NULL,                                           /* fanHwDutyCycleMethodSetFun */
    NULL,                                           /* fanHwFaultSetFun */
    NULL,                                           /* fanHwPulsesPerRorationSetFun */
    NULL,                                           /* fanHwThresholdSetFun */
    &prvPdlFanAdt7476HwInit,                        /* fanControllerHwInitFun */
    &prvPdlFanAdt7476DbInit,                        /* fanDbInitFun */
    &prvPdlFanAdt7476DbDestroy,                     /* fanDbDeInitFun */
    NULL,                                           /* fanRpmGetFun */
    &prvPdlFanADT7476HwGetTemperature               /* fanTemperatureGetFun */
};

/*PDL_FAN_PRV_CALLBACK_FUNC_STC pdlFanCallbacksAdt7476 = {pdlFanAdt7476DbCapabilitiesGet,NULL,NULL,NULL,NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlFanDebugFlag)


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwStatusGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT PDL_FAN_STATE_INFO_ENT * statePtr );
 *
 * @brief   Get fan operational state (ok/overflow/fault)
 *
 * @param [in]      fanControllerId       - fan controller identifier.
 * @param [in]      fanNumber             - fan identifier.
 * @param [in,out]  statePtr              - If non-null, the state pointer.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476HwStatusGet(
    IN  UINT_32                      fanControllerId,
    IN  UINT_32                      fanNumber,
    OUT PDL_FAN_STATE_INFO_ENT     * statePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    UINT_32                             maskBit;
    UINT_32                             state;
    PDL_FAN_INTERFACE_STC               fanInterface;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (statePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "statePtr NULL, FAN number: %d", fanNumber);
        return PDL_BAD_PTR;
    }
    switch (fanNumber) {
        case 1:
            maskBit = 2;
            break;
        case 2:
            maskBit = 3;
            break;
        case 3:
            maskBit = 4;
            break;
        case 4:
            maskBit = 5;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL FAN number: %d", fanNumber);
            return PDL_BAD_VALUE;
    }
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, 0x42, 1<<maskBit, &state);
    PDL_CHECK_STATUS(pdlStatus);
    switch (state) {
        case 0:
            *statePtr = PDL_FAN_STATE_INFO_OK_E;
            break;
        default:
            *statePtr = PDL_FAN_STATE_INFO_FAULT_E;
            break;
    }
    return PDL_OK;
}


/**
 * @fn  PDL_STATUS prvPdlFanADT7476HwGetTemperature ()
 *
 * @brief   get Temperature for ADT7476
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanSensorId         - fan sensor identifier.
 * @param [out] tempPtr             - Temperature for current fan
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanADT7476HwGetTemperature(
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
    switch ((PDL_FAN_ADT7476_SENSOR_ID_ENT)fanSensorId) {
        case PDL_FAN_ADT7476_SENSOR_ID_REMOTE_1_E:
            offset = 0x25;
            break;
        case PDL_FAN_ADT7476_SENSOR_ID_LOCAL_E:
            offset = 0x26;
            break;
        case PDL_FAN_ADT7476_SENSOR_ID_REMOTE_2_E:
            offset = 0x27;
            break;
        default:
            return PDL_BAD_PARAM;
    }

    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, offset, 0xFF, &val);
    PDL_CHECK_STATUS(pdlStatus);

    * tempPtr = (INT_8) val;

    return PDL_OK;
}

/*$ END OF prvPdlFanADT7476HwGetTemperature */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476SetStat ( IN PRV_PDLIB_DB_STC * statementsDbPtr, IN PDL_FAN_ADT7476_PRV_DB_STC * fanPtr )
 * @brief   Pdl set statement value to entire statements db
 * @param [IN]     PRV_PDLIB_DB_TYP                    statementsDb      the fan's statement db handle.
 * @param [IN]     PDL_FAN_ADT7476_PRV_DB_STC    *   fanPtr            the fan pointer .
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476SetState(
    IN   PRV_PDLIB_DB_TYP                 statementsDb,
    IN   PDL_FAN_ADT7476_PRV_DB_STC  *  fanPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    UINT_32                             i, numOfEntries;
    PDL_FAN_ADT7476_PRV_STATEMENT_STC * statement;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    PDL_UNUSED_PARAM(fanPtr);

    pdlStatus = prvPdlibDbGetNumOfEntries(statementsDb, &numOfEntries);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibDbGetFirst(statementsDb, (void **)&statement);
    for (i=0; i < numOfEntries; i++) {
        pdlStatus = prvPdlInterfaceHwSetValue(PDL_INTERFACE_TYPE_INTERFACE_I2C_E, statement->i2cInterfaceId, statement->value);
        PDL_CHECK_STATUS(pdlStatus);
        /* don't getnext for the last entry in db (it'll fail) */
        if (i < numOfEntries - 1) {
            pdlStatus = prvPdlibDbGetNext(statementsDb, (void *)&(statement->statementKey), (void**) &statement);
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476SetState */


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwStateSet ( IN UINT_32 fanControllerId, IN PDL_FAN_STATE_CONTROL_ENT state );
 *
 * @brief   Configure fan state (normal/shutdown)
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  state               - state to configure.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476HwStateSet(
    IN  UINT_32                         fanControllerId,
    IN UINT_32                          fanNumber,
    IN  PDL_FAN_STATE_CONTROL_ENT       state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC        * fanPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    PDL_UNUSED_PARAM(fanNumber);

    fanKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlibDbFind(pdlFanAdt7476Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (state == PDL_FAN_STATE_CONTROL_NORMAL_E) {
        pdlStatus = prvPdlFanAdt7476SetState(fanPtr->allNormalSpeedStatementDb, fanPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else if (state == PDL_FAN_STATE_CONTROL_SHUTDOWN_E) {
        return PDL_NOT_SUPPORTED;
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL STATE");
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476HwStateSet */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwInit ( IN UINT_32 fanControllerId )
 *
 * @brief   Pdl fan hardware init
 *
 * @param [in]  fanControllerId         Identifier for the fan controller.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476HwInit(
    IN UINT_32                  fanControllerId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC        * fanPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    fanKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlibDbFind(pdlFanAdt7476Db, (void*) &fanKey, (void*) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlFanAdt7476SetState(fanPtr->initStatementDb, fanPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476HwInit */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwDutyCycleGet ( IN UINT_32 fanControllerId, IN UINT_32 fanNumber, OUT UINT_32 * speedPtr, OUT UINT_32 * speedPercentPtr );
 *
 * @brief   Get fan speed
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  fanNumber           - fan number.
 * @param [out] speedPtr            - fan speed.
 * @param [out] speedPercentPtr     - fan speed percents.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476HwDutyCycleGet(
    IN UINT_32                      fanControllerId,
    IN UINT_32                      fanNumber,
    OUT UINT_32                   * speedPtr,
    OUT UINT_32                   * speedPercentPtr
)
{

    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_FAN_INTERFACE_STC           fanInterface;
    UINT_32                         low_byte_offset, high_byte_offset, min_low_byte_offset, min_high_byte_offset, pulses_byte_offset;
    UINT_32                         low_byte_val, high_byte_val, min_low_byte_val, min_high_byte_val, pulses_byte_val, pulses_calc_val;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (speedPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "speedPtr NULL, FAN number: %d", fanNumber);
        return PDL_BAD_PTR;
    }
    if (speedPercentPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "speedPercentPtr NULL, FAN number: %d", fanNumber);
        return PDL_BAD_PTR;
    }

    switch (fanNumber) {
        case 1:
            low_byte_offset = 0x28;             /* TACH1 Low Byte */
            high_byte_offset = 0x29;            /* TACH1 High Byte */
            min_low_byte_offset = 0x54;         /* TACH1 Minimum Low Byte */
            min_high_byte_offset = 0x55;        /* TACH1 Minimum High Byte */
            break;
        case 2:
            low_byte_offset = 0x2A;             /* TACH2 Low Byte */
            high_byte_offset = 0x2B;            /* TACH2 High Byte */
            min_low_byte_offset = 0x56;         /* TACH2 Minimum Low Byte */
            min_high_byte_offset = 0x57;        /* TACH2 Minimum High Byte */
            break;
        case 3:
            low_byte_offset = 0x2C;             /* TACH3 Low Byte */
            high_byte_offset = 0x2D;            /* TACH3 High Byte */
            min_low_byte_offset = 0x58;         /* TACH3 Minimum Low Byte */
            min_high_byte_offset = 0x59;        /* TACH3 Minimum High Byte */
            break;
        case 4:
            low_byte_offset = 0x2E;             /* TACH4 Low Byte */
            high_byte_offset = 0x2F;            /* TACH4 High Byte */
            min_low_byte_offset = 0x5A;         /* TACH4 Minimum Low Byte */
            min_high_byte_offset = 0x5B;        /* TACH4 Minimum High Byte */
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL FAN number: %d", fanNumber);
            return PDL_BAD_VALUE;
    }

    /* get FAN interface id */
    pdlStatus = prvPdlFanGetInterface(fanControllerId, &fanInterface);
    PDL_CHECK_STATUS(pdlStatus);

    /* get TACH[x] Low Byte */
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, low_byte_offset, 0xFF, &low_byte_val);
    PDL_CHECK_STATUS(pdlStatus);

    /* get TACH[x] High Byte */
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, high_byte_offset, 0xFF, &high_byte_val);
    PDL_CHECK_STATUS(pdlStatus);

    /* get TACH[x] Minimum Low Byte */
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, min_low_byte_offset, 0xFF, &min_low_byte_val);
    PDL_CHECK_STATUS(pdlStatus);

    /* get TACH[x] Minimum High Byte */
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, min_high_byte_offset, 0xFF, &min_high_byte_val);
    PDL_CHECK_STATUS(pdlStatus);

    /* get FAN PULSES PER REVOLUTION Byte */
    pulses_byte_offset = 0x7B;
    pdlStatus = pdlI2CHwGetValueWithOffsetMask(fanInterface.interfaceId, pulses_byte_offset, 0xFF, &pulses_byte_val);
    PDL_CHECK_STATUS(pdlStatus);

    pulses_calc_val = ((pulses_byte_val >> ((fanNumber-1) * 2)) & 3) + 1;
    *speedPercentPtr = 0;

    if ((high_byte_val == 0 && low_byte_val == 0) || ((high_byte_val >= min_high_byte_val) &&
                                                      (low_byte_val  >= min_low_byte_val))) {
        *speedPtr = 0;
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,
                                              "fanNumber=%d error , pulses=%d, high_byte=0x%2x, low_byte=0x%2x, min_high_byte=0x%2x, min_low_byte=0x%2x",
                                              fanNumber, pulses_calc_val, high_byte_val, low_byte_val, min_high_byte_val, min_low_byte_val);
    }
    else {
        *speedPtr = 2700000 * pulses_calc_val / ((high_byte_val << 8) | low_byte_val); /* RPM=90000*60/(tachometer_output) (for 2 pulses per revolution) */
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__,
                                              "fanNumber=%d, fan_speed=%d, pulses=%d, high_byte=0x%2x, low_byte=0x%2x",
                                              fanNumber, *speedPtr, pulses_calc_val, high_byte_val, low_byte_val);
    }

    return PDL_OK;
}

/*$ END OF prvPdlFanAdt7476HwDutyCycleGet */


/**
 * @fn  PDL_STATUS prvPdlFanAdt7476HwDutyCycleSet ( IN UINT_32 fanControllerId, IN UINT_32 speed );
 *
 * @brief   for Adt7476, this will execute the normal script in case value is 0-127 and max speed script in case speed is 128-255
 *
 * @param [in]  fanControllerId     - fan controller identifier.
 * @param [in]  speed               - fan speed (0-255)
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS prvPdlFanAdt7476HwDutyCycleSet(
    IN UINT_32                      fanControllerId,
    IN UINT_32                      speed
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC        * fanPtr;
    PDL_FAN_CONTROLLER_PRV_KEY_STC      fanKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    fanKey.fanControllerId = fanControllerId;
    pdlStatus = prvPdlibDbFind(pdlFanAdt7476Db, (void*) &fanKey, (void**) &fanPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (speed < 128) {
        pdlStatus = prvPdlFanAdt7476SetState(fanPtr->allNormalSpeedStatementDb, fanPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else if (speed >= 128 && speed < 256) {
        pdlStatus = prvPdlFanAdt7476SetState(fanPtr->allMaxSpeedStatementDb, fanPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlFanDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL SPEED(NOT IN RANGE 0-255)");
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476HwDutyCycleSet */

/**
 * @fn  static PDL_STATUS prvPdlFanAdtDbStructInit ( IN PDL_FAN_ADT7476_PRV_DB_STC fanAdtDbStruct )
 *
 * @brief   Pdl fan controller db node init (PDL_FAN_ADT7476_PRV_DB_STC)
 * @param   fanAdtDbStruct*   db struct to init.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlFanAdtDbStructInit(
    IN  PDL_FAN_ADT7476_PRV_DB_STC *   fanAdtDbStruct
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PRV_PDLIB_DB_ATTRIBUTES_STC           dbAttributes;
    PDL_STATUS                          pdlStatus;
    UINT_32                             i;
    PRV_PDLIB_DB_TYP                    * pdlFanAdt7476DbListsPtr[PDL_FAN_ADT7476_NUM_OF_DB_LISTS] = {&fanAdtDbStruct->initStatementDb,
                                                                                                      &fanAdtDbStruct->allNormalSpeedStatementDb,
                                                                                                      &fanAdtDbStruct->allMaxSpeedStatementDb,
                                                                                                      &fanAdtDbStruct->enableFanStatementDb,
                                                                                                      &fanAdtDbStruct->disableRedundentFanStatementDb
                                                                                                     };
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /*create PDL_FAN_ADT7476_PRV_DB_STC*/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_ADT7476_PRV_STATEMENT_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_ADT7476_PRV_STATEMENT_KEY_STC);

    /*init statement lists in db*/
    for (i=0; i<PDL_FAN_OP_TYPE_ADT_7476_LAST_E; i++) {
        pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E, &dbAttributes, pdlFanAdt7476DbListsPtr[i]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}
/*$ END OF prvPdlFanAdtDbStructInit */

/**
 * @fn  static PDL_STATUS prvPdlFanAdt7476BuildDbs ( void )
 *
 * @brief   Prv pdl fan adt7476 db build
 *
 * @return  A PDL_STATUS.
 */
static PDL_STATUS prvPdlFanAdt7476BuildDbs(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus1, pdlStatus2, pdlStatus3;
    PDL_FAN_ADT7476_PRV_DB_STC                  fanDbStruct;
    PDL_FAN_ADT7476_PRV_DB_STC                * fanAdt7476ControllerPtr;
    PDL_FANS_PARAMS_STC                         params;
    PDL_FAN_CONTROLLER_LIST_PARAMS_STC        * fanControllerPtr;
    PDL_FAN_ADT7476_OPERATION_LIST_PARAMS_STC * fanAdt7476OperationPtr;
    PDL_FAN_OPERATION_LIST_PARAMS_STC         * fanOperationPtr;
    PDL_FAN_ADT7476_PRV_STATEMENT_STC           fanStatment, * fanStatementPtr;
    PRV_PDLIB_DB_TYP                              dbToAddTo;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus1 = pdlProjectParamsGet(PDL_FEATURE_ID_FANS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    if (pdlStatus1 != PDL_OK) {
        return PDL_OK;
    }

    pdlStatus1 = prvPdlibDbGetFirst(params.fanControllerList_PTR, (void **)&fanControllerPtr);
    while (pdlStatus1 == PDL_OK) {
        if (TRUE == pdlIsFieldHasValue(fanControllerPtr->fanAdt7476Specific_mask)) {
            memset(&fanDbStruct, 0, sizeof(fanDbStruct));
            /*init fan db struct*/
            pdlStatus1 = prvPdlFanAdtDbStructInit(&fanDbStruct);
            PDL_CHECK_STATUS(pdlStatus1);

            fanDbStruct.key.fanControllerId = fanControllerPtr->list_keys.fanControllerNumber;
            pdlStatus2 = prvPdlibDbGetFirst(fanControllerPtr->fanAdt7476Specific.fanAdt7476OperationList_PTR, (void **)&fanAdt7476OperationPtr);
            while (pdlStatus2 == PDL_OK) {
                switch (fanAdt7476OperationPtr->list_keys.fanOperationType) {
                    case PDL_HW_FAN_OPERATION_TYPE_INITIALIZE_E:
                        dbToAddTo = fanDbStruct.initStatementDb;
                        break;

                    case PDL_HW_FAN_OPERATION_TYPE_ALL_FAN_NORMAL_SPEED_E:
                        dbToAddTo = fanDbStruct.allNormalSpeedStatementDb;
                        break;

                    case PDL_HW_FAN_OPERATION_TYPE_ALL_FAN_MAX_SPEED_E:
                        dbToAddTo = fanDbStruct.allMaxSpeedStatementDb;
                        break;

                    case PDL_HW_FAN_OPERATION_TYPE_ENABLE_FAN_E:
                        dbToAddTo = fanDbStruct.enableFanStatementDb;
                        break;

                    case PDL_HW_FAN_OPERATION_TYPE_DISABLE_REDUNDANT_FAN_E:
                        dbToAddTo = fanDbStruct.disableRedundentFanStatementDb;
                        break;

                    default:
                        PDL_LIB_DEBUG_MAC(prvPdlFanControllerDebugFlag)(__FUNCTION__, __LINE__, __FUNCTION__, __LINE__, "INVALID OPERATION TYPE: %s", fanAdt7476OperationPtr->list_keys.fanOperationType);
                        return PDL_BAD_STATE;
                }
                pdlStatus3 = prvPdlibDbGetFirst(fanAdt7476OperationPtr->fanOperationList_PTR, (void **)&fanOperationPtr);
                while (pdlStatus3 == PDL_OK) {
                    fanStatment.statementKey.statmentId = fanOperationPtr->list_keys.fanOperationNumber;
                    fanStatment.value = fanOperationPtr->fanOperationValue;
                    pdlStatus3 = prvPdlInterfaceI2CReadWriteGroupRegister(&fanOperationPtr->i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType, &fanStatment.i2cInterfaceId);
                    PDL_CHECK_STATUS(pdlStatus3);
                    /*add to db*/
                    pdlStatus3 = prvPdlibDbAdd(dbToAddTo, (void*) &fanStatment.statementKey, (void*)&fanStatment, (void**)&fanStatementPtr);
                    PDL_CHECK_STATUS(pdlStatus3);
                    pdlStatus3 = prvPdlibDbGetNext(fanAdt7476OperationPtr->fanOperationList_PTR, (void *)&fanOperationPtr->list_keys, (void **)&fanOperationPtr);
                }
                pdlStatus2 = prvPdlibDbGetNext(fanControllerPtr->fanAdt7476Specific.fanAdt7476OperationList_PTR, (void *)&fanAdt7476OperationPtr->list_keys, (void **)&fanAdt7476OperationPtr);
            }
            pdlStatus3 = prvPdlibDbAdd(pdlFanAdt7476Db, (void*) &fanDbStruct.key, (void*) &fanDbStruct, (void**) &fanAdt7476ControllerPtr);
            PDL_CHECK_STATUS(pdlStatus3);
        }
        pdlStatus1 = prvPdlibDbGetNext(params.fanControllerList_PTR, (void *)&fanControllerPtr->list_keys, (void **)&fanControllerPtr);
    }

    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476BuildDbs */


/**
 * @fn  static PDL_STATUS prvPdlFanAdt7476DestroyDbs ( void )
 *
 * @brief   Prv pdl fan adt7476 db destroy
 *
 * @return  A PDL_STATUS.
 */
static PDL_STATUS prvPdlFanAdt7476DestroyDbs(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_FAN_ADT7476_PRV_DB_STC                * fanAdt7476ControllerPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    pdlStatus = prvPdlibDbGetFirst(pdlFanAdt7476Db, (void**)&fanAdt7476ControllerPtr);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbDestroy(fanAdt7476ControllerPtr->allMaxSpeedStatementDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbDestroy(fanAdt7476ControllerPtr->allNormalSpeedStatementDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbDestroy(fanAdt7476ControllerPtr->disableRedundentFanStatementDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbDestroy(fanAdt7476ControllerPtr->enableFanStatementDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbDestroy(fanAdt7476ControllerPtr->initStatementDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbGetNext(pdlFanAdt7476Db, (void*)&fanAdt7476ControllerPtr->key, (void**)&fanAdt7476ControllerPtr);
    }

    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476DestroyDbs */

/**
 * @fn  PDL_STATUS pdlFanControllerDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl fan controller debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlFanControllerDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlFanControllerDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlFanControllerDebugSet */

/* ***************************************************************************
* FUNCTION NAME: pdlFanControllerInit
*
* DESCRIPTION:   Init fan controller module
*                Create fan controller DB and initialize
*
* PARAMETERS:
*
*
*****************************************************************************/
/**
 * @fn  PDL_STATUS prvPdlFanAdt7476DbInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Pdl fan Adt7476 initialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476DbInit(
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

    dbAttributes.listAttributes.entrySize = sizeof(PDL_FAN_ADT7476_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_FAN_CONTROLLER_PRV_KEY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,  &dbAttributes,  &pdlFanAdt7476Db);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlFanAdt7476BuildDbs();
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476DbInit */

/**
 * @fn  PDL_STATUS prvPdlFanAdt7476DbDestroy ( void )
 *
 * @brief   Pdl fan Adt7476 deinitialize
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlFanAdt7476DbDestroy(
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
    pdlStatus = prvPdlFanAdt7476DestroyDbs();
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbDestroy(pdlFanAdt7476Db);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF prvPdlFanAdt7476DbDestroy */
