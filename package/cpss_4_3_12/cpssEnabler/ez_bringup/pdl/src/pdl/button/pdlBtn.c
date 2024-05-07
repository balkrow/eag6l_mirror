/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlBtn.c
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
#include <pdl/button/pdlBtn.h>
/*#include <pdl/lib/pdlLib.h>*/
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/button/private/prvPdlBtn.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/parser/pdlParser.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

static PRV_PDLIB_DB_TYP   prvPdlBtnDb;

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlBtnDebugFlag)

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @fn  PDL_STATUS pdlBtnDbCapabilitiesGet ( IN PDL_BTN_ID_ENT buttonId, OUT PDL_BTN_XML_CAPABILITIES_STC * capabilitiesPtr );
 *
 * @brief   Get \b buttonId supported capabilities
 *          @note
 *          Based on data retrieved from XML
 *
 * @param [in]  buttonId        - button identifier.
 * @param [out] capabilitiesPtr - button capabilities.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlBtnDbCapabilitiesGet(
    IN  PDL_BTN_ID_ENT                  buttonId,
    OUT PDL_BTN_XML_CAPABILITIES_STC  * capabilitiesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_BUTTONS_PARAMS_STC              params;
    PDL_BUTTONS_LIST_PARAMS_STC       * buttonPtr;
    PDL_BUTTONS_LIST_KEYS_STC           btnKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL POINTER");
        return PDL_BAD_PTR;
    }
    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_BUTTONS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "BUTTON FEATURE DOESNT EXIST");
        return pdlStatus;
    }

    switch (buttonId) {
        case PDL_BTN_ID_RESET_E:
            btnKey.buttonId = PDL_BUTTON_TYPE_RESET_E;
            break;
        case PDL_BTN_ID_LED_MODE_E:
            btnKey.buttonId = PDL_BUTTON_TYPE_LED_E;
            break;
        default:
            return PDL_BAD_PARAM;
    }

    pdlStatus = prvPdlibDbFind(params.buttonsList_PTR, (void*) &btnKey, (void**) &buttonPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        capabilitiesPtr->isSupported = PDL_IS_SUPPORTED_NO_E;
        return PDL_OK;
    }
    PDL_CHECK_STATUS(pdlStatus);
    capabilitiesPtr->isSupported = PDL_IS_SUPPORTED_YES_E;

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, capabilitiesPtr->isSupported);
    return PDL_OK;
}

/*$ END OF pdlBtnDbCapabilitiesGet */

/**
 * @fn  PDL_STATUS pdlBtnHwStatusGet ( IN PDL_BTN_ID_ENT buttonId, OUT PDL_BTN_STATUS_ENT * statusPtr );
 *
 * @brief   get button operational status (pressed/not pressed)
 *          based on information from HW access
 *
 * @param [in]  buttonId    - button identifier.
 * @param [out] statusPtr   - button status.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlBtnHwStatusGet(
    IN  PDL_BTN_ID_ENT                 buttonId,
    OUT PDL_BTN_STATUS_ENT         *   statusPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                             data;
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC                * buttonPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (statusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL statusPtr");
        return PDL_BAD_PTR;
    }

    btnKey.btnId = buttonId;
    pdlStatus = prvPdlibDbFind(prvPdlBtnDb, (void*) &btnKey, (void**) &buttonPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlInterfaceHwGetValue(buttonPtr->btnInterface.interfaceType, buttonPtr->btnInterface.buttonInterfaceId, &data);
    PDL_CHECK_STATUS(pdlStatus);
    if (data == buttonPtr->btnInterface.values.pushValue) {
        *statusPtr = PDL_BTN_STATUS_PRESSED_E;
    }
    else {
        *statusPtr = PDL_BTN_STATUS_NOT_PRESSED_E;
    }

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, *statusPtr);
    return PDL_OK;
}

/*$ END OF pdlBtnHwStatusGet */

/**
 * @fn  PDL_STATUS pdlBtnDebugInterfaceGet ( IN  PDL_BTN_ID_ENT buttonId, OUT PDL_BTN_INTERFACE_STC * interfacePtr )
 *
 * @brief   Get interface information that button is using
 *
 * @param   [in]  buttonId     - button identifier.
 * @param   [out] interfacePtr - button's interface information
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlBtnDebugInterfaceGet(
    IN  PDL_BTN_ID_ENT            buttonId,
    OUT PDL_BTN_INTERFACE_STC   * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC                * btnPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL POINTER");
        return PDL_BAD_PTR;
    }

    btnKey.btnId = buttonId;
    pdlStatus = prvPdlibDbFind(prvPdlBtnDb, (void*) &btnKey, (void**) &btnPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(interfacePtr, &btnPtr->btnInterface, sizeof(btnPtr->btnInterface));

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->buttonInterfaceId);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->interfaceType);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, interfacePtr->values.pushValue);
    return PDL_OK;
}

/*$ END OF pdlBtnDebugInterfaceGet */

/**
 * @fn  PDL_STATUS pdlBtnDebugInterfaceSet ( IN  PDL_BTN_ID_ENT buttonId, IN  PDL_BTN_VALUES_STC * valuesPtr )
 *
 * @brief   Set value to be recognized as push button value
 *
 * @param   [in] buttonId  - button identifier.
 * @param   [in] valuesPtr - button values to use
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlBtnDebugInterfaceSet(
    IN  PDL_BTN_ID_ENT            buttonId,
    IN  PDL_BTN_VALUES_STC      * valuesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_BTN_PRV_DB_STC                * btnPtr;
    PDL_BTN_PRV_KEY_STC                 btnKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (valuesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "NULL POINTER");
        return PDL_BAD_PTR;
    }

    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_32, valuesPtr->pushValue);

    btnKey.btnId = buttonId;
    pdlStatus = prvPdlibDbFind(prvPdlBtnDb, (void*) &btnKey, (void**) &btnPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(&btnPtr->btnInterface.values, valuesPtr, sizeof(PDL_BTN_VALUES_STC));

    return PDL_OK;
}
/*$ END OF pdlBtnDebugInterfaceSet */

/**
 * @fn  PDL_STATUS pdlBtnDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Btn debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlBtnDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlBtnDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlBtnDebugSet */

/**
 * @fn  PDL_STATUS pdlBtnInit ( void );
 *
 * @brief   Init button module Create button DB and initialize
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlBtnInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC           dbAttributes;
    PDL_BUTTONS_PARAMS_STC              params;
    PDL_BUTTONS_LIST_PARAMS_STC       * buttonPtr;
    PDL_BTN_PRV_DB_STC                * btnEntryPtr, btnEntry;
    PDL_BTN_PRV_KEY_STC                 btnKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_BTN_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_BTN_PRV_KEY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlBtnDb);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_BUTTONS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    if (pdlStatus == PDL_NOT_SUPPORTED) {
        return PDL_OK;
    }
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "BUTTON FEATURE DOESNT EXIST");
        return pdlStatus;
    }

    pdlStatus = prvPdlibDbGetFirst(params.buttonsList_PTR, (void**) &buttonPtr);
    while (pdlStatus == PDL_OK) {
        switch (buttonPtr->list_keys.buttonId) {
            case PDL_BUTTON_TYPE_RESET_E:
                btnKey.btnId = PDL_BTN_ID_RESET_E;
                break;
            case PDL_BUTTON_TYPE_LED_E:
                btnKey.btnId = PDL_BTN_ID_LED_MODE_E;
                break;
            default:
                return PDL_BAD_STATE;
        }

        pdlStatus = prvPdlInterfaceReadGroupRegister(&buttonPtr->pushValue.readInterfaceGroupType, &btnEntry.btnInterface.buttonInterfaceId);
        PDL_CHECK_STATUS(pdlStatus);
        btnEntry.btnInterface.interfaceType = buttonPtr->pushValue.readInterfaceGroupType.readInterfaceSelect;
        if (btnEntry.btnInterface.interfaceType == PDL_INTERFACE_TYPE_INTERFACE_I2C_E) {
            btnEntry.btnInterface.values.pushValue = buttonPtr->pushValue.readInterfaceGroupType.readI2CInterface.i2CReadValue;
        }
        else {
            btnEntry.btnInterface.values.pushValue = buttonPtr->pushValue.readInterfaceGroupType.readGpioInterface.gpioReadValue;
        }

        pdlStatus = prvPdlibDbAdd(prvPdlBtnDb, (void *)&btnKey, (void *)&btnEntry, (void **)&btnEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);

        pdlStatus = prvPdlibDbGetNext(params.buttonsList_PTR, (void*) &buttonPtr->list_keys, (void**) &buttonPtr);
    }

    return PDL_OK;
}
/*$ END OF pdlBtnInit */

/**
 * @fn  PDL_STATUS prvPdlBtnDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlBtnDestroy(
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
    pdlStatus = prvPdlibDbDestroy(prvPdlBtnDb);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF prvPdlBtnDestroy */
