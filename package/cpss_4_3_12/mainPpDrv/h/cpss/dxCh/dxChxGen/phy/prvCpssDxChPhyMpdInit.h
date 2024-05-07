/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file prvCpssDxChPhyMpdInit.h
*
* @brief PHY management through MPD( Marvell PHY driver)
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChPhyMpdInith
#define __prvCpssDxChPhyMpdInith

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#ifdef MIN
#undef MIN
#endif
#include <cpss/dxCh/dxChxGen/phy/private/mpdPrv.h>

/* macro to get the PHY port MPD index */
#define PRV_CPSS_PHY_PORT_MPD_INDEX_GET_MAC(devNum,portMacNum)        \
    PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[portMacNum].mpdIndex;

typedef struct PRV_CPSS_DXCH_PHY_STATIC_INFO_STCT
{
    GT_U32                              portNum;
    GT_U16                              phyAddr;
    GT_U32                              phyNumber;
    MPD_TYPE_ENT                        phyType;
    MPD_PHY_USX_TYPE_ENT                usxType;
    MPD_TRANSCEIVER_TYPE_ENT            transceiverType;
    CPSS_PHY_SMI_INTERFACE_ENT          smiInterface;
} PRV_CPSS_DXCH_PHY_STATIC_INFO_STC;

#define MPD_NUM_OF_DEBUG_FLAGS_CNS       (10)

#ifdef PRV_MPD_DEBUG
#define MPD_DBG(x) cpssOsPrintf x
#else
#define MPD_DBG(x)
#endif

typedef struct {
        BOOLEAN   isFree;
        char    * flagName_PTR;
        char    * flagHelp_PTR;
        BOOLEAN   flagStatus; /* TRUE - On */
}MPD_SAMPLE_DEBUG_FLAG_INFO_STC;

#define MPD_TO_CPSS_SPEED(speed, cpssSpeed)     \
    switch(speed)                               \
    {                                           \
        case MPD_SPEED_10M_E:                   \
            cpssSpeed = CPSS_PORT_SPEED_10_E;   \
            break;                              \
        case MPD_SPEED_100M_E:                  \
            cpssSpeed = CPSS_PORT_SPEED_100_E;  \
            break;                              \
        case MPD_SPEED_1000M_E:                 \
            cpssSpeed = CPSS_PORT_SPEED_1000_E; \
            break;                              \
        default:                                \
            cpssSpeed = CPSS_PORT_SPEED_NA_E;   \
            break;                              \
    }

#define CPSS_TO_MPD_SPEED(speed, mpdSpeed)         \
    switch(speed)                                  \
    {                                              \
        case CPSS_PORT_SPEED_10_E:                 \
            mpdSpeed = MPD_SPEED_10M_E;            \
            break;                                 \
        case CPSS_PORT_SPEED_100_E:                \
            mpdSpeed = MPD_SPEED_100M_E;           \
            break;                                 \
        case CPSS_PORT_SPEED_1000_E:               \
            mpdSpeed = MPD_SPEED_1000M_E;          \
            break;                                 \
        default:                                   \
            mpdSpeed = MPD_SPEED_LAST_E;           \
            break;                                 \
    }

/**
* @internal prvCpssDxChCfgPortPhyObjReload function
* @endinternal
*
* @brief  Init/Reload the MAC DRV functions.
* @note   Called once for Static library linking.
*         Called multiple times during Shared lib usage.
*
* @param[in,out] macPhyObjPtr       - Pointer to MAC DRV object.
*/
GT_VOID prvCpssDxChCfgPortPhyObjReload
(
    CPSS_MACDRV_OBJ_STC     * macPhyObjPtr
);

/**
* @internal prvCpssDxChPhyMpdSharedDbInit function
* @endinternal
*
* @brief  Init the MPD Global Shared Memory
* @note   This function should be called before prvCpssDxChPhyMpdInit]
*
* @param[in,out] mpdGlobalSharedDbPtr   - Pointer to MPD Global Shared Data
*
* @retval GT_OK                   - on success
*/
GT_VOID prvCpssDxChPhyMpdSharedDbInit
(
    INOUT GT_VOID       ** mpdGlobalSharedDbPtr
);

/**
* @internal prvCpssDxChPhyMpdNonSharedDbInit function
* @endinternal
*
* @brief  Init the MPD Global Non-Shared Variable Structure
* @note   This function should be called before prvCpssDxChPhyMpdInit
*
* @param[in,out] mpdGlobalNonSharedDbPtr    - Pointer to MPD Global Non-Shared Data
*/
GT_VOID prvCpssDxChPhyMpdNonSharedDbInit
(
    INOUT GT_VOID       ** mpdGlobalNonSharedDbPtr
);

/**
* @internal prvCpssDxChPhyMpdInit function
* @endinternal
*
* @brief   Init the phy driver
*
* @param[in] devNum                - device number
* @param[in] phyInfo               - array of phy info.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhyMpdInit
(
    IN GT_U8                                    devNum,
    IN const PRV_CPSS_DXCH_PHY_STATIC_INFO_STC* phyInfo
);

/**
* @internal prvCpssDxChPhyMpdReducedInit function
* @endinternal
*
* @brief   Init the phy driver for Secondary Applications in
*          shared library mode.
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on passing null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChPhyMpdReducedInit
(
    IN GT_U8                                   devNum
);

/**
* @internal prvCpssDxChIronman_phyInit function
* @endinternal
*
* @brief   PHY related configurations
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS prvCpssDxChIronman_phyInit
(
    IN  GT_U8  devNum
);

/**
* @internal prvCpssDxChPhyMpdDbClear function
* @endinternal
*
* @brief   clear phy related DB
*
* @param[in] devNum                - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChPhyMpdDbClear
(
    IN GT_U8         devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

