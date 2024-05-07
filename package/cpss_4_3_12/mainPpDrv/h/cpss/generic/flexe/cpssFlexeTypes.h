/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssFlexeTypes.h
*
* @brief CPSS FlexE API definitions
*
* @version   1
********************************************************************************
*/

#ifndef _cpssFlexeTypesh
#define _cpssFlexeTypesh

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include <cpss/generic/tm/cpssTmPublicDefs.h>

/* [TBD] Change the values */

/** MAC/PHYs Macros */
#define CPSS_FLEXE_PHYS_NUM_MAX_CNS 80

#define CPSS_FLEXE_PHYS_ID_MAX_CNS 80

#define CPSS_FLEXE_MAC_NUM_MAX_CNS 80

#define CPSS_FLEXE_PHY_SPEED_MIN_CNS 25

#define CPSS_FLEXE_MAX_PHYS_IN_GROUP_CNS 8

#define CPSS_FLEXE_50G_SERDES_MAX 32

#define CPSS_FLEXE_100G_SERDES_MAX 16

/** Group Macros */
#define CPSS_FLEXE_GROUPS_NUM_MAX_CNS 128

#define CPSS_FLEXE_GROUPS_ID_MAX_CNS 8

#define CPSS_FLEXE_GROUP_INSTANCES_MAX_CNS 8

#define CPSS_FLEXE_GROUP_MAX_BANDWIDTH 400

/** Client Macros */
#define CPSS_FLEXE_CLIENTS_NUM_MAX_CNS 160

#define CPSS_FLEXE_CLIENTS_ID_MAX_CNS 160

#define CPSS_FLEXE_TILES_MAX_CNS 2

#define CPSS_FLEXE_SHIMS_MAX_CNS 2

#define CPSS_FLEXE_GROUP_CLIENTS_MAX_CNS 80

/** Calendar Macros */

#define CPSS_FLEXE_CALENDARS_MAX_CNS 2

#define CPSS_FLEXE_CALENDAR_SLOTS_MAX_CNS 20

/**
 * @enum CPSS_FLEXE_SERDES_LANE_SPEED_ENT
 *
 * @brief enumerator for supported SerDes Lane speed
 */
typedef enum {
    /** @brief 50G speed */
    CPSS_FLEXE_SERDES_LANE_SPEED_50G_E,

    /** @brief 100G speed */
    CPSS_FLEXE_SERDES_LANE_SPEED_100G_E,
} CPSS_FLEXE_SERDES_LANE_SPEED_ENT;

/**
 * @struct CPSS_FLEXE_PHY_CONFIG_STC
 *
 * @brief structure for phy config parameters
 */
typedef struct {
    /** @brief The Mac index */
    GT_U32                           macIdx;

    /** @brief number of SerDes Lanes
     *         RANGE: 1 to 8 */
    GT_U32                           numSerdesLanes;

    /** @brief SerDes lane speed
     *         Supported Speeds: 50G and 100G */
    CPSS_FLEXE_SERDES_LANE_SPEED_ENT serdesLaneSpeed;

    /** @brief AN/non-AN */
    GT_BOOL                          autoNegEnabled;
} CPSS_FLEXE_PHY_CONFIG_STC;

/**
 * @enum CPSS_FLEXE_GROUP_BANDWIDTH_ENT
 *
 * @brief enumerator for group bandwidth
 */
typedef enum {
    CPSS_FLEXE_GROUP_BANDWIDTH_50G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_100G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_150G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_200G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_250G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_300G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_350G_E,
    CPSS_FLEXE_GROUP_BANDWIDTH_400G_E,
} CPSS_FLEXE_GROUP_BANDWIDTH_ENT;

/**
 * @struct CPSS_FLEXE_GROUP_INFO_STC
 *
 * @brief FlexE group info structure
 */
typedef struct {
    /** @brief group bandwidth */
    CPSS_FLEXE_GROUP_BANDWIDTH_ENT  bandwidth;

    /** @brief number of PHY elements in the group */
    GT_U32        numPhys;

    /** @brief number of Instances */
    GT_U32        numInstances;

    /** @brief list of associated Instance numbers */
    GT_U32        instanceArr[CPSS_FLEXE_GROUP_INSTANCES_MAX_CNS];
} CPSS_FLEXE_GROUP_INFO_STC;


/**
 * @enum CPSS_FLEXE_CLIENT_TYPE_ENT
 *
 * @brief enumerator for client type (L1/L2)
 */
typedef enum {
    /** @brief client type is L1 */
    CPSS_FLEXE_CLIENT_TYPE_L1_E,

    /** @brief client type is L2 */
    CPSS_FLEXE_CLIENT_TYPE_L2_E
} CPSS_FLEXE_CLIENT_TYPE_ENT;

/**
 * @enum CPSS_FLEXE_CAL_SWITCH_OPERATION_ENT
 *
 * @brief client add/delete operation during calendar switch
 */
typedef enum {
    CPSS_FLEXE_CAL_SWITCH_OPERATION_ADD_E,
    CPSS_FLEXE_CAL_SWITCH_OPERATION_DELETE_E
} CPSS_FLEXE_CAL_SWITCH_OPERATION_ENT;

/**
 * @struct CPSS_FLEXE_CAL_SWITCH_CLIENT_INFO_STC
 *
 * @brief structure for Calendar switch client information
 */
typedef struct {
    /** @brief client number */
    GT_U32      clientNum;

    /** @brief instance number */
    GT_U32      instanceNum;

    /** @brief Client type L1/L2 */
    CPSS_FLEXE_CLIENT_TYPE_ENT  clientType;

    /** @brief Client speed. It must be multiple of 5G.
     *         Range = (5G - 400G) */
    GT_U32                      clientSpeed;

    /** @brief Client add/delete operation */
    CPSS_FLEXE_CAL_SWITCH_OPERATION_ENT operation;
} CPSS_FLEXE_CAL_SWITCH_CLIENT_INFO_STC;

/**
 * @struct CPSS_FLEXE_GROUP_CALENDAR_SWITCH_PARAMS_STC
 *
 * @brief structure for group calendar switch parameters.
 */
typedef struct {
    /** @brief number of clients effected */
    GT_U32          numClients;

    /** @brief Array of instance indices */
    CPSS_FLEXE_CAL_SWITCH_CLIENT_INFO_STC  clientInfoArr[CPSS_FLEXE_GROUP_CLIENTS_MAX_CNS];

} CPSS_FLEXE_GROUP_CALENDAR_SWITCH_PARAMS_STC;

/**
 * @enum CPSS_FLEXE_CALENDAR_TYPE_ENT
 *
 * @brief enumerator for calendar type
 */
typedef enum {
    /** @brief calendar A */
    CPSS_FLEXE_CALENDAR_A_E,

    /** @brief calendar B */
    CPSS_FLEXE_CALENDAR_B_E
} CPSS_FLEXE_CALENDAR_TYPE_ENT;

/**
 * @enum CPSS_FLEXE_TRANSMIT_DIRECTION_ENT
 *
 * @brief enumerator for transmit direction
 */
typedef enum {
    /** @brief tx direction */
    CPSS_FLEXE_TRANSMIT_DIRECTION_TX_E,

    /** @brief rx direction */
    CPSS_FLEXE_TRANSMIT_DIRECTION_RX_E
} CPSS_FLEXE_TRANSMIT_DIRECTION_ENT;

/**
 * @struct CPSS_FLEXE_CLIENT_INFO_STC
 *
 * @brief structure to hold client information
 */
typedef struct {
    /** @brief Flexe group number it is attached to */
    GT_U32                      groupNum;

    /** @brief Flexe group instance number it attached to */
    GT_U32                      instanceNum;

    /** @brief Client type L1/L2 */
    CPSS_FLEXE_CLIENT_TYPE_ENT  clientType;

    /** @brief Client speed. It must be multiple of 5G.
     *         Range = (5G - 400G) */
    GT_U32                      clientSpeed;
} CPSS_FLEXE_CLIENT_INFO_STC;

/**
 * @struct CPSS_FLEXE_CALENDAR_INFO_STC
 *
 * @brief structure to hold calendar information
 */
typedef struct {
    GT_U32                      numClients;
    GT_U32                      clientNumArr[CPSS_FLEXE_GROUP_CLIENTS_MAX_CNS];
    CPSS_FLEXE_CLIENT_INFO_STC  clientInfoArr[CPSS_FLEXE_GROUP_CLIENTS_MAX_CNS];
} CPSS_FLEXE_CALENDAR_INFO_STC;

#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _cpssFlexeTypesh */
