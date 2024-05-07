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
* @file cpssDxChIpfixFw.h
*
* @brief Structures, enums and CPSS APIs declarations for Ipfix Firmware config.
*
* @version   1
*********************************************************************************
**/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/ipfix/cpssDxChIpfixFw.h>
#include <cpss/dxCh/dxChxGen/ipfix/private/prvCpssDxChIpfixFw.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

#define PRV_IPFIX_FW_HW_TESTS_DISABLE 1
#if PRV_IPFIX_FW_HW_TESTS_DISABLE
    #define PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC SKIP_TEST_MAC
#else
    #define PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC
#endif

/* [TBD] update if required */
#define PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS    0
#define PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS     200
#define PRV_IPFIX_FW_UT_NUM_FLOWS_DEFAULT_CNS   CPSS_DXCH_IPFIX_FW_MAX_FLOWS_1K_E
#define PRV_IPFIX_FW_UT_EM_INDEX_DEFAULT_CNS    350

/*
GT_STATUS cpssDxChIpfixFwEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN GT_BOOL ipfixEnable
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwEnableSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    GT_U32      ipfixEnable = GT_TRUE;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwEnableSet(dev, sCpuNum, ipfixEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        ipfixEnable = GT_FALSE;
        rc = cpssDxChIpfixFwEnableSet(dev, sCpuNum, ipfixEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        ipfixEnable = GT_TRUE;

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwEnableSet(dev, sCpuNum, ipfixEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwEnableSet(dev, sCpuNum, ipfixEnable);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwEnableSet(dev, sCpuNum, ipfixEnable);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwGlobalConfigSet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC *globalCfgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwGlobalConfigSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    CPSS_DXCH_IPFIX_FW_GLOBAL_CONFIG_STC globalCfg;

    cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    globalCfg.scanCycleInterval = 10;
    globalCfg.scanLoopItems = 10;

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwGlobalConfigSet(dev, sCpuNum, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        rc = cpssDxChIpfixFwGlobalConfigSet(dev, sCpuNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwGlobalConfigSet(dev, sCpuNum, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        /* [TBD] check more global cfg params */

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwGlobalConfigSet(dev, sCpuNum, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwGlobalConfigSet(dev, sCpuNum, &globalCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwGlobalConfigGet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwGlobalConfigGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwGlobalConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwGlobalConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwGlobalConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwGlobalConfigGet(dev,sCpuNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*
GT_STATUS cpssDxChIpfixFwEntryAdd
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN GT_U32 flowId,
    IN GT_U32 emIndex
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwEntryAdd)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    GT_U32      flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;
    CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC  ipfixEntryParams;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    cpssOsMemSet(&ipfixEntryParams, 0, sizeof(CPSS_DXCH_IPFIX_FW_ENTRY_PARAMS_STC));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwEntryAdd(dev, sCpuNum, flowId, &ipfixEntryParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwEntryAdd(dev, sCpuNum, flowId, &ipfixEntryParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
        flowId = PRV_CPSS_DXCH_IPFIX_FW_FLOWS_MAX_CNS(dev);

        rc = cpssDxChIpfixFwEntryAdd(dev, sCpuNum, flowId, &ipfixEntryParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;
        ipfixEntryParams.emIndex = PRV_CPSS_DXCH_IPFIX_FW_EM_INDEX_MAX_CNS(dev);

        rc = cpssDxChIpfixFwEntryAdd(dev, sCpuNum, flowId, &ipfixEntryParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        ipfixEntryParams.emIndex = PRV_IPFIX_FW_UT_EM_INDEX_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwEntryAdd(dev, sCpuNum, flowId, &ipfixEntryParams);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwEntryAdd(dev, sCpuNum, flowId, &ipfixEntryParams);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwEntryDelete
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN GT_U32 flowId
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwEntryDelete)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    GT_U32      flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwEntryDelete(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwEntryDelete(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
        flowId = PRV_CPSS_DXCH_IPFIX_FW_FLOWS_MAX_CNS(dev);
        rc = cpssDxChIpfixFwEntryDelete(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwEntryDelete(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwEntryDelete(dev,sCpuNum,flowId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwEntryDeleteAll
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwEntryDeleteAll)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwEntryDeleteAll(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwEntryDeleteAll(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwEntryDeleteAll(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwEntryDeleteAll(dev,sCpuNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwDataGet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN GT_U32 flowId
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwDataGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    GT_U32      flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwDataGet(dev, sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwDataGet(dev, sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
        flowId = PRV_CPSS_DXCH_IPFIX_FW_FLOWS_MAX_CNS(dev);
        rc = cpssDxChIpfixFwDataGet(dev, sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwDataGet(dev, sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwDataGet(dev, sCpuNum,flowId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwDataGetAll
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT flowType,
    IN GT_U32 maxNumOfFlows
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwDataGetAll)
{
    GT_U8                            dev;
    GT_STATUS                        rc = GT_OK;
    GT_U32                           sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    CPSS_DXCH_IPFIX_FW_MAX_FLOWS_ENT maxNumOfFlows = PRV_IPFIX_FW_UT_NUM_FLOWS_DEFAULT_CNS;
    CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ENT flowType = CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        for (flowType=CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E;
               flowType<=CPSS_DXCH_IPFIX_FW_FLOW_TYPE_LASTLY_ACTIVE_ELEPHANTS_E; flowType++)
        {
            rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
        }

        rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        flowType = CPSS_DXCH_IPFIX_FW_FLOW_TYPE_ALL_ACTIVE_E;
        maxNumOfFlows = CPSS_DXCH_IPFIX_FW_MAX_FLOWS_LAST_E;
        rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        maxNumOfFlows = PRV_IPFIX_FW_UT_NUM_FLOWS_DEFAULT_CNS;
        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwDataGetAll(dev,sCpuNum,flowType,maxNumOfFlows);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwIpcMsgFetch
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN GT_U32 numOfMsgs,
    OUT GT_U32 *numOfMsgsFetchedPtr,
    OUT GT_U32 *pendingMsgBuffSizePtr,
    OUT CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT *ipcMsgArrPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwIpcMsgFetch)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    GT_U32      numOfMsgs = 3;
    GT_U32      numOfMsgsFetched = 0;
    GT_U32      pendingMsgBuffSize = 0;
    CPSS_DXCH_IPFIX_FW_IPC_MSG_RECV_UNT ipcMsgArr[3];

    cpssOsMemSet(ipcMsgArr, 0, sizeof(ipcMsgArr));

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         &pendingMsgBuffSize, ipcMsgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         &pendingMsgBuffSize, ipcMsgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, NULL,
                                         &pendingMsgBuffSize, ipcMsgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         NULL, ipcMsgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         &pendingMsgBuffSize, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        numOfMsgs = 9999;
        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         &pendingMsgBuffSize, ipcMsgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        numOfMsgs = 3;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         &pendingMsgBuffSize, ipcMsgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwIpcMsgFetch(dev, sCpuNum, numOfMsgs, &numOfMsgsFetched,
                                         &pendingMsgBuffSize, ipcMsgArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwExportConfigSet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC *exportCfgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwExportConfigSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    CPSS_DXCH_IPFIX_FW_EXPORT_CONFIG_STC exportCfg;

    cpssOsMemSet(&exportCfg, 0, sizeof(exportCfg));
    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwExportConfigSet(dev,sCpuNum,&exportCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwExportConfigSet(dev,sCpuNum,&exportCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
        rc = cpssDxChIpfixFwExportConfigSet(dev,sCpuNum, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        /* [TBD] check other params of exportCfg */

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwExportConfigSet(dev,sCpuNum,&exportCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwExportConfigSet(dev,sCpuNum,&exportCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwExportConfigGet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwExportConfigGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwExportConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwExportConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwExportConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwExportConfigGet(dev,sCpuNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwElephantConfigSet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC *elephantCfgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwElephantConfigSet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    CPSS_DXCH_IPFIX_FW_ELEPHANT_CONFIG_STC elephantCfg;

    cpssOsMemSet(&elephantCfg, 0, sizeof(elephantCfg));
    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwElephantConfigSet(dev,sCpuNum,&elephantCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwElephantConfigSet(dev,sCpuNum,&elephantCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
        rc = cpssDxChIpfixFwElephantConfigSet(dev,sCpuNum,NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, rc, dev);

        /* [TBD] check other params of elephantCfg */

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwElephantConfigSet(dev,sCpuNum,&elephantCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwElephantConfigSet(dev,sCpuNum,&elephantCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwElephantConfigGet
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwElephantConfigGet)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwElephantConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwElephantConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwElephantConfigGet(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwElephantConfigGet(dev,sCpuNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwDataGetAllStop
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwDataGetAllStop)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwDataGetAllStop(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwDataGetAllStop(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwDataGetAllStop(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwDataGetAllStop(dev,sCpuNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwDataClear
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum,
    IN GT_U32 flowId
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwDataClear)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
    GT_U32      flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwDataClear(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwDataClear(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;
        flowId = PRV_CPSS_DXCH_IPFIX_FW_FLOWS_MAX_CNS(dev);
        rc = cpssDxChIpfixFwDataClear(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OUT_OF_RANGE, rc, dev);

        flowId = 0;
        rc = cpssDxChIpfixFwDataClear(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        flowId  = PRV_IPFIX_FW_UT_FLOW_ID_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwDataClear(dev,sCpuNum,flowId);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwDataClear(dev,sCpuNum,flowId);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}


/*
GT_STATUS cpssDxChIpfixFwDataClearAll
(
    IN GT_U8 devNum,
    IN GT_U32 sCpuNum
);
*/
UTF_TEST_CASE_MAC(cpssDxChIpfixFwDataClearAll)
{
    GT_U8       dev;
    GT_STATUS   rc = GT_OK;
    GT_U32      sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

    /* This test is not applicable for simulation */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_IPFIX_FW_SKIP_TEST_ON_BOARD_MAC

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        /* Load FW */
        rc = cpssDxChIpfixFwInit(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        /* check for proper register setting */
        rc = cpssDxChIpfixFwDataClearAll(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);

        sCpuNum = PRV_CPSS_DXCH_IPFIX_AC5PX_FW_SCPU_MAX_CNS+1;
        rc = cpssDxChIpfixFwDataClearAll(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);

        sCpuNum = PRV_IPFIX_FW_UT_SCPU_NUM_DEFAULT_CNS;

        /* Restore FW */
        rc = cpssDxChIpfixFwRestore(dev, sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, dev);
    }

    /*  For not-active devices and devices from non-applicable family
        check that function returns GT_BAD_PARAM.
    */

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_AC5P_E | UTF_AC5X_E));

    /* Go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        rc = cpssDxChIpfixFwDataClearAll(dev,sCpuNum);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, rc, dev);
    }

    /* Call function with out of bound value for device id. */
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;
    rc = cpssDxChIpfixFwDataClearAll(dev,sCpuNum);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, rc, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChIpfixManager suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChIpfixFw)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwEnableSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwGlobalConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwGlobalConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwEntryAdd)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwEntryDelete)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwEntryDeleteAll)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwDataGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwDataGetAll)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwIpcMsgFetch)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwExportConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwExportConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwElephantConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwElephantConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwDataGetAllStop)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwDataClear)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIpfixFwDataClearAll)
UTF_SUIT_END_TESTS_MAC(cpssDxChIpfixFw)
