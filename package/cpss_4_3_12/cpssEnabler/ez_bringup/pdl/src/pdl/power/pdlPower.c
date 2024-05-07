/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlPower.c
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
 * @brief Platform driver layer - Power related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/power_supply/pdlPower.h>
#include <pdl/power_supply/pdlPowerDebug.h>
#include <pdl/power_supply/private/prvPdlPower.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl power ps db[ pdl power ps type last e] */
static PDL_POWER_PS_INTERFACE_STC           pdlPowerPsDb[PDL_POWER_PS_TYPE_LAST_E] = {{0, 0, {0}}}; /* keeps interface for reading PS status*/
static PRV_PDLIB_DB_TYP                       pdlPowerRpsStatusDb[PDL_POWER_RPS_STATUS_LAST_E];

/** @brief   The pdl power bank db[ pdl power ps type last e][pdl power bank max] */
static PDL_POWER_BANK_WATTS_TYP             pdlPowerBankDb[PDL_POWER_PS_TYPE_LAST_E][PDL_POWER_BANK_MAX];           /* keeps watts per bank */
static PDL_POWER_BANK_NUMBER_TYP            pdlPowerBankCount[PDL_POWER_PS_TYPE_LAST_E] = {0};
/** @brief   The pdl power capabilites */
static PDL_POWER_CAPABILITIES_STC           pdlPowerCapabilites;                                                    /* global - system power capabilities */
/** @brief   The pdl power port database */
static PRV_PDLIB_DB_TYP                       pdlPowerPortDb;                                                         /* per port power information */

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlPowerDebugFlag)

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbCapabilitiesGet
*
* DESCRIPTION:   Get power supported capabilities
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDbCapabilitiesGet ( OUT PDL_POWER_CAPABILITIES_STC * capabilitiesPtr )
 *
 * @brief   Pdl power database capabilities get
 *
 * @param [in,out]  capabilitiesPtr If non-null, the capabilities pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbCapabilitiesGet(
    OUT PDL_POWER_CAPABILITIES_STC *  capabilitiesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (capabilitiesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "capabilitiesPtr NULL");
        return PDL_BAD_PTR;
    }
    memcpy(capabilitiesPtr, &pdlPowerCapabilites, sizeof(pdlPowerCapabilites));
    return PDL_OK;

}

/*$ END OF pdlPowerDbCapabilitiesGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerHwStatusGet
*
* DESCRIPTION:   Get power status of the power supply
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerHwStatusGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_STATUS_ENT * statusPtr )
 *
 * @brief   Pdl power hardware status get
 *
 * @param           type        The type.
 * @param [in,out]  statusPtr   If non-null, the status pointer.
 * @param [in,out]  statusPtr   If non-null, rps explicit status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerHwStatusGet(
    IN  PDL_POWER_PS_TYPE_ENT         type,
    OUT PDL_POWER_STATUS_ENT        * statusPtr,
    OUT PDL_POWER_RPS_STATUS_ENT    * rpsStatusPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
#ifdef LINUX_HW
    BOOLEAN                                         found;
    UINT_32                                         i, data;
    PDL_STATUS                                      pdlStatus;
    PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC        * rpsStatusInterfacePtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (type >= PDL_POWER_PS_TYPE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "type >= PDL_POWER_PS_TYPE_LAST_E");
        return PDL_BAD_PARAM;
    }
    if (statusPtr == NULL || rpsStatusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "statusPtr NULL");
        return PDL_BAD_PTR;
    }
    pdlStatus = prvPdlInterfaceHwGetValue(pdlPowerPsDb[type].interfaceType, pdlPowerPsDb[type].interfaceId, &data);
    PDL_CHECK_STATUS(pdlStatus);
    if (data == pdlPowerPsDb[type].values.activeValue) {
        *statusPtr = PDL_POWER_STATUS_ACTIVE_E;
    }
    else {
        *statusPtr = PDL_POWER_STATUS_NOT_ACTIVE_E;
    }

    /* check extended RPS status */
    if (type == PDL_POWER_PS_TYPE_RPS_E && *statusPtr == PDL_POWER_STATUS_ACTIVE_E) {
        for (i = 0; i <PDL_POWER_RPS_STATUS_LAST_E; i++) {
            found = FALSE;
            pdlStatus = prvPdlibDbGetFirst(pdlPowerRpsStatusDb[i], (void*) &rpsStatusInterfacePtr);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlInterfaceHwGetValue(rpsStatusInterfacePtr->interfaceType, rpsStatusInterfacePtr->interfaceId, &data);
                PDL_CHECK_STATUS(pdlStatus);
                if (data != rpsStatusInterfacePtr->values.activeValue) {
                    found = FALSE;
                    break;
                }
                else {
                    found = TRUE;
                }
                pdlStatus = prvPdlibDbGetNext(pdlPowerRpsStatusDb[i], (void*)& rpsStatusInterfacePtr->key, (void*) &rpsStatusInterfacePtr);
            }
            if (found == TRUE) {
                *rpsStatusPtr = i;
                break;
            }
        }
    }
#else
    *statusPtr = PDL_POWER_STATUS_ACTIVE_E;
    if (rpsStatusPtr) {
        *rpsStatusPtr = PDL_POWER_RPS_STATUS_READY_E;
    }
#endif

    return PDL_OK;

}

/*$ END OF pdlPowerHwStatusGet */

/**
 * @fn  PDL_STATUS pdlPowerDbBankWattsGet ( IN PDL_POWER_PS_TYPE_ENT type, IN PDL_power_bank_number_TYP bankNumber, OUT PDL_power_bank_watts_TYP * bankWattsPtr )
 *
 * @brief   Pdl power database bank watts get
 *
 * @param           type            The type.
 * @param           bankNumber      The bank number.
 * @param [in,out]  bankWattsPtr    If non-null, the bank watts pointer.
 *
 * @return          PDL_OK          success.
 * @return          PDL_NO_SUCH     power bank doesn't exist in given bank type
 */

PDL_STATUS pdlPowerDbBankWattsGet(
    IN  PDL_POWER_PS_TYPE_ENT        type,
    IN  PDL_POWER_BANK_NUMBER_TYP    bankNumber,
    OUT PDL_POWER_BANK_WATTS_TYP   * bankWattsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (type >= PDL_POWER_PS_TYPE_LAST_E || bankNumber >= PDL_POWER_BANK_MAX) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "ILLIGAL bankNumber OR type");
        return PDL_BAD_PARAM;
    }
    if (bankWattsPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "bankWattsPtr NULL");
        return PDL_BAD_PTR;
    }
    if (pdlPowerBankDb[type][bankNumber] == PDL_POWER_BANK_INVALID) {
        return PDL_NO_SUCH;
    }
    *bankWattsPtr = pdlPowerBankDb[type][bankNumber];
    return PDL_OK;
}

/*$ END OF pdlPowerDbBankWattsGet */

/**
 * @fn  PDL_STATUS pdlPowerDbBankWattsGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_BANK_NUMBER_TYP *nunOfBanksPtr )
 *
 * @brief   Pdl power database bank watts get
 *
 * @param           type            The type.
 * @param [out]     nunOfBanksPtr   The number of banks for given ps type
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbNumOfBanksGet(
    IN  PDL_POWER_PS_TYPE_ENT        type,
    OUT PDL_POWER_BANK_NUMBER_TYP  * nunOfBanksPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (type >= PDL_POWER_PS_TYPE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "ILLIGAL type");
        return PDL_BAD_PARAM;
    }
    if (nunOfBanksPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "nunOfBanksPtr NULL");
        return PDL_BAD_PTR;
    }
    *nunOfBanksPtr = pdlPowerBankCount[type];
    return PDL_OK;
}

/*$ END OF pdlPowerDbNumOfBanksGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbPortCapabilityGet
*
* DESCRIPTION:   Get port power capability according to XML
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDbPortCapabilityGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_POE_CAPABILITY_ENT * capabilityPtr )
 *
 * @brief   Pdl power database port capability get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  capabilityPtr   If non-null, the capability pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbPortCapabilityGet(
    IN  UINT_32                         device,
    IN  UINT_32                         logicalPort,
    OUT PDL_POWER_POE_CAPABILITY_ENT  * capabilityPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (capabilityPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "capabilityPtr NULL");
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(capabilityPtr, &portPtr->poeCapability, sizeof(PDL_POWER_POE_CAPABILITY_ENT));
    return PDL_OK;
}

/*$ END OF pdlPowerDbPortCapabilityGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbPsePortTypeGet
*
* DESCRIPTION:   Get port power info according to XML
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDbPsePortTypeGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PSE_PORT_INFO_STC * psePortinfoPtr )
 *
 * @brief   Pdl power database pse port type get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  psePortinfoPtr  If non-null, the pse portinfo pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDbPsePortTypeGet(
    IN  UINT_32                         device,
    IN  UINT_32                         logicalPort,
    OUT PDL_POWER_PSE_PORT_INFO_STC   * psePortinfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (psePortinfoPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "psePortinfoPtr NULL, PORT ID: %d, DEVICE ID: %d", logicalPort, device);
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(psePortinfoPtr, &portPtr->pseInfo, sizeof(PDL_POWER_PSE_PORT_INFO_STC));
    return PDL_OK;

}

/*$ END OF pdlPowerDbPsePortTypeGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPdClear
*
* DESCRIPTION:   clears a PD port
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPdClear ( IN UINT_32 device, IN UINT_32 port )
 *
 * @brief   Pdl power pd clear
 *
 * @param   device  The device.
 * @param   port    The port.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPdClear(
    IN  UINT_32                     device,
    IN  UINT_32                     logicalPort
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    (void)device;
    (void)logicalPort;
    return PDL_OK;
}

/*$ END OF pdlPowerPdClear */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPdSet
*
* DESCRIPTION:
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPdSet ( IN UINT_32 device, IN UINT_32 logicalPort, IN PDL_POWER_PORT_TYPE_ENT powerType )
 *
 * @brief   Pdl power pd set
 *
 * @param   device      The device.
 * @param   port        The port.
 * @param   powerType   Type of the power.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPdSet(
    IN  UINT_32                     device,
    IN  UINT_32                     logicalPort,
    IN  PDL_POWER_PORT_TYPE_ENT     powerType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlPowerPortDb, (void*) &portPowerKey, (void**) &portPtr);
    PDL_CHECK_STATUS(pdlStatus);
    switch (powerType) {
        case PDL_POWER_PORT_TYPE_AT_E:
            pdlStatus = prvPdlInterfaceHwSetValue(portPtr->pdPortInterface.atInfo.interfaceType, portPtr->pdPortInterface.atInfo.interfaceId, (portPtr->pdPortInterface.values.atValue));
            break;
        case PDL_POWER_PORT_TYPE_AF_E:
            pdlStatus = prvPdlInterfaceHwSetValue(portPtr->pdPortInterface.afInfo.interfaceType, portPtr->pdPortInterface.afInfo.interfaceId, (portPtr->pdPortInterface.values.afValue));
            break;
        case PDL_POWER_PORT_TYPE_60W_E:
            pdlStatus = prvPdlInterfaceHwSetValue(portPtr->pdPortInterface.sixtyWattInfo.interfaceType, portPtr->pdPortInterface.sixtyWattInfo.interfaceId, (portPtr->pdPortInterface.values.sixtyWattValue));
            break;
        default:
            PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL powerType, PORT ID: %d, DEVICE ID: %d", logicalPort, device);
            return PDL_BAD_PARAM;
    }
    PDL_CHECK_STATUS(pdlStatus);
    return PDL_OK;
}

/*$ END OF pdlPowerPdSet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPseAddressGet
*
* DESCRIPTION:
*
* PARAMETERS:
*
*****************************************************************************/
/**
 * @fn  PDL_STATUS pdlPowerPseAddressGet ( IN UINT_8 pseNumber, OUT UINT_8 * pseAddressPtr)
 *
 * @brief   Pdl power PSE SPI address get
 *
 * @param           pseNumber       PSE ID
 * @param [in,out]  pseAddressPtr   If non-null, the pse SPI address pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlPowerPseAddressGet(
    IN  UINT_8                      pseNumber,
    OUT UINT_8                    * pseAddressPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POE_PARAMS_STC                  poeParams;
    PDL_PSE_LIST_PARAMS_STC           * psePtr;
    PDL_PSE_LIST_KEYS_STC               pseKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (pseAddressPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "pseAddressPtr=NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POE_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&poeParams);
    PDL_CHECK_STATUS(pdlStatus);

    if (pdlIsFieldHasValue(poeParams.pseportsAreSupported_mask) && poeParams.pseportsAreSupported == TRUE && pdlIsFieldHasValue(poeParams.pseports.pseList_mask)) {
        pseKey.pseNumber = pseNumber;
        pdlStatus = prvPdlibDbFind(poeParams.pseports.pseList_PTR, (void*) &pseKey, (void*) &psePtr);
        PDL_CHECK_STATUS(pdlStatus);
        *pseAddressPtr = psePtr->pseAddress;
        return PDL_OK;
    }

    return PDL_NOT_FOUND;
}
/*$ END OF pdlPowerPseAddressGet */


/* ***************************************************************************
* FUNCTION NAME: pdlPowerDbNumOfPsesGet
*
* DESCRIPTION:
*
* PARAMETERS:
*
*****************************************************************************/
/**
* @fn  PDL_STATUS pdlPowerDbNumOfPsesGet ( OUT UINT_32  *numOfPsesPtrr );
 *
 * @brief   Pdl get number of PSEs devices
 *
 * @param [OUT] *numOfPsesPtr   .
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPowerDbNumOfPsesGet(
    OUT  UINT_32    *  numOfPsesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POE_PARAMS_STC                  poeParams;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (numOfPsesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "numOfPsesPtr=NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POE_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&poeParams);
    PDL_CHECK_STATUS(pdlStatus);

    if (pdlIsFieldHasValue(poeParams.pseportsAreSupported_mask) && poeParams.pseportsAreSupported == TRUE && pdlIsFieldHasValue(poeParams.pseports.pseList_mask)) {
        return prvPdlibDbGetNumOfEntries(poeParams.pseports.pseList_PTR, numOfPsesPtr);
    }

    return PDL_NOT_SUPPORTED;
}
/*$ END OF pdlPowerDbNumOfPsesGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerIsPoeSupportedGet
*
* DESCRIPTION:
*
* PARAMETERS:
*
*****************************************************************************/
/**
* @fn  PDL_STATUS pdlPowerIsPoeSupportedGet ( OUT UINT_32  *isPoeSupportedPtr );
 *
 * @brief   Pdl is poe supported
 *
 * @param [OUT] *isPoeSupportedPtr   .
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPowerIsPoeSupportedGet(
    OUT  BOOLEAN    *  isPoeSupportedPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (isPoeSupportedPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "isPoeSupportedPtr=NULL");
        return PDL_BAD_PTR;
    }

    *isPoeSupportedPtr = pdlIsProjectFeatureSupported(PDL_FEATURE_ID_POE_E);

    return PDL_OK;
}
/*$ END OF pdlPowerIsPoeSupportedGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPoEAttributesGet
*
* DESCRIPTION:   Get PoE params/attributes
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPoEAttributesGet ( OUT PDL_POE_PARAMS_STC * poeAttributesPtr )
 *
 * @brief   Pdl get poe attributes/params
 *
 * @param [in,out]  poeAttributesPtr If non-null, the poeAttributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPoEAttributesGet(
    OUT PDL_POE_PARAMS_STC *  poeAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POE_PARAMS_STC                  poeParams;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (poeAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "poeAttributesPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POE_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&poeParams);
    PDL_CHECK_STATUS(pdlStatus);

    if (pdlIsProjectFeatureSupported(PDL_FEATURE_ID_POE_E) == TRUE) {
        /* if at least 1 value that has no data, then return an error */
        if ((pdlIsFieldHasValue(poeParams.hostSerialChannelId_mask)) &&
            (pdlIsFieldHasValue(poeParams.poeCommunicationTypeValue_mask)) &&
            (pdlIsFieldHasValue(poeParams.mcuType_mask)) &&
            (pdlIsFieldHasValue(poeParams.class0MappingMethod_mask)) &&
            (pdlIsFieldHasValue(poeParams.poeHwTypeValue_mask)) &&
            (pdlIsFieldHasValue(poeParams.fwFileName_mask))) {
            memcpy(poeAttributesPtr, &poeParams, sizeof(PDL_POE_PARAMS_STC));

            return PDL_OK;
        }
    }

    return PDL_NOT_SUPPORTED;
}

/*$ END OF pdlPowerPoEAttributesGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerIsPoePDSupportedGet
*
* DESCRIPTION:
*
* PARAMETERS:
*
*****************************************************************************/
/**
* @fn  PDL_STATUS pdlPowerIsPoePDSupportedGet ( OUT UINT_32  *isPoePDSupportedPtr );
 *
 * @brief   Pdl is poe pd supported
 *
 * @param [OUT] *isPoePDSupportedPtr   .
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPowerIsPoePDSupportedGet(
    OUT  BOOLEAN    *  isPoePDSupportedPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/

    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (isPoePDSupportedPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "isPoeSupportedPtr=NULL");
        return PDL_BAD_PTR;
    }

    *isPoePDSupportedPtr = pdlIsProjectFeatureSupported(PDL_FEATURE_ID_POEPD_E);

    return PDL_OK;
}
/*$ END OF pdlPowerIsPoeSupportedGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPoEPDAttributesGet
*
* DESCRIPTION:   Get PoE PD params/attributes
*
* PARAMETERS:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPoEPDAttributesGet ( OUT PDL_POEPD_PARAMS_STC * poePdAttributesPtr )
 *
 * @brief   Pdl get poe pd attributes/params
 *
 * @param [in,out]  poePdAttributesPtr If non-null, the poePdAttributesPtr pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPoEPDAttributesGet(
    OUT PDL_POEPD_PARAMS_STC *  poePdAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                            pdlStatus;
    PDL_POEPD_PARAMS_STC                  poePdParams;
    PDL_PDPORT_LIST_PARAMS_STC          * pdPortListEntryPtr;
    static BOOLEAN                        first_read = TRUE;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (poePdAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "poePdAttributesPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POEPD_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&poePdParams);
    PDL_CHECK_STATUS(pdlStatus);

    if (pdlIsProjectFeatureSupported(PDL_FEATURE_ID_POEPD_E) == TRUE) {
        /* if at least 1 value that has no data, then return an error */
        if (/*(pdlIsFieldHasValue(poePdParams.pdports.pdportList_mask)) &&
            (poePdParams.pdports.pdportList_PTR != NULL) &&*/
            (pdlIsFieldHasValue(poePdParams.poePoweredPdNumOfPorts_mask))
        ) {
            memcpy(poePdAttributesPtr, &poePdParams, sizeof(PDL_POEPD_PARAMS_STC));

            if (pdlIsFieldHasValue(poePdParams.pdports_mask)) {
                if(first_read) {
                    pdlStatus = prvPdlibDbGetFirst(poePdParams.pdports.pdportList_PTR, (void **)&pdPortListEntryPtr);
                    PDL_CHECK_STATUS(pdlStatus);

                    memcpy(poePdAttributesPtr->pdports.pdportList_PTR, pdPortListEntryPtr, sizeof(PDL_PDPORT_LIST_PARAMS_STC));
                    first_read = FALSE;
                }
            }

            return PDL_OK;
        }
    }

    return PDL_NOT_SUPPORTED;
}

/*$ END OF pdlPowerPoEPDAttributesGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugStatusAccessInfoGet
*
* DESCRIPTION:   get PS interface specification
*
* PARAMETERS:
*
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDebugStatusAccessInfoGet ( IN PDL_POWER_PS_TYPE_ENT type, OUT PDL_POWER_PS_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl power debug status access information get
 *
 * @param           type            The type.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugStatusAccessInfoGet(
    IN  PDL_POWER_PS_TYPE_ENT          type,
    OUT PDL_POWER_PS_INTERFACE_STC   * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (type >= PDL_POWER_PS_TYPE_LAST_E) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "ILLIGAL type");
        return PDL_BAD_PARAM;
    }
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "interfacePtr NULL");
        return PDL_BAD_PTR;
    }
    memcpy(interfacePtr, &pdlPowerPsDb[type], sizeof(PDL_POWER_PS_INTERFACE_STC));
    return PDL_OK;
}

/*$ END OF pdlPowerDebugStatusAccessInfoGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugPseActiveAccessInfoGet
*
* DESCRIPTION:   get PSE interface specification
*
* PARAMETERS:
*
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDebugPseActiveAccessInfoGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PORT_PSE_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl power debug pse active access information get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugPseActiveAccessInfoGet(
    IN  UINT_32                             device,
    IN  UINT_32                             logicalPort,
    OUT PDL_POWER_PORT_PSE_INTERFACE_STC  * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, device);
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(interfacePtr, &portPtr->pseInfo.interfaceInfo, sizeof(PDL_POWER_PORT_PSE_INTERFACE_STC));
    return PDL_OK;
}

/*$ END OF pdlPowerDebugPseActiveAccessInfoGet */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugPdActiveAccessInfoGet
*
* DESCRIPTION:   get PD interface specification
*
* PARAMETERS:
*
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerDebugPdActiveAccessInfoGet ( IN UINT_32 device, IN UINT_32 logicalPort, OUT PDL_POWER_PD_PORT_INTERFACE_STC * interfacePtr )
 *
 * @brief   Pdl power debug pd active access information get
 *
 * @param           device          The device.
 * @param           port            The port.
 * @param [in,out]  interfacePtr    If non-null, the interface pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugPdActiveAccessInfoGet(
    IN  UINT_32                             device,
    IN  UINT_32                             logicalPort,
    OUT PDL_POWER_PD_PORT_INTERFACE_STC   * interfacePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC         * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC          portPowerKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (interfacePtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "interfacePtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, device);
        return PDL_BAD_PTR;
    }
    portPowerKey.dev = device;
    portPowerKey.logicalPort = logicalPort;
    pdlStatus = prvPdlibDbFind(pdlPowerPortDb, (void*) &portPowerKey, (void*) &portPtr);
    PDL_CHECK_STATUS(pdlStatus);
    memcpy(interfacePtr, &portPtr->pdPortInterface, sizeof(PDL_POWER_PD_PORT_INTERFACE_STC));
    return PDL_OK;
}

/*$ END OF pdlPowerDebugPdActiveAccessInfoGet */

static PDL_STATUS prvPdlPowerParseReadOnlyInterface(
    IN  PDL_GROUPING_POWER_CONNECTION_READ_ONLY_GROUP_TYPE_PARAMS_STC * extdrvGroupPtr,
    OUT PDL_INTERFACE_TYPE_ENT                                        * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                                             * interfaceIdPtr,
    OUT UINT_32                                                       * valuePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                                  pdlStatus;
    PDL_POWER_CONNECTION_READ_ONLY_GPIO_GROUP_LIST_PARAMS_STC * gpioPowerEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (extdrvGroupPtr == NULL || interfaceTypePtr == NULL || interfaceIdPtr == NULL || valuePtr == NULL) {
        return PDL_BAD_PARAM;
    }

    switch (extdrvGroupPtr->interfaceSelect) {
        case PDL_INTERFACE_TYPE_INTERFACE_GPIO_E:
            /* TODO: this is list but there is only one interface returned ??? */
            pdlStatus = prvPdlibDbGetFirst(extdrvGroupPtr->powerConnectionReadOnlyGpioGroupList_PTR, (void **)&gpioPowerEntryPtr);
            while (pdlStatus == PDL_OK) {
                pdlStatus = prvPdlInterfaceGpioReadGroupRegister(&gpioPowerEntryPtr->gpioReadAddressGroupType,
                                                                 gpioPowerEntryPtr->activeValue,
                                                                 interfaceIdPtr);
                PDL_CHECK_STATUS(pdlStatus);

                *interfaceTypePtr = PDL_INTERFACE_TYPE_INTERFACE_GPIO_E;
                *valuePtr = gpioPowerEntryPtr->activeValue;

                pdlStatus = prvPdlibDbGetNext(extdrvGroupPtr->powerConnectionReadOnlyGpioGroupList_PTR, (void *)&gpioPowerEntryPtr->list_keys, (void **)&gpioPowerEntryPtr);
            }
            break;
        case PDL_INTERFACE_TYPE_INTERFACE_I2C_E:
            pdlStatus = prvPdlInterfaceI2CReadWriteGroupRegister(&extdrvGroupPtr->powerConnectionReadOnlyI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType,
                                                                 interfaceIdPtr);
            PDL_CHECK_STATUS(pdlStatus);
            *interfaceTypePtr = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
            *valuePtr = extdrvGroupPtr->powerConnectionReadOnlyI2C.activeValue;
            break;
        default:
            return PDL_BAD_STATE;
    }
    return PDL_OK;
}

/*
static PDL_STATUS prvPdlPowerParseReadWriteInterface(
    IN  PDL_GROUPING_POWER_CONNECTION_READ_WRITE_GROUP_TYPE_PARAMS_STC * extdrvGroupPtr,
    OUT PDL_INTERFACE_TYPE_ENT                                         * interfaceTypePtr,
    OUT PDL_INTERFACE_TYP                                              * interfaceIdPtr,
    OUT UINT_32                                                        * valuePtr
)
{
     ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /*PDL_STATUS                                                  pdlStatus;*/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    /*if (extdrvGroupPtr == NULL || interfaceTypePtr == NULL || interfaceIdPtr == NULL || valuePtr == NULL) {
        return PDL_BAD_PARAM;
    }

    switch (extdrvGroupPtr->interfaceSelect) {
        case PDL_INTERFACE_TYPE_INTERFACE_GPIO_E:
            pdlStatus = prvPdlInterfaceGpioWriteGroupRegister(&extdrvGroupPtr->powerConnectionReadWriteGpio.gpioWriteAddressGroupType,
                                                              extdrvGroupPtr->powerConnectionReadWriteGpio.gpioActiveValue,
                                                              interfaceIdPtr);
            PDL_CHECK_STATUS(pdlStatus);

            *interfaceTypePtr = PDL_INTERFACE_TYPE_INTERFACE_GPIO_E;
            *valuePtr = extdrvGroupPtr->powerConnectionReadWriteGpio.gpioActiveValue;
            break;
        case PDL_INTERFACE_TYPE_INTERFACE_I2C_E:
            pdlStatus = prvPdlInterfaceI2CReadWriteGroupRegister(&extdrvGroupPtr->powerConnectionReadWriteI2C.i2CReadWriteAddressGroupType.i2CReadWriteAddress.i2CReadWriteAddressFieldsGroupType,
                                                                 interfaceIdPtr);
            PDL_CHECK_STATUS(pdlStatus);
            *interfaceTypePtr = PDL_INTERFACE_TYPE_INTERFACE_I2C_E;
            *valuePtr = extdrvGroupPtr->powerConnectionReadWriteI2C.i2CActiveValue;
            break;
        default:
            return PDL_BAD_STATE;
    }
    return PDL_ERROR;
}*/

/**
 * @fn  PDL_STATUS prvPdlPowerDbsBuild ( void )
 *
 * @brief   Prv pdl power databases build
 *
 * @return  A PDL_STATUS.
 */

static PDL_STATUS prvPdlPowerDbsBuild(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                     psePorts = 0, pdPorts = 0;
    PDL_STATUS                                  pdlStatus;
    PDL_POWER_PORT_PRV_DB_STC                   portInfo;
    PDL_POWER_PORT_PRV_DB_STC                 * portPtr;
    PDL_POWER_PORT_PRV_KEY_STC                  portKey;
    PDL_POWER_RPS_STATUS_ENT                    rpsStatusType = PDL_POWER_RPS_STATUS_STANDBY_E;
    PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC      rpsStatusInterface;
    PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC    * rpsStatusInterfacePtr;
    PDL_POWER_PARAMS_STC                        powerParams;
    PDL_POE_PARAMS_STC                          poeParams;
    PDL_POEPD_PARAMS_STC                        poePdParams;
    PDL_STATUS_LIST_PARAMS_STC                * statusListEntryPtr;
    PDL_POEBANK_LIST_PARAMS_STC               * poeBanksEntryPtr;
    PDL_PSEPORT_LIST_PARAMS_STC               * psePortListEntryPtr;
    PDL_PDPORT_LIST_PARAMS_STC                * pdPortListEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POWER_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&powerParams);
    if (pdlStatus == PDL_OK) {
        if (pdlIsFieldHasValue(powerParams.ps.psActiveinfo.powerConnectionReadOnlyGroupType_mask)) {
            pdlPowerCapabilites.PsIsSupported = TRUE;
            pdlStatus = prvPdlPowerParseReadOnlyInterface(&powerParams.ps.psActiveinfo.powerConnectionReadOnlyGroupType,
                                                          &pdlPowerPsDb[PDL_POWER_PS_TYPE_PS_E].interfaceType,
                                                          &pdlPowerPsDb[PDL_POWER_PS_TYPE_PS_E].interfaceId,
                                                          &pdlPowerPsDb[PDL_POWER_PS_TYPE_PS_E].values.activeValue);
            PDL_CHECK_STATUS(pdlStatus);
        }

        if (pdlIsFieldHasValue(powerParams.rps.statusList_mask)) {
            pdlPowerCapabilites.RpsIsSupported = TRUE;
            pdlStatus = prvPdlibDbGetFirst(powerParams.rps.statusList_PTR, (void **)&statusListEntryPtr);
            while (pdlStatus == PDL_OK) {
                rpsStatusType = (PDL_POWER_RPS_STATUS_ENT)(statusListEntryPtr->list_keys.statusType - 1);

                pdlStatus = prvPdlPowerParseReadOnlyInterface(&powerParams.rps.connectedinfo.powerConnectionReadOnlyGroupType,
                                                              &rpsStatusInterface.interfaceType,
                                                              &rpsStatusInterface.interfaceId,
                                                              &rpsStatusInterface.values.activeValue);
                PDL_CHECK_STATUS(pdlStatus);

                pdlStatus = prvPdlibDbGetNumOfEntries(pdlPowerRpsStatusDb[rpsStatusType], &rpsStatusInterface.key.index);
                PDL_CHECK_STATUS(pdlStatus);

                pdlStatus = prvPdlibDbAdd(pdlPowerRpsStatusDb[rpsStatusType], (void*) &rpsStatusInterface.key, (void*) &rpsStatusInterface, (void**) &rpsStatusInterfacePtr);
                PDL_CHECK_STATUS(pdlStatus);

                pdlStatus = prvPdlibDbGetNext(powerParams.rps.statusList_PTR, (void *)&statusListEntryPtr->list_keys, (void **)&statusListEntryPtr);
            }
        }
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POE_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&poeParams);
    if (pdlStatus == PDL_OK) {
        PDL_CHECK_STATUS(pdlStatus);

        if (pdlIsFieldHasValue(poeParams.poePowerBanks.poebankList_mask)) {
            pdlStatus = prvPdlibDbGetFirst(poeParams.poePowerBanks.poebankList_PTR, (void **)&poeBanksEntryPtr);
            while (pdlStatus == PDL_OK) {
                pdlPowerBankDb[poeBanksEntryPtr->bankSource-1][poeBanksEntryPtr->list_keys.bankNumber] = poeBanksEntryPtr->bankWatts;
                pdlPowerBankCount[poeBanksEntryPtr->bankSource-1]++;

                pdlStatus = prvPdlibDbGetNext(poeParams.poePowerBanks.poebankList_PTR, (void *)&poeBanksEntryPtr->list_keys, (void **)&poeBanksEntryPtr);
            }
        }

        if (pdlIsFieldHasValue(poeParams.pseports.pseInformation_mask)) {
            pdlPowerCapabilites.pseType = (PDL_POWER_PSE_TYPE_ENT)(poeParams.pseports.pseInformation-1);
        }

        if (pdlIsFieldHasValue(poeParams.pseportsAreSupported_mask) && poeParams.pseportsAreSupported == TRUE) {
            if (pdlIsFieldHasValue(poeParams.pseports.pseportList_mask)) {
                pdlStatus = prvPdlibDbGetFirst(poeParams.pseports.pseportList_PTR, (void **)&psePortListEntryPtr);
                while (pdlStatus == PDL_OK) {
                    memset(&portInfo, 0, sizeof(portInfo));

                    if (pdlIsFieldHasValue(psePortListEntryPtr->pseGroup.pseGroupActiveinfo.powerConnectionReadOnlyGroupType_mask)) {
                        pdlStatus = prvPdlPowerParseReadOnlyInterface(&psePortListEntryPtr->pseGroup.pseGroupActiveinfo.powerConnectionReadOnlyGroupType,
                            &portInfo.pseInfo.interfaceInfo.interfaceType,
                            &portInfo.pseInfo.interfaceInfo.InterfaceId,
                            &portInfo.pseInfo.interfaceInfo.value.activeValue);
                        PDL_CHECK_STATUS(pdlStatus);
                    }

                    pdlStatus = pdlPpPortConvertFrontPanel2DevPort(psePortListEntryPtr->list_keys.panelGroupNumber, psePortListEntryPtr->list_keys.panelPortNumber, &portKey.dev, &portKey.logicalPort);
                    PDL_CHECK_STATUS(pdlStatus);

                    portInfo.pseInfo.index1 = psePortListEntryPtr->pseGroup.index1;
                    portInfo.pseInfo.index2 = psePortListEntryPtr->pseGroup.index2;
                    portInfo.pseInfo.port_power_type = (PDL_POWER_PORT_TYPE_ENT)(psePortListEntryPtr->pseGroup.portType-1);
                    portInfo.pseInfo.logicalPort = portKey.logicalPort;

                    portInfo.poeCapability = PDL_POWER_POE_CAPABILITY_PSE_E;

                    psePorts++;

                    pdlStatus = prvPdlibDbAdd(pdlPowerPortDb, (void*) &portKey, (void*) &portInfo, (void**) &portPtr);
                    PDL_CHECK_STATUS(pdlStatus);

                    pdlStatus = prvPdlibDbGetNext(poeParams.pseports.pseportList_PTR, (void *)&psePortListEntryPtr->list_keys, (void **)&psePortListEntryPtr);
                }
            }
        }
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_POEPD_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&poePdParams);
    if (pdlStatus == PDL_OK) {
        PDL_CHECK_STATUS(pdlStatus);

        if (pdlIsFieldHasValue(poePdParams.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType_mask)) {
            pdlStatus = prvPdlPowerParseReadOnlyInterface(&poePdParams.pdports.pd.pdActiveinfo.powerConnectionReadOnlyGroupType,
                                                            &pdlPowerPsDb[PDL_POWER_PS_TYPE_PD_E].interfaceType,
                                                            &pdlPowerPsDb[PDL_POWER_PS_TYPE_PD_E].interfaceId,
                                                            &pdlPowerPsDb[PDL_POWER_PS_TYPE_PD_E].values.activeValue);
            PDL_CHECK_STATUS(pdlStatus);
        }

        if (pdlIsFieldHasValue(poePdParams.pdports.pdportList_mask)) {
            pdlStatus = prvPdlibDbGetFirst(poePdParams.pdports.pdportList_PTR, (void **)&pdPortListEntryPtr);
            while (pdlStatus == PDL_OK) {
                memset(&portInfo, 0, sizeof(portInfo));

                pdlStatus = pdlPpPortConvertFrontPanel2DevPort(pdPortListEntryPtr->list_keys.panelGroupNumber, pdPortListEntryPtr->list_keys.panelPortNumber, &portKey.dev, &portKey.logicalPort);
                PDL_CHECK_STATUS(pdlStatus);

                portInfo.poeCapability = PDL_POWER_POE_CAPABILITY_PD_E;

                /* check if some port information already exists */
                pdlStatus = prvPdlibDbFind(pdlPowerPortDb, (void*) &portKey, (void**) &portPtr);

                if (pdlStatus == PDL_NOT_FOUND) {
                    pdPorts++;
                    pdlStatus = prvPdlibDbAdd(pdlPowerPortDb, (void*) &portKey, (void*) &portInfo, (void**) &portPtr);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    memcpy(&portPtr->pdPortInterface, &portInfo.pdPortInterface, sizeof(portInfo.pdPortInterface));
                }

                pdlStatus = prvPdlibDbGetNext(poePdParams.pdports.pdportList_PTR, (void *)&pdPortListEntryPtr->list_keys, (void **)&pdPortListEntryPtr);
            }
        }
    }

    pdlPowerCapabilites.numberOfPdPorts = pdPorts;
    pdlPowerCapabilites.numberOfPsePorts = psePorts;

    return PDL_OK;
}

/*$ END OF prvPdlPowerXMLParser */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerDebugSet
*
* DESCRIPTION:   Enable/disable debug flag
*
* PARAMETERS:
*
*
*****************************************************************************/
/**
 * @fn  PDL_STATUS pdlPowerDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Power debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlPowerDebugFlag = state;
    return PDL_OK;
}
/*$ END OF pdlPowerDebugSet */


/**
 * @fn  PDL_STATUS pdlPowerInit ( void )
 *
 * @brief   Pdl power initialize
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlPowerInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                         i, j;
    PDL_STATUS                      pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC       dbAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    dbAttributes.listAttributes.entrySize = sizeof(PDL_POWER_PORT_PRV_DB_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_POWER_PORT_PRV_KEY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &pdlPowerPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    memset(&pdlPowerCapabilites, 0, sizeof(pdlPowerCapabilites));

    for (i = 0; i < PDL_POWER_PS_TYPE_LAST_E; i++) {
        for (j = 0; j <PDL_POWER_BANK_MAX; j++) {
            pdlPowerBankDb[i][j] = PDL_POWER_BANK_INVALID;
        }
    }

    /* create DB for RPS status types */
    dbAttributes.listAttributes.entrySize = sizeof(PDL_POWER_RPS_STATUS_PRV_INTERFACE_STC);
    dbAttributes.listAttributes.keySize = sizeof(PDL_POWER_RPS_STATUS_PRV_KEY_STC);
    for (i = 0; i < PDL_POWER_RPS_STATUS_LAST_E; i++) {
        pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                     &dbAttributes,
                                     &pdlPowerRpsStatusDb[i]);
        PDL_CHECK_STATUS(pdlStatus);
    }

    pdlStatus = prvPdlPowerDbsBuild();
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF pdlPowerInit */

/**
 * @fn  PDL_STATUS prvPdlPowerDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPowerDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                 i;
    PDL_STATUS                              pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibDbDestroy(pdlPowerPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    for (i = 0; i < PDL_POWER_RPS_STATUS_LAST_E; i++) {
        pdlStatus = prvPdlibDbDestroy(pdlPowerRpsStatusDb[i]);
        PDL_CHECK_STATUS(pdlStatus);
    }
    return PDL_OK;
}

/*$ END OF prvPdlPowerDestroy */

/* ***************************************************************************
* FUNCTION NAME: pdlPowerPDPortPoweredStatusGet
*
* DESCRIPTION:   get PD port powered status 
*
* PARAMETERS:
*
* XML structure:
*
*****************************************************************************/

/**
 * @fn  PDL_STATUS pdlPowerPDPortPoweredStatusGet ( OUT PDL_POWER_PD_STATUS_ENT * pdPoweredStatusPtr )
 *
 * @brief   Pdl power debug status access information get (powered/absent)
 *
 * @param [in,out]  interfacePtr    If non-null, the pd port status pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPowerPDPortPoweredStatusGet(
    OUT PDL_POWER_PD_STATUS_ENT   * pdPoweredStatusPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32    data;
    PDL_STATUS pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (pdPoweredStatusPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPowerDebugFlag)(__FUNCTION__, __LINE__, "pdPoweredStatusPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlInterfaceHwGetValue(pdlPowerPsDb[PDL_POWER_PS_TYPE_PD_E].interfaceType, pdlPowerPsDb[PDL_POWER_PS_TYPE_PD_E].interfaceId, &data);

    PDL_CHECK_STATUS(pdlStatus);

    if (data == PDL_POWER_PD_STATUS_POWERED_E) {
        *pdPoweredStatusPtr = PDL_POWER_PD_STATUS_POWERED_E;
    }
    else {
        *pdPoweredStatusPtr = PDL_POWER_PD_STATUS_ABSENT_E;
    }

    return PDL_OK;
}

/* END OF pdlPowerPDPortPoweredStatusGet */