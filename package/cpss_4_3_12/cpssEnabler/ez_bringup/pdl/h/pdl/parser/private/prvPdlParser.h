/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\lib\private\prvpdllib.h.
 *
 * @brief   Declares the prvpdllib class
 */

#ifndef __PRV_PDL_PARSER_HEADER_FILE__
#define __PRV_PDL_PARSER_HEADER_FILE__

/**
********************************************************************************
 * @file prvPdlParser.h
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
 * @brief Platform driver layer - parser private declarations and APIs
 *
 * @version   1
********************************************************************************
*/

#define PRV_PDL_PARSER_CAST_PDL_ENUM_TO_YANG_ENUM_MAC(__val) (__val+1)
#define PRV_PDL_PARSER_CAST_YANG_ENUM_TO_PDL_ENUM_MAC(__val) (__val-1)

#endif/* #ifndef __PRV_PDL_PARSER_HEADER_FILE__ */
