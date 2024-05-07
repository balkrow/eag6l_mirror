/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlCpu.c
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
 * @brief Platform driver layer - CPU related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdlib/xml/private/prvXmlParser.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/cpu/pdlCpu.h>
#include <pdl/cpu/private/prvPdlCpu.h>
#include <pdl/cpu/pdlCpuDebug.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/** @brief   Type of the prv pdl CPU */
static PDL_CPU_TYPE_TYPE_ENT         prvPdlCpuType = PDL_CPU_TYPE_TYPE_LAST_E;
/** @brief   Type of the prv pdl CPU family */
static PDL_CPU_FAMILY_TYPE_ENT  prvPdlCpuFamilyType;

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlCpuDebugFlag)

/** @brief   The cpu family string to enum pairs */
static PRV_PDL_LIB_STR_TO_ENUM_STC prvPdlCpuFamilyStrToEnumPairs[] = {
    {"MSYS", PDL_CPU_FAMILY_TYPE_MSYS_E },
    {"AXP", PDL_CPU_FAMILY_TYPE_AXP_E  },
    {"A38X", PDL_CPU_FAMILY_TYPE_A38X_E },
};
/** @brief   The xml error string to enum */
PRV_PDL_LIB_STR_TO_ENUM_CONVERT_STC prvPdlCpuFamilyStrToEnum = {prvPdlCpuFamilyStrToEnumPairs, sizeof(prvPdlCpuFamilyStrToEnumPairs)/sizeof(PRV_PDL_LIB_STR_TO_ENUM_STC)};

/**
 * @fn  PDL_STATUS pdlCpuTypeGet ( OUT PDL_CPU_TYPE_TYPE_ENT * cpuTypePtr )
 *
 * @brief   Get cpu type
 *
 * @param [out] cpuTypePtr  cpu type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuTypeGet(
    OUT PDL_CPU_TYPE_TYPE_ENT          * cpuTypePtr
)
{
    if (prvPdlCpuType == PDL_CPU_TYPE_TYPE_LAST_E) {
        return PDL_NOT_INITIALIZED;
    }

    if (cpuTypePtr == NULL) {
        return PDL_BAD_PTR;
    }

    * cpuTypePtr = prvPdlCpuType;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlCpuTypeSet ( IN PDL_CPU_TYPE_TYPE_ENT cpuType )
 *
 * @brief   Set cpu type
 *
 * @param [in] cpuType  cpu type.
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlCpuTypeSet(
    IN PDL_CPU_TYPE_TYPE_ENT          cpuType
)
{
    if (prvPdlCpuType == PDL_CPU_TYPE_TYPE_LAST_E) {
        return PDL_NOT_INITIALIZED;
    }

    switch (cpuType) {
        /* MSYS Family */
        case PDL_CPU_TYPE_TYPE_XP_EMBEDDED_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_MSYS_E;
            break;
        case PDL_CPU_TYPE_TYPE_ARMADA_MV78130_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_MV78160_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_MV78230_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_MV78260_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_MV78460_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_AXP_E;
            break;
        case PDL_CPU_TYPE_TYPE_ARMADA_88F6810_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_88F6811_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_88F6820_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_88F6821_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_88F6W21_E:
        case PDL_CPU_TYPE_TYPE_ARMADA_88F6828_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_A38X_E;
            break;
        case PDL_CPU_TYPE_TYPE_ARMV8_A55_E:
            prvPdlCpuFamilyType = PDL_CPU_FAMILY_TYPE_ARMv8_E;
            break;
        default:
            prvPdlCpuType = PDL_CPU_TYPE_TYPE_LAST_E;
            return PDL_BAD_PARAM;
    }

    prvPdlCpuType = cpuType;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlCpuFamilyTypeGet ( OUT PDL_CPU_FAMILY_TYPE_ENT * cpuFamilyTypePtr )
 *
 * @brief   Get cpu family type
 *
 * @param [out] cpuFamilyTypePtr    cpu family type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuFamilyTypeGet(
    OUT PDL_CPU_FAMILY_TYPE_ENT          * cpuFamilyTypePtr
)
{
    if (prvPdlCpuType == PDL_CPU_TYPE_TYPE_LAST_E) {
        return PDL_NOT_INITIALIZED;
    }

    * cpuFamilyTypePtr = prvPdlCpuFamilyType;

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlCpuNumOfUsbDevicesGet (  OUT UINT_32 * numOfUsbDevicesPtr )
 *
 * @brief   Get number of USB devices on board
 *
 * @param [out] numOfUsbDevicesPtr - number of usb devices
 *
 * @return  PDL_STATUS.
 */
extern PDL_STATUS pdlCpuNumOfUsbDevicesGet(
    OUT UINT_32          * numOfUsbDevicesPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_CPU_INFO_TYPE_PARAMS_STC                params;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    if (numOfUsbDevicesPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_CPU_INFO_TYPE_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);

    if (pdlStatus == PDL_NOT_SUPPORTED || (pdlStatus == PDL_OK && params.type == PDL_CPU_INFO_TYPE_CPU_INFO_TYPE_LEGACY_E)) {
        *numOfUsbDevicesPtr = 0;
        return PDL_OK;
    }

    PDL_CHECK_STATUS(pdlStatus);

    * numOfUsbDevicesPtr = params.data.cpuInfoTypeCurrent.cpuCurrentInfo.numOfUsbDevices;

    return PDL_OK;
}
/*$ END OF pdlCpuNumOfUsbDevicesGet */

/**
 * @fn  PDL_STATUS pdlCpuDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Cpu debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlCpuDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlCpuDebugFlag = state;
    return PDL_OK;
}

/*$ END OF pdlCpuDebugSet */

/**
 * @fn  PDL_STATUS pdlCpuSdmaMapDbGetFirst (  OUT PDL_CPU_SDMA_MAP_INFO_STC * cpuSdmaMapInfoPtr )
 *
 * @brief   Get first sdma mapping info
 *
 * @param [out] cpuSdmaMapInfoPtr - sdma mapping information
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlCpuSdmaMapDbGetFirst(
    OUT PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapInfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_CPU_SDMA_PORT_MAPPING_PARAMS_STC        params;
    PDL_CPU_SDMA_PORT_LIST_PARAMS_STC         * tempCpuSdmaMapInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (cpuSdmaMapInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_CPU_SDMA_PORT_MAPPING_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "CPU SDMA MAPPING FEATURE DOESNT EXIST");
        return pdlStatus;
    }

    pdlStatus = prvPdlibDbGetFirst(params.cpuSdmaPortList_PTR, (void **)&tempCpuSdmaMapInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    memset(cpuSdmaMapInfoPtr, 0, sizeof(PDL_CPU_SDMA_MAP_INFO_STC));
    cpuSdmaMapInfoPtr->key.index = (UINT_32)tempCpuSdmaMapInfoPtr->list_keys.cpuSdmaPortIndex;
    cpuSdmaMapInfoPtr->dev = tempCpuSdmaMapInfoPtr->ppDeviceNumber;
    cpuSdmaMapInfoPtr->macPort = tempCpuSdmaMapInfoPtr->ppPortNumber;
    cpuSdmaMapInfoPtr->logicalPort = tempCpuSdmaMapInfoPtr->logicalPortNumber;

    return PDL_OK;
}
/*$ END OF pdlCpuSdmaMapDbGetFirst */

/**
 * @fn  PDL_STATUS pdlCpuSdmaMapDbGetNext ( IN  PDL_CPU_SDMA_MAP_INFO_STC * cpuSdmaMapInfoPtr, OUT PDL_CPU_SDMA_MAP_INFO_STC * cpuSdmaMapNextInfoPtr )
 *
 * @brief   Get next sdma mapping info
 *
 * @param [out] cpuSdmaMapInfoPtr     - pointer to current sdma mapping information
 * @param [out] cpuSdmaMapNextInfoPtr - pointer to next sdma mapping information
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlCpuSdmaMapDbGetNext(
    IN  PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapInfoPtr,
    OUT PDL_CPU_SDMA_MAP_INFO_STC     * cpuSdmaMapNextInfoPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                                  pdlStatus;
    PDL_CPU_SDMA_PORT_MAPPING_PARAMS_STC        params;
    PDL_CPU_SDMA_PORT_LIST_PARAMS_STC         * tempCpuSdmaMapInfoPtr;
    PDL_CPU_SDMA_PORT_LIST_KEYS_STC             tempCpuSdmaMapKey;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    if (cpuSdmaMapInfoPtr == NULL || cpuSdmaMapNextInfoPtr == NULL) {
        return PDL_BAD_PTR;
    }

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_CPU_SDMA_PORT_MAPPING_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    if (pdlStatus != PDL_OK) {
        PDL_LIB_DEBUG_MAC(prvPdlBtnDebugFlag)(__FUNCTION__, __LINE__, "CPU SDMA MAPPING FEATURE DOESNT EXIST");
        return pdlStatus;
    }

    tempCpuSdmaMapKey.cpuSdmaPortIndex = (PDL_CPU_SDMA_PORT_INDEX_TYPE_TYP)cpuSdmaMapInfoPtr->key.index;

    pdlStatus = prvPdlibDbGetNext(params.cpuSdmaPortList_PTR, &tempCpuSdmaMapKey, (void **)&tempCpuSdmaMapInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    memset(cpuSdmaMapNextInfoPtr, 0, sizeof(PDL_CPU_SDMA_MAP_INFO_STC));
    cpuSdmaMapNextInfoPtr->key.index = (UINT_32)tempCpuSdmaMapInfoPtr->list_keys.cpuSdmaPortIndex;
    cpuSdmaMapNextInfoPtr->dev = tempCpuSdmaMapInfoPtr->ppDeviceNumber;
    cpuSdmaMapNextInfoPtr->macPort = tempCpuSdmaMapInfoPtr->ppPortNumber;
    cpuSdmaMapNextInfoPtr->logicalPort = tempCpuSdmaMapInfoPtr->logicalPortNumber;

    return PDL_OK;
}
/*$ END OF pdlCpuSdmaMapDbGetNext */
