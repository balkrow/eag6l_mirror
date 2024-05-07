/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\common\pdltypes.h.
 *
 * @brief   Declares the pdltypes class
 */

#ifndef __pdlTypesh

#define __pdlTypesh
/**
********************************************************************************
 * @file pdlTypes.h
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
 * @brief Platform driver layer - main file
 *
 * @version   1
********************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pdlib/common/pdlTypes.h>

#ifdef LINUX
#ifndef ASIC_SIMULATION
/* differ between code that run on linux HW and code that run on linux WM
   and code that run on windows WM */
#define LINUX_HW
#endif /*ASIC_SIMULATION*/
#endif /*LINUX*/

#include <stdint.h> /* for uintptr_t to support pointer casting to integer in 64 bits */
typedef uintptr_t   UINTPTR;
typedef intptr_t   INTPTR;

/* use PDL_UNUSED_PARAM to avoid next compilation errors/warnings       */
/* the CPSS compile GCC with very strict compilation warnings as errors */
/* VC10 : warning C4100: 'resultPtr' : unreferenced formal parameter    */
/* GCC error: unused parameter 'resultPtr' [-Werror=unused-parameter]   */
#define PDL_UNUSED_PARAM(x) x = x

#define PDL_TO_TEXT(_val) #_val
#undef PDL_CHECK_STATUS_GEN
#define PDL_CHECK_STATUS_GEN(__pdlStatus, __flagId)                                                                 \
    if (__pdlStatus != PDL_OK) {                                                                                    \
        if (__flagId) {                                                                                             \
                prvPdlStatusDebugLogHdr(__FUNCTION__,__LINE__,__pdlStatus,PDL_STATUS_ERROR_PRINT_IDENTIFIER);       \
        }                                                                                                           \
    return __pdlStatus;                                                                                             \
    }

#define PDL_ASSERT_TRUE_GEN(_condition,  __flagId)                                                                      \
    if (!(_condition) ) {                                                                                           \
        if (__flagId) {                                                                                             \
            prvPdlStatusDebugLogHdr(PDL_TO_TEXT(_condition),__LINE__,PDL_FAIL,PDL_STATUS_ERROR_PRINT_IDENTIFIER);   \
        }                                                                                                           \
        return PDL_FAIL;                                                                                            \
    }

/*! Maximum number of Packet Processors */
#define PDL_MAX_PP_NUM              8

/*! 32 bits mask */
#define PDL_MASK_32_BIT             0xFFFFFFFF

#define PDL_ALIGN_ARCH_WORD(_size)   \
            (((_size) + (sizeof(UINTPTR) - 1L)) & ~(sizeof(UINTPTR) - 1L))

/**
*  @def PDL_CHECK_STATUS(status)
*  Return in case status != PDL_OK
*/
#undef PDL_CHECK_STATUS
#define PDL_CHECK_STATUS(__pdlStatus)     \
            if (__pdlStatus != PDL_OK) {   \
                return __pdlStatus;       \
            }
/**
 * @enum    PDL_IS_SUPPORTED_ENT
 *
 * @brief   is option supported
 */

typedef enum {
    PDL_IS_SUPPORTED_NO_E = 0,
    PDL_IS_SUPPORTED_YES_E
} PDL_IS_SUPPORTED_ENT;
/**
 * @enum    PDL_PORT_SPEED_ENT
 *
 * @brief   port speed
 */

typedef enum {
    PDL_PORT_SPEED_10_E,
    PDL_PORT_SPEED_100_E,
    PDL_PORT_SPEED_1000_E,
    PDL_PORT_SPEED_2500_E,
    PDL_PORT_SPEED_5000_E,
    PDL_PORT_SPEED_10000_E,
    PDL_PORT_SPEED_20000_E,
    PDL_PORT_SPEED_24000_E,
    PDL_PORT_SPEED_25000_E,
    PDL_PORT_SPEED_40000_E,
    PDL_PORT_SPEED_50000_E,
    PDL_PORT_SPEED_100000_E,
    PDL_PORT_SPEED_22000_E,
    PDL_PORT_SPEED_LAST_E
} PDL_PORT_SPEED_ENT;

/**
 * @enum    PDL_PHY_TYPE_ENT
 *
 * @brief   phy type
 */

typedef enum {
    PDL_PHY_TYPE_direct_attached_fiber_E,       /* direct attached - no PHY */
    PDL_PHY_TYPE_alaska_88E1543_E,              /* combo ports          */
    PDL_PHY_TYPE_alaska_88E1545_E,              /* copper GE with mac on PHY */
    PDL_PHY_TYPE_alaska_88E1548_E,              /* fiber only GE - not MAC in PHY */
    PDL_PHY_TYPE_alaska_88E1680_E,              /* Copper 10M/100M/1G */
    PDL_PHY_TYPE_alaska_88E1680L_E,             /* Copper 10M/100M/1G */
    PDL_PHY_TYPE_alaska_88E151X_E,              /* copper (HW supports combo and fiber) */
    PDL_PHY_TYPE_alaska_88E3680_E,              /* 100M X 8*/
    PDL_PHY_TYPE_alaska_88E32x0_E,              /* 100M, 1G, 10G copper (with FW, SolarFlare next generation) */
    PDL_PHY_TYPE_alaska_88E33X0_E,              /* MGIG combo */
    PDL_PHY_TYPE_alaska_88E20X0_E,              /* copper - 10M, 100M, 1G, 2.5G, 5G */
    PDL_PHY_TYPE_alaska_88E2180_E,              /* copper - 10M, 100M, 1G, 2.5G, 5G */
    PDL_PHY_TYPE_alaska_88E2540_E,              /* copper - 10M, 100M, 1G, 2.5G, 5G */
    PDL_PHY_TYPE_alaska_88X3540_E,              /* copper - 10M, 100M, 1G, 2.5G, 5G, 10G */
    PDL_PHY_TYPE_alaska_88E1780_E,              /* Copper 10M/100M/1G */
    PDL_PHY_TYPE_alaska_88E2580_E,              /* copper - 10M, 100M, 1G, 2.5G, 5G */
    PDL_PHY_TYPE_alaska_88X3580_E,              /* copper - 10M, 100M, 1G, 2.5G, 5G, 10G */
    PDL_PHY_TYPE_alaska_88E1112_E,              /* combo ports          */
    PDL_PHY_TYPE_alaska_88E1781_E,              /* Copper 10M/100M/1G */
    PDL_PHY_TYPE_alaska_88E1781_internal_E,     /* Copper 10M/100M/1G (Integrated/Internal to the Packet Processor */
    PDL_PHY_TYPE_alaska_88E2780_E,              /* Copper - 10M, 100M, 1G, 2.5G */
    PDL_PHY_TYPE_alaska_MAD_E,                  /* All Copper and Combo PHYs with 10M/100M/1G support */
    PDL_PHY_TYPE_LAST_E
} PDL_PHY_TYPE_ENT;

#define PDL_PHY_NUM_OF_TYPES    PDL_PHY_TYPE_LAST_E

/**
 * @enum    PDL_PHY_DOWNLOAD_TYPE_ENT
 *
 * @brief   phy download type
 */
typedef enum {
    PDL_PHY_DOWNLOAD_TYPE_NONE_E,                   /* no download          */
    PDL_PHY_DOWNLOAD_TYPE_RAM_E,                    /* ram download         */
    PDL_PHY_DOWNLOAD_TYPE_FLASH_E,                  /* flash download       */
    PDL_PHY_DOWNLOAD_TYPE_LAST_E
} PDL_PHY_DOWNLOAD_TYPE_ENT;

/**
 * @enum    PDL_INTERFACE_MODE_ENT
 *
 * @brief   interface mode
 */

typedef enum {
    PDL_INTERFACE_MODE_SGMII_E = 0,
    PDL_INTERFACE_MODE_1000BASE_X_E = 2,
    PDL_INTERFACE_MODE_100BASE_FX_E = 6,
    PDL_INTERFACE_MODE_KR_E = 7,
    PDL_INTERFACE_MODE_SR_LR_E = 8,
    PDL_INTERFACE_MODE_QSGMII_E = 9,
    PDL_INTERFACE_MODE_XHGS_E = 12,
    PDL_INTERFACE_MODE_KR2_E = 13,
    PDL_INTERFACE_MODE_KR4_E = 15,
    PDL_INTERFACE_MODE_SXGMII_E = 16,
    PDL_INTERFACE_MODE_DXGMII_E = 17,
    PDL_INTERFACE_MODE_QUSGMII_E = 18,
    PDL_INTERFACE_MODE_QXGMII_E = 19,
    PDL_INTERFACE_MODE_OUSGMII_E = 20,
    PDL_INTERFACE_MODE_OXGMII_E = 21,
    PDL_INTERFACE_MODE_XHGS_SR_E = 22,
	PDL_INTERFACE_MODE_2500BASE_X_E = 23,
	PDL_INTERFACE_MODE_CR_E = 24,
    PDL_INTERFACE_MODE_NA_E = 25,
    PDL_INTERFACE_MODE_LAST_E,
} PDL_INTERFACE_MODE_ENT;

/**
 * @enum    PDL_CONNECTOR_TYPE_ENT
 *
 * @brief   Copper/Fiber/Combo
 */

typedef enum {
    PDL_CONNECTOR_TYPE_SFPPLUS_E,
    PDL_CONNECTOR_TYPE_DAC_E,
    PDL_CONNECTOR_TYPE_RJ45_E,
    PDL_CONNECTOR_TYPE_LAST_E,
} PDL_CONNECTOR_TYPE_ENT;


typedef enum {
	/*unknown */
	PDL_CONNECTOR_TECHNOLOGY_UNKNOWN_E = 1, /* unknown type or used as a don't care */
	PDL_CONNECTOR_TECHNOLOGY_PASSIVE_E = 2, /* bit2*/
	PDL_CONNECTOR_TECHNOLOGY_ACTIVE_E = 3, /* bit 3*/
	PDL_CONNECTOR_TECHNOLOGY_LAST_E = 4
} PDL_CONNECTOR_TECHNOLOGY_ENT;

/**
 * @enum    PDL_CPU_TYPE_ENT
 *
 * @brief   CPU type
 */

/* @}*/
/* @}*/

#endif
