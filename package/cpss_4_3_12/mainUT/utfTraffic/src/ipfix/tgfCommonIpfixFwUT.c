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
* @file tgfCommonIpfixFwUT.c
*
* @brief  Functional tests for IPFIX Manager Feature
*
* @version   1
********************************************************************************
*/

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

#include <ipFix/prvAppIpfixFw.h>
#include <ipFix/prvAppIpfixFwDbg.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <ipfix/prvTgfIpfixFw.h>

#define PRV_TGF_IPFIX_FW_HW_TESTS_DISABLE     1
#define IPFIX_FW_SKIP_RUN_ON_SIMULATION       1

#if PRV_TGF_IPFIX_FW_HW_TESTS_DISABLE
    #define PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC SKIP_TEST_MAC
#else
    #define PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC
#endif


/**
 * @brief Test Learning of new flows
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 4 flows and check they are learned
 *          1.3: Send same 4 flows again and check they are not learned again
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixFwNewFlowsLearn)
{
    GT_BOOL withFw = GT_TRUE;
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

#ifdef ASIC_SIMULATION
    withFw = GT_FALSE;
#endif

    prvTgfIpfixFwNewFlowsLearn(withFw, GT_FALSE/*checkDataPkts*/, hwAutoLearnEnable);
}

/**
 * @brief Test whether data packets are being received
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 4 flows from each of the 4 test ports and
 *               check they are learned
 *          1.3: Send same 4 flows from each of the 4 test ports again
 *               for 5 times and check they are not learned again and
 *               flow statistics are as expected
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixFwDataPacketsCheck)
{
    GT_BOOL withFw = GT_TRUE;
    GT_BOOL hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

#ifdef ASIC_SIMULATION
    withFw = GT_FALSE;
#endif

    prvTgfIpfixFwNewFlowsLearn(withFw, GT_TRUE /*checkDataPkts*/, hwAutoLearnEnable);
}

/**
 * @brief Test whether data packets are being received
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 2 flows from each of 4 packet types and
 *               check they are learned
 *          1.3: Send same 2 flows from each of the 4 packet types again
 *               for 5 times and check they are not learned again and
 *               flow statistics are as expected
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixFwAllL4DataPacketsCheck)
{

    GT_STATUS   rc = GT_OK;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      portIdx = 2;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    CPSS_DXCH_EXACT_MATCH_AUTO_LEARN_FLOW_ID_ALLOC_STATUS_STC alStatus;
    GT_U32      i=0;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC flow;
    GT_U32      startFlowId;
    CPSS_DXCH_IPFIX_ENTRY_STC ipfixEntry;
    GT_U32      burstCount = 1;

    GT_BOOL withFw = GT_TRUE;
    GT_BOOL hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

#ifdef ASIC_SIMULATION
    withFw = GT_FALSE;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixFwMain(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwMain failed");

    /* 1.2: Send 4 flows and check they are learned */
    prvTgfIpfixFwAllL4TrafficGenerate(portIdx, burstCount);

    cpssOsTimerWkAfter(3000);
    appDemoIpfixFwDebugCountersDump();

    if ((withFw == GT_FALSE) && (hwAutoLearnEnable == GT_TRUE))
    {
        /* check auto Learning */
        cpssOsMemSet(&alStatus, 0, sizeof(alStatus));
        rc = cpssDxChExactMatchPortGroupAutoLearnFlowIdAllocStatusGet(prvTgfDevNum, CPSS_DXCH_EXACT_MATCH_UNIT_0_E,
                                                                      1, &alStatus);
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, alStatus.numAllocated,
                                         "Auto Learn flows check failed");

        /* Wait for some time so that the new flows are relocated */
        cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_IPFIX_ENTRY_RELOCATION);
    }

    /* 1.3: Send same 4 flows again and check they are not learned again */
    prvTgfIpfixFwAllL4TrafficGenerate(portIdx, burstCount);

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_DEBUG_DUMP);
    appDemoIpfixFwDebugCountersDump();
    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);

    /* Packets are not mirrored for transmit of just one packet  */
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount*4, dbgCounters.firstPktsFetchedByHost,
                                 "First packets fetch check failed");

    if (withFw == GT_TRUE)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount * 4, dbgCounters.activeFlows,
                                     "Active flows check failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_CONFIG_ERROR_E],
                                     "Received config errors");
        UTF_VERIFY_EQUAL0_STRING_MAC(dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_DATA_GET_ALL_E],
                                     dbgCounters.ipcMsgsCount[CPSS_DXCH_IPFIX_FW_IPC_MSG_OPCODE_EXPORT_COMPLETION_E],
                                     "DataGetAll is not matching export completion");

        startFlowId = 0;
        for (i=0; i<burstCount; i++)
        {
            cpssOsMemSet(&flow, 0, sizeof(flow));
            rc = appDemoIpfixFwNextActiveFlowGet(prvTgfDevNum, startFlowId, &flow);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwNextActiveFlowGet failed");
            if (rc != GT_OK)
            {
                break;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(1, flow.dataRecord.packetCount[0],
                                         "Packet count didn't match for flow: %d", flow.dataRecord.flowId);
            startFlowId = flow.info.flowId + 1;
        }
    }
    else
    {
        startFlowId = 0;
        for (i=0; i<burstCount; i++)
        {
            cpssOsMemSet(&flow, 0, sizeof(flow));
            rc = appDemoIpfixFwNextActiveFlowGet(prvTgfDevNum, startFlowId, &flow);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwNextActiveFlowGet failed");
            if (rc != GT_OK)
            {
                break;
            }

            cpssOsMemSet(&ipfixEntry, 0, sizeof(ipfixEntry));
            rc = cpssDxChIpfixEntryGet(prvTgfDevNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E,
                                       (flow.info.flowId & 0xFFF), CPSS_DXCH_IPFIX_RESET_MODE_DISABLED_E,
                                       &ipfixEntry);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChIpfixEntryGet failed");
            UTF_VERIFY_EQUAL1_STRING_MAC(1, ipfixEntry.packetCount,
                                         "Packet count didn't match for flow: %d", flow.info.flowId);

            startFlowId = flow.info.flowId + 1;
        }
    }
#if 0
    rc = appDemoIpfixFwCpuRxDumpEnable(GT_FALSE);
#endif
    /* 1.4: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @brief Test whether data packets are being received
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 4 flows and check they are learned
 *          1.3: delete 2 Active flows
 *          1.4: Send same 4 flows again and check they are learned again and
 *               flow statistics are as expected
 *          1.5: Delete all flows
 *          1.6: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixFwEntriesAddDelete)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC flow;
    GT_U32      portIdx = 1;
    GT_U32      deletedFlows = 0;
    GT_U32      burstCount = 4;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_U32      currFlowId = 0;
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixFwMain(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwMain failed");

    appDemoIpfixFwCpuRxDumpEnable(GT_FALSE);

    /* 1.2: Send 4 flows and check they are learned */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwStatisticsDump failed");

    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "activeFlows counters check failed");

    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwStatisticsDump failed");

    /* 1.3: Delete half of the flows */
    deletedFlows = 0;
    currFlowId = 0;
    for (i = 0; i < burstCount/2; i++)
    {
        cpssOsMemSet(&flow, 0, sizeof(flow));
        rc = appDemoIpfixFwNextActiveFlowGet(prvTgfDevNum, currFlowId, &flow);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwFlowGet failed");
        if (rc == GT_OK)
        {
            rc = appDemoIpfixFwEntryDelete(prvTgfDevNum, sCpuNum, flow.info.flowId);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwEntryDelete failed");

            currFlowId = flow.info.flowId + 1;
            deletedFlows++;
        }
        else
        {
            break;
        }
    }

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
    {
        /* Wait some time so that the entries were deleted. */
        cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_FOR_IPC_MSG_RETURN);
        rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwStatisticsDump failed");
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount-burstCount/2, dbgCounters.activeFlows,
                                     "activeFlows counters check failed");

        rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwStatisticsDump failed");
    }

    /* 1.4: Send 4 flows again two times and check they are learned */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);

    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwDebugCountersGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount,
                                 dbgCounters.activeFlows,
                                 "activeFlows counters check failed");
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwStatisticsDump failed");

    /* 1.5 delete all flows */
    rc = appDemoIpfixFwEntryDeleteAll(prvTgfDevNum, sCpuNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwEntryDeleteAll failed");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwDebugCountersGet failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                 dbgCounters.activeFlows,
                                 "activeFlows counters check failed");
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwStatisticsDump failed");

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @brief Aging due to idle timeout test
 *        1. After learning of flows, wait until timeout.
 *        2. Check the entries are timeout and deleted.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingIdleTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_DEBUG_DUMP);

    /* 1.3 check flows are learnt */
    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    /* 1.4 Wait for age out */
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_FOR_AGED_OUT);

    /* 1.5 check flows are aged out */
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");

}

/**
 * @brief Aging due to max idle timeout test
 *        1. After learning of flows, wait until timeout.
 *        2. Check the entries are timeout and deleted.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingMaxIdleTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_IDLE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_DEBUG_DUMP);

    /* 1.3 check flows are learnt */
    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    /* 1.4 Wait for age out */
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_FOR_MAX_AGED_OUT);

    /* 1.5 check flows are aged out */
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");

}

/**
 * @brief Aging due to max extended idle timeout test
 *        1. After learning of flows, wait until timeout.
 *        2. Check the entries are timeout and deleted.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingMaxExtendedIdleTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_EXTENDED_IDLE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_DEBUG_DUMP);

    /* 1.3 check flows are learnt */
    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    /* 1.4 Wait for age out */
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_FOR_MAX_EXTENDED_AGED_OUT);

    /* 1.5 check flows are aged out */
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");

}

/**
 * @brief Aging due to Long Lasting timeout test
 *        1. After learning of flows, keep sending traffic regularly.
 *        2. Check the entries are timeout and deleted after long lasting timeout.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingActiveTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);
    GT_U32      i = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_ACTIVE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(3000);

    /* Longlasting time out is 20sec. every 4 seconds send a packet for three times */
    for(i=0; i<6; i++)
    {
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(4 * 1000);
        if(i==0)
        {
            /* 1.3 check flows are learnt */
            appDemoIpfixFwDebugCountersDump();
            appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        }
        if(i != 5)
        {
            rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                     "Debug counters check failed for active entries");
        }

    }

    /* 1.5 check flows are aged out */
    appDemoIpfixFwDebugCountersDump();
    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @brief Aging due to Long Lasting max timeout test
 *        1. After learning of flows, keep sending traffic regularly.
 *        2. Check the entries are timeout and deleted after long lasting timeout.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingMaxActiveTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);
    GT_U32      i = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_ACTIVE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(3000);

    /* Longlasting time out is 240sec. every 4 seconds send a packet for three times */
    for(i=0; i<61; i++)
    {
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(4 * 1000);
        if(i==0)
        {
            /* 1.3 check flows are learnt */
            appDemoIpfixFwDebugCountersDump();
            appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        }
        if(i != 60)
        {
            rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                     "Debug counters check failed for active entries");
        }

    }

    /* 1.5 check flows are aged out */
    appDemoIpfixFwDebugCountersDump();
    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @brief Aging due to Long Lasting max extended timeout test
 *        1. After learning of flows, keep sending traffic regularly.
 *        2. Check the entries are timeout and deleted after long lasting timeout.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingMaxExtendedActiveTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 4;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);
    GT_U32      i = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_MAX_EXTENDED_ACTIVE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(3000);

    /* Longlasting time out is 900sec. every 4 seconds send a packet for three times */
    for(i=0; i<226; i++)
    {
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(4 * 1000);
        if(i==0)
        {
            /* 1.3 check flows are learnt */
            appDemoIpfixFwDebugCountersDump();
            appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        }
        if(i != 225)
        {
            rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                     "Debug counters check failed for active entries");
        }

    }

    /* 1.5 check flows are aged out */
    appDemoIpfixFwDebugCountersDump();
    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @brief Aging due to idle or active timeout test
 *        1. After learning of flows, wait until timeout.
 *        2. Check the entries are timeout and deleted.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingIdleActiveTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 10;
    GT_U32      i = 0;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_ACTIVE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(3000);

    /* Longlasting time out is 50sec. every 4 seconds send a packet for three times */
    for(i=0; i<13; i++)
    {
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(4 * 1000);
        if(i==0)
        {
            /* 1.3 check flows are learnt */
            appDemoIpfixFwDebugCountersDump();
            appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        }
        if(i != 12)
        {
            rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                     "Debug counters check failed for active entries");
        }
    }

    /* 1.4 Wait for age out */
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_FOR_IDLE_ACTIVE_AGED_OUT);

    /* 1.5 check flows are aged out */
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

/**
 * @brief Aging due to idle or active timeout test
 *        1. After learning of flows, wait until timeout.
 *        2. Check the entries are timeout and deleted.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwAgingIdleActiveMultipleTimeOut)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 10;
    GT_U32      i = 0;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_AGING_IDLE_ACTIVE_TIMEOUT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    cpssOsTimerWkAfter(100000);

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(3000);

    /* Longlasting time out is 50sec. every 4 seconds send a packet for three times */
    for(i=0; i<13; i++)
    {
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(4 * 1000);
        if(i==0)
        {
            /* 1.3 check flows are learnt */
            appDemoIpfixFwDebugCountersDump();
            appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        }
        if(i != 12)
        {
            rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
            UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                     "Debug counters check failed for active entries");
        }
    }

    /* 1.4 Wait for age out */
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_FOR_IDLE_ACTIVE_AGED_OUT);

    /* 1.5 check flows are aged out */
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}

UTF_TEST_CASE_MAC(tgfIpfixFwElephantFlows)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 2;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_ELEPHANT_BASIC_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 2 elephant flows */
    prvTgfIpfixFwElephantFlowsTrafficGenerate(portIdx, burstCount, GT_TRUE /*isJumbo*/);
    prvTgfIpfixFwElephantFlowsTrafficGenerate(portIdx, burstCount, GT_TRUE /*isJumbo*/);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);

    /* 1.3 check flows are learnt */
    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ELEPHANTS_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows, "active flows check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.elephantFlows, "elephant flows check failed");

    /* 1.4 Send same flows but normal flows */
    prvTgfIpfixFwElephantFlowsTrafficGenerate(portIdx, burstCount, GT_FALSE /*isJumbo*/);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);

    /* 1.5 check flows are changed to mice */
    appDemoIpfixFwDebugCountersDump();
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ELEPHANTS_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows, "active flows check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.elephantFlows, "elephant flows check failed");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");

}

/**
 * @brief Periodic data export.
 *        1. After learning of flows, keep sending traffic regularly.
 *        2. Check counters are regularly counted in poriodic export mode.
*/
UTF_TEST_CASE_MAC(tgfIpfixFwPeriodicDataExport)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portIdx = 1;
    GT_U32      burstCount = 2;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);
    GT_U32      i=0, j = 0;
    PRV_APP_IPFIX_FW_FLOWS_DB_ENTRY_STC flow;
    GT_U32      startFlowId;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = prvTgfIpfixFwMain(prvTgfDevNum, sCpuNum, PRV_TGF_IPFIX_FW_TEST_TYPE_PERIODIC_DATA_EXPORT_E, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpfixFwMain failed");

    /* 1.2 Send 4 flows */
    prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);

    /* Longlasting time out is 20sec. every 6 seconds send a packet for three times */
    for(i=0; i<8; i++)
    {
        prvTgfIpfixFwTrafficGenerate(portIdx, burstCount, GT_FALSE);
        cpssOsTimerWkAfter(4000);
        if(i==0)
        {
            /* 1.3 check flows are learnt */
            appDemoIpfixFwDebugCountersDump();
            appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
        }
        rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
        UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");
        startFlowId = 0;
        for (j=0; j<burstCount; j++)
        {
            cpssOsMemSet(&flow, 0, sizeof(flow));
            rc = appDemoIpfixFwNextActiveFlowGet(prvTgfDevNum, startFlowId, &flow);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwNextActiveFlowGet failed");
            if (rc != GT_OK)
            {
                break;
            }

            UTF_VERIFY_EQUAL1_STRING_MAC(1+i, flow.dataRecord.packetCount[0],
                                         "Packet count didn't match for flow: %d", flow.dataRecord.flowId);
            startFlowId = flow.info.flowId + 1;
        }
    }

    /* 1.5 check flows are aged out */
    appDemoIpfixFwDebugCountersDump();
    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 0xFF, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);
    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "Debug counters check failed for active entries");

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /*1.7: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");
}


/**
 * @brief Test Learning of new flows
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 15K flows and check they are learned
 *          1.4: Delete Ipfix manager and restore configurations
 */
UTF_TEST_CASE_MAC(tgfIpfixFwFlowsLearnFullCapacity)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum           = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);
    GT_U32      burstCount        = PRV_TGF_IPFIX_FULL_CAPACITY_FLOWS_NUM_CNS(prvTgfDevNum);
    GT_U32      collisonCount = 0;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;

    (void) dbgCounters;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixFwMain(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwMain failed");

    rc = appDemoIpfixFwCpuRxDumpEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwCpuRxDumpEnable failed");

    /* 1.2: Send flows and check they are learned */
    prvTgfIpfixFwFlowsLearnFullCapacityTrafficGenerate(burstCount, &collisonCount);

    cpssOsTimerWkAfter(2*IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixFwDebugCountersDump();

    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount - collisonCount, dbgCounters.activeFlows,
                                 "activeFlows counters check failed");

    PRV_UTF_LOG2_MAC("[Info] activeFlows: %d, collisonCount: %d\n", dbgCounters.activeFlows, collisonCount);
    PRV_UTF_LOG2_MAC("[Info] TotalFlows: %d, AccountedFlows: %d\n", burstCount, dbgCounters.activeFlows + collisonCount);

    /* appDemoIpfixFwStatisticsDump(prvTgfDevNum, 0, 128, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E); */

    /* 1.4: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");

}

/**
 * @brief Test Add Delete Flows
 *          1.1: Ipfix Manager config and initialization
 *          1.2: Send 100 flows and check they are learned.
 *          1.3: Delete all flow and send traffic again
 *          Delete Ipfix manager and restore configurations
 *
 */
UTF_TEST_CASE_MAC(tgfIpfixFwFlowsAddDeleteFullCapacity)
{
    GT_STATUS   rc = GT_OK;
    PRV_APP_IPFIX_FW_DBG_COUNTERS_STC dbgCounters;
    GT_U32      sCpuNum = PRV_TGF_IPFIX_SCPU_NUM_CNS(prvTgfDevNum);
    GT_BOOL     hwAutoLearnEnable = PRV_TGF_IPFIX_HW_AUTO_LEARN_ENABLE_DEFAULT(prvTgfDevNum);
    GT_U32      burstCount = 100;
    GT_U32      collisonCount = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E|
                                        UTF_BOBCAT2_E|UTF_BOBCAT3_E|UTF_CAELUM_E|UTF_AC3X_E|
                                        UTF_ALDRIN_E|UTF_ALDRIN2_E | UTF_AC3X_E |
                                        UTF_IRONMAN_L_E | UTF_AAS_E|UTF_HARRIER_E);

#if IPFIX_FW_SKIP_RUN_ON_SIMULATION /* Set it to 0 to run in simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TGF_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;
#endif

    /* 1.1: Ipfix Manager config and initialization */
    rc = appDemoIpfixFwMain(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwMain failed");

    rc = appDemoIpfixFwCpuRxDumpEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwCpuRxDumpEnable failed");

    /* 1.2: Send flows and check they are learned */
    prvTgfIpfixFwFlowsLearnFullCapacityTrafficGenerate(burstCount, &collisonCount);

    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_TRAFFIC_SEND);
    appDemoIpfixFwDebugCountersDump();

    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.firstPktsFetchedByHost,
                                 "firstPktsFetchedByHost counters check failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(burstCount, dbgCounters.activeFlows,
                                 "activeFlows counters check failed");

    appDemoIpfixFwStatisticsDump(prvTgfDevNum, 1024, 128, CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E);

    /* 1.3 Delete All Flows */
    rc = appDemoIpfixFwEntryDeleteAll(prvTgfDevNum, sCpuNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwEntryDeleteAll failed");
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_AFTER_DELETE_SEND);

    rc = appDemoIpfixFwDebugCountersGet(&dbgCounters);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, dbgCounters.activeFlows,
                                 "activeFlows counters check failed");

    /* 1.4: Send flows and check they are learned */
    prvTgfIpfixFwFlowsLearnFullCapacityTrafficGenerate(burstCount, &collisonCount);
    cpssOsTimerWkAfter(IPFIX_FW_WAIT_TIME_BEFORE_CLEAN);

    /* 1.5: Delete Ipfix manager and restore configurations */
    rc = appDemoIpfixFwClear(prvTgfDevNum, sCpuNum, hwAutoLearnEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoIpfixFwClear failed");

}

UTF_SUIT_BEGIN_TESTS_MAC(tgfIpfixFw)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwNewFlowsLearn)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwDataPacketsCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwEntriesAddDelete)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwElephantFlows)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingIdleTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingMaxIdleTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingMaxExtendedIdleTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingActiveTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingMaxActiveTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingMaxExtendedActiveTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingIdleActiveTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAgingIdleActiveMultipleTimeOut)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwFlowsLearnFullCapacity)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwFlowsAddDeleteFullCapacity)

    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwAllL4DataPacketsCheck)
    UTF_SUIT_DECLARE_TEST_MAC(tgfIpfixFwPeriodicDataExport)
UTF_SUIT_END_TESTS_MAC(tgfIpfixFw)
