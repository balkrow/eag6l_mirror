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
* @file tgfCommonIfaUT.c
*
* @brief Enhanced UTs for CPSS IFA
*
* @version   18
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>

#include <common/tgfCscdGen.h>
#include <tam/prvTgfIfaUseCase.h>

/* AUTODOC: Test IFA for Falcon device as Ingress Node.
   Configure Bridge, TTI to clone packet, EQ to map mirror ePort to LB port
   Configure EPCL to match mirror packet and assign IFA_Header PHA thread
   First Pass assign Header PHA thread and adds 28B IFA Header to packet
   PHA also inserts Outer VLAN tag used in second pass to assign source port
   In second Pass My Physical Ports table maps outer VLAN tag to local device source port
   For second pass configure TTI to only match packet post first pass
   Match second pass packet in EPCL to assign IFA_Metadata PHA Thread
   Verify:
   EPCL rule match in first pass, PHA triggered so validate IFA Header and Outer VLAN tag assigment
   EPCL rule mtach in second pass, PHA triggered so validate IFA Header and Metadata update in final egress packet
 */
UTF_TEST_CASE_MAC (prvTgfIfaIngressNode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /*config set*/
    prvTgfIngressNodeIfaConfigurationSet();

    /*Test with traffic*/
    prvTgfIngressNodeIfaTrafficSend();

    /*config restore */
    prvTgfIngressNodeIfaConfigurationRestore();
}

/* AUTODOC: Test IFA for Falcon device as Transit Node.
   Configure Bridge
   Configure EPCL rule to match and assign IFA_Transit_Metadata PHA thread
   Verify:
   EPCL rule mtach, PHA triggered so validate IFA Header and Metadata update in final egress packet
 */
UTF_TEST_CASE_MAC (prvTgfIfaTransitNode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /*config set*/
    prvTgfTransitNodeIfaConfigurationSet();

    /*Test with traffic*/
    prvTgfTransitNodeIfaTrafficSend();

    /*config restore */
    prvTgfTransitNodeIfaConfigurationRestore();
}

/* AUTODOC: Test IFA for Falcon device as Egress Node.
   Pass#1 Thread 1: EPCL rule match incoming packet and assign PHA thread EGRESS_NODE_MIRROR and packet command as SOFT_DROP
   EREP replicates descriptor to EQ
   Pass#1 Thread 2: EQ - Analyzer-ID table assigns to IFA egress mirrored packets analyzer port which is LB port
   and Treat Egress Mirrored packets as Ingress Mirrored
   EPCL key match the incoming mirror traffic and trigger PHA Thread 2 EGRESS_NODE_METADATA
   Pass#2: TTI match the loopback traffic and assign ePort for collector
   HA: target ePort attributes assign  IPv4-UDP TS
   Verify:
   EPCL rule mtach, PHA triggered so validate Tunnel encapsulation, IFA Header and Metadata update in final egress packet
 */
UTF_TEST_CASE_MAC (prvTgfIfaEgressNode)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, ~UTF_FALCON_E);

    /* Test uses PHA feature but GM does not support PHA, therefore skip test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /*config set*/
    prvTgfEgressNodeIfaConfigurationSet();

    /*Test with traffic*/
    prvTgfEgressNodeIfaTrafficSend();

    /*config restore */
    prvTgfEgressNodeIfaConfigurationRestore();
}

/*
 * Configuration of tgfTam suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfTam)

      /* Test IFA on Ingress Node */
      UTF_SUIT_DECLARE_TEST_MAC(prvTgfIfaIngressNode)

      /* Test IFA on Transit Node */
      UTF_SUIT_DECLARE_TEST_MAC(prvTgfIfaTransitNode)

      /* Test IFA on Egress Node */
      UTF_SUIT_DECLARE_TEST_MAC(prvTgfIfaEgressNode)

UTF_SUIT_END_TESTS_MAC(tgfTam)

