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
* @file cpssDxChTamUT.c
*
* @brief Unit tests for cpssDxChTam
*
* @version   1
********************************************************************************
*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/tam/private/prvCpssDxChTamIfa.h>
#include <cpss/dxCh/dxChxGen/tam/cpssDxChTamIfa.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>

/* TBD */
#define PRV_TAM_IFA_HW_TESTS_DISABLE 1

#if PRV_TAM_IFA_HW_TESTS_DISABLE
    #define PRV_TAM_IFA_SKIP_TEST_ON_BOARD_MAC SKIP_TEST_MAC
#else
    #define PRV_TAM_IFA_SKIP_TEST_ON_BOARD_MAC
#endif

/*
GT_STATUS cpssDxChIfaGlobalConfigSet
(
    IN  GT_U8                    devNum,
    IN  CPSS_DXCH_IFA_PARAMS_STC *ifaParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIfaGlobalConfigSet)
{
    GT_STATUS   st = GT_OK;
    GT_U8       dev;

    CPSS_DXCH_IFA_PARAMS_STC  globalCfg;
    CPSS_DXCH_IFA_PARAMS_STC  globalCfgGet;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TAM_IFA_SKIP_TEST_ON_BOARD_MAC;

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E) );

    /* 1. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&globalCfg,0,sizeof(globalCfg));
        cpssOsMemSet(&globalCfgGet,0,sizeof(globalCfgGet));

        globalCfg.samplePeriod = BIT_8;
        st = cpssDxChIfaGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                    "cpssDxChIfaGlobalConfigSet failed, dev : %d", dev);

        /* NULL pointer check */
        globalCfg.samplePeriod = BIT_8 - 1;
        st = cpssDxChIfaGlobalConfigSet(dev, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PTR, st,
                                "cpssDxChIfaGlobalConfigSet failed, dev: %d", dev);

        /* egressPortDropCountingMode check */
        globalCfg.egressPortDropCountingMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_PACKET_BASED_DROP_COUNTING_MODE_E + 1;
        st = cpssDxChIfaGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIfaGlobalConfigSet failed, dev: %d", dev);

        /* egressPortDropByteMode check */
        globalCfg.egressPortDropCountingMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_PACKET_BASED_DROP_COUNTING_MODE_E;
        globalCfg.egressPortDropByteMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_L3_BASED_DROP_BYTE_MODE_E + 1;
        st = cpssDxChIfaGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                                "cpssDxChIfaGlobalConfigSet failed, dev: %d", dev);

        globalCfg.egressPortDropByteMode = CPSS_DXCH_TAM_IFA_EGRESS_PORT_L3_BASED_DROP_BYTE_MODE_E;
        st = cpssDxChIfaGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChIfaGlobalConfigSet failed, dev: %d", dev);

        st = cpssDxChIfaGlobalConfigGet(dev, &globalCfgGet);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, st,
                                "cpssDxChIfaGlobalConfigGet failed, dev: %d", dev);

        st = cpssOsMemCmp(&globalCfg,&globalCfgGet,sizeof(globalCfg)) == 0 ? GT_OK : GT_BAD_STATE;
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E) );

    /* 2. Go over all active devices. */
    while(GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        /* Global config Set */
        cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
        st = cpssDxChIfaGlobalConfigSet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_NOT_APPLICABLE_DEVICE, st,
                                "cpssDxChIfaGlobalConfigSet failed, dev: %d", dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    /* Global config Set */
    cpssOsMemSet(&globalCfg, 0, sizeof(globalCfg));
    st = cpssDxChIfaGlobalConfigSet(dev, &globalCfg);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_BAD_PARAM, st,
                            "cpssDxChIfaGlobalConfigSet failed, dev: %d", dev);
}

/*
GT_STATUS cpssDxChIfaGlobalConfigGet
(
    IN   GT_U8                    devNum,
    OUT  CPSS_DXCH_IFA_PARAMS_STC *ifaParamsPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIfaGlobalConfigGet)
{
    GT_STATUS                                        st;
    GT_U8                                            dev;
    CPSS_DXCH_IFA_PARAMS_STC                         globalCfg;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TAM_IFA_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&globalCfg,0,sizeof(globalCfg));

        /* check NULL pointer */
        st = cpssDxChIfaGlobalConfigGet(dev, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChIfaGlobalConfigGet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIfaGlobalConfigGet(dev, &globalCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIfaGlobalConfigGet(dev, &globalCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChIfaPortConfigSet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     portCfgArraySize,
    IN  CPSS_DXCH_IFA_PORT_CFG_STC portCfgArr[]
);
*/
UTF_TEST_CASE_MAC(cpssDxChIfaPortConfigSet)
{
    GT_STATUS                                        st;
    GT_U8                                            dev;
    CPSS_DXCH_IFA_PORT_CFG_STC                       portCfgArr;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TAM_IFA_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&portCfgArr,0,sizeof(portCfgArr));
        /* check NULL pointer */
        st = cpssDxChIfaPortConfigSet(dev, 1, NULL);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

        st = cpssDxChIfaPortConfigSet(dev, 1, &portCfgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
    }

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIfaPortConfigSet(dev, 1, &portCfgArr);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIfaPortConfigSet(dev, 1, &portCfgArr);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*
GT_STATUS cpssDxChIfaPortConfigGet
(
    IN   GT_U8                      devNum,
    IN   GT_PHYSICAL_PORT_NUM       portNum,
    OUT  CPSS_DXCH_IFA_PORT_CFG_STC *portCfgPtr
);
*/
UTF_TEST_CASE_MAC(cpssDxChIfaPortConfigGet)
{
    GT_STATUS                                        st;
    GT_U8                                            dev;
    CPSS_DXCH_IFA_PORT_CFG_STC                       portCfg;
    GT_PORT_NUM                                      port;

    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;
    PRV_TAM_IFA_SKIP_TEST_ON_BOARD_MAC;

    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* 1. Go over all active devices. */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_TRUE))
    {
        cpssOsMemSet(&portCfg,0,sizeof(portCfg));
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        /* For all active devices go over all available physical ports. */
        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_TRUE))
        {
            /* check NULL pointer */
            st = cpssDxChIfaPortConfigGet(dev, port, NULL);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PTR, st, dev);

            st = cpssDxChIfaPortConfigGet(dev, port, &portCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, st, dev);
        }

        /* For all active devices go over all non available physical ports. */
        st = prvUtfNextPhyPortReset(&port, dev);
        UTF_VERIFY_EQUAL0_PARAM_MAC(GT_OK, st);

        while(GT_OK == prvUtfNextPhyPortGet(&port, GT_FALSE))
        {
            /* Call function for each non-active port */
            st = cpssDxChIfaPortConfigGet(dev, port, &portCfg);
            UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
        }

        /* For active device check that function returns GT_BAD_PARAM
           for out of bound value for port number.*/
        port = UTF_CPSS_PP_MAX_PHYSICAL_PORTS_NUM_CNS(dev);

        st = cpssDxChIfaPortConfigGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);

        /* 1.4. For active device check that function returns GT_BAD_PARAM for CPU port number. */
        port = CPSS_CPU_PORT_NUM_CNS;

        st = cpssDxChIfaPortConfigGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
    }

    port = 0;
    /* prepare device iterator */
    PRV_TGF_APPLIC_DEV_RESET_MAC(&dev, (UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* go over all non active devices */
    while (GT_OK == prvUtfNextDeviceGet(&dev, GT_FALSE))
    {
        st = cpssDxChIfaPortConfigGet(dev, port, &portCfg);
        UTF_VERIFY_EQUAL1_PARAM_MAC(GT_NOT_APPLICABLE_DEVICE, st, dev);
    }

    /* 3. Call function with out of bound value for device id.*/
    dev = PRV_CPSS_MAX_PP_DEVICES_CNS;

    st = cpssDxChIfaPortConfigGet(dev, port, &portCfg);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_BAD_PARAM, st, dev);
}

/*----------------------------------------------------------------------------*/
/*
 * Configuration of cpssDxChTam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDxChTam)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIfaGlobalConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIfaGlobalConfigGet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIfaPortConfigSet)
    UTF_SUIT_DECLARE_TEST_MAC(cpssDxChIfaPortConfigGet)
UTF_SUIT_END_TESTS_MAC(cpssDxChTam)

