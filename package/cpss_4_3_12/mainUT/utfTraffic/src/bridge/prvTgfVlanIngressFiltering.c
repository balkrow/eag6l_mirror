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
* @file prvTgfVlanIngressFiltering.c
*
* @brief VLAN Ingress Filtering Tagged Packets
*
* @version   19
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <bridge/prvTgfVlanIngressFiltering.h>
#include <cpss/generic/config/private/prvCpssConfigTypes.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* number of send traffic iteration */
#define PRV_TGF_MAX_ITER_CNS        2

/* port number to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS   0

/* default VLAN ID */
#define PRV_TGF_DEF_VLAN_ID_CNS     1014

/* invalid VLAN ID */
#define PRV_TGF_INV_VLAN_ID_CNS     3

/*  Inlif number */
#define PRV_TGF_INLIF_CNS     6000


/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS           2

/* default number of packets to send */
static GT_U32       prvTgfBurstCount = 4;


/* VLAN Id array */
static GT_U16        prvTgfVlanIdArray[] = {PRV_TGF_DEF_VLAN_ID_CNS, PRV_TGF_INV_VLAN_ID_CNS};

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {{0x00, 0x00, 0x00, 0x00, 0x34, 0x03},
                                               {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}};

/* VLAN tag part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTagPart = {TGF_ETHERTYPE_8100_VLAN_TAG_CNS,
                                                          0, 0, PRV_TGF_DEF_VLAN_ID_CNS};

/* Data of packet */
static GT_U8 prvTgfPayloadDataArr[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                       0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                                       0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                       0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
                                       0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
                                       0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F};

/* Payload part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {sizeof(prvTgfPayloadDataArr), prvTgfPayloadDataArr};

/* Parts of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {{TGF_PACKET_PART_L2_E, &prvTgfPacketL2Part},
                                                      {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTagPart},
                                                      {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}};


/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/* stored default drop mode */
static PRV_TGF_BRG_DROP_CNTR_MODE_ENT prvTgfDefDropMode = PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E;

/* max vlan id value. For random generation */
#define PRV_TGF_MAX_VLAN_ID         ((UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(prvTgfDevNum) > _4K) ? _4K : UTF_CPSS_PP_MAX_VLAN_INDEX_CNS(prvTgfDevNum))

/**
* @internal prvTgfBrgTwoRandVlansGenerate function
* @endinternal
*
* @brief   Generate two random VLANs
*/
GT_VOID prvTgfBrgTwoRandVlansGenerate
(
    OUT GT_U16 vidArray[2]
)
{
    /* renerate random value in range 2..max */
    vidArray[0]  = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;

    do
    {
        vidArray[1]  = cpssOsRand() % (PRV_TGF_MAX_VLAN_ID - 2) + 2;
    }while(vidArray[0] == vidArray[1]);

    PRV_UTF_LOG2_MAC("We'll use valid VLAN %d, invalid VLAN %d\n", vidArray[0], vidArray[1]);
    return;
}


/**
* @internal prvTgfBrgVlanIngrFltConfigurationSet function
* @endinternal
*
* @brief   Set test configuration
*/
GT_VOID prvTgfBrgVlanIngrFltConfigurationSet
(
    GT_VOID
)
{
    GT_BOOL                             isTagged  = GT_FALSE;
    GT_U32                              portIter  = 0;
    GT_U32                              vlanIter  = 0;
    GT_U32                              portCount = 0;
    GT_U32                              vlanCount = 0;
    PRV_TGF_BRG_DROP_CNTR_MODE_ENT      dropMode  = PRV_TGF_BRG_DROP_CNTR_COUNT_ALL_E;
    GT_STATUS                           rc        = GT_OK;
    CPSS_PORTS_BMP_STC                  portsMembers;
    CPSS_PORTS_BMP_STC                  portsTagging;
    PRV_TGF_BRG_VLAN_INFO_STC           vlanInfo;
    PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;


    /* AUTODOC: SETUP CONFIGURATION: */

    /* clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

    /* set vlan entry */
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);

    /* clear entry */
    cpssOsMemSet(&vlanInfo, 0, sizeof(vlanInfo));
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(PRV_TGF_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set vlan entry */
    vlanInfo.unkSrcAddrSecBreach  = GT_FALSE;
    vlanInfo.unregNonIpMcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv6McastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unkUcastCmd          = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregIpv4BcastCmd    = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.unregNonIpv4BcastCmd = CPSS_PACKET_CMD_FORWARD_E;
    vlanInfo.ipv4IgmpToCpuEn      = GT_FALSE;
    vlanInfo.mirrToRxAnalyzerEn   = GT_FALSE;
    vlanInfo.ipv6IcmpToCpuEn      = GT_FALSE;
    vlanInfo.ipCtrlToCpuEn        = PRV_TGF_BRG_IP_CTRL_NONE_E;
    vlanInfo.ipv4IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv6IpmBrgMode       = CPSS_BRG_IPM_SGV_E;
    vlanInfo.ipv4IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6IpmBrgEn         = GT_FALSE;
    vlanInfo.ipv6SiteIdMode       = CPSS_IP_SITE_ID_INTERNAL_E;
    vlanInfo.ipv4UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv4McastRouteEn     = GT_FALSE;
    vlanInfo.ipv6UcastRouteEn     = GT_FALSE;
    vlanInfo.ipv6McastRouteEn     = GT_FALSE;
    vlanInfo.stgId                = 0;
    vlanInfo.autoLearnDisable     = GT_TRUE;/* working in controlled learning */
    vlanInfo.naMsgToCpuEn         = GT_TRUE;/* working in controlled learning */
    vlanInfo.mruIdx               = 0;
    vlanInfo.bcastUdpTrapMirrEn   = GT_FALSE;
    vlanInfo.vrfId                = 0;
    vlanInfo.floodVidx            = 0xFFF;
    vlanInfo.floodVidxMode        = PRV_TGF_BRG_VLAN_FLOOD_VIDX_MODE_UNREG_MC_E;
    vlanInfo.portIsolationMode    = PRV_TGF_BRG_VLAN_PORT_ISOLATION_DISABLE_CMD_E;
    vlanInfo.ucastLocalSwitchingEn = GT_FALSE;
    vlanInfo.mcastLocalSwitchingEn = GT_FALSE;

    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        portsTaggingCmd.portsCmd[prvTgfPortsArray[portIter]] =
            (CPSS_PORTS_BMP_IS_PORT_SET_MAC(&portsTagging,prvTgfPortsArray[portIter]))
                                    ? PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E
                                    : PRV_TGF_BRG_VLAN_PORT_UNTAGGED_CMD_E;
    }

    /* generate random VLANs for test */
    prvTgfBrgTwoRandVlansGenerate(prvTgfVlanIdArray);

    /* get vlan count */
    vlanCount = sizeof(prvTgfVlanIdArray) / sizeof(prvTgfVlanIdArray[0]);

    /* AUTODOC: create VLAN 2 with ports 0, 1 */
    /* AUTODOC: create VLAN 3 with ports 2, 3 */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        rc = prvTgfBrgVlanEntryWrite(prvTgfDevNum, prvTgfVlanIdArray[vlanIter],
                                     &portsMembers, &portsTagging,
                                     &vlanInfo, &portsTaggingCmd);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryWrite: %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[vlanIter]);
    }

    /* add ports to vlan member */
    portCount = prvTgfPortsNum;
    for (portIter = 0; portIter < portCount; portIter++)
    {
        isTagged = GT_TRUE;

        rc = prvTgfBrgVlanMemberAdd(prvTgfDevNum, prvTgfVlanIdArray[portIter / (portCount / 2)],
                                    prvTgfPortsArray[portIter], isTagged);
        UTF_VERIFY_EQUAL4_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberAdd: %d, %d, %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[portIter / (portCount / 2)],
                                     prvTgfPortsArray[portIter], isTagged);
    }

    /* get default counter for drop packets */
    rc = prvTgfBrgCntDropCntrModeGet(&prvTgfDefDropMode);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeGet: %d", prvTgfDevNum);

    /* AUTODOC: set drop counter mode in "Port not Member in VLAN Drop" */
    dropMode = PRV_TGF_BRG_DROP_CNTR_PORT_NOT_IN_VLAN_E;

    rc = prvTgfBrgCntDropCntrModeSet(dropMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, dropMode);
}

/**
* @internal prvTgfBrgVlanIngrFltTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfBrgVlanIngrFltTrafficGenerate
(
    GT_VOID
)
{
    GT_STATUS       rc         = GT_OK;
    GT_U32          portsCount = 0;
    GT_U32          partsCount = 0;
    GT_U32          packetSize = 0;
    GT_U32          portIter,i = 0;
    GT_U32          numVfd     = 0;
    GT_U32          dropCount  = 0;
    GT_U16          vlanId     = 0;
    GT_BOOL         enable     = GT_FALSE;
    TGF_PACKET_STC  packetInfoPtr;
    CPSS_PORT_MAC_COUNTER_SET_STC portCntrs;


    /* AUTODOC: GENERATE TRAFFIC: */

    /* number of parts in packet */
    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(prvTgfPacketPartArray, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfoPtr.totalLen   = packetSize;
    packetInfoPtr.numOfParts = partsCount;
    packetInfoPtr.partsArray = prvTgfPacketPartArray;

    /* AUTODOC: Make 2 iterations: */
    for (i = 0; i < PRV_TGF_MAX_ITER_CNS; i++)
    {
        /* AUTODOC: enable ingress filtering on send port 0 */
        enable = GT_TRUE;

        rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* update packet */
        ((TGF_PACKET_VLAN_TAG_STC*) packetInfoPtr.partsArray[1].partPtr)->vid = prvTgfVlanIdArray[0];

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending VALID pakets to port [%d] -------\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send traffic with VID 2 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get traffic on ports [0, 1] */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* get Vlan ID */
            rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[portIter], &vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* check counters from def Vlan */
            if (portIter < portsCount / 2)
            {
                if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
                {
                    /* check Rx counters */
                    UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                                 "get another goodOctetsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                                 "get another goodPktsRcv counter than expected");
                    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                                 "get another ucPktsRcv counter than expected");
                }

                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                             "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                             "get another ucPktsSent counter than expected");
            }
            else
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");

                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0], "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0], "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0], "get another ucPktsRcv counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check drop counter */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * i, dropCount, "get another drop counter than expected");

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* update packet */
        ((TGF_PACKET_VLAN_TAG_STC*) packetInfoPtr.partsArray[1].partPtr)->vid = prvTgfVlanIdArray[1];

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending NOT VALID pakets to port [%d] -------\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send traffic with VID 3 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get no traffic */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                             "get another ucPktsRcv counter than expected");
            }
            else
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check dropped counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * (i + 1), dropCount, "get another drop counter than expected");

        /* AUTODOC: disable ingress filtering on send port 0 */
        enable = GT_FALSE;

        rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
        UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

        /* reset counters */
        portsCount = prvTgfPortsNum;

        for (portIter = 0; portIter < portsCount; portIter++)
        {
            rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfResetCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);
        }

        /* update packet */
        ((TGF_PACKET_VLAN_TAG_STC*) packetInfoPtr.partsArray[1].partPtr)->vid = prvTgfVlanIdArray[1];

        /* no VFD applied */
        numVfd = 0;

        /* setup transmit params */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfoPtr, prvTgfBurstCount, numVfd, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfSetTxSetupEth: %d", prvTgfDevNum);

        PRV_UTF_LOG1_MAC("------- Sending VALID pakets to port [%d] -------\n", prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: send traffic with VID 3 on port 0 */
        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfStartTransmitingEth: %d, %d",
                                     prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

        /* AUTODOC: verify to get traffic on ports [2, 3] */
        for (portIter = 0; portIter < portsCount; portIter++)
        {
            /* read counters */
            rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            /* get Vlan ID */
            rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[portIter], &vlanId);
            UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidGet: %d, %d",
                                         prvTgfDevNum, prvTgfPortsArray[portIter]);

            if (PRV_TGF_SEND_PORT_IDX_CNS == portIter)
            {
                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsRcv.l[0],
                                             "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsRcv.l[0],
                                             "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsRcv.l[0],
                                             "get another ucPktsRcv counter than expected");
            }

            /* check counters from def Vlan */
            if (portIter >= portsCount / 2)
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC((packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount, portCntrs.goodOctetsSent.l[0],
                                             "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.goodPktsSent.l[0],
                                             "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount, portCntrs.ucPktsSent.l[0],
                                             "get another ucPktsSent counter than expected");
            }
            else if (PRV_TGF_SEND_PORT_IDX_CNS != portIter)
            {
                /* check Tx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsSent.l[0], "get another goodOctetsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsSent.l[0], "get another goodPktsSent counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsSent.l[0], "get another ucPktsSent counter than expected");

                /* check Rx counters */
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodOctetsRcv.l[0], "get another goodOctetsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.goodPktsRcv.l[0], "get another goodPktsRcv counter than expected");
                UTF_VERIFY_EQUAL0_STRING_MAC(0, portCntrs.ucPktsRcv.l[0], "get another ucPktsRcv counter than expected");
            }
        }

        /* get drop counter */
        rc = prvTgfBrgCntDropCntrGet(&dropCount);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrGet: %d", prvTgfDevNum);

        /* AUTODOC: check dropped counters */
        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfBurstCount * (i + 1), dropCount, "get another drop counter than expected");
    }
}

/**
* @internal prvTgfBrgVlanIngrFltInvalidVidGet function
* @endinternal
*
* @brief   Get invalid VLAN that is used for the test
*/
GT_U16 prvTgfBrgVlanIngrFltInvalidVidGet
(
    GT_VOID
)
{
    return prvTgfVlanIdArray[1];
}


/**
* @internal prvTgfBrgVlanIngrFltConfigurationRestore function
* @endinternal
*
* @brief   Restore test configuration
*/
GT_VOID prvTgfBrgVlanIngrFltConfigurationRestore
(
    GT_VOID
)
{
    GT_U32      vlanIter  = 0;
    GT_U32      vlanCount = 0;
    GT_BOOL     enable    = GT_FALSE;
    GT_STATUS   rc        = GT_OK;


    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: clear drop counters */
    rc = prvTgfBrgCntDropCntrSet(0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrSet: %d", prvTgfDevNum);

    /* get vlan count */
    vlanCount = sizeof(prvTgfVlanIdArray) / sizeof (prvTgfVlanIdArray[0]);

    /* invalidate vlan entry */
    for (vlanIter = 0; vlanIter < vlanCount; vlanIter++)
    {
        /* invalidate vlan entry (and reset vlan entry) */
        rc = prvTgfBrgDefVlanEntryInvalidate(prvTgfVlanIdArray[vlanIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                     prvTgfDevNum, prvTgfVlanIdArray[vlanIter]);
    }

    /* AUTODOC: restore default drop counter mode */
    rc = prvTgfBrgCntDropCntrModeSet(prvTgfDefDropMode);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgCntDropCntrModeSet: %d, %d",
                                 prvTgfDevNum, prvTgfDefDropMode);

    /* AUTODOC: disable ingress filtering */
    enable = GT_FALSE;

    rc = prvTgfBrgVlanPortIngFltEnable(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortIngFltEnable: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], enable);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

}

