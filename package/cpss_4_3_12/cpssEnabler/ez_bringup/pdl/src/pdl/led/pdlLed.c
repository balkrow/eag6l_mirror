/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlLed.c
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
 * @brief Platform driver layer - Led related API
 *
 * @version   1
********************************************************************************
*/


/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/led/pdlLed.h>
#include <pdl/led/pdlLedDebug.h>
#include <pdl/led/private/prvPdlLed.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>
#include <pdl/interface/private/prvPdlGpio.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/private/prvPdlPpReg.h>
#include <pdl/interface/pdlGpioDebug.h>
#include <pdl/oob_port/pdlOobPort.h>
#include <pdl/oob_port/private/prvPdlOobPort.h>
/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlLedDebugFlag)

#undef PDL_ASSERT_TRUE
#define PDL_ASSERT_TRUE(_condition) PDL_ASSERT_TRUE_GEN(_condition, prvPdlLedDebugFlag)

/** @brief   The pdl LED port database */
static PRV_PDLIB_DB_TYP     pdlLedPortDb;                                       /* DB for ports led */
/** @brief   The pdl LED device db[ pdl LED type device last e] */
static PDL_LED_PRV_DB_STC   pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_LAST_E];         /* DB for system led */
static PRV_PDLIB_DB_TYP     pdlLedGpioBlinkRateControlDb;
static PRV_PDLIB_DB_TYP     pdlLedGpioBlinkMappingDb;
static BOOLEAN              pdlLedGpioRegInitialized = FALSE;

/** @brief   The pdl LED state string to enum pairs[ pdl LED state last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedStateStrToEnumPairs[PDL_LED_STATE_LAST_E] = {
    {"off", PDL_LED_STATE_OFF_E},
    {"solid", PDL_LED_STATE_SOLID_E},
    {"lowRateblinking", PDL_LED_STATE_LOWRATEBLINKING_E},
    {"normalRateblinking", PDL_LED_STATE_NORMALRATEBLINKING_E},
    {"highRateblinking", PDL_LED_STATE_HIGHRATEBLINKING_E},
    {"initialize", PDL_LED_STATE_INIT_E},
    {"alternate", PDL_LED_STATE_ALTERNATE_E},
};
/** @brief   The pdl LED state string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedStateStrToEnum = {pdlLedStateStrToEnumPairs, PDL_LED_STATE_LAST_E};

/** @brief   The pdl LED color string to enum pairs[ pdl LED color last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedColorStrToEnumPairs[PDL_LED_COLOR_LAST_E] = {
    {"any", PDL_LED_COLOR_NONE_E},
    {"green", PDL_LED_COLOR_GREEN_E},
    {"amber", PDL_LED_COLOR_AMBER_E},
    {"blue", PDL_LED_COLOR_BLUE_E}
};
/** @brief   The pdl LED color string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedColorStrToEnum = {pdlLedColorStrToEnumPairs, PDL_LED_COLOR_LAST_E};

/** @brief   The pdl LED type string to enum pairs[ pdl LED type port last e] */
static PRV_PDL_LIB_STR_TO_ENUM_STC pdlLedTypeStrToEnumPairs[PDL_LED_TYPE_PORT_LAST_E] = {
    {"system", PDL_LED_TYPE_DEVICE_SYSTEM_E},
    {"rps", PDL_LED_TYPE_DEVICE_RPS_E},
    {"fan", PDL_LED_TYPE_DEVICE_FAN_E},
    {"poe", PDL_LED_TYPE_PORT_POE_E},
    {"port_led_state", PDL_LED_TYPE_PORT_PORT_LED_STATE_E},
    {"unit1", PDL_LED_TYPE_DEVICE_STACK_UNIT1_E},
    {"unit2", PDL_LED_TYPE_DEVICE_STACK_UNIT2_E},
    {"unit3", PDL_LED_TYPE_DEVICE_STACK_UNIT3_E},
    {"unit4", PDL_LED_TYPE_DEVICE_STACK_UNIT4_E},
    {"unit5", PDL_LED_TYPE_DEVICE_STACK_UNIT5_E},
    {"unit6", PDL_LED_TYPE_DEVICE_STACK_UNIT6_E},
    {"unit7", PDL_LED_TYPE_DEVICE_STACK_UNIT7_E},
    {"unit8", PDL_LED_TYPE_DEVICE_STACK_UNIT8_E},
    {"master", PDL_LED_TYPE_DEVICE_STACK_MASTER_E},
    {"cloud_managment", PDL_LED_TYPE_DEVICE_CLOUD_MGMT_E},
    {"locator", PDL_LED_TYPE_DEVICE_LOCATOR_E},
    {"speed", PDL_LED_TYPE_DEVICE_SPEED_E},
    {"illegal", PDL_LED_TYPE_DEVICE_LAST_E},
    {"oob_left", PDL_LED_TYPE_PORT_OOB_LEFT_E},
    {"oob_right", PDL_LED_TYPE_PORT_OOB_RIGHT_E},
    {"port_left", PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E},
    {"port_right", PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E},
};

/** @brief   The pdl LED type string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC pdlLedTypeStrToEnum = {pdlLedTypeStrToEnumPairs, PDL_LED_TYPE_PORT_LAST_E};

static PRV_PDLIB_DB_TYP   pdlLedCallbacksDb;
static BOOLEAN          pdlLedDbInitDone = FALSE;

static PDL_LED_PRV_CALLBACK_INFO_STC pdlLedCallbacksInternal = {
    NULL,
    NULL,
    &prvPdlLedHwDeviceModeSet,
    &prvPdlLedHwPortModeSet
};

static BOOLEAN          pdlLedInternalDriverBind = FALSE;



/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedGpioRegInit
*
* DESCRIPTION:   Init Led GPIO registers (on/off duration)
*
* PARAMETERS:
*
*
*****************************************************************************/
PDL_STATUS prvPdlLedGpioRegInit(
    IN  void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_LED_PRV_GPIO_BLINK_MAPPING_KEY_STC      gpioBlinkMapKey;
    PDL_LED_PRV_GPIO_BLINK_MAPPING_DATA_STC   * gpioBlinkInfoPtr;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    pdlStatus = prvPdlibDbGetNextKey(pdlLedGpioBlinkMappingDb, NULL, (void*) &gpioBlinkMapKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbFind(pdlLedGpioBlinkMappingDb, (void*) &gpioBlinkMapKey, (void*) &gpioBlinkInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlGpioRegisterSet (gpioBlinkMapKey.dev, gpioBlinkInfoPtr->offDurationRegisterOffset, 0xFFFFFFFF, gpioBlinkInfoPtr->offDurationValue);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlGpioRegisterSet (gpioBlinkMapKey.dev, gpioBlinkInfoPtr->onDurationRegisterOffset, 0xFFFFFFFF, gpioBlinkInfoPtr->onDurationValue);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbGetNextKey(pdlLedGpioBlinkMappingDb, (void*) &gpioBlinkMapKey, (void*) &gpioBlinkMapKey);
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedGpioRegInit */


/* ***************************************************************************
* FUNCTION NAME: pdlLedDbDeviceCapabilitiesGet
*
* DESCRIPTION:   Get led supported capabilities Based on data retrieved from XML
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedDbDeviceCapabilitiesGet(
    IN  PDL_LED_ID_ENT                  ledId,
    OUT PDL_LED_CAPABILITIES_STC      * capabilitiesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char                              * ledStr;
    PDL_STATUS                          pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "capabilitiesPtr NULL, LedId %s", ledStr);
        return PDL_BAD_PTR;
    }

    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        memcpy(capabilitiesPtr, &pdlLedDeviceDb[ledId].capabilities, sizeof(PDL_LED_CAPABILITIES_STC));
    }
    else {
        pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "capabilitiesPtr NULL, LedId %s", ledStr);
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDbDeviceCapabilitiesGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDbPortCapabilitiesGet
*
* DESCRIPTION:   Get led supported capabilities Based on data retrieved from XML
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedDbPortCapabilitiesGet(
    IN  PDL_LED_ID_ENT                  ledId,
    IN  UINT_32                         dev,
    IN  UINT_32                         logicalPort,
    OUT PDL_LED_CAPABILITIES_STC      * capabilitiesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char                              * ledStr;
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC                * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "capabilitiesPtr NULL, ledId %s, PORT NUMBER: %d, DEVICE NUMBER: %d", ledStr, logicalPort, dev);
        return PDL_BAD_PTR;
    }

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
        memcpy(capabilitiesPtr, &ledPtr->capabilities, sizeof(PDL_LED_CAPABILITIES_STC));
    }
    else {
        pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus);
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "capabilitiesPtr NULL, ledId %s, PORT NUMBER: %d, DEVICE NUMBER: %d", ledStr, logicalPort, dev);
        return PDL_BAD_PARAM;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDbPortCapabilitiesGet */


/* ***************************************************************************
* FUNCTION NAME: pdlSensorDbGetCallbacks
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlLedDbGetCallbacks(
    IN  UINT_32                              ledId,
    IN  UINT_32                              dev,
    IN  UINT_32                              logicalPort,
    OUT PDL_LED_PRV_CALLBACK_STC         **  callbacksPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    char                                  * ledStr;
    PDL_STATUS                              pdlStatus, pdlStatus2;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP    ledDriverId = PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (callbacksPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        if (pdlLedDeviceDb[ledId].capabilities.ledType == PDL_LED_TYPE_EXTERNAL_DRIVER_E) {
            ledDriverId = pdlLedDeviceDb[ledId].interface.externalDriverId;
        }
    }
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (ledPtr->capabilities.ledType == PDL_LED_TYPE_EXTERNAL_DRIVER_E) {
            ledDriverId = ledPtr->interface.externalDriverId;
        }
    }

    pdlStatus = prvPdlibDbFind(pdlLedCallbacksDb, (void*) &(ledDriverId), (void**) callbacksPtr);
    if (pdlStatus != PDL_OK) {
        pdlStatus2 = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
        PDL_CHECK_STATUS(pdlStatus2);
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "can't find callback for ledId=[%s] dev=[%d] port=[%d] pdlStatus=[%d]", ledStr, dev, logicalPort, pdlStatus);
    }
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlSensorDbGetCallbacks */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwDeviceModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedHwDeviceModeSet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                            pdlStatus;
    PDL_LED_PRV_CALLBACK_STC            * callbacksPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlLedDbGetCallbacks(ledId, PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.ledHwDeviceModeSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.ledHwDeviceModeSetFun(ledId, state, color);
    prvPdlUnlock(callbacksPtr->callbackInfo.ledHwDeviceModeSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlLedHwDeviceModeSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwPortModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedHwPortModeSet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                            pdlStatus;
    PDL_LED_PRV_CALLBACK_STC            * callbacksPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlLedDbGetCallbacks(ledId, dev, logicalPort, &callbacksPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.ledHwPortModeSetFun == NULL) {
        return PDL_NOT_SUPPORTED;
    }
    prvPdlLock(callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.lockType);
    pdlStatus = callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.ledHwPortModeSetFun(ledId, dev, logicalPort, state, color);
    prvPdlUnlock(callbacksPtr->callbackInfo.ledHwPortModeSetCallbackInfo.lockType);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlLedHwPortModeSet */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedSetLed
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:
*
*****************************************************************************/


static PDL_STATUS prvPdlLedSetLed(
    IN  PDL_LED_PRV_DB_STC    * ledPtr,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                 i;
    PDL_STATUS                              pdlStatus;
    PRV_PDLIB_DB_TYP                        ppRegInterfaceDbId, ppRegValueDbId;
    PDL_INTERFACE_GPIO_STC                  gpioAttributes;
    PDL_PHY_MASK_VALUE_PARAMS_STC         * smiValuesInfoPtr;
    PDL_LED_PHY_INIT_LIST_PARAMS_STC      * initValueEntryPtr;
    PDL_LED_PHY_VALUE_LIST_PARAMS_STC     * phyValueListEntryPtr;
    PDL_LED_PP_REG_KEY_STC                  ppRegKey;
    PDL_LED_PP_REG_VALUE_ENTRY_STC        * ppRegValueEntryPtr;
    PDL_LED_PP_REG_INTERFACE_ENTRY_STC    * ppRegInterfaceEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (ledPtr == NULL) {
        return PDL_BAD_PTR;
    }

    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            if (pdlLedGpioRegInitialized == FALSE) {
                if (prvPdlLedGpioRegInit() == PDL_OK) {
                    pdlLedGpioRegInitialized = TRUE;
                }
            }
            if (state == PDL_LED_STATE_SOLID_E || state == PDL_LED_STATE_NORMALRATEBLINKING_E || state == PDL_LED_STATE_LOWRATEBLINKING_E || state == PDL_LED_STATE_HIGHRATEBLINKING_E) {
                /* turn active color on and shut down in-active color */
                for (i = PDL_LED_COLOR_GREEN_E; i < PDL_LED_COLOR_LAST_E; i++) {
                    if (ledPtr->interface.gpioInterface.ledInterfaceId[color] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                        if (color == i) {
                            pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, ledPtr->values.gpioValues.colorValue[i]);
                            PDL_CHECK_STATUS(pdlStatus);
                            /* write solid or blinking to active color gpio */
                            if (ledPtr->values.gpioValues.blinkRateControlIsSupported[color] == FALSE) {
                                if (state == PDL_LED_STATE_LOWRATEBLINKING_E || state == PDL_LED_STATE_HIGHRATEBLINKING_E) {
                                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "state %d not supported for this led", state);
                                    return PDL_NOT_SUPPORTED;
                                }
                                pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, state == PDL_LED_STATE_NORMALRATEBLINKING_E ? 1 : 0);
                                PDL_CHECK_STATUS(pdlStatus);
                            }
                            else {
                                if (ledPtr->values.gpioValues.blinkSelect[color][state] == PDL_LED_UNSUPPORTED_VALUE) {
                                    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "state %d not supported for this led", state);
                                    return PDL_NOT_SUPPORTED;
                                }
                                pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, state != PDL_LED_STATE_SOLID_E ? 1 : 0);
                                PDL_CHECK_STATUS(pdlStatus);
                                if (state != PDL_LED_STATE_SOLID_E) {
                                    pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINK_SELECT_E, ledPtr->values.gpioValues.blinkSelect[color][state]);
                                    PDL_CHECK_STATUS(pdlStatus);
                                }
                            }
                        }
                        else if (ledPtr->interface.gpioInterface.ledInterfaceId[i] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                            /* write off and not blinking to non-active color gpio */
                            pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, ledPtr->values.gpioValues.colorValue[i] ? 0 : 1);
                            PDL_CHECK_STATUS(pdlStatus);
                            pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, 0);
                            PDL_CHECK_STATUS(pdlStatus);
                        }
                    }
                }
            }
            /* turn off all colors */
            else if (state == PDL_LED_STATE_OFF_E) {
                for (i = PDL_LED_COLOR_GREEN_E; i < PDL_LED_COLOR_LAST_E; i++) {
                    if (ledPtr->interface.gpioInterface.ledInterfaceId[i] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                        pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, ledPtr->values.gpioValues.colorValue[i] ? 0 : 1);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, 0);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                }
            }
            else if (state == PDL_LED_STATE_INIT_E) {
                for (i = PDL_LED_COLOR_GREEN_E; i < PDL_LED_COLOR_LAST_E; i++) {
                    if (ledPtr->interface.gpioInterface.ledInterfaceId[i] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                        pdlStatus = PdlGpioDebugDbGetAttributes(ledPtr->interface.gpioInterface.ledInterfaceId[i], &gpioAttributes);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_OUTPUT_E, gpioAttributes.initialValue);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlGpioHwSetValue(ledPtr->interface.gpioInterface.ledInterfaceId[i], PDL_INTERFACE_GPIO_OFFSET_BLINKING_E, 0);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                }
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL state");
                return PDL_BAD_VALUE;
            }
            break;
        case PDL_LED_TYPE_I2C_E:
            if (ledPtr->values.i2cValues.value[color][state] == PDL_LED_UNSUPPORTED_VALUE) {
                return PDL_NOT_SUPPORTED;
            }
            pdlStatus = prvPdlInterfaceHwSetValue(PDL_INTERFACE_TYPE_INTERFACE_I2C_E, ledPtr->interface.i2cInterface.ledInterfaceId, ledPtr->values.i2cValues.value[color][state]);
            PDL_CHECK_STATUS(pdlStatus);
            break;
        case PDL_LED_TYPE_SMI_E:
        case PDL_LED_TYPE_XSMI_E:
        case PDL_LED_TYPE_MPD_E:
            if (state == PDL_LED_STATE_INIT_E) {
                pdlStatus = prvPdlibDbGetFirst(ledPtr->values.smiValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E], (void**) &initValueEntryPtr);
                while (pdlStatus == PDL_OK) {
                    smiValuesInfoPtr = &initValueEntryPtr->phyMaskValueGroupType.phyMaskValue;
                    pdlStatus = pdlSmiXsmiHwSetValue(ledPtr->interface.smiInterface.ledInterfaceId, smiValuesInfoPtr->phyDeviceOrPage, smiValuesInfoPtr->phyRegister, smiValuesInfoPtr->phyMask, smiValuesInfoPtr->phyValue);
                    PDL_CHECK_STATUS(pdlStatus);
                    pdlStatus = prvPdlibDbGetNext(ledPtr->values.smiValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E], (void*)&initValueEntryPtr->list_keys, (void**)&initValueEntryPtr);
                }
            }
            else {
                if (ledPtr->values.smiValues.value[color][state] == NULL) {
                    return PDL_NOT_SUPPORTED;
                }
                /* start by setting off state */
                pdlStatus = prvPdlibDbGetFirst(ledPtr->values.smiValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E], (void**) &phyValueListEntryPtr);
                while (pdlStatus == PDL_OK) {
                    smiValuesInfoPtr = &phyValueListEntryPtr->phyMaskValueGroupType.phyMaskValue;
                    pdlStatus = pdlSmiXsmiHwSetValue(ledPtr->interface.smiInterface.ledInterfaceId, smiValuesInfoPtr->phyDeviceOrPage, smiValuesInfoPtr->phyRegister, smiValuesInfoPtr->phyMask, smiValuesInfoPtr->phyValue);
                    PDL_CHECK_STATUS(pdlStatus);
                    pdlStatus = prvPdlibDbGetNext(ledPtr->values.smiValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E], (void*)&phyValueListEntryPtr->list_keys, (void**)&phyValueListEntryPtr);
                }
                if (state != PDL_LED_STATE_OFF_E) {
                    pdlStatus = prvPdlibDbGetFirst(ledPtr->values.smiValues.value[color][state], (void**) &phyValueListEntryPtr);
                    while (pdlStatus == PDL_OK) {
                        smiValuesInfoPtr = &phyValueListEntryPtr->phyMaskValueGroupType.phyMaskValue;
                        pdlStatus = pdlSmiXsmiHwSetValue(ledPtr->interface.smiInterface.ledInterfaceId, smiValuesInfoPtr->phyDeviceOrPage, smiValuesInfoPtr->phyRegister, smiValuesInfoPtr->phyMask, smiValuesInfoPtr->phyValue);
                        PDL_CHECK_STATUS(pdlStatus);
                        pdlStatus = prvPdlibDbGetNext(ledPtr->values.smiValues.value[color][state], (void*)&phyValueListEntryPtr->list_keys, (void**)&phyValueListEntryPtr);
                    }
                }
            }           
            break;
        case PDL_LED_TYPE_LEDSTREAM_E:
            return PDL_NOT_SUPPORTED;
        case PDL_LED_TYPE_EXTERNAL_DRIVER_E:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "Internal driver can't handle external led");
            return PDL_BAD_VALUE;
            break;
        case PDL_LED_TYPE_PP_REG_E:
            ppRegInterfaceDbId = ledPtr->interface.ppRegInterface.colorStateInterfaceDb[color][state];
            ppRegValueDbId = ledPtr->values.ppRegValues.valueDb[color][state];
            pdlStatus = prvPdlibDbGetNextKey(ppRegInterfaceDbId, NULL, (void*) &ppRegKey);
            PDL_CHECK_STATUS(pdlStatus);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlibDbFind(ppRegInterfaceDbId, (void*) &ppRegKey, (void*) &ppRegInterfaceEntryPtr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = prvPdlibDbFind(ppRegValueDbId, (void*) &ppRegKey, (void*) &ppRegValueEntryPtr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = prvPdlInterfaceHwSetValue(PDL_INTERFACE_TYPE_INTERFACE_PPREG_E, ppRegInterfaceEntryPtr->interfaceId, ppRegValueEntryPtr->value);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = prvPdlibDbGetNextKey(ppRegInterfaceDbId, (void*) &ppRegKey, (void*) &ppRegKey);
            }
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL interfaceType");
            return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF prvPdlLedSetLed */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedHwDeviceModeSet
*
* DESCRIPTION:   set LED mode (state & color)
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS prvPdlLedHwDeviceModeSet ( IN PDL_LED_ID_ENT ledId, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color )
 *
 * @brief   Pdl LED hardware mode set
 *
 * @param   ledId   Identifier for the LED.
 * @param   state   The state.
 * @param   color   The color.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedHwDeviceModeSet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    char                                  * stateStr, *colorStr, *ledStr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedStateStrToEnum, state, &stateStr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedColorStrToEnum, color, &colorStr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
    PDL_CHECK_STATUS(pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "%s, state[%s] color[%s]", ledStr, stateStr, colorStr);

    /* init / turn off / alternate are color invariant */
    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E || state == PDL_LED_STATE_ALTERNATE_E) {
        color = PDL_LED_COLOR_NONE_E;
    }
    /* device led */
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL ledId %s", ledStr);
        return PDL_BAD_PARAM;
    }
    if (ledPtr->capabilities.isSupported == PDL_IS_SUPPORTED_NO_E) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "Unsupported ledId %d", ledId);
        return PDL_NOT_SUPPORTED;
    }
    pdlStatus = prvPdlLedSetLed(ledPtr, state, color);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "Error setting led %s state=%s color=%s", ledStr, stateStr, colorStr);
    }
    return pdlStatus;
}

/*$ END OF prvPdlLedHwDeviceModeSet */

/**
 * @fn  PDL_STATUS prvPdlLedHwPortModeSet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color )
 *
 * @brief   Pdl LED hardware mode set
 *
 * @param   ledId   Identifier for the LED.
 * @param   dev     The development.
 * @param   port    The port.
 * @param   state   The state.
 * @param   color   The color.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedHwPortModeSet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    char                                  * stateStr, *colorStr, *ledStr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedStateStrToEnum, state, &stateStr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedColorStrToEnum, color, &colorStr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibLibEnumToStrConvert(&pdlLedTypeStrToEnum, ledId, &ledStr);
    PDL_CHECK_STATUS(pdlStatus);
    PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "%s, Port [%d], Device[%d] state[%s] color[%s]", ledStr, logicalPort, dev, stateStr, colorStr);

    /* init / turn off are color invariant */
    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
        color = PDL_LED_COLOR_NONE_E;
    }
    /* port led */
    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PARAM;
    }

    pdlStatus = prvPdlLedSetLed(ledPtr, state, color);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "Error setting led %s dev[%d] port[%d] state=%s color=%s", ledStr, dev, logicalPort, stateStr, colorStr);
    }
    return pdlStatus;
}

/*$ END OF prvPdlLedHwPortModeSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInfoSet
*
* DESCRIPTION:   change LED value for given state & color
*
* PARAMETERS:
*
* RETURN VALUES: PDL_OK             -   success
                 PDL_NOT_FOUND      -   led doesn't exists in XML
                 PDL_BAD_STATE      -   given state&color doesn't exist for this led type (GPIO)
                 PDL_EMPTY          -   user is trying to set a value that doesn't already exist in XML
                 PDL_NOT_SUPPORTED  -   changing ledstream led value or init values for smi led aren't supported
*****************************************************************************/

PDL_STATUS pdlLedDebugInfoSet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    IN  UINT_32                 value,
    IN  UINT_32                 devOrPage,
    IN  UINT_32                 registerAddress,
    IN  UINT_32                 mask
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    PDL_UNUSED_PARAM(devOrPage);
    PDL_UNUSED_PARAM(registerAddress);
    PDL_UNUSED_PARAM(mask);
    /* init / turn off are color invariant */
    if (state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
        color = PDL_LED_COLOR_NONE_E;
    }
    /* device led */
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    /* port led */
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PARAM;
    }

    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            /* validity checks */
            /* only solid state value can be set for GPIO leds, other values are automatically deducted*/
            if (state != PDL_LED_STATE_SOLID_E) {
                return PDL_BAD_STATE;
            }
            if (value > 1) {
                return  PDL_OUT_OF_RANGE;
            }
            if (ledPtr->interface.gpioInterface.ledInterfaceId[color] != PDL_INTERFACE_INVALID_INTERFACE_ID) {
                ledPtr->values.gpioValues.colorValue[color] = value;
            }
            else {
                return PDL_EMPTY;
            }
            break;
        case PDL_LED_TYPE_I2C_E:
            if (ledPtr->values.i2cValues.value[color][state] != PDL_LED_UNSUPPORTED_VALUE) {
                ledPtr->values.i2cValues.value[color][state] = value;
            }
            else {
                return PDL_EMPTY;
            }
            break;
        case PDL_LED_TYPE_SMI_E:
        case PDL_LED_TYPE_XSMI_E:
        case PDL_LED_TYPE_MPD_E:
            /* not supported for set info since it might be a list of values */
            return PDL_NOT_SUPPORTED;
            break;
        case PDL_LED_TYPE_LEDSTREAM_E:
            return PDL_NOT_SUPPORTED;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
            return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDebugInfoSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInterfaceGet
*
* DESCRIPTION:   Get led access information Based on data retrieved from XML
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlLedDebugInterfaceGetFirst ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color, OUT PDL_LED_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl LED debug interface get
 *
 * @param           ledId           Identifier for the LED.
 * @param           dev             The development.
 * @param           port            The port.
 * @param           state           The state.
 * @param           color           The color.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLedDebugInterfaceGetFirst(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    OUT UINT_32               * keyPtr,
    OUT PDL_LED_INTERFACE_STC * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC                * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
    PDL_LED_PHY_INIT_LIST_PARAMS_STC  * smiValuesInfoPtr;
    PDL_LED_PP_REG_VALUE_ENTRY_STC    * ppRegValueEntryPtr;
    PDL_LED_PP_REG_INTERFACE_ENTRY_STC * ppRegInterfaceEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, ledId);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, dev);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, logicalPort);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, state);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, color);
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else if (ledId > PDL_LED_TYPE_DEVICE_LAST_E && ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_VALUE;
    }
    if (color >= PDL_LED_COLOR_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL color, PORT NUMBER: %d, DEVICE NUMBER: %d COLOR: %d", logicalPort, dev, color);
        return PDL_BAD_VALUE;
    }
    if (state >= PDL_LED_STATE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL state, PORT NUMBER: %d, DEVICE NUMBER: %d STATE: %d", logicalPort, dev, state);
        return PDL_BAD_VALUE;
    }

    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            interfacePtr->ledInterfaceId = ledPtr->interface.gpioInterface.ledInterfaceId[color];
            interfacePtr->values.value.gpioValue = ledPtr->values.gpioValues.colorValue[color];
            interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_GPIO_E;
            break;
        case PDL_LED_TYPE_I2C_E:
            interfacePtr->ledInterfaceId = ledPtr->interface.i2cInterface.ledInterfaceId;
            interfacePtr->values.value.i2cValue = ledPtr->values.i2cValues.value[color][state];
            interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_I2C_E;
            break;
        case PDL_LED_TYPE_SMI_E:
        case PDL_LED_TYPE_XSMI_E:
        case PDL_LED_TYPE_MPD_E:
            interfacePtr->ledInterfaceId = ledPtr->interface.smiInterface.ledInterfaceId;
            pdlStatus = prvPdlibDbGetFirst(ledPtr->values.smiValues.value[color][state], (void*) &smiValuesInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
            interfacePtr->values.value.smiValue.devOrPage = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage;
            interfacePtr->values.value.smiValue.mask = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask;
            interfacePtr->values.value.smiValue.registerAddress = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister;
            interfacePtr->values.value.smiValue.value = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue;
            *keyPtr = smiValuesInfoPtr->list_keys.ledPhyInitNumber;
            if (ledPtr->capabilities.ledType == PDL_LED_TYPE_SMI_E) {
                interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_SMI_E;
            }
            else if (ledPtr->capabilities.ledType == PDL_LED_TYPE_XSMI_E) {
                interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_XSMI_E;
            }
            else {
                interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_MPD_E;
            }
            break;
        case PDL_LED_TYPE_EXTERNAL_DRIVER_E:
            interfacePtr->ledInterfaceId = ledPtr->interface.externalDriverId;
            interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_EXTERNAL_E;
            break;
        case PDL_LED_TYPE_PP_REG_E:
            interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_PPREG_E;
            pdlStatus = prvPdlibDbGetNextKey(ledPtr->interface.ppRegInterface.colorStateInterfaceDb[color][state], NULL, (void*) keyPtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbGetFirst(ledPtr->interface.ppRegInterface.colorStateInterfaceDb[color][state], (void*) &ppRegInterfaceEntryPtr);
            PDL_CHECK_STATUS(pdlStatus);
            interfacePtr->ledInterfaceId = ppRegInterfaceEntryPtr->interfaceId;
            pdlStatus = prvPdlibDbGetFirst(ledPtr->values.ppRegValues.valueDb[color][state], (void*) &ppRegValueEntryPtr);
            PDL_CHECK_STATUS(pdlStatus);
            interfacePtr->values.value.ppRegValue = ppRegValueEntryPtr->value;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
            return PDL_ERROR;
    }
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, *keyPtr);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(PTR, interfacePtr);
    return PDL_OK;
}

/*$ END OF pdlLedDebugInterfaceGetFirst */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInterfaceGetNext
*
* DESCRIPTION:   Get led access information Based on data retrieved from XML
*                (next is only relevant for multi entry values like init in smi/xsmi led)
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedDebugInterfaceGetNext(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    INOUT UINT_32             * keyPtr,
    OUT PDL_LED_INTERFACE_STC * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC                * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
    PDL_LED_PHY_INIT_LIST_PARAMS_STC  * smiValuesInfoPtr;
    PDL_LED_PHY_INIT_LIST_KEYS_STC      initValueKey;
    PDL_LED_PP_REG_VALUE_ENTRY_STC     * ppRegValueEntryPtr;
    PDL_LED_PP_REG_INTERFACE_ENTRY_STC * ppRegInterfaceEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, ledId);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, dev);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, logicalPort);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, state);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, color);

    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else if (ledId > PDL_LED_TYPE_DEVICE_LAST_E && ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_VALUE;
    }

    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            return PDL_NO_MORE;
            break;
        case PDL_LED_TYPE_I2C_E:
            return PDL_NO_MORE;
            break;
        case PDL_LED_TYPE_SMI_E:
        case PDL_LED_TYPE_XSMI_E:
        case PDL_LED_TYPE_MPD_E:
            interfacePtr->ledInterfaceId = ledPtr->interface.smiInterface.ledInterfaceId;
            initValueKey.ledPhyInitNumber = *keyPtr;
            pdlStatus = prvPdlibDbGetNext(ledPtr->values.smiValues.value[color][state], &initValueKey, (void*)&smiValuesInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
            interfacePtr->values.value.smiValue.devOrPage = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage;
            interfacePtr->values.value.smiValue.mask = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask;
            interfacePtr->values.value.smiValue.registerAddress = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister;
            interfacePtr->values.value.smiValue.value = smiValuesInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue;
            *keyPtr = smiValuesInfoPtr->list_keys.ledPhyInitNumber;
            if (ledPtr->capabilities.ledType == PDL_LED_TYPE_SMI_E) {
                interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_SMI_E;
            }
            else if (ledPtr->capabilities.ledType == PDL_LED_TYPE_XSMI_E) {
                interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_XSMI_E;
            }
            else {
                interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_MPD_E;
            }
            break;
        case PDL_LED_TYPE_PP_REG_E:
            interfacePtr->interfaceType = PDL_INTERFACE_EXTENDED_TYPE_PPREG_E;
            pdlStatus = prvPdlibDbGetNextKey(ledPtr->interface.ppRegInterface.colorStateInterfaceDb[color][state], (void*) keyPtr, (void*) keyPtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbFind(ledPtr->interface.ppRegInterface.colorStateInterfaceDb[color][state], (void*) keyPtr, (void*) &ppRegInterfaceEntryPtr);
            PDL_CHECK_STATUS(pdlStatus);
            interfacePtr->ledInterfaceId = ppRegInterfaceEntryPtr->interfaceId;
            pdlStatus = prvPdlibDbFind(ledPtr->values.ppRegValues.valueDb[color][state], (void*) keyPtr, (void*) &ppRegValueEntryPtr);
            PDL_CHECK_STATUS(pdlStatus);
            interfacePtr->values.value.ppRegValue = ppRegValueEntryPtr->value;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
            return PDL_ERROR;
    }
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, *keyPtr);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(PTR, interfacePtr);
    return PDL_OK;
}

/*$ END OF pdlLedDebugInterfaceGetNext */

/* ***************************************************************************
* FUNCTION NAME: pdlLedDebugInterfaceSet
*
* DESCRIPTION:   Set led access information
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlLedDebugInterfaceSet ( IN PDL_LED_ID_ENT ledId, IN UINT_32 dev, IN UINT_32 logicalPort, IN PDL_LED_STATE_ENT state, IN PDL_LED_COLOR_ENT color, IN PDL_LED_VALUES_STC * valuesPtr )
 *
 * @brief   Pdl LED debug interface set
 *
 * @param           ledId       Identifier for the LED.
 * @param           dev         The development.
 * @param           port        The port.
 * @param           state       The state.
 * @param           color       The color.
 * @param [in,out]  valuesPtr   If non-null, the values pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLedDebugInterfaceSet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color,
    IN  PDL_LED_VALUES_STC    * valuesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_PRV_DB_STC                * ledPtr;
    PDL_LED_PRV_KEY_STC                 ledKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (valuesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "valuesPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    else if (ledId > PDL_LED_TYPE_DEVICE_LAST_E && ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL ledId, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_VALUE;
    }
    switch (ledPtr->capabilities.ledType) {
        case PDL_LED_TYPE_GPIO_E:
            return PDL_NOT_SUPPORTED;
        case PDL_LED_TYPE_I2C_E:
            ledPtr->values.i2cValues.value[color][state] = valuesPtr->value.i2cValue;
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlLedDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL interfaceType, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
            return PDL_ERROR;
    }
    return PDL_OK;
}

/*$ END OF pdlLedDebugInterfaceGet */

/**
 * @fn  PDL_STATUS prvPdlStackNumToLedId ( )
 *
 * @brief   convert stack unit number to led id
 *
 * @param   unitNum         number of unit in stack
 * @param   xmlledIdId      Identifier for stack led
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlStackNumToLedId(
    IN   UINT_8                              unitNum,
    OUT  PDL_LED_ID_ENT                    * ledIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (ledIdPtr == NULL) {
        return PDL_BAD_PTR;
    }
    if (unitNum > 8) {
        return PDL_BAD_PARAM;
    }
    *ledIdPtr = (PDL_LED_ID_ENT)(PDL_LED_TYPE_DEVICE_STACK_UNIT1_E + (unitNum-1));
    return PDL_OK;
}
/*$ END OF prvPdlStackNumToLedId */

/* ***************************************************************************
* FUNCTION NAME: pdlLedTypeGet
*
* DESCRIPTION:   get LED type (i2c/gpio/smi/xsmi)
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedTypeGet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    OUT PDL_LED_TYPE_ENT      * ledTypePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (ledTypePtr == NULL) {
        return PDL_BAD_PTR;
    }

    /* device led */
    if (ledId < PDL_LED_TYPE_DEVICE_LAST_E) {
        ledPtr = &pdlLedDeviceDb[ledId];
    }
    /* port led */
    else if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    *ledTypePtr = ledPtr->capabilities.ledType;
    return PDL_OK;
}

/*$ END OF pdlLedTypeGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionNumGet
*
* DESCRIPTION:   get number of function calls for given ledstream
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionNumGet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    OUT UINT_32               * functionNumPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (functionNumPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }

    pdlStatus = prvPdlibDbGetNumOfEntries(ledPtr->values.ledStreamFuncCalls.functionCallDb[state], functionNumPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlLedStreamFunctionGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionGet
*
* DESCRIPTION:   get function call string for ledstream led
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionGet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  UINT_32                 functionId,
    IN  UINT_32                 functionNameSize,
    OUT char                  * functionNamePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (functionNamePtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    funcKey.functionCallNumber = functionId;
    pdlStatus = prvPdlibDbFind(ledPtr->values.ledStreamFuncCalls.functionCallDb[state], &funcKey, (void**) &funcInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    if (functionNameSize < strlen(funcInfoPtr->functionName)+1) {
        return PDL_NO_RESOURCE;
    }
    memcpy(functionNamePtr, funcInfoPtr->functionName, strlen(funcInfoPtr->functionName)+1);
    return PDL_OK;
}

/*$ END OF pdlLedStreamFunctionGet */


/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionParamNumGet
*
* DESCRIPTION:   get number of parameters for given function
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionParamNumGet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  UINT_32                 functionId,
    OUT UINT_32               * paramNumPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (paramNumPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    funcKey.functionCallNumber = functionId;
    pdlStatus = prvPdlibDbFind(ledPtr->values.ledStreamFuncCalls.functionCallDb[state], &funcKey, (void**) &funcInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibDbGetNumOfEntries(funcInfoPtr->params, paramNumPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlLedStreamFunctionParamNumGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamFunctionParamGet
*
* DESCRIPTION:   get parameter string & value for function
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedStreamFunctionParamGet(
    IN  PDL_LED_ID_ENT          ledId,
    IN  UINT_32                 dev,
    IN  UINT_32                 logicalPort,
    IN  PDL_LED_STATE_ENT       state,
    IN  UINT_32                 functionId,
    IN  UINT_32                 paramId,
    IN  UINT_32                 paramNameSize,
    OUT char                  * paramNamePtr,
    OUT UINT_32               * paramValuePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC * paramInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC    paramKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (paramNamePtr == NULL || paramValuePtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (ledId > PDL_LED_TYPE_PORT_FIRST_E &&  ledId < PDL_LED_TYPE_PORT_LAST_E) {
        ledKey.dev = dev;
        ledKey.logicalPort = logicalPort;
        ledKey.ledId = ledId;
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    else {
        return PDL_BAD_PARAM;
    }
    funcKey.functionCallNumber = functionId;
    pdlStatus = prvPdlibDbFind(ledPtr->values.ledStreamFuncCalls.functionCallDb[state], &funcKey, (void**) &funcInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    paramKey.keyNumber = paramId;
    pdlStatus = prvPdlibDbFind(funcInfoPtr->params, &paramKey, (void**) &paramInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (paramNameSize < strlen(paramInfoPtr->paramName)+1) {
        return PDL_NO_RESOURCE;
    }
    memcpy(paramNamePtr, paramInfoPtr->paramName, strlen(paramInfoPtr->paramName)+1);
    *paramValuePtr = paramInfoPtr->paramValue;
    return PDL_OK;
}

/*$ END OF pdlLedStreamFunctionParamGet */


/**
 * @fn  PDL_STATUS pdlLedDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Led debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlLedDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlLedDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlLedDebugSet */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedBindDriver
*
* DESCRIPTION:   bind driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlLedBindDriver(
    IN PDL_LED_CALLBACK_INFO_STC            * callbacksInfoPtr,
    IN PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP   externalDriverId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC                     dbAttributes;
    PDL_LED_PRV_CALLBACK_STC                      callbacksInfo, * callbackInfoPtr = NULL;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /*create callbacks db if needed*/
    if (pdlLedDbInitDone == FALSE) {
        dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_CALLBACK_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_INTERFACE_EXTERNAL_DRIVER_ID_TYP);
        pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,  &dbAttributes,  &pdlLedCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlLedDbInitDone = TRUE;
    }

    /* verify entry with this externalDriverId doesn't exist already */
    pdlStatus = prvPdlibDbFind(pdlLedCallbacksDb, (void*) &(externalDriverId), (void**) &callbackInfoPtr);
    if (pdlStatus == PDL_OK) {
        return PDL_ALREADY_EXIST;
    }

    callbacksInfo.externalDriverId = externalDriverId;
    memcpy(&callbacksInfo.callbackInfo, callbacksInfoPtr, sizeof(PDL_LED_CALLBACK_INFO_STC));
    /*add to db*/
    pdlStatus = prvPdlibDbAdd(pdlLedCallbacksDb, (void*) &externalDriverId, (void*) &callbacksInfo, (void**) &callbackInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF prvPdlLedBindDriver */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedBindInternalDriver
*
* DESCRIPTION:   bind internal driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlLedBindInternalDriver(
    IN PDL_LED_PRV_CALLBACK_INFO_STC         * callbacksInfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_LED_CALLBACK_INFO_STC                  callbacksInfo;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    memset(&callbacksInfo, 0, sizeof(callbacksInfo));
    callbacksInfo.ledHwDeviceModeSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledHwDeviceModeSetCallbackInfo.ledHwDeviceModeSetFun = callbacksInfoPtr->ledHwDeviceModeSetFun;
    callbacksInfo.ledHwPortModeSetCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledHwPortModeSetCallbackInfo.ledHwPortModeSetFun = callbacksInfoPtr->ledHwPortModeSetFun;
    callbacksInfo.ledInitDbCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledInitDbCallbackInfo.initFun = callbacksInfoPtr->ledInitDbFun;
    callbacksInfo.ledInitHwCallbackInfo.lockType = PDL_OS_LOCK_TYPE_NONE_E;
    callbacksInfo.ledInitHwCallbackInfo.initFun = callbacksInfoPtr->ledInitHwFun;
    return prvPdlLedBindDriver(&callbacksInfo, PDL_INTERFACE_INTERNAL_DRIVER_ID_BOARD);
}

/*$ END OF prvPdlLedBindInternalDriver */

/* ***************************************************************************
* FUNCTION NAME: pdlLedBindExternalDriver
*
* DESCRIPTION:   bind external driver with callback set & id
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS pdlLedBindExternalDriver(
    IN PDL_LED_CALLBACK_INFO_STC            * callbacksInfoPtr,
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
    return prvPdlLedBindDriver(callbacksInfoPtr, externalDriverId);
}

/*$ END OF pdlLedBindExternalDriver */

/* ***************************************************************************
* FUNCTION NAME: prvPdlLedVerifyExternalDrivers
*
* DESCRIPTION:   verify all external drivers have been binded
*
* PARAMETERS:
*
*
*****************************************************************************/

PDL_STATUS prvPdlLedVerifyExternalDrivers(
    IN void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                       ledId;
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_CALLBACK_STC                    * callbacksPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    for (ledId = 0; ledId < PDL_LED_TYPE_DEVICE_LAST_E; ledId++) {
        if (pdlLedDeviceDb[ledId].capabilities.isSupported == TRUE) {
            pdlStatus = prvPdlLedDbGetCallbacks(ledId, PDL_LED_DONT_CARE_VALUE_CNS, PDL_LED_DONT_CARE_VALUE_CNS, &callbacksPtr);
            PDL_CHECK_STATUS(pdlStatus);
        }
    }
    pdlStatus = prvPdlibDbGetNextKey(pdlLedPortDb, NULL, (void*) &ledKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlLedDbGetCallbacks(ledKey.ledId, ledKey.dev, ledKey.logicalPort, &callbacksPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbGetNextKey(pdlLedPortDb, (void*) &ledKey, (void*) &ledKey);
    }

    return PDL_OK;
}

/*$ END OF prvPdlLedVerifyExternalDrivers */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedStreamInterfaceManipulationClassSet
*
* DESCRIPTION:   update class manipulation data
*
* PARAMETERS:
*
*
*****************************************************************************/
static PDL_STATUS prvPdlLedStreamInterfaceManipulationClassSet(
    GT_U8                                                                   devIdx,
    GT_U32                                                                  ledStreamInterfaceNumber,
    PDL_GROUPING_LEDSTREAM_INTERFACE_MANIPULATION_GROUP_TYPE_PARAMS_STC   * manipulationParamsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_LED_CLASS_MANIPULATION_STC      classManipulation;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    memset(&classManipulation, 0, sizeof(classManipulation));

    classManipulation.blinkEnable = manipulationParamsPtr->ledstreamManipulationGroupType.blinkenable;
    classManipulation.blinkSelect = (PDL_LED_BLINK_SELECT_ENT)(manipulationParamsPtr->ledstreamManipulationGroupType.blinkselect-1);
    classManipulation.disableOnLinkDown = manipulationParamsPtr->ledstreamManipulationGroupType.disableonlinkdown;
    if (pdlIsFieldHasValue(manipulationParamsPtr->forcedata_mask) == TRUE) {
        classManipulation.forceData = manipulationParamsPtr->forcedata;
    }
    classManipulation.invertEnable = manipulationParamsPtr->ledstreamManipulationGroupType.invertenable;
    classManipulation.pulseStretchEnable = manipulationParamsPtr->ledstreamManipulationGroupType.pulsestretchenable;

    pdlStatus = prvPdlLedStreamClassManipulationSet(devIdx,
                                                    ledStreamInterfaceNumber,
                                                    (PDL_LED_PORT_TYPE_ENT)(manipulationParamsPtr->ledstreamManipulationGroupType.porttype - 1),
                                                    manipulationParamsPtr->ledstreamManipulationGroupType.classnum,
                                                    &classManipulation);
    return pdlStatus;
}
/*$ END OF prvPdlLedStreamInterfaceManipulationClassSet */


/* ***************************************************************************
* FUNCTION NAME: prvPdlLedStreamInterfaceManipulationClassPortModeUpdate
*
* DESCRIPTION:   update port mode class manipulation data
*
* PARAMETERS:
*
*
*****************************************************************************/
static PDL_STATUS prvPdlLedStreamInterfaceManipulationClassPortModeUpdate(
    IN GT_U8                                                        devIdx,
    IN UINT_8                                                       ledInterface,
    IN UINT_32                                                      portModeIndex,
    IN PDL_LED_PRV_DB_STC                                        *  ledPtr,
    IN PDL_GROUPING_LEDSTREAM_PORT_VALUES_LIST_TYPE_PARAMS_STC   *  portValuesParamsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                          pdlStatus;
    PDL_LEDSTREAM_PORT_MODE_LIST_KEYS_STC               portModeKey;
    PDL_LEDSTREAM_PORT_MODE_LIST_PARAMS_STC           * portModeEntryPtr;
    PDL_LEDSTREAM_PORT_MANIPULATION_LIST_PARAMS_STC   * manipulationEntryPtr;
    PDL_LED_CLASS_MANIPULATION_STC                      classManipulationCurrentValue, classManipulationValue;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    portModeKey.ledstreamPortModeIndex = portModeIndex;
    pdlStatus = prvPdlibDbFind(portValuesParamsPtr->ledstreamPortModeList_PTR, (void*)&portModeKey, (void**) &portModeEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetFirst(portModeEntryPtr->ledstreamPortManipulationListType.ledstreamPortManipulationList_PTR, (void**)&manipulationEntryPtr);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlLedStreamClassManipulationGet(devIdx,
                                                        ledInterface,
                                                        (PDL_LED_PORT_TYPE_ENT)(manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.porttype-1),
                                                        manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.classnum,
                                                        &classManipulationCurrentValue);
        PDL_CHECK_STATUS(pdlStatus);

        memset(&classManipulationValue, 0, sizeof(PDL_LED_CLASS_MANIPULATION_STC));
        classManipulationValue.blinkEnable = manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.blinkenable;
        classManipulationValue.blinkSelect = (PDL_LED_BLINK_SELECT_ENT)(manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.blinkselect-1);
        classManipulationValue.disableOnLinkDown = manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.disableonlinkdown;
        if (pdlIsFieldHasValue(manipulationEntryPtr->ledstreamPortManipulationGroupType.forcedata_mask) == TRUE) {
            classManipulationValue.forceData = manipulationEntryPtr->ledstreamPortManipulationGroupType.forcedata;
        }
        classManipulationValue.invertEnable = manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.invertenable;
        classManipulationValue.pulseStretchEnable = manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.pulsestretchenable;

        if (classManipulationValue.forceData == 1) {
            classManipulationValue.forceData = classManipulationCurrentValue.forceData | 1 << ledPtr->values.ledStreamLogic.ledPosition;
        }
        else {
            classManipulationValue.forceData = classManipulationCurrentValue.forceData & ~(1 << ledPtr->values.ledStreamLogic.ledPosition);
        }

        pdlStatus = prvPdlLedStreamClassManipulationSet(devIdx,
                                                        ledInterface,
                                                        (PDL_LED_PORT_TYPE_ENT)(manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.porttype-1),
                                                        manipulationEntryPtr->ledstreamPortManipulationGroupType.ledstreamManipulationGroupType.classnum,
                                                        &classManipulationValue);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlibDbGetNext(portModeEntryPtr->ledstreamPortManipulationListType.ledstreamPortManipulationList_PTR, (void*)&manipulationEntryPtr->list_keys, (void**)&manipulationEntryPtr);
    }
    return PDL_OK;
}
/*$ END OF prvPdlLedStreamInterfaceManipulationClassPortModeUpdate */

/* ***************************************************************************
* FUNCTION NAME: pdlLedInitLedStream
*
* DESCRIPTION:   this function will initialize ledstream port's position, polarity and interface settings
*                this should be executed when ports links are forced down (cpssDxChPortForceLinkDownEnableSet)
*                only neccassery when ledstream leds are configured in XML
*                will return PDL_NO_SUCH if nothing needs to be done
*
* PARAMETERS:
*
*
*****************************************************************************/
PDL_STATUS pdlLedInitLedStream(
    IN UINT_32              devIdx
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                                pdlStatus;
    UINT_32                                                   sip6LedStartCount=0, sip6LedEndCount=0, sip6LedChainBypassCount=0;
    PDL_LED_PRV_KEY_STC                                       ledKey;
    PDL_LED_PRV_DB_STC                                      * ledPtr;
    PDL_LEDS_PARAMS_STC                                       ledsParam;
    PDL_LEDEND_LIST_PARAMS_STC                              * ledStreamEndEntryPtr;
    PDL_LEDSTART_LIST_PARAMS_STC                            * ledStreamStartEntryPtr;
    PDL_LEDCHAINBYPASS_LIST_PARAMS_STC                      * ledStreamChainBypassEntryPtr;
    PDL_LEDSTREAM_INTERFACE_LIST_PARAMS_STC                 * ledStreamInterfaceEntryPtr;
    PDL_LEDSTREAM_INTERFACE_MANIPULATION_LIST_PARAMS_STC    * ledStreamInterfaceManipulationEntryPtr;
    PDL_LED_STEAM_INTERFACE_CONF_STC                          publicInfo;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /* go over ledstream ports and set port polarity and position */
    pdlStatus = prvPdlibDbGetNextKey(pdlLedPortDb, NULL, (void*) &ledKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (ledKey.dev == devIdx && ledPtr->capabilities.ledType == PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E) {
            pdlStatus = prvPdlLedStreamPortPositionSet((GT_U8)ledKey.dev, ledKey.logicalPort, 0x3F);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlLedStreamPortPositionSet((GT_U8)ledKey.dev, ledKey.logicalPort, ledPtr->values.ledStreamLogic.ledPosition);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlLedStreamPortClassPolarityInvertEnableSet((GT_U8)ledKey.dev, ledKey.logicalPort, ledPtr->values.ledStreamLogic.classNum, ledPtr->values.ledStreamLogic.polarityEnable);
            PDL_CHECK_STATUS(pdlStatus);
        }
        pdlStatus = prvPdlibDbGetNextKey(pdlLedPortDb, (void*) &ledKey, (void*) &ledKey);
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_LEDS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&ledsParam);
    if (pdlStatus == PDL_NOT_SUPPORTED) {
        return PDL_OK;
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* set interface parameters */
    if (pdlIsFieldHasValue(ledsParam.ledstreamIsSupported_mask) == TRUE && ledsParam.ledstreamIsSupported == TRUE && pdlIsFieldHasValue(ledsParam.ledstreamleds.ledstreamInterfaceList_mask) == TRUE) {
        pdlStatus = prvPdlibDbGetFirst(ledsParam.ledstreamleds.ledstreamInterfaceList_PTR, (void**)&ledStreamInterfaceEntryPtr);
        while (pdlStatus == PDL_OK) {
            memset(&publicInfo, 0, sizeof(publicInfo));
            publicInfo.blink0Duration = (PDL_LED_BLINK_DURATION_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.blink0Duration-1);
            publicInfo.blink1Duration = (PDL_LED_BLINK_DURATION_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.blink1Duration-1);
            publicInfo.blink0DutyCycle = (PDL_LED_BLINK_DUTY_CYCLE_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.blink0Dutycycle-1);
            publicInfo.blink1DutyCycle = (PDL_LED_BLINK_DUTY_CYCLE_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.blink1Dutycycle-1);
            publicInfo.class5select = (PDL_LED_CLASS_5_SELECT_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.class5Select-1);
            publicInfo.class13select = (PDL_LED_CLASS_13_SELECT_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.class13Select-1);
            publicInfo.clkInvert = ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.clkinvert;
            publicInfo.disableOnLinkDown = ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.disableonlinkdown;
            publicInfo.invertEnable = ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.invertenable;
            publicInfo.ledClockFrequency = (PDL_LED_CLOCK_OUT_FREQUENCY_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.ledclockfrequency-1);
            publicInfo.ledStart = ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.ledstart;
            publicInfo.ledEnd = ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.ledend;
            publicInfo.ledOrganize = (PDL_LED_ORDER_MODE_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.ledorganize-1);
            publicInfo.pulseStretch = (PDL_LED_PULSE_STRETCH_ENT)(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.pulsestretch-1);
            /* check if sip6 config set params exist */
            if (pdlIsFieldHasValue(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Config_mask) == TRUE && ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Config == TRUE) {
                publicInfo.sip6LedConfig.ledClockFrequency = ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledclockfrequency;
                pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledstartList_PTR, (void**)&ledStreamStartEntryPtr);
                while (pdlStatus == PDL_OK) {
                    publicInfo.sip6LedConfig.ledStart[sip6LedStartCount++] = ledStreamStartEntryPtr->ledstart;
                    pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledstartList_PTR, (void*)&ledStreamStartEntryPtr->list_keys, (void**)&ledStreamStartEntryPtr);
                }
                pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledendList_PTR, (void**)&ledStreamEndEntryPtr);
                while (pdlStatus == PDL_OK) {
                    publicInfo.sip6LedConfig.ledEnd[sip6LedEndCount++] = ledStreamEndEntryPtr->ledend;
                    pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledendList_PTR, (void*)&ledStreamEndEntryPtr->list_keys, (void**)&ledStreamEndEntryPtr);
                }
                pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledchainbypassList_PTR, (void**)&ledStreamChainBypassEntryPtr);
                while (pdlStatus == PDL_OK) {
                    publicInfo.sip6LedConfig.ledChainBypass[sip6LedChainBypassCount++] = ledStreamChainBypassEntryPtr->ledchainbypass;
                    pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceEntryPtr->ledstreamInterfaceConfigset.sip6Ledconfig.ledstreamSip6ConfigType.ledchainbypassList_PTR, (void*)&ledStreamChainBypassEntryPtr->list_keys, (void**)&ledStreamChainBypassEntryPtr);
                }
            }

            pdlStatus = prvPdlLedStreamConfigSet((GT_U8)ledStreamInterfaceEntryPtr->list_keys.ppDeviceNumber, ledStreamInterfaceEntryPtr->list_keys.ledstreamInterfaceNumber, &publicInfo);
            PDL_CHECK_STATUS(pdlStatus);

            if (pdlIsFieldHasValue(ledStreamInterfaceEntryPtr->ledstreamInterfaceInitExist_mask) == TRUE && ledStreamInterfaceEntryPtr->ledstreamInterfaceInitExist == TRUE) {
                pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceEntryPtr->ledstreamInterfaceInitValue.ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void**)&ledStreamInterfaceManipulationEntryPtr);
                while (pdlStatus == PDL_OK) {
                    pdlStatus = prvPdlLedStreamInterfaceManipulationClassSet((GT_U8)ledStreamInterfaceEntryPtr->list_keys.ppDeviceNumber,
                                                                             (GT_U32)ledStreamInterfaceEntryPtr->list_keys.ledstreamInterfaceNumber,
                                                                             &ledStreamInterfaceManipulationEntryPtr->ledstreamInterfaceManipulationGroupType);
                    PDL_CHECK_STATUS(pdlStatus);

                    pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceEntryPtr->ledstreamInterfaceInitValue.ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void*)&ledStreamInterfaceManipulationEntryPtr->list_keys, (void**)&ledStreamInterfaceManipulationEntryPtr);
                }

            }

            pdlStatus = prvPdlibDbGetNext(ledsParam.ledstreamleds.ledstreamInterfaceList_PTR, (void*)&ledStreamInterfaceEntryPtr->list_keys, (void**)&ledStreamInterfaceEntryPtr);
        }
    }

    return PDL_OK;
}
/*$ END OF pdlLedInitLedStream */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwLedStreamInterfaceSet
*
* DESCRIPTION:   this function will set interface level class manipulations for the given state & color
*
* PARAMETERS:
*
*
*****************************************************************************/
PDL_STATUS pdlLedHwLedStreamInterfaceModeSet(
    IN  UINT_32                 devIdx,
    IN  UINT_8                  interfaceNumber,
    IN  PDL_LED_STATE_ENT       state,
    IN  PDL_LED_COLOR_ENT       color
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_LEDSTREAM_INTERFACE_LIST_KEYS_STC                   ledStreamInterfaceKey;
    PDL_LEDSTREAM_INTERFACE_LIST_PARAMS_STC               * ledStreamInterfaceInfoPtr;
    PDL_LEDS_PARAMS_STC                                     ledsParam;
    PDL_LEDSTREAM_INTERFACE_MODE_LIST_KEYS_STC              ledStreamInterfaceModeKey;
    PDL_LEDSTREAM_INTERFACE_MODE_LIST_PARAMS_STC          * ledStreamInterfaceModeEntryPtr;
    PDL_LEDSTREAM_INTERFACE_MANIPULATION_LIST_PARAMS_STC  * ledStreamManipulationEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_LEDS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&ledsParam);
    if (pdlStatus == PDL_NOT_SUPPORTED) {
        return PDL_OK;
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* set interface parameters */
    if (pdlIsFieldHasValue(ledsParam.ledstreamIsSupported_mask) == TRUE &&
        ledsParam.ledstreamIsSupported == TRUE &&
        pdlIsFieldHasValue(ledsParam.ledstreamleds.ledstreamInterfaceList_mask) == TRUE) {
        ledStreamInterfaceKey.ppDeviceNumber = (PDL_PP_DEVICE_NUMBER_TYPE_TYP)devIdx;
        ledStreamInterfaceKey.ledstreamInterfaceNumber = interfaceNumber;
        pdlStatus = prvPdlibDbFind(ledsParam.ledstreamleds.ledstreamInterfaceList_PTR, (void*) &ledStreamInterfaceKey, (void**) &ledStreamInterfaceInfoPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (color == PDL_LED_COLOR_NONE_E || state == PDL_LED_STATE_INIT_E || state == PDL_LED_STATE_OFF_E) {
            if (state == PDL_LED_STATE_INIT_E) {
                if (pdlIsFieldHasValue(ledStreamInterfaceInfoPtr->ledstreamInterfaceInitExist_mask) == TRUE && ledStreamInterfaceInfoPtr->ledstreamInterfaceInitExist == TRUE) {
                    pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceInfoPtr->ledstreamInterfaceInitValue.ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void **)&ledStreamManipulationEntryPtr);
                    while (pdlStatus == PDL_OK) {
                        pdlStatus = prvPdlLedStreamInterfaceManipulationClassSet((GT_U8)ledStreamInterfaceKey.ppDeviceNumber,
                                                                                 (GT_U32)ledStreamInterfaceKey.ledstreamInterfaceNumber,
                                                                                 &ledStreamManipulationEntryPtr->ledstreamInterfaceManipulationGroupType);
                        PDL_CHECK_STATUS(pdlStatus);

                        pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceInfoPtr->ledstreamInterfaceInitValue.ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void*)&ledStreamManipulationEntryPtr->list_keys, (void**)&ledStreamManipulationEntryPtr);
                    }
                }

            }
            else if (state == PDL_LED_STATE_OFF_E) {
                if (pdlIsFieldHasValue(ledStreamInterfaceInfoPtr->ledstreamInterfaceModes_mask) == TRUE &&
                    pdlIsFieldHasValue(ledStreamInterfaceInfoPtr->ledstreamInterfaceModes.ledstreamGroupType.ledstreamInterfaceOffExist_mask) == TRUE &&
                    ledStreamInterfaceInfoPtr->ledstreamInterfaceModes.ledstreamGroupType.ledstreamInterfaceOffExist == TRUE) {
                    pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceInfoPtr->ledstreamInterfaceModes.ledstreamGroupType.ledstreamInterfaceOffValue.ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void**)&ledStreamManipulationEntryPtr);
                    while (pdlStatus == PDL_OK) {
                        pdlStatus = prvPdlLedStreamInterfaceManipulationClassSet((GT_U8)ledStreamInterfaceKey.ppDeviceNumber,
                                                                                 (GT_U32)ledStreamInterfaceKey.ledstreamInterfaceNumber,
                                                                                 &ledStreamManipulationEntryPtr->ledstreamInterfaceManipulationGroupType);
                        PDL_CHECK_STATUS(pdlStatus);

                        pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceInfoPtr->ledstreamInterfaceModes.ledstreamGroupType.ledstreamInterfaceOffValue.ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void*)&ledStreamManipulationEntryPtr->list_keys, (void**)&ledStreamManipulationEntryPtr);
                    }
                }
            }
        }
        else {
            ledStreamInterfaceModeKey.ledColor = (PDL_COLOR_TYPE_ENT)color;
            ledStreamInterfaceModeKey.ledState = (PDL_LED_STATE_TYPE_ENT)state;
            pdlStatus = prvPdlibDbFind(ledStreamInterfaceInfoPtr->ledstreamInterfaceModes.ledstreamGroupType.ledstreamInterfaceModeList_PTR, (void*)&ledStreamInterfaceModeKey, (void**)&ledStreamInterfaceModeEntryPtr);
            if (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlibDbGetFirst(ledStreamInterfaceModeEntryPtr->ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void**)&ledStreamManipulationEntryPtr);
                while (pdlStatus == PDL_OK) {
                    pdlStatus = prvPdlLedStreamInterfaceManipulationClassSet((GT_U8)ledStreamInterfaceKey.ppDeviceNumber,
                                                                             (GT_U32)ledStreamInterfaceKey.ledstreamInterfaceNumber,
                                                                             &ledStreamManipulationEntryPtr->ledstreamInterfaceManipulationGroupType);
                    PDL_CHECK_STATUS(pdlStatus);

                    pdlStatus = prvPdlibDbGetNext(ledStreamInterfaceModeEntryPtr->ledstreamInterfaceManipulationListType.ledstreamInterfaceManipulationList_PTR, (void*)&ledStreamManipulationEntryPtr->list_keys, (void**)&ledStreamManipulationEntryPtr);
                }
            }
        }
    }

    return PDL_OK;
}

/*$ END OF pdlLedHwLedStreamInterfaceSet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedStreamDbPortInterfaceGet
*
* DESCRIPTION:   get given port ledstream interface number
*
* PARAMETERS:
*
*****************************************************************************/

PDL_STATUS pdlLedStreamDbPortInterfaceGet(
    IN  UINT_32              devIdx,
    IN  UINT_32              logicalPort,
    OUT UINT_8             * interfaceIndexPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_DB_STC                          * ledPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceIndexPtr == NULL) {
        return PDL_BAD_PTR;
    }
    ledKey.dev = devIdx;
    ledKey.logicalPort = logicalPort;
    ledKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
    pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void*) &ledPtr);
    if (pdlStatus != PDL_OK || ledPtr->capabilities.ledType != PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E) {
        return PDL_NO_SUCH;
    }
    *interfaceIndexPtr = ledPtr->values.ledStreamLogic.ledInterface;
    return PDL_OK;

}

/*$ END OF pdlLedStreamDbPortInterfaceGet */

/* ***************************************************************************
* FUNCTION NAME: pdlLedHwLedStreamPortModeSet
*
* DESCRIPTION:   set given port to execute manipulation index
*
* PARAMETERS:
*
*****************************************************************************/
PDL_STATUS pdlLedHwLedStreamPortModeSet(
    IN  PDL_LED_ID_ENT      ledId,
    IN  UINT_32             devIdx,
    IN  UINT_32             logicalPort,
    IN  UINT_32             modeIndex
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_8                                        ledInterface;
    PDL_STATUS                                    pdlStatus;
    PDL_LED_PRV_KEY_STC                           ledKey;
    PDL_LED_PRV_DB_STC                          * ledPtr;
    PDL_LEDS_PARAMS_STC                           ledsParam;
    PDL_LEDSTREAM_INTERFACE_LIST_KEYS_STC         ledStreamInterfaceKey;
    PDL_LEDSTREAM_INTERFACE_LIST_PARAMS_STC     * ledStreamInterfaceEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    ledKey.dev = devIdx;
    ledKey.logicalPort = logicalPort;
    ledKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
    pdlStatus = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
    if (pdlStatus != PDL_OK || ledPtr->capabilities.ledType != PDL_LED_TYPE_LEDSTREAM_LOGIC_BASED_E) {
        return PDL_NO_SUCH;
    }

    ledInterface = ledPtr->values.ledStreamLogic.ledInterface;

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_LEDS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&ledsParam);
    if (pdlStatus == PDL_NOT_SUPPORTED) {
        return PDL_OK;
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (pdlIsFieldHasValue(ledsParam.ledstreamIsSupported_mask) == TRUE && ledsParam.ledstreamIsSupported == TRUE && pdlIsFieldHasValue(ledsParam.ledstreamleds.ledstreamInterfaceList_mask) == TRUE) {
        ledStreamInterfaceKey.ppDeviceNumber = (PDL_PP_DEVICE_NUMBER_TYPE_TYP)devIdx;
        ledStreamInterfaceKey.ledstreamInterfaceNumber = ledInterface;

        pdlStatus = prvPdlibDbFind(ledsParam.ledstreamleds.ledstreamInterfaceList_PTR, (void*) &ledStreamInterfaceKey, (void**) &ledStreamInterfaceEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);

        if ((ledsParam.portledsIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.portsleds_mask) == TRUE)) {
            pdlStatus = PDL_OK;
            if (ledId == PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E) {
                if (ledsParam.portsleds.portledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_SINGLE_E) {
                    pdlStatus = prvPdlLedStreamInterfaceManipulationClassPortModeUpdate((GT_U8)devIdx, ledInterface, modeIndex, ledPtr, &ledStreamInterfaceEntryPtr->ledstreamInterfaceModes.ledstreamPortModes.singleledPortModes.ledstreamPortValuesListType);
                }
                else if (ledsParam.portsleds.portledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_LEFT_AND_RIGHT_E) {
                    pdlStatus = prvPdlLedStreamInterfaceManipulationClassPortModeUpdate((GT_U8)devIdx, ledInterface, modeIndex, ledPtr, &ledStreamInterfaceEntryPtr->ledstreamInterfaceModes.ledstreamPortModes.leftledPortModes.ledstreamPortValuesListType);
                }
            }
            else if (ledId == PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E) {
                if (ledsParam.portsleds.portledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_LEFT_AND_RIGHT_E) {
                    pdlStatus = prvPdlLedStreamInterfaceManipulationClassPortModeUpdate((GT_U8)devIdx, ledInterface, modeIndex, ledPtr, &ledStreamInterfaceEntryPtr->ledstreamInterfaceModes.ledstreamPortModes.rightledPortModes.ledstreamPortValuesListType);
                }
            }
            PDL_CHECK_STATUS(pdlStatus);
        }
    }

    return PDL_OK;
}
/*$ END OF pdlLedStreamDbPortInterfaceGet */
/**
 * @fn  PDL_STATUS prvPdlLedParsePpRegInterface ( IN PRV_PDLIB_DB_TYP * ppRegInfoDbPtr, OUT PRV_PDLIB_DB_TYP  * interfaceDbPtr,  OUT PRV_PDLIB_DB_TYP  * valueDbPtr)
 *
 * @brief   Prv pdl packer procesor register based  led parser (system leds)
 *
 * @param    [in]   ppRegInfoDbPtr      Pointer to led data.
 * @param    [out]  interfaceDbPtr      Pointer to led interface info.
 * @param    [out]  valueDbPtr          Pointer to led values info.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlLedParsePpRegInterface(
    IN  PRV_PDLIB_DB_TYP                                ppRegInfoDbPtr,
    OUT PRV_PDLIB_DB_TYP                              * interfaceDbPtr,
    OUT PRV_PDLIB_DB_TYP                              * valueDbPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_LED_PP_REG_KEY_STC                      ppRegKey;
    PRV_PDLIB_DB_ATTRIBUTES_STC                 dbAttributes;
    PDL_LED_PP_REG_VALUE_ENTRY_STC              ppRegValueEntry, *ppRegValueEntryPtr;
    PDL_LED_PP_REG_INTERFACE_ENTRY_STC          ppRegInterfaceEntry, *ppRegInterfaceEntryPtr;
    PDL_PP_REG_WRITE_LIST_PARAMS_STC          * ppRegEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfaceDbPtr == NULL || valueDbPtr == NULL) {
        return PDL_BAD_PARAM;
    }

    pdlStatus = prvPdlibDbGetFirst(ppRegInfoDbPtr, (void **)&ppRegEntryPtr);
    ppRegKey.index = 0;
    if (pdlStatus == PDL_OK) {
        dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PP_REG_KEY_STC);
        dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PP_REG_INTERFACE_ENTRY_STC);
        pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                     &dbAttributes,
                                     interfaceDbPtr);
        PDL_CHECK_STATUS(pdlStatus);

        dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PP_REG_KEY_STC);
        dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PP_REG_VALUE_ENTRY_STC);
        pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                     &dbAttributes,
                                     valueDbPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }
    while (pdlStatus == PDL_OK) {
        prvPdlPpRegInterfaceRegister(ppRegEntryPtr->ppRegReadWriteAddressGroupType.ppRegReadWriteAddress.ppRegReadWriteAddressFieldsGroupType.ppRegDevice,
                                     ppRegEntryPtr->ppRegReadWriteAddressGroupType.ppRegReadWriteAddress.ppRegReadWriteAddressFieldsGroupType.ppRegAddress,
                                     ppRegEntryPtr->ppRegReadWriteAddressGroupType.ppRegReadWriteAddress.ppRegReadWriteAddressFieldsGroupType.ppRegMask,
                                     &ppRegInterfaceEntry.interfaceId);
        ppRegValueEntry.value = ppRegEntryPtr->ppRegWriteValue;
        pdlStatus = prvPdlibDbAdd(*interfaceDbPtr, (void*) &ppRegKey, (void*) &ppRegInterfaceEntry, (void**) &ppRegInterfaceEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbAdd(*valueDbPtr, (void*) &ppRegKey, (void*) &ppRegValueEntry, (void**) &ppRegValueEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
        ppRegKey.index++;
        pdlStatus = prvPdlibDbGetNext(ppRegInfoDbPtr, (void *)&ppRegEntryPtr->list_keys, (void **)&ppRegEntryPtr);
    }
    return PDL_OK;
}

/*$ END OF pdlLedStreamDbPortInterfaceGet */

/**
 * @fn  PDL_STATUS prvPdlLedParseInterface ( IN PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC * extdrvGroupPtr, OUT PDL_LED_PRV_DB_STC  * dbEntryPtr )
 *
 * @brief   Prv pdl led parser (system leds)
 *
 * @param    [in]   extdrvGroupPtr      Pointer to led data.
 * @param    [out]  dbEntryPtr          Pointer to led info.
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlLedParseInterface(
    IN  PDL_GROUPING_LED_EXTDRV_GROUP_TYPE_PARAMS_STC * extdrvGroupPtr,
    OUT PDL_LED_PRV_DB_STC                            * dbEntryPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     i;
    PDL_STATUS                                  pdlStatus;
    PDL_LED_COLOR_ENT                           ledColor;
    PDL_LED_EXTDRV_GPIO_COLOR_LIST_PARAMS_STC * gpioColorEntryPtr;
    PDL_LED_EXTDRV_I2C_COLOR_LIST_PARAMS_STC  * i2cColorEntryPtr;
    PDL_LED_PP_REGISTER_COLOR_LIST_PARAMS_STC * ppRegColorEntryPtr;
    PDL_LED_PRV_GPIO_BLINK_MAPPING_KEY_STC      gpioBlinkMapKey;
    PDL_LED_PRV_GPIO_BLINK_MAPPING_DATA_STC   * gpioBlinkInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (extdrvGroupPtr == NULL || dbEntryPtr == NULL) {
        return PDL_BAD_PARAM;
    }

    switch (extdrvGroupPtr->ledInterfaceSelect) {
        case PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_GPIO_E:
            for (ledColor = PDL_LED_COLOR_NONE_E; ledColor< PDL_LED_COLOR_LAST_E; ledColor++) {
                dbEntryPtr->interface.gpioInterface.ledInterfaceId[ledColor] = PDL_INTERFACE_INVALID_INTERFACE_ID;
                dbEntryPtr->values.gpioValues.colorValue[ledColor] = PDL_LED_UNSUPPORTED_VALUE;
                dbEntryPtr->values.gpioValues.blinkRateControlIsSupported[ledColor] = FALSE;
            }
            pdlStatus = prvPdlibDbGetFirst(extdrvGroupPtr->ledExtdrvGpioColorList_PTR, (void **)&gpioColorEntryPtr);
            while (pdlStatus == PDL_OK) {
                PDL_ASSERT_TRUE(gpioColorEntryPtr->list_keys.ledColor < PDL_COLOR_TYPE_LAST_E);
                pdlStatus = prvPdlInterfaceGpioWriteGroupRegister(&gpioColorEntryPtr->gpioWriteAddressGroupType,
                                                                  gpioColorEntryPtr->ledPinValue,
                                                                  &dbEntryPtr->interface.gpioInterface.ledInterfaceId[gpioColorEntryPtr->list_keys.ledColor]);
                PDL_CHECK_STATUS(pdlStatus);
                dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_GPIO_E;
                dbEntryPtr->values.gpioValues.colorValue[gpioColorEntryPtr->list_keys.ledColor] = gpioColorEntryPtr->ledPinValue;
                dbEntryPtr->values.gpioValues.blinkRateControlIsSupported[gpioColorEntryPtr->list_keys.ledColor] = FALSE;
                for (i = PDL_LED_STATE_LOWRATEBLINKING_E; i <= PDL_LED_STATE_HIGHRATEBLINKING_E; i++) {
                    gpioBlinkMapKey.dev = gpioColorEntryPtr->gpioWriteAddressGroupType.gpioWriteAddress.gpioWriteDeviceNumber;
                    gpioBlinkMapKey.ledState = i;
                    dbEntryPtr->values.gpioValues.blinkSelect[gpioColorEntryPtr->list_keys.ledColor][i] = PDL_LED_UNSUPPORTED_VALUE;
                    pdlStatus = prvPdlibDbFind(pdlLedGpioBlinkMappingDb, (void*) &gpioBlinkMapKey, (void*) &gpioBlinkInfoPtr);
                    if (pdlStatus == PDL_OK) {
                        dbEntryPtr->values.gpioValues.blinkRateControlIsSupported[gpioColorEntryPtr->list_keys.ledColor] = TRUE;
                        dbEntryPtr->values.gpioValues.blinkSelect[gpioColorEntryPtr->list_keys.ledColor][i] = gpioBlinkInfoPtr->blinkSelect;
                    }
                }
                pdlStatus = prvPdlibDbGetNext(extdrvGroupPtr->ledExtdrvGpioColorList_PTR, (void *)&gpioColorEntryPtr->list_keys, (void **)&gpioColorEntryPtr);
            }
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        case PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_I2C_E:
            pdlStatus = prvPdlInterfaceI2CReadWriteGroupRegister(&extdrvGroupPtr->ledExtdrvI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType,
                                                                 &dbEntryPtr->interface.i2cInterface.ledInterfaceId);
            PDL_CHECK_STATUS(pdlStatus);
            dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_I2C_E;

            pdlStatus = prvPdlibDbGetFirst(extdrvGroupPtr->ledExtdrvI2CColorList_PTR, (void **)&i2cColorEntryPtr);
            while (pdlStatus == PDL_OK) {
                dbEntryPtr->values.i2cValues.value[i2cColorEntryPtr->list_keys.ledColor][i2cColorEntryPtr->list_keys.ledState] = i2cColorEntryPtr->ledI2C.ledI2CValue;
                pdlStatus = prvPdlibDbGetNext(extdrvGroupPtr->ledExtdrvI2CColorList_PTR, (void *)&i2cColorEntryPtr->list_keys, (void **)&i2cColorEntryPtr);
            }
            dbEntryPtr->values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E] = extdrvGroupPtr->ledExtdrvI2C.ledI2COffValue;
            dbEntryPtr->values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E] = extdrvGroupPtr->ledExtdrvI2C.ledI2CInitValue;
            if (extdrvGroupPtr->ledExtdrvI2C.ledI2CAlternateExist) {
                dbEntryPtr->values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_ALTERNATE_E] = extdrvGroupPtr->ledExtdrvI2C.ledI2CAlternateValue;
            }
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        case PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_EXTERNAL_DRIVER_E:
            dbEntryPtr->interface.externalDriverId = extdrvGroupPtr->externalDriverId;
            dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_EXTERNAL_DRIVER_E;
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        case PDL_LED_GPIO_I2C_PPREG_INTERFACE_TYPE_LED_INTERFACE_PP_REGISTER_E:
            /* DB create - init interfaces & values */
            dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_PP_REG_E;
            memset(&dbEntryPtr->interface.ppRegInterface, 0, sizeof(dbEntryPtr->interface.ppRegInterface));
            memset(&dbEntryPtr->values.ppRegValues, 0, sizeof(dbEntryPtr->values.ppRegValues));
            pdlStatus = prvPdlLedParsePpRegInterface(extdrvGroupPtr->ledPpRegister.ledPpRegisterInitValue.ppRegWriteListGroupType.ppRegWriteList_PTR,
                                                     &dbEntryPtr->interface.ppRegInterface.colorStateInterfaceDb[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E],
                                                     &dbEntryPtr->values.ppRegValues.valueDb[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E]);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlLedParsePpRegInterface(extdrvGroupPtr->ledPpRegister.ledPpRegisterOffValue.ppRegWriteListGroupType.ppRegWriteList_PTR,
                                                     &dbEntryPtr->interface.ppRegInterface.colorStateInterfaceDb[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E],
                                                     &dbEntryPtr->values.ppRegValues.valueDb[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E]);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbGetFirst(extdrvGroupPtr->ledPpRegister.ledPpRegisterColorList_PTR, (void **)&ppRegColorEntryPtr);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlLedParsePpRegInterface(ppRegColorEntryPtr->ppRegWriteListGroupType.ppRegWriteList_PTR,
                                                         &dbEntryPtr->interface.ppRegInterface.colorStateInterfaceDb[ppRegColorEntryPtr->list_keys.ledColor][ppRegColorEntryPtr->list_keys.ledState],
                                                         &dbEntryPtr->values.ppRegValues.valueDb[ppRegColorEntryPtr->list_keys.ledColor][ppRegColorEntryPtr->list_keys.ledState]);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = prvPdlibDbGetNext(extdrvGroupPtr->ledPpRegister.ledPpRegisterColorList_PTR, (void *)&ppRegColorEntryPtr->list_keys, (void **)&ppRegColorEntryPtr);
            }

            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        default:
            return PDL_BAD_STATE;
    }
    return PDL_OK;
}


static PDL_STATUS prvPdlLedParseLedStreamValues(
    IN  PDL_GROUPING_FUNCTION_CALL_GROUP_TYPE_PARAMS_STC  * ledStreamEntryPtr,
    OUT PRV_PDLIB_DB_TYP                                    * dbEntryPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus1, pdlStatus2;
    PRV_PDLIB_DB_ATTRIBUTES_STC               dbAttributes;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC    funcInfo, *funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;
    PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC   paramInfo, *paramInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC    paramKey;

    PDL_FUNCTION_CALL_LIST_PARAMS_STC             * ledStreamFunctionEntryPtr;
    PDL_FUNCTION_CALL_PARAMETERS_LIST_PARAMS_STC  * ledStreamParamEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC);
    pdlStatus1 = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                  &dbAttributes,
                                  dbEntryPtr);
    PDL_CHECK_STATUS(pdlStatus1);

    funcKey.functionCallNumber = 0;

    pdlStatus1 = prvPdlibDbGetFirst(ledStreamEntryPtr->functionCallList_PTR, (void **)&ledStreamFunctionEntryPtr);
    while (pdlStatus1 == PDL_OK) {
        memset(&funcInfo, 0, sizeof(funcInfo));

        if (sizeof(funcInfo.functionName) < strlen(ledStreamFunctionEntryPtr->functionCallName) - 1) {
            prvPdlibDbDestroy(*dbEntryPtr);
            return PDL_BAD_STATE;
        }
        strcpy(funcInfo.functionName, ledStreamFunctionEntryPtr->functionCallName);

        dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_LEDSTREAM_FUNC_PARAM_INFO_STC);
        dbAttributes.listAttributes.keySize = sizeof(PDL_LED_LEDSTREAM_FUNC_PARAM_KEY_STC);
        pdlStatus1 = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                      &dbAttributes,
                                      &funcInfo.params);
        PDL_CHECK_STATUS(pdlStatus1);

        paramKey.keyNumber = 0;

        pdlStatus2 = prvPdlibDbGetFirst(ledStreamFunctionEntryPtr->functionCallParametersList_PTR, (void **)&ledStreamParamEntryPtr);
        while (pdlStatus2 == PDL_OK) {
            if (sizeof(paramInfo.paramName) < strlen(ledStreamParamEntryPtr->list_keys.functionCallParameterName) - 1) {
                prvPdlibDbDestroy(funcInfo.params);
                prvPdlibDbDestroy(*dbEntryPtr);
                return PDL_BAD_STATE;
            }
            strcpy(paramInfo.paramName, ledStreamParamEntryPtr->list_keys.functionCallParameterName);
            paramInfo.paramValue = ledStreamParamEntryPtr->functionCallParameterValue;

            pdlStatus2 = prvPdlibDbAdd(funcInfo.params, (void*) &paramKey, (void*) &paramInfo, (void**) &paramInfoPtr);
            PDL_CHECK_STATUS(pdlStatus2);

            paramKey.keyNumber++;
            pdlStatus2 = prvPdlibDbGetNext(ledStreamFunctionEntryPtr->functionCallParametersList_PTR, (void *)&ledStreamParamEntryPtr->list_keys, (void **)&ledStreamParamEntryPtr);
        }

        pdlStatus1 = prvPdlibDbAdd(*dbEntryPtr, (void*) &funcKey, (void*) &funcInfo, (void**) &funcInfoPtr);
        PDL_CHECK_STATUS(pdlStatus1);
        funcKey.functionCallNumber++;

        pdlStatus1 = prvPdlibDbGetNext(ledStreamEntryPtr->functionCallList_PTR, (void *)&ledStreamFunctionEntryPtr->list_keys, (void **)&ledStreamFunctionEntryPtr);
    }
    return PDL_OK;
}
/*$ END OF prvPdlLedParseLedStreamValues */

static PDL_STATUS prvPdlLedParsePortInterface(
    IN  PDL_GROUPING_LED_PP_GROUP_TYPE_PARAMS_STC     * portGroupPtr,
    IN  PDL_LED_PRV_KEY_STC                           * dbEntryKeyPtr,
    OUT PDL_LED_PRV_DB_STC                            * dbEntryPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_LED_COLOR_ENT                           ledColor;
    PDL_LED_STATE_ENT                           ledState;
    PDL_STATUS                                  pdlStatus;
    PDL_LED_PP_GPIO_COLOR_LIST_PARAMS_STC     * gpioColorEntryPtr;
    PDL_PP_NETWORK_PORT_ATTRIBUTES_STC          portAttributes;
    PDL_LED_PP_COLOR_LIST_PARAMS_STC          * portLedEntryPtr;
    PDL_OOB_PORT_ATTRIBUTES_STC                 oobPortAttributes;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (portGroupPtr == NULL || dbEntryPtr == NULL) {
        return PDL_BAD_PARAM;
    }

    switch (portGroupPtr->ledInterfaceSelect) {
        case PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_GPIO_E:
            for (ledColor = PDL_LED_COLOR_NONE_E; ledColor< PDL_LED_COLOR_LAST_E; ledColor++) {
                dbEntryPtr->interface.gpioInterface.ledInterfaceId[ledColor] = PDL_INTERFACE_INVALID_INTERFACE_ID;
                dbEntryPtr->values.gpioValues.colorValue[ledColor] = PDL_LED_UNSUPPORTED_VALUE;
                dbEntryPtr->values.gpioValues.blinkRateControlIsSupported[ledColor] = FALSE;
            }
            pdlStatus = prvPdlibDbGetFirst(portGroupPtr->ledPpGpioColorList_PTR, (void **)&gpioColorEntryPtr);
            while (pdlStatus == PDL_OK) {
                PDL_ASSERT_TRUE(gpioColorEntryPtr->list_keys.ledColor < PDL_COLOR_TYPE_LAST_E);
                pdlStatus = prvPdlInterfaceGpioWriteGroupRegister(&gpioColorEntryPtr->ledGpio.gpioWriteAddressGroupType,
                                                                  gpioColorEntryPtr->ledGpio.ledPinValue,
                                                                  &dbEntryPtr->interface.gpioInterface.ledInterfaceId[gpioColorEntryPtr->list_keys.ledColor]);
                PDL_CHECK_STATUS(pdlStatus);
                dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_GPIO_E;
                dbEntryPtr->values.gpioValues.colorValue[gpioColorEntryPtr->list_keys.ledColor] = gpioColorEntryPtr->ledGpio.ledPinValue;
                pdlStatus = prvPdlibDbGetNext(portGroupPtr->ledPpGpioColorList_PTR, (void *)&gpioColorEntryPtr->list_keys, (void **)&gpioColorEntryPtr);
            }
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        case PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_I2C_E:

            for (ledColor = PDL_LED_COLOR_NONE_E; ledColor < PDL_LED_COLOR_LAST_E; ledColor ++) {
                for (ledState = PDL_LED_STATE_OFF_E; ledState < PDL_LED_STATE_LAST_E; ledState ++) {
                    dbEntryPtr->values.i2cValues.value[ledColor][ledState] = PDL_LED_UNSUPPORTED_VALUE;
                }
            }

            pdlStatus = prvPdlInterfaceI2CReadWriteGroupRegister(&portGroupPtr->ledPpI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType,
                                                                 &dbEntryPtr->interface.i2cInterface.ledInterfaceId);
            PDL_CHECK_STATUS(pdlStatus);

            dbEntryPtr->values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E] = portGroupPtr->ledPpI2C.ledI2COffValue;
            dbEntryPtr->values.i2cValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E] = portGroupPtr->ledPpI2C.ledI2CInitValue;

            if (pdlIsFieldHasValue(portGroupPtr->ledPpColorList_mask)) {
                pdlStatus = prvPdlibDbGetFirst(portGroupPtr->ledPpColorList_PTR, (void **)&portLedEntryPtr);
                while (pdlStatus == PDL_OK) {
                    dbEntryPtr->values.i2cValues.value[portLedEntryPtr->list_keys.ledColor][portLedEntryPtr->list_keys.ledState] = portLedEntryPtr->ledPpColorI2C.ledI2CValue;

                    pdlStatus = prvPdlibDbGetNext(portGroupPtr->ledPpColorList_PTR, (void *)&portLedEntryPtr->list_keys, (void **)&portLedEntryPtr);
                }
            }

            dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_I2C_E;
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;

        case PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_EXTERNAL_DRIVER_E:
            dbEntryPtr->interface.externalDriverId = portGroupPtr->externalDriverId;
            dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_EXTERNAL_DRIVER_E;
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;

        case PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_PHY_E:

            for (ledColor = PDL_LED_COLOR_NONE_E; ledColor < PDL_LED_COLOR_LAST_E; ledColor ++) {
                for (ledState = PDL_LED_STATE_OFF_E; ledState < PDL_LED_STATE_LAST_E; ledState ++) {
                    dbEntryPtr->values.smiValues.value[ledColor][ledState] = NULL;
                }
            }

            if (pdlIsFieldHasValue(portGroupPtr->ledPpPhy.ledPhyOffExist_mask) && portGroupPtr->ledPpPhy.ledPhyOffExist == TRUE) {
                dbEntryPtr->values.smiValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_OFF_E] = portGroupPtr->ledPpPhy.ledPhyOffValue.ledPhyValueListGroupType.ledPhyValueList_PTR;
            }

            if (pdlIsFieldHasValue(portGroupPtr->ledPpPhy.ledPhyInitValuesExist_mask) && portGroupPtr->ledPpPhy.ledPhyInitValuesExist == TRUE) {
                dbEntryPtr->values.smiValues.value[PDL_LED_COLOR_NONE_E][PDL_LED_STATE_INIT_E] = portGroupPtr->ledPpPhy.ledPhyInitList_PTR;
            }

            if (pdlIsFieldHasValue(portGroupPtr->ledPpColorList_mask)) {
                pdlStatus = prvPdlibDbGetFirst(portGroupPtr->ledPpColorList_PTR, (void **)&portLedEntryPtr);
                while (pdlStatus == PDL_OK) {
                    dbEntryPtr->values.smiValues.value[portLedEntryPtr->list_keys.ledColor][portLedEntryPtr->list_keys.ledState] = portLedEntryPtr->ledPpColorPhy.ledPhyValueListGroupType.ledPhyValueList_PTR;
                    pdlStatus = prvPdlibDbGetNext(portGroupPtr->ledPpColorList_PTR, (void *)&portLedEntryPtr->list_keys, (void **)&portLedEntryPtr);
                }
            }

            if (dbEntryKeyPtr->ledId == PDL_LED_TYPE_PORT_OOB_LEFT_E || dbEntryKeyPtr->ledId == PDL_LED_TYPE_PORT_OOB_RIGHT_E) {
                pdlStatus = pdlOobPortAttributesGet(1, &oobPortAttributes);
                PDL_CHECK_STATUS(pdlStatus);
                switch (oobPortAttributes.phyConfig.smiXmsiInterface.interfaceType) {
                case PDL_INTERFACE_EXTENDED_TYPE_SMI_E:
                    dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_SMI_E;
                    break;
                case PDL_INTERFACE_EXTENDED_TYPE_XSMI_E:
                    dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_XSMI_E;
                    break;
                case PDL_INTERFACE_EXTENDED_TYPE_MPD_E:
                    dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_MPD_E;
                    break;
                default:
                    return PDL_BAD_STATE;
                }
                pdlStatus = prvPdlOobGetPhyInterfceId(&dbEntryPtr->interface.smiInterface.ledInterfaceId);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else {
                pdlStatus = pdlPpDbPortAttributesGet(dbEntryKeyPtr->dev, dbEntryKeyPtr->logicalPort, &portAttributes);
                PDL_CHECK_STATUS(pdlStatus);

                if (portAttributes.isPhyExists) {
                    if (portAttributes.phyData.smiXmsiInterface.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_SMI_E) {
                        dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_SMI_E;
                    }
                    else if (portAttributes.phyData.smiXmsiInterface.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_XSMI_E) {
                        dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_XSMI_E;
                    }
                    else if (portAttributes.phyData.smiXmsiInterface.interfaceType == PDL_INTERFACE_EXTENDED_TYPE_MPD_E) {
                        dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_MPD_E;
                    }
                    else {
                        return PDL_BAD_STATE;
                    }
                }
                else {
                    return PDL_BAD_STATE;
                }
                pdlStatus = prvPdlPpPortGetPhyInterfceId(dbEntryKeyPtr->dev, dbEntryKeyPtr->logicalPort, &dbEntryPtr->interface.smiInterface.ledInterfaceId);
                PDL_CHECK_STATUS(pdlStatus);
            }
            
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        case PDL_LED_GPIO_I2C_PHY_PP_INTERFACE_TYPE_LED_INTERFACE_PP_E:
            if (pdlIsFieldHasValue(portGroupPtr->ledPpValues.ledPpOffValue_mask)) {
                pdlStatus = prvPdlLedParseLedStreamValues(&portGroupPtr->ledPpValues.ledPpOffValue.functionCallGroupType, &dbEntryPtr->values.ledStreamFuncCalls.functionCallDb[PDL_LED_STATE_OFF_E]);
                PDL_CHECK_STATUS(pdlStatus);
            }

            if (pdlIsFieldHasValue(portGroupPtr->ledPpValues.ledPpInitValue_mask)) {
                pdlStatus = prvPdlLedParseLedStreamValues(&portGroupPtr->ledPpValues.ledPpInitValue.functionCallGroupType, &dbEntryPtr->values.ledStreamFuncCalls.functionCallDb[PDL_LED_STATE_INIT_E]);
                PDL_CHECK_STATUS(pdlStatus);
            }

            if (pdlIsFieldHasValue(portGroupPtr->ledPpColorList_mask)) {
                pdlStatus = prvPdlibDbGetFirst(portGroupPtr->ledPpColorList_PTR, (void **)&portLedEntryPtr);
                while (pdlStatus == PDL_OK) {
                    /* currently only green color is supported for led stream */
                    if (portLedEntryPtr->list_keys.ledColor == PDL_COLOR_TYPE_GREEN_E) {
                        pdlStatus = prvPdlLedParseLedStreamValues(&portLedEntryPtr->ledPpColorPp.functionCallGroupType, &dbEntryPtr->values.ledStreamFuncCalls.functionCallDb[portLedEntryPtr->list_keys.ledState]);
                        PDL_CHECK_STATUS(pdlStatus);
                    }

                    pdlStatus = prvPdlibDbGetNext(portGroupPtr->ledPpColorList_PTR, (void *)&portLedEntryPtr->list_keys, (void **)&portLedEntryPtr);
                }
            }

            dbEntryPtr->capabilities.ledType = PDL_LED_TYPE_LEDSTREAM_E;
            dbEntryPtr->capabilities.isSupported = PDL_IS_SUPPORTED_YES_E;
            break;
        default:
            return PDL_BAD_STATE;
    }
    return PDL_OK;
}

PDL_STATUS pdlLedInit(
    void
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC                   dbAttributes;
    PDL_LED_PRV_CALLBACK_STC                  * callbackInfoPtr;
    PDL_LEDS_PARAMS_STC                         ledsParam;
    PDL_LED_ID_ENT                              ledId;
    PDL_PORTLEDS_LIST_PARAMS_STC              * portEntryPtr;
    PDL_LED_PRV_DB_STC                          portLedInfo, * portLedInfoPtr;
    PDL_LED_PRV_KEY_STC                         portLedKey;
    UINT_32                                     dev, logicalPort, blinkCounter=0;
    PDL_LED_GPIO_RATE_CONTROL_LIST_PARAMS_STC * gpioBlinkRateControlEntryPtr;
    PDL_LED_PRV_GPIO_BLINK_MAPPING_KEY_STC      gpioBlinkMapKey;
    PDL_LED_PRV_GPIO_BLINK_MAPPING_DATA_STC     gpioBlinkInfo, * gpioBlinkInfoPtr;
    PDL_OOB_PORT_PARAMS_STC                     oobParams;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PRV_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_DB_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &pdlLedPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    dbAttributes.listAttributes.keySize = sizeof(PDL_LED_PRV_GPIO_BLINK_MAPPING_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_LED_PRV_GPIO_BLINK_MAPPING_DATA_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &pdlLedGpioBlinkMappingDb);
    PDL_CHECK_STATUS(pdlStatus);


    if (pdlLedInternalDriverBind == FALSE) {
        pdlStatus = prvPdlLedBindInternalDriver(&pdlLedCallbacksInternal);
        PDL_CHECK_STATUS(pdlStatus);
        pdlLedInternalDriverBind = TRUE;
    }

    memset(&pdlLedDeviceDb, 0, sizeof(pdlLedDeviceDb));

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_LEDS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&ledsParam);
    if (pdlStatus == PDL_NOT_SUPPORTED) {
        return PDL_OK;
    }
    PDL_CHECK_STATUS(pdlStatus);

    if ((pdlIsFieldHasValue(ledsParam.gpioblinkrateconrtolIsSupported_mask) == TRUE) && (ledsParam.gpioblinkrateconrtolIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.gpioblinkratecontrol_mask) == TRUE)) {
        pdlLedGpioBlinkRateControlDb = ledsParam.gpioblinkratecontrol.ledGpiorateGroupType.ledGpioRateControlList_PTR;
        pdlStatus = prvPdlibDbGetFirst(pdlLedGpioBlinkRateControlDb, (void**)&gpioBlinkRateControlEntryPtr);
        while (pdlStatus == PDL_OK) {
            gpioBlinkMapKey.dev = gpioBlinkRateControlEntryPtr->list_keys.mppDeviceNumber;
            blinkCounter = 0;
            if (gpioBlinkRateControlEntryPtr->lowrateblinkIsSupported == TRUE) {
                gpioBlinkMapKey.ledState = PDL_LED_STATE_LOWRATEBLINKING_E;
                gpioBlinkInfo.blinkSelect = blinkCounter++;
                gpioBlinkInfo.offDurationRegisterOffset = gpioBlinkRateControlEntryPtr->lowrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOffRegisterOffset;
                gpioBlinkInfo.offDurationValue = gpioBlinkRateControlEntryPtr->lowrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOffDuration;
                gpioBlinkInfo.onDurationRegisterOffset = gpioBlinkRateControlEntryPtr->lowrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOnRegisterOffset;
                gpioBlinkInfo.onDurationValue = gpioBlinkRateControlEntryPtr->lowrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOnDuration;
                pdlStatus = prvPdlibDbAdd(pdlLedGpioBlinkMappingDb, (void*) &gpioBlinkMapKey, (void*) &gpioBlinkInfo, (void**) &gpioBlinkInfoPtr);
                PDL_CHECK_STATUS(pdlStatus);
            }
            if (gpioBlinkRateControlEntryPtr->normalrateblinkIsSupported == TRUE) {
                gpioBlinkMapKey.ledState = PDL_LED_STATE_NORMALRATEBLINKING_E;
                gpioBlinkInfo.blinkSelect = blinkCounter++;
                gpioBlinkInfo.offDurationRegisterOffset = gpioBlinkRateControlEntryPtr->normalrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOffRegisterOffset;
                gpioBlinkInfo.offDurationValue = gpioBlinkRateControlEntryPtr->normalrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOffDuration;
                gpioBlinkInfo.onDurationRegisterOffset = gpioBlinkRateControlEntryPtr->normalrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOnRegisterOffset;
                gpioBlinkInfo.onDurationValue = gpioBlinkRateControlEntryPtr->normalrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOnDuration;
                pdlStatus = prvPdlibDbAdd(pdlLedGpioBlinkMappingDb, (void*) &gpioBlinkMapKey, (void*) &gpioBlinkInfo, (void**) &gpioBlinkInfoPtr);
                PDL_CHECK_STATUS(pdlStatus);
            }
            if (gpioBlinkRateControlEntryPtr->highrateblinkIsSupported == TRUE) {
                gpioBlinkMapKey.ledState = PDL_LED_STATE_HIGHRATEBLINKING_E;
                gpioBlinkInfo.blinkSelect = blinkCounter++;
                gpioBlinkInfo.offDurationRegisterOffset = gpioBlinkRateControlEntryPtr->highrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOffRegisterOffset;
                gpioBlinkInfo.offDurationValue = gpioBlinkRateControlEntryPtr->highrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOffDuration;
                gpioBlinkInfo.onDurationRegisterOffset = gpioBlinkRateControlEntryPtr->highrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOnRegisterOffset;
                gpioBlinkInfo.onDurationValue = gpioBlinkRateControlEntryPtr->highrateblinkcounters.ledGpioBlinkCounterGroupType.blinkCounterOnDuration;
                pdlStatus = prvPdlibDbAdd(pdlLedGpioBlinkMappingDb, (void*) &gpioBlinkMapKey, (void*) &gpioBlinkInfo, (void**) &gpioBlinkInfoPtr);
                PDL_CHECK_STATUS(pdlStatus);
            }
            pdlStatus = prvPdlibDbGetNext(pdlLedGpioBlinkRateControlDb, (void*)&gpioBlinkRateControlEntryPtr->list_keys, (void**)&gpioBlinkRateControlEntryPtr);
        }
    }

    if ((ledsParam.systemledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.systemled_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.systemled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_SYSTEM_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.rpsledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.rpsled_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.rpsled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_RPS_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.poeledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.poeled_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.poeled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_PORT_POE_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.fanledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.fanled_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.fanled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_FAN_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.portledstateIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.portledstateled_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.portledstateled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_PORT_PORT_LED_STATE_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.locatorledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.locatorledIsSupported_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.locatorled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_LOCATOR_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.speedledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.speedledIsSupported_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.speedled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_SPEED_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.cloudmgmtledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.cloudmgmtledIsSupported_mask) == TRUE)) {
        pdlStatus = prvPdlLedParseInterface(&ledsParam.cloudmgmtled.ledExtdrvGroupType, &pdlLedDeviceDb[PDL_LED_TYPE_DEVICE_CLOUD_MGMT_E]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    if ((ledsParam.stackledIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.stackleds_mask) == TRUE)) {
        PDL_STACKLED_LIST_PARAMS_STC    *   stackPortEntryPtr;
        pdlStatus = prvPdlibDbGetFirst(ledsParam.stackleds.stackledList_PTR, (void **)&stackPortEntryPtr);
        while (pdlStatus == PDL_OK) {
            pdlStatus = prvPdlStackNumToLedId(stackPortEntryPtr->list_keys.stackLedId, &ledId);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_ASSERT_TRUE(ledId < PDL_LED_TYPE_DEVICE_LAST_E);
            pdlStatus = prvPdlLedParseInterface(&stackPortEntryPtr->stackLed.ledExtdrvGroupType, &pdlLedDeviceDb[ledId]);
            PDL_CHECK_STATUS(pdlStatus);

            pdlStatus = prvPdlibDbGetNext(ledsParam.stackleds.stackledList_PTR, (void *)&stackPortEntryPtr->list_keys, (void **)&stackPortEntryPtr);
        }
    }

    if ((ledsParam.portledsIsSupported == TRUE) && (pdlIsFieldHasValue(ledsParam.portsleds_mask) == TRUE)) {
        pdlStatus = prvPdlibDbGetFirst(ledsParam.portsleds.portledsList_PTR, (void **)&portEntryPtr);
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlPpPortConvertFrontPanel2DevPort(portEntryPtr->list_keys.panelGroupNumber, portEntryPtr->list_keys.panelPortNumber, &dev, &logicalPort);
            PDL_CHECK_STATUS(pdlStatus);

            portLedKey.dev = dev;
            portLedKey.logicalPort = logicalPort;
            if (ledsParam.portsleds.portledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_LEFT_AND_RIGHT_E) {
                memset(&portLedInfo, 0, sizeof(portLedInfo));
                portLedKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
                pdlStatus = prvPdlLedParsePortInterface(&portEntryPtr->leftled.ledPpGroupType, &portLedKey, &portLedInfo);
                PDL_CHECK_STATUS(pdlStatus);

                pdlStatus = prvPdlibDbAdd(pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
                PDL_CHECK_STATUS(pdlStatus);

                memset(&portLedInfo, 0, sizeof(portLedInfo));
                portLedKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_RIGHT_E;
                pdlStatus = prvPdlLedParsePortInterface(&portEntryPtr->rightled.ledPpGroupType, &portLedKey, &portLedInfo);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else if (ledsParam.portsleds.portledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_SINGLE_E) {
                memset(&portLedInfo, 0, sizeof(portLedInfo));
                portLedKey.ledId = PDL_LED_TYPE_PORT_INBAND_PORT_LEFT_E;
                pdlStatus = prvPdlLedParsePortInterface(&portEntryPtr->singleled.ledPpGroupType, &portLedKey, &portLedInfo);
                PDL_CHECK_STATUS(pdlStatus);
            }
            else {
                return PDL_BAD_STATE;
            }

            pdlStatus = prvPdlibDbAdd(pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbGetNext(ledsParam.portsleds.portledsList_PTR, (void *)&portEntryPtr->list_keys, (void **)&portEntryPtr);
        }
    }
    /* OOB port LED */
    if (ledsParam.oobledstateIsSupported == TRUE) {
        pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_OOB_PORT_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&oobParams);
        PDL_CHECK_STATUS(pdlStatus);
        portLedKey.dev = oobParams.oobDevice;
        portLedKey.logicalPort = oobParams.logicalPort;
        if (ledsParam.oobleds.oobledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_LEFT_AND_RIGHT_E) {
            memset(&portLedInfo, 0, sizeof(portLedInfo));
            portLedKey.ledId = PDL_LED_TYPE_PORT_OOB_LEFT_E;
            pdlStatus = prvPdlLedParsePortInterface(&ledsParam.oobleds.oobleftled.ledPpGroupType, &portLedKey, &portLedInfo);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbAdd(pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
            memset(&portLedInfo, 0, sizeof(portLedInfo));
            portLedKey.ledId = PDL_LED_TYPE_PORT_OOB_RIGHT_E;
            pdlStatus = prvPdlLedParsePortInterface(&ledsParam.oobleds.oobrightled.ledPpGroupType, &portLedKey, &portLedInfo);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbAdd(pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else if (ledsParam.oobleds.oobledsSupportedType == PDL_PORT_LEDS_SUPPORTED_TYPE_SINGLE_E) {
            memset(&portLedInfo, 0, sizeof(portLedInfo));
            portLedKey.ledId = PDL_LED_TYPE_PORT_OOB_LEFT_E;
            pdlStatus = prvPdlLedParsePortInterface(&ledsParam.oobleds.oobsingleled.ledPpGroupType, &portLedKey, &portLedInfo);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = prvPdlibDbAdd(pdlLedPortDb, (void*) &portLedKey, (void*) &portLedInfo, (void**) &portLedInfoPtr);
            PDL_CHECK_STATUS(pdlStatus);
        }
        else {
            return PDL_BAD_STATE;
        }
    }

    /* init all external fan drivers */
    pdlStatus = prvPdlibDbGetFirst(pdlLedCallbacksDb, (void **)&callbackInfoPtr);
    while (pdlStatus == PDL_OK) {
        if (callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.initFun != NULL) {
            prvPdlLock(callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.lockType);
            callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.initFun();
            prvPdlUnlock(callbackInfoPtr->callbackInfo.ledInitDbCallbackInfo.lockType);
        }
        pdlStatus = prvPdlibDbGetNext(pdlLedCallbacksDb, (void*)&callbackInfoPtr->externalDriverId, (void **)&callbackInfoPtr);
    }

    return PDL_OK;
}
/*$ END OF pdlLedInit */

/**
 * @fn  PDL_STATUS prvPdlLedDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlLedDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                 i;
    PDL_STATUS                              pdlStatus;
    PDL_LED_STATE_ENT                       state;
    PDL_LED_COLOR_ENT                       color;
    PDL_LED_PRV_DB_STC                    * ledPtr;
    PDL_LED_PRV_KEY_STC                     ledKey;
    PDL_LED_LEDSTREAM_FUNC_CALL_INFO_STC  * funcInfoPtr;
    PDL_LED_LEDSTREAM_FUNC_CALL_KEY_STC     funcKey;

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibDbGetNextKey(pdlLedPortDb, NULL, (void*) &ledKey);
    while (pdlStatus == PDL_OK) {
        pdlStatus  = prvPdlibDbFind(pdlLedPortDb, (void*) &ledKey, (void**) &ledPtr);
        PDL_CHECK_STATUS(pdlStatus);
        if (ledPtr->capabilities.ledType == PDL_LED_TYPE_LEDSTREAM_E) {
            for (i = 0; i < PDL_LED_STATE_LAST_E; i++) {
                pdlStatus = prvPdlibDbGetNextKey(ledPtr->values.ledStreamFuncCalls.functionCallDb[i], NULL, (void*) &funcKey);
                while (pdlStatus == PDL_OK) {
                    pdlStatus  = prvPdlibDbFind(ledPtr->values.ledStreamFuncCalls.functionCallDb[i], (void*) &funcKey, (void**) &funcInfoPtr);
                    PDL_CHECK_STATUS(pdlStatus);
                    pdlStatus = prvPdlibDbDestroy(funcInfoPtr->params);
                    PDL_CHECK_STATUS(pdlStatus);
                    pdlStatus = prvPdlibDbGetNextKey(ledPtr->values.ledStreamFuncCalls.functionCallDb[i], (void*) &funcKey, (void*) &funcKey);
                }
                pdlStatus = prvPdlibDbDestroy(ledPtr->values.ledStreamFuncCalls.functionCallDb[i]);
                PDL_CHECK_STATUS(pdlStatus);
            }
        }
        pdlStatus = prvPdlibDbGetNextKey(pdlLedPortDb, (void*) &ledKey, (void*) &ledKey);
    }
    for (i = 0; i < PDL_LED_TYPE_DEVICE_LAST_E; i++) {
        if (pdlLedDeviceDb[i].capabilities.ledType == PDL_LED_TYPE_PP_REG_E) {
            for (color = 0; color < PDL_LED_COLOR_LAST_E; color++) {
                for (state = 0; state < PDL_LED_STATE_LAST_E; state++) {
                    if (pdlLedDeviceDb[i].values.ppRegValues.valueDb[color][state] != NULL) {
                        pdlStatus = prvPdlibDbDestroy(pdlLedDeviceDb[i].values.ppRegValues.valueDb[color][state]);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                    if (pdlLedDeviceDb[i].interface.ppRegInterface.colorStateInterfaceDb[color][state] != NULL) {
                        pdlStatus = prvPdlibDbDestroy(pdlLedDeviceDb[i].interface.ppRegInterface.colorStateInterfaceDb[color][state]);
                        PDL_CHECK_STATUS(pdlStatus);
                    }
                }
            }
        }
    }

    pdlStatus = prvPdlibDbDestroy(pdlLedPortDb);
    PDL_CHECK_STATUS(pdlStatus);


    pdlLedInternalDriverBind = FALSE;

    if (pdlLedDbInitDone == TRUE) {
        pdlStatus = prvPdlibDbDestroy(pdlLedCallbacksDb);
        PDL_CHECK_STATUS(pdlStatus);
        pdlLedDbInitDone = FALSE;
    }

    return PDL_OK;
}
/*$ END OF prvPdlLedDestroy */
