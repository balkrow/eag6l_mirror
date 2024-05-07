/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlPacketProcessor.c
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
 * @brief Platform driver layer - Packet Processor related API
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
#include <pdl/parser/pdlParser.h>
#include <pdl/common/pdlTypes.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdl/packet_processor/private/prvPdlPacketProcessor.h>
#include <pdl/sfp/private/prvPdlSfp.h>
#include <pdl/serdes/private/prvPdlSerdes.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/interface/pdlSmiXsmiDebug.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   The pdl pp b 2b db[ prv pdl pp b 2 b maximum num] */
static PDL_PP_B2B_ATTRIBUTES_STC    pdlPpB2bDb[PRV_PDL_PP_B2B_MAX_NUM];

/** @brief   The prv pdl pp device database */
static PRV_PDLIB_DB_TYP               prvPdlPpDevDb;

/**************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlPpDebugFlag)

/**
 * @fn  PDL_STATUS prvPdlPpDbFrontPanelSpeedConvert ( IN  PDL_SPEED_TYPE_ENT yangPortSpeed, OUT PDL_PORT_SPEED_ENT  *userPortSpeedPtr )
 *
 * @brief   Convert from yang to user port speed representation
 *
 * @param [in]  yangPortSpeed           yang port speed.
 * @param [out] userPortSpeedPtr        user defined port speed.
 *
 * @return  PDL_BAD_PARAM  impossible conversion encountered.
 */

static PDL_STATUS prvPdlPpDbFrontPanelSpeedConvert(
    IN  PDL_SPEED_TYPE_ENT   yangPortSpeed,
    OUT PDL_PORT_SPEED_ENT  *userPortSpeedPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    *userPortSpeedPtr = (PDL_PORT_SPEED_ENT)(yangPortSpeed-1);
    return PDL_OK;
}
/*$ END OF prvPdlPpDbFrontPanelSpeedConvert */

/**
 * @fn  PDL_STATUS prvPdlPpDbFrontPanelSpeedBackConvert ( IN  PDL_PORT_SPEED_ENT  userPortSpeed, OUT PDL_SPEED_TYPE_ENT * yangPortSpeedPtr )
 *
 * @brief   Convert from user to yang port speed representation
 *
 * @param [in]  userPortSpeed           user defined port speed.
 * @param [out] yangPortSpeedPtr        yang port speed.
 *
 * @return  PDL_BAD_PARAM  impossible conversion encountered.
 */

static PDL_STATUS prvPdlPpDbFrontPanelSpeedBackConvert(
    IN  PDL_PORT_SPEED_ENT   userPortSpeed,
    OUT PDL_SPEED_TYPE_ENT * yangPortSpeedPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    *yangPortSpeedPtr = (PDL_SPEED_TYPE_ENT)(userPortSpeed+1);
    return PDL_OK;
}
/*$ END OF prvPdlPpDbFrontPanelSpeedConvert */

/**
 * @fn  PDL_STATUS prvPdlPpDbFrontPanelIfModeConvert ( IN  PDL_L1_INTERFACE_MODE_TYPE_ENT  yangIfMode, OUT PDL_INTERFACE_MODE_ENT  *userIfModePtr )
 *
 * @brief   Convert from user to yang interface mode speed representation
 *
 * @param [in]  yangIfMode           yang interface mode.
 * @param [out] userIfModePtr        user defined interface mode.
 *
 * @return  PDL_BAD_PARAM  impossible conversion encountered.
 */

static PDL_STATUS prvPdlPpDbFrontPanelIfModeConvert(
    IN  PDL_L1_INTERFACE_MODE_TYPE_ENT  yangIfMode,
    OUT PDL_INTERFACE_MODE_ENT         *userIfModePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    *userIfModePtr = (PDL_INTERFACE_MODE_ENT)(yangIfMode-1);
    return PDL_OK;
}
/*$ END OF prvPdlPpDbFrontPanelIfModeConvert */

/**
 * @fn  PDL_STATUS prvPdlPpDbFrontPanelIfModeBackConvert ( IN  PDL_INTERFACE_MODE_ENT  userIfMode, OUT PDL_L1_INTERFACE_MODE_TYPE_ENT * yangIfModePtr )
 *
 * @brief   Convert from user to yang interface mode speed representation
 *
 * @param [in]  userIfMode              user defined interface mode.
 * @param [out] yangIfModePtr           yang interface mode.
 *
 * @return  PDL_BAD_PARAM  impossible conversion encountered.
 */

static PDL_STATUS prvPdlPpDbFrontPanelIfModeBackConvert(
    IN  PDL_INTERFACE_MODE_ENT          userIfMode,
    OUT PDL_L1_INTERFACE_MODE_TYPE_ENT *yangIfModePtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    *yangIfModePtr = (PDL_L1_INTERFACE_MODE_TYPE_ENT)(userIfMode+1);
    return PDL_OK;
}
/*$ END OF prvPdlPpDbFrontPanelIfModeBackConvert */

/**
 * @fn  PDL_STATUS pdlPpDbAttributesGet ( OUT PDL_PP_ATTRIBUTES_STC * ppAttributesPtr )
 *
 * @brief   Get packet processor attributes
 *
 * @param [out] ppAttributesPtr packet processor attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbAttributesGet(
    OUT  PDL_PP_ATTRIBUTES_STC         * ppAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                      pdlStatus;
    PDL_PACKET_PROCESSORS_PARAMS_STC                ppParams;
    PDL_NETWORK_PORTS_PARAMS_STC                    fpParams;
    PDL_BACK_TO_BACK_INFORMATION_PARAMS_STC         b2bParams;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (ppAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "ppAttributesPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_PACKET_PROCESSORS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&ppParams);
    PDL_CHECK_STATUS(pdlStatus);

    ppAttributesPtr->numOfPps = ppParams.numberOfPps;

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&fpParams);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetNumOfEntries(fpParams.frontPanelGroupList_PTR, &ppAttributesPtr->numOfFrontPanelGroups);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_BACK_TO_BACK_INFORMATION_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&b2bParams);
    if (pdlStatus == PDL_OK) {
        if (b2bParams.type == PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_CURRENT_E) {
            pdlStatus = prvPdlibDbGetNumOfEntries(b2bParams.data.backToBackCurrent.backToBackLinkInformation.backToBackPortList_PTR, &ppAttributesPtr->numOfBackToBackLinksPerPp);
        }
        else if (b2bParams.type ==  PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_LEGACY_V1_E) {
            pdlStatus = prvPdlibDbGetNumOfEntries(b2bParams.data.backToBackLegacyV1.backToBackLinks.backToBackLinkList_PTR, &ppAttributesPtr->numOfBackToBackLinksPerPp);
        }
        else {
            pdlStatus = PDL_NOT_SUPPORTED;
        }
    }
    else {
        ppAttributesPtr->numOfBackToBackLinksPerPp = 0;
    }

    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, ppAttributesPtr->numOfPps);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, ppAttributesPtr->numOfFrontPanelGroups);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, ppAttributesPtr->numOfBackToBackLinksPerPp);
    return PDL_OK;
}
/*$ END OF pdlPpDbAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesGet ( IN UINT_32 b2bLinkId, OUT PDL_PP_B2B_ATTRIBUTES_STC * b2bAttributesPtr )
 *
 * @brief   Get PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [out] b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_BAD_PARAM  b2b id is out of range.
 */

PDL_STATUS pdlPpDbB2bAttributesGet(
    IN   UINT_32                                 b2bLinkId,
    OUT  PDL_PP_B2B_ATTRIBUTES_STC             * b2bAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus;
    PDL_PP_ATTRIBUTES_STC                   ppAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (b2bAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "b2bAttributesPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
    PDL_CHECK_STATUS(pdlStatus);

    if (b2bLinkId >= ppAttributes.numOfBackToBackLinksPerPp) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL VALUE TO b2bLinkId(VALUE: %d)", b2bLinkId);
        return PDL_BAD_PARAM;
    }

    memcpy(b2bAttributesPtr, &pdlPpB2bDb[b2bLinkId], sizeof(PDL_PP_B2B_ATTRIBUTES_STC));
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->firstDev);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->firstPort);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->secondDev);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->secondPort);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->maxSpeed);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, b2bAttributesPtr->interfaceMode);
    return PDL_OK;
}
/*$ END OF pdlPpDbB2bAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbIsB2bLink ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT BOOLEAN * isB2bLinkPtr )
 *
 * @brief   Determines whether PP port is back-to-back link.
 *
 * @param [in]  dev             dev number.
 * @param [in]  logicalPort     port number.
 * @param [out] isB2bLinkPtr    is b2b link.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  no such port was found.
 */
PDL_STATUS pdlPpDbIsB2bLink(
    IN   UINT_32            dev,
    IN   UINT_32            logicalPort,
    OUT  BOOLEAN          * isB2bLinkPtr
)
{
    /*****************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*****************************************************************************/
    UINT_32                                 b2bLink;
    PDL_STATUS                              pdlStatus;
    PDL_PP_ATTRIBUTES_STC                   ppAttributes;
    PDL_PP_B2B_ATTRIBUTES_STC               b2bAttributes;
    PDL_PP_NETWORK_PORT_ATTRIBUTES_STC      portAttributes;
    /*****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /*****************************************************************************/

    if (isB2bLinkPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "isB2bLinkPtr NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
    if (PDL_OK != pdlStatus) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "can't get pp attributes");
        return pdlStatus;
    }

    if (ppAttributes.numOfBackToBackLinksPerPp > 0) {
        for (b2bLink = 0; b2bLink < ppAttributes.numOfBackToBackLinksPerPp; b2bLink ++) {
            pdlStatus = pdlPpDbB2bAttributesGet(b2bLink, &b2bAttributes);
            if (PDL_OK != pdlStatus) {
                continue;
            }
            if (b2bAttributes.firstDev == dev && b2bAttributes.firstPort == logicalPort) {
                *isB2bLinkPtr = TRUE;
                return PDL_OK;
            }
            if (b2bAttributes.secondDev == dev && b2bAttributes.secondPort == logicalPort) {
                *isB2bLinkPtr = TRUE;
                return PDL_OK;
            }
        }

        /* not b2b link - validate port exists */
        if (pdlPpDbPortAttributesGet(dev, logicalPort, &portAttributes) == PDL_OK) {
            *isB2bLinkPtr = FALSE;
            return PDL_OK;
        }
    }

    return PDL_NOT_FOUND;
}
/*$ END OF pdlPpDbIsB2bLink */

/**
 * @fn  PDL_STATUS pdlPpDbPortAttributesGet ( IN UINT_32 dev, IN UINT_32 logicalPort, OUT PDL_PP_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Get PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  port                logicalPort number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbPortAttributesGet(
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  PDL_PP_NETWORK_PORT_ATTRIBUTES_STC       * portAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                                 i;
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC                  *frontPanelGroupPtr;
    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;
    PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC                  *l1IfEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portAttributesPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelGroupPtr = (PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC *)portEntryPtr->frontPanelEntryPtr;
    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    portAttributesPtr->frontPanelNumber = frontPanelGroupPtr->list_keys.groupNumber;
    portAttributesPtr->portNumberInGroup = frontPanelPortEntryPtr->list_keys.portNumber;

    i = 0;
    pdlStatus = prvPdlibDbGetFirst(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, (void **)&l1IfEntryPtr);
    while (pdlStatus == PDL_OK) {
        prvPdlPpDbFrontPanelSpeedConvert(l1IfEntryPtr->list_keys.speed, &portAttributesPtr->copperModesArr[i].speed);
        prvPdlPpDbFrontPanelIfModeConvert(l1IfEntryPtr->mode, &portAttributesPtr->copperModesArr[i].mode);

        i++;
        pdlStatus = prvPdlibDbGetNext(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, (void *)&l1IfEntryPtr->list_keys, (void **)&l1IfEntryPtr);
    }
    portAttributesPtr->numOfCopperModes = i;

    i = 0;
    pdlStatus = prvPdlibDbGetFirst(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, (void **)&l1IfEntryPtr);
    while (pdlStatus == PDL_OK) {
        prvPdlPpDbFrontPanelSpeedConvert(l1IfEntryPtr->list_keys.speed, &portAttributesPtr->fiberModesArr[i].speed);
        prvPdlPpDbFrontPanelIfModeConvert(l1IfEntryPtr->mode, &portAttributesPtr->fiberModesArr[i].mode);

        i++;
        pdlStatus = prvPdlibDbGetNext(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, (void *)&l1IfEntryPtr->list_keys, (void **)&l1IfEntryPtr);
    }
    portAttributesPtr->numOfFiberModes = i;

    portAttributesPtr->macPort = frontPanelPortEntryPtr->ppPortNumber;
    portAttributesPtr->transceiverType = frontPanelPortEntryPtr->transceiver;
    portAttributesPtr->isPhyExists = frontPanelPortEntryPtr->isPhyUsed;
    portAttributesPtr->swapAbcd = FALSE;
    if (pdlIsFieldHasValue(frontPanelPortEntryPtr->swapAbcd_mask)) {
        portAttributesPtr->swapAbcd = frontPanelPortEntryPtr->swapAbcd;
    }
    if ((portAttributesPtr->isPhyExists) && (pdlIsFieldHasValue(frontPanelPortEntryPtr->portPhy_mask))) {
        if (frontPanelPortEntryPtr->portPhy.phyLogicalPort_mask) {
            portAttributesPtr->phyData.phyNumber = 0;
            portAttributesPtr->phyData.phyPosition = 0;
            portAttributesPtr->phyData.smiXmsiInterface.phyInfo.mpdLogicalPort = frontPanelPortEntryPtr->portPhy.phyLogicalPort;
            portAttributesPtr->phyData.smiXmsiInterface.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_MPD_E;
        }
        else {
            portAttributesPtr->phyData.phyNumber = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyNumber;
            portAttributesPtr->phyData.phyPosition = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPosition;
            portAttributesPtr->phyData.smiXmsiInterface.phyInfo.readWriteAddress.address = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress;
            portAttributesPtr->phyData.smiXmsiInterface.phyInfo.readWriteAddress.dev = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice;
            portAttributesPtr->phyData.smiXmsiInterface.phyInfo.readWriteAddress.interfaceId = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId;
            if (frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType == PDL_SMI_XSMI_TYPE_TYPE_SMI_E) {
                portAttributesPtr->phyData.smiXmsiInterface.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_SMI_E;
            }
            else if (frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType == PDL_SMI_XSMI_TYPE_TYPE_XSMI_E) {
                portAttributesPtr->phyData.smiXmsiInterface.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_XSMI_E;
            }
            else {
                return PDL_BAD_STATE;
            }
        }
    }

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhySetPostInitValues ()
 *
 * @brief   write port phy post init values
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         logicalPort number.
 *
 * @return  PDL_OK              Registers configured ok or not required
 */

PDL_STATUS pdlPpDbPortPhySetPostInitValues(
    IN   UINT_32                                      dev,
    IN   UINT_32                                      logicalPort
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;

    PDL_PHY_POST_INIT_LIST_PARAMS_STC                      *phyPostInitValueInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    if (frontPanelPortEntryPtr->isPhyUsed) {
        pdlStatus = prvPdlibDbGetFirst(frontPanelPortEntryPtr->portPhy.phyPostInitList_PTR, (void**) &phyPostInitValueInfoPtr);
        while (pdlStatus == PDL_OK) {
            pdlStatus = pdlSmiXsmiHwSetValue(portEntryPtr->phyInterfaceId,
                                             phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage,
                                             phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister,
                                             phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask,
                                             phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "devOrPage=[%d] regAddress=[0x%x] mask=0x%x] value=[0x%x] ", phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage,
                                                  phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister,
                                                  phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask,
                                                  phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue);
            pdlStatus = prvPdlibDbGetNext(frontPanelPortEntryPtr->portPhy.phyPostInitList_PTR, &phyPostInitValueInfoPtr->list_keys, (void**)&phyPostInitValueInfoPtr);
        }
    }
    return PDL_OK;
}
/*$ END OF pdlPpDbPortPhySetPostInitValues */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyPostInitValuesGetFirst ()
 *
 * @brief   Get first port phy post init values
 *
 * @param [in]  dev              dev number.
 * @param [in]  logicalPort      logicalPort number.
 * @param [out] initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyPostInitValuesGetFirst(
    IN  UINT_32                                     dev,
    IN  UINT_32                                     logicalPort,
    OUT PDL_PP_NETWORK_PORT_PHY_INIT_INFO_STC     * initValuesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;

    PDL_PHY_POST_INIT_LIST_PARAMS_STC                      *phyPostInitValueInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (initValuesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    if (frontPanelPortEntryPtr->isPhyUsed == FALSE) {
        return PDL_BAD_PARAM;
    }

    pdlStatus = prvPdlibDbGetFirst(frontPanelPortEntryPtr->portPhy.phyPostInitList_PTR, (void**) &phyPostInitValueInfoPtr);
    if (pdlStatus == PDL_OK) {
        initValuesPtr->key.index = phyPostInitValueInfoPtr->list_keys.phyInitIndex;
        initValuesPtr->info.devOrPage = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage;
        initValuesPtr->info.registerAddress = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister;
        initValuesPtr->info.mask = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask;
        initValuesPtr->info.value = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue;
    }
    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyPostInitValuesGetFirst */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyPostInitValuesGetNext ()
 *
 * @brief   Get next port phy post init values
 *
 * @param [in]    dev              dev number.
 * @param [in]    logicalPort      logicalPort number.
 * @param [inout] initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyPostInitValuesGetNext(
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    INOUT PDL_PP_NETWORK_PORT_PHY_INIT_INFO_STC     * initValuesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;

    PDL_PHY_POST_INIT_LIST_KEYS_STC                         phyPostInitValueInfoKey;
    PDL_PHY_POST_INIT_LIST_PARAMS_STC                      *phyPostInitValueInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (initValuesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    if (frontPanelPortEntryPtr->isPhyUsed == FALSE) {
        return PDL_BAD_PARAM;
    }

    phyPostInitValueInfoKey.phyInitIndex = initValuesPtr->key.index;
    pdlStatus = prvPdlibDbGetNext(frontPanelPortEntryPtr->portPhy.phyPostInitList_PTR, &phyPostInitValueInfoKey, (void**) &phyPostInitValueInfoPtr);
    if (pdlStatus == PDL_OK) {
        initValuesPtr->key.index = phyPostInitValueInfoPtr->list_keys.phyInitIndex;
        initValuesPtr->info.devOrPage = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage;
        initValuesPtr->info.registerAddress = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister;
        initValuesPtr->info.mask = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask;
        initValuesPtr->info.value = phyPostInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue;
    }
    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyPostInitValuesGetNext */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhySetInitValues ()
 *
 * @brief   write port phy init values for given speed & transceiver type
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         logicalPort number.
 * @param [in]  speed               port speed
 * @param [in]  transceiverType     transceiverType (copper/fiber)
 *
 * @return  PDL_BAD_PARAM       illegal transceiverType
            PDL_NO_SUCH         phy_init configuration is required but not for this speed & transceiverType
            PDL_NOT_SUPPORTED   phy_init configuration isn't required
            PDL_OK              phy_init configured ok
 */

PDL_STATUS pdlPpDbPortPhySetInitValues(
    IN   UINT_32                                      dev,
    IN   UINT_32                                      logicalPort,
    IN   PDL_PORT_SPEED_ENT                           speed,
    IN   PDL_TRANSCEIVER_TYPE_ENT                     transceiverType
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN                                     phyInitRequired = FALSE;
    UINT_32                                     numOfEntries;
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                     portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                   *frontPanelPortEntryPtr;

    PDL_L1_INTERFACE_MODE_LIST_KEYS_STC         l1IfModeEntryKey;
    PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC     * l1IfModeEntryPtr;

    PDL_PHY_INIT_LIST_PARAMS_STC              * phyInitValueInfoPtr;
    PDL_SPEED_TYPE_ENT 							speedUtil;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    pdlStatus = prvPdlPpDbFrontPanelSpeedBackConvert(speed, &speedUtil);
    PDL_CHECK_STATUS(pdlStatus);
    l1IfModeEntryKey.speed = speedUtil;

    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E) {
        pdlStatus = prvPdlibDbFind(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryKey, (void**) &l1IfModeEntryPtr);
    }
    else  if (transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E) {
        pdlStatus = prvPdlibDbFind(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryKey, (void**) &l1IfModeEntryPtr);
    }
    else {
        return PDL_BAD_PARAM;
    }

    if (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbGetFirst(l1IfModeEntryPtr->phyInitList_PTR, (void**)&phyInitValueInfoPtr);
        while (pdlStatus == PDL_OK) {
            phyInitRequired = TRUE;
            pdlStatus = pdlSmiXsmiHwSetValue(portEntryPtr->phyInterfaceId,
                                             phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage,
                                             phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister,
                                             phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask,
                                             phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue);
            PDL_CHECK_STATUS(pdlStatus);
            PDL_LIB_DEBUG_MAC(prvPdlPhyDebugFlag)(__FUNCTION__, __LINE__, "(Copper) devOrPage=[%d] regAddress=[0x%x] mask=0x%x] value=[0x%x] ",
                                                  phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage,
                                                  phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister,
                                                  phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask,
                                                  phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue);
            pdlStatus = prvPdlibDbGetNext(l1IfModeEntryPtr->phyInitList_PTR, &phyInitValueInfoPtr->list_keys, (void**)&phyInitValueInfoPtr);
        }
    }
    /* check if any interface mode requires phy_init for this interface */
    if (phyInitRequired == TRUE) {
        return PDL_OK;
    }
    else {
        pdlStatus = prvPdlibDbGetFirst(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, (void**) &l1IfModeEntryPtr);
        while (pdlStatus == PDL_OK) {
            if (PDL_OK == prvPdlibDbGetNumOfEntries(l1IfModeEntryPtr->phyInitList_PTR, & numOfEntries) && numOfEntries != 0) {
                return PDL_NO_SUCH;
            }

            pdlStatus = prvPdlibDbGetNext(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryPtr->list_keys, (void**) &l1IfModeEntryPtr);
        }

        pdlStatus = prvPdlibDbGetFirst(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, (void**) &l1IfModeEntryPtr);
        while (pdlStatus == PDL_OK) {
            if (PDL_OK == prvPdlibDbGetNumOfEntries(l1IfModeEntryPtr->phyInitList_PTR, & numOfEntries) && numOfEntries != 0) {
                return PDL_NO_SUCH;
            }

            pdlStatus = prvPdlibDbGetNext(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryPtr->list_keys, (void**) &l1IfModeEntryPtr);
        }
    }

    return PDL_NOT_SUPPORTED;
}
/*$ END OF pdlPpDbPortPhySetInitValues */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyInitValuesGetFirst ()
 *
 * @brief   Get first port phy init values
 *
 * @param [in]     dev              dev number.
 * @param [in]     logicalPort      logicalPort number.
 * @param [in]     speed            port speed
 * @param [in]     transceiverType  transceiverType (copper/fiber)
 * @param [inout]  initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PARAM        illegal speed or transceiverType
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyInitValuesGetFirst(
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    IN    PDL_PORT_SPEED_ENT                          speed,
    IN    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType,
    INOUT PDL_PP_NETWORK_PORT_PHY_INIT_INFO_STC     * initValuesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                     portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                   *frontPanelPortEntryPtr;

    PDL_L1_INTERFACE_MODE_LIST_KEYS_STC         l1IfModeEntryKey;
    PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC     * l1IfModeEntryPtr;

    PDL_PHY_INIT_LIST_PARAMS_STC              * phyInitValueInfoPtr;
    PDL_SPEED_TYPE_ENT 							speedUtil;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (speed >= PDL_PORT_SPEED_LAST_E) {
        return PDL_BAD_PARAM;
    }

    if (initValuesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    pdlStatus = prvPdlPpDbFrontPanelSpeedBackConvert(speed, &speedUtil);
    PDL_CHECK_STATUS(pdlStatus);
    l1IfModeEntryKey.speed = speedUtil;
    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E) {
        pdlStatus = prvPdlibDbFind(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryKey, (void**) &l1IfModeEntryPtr);
    }
    else  if (transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E) {
        pdlStatus = prvPdlibDbFind(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryKey, (void**) &l1IfModeEntryPtr);
    }
    else {
        return PDL_BAD_PARAM;
    }

    if (pdlStatus == PDL_OK) {
        pdlStatus = prvPdlibDbGetFirst(l1IfModeEntryPtr->phyInitList_PTR, (void**)&phyInitValueInfoPtr);
    }

    if (pdlStatus == PDL_OK) {
        initValuesPtr->key.index = phyInitValueInfoPtr->list_keys.phyInitIndex;
        initValuesPtr->info.devOrPage = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage;
        initValuesPtr->info.registerAddress = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister;
        initValuesPtr->info.mask = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask;
        initValuesPtr->info.value = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue;
    }

    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyInitValuesGetFirst */

/**
 * @fn  PDL_STATUS pdlPpDbPortPhyInitValuesGetNext ()
 *
 * @brief   Get next port phy init values
 *
 * @param [in]     dev              dev number.
 * @param [in]     logicalPort      logicalPort number.
 * @param [in]     speed            port speed
 * @param [in]     transceiverType  transceiverType (copper/fiber)
 * @param [inout]  initValuesPtr    ptr to data.
 *
 * @return  PDL_BAD_PARAM        illegal speed or transceiverType
 * @return  PDL_BAD_PTR          if initValuesPtr is NULL
 * @return  PDL_NO_MORE          No more data available
 */
PDL_STATUS pdlPpDbPortPhyInitValuesGetNext(
    IN    UINT_32                                     dev,
    IN    UINT_32                                     logicalPort,
    IN    PDL_PORT_SPEED_ENT                          speed,
    IN    PDL_TRANSCEIVER_TYPE_ENT                    transceiverType,
    INOUT PDL_PP_NETWORK_PORT_PHY_INIT_INFO_STC     * initValuesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                     portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                   *frontPanelPortEntryPtr;

    PDL_L1_INTERFACE_MODE_LIST_KEYS_STC         l1IfModeEntryKey;
    PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC     * l1IfModeEntryPtr;

    PDL_PHY_INIT_LIST_KEYS_STC                  phyInitValueInfoKey;
    PDL_PHY_INIT_LIST_PARAMS_STC              * phyInitValueInfoPtr;
    PDL_SPEED_TYPE_ENT 							speedUtil;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (speed >= PDL_PORT_SPEED_LAST_E) {
        return PDL_BAD_PARAM;
    }

    if (initValuesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    pdlStatus = prvPdlPpDbFrontPanelSpeedBackConvert(speed, &speedUtil);
    PDL_CHECK_STATUS(pdlStatus);
    l1IfModeEntryKey.speed = speedUtil;
    if (transceiverType == PDL_TRANSCEIVER_TYPE_COPPER_E) {
        pdlStatus = prvPdlibDbFind(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryKey, (void**) &l1IfModeEntryPtr);
    }
    else  if (transceiverType == PDL_TRANSCEIVER_TYPE_FIBER_E) {
        pdlStatus = prvPdlibDbFind(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR, &l1IfModeEntryKey, (void**) &l1IfModeEntryPtr);
    }
    else {
        return PDL_BAD_PARAM;
    }

    if (pdlStatus == PDL_OK) {
        phyInitValueInfoKey.phyInitIndex = initValuesPtr->key.index;
        pdlStatus = prvPdlibDbGetNext(l1IfModeEntryPtr->phyInitList_PTR, &phyInitValueInfoKey, (void**)&phyInitValueInfoPtr);
    }

    if (pdlStatus == PDL_OK) {
        initValuesPtr->key.index = phyInitValueInfoPtr->list_keys.phyInitIndex;
        initValuesPtr->info.devOrPage = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyDeviceOrPage;
        initValuesPtr->info.registerAddress = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyRegister;
        initValuesPtr->info.mask = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyMask;
        initValuesPtr->info.value = phyInitValueInfoPtr->phyMaskValueGroupType.phyMaskValue.phyValue;
    }

    return pdlStatus;
}
/*$ END OF pdlPpDbPortPhyInitValuesGetNext */

/**
* @public pdlPpDbFirstPortAttributesGet
*
* @brief  Get first existing dev/port attributes.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[out] firstDevPtr                first dev number
* @param[out] firstPortPtr               first port number
*/
PDL_STATUS pdlPpDbFirstPortAttributesGet(
    OUT  UINT_32                                   *firstDevPtr,
    OUT  UINT_32                                   *firstPortPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (firstDevPtr == NULL || firstPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = prvPdlibDbGetFirst(prvPdlPpDevDb, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetFirst(devEntryPtr->logicalPortsList, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *firstDevPtr = devEntryPtr->key.dev;
    *firstPortPtr = portEntryPtr->key.port;

    return PDL_OK;
}
/*$ END OF pdlPpDbFirstPortAttributesGet */

/**
* @public pdlPpDbPortAttributesGetNext
*
* @brief  Get next existing dev/port.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in] currDev                    current dev number
* @param[in] currPort                   current port number
* @param[out] nextDevPtr                next dev number
* @param[out] nextPortPtr               next port number
*/
PDL_STATUS pdlPpDbPortAttributesGetNext(
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  UINT_32                                  * nextDevPtr,
    OUT  UINT_32                                  * nextPortPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                     portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (nextDevPtr == NULL || nextPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbGetNext(devEntryPtr->logicalPortsList, &portKey, (void**) &portEntryPtr);
    if (pdlStatus != PDL_OK) {
        pdlStatus = prvPdlibDbGetNext(prvPdlPpDevDb, &devEntryPtr->key, (void**) &devEntryPtr);
        if (pdlStatus == PDL_NO_MORE) {
            return pdlStatus;
        }
        PDL_CHECK_STATUS(pdlStatus);
        pdlStatus = prvPdlibDbGetFirst(devEntryPtr->logicalPortsList, (void**) &portEntryPtr);
    }
    if (pdlStatus == PDL_NO_MORE) {
        return pdlStatus;
    }
    PDL_CHECK_STATUS(pdlStatus);

    *nextDevPtr = devEntryPtr->key.dev;
    *nextPortPtr = portEntryPtr->key.port;

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGetNext */

/**
* @public pdlPpDbFirstPortAttributesGet
*
* @brief  Get first existing front panel number.
*
* @param[out] pdlPpDbFirstFrontPanelGet  first front panel number
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*/
PDL_STATUS pdlPpDbFirstFrontPanelGet(
    OUT  UINT_32                *firstFrontPanelPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                      pdlStatus;
    PDL_NETWORK_PORTS_PARAMS_STC                    portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC          *frontPanelEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (firstFrontPanelPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    pdlStatus = prvPdlibDbGetFirst(portsParams.frontPanelGroupList_PTR, (void**)&frontPanelEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *firstFrontPanelPtr = frontPanelEntryPtr->list_keys.groupNumber;

    return PDL_OK;
}
/*$ END OF pdlPpDbFirstFrontPanelGet */

/**
* @public pdlPpDbFrontPanelGetNext
*
* @brief  Get next existing front panel
*
* @param[in] frontPanelNumber          current front panel number
* @param[out] nextFrontPanelNumberPtr  next front panel number
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*/
PDL_STATUS pdlPpDbFrontPanelGetNext(
    IN   UINT_32                frontPanelNumber,
    OUT  UINT_32                *nextFrontPanelNumberPtr
)
{
    /*****************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                      pdlStatus;
    PDL_NETWORK_PORTS_PARAMS_STC                    portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_KEYS_STC             frontPanelEntryKey;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC          *frontPanelEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (nextFrontPanelNumberPtr == NULL) {
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    frontPanelEntryKey.groupNumber = frontPanelNumber;

    pdlStatus = prvPdlibDbGetNext(portsParams.frontPanelGroupList_PTR, (void *)&frontPanelEntryKey, (void**)&frontPanelEntryPtr);
    if (pdlStatus == PDL_NO_MORE) {
        return pdlStatus;
    }
    PDL_CHECK_STATUS(pdlStatus);

    *nextFrontPanelNumberPtr = frontPanelEntryPtr->list_keys.groupNumber;

    return PDL_OK;
}
/*$ END OF pdlPpDbFrontPanelGetNext */

/**
* @public pdlPpDbDevAttributesGetFirstPort
*
* @brief  Get first existing port of device.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in]  dev                dev number
* @param[out] firstPortPtr       first port number
*/
PDL_STATUS pdlPpDbDevAttributesGetFirstPort(
    IN   UINT_32       dev,
    OUT  UINT_32       *firstPortPtr
)
{
    /****************************************************************************/
    /*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /****************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    if (firstPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, &devKey, (void**) &devEntryPtr);
    if (pdlStatus == PDL_NO_MORE) {
        return pdlStatus;
    }
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetFirst(devEntryPtr->logicalPortsList, (void**) &portEntryPtr);
    if (pdlStatus == PDL_NO_MORE) {
        return pdlStatus;
    }
    PDL_CHECK_STATUS(pdlStatus);

    *firstPortPtr = portEntryPtr->key.port;

    return PDL_OK;
}
/*$ END OF pdlPpDbDevAttributesGetFirstPort */

/**
* @public pdlPpDbDevAttributesGetNextPort
*
* @brief  Get next existing port of device.
*
* @returns PDL_BAD_PTR  illegal pointer supplied
*
* @param[in]  dev                dev number
* @param[in]  currentPort        current port number
* @param[out] nextPortPtr        next port number
*/
PDL_STATUS pdlPpDbDevAttributesGetNextPort(
    IN   UINT_32       dev,
    IN   UINT_32       currentPort,
    OUT  UINT_32       *nextPortPtr
)
{
    /****************************************************************************/
    /*L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /****************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                     portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                  *portEntryPtr;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    if (nextPortPtr == NULL) {
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, &devKey, (void**) &devEntryPtr);
    if (pdlStatus == PDL_NO_MORE) {
        return pdlStatus;
    }
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = currentPort;
    pdlStatus = prvPdlibDbGetNext(devEntryPtr->logicalPortsList, &portKey, (void**) &portEntryPtr);
    if (pdlStatus == PDL_NO_MORE) {
        return pdlStatus;
    }
    PDL_CHECK_STATUS(pdlStatus);

    *nextPortPtr = portEntryPtr->key.port;

    return PDL_OK;
}
/*$ END OF pdlPpDbDevAttributesGetNextPort */

/**
 * @fn  PDL_STATUS pdlPpDbDevAttributesGet ( IN UINT_32 dev, OUT UINT_32 * numOfPortsPtr )
 *
 * @brief   Get number of pp port
 *
 * @param [in]  dev             dev number.
 * @param [out] numOfPortsPtr   device's number of ports.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlPpDbDevAttributesGet(
    IN   UINT_32                                    dev,
    OUT  UINT_32                                  * numOfPortsPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                   devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                *devEntryPtr;
    UINT_32                                     count = 0;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (numOfPortsPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "numOfPortsPtr NULL");
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetNumOfEntries(devEntryPtr->logicalPortsList, &count);
    if (pdlStatus == PDL_OK) {
        *numOfPortsPtr = count;
    }
    else {
        *numOfPortsPtr = 0;
    }

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGet */

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesGet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr )
 *
 * @brief   Get attributes for front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR  illegal pointer supplied.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesGet(
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_FRONT_PANEL_ATTRIBUTES_STC     * frontPanelAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                      pdlStatus;
    PDL_NETWORK_PORTS_PARAMS_STC                    portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_KEYS_STC             fpEntryKey;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC          *fpEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (frontPanelAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "frontPanelAttributesPtr NULL, FRONT PANEL NUMBER: %d", frontPanelNumber);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    fpEntryKey.groupNumber = frontPanelNumber;

    pdlStatus = prvPdlibDbFind(portsParams.frontPanelGroupList_PTR, (void *)&fpEntryKey, (void**)&fpEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    frontPanelAttributesPtr->firstPortNumberShift = fpEntryPtr->firstPortNumberShift;
    frontPanelAttributesPtr->isLogicalPortRequired = fpEntryPtr->isLogicalPortRequired;
    frontPanelAttributesPtr->ordering = fpEntryPtr->groupOrdering;
    pdlStatus = prvPdlPpDbFrontPanelSpeedConvert(fpEntryPtr->speed, &frontPanelAttributesPtr->portMaximumSpeed);
    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibDbGetNumOfEntries(fpEntryPtr->portList_PTR, &frontPanelAttributesPtr->numOfPortsInGroup);
    PDL_CHECK_STATUS(pdlStatus);

    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, frontPanelAttributesPtr->ordering);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, frontPanelAttributesPtr->portMaximumSpeed);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, frontPanelAttributesPtr->firstPortNumberShift);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(UINT_32, frontPanelAttributesPtr->numOfPortsInGroup);
    IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(BOOLEAN, frontPanelAttributesPtr->isLogicalPortRequired);

    return PDL_OK;
}
/*$ END OF pdlPpDbPortAttributesGet */

/**
 * @fn  PDL_STATUS pdlPacketProcessorInit ( void )
 *
 * @brief   Init Packet Processor module Create DB and initialize
 *
 * @return  PDL_STATUS
 */

PDL_STATUS pdlPacketProcessorInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PRV_PDLIB_DB_ATTRIBUTES_STC                     dbAttributes;
    PRV_PDL_DEVICE_DB_KEY_STC                       devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                     devEntry, *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                         portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                       portEntry, *portEntryPtr;
    PDL_STATUS                                      pdlStatus, pdlStatus2;
    UINT_32                                         count;
    PDL_BACK_TO_BACK_INFORMATION_PARAMS_STC         b2bParams;
    PDL_BACK_TO_BACK_LINK_LIST_PARAMS_STC          *b2bLegacyEntryPtr = NULL;
    PDL_BACK_TO_BACK_PORT_LIST_PARAMS_STC          *b2bCurrentEntryPtr;
    PDL_NETWORK_PORTS_PARAMS_STC                    portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC          *frontPanelGroupPtr;
    PDL_PORT_LIST_PARAMS_STC                       *frontPanelPortEntryPtr;
    PDL_INTERFACE_SMI_XSMI_STC                      smiPublicInfo;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_DEVICE_DB_KEY_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_DEVICE_DB_ENTRY_STC);

    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &prvPdlPpDevDb);
    PDL_CHECK_STATUS(pdlStatus);

    /* need to initialize Sfp module for sfp parsing */
    pdlStatus = pdlSfpInit();
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_BACK_TO_BACK_INFORMATION_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&b2bParams);
    if (pdlStatus == PDL_OK) {
        if (b2bParams.type == PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_CURRENT_E) {
            pdlStatus = prvPdlibDbGetFirst(b2bParams.data.backToBackCurrent.backToBackLinkInformation.backToBackPortList_PTR, (void **)&b2bCurrentEntryPtr);
        }
        else if (b2bParams.type ==  PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_LEGACY_V1_E) {
            pdlStatus = prvPdlibDbGetFirst(b2bParams.data.backToBackLegacyV1.backToBackLinks.backToBackLinkList_PTR, (void **)&b2bLegacyEntryPtr);
        }
        else {
            return PDL_NOT_SUPPORTED;
        }
        PDL_CHECK_STATUS(pdlStatus);
        count = 0;
        while (pdlStatus == PDL_OK) {
            if (count == PRV_PDL_PP_B2B_MAX_NUM) {
                break;
            }

            if (b2bParams.type == PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_CURRENT_E) {
                if (b2bCurrentEntryPtr) {
                    pdlPpB2bDb[count].firstDev =  b2bParams.data.backToBackCurrent.backToBackLinkInformation.leftPpDeviceNumber;
                    pdlPpB2bDb[count].secondDev = b2bParams.data.backToBackCurrent.backToBackLinkInformation.rightPpDeviceNumber;
                    if (pdlIsFieldHasValue(b2bCurrentEntryPtr->leftLogicalPortNumber_mask)) {
                        pdlPpB2bDb[count].firstPort = b2bCurrentEntryPtr->leftLogicalPortNumber;
                    }
                    else {
                        pdlPpB2bDb[count].firstPort = b2bCurrentEntryPtr->list_keys.leftPpPortNumber;
                    }
                    if (pdlIsFieldHasValue(b2bCurrentEntryPtr->rightLogicalPortNumber_mask)) {
                        pdlPpB2bDb[count].secondPort = b2bCurrentEntryPtr->rightLogicalPortNumber;
                    }
                    else {
                        pdlPpB2bDb[count].secondPort = b2bCurrentEntryPtr->rightPpPortNumber;
                    }
                    pdlPpB2bDb[count].firstMacPort = b2bCurrentEntryPtr->list_keys.leftPpPortNumber;
                    pdlPpB2bDb[count].secondMacPort = b2bCurrentEntryPtr->rightPpPortNumber;
                    prvPdlPpDbFrontPanelIfModeConvert(b2bCurrentEntryPtr->mode, &pdlPpB2bDb[count].interfaceMode);
                    prvPdlPpDbFrontPanelSpeedConvert(b2bCurrentEntryPtr->speed, &pdlPpB2bDb[count].maxSpeed);
                    pdlStatus = prvPdlibDbGetNext(b2bParams.data.backToBackCurrent.backToBackLinkInformation.backToBackPortList_PTR, (void *)&b2bCurrentEntryPtr->list_keys, (void **)&b2bCurrentEntryPtr);
                }
                else {
                    return PDL_NOT_SUPPORTED;
                }
            }
            else {
                if (b2bLegacyEntryPtr) {
                    pdlPpB2bDb[count].firstDev =  b2bLegacyEntryPtr->list_keys.leftPpDeviceNumber;
                    pdlPpB2bDb[count].secondDev = b2bLegacyEntryPtr->rightPpDeviceNumber;
                    if (pdlIsFieldHasValue(b2bLegacyEntryPtr->leftLogicalPortNumber_mask)) {
                        pdlPpB2bDb[count].firstPort = b2bLegacyEntryPtr->leftLogicalPortNumber;
                    }
                    else {
                        pdlPpB2bDb[count].firstPort = b2bLegacyEntryPtr->list_keys.leftPpPortNumber;
                    }
                    if (pdlIsFieldHasValue(b2bLegacyEntryPtr->rightLogicalPortNumber_mask)) {
                        pdlPpB2bDb[count].secondPort = b2bLegacyEntryPtr->rightLogicalPortNumber;
                    }
                    else {
                        pdlPpB2bDb[count].secondPort = b2bLegacyEntryPtr->rightPpPortNumber;
                    }
                    pdlPpB2bDb[count].firstMacPort = b2bLegacyEntryPtr->list_keys.leftPpPortNumber;
                    pdlPpB2bDb[count].secondMacPort = b2bLegacyEntryPtr->rightPpPortNumber;
                    prvPdlPpDbFrontPanelIfModeConvert(b2bLegacyEntryPtr->mode, &pdlPpB2bDb[count].interfaceMode);
                    prvPdlPpDbFrontPanelSpeedConvert(b2bLegacyEntryPtr->speed, &pdlPpB2bDb[count].maxSpeed);
                    pdlStatus = prvPdlibDbGetNext(b2bParams.data.backToBackLegacyV1.backToBackLinks.backToBackLinkList_PTR, (void *)&b2bLegacyEntryPtr->list_keys, (void **)&b2bLegacyEntryPtr);
                }
                else {
                    return PDL_NOT_SUPPORTED;
                }
            }

            count ++;
        }
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    /* create dev/port mapping db */
    pdlStatus = prvPdlibDbGetFirst(portsParams.frontPanelGroupList_PTR, (void **)&frontPanelGroupPtr);
    while (pdlStatus == PDL_OK) {
        pdlStatus2 = prvPdlibDbGetFirst(frontPanelGroupPtr->portList_PTR, (void **)&frontPanelPortEntryPtr);
        while (pdlStatus2 == PDL_OK) {
            devKey.dev = frontPanelPortEntryPtr->ppDeviceNumber;
            if ((pdlStatus2 = prvPdlibDbFind(prvPdlPpDevDb, (void *)&devKey, (void **)&devEntryPtr)) != PDL_OK) {
                devEntry.key = devKey;

                dbAttributes.listAttributes.keySize = sizeof(PRV_PDL_PORT_DB_KEY_STC);
                dbAttributes.listAttributes.entrySize = sizeof(PRV_PDL_PORT_DB_ENTRY_STC);

                pdlStatus2 = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                              &dbAttributes,
                                              &devEntry.logicalPortsList);
                PDL_CHECK_STATUS(pdlStatus2);
                pdlStatus2 = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                              &dbAttributes,
                                              &devEntry.macPortsList);
                PDL_CHECK_STATUS(pdlStatus2);

                pdlStatus2 = prvPdlibDbAdd(prvPdlPpDevDb, (void *)&devKey, (void *)&devEntry, (void **)&devEntryPtr);
                PDL_CHECK_STATUS(pdlStatus2);
            }

            memset(&smiPublicInfo, 0, sizeof(smiPublicInfo));
            if ((frontPanelPortEntryPtr->isPhyUsed) && (pdlIsFieldHasValue(frontPanelPortEntryPtr->portPhy_mask))) {
                if (frontPanelPortEntryPtr->portPhy.phyLogicalPort_mask) {
                    pdlStatus = prvPdlMpdSmiXsmiInterfaceRegister(frontPanelPortEntryPtr->portPhy.phyLogicalPort,
                                                                  &portEntry.phyInterfaceId);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                else {
                    smiPublicInfo.phyInfo.readWriteAddress.address = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress;
                    smiPublicInfo.phyInfo.readWriteAddress.dev = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice;
                    smiPublicInfo.phyInfo.readWriteAddress.interfaceId = frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId;
                    if (frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType == PDL_SMI_XSMI_TYPE_TYPE_SMI_E) {
                        smiPublicInfo.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_SMI_E;
                    }
                    else if (frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiInterfaceType == PDL_SMI_XSMI_TYPE_TYPE_XSMI_E) {
                        smiPublicInfo.interfaceType = PDL_INTERFACE_EXTENDED_TYPE_XSMI_E;
                    }
                    else {
                        return PDL_BAD_STATE;
                    }
                    pdlStatus = prvPdlSmiXsmiInterfaceRegister(smiPublicInfo.phyInfo.readWriteAddress.dev,
                                                               smiPublicInfo.phyInfo.readWriteAddress.address,
                                                               smiPublicInfo.phyInfo.readWriteAddress.interfaceId,
                                                               smiPublicInfo.interfaceType,
                                                               &portEntry.phyInterfaceId);
                    PDL_CHECK_STATUS(pdlStatus);
                }
            }

            /* add logical port to list */
            if (pdlIsFieldHasValue(frontPanelPortEntryPtr->logicalPortNumber_mask)) {
                portKey.port = frontPanelPortEntryPtr->logicalPortNumber;
            }
            else {
                portKey.port = frontPanelPortEntryPtr->ppPortNumber;
            }
            portEntry.key = portKey;
            portEntry.frontPanelEntryPtr = frontPanelGroupPtr;
            portEntry.frontPanelPortEntryPtr = frontPanelPortEntryPtr;
            pdlStatus2 = prvPdlibDbAdd(devEntryPtr->logicalPortsList, (void *)&portKey, (void *)&portEntry, (void **)&portEntryPtr);
            PDL_CHECK_STATUS(pdlStatus2);

            /* add mac port to list */
            portKey.port = frontPanelPortEntryPtr->ppPortNumber;
            portEntry.key = portKey;
            portEntry.frontPanelEntryPtr = frontPanelGroupPtr;
            portEntry.frontPanelPortEntryPtr = frontPanelPortEntryPtr;
            pdlStatus2 = prvPdlibDbAdd(devEntryPtr->macPortsList, (void *)&portKey, (void *)&portEntry, (void **)&portEntryPtr);
            PDL_CHECK_STATUS(pdlStatus2);

            pdlStatus2 = prvPdlibDbGetNext(frontPanelGroupPtr->portList_PTR, (void *)&frontPanelPortEntryPtr->list_keys, (void **)&frontPanelPortEntryPtr);
        }

        pdlStatus = prvPdlibDbGetNext(portsParams.frontPanelGroupList_PTR, (void *)&frontPanelGroupPtr->list_keys, (void **)&frontPanelGroupPtr);
    }
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPpDbPortAttributesSet ( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PP_NETWORK_PORT_ATTRIBUTES_STC * portAttributesPtr )
 *
 * @brief   Update PP port attributes
 *
 * @param [in]  dev                 dev number.
 * @param [in]  logicalPort         logicalPort number.
 * @param [out] portAttributesPtr   port attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  port's entry wasn't found.
 */

PDL_STATUS pdlPpDbPortAttributesSet(
    IN   UINT_32                                    dev,
    IN   UINT_32                                    logicalPort,
    OUT  PDL_PP_NETWORK_PORT_ATTRIBUTES_STC       * portAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                                 i;

    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;

    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;

    PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC                  *l1IfEntryPtr, l1IfEntry;

    PRV_PDLIB_DB_ATTRIBUTES_STC                               dbAttributes;
    PDL_SPEED_TYPE_ENT 										speedUtil;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (portAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portAttributesPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    frontPanelPortEntryPtr->isPhyUsed = portAttributesPtr->isPhyExists;
    if (frontPanelPortEntryPtr->isPhyUsed) {
        frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyNumber = portAttributesPtr->phyData.phyNumber;
        frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPosition = portAttributesPtr->phyData.phyPosition;
        frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiAddress = portAttributesPtr->phyData.smiXmsiInterface.phyInfo.readWriteAddress.address;
        frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phyPpDevice = portAttributesPtr->phyData.smiXmsiInterface.phyInfo.readWriteAddress.dev;
        frontPanelPortEntryPtr->portPhy.phyReadWriteAddressGroupType.phyReadWriteAddress.phySmiXsmiInterfaceId = portAttributesPtr->phyData.smiXmsiInterface.phyInfo.readWriteAddress.interfaceId;
    }
    frontPanelPortEntryPtr->ppPortNumber = portAttributesPtr->macPort;
    frontPanelPortEntryPtr->transceiver = portAttributesPtr->transceiverType;

    pdlStatus = prvPdlibDbDestroy(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR);
    PDL_CHECK_STATUS(pdlStatus);
    dbAttributes.listAttributes.keySize = sizeof(PDL_L1_INTERFACE_MODE_LIST_KEYS_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR);
    PDL_CHECK_STATUS(pdlStatus);

    for (i = 0; i < portAttributesPtr->numOfCopperModes; i++) {
        memset(&l1IfEntry, 0, sizeof(l1IfEntry));
        prvPdlPpDbFrontPanelSpeedBackConvert(portAttributesPtr->copperModesArr[i].speed, &speedUtil);
        l1IfEntry.list_keys.speed = speedUtil;
        prvPdlPpDbFrontPanelIfModeBackConvert(portAttributesPtr->copperModesArr[i].mode, &l1IfEntry.mode);
        l1IfEntry.mode_mask = PDL_FIELD_HAS_VALUE_CNS;
        pdlStatus = prvPdlibDbAdd(frontPanelPortEntryPtr->copperL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR,
                                  &l1IfEntry.list_keys,
                                  (void **)&l1IfEntry,
                                  (void **)&l1IfEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }

    pdlStatus = prvPdlibDbDestroy(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR);
    PDL_CHECK_STATUS(pdlStatus);
    dbAttributes.listAttributes.keySize = sizeof(PDL_L1_INTERFACE_MODE_LIST_KEYS_STC);
    dbAttributes.listAttributes.entrySize = sizeof(PDL_L1_INTERFACE_MODE_LIST_PARAMS_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E,
                                 &dbAttributes,
                                 &frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR);
    PDL_CHECK_STATUS(pdlStatus);

    for (i = 0; i < portAttributesPtr->numOfFiberModes; i++) {
        memset(&l1IfEntry, 0, sizeof(l1IfEntry));
        prvPdlPpDbFrontPanelSpeedBackConvert(portAttributesPtr->fiberModesArr[i].speed, &speedUtil);
        l1IfEntry.list_keys.speed = speedUtil;
        prvPdlPpDbFrontPanelIfModeBackConvert(portAttributesPtr->fiberModesArr[i].mode, &l1IfEntry.mode);
        l1IfEntry.mode_mask = PDL_FIELD_HAS_VALUE_CNS;
        pdlStatus = prvPdlibDbAdd(frontPanelPortEntryPtr->fiberL1InterfaceModesGroup.portL1InterfaceModesGroupType.l1InterfaceModeList_PTR,
                                  &l1IfEntry.list_keys,
                                  (void **)&l1IfEntry,
                                  (void **)&l1IfEntryPtr);
        PDL_CHECK_STATUS(pdlStatus);
    }

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPpDbB2bAttributesSet ( IN UINT_32 b2bLinkId, IN PDL_PP_B2B_ATTRIBUTES_STC * b2bAttributesPtr )
 *
 * @brief   Update PP back-to-back link attributes
 *
 * @param [in]  b2bLinkId           b2b link number.
 * @param [in]  b2bAttributesPtr    b2b link attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  b2b's entry wasn't found.
 */

PDL_STATUS pdlPpDbB2bAttributesSet(
    IN   UINT_32                                 b2bLinkId,
    IN   PDL_PP_B2B_ATTRIBUTES_STC             * b2bAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_PP_ATTRIBUTES_STC               ppAttributes;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (b2bAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "b2bAttributesPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlPpDbAttributesGet(&ppAttributes);
    PDL_CHECK_STATUS(pdlStatus);

    if (b2bLinkId >= ppAttributes.numOfBackToBackLinksPerPp) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "ILLEGAL b2bLinkId NUMBER");
        return PDL_NOT_FOUND;
    }

    memcpy(&pdlPpB2bDb[b2bLinkId], b2bAttributesPtr, sizeof(PDL_PP_B2B_ATTRIBUTES_STC));

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPpDbFrontPanelAttributesSet ( IN UINT_32 frontPanelNumber, OUT PDL_PP_FRONT_PANEL_ATTRIBUTES_STC * frontPanelAttributesPtr )
 *
 * @brief   Update attributes of front panel group
 *
 * @param [in]  frontPanelNumber        front panel group id.
 * @param [out] frontPanelAttributesPtr front panel attributes.
 *
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpDbFrontPanelAttributesSet(
    IN   UINT_32                                 frontPanelNumber,
    OUT  PDL_PP_FRONT_PANEL_ATTRIBUTES_STC     * frontPanelAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                      pdlStatus;
    PDL_NETWORK_PORTS_PARAMS_STC                    portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_KEYS_STC             fpEntryKey;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC          *fpEntryPtr;
    UINT_32                                         count;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (frontPanelAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "frontPanelAttributesPtr POINTER NULL, FRONT PANEL NUMBER: %d", frontPanelNumber);
        return PDL_BAD_PTR;
    }

    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, frontPanelAttributesPtr->ordering);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, frontPanelAttributesPtr->portMaximumSpeed);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, frontPanelAttributesPtr->firstPortNumberShift);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(UINT_32, frontPanelAttributesPtr->numOfPortsInGroup);
    IDBG_PDL_LOGGER_API_IN_PARAM_MAC(BOOLEAN, frontPanelAttributesPtr->isLogicalPortRequired);

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    fpEntryKey.groupNumber = frontPanelNumber;

    pdlStatus = prvPdlibDbFind(portsParams.frontPanelGroupList_PTR, &fpEntryKey, (void**)&fpEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetNumOfEntries(fpEntryPtr->portList_PTR, &count);
    PDL_CHECK_STATUS(pdlStatus);

    if (count != frontPanelAttributesPtr->numOfPortsInGroup) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "numOfPortsInGroup cannot be changed, FRONT PANEL NUMBER: %d", frontPanelNumber);
        return PDL_BAD_PARAM;
    }

    fpEntryPtr->firstPortNumberShift = frontPanelAttributesPtr->firstPortNumberShift;
    fpEntryPtr->isLogicalPortRequired = frontPanelAttributesPtr->isLogicalPortRequired;
    fpEntryPtr->groupOrdering = frontPanelAttributesPtr->ordering;
    pdlStatus = prvPdlPpDbFrontPanelSpeedBackConvert(frontPanelAttributesPtr->portMaximumSpeed, &fpEntryPtr->speed);
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}

/*$ END OF pdlPpDbFrontPanelAttributesSet */

/**
 * @fn  PDL_STATUS pdlPpPortConvertFrontPanel2DevPort ( IN UINT_32 frontPanelNumber, IN UINT_32 portNumberInGroup, OUT UINT_32 * devPtr, OUT UINT_32 * portPtr )
 *
 * @brief   Convert between front panel group &amp; port to dev &amp; port
 *
 * @param [in]  frontPanelNumber    front panel group id.
 * @param [in]  portNumberInGroup   front panel port number.
 * @param [out] devPtr              packet processor device number.
 * @param [out] portPtr             packet processor port number.
 *
 * @return  PDL_OK         convert succesfull.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertFrontPanel2DevPort(
    IN   UINT_32                                 frontPanelNumber,
    IN   UINT_32                                 portNumberInGroup,
    OUT  UINT_32                               * devPtr,
    OUT  UINT_32                               * portPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_NETWORK_PORTS_PARAMS_STC                            portsParams;
    PDL_FRONT_PANEL_GROUP_LIST_KEYS_STC                     frontPanelEntryKey;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC                  *frontPanelGroupPtr;
    PDL_PORT_LIST_KEYS_STC                                  frontPanelPortKey;
    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (devPtr == NULL || portPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "NULL devPtr OR portPtr, FRONT PANEL NUMBER: %d", frontPanelNumber);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&portsParams)) {
        return PDL_NOT_SUPPORTED;
    }

    frontPanelEntryKey.groupNumber = frontPanelNumber;

    pdlStatus = prvPdlibDbFind(portsParams.frontPanelGroupList_PTR, &frontPanelEntryKey, (void**)&frontPanelGroupPtr);
    PDL_CHECK_STATUS(pdlStatus);

    frontPanelPortKey.portNumber = portNumberInGroup;

    pdlStatus = prvPdlibDbFind(frontPanelGroupPtr->portList_PTR, &frontPanelPortKey, (void**)&frontPanelPortEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    *devPtr = frontPanelPortEntryPtr->ppDeviceNumber;
    if (pdlIsFieldHasValue(frontPanelPortEntryPtr->logicalPortNumber_mask)) {
        *portPtr = frontPanelPortEntryPtr->logicalPortNumber;
    }
    else {
        *portPtr = frontPanelPortEntryPtr->ppPortNumber;
    }

    return PDL_OK;
}
/*$ END OF pdlPpPortConvertFrontPanel2DevPort */


/**
 * @fn  PDL_STATUS pdlPpDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Pp debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPpDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlPpDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlPpDebugSet */



/**
 * @fn  PDL_STATUS pdlPpPortConvertDevPort2FrontPanel ( IN UINT_32 dev, IN UINT_32 port, OUT UINT_32 * frontPanelNumberPtr, OUT UINT_32 * portNumberInGroupPtr )
 *
 * @brief   Convert between dev &amp; port to front panel group &amp; port
 *
 * @param [in]      dev                     packet processor device number.
 * @param [in]      logicalPort             packet processor port number.
 * @param [out]     frontPanelNumberPtr     front panel group id.
 * @param [in,out]  portNumberInGroupPtr    If non-null, the port number in group pointer.
 *
 * @return  PDL_OK         convert successfully.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 *
 * ### param [out]      frontPanelPortPtr   front panel port number.
 */

PDL_STATUS pdlPpPortConvertDevPort2FrontPanel(
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * frontPanelNumberPtr,
    OUT UINT_32                                * portNumberInGroupPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC                  *frontPanelGroupPtr;
    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (frontPanelNumberPtr == NULL || portNumberInGroupPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "frontPanelAttributesPtr POINTER NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", logicalPort, dev);
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelGroupPtr = (PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC *)portEntryPtr->frontPanelEntryPtr;
    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    *frontPanelNumberPtr = frontPanelGroupPtr->list_keys.groupNumber;
    *portNumberInGroupPtr = frontPanelPortEntryPtr->list_keys.portNumber;

    return PDL_OK;
}
/*$ END OF pdlPpPortConvertDevPort2FrontPanel */

/**
 * @fn  PDL_STATUS pdlPpPortConvertMacToLogical (IN UINT_32 dev, IN UINT_32 macPort, OUT UINT_32 * logicalPortPtr );
 *
 * @brief   Convert between macPort to logicalPort.
 *
 * @param [in]      dev                     dev number.
 * @param [in]      macPort                 macPort number.
 * @param [out]     logicalPortPtr          logicalPort number ptr.
 * @param [out]     devNumPtr               dev number ptr.
 *
 * @return  PDL_OK         convert successfully.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertMacToLogical(
    IN  UINT_32                                  dev,
    IN  UINT_32                                  macPort,
    OUT UINT_32                                * logicalPortPtr,
    OUT UINT_32                                * devNumPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;
    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if ((logicalPortPtr == NULL) || (devNumPtr == NULL)) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "logicalPortPtr or devNumPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = macPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->macPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", macPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    *devNumPtr = dev;
    if (pdlIsFieldHasValue(frontPanelPortEntryPtr->logicalPortNumber_mask)) {
        *logicalPortPtr = frontPanelPortEntryPtr->logicalPortNumber;
    }
    else {
        *logicalPortPtr = frontPanelPortEntryPtr->ppPortNumber;
    }

    return PDL_OK;
}
/*$ END OF pdlPpPortConvertMacToLogical */


/**
 * @fn  PDL_STATUS pdlPpPortConvertLogicalToMac (IN UINT_32 dev, IN UINT_32 logicalPort, OUT UINT_32 * macPortPtr );
 *
 * @brief   Convert between logicalPort to macPort.
 *
 * @param [in]      dev                     dev number.
 * @param [in]      logiaclPort             logicalPort number.
 * @param [out]     macPortPtr              macPort number.
 * @param [out]     devNumPtr               dev number ptr.
 *
 * @return  PDL_OK         convert successfully.
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS pdlPpPortConvertLogicalToMac(
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * devNumPtr,
    OUT UINT_32                                * macPortPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;
    PDL_PORT_LIST_PARAMS_STC                               *frontPanelPortEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if ((macPortPtr == NULL) || (devNumPtr == NULL)) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "macPortPtr or devNumPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }

    frontPanelPortEntryPtr = (PDL_PORT_LIST_PARAMS_STC *)portEntryPtr->frontPanelPortEntryPtr;

    *devNumPtr = dev;
    *macPortPtr = frontPanelPortEntryPtr->ppPortNumber;

    return PDL_OK;
}
/*$ END OF pdlPpPortConvertLogicalToMac */

/**
 * @fn  PDL_STATUS prvPdlPpPortGetPhyInterfceId (IN UINT_32 dev, IN UINT_32 logicalPort, OUT UINT_32 * phyInterfaceIdPtr );
 *
 * @brief   Get the phy interface id that is used inside PDL private interface database
 *
 * @param [in]      dev                     dev number.
 * @param [in]      logiaclPort             logicalPort number.
 * @param [out]     phyInterfaceIdPtr       phyInterfaceId.
  *
 * @return  PDL_OK         success
 * @return  PDL_BAD_PTR    illegal pointer supplied.
 * @return  PDL_NOT_FOUND  front panel's entry wasn't found.
 */

PDL_STATUS prvPdlPpPortGetPhyInterfceId(
    IN  UINT_32                                  dev,
    IN  UINT_32                                  logicalPort,
    OUT UINT_32                                * phyInterfaceIdPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PRV_PDL_DEVICE_DB_KEY_STC                               devKey;
    PRV_PDL_DEVICE_DB_ENTRY_STC                            *devEntryPtr;
    PRV_PDL_PORT_DB_KEY_STC                                 portKey;
    PRV_PDL_PORT_DB_ENTRY_STC                              *portEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (phyInterfaceIdPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "macPortPtr or devNumPtr POINTER NULL");
        return PDL_BAD_PTR;
    }

    devKey.dev = dev;
    pdlStatus = prvPdlibDbFind(prvPdlPpDevDb, (void*) &devKey, (void**) &devEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    portKey.port = logicalPort;
    pdlStatus = prvPdlibDbFind(devEntryPtr->logicalPortsList, (void*) &portKey, (void**) &portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->frontPanelPortEntryPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlPpDebugFlag)(__FUNCTION__, __LINE__, "portEntryPtr->frontPanelPortEntryPtr NULL, PORT NUM: %d, DEVICE NUM: %d", logicalPort, dev);
        return PDL_BAD_STATE;
    }
    *phyInterfaceIdPtr = portEntryPtr->phyInterfaceId;
    return PDL_OK;
}
/*$ END OF prvPdlPpPortGetPhyInterfceId */


/**
 * @fn  PDL_STATUS prvPdlPacketProcessorDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPacketProcessorDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PRV_PDL_DEVICE_DB_ENTRY_STC   * devEntryPtr = NULL, *devNextEntryPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    /* since this module also initializes the SFP module, it'll need to destroy it */
    pdlStatus = prvPdlSfpDestroy();
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = prvPdlibDbGetFirst(prvPdlPpDevDb, (void **) &devNextEntryPtr);
    while (pdlStatus == PDL_OK) {
        devEntryPtr = devNextEntryPtr;

        pdlStatus  = prvPdlibDbGetNext(prvPdlPpDevDb, (void*) &devEntryPtr->key, (void**)&devNextEntryPtr);

        PDL_CHECK_STATUS(prvPdlibDbDestroy(devEntryPtr->logicalPortsList));
        PDL_CHECK_STATUS(prvPdlibDbDestroy(devEntryPtr->macPortsList));

    }
    PDL_CHECK_STATUS(prvPdlibDbDestroy(prvPdlPpDevDb));

    return PDL_OK;
}
/*$ END OF prvPdlPacketProcessorDestroy */
