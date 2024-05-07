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
* @file tgfCommonStreamUT.c
*
* @brief Enhanced UTs for Stream
*
* @version   1
********************************************************************************
*/

#include <stream/prvTgfStreamSgcMaxSduSizeProfileCheck.h>
#include <stream/prvTgfStreamSgcExceptions.h>
#include <stream/prvTgfStreamSgcQosProfileUpdate.h>
#include <stream/prvTgfStreamSgcMultipleGates.h>
#include <stream/prvTgfStreamSgcGateTimeSlotsCheck.h>
#include <stream/prvTgfStreamSgcTimeBasedGateReConfig.h>
#include <stream/prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck.h>
#include <stream/prvTgfStreamQch.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <common/tgfCscdGen.h>


/*----------------------------------------------------------------------------------*/
/* AUTODOC: Stream Gate Control maximum SDU size profile check Ingress test
*
*  Test description:
*  - Configure entry 1 in Ingress max SDU size profile table to 64 bytes. Rest entries are cleared to '0'
*  - Set IPCL actions with Max SDU Size Profile index 1
*  - Configure Ingress max SDU size exception parameters (packet command & drop code) in case exception occurs
*  - Send 2 packets:
*     - first packet with size less than SDU size entry (64 bytes)
*     - second packet with size bigger than SDU size entry (94 bytes)
*  - Expect first packet to be valid and get to target port (since packet size is less than max SDU size that was set in test)
*    - check CNC counter (pcl.counterIndex << 1)| 0 (pass)
*  - Expect second packet to be invalid and to be dropped (since packet size exceeds max SDU size that was set in test)
*    - check CNC counter (pcl.counterIndex << 1)| 1 (fail)
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcMaxSduSizeProfileCheckIngress)
{
    /*
     * Stream Gate Control maximum SDU size profile check Ingress use case
     */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Set configuration */
    prvTgfStreamSgcMaxSduSizeProfileCheckConfigSet(CPSS_PCL_DIRECTION_INGRESS_E);

    /* Generate traffic */
    prvTgfStreamSgcMaxSduSizeProfileCheckTraffic();

    /* Packets output verification */
    prvTgfStreamSgcMaxSduSizeProfileCheckVerification();

    /* Restore configuration */
    prvTgfStreamSgcMaxSduSizeProfileCheckConfigRestore(CPSS_PCL_DIRECTION_INGRESS_E);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Stream Gate Control maximum SDU size profile check Egress use case test
*
*  Test description:
*  - Configure entry 2 in Egress max SDU size profile table to 64 bytes. Rest entries are cleared to '0'
*  - Set EPCL actions with Max SDU Size Profile indexe 2
*  - Configure Egress max SDU size exception parameters (packet command & drop code) in case exception occurs
*  - Send 2 packets:
*     - first packet with size less than SDU size entry (64 bytes)
*     - second packet with size bigger than SDU size entry (94 bytes)
*  - Expect first packet to be valid and get to target port (since packet size is less than max SDU size that was set in test)
*    - check CNC counter (pcl.counterIndex << 1)| 0 (pass)
*  - Expect second packet to be invalid and to be dropped (since packet size exceeds max SDU size that was set in test)
*    - check CNC counter (pcl.counterIndex << 1)| 1 (fail)
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcMaxSduSizeProfileCheckEgress)
{
    /*
     * Stream Gate Control maximum SDU size profile check Egress use case
     */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Set configuration */
    prvTgfStreamSgcMaxSduSizeProfileCheckConfigSet(CPSS_PCL_DIRECTION_EGRESS_E);

    /* Generate traffic */
    prvTgfStreamSgcMaxSduSizeProfileCheckTraffic();

    /* Packets output verification */
    prvTgfStreamSgcMaxSduSizeProfileCheckVerification();

    /* Restore configuration */
    prvTgfStreamSgcMaxSduSizeProfileCheckConfigRestore(CPSS_PCL_DIRECTION_EGRESS_E);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC gate interval max octet exceeded exception test
*
*  Test description:
*  - Assign gate id to specific flow
*  - Configure the gate
*  - Send first packet of this flow with size less than it is set in Interval Max table
*  - Send second packet of this flow. The accamulated number of bytes of the 2 packets
*    is now bigger than in Interval Max table
*  - Expect first packet to pass and second packet to be dropped due to exception of
*    Interval Max octet exceeded type
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcGateIntervalMaxOctetExceededException)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType;
    testType = STREAM_SGC_EXCEPTION_TESTS_GATE_INTERVAL_MAX_OCTET_EXCEEDED_E;

    /* Set configuration */
    prvTgfStreamSgcExceptionsConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcExceptionsTraffic(testType);

    /* Packets output verification */
    prvTgfStreamSgcExceptionsVerification(testType);

    /* Restore configuration */
    prvTgfStreamSgcExceptionsConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC gate closed exception test
*
*  Test description:
*  - Assign gate id to specific flow
*  - Configure the gate to be opened
*  - Send first packet of this flow
*  - Configure again the gate, this time close the gate
*  - Send second packet of this flow.
*  - Expect first packet to pass and second packet to be dropped due to exception of
*    Gate closed type
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcGateClosedException)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType = STREAM_SGC_EXCEPTION_TESTS_GATE_CLOSED_E;

    /* Set configuration */
    prvTgfStreamSgcExceptionsConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcExceptionsTraffic(testType);

    /* Packets output verification */
    prvTgfStreamSgcExceptionsVerification(testType);

    /* Restore configuration */
    prvTgfStreamSgcExceptionsConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC gate byte count adjust test
*
*  Test description:
*  - Assign gate id to specific flow
*  - Configure the gate
*  - Send first packet of this flow with size less than it is set in Interval Max table
*  - Configure the gate again this time set byte count adjust to reduce 40 bytes of Octet counter
*  - Send second packet of this flow.
*  - Expect first packet to pass. Expect the second packet to pass also since now the accamulated
*    number of bytes is 40 bytes less and therefore it is less than Interval Max
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcGateByteCountAdjust)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType = STREAM_SGC_EXCEPTION_TESTS_GATE_BYTE_COUNT_ADJUST_E;

    /* Set configurations */
    prvTgfStreamSgcExceptionsConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcExceptionsTraffic(testType);

    /* Packets output verifications */
    prvTgfStreamSgcExceptionsVerification(testType);

    /* Restore configurations */
    prvTgfStreamSgcExceptionsConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC gate QoS profile update test
*
*  Test description:
*  - Assign gate id to specific flow
*  - Configure the gate to be open and with IPV set to NULL value
*  - Send first packet with vlan of this flow
*  - Configure the gate again this time set valid IPV value
*  - Send second packet with vlan of this flow.
*  - Expect both packets to pass.
*    - For first packet expect Desc<QosProfile> to remain the same
*    - For second packet expect Desc<QosProfile> to be changed.
*      The 3 LS bits of Desc<QoS profile> should be overwritten by the IPV value
*      and therefore vlan<up> should now be set as the up that was configured for
*      the modified QoS profile
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcQosProfileUpdate)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configurations */
    prvTgfStreamSgcQosProfileUpdateConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcQosProfileUpdateTraffic();

    /* Packets output verifications */
    prvTgfStreamSgcQosProfileUpdateVerification();

    /* Restore configurations */
    prvTgfStreamSgcQosProfileUpdateConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC multiple gates test
*
*  Test description:
*  - Send 5 packets with different sizes
*  - Configure 4 different gates with different tableSets
*  - Assign each gate with different packet except for one gate which is assigned
*    to two packets (first and last)
*  - Configure all gates to be opened and with large interval max values
*  - Expect all packets to pass.
*  - Check counters and packets
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcMultipleGates)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configurations */
    prvTgfStreamSgcMultipleGatesConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcMultipleGatesTraffic();

    /* Packets output verifications */
    prvTgfStreamSgcMultipleGatesVerification();

    /* Restore configurations */
    prvTgfStreamSgcMultipleGatesConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC time based gate re-configuration functionality test
*
*  Test description:
*  - Configure gate directly with tableSet set to gate close
*  - Configure same gate but with different tableSet which set gate to open
*    Set this new configurations to occur in specific time
*  - Send 10 packets after re-configuration time
*  - Expect gate to hold new configuration
*  - Expect all 5 packets to pass since gate now should be opened
*  - Check counters and packets
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcTimeBasedGateReConfig)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configurations */
    prvTgfStreamSgcTimeBasedGateReConfigConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcTimeBasedGateReConfigTraffic();

    /* Packets output verifications */
    prvTgfStreamSgcTimeBasedGateReConfigVerification();

    /* Restore configurations */
    prvTgfStreamSgcTimeBasedGateReConfigConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: SGC gate time slots functionality test
*
*  Test description:
* - Configure only 2 time slots
*   - Set first time slot to open gate
*   - Set second time slot to close gate
*   - Set tableSet cycle time to 2 time slots * time slot duration
* - Send 4 packets with 1 time slot duration gap between each packet
*   this is done so each packet will be sent on different time slot
* - Check results: since packets are sent each time in different time slot
*   expect first packet to pass then next packet to drop and so on.
*   so if we send 4 packets expect at least 1 to pass and 1 to drop.
* - Check counters and packets
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcGateTimeSlotsCheck)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configurations */
    prvTgfStreamSgcGateTimeSlotsCheckConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcGateTimeSlotsCheckTraffic();

    /* Packets output verifications */
    prvTgfStreamSgcGateTimeSlotsCheckVerification();

    /* Restore configurations */
    prvTgfStreamSgcGateTimeSlotsCheckConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: Egress mirroring of Qci stream with time slot added to the mirrored copy
*
*  Test description:
*  - Configure 4 time slots with gate set to open
*  - Set IPCL rule for each packet
*    - assign the configured gate to all packets
*  - Set EPCL rule for each packet
*    - use EPCL UDB metadata to set rule for each time slot
*    - Set flow ID to time slot ID
*    - Mirror the packet to CPU
*  - Send 4 packets with 1 time slot duration gap between each packet
*    this is done so each packet will be sent on different time slot
*  - Check results
*    - Expect all 4 packets to pass that gate
*    - Expect each packet to have different time slot ID
*    - Check DSA<flowId> is equal = time slot per packet
*/
UTF_TEST_CASE_MAC(prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configurations */
    prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigSet();

    /* Generate traffic */
    prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckTraffic();

    /* Packets output verifications */
    prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckVerification();

    /* Restore configurations */
    prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigRestore();
#endif
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC: 802.1Qch Cyclic Queuing and Forwarding feature check
*
*  Test description:
*  - Configure Qci on the Ingress and Qbv on the Egress
*  - Set 2 time slots for both Qci and Qbv
*  - Set same time slot duration and same cycle time for both Qci and Qbv
*  - For Qci
*    - In first time slot open gate and direct packet to queue 7
*    - In second time slot open gate and direct packet to queue 6
*  - For Qbv
*    - In first time slot open gate for queue 6 and close all others
*    - In second time slot open gate for queue 7 and close all others
*  - Send 2 packets with 1 time slot duration gap between each packet
*    this is done so each packet will be sent on different time slot
*  - Check results
*    - Expect all packets to pass the gates and reach target port
*      - for first time slot, packets recevied in Queue 7 and transmit from queue 6
*      - for second time slot, packets recevied in Queue 6 and transmit from queue 7
*    - Check port,CNC and gate counters
*/
UTF_TEST_CASE_MAC(prvTgfStreamQch)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_30_CNS))
    PRV_TGF_TSN_SUPPORTED_MAC(prvTgfDevNum)

    /* Simulation does not support the test */
    ASIC_SIMULATION_NOT_SUPPORT_THIS_TEST_MAC;

#ifndef ASIC_SIMULATION
    /* Set configurations */
    prvTgfStreamQchConfigSet();

    /* Generate traffic */
    prvTgfStreamQchTraffic();

    /* Packets output verifications */
    prvTgfStreamQchVerification();

    /* Restore configurations */
    prvTgfStreamQchConfigRestore();
#endif
}



/*
 * Configuration of tgfStream suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfStream)

    /* Test SGC Ingress Max SDU Size Profile */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcMaxSduSizeProfileCheckIngress)

    /* Test SGC Egress Max SDU Size Profile */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcMaxSduSizeProfileCheckEgress)

    /* Test SGC gate interval max octet exception */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcGateIntervalMaxOctetExceededException)

    /* Test SGC gate closed exception */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcGateClosedException)

    /* Test SGC gate byte count adjust functionality */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcGateByteCountAdjust)

    /* Test SGC gate QoS profile update functionality */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcQosProfileUpdate)

    /* Test SGC multiple gates functionality */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcMultipleGates)

    /* Test SGC time based gate re-configuration functionality */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcTimeBasedGateReConfig)

    /* Test SGC gate time slots functionality */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcGateTimeSlotsCheck)

    /* Test egress mirroring of Qci stream with time slot added to the mirrored copy */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck)

    /* Test 802.1Qch Cyclic Queuing and Forwarding feature */
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfStreamQch)

UTF_SUIT_END_TESTS_MAC(tgfStream)

