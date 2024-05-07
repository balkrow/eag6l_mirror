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
* @file prvCpssTmCatchUp.h
*
* @brief CPSS BobK TM  catch up utilities.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssTmCatchUp
#define __prvCpssTmCatchUp

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TM_HA_ENABLE_DUMPS 1
#define TM_HA_EXIT_ON_ERROR 0
#define TM_HA_BREAK_ON_ERROR 0

#define __TM_CATCHUP_ERR_LOG(_rc,x)     \
    if (_rc != 0)                           \
    {                                       \
        rc1 |= rc;                          \
        if(TM_HA_ENABLE_DUMPS)              \
        {                                   \
            /*cpssOsPrintf("Error (rc=%d): %s: %d\n  " x, _rc, __FILE__, __LINE__,##__VA_ARGS__);*/ \
            CPSS_LOG_ERROR_MAC(" (rc=%d) " x, _rc); \
        }                                   \
        if(TM_HA_EXIT_ON_ERROR)             \
        {                                   \
            goto out;                       \
        }                                   \
    }

#define __TM_CATCHUP_ERR_ARG1_LOG(_rc,x, arg1)     \
    if (_rc != 0)                           \
    {                                       \
        rc1 |= rc;                          \
        if(TM_HA_ENABLE_DUMPS)              \
        {                                   \
            /*cpssOsPrintf("Error (rc=%d): %s: %d\n  " x, _rc, __FILE__, __LINE__,##__VA_ARGS__);*/ \
            CPSS_LOG_ERROR_MAC(" (rc=%d) " x, _rc, arg1); \
        }                                   \
        if(TM_HA_EXIT_ON_ERROR)             \
        {                                   \
            goto out;                       \
        }                                   \
    }

#define __TM_CATCHUP_ERR_ARG2_LOG(_rc,x, arg1, arg2)     \
    if (_rc != 0)                           \
    {                                       \
        rc1 |= rc;                          \
        if(TM_HA_ENABLE_DUMPS)              \
        {                                   \
            /*cpssOsPrintf("Error (rc=%d): %s: %d\n  " x, _rc, __FILE__, __LINE__,##__VA_ARGS__);*/ \
            CPSS_LOG_ERROR_MAC(" (rc=%d) " x, _rc, arg1, arg2); \
        }                                   \
        if(TM_HA_EXIT_ON_ERROR)             \
        {                                   \
            goto out;                       \
        }                                   \
    }

#define __TM_CATCHUP_DBG_LOG(x)     \
    if (TM_HA_ENABLE_DUMPS)             \
    {                                   \
        /*cpssOsPrintf(x, ##__VA_ARGS__);*/ \
        CPSS_LOG_INFORMATION_MAC(x); \
    }

#define __TM_BREAK_LOOP_ON_ERROR             \
    if ((rc != 0) && (TM_HA_BREAK_ON_ERROR)) \
    {                                        \
        break;                               \
    }

/**
* @internal  prvCpssTmCatchUp  function
* @endinternal
*
* @brief   Perform catch up of TM SW database for specific
*          device.
*
* @note   APPLICABLE DEVICES:     Caelum;
* @note  NOT APPLICABLE DEVICES:   Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                  AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssTmCatchUp
(
    IN GT_U8 devNum
);

/**
* @internal  prvCpssTmSyncSwHwForHa  function
* @endinternal
*
* @brief   Loop over all devices and perform catch up of TM
*          configuration for all devices
*
* @note   APPLICABLE DEVICES:     Caelum
* @note  NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    -  on success
*/
GT_STATUS prvCpssTmSyncSwHwForHa
(
    GT_VOID
);

/**
* @internal  prvCpssTmSyncSchedulerValidityCheck  function
* @endinternal
*
* @brief   Check synchronization of hardware and software database
*          of TM confguration.
*
* @note   APPLICABLE DEVICES:     Caelum
* @note  NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    -  on success
*/
GT_STATUS prvCpssTmSyncCatchupValidityCheck
(
    GT_U32  devNum
);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTxqCatchUp */

