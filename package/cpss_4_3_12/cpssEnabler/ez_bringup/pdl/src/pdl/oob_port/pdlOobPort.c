/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlOobPort.c
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
 * @brief Platform driver layer - OOB port related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/oob_port/private/prvPdlOobPort.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The prv pdl oob port database */
static PRV_PDLIB_DB_TYP   prvPdlOobPortDb;


/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlOobPortDebugFlag)
/**
 * @fn  PDL_STATUS pdlOobPortNumOfObbPortsGet ( OUT UINT_32 * numOfOobsPtr )
 *
 * @brief   Get number of OOB ports
 *
 * @param [out] numOfOobsPtr    number of oob ports in system.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortNumOfObbPortsGet(
    OUT  UINT_32    * numOfOobsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS      pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    pdlStatus = prvPdlibDbGetNumOfEntries(prvPdlOobPortDb, numOfOobsPtr);

    return pdlStatus;
}

/**
 * @fn  PDL_STATUS pdlOobPortAttributesGet ( IN UINT_32 portNumber, OUT PDL_OOB_PORT_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Get OOB port attributes
 *
 * @param [in]  portNumber          OOB port number.
 * @param [out] portAttributesPtr   OOB port attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortAttributesGet(
    IN   UINT_32                                portNumber,
    OUT  PDL_OOB_PORT_ATTRIBUTES_STC          * portAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                       pdlStatus;
    PRV_PDL_OOB_PORT_ENTRY_STC     * oobEntryPtr;
    PRV_PDL_OOB_PORT_KEY_STC         oobKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    oobKey.portNumber = portNumber;
    pdlStatus = prvPdlibDbFind(prvPdlOobPortDb, &oobKey, (void **)&oobEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *portAttributesPtr = oobEntryPtr->oobInfo;
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.ethId);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.cpuPortNumber);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.portMaximumSpeed);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.phyNumber);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.phyPosition);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.interfaceId);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.interfaceType);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.dev);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, oobEntryPtr->oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.address);

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlOobPortAttributesSet ( IN UINT_32 portNumber, IN PDL_OOB_PORT_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Set OOB port attributes
 *
 * @param [in] portNumber           OOB port number.
 * @param [in] portAttributesPtr    OOB port attributes.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlOobPortAttributesSet(
    IN   UINT_32                                portNumber,
    IN   PDL_OOB_PORT_ATTRIBUTES_STC          * portAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                       pdlStatus;
    PRV_PDL_OOB_PORT_ENTRY_STC     * oobEntryPtr;
    PRV_PDL_OOB_PORT_KEY_STC         oobKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->ethId);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->cpuPortNumber);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->portMaximumSpeed);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.phyNumber);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.phyPosition);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.interfaceId);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.interfaceType);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.dev);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, portAttributesPtr->phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.address);

    oobKey.portNumber = portNumber;
    pdlStatus = prvPdlibDbFind(prvPdlOobPortDb, &oobKey, (void **)&oobEntryPtr);
    if (pdlStatus != PDL_OK) {
        return PDL_NOT_FOUND;
    }

    oobEntryPtr->oobInfo = *portAttributesPtr;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlOobPortInit ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId )
 *
 * @brief   Init oob port module Create oob port DB and initialize
 *
 * @param [in]  xmlId   Xml id.
 *
 * @return  PDL_STATUS.
 */

/**
 * @fn  PDL_STATUS pdlOobDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Oob debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlOobDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlOobPortDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlOobDebugSet */

/**
 * @fn  PDL_STATUS prvPdlOobGetPhyInterfceId (OUT UINT_32 * phyInterfaceIdPtr );
 *
 * @brief   Get the OOB interface id that is used inside PDL private interface database
 *
 * @param [out]     phyInterfaceIdPtr       phyInterfaceId.
  *
 * @return  PDL_OK         success
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  OOB port not found
 */

PDL_STATUS prvPdlOobGetPhyInterfceId(
    OUT UINT_32                                * phyInterfaceIdPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                       pdlStatus;
    PRV_PDL_OOB_PORT_ENTRY_STC     * oobEntryPtr;
    PRV_PDL_OOB_PORT_KEY_STC         oobKey;
/* ***************************************************************************/
/*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    if (phyInterfaceIdPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "macPortPtr or devNumPtr POINTER NULL");
        return PDL_BAD_PTR;
    }
    oobKey.portNumber = 1;
    pdlStatus = prvPdlibDbFind(prvPdlOobPortDb, &oobKey, (void **)&oobEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);
    *phyInterfaceIdPtr = oobEntryPtr->phyInterfaceId;
    return PDL_OK;
}

/*$ END OF prvPdlOobGetPhyInterfceId */

PDL_STATUS prvPdlOobPortInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDLIB_DB_ATTRIBUTES_STC                             dbAttributes;
    PDL_OOB_PORT_PARAMS_STC                                 oobParams;
    PRV_PDL_OOB_PORT_ENTRY_STC                              oobPortEntry, *oobPortEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_OOB_PORT_ENTRY_STC);
    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_OOB_PORT_KEY_STC);

    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlOobPortDb);
    PDL_CHECK_STATUS(pdlStatus);

    memset(&oobPortEntry, 0, sizeof(oobPortEntry));
    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_OOB_PORT_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&oobParams);
    if (pdlStatus == PDL_OK) {

        oobPortEntry.key.portNumber = 1;
        if (pdlIsFieldHasValue(oobParams.cpuPortNumber_mask)) {
            oobPortEntry.oobInfo.cpuPortNumber = oobParams.cpuPortNumber;
        }
        else {
            oobPortEntry.oobInfo.cpuPortNumber = 0;
        }
        oobPortEntry.oobInfo.ethId = oobParams.ethId;
        oobPortEntry.oobInfo.portMaximumSpeed = (PDL_PORT_SPEED_ENT)(oobParams.speed - 1);
        if (pdlIsFieldHasValue(oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress_mask)) {
            oobPortEntry.oobInfo.phyConfig.phyNumber = oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress.phyNumber;
            oobPortEntry.oobInfo.phyConfig.phyPosition = oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPosition;
            oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.dev = oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice;
            oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.address = oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress;
            oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.interfaceId = oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId;
            oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.interfaceType = oobParams.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType == PDL_SMI_XSMI_TYPE_TYPE_SMI_E ?
                                                                            PDL_INTERFACE_EXTENDED_TYPE_SMI_E : PDL_INTERFACE_EXTENDED_TYPE_XSMI_E;
            pdlStatus = prvPdlSmiXsmiInterfaceRegister(oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.dev,
                                                       oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.address,
                                                       oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.phyInfo.readWriteAddress.interfaceId,
                                                       oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.interfaceType,
                                                       &oobPortEntry.phyInterfaceId);
            PDL_CHECK_STATUS(pdlStatus);
        }
        /* we assume OOB port is connected using PHY. information is stored either in EZB xml or MPD xml */
        else {
            oobPortEntry.oobInfo.phyConfig.smiXmsiInterface.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_MPD_E;
            if (pdlIsFieldHasValue (oobParams.logicalPort_mask)) {
                pdlStatus = prvPdlMpdSmiXsmiInterfaceRegister(oobParams.logicalPort,
                                                              &oobPortEntry.phyInterfaceId);
                PDL_CHECK_STATUS(pdlStatus);
            }
        }

        pdlStatus = prvPdlibDbAdd(prvPdlOobPortDb, &oobPortEntry.key, &oobPortEntry, (void **)&oobPortEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlOobDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlOobDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = prvPdlibDbDestroy(prvPdlOobPortDb);
    return pdlStatus;
}
/*$ END OF prvPdlOobDestroy */
