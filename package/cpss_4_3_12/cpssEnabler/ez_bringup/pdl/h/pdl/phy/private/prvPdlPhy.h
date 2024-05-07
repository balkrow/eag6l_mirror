/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\phy\private\prvpdlphy.h.
 *
 * @brief   Declares the prvpdlphy class
 */

#ifndef __prvPdlPhyh

#define __prvPdlPhyh
/**
********************************************************************************
 * @file prvPdlPhy.h
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
 * @brief Platform driver layer - Phy private declarations and APIs
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/phy/pdlPhy.h>

/**
* @addtogroup Phy
* @{
*/

/**
* @defgroup Phy_private
* @{Phy private definitions and declarations including:
*/

/**
 * @fn  PDL_STATUS pdlPhyInit ( void );
 *
 * @brief   Init phy module Create phy DB and initialize
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlPhyInit(
    void
);

/**
 * @fn  PDL_STATUS prvPdlPhyDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPhyDestroy(
    void
);

/* @}*/
/* @}*/

#endif
