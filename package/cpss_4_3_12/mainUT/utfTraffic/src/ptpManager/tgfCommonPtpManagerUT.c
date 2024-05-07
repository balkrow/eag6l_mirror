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
* @file tgfCommonPtpManagerUT.c
*
* @brief Enhanced UTs for CPSS PTP Manager
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
#include <common/tgfPtpGen.h>
#include <common/tgfPtpManagerGen.h>
#include <ptp/prvTgfPtpGen.h>
#include <ptpManager/prvTgfPtpManagerGen.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>

#define EGRESS_PORT_INDEX_PIPE_0 0
#define EGRESS_PORT_INDEX_PIPE_1 3

/* AUTODOC:
   Test Insert transmission time into the packet in the packet payload.                                                                                                 .                                                                                       					 .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}.
       - Send PTP packet.
       - Expect capture packet to have: timestamp tag in packet payload.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerAddTimeAfterPtpHeaderTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManagerGenAddTimeAfterPtpHeaderCfgSet(PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpGenAddTimeAfterPtpHeaderTrafficGenerateAndCheck();

    /* restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_1);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/* AUTODOC:
   Test Store packet's Ingress timestamp (time of reception) in the Ingress Timestamp Queues.
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}.
       - Send PTP packet.                                .
       - Expect Valid entry in Ingress Timestamp Global Queue with appropriate sequenceId.                                                                                           .
*/
UTF_TEST_CASE_MAC(tgfPtpManagerIngressCaptureTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* egress port index for PIPE1 */
    prvTgfPtpManagerGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpGenTrafficGenerate();

    /* check Ingress Timestamp Global Queue Entry */
    prvTgfPtpManagerGenIngressCaptureQueue1Check(GT_TRUE);

    /* restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_1);
    prvTgfPtpGenIngressCaptureRestore(EGRESS_PORT_INDEX_PIPE_1);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   test checks that timstemps entrys Ingress queues are reading in in both Pipe0 (Egress port index 0) and Pipe1 (Egress port index 3)
   without starvation by sendind packets to both pipes and expected to read each time from another pipe (RR).
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}.
       - Send PTP packet.                                .
       - Expect 4 Valid entries in Ingress Timestamp Global Queue with appropriate sequenceId.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerIngressTimestampQueueReading)
{

  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

  /* general bridge configuration */
  prvTgfPtpGenVidAndFdbDefCfgSet();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);

  /* global PTP-related settings */
  prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

  /* egress port index for PIPE1 */
  prvTgfPtpManagerGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpManagerGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE1 */
  prvTgfPtpManagerGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpManagerGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();


  /* AUTODOC: read the packets from ingress , queue 1, one pipe at a time */
  prvTgfPtpManagerGenIngressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpManagerGenIngressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpManagerGenIngressCaptureQueue1Check(GT_TRUE);
  prvTgfPtpManagerGenIngressCaptureQueue1Check(GT_TRUE);
  /* NOTE : the Golden model not supporting dequeueing */
  if (GT_FALSE == prvUtfIsGmCompilation())
  {
      prvTgfPtpManagerGenIngressCaptureQueue1Check(GT_FALSE);
  }

  /* restore PTP configuration */
  prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_1);

  /* restore bridge configuration */
  prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   test checks that timstemps entrys Egress queues are reading in in both Pipe0 (Egress port index 0) and Pipe1 (Egress port index 3)
   without starvation by sendind packets to both pipes and expected to read each time from another pipe (RR).
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}.
       - Send PTP packet.                                .
       - Expect 4 Valid entries in Ingress Timestamp Global Queue with appropriate sequenceId.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerEgressTimestampQueueReading)
{

  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

  /* global PTP-related settings */
  prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

  /* re-arrange prvTgfPortsArray to ensure ports are not remote ports */
  prvTgfPtpGenEgressTimestempPortsConfig(EGRESS_PORT_INDEX_PIPE_0,EGRESS_PORT_INDEX_PIPE_1);

  /* general bridge configuration */
  prvTgfPtpGenVidAndFdbDefCfgSet();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);

  /* egress port index for PIPE1 */
  prvTgfPtpManagerGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpManagerGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE1 */
  prvTgfPtpManagerGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* egress port index for PIPE0 */
  prvTgfPtpManagerGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  /* Generate traffic: send to device's port given packet */
  prvTgfPtpGenTrafficGenerate();

  /* AUTODOC: read the packets from egress , queue 1, one pipe at a time */
  prvTgfPtpManagerGenEgressCaptureQueue1Check(GT_TRUE,EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpManagerGenEgressCaptureQueue1Check(GT_TRUE,EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpManagerGenEgressCaptureQueue1Check(GT_TRUE,EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpManagerGenEgressCaptureQueue1Check(GT_TRUE,EGRESS_PORT_INDEX_PIPE_0);
  /* NOTE : the Golden model not supporting dequeueing */
  if (GT_FALSE == prvUtfIsGmCompilation())
  {
      prvTgfPtpManagerGenEgressCaptureQueue1Check(GT_FALSE,EGRESS_PORT_INDEX_PIPE_1);
      prvTgfPtpManagerGenEgressCaptureQueue1Check(GT_FALSE,EGRESS_PORT_INDEX_PIPE_0);
  }
  /* restore PTP configuration */
  prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_1);

  /* restore bridge configuration */
  prvTgfPtpGenVidAndFdbDefCfgRestoreEgress();

}

#if 0
UTF_TEST_CASE_MAC(tgfPtpEgressTimestampQueueinterrupt)
{
    /* AUTODOC: test checks support on cpss unify event for egress ptp timestemps interrupts:
       new timestamp and full queue in per-port and ermrk queues.
       NOTE : full ermrk queue interrupt disables per port queue. */

  PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_XCAT3_E | UTF_AC5_E | UTF_LION2_E);

    CPSS_TBD_BOOKMARK_FALCON    /* Skip test until PTP Event support is implemented at Falcon */
    CPSS_TBD_BOOKMARK_AC5P      /* Skip test until PTP Event support is implemented at Hawk */
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, UTF_FALCON_E | UTF_AC5P_E | UTF_AC5X_E | UTF_HARRIER_E | UTF_IRONMAN_L_E);

  /* GM does not support the test */
  GM_NOT_SUPPORT_THIS_TEST_MAC

  prvTgfPtpGenEgressTimestempQueueInteruptsConfig(EGRESS_PORT_INDEX_PIPE_0,EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenVidAndFdbDefCfgSet();

  /* egress port index for PIPE1 */
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  /* check for interrupt*/
  prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,2,GT_TRUE);

  /* for BC3: egress port index for PIPE0 ,PIPE1 ,PIPE0 ,PIPE1 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();

  prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,8,GT_TRUE);

  /* for BC3: egress port index for PIPE0 ,PIPE1 ,PIPE0 ,PIPE1 */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
 /* ptp queue port index 3 is full  (ptp queue size is 4)
     ermrk ptp queue get the new timestemp   */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
 /* ptp queue port index 0 is full (ptp queue size is 4)
      ermrk ptp queue get the new timestemp  */
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();
  /* ermrk ptp port queue and egress ptp queue are full*/
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);
  prvTgfPtpGenTrafficGenerate();
  /* ermrk ptp port queue and egress ptp queue are full*/
  prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenEgressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_0);
  prvTgfPtpGenTrafficGenerate();

  prvTgfPtpGenGetEvent(CPSS_PP_GTS_GLOBAL_FIFO_FULL_E, 4, GT_TRUE);
  prvTgfPtpGenGetEvent(CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,8,GT_TRUE);

  prvTgfPtpGenEgressTimestempQueueInteruptsRestore();

}
#endif
/* AUTODOC:
   Test for Timestamping of Mirrored To CPU packets.
       - Configure VLAN and FDB Entries.
       - Configure OAM engine based timestamping:
          - Configure IPCL entry to trigger OAM
          - Configure OAM entry with timestamp enable and OAM/PTP Offset
          - Configure Timestamp Configuration entry with AddTime action on offset 0
          - Configure Timestamp tag to be Extended on CPU port
          - Configure Timestamp global parameters
       - Send packet that matches IPCL entry.
       - Verify if packet Send to CPU and verify TS EtherType.
         Verify that new TS extended is after L2 header.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerTimestampTagOnMirroredToCPU)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        UTF_SIP6_GM_NOT_READY_SKIP_MAC("\nTest is skipped since OAM is not supported yet in GM\n");
    }

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpDefaultInit();

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();
    prvTgfPtpGenFdbDefCfgSet(EGRESS_PORT_INDEX_PIPE_1);

    /* Configure OAM engine based timestamping */
    prvTgfPtpManagerGenAddTimeStampTagAndMirrorSet();

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpGenTrafficAndCheckCpuGetTimeStampTag();

    /* Restore configuration */
    prvTgfPtpManagerGenAddTimeStampTagAndMirrorrRestore();
}

/* AUTODOC:
   Test for Ingress and Egress Timestamp action. The action is used for OAM based timestamping.
       - Configure VLAN and FDB Entries.
       - Configure OAM engine based timestamping:
          - Configure IPCL entry to trigger OAM
          - Configure OAM entry with timestamp enable and OAM/PTP Offset
          - Configure Timestamp Configuration entry with AddIngressEgressTime action on offset=0 and offset2=16
          - Configure Timestamp tag to be Extended on egress port
          - Configure Timestamp global parameters
       - Send packet that matches IPCL entry.
       - Verify if packet captured on egress port and verify TS EtherType.
         Verify that new TS extended is after L2 header.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerAddIngressEgressTest)
{
    GT_STATUS  status = GT_OK;
    GT_U32 counter = 0;

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        UTF_SIP6_GM_NOT_READY_SKIP_MAC("\nTest is skipped since OAM is not supported yet in GM\n");

    }

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpDefaultInit();

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* Configure OAM engine based timestamping */
    prvTgfPtpManagerGenIngressEgressCaptureCfgSet();

    /* try max 3 times to run traffic; possiable WA in ingress time & egress time */
    while (counter < 3)
    {
        status = GT_OK;
        /* Generate traffic: send to device's port given packet */
        prvTgfPtpGenAddIngressEgressTrafficGenerateAndCheck(counter,&status);
        if (status == GT_OK)
            break;
        counter++;
    }

    /* Restore configuration */
    prvTgfPtpManagerGenIngressEgressCaptureRestore();
}

/* AUTODOC:
   Test for PTP Egress Exception Configuration
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet.
       - COnfigure PTP Egress Execption
             In Case of "Invalid Incoming Piggyback Interrupt"
             Set Pkt Cmd = TRAP_TO_CPU,
                CPU CODE = CPSS_NET_PTP_HEADER_ERROR_E.
       - Send PTP packet with invalid timestamp format.
       - Verify if packet Send to CPU and verify CPU Code.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerEgressExceptionTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* PTP egress exception configuration */
    prvTgfPtpManagerEgresssExceptionCfgSet();

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpEgressExceptionTrafficGenerateAndCheck();

    /* Restore configuration */
    prvTgfPtpManagerEgressExceptionTestRestore();
}

/* AUTODOC:
   Test for Invalid Ptp packet Command and CPU code config
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet Over UDP IPv4.
             --Enable PTP over UDP IPv4.
             --Set PTP UDP Destination port.
       - COnfigure PTP Egress Execption config with
             In Case of "Invalid PTP Interrupt"
             Set Pkt Cmd = TRAP_TO_CPU,
                CPU CODE = CPSS_NET_PTP_HEADER_ERROR_E.
       - Send PTP packet Over UDP IPv4 with invalid PTP header (only first 4 bytes).
            - "Invalid PTP interrupt" is generated and packet command and cpu code is set as per config.
       - Verify if packet Send to CPU and verify CPU Code.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerInvalidPtpTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E);

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* PTP egress exception configuration */
    prvTgfPtpManagerInvalidPtpCfgSet();

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpInvalidPtpTrafficGenerateAndCheck();

    /* Restore configuration */
    prvTgfPtpManagerInvalidPtpCfgRestore();
}

/* AUTODOC:
   Test for Capture Previous Mode check.
       -  Set the TOD to N sec by "cpssDxChPtpTaiTodSet"
       -  Enable Previous Mode Capture by "cpssDxChPtpTsuControlSet"
       -  Set the TOD to N+1000 sec by "cpssDxChPtpTaiTodSet"
       -  Read & verify the capture value  should be N+1 by cpssDxChPtpTsIngressTimestampQueueEntryRead
*/
UTF_TEST_CASE_MAC(tgfPtpManagerCapturePreviousModeTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* GM does not support the test */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpDefaultInit();

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* Enable capture previous mode; Set the TOD+1000 sec; Verify the capture value.*/
    prvTgfPtpManagerCapturePreviousModeCfgAndVerifyCheck();

    /* Restore configuration */
    prvTgfPtpManagerCapturePreviousModeCfgRestore();
}

#if 0
UTF_TEST_CASE_MAC(tgfPtpTimeStampUDBTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(
        prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS));

    prvTgfPtpGenVidAndFdbDefCfgSet();

    prvTgfPtpGenTimeStampUDBCfgSet();

    prvTgfPtpGenTimeStampTrafficGenerateAndCheck();

    prvTgfPtpGenTimeStampUDBRestore();
}
#endif

/* AUTODOC:
   Test for PTP PiggyBack Configuration:                                                                         .
   In this mode the timestamp is piggybacked onto the 1588v2 packet, using the 4B <reserved> field in the PTP header.                                            .
   In this mode no tag is actually added to the packet.                                                                                                                                                              .                                          																	 .
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet.
       - Configure on egress port piggyback enable.
       - Configure Timestamp Configuration entry on egress port with CaptureIngress action.
       - Send PTP packet.
       - Expect capture packet to have: timestamp tag in <resreved> field == timestamp tag in ingress queue entry.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerPiggyBackTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* Test use timestamp unit(TSU/PSU). There is no TSU/PUS in GM */
    GM_NOT_SUPPORT_THIS_TEST_MAC

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* egress port index for PIPE1 */
    prvTgfPtpManagerGenIngressCaptureCfgSet(EGRESS_PORT_INDEX_PIPE_1);

    /* PTP piggyBack configuration */
    prvTgfPtpManagerGenTsTagPortCfgPiggyBackSet(EGRESS_PORT_INDEX_PIPE_1,1);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerGenPiggyBackTimestampTrafficGenerateAndCheck(1);

    /* restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(EGRESS_PORT_INDEX_PIPE_1);

    /* restore PTP piggyBack configuration */
    prvTgfPtpManagerGenTsTagPortCfgPiggyBackRestore(EGRESS_PORT_INDEX_PIPE_1);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/* AUTODOC:
   Test 1-step device in a 1-step network. Updating followUp message TLV:
   802.1AS-2020 calculates the rateRatio, the ratio of the GM frequency from the free running clk (LocalClock) of each device.
   Each device passes this value to its peer. This is used for calculating the accumulated ratio of the peer LocalClock frequency from the GM.
   Follow-up information TLV shall reside on the Sync message itself and the HW needs to be able to adjust this field on the fly.

   The TLV is located in offset 44B, relative to the desc<ptp_offset>.                                                 .
   In this mode no tag is actually added to the packet.                                                                                                                                                              .                                          																	 .
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet.
       - Configure PTP Cumulative Scaled Rate Offset enable + value per message Type SYNC.
       - Configure Timestamp Configuration entry on egress port with addCorrectedTime action.
       - Send PTP packet.
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have CumulativeScaledRateOffset field updated in TLV header.
*/

UTF_TEST_CASE_MAC(tgfPtpManager1StepFollowUpInformationTlvTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_IRONMAN_L_E))

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManager1StepFollowUpInformationTlvCfgSet();

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManager1StepFollowUpInformationTlvTrafficGenerateAndCheck();

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

#define PART_1  1
#define PART_2  2

/* AUTODOC:
   Test 1-step mechanism on tansperent clock End-to-End.                                                                                                 .                                                                                                      	 .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
            {SLAVE port,domain index,DELAY_REQ}     => FORWARD
            {MASTER port,domain index,DELAY_RESP}   => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => ADD_CORRECTED_TIME
            {SLAVE port,domain index,DELAY_REQ}     => ADD_CORRECTED_TIME
            {MASTER port,domain index,DELAY_RESP}   => NONE
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have correctionField updated in PTP header.
       - Send PTP DELAY_REQ packet from SLAVE port to MASTER port: expact to have correctionField updated in PTP header.
       - Send PTP DELAY_RESP packet from MASTER port to SLAVE port: expact no change in packet.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerTC1StepE2ETest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration part 1 */
    prvTgfPtpManagerTC1StepE2ECfgSet(PART_1);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E);

    /* test spesific PTP configuration part 2 */
    prvTgfPtpManagerTC1StepE2ECfgSet(PART_2);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

#define MASTER_2_SLAVE  1
#define SLAVE_2_MASTER  2

/* AUTODOC:
   Test 1-step mechanism on tansperent clock Peer-to-Peer.                                                                                                 .                                                                                                      	 .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
            {MASTER port,domain index,PDELAY_REQ}   => FORWARD
            {SLAVE port,domain index,DELAY_REQ}     => FORWARD
            {MASTER port,domain index,DELAY_RESP}   => FORWARD
            {SLAVE port,domain index,DELAY_RESP}    => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => ADD_CORRECTED_TIME
            {MASTER port,domain index,PDELAY_REQ}   => ADD_TIME
            {SLAVE port,domain index,PDELAY_REQ}    => ADD_TIME
            {MASTER port,domain index,PDELAY_RESP}  => ADD_TIME
            {SLAVE port,domain index,PDELAY_RESP}   => ADD_TIME
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have correctionField updated in PTP header.
       - Send PTP PDELAY_REQ packet from MASTER port to SLAVE port: expact to have originTimestamp updated in PTP header.
       - Send PTP DELAY_REQ packet from SLAVE port to MASTER port: expact to have originTimestamp updated in PTP header.
       - Send PTP PDELAY_RESP packet from MASTER port to SLAVE port: expact to have originTimestamp updated in PTP header.
       - Send PTP DELAY_RESP packet from SLAVE port to MASTER port: expact to have originTimestamp updated in PTP header.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerTC1StepP2PTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManagerTC1StepP2PCfgSet();

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,MASTER_2_SLAVE);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,MASTER_2_SLAVE);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,SLAVE_2_MASTER);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,SLAVE_2_MASTER);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,MASTER_2_SLAVE);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test 2-step mechanism on boundary clock End-to-End.                                                                                                 .                                                                                                         .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
            {MASTER port,domain index,FOLLOW_UP}    => FORWARD
            {SLAVE port,domain index,DELAY_REQ}     => FORWARD
            {MASTER port,domain index,DELAY_RESP}   => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => CAPTURE
            {MASTER port,domain index,FOLLOW_UP}    => NONE
            {SLAVE port,domain index,DELAY_REQ}     => CAPTURE_ADD_TIME
            {MASTER port,domain index,DELAY_RESP}   => NONE
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact the time of transmission to be captured in timestamp queue.
       - Send PTP FOLLOW_UP packet from MASTER port to SLAVE port: expact no change in packet.
       - Send PTP DELAY_REQ packet from SLAVE port to MASTER port: expact the time of transmission to be captured in timestamp queue
                                                                   and inserted into the packet <originTimestamp> field.
       - Send PTP DELAY_RESP packet from MASTER port to SLAVE port: expact no change in packet.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerBC2StepE2ETest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration part 1 */
    prvTgfPtpManagerBC2StepE2ECfgSet(PART_1);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC2StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC2StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC2StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E);

    /* test spesific PTP configuration part 2 */
    prvTgfPtpManagerBC2StepE2ECfgSet(PART_2);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC2StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test 1-step mechanism on boundary clock End-to-End.                                                                                                 .                                                                                                         .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
            {SLAVE port,domain index,DELAY_REQ}     => FORWARD
            {MASTER port,domain index,DELAY_RESP}   => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => ADD_TIME
            {SLAVE port,domain index,DELAY_REQ}     => CAPTURE_ADD_TIME
            {MASTER port,domain index,DELAY_RESP}   => NONE
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have originTimestamp updated in PTP header.
       - Send PTP DELAY_REQ packet from SLAVE port to MASTER port: expact the time of transmission to be captured in timestamp queue
                                                                   and inserted into the packet <originTimestamp> field.
       - Send PTP DELAY_RESP packet from MASTER port to SLAVE port: expact no change in packet.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerBC1StepE2ETest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration part 1 */
    prvTgfPtpManagerBC1StepE2ECfgSet(PART_1);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC1StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC1StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E);

    /* test spesific PTP configuration part 2 */
    prvTgfPtpManagerBC1StepE2ECfgSet(PART_2);

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerBC1StepE2ECfgSetTrafficGenerateAndCheck(CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test USXGMII MCH - Egress 1-step MCH in the Switch + PHY 1-step.

    1. The CPU sends a PTP event message. the frame is forwarded to a target port that is configured (per message type) with a PTP_ACTION = ADD_TIME.
       As such the CP doesn't timestamp the packet and points the desc<PTP Timestamp offset> to the originTimestamp/requestRecieptTimestamp field location.
    2. The CTSU adds the egress time per the PTP action according to the TX_PCH<TaiSel[1:0]> and subtracts the current time in nanosec of TAI4 from it.
       The checksum is then updated (if needed). The CTSU passes the PTP_PHY_TAG to the EPI in ctsu2epi usx bus<Signature>.
       The CTSU indicates to the EPI that a PCH should be added (ctsu2epi_usx_bus<pkt_tx_ts_pch_mac_tmps_en>=1)
    3. The EPI adds a PCH and set its Signature field according to the ctsu2epi bus (CFG<USX PCH Signature mode>=Q_ENTRY_ID).
    4. The PHY updates the CF by adding the egress time (in TAI4) and updating the checksum (as needed for IPv6, in the trailer)
                                                                                                         .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => ADD_TIME
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have (CSTU) originTimestamp and (PHY) correctionField updated in PTP header.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_IRONMAN_L_E))

    /* this test should be run only on simulation when the port is connected to PHY */
    #ifndef ASIC_SIMULATION

        SKIP_TEST_MAC

    #endif

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyCfgSet();

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTrafficGenerateAndCheck();

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test USXGMII MCH - Egress 2-step MCH in the Switch + PHY 1-step.

    1. The CPU sends a PTP event message. the frame is forwarded to a target port that is configured (per message type) with a PTP_ACTION = CAPTURE.
       As such the CP doesn't timestamp the packet, but stores the packet's info in the ERMRK Egress TSQ. The CP passes the index to the
       queue entry (PCH<QueueEntryId>) to the CTSU.
    2. The CTSU adds the egress time per the PTP action according to the TX_PCH<TaiSel[1:0]> and subtracts the current time in nanosec of TAI4 from it.
       The checksum is then updated (if needed). The CTSU passes the PTP_PHY_TAG to the EPI in ctsu2epi usx bus<Signature>.
       The CTSU indicates to the EPI that a PCH should be added (ctsu2epi_usx_bus<pkt_tx_ts_pch_mac_tmps_en>=1)
    3. The EPI adds a PCH and set its Signature field according to the ctsu2epi bus (CFG<USX PCH Signature mode>=Q_ENTRY_ID).
    4. The PHY updates the CF by adding the egress time (in TAI4) and updating the checksum (as needed for IPv6, in the trailer)
                                                                                                         .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => CAPTURE
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have (CSTU) store egress time in TSQ and (PHY) correctionField updated in PTP header.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_IRONMAN_L_E))

    /* this test should be run only on simulation when the port is connected to PHY */
    #ifndef ASIC_SIMULATION

        SKIP_TEST_MAC

    #endif

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyCfgSet();

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyTrafficGenerateAndCheck();

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test USXGMII MCH TC mode- Egress 1-step MCH in the Switch + PHY 1-step.

    1. The CPU sends a PTP event message. the frame is forwarded to a target port that is configured (per message type) with a PTP_ACTION = ADD_CORRECTED_TIME.
       As such the HEP timestamp the packet by adding the "-ingress_time" to the CF. Checksum is updated as needed, The CP points the desc<PTP Timestamp offset>
       to the CF field location and passes the desc<SecLsb> of the ingress time.
    2. The CTSU adds the egress time per the PTP action according to the TX_PCH<TaiSel[1:0]> and subtracts the current time in nanosec of TAI4 from it.
       The checksum is then updated (if needed). The CTSU passes the PTP_PHY_TAG to the EPI in ctsu2epi usx bus<Signature>.
       The CTSU indicates to the EPI that a PCH should be added (ctsu2epi_usx_bus<pkt_tx_ts_pch_mac_tmps_en>=1)
    3. The EPI adds a PCH and set its Signature field according to the ctsu2epi bus (CFG<USX PCH Signature mode>=Q_ENTRY_ID).
    4. The PHY updates the CF by adding the egress time (in TAI4) and updating the checksum (as needed for IPv6, in the trailer)
                                                                                                         .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => ADD_CORRECTED_TIME
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have (CSTU) + (PHY) correctionField updated in PTP header.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_IRONMAN_L_E))

    /* this test should be run only on simulation when the port is connected to PHY */
    #ifndef ASIC_SIMULATION

        SKIP_TEST_MAC

    #endif

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeCfgSet();

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeTrafficGenerateAndCheck();

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test USXGMII MCH TC mode- Egress 1-step MCH in the Switch + PHY 2-step.

    1. The CPU sends a PTP event message. the frame is forwarded to a target port that is configured (per message type) with a PTP_ACTION = CAPTURE.
       The HEP stores the packet's info in the egress TSQ.
    2. The CTSU stores the egress time in TSQ per CAPTURE (though not needed) and pass to EPI the Signature fields from the PCH<QueueSelect,QueueEntryId>.
    3. The EPI adds a PCH amd sets the signature from the CTSU.
    4. The PHY Captures the egress time of the packet along with the signature.
                                                                                                         .
       - Configure VLAN FDB Entries.
       - Configure global PTP-related settings to receive PTP packet.
       - Configure packet command to performed on PTP packets per{ingress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => FORWARD
       - Configure action to be performed on PTP packets per{egress port, domain index, message type}:
            {MASTER port,domain index,SYNC}         => CAPTURE
       - Send PTP SYNC packet from MASTER port to SLAVE port: expact to have (CSTU) + (PHY) store egress time in TSQ.
*/

UTF_TEST_CASE_MAC(tgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeTest)
{

    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_IRONMAN_L_E))

    /* this test should be run only on simulation when the port is connected to PHY */
    #ifndef ASIC_SIMULATION

        SKIP_TEST_MAC

    #endif

    /* general bridge configuration */
    prvTgfPtpManagerGenVidAndFdbDefCfgSet();

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* test spesific PTP configuration */
    prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeCfgSet();

    /* Generate traffic: send to device's port given packet under capture and check captured packet */
    prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeTrafficGenerateAndCheck();

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(MASTER_PORT_INDEX);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(SLAVE_PORT_INDEX);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

}

/* AUTODOC:
   Test for checking checksum trailer calculations for IP over UDP packets.
       - Configure VLAN FDB Entries.
       - Configure basic PTP Cofiguration to receive PTP packet Over UDP IP.
       - Configure Timestamp Configuration entry on egress port with 'ADD TIME' action                                                                                                                                                                          									.
       - Configure UDP checksum mode 'CLEAR'.                                                                                                                                                                                                                   																		.
       - Send PTP Over UDP IPv4 packet with length > 128                                                                                                                                                                                                        									.
       - Expect capture packet to have originTimestamp updated in PTP header and UDP checksum cleared; trailer was not updated.                                                                                                                                 									  .
       - Configure UDP checksum mode 'KEEP'                                                                                                                                                                                                                     									.
       - Send PTP Over UDP IPv4 packet with length > 128                                                                                                                                                                                                        																						.
       - Expect capture packet to have originTimestamp updated in PTP header and UDP checksum & trailer were not updated                                                                                                                                        									.
       - Configure UDP checksum mode 'RECALCULATE'                                                                                                                                                                                                              																				.
       - Configure Timestamp Configuration entry on egress port with 'ADD CORRECTED TIME' action.                                                                                                                                                               									.
       - Send PTP Over UDP IPv4 packet with length > 128                                                                                                                                                                                                        																					.
       - Expect capture packet to have correctionField updated in PTP header and UDP checksum trailer was updated in udp payload.                                                                                                                               					.
       - Configure Timestamp Configuration entry on egress port with 'ADD TIME' action.
       - Send PTP Over UDP IPv4 packet with length < 128.
       - Expect capture packet to have originTimestamp updated in PTP header and UDP checksum trailer was updated in udp payload.
       - Configure Timestamp Configuration entry on egress port with 'ADD CORRECTED TIME' action.                                                                                                                                                               									.
       - Send PTP Over UDP IPv4 packet with length < 128.                                                                                                                                                                                                       																					.
       - Expect capture packet to have correctionField updated in PTP header and UDP checksum trailer was updated in udp payload.                                                                                                                               									.
       - Repeat the sequence for PTP Over UDP IPv6 packet.                                                                                                                                                                                                      																					.
*/
UTF_TEST_CASE_MAC(tgfPtpManagerPtpOverIpUDPTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    /* check IPv4 packet */

    /* check packet size > 128 */

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E);

    /* general bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgSet();

    /* test spesific PTP configuration */
    prvTgfPtpManagerPtpOverIpUDPCfgSet(PRV_TGF_PTP_TS_ACTION_ADD_TIME_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_TRUE,GT_FALSE);

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_TRUE,GT_FALSE);

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_TRUE,GT_FALSE);

    /* test spesific PTP configuration */
    prvTgfPtpManagerPtpOverIpUDPCfgSet(PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E,GT_TRUE,GT_FALSE);

    /* check packet size < 128 */

    /* test spesific PTP configuration */
    prvTgfPtpManagerPtpOverIpUDPCfgSet(PRV_TGF_PTP_TS_ACTION_ADD_TIME_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_FALSE,GT_FALSE);

    /* test spesific PTP configuration */
    prvTgfPtpManagerPtpOverIpUDPCfgSet(PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E,GT_FALSE,GT_FALSE);

    /* check IPv6 packet */

    /* check packet size > 128 */

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E);

    /* test spesific PTP configuration */
    prvTgfPtpManagerPtpOverIpUDPCfgSet(PRV_TGF_PTP_TS_ACTION_ADD_TIME_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_CLEAR_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_TRUE,GT_TRUE);

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_KEEP_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_TRUE,GT_TRUE);

    /* global PTP-related settings */
    prvTgfPtpManagerGenPtpOverUdpEnablePtpInit(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_TRUE,GT_TRUE);

    /* check packet size < 128 */

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E,GT_FALSE,GT_TRUE);

    /* test spesific PTP configuration */
    prvTgfPtpManagerPtpOverIpUDPCfgSet(PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);

    /* Generate traffic: send to device's port given packet */
    prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck(PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_RECALCULATE_E,PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E,GT_FALSE,GT_TRUE);

    /* restore PTP configuration for port */
    prvTgfPtpManagerGenPtpRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/* AUTODOC:
   Test for PTP Timstamp Tag Configuration:
   1. In Hybrid mode: combines an 8-byte timestamp tag with a 4-byte piggybacked timestamp;
      - 8B tag = a configurable <Hybrid TST EtherType> (2B) + the 6B <Seconds> field of the timestamp
      - Piggybacked 4B in the PTP <reserved> field, containing the 4B <Nanosecond> field.                                                                 .
   2. In PiggyBack mode: the timestamp is piggybacked onto the 1588v2 packet, using the 4B <reserved> field in the PTP header;
      containing the 4B <Nanosecond> field.
      In this mode, no tag is added to the packet.
   3. In Non-Extended mode: An 8 byte tag comprised of a configurable <TST EtherType> (2B) + payload (6B).
   4. In Extended mode: A 16 byte tag comprised of a configurable <TST EtherType> (2B) + payload (14B).                                     .
*/UTF_TEST_CASE_MAC(tgfPtpManagerTimestampTaggingTest)
{
    PRV_TGF_NOT_SUPPORTED_DEV_CHECK_MAC(prvTgfDevNum, (~ UTF_CPSS_PP_ALL_SIP6_CNS))

    if(PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) == GT_TRUE)
    {
        UTF_SIP6_GM_NOT_READY_SKIP_MAC("\nTest is skipped since OAM is not supported yet in GM\n");
    }

    /* check Hybrid Timestamp Tag */
    prvTgfPtpManagerTimestampTagging(PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E);

    /* check PiggyBack Timestamp Tag */
    prvTgfPtpManagerTimestampTagging(PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E);

    /* check Non-Extended Timestamp Tag */
    prvTgfPtpManagerTimestampTagging(PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E);

    /* check Extended Timestamp Tag */
    prvTgfPtpManagerTimestampTagging(PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E);

}

/*
 * Configuration of tgfPtp suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(tgfPtpManager)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerAddTimeAfterPtpHeaderTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerIngressCaptureTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerIngressTimestampQueueReading)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerEgressTimestampQueueReading)
    /*UTF_SUIT_DECLARE_TEST_MAC(tgfPtpEgressTimestampQueueinterrupt)*/
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerTimestampTagOnMirroredToCPU)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerAddIngressEgressTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerEgressExceptionTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerInvalidPtpTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerCapturePreviousModeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerPiggyBackTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerTC1StepE2ETest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerTC1StepP2PTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerBC2StepE2ETest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerBC1StepE2ETest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManager1StepFollowUpInformationTlvTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerPtpOverIpUDPTest)
    UTF_SUIT_DECLARE_TEST_MAC(tgfPtpManagerTimestampTaggingTest)
UTF_SUIT_END_TESTS_MAC(tgfPtpManager)

