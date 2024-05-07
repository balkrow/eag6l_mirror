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
* @file prvTgfPortRadioHeaderUseCase.c
*
* @brief Radio Header addition use case testing
*
* @version   1
********************************************************************************
*/

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>
#include <port/prvTgfPortRadioHeader.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCosGen.h>
#include <ptpManager/prvTgfPtpManagerGen.h>

#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* PHA Tread ID for test */
#define PRV_TGF_RH_PHA_THREAD_ID_CNS         26

/* port to Send/Receive traffic*/
#define PRV_TGF_RH_INGRESS_PORT_IDX_CNS        3
#define PRV_TGF_RH_EGR_PORT_IDX_CNS            1

/* QoS Profile index for test and TC for second packet */
#define PRV_TGF_RH_QOS_PROFILE_INDEX_CNS       123
#define PRV_TGF_RH_TC_CNS                        7

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

/* Packet 1 length including FCS */
#define PRV_TGF_RH_PACKET_1_LENGTH_CNS 64

/* Packet 2 length including FCS */
#define PRV_TGF_RH_PACKET_2_LENGTH_CNS 127

/* maximal captured packet size. It should be at least 4 bytes more
   than ingress packet to hold added radio header */
#define PRV_TGF_RH_PACKET_SIZE_CNS (PRV_TGF_RH_PACKET_2_LENGTH_CNS + 4)

/* DATA of packet 2 */
static GT_U8 prvTgfPayload2DataArr[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x02, 0x03, 0x04, 0x05, 0x06
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacket2PayloadPart = {
    sizeof(prvTgfPayload2DataArr),                       /* dataLength */
    prvTgfPayload2DataArr                                /* dataPtr */
};


/* PARTS of packet #2 */
static TGF_PACKET_PART_STC prvTgfPacket2PartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfOriginalPacketIpv4Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacket2PayloadPart}
};


/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketPtpV2EtherTypePart =
{0x88F7};

static TGF_PACKET_PTP_V2_STC prvTgfPacketPtpV2Part =
{
    0    /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPayload2DataArr)) /*messageLength*/,
    0    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of PTP Over Ethernet */
static TGF_PACKET_PART_STC prvTgfPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketPtpV2EtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/*************************** Restore config ***********************************/

/* parameters that is needed to be restored */


/*************************** Test config ***********************************/

/**
* @internal tgfPortRadioHeaderForwardConfigSet function
* @endinternal
*
* @brief   Forwarding configuration for the use case
*/
static GT_VOID tgfPortRadioHeaderForwardConfigSet
(
    GT_VOID
)
{
    GT_HW_DEV_NUM           localDevNum;
    GT_STATUS               rc;
    CPSS_QOS_ENTRY_STC      qosEntry;

    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &localDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    /* use PVE for forwarding */
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS], GT_TRUE,
                                          prvTgfPortsArray[PRV_TGF_RH_EGR_PORT_IDX_CNS], localDevNum, GT_FALSE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgPrvEdgeVlanPortEnable: %d %d",
                                    prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS],
                                    prvTgfPortsArray[PRV_TGF_RH_EGR_PORT_IDX_CNS]);

    /* AUTODOC: Configure ingress port with default profile */
    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
    qosEntry.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_HARD_E;
    qosEntry.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.qosProfileId = PRV_TGF_RH_QOS_PROFILE_INDEX_CNS;
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS], &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

    /* AUTODOC: Disable QoS trust on ingress port to use only default profile */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS],
        CPSS_QOS_PORT_NO_TRUST_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

    /* AUTODOC: global PTP Over Ethrenet settings */
    prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

    /* AUTODOC: enable timestamp for PTP packet */
    prvTgfPtpManagerGenAddTimeAfterPtpHeaderCfgSet(PRV_TGF_RH_EGR_PORT_IDX_CNS);
}


/**
* @internal tgfPortRadioHeaderForwardConfigRestore function
* @endinternal
*
* @brief   Forwarding configuration restore for the use case
*/
static GT_VOID tgfPortRadioHeaderForwardConfigRestore
(
    GT_VOID
)
{
    GT_STATUS                   rc;
    CPSS_QOS_ENTRY_STC          qosEntry;
    PRV_TGF_COS_PROFILE_STC     qosProfile;

    cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));

    /* AUTODOC: Restore QoS profile to HW default */
    rc = prvTgfCosProfileEntrySet(
        PRV_TGF_RH_QOS_PROFILE_INDEX_CNS, &qosProfile);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

    cpssOsMemSet(&qosEntry, 0, sizeof(qosEntry));
    qosEntry.assignPrecedence =
        CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    qosEntry.enableModifyDscp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.enableModifyUp =
        CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    qosEntry.qosProfileId = 0;

    /* AUTODOC: Restore Port QoS config to default */
    rc = prvTgfCosPortQosConfigSet(
        prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS], &qosEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosPortQosConfigSet");

    /* AUTODOC: Restore Port QoS Trust mode to default */
    rc = prvTgfCosPortQosTrustModeSet(
        prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS],
        CPSS_QOS_PORT_TRUST_L2_L3_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCosPortQosTrustModeSet");

    /* AUTODOC: disable PVE */
    rc = cpssDxChBrgPrvEdgeVlanPortEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS], GT_FALSE,
                                          0, 0, GT_FALSE);

    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "cpssDxChBrgPrvEdgeVlanPortEnable: %d %d",
                                    prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS],
                                    0);

    /* AUTODOC: restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(PRV_TGF_RH_EGR_PORT_IDX_CNS);
}

/**
* internal prvTgfPortRadioHeaderPhaConfigSet function
* @endinternal
*
* @brief   Radio Header use case PHA configurations
*/
static GT_VOID prvTgfPortRadioHeaderPhaConfigSet
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC    commonInfo;
    CPSS_DXCH_PHA_THREAD_INFO_UNT           extInfo;

    /* AUTODOC: Enable PHA on egress port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RH_EGR_PORT_IDX_CNS],
                                        GT_TRUE, PRV_TGF_RH_PHA_THREAD_ID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED,");

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    extInfo.notNeeded = 0;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /* AUTODOC: Set the thread entry */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                                     PRV_TGF_RH_PHA_THREAD_ID_CNS,
                                     &commonInfo,
                                     CPSS_DXCH_PHA_THREAD_TYPE_RADIO_HEADER_ADD_E,
                                     &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,"[TGF]: cpssDxChPhaThreadIdEntrySet FAILED");

}

/**
* internal tgfPortRadioHeaderConfigSet function
* @endinternal
*
* @brief   Radio Header use case configurations
*/
GT_VOID tgfPortRadioHeaderConfigSet
(
    GT_VOID
)
{
    /* Forwarding configuration from Ingress to Egress port */
    tgfPortRadioHeaderForwardConfigSet();

    /* PHA */
    prvTgfPortRadioHeaderPhaConfigSet();

}

/**
* internal tgfPortRadioHeaderVerification function
* @endinternal
*
* @brief   Radio Header use case verification
*/
GT_VOID tgfPortRadioHeaderVerification
(
    GT_U32  packetSize,
    GT_U32  isPtp,
    GT_U32  tc
)
{
    GT_STATUS                           rc;
    GT_U8                               packetBuf[PRV_TGF_RH_PACKET_SIZE_CNS] = {0};
    GT_U32                              packetBufLen = PRV_TGF_RH_PACKET_SIZE_CNS;
    GT_U32                              packetActualLength = 0;
    GT_U8                               queue = 0;
    GT_U8                               dev = 0;
    TGF_NET_DSA_STC                     rxParam;
    GT_BOOL                             getFirst = GT_TRUE;
    GT_U32                              rh_packet_length;
    GT_U32                              rh_pri;
    GT_U32                              rh_ptp;
    GT_U32                              rh_res1, rh_res2;
    CPSS_INTERFACE_INFO_STC             egrPortInterface;
    GT_32                               cmpRes;
    GT_U32                              offset;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_RH_EGR_PORT_IDX_CNS];

    rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&egrPortInterface,
                                                TGF_PACKET_TYPE_CAPTURE_E,
                                                getFirst, GT_TRUE, packetBuf,
                                                &packetBufLen, &packetActualLength,
                                                &dev, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet");

    /* Radio Header format (Network Order). It's before MAC header.
     * |      Byte 0     |      Byte 1     |      Byte 2     |      Byte 3     |
     * | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 | 7 6 5 4 3 2 1 0 |
     * |           Packet length           | Res    |P| PRI  |    Reserved     |
     * |                                   |        |T|      |                 |
     * |                                   |        |P|      |                 |
    */

    rh_packet_length = (packetBuf[1]) | (packetBuf[0] << 8);
    rh_pri           = packetBuf[2] & 0x7;
    rh_ptp           = (packetBuf[2] >> 3) & 1;
    rh_res1          = (packetBuf[2] >> 4) & 0xF;
    rh_res2          =  packetBuf[3];

    /* AUTODOC: Verify the Radio Header */
    UTF_VERIFY_EQUAL0_STRING_MAC(packetSize, rh_packet_length,
                                 "Radio Header Packet length is not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(isPtp, rh_ptp,
                                 "Radio Header PTP is not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(tc, rh_pri,
                                 "Radio Header PRI is not as expected");

    /* reserved bits must be 0 */
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rh_res1,
                                 "Radio Header Res is not as expected");
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rh_res2,
                                 "Radio Header Reserved is not as expected");

    /* AUTODOC: egress packet size should be 4 bytes more */
    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + 4),
                                 packetActualLength,
                                 "Radio Header Reserved is not as expected");

    /* AUTODOC: validate MAC header */
    offset = 4;
    cmpRes = cpssOsMemCmp(prvTgfOriginalPacketL2Part.daMac, packetBuf + offset, 6);
    UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                 cmpRes,
                                 "MAC DA is not as expected");
    offset += 6;
    cmpRes = cpssOsMemCmp(prvTgfOriginalPacketL2Part.saMac, packetBuf + offset, 6);
    UTF_VERIFY_EQUAL0_STRING_MAC(0,
                                 cmpRes,
                                 "MAC SA is not as expected");

    if (isPtp)
    {
        /* timestamp overwrites first 10 bytes of payload:
           - 6 bytes seconds
           - 4 bytes nanoseconds.
           Check that these 10 bytes were changed */
        offset = 4 + TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_PTP_V2_HDR_SIZE_CNS;
        cmpRes = cpssOsMemCmp(prvTgfPayloadDataArr, packetBuf + offset, 10);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(0,
                                     cmpRes,
                                     "Timestamp is not as expected");
    }

}

/**
* @internal tgfPortRadioHeaderConfigRestore function
* @endinternal
*
* @brief   Radio Header use case configurations restore.
* @note    1. Restore Forwarding
*          2. Restore PHA Configuration
*          3. Restore Base Configuration
*
*/
GT_VOID tgfPortRadioHeaderConfigRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* AUTODOC: Restore Forwarding */
    tgfPortRadioHeaderForwardConfigRestore();

    /* AUTODOC: Restore PHA Configuration */

    /* Clear Radio Header thread from the egress port */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RH_EGR_PORT_IDX_CNS],
                                        GT_FALSE, 0);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED");

    /* AUTODOC: Restore base Configuration   */

    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxStartCapture");
}

/**
* @struct RH_TEST_PACKET_STC
 *
*  @brief Defines info for packet that is used for test
*
*/
typedef struct {
    /** pointer to packet's info */
    TGF_PACKET_STC *packetInfoPtr;

    /** 0 - not PTP packet
     *  1 - PTP packet */
    GT_U32          isPtp;

    /** Traffic Class of packet */
    GT_U32          tc;
} RH_TEST_PACKET_STC;

/**
* @internal tgfPortRadioHeaderTest function
* @endinternal
*
* @brief   Radio Header use case test.
*/
GT_VOID tgfPortRadioHeaderTest
(
    GT_VOID
)
{
    GT_STATUS   rc;
    PRV_TGF_COS_PROFILE_STC             qosProfile; /* QoS Profile */
    GT_U32      portIter;
    CPSS_INTERFACE_INFO_STC egrPortInterface;
    TGF_PACKET_STC          packetInfo1;
    TGF_PACKET_STC          packetInfo2;
    TGF_PACKET_STC          packetInfoPtpEth;
    GT_U32                  isPtp;
    GT_U32                  tc;
    GT_U32                  packetLength;
    GT_U32                  ii;
    GT_U32                  packetsNum;
    RH_TEST_PACKET_STC      packetsArr[] = {{&packetInfo1, 0, 0},
                                            {&packetInfo2, 0, 7},
                                            {&packetInfoPtpEth, 1, 5}};

    /* reset counters */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc,
                    "prvTgfResetCountersEth: %d, %d",
                    prvTgfDevNum,
                    prvTgfPortsArray[portIter]);
    }

    packetInfo1.numOfParts =  sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetInfo1.partsArray = prvTgfPacketPartArray;
    packetInfo1.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    packetInfo2.numOfParts =  sizeof(prvTgfPacket2PartArray) / sizeof(prvTgfPacket2PartArray[0]);
    packetInfo2.partsArray = prvTgfPacket2PartArray;
    packetInfo2.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    packetInfoPtpEth.numOfParts =  sizeof(prvTgfPtpV2PacketPartArray) / sizeof(prvTgfPtpV2PacketPartArray[0]);
    packetInfoPtpEth.partsArray = prvTgfPtpV2PacketPartArray;
    packetInfoPtpEth.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;

    /* setup receive portInterface for capturing */
    egrPortInterface.type               = CPSS_INTERFACE_PORT_E;
    egrPortInterface.devPort.hwDevNum   = prvTgfDevNum;
    egrPortInterface.devPort.portNum    = prvTgfPortsArray[PRV_TGF_RH_EGR_PORT_IDX_CNS];


    /* AUTODOC: GENERATE TRAFFIC & Verify at egress port */
    /* enable capture on egress port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                        &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgftrafficgeneratorporttxethcaptureset port - %d",
                                        egrPortInterface.devPort.portNum);

    packetsNum = sizeof(packetsArr) / sizeof(packetsArr[0]);

    for (ii = 0; ii < packetsNum; ii++)
    {
        /* calculate packet size */
        rc = prvTgfPacketSizeGet(packetsArr[ii].packetInfoPtr->partsArray,
                                 packetsArr[ii].packetInfoPtr->numOfParts,
                                 &packetLength);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* add FCS bytes */
        packetLength += 4;
        isPtp = packetsArr[ii].isPtp;
        tc    = packetsArr[ii].tc;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, packetsArr[ii].packetInfoPtr, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth");

        /* AUTODOC: Set TC for a packet */
        cpssOsMemSet(&qosProfile, 0, sizeof(qosProfile));
        qosProfile.trafficClass   = tc;
        rc = prvTgfCosProfileEntrySet(PRV_TGF_RH_QOS_PROFILE_INDEX_CNS, &qosProfile);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCosProfileEntrySet");

        /* Transmit packet */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_RH_INGRESS_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth");

        /* AUTODOC: Verification Started */
        tgfPortRadioHeaderVerification(packetLength, isPtp, tc);

        /* clear capturing RxPcktTable */
        rc = tgfTrafficTableRxPcktTblClear();
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
    }

    /* disable capture on receive port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &egrPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthCaptureSet port - %d",
            egrPortInterface.devPort.portNum);

}
