/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\cpu\pdlcpu.h.
 *
 * @brief   Declares the pdlcpu class
 */

#ifndef __pdlCpuh

#define __pdlCpuh
/**
********************************************************************************
 * @file pdlCpu.h
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
 * @brief Platform driver layer - CPU library
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/parser/pdlParser.h>

/**
* @defgroup CPU CPU
* @{
*/

/**
 * @enum    PDL_CPU_FAMILY_TYPE_ENT
 *
 * @brief   CPU Family Type (MSYS/AXP ...)
 */

typedef enum {
    PDL_CPU_FAMILY_TYPE_MSYS_E,
    PDL_CPU_FAMILY_TYPE_AXP_E,
    PDL_CPU_FAMILY_TYPE_A38X_E,
    PDL_CPU_FAMILY_TYPE_ARMv8_E,
    PDL_CPU_FAMILY_TYPE_LAST_E
} PDL_CPU_FAMILY_TYPE_ENT;

typedef struct {
    UINT_32                                 index;
} PDL_CPU_SDMA_MAP_KEY_STC;

typedef struct {
    PDL_CPU_SDMA_MAP_KEY_STC            key;
    UINT_32                             dev;
    UINT_32                             macPort;
    UINT_32                             logicalPort;
} PDL_CPU_SDMA_MAP_INFO_STC;

/**
 * @fn  PDL_STATUS pdlCpuTypeGet ( OUT PDL_CPU_TYPE_TYPE_ENT * cpuTypePtr );
 *
 * @brief   Get cpu type
 *
 * @param [out] cpuTypePtr  cpu type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuTypeGet(
    OUT PDL_CPU_TYPE_TYPE_ENT          * cpuTypePtr
);

/**
 * @fn  PDL_STATUS pdlCpuFamilyTypeGet ( OUT PDL_CPU_FAMILY_TYPE_ENT * cpuFamilyTypePtr );
 *
 * @brief   Get cpu family type
 *
 * @param [out] cpuFamilyTypePtr    cpu family type.
 *
 * @return  PDL_STATUS.
 */

PDL_STATUS pdlCpuFamilyTypeGet(
    OUT PDL_CPU_FAMILY_TYPE_ENT          * cpuFamilyTypePtr
);

/**
 * @fn  PDL_STATUS pdlCpuNumOfUsbDevicesGet (  OUT UINT_32 * numOfUsbDevicesPtr )
 *
 * @brief   Get number of USB devices on board
 *
 * @param [out] numOfUsbDevicesPtr - number of usb devices
 *
 * @return  PDL_STATUS.
 */
PDL_STATUS pdlCpuNumOfUsbDevicesGet(
    OUT UINT_32          * numOfUsbDevicesPtr
);
/*$ END OF pdlCpuNumOfUsbDevicesGet */

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
);
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
);
/*$ END OF pdlCpuSdmaMapDbGetNext */

/* @}*/

#endif