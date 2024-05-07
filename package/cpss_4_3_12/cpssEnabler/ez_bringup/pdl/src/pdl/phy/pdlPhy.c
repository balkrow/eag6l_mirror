/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlPhy.c
 * @copyright
 *    (c), Copyright (C) 2023, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,COMPLETENESS OR PERFORMANCE.
********************************************************************************
 *
 * @brief Platform driver layer - Phy related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/phy/pdlPhy.h>
#include <pdl/phy/private/prvPdlPhy.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>
#include <pdl/interface/private/prvPdlSmiXsmi.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/

/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/*macro to check the pdl status and debug prints if debug flag on*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)  PDL_CHECK_STATUS_GEN(__pdlStatus, prvPdlPhyDebugFlag)

/**
 * @fn  PDL_STATUS pdlPhyInit ( void )
 *
 * @brief   Init phy module Create phy DB and initialize
 *
 * @param [in]  xmlId   Xml id.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPhyInit(
    void
)
{
    return PDL_OK;
}

/**
 * @fn  PDL_STATUS pdlPhyDbNumberOfPhysGet( OUT UINT_32 * numOfPhysPtr )
 *
 * @brief   Get number of phys
 *
 *          @note Based on data retrieved from XML
 *
 * @param [out] numOfPhysPtr    number of phys.
 *
 * @return  PDL_STATUS  .
 */

PDL_STATUS pdlPhyDbNumberOfPhysGet(
    OUT UINT_32   * numOfPhysPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                          pdlStatus;
    PDL_PHYS_PARAMS_STC                 params1;
    PDL_MPD_EXTERNAL_FILE_PARAMS_STC    params2;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_PHYS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params1);
    if ((pdlStatus == PDL_NOT_SUPPORTED) ||
        (pdlProjectParamsGet(PDL_FEATURE_ID_MPD_EXTERNAL_FILE_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params2) == PDL_OK)) {
        *numOfPhysPtr = 0;
        return PDL_OK;
    }

    PDL_CHECK_STATUS(pdlStatus);
    pdlStatus = prvPdlibDbGetNumOfEntries(params1.phyList_PTR, numOfPhysPtr);

    return pdlStatus;
}

/**
 * @fn  PDL_STATUS pdlPhyDbPhyConfigurationGet ( IN UINT_32 phyId, OUT PDL_PHY_CONFIGURATION_STC * phyConfigPtr )
 *
 * @brief   Get phy configuration data
 *
 * @param [in]  phyId           phy identifier starting from 1 up to numOfPhys.
 * @param [out] phyConfigPtr    phy configuration data.
 *
 * @return  PDL_BAD_PARAM if bad pointer supplied.
 */

PDL_STATUS pdlPhyDbPhyConfigurationGet(
    IN  UINT_32                             phyId,
    OUT PDL_PHY_CONFIGURATION_STC         * phyConfigPtr
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_PHYS_PARAMS_STC             params;
    PDL_PHY_LIST_KEYS_STC           phyKey;
    PDL_PHY_LIST_PARAMS_STC       * phyInfoPtr;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_PHYS_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    PDL_CHECK_STATUS(pdlStatus);

    phyKey.phyNumber = (PDL_PHY_NUMBER_TYPE_TYP)phyId;
    pdlStatus = prvPdlibDbFind(params.phyList_PTR, (void *)&phyKey, (void **)&phyInfoPtr);
    PDL_CHECK_STATUS(pdlStatus);

    phyConfigPtr->phyType = (PDL_PHY_TYPE_ENT)(phyInfoPtr->phyType - 1);
    phyConfigPtr->phyDownloadType = (PDL_PHY_DOWNLOAD_TYPE_ENT)(phyInfoPtr->downloadMethod - 1);

    return PDL_OK;
}

/**
 * @fn  PDL_STATUS prvPdlPhyPortDataGet ( IN XML_PARSER_NODE_DESCRIPTOR_TYP xmlId, OUT PDL_PHY_PORT_CONFIGURATION_STC *phyDataPtr )
 *
 * @brief   Parse phy port xml section
 *
 * @param [in]  xmlId       Xml id pointing to 'port-phy-connection' to parse.
 * @param [out] phyDataPtr  phy data.
 *
 * @return  PDL_XML_PARSE_ERROR if xml section coul not be parsed.
 * @return  PDL_BAD_PTR         if supplied pointer is NULL.
 */

/**
 * @fn  PDL_STATUS pdlPhyDebugSet ( IN BOOLEAN state )
 *
 * @brief   Pdl Phy debug set
 *
 * @param   state   True to state.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlPhyDebugSet(
    IN  BOOLEAN             state
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    prvPdlPhyDebugFlag = state;
    return PDL_OK;
}
/*$ END OF pdlPhyDebugSet */

/**
 * @fn  PDL_STATUS prvPdlPhyDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPhyDestroy(
    void
)
{
    /* ***************************************************************************/
    /* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
    /* ***************************************************************************/
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
    /* ***************************************************************************/
    return PDL_OK;
}

/*$ END OF prvPdlPhyDestroy */
