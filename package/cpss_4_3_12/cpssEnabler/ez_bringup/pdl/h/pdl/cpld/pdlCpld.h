/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\cpld\pdlCpld.h.
 *
 * @brief   Declares the pdlCpld class
 */

#ifndef __pdlCpldh
#define __pdlCpldh
/**
********************************************************************************
 * @file pdlCpld.h
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


#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/parser/auto/pdlParser.h>
#include <pdl/common/pdlTypes.h>

/**
 * @fn  PDL_STATUS pdlCpldDbGetParameters ( OUT PDL_CPLD_PARAMS_STC  * outCpldParamsPtr )
 *
 * @brief   Pdl CPLD parameters
 *
 * @param   outCpldParamsPtr Out CPLD parameters.
 * @return  A PDL_STATUS.
 */
PDL_STATUS pdlCpldDbGetParameters(
    OUT PDL_CPLD_PARAMS_STC  * paramsPtr
);

#endif
