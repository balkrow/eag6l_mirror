/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlSensor.c
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
#include <pdlib/lib/pdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <pdl/sensor/pdlSensor.h>
#include <pdl/sensor/private/prvPdlSensor.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/fan/pdlFan.h>     /* for fan sensor temp readings */

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl sensor database */
static PRV_PDLIB_DB_TYP     pdlSensorDb = NULL;
static BOOLEAN              pdlSensorInitDone = FALSE;
static PRV_PDLIB_DB_TYP     pdlSensorCallbacksDb;
static BOOLEAN              pdlSensorCallbackInitDone = FALSE;
static UINT_8               pdlSensorToShow = 0xFF;

static PDL_SENSOR_PRV_CALLBACK_INFO_STC pdlSensorCallbacksInternal = {
    NULL,
    NULL,
    &prvPdlSensorInternalDriverHwTemperatureGet
};

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlSensorDebugFlag)


/* ***************************************************************************
* FUNCTION NAME: pdlSensorDebugTemperatureInterfaceGet
*
* DESCRIPTION:   Get sensorId access information
*
* PARAMETERS:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSensorDebugInfoGet ( IN UINT_32 sensorId, OUT PDL_SENSOR_INFO_STC * infoPtr )
 *
 * @brief   Pdl sensor debug temperature interface get
 *
 * @param           sensorId        Identifier for the sensor.
 * @param [in,out]  infoPtr         If non-null, the sensor information pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorDebugInfoGet(
    IN  UINT_32                    sensorId,
    OUT PDL_SENSOR_INFO_STC      * infoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SENSOR_INFO_STC               * sensorPtr;
    PDL_SENSOR_PRV_KEY_STC              sensorKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (infoPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__, "infoPtr NULL, SENSOR ID: %d", sensorId);
        return PDL_BAD_PTR;
    }
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlibDbFind(pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    infoPtr->sensorType = sensorPtr->sensorType;
    infoPtr->sensorNamePtr = sensorPtr->sensorNamePtr;
    memcpy(&infoPtr->sensorTempratureInterface, &sensorPtr->sensorTempratureInterface, sizeof(sensorPtr->sensorTempratureInterface));

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, sensorPtr->sensorType);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(CHAR_PTR, sensorPtr->sensorNamePtr);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, sensorPtr->sensorTempratureInterface.sensorInterfaceId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, sensorPtr->sensorTempratureInterface.interfaceType);
    return PDL_OK;

}

/*$ END OF pdlSensorDebugTemperatureInterfaceGet */

/**
 * @fn  BOOLEAN pdlSensorDebugInfoGet ( IN UINT_32 sensorId)
 *
 * @brief   Verify sensor extended info exist in system. for phy/cpu/fan/external sensor verify their metadata (phy-number/fan controller/device-number) exist
 *          Mainly created to support legacy XMLs in which some of this data didn't exist
 *
 * @param           sensorId        Identifier for the sensor.
 *
 * @return  TRUE    - sensor + data exist
 *          FALSE   - otherwise
 */

BOOLEAN pdlSensorDataExist(
    IN  UINT_32                    sensorId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_SENSOR_INFO_STC               * sensorPtr;
    PDL_SENSOR_PRV_KEY_STC              sensorKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlibDbFind(pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    if (pdlStatus != PDL_OK) {
        return FALSE;
    }
    return sensorPtr->sensorTempratureInterface.sensorDataExist;
}

/*$ END OF pdlSensorDataExist */

/*****************************************************************************
* FUNCTION NAME: pdlSensorHwTemperatureGet
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSensorHwTemperatureGet ( IN UINT_32 sensorId, OUT INT_32 * tempPtr )
 *
 * @brief   Pdl sensor hardware temperature get
 *
 * @param           sensorId    Identifier for the sensor.
 * @param [in,out]  tempPtr     If non-null, the temporary pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorHwTemperatureGet(
    IN   UINT_32                        sensorId,
    OUT  INT_32                       * tempPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS                            pdlStatus;
    PDL_SENSOR_PRV_CALLBACK_STC         * callbacksPtr;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    pdlStatus = prvPdlSensorDbGetCallbacks(sensorId, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.lockType);
    pdlStatus = callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun(sensorId, tempPtr);
    prvPdlUnlock(callbacksPtr->callbackInfo.sensorHwTemperatureGetInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}
/*$ END OF pdlSensorHwTemperatureGet */

/*****************************************************************************
* FUNCTION NAME: pdlSensorInternalDriverHwTemperatureGet
*
* DESCRIPTION:   get sensor temperature - internal implementation for all supported interface types
*
*
*****************************************************************************/

PDL_STATUS prvPdlSensorInternalDriverHwTemperatureGet(
    IN   UINT_32                        sensorId,
    OUT  INT_32                       * tempPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_SENSOR_PRV_KEY_STC                  sensorKey;
    PDL_SENSOR_INFO_STC                   * sensorPtr;
    UINT_8                                  data[4];
    UINT_32                                 temp;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    memset(data, 0, sizeof(data));
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlibDbFind(pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sensorPtr->sensorTempratureInterface.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_I2C_E) {
        pdlStatus = prvPdlInterfaceHwGetValue(PDL_INTERFACE_TYPE_INTERFACE_I2C_E, sensorPtr->sensorTempratureInterface.sensorInterfaceId, (UINT_32*) &data);
    }
    else {
        pdlStatus = PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS(pdlStatus);
    /* temperature is swapped */
    temp = data[0] * 256 + data[1];
    /* convert read data to temperature Celsius (lm75) */
    PDL_SENSOR_LM75_TEMP_CALC_MAC(temp);
    *tempPtr = temp;

    return PDL_OK;
}
/*$ END OF prvPdlSensorInternalDriverHwTemperatureGet */

/*****************************************************************************
* FUNCTION NAME: pdlSensorFanDriverHwTemperatureGet
*
* DESCRIPTION:   get sensor temperature - fan sensor temperature get
*
*
*****************************************************************************/

PDL_STATUS pdlSensorFanDriverHwTemperatureGet(
    IN   UINT_32                        sensorId,
    OUT  INT_32                       * tempPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_SENSOR_INFO_STC                   * sensorPtr;
    PDL_SENSOR_PRV_KEY_STC                  sensorKey;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    if (tempPtr == NULL) {
        return PDL_BAD_PTR;
    }
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlibDbFind(pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return pdlFanHwTemperatureGet(sensorPtr->sensorTempratureInterface.fanInfo.fanControllerId, sensorPtr->sensorTempratureInterface.fanInfo.fanSensorId, tempPtr);
}
/*$ END OF pdlSensorFanDriverHwTemperatureGet */


/*****************************************************************************
* FUNCTION NAME: pdlSensorDbNumberOfSensorsGet
*
* DESCRIPTION:
*
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlSensorDbNumberOfSensorsGet ( OUT UINT_32 * numOfSensorsPtr )
 *
 * @brief   Pdl sensor database number of sensors get
 *
 * @param [in,out]  numOfSensorsPtr If non-null, number of sensors pointers.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorDbNumberOfSensorsGet(
    OUT  UINT_32                 *  numOfSensorsPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/

    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/

    return prvPdlibDbGetNumOfEntries(pdlSensorDb, numOfSensorsPtr);
}
/*$ END OF pdlSensorDbNumberOfSensorsGet */

/*****************************************************************************
* FUNCTION NAME: pdlSensorDbSensorsToShowGet
*
* DESCRIPTION:
*
*
*****************************************************************************/

UINT_8 pdlSensorDbSensorsToShowGet(
    IN  void
)
{
/*!****************************************************************************/
/*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/*!****************************************************************************/

/*!****************************************************************************/
/*!                      F U N C T I O N   L O G I C                          */
/*!****************************************************************************/
    return pdlSensorToShow;
}
/*$ END OF pdlSensorDbSensorsToShowGet */

/**
 * @fn  PDL_STATUS pdlSensorDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Sensor debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSensorDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlSensorDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlSensorDebugSet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorBindDriver
*
* DESCRIPTION:   bind driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlSensorBindDriver(
    IN PDL_SENSOR_CALLBACK_INFO_STC         * callbacksInfoPtr,
    IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC                     dbAttributes;
    PDL_SENSOR_PRV_CALLBACK_STC                   callbacksInfo, * callbackInfoPtr = NULL;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /*create callbacks db if needed*/
    if (pdlSensorCallbackInitDone == FALSE) {
        dbAttributes.listAttributes.entrySize = sizeof(PDL_SENSOR_PRV_CALLBACK_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP);
        pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,  &dbAttributes,  &pdlSensorCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlSensorCallbackInitDone = TRUE;
    }

    /* verify entry with this externalDriverId doesn't exist already */
    pdlStatus = prvPdlibDbFind(pdlSensorCallbacksDb, (void*) &(externalDriverId), (void**) &callbackInfoPtr);
    if (pdlStatus == PDL_OK) {
        return PDL_ALREADY_EXIST;
    }

    callbacksInfo.externalDriverId = externalDriverId;
    memcpy(&callbacksInfo.callbackInfo, callbacksInfoPtr, sizeof(PDL_SENSOR_CALLBACK_INFO_STC));
    /*add to db*/
    pdlStatus = prvPdlibDbAdd(pdlSensorCallbacksDb, (void*) &externalDriverId, (void*) &callbacksInfo, (void**) &callbackInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlSensorBindDriver */

/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorBindInternalDriver
*
* DESCRIPTION:   bind internal driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlSensorBindInternalDriver(
    IN PDL_SENSOR_PRV_CALLBACK_INFO_STC         * callbacksInfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_SENSOR_CALLBACK_INFO_STC                  callbacksInfo;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* bind board sensor implementation */
    memset(&callbacksInfo, 0, sizeof(callbacksInfo));
    callbacksInfo.sensorHwTemperatureGetInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun = callbacksInfoPtr->sensorHwTemperatureGetFun;
    callbacksInfo.sensorInitDbCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorInitDbCallbackInfo.initFun = callbacksInfoPtr->sensorInitDbFun;
    callbacksInfo.sensorInitHwCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorInitHwCallbackInfo.initFun = callbacksInfoPtr->sensorInitHwFun;
    pdlStatus = prvPdlSensorBindDriver(&callbacksInfo, PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD);
    PDL_CHECK_STATUS(pdlStatus);

    /* bind fan sensor implementation */
    memset(&callbacksInfo, 0, sizeof(callbacksInfo));
    callbacksInfo.sensorHwTemperatureGetInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorHwTemperatureGetInfo.sensorHwTemperatureGetFun = pdlSensorFanDriverHwTemperatureGet;
    callbacksInfo.sensorInitDbCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorInitDbCallbackInfo.initFun = NULL;
    callbacksInfo.sensorInitHwCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.sensorInitHwCallbackInfo.initFun = NULL;
    return prvPdlSensorBindDriver(&callbacksInfo, PDL_INTERFACE_INTERNAL_DRIVER_ID_FAN);


}

/*$ END OF prvPdlSensorBindInternalDriver */

/* ***************************************************************************
* FUNCTION NAME: pdlSensorBindExternalDriver
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS pdlSensorBindExternalDriver(
    IN PDL_SENSOR_CALLBACK_INFO_STC         * callbacksInfoPtr,
    IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (externalDriverId >= PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__, "ids from [%d] are reserved for internal usage", PDL_INTERFACE_INTERNAL_DRIVER_ID_MIN);
        return PDL_OUT_OF_RANGE;
    }
    return prvPdlSensorBindDriver(callbacksInfoPtr, externalDriverId);
}

/*$ END OF pdlSensorBindExternalDriver */


/* ***************************************************************************
* FUNCTION NAME: prvPdlSensorVerifyExternalDrivers
*
* DESCRIPTION:   verify all external drivers have been binded
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlSensorVerifyExternalDrivers(
    IN void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_SENSOR_PRV_KEY_STC                        sensorKey;
    PDL_SENSOR_INFO_STC                         * sensorPtr;
    PDL_SENSOR_PRV_CALLBACK_STC                 * callbacksPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibDbGetNextKey(pdlSensorDb, NULL, (void*) &sensorKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbFind(pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (sensorPtr->sensorTempratureInterface.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_EXTERNAL_E) {
            pdlStatus = prvPdlSensorDbGetCallbacks(sensorKey.sensorId, &callbacksPtr);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = prvPdlibDbGetNextKey(pdlSensorDb, (void*) &sensorKey, (void*) &sensorKey);
    }
    return PDL_OK;
}

/*$ END OF prvPdlSensorVerifyExternalDrivers */

/* ***************************************************************************
* FUNCTION NAME: pdlSensorDbGetCallbacks
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlSensorDbGetCallbacks(
    IN  UINT_32                              sensorId,
    OUT PDL_SENSOR_PRV_CALLBACK_STC      **  callbacksPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_SENSOR_PRV_KEY_STC                  sensorKey;
    PDL_SENSOR_INFO_STC                   * sensorPtr;
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP    sensorDriverId = PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (callbacksPtr == NULL) {
        return PDL_BAD_PTR;
    }
    sensorKey.sensorId = sensorId;
    pdlStatus = prvPdlibDbFind(pdlSensorDb, (void*) &sensorKey, (void*) &sensorPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (sensorPtr->sensorTempratureInterface.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_EXTERNAL_E) {
        sensorDriverId = sensorPtr->sensorTempratureInterface.externalDriverId;
    }
    else {
        switch (sensorPtr->sensorType) {
            case PDL_SENSOR_TYPE_BOARD_E:
                sensorDriverId = PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD;
                break;
            case PDL_SENSOR_TYPE_FAN_E:
                sensorDriverId = PDL_INTERFACE_INTERNAL_DRIVER_ID_FAN;
                break;
            default:
                PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__, "sensorId=[%d], temperature read function not implemented", sensorId);
                return PDL_BAD_VALUE;
        }
    }
    pdlStatus = prvPdlibDbFind(pdlSensorCallbacksDb, (void*) &(sensorDriverId), (void**) callbacksPtr);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlSensorDebugFlag)(__FUNCTION__, __LINE__, "can't find callback for sensorId=[%d], pdlStatus=[%d]", sensorId, pdlStatus);
    }
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlSensorDbGetCallbacks */

/* ***************************************************************************
* FUNCTION NAME: pdlSensorInit
*
* DESCRIPTION:   Init sensor module
*                Create sensor DB and initialize
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS pdlSensorInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC               dbAttributes;
    PDL_SENSOR_PRV_CALLBACK_STC           * callbackInfoPtr;
    PDL_SENSORS_PARAMS_STC                  params;
    PDL_SENSOR_LIST_PARAMS_STC            * sensorEntryPtr;
    PDL_SENSOR_PRV_KEY_STC                  sensorKey;
    PDL_SENSOR_INFO_STC                     sensorInfo, * sensorPtr = NULL;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    dbAttributes.listAttributes.entrySize = sizeof(PDL_SENSOR_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_SENSOR_PRV_KEY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
        &dbAttributes,
        &pdlSensorDb);
    PDL_CHECK_STATUS(pdlStatus);
    
    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_SENSORS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    if (pdlStatus == PDL_NOT_SUPPORTED) {
        return PDL_OK;
    }
    PDL_CHECK_STATUS(pdlStatus);
    if (pdlIsFieldHasValue(params.sensorToShow_mask) == TRUE) {
        pdlSensorToShow = params.sensorToShow;
    }

    if (pdlIsFieldHasValue(params.sensorList_mask) == FALSE) {
        return PDL_BAD_STATE;
    }    

    pdlStatus = prvPdlibDbGetFirst(params.sensorList_PTR, (void **)&sensorEntryPtr);
    while (pdlStatus == PDL_OK) {
        sensorKey.sensorId = sensorEntryPtr->list_keys.sensorNumber;
        memset(&sensorInfo, 0, sizeof(sensorInfo));

        if (pdlIsFieldHasValue(sensorEntryPtr->sensorDescription_mask)) {
            sensorInfo.sensorNamePtr = sensorEntryPtr->sensorDescription;
        }

        switch (sensorEntryPtr->sensorType) {
            case PDL_SENSOR_HW_TYPE_BOARD_E:
                if (pdlIsFieldHasValue(sensorEntryPtr->boardSensor_mask) && pdlIsFieldHasValue(sensorEntryPtr->boardSensor.temperature_mask)) {

                    pdlStatus = prvPdlInterfaceI2CReadWriteGroupRegister(&sensorEntryPtr->boardSensor.temperature.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType,
                                                                         &sensorInfo.sensorTempratureInterface.sensorInterfaceId);
                    PDL_CHECK_STATUS(pdlStatus);
                    sensorInfo.sensorTempratureInterface.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_I2C_E;
                }
                else if (pdlIsFieldHasValue(sensorEntryPtr->externalDriverId_mask)) {
                    sensorInfo.sensorTempratureInterface.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_EXTERNAL_E;
                    sensorInfo.sensorTempratureInterface.externalDriverId = sensorEntryPtr->externalDriverId;
                }
                else {
                    return PDL_BAD_STATE;
                }
                break;
            case PDL_SENSOR_HW_TYPE_CPU_E:
                sensorInfo.sensorType = PDL_SENSOR_TYPE_CPU_E;
                sensorInfo.sensorTempratureInterface.dev = sensorEntryPtr->deviceNumber;
                break;
            case PDL_SENSOR_HW_TYPE_PHY_E:
                sensorInfo.sensorType = PDL_SENSOR_TYPE_PHY_E;
                sensorInfo.sensorTempratureInterface.phyNumber = sensorEntryPtr->phyNumber;
                break;
            case PDL_SENSOR_HW_TYPE_MAC_E:
                sensorInfo.sensorType = PDL_SENSOR_TYPE_MAC_E;
                sensorInfo.sensorTempratureInterface.dev = sensorEntryPtr->deviceNumber;
                break;
            case PDL_SENSOR_HW_TYPE_POE_E:
                sensorInfo.sensorType = PDL_SENSOR_TYPE_POE_E;
                break;
            case PDL_SENSOR_HW_TYPE_FAN_E:
                sensorInfo.sensorType = PDL_SENSOR_TYPE_FAN_E;
                sensorInfo.sensorTempratureInterface.fanInfo.fanControllerId = sensorEntryPtr->fanControllerNumber;
                if (pdlIsFieldHasValue(sensorEntryPtr->fanNct7802SpecificSensorId_mask)) {
                    sensorInfo.sensorTempratureInterface.fanInfo.fanSensorId = sensorEntryPtr->fanNct7802SpecificSensorId;
                }
                else if (pdlIsFieldHasValue(sensorEntryPtr->fanAdt7476SpecificSensorId_mask)) {
                    sensorInfo.sensorTempratureInterface.fanInfo.fanSensorId = sensorEntryPtr->fanAdt7476SpecificSensorId;
                }

                else {
                    return PDL_NOT_IMPLEMENTED;
                }
                break;
            case PDL_SENSOR_HW_TYPE_LAST_E:
            default:
                break;
        }

        pdlStatus = prvPdlibDbAdd(pdlSensorDb, (void*) &sensorKey, (void*) &sensorInfo, (void**) &sensorPtr);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlibDbGetNext(params.sensorList_PTR, (void *)&sensorEntryPtr->list_keys, (void **)&sensorEntryPtr);
    }

    pdlStatus = prvPdlSensorBindInternalDriver(&pdlSensorCallbacksInternal);
    PDL_CHECK_STATUS(pdlStatus);
    /* init all external Sensor drivers */
    pdlStatus = prvPdlibDbGetFirst(pdlSensorCallbacksDb, (void **)&callbackInfoPtr);
    while (pdlStatus == PDL_OK) {
        if (callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.initFun != NULL) {
            prvPdlLock(callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.lockType);
            callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.initFun();
            prvPdlUnlock(callbackInfoPtr->callbackInfo.sensorInitDbCallbackInfo.lockType);
        }
        pdlStatus = prvPdlibDbGetNext(pdlSensorCallbacksDb, (void*)&callbackInfoPtr->externalDriverId, (void **)&callbackInfoPtr);
    }

    pdlSensorInitDone = TRUE;

    return PDL_OK;
}

/*$ END OF pdlSensorInit */

/**
 * @fn  PDL_STATUS prvPdlSensorDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSensorDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (pdlSensorInitDone) {
        pdlStatus = prvPdlibDbDestroy(pdlSensorDb);
        PDL_CHECK_STATUS(pdlStatus);
    }
    if (pdlSensorCallbackInitDone == TRUE) {
        pdlStatus = prvPdlibDbDestroy(pdlSensorCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlSensorCallbackInitDone = FALSE;
    }

    pdlSensorInitDone = FALSE;

    return PDL_OK;
}

/*$ END OF prvPdlSensorDestroy */


