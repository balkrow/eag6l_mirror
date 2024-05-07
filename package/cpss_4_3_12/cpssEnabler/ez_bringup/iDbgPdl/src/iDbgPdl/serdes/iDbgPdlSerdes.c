/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file iDbgPdlSerdes.c
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
 * @brief Debug lib button module
 *
 * @version   1
********************************************************************************
*/
/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/serdes/pdlSerdes.h>
#include <pdl/packet_processor/pdlPacketProcessor.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <iDbgPdl/utils/iDbgPdlUtils.h>

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlSerdesInfoGetFirstWrapper
*
* DESCRIPTION:
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlSerdesInfoGetFirstWrapper(

    /*!     INPUTS:             */
    IN UINT_32                  dev,
    IN UINT_32                  port,
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PDL_PORT_LANE_DATA_STC *laneDataPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS pdlStatus;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesPortDbSerdesInfoGetFirst, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, laneDataPtr));
    return pdlStatus;
}
/*$ END OF prvIDbgPdlSerdesInfoGetFirstWrapper */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlSerdesInfoGetNextWrapper
*
* DESCRIPTION:
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlSerdesInfoGetNextWrapper(

    /*!     INPUTS:             */
    IN UINT_32                  dev,
    IN UINT_32                  port,
    IN PDL_PORT_LANE_DATA_STC  *currLaneDataPtr,
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PDL_PORT_LANE_DATA_STC *nextLaneDataPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS pdlStatus;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesPortDbSerdesInfoGetNext, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, port),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, currLaneDataPtr),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, nextLaneDataPtr));
    return pdlStatus;
}
/*$ END OF prvIDbgPdlSerdesInfoGetNextWrapper */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlSerdesGetPort
*
* DESCRIPTION:
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlSerdesGetPort(

    /*!     INPUTS:             */
    IN  UINT_32                dev,
    IN  UINT_32                laneId,
    /*!     INPUTS / OUTPUTS:   */
    /*!     OUTPUTS:            */
    OUT UINT_32                * portPtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    UINT_32                                 tempDev, tempPort, b2bIdx;
    PDL_STATUS                              pdlStatus;
    PDL_PORT_LANE_DATA_STC                  serdesInfo;
    PDL_PP_ATTRIBUTES_STC                   ppAttr;
    PDL_PP_B2B_ATTRIBUTES_STC               b2bAttr;

    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    if (portPtr == NULL) {
        return PDL_BAD_PTR;
    }
    pdlStatus = pdlPpDbFirstPortAttributesGet(&tempDev, &tempPort);
    while (pdlStatus == PDL_OK) {
        if (tempDev == dev) {
            pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(tempDev, tempPort, &serdesInfo);
            while (pdlStatus == PDL_OK) {
                if (serdesInfo.absSerdesNum == laneId) {
                    *portPtr = tempPort;
                    return PDL_OK;
                }
                pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(tempDev, tempPort, &serdesInfo, &serdesInfo);
            }
        }
        pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort);
    }

    /* if not found in network ports, search serdes in b2b links */
    pdlStatus = pdlPpDbAttributesGet(&ppAttr);
    PDL_CHECK_STATUS(pdlStatus);
    for (b2bIdx = 0; b2bIdx < ppAttr.numOfBackToBackLinksPerPp; b2bIdx++) {
        pdlStatus = pdlPpDbB2bAttributesGet(b2bIdx, &b2bAttr);
        PDL_CHECK_STATUS(pdlStatus);
        tempDev = dev;
        if (b2bAttr.firstDev == dev) {
            tempPort = b2bAttr.firstPort;
        }
        else {
            tempPort = b2bAttr.secondPort;
        }
        pdlStatus = pdlSerdesPortDbSerdesInfoGetFirst(tempDev, tempPort, &serdesInfo);
        while (pdlStatus == PDL_OK) {
            if (serdesInfo.absSerdesNum == laneId) {
                *portPtr = tempPort;
                return PDL_OK;
            }
            pdlStatus = pdlSerdesPortDbSerdesInfoGetNext(tempDev, tempPort, &serdesInfo, &serdesInfo);
        }
    }

    return PDL_NOT_FOUND;
}
/*$ END OF prvIDbgPdlSerdesGetPort */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesGetInfo
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesGetInfo(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            logicalPort
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN                         portListArr[128];
    UINT_32                         i_port, tempDev, tempPort;
    PDL_STATUS                      pdlStatus;
    PDL_PORT_LANE_DATA_STC          laneInfo;
    PDL_PP_ATTRIBUTES_STC           ppAttr;
    PDL_PP_B2B_ATTRIBUTES_STC       b2bAttr;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    memset(portListArr, 0, sizeof(portListArr));

    for (pdlStatus = pdlPpDbFirstPortAttributesGet(&tempDev, &tempPort);
         pdlStatus == PDL_OK;
         pdlStatus = pdlPpDbPortAttributesGetNext(tempDev, tempPort, &tempDev, &tempPort)) {
        if (tempDev != dev) {
            continue;
        }

        if (tempPort >= sizeof(portListArr)) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG("Illegal port number encountered: dev %lu port %lu\n", tempDev, tempPort);
            return PDL_OUT_OF_RANGE;
        }

        if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == tempPort)) {
            portListArr[tempPort] = TRUE;
            if (logicalPort == tempPort) {
                break;
            }
        }
    }

    pdlStatus = pdlPpDbAttributesGet(&ppAttr);
    PDL_CHECK_STATUS(pdlStatus);
    for (tempPort = 0; tempPort < ppAttr.numOfBackToBackLinksPerPp; tempPort++) {
        pdlStatus = pdlPpDbB2bAttributesGet(tempPort, &b2bAttr);
        PDL_CHECK_STATUS(pdlStatus);
        if (b2bAttr.firstDev == dev) {
            if (b2bAttr.firstPort >= sizeof(portListArr)) {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("Illegal b2b left port number encountered: dev %lu port %lu\n", b2bAttr.firstDev, b2bAttr.firstPort);
                return PDL_OUT_OF_RANGE;
            }
            if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == b2bAttr.firstPort)) {
                portListArr[b2bAttr.firstPort] = TRUE;
            }
        }
        if (b2bAttr.secondDev == dev) {
            if (b2bAttr.secondPort >= sizeof(portListArr)) {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG("Illegal b2b left port number encountered: dev %lu port %lu\n", b2bAttr.secondDev, b2bAttr.secondPort);
                return PDL_OUT_OF_RANGE;
            }
            if ((logicalPort == IDBG_PDL_ID_ALL) || (logicalPort == b2bAttr.secondPort)) {
                portListArr[b2bAttr.secondPort] = TRUE;
            }
        }
    }

    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-15s | %-15s\n", "DEVICE", "PORT", "ABSOLUTE NUMBER", "RELATIVE NUMBER");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-15s | %-15s\n", "------", "----", "---------------", "---------------");

    for (i_port = 0; i_port < sizeof(portListArr); i_port++) {
        if (portListArr[i_port] == FALSE) {
            continue;
        }
        for (pdlStatus = prvIDbgPdlSerdesInfoGetFirstWrapper(dev, i_port, &laneInfo);
             pdlStatus == PDL_OK;
             pdlStatus = prvIDbgPdlSerdesInfoGetNextWrapper(dev, i_port,  &laneInfo, &laneInfo)) {
            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6d | %-4d | %-15d | %-15d\n", dev, i_port, laneInfo.absSerdesNum, laneInfo.relSerdesNum);
        }
    }
    return PDL_OK;
}
/*$ END OF iDbgPdlSerdesGetInfo */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetTxFineTune
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlSerdesSetTxFineTune(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN void                             * devPtr,
    IN void                             * lanePtr,
    IN void                             * ifModePtr,
    IN void                             * connectTypePtr,
	IN void                             * techTypePtr,
    IN void                             * txAmplPtr,
    IN void                             * txAmplAdjEnPtr,
    IN void                             * txAmplShtEnPtr,
    IN void                             * emph0Ptr,
    IN void                             * emph1Ptr,
    IN void                             * txEmphEnPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                 value;
    PDL_STATUS                              pdlStatus = PDL_OK;
    PDL_INTERFACE_MODE_ENT                  ifMode;
    PDL_CONNECTOR_TYPE_ENT                  connectType;
    PDL_CONNECTOR_TECHNOLOGY_ENT					techType;
    BOOLEAN                                 fineTuneTxAttributesExists;
    PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC    fineTuneTxAttributes;
    BOOLEAN                                 fineTuneRxAttributesExists;
    PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC    fineTuneRxAttributes;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    if (!resultPtr || !devPtr || !lanePtr || !ifModePtr || !connectTypePtr || !techTypePtr) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Mandatory parameter is missing!\n");
        return PDL_BAD_PARAM;
    }
    if ((pdlStatus = pdlLibConvertStringToEnumValue(PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, (char *)ifModePtr, &value)) != PDL_OK) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown interface mode (%s)!\n", (char *)ifModePtr);
        return PDL_BAD_PARAM;
    }
    ifMode = (PDL_INTERFACE_MODE_ENT) value;

    if ((pdlStatus = pdlLibConvertStringToEnumValue(PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E, (char *)connectTypePtr, &value)) != PDL_OK) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown connector type (%s)!\n", (char *)connectTypePtr);
        return PDL_BAD_PARAM;
    }
    connectType = (PDL_CONNECTOR_TYPE_ENT) value;

    if ((pdlStatus = pdlLibConvertStringToEnumValue(PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E, (char *)techTypePtr, &value)) != PDL_OK) {
		IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown techTypePtr (%s)!\n", (char *)techTypePtr);
		return PDL_BAD_PARAM;
	}
    techType = (PDL_CONNECTOR_TECHNOLOGY_ENT) value;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)devPtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)lanePtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ifMode),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, connectType),
								IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, techType),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneTxAttributesExists),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneTxAttributes),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneRxAttributesExists),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneRxAttributes));
    if (pdlStatus == PDL_NOT_FOUND) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [lane %d] [mode %s] [connector %s] [techType %s] NOT FOUND", *(UINT_32*)devPtr, *(UINT_32*)lanePtr, (char *)ifModePtr, (char *)connectTypePtr, (char *)techTypePtr);
        return PDL_NOT_FOUND;
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (fineTuneTxAttributesExists == FALSE) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("TX fine tuning doesn't exist for [dev %d] [lane %d] [interface mode %s] [connector type %s] [techType %s]!\n",
                                       *(UINT_32*)devPtr, *(UINT_32*)lanePtr, (char *)ifModePtr, (char *)connectTypePtr, (char *)techTypePtr);
        return PDL_BAD_PARAM;
    }

    if (txAmplPtr) {
        fineTuneTxAttributes.txAmpl = (UINT_8)(*(UINT_32 *)txAmplPtr);
    }

    if (txAmplAdjEnPtr) {
        fineTuneTxAttributes.txAmplAdjEn = (BOOLEAN)*(UINT_32 *)txAmplAdjEnPtr;
    }

    if (txAmplShtEnPtr) {
        fineTuneTxAttributes.txAmplShtEn = (BOOLEAN)*(UINT_32 *)txAmplShtEnPtr;
    }

    if (emph0Ptr) {
        fineTuneTxAttributes.emph0 = (UINT_8)(*(UINT_32 *)emph0Ptr);
    }

    if (emph1Ptr) {
        fineTuneTxAttributes.emph1 = (UINT_8)(*(UINT_32 *)emph1Ptr);
    }

    if (txEmphEnPtr) {
        fineTuneTxAttributes.txEmphEn = (BOOLEAN)*(UINT_32 *)txEmphEnPtr;
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)devPtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)lanePtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ifMode),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, connectType),
								IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, techType),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &fineTuneTxAttributes),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, NULL));
    return pdlStatus;
}
/*$ END OF iDbgPdlSerdesSetTxFineTune */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetRxFineTune
*
* DESCRIPTION:
*
*
*****************************************************************************/
extern PDL_STATUS iDbgPdlSerdesSetRxFineTune(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN void                             * devPtr,
    IN void                             * lanePtr,
    IN void                             * ifModePtr,
    IN void                             * connectTypePtr,
	IN void                             * techTypePtr,
    IN void                             * sqlchPtr,
    IN void                             * ffeResPtr,
    IN void                             * ffeCapPtr,
    IN void                             * align90Ptr,
    IN void                             * dcGainPtr,
    IN void                             * bandWidthPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    UINT_32                                 value;
    PDL_STATUS                              pdlStatus = PDL_OK;
    PDL_INTERFACE_MODE_ENT                  ifMode;
    PDL_CONNECTOR_TYPE_ENT                  connectType;
    PDL_CONNECTOR_TECHNOLOGY_ENT					techType;
    BOOLEAN                                 fineTuneTxAttributesExists;
    PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC    fineTuneTxAttributes;
    BOOLEAN                                 fineTuneRxAttributesExists;
    PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC    fineTuneRxAttributes;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    if (!resultPtr || !devPtr || !lanePtr || !ifModePtr || !connectTypePtr || !techTypePtr) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Mandatory parameter is missing!\n");
        return PDL_BAD_PARAM;
    }

    if ((pdlStatus = pdlLibConvertStringToEnumValue(PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, (char *)ifModePtr, &value)) != PDL_OK) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown interface mode (%s)!\n", (char *)ifModePtr);
        return PDL_BAD_PARAM;
    }
    ifMode = (PDL_INTERFACE_MODE_ENT) value;

    if ((pdlStatus = pdlLibConvertStringToEnumValue(PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E, (char *)connectTypePtr, &value)) != PDL_OK) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown connector type (%s)!\n", (char *)connectTypePtr);
        return PDL_BAD_PARAM;
    }
    connectType = (PDL_CONNECTOR_TYPE_ENT) value;

    if ((pdlStatus = pdlLibConvertStringToEnumValue(PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E, (char *)techTypePtr, &value)) != PDL_OK) {
   		IDBG_PDL_LOGGER_API_OUTPUT_LOG("Unknown techTypePtr (%s)!\n", (char *)techTypePtr);
   		return PDL_BAD_PARAM;
   	}
    techType = (PDL_CONNECTOR_TECHNOLOGY_ENT) value;

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)devPtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)lanePtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ifMode),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, connectType),
								IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, techType),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneTxAttributesExists),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneTxAttributes),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneRxAttributesExists),
                                IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneRxAttributes));
    if (pdlStatus == PDL_NOT_FOUND) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [lane %d] [mode %s] [connector %s] [techType %s] NOT FOUND", *(UINT_32*)devPtr, *(UINT_32*)lanePtr, (char *)ifModePtr, (char *)connectTypePtr, (char *)techTypePtr);
        return PDL_NOT_FOUND;
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (fineTuneRxAttributesExists == FALSE) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("RX fine tuning doesn't exist for [dev %d] [lane %d] [interface mode %s] [connector type %s] [techType %s]!\n",
                                       *(UINT_32*)devPtr, *(UINT_32*)lanePtr, (char *)ifModePtr, (char *)connectTypePtr, (char *)techTypePtr);
        return PDL_BAD_PARAM;
    }

    if (sqlchPtr) {
        fineTuneRxAttributes.sqlch = *(UINT_32 *)sqlchPtr;
    }

    if (ffeResPtr) {
        fineTuneRxAttributes.ffeRes = *(UINT_32 *)ffeResPtr;
    }

    if (ffeCapPtr) {
        fineTuneRxAttributes.ffeCap = *(UINT_32 *)ffeCapPtr;
    }

    if (align90Ptr) {
        fineTuneRxAttributes.align90 = *(UINT_32 *)align90Ptr;
    }

    if (dcGainPtr) {
        fineTuneRxAttributes.dcGain = *(UINT_32 *)dcGainPtr;
    }

    if (bandWidthPtr) {
        fineTuneRxAttributes.bandWidth = *(UINT_32 *)bandWidthPtr;
    }

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)devPtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, *(UINT_32*)lanePtr),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, ifMode),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, connectType),
								IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, techType),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, NULL),
                                IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(PTR, &fineTuneRxAttributes));
    return pdlStatus;
}
/*$ END OF iDbgPdlSerdesSetRxFineTune */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesGetPolarity
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesGetPolarity(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            laneId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    BOOLEAN                             isB2bLink;
    UINT_32                             i_lane, port, frontPanelNumber, portNumberInGroup;
    PDL_STATUS                          pdlStatus = PDL_OK;
    PDL_LANE_POLARITY_ATTRIBUTES_STC    polarityAttributes;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    if (laneId == IDBG_PDL_ID_ALL) {
        i_lane = 0;
    }
    else {
        i_lane = laneId;
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-7s | %-7s\n", "", "", "MAC", "GROUP", "PORT", "", "");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-7s | %-7s\n", "DEVICE", "LANE", "PORT", "NUMBER", "NUMBER", "TX_SWAP", "RX_SWAP");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-7s | %-7s\n", "------", "----", "----", "------", "-----", "-------", "-------");
    while (i_lane < PDL_SERDES_MAX_ABS_LANE_ID_CNS) {
        IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbPolarityAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                                  IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i_lane),
                                                  IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &polarityAttributes));
        if (laneId != IDBG_PDL_ID_ALL) {
            PDL_CHECK_STATUS(pdlStatus);
        }
        if (pdlStatus == PDL_OK) {
            pdlStatus = prvIDbgPdlSerdesGetPort(dev, i_lane, &port);
            PDL_CHECK_STATUS(pdlStatus);
            pdlStatus = pdlPpDbIsB2bLink(dev, port, &isB2bLink);
            if (pdlStatus == PDL_OK && isB2bLink == TRUE) {
                frontPanelNumber = portNumberInGroup = 0;
            }
            else {
                pdlStatus = pdlPpPortConvertDevPort2FrontPanel(dev, port, &frontPanelNumber, &portNumberInGroup);
                PDL_CHECK_STATUS(pdlStatus);
            }
            IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(BOOLEAN, polarityAttributes.txSwap);
            IDBG_PDL_LOGGER_API_OUT_PARAM_MAC(BOOLEAN, polarityAttributes.rxSwap);

            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6d | %-4d | %-4d | %-6d | %-6d | %-7s | %-7s\n", dev, i_lane, port, frontPanelNumber, portNumberInGroup, polarityAttributes.txSwap ? "true" : "false", polarityAttributes.rxSwap ? "true" : "false");
        }
        if (laneId == i_lane) {
            break;
        }
        i_lane ++;
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlSerdesGetPolarity */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetPolarity
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesSetPolarity(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            laneId,
    IN BOOLEAN                          * txSwapPtr,
    IN BOOLEAN                          * rxSwapPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus = PDL_OK;
    PDL_LANE_POLARITY_ATTRIBUTES_STC    polarityAttributes;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/

    if (!resultPtr || (!txSwapPtr && !rxSwapPtr)) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("Mandatory parameter is missing!\n");
        return PDL_BAD_PARAM;
    }

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbPolarityAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, laneId),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &polarityAttributes));
    if (pdlStatus == PDL_NOT_FOUND) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG("[dev %d] [lane %d] NOT FOUND", dev, laneId);
        return PDL_NOT_FOUND;
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (txSwapPtr) {
        polarityAttributes.txSwap = *txSwapPtr;
    }
    if (rxSwapPtr) {
        polarityAttributes.rxSwap = *rxSwapPtr;
    }

    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbPolarityAttrSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, laneId),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &polarityAttributes));
    PDL_CHECK_STATUS(pdlStatus);

    return PDL_OK;
}
/*$ END OF iDbgPdlSerdesSetPolarity */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlSerdesFineTuneGetFirstWrapper
*
* DESCRIPTION:
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlSerdesFineTuneGetFirstWrapper(

    /*!     INPUTS:             */
    IN UINT_32                      dev,
    IN UINT_32                      laneId,
    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PDL_INTERFACE_MODE_ENT     *interfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT     *connectorTypePtr,
	OUT PDL_CONNECTOR_TECHNOLOGY_ENT	   *techTypePtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS pdlStatus;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrGetFirst, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, laneId),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, interfaceModePtr),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, connectorTypePtr),
											  IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, techTypePtr));
    return pdlStatus;
}
/*$ END OF prvIDbgPdlSerdesFineTuneGetFirstWrapper */

/*****************************************************************************
* FUNCTION NAME: prvIDbgPdlSerdesFineTuneGetNextWrapper
*
* DESCRIPTION:
*
*
*****************************************************************************/
static PDL_STATUS prvIDbgPdlSerdesFineTuneGetNextWrapper(

    /*!     INPUTS:             */
    IN UINT_32                      dev,
    IN UINT_32                      laneId,
    IN PDL_INTERFACE_MODE_ENT       currInterfaceMode,
    IN PDL_CONNECTOR_TYPE_ENT       currConnectorType,
	IN PDL_CONNECTOR_TECHNOLOGY_ENT       currTechType,

    /*!     INPUTS / OUTPUTS:   */

    /*!     OUTPUTS:            */
    OUT PDL_INTERFACE_MODE_ENT     *nextInterfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT     *nextConnectorTypePtr,
	OUT PDL_CONNECTOR_TECHNOLOGY_ENT     *nextTechTypePtr
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS pdlStatus;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    IDBG_PDL_LOGGER_IGNORE_RESULT_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrGetNext, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, laneId),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, currInterfaceMode),
                                              IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, currConnectorType),
											  IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, currTechType),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, nextInterfaceModePtr),
                                              IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, nextConnectorTypePtr),
											  IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(UINT_32, nextTechTypePtr));
    return pdlStatus;
}
/*$ END OF prvIDbgPdlSerdesFineTuneGetNextWrapper */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesGetFineTune
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesGetFineTune(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN UINT_32                            dev,
    IN UINT_32                            laneId
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                            pdlStatus;
    UINT_32                               serdesBitmap[8]; /* possible serdes ids */
    UINT_32                               i, interfaceMode, connectorType, field, port, frontPanelNumber, portNumberInGroup, techType;
    char                                 *interfaceStr, *connectorStr, *techStr;
    BOOLEAN                               fineTuneTxAttributesExists, fineTuneRxAttributesExists, hasData, isB2bLink;
    PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC  fineTuneTxAttributes;
    PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC  fineTuneRxAttributes;
    /****************************************************************************/
    /*                     F U N C T I O N   L O G I C                          */
    /****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

#define id2word_MAC(id) (id / 32)
#define id2bit_MAC(id)  (1 << (id % 32))

    if (laneId == IDBG_PDL_ID_ALL) {
        memset(serdesBitmap, 0xFF, sizeof(serdesBitmap));
    }
    else if (laneId > 255) {
        IDBG_PDL_LOGGER_API_OUTPUT_LOG(" Lane id (%d) must be < 256\n", laneId);
        return PDL_BAD_VALUE;
    }
    else {
        memset(serdesBitmap, 0, sizeof(serdesBitmap));
        serdesBitmap[id2word_MAC(laneId)] = id2bit_MAC(laneId);
    }
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-14s | %-14s | %-14s | %-27s | %-14s\n", "", "", "MAC", "GROUP", " PORT ", "", "", "", "", "");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-14s | %-14s | %-14s | %-27s | %-14s\n", "DEVICE", "LANE", "PORT", "NUMBER", "NUMBER", "INTERFACE MODE", "CONNECTOR TYPE", "TECH TYPE", "Tx Attributes",               "Rx Attributes");
    IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-14s | %-14s | %-14s | %-27s | %-14s\n", "------", "----", "----", "------", "-----", "--------------", "--------------", "--------------", "---------------------------", "--------------");

    for (i = 0; i < 256; i++) {
        if (serdesBitmap[id2word_MAC(i)] & id2bit_MAC(i)) {
            hasData = FALSE;
            for (pdlStatus = prvIDbgPdlSerdesFineTuneGetFirstWrapper(dev, i, (PDL_INTERFACE_MODE_ENT *)&interfaceMode, (PDL_CONNECTOR_TYPE_ENT *)&connectorType, (PDL_CONNECTOR_TECHNOLOGY_ENT*) &techType);
                 pdlStatus == PDL_OK;
                 pdlStatus = prvIDbgPdlSerdesFineTuneGetNextWrapper(dev, i, (PDL_INTERFACE_MODE_ENT)interfaceMode, (PDL_CONNECTOR_TYPE_ENT)connectorType, (PDL_CONNECTOR_TECHNOLOGY_ENT)techType, (PDL_INTERFACE_MODE_ENT *)&interfaceMode, (PDL_CONNECTOR_TYPE_ENT *)&connectorType, (PDL_CONNECTOR_TECHNOLOGY_ENT*) &techType)) {
                pdlStatus = prvIDbgPdlSerdesGetPort(dev, i, &port);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlPpDbIsB2bLink(dev, port, &isB2bLink);
                if (pdlStatus == PDL_OK && isB2bLink == TRUE) {
                    frontPanelNumber = portNumberInGroup = 0;
                }
                else {
                    pdlStatus = pdlPpPortConvertDevPort2FrontPanel(dev, port, &frontPanelNumber, &portNumberInGroup);
                    PDL_CHECK_STATUS(pdlStatus);
                }
                pdlStatus = pdlLibConvertEnumValueToString(PDL_ENUM_ID_L1_INTERFACE_MODE_TYPE_E, (PDL_INTERFACE_MODE_ENT)interfaceMode, &interfaceStr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlLibConvertEnumValueToString(PDL_ENUM_ID_CONNECTOR_TYPE_TYPE_E, (PDL_CONNECTOR_TYPE_ENT)connectorType, &connectorStr);
                PDL_CHECK_STATUS(pdlStatus);
                pdlStatus = pdlLibConvertEnumValueToString(PDL_ENUM_ID_CONNECTOR_TECHNOLOGY_TYPE_E, (PDL_CONNECTOR_TECHNOLOGY_ENT)techType, &techStr);
				PDL_CHECK_STATUS(pdlStatus);

                IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSerdesDbFineTuneAttrGet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, dev),
                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, i),
                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, interfaceMode),
                                            IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, connectorType),
											IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(UINT_32, techType),
                                            IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneTxAttributesExists),
                                            IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneTxAttributes),
                                            IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(BOOLEAN, &fineTuneRxAttributesExists),
                                            IDBG_PDL_LOGGER_ADD_OUT_PARAM_MAC(PTR, &fineTuneRxAttributes));
                PDL_CHECK_STATUS(pdlStatus);
                if (fineTuneRxAttributesExists || fineTuneTxAttributesExists) {
                    for (field = 0; field < 7; field ++) {
                        if (field == 0) {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6d | %-4d | %-4d | %-6d | %-6d | %-14s | %-14s | %-14s | ", dev, i, port, frontPanelNumber, portNumberInGroup, interfaceStr, connectorStr, techStr);
                        }
                        else {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-14s | %-14s | %-14s | ", "", "", "", "", "", "", "", "");
                        }

                        if (fineTuneTxAttributesExists) {
                            if (field == 0) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5d | ", "Tx Amp", fineTuneTxAttributes.txAmpl);
                            }
                            else if (field == 1) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5s | ", "Tx Amp Adj Enable", fineTuneTxAttributes.txAmplAdjEn ? "true" : "false");
                            }
                            else if (field == 2) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5d | ", "emph0", fineTuneTxAttributes.emph0);
                            }
                            else if (field == 3) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5d | ", "emph1", fineTuneTxAttributes.emph1);
                            }
                            else if (field == 4) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5s | ", "Tx Amp Shift Enable", fineTuneTxAttributes.txAmplShtEn ? "true" : "false");
                            }
                            else if (field == 5) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5s | ", "Tx Emph Enable", fineTuneTxAttributes.txEmphEn ? "true" : "false");
                            }
                            else if (field == 6) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-20s: %5d | ", "Slew Rate", fineTuneTxAttributes.slewRate);
                            }
                        }
                        else {
                            IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-27s | ", "");
                        }

                        if (fineTuneRxAttributesExists) {
                            if (field == 0) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s: %d", "Sql Ch", fineTuneRxAttributes.sqlch);
                            }
                            else if (field == 1) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s: %d", "ffe Res", fineTuneRxAttributes.ffeRes);
                            }
                            else if (field == 2) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s: %d", "ffe Cap", fineTuneRxAttributes.ffeCap);
                            }
                            else if (field == 3) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s: %d", "Align90", fineTuneRxAttributes.align90);
                            }
                            else if (field == 4) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s: %d", "dcGain", fineTuneRxAttributes.dcGain);
                            }
                            else if (field == 5) {
                                IDBG_PDL_LOGGER_API_OUTPUT_LOG("%-10s: %d", "Bandwidth", fineTuneRxAttributes.bandWidth);
                            }
                        }
                        IDBG_PDL_LOGGER_API_OUTPUT_LOG("\n");
                        hasData = TRUE;
                    }
                }
            }
            if (hasData) {
                IDBG_PDL_LOGGER_API_OUTPUT_LOG(" %-6s | %-4s | %-4s | %-6s | %-6s | %-14s | %-14s | %-27s | %-14s\n", "------", "----", "----", "------", "------", "--------------", "--------------", "---------------------------", "--------------");
            }
        }
    }

    return PDL_OK;
}
/*$ END OF iDbgPdlSerdesGetFineTune */

/*****************************************************************************
* FUNCTION NAME: iDbgPdlSerdesSetDebug
*
* DESCRIPTION:
*
*
*****************************************************************************/
PDL_STATUS iDbgPdlSerdesSetDebug
(
    IN IDBG_PDL_LOGGER_RESULT_DATA_STC  * resultPtr,
    IN BOOLEAN                            enable
)
{
    /*!****************************************************************************/
    /*! L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /*!****************************************************************************/
    PDL_STATUS                      pdlStatus;
    /*!****************************************************************************/
    /*!                      F U N C T I O N   L O G I C                          */
    /*!****************************************************************************/
    PDL_UNUSED_PARAM(resultPtr);

    IDBG_PDL_LOGGER_PDL_EXECUTE(pdlStatus, pdlSerdesDebugSet, IDBG_PDL_LOGGER_ADD_IN_PARAM_MAC(BOOLEAN, enable));

    return pdlStatus;
}
