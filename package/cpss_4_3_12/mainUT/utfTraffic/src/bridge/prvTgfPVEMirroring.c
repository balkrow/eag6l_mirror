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
* @file prvTgfPVEMirroring.c
*
* @brief
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <trafficEngine/tgfTrafficEngine.h>


#include <common/tgfPortGen.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>

#include <bridge/prvTgfPVEMirroring.h>


/* PCL rule index */
static GT_U32   prvTgfPclRuleIndex = 1;
/* ControlPktsToUplink restore enable*/
static GT_U32   upLinkEnRestore = 0;

/* Original Ingress Packet */
static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x10, 0x11, 0x22, 0x33, 0x44, 0x55},               /* daMac */
    {0x20, 0x21, 0x32, 0x43, 0x56, 0x78}                /* saMac */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

static TGF_PACKET_IPV4_STC prvTgfOriginalPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    0x04,               /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,      /* csum */ /*0x5EA0*/
    {10,  1,  1,  4},   /* srcAddr */
    {10,  1,  1,  3}    /* dstAddr */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};


/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfOriginalPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


static GT_VOID tgfBridgePVEMirroringVerification(IN GT_U32  packetSize, IN GT_BOOL controlPacketUpLinkEn);

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

static GT_VOID egrCntrsLog(GT_VOID)
{
    GT_STATUS rc;
    CPSS_PORT_EGRESS_CNTR_STC egrCntr;

    /* clear counters */
    cpssOsMemSet(&egrCntr, 0, sizeof(CPSS_PORT_EGRESS_CNTR_STC));

    rc = prvTgfPortEgressCntrsGet(0, &egrCntr);
    if (GT_OK != rc)
    {
        PRV_UTF_LOG1_MAC(
            "[TGF]: cpssDxChPortEgressCntrsGet FAILED, rc = [%d]", rc);
    }
    PRV_UTF_LOG2_MAC("\negrCntr={ outCtrlFrames=%d, outUcFrames=%d}\n", egrCntr.outCtrlFrames, egrCntr.outUcFrames);
}

static GT_VOID prvTgfBrgPVEConfigSet(GT_VOID)
{
    GT_HW_DEV_NUM           hwDevNum;
    GT_STATUS               rc;
    GT_BOOL enable = GT_TRUE;


    /* Go over all active devices. */
    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &hwDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    /* Enable PVE */
    rc = cpssDxChBrgPrvEdgeVlanEnable(prvTgfDevNum, enable);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgPrvEdgeVlanEnable %d, %d",
                                         prvTgfDevNum, enable);

    /* use PVE for forwarding */
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], GT_TRUE,
                                          prvTgfPortsArray[PRV_TGF_PVE_EGR_PORT_IDX_CNS], hwDevNum, GT_FALSE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgPrvEdgeVlanPortEnable: %d %d",
                                    prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS],
                                    prvTgfPortsArray[PRV_TGF_PVE_EGR_PORT_IDX_CNS]);

    /* save enable for restore  */
    rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], &upLinkEnRestore);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

    rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);
}


/**
* @internal prvTgfBrgPCLConfigSet function
* @endinternal
*
* @brief   Set PCL configuration
*/
static GT_VOID prvTgfBrgPCLConfigSet(GT_VOID)
{

    GT_STATUS                   rc;
    PRV_TGF_PCL_RULE_FORMAT_UNT mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT pattern;
    PRV_TGF_PCL_ACTION_STC      action;

    /**************************************************************************
    * 4. PCL configuration
    */

    rc = prvTgfPclInit();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfPclDefPortInit: %d");

    /* mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));

    /* define mask, pattern and action */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));


    /* action redirect */
    cpssOsMemSet(&action, 0, sizeof(action));
    action.pktCmd = CPSS_PACKET_CMD_MIRROR_TO_CPU_E;
    action.mirror.cpuCode = CPSS_NET_USER_DEFINED_30_E;
    action.egressPolicy = GT_FALSE;


    /* AUTODOC: init PCL Engine for send port 1: */
    /* AUTODOC:   ingress direction, lookup0 */
    /* AUTODOC:   nonIpKey INGRESS_STD_NOT_IP */
    /* AUTODOC:   ipv4Key INGRESS_UDB_30 */
    /* AUTODOC:   ipv6Key INGRESS_STD_IP_L2_QOS */
    rc = prvTgfPclDefPortInit(
        prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS],
        CPSS_PCL_DIRECTION_INGRESS_E,
        CPSS_PCL_LOOKUP_0_E,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E /*nonIpKey*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E /*ipv4Key*/,
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E /*ipv6Key*/);
   UTF_VERIFY_EQUAL3_STRING_MAC(
                GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d",
                prvTgfDevNum,
                prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS],
                CPSS_PCL_DIRECTION_INGRESS_E);

    /* AUTODOC: set PCL rule 1 with: */
    /* AUTODOC:   PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E, cmd=MIRROR */
    rc = prvTgfPclRuleSet(
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E,
        prvTgfPclRuleIndex, &mask, &pattern, &action);

    UTF_VERIFY_EQUAL2_STRING_MAC(
        GT_OK, rc, "prvTgfPclRuleSet: %d, %d",
        PRV_TGF_PCL_RULE_FORMAT_INGRESS_UDB_30_E, prvTgfPclRuleIndex);
}


GT_VOID prvTgfBrgPVEMirroringConfigSet()
{
    prvTgfBrgPVEConfigSet();
    prvTgfBrgPCLConfigSet();
}

/**
* @internal prvTgfBrgPVEMirroringForwardConfigRestore function
* @endinternal
*
* @brief   Forwarding configuration restore for the use case
*/
static GT_VOID prvTgfBrgPVEConfigRestore(GT_VOID)
{
    GT_STATUS                   rc;


    /* AUTODOC: disable PVE */
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], GT_FALSE,
                                          0, 0, GT_FALSE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgPrvEdgeVlanPortEnable: %d %d",
                                    prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS],
                                    0);

    rc = cpssDxChBrgPrvEdgeVlanEnable(prvTgfDevNum, GT_FALSE);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgPrvEdgeVlanEnable: %d, %d",
                                         prvTgfDevNum, GT_FALSE);

    /* reset to default enable  */
    rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], upLinkEnRestore);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);


}

static GT_VOID prvTgfBrgPCLRestore(GT_VOID)
{
    GT_STATUS rc;


    /* AUTODOC: Invalidate PCL rule  */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_30_BYTES_E, prvTgfPclRuleIndex, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", prvTgfPclRuleIndex);

    /* AUTODOC: Clear IPCL Engine configuration tables */
    prvTgfPclRestore();


    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

}

GT_VOID prvTgfBrgPVEMirroringConfigRestore(GT_VOID)
{
    GT_STATUS rc;

    /* AUTODOC: Restore Forwarding */
    prvTgfBrgPVEConfigRestore();

    /* AUTODOC: Restore PCL configuration */
    prvTgfBrgPCLRestore();

    /* AUTODOC: Restore base Configuration   */

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}


GT_VOID prvTgfBrgPVEMirroringTrafficGenerate(GT_VOID)
{

    GT_STATUS   rc;
    GT_U32      portIter;
    TGF_PACKET_STC          packetInfo1;

    /* AUTODOC: reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }
    /* AUTODOC: Log EGR counters*/
    egrCntrsLog();

    packetInfo1.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo1.partsArray = prvTgfPacketPartArray;
    packetInfo1.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* AUTODOC: GENERATE TRAFFIC & Verify at egress port */
    rc = tgfTrafficTableRxStartCapture(GT_TRUE);


    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetInfo1.partsArray,
                                 packetInfo1.numOfParts,
                                 &packetInfo1.totalLen);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* AUTODOC: Test with rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(GT_TRUE) */
    /* setup transmit params */
    rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo1, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");
    cpssOsTimerWkAfter(20);

    /* AUTODOC: Verification Started */
    tgfBridgePVEMirroringVerification(packetInfo1.totalLen, GT_TRUE);

    /* AUTODOC: Test with rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(GT_FALSE) */
    /* setup transmit params */
    rc = cpssDxChBrgPrvEdgeVlanPortControlPktsToUplinkSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS], GT_FALSE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(GT_OK, rc, prvTgfDevNum);

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo1, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

    /* Transmit packet */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PVE_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");
    cpssOsTimerWkAfter(20);

    /* AUTODOC: Verification Started */
    tgfBridgePVEMirroringVerification(packetInfo1.totalLen, GT_FALSE);

   /* stop Rx capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
    GT_OK, rc, "tgfTrafficTableRxStartCapture");


    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}


static GT_VOID tgfBridgePVEMirroringVerification(IN GT_U32  packetSize, IN GT_BOOL controlPacketUpLinkEn )
{
    GT_STATUS                           rc;
    GT_STATUS                           expected_rc;
    GT_U8                               packetBuf[PRV_TGF_PVE_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetBufLen = PRV_TGF_PVE_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = packetSize;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;


   /* AUTODOC: enable packet trace */
    rc = tgfTrafficTracePacketByteSet(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(rc, GT_OK, "Error in tgfTrafficTracePacketByteSet %d", GT_TRUE);

    expected_rc = (controlPacketUpLinkEn == GT_TRUE) ? GT_NO_MORE: GT_OK ;

    /* AUTODOC: get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                       getFirst, GT_TRUE, packetBuf,
                                       &packetBufLen, &packetActualLength,
                                       &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(expected_rc, rc,
                                 "tgfTrafficGeneratorRxInCpuGet");


    /* AUTODOC: Log EGR counters*/
    egrCntrsLog();
}



