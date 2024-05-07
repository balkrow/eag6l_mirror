/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfBrgNestedDsaToAccess.c
*
* DESCRIPTION: Nested vlan over cascase port enhanced UT
*
*
* FILE REVISION NUMBER:
*       $Revision: 0 $
*
*******************************************************************************/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfCscdGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgNestVlan.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  1

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         0

/* port number to send traffic to */
#define PRV_TGF_EGRESS_PORT_IDX_CNS        1

/* default number of packets to send */
static GT_U32  prvTgfBurstCount   = 1;

/* original Vlan Nested Mode per port */
static struct
{
    CPSS_CSCD_PORT_TYPE_ENT     portCscdType;
    GT_BOOL                     originalVlanNestedMode;
    GT_BOOL                     enableOwnDevFltr;
}prvTgfRestore;

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x22}                /* srcMac */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00,
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00,
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00,
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00,
    0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x00,
    0xaa, 0xbb, 0xcc, 0xdd
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};
static GT_U8 TGF_PACKET_VLAN_TAG[] = {0x81, 0x00, PRV_TGF_VLANID_CNS>>8, PRV_TGF_VLANID_CNS&0xFF};

/* DSA tag without command-specific part(dsaInfo) */
static TGF_PACKET_DSA_TAG_STC  prvTgfDsaTag = {
    TGF_DSA_CMD_FORWARD_E, /*dsaCommand*/
    TGF_DSA_4_WORD_TYPE_E, /*dsaType*/

    /*TGF_DSA_DSA_COMMON_STC*/
    {
        0,        /*vpt*/
        0,        /*cfiBit*/
        PRV_TGF_VLANID_CNS,        /*vid*/
        GT_FALSE, /*dropOnSource*/
        GT_FALSE  /*packetIsLooped*/
    },/*commonParams*/
    /*dsa info*/
    /*filled in the test section*/
    {
        {
            GT_FALSE, /* isEgressPipe */
            GT_FALSE, /* isTagged */
            0,        /* hwDevNum */
            GT_FALSE, /* srcIsTrunk */
            {
                0, /* srcTrunkId */
                0, /* portNum */
                0  /* ePort */
            },        /* interface */
            0,        /* cpuCode */
            GT_FALSE, /* wasTruncated */
            0,        /* originByteCount */
            0,        /* timestamp */
            GT_FALSE, /* packetIsTT */
            {
                0 /* flowId */
            },        /* flowIdTtOffset */
            0
        } /* TGF_DSA_DSA_TO_CPU_STC */

    }/* dsaInfo */
};
/* size of the tagged packet that came from cascade port */
#define PRV_TGF_CSCD_TAG_PACKET_SIZE_CNS (TGF_L2_HEADER_SIZE_CNS           \
                                          + TGF_eDSA_TAG_SIZE_CNS          \
                                          + TGF_ETHERTYPE_SIZE_CNS         \
                                          + sizeof(prvTgfPayloadDataArr))
/* parts of the tagged packet that came from cascade port */
static TGF_PACKET_PART_STC prvTgfCscdPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
     {TGF_PACKET_PART_DSA_TAG_E,   &prvTgfDsaTag},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };
/* a cascade tagged packet info */
static TGF_PACKET_STC prvTgfCscdTaggedPacketInfo = {
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                                    /* totalLen */
    sizeof(prvTgfCscdPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfCscdPacketPartsArray                                       /* partsArray */
};
/**
* @internal prvTgfBrgNestedDsaToAccessConfig function
* @endinternal
*
* @brief   Config the ingress port as cascade and enable nested vlan.
*
*/
GT_VOID prvTgfBrgNestedDsaToAccessConfig()
{
    GT_STATUS                           rc = GT_OK;
    rc = cpssDxChCscdPortTypeGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_PORT_DIRECTION_RX_E, &prvTgfRestore.portCscdType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeGet");
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_PORT_DIRECTION_RX_E, CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeSet");

    rc = cpssDxChBrgNestVlanAccessPortGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], &prvTgfRestore.originalVlanNestedMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgNestVlanAccessPortGet");
    rc = cpssDxChBrgNestVlanAccessPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgNestVlanAccessPortSet");

    rc = prvTgfCscdDsaSrcDevFilterGet(&prvTgfRestore.enableOwnDevFltr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDsaSrcDevFilterGet");

    rc = prvTgfCscdDsaSrcDevFilterSet(GT_FALSE);/*disable the filter*/
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDsaSrcDevFilterSet");
}
/**
* @internal prvTgfBrgNestedDsaToAccessConfigRestore function
* @endinternal
*
* @brief  Restore the cascade and nested vlan config for the ingress port
*         and other cleanup
*
*/
GT_VOID prvTgfBrgNestedDsaToAccessConfigRestore()
{
    GT_STATUS                           rc = GT_OK;
    rc = cpssDxChCscdPortTypeSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], CPSS_PORT_DIRECTION_RX_E, prvTgfRestore.portCscdType);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChCscdPortTypeSet");

    rc = cpssDxChBrgNestVlanAccessPortSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], prvTgfRestore.originalVlanNestedMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChBrgNestVlanAccessPortGet");

    rc = prvTgfCscdDsaSrcDevFilterSet(prvTgfRestore.enableOwnDevFltr);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCscdDsaSrcDevFilterSet");

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush");

    /*restore all buffers and reset mac counters*/
    tgfTrafficTableRxPcktTblClear();
    prvTgfEthCountersReset(prvTgfDevNum);
}
/**
* @internal prvTgfBrgNestedDsaToAccessTrafficSend function
* @endinternal
*
* @brief  Traffic test for eDSA traffic over nested vlan port
*           For different vlan tag types setting in the eDSA tag for ingress port,
*           check the tag state of the egress access port
*/
GT_VOID prvTgfBrgNestedDsaToAccessTrafficSend()
{

    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_U8                 getFirst = 0;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS             rc = GT_OK;
    GT_U32                localDevNum, offset;
    PRV_TGF_BRG_VLAN_PORT_TAG_CMD_ENT type;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &localDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    prvTgfDsaTag.commonParams.vid = PRV_TGF_VLANID_CNS;
    prvTgfDsaTag.dsaInfo.forward.dstInterface.type = CPSS_INTERFACE_PORT_E;
    prvTgfDsaTag.dsaInfo.forward.dstInterface.devPort.hwDevNum = localDevNum;
    prvTgfDsaTag.dsaInfo.forward.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_EGRESS_PORT_IDX_CNS];
    prvTgfDsaTag.dsaInfo.forward.isTrgPhyPortValid = GT_TRUE;
    prvTgfDsaTag.dsaInfo.forward.skipFdbSaLookup = GT_TRUE;

    PRV_UTF_LOG0_MAC("======= Send packet =======\n");
    for (type =PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E ; type<=PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E; type++)
    {
        prvTgfDsaTag.dsaInfo.forward.srcIsTagged = type;
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfCscdTaggedPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");


        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* AUTODOC: Get the rx pkt on egress port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                !getFirst++, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "Error: failed capture with tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet \n" );
        /*verify the packet*/
        offset = TGF_L2_HEADER_SIZE_CNS;
        switch(type)
        {
            /*untag cases*/
            case PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E:
            case PRV_TGF_BRG_VLAN_PORT_TAG1_CMD_E:
                break;

                /*tag cases*/
            case PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E:
            case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E:
            case PRV_TGF_BRG_VLAN_PORT_OUTER_TAG1_INNER_TAG0_CMD_E:
                rc = cpssOsMemCmp(&packetBuf[offset], TGF_PACKET_VLAN_TAG ,TGF_VLAN_TAG_SIZE_CNS);
                UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "vlan tag doesn't match");
                offset += TGF_VLAN_TAG_SIZE_CNS ;
                break;

            default:
                /*Not handled type*/
                break;

        }
        rc = cpssOsMemCmp(&packetBuf[offset], prvTgfPayloadDataArr, TGF_VLAN_TAG_SIZE_CNS);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "payload doesn't match");

    }
    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

}
