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
* @file tgfCommonBridgeFdbManagerUT.c
*
* @brief Enhanced UTs for CPSS Bridge FDB Manager
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <bridgeFdbManager/prvTgfCommonFdbManager.h>

/* includes */
/* the define of UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC must come from C files that
   already fixed the types of ports from GT_U8 !

   NOTE: must come before ANY include to H files !!!!
*/
#define UT_FIXED_PORT_TO_SUPPORT_U32_TYPE_MAC

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify statistics for learning scan:
*/
static GT_VOID prvTgfBrgFdbManagerLearningScan(GT_VOID)
{
    GT_U32      fdbManagerId = 1;
    GT_U32      portMgrValue = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_SHLIB_WM_MAC(UTF_ALL_FAMILY_E);

    /* AUTODOC: SETUP CONFIGURATION: */
    if((prvWrAppDbEntryGet("portMgr", &portMgrValue) == GT_OK) && (portMgrValue != 0))
    {
        /* disable PM task to guaranty CPU time for FDB Manager in the test */
        prvWrAppPortManagerTaskEnableSet(0);
    }

    /* Set configuration */
    prvTgfBrgFdbManagerLearningScan_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerLearningScan_traffic_and_verify(fdbManagerId);

    if (portMgrValue)
    {
        /* enable PM task back */
        prvWrAppPortManagerTaskEnableSet(1);
    }

    /* Restore configuration */
    prvTgfBrgFdbManagerLearningScan_restore(fdbManagerId);
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify statistics for learning scan:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerLearningScan_verify)
{
    prvTgfBrgFdbManagerLearningScan();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify statistics for learning scan (XOR for SIP6 devices):
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerLearningScan_verify_sip6_xor)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_CNS)

    /* Enable legacy HASH mode state for SIP6 devices */
    prvFdbManagerLegacyHashModeForSip6DevicesEnable(1);

    prvTgfBrgFdbManagerLearningScan();

    /* Restore CRC HASH mode state for SIP6 devices */
    prvFdbManagerLegacyHashModeForSip6DevicesEnable(0);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify NO_SPACE and FULL events in learning scan:
*/
static GT_VOID prvTgfBrgFdbManagerLearningScan_no_space(GT_VOID)
{
    GT_U32           fdbManagerId = 1;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_AC5_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    {
        GT_U32      boardIdx, boardRevId, reloadEeprom;
        prvWrAppInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

        /* test takes a lot of time for Falcon 12.8.
           reduce baseline tests time by balance running of this test
           between 35,1 and 35,2 variants */
        if ((boardIdx == 35) && (boardRevId == 1))
        {
            PRV_TGF_SKIP_BASELINE_TYPE_TEST_MAC(PRV_UTF_BASELINE_TYPE_ODD_E);
        }
        else if ((boardIdx == 35) && (boardRevId == 2))
        {
            PRV_TGF_SKIP_BASELINE_TYPE_TEST_MAC(PRV_UTF_BASELINE_TYPE_EVEN_E);
        }
    }

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_SHLIB_WM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfBrgFdbManagerLearningScan_statistics_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerLearningScan_no_space_traffic_and_verify(fdbManagerId);

    /* Restore configuration */
    prvTgfBrgFdbManagerLearningScan_statistics_restore(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify statistics for learning scan:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerLearningScan_no_space)
{
    prvTgfBrgFdbManagerLearningScan_no_space();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify statistics for learning scan:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerLearningScan_statistics_verify)
{
    GT_U32           fdbManagerId = 5;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    {
        GT_U32      boardIdx, boardRevId, reloadEeprom;
        prvWrAppInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

        /* test takes a lot of time for Falcon 12.8.
           reduce baseline tests time by balance running of this test
           between 35,1 and 35,2 variants */
        if ((boardIdx == 35) && (boardRevId == 1))
        {
            PRV_TGF_SKIP_BASELINE_TYPE_TEST_MAC(PRV_UTF_BASELINE_TYPE_ODD_E);
        }
        else if ((boardIdx == 35) && (boardRevId == 2))
        {
            PRV_TGF_SKIP_BASELINE_TYPE_TEST_MAC(PRV_UTF_BASELINE_TYPE_EVEN_E);
        }
    }

    /* Set configuration */
    prvTgfBrgFdbManagerLearningScan_statistics_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerLearningScan_statistics_traffic_and_verify(fdbManagerId,
            GT_FALSE/* reduced Run */);

    /* Restore configuration */
    prvTgfBrgFdbManagerLearningScan_statistics_restore(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify aging scan:
*/
static GT_VOID prvTgfBrgFdbManagerAgingScan_verify(GT_VOID)
{
    GT_U32      fdbManagerId = 0;
    GT_U32      portMgrValue = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    if((prvWrAppDbEntryGet("portMgr", &portMgrValue) == GT_OK) && (portMgrValue != 0))
    {
        /* disable PM task to guaranty CPU time for FDB Manager in the test */
        prvWrAppPortManagerTaskEnableSet(0);
    }

    /* Set configuration */
    prvTgfBrgFdbManagerAgingScan_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerAgingScan_traffic_and_verify(fdbManagerId);

    if (portMgrValue)
    {
        /* enable PM task back */
        prvWrAppPortManagerTaskEnableSet(1);
    }

    /* Restore configuration */
    prvTgfBrgFdbManagerAgingScan_restore(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify aging scan:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerAgingScan_verify)
{
    prvTgfBrgFdbManagerAgingScan_verify();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify aging scan (XOR for SIP6 devices):
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerAgingScan_verify_sip6_xor)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_CNS)

    /* Enable legacy HASH mode state for SIP6 devices */
    prvFdbManagerLegacyHashModeForSip6DevicesEnable(1);

    prvTgfBrgFdbManagerAgingScan_verify();

    /* Restore CRC HASH mode state for SIP6 devices */
    prvFdbManagerLegacyHashModeForSip6DevicesEnable(0);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify Delete scan:
*/
static GT_VOID prvTgfBrgFdbManagerDeleteScan_verify(GT_VOID)
{
    GT_U32           fdbManagerId = 10;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* Set configuration */
    prvTgfBrgFdbManagerDeleteScan_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerDeleteScan_traffic_and_verify(fdbManagerId);

    /* Restore configuration */
    prvTgfBrgFdbManagerDeleteScan_restore(fdbManagerId);
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify Delete scan:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerDeleteScan_verify)
{
    prvTgfBrgFdbManagerDeleteScan_verify();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify Delete scan (XOR for SIP6 devices):
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerDeleteScan_verify_sip6_xor)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_CPSS_PP_ALL_SIP6_CNS)

    /* Enable legacy HASH mode state for SIP6 devices */
    prvFdbManagerLegacyHashModeForSip6DevicesEnable(1);

    prvTgfBrgFdbManagerDeleteScan_verify();

    /* Restore CRC HASH mode state for SIP6 devices */
    prvFdbManagerLegacyHashModeForSip6DevicesEnable(0);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify Transplant scan:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerTransplantScan_verify)
{
    GT_U32           fdbManagerId = 15;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* Set configuration */
    prvTgfBrgFdbManagerTransplantScan_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerTransplantScan_traffic_and_verify(fdbManagerId);

    /* Restore configuration */
    prvTgfBrgFdbManagerTransplantScan_restore(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify aging scan in full scale
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerAgingScan_performance_verify)
{
    GT_U32           fdbManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_SHLIB_WM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfBrgFdbManagerAgingScan_performance_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerAgingScan_performance_traffic_and_verify(fdbManagerId);

    /* Restore configuration */
    prvTgfBrgFdbManagerAgingScan_performance_restore(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify add entry
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerAddEntry_verify)
{
    GT_U32           fdbManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* Set configuration */
    prvTgfBrgFdbManagerAddEntry_config(fdbManagerId);

    /* Generate traffic & verify statistics */
    prvTgfBrgFdbManagerAddEntry_traffic_and_verify(fdbManagerId);

    /* Restore configuration */

    prvTgfBrgFdbManagerAddEntry_restore(fdbManagerId);
}
/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify adding entries using cuckoo algorithm:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerAddEntry_cuckoo)
{
    GT_U32           fdbManagerId = 1;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    if (prvFdbManagerLegacyHashModeForSip6DevicesGet())
    {
        SKIP_TEST_MAC;
    }
    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_SHLIB_WM_MAC(UTF_ALL_FAMILY_E);

    {
        GT_U32      boardIdx, boardRevId, reloadEeprom;
        prvWrAppInitSystemGet(&boardIdx, &boardRevId, &reloadEeprom);

        /* test takes a lot of time for Falcon 12.8.
           reduce baseline tests time by balance running of this test
           between 35,1 and 35,2 variants */
        if ((boardIdx == 35) && (boardRevId == 1))
        {
            PRV_TGF_SKIP_BASELINE_TYPE_TEST_MAC(PRV_UTF_BASELINE_TYPE_ODD_E);
        }
        else if ((boardIdx == 35) && (boardRevId == 2))
        {
            PRV_TGF_SKIP_BASELINE_TYPE_TEST_MAC(PRV_UTF_BASELINE_TYPE_EVEN_E);
        }
    }

    /* Set configuration */
    prvTgfBrgFdbManagerAddEntry_cuckoo_config(fdbManagerId);

    /* Generate traffic */
    prvTgfBrgFdbManagerAddEntry_cuckoo_traffic(fdbManagerId);

    /* Restore configuration */
    prvTgfBrgFdbManagerAddEntry_cuckoo_restore(fdbManagerId);
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify hashing scenario in AC5P/AC5X
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerHashTest1)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E | UTF_FALCON_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_SHLIB_WM_MAC(UTF_ALL_FAMILY_E);

    /* Set configuration */
    prvTgfBrgFdbManagerHashTest1_test();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify hashing scenario in AC5P/AC5X
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerInvalidAuq_verify)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* Set configuration */
    prvTgfBrgFdbManagerInvalidAuq_verify_test();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Clean up FDB:
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManager_clean)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    prvTgfBrgFdbManager_clean();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify add entry
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerPortLearning_movedMacSaCmd)
{
    GT_U32           fdbManagerId = 20;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    /* Set configuration , test it , restore configurations */
    prvTgfBrgFdbManagerPortLearning_movedMacSaCmd(fdbManagerId);
}


/*----------------------------------------------------------------------------*/
/* AUTODOC: test the limits on learning of dynamic UC MAC entries.
*       according to port/trunk/vlan/global limits
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerBasicLimitDynamicUcLearn)
{
    GT_U32      fdbManagerId = 20;
    GT_U32      portMgrValue = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E);

    /* GM is too slow : the test send many packets */
    /* and this is fully SW feature that no added value to run on the GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    if((prvWrAppDbEntryGet("portMgr", &portMgrValue) == GT_OK) && (portMgrValue != 0))
    {
        /* disable PM task to guaranty CPU time for FDB Manager in the test */
        prvWrAppPortManagerTaskEnableSet(0);
    }

    /* Set configuration , test it , restore configurations */
    prvTgfBrgFdbManagerBasicLimitDynamicUcLearn(fdbManagerId);

    if (portMgrValue)
    {
        /* enable PM task back */
        prvWrAppPortManagerTaskEnableSet(1);
    }
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify cases for hybrid model
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManager_hybridShadow)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E  | UTF_AC5_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* BC2 A0 is not supported */
    if(UTF_CPSS_SIP5_0_ONLY_CHECK_MAC(prvTgfDevNum))
         SKIP_TEST_MAC;

    if(prvFdbManagerLegacyHashModeForSip6DevicesGet())
    {
        SKIP_TEST_MAC;
    }

    /* skip this test when 'prvUtfSkipLongTestsFlagSet' for Mutex Profiler */
    PRV_TGF_SKIP_LONG_TEST_MUTEX_PROFILER_MAC(UTF_ALL_FAMILY_E);
    PRV_TGF_SKIP_LONG_TEST_SHLIB_WM_MAC(UTF_ALL_FAMILY_E);

    prvTgfBrgFdbManager_hybridShadowTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test verify cases for hybrid model
*/
UTF_TEST_CASE_MAC(prvTgfBrgFdbManagerBasicRouting)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_AC5_E | UTF_ALDRIN2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    prvTgfBrgFdbManagerBasicRoutingTest();
}

/*----------------------------------------------------------------------------*/
/* AUTODOC: convert python test '_2_50_1_1_Add_and_Delete_Mac_entries' of CV
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
UTF_TEST_CASE_MAC(prvTgfBrgFdbManager_2_50_1_1_Add_and_Delete_Mac_entries)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_LION2_E | UTF_AC5_E | UTF_ALDRIN2_E);

    /* GM Not yet tested, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    prvTgfBrgFdbManager_2_50_1_1_Add_and_Delete_Mac_entries();
}

/*
 * Configuration of tgfBridgeManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfBridgeFdbManager)

    /* Clean Up FDB before FDB manager test suite starts (Must be the first)*/
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManager_clean)

    /* Basic Test case */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerAddEntry_verify)

    /* test from CV for performance comparison */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManager_2_50_1_1_Add_and_Delete_Mac_entries)

    /* per port test : 'movedMacSaCmd' */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerPortLearning_movedMacSaCmd)

    /* Learning Scan */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerLearningScan_verify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerLearningScan_statistics_verify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerLearningScan_no_space)

    /* Aging Scan */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerAgingScan_verify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerDeleteScan_verify)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerTransplantScan_verify)

    /* Performance tests */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerAgingScan_performance_verify)

    /* Cuckoo algorithm */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerAddEntry_cuckoo)

    /* invalid packet in queue - multicast SA */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerInvalidAuq_verify)

    /* learning limits */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerBasicLimitDynamicUcLearn)

    /* Hashing test cases */
    /* AC5P/AC5X - 16K FDB - MHT16 - without cuckoo */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerHashTest1)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManager_hybridShadow)

    /* XOR mode for SIP6 devices */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerLearningScan_verify_sip6_xor)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerAgingScan_verify_sip6_xor)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerDeleteScan_verify_sip6_xor)

    /* FDB manager based basic routing using ipv6 */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfBrgFdbManagerBasicRouting)

UTF_SUIT_END_TESTS_MAC(tgfBridgeFdbManager)
