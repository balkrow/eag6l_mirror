/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlSerdes.c
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
#include <pdl/serdes/pdlSerdes.h>
#include <pdl/serdes/private/prvPdlSerdes.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <iDbgPdl/logger/iDbgPdlLogger.h>
#include <pdl/parser/private/prvPdlParser.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @struct  PRV_PDL_SERDES_DB_STC
 *
 * @brief   Serdes database.
 */

static PRV_PDLIB_DB_TYP               pdlPortSerdesDb;

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlSerdesDebugFlag)

#undef PDL_ASSERT_TRUE
#define PDL_ASSERT_TRUE(_condition) PDL_ASSERT_TRUE_GEN(_condition, prvPdlSerdesDebugFlag)

/**
 * @fn  PDL_STATUS pdlSerdesDbPolarityAttrGet( IN UINT_32 ppId, IN UINT_32 laneId, OUT PDL_LANE_POLARITY_ATTRIBUTES_STC * polarityAttributesPtr );
 *
 * @brief   Get polarity attributes of serdes
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                    devId
 * @param [in]  laneId                  serdes's absolute lane identifier.
 * @param [out] polarityAttributesPtr   lane polarity attributes.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given laneId
 * @return  PDL_BAD_OK                  lane polarity attributes found and returned
 */

PDL_STATUS pdlSerdesDbPolarityAttrGet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    OUT PDL_LANE_POLARITY_ATTRIBUTES_STC                  * polarityAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_SERDES_PARAMS_STC                                   params;
    PDL_PACKET_PROCESSOR_LIST_KEYS_STC                      ppListKey;
    PDL_PACKET_PROCESSOR_LIST_PARAMS_STC                   *ppListPtr;
    PDL_LANE_LIST_KEYS_STC                                  laneListKey;
    PDL_LANE_LIST_PARAMS_STC                               *laneListPtr;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (polarityAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "polarityAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_SERDES_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    ppListKey.ppDeviceNumber = ppId;
    pdlStatus = prvPdlibDbFind(params.packetProcessorList_PTR, &ppListKey, (void **)&ppListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListKey.laneNumber = laneId;
    pdlStatus = prvPdlibDbFind(ppListPtr->laneList_PTR, &laneListKey, (void **)&laneListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    polarityAttributesPtr->rxSwap = laneListPtr->laneAttributesGroup.rxSwap;
    polarityAttributesPtr->txSwap = laneListPtr->laneAttributesGroup.txSwap;

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(BOOLEAN, polarityAttributesPtr->txSwap);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(BOOLEAN, polarityAttributesPtr->rxSwap);

    return PDL_OK;
}
/*$ END OF pdlSerdesDbPolarityAttrGet */

/**
 * @fn  PDL_STATUS pdlSerdesDbPolarityAttrSet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_LANE_POLARITY_ATTRIBUTES_STC * polarityAttributesPtr );
 *
 * @brief   Set polarity attributes of serdes
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                    devId
 * @param [in]  laneId                  serdes's absolute lane identifier.
 * @param [in]  polarityAttributesPtr   lane polarity attributes.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given laneId
 * @return  PDL_BAD_OK                  lane polarity attributes found and returned
 */

PDL_STATUS pdlSerdesDbPolarityAttrSet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_LANE_POLARITY_ATTRIBUTES_STC                  * polarityAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_SERDES_PARAMS_STC                                   params;
    PDL_PACKET_PROCESSOR_LIST_KEYS_STC                      ppListKey;
    PDL_PACKET_PROCESSOR_LIST_PARAMS_STC                   *ppListPtr;
    PDL_LANE_LIST_KEYS_STC                                  laneListKey;
    PDL_LANE_LIST_PARAMS_STC                               *laneListPtr;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (polarityAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "polarityAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(BOOLEAN, polarityAttributesPtr->txSwap);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(BOOLEAN, polarityAttributesPtr->rxSwap);

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_SERDES_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    ppListKey.ppDeviceNumber = ppId;
    pdlStatus = prvPdlibDbFind(params.packetProcessorList_PTR, &ppListKey, (void **)&ppListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListKey.laneNumber = laneId;
    pdlStatus = prvPdlibDbFind(ppListPtr->laneList_PTR, &laneListKey, (void **)&laneListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListPtr->laneAttributesGroup.rxSwap = polarityAttributesPtr->rxSwap;
    laneListPtr->laneAttributesGroup.txSwap = polarityAttributesPtr->txSwap;

    return PDL_OK;
}
/*$ END OF pdlSerdesDbPolarityAttrSet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrSet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode IN PDL_CONNECTOR_TYPE_ENT connectorType, IN PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC * fineTuneTxAttributesPtr , IN PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC * fineTuneRxAttributesPtr );
 *
 * @brief   Set attributes of serdes for specified interfaceMode & connectorType
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [in]  interfaceMode                       interface mode.
 * @param [in]  connectorType                       connector Type
 * @param [out] fineTuneTxAttributesPtr             lane fine tune tx params or NULL.
 * @param [out] fineTuneRxAttributesPtr             lane fine tune rx params or NULL.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_OK                      lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrSet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              interfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              connectorType,
	IN  PDL_CONNECTOR_TECHNOLOGY_ENT								cableTechnology,
    IN  PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneTxAttributesPtr,
    IN  PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneRxAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_SERDES_PARAMS_STC                                   params;
    PDL_PACKET_PROCESSOR_LIST_KEYS_STC                      ppListKey;
    PDL_PACKET_PROCESSOR_LIST_PARAMS_STC                   *ppListPtr;
    PDL_LANE_LIST_KEYS_STC                                  laneListKey;
    PDL_LANE_LIST_PARAMS_STC                               *laneListPtr;
    PDL_LANE_ATTRIBUTES_INFO_LIST_KEYS_STC                  currentLaneKey;
    PDL_LANE_ATTRIBUTES_INFO_LIST_PARAMS_STC               *currentLanePtr = NULL;
    PDL_INFO_LIST_KEYS_STC                                  legacyLaneKey;
    PDL_INFO_LIST_PARAMS_STC                               *legacyLanePtr = NULL;
    BOOLEAN                                                 isTxSupported, isRxSupported;
    PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_PARAMS_STC  *serdesRxTunePtr = NULL;
    PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_KEYS_STC     serdesRxListKeyParam;
    PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_PARAMS_STC        *legacyRxTunePtr = NULL;
    PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_KEYS_STC           legacySerdesRxListKeyParam;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (fineTuneTxAttributesPtr == NULL && fineTuneRxAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "fineTuneTxAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_SERDES_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    ppListKey.ppDeviceNumber = ppId;
    pdlStatus = prvPdlibDbFind(params.packetProcessorList_PTR, &ppListKey, (void **)&ppListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListKey.laneNumber = laneId;
    pdlStatus = prvPdlibDbFind(ppListPtr->laneList_PTR, &laneListKey, (void **)&laneListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    pdlStatus = PDL_NOT_FOUND;
    if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
        currentLaneKey.connectorType = (PDL_CONNECTOR_TYPE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(connectorType);
        currentLaneKey.interfaceMode = (PDL_L1_INTERFACE_MODE_TYPE_ENT) PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(interfaceMode);
        currentLaneKey.connectorTechnology = (PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT) cableTechnology;
        pdlStatus = prvPdlibDbFind(laneListPtr->laneInformation.data.laneAttributesCurrent.laneAttributesInfoList_PTR, &currentLaneKey, (void **)&currentLanePtr);
    }
    else if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_LEGACY_V1_E) {
        if (laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributesExist) {
            legacyLaneKey.connectorType = (PDL_CONNECTOR_TYPE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(connectorType);
            legacyLaneKey.interfaceMode = (PDL_L1_INTERFACE_MODE_TYPE_ENT) PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(interfaceMode);
            legacyLaneKey.connectorTechnology = (PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT) cableTechnology;
            pdlStatus = prvPdlibDbFind(laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributes.infoList_PTR, &legacyLaneKey, (void **)&legacyLanePtr);
        }
    }

    PDL_CHECK_STATUS(pdlStatus);

    if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
        if (currentLanePtr) {
            isTxSupported = currentLanePtr->isTxFineTuneSupported;
            isRxSupported = currentLanePtr->isRxFineTuneSupported;
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "currentLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
            return PDL_BAD_PTR;
        }
    }
    else {
        if (legacyLanePtr) {
            isTxSupported = legacyLanePtr->laneAttribute.isTxFineTuneSupported;
            isRxSupported = legacyLanePtr->laneAttribute.isRxFineTuneSupported;
        }
        else {
            PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "legacyLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
            return PDL_BAD_PTR;
        }
    }

    if (isTxSupported == FALSE && fineTuneTxAttributesPtr) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "TX Attributes doesn't exist and fineTuneTxAttributesPtr is not NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (isRxSupported == FALSE && fineTuneRxAttributesPtr) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "RX Attributes doesn't exist and fineTuneRxAttributesPtr is not NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (fineTuneTxAttributesPtr) {
        if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
            if (currentLanePtr) {
                currentLanePtr->laneAttributesInfoTxFineTune.emph0       = fineTuneTxAttributesPtr->emph0;
                currentLanePtr->laneAttributesInfoTxFineTune.emph1       = fineTuneTxAttributesPtr->emph1;
                currentLanePtr->laneAttributesInfoTxFineTune.slewrate    = fineTuneTxAttributesPtr->slewRate;
                currentLanePtr->laneAttributesInfoTxFineTune.txamp       = fineTuneTxAttributesPtr->txAmpl;
                currentLanePtr->laneAttributesInfoTxFineTune.txampadjen  = fineTuneTxAttributesPtr->txAmplAdjEn;
                currentLanePtr->laneAttributesInfoTxFineTune.txampshft   = fineTuneTxAttributesPtr->txAmplShtEn;
                currentLanePtr->laneAttributesInfoTxFineTune.txemphen    = fineTuneTxAttributesPtr->txEmphEn;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "currentLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
                return PDL_BAD_PTR;
            }
        }
        else {
            if (legacyLanePtr) {
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.emph0       = fineTuneTxAttributesPtr->emph0;
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.emph1       = fineTuneTxAttributesPtr->emph1;
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.slewrate    = fineTuneTxAttributesPtr->slewRate;
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txamp       = fineTuneTxAttributesPtr->txAmpl;
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txampadjen  = fineTuneTxAttributesPtr->txAmplAdjEn;
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txampshft   = fineTuneTxAttributesPtr->txAmplShtEn;
                legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txemphen    = fineTuneTxAttributesPtr->txEmphEn;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "legacyLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
                return PDL_BAD_PTR;
            }
        }
    }
    if (fineTuneRxAttributesPtr) {
        if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
            if (currentLanePtr) {
                serdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_SQLCH_E;
                pdlStatus = prvPdlibDbFind(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, &serdesRxListKeyParam, (void **)&serdesRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d SQLCH NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                serdesRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->sqlch;

                serdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_FFERES_E;
                pdlStatus = prvPdlibDbFind(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, &serdesRxListKeyParam, (void **)&serdesRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d FFE RES NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                serdesRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->ffeRes;

                serdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_FFECAP_E;
                pdlStatus = prvPdlibDbFind(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, &serdesRxListKeyParam, (void **)&serdesRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d FFE CAP NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                serdesRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->ffeCap;

                serdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_ALIGN90_E;
                pdlStatus = prvPdlibDbFind(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, &serdesRxListKeyParam, (void **)&serdesRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d ALIGN90 NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                serdesRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->align90;

                serdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_DCGAIN_E;
                pdlStatus = prvPdlibDbFind(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, &serdesRxListKeyParam, (void **)&serdesRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d DCGAIN NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                serdesRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->dcGain;

                serdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_BANDWIDTH_E;
                pdlStatus = prvPdlibDbFind(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, &serdesRxListKeyParam, (void **)&serdesRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d BANDWIDTH NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                serdesRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->bandWidth;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "currentLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
                return PDL_BAD_PTR;
            }
        }
        else {
            if (legacyLanePtr) {
                legacySerdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_SQLCH_E;
                pdlStatus = prvPdlibDbFind(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, &legacySerdesRxListKeyParam, (void **)&legacyRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d SQLCH NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                legacyRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->sqlch;

                legacySerdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_FFERES_E;
                pdlStatus = prvPdlibDbFind(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, &legacySerdesRxListKeyParam, (void **)&legacyRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d FFE RES NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                legacyRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->ffeRes;

                legacySerdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_FFECAP_E;
                pdlStatus = prvPdlibDbFind(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, &legacySerdesRxListKeyParam, (void **)&legacyRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d FFE CAP NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                legacyRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->ffeCap;

                legacySerdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_ALIGN90_E;
                pdlStatus = prvPdlibDbFind(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, &legacySerdesRxListKeyParam, (void **)&legacyRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d ALIGN90 NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                legacyRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->align90;

                legacySerdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_DCGAIN_E;
                pdlStatus = prvPdlibDbFind(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, &legacySerdesRxListKeyParam, (void **)&legacyRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d DCGAIN NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                legacyRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->dcGain;

                legacySerdesRxListKeyParam.rxParam = PDL_RX_PARAM_TYPE_BANDWIDTH_E;
                pdlStatus = prvPdlibDbFind(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, &legacySerdesRxListKeyParam, (void **)&legacyRxTunePtr);
                if (pdlStatus == PDL_NOT_FOUND) {
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d BANDWIDTH NOT FOUND", laneId, ppId);
                    return PDL_NOT_FOUND;
                }
                legacyRxTunePtr->rxParamValue = fineTuneRxAttributesPtr->bandWidth;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "legacyLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
                return PDL_BAD_PTR;
            }
        }
    }
    return PDL_OK;
}
/*$ END OF pdlSerdesDbFineTuneAttrSet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode IN PDL_CONNECTOR_TYPE_ENT connectorType, OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC * fineTuneTxAttributesPtr );
 *
 * @brief   Get attributes of serdes for specified interfaceMode & connectorType
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [in]  interfaceMode                       interface mode.
 * @param [in]  connectorType                       connector Type
 * @param [out] fineTuneTxAttributesExistsPtr       lane fine tune tx params exist.
 * @param [out] fineTuneTxAttributesPtr             lane fine tune tx params.
 * @param [out] fineTuneRxAttributesExistsPtr       lane fine tune rx params exist.
 * @param [out] fineTuneRxAttributesPtr             lane fine tune rx params.
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_OK                      lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGet(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    IN  PDL_INTERFACE_MODE_ENT                              interfaceMode,
    IN  PDL_CONNECTOR_TYPE_ENT                              connectorType,
	IN  PDL_CONNECTOR_TECHNOLOGY_ENT								cableTechnology,
    OUT BOOLEAN                                           * fineTuneTxAttributesExistsPtr,
    OUT PDL_LANE_TX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneTxAttributesPtr,
    OUT BOOLEAN                                           * fineTuneRxAttributesExistsPtr,
    OUT PDL_LANE_RX_FINE_TUNE_ATTRIBUTES_STC              * fineTuneRxAttributesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_SERDES_PARAMS_STC                                   params;
    PDL_PACKET_PROCESSOR_LIST_KEYS_STC                      ppListKey;
    PDL_PACKET_PROCESSOR_LIST_PARAMS_STC                   *ppListPtr;
    PDL_LANE_LIST_KEYS_STC                                  laneListKey;
    PDL_LANE_LIST_PARAMS_STC                               *laneListPtr;
    PDL_LANE_ATTRIBUTES_INFO_LIST_KEYS_STC                  currentLaneKey;
    PDL_LANE_ATTRIBUTES_INFO_LIST_PARAMS_STC               *currentLanePtr;
    PDL_INFO_LIST_KEYS_STC                                  legacyLaneKey;
    PDL_INFO_LIST_PARAMS_STC                               *legacyLanePtr;
    PDL_LANE_ATTRIBUTES_INFO_RX_FINE_TUNE_LIST_PARAMS_STC  *serdesRxTunePtr = NULL;
    PDL_LANE_ATTRIBUTE_RX_FINE_TUNE_LIST_PARAMS_STC        *legacyRxTunePtr = NULL;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (fineTuneTxAttributesExistsPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "fineTuneTxAttributesExistsPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }
    if (fineTuneTxAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "fineTuneTxAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }
    if (fineTuneRxAttributesExistsPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "fineTuneRxAttributesExistsPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }
    if (fineTuneRxAttributesPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "fineTuneRxAttributesPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_SERDES_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    ppListKey.ppDeviceNumber = ppId;
    pdlStatus = prvPdlibDbFind(params.packetProcessorList_PTR, &ppListKey, (void **)&ppListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListKey.laneNumber = laneId;
    pdlStatus = prvPdlibDbFind(ppListPtr->laneList_PTR, &laneListKey, (void **)&laneListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    pdlStatus = PDL_NOT_FOUND;
    if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
        currentLaneKey.connectorType = (PDL_CONNECTOR_TYPE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(connectorType);
        currentLaneKey.interfaceMode = (PDL_L1_INTERFACE_MODE_TYPE_ENT) PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(interfaceMode);
        currentLaneKey.connectorTechnology = (PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT) cableTechnology;
        pdlStatus = prvPdlibDbFind(laneListPtr->laneInformation.data.laneAttributesCurrent.laneAttributesInfoList_PTR, &currentLaneKey, (void **)&currentLanePtr);
        if (pdlStatus == PDL_NOT_FOUND) {
            PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d INTERFACE MODE: %d CONNECTOR: %d TECHNOLOGY %d NOT FOUND", laneId, ppId, interfaceMode, connectorType, cableTechnology);
        }
        PDL_CHECK_STATUS(pdlStatus);
        *fineTuneTxAttributesExistsPtr = currentLanePtr->isTxFineTuneSupported;
        *fineTuneRxAttributesExistsPtr = currentLanePtr->isRxFineTuneSupported;
        if (currentLanePtr->isTxFineTuneSupported) {
            fineTuneTxAttributesPtr->emph0 = currentLanePtr->laneAttributesInfoTxFineTune.emph0;
            fineTuneTxAttributesPtr->emph1 = currentLanePtr->laneAttributesInfoTxFineTune.emph1;
            fineTuneTxAttributesPtr->slewRate = currentLanePtr->laneAttributesInfoTxFineTune.slewrate;
            fineTuneTxAttributesPtr->txAmpl = currentLanePtr->laneAttributesInfoTxFineTune.txamp;
            fineTuneTxAttributesPtr->txAmplAdjEn = currentLanePtr->laneAttributesInfoTxFineTune.txampadjen;
            fineTuneTxAttributesPtr->txAmplShtEn = currentLanePtr->laneAttributesInfoTxFineTune.txampshft;
            fineTuneTxAttributesPtr->txEmphEn = currentLanePtr->laneAttributesInfoTxFineTune.txemphen;
        }
        if (currentLanePtr->isRxFineTuneSupported) {
            /* get first entry */
            pdlStatus = prvPdlibDbGetFirst(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, (void**) &serdesRxTunePtr);
            PDL_CHECK_STATUS(pdlStatus);
            /* mark as default */
            fineTuneRxAttributesPtr->sqlch =
                    fineTuneRxAttributesPtr->ffeRes =
                    fineTuneRxAttributesPtr->ffeCap =
                    fineTuneRxAttributesPtr->align90 =
                    fineTuneRxAttributesPtr->dcGain =
                    fineTuneRxAttributesPtr->bandWidth = MAX_UINT_16;

            while (pdlStatus == PDL_OK) { /*search in all the connected fans*/
                switch (serdesRxTunePtr->list_keys.rxParam) {
                case PDL_RX_PARAM_TYPE_SQLCH_E:
                    fineTuneRxAttributesPtr->sqlch = serdesRxTunePtr->rxParamValue;
                    break;
                case PDL_RX_PARAM_TYPE_FFERES_E:
                    fineTuneRxAttributesPtr->ffeRes = serdesRxTunePtr->rxParamValue;
                    break;
                case PDL_RX_PARAM_TYPE_FFECAP_E:
                    fineTuneRxAttributesPtr->ffeCap = serdesRxTunePtr->rxParamValue;
                    break;
                case PDL_RX_PARAM_TYPE_ALIGN90_E:
                    fineTuneRxAttributesPtr->align90 = serdesRxTunePtr->rxParamValue;
                    break;
                case PDL_RX_PARAM_TYPE_DCGAIN_E:
                    fineTuneRxAttributesPtr->dcGain = serdesRxTunePtr->rxParamValue;
                    break;
                case PDL_RX_PARAM_TYPE_BANDWIDTH_E:
                    fineTuneRxAttributesPtr->bandWidth = serdesRxTunePtr->rxParamValue;
                    break;
                case PDL_RX_PARAM_TYPE_LAST_E:
                default:
                    PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "rxParam is unknown %d", serdesRxTunePtr->list_keys.rxParam);
                    return PDL_BAD_PTR;
                }
                /*next serdes param*/
                pdlStatus = prvPdlibDbGetNext(currentLanePtr->laneAttributesInfoRxFineTune.laneAttributesInfoRxFineTuneList_PTR, (void*) &serdesRxTunePtr->list_keys, (void**)&serdesRxTunePtr);
            }
        }
    }
    else if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_LEGACY_V1_E) {
        if (laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributesExist) {
            legacyLaneKey.connectorType = (PDL_CONNECTOR_TYPE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(connectorType);
            legacyLaneKey.interfaceMode = (PDL_L1_INTERFACE_MODE_TYPE_ENT) PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(interfaceMode);
            legacyLaneKey.connectorTechnology = (PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT) cableTechnology;
            pdlStatus = prvPdlibDbFind(laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributes.infoList_PTR, &legacyLaneKey, (void **)&legacyLanePtr);
            if (pdlStatus == PDL_NOT_FOUND) {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d INTERFACE MODE: %d CONNECTOR: %d TECHNOLOGY %d NOT FOUND", laneId, ppId, interfaceMode, connectorType, cableTechnology);
            }
            PDL_CHECK_STATUS(pdlStatus);
            *fineTuneTxAttributesExistsPtr = legacyLanePtr->laneAttribute.isTxFineTuneSupported;
            *fineTuneRxAttributesExistsPtr = legacyLanePtr->laneAttribute.isRxFineTuneSupported;
            if (legacyLanePtr->laneAttribute.isTxFineTuneSupported) {
                fineTuneTxAttributesPtr->emph0 = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.emph0;
                fineTuneTxAttributesPtr->emph1 = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.emph1;
                fineTuneTxAttributesPtr->slewRate = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.slewrate;
                fineTuneTxAttributesPtr->txAmpl = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txamp;
                fineTuneTxAttributesPtr->txAmplAdjEn = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txampadjen;
                fineTuneTxAttributesPtr->txAmplShtEn = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txampshft;
                fineTuneTxAttributesPtr->txEmphEn = legacyLanePtr->laneAttribute.laneAttributeTxFineTune.txemphen;
            }
            if (legacyLanePtr->laneAttribute.isRxFineTuneSupported) {
                /* get first entry */
                pdlStatus = prvPdlibDbGetFirst(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, (void**) &legacyRxTunePtr);
                PDL_CHECK_STATUS(pdlStatus);
                /* mark as default */
                fineTuneRxAttributesPtr->sqlch =
                        fineTuneRxAttributesPtr->ffeRes =
                        fineTuneRxAttributesPtr->ffeCap =
                        fineTuneRxAttributesPtr->align90 =
                        fineTuneRxAttributesPtr->dcGain =
                        fineTuneRxAttributesPtr->bandWidth = MAX_UINT_16;

                while (pdlStatus == PDL_OK) { /*search in all the connected fans*/
                    switch (legacyRxTunePtr->list_keys.rxParam) {
                    case PDL_RX_PARAM_TYPE_SQLCH_E:
                        fineTuneRxAttributesPtr->sqlch = legacyRxTunePtr->rxParamValue;
                        break;
                    case PDL_RX_PARAM_TYPE_FFERES_E:
                        fineTuneRxAttributesPtr->ffeRes = legacyRxTunePtr->rxParamValue;
                        break;
                    case PDL_RX_PARAM_TYPE_FFECAP_E:
                        fineTuneRxAttributesPtr->ffeCap = legacyRxTunePtr->rxParamValue;
                        break;
                    case PDL_RX_PARAM_TYPE_ALIGN90_E:
                        fineTuneRxAttributesPtr->align90 = legacyRxTunePtr->rxParamValue;
                        break;
                    case PDL_RX_PARAM_TYPE_DCGAIN_E:
                        fineTuneRxAttributesPtr->dcGain = legacyRxTunePtr->rxParamValue;
                        break;
                    case PDL_RX_PARAM_TYPE_BANDWIDTH_E:
                        fineTuneRxAttributesPtr->bandWidth = legacyRxTunePtr->rxParamValue;
                        break;
                    case PDL_RX_PARAM_TYPE_LAST_E:
                    default:
                        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "rxParam is unknown %d", legacyRxTunePtr->list_keys.rxParam);
                        return PDL_BAD_PTR;
                    }
                    /*next serdes param*/
                    pdlStatus = prvPdlibDbGetNext(legacyLanePtr->laneAttribute.laneAttributeRxFineTune.laneAttributeRxFineTuneList_PTR, (void*) &legacyRxTunePtr->list_keys, (void**)&legacyRxTunePtr);
                }
            }
        }
    }

    return PDL_OK;
}
/*$ END OF pdlSerdesDbFineTuneAttrGet */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGetFirst( IN UINT_32 ppId, IN UINT_32 laneId, OUT PDL_INTERFACE_MODE_ENT *interfaceModePtr, OUT PDL_CONNECTOR_TYPE_ENT *connectorTypePtr );
 *
 * @brief   Get serdes first interfaceMode and connectorType 's
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                devId
 * @param [in]  laneId                              serdes's absolute lane identifier.
 * @param [out] interfaceModePtr                    first interface mode.
 * @param [out] connectorTypePtr                    first connector Type
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGetFirst(
    IN  UINT_32                                             ppId,
    IN  UINT_32                                             laneId,
    OUT PDL_INTERFACE_MODE_ENT                             *interfaceModePtr,
    OUT PDL_CONNECTOR_TYPE_ENT                             *connectorTypePtr,
	OUT PDL_CONNECTOR_TECHNOLOGY_ENT					   *cableTechnologyPtr

)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_SERDES_PARAMS_STC                                   params;
    PDL_PACKET_PROCESSOR_LIST_KEYS_STC                      ppListKey;
    PDL_PACKET_PROCESSOR_LIST_PARAMS_STC                   *ppListPtr;
    PDL_LANE_LIST_KEYS_STC                                  laneListKey;
    PDL_LANE_LIST_PARAMS_STC                               *laneListPtr;
    PDL_LANE_ATTRIBUTES_INFO_LIST_PARAMS_STC               *currentLanePtr  = NULL;
    PDL_INFO_LIST_PARAMS_STC                               *legacyLanePtr = NULL;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (interfaceModePtr == NULL || connectorTypePtr == NULL || cableTechnologyPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "interfaceModePtr/connectorTypePtr/cableTechnologyPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_SERDES_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    ppListKey.ppDeviceNumber = ppId;
    pdlStatus = prvPdlibDbFind(params.packetProcessorList_PTR, &ppListKey, (void **)&ppListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListKey.laneNumber = laneId;
    pdlStatus = prvPdlibDbFind(ppListPtr->laneList_PTR, &laneListKey, (void **)&laneListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    pdlStatus = PDL_NOT_FOUND;
    if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
        pdlStatus = prvPdlibDbGetFirst(laneListPtr->laneInformation.data.laneAttributesCurrent.laneAttributesInfoList_PTR, (void **)&currentLanePtr);
    }
    else if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_LEGACY_V1_E) {
        if (laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributesExist) {
            pdlStatus = prvPdlibDbGetFirst(laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributes.infoList_PTR, (void **)&legacyLanePtr);
        }
    }

    if (pdlStatus == PDL_OK) {
        if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
            if (currentLanePtr) {
                *interfaceModePtr = (PDL_INTERFACE_MODE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(currentLanePtr->list_keys.interfaceMode);
                *connectorTypePtr = (PDL_CONNECTOR_TYPE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(currentLanePtr->list_keys.connectorType);
                *cableTechnologyPtr = (PDL_CONNECTOR_TECHNOLOGY_ENT)currentLanePtr->list_keys.connectorTechnology;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "currentLanePtr is NULL");
                return PDL_BAD_PTR;
            }
        }
        else {
            if (legacyLanePtr) {
                *interfaceModePtr = (PDL_INTERFACE_MODE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(legacyLanePtr->list_keys.interfaceMode);
                *connectorTypePtr = (PDL_CONNECTOR_TYPE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(legacyLanePtr->list_keys.connectorType);
                *cableTechnologyPtr = (PDL_CONNECTOR_TECHNOLOGY_ENT)legacyLanePtr->list_keys.connectorTechnology;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "legacyLanePtr is NULL");
                return PDL_BAD_PTR;
            }
        }
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "CAN'T FIND PDL");
    }
    return pdlStatus;
}
/*$ END OF pdlSerdesDbFineTuneAttrGetFirst */

/**
 * @fn  PDL_STATUS pdlSerdesDbFineTuneAttrGetNext( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT currInterfaceMode, IN PDL_CONNECTOR_TYPE_ENT currConnectorType, OUT PDL_INTERFACE_MODE_ENT *nextInterfaceModePtr, OUT PDL_CONNECTOR_TYPE_ENT *nextConnectorTypePtr );
 *
 * @brief   Get serdes next interfaceMode and connectorType 's
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  ppId                                    devId
 * @param [in]  laneId                                  serdes's absolute lane identifier.
 * @param [in]  currInterfaceMode                       current interface mode.
 * @param [in]  currConnectorType                       current connector Type
 * @param [out] nextInterfaceModePtr                    next interface mode.
 * @param [out] nextConnectorTypePtr                    next connector Type
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given interfaceMode & connectorType
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesDbFineTuneAttrGetNext(
	IN  UINT_32                                             ppId,
	IN  UINT_32                                             laneId,
	IN  PDL_INTERFACE_MODE_ENT                              currInterfaceMode,
	IN  PDL_CONNECTOR_TYPE_ENT                              currConnectorType,
	IN  PDL_CONNECTOR_TECHNOLOGY_ENT					    currcableTechnology,
	OUT PDL_INTERFACE_MODE_ENT                             *nextInterfaceModePtr,
	OUT PDL_CONNECTOR_TYPE_ENT                             *nextConnectorTypePtr,
	OUT PDL_CONNECTOR_TECHNOLOGY_ENT					   *nextcableTechnologyPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                              pdlStatus;
    PDL_SERDES_PARAMS_STC                                   params;
    PDL_PACKET_PROCESSOR_LIST_KEYS_STC                      ppListKey;
    PDL_PACKET_PROCESSOR_LIST_PARAMS_STC                   *ppListPtr;
    PDL_LANE_LIST_KEYS_STC                                  laneListKey;
    PDL_LANE_LIST_PARAMS_STC                               *laneListPtr;
    PDL_LANE_ATTRIBUTES_INFO_LIST_KEYS_STC                  currentLaneKey;
    PDL_LANE_ATTRIBUTES_INFO_LIST_PARAMS_STC               *currentLanePtr;
    PDL_INFO_LIST_KEYS_STC                                  legacyLaneKey;
    PDL_INFO_LIST_PARAMS_STC                               *legacyLanePtr;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (nextInterfaceModePtr == NULL || nextConnectorTypePtr == NULL || nextcableTechnologyPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "nextInterfaceModePtr/nextConnectorTypePtr/nextcableTechnologyPtr NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
        return PDL_BAD_PTR;
    }

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_SERDES_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    ppListKey.ppDeviceNumber = ppId;
    pdlStatus = prvPdlibDbFind(params.packetProcessorList_PTR, &ppListKey, (void **)&ppListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    laneListKey.laneNumber = laneId;
    pdlStatus = prvPdlibDbFind(ppListPtr->laneList_PTR, &laneListKey, (void **)&laneListPtr);
    if (pdlStatus == PDL_NOT_FOUND) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "LANE ID: %d, DEVICE ID: %d NOT FOUND", laneId, ppId);
    }
    PDL_CHECK_STATUS(pdlStatus);

    /* now find tune params relevant to connector type & interface mode */
    pdlStatus = PDL_NOT_FOUND;
    if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
        currentLaneKey.connectorType = (PDL_CONNECTOR_TYPE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(currConnectorType);
        currentLaneKey.interfaceMode = (PDL_L1_INTERFACE_MODE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(currInterfaceMode);
        currentLaneKey.connectorTechnology = (PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT)currcableTechnology;
        pdlStatus = prvPdlibDbGetNext(laneListPtr->laneInformation.data.laneAttributesCurrent.laneAttributesInfoList_PTR, &currentLaneKey, (void **)&currentLanePtr);
    }
    else if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_LEGACY_V1_E) {
        if (laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributesExist) {
            legacyLaneKey.connectorType = (PDL_CONNECTOR_TYPE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(currConnectorType);
            legacyLaneKey.interfaceMode = (PDL_L1_INTERFACE_MODE_TYPE_ENT)PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(currInterfaceMode);
            legacyLaneKey.connectorTechnology = (PDL_CONNECTOR_TECHNOLOGY_TYPE_ENT)currcableTechnology;
            pdlStatus = prvPdlibDbGetNext(laneListPtr->laneInformation.data.laneAttributesLegacyV1.laneAttributes.infoList_PTR, &legacyLaneKey, (void **)&legacyLanePtr);
        }
    }

    if (pdlStatus == PDL_OK) {
        if (laneListPtr->laneInformation.type == PDL_LANE_INFORMATION_LANE_ATTRIBUTES_CURRENT_E) {
            if (currentLanePtr) {
                *nextInterfaceModePtr = (PDL_INTERFACE_MODE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(currentLanePtr->list_keys.interfaceMode);
                *nextConnectorTypePtr = (PDL_CONNECTOR_TYPE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(currentLanePtr->list_keys.connectorType);
                *nextcableTechnologyPtr = (PDL_CONNECTOR_TECHNOLOGY_ENT)currentLanePtr->list_keys.connectorTechnology;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "currentLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
                return PDL_BAD_PTR;
            }
        }
        else {
            if (legacyLanePtr) {
                *nextInterfaceModePtr = (PDL_INTERFACE_MODE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(legacyLanePtr->list_keys.interfaceMode);
                *nextConnectorTypePtr = (PDL_CONNECTOR_TYPE_ENT)PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(legacyLanePtr->list_keys.connectorType);
                *nextcableTechnologyPtr = (PDL_CONNECTOR_TECHNOLOGY_ENT)legacyLanePtr->list_keys.connectorTechnology;
            }
            else {
                PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "legacyLanePtr is NULL, LANE ID: %d, DEVICE ID: %d", laneId, ppId);
                return PDL_BAD_PTR;
            }
        }
    }
    else {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "CAN'T FIND PDL");
    }
    return pdlStatus;
}
/*$ END OF pdlSerdesDbFineTuneAttrGetNext */

/**
 * @fn  PDL_STATUS pdlSerdesPortDbSerdesGetFirst( IN UINT_32 dev, IN UINT_32 port, OUT PDL_PORT_LANE_DATA_STC * serdesInfo );
 *
 * @brief   Get info for first serdes
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  dev                     packet processor identifier.
 * @param [in]  port                    port
 * @param [out] serdesInfo              first serdes connected to port
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find lane attributes for given dev & port
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesPortDbSerdesInfoGetFirst(
    IN  UINT_32                                             dev,
    IN  UINT_32                                             port,
    OUT PDL_PORT_LANE_DATA_STC                            * serdesInfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PRV_PDL_PORT_KEY_STC                   portKey;
    PRV_PDL_PORT_LANE_ENTRY_STC           *portEntryPtr;
    PDL_STATUS                             pdlStatus;
    PDL_SERDES_LANE_LIST_PARAMS_STC       *currentLaneEntryPtr = NULL;
    PDL_PORT_SERDES_LIST_PARAMS_STC       *legacyLaneEntryPtr = NULL;

    PDL_LEFT_SERDES_LANE_LIST_PARAMS_STC  *b2bCurrentLeftPortPtr;
    PDL_RIGHT_SERDES_LANE_LIST_PARAMS_STC *b2bCurrentRightPortPtr;
    /*****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (serdesInfoPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "serdesInfoPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", port, dev);
        return PDL_BAD_PTR;
    }

    portKey.dev = dev;
    portKey.port = port;
    pdlStatus = prvPdlibDbFind(pdlPortSerdesDb, &portKey, (void **)&portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = PDL_NOT_FOUND;
    if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_NETWORK_PORT_E) {
        if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.type == PDL_SERDES_LANE_INFORMATION_SERDES_CURRENT_E) {
            pdlStatus = prvPdlibDbGetFirst(portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.data.serdesCurrent.serdesLaneList_PTR, (void**)&currentLaneEntryPtr);
        }
        else if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.type == PDL_SERDES_LANE_INFORMATION_SERDES_LEGACY_V1_E) {
            if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.data.serdesLegacyV1.serdesExist) {
                pdlStatus = prvPdlibDbGetFirst(portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.data.serdesLegacyV1.serdesGroup.portSerdesList_PTR, (void**)&legacyLaneEntryPtr);
            }
        }
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_LEFT_PORT_E) {
        pdlStatus = prvPdlibDbGetFirst(portEntryPtr->data.b2bPortSerdesInfoPtr, (void**)&b2bCurrentLeftPortPtr);
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_RIGHT_PORT_E) {
        pdlStatus = prvPdlibDbGetFirst(portEntryPtr->data.b2bPortSerdesInfoPtr, (void**)&b2bCurrentRightPortPtr);
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_NETWORK_PORT_E) {
        if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.type == PDL_SERDES_LANE_INFORMATION_SERDES_CURRENT_E) {
            PDL_ASSERT_TRUE(currentLaneEntryPtr != NULL);
            serdesInfoPtr->absSerdesNum = currentLaneEntryPtr->list_keys.laneNumber;
            serdesInfoPtr->relSerdesNum = currentLaneEntryPtr->portLaneNumber;
        }
        else {
            PDL_ASSERT_TRUE(legacyLaneEntryPtr != NULL);
            serdesInfoPtr->absSerdesNum = legacyLaneEntryPtr->list_keys.laneNumber;
            serdesInfoPtr->relSerdesNum = legacyLaneEntryPtr->portLaneNumber;
        }
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_LEFT_PORT_E) {
        serdesInfoPtr->absSerdesNum = b2bCurrentLeftPortPtr->list_keys.laneNumber;
        serdesInfoPtr->relSerdesNum = b2bCurrentLeftPortPtr->portLaneNumber;
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_RIGHT_PORT_E) {
        serdesInfoPtr->absSerdesNum = b2bCurrentRightPortPtr->list_keys.laneNumber;
        serdesInfoPtr->relSerdesNum = b2bCurrentRightPortPtr->portLaneNumber;
    }

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_32, serdesInfoPtr->absSerdesNum);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesInfoPtr->relSerdesNum);
    return PDL_OK;
}

/*$ END OF pdlSerdesPortDbSerdesInfoGetFirst */


/**
 * @fn  PDL_STATUS pdlSerdesPortDbSerdesInfoGet( IN UINT_32 ppId, IN UINT_32 laneId, IN PDL_INTERFACE_MODE_ENT interfaceMode, OUT PDL_L1_INTERFACE_CONNECTED_PORTS_GROUP_DATA_STC * portsGroupAttributesPtr );
 *
 * @brief   Get port serdes information for dev & port
 *
 *          @note Based on data retrieved from XML
 *
 * @param [in]  dev                     packet processor identifier.
 * @param [in]  port                    port
 * @param [in]  serdesInfoPtr           current serdes info
 * @param [out] serdesNextInfoPtr       next port's serdes info
 *
 * @return  PDL_BAD_PTR                 bad pointer
 * @return  PDL_NOT_FOUND               can't find port index for given dev & port
 * @return  PDL_BAD_OK                  lane attributes found and returned
 */
PDL_STATUS pdlSerdesPortDbSerdesInfoGetNext(
    IN  UINT_32                                             dev,
    IN  UINT_32                                             port,
    IN  PDL_PORT_LANE_DATA_STC                            * serdesInfoPtr,
    OUT PDL_PORT_LANE_DATA_STC                            * serdesNextInfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PRV_PDL_PORT_KEY_STC                   portKey;
    PRV_PDL_PORT_LANE_ENTRY_STC           *portEntryPtr;
    PDL_STATUS                             pdlStatus;
    PDL_SERDES_LANE_LIST_PARAMS_STC       *currentLaneEntryPtr = NULL;
    PDL_SERDES_LANE_LIST_KEYS_STC          currentLaneKey;
    PDL_PORT_SERDES_LIST_PARAMS_STC       *legacyLaneEntryPtr = NULL;
    PDL_PORT_SERDES_LIST_KEYS_STC          legacyLaneKey;

    PDL_LEFT_SERDES_LANE_LIST_KEYS_STC     b2bCurrentLeftPortKey;
    PDL_LEFT_SERDES_LANE_LIST_PARAMS_STC  *b2bCurrentLeftPortEntryPtr;
    PDL_RIGHT_SERDES_LANE_LIST_KEYS_STC    b2bCurrentRightPortKey;
    PDL_RIGHT_SERDES_LANE_LIST_PARAMS_STC *b2bCurrentRightPortEntryPtr;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (serdesInfoPtr == NULL || serdesNextInfoPtr == NULL) {
        PDL_LIB_DEBUG_MAC(prvPdlSerdesDebugFlag)(__FUNCTION__, __LINE__, "serdesInfoPtr OR serdesNextInfoPtr NULL, PORT NUMBER: %d, DEVICE NUMBER: %d", port, dev);
        return PDL_BAD_PTR;
    }
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_8, serdesInfoPtr->absSerdesNum);
    IDBG_PDL_LOGGER_PDL_IN_PARAM_MAC(UINT_8, serdesInfoPtr->relSerdesNum);

    portKey.dev = dev;
    portKey.port = port;
    pdlStatus = prvPdlibDbFind(pdlPortSerdesDb, &portKey, (void **)&portEntryPtr);
    PDL_CHECK_STATUS(pdlStatus);

    pdlStatus = PDL_NOT_FOUND;
    if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_NETWORK_PORT_E) {
        if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.type == PDL_SERDES_LANE_INFORMATION_SERDES_CURRENT_E) {
            currentLaneKey.laneNumber = serdesInfoPtr->absSerdesNum;
            pdlStatus = prvPdlibDbGetNext(portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.data.serdesCurrent.serdesLaneList_PTR, &currentLaneKey, (void**)&currentLaneEntryPtr);
        }
        else if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.type == PDL_SERDES_LANE_INFORMATION_SERDES_LEGACY_V1_E) {
            if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.data.serdesLegacyV1.serdesExist) {
                legacyLaneKey.laneNumber = serdesInfoPtr->absSerdesNum;
                pdlStatus = prvPdlibDbGetNext(portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.data.serdesLegacyV1.serdesGroup.portSerdesList_PTR, &legacyLaneKey, (void**)&legacyLaneEntryPtr);
            }
        }
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_LEFT_PORT_E) {
        b2bCurrentLeftPortKey.laneNumber = serdesInfoPtr->absSerdesNum;
        pdlStatus = prvPdlibDbGetNext(portEntryPtr->data.b2bPortSerdesInfoPtr, (void*)&b2bCurrentLeftPortKey, (void**)&b2bCurrentLeftPortEntryPtr);
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_RIGHT_PORT_E) {
        b2bCurrentRightPortKey.laneNumber = serdesInfoPtr->absSerdesNum;
        pdlStatus = prvPdlibDbGetNext(portEntryPtr->data.b2bPortSerdesInfoPtr, (void*)&b2bCurrentRightPortKey, (void**)&b2bCurrentRightPortEntryPtr);
    }
    PDL_CHECK_STATUS(pdlStatus);

    if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_NETWORK_PORT_E) {
        if (portEntryPtr->data.portSerdesInfoPtr->serdesLaneInformation.type == PDL_SERDES_LANE_INFORMATION_SERDES_CURRENT_E) {
            PDL_ASSERT_TRUE(currentLaneEntryPtr != NULL);
            serdesNextInfoPtr->absSerdesNum = currentLaneEntryPtr->list_keys.laneNumber;
            serdesNextInfoPtr->relSerdesNum = currentLaneEntryPtr->portLaneNumber;
        }
        else {
            PDL_ASSERT_TRUE(legacyLaneEntryPtr != NULL);
            serdesNextInfoPtr->absSerdesNum = legacyLaneEntryPtr->list_keys.laneNumber;
            serdesNextInfoPtr->relSerdesNum = legacyLaneEntryPtr->portLaneNumber;
        }
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_LEFT_PORT_E) {
        serdesNextInfoPtr->absSerdesNum = b2bCurrentLeftPortEntryPtr->list_keys.laneNumber;
        serdesNextInfoPtr->relSerdesNum = b2bCurrentLeftPortEntryPtr->portLaneNumber;
    }
    else if (portEntryPtr->type == PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_RIGHT_PORT_E) {
        serdesNextInfoPtr->absSerdesNum = b2bCurrentRightPortEntryPtr->list_keys.laneNumber;
        serdesNextInfoPtr->relSerdesNum = b2bCurrentRightPortEntryPtr->portLaneNumber;
    }

    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesNextInfoPtr->absSerdesNum);
    IDBG_PDL_LOGGER_PDL_OUT_PARAM_MAC(UINT_8, serdesNextInfoPtr->relSerdesNum);

    return PDL_OK;
}
/*$ END OF pdlSerdesPortDbSerdesInfoGetNext */

/**
 * @fn  PDL_STATUS pdlSerdesDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Serdes debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSerdesDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlSerdesDebugFlag = state;
    return PDL_OK;
}
/*$ END OF pdlSerdesDebugSet */

/**
 * @fn  PDL_STATUS pdlSerdesInit ( void )
 *
 * @brief   Init serdes module Build serdes DB from XML data
 *
 * @return  PDL_NOT_FOUND if xml parsing wasn't successful because mandatory tag not found.
 * @return  PDL_CREATE_ERROR if number of packet processors doesn't match xml parsing.
 */
PDL_STATUS pdlSerdesInit(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                              pdlStatus, pdlStatus2;
    PRV_PDLIB_DB_ATTRIBUTES_STC               dbAttributes;
    PDL_NETWORK_PORTS_PARAMS_STC            params;
    PDL_FRONT_PANEL_GROUP_LIST_PARAMS_STC  *frontPanelGroupPtr;
    PDL_PORT_LIST_PARAMS_STC               *frontPanelPortEntryPtr;
    PRV_PDL_PORT_KEY_STC                    portKey;
    PRV_PDL_PORT_LANE_ENTRY_STC             portEntry, *portEntryPtr;

    PDL_BACK_TO_BACK_INFORMATION_PARAMS_STC b2bParams;
    PDL_BACK_TO_BACK_PORT_LIST_PARAMS_STC  *b2bCurrentPtr;
    /****************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    dbAttributes.listAttributes.keySize        = sizeof(PRV_PDL_PORT_KEY_STC);
    dbAttributes.listAttributes.entrySize      = sizeof(PRV_PDL_PORT_LANE_ENTRY_STC);
    pdlStatus = prvPdlibDbCreate(PRV_PDLIB_DB_TYPE_LIST_E, &dbAttributes, &pdlPortSerdesDb);
    PDL_CHECK_STATUS(pdlStatus);

    if (PDL_OK !=  pdlProjectParamsGet(PDL_FEATURE_ID_NETWORK_PORTS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params)) {
        return PDL_NOT_SUPPORTED;
    }

    /* create dev/port mapping to serdes lane info */
    pdlStatus = prvPdlibDbGetFirst(params.frontPanelGroupList_PTR, (void **)&frontPanelGroupPtr);
    while (pdlStatus == PDL_OK) {
        pdlStatus2 = prvPdlibDbGetFirst(frontPanelGroupPtr->portList_PTR, (void **)&frontPanelPortEntryPtr);
        while (pdlStatus2 == PDL_OK) {
            portKey.dev = frontPanelPortEntryPtr->ppDeviceNumber;
			if (pdlIsFieldHasValue(frontPanelPortEntryPtr->logicalPortNumber_mask)) {
				portKey.port = frontPanelPortEntryPtr->logicalPortNumber;
			}
			else {
				portKey.port = frontPanelPortEntryPtr->ppPortNumber;
			}
            memset(&portEntry, 0, sizeof(portEntry));
            portEntry.key = portKey;
            portEntry.type = PRV_PDL_PORT_LANE_ENTRY_TYPE_NETWORK_PORT_E;
            portEntry.data.portSerdesInfoPtr = &frontPanelPortEntryPtr->serdesGroupType;
            pdlStatus2 = prvPdlibDbAdd(pdlPortSerdesDb, (void *)&portKey, (void *)&portEntry, (void **)&portEntryPtr);
            PDL_CHECK_STATUS(pdlStatus2);
            pdlStatus2 = prvPdlibDbGetNext(frontPanelGroupPtr->portList_PTR, (void *)&frontPanelPortEntryPtr->list_keys, (void **)&frontPanelPortEntryPtr);
        }

        pdlStatus = prvPdlibDbGetNext(params.frontPanelGroupList_PTR, (void *)&frontPanelGroupPtr->list_keys, (void **)&frontPanelGroupPtr);
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_BACK_TO_BACK_INFORMATION_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&b2bParams);
    if (pdlStatus == PDL_OK) {
        if (b2bParams.type == PDL_BACK_TO_BACK_INFORMATION_BACK_TO_BACK_CURRENT_E) {
            pdlStatus = prvPdlibDbGetFirst(b2bParams.data.backToBackCurrent.backToBackLinkInformation.backToBackPortList_PTR, (void **)&b2bCurrentPtr);
        }
        else {
            pdlStatus = PDL_NOT_SUPPORTED;
        }
    }

    while (pdlStatus == PDL_OK) {
        if (pdlIsFieldHasValue(b2bCurrentPtr->leftSerdesLaneList_mask)) {
            portKey.dev = b2bParams.data.backToBackCurrent.backToBackLinkInformation.leftPpDeviceNumber;
            portKey.port = b2bCurrentPtr->list_keys.leftPpPortNumber;
            memset(&portEntry, 0, sizeof(portEntry));
            portEntry.key = portKey;
            portEntry.type = PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_LEFT_PORT_E;
            portEntry.data.b2bPortSerdesInfoPtr = b2bCurrentPtr->leftSerdesLaneList_PTR;
            pdlStatus2 = prvPdlibDbAdd(pdlPortSerdesDb, (void *)&portKey, (void *)&portEntry, (void **)&portEntryPtr);
            PDL_CHECK_STATUS(pdlStatus2);
        }
        if (pdlIsFieldHasValue(b2bCurrentPtr->rightSerdesLaneList_mask)) {
            portKey.dev = b2bParams.data.backToBackCurrent.backToBackLinkInformation.rightPpDeviceNumber;
            portKey.port = b2bCurrentPtr->rightPpPortNumber;
            memset(&portEntry, 0, sizeof(portEntry));
            portEntry.key = portKey;
            portEntry.type = PRV_PDL_PORT_LANE_ENTRY_TYPE_B2B_RIGHT_PORT_E;
            portEntry.data.b2bPortSerdesInfoPtr = b2bCurrentPtr->rightSerdesLaneList_PTR;
            pdlStatus2 = prvPdlibDbAdd(pdlPortSerdesDb, (void *)&portKey, (void *)&portEntry, (void **)&portEntryPtr);
            PDL_CHECK_STATUS(pdlStatus2);
        }

        pdlStatus = prvPdlibDbGetNext(b2bParams.data.backToBackCurrent.backToBackLinkInformation.backToBackPortList_PTR, (void *)&b2bCurrentPtr->list_keys, (void **)&b2bCurrentPtr);
    }

    return PDL_OK;
}
/*$ END OF pdlSerdesInit */

/**
 * @fn  PDL_STATUS prvPdlSerdesDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSerdesDestroy(
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
    pdlStatus  = prvPdlibDbDestroy(pdlPortSerdesDb);
    return pdlStatus;
}

/*$ END OF prvPdlSerdesDestroy */


