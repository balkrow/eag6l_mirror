/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\interface\pdlPpRegdebug.h.
 *
 * @brief   Declares the pdlppregdebug class
 */

#ifndef __prvPdlPpRegDebugh
#define __prvPdlPpRegDebugh
/**
********************************************************************************
 * @file pdlPpRegDebug.h
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
 * @brief Platform driver layer - packet processor register interface debug related API (internal)
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
/**
 * @fn  PDL_STATUS PdlPpRegDebugDbGetAttributes ( IN PDL_INTERFACE_TYP interfaceId, OUT PDL_INTERFACE_PP_REG_STC * attributesPtr )
 *
 * @brief   Pdl packet processor register debug database get attributes
 *
 * @param           interfaceId     Identifier for the interface.
 * @param [in,out]  attributesPtr   If non-null, the attributes pointer.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS PdlPpRegDebugDbGetAttributes(
    IN  PDL_INTERFACE_TYP                             interfaceId,
    OUT PDL_INTERFACE_PP_REG_STC                    * attributesPtr
);

/*$ END OF PdlPpRegDebugDbGetAttributes */

/* @}*/
/* @}*/
/* @}*/

#endif
