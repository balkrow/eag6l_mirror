/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
********************************************************************************
 * @file pdlCpld.c
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
 * @brief Platform driver layer - CPLD related API
 *
 * @version   1
********************************************************************************
*/

/* *************************************************************************
 *$              EXTERNAL DECLARATIONS (IMPORT)
 *!**************************************************************************
 *!*/
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdlib/lib/pdlLib.h>
#include <pdl/lib/private/prvPdlLib.h>
#include <pdl/parser/pdlParser.h>

/* *************************************************************************
 *$              LOCAL VARIABLE DEFINITIONS
 *!**************************************************************************
 *!*/


/* *************************************************************************
 *$              LOCAL FUNCTION DEFINITIONS
 *!**************************************************************************
 *!*/

/**
 * @fn  PDL_STATUS pdlCpldDbGetParameters ( OUT PDL_CPLD_PARAMS_STC  * outCpldParamsPtr )
 *
 * @brief   Pdl CPLD parameters
 *
 * @param   outCpldParamsPtr Out CPLD parameters.
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlCpldDbGetParameters(
     OUT PDL_CPLD_PARAMS_STC  * outCpldParamsPtr
)
{
/* ***************************************************************************/
/* L O C A L   D E C L A R A T I O N S   A N D   I N I T I A L I Z A T I O N */
/* ***************************************************************************/
    PDL_STATUS                      pdlStatus;
    PDL_CPLD_PARAMS_STC             params;
    /* ***************************************************************************/
    /*                      F U N C T I O N   L O G I C                          */
/* ***************************************************************************/
    
    if (outCpldParamsPtr == NULL)
        return PDL_BAD_PTR;

    pdlStatus = pdlProjectParamsGet(PDL_FEATURE_ID_CPLD_E, (PDL_FEATURE_DATA_PARAMS_UNT *)&params);
    PDL_CHECK_STATUS(pdlStatus);

    if (params.onlineUpgradeSupported == FALSE)
        return PDL_NOT_SUPPORTED;
    
    memcpy(outCpldParamsPtr, &params, sizeof(PDL_CPLD_PARAMS_STC));

    return PDL_OK;
}

