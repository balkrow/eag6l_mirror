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
* @file  prvTgfTmHaBasic.c
*
* @brief Traffic Manager High Availability Basic API testing
*
* @version   1
********************************************************************************
*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE) /*not supported in CAP - UTs have appDemo references */

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

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
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <tm/prvTgfTmHaBasic.h>
#include <systemRecovery/prvCpssTmCatchup.h>
#include <systemRecovery/prvCpssTmDbgUtils.h>

extern GT_STATUS appDemoBc2TmScenarioInit(GT_U8 dev);

GT_VOID prvTgfTmHaBasicValidityCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck");
}


GT_VOID prvTgfTmHaBasicCatchupCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    CPSS_SYSTEM_RECOVERY_INFO_STC  systemRecoveryInfo;

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck");

    /* Change state to HA mode */
    cpssOsMemSet(&systemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* Delete TM SW database */
    rc = cpssTmClose(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");

    /* Re-initialize TM sw database */
    rc = cpssTmInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck");

    /* Perform catchup to sync with hardware */
    rc = prvCpssTmCatchUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmCatchUp");

    /* Return to normal mode */
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* check syncup after HA process is complete */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck");

}

GT_VOID prvTgfTmHaScenariosCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    CPSS_SYSTEM_RECOVERY_INFO_STC  systemRecoveryInfo;

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Pre HA");

    /* Change state to HA mode */
    cpssOsMemSet(&systemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* Delete TM SW database */
    rc = cpssTmClose(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");

    /* Re-initialize TM sw database */
    rc = appDemoBc2TmScenarioInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Before Catchup");

    /* Perform catchup to sync with hardware */
    rc = prvCpssTmCatchUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmCatchUp");

    /* Return to normal mode */
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* check syncup after HA process is complete */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: After HA");

}

GT_VOID prvTgfTmHaBasicMemLeakCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 total_mem_start=0, total_mem_end=0;

    total_mem_start = osMemGetHeapBytesAllocated();
    /* Delete TM SW database */
    rc = cpssTmClose(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");
    total_mem_end = osMemGetHeapBytesAllocated();
    cpssOsPrintf("[TM Trace] Mem Freed %d\n", total_mem_start - total_mem_end);

    total_mem_start = osMemGetHeapBytesAllocated();
    /* Re-initialize TM sw database */
    rc = cpssTmInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");
    total_mem_end = osMemGetHeapBytesAllocated();
    cpssOsPrintf("[TM Trace] Mem Allocated %d\n", total_mem_end - total_mem_start);
}

GT_VOID prvTgfTmHaBasicResourceManagerStoreCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_VOID_PTR rmDbPtr = NULL;

    rc = prvCpssTmDbgSwDbStore(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbStore");

    rc = prvCpssTmDbgSwDbCmp(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbCmp");

    rc = prvCpssTmDbgSwDbFree(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbFree");
}

GT_VOID prvTgfTmHaBasicResourceManagerRecoveryCheck
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_VOID_PTR rmDbPtr =  NULL;

    CPSS_SYSTEM_RECOVERY_INFO_STC  systemRecoveryInfo;

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Pre HA");

    /* Take the backup of resource manager */
    rc = prvCpssTmDbgSwDbStore(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbStore");

    /* check resource manger sync */
    rc = prvCpssTmDbgSwDbCmp(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbCmp: Pre HA");

    /* Change state to HA mode */
    cpssOsMemSet(&systemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* Delete TM SW database */
    rc = cpssTmClose(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");

    /* Re-initialize TM sw database */
    rc = appDemoBc2TmScenarioInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Before Catchup");

    /* check resource manger sync */
    rc = prvCpssTmDbgSwDbCmp(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbCmp: Before Catchup");

    /* Perform catchup to sync with hardware */
    rc = prvCpssTmCatchUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmCatchUp");

    /* Return to normal mode */
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* check syncup after HA process is complete */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: After HA");

    /* check resource manger sync */
    rc = prvCpssTmDbgSwDbCmp(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbCmp: After HA");

    rc = prvCpssTmDbgSwDbFree(prvTgfDevNum, &rmDbPtr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDbgSwDbFree");

}

#endif
