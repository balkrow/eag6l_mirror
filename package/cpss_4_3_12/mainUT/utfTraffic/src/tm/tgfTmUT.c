/*******************************************************************************
*              (c), Copyright 2021, Marvell International Ltd.                 *
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
* @file tgfTmUT.c
*
* @brief  Functional tests for Traffic Manager Feature
*
* @version   1
********************************************************************************
*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE) /*not supported in CAP - UTs have appDemo references */

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssCommon/cpssPresteraDefs.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>

#include <tm/prvTgfTm17Ports.h>
#include <tm/prvTgfTmHaBasic.h>

#define PRV_TGF_TM_SKIP_TEST_MAC 1

UTF_TEST_CASE_MAC(tgfTm17Ports)
{
    GT_U32      testPorts = 1;
    GT_U32      burstCount = 1;
    GT_U32      portIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    /* This test involves using of application APIs */
    SKIP_TEST_MAC;
#endif

    prvTgfTm17PortsInitSystem();

    prvTgfTm17PortsVlanConfigSet();

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTm17PortsConfigRestore();
}

UTF_TEST_CASE_MAC(tgfTmHaBasicValidityCheck)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTmHaBasicValidityCheck();
}

UTF_TEST_CASE_MAC(tgfTmHaBasicCatchupCheck)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTmHaBasicCatchupCheck();
}

UTF_TEST_CASE_MAC(tgfTmHaScenariosCheck)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTmHaScenariosCheck();
}

UTF_TEST_CASE_MAC(tgfTmHaBasicMemLeakCheck)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTmHaBasicMemLeakCheck();

}

UTF_TEST_CASE_MAC(tgfTmHaResourceManagerStoreCheck)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTmHaBasicResourceManagerStoreCheck();

}

UTF_TEST_CASE_MAC(tgfTmHaResourceManagerRecoveryCheck)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTmHaBasicResourceManagerRecoveryCheck();

}

/**
 * @brief
 *  1. Perform TM Init and 17 ports init configuration.
 *  2. Perform Play which does add of single flow. This involved adding of new scheduler nodes
 *  3. Send traffic and check if the packet egresses as expected.
 *  4. Perform HA
 *  5. Replay the entires. During replay we play adding of single flow.
 *  6. Check validity
 *  7. Send traffic and check if the packet egresses as expected.
 *  8. Restore configuration
 */
UTF_TEST_CASE_MAC(tgfTmHa17Ports)
{
    GT_U32      testPorts = 1;
    GT_U32      burstCount = 1;
    GT_U32      portIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTm17PortsInitSystem();

    prvTgfTm17PortsVlanConfigSet();

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTmHa17Ports();

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTm17PortsConfigRestore();

}

/**
 * @brief
 *  1. Perform TM Init and 17 ports init configuration and add additional one flow.
 *  2, Perform Play which does add and delete of flows. This involves adding of scheduler nodes
 *  3. Send traffic and check if the packet egresses as expected.
 *  4. Perform HA
 *  5  Replay the entires. During replay only play existing
 *      entries in different order
 *  6. Send traffic and check if the packet egresses as expected.
 *  7. Check validity
 *  8. Restore configuration
 */
UTF_TEST_CASE_MAC(tgfTmHa17PortsFlowsAddOrderChange)
{
    GT_U32      testPorts = 1;
    GT_U32      burstCount = 1;
    GT_U32      portIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTm17PortsInitSystem();

    prvTgfTm17PortsVlanConfigSet();

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTmHa17PortsFlowsAddTest(PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_ORDER_CHANGE_E, GT_FALSE /* Emulate Crash */);

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTm17PortsConfigRestore();

}

/**
 * @brief
 *  1. Perform TM Init and 17 ports init configuration and add additional one flow.
 *  2, Perform Play which does add and delete of flows. This
 *     involved adding and deleting of scheduler nodes
 *  3. Send traffic and check if the packet egresses as expected.
 *  4. Perform HA
 *  5  Replay the entires. During replay only play existing
 *      entries and ignore deleted entries
 *  6. Send traffic and check if the packet egresses as expected.
 *  7. Check validity
 *  8. Restore configuration
 */
UTF_TEST_CASE_MAC(tgfTmHa17PortsFlowsAddDelete)
{
    GT_U32      testPorts = 1;
    GT_U32      burstCount = 1;
    GT_U32      portIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTm17PortsInitSystem();

    prvTgfTm17PortsVlanConfigSet();

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTmHa17PortsFlowsAddTest(PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_DELETE_E, GT_FALSE /* Emulate Crash */);

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTm17PortsConfigRestore();

}

/**
 * @brief
 *  1. Perform TM Init and 17 ports init configuration and add additional one flow.
 *  2, Perform Play which does add and delete of flows. This involves adding of scheduler nodes
 *  3. Send traffic and check if the packet egresses as expected.
 *  4. Perform HA
 *  5  Replay the entires. During replay only play existing
 *      entries in different order
 *  6. Send traffic and check if the packet egresses as expected.
 *  7. Check validity
 *  8. Restore configuration
 */
UTF_TEST_CASE_MAC(tgfTmHa17PortsFlowsAddOrderChangeWithEmulateCrash)
{
    GT_U32      testPorts = 1;
    GT_U32      burstCount = 1;
    GT_U32      portIdx;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E| UTF_AC3X_E | UTF_FALCON_E |
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E | UTF_AC5X_E|
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if PRV_TGF_TM_SKIP_TEST_MAC
    /* This test not to be included in regression */
    SKIP_TEST_MAC;
#endif

    prvTgfTm17PortsInitSystem();

    prvTgfTm17PortsVlanConfigSet();

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTmHa17PortsFlowsAddTest(PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_ORDER_CHANGE_E, GT_TRUE /* Emulate Crash */);

    /* 1.2: Send 4 flows from each of the 2 test ports and check they are learned */
    for (portIdx = 0; portIdx < testPorts; portIdx++)
    {
        prvTgfTm17PortsTrafficGenerate(portIdx, burstCount);
    }

    prvTgfTm17PortsConfigRestore();

}

/**
 * @brief
 * Init commands to be used for running the tests.
 * When appDemo is running as fresh start
 * 1. shell-execute appDemoTmScenarioModeSet 123
 * 2. cpssInitSystem 29,2,0 portMgr
 *
 * When appDemo is not running as fresh start
 * 1. do shell-execute appDemoDbEntryAdd "tmDramFailIgnore" 1
 * 2. do shell-execute appDemoDbgSoftResetSet 0, 1, 1
 * 3. shell-execute appDemoTmScenarioModeSet 123
 * 4. cpssInitSystem 29,2,0 portMgr forceReset
 */

UTF_SUIT_BEGIN_TESTS_MAC(tgfTm)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTm17Ports)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHaBasicValidityCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHaBasicCatchupCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHaScenariosCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHaBasicMemLeakCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHaResourceManagerStoreCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHaResourceManagerRecoveryCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHa17Ports)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHa17PortsFlowsAddOrderChange)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHa17PortsFlowsAddDelete)
    UTF_SUIT_DECLARE_TEST_MAC(tgfTmHa17PortsFlowsAddOrderChangeWithEmulateCrash)
UTF_SUIT_END_TESTS_MAC(tgfTm)

#endif
