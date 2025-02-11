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
* @file prvTgfCommonFdbManager.h
*
* @brief Fdb manager common test case macros
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfCommonFdbManagerh
#define __prvTgfCommonFdbManagerh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GT_VOID prvTgfBrgFdb_cleanup
(
    GT_BOOL initialClean
);

GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_config
(
    IN GT_U32       fdbManagerId
);


GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_traffic_and_verify
(
    IN GT_U32       fdbManagerId,
    IN GT_BOOL      reducedRun
);


GT_VOID prvTgfBrgFdbManagerLearningScan_statistics_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerLearningScan_config
(
    IN GT_U32       fdbManagerId
);


GT_VOID prvTgfBrgFdbManagerLearningScan_traffic_and_verify
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerLearningScan_no_space_traffic_and_verify
(
    IN GT_U32       fdbManagerId
);


GT_VOID prvTgfBrgFdbManagerLearningScan_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAgingScan_performance_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerDeleteScan_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerDeleteScan_traffic_and_verify
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerDeleteScan_restore
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerTransplantScan_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerTransplantScan_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerTransplantScan_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_traffic_and_verify
(
    IN GT_U32   fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_config
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_restore
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerAddEntry_cuckoo_traffic
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManagerHashTest1_test(GT_VOID);


GT_VOID prvTgfBrgFdbManagerPortLearning_movedMacSaCmd
(
    IN GT_U32       fdbManagerId
);

GT_VOID prvTgfBrgFdbManager_clean
(
    GT_VOID
);

GT_VOID prvTgfBrgFdbManagerInvalidAuq_verify_test
(
    GT_VOID
);

/**
* @internal prvTgfBrgFdbManagerLearningScan_statistics_config function
* @endinternal
*
* @brief  test the limits on learning of dynamic UC MAC entries.
*       according to port/trunk/vlan/global limits
*/
GT_VOID prvTgfBrgFdbManagerBasicLimitDynamicUcLearn
(
    IN GT_U32       fdbManagerId
);

/**
* @internal prvTgfBrgFdbManager_hybridShadowTest function
* @endinternal
*
* @brief  test Hybrid shadow type.
*/
GT_VOID prvTgfBrgFdbManager_hybridShadowTest
(
    GT_VOID
);

/**
* @internal prvTgfBrgFdbManagerBasicRoutingTest function
* @endinternal
*
* @brief   Basic test for FDB based routing.
*/
GT_VOID prvTgfBrgFdbManagerBasicRoutingTest
(
    GT_VOID
);

/* Enable legacy HASH mode state for SIP6 devices */
GT_VOID prvFdbManagerLegacyHashModeForSip6DevicesEnable
(
    GT_U32 enable
);

/* Returns legacy HASH mode state for SIP6 devices */
GT_BOOL prvFdbManagerLegacyHashModeForSip6DevicesGet
(
    GT_VOID
);

/**
* @internal prvTgfBrgFdbManager_2_50_1_1_Add_and_Delete_Mac_entries function
* @endinternal
*
* @brief   convert python test '_2_50_1_1_Add_and_Delete_Mac_entries' of CV
*  to run as embedded test (EnhUT) so , we can compare performance between the
*  2 runs.
****************** the test steps as declared in the CV test
*   Step 1 = Init test parameters
*   Step 2 = Control learning configuration
*   Step 3 = Create FDB manager Add device Add entries. mac DA before adding device to FDB manager, mac DA afterwards
*   Step 4 = Send UC to mac DA and check it was received
*   Step 5 = Send UC to mac DA and check it was received
*   Step 6 = Read FDB manager. Expected 4 entries - 2 added by manager, 2 learned
*   Step 7 = Delete FDB manager entry for mac
*   Step 8 = Send UC to deleted mac DA and check flooding
*   Step 9 = Post test
*
*/
GT_VOID prvTgfBrgFdbManager_2_50_1_1_Add_and_Delete_Mac_entries
(
    GT_VOID
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfCommonFdbManagerh */


