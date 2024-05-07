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
* @file prvCpssFlexe.h
*
* @brief Private CPSS FlexE API and macro definitions
*
* @version   1
********************************************************************************
*/

#ifndef _pvCpssFlexeh
#define _prvCpssFlexeh

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/flexe/cpssFlexe.h>
#include <cpss/generic/flexe/prvCpssFlexeTransit.h>

#define PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, errMsg, ...)      \
    do {                                                              \
        if (rc)                                                       \
        {                                                             \
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, errMsg, ##__VA_ARGS__); \
        }                                                             \
    } while (0)

#define PRV_CPSS_FLEXE_DB_PTR_GET(_dbPtr) \
    _dbPtr = (PRV_CPSS_FLEXE_DB_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.genericFlexeDir.flexeDbPtr)

#define PRV_CPSS_FLEXE_DB_PTR_CHECK_MAC(_dbPtr)                                  \
    do {                                                                         \
        if (_dbPtr == NULL)                                                      \
        {                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG); \
        }                                                                        \
    } while (0)

#define PRV_CPSS_FLEXE_ID_MAX_CNS 0xFFFFFFFF
#define PRV_CPSS_FLEXE_TILES_MAX_CNS 2

/**
 * @struct PRV_CPSS_FLEXE_DB_PHY_TABLE_ENTRY_STC
 *
 * @brief structure for phy entry information
 */
typedef struct {
    /** @brief PHY config info */
    CPSS_FLEXE_PHY_CONFIG_STC   phyCfg;

    /** @brief  valid status of the PHY*/
    GT_BOOL     isValid;

    /** @brief associated group */
    GT_U8       groupId;

    /** @brief associated phy number */
    GT_U32      phyNum;

    /** @brief link up/down */
    GT_BOOL     linkStatus;

    /** @brief Bitmap of associated serdes lanes */
    GT_BOOL     serdesBmp;

} PRV_CPSS_FLEXE_DB_PHY_TABLE_ENTRY_STC;

/**
 * @struct PRV_CPSS_FLEXE_DB_PHY_TABLE_STC
 *
 * @brief structure to hold phy information
 */
typedef struct {
    /** @brief number of active PHYs */
    GT_U32      numActivePhys;

    /** @brief Phy number to phy ID mapping table */
    GT_U32      num2Id[CPSS_FLEXE_PHYS_NUM_MAX_CNS];

    /** @brief Array of PHY entries */
    PRV_CPSS_FLEXE_DB_PHY_TABLE_ENTRY_STC     phyEntryArr[CPSS_FLEXE_PHYS_ID_MAX_CNS];

    GT_BOOL     macsUsed[CPSS_FLEXE_MAC_NUM_MAX_CNS];

    /* [TBD] */
    GT_BOOL     serdes50GUsed[8];
    GT_BOOL     serdes100GUsed[8];

} PRV_CPSS_FLEXE_DB_PHY_TABLE_STC;

typedef struct
{
    /** @brief slot bitmap */
    GT_U32      slotBmp;
} PRV_CPSS_FLEXE_CALENDAR_SLOT_DATA_STC;

/**
 * @struct PRV_CPSS_FLEXE_DB_CALENDAR_STC
 *
 * @brief structure to hold calendar
 */
typedef struct {
    /** @brief Active calendar status */
    GT_BOOL                             isActiveCalendar;

    /** @brief instance number */
    GT_U32                              instanceNumber;

    /** @brief slot data */
    PRV_CPSS_FLEXE_CALENDAR_SLOT_DATA_STC   slotData;
} PRV_CPSS_FLEXE_DB_CALENDAR_STC;

/**
 * @struct PRV_CPSS_FLEXE_DB_GROUP_TABLE_ENTRY_STC
 *
 * @brief structure to hold group entry information
 */
typedef struct {
    /** @brief group information. Valid only when isActive = GT_TRUE */
    CPSS_FLEXE_GROUP_INFO_STC   groupInfo;

    /** @brief valid state of the group */
    GT_BOOL                     isValid;

    /** @brief associated group number */
    GT_U32                      groupNum;

    /** @brief tile ID */
    GT_U8                       tileId;

    /** @brief shim ID */
    GT_U8                       shimId;

    /** @brief instance bitmap */
    GT_U8                          instanceBmp;

    /** @brief bondMaskArr */
    GT_U8                          bondMaskArr[CPSS_FLEXE_GROUP_INSTANCES_MAX_CNS];

    /** @brief interlieve config */
    GT_U8                          interleaveCfg;

    /** @brief Calendars information */
    PRV_CPSS_FLEXE_DB_CALENDAR_STC calendar[CPSS_FLEXE_CALENDARS_MAX_CNS];

    /** @brief associated first phy */
    GT_U32                      firstPhyId;
} PRV_CPSS_FLEXE_DB_GROUP_TABLE_ENTRY_STC;

/**
 * @struct PRV_CPSS_FLEXE_DB_GROUP_TABLE_STC
 *
 * @brief structure to hold Flexe Groups information
 */
typedef struct {
    /** @brief number of active Groups */
    GT_U32      numActiveGroups;

    /** @brief Group number to Group Id mapping table */
    GT_U32      num2Id[CPSS_FLEXE_GROUPS_NUM_MAX_CNS];

    /** @brief Array of Group entries */
    PRV_CPSS_FLEXE_DB_GROUP_TABLE_ENTRY_STC   groupEntryArr[CPSS_FLEXE_GROUPS_ID_MAX_CNS];
} PRV_CPSS_FLEXE_DB_GROUP_TABLE_STC;


/**
 * @struct PRV_CPSS_FLEXE_DB_CLIENT_TABLE_ENTRY_STC
 *
 * @brief structure for Client entry informaion
 */
typedef struct {
    /** @brief client info */
    CPSS_FLEXE_CLIENT_INFO_STC  clientInfo;

    /** @brief valid state of the client */
    GT_BOOL     isValid;

    /** @brief associated client number */
    GT_U32      clientNum;

    /** @brief number of slots */
    GT_U32      numSlots;

    /** @brief enable/disable traffic */
    GT_BOOL     trafficEnable;

    /** @brief slots bitmap */
    GT_U32      slotBmp;
} PRV_CPSS_FLEXE_DB_CLIENT_TABLE_ENTRY_STC;

/**
 * @struct PRV_CPSS_FLEXE_DB_CLIENT_TABLE_STC
 *
 * @brief structure to hold Clients information
 */
typedef struct {
    /** @brief number of active Clients */
    GT_U32      numActiveClients;

    /** @brief Client number to client Id mapping */
    GT_U32      num2Id[CPSS_FLEXE_CLIENTS_NUM_MAX_CNS];

    /** @brief Array of Client entries */
    PRV_CPSS_FLEXE_DB_CLIENT_TABLE_ENTRY_STC  clientEntryArr[CPSS_FLEXE_CLIENTS_ID_MAX_CNS];
} PRV_CPSS_FLEXE_DB_CLIENT_TABLE_STC;


/**
 * @struct PRV_CPSS_FLEXE_STATE_INFO_STC
 *
 * @brief structure to hold various FlexE states information
 */
typedef struct {
    /** @brief Hardware Initialization status */
    GT_BOOL     hwInitDone;

    /** @brief Hardware Soft reset status */
    GT_BOOL     hwSoftResetDone;

    /** @brief Calendar switch status */
    GT_BOOL     calSwitchDone;
} PRV_CPSS_FLEXE_STATE_INFO_STC;

typedef struct {
    GT_U32          maxTiles;
    GT_U32          maxGroups;
    GT_U32          maxPHYs;
    GT_U32          maxClients;
    GT_U32          usedTileBmp;
    GT_U32          usedGroupBmp;
    GT_U32          usedPhysBmp;
    GT_U32          usedClientsBmp;
} PRV_CPSS_FLEXE_RESOURCE_INFO_STC;

/**
 * @struct PRV_CPSS_FLEXE_DB_STC
 *
 * @brief structure for FLEXE database
 */
typedef struct {

#ifdef FLEXE_AGENT_IN_CPSS
    /** @brief Transit Data. This param should be at the beginning of the structure */
    PRV_CPSS_FLEXE_TRANSIT_DATA_STC     transitData;
#endif

    /** @brief resources and usage */
    PRV_CPSS_FLEXE_RESOURCE_INFO_STC    resources;

    /** @brief Flexe state informatino  */
    PRV_CPSS_FLEXE_STATE_INFO_STC       stateInfo;

    /** @brief Table holds defined PHYs information */
    PRV_CPSS_FLEXE_DB_PHY_TABLE_STC     phyTable;

    /** @brief Table holds defined groups information */
    PRV_CPSS_FLEXE_DB_GROUP_TABLE_STC   groupTable;

    /** @brief Table holds defined client information */
    PRV_CPSS_FLEXE_DB_CLIENT_TABLE_STC  clientTable;

} PRV_CPSS_FLEXE_DB_STC;

/***************************************************************************/

/* Maximum number of flexe groups */
#define PRV_CPSS_FLEXE_GROUPS_MAX_CNS 8

/* [TBD] Defined values may change later */
#define PRV_CPSS_FLEXE_PORT_NUM_MAX_CNS 128
#define PRV_CPSS_FLEXE_PHYS_PER_GROUP_MAX_CNS 8
#define PRV_CPSS_FLEXE_PHY_ID_MAX_CNS 128

/* check group id */
#define PRV_CPSS_GROUP_ID_CHECK_MAC(_groupId)                               \
    if ((_groupId) >= PRV_CPSS_FLEXE_GROUPS_MAX_CNS)                        \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);   \
    }

/* check group info */
#define PRV_CPSS_GROUP_INFO_CHECK_MAC(_groupInfo)                           \
    if (/*((_groupInfo)->portNum >= PRV_CPSS_FLEXE_PORT_NUM_MAX_CNS) || */      \
        ((_groupInfo)->numPhys >= PRV_CPSS_FLEXE_PHYS_PER_GROUP_MAX_CNS)/* ||*/ \
        /*((_groupInfo)->firstPhyId >= PRV_CPSS_FLEXE_PHY_ID_MAX_CNS)*/)        \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);   \
    }

/**
 * @internal prvCpssFlexeGroupCreate function
 * @endinternal
 *
 * @brief Create an FlexE group and add related PHYs
 *
 * @param[in] devNum    - device number
 * @param[in] groupId   - FlexE group identifier
 * @param[in] groupInfo - FlexE group information
 *
 */
GT_STATUS prvCpssFlexeGroupCreate
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         groupNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC      *groupInfo
);

#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _prvCpssFlexeh */
