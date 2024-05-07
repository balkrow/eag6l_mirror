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
* @file tgfCommonConfigInitUT.c
*
* @brief Enhanced UTs for Config Init.
*
* @version   2
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <config/prvTgfConfigInit.h>
#include <config/prvTgfCfgIngressDropEnable.h>
#include <config/prvTgfCfgReservedFlows.h>
#include <ip/prvTgfBasicIpv4UcRouting.h>

/* Router SA Modification test ports */
#define PRV_TGF_CFG_PHA_SA_MODIFICATION_RECV_PORT_IDX_CNS        1
#define PRV_TGF_CFG_PHA_SA_MODIFICATION_NEXT_HOP_PORT_IDX_CNS    3

#define PRV_TGF_VRF_ID_CNS    0

/*----------------------------------------------------------------------------*/
/* AUTODOC: Test ingress drop counters:
    configure VLAN, FDB entries;
    set different ingress drop counter modes;
    send Ethernet traffic for each mode;
    verify drop counters are correct.
*/
UTF_TEST_CASE_MAC(prvTgfCfgIngressDropCounter)
{
/********************************************************************
    Test 1 - Ingress drop counter.

    1. Set configuration
    2. Generate traffic
    3. Restore configuration
********************************************************************/
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_PUMA_E);

    /* Set configuration */
    prvTgfCfgIngressDropCounterSet();

    /* Generate traffic */
    prvTgfCfgIngressDropCounterTrafficGenerate();

    /* Restore configuration */
    prvTgfCfgIngressDropCounterRestore();
}

UTF_TEST_CASE_MAC(prvTgfCfgProbePacketDropCode)
{
    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    prvTgfCfgProbePacketBasicConfigSet();
    prvTgfCfgProbePacketPclConfigSet();
    prvTgfCfgProbePacketTrafficGenerate();
    prvTgfCfgProbePacketSourceIdUpdate();
    prvTgfCfgProbePacketTrafficGenerate();
    prvTgfCfgProbePacketConfigRestore();
}

UTF_TEST_CASE_MAC(prvTgfCfgProbePacketEgressDropCodeTest)
{

    PRV_TGF_SUPPORTED_DEV_DECLARE_MAC( UTF_FALCON_E )

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    prvTgfCfgProbePacketBasicConfigSet();
    prvTgfCfgProbePacketEgressPclConfigSet();
    prvTgfCfgProbePacketTrafficGenerate();
    prvTgfCfgProbePacketConfigRestore();
}

/* AUTODOC: Test - prvTgfCfgIngressDropEnable:
   To test drop in ingress processing pipe by enabling/disabling "drop in EQ enable"
   Check the packet dropped by TTI/PCL is counted in bridge
*/
UTF_TEST_CASE_MAC(prvTgfCfgIngressDropEnable)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_10_CNS))

    prvTgfCfgIngressDropEnableTest();
}

/* AUTODOC: Test PHA thread77 Reserved Flows.
   1) Configure UC bridging.
   2) Configure egress ePort to add Tunnel Start.
   3) Configure Tunnel Start entries with data needed
   4) Configure EPCL for assignment data flowId, copyReserved
   5) check packet's tunnel header for tunnel header ethertype and MAC DA.
 */
UTF_TEST_CASE_MAC (prvTgfCfgReservedFlow_1_4)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /*config set*/
    prvTgfCfgReservedFlowsConfigurationSet(PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS);

    /*Test with traffic*/
    prvTgfCfgReservedFlowsTrafficSend(PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS);

    /*config restore */
    prvTgfCfgReservedFlowsConfigurationRestore(PRV_TGF_PHA_THREAD_ID_RESERVED_1_0_CNS);
}

/* AUTODOC: Test PHA thread78 Reserved Flows.
   1) Configure UC bridging.
   2) Configure egress ePort to add Tunnel Start.
   3) Configure Tunnel Start entries with data needed
   4) check packet's tunnel header for tunnel header ethertype and MAC DA.
 */
UTF_TEST_CASE_MAC (prvTgfCfgReservedFlow_5_8)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /*config set*/
    prvTgfCfgReservedFlowsConfigurationSet(PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS);

    /*Test with traffic*/
    prvTgfCfgReservedFlowsTrafficSend(PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS);

    /*config restore */
    prvTgfCfgReservedFlowsConfigurationRestore(PRV_TGF_PHA_THREAD_ID_RESERVED_1_1_CNS);
}

/* AUTODOC: Test PHA thread7y Reserved Flows.
   1) Configure UC bridging.
   2) Configure egress ePort to add Tunnel Start.
   3) Configure Tunnel Start entries with data needed
   4) check packet's tunnel header for tunnel header ethertype, MAC DA, PType, hopLimit and forwarding table.
 */
UTF_TEST_CASE_MAC (prvTgfCfgReservedFlow_6_2_3_7)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /*config set*/
    prvTgfCfgReservedFlowsConfigurationSet(PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS);

    /*Test with traffic*/
    prvTgfCfgReservedFlowsTrafficSend(PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS);

    /*config restore */
    prvTgfCfgReservedFlowsConfigurationRestore(PRV_TGF_PHA_THREAD_ID_RESERVED_1_2_CNS);
}

/* AUTODOC: Test Router SA Modification:
    1) Set Base configuration
    2) Set Route configuration for vrfId[0]
    3) Set Router SA Modification
    4) Config EPCL Generate traffic and Validate outer L2 MAC SA
    5) Restore configuration
*/
UTF_TEST_CASE_MAC(prvTgfCfgRouterSaModification)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* Set Base configuration */
    prvTgfBasicIpv4UcRoutingBaseConfigurationSet(prvTgfPortsArray[PRV_TGF_CFG_PHA_SA_MODIFICATION_RECV_PORT_IDX_CNS]);

    /* Set Route configuration for vrfId[0] */
    prvTgfBasicIpv4UcRoutingRouteConfigurationSet(PRV_TGF_VRF_ID_CNS, 0,
                                                  prvTgfPortsArray[PRV_TGF_CFG_PHA_SA_MODIFICATION_NEXT_HOP_PORT_IDX_CNS], GT_FALSE,
                                                  GT_FALSE, GT_FALSE, GT_FALSE);

    /* Set Router SA Modification configuration */
    prvTgfCfgRouterSaModificationConfigurationSet();

    /* Generate traffic and config EPCL */
    prvTgfCfgRouterSaModificationTrafficGenerate();

    /* Restore configuration */
    prvTgfBasicIpv4UcRoutingConfigurationRestore(PRV_TGF_VRF_ID_CNS, 0, PRV_TGF_DEFAULT_U8_CNS, GT_FALSE);

    /* Restore Router SA Modification configuration */
    prvTgfCfgRouterSaModificationConfigurationRestore();
}

/* AUTODOC: Test INQA ECN:
    1) Set Bridge configuration
    3) Set PHA config for thread INQA ECN
    4) Config EPCL Generate traffic and Validate outer L2 MAC DA, ECN and reserved flag fields
    5) Restore configuration
*/
UTF_TEST_CASE_MAC(prvTgfCfgInqaEcn)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_FALCON_E));

    /* Set INQA ECN configuration */
    prvTgfCfgInqaEcnConfigurationSet();

    /* Generate traffic and config EPCL */
    prvTgfCfgInqaEcnTrafficGenerate();

    /* Restore INQA ECN configuration */
    prvTgfCfgInqaEcnConfigurationRestore();
}

/* AUTODOC: Test VXLAN DCI VNI REMAP:
    1) Set Bridge configuration
    3) Set PHA config for thread VXLAN DCI VNI REMAP
    4) Config EPCL Generate traffic and Validate outer L2 MAC DA, egress packets VNI
    5) Restore configuration
*/
UTF_TEST_CASE_MAC(prvTgfCfgVxlanDciVniRemap)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_FALCON_E));

    /* Set VXLAN DCI VNI REMAP configuration */
    prvTgfCfgVxlanDciVniRemapConfigurationSet();

    /* Generate traffic and config EPCL */
    prvTgfCfgVxlanDciVniRemapTrafficGenerate();

    /* Restore VXLAN DCI VNI REMAP configuration */
    prvTgfCfgVxlanDciVniRemapConfigurationRestore();
}

/* AUTODOC: Test PHA thread93 MPLS LSR In Stacking System.
    1) Set Bridge configuration
    2) Set PHA config for thread MPLS LSR In Stacking System
    3) Generate eDSA tagged traffic.
    4) Verify packet's TrgEport = desc<tgrPhyPort> and TrgDev = TrgDev ^ template<tagetDevSelectBit>
       when TrgDev != template<ownHwDevNum>
    5) Restore configuration
 */
UTF_TEST_CASE_MAC (prvTgfCfgMplsLsrInStackingSystem)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~(UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E));

    /* Set MPLS LSR In Stacking System configuration */
    prvTgfCfgMplsLsrInStackingSystemConfigSet();

    /* Generate traffic and config PHA */
    prvTgfCfgMplsLsrInStackingSystemTrafficGenerate();

    /* Restore MPLS LSR In Stacking System configuration */
    prvTgfCfgMplsLsrInStackingSystemConfigRestore();
}


/*
 * Configuration of tgfConfig suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfConfig)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgIngressDropCounter)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgProbePacketDropCode)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgProbePacketEgressDropCodeTest)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgIngressDropEnable)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgReservedFlow_1_4)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgReservedFlow_5_8)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgReservedFlow_6_2_3_7)

    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgRouterSaModification)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgInqaEcn)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgVxlanDciVniRemap)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfCfgMplsLsrInStackingSystem)
UTF_SUIT_END_TESTS_MAC(tgfConfig)


