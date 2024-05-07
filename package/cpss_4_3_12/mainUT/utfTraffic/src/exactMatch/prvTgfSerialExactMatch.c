/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfSerialExactMatch.c
*
* @brief Functions to support Serial Exact Match testcases
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfBridgeGen.h>
#include <common/tgfCommon.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <exactMatch/prvTgfSerialExactMatch.h>
#include <tunnel/prvTgfTunnelTermEtherType.h>


/******************************* Test packet **********************************/

/* default VLAN Id */
#define PRV_TGF_VLANID_CNS             10
#define PRV_TGF_VLANID_1_CNS           100

/* port to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS       0

/* egress port number to receive traffic from  */
#define PRV_TGF_EGR1_PORT_IDX_CNS       1
#define PRV_TGF_EGR2_PORT_IDX_CNS       2
#define PRV_TGF_EGR3_PORT_IDX_CNS       3

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS          4

#define PRV_TGF_CPU_PORT_IDX_CNS        3

/* profileId number to set */
#define PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS 10

/* cpu code to set */
#define PRV_TGF_EXACT_MATCH_CPU_CODE_CNS    CPSS_NET_FIRST_USER_DEFINED_E + 2

/* default tunnel term entry index */
#define PRV_TGF_TTI_INDEX_CNS       1

/* Serial EM Port profileId */
#define PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS  0
#define PRV_TGF_SRC_PORT_PROFILE_ID_CNS        1

/* Serial EM0 params */
#define PRV_TGF_SERIAL_EM0_PROFILE_ID_CNS 1
#define PRV_TGF_SERIAL_EM0_PCL_ID_CNS     0
/* Serial EM0 Action Params */
#define PRV_TGF_SERIAL_EM0_EGR_VLANID_CNS 100
#define PRV_TGF_SERIAL_EM0_SRC_VPORT_CNS  512

/* Serial EM1 params */
#define PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS 1
#define PRV_TGF_SERIAL_EM1_PCL_ID_CNS     0
/* Serial EM1 Action Params */
#define PRV_TGF_SERIAL_EM1_EGR_VLANID_CNS 101
#define PRV_TGF_SERIAL_EM1_SRC_VPORT_CNS  513

/* Serial EM2 params */
#define PRV_TGF_SERIAL_EM2_PROFILE_ID_CNS 2
#define PRV_TGF_SERIAL_EM2_PCL_ID_CNS     1
/* Serial EM2 Action Params */
#define PRV_TGF_SERIAL_EM2_EGR_VLANID_CNS 102
#define PRV_TGF_SERIAL_EM2_SRC_VPORT_CNS  514

/* default number of packets to send */
static GT_U32  prvTgfBurstCount = 1;

/* parameters that is needed to be restored */
static struct
{
    GT_U32                                      sendPort;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT              lookupClientType;
    struct
    {
        GT_U32                                      profileId;
        GT_BOOL                                     defaultActionEn;
        PRV_TGF_EXACT_MATCH_ACTION_UNT              defaultAction;
        PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC  keyParams;
        GT_U32                                      exactMatchEntryIndex;
        GT_BOOL                                     exactMatchEntryValid;
        PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT         exactMatchEntryActionType;
        PRV_TGF_EXACT_MATCH_ACTION_UNT              exactMatchEntryAction;
        PRV_TGF_EXACT_MATCH_ENTRY_STC               exactMatchEntry;
        GT_U32                                      exactMatchEntryExpandedActionIndex;
        PRV_TGF_EXACT_MATCH_SERIAL_EM_ENTRY_STC     serialEmProfileParams;
    } serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM_LAST_E];
    CPSS_INTERFACE_INFO_STC  physicalInfo[3];
} prvTgfRestoreCfg;

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x44, 0x33, 0x22, 0x11},               /* dstMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x33}                /* srcMac */
};

/* VLAN_TAG0 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* VLAN_TAG1 part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_1_CNS                          /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55
};
/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfEthPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfSingleTagPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

static TGF_PACKET_PART_STC prvTgfDoubleTagPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,       &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag0Part},
    {TGF_PACKET_PART_VLAN_TAG_E, &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_PAYLOAD_E,  &prvTgfPacketPayloadPart}
};

/* packet to send */
static TGF_PACKET_STC prvTgfEthPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen   */
    (sizeof(prvTgfEthPacketPartArray)
        / sizeof(prvTgfEthPacketPartArray[0])), /* numOfParts */
    prvTgfEthPacketPartArray                    /* partsArray */
};

static TGF_PACKET_STC prvTgfSingleTagPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen   */
    (sizeof(prvTgfSingleTagPacketPartArray)
        / sizeof(prvTgfSingleTagPacketPartArray[0])), /* numOfParts */
    prvTgfSingleTagPacketPartArray                    /* partsArray */
};

/* packet to send */
static TGF_PACKET_STC prvTgfDoubleTagPacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen   */
    (sizeof(prvTgfDoubleTagPacketPartArray)
        / sizeof(prvTgfDoubleTagPacketPartArray[0])), /* numOfParts */
    prvTgfDoubleTagPacketPartArray                    /* partsArray */
};

static GT_BOOL prvTgfActionTrapToCpuCheck[] = {
    GT_FALSE, GT_FALSE, GT_FALSE, GT_TRUE, GT_FALSE, GT_FALSE
};

/* LENGTH of packet */
#define PRV_TGF_PACKET_LEN_CNS \
    TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + sizeof(prvTgfPayloadDataArr)

/* LENGTH of packet with CRC */
#define PRV_TGF_PACKET_CRC_LEN_CNS  PRV_TGF_PACKET_LEN_CNS + TGF_CRC_LEN_CNS

/************************************************************************************/

/*Reset the port counters */
static GT_STATUS prvTgfCountersEthReset
(
    GT_VOID
)
{
    GT_U32    portIter;
    GT_STATUS rc, rc1 = GT_OK;

    for (portIter = 0; portIter < PRV_TGF_PORTS_NUM_CNS; portIter++)
    {
        /* reset ethernet counters */
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        PRV_UTF_VERIFY_RC1(rc, "prvTgfResetCountersEth");
    }

    /* Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_VERIFY_RC1(rc, "tgfTrafficTableRxPcktTblClear");

    return rc1;
};

/**
* @internal prvTgfSerialExactMatchTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
*/
GT_VOID prvTgfSerialExactMatchTtiGenericConfigSet
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    CPSS_INTERFACE_INFO_STC  physicalInfo;
    GT_U32                   portItr;
    GT_U32                   srcEPort = 0;

    /* create VLAN 10 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* create VLAN 100 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_SERIAL_EM0_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* create VLAN 101 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_SERIAL_EM1_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* create VLAN 102 */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_SERIAL_EM2_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgDefVlanEntryWrite");

    /* Bind VLAN with port */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
                                 prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]/*Ingress port*/,
                                 PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgVlanPortVidSet");

    /* Get ePort mapping configuration */
    for (portItr = 1; portItr < 4; portItr++)
    {
        switch(portItr)
        {
            case 1:
                srcEPort = PRV_TGF_SERIAL_EM0_SRC_VPORT_CNS;
                break;
            case 2:
                srcEPort = PRV_TGF_SERIAL_EM1_SRC_VPORT_CNS;
                break;
            case 3:
                srcEPort = PRV_TGF_SERIAL_EM2_SRC_VPORT_CNS;
                break;
        }
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                               srcEPort,
                                                               &(prvTgfRestoreCfg.physicalInfo[portItr-1]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    }

    for (portItr = 1; portItr < 4; portItr++)
    {
        switch(portItr)
        {
            case 1:
                srcEPort = PRV_TGF_SERIAL_EM0_SRC_VPORT_CNS;
                break;
            case 2:
                srcEPort = PRV_TGF_SERIAL_EM1_SRC_VPORT_CNS;
                break;
            case 3:
                srcEPort = PRV_TGF_SERIAL_EM2_SRC_VPORT_CNS;
                break;
        }
        /* set ePort mapping configuration */
        physicalInfo.type             = CPSS_INTERFACE_PORT_E;
        physicalInfo.devPort.hwDevNum = prvTgfDevNum;
        physicalInfo.devPort.portNum  = prvTgfPortsArray[portItr];
        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               srcEPort,
                                                               &physicalInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgEportToPhysicalPortTargetMappingTableSet");
    }

    /* SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Configuration =======\n");

    prvTgfRestoreCfg.sendPort = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    if(MUST_USE_ORIG_PORTS_MAC(prvTgfDevNum))
    {
        /* do not modify the 12 ports in prvTgfPortsArray[] !!! */
    }
    else
    {
        prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = 16;
    }

    /* Enable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E,
                                      GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);
}

/**
* @internal prvTgfSerialExactMatchTtiGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfSerialExactMatchTtiGenericConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      portItr;
    GT_U32      srcEPort = 0;

    PRV_UTF_LOG0_MAC("======= Restore Configuration =======\n");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* Disable TTI lookup for port 0, key TTI_KEY_ETH */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_ETH_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* Disable TTI lookup for port 0, key TTI_KEY_UDB_ETHERNET_OTHER_E */
    rc = prvTgfTtiPortLookupEnableSet(prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                      PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d", prvTgfDevNum);

    /* invalidate vlan entry (and reset vlan entry) */
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SERIAL_EM0_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SERIAL_EM0_EGR_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SERIAL_EM1_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SERIAL_EM1_EGR_VLANID_CNS);

    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_SERIAL_EM2_EGR_VLANID_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SERIAL_EM2_EGR_VLANID_CNS);

    /* restore default PVID for port 0 */
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 1);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanPortVidSet: %d, %d, %d",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS], 1);

    for (portItr = 1; portItr < 4; portItr++)
    {
        switch(portItr)
        {
            case 1:
                srcEPort = PRV_TGF_SERIAL_EM0_SRC_VPORT_CNS;
                break;
            case 2:
                srcEPort = PRV_TGF_SERIAL_EM1_SRC_VPORT_CNS;
                break;
            case 3:
                srcEPort = PRV_TGF_SERIAL_EM2_SRC_VPORT_CNS;
                break;
        }

        rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                               srcEPort,
                                                               &(prvTgfRestoreCfg.physicalInfo[portItr-1]));
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Error of prvTgfBrgEportToPhysicalPortTargetMappingTableSet");
    }

    /* Restore send port */
    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS] = prvTgfRestoreCfg.sendPort;
}

/**
* @internal prvTgfSerialExactMatchTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*
*/
GT_VOID prvTgfSerialExactMatchTrafficGenerate
(
    GT_U32      tag,
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
)
{
    GT_STATUS                       rc          = GT_OK;
    GT_U32                          partsCount  = 0;
    GT_U32                          packetSize  = 0;
    GT_U32                          portIter    = 0;
    GT_U32                          numTriggers = 0;
    TGF_VFD_INFO_STC                vfdArray[1];
    CPSS_INTERFACE_INFO_STC         portInterface;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    CPSS_PORT_MAC_COUNTER_SET_STC   expectedCntrs;
    GT_U32                          vlanTagsSize;
    GT_U32                          egressPortIdx = 0;

    /* GENERATE TRAFFIC: */
    PRV_UTF_LOG0_MAC("======= Generating Traffic prvTgfTunnelTermEtherTypeTrafficGenerateExpectTraffic =======\n");

    /* clear VFD array */
    cpssOsMemSet((GT_VOID*) vfdArray, 0, sizeof(vfdArray));

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* Reset the port Counter */
    rc = prvTgfCountersEthReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfCountersEthReset");

    /* setup portInterface for capturing */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;

    switch(tag)
    {
        case PRV_TGF_UNTAGGED_E:
            egressPortIdx = PRV_TGF_EGR1_PORT_IDX_CNS;
            break;
        case PRV_TGF_SINGLE_TAGGED_E:
            egressPortIdx = PRV_TGF_EGR2_PORT_IDX_CNS;
            break;
        case PRV_TGF_DOUBLE_TAGGED_E:
            egressPortIdx = PRV_TGF_EGR3_PORT_IDX_CNS;
            break;
    }

    PRV_UTF_LOG0_MAC("======= set capture on all ports =======\n");
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIdx];
    /* enable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIdx]);
    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */
    if (tag == PRV_TGF_UNTAGGED_E)
    {
        /* number of parts in packet */
        partsCount = sizeof(prvTgfEthPacketPartArray) / sizeof(prvTgfEthPacketPartArray[0]);

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfEthPacketPartArray, partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEthPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        vlanTagsSize = 0;
    }
    else if (tag == PRV_TGF_SINGLE_TAGGED_E)
    {
        /* number of parts in packet */
        partsCount = sizeof(prvTgfSingleTagPacketPartArray) / sizeof(prvTgfSingleTagPacketPartArray[0]);

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfSingleTagPacketPartArray, partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfSingleTagPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

        vlanTagsSize = TGF_VLAN_TAG_SIZE_CNS;
    }
    else
    {
        /* number of parts in packet */
        partsCount = sizeof(prvTgfDoubleTagPacketPartArray) / sizeof(prvTgfDoubleTagPacketPartArray[0]);

        /* calculate packet size */
        rc = prvTgfPacketSizeGet(prvTgfDoubleTagPacketPartArray, partsCount, &packetSize);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

        /* setup packet */
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfDoubleTagPacketInfo, prvTgfBurstCount, 0, NULL);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);
        vlanTagsSize = (TGF_VLAN_TAG_SIZE_CNS*2);
    }

    /* send Ethernet packet from port 0 with: */
    /* DA=00:00:44:33:22:11, SA=00:00:00:00:00:33, VID=10 */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS]);

    /* ======= disable capture on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIdx];
    /* disable capture on port 1 */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&portInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet: %d, &d\n",
                                 prvTgfDevNum, prvTgfPortsArray[egressPortIdx]);

    /* -------------------------------------------------------------------------
     * 3. Get Ethernet Counters
     */

    /* verify to get no traffic - dropped */
    for (portIter = 0; portIter < PRV_TGF_PORT_COUNT_CNS; portIter++)
    {
        GT_BOOL isOk;
        if ((portIter != egressPortIdx) && portIter!=0)
        {
            continue;
        }
        /* read counters */
        rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter], GT_TRUE, &portCntrs);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfReadPortCountersEth: %d, %d\n",
                                     prvTgfDevNum, prvTgfPortsArray[portIter]);

        /* check Tx and Rx counters */
        switch (portIter) {
            case PRV_TGF_SEND_PORT_IDX_CNS:

                /* packetSize is not changed */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;

                break;

        default:
             if (expectForwardTraffic == GT_TRUE)
             {
                /* port has both Rx and Tx counters because of capture */
                expectedCntrs.goodOctetsSent.l[0] = (packetSize + TGF_CRC_LEN_CNS - vlanTagsSize) * prvTgfBurstCount;
                expectedCntrs.goodPktsSent.l[0]   = prvTgfBurstCount;
                expectedCntrs.ucPktsSent.l[0]     = prvTgfBurstCount;
                expectedCntrs.brdcPktsSent.l[0]   = 0;
                expectedCntrs.mcPktsSent.l[0]     = 0;
                expectedCntrs.goodOctetsRcv.l[0]  = (packetSize + TGF_CRC_LEN_CNS - vlanTagsSize) * prvTgfBurstCount;
                expectedCntrs.goodPktsRcv.l[0]    = prvTgfBurstCount;
                expectedCntrs.ucPktsRcv.l[0]      = prvTgfBurstCount;
                expectedCntrs.brdcPktsRcv.l[0]    = 0;
                expectedCntrs.mcPktsRcv.l[0]      = 0;
             }
             else
             {
                expectedCntrs.goodOctetsSent.l[0] = 0;
                expectedCntrs.goodPktsSent.l[0]    = 0;
                expectedCntrs.ucPktsSent.l[0]      = 0;
                expectedCntrs.brdcPktsSent.l[0]    = 0;
                expectedCntrs.mcPktsSent.l[0]      = 0;
                expectedCntrs.goodOctetsRcv.l[0]   = 0;
                expectedCntrs.goodPktsRcv.l[0]     = 0;
                expectedCntrs.ucPktsRcv.l[0]       = 0;
                expectedCntrs.brdcPktsRcv.l[0]     = 0;
                expectedCntrs.mcPktsRcv.l[0]       = 0;
             }
        }

        isOk =
            portCntrs.goodOctetsSent.l[0] == expectedCntrs.goodOctetsSent.l[0] &&
            portCntrs.goodPktsSent.l[0]   == expectedCntrs.goodPktsSent.l[0] &&
            portCntrs.ucPktsSent.l[0]     == expectedCntrs.ucPktsSent.l[0] &&
            portCntrs.brdcPktsSent.l[0]   == expectedCntrs.brdcPktsSent.l[0] &&
            portCntrs.mcPktsSent.l[0]     == expectedCntrs.mcPktsSent.l[0] &&
            portCntrs.goodOctetsRcv.l[0]  == expectedCntrs.goodOctetsRcv.l[0] &&
            portCntrs.goodPktsRcv.l[0]    == expectedCntrs.goodPktsRcv.l[0] &&
            portCntrs.ucPktsRcv.l[0]      == expectedCntrs.ucPktsRcv.l[0] &&
            portCntrs.brdcPktsRcv.l[0]    == expectedCntrs.brdcPktsRcv.l[0] &&
            portCntrs.mcPktsRcv.l[0]      == expectedCntrs.mcPktsRcv.l[0];

        UTF_VERIFY_EQUAL0_STRING_MAC(isOk, GT_TRUE, "get another counters values.");

        /* print expected values if bug */
        if (isOk != GT_TRUE) {
            PRV_UTF_LOG0_MAC("Expected values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", expectedCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", expectedCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", expectedCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", expectedCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", expectedCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", expectedCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", expectedCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", expectedCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", expectedCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", expectedCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");

            PRV_UTF_LOG0_MAC("Current values:\n");
            PRV_UTF_LOG1_MAC(" goodOctetsSent = %d\n", portCntrs.goodOctetsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsSent = %d\n", portCntrs.goodPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsSent = %d\n", portCntrs.ucPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsSent = %d\n", portCntrs.brdcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsSent = %d\n", portCntrs.mcPktsSent.l[0]);
            PRV_UTF_LOG1_MAC(" goodOctetsRcv = %d\n", portCntrs.goodOctetsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" goodPktsRcv = %d\n", portCntrs.goodPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" ucPktsRcv = %d\n", portCntrs.ucPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" brdcPktsRcv = %d\n", portCntrs.brdcPktsRcv.l[0]);
            PRV_UTF_LOG1_MAC(" mcPktsRcv = %d\n", portCntrs.mcPktsRcv.l[0]);
            PRV_UTF_LOG0_MAC("\n");
        }
    }

    /* -------------------------------------------------------------------------
     * 4. Get Trigger Counters
     */

    /* get trigger counters */
    vfdArray[0].mode = TGF_VFD_MODE_STATIC_E;
    vfdArray[0].modeExtraInfo = 0;
    vfdArray[0].offset = 0;
    vfdArray[0].cycleCount = sizeof(TGF_MAC_ADDR);
    cpssOsMemCpy(vfdArray[0].patternPtr, prvTgfPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* ======= check on all ports ======= */
    portInterface.devPort.portNum = prvTgfPortsArray[egressPortIdx];

    rc = tgfTrafficGeneratorPortTxEthTriggerCountersGet(&portInterface, 1, vfdArray, &numTriggers);
    PRV_UTF_LOG2_MAC("    numTriggers = %d, rc = 0x%02X\n\n", numTriggers, rc);

    /* check TriggerCounters */
    rc = rc == GT_NO_MORE ? GT_OK : rc;
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorPortTxEthTriggerCountersGet: %d, %d\n",
                                 portInterface.devPort.hwDevNum, portInterface.devPort.portNum);

    if (expectForwardTraffic==GT_TRUE)
    {
        /* check if there are captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(1, numTriggers, "the test expected traffic to be forwarded\n");
    }
    else
    {
        /* check if no captured packets at all */
        UTF_VERIFY_EQUAL0_STRING_MAC(0, numTriggers, "the test expected traffic NOT to be forwarded\n");

        if (GT_TRUE == prvTgfActionTrapToCpuCheck[PRV_TGF_CPU_PORT_IDX_CNS])
        {
            GT_U8                           packetBufPtr[64] = {0};
            GT_U32                          packetBufLen = 64;
            GT_U32                          packetLen;
            GT_U8                           devNum;
            GT_U8                           queueCpu;
            TGF_NET_DSA_STC                 rxParams;

            PRV_UTF_LOG0_MAC("CPU port capturing:\n");

            cpssOsBzero((GT_VOID*)&rxParams, sizeof(TGF_NET_DSA_STC));
            rc = tgfTrafficGeneratorRxInCpuGet(TGF_PACKET_TYPE_REGULAR_E,
                                               GT_TRUE, GT_TRUE,
                                               packetBufPtr,
                                               &packetBufLen,
                                               &packetLen,
                                               &devNum,
                                               &queueCpu,
                                               &rxParams);

            if(expectTrapTraffic==GT_FALSE)
            {
                /* we want to verify no packet was trapped to CPU
                   the command in the Exact Match Default Action was DROP */
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                UTF_VERIFY_NOT_EQUAL0_PARAM_MAC(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS,
                                                rxParams.cpuCode);

                PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                PRV_UTF_LOG0_MAC("Packet is NOT Trapped to  CPU. Packet is dropped\n\n");
            }
            else
            {
                 /* we want to verify packet was trapped to CPU */
                 UTF_VERIFY_EQUAL0_PARAM_MAC(GT_TRUE,rxParams.dsaCmdIsToCpu);
                 UTF_VERIFY_EQUAL0_PARAM_MAC(PRV_TGF_EXACT_MATCH_CPU_CODE_CNS,rxParams.cpuCode);

                 PRV_UTF_LOG1_MAC("Source Port :%d\n",rxParams.portNum);
                 PRV_UTF_LOG1_MAC("Packet is Trapped to  CPU with CPU Code  :%d\n\n",rxParams.cpuCode);
            }
        }
    }

    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficTableRxStartCapture");

    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");
}

/**
* @internal prvTgfSerialExactMatchInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfSerialExactMatchInvalidateEmEntry
(
    GT_VOID
)
{
    GT_STATUS           rc;
    GT_PORT_GROUPS_BMP  portGroupsBmp = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;

    /* invalidate the entry */
    rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E, PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E,
                                                  portGroupsBmp, prvTgfRestoreCfg.serialEmParams[0].exactMatchEntryIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
}

/*-----------------------------------------------------------------------------------------------------------------*/

/****** Functions for Basic Default Action Testcase *******/

/**
* @internal prvTgfSerialExactMatchBasicDefaultActionTtiCommandTrap function
* @endinternal
*
* @brief   set default action command to be trap with cpu code [502]
*
*/
GT_VOID prvTgfSerialExactMatchBasicDefaultActionTtiCommandTrap
(
    GT_VOID
)
{
    GT_STATUS                               rc;
    GT_BOOL                                 enableDefaultAction;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          action ;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &action, 0, sizeof(action));

    action.ttiAction.command            = CPSS_PACKET_CMD_TRAP_TO_CPU_E ;
    action.ttiAction.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    action.ttiAction.userDefinedCpuCode = PRV_TGF_EXACT_MATCH_CPU_CODE_CNS;
    actionType                          = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    enableDefaultAction                 = GT_TRUE ;

    rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                 PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS, actionType,
                                                 &action, enableDefaultAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
}

/**
* @internal prvTgfSerialExactMatchBasicDefaultActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Serial Exact Match Default Action
*
*/
GT_VOID prvTgfSerialExactMatchBasicDefaultActionTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    GT_BOOL                                 enableDefaultAction;

    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          action ;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT          clientType;
    PRV_TGF_EXACT_MATCH_SERIAL_EM_ENTRY_STC serialEmProfileParams;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &action, 0, sizeof(action));

    ttiLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    action.ttiAction.command            = CPSS_PACKET_CMD_FORWARD_E;
    action.ttiAction.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;
    actionType                          = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;
    enableDefaultAction                 = GT_TRUE ;

    /*save */
    rc = prvTgfExactMatchProfileDefaultActionGet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                 PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS ,
                                                 actionType,&prvTgfRestoreCfg.serialEmParams[1].defaultAction,
                                                 &prvTgfRestoreCfg.serialEmParams[1].defaultActionEn);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionGet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                         ttiLookupNum,
                                         &prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /*set TTI Serial EM Client registration */
    clientType = PRV_TGF_EXACT_MATCH_CLIENT_TTI_SERIAL_EM_E;
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E, ttiLookupNum, clientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* set port profile ID */
    rc = prvTgfExactMatchSerialPortProfileIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialPortProfileIdSet FAILED: %d", prvTgfDevNum);

    /* Populate params for Serial EM1 */
    serialEmProfileParams.serialEmProfileId             = PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS;
    serialEmProfileParams.serialEmKeyType               = PRV_TGF_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E;
    serialEmProfileParams.serialEmEnableUDB28VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmEnableUDB29VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmPclId                 = PRV_TGF_SERIAL_EM1_PCL_ID_CNS;
    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM1_E,
                                                               &serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                 PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS,
                                                 actionType, &action, enableDefaultAction);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);

}

/**
* @internal prvTgfSerialExactMatchBasicDefaultActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Serial Exact Match
*          Default Action
*/
GT_VOID prvTgfSerialExactMatchBasicDefaultActionTtiConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          zeroAction;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));

    ttiLookupNum = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    actionType = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    if (prvTgfRestoreCfg.serialEmParams[1].defaultActionEn == GT_TRUE )
    {
        /* Restore Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                     PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS,
                                                     actionType,
                                                     &prvTgfRestoreCfg.serialEmParams[1].defaultAction,
                                                     GT_TRUE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }
    else
    {
        /* Restore Exact Match default Action */
        rc = prvTgfExactMatchProfileDefaultActionSet(prvTgfDevNum,
                                                     PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                     PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS,
                                                     actionType,
                                                     &zeroAction,
                                                     GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileDefaultActionSet FAILED: %d", prvTgfDevNum);
    }

    /* Restore first lookup client type */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                         ttiLookupNum, prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

}

/*-----------------------------------------------------------------------------------------------------------------*/

/****** Functions for Basic Expanded Action Testcase *******/

/**
* @internal prvTgfSerialExactMatchBasicExpandedActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Serial Exact Match Expanded Action
*
*/
GT_VOID prvTgfSerialExactMatchBasicExpandedActionTtiConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;

    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  lookupClientType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  lookupNum;
    GT_U32                                          profileId;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;
    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];
    PRV_TGF_EXACT_MATCH_SERIAL_EM_ENTRY_STC         serialEmProfileParams;

    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));

    lookupNum                   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    lookupClientType            = PRV_TGF_EXACT_MATCH_CLIENT_TTI_SERIAL_EM_E;
    profileId                   = PRV_TGF_EXACT_MATCH_SERIAL_EM1_E;
    expandedActionIndex         = PRV_TGF_EXACT_MATCH_SERIAL_EM1_E;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E ;

    actionData.ttiAction.command            = CPSS_PACKET_CMD_FORWARD_E;
    actionData.ttiAction.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E;

    keyParams.keySize           = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;

    cpssOsMemSet((GT_VOID*) &keyParams.mask[0], 0, sizeof(keyParams.mask));

    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.lookupNum   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;

    /* Check will all zero */
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));

    /* calculate index */
    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-1];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }
    prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryIndex=index;

    /* save config */
    /* keep clientType configured for first lookup , for restore */
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                         lookupNum, &prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* Ssave Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E, profileId,
                                              &prvTgfRestoreCfg.serialEmParams[1].keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* Save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,
                                            PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                            PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E,
                                            portGroupsBmp,index,
                                            &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryValid,
                                            &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryActionType,
                                            &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryAction,
                                            &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntry,
                                            &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    /* Set TTI Serial EM Client registration */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                         lookupNum, lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* Set port profile ID */
    rc = prvTgfExactMatchSerialPortProfileIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialPortProfileIdSet FAILED: %d", prvTgfDevNum);

    /* Populate params for Serial EM1 */
    serialEmProfileParams.serialEmProfileId             = PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS;
    serialEmProfileParams.serialEmKeyType               = PRV_TGF_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E;
    serialEmProfileParams.serialEmEnableUDB28VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmEnableUDB29VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmPclId                 = PRV_TGF_SERIAL_EM1_PCL_ID_CNS;
    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM1_E,
                                                               &serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet FAILED: %d", prvTgfDevNum);

    /* Set Expanded Action for Serial EM lookup, profileId=1, ActionType=TTI, packet Command = Forward */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E, profileId,
                                           actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    /* Set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                              profileId, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* Set Exact Match Entry */
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,
                                            PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                            PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E,
                                            portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same as configured in Expanded Entry */
                                            &actionData);/* same as configured in Expanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfSerialExactMatchBasicExpandedActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Serial Exact Match
*          Expanded Action
*/
GT_VOID prvTgfSerialExactMatchBasicExpandedActionTtiConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          zeroAction;
    GT_PORT_GROUPS_BMP                      portGroupsBmp;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  zeroActionOrigin;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));
    cpssOsMemSet(&zeroActionOrigin, 0, sizeof(zeroActionOrigin));

    /* Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    ttiLookupNum    = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    actionType      = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    /* Restore first lookup client type */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                         ttiLookupNum, prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    if (prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryValid == GT_TRUE)
    {
        /* Restore Exact Match Entry */
        rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,
                                                PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                                PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E,
                                                portGroupsBmp,
                                                prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryIndex,
                                                prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryExpandedActionIndex,
                                                &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntry,
                                                prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryActionType,/* same as configured in Extpanded Entry */
                                                &prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryAction);/* same a sconfigured in Extpanded Entry */
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);
    }
    else
    {
        /* invalidate the entry */
        rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E, PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E, portGroupsBmp,
                                                      prvTgfRestoreCfg.serialEmParams[1].exactMatchEntryIndex);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
    }

    /* Restore Expanded Action for TTI lookup */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                           PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                           actionType, &zeroAction,
                                           &zeroActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);


    /* Restore Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum, PRV_TGF_EXACT_MATCH_UNIT_0_E,
                                              PRV_TGF_EXACT_MATCH_PROFILE_ID_CNS,
                                              &prvTgfRestoreCfg.serialEmParams[1].keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    return;
}

/*-----------------------------------------------------------------------------------------------------------------*/

/**
* @internal prvTgfSerialExactMatchKeyAndActionConfigSet function
* @endinternal
*
* @brief Serial Exact Match Configurations for Serial-EM0/EM1/EM2 handing
*
*/
GT_VOID prvTgfSerialExactMatchKeyAndActionConfigSet
(
    GT_VOID
)
{
    GT_STATUS rc;
    PRV_TGF_EXACT_MATCH_CLIENT_ENT                  lookupClientType;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT                  lookupNum;
    GT_U32                                          profileId;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT             actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT                  actionData;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  expandedActionOrigin;
    PRV_TGF_EXACT_MATCH_PROFILE_KEY_PARAMS_STC      keyParams;
    GT_PORT_GROUPS_BMP                              portGroupsBmp=CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    GT_U32                                          expandedActionIndex;
    PRV_TGF_EXACT_MATCH_ENTRY_STC                   exactMatchEntry;
    GT_U32                                          index=0;
    GT_U32                                          numberOfElemInCrcMultiHashArr;
    GT_U32                                          exactMatchCrcMultiHashArr[16];
    PRV_TGF_EXACT_MATCH_SERIAL_EM_ENTRY_STC         serialEmProfileParams;
    PRV_TGF_EXACT_MATCH_UNIT_NUM_ENT                emUnitNum;
    /* clear entry */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));

    /* take all fields from Expanded Action: all fields overwriteExpande=GT_FALSE*/
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));
    cpssOsMemSet((GT_VOID*) &keyParams.mask[0], 0, sizeof(keyParams.mask));
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));

     /* ==============================Configurations for Serial EM0================
     * Serial EM port Profile ID = 0
     * Incoming packet = Ethernet(Type = 5)
     * =======================Serial EM0 Profile ID Table===============================================
     *    Index	  ProfileId	   KeyType	EnableUDB29  EnableUDB28   PCL-ID
     *     40	      1	          0	        0	          0	         0
     * =================================================================================================
     * ========================EMX Profile Table (EM0)==================================================
     *    Index   KeySize	  KeyMask	     DefActionEnable	DefAction          Description
     *      1	     5B	     FF:F8:00:00:00	    False	           ∅         Any Tagged/Untagged packets
     * =================================================================================================
     * ================================EM Table (EM0):==================================================
     * ========= Key ===================================================================================
     *    PclId   SourcePortProfile   Tag0Found	   Tag0    Tag1Found	 Tag1
     *      0	        1	             Yes	    ∅	       ∅          ∅
     *      0	        1	             No 	    ∅	       ∅          ∅
     * ========= Action ================================================================================
     *    Src vPort   IntAssignmentEnable    eVlan	  PLRIndex    GCF
     *        0	            2	              NA	    NA	       NA
     *       50	            2	              100	    0	       GCF[12:1] = 0
     * =================================================================================================
     */
    emUnitNum                   = PRV_TGF_EXACT_MATCH_UNIT_REDUCED_E;

    /* KeySize is 5B */
    keyParams.keySize           = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;

    /* Key Mask is 0xFFF8000000*/
    keyParams.mask[4]           = 0xFF;
    keyParams.mask[3]           = 0xF8;

    /* EM key configurations */
    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.lookupNum   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;    /* Dummy initialization for Reduced EM */
    /* PCL ID           = 0 , bits 39...38
     * Src Port Profile = 1 , bits 37...28 */
    exactMatchEntry.key.pattern[3] = 0x10;

    /* this is dummy initialization for Reduced EM */
    profileId                   = PRV_TGF_SERIAL_EM0_PROFILE_ID_CNS;
    expandedActionIndex         = PRV_TGF_SERIAL_EM0_PROFILE_ID_CNS;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    /*actionData.ttiAction.interfaceAssignmentEnable    = 2;*/
    actionData.ttiAction.command                      = CPSS_PACKET_CMD_FORWARD_E;
    actionData.ttiAction.userDefinedCpuCode           = CPSS_NET_FIRST_USER_DEFINED_E;
    actionData.ttiAction.tag0VlanCmd                  = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    actionData.ttiAction.tag0VlanId                   = PRV_TGF_SERIAL_EM0_EGR_VLANID_CNS;

    /* Eport Action configs */
    actionData.ttiAction.sourceEPortAssignmentEnable      = GT_TRUE;
    actionData.ttiAction.sourceEPort                      = PRV_TGF_SERIAL_EM0_SRC_VPORT_CNS;
    actionData.ttiAction.redirectCommand                  = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    actionData.ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    actionData.ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    actionData.ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR1_PORT_IDX_CNS];
    actionData.ttiAction.ingressPipeBypass                = GT_FALSE;
    actionData.ttiAction.bridgeBypass                     = GT_TRUE;

    prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].profileId = profileId;

    /* calculate index */
    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum, emUnitNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-1];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }
    prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].exactMatchEntryIndex=index;
    /* save config */
    /* Save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum, emUnitNum, profileId,
                                              &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* Save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,
                                            emUnitNum,
                                            PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E, /* Dummy for Reduced EM */
                                            portGroupsBmp,index,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].exactMatchEntryValid,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].exactMatchEntryActionType,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].exactMatchEntryAction,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].exactMatchEntry,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntryGet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM0_E,
                                                               &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM0_E].serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntryGet FAILED: %d", prvTgfDevNum);

    /* Set port profile ID */
    rc = prvTgfExactMatchSerialPortProfileIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialPortProfileIdSet FAILED: %d", prvTgfDevNum);

    /* Populate params for Serial EM0 */
    serialEmProfileParams.serialEmProfileId             = profileId;
    serialEmProfileParams.serialEmKeyType               = PRV_TGF_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E;
    serialEmProfileParams.serialEmEnableUDB28VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmEnableUDB29VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmPclId                 = PRV_TGF_SERIAL_EM0_PCL_ID_CNS;
    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM0_E,
                                                               &serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet FAILED: %d", prvTgfDevNum);

    /* Set the Generic Range Source Port Profile ID for specific port */
    rc = cpssDxChTtiGenericRangeSrcPortProfileIdSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    PRV_TGF_SRC_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTtiGenericRangeSrcPortProfileIdSet: %d", prvTgfDevNum);

    /* Set Expanded Action for Serial EM lookup, profileId=1, ActionType=TTI, packet Command = Forward */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, emUnitNum, profileId,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    /* Set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum, emUnitNum,
                                              profileId, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* Set Exact Match Entry */
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,
                                            emUnitNum,
                                            PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E, /* Dummy for Reduced EM*/
                                            portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same as configured in Expanded Entry */
                                            &actionData);/* same as configured in Expanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    /*
     * ==============================Configurations for Serial EM1================
     * Serial EM port Profile ID = 0
     * Incoming packet = Ethernet(Type = 5)
     * =======================Serial EM1 Profile ID Table===============================================
     *    Index	  ProfileId	   KeyType	EnableUDB29  EnableUDB28   PCL-ID
     *     40	      1	          0	        0	          0	         0
     * =================================================================================================
     * ========================EMX Profile Table (EM0)==================================================
     *    Index   KeySize	  KeyMask	     DefActionEnable	DefAction          Description
     *      1	     5B	     FF:FB:FF:E0:00	    False	           ∅                   Tag0
     * =================================================================================================
     * ================================EM Table 0:==================================================
     * ========= Key ===================================================================================
     *    PclId   SourcePortProfile   Tag0Found	   Tag0    Tag1Found	 Tag1
     *      0	        1	             Yes	    10	       No          ∅
     * ========= Action ================================================================================
     *    Src vPort   IntAssignmentEnable    eVlan	  PLRIndex        GCF
     *        51	            2	           101	     1	       GCF[12:1] = 1
     * =================================================================================================
     */

    /* Clear the fields */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));
    cpssOsMemSet((GT_VOID*) &keyParams.mask[0], 0, sizeof(keyParams.mask));
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));
    cpssOsMemSet((GT_VOID*) &exactMatchCrcMultiHashArr, 0, sizeof(exactMatchCrcMultiHashArr));
    numberOfElemInCrcMultiHashArr = 0;

    /* EM unit and Client is same for both Serial-EM1 and Serial-EM2 */
    emUnitNum                   = PRV_TGF_EXACT_MATCH_UNIT_0_E;
    lookupClientType            = PRV_TGF_EXACT_MATCH_CLIENT_TTI_SERIAL_EM_E;

    lookupNum                   = PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E;
    /* KeySize is 5B */
    keyParams.keySize           = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;

    /* Key Mask is 0xFFFBFFE000*/
    keyParams.mask[4]           = 0xFF;
    keyParams.mask[3]           = 0xFB;
    keyParams.mask[2]           = 0xFF;
    keyParams.mask[1]           = 0xE0;

    /* EM key configurations */
    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.lookupNum   = lookupNum;
    /* PCL ID           = 0 , bits 39...38
     * Src Port Profile = 1 , bits 37...28
     * Tag0 Found       = 1 , bit  27
     * Tag0             = 10, bits 14...25 */
    exactMatchEntry.key.pattern[3] = 0x18;
    exactMatchEntry.key.pattern[2] = 0x02;
    exactMatchEntry.key.pattern[1] = 0x80;

    profileId                   = PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS;
    expandedActionIndex         = PRV_TGF_SERIAL_EM1_PROFILE_ID_CNS;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    actionData.ttiAction.command                      = CPSS_PACKET_CMD_FORWARD_E;
    actionData.ttiAction.userDefinedCpuCode           = CPSS_NET_FIRST_USER_DEFINED_E;

    prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].profileId = profileId;

    /* actionData.ttiAction.interfaceAssignmentEnable    = 2;*/
    actionData.ttiAction.tag0VlanCmd                  = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    actionData.ttiAction.tag0VlanId                   = PRV_TGF_SERIAL_EM1_EGR_VLANID_CNS;

    /* Eport Action configs */
    actionData.ttiAction.sourceEPortAssignmentEnable      = GT_TRUE;
    actionData.ttiAction.sourceEPort                      = PRV_TGF_SERIAL_EM1_SRC_VPORT_CNS;
    actionData.ttiAction.redirectCommand                  = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    actionData.ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    actionData.ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    actionData.ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR2_PORT_IDX_CNS];
    actionData.ttiAction.ingressPipeBypass                = GT_FALSE;
    actionData.ttiAction.bridgeBypass                     = GT_TRUE;

    /* calculate index */
    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum, emUnitNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-1];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }
    prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].exactMatchEntryIndex = index;

    /* save config */
    /* keep clientType configured for first lookup , for restore */
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum, emUnitNum,
                                         lookupNum,
                                         &prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* Save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum, emUnitNum, profileId,
                                              &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* Save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,
                                            emUnitNum,
                                            lookupNum,
                                            portGroupsBmp,index,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].exactMatchEntryValid,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].exactMatchEntryActionType,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].exactMatchEntryAction,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].exactMatchEntry,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntryGet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM1_E,
                                                               &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM1_E].serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntryGet FAILED: %d", prvTgfDevNum);

    /* Set port profile ID */
    rc = prvTgfExactMatchSerialPortProfileIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialPortProfileIdSet FAILED: %d", prvTgfDevNum);

    /* Populate params for Serial-EM1 */
    serialEmProfileParams.serialEmProfileId             = profileId;
    serialEmProfileParams.serialEmKeyType               = PRV_TGF_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E;
    serialEmProfileParams.serialEmEnableUDB28VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmEnableUDB29VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmPclId                 = PRV_TGF_SERIAL_EM1_PCL_ID_CNS;
    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM1_E,
                                                               &serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet FAILED: %d", prvTgfDevNum);

    /* Set TTI Serial EM Client registration */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, emUnitNum,
                                         lookupNum, lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* Set the Generic Range Source Port Profile ID for specific port */
    rc = cpssDxChTtiGenericRangeSrcPortProfileIdSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    PRV_TGF_SRC_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTtiGenericRangeSrcPortProfileIdSet: %d", prvTgfDevNum);

    /* Set Expanded Action for Serial EM1 lookup, profileId=1, ActionType=TTI, packet Command = Forward */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, emUnitNum, profileId,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    /* Set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum, emUnitNum,
                                              profileId, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* Set Exact Match Entry */
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,
                                            emUnitNum,
                                            lookupNum,
                                            portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same as configured in Expanded Entry */
                                            &actionData);/* same as configured in Expanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    /*
     * ==============================Configurations for Serial EM2================
     * Serial EM port Profile ID = 0
     * Incoming packet = Ethernet(Type = 5)
     * =======================Serial EM1 Profile ID Table===============================================
     *    Index	  ProfileId	   KeyType	EnableUDB29  EnableUDB28   PCL-ID
     *     40	      2	          0	        0	          0	         0
     * =================================================================================================
     * ========================EMX Profile Table (EM0)==================================================
     *    Index   KeySize	  KeyMask	     DefActionEnable	DefAction          Description
     *      2	     5B	     FF:FB:FF:EF:FF	    True	        Set GCF[0]            Tag0, Tag1
     * =================================================================================================
     * ================================EM Table 1:==================================================
     * ========= Key ===================================================================================
     *    PclId   SourcePortProfile   Tag0Found	   Tag0    Tag1Found	 Tag1
     *      1	        1	             Yes	    10	       Yes        100
     * ========= Action ================================================================================
     *    Src vPort   IntAssignmentEnable    eVlan	  PLRIndex        GCF
     *        52	            2	           102	     2	       GCF[12:1] = 2
     * =================================================================================================
     */

    /* Clear the fields */
    cpssOsMemSet((GT_VOID*) &actionData, 0, sizeof(actionData));
    cpssOsMemSet((GT_VOID*) &expandedActionOrigin, 0, sizeof(expandedActionOrigin));
    cpssOsMemSet((GT_VOID*) &keyParams.mask[0], 0, sizeof(keyParams.mask));
    cpssOsMemSet((GT_VOID*) &exactMatchEntry.key.pattern[0], 0, sizeof(exactMatchEntry.key.pattern));
    cpssOsMemSet((GT_VOID*) &exactMatchCrcMultiHashArr, 0, sizeof(exactMatchCrcMultiHashArr));
    numberOfElemInCrcMultiHashArr = 0;

    /*  EM unit and ClientType is not changed */
    lookupNum                   = PRV_TGF_EXACT_MATCH_LOOKUP_SECOND_E;
    /* KeySize is 5B */
    keyParams.keySize           = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;

    /* Key Mask is 0xFFFBFFEFFF*/
    keyParams.mask[4]           = 0xFF;
    keyParams.mask[3]           = 0xFB;
    keyParams.mask[2]           = 0xFF;
    keyParams.mask[1]           = 0xEF;
    keyParams.mask[0]           = 0xFF;

    /* EM key configurations */
    exactMatchEntry.key.keySize = PRV_TGF_EXACT_MATCH_KEY_SIZE_5B_E;
    exactMatchEntry.lookupNum   = lookupNum;
    /* PCL ID           = 1 , bits 39...38
     * Src Port Profile = 1 , bits 37...28
     * Tag0 Found       = 1 , bit  27
     * Tag0             = 10, bits 25...14
     * Tag1 Found       = 1 , bit  13
     * Tag1             = 100, bits 11...0 */
    exactMatchEntry.key.pattern[4] = 0x40;
    exactMatchEntry.key.pattern[3] = 0x18;
    exactMatchEntry.key.pattern[2] = 0x02;
    exactMatchEntry.key.pattern[1] = 0xA0;
    exactMatchEntry.key.pattern[0] = 0x64;

    profileId                   = PRV_TGF_SERIAL_EM2_PROFILE_ID_CNS;
    expandedActionIndex         = PRV_TGF_SERIAL_EM2_PROFILE_ID_CNS;
    actionType                  = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    /* actionData.ttiAction.interfaceAssignmentEnable    = 2;*/
    actionData.ttiAction.command                      = CPSS_PACKET_CMD_FORWARD_E;
    actionData.ttiAction.userDefinedCpuCode           = CPSS_NET_FIRST_USER_DEFINED_E;
    actionData.ttiAction.tag0VlanCmd                  = PRV_TGF_TTI_VLAN_MODIFY_ALL_E;
    actionData.ttiAction.tag0VlanId                   = PRV_TGF_SERIAL_EM2_EGR_VLANID_CNS;

    /* Eport Action configs */
    actionData.ttiAction.sourceEPortAssignmentEnable      = GT_TRUE;
    actionData.ttiAction.sourceEPort                      = PRV_TGF_SERIAL_EM2_SRC_VPORT_CNS;
    actionData.ttiAction.redirectCommand                  = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    actionData.ttiAction.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    actionData.ttiAction.egressInterface.devPort.hwDevNum = prvTgfDevNum;
    actionData.ttiAction.egressInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_EGR3_PORT_IDX_CNS];
    actionData.ttiAction.ingressPipeBypass                = GT_FALSE;
    actionData.ttiAction.bridgeBypass                     = GT_TRUE;

    prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].profileId = profileId;

    /* calculate index */
    rc =  prvTgfExactMatchHashCrcMultiResultsByParamsCalc(prvTgfDevNum, emUnitNum,
                                                          &exactMatchEntry.key,
                                                          &numberOfElemInCrcMultiHashArr,
                                                          exactMatchCrcMultiHashArr);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchHashCrcMultiResultsByParamsCalc FAILED: %d", prvTgfDevNum);

    if (numberOfElemInCrcMultiHashArr!=0)
    {
        index = exactMatchCrcMultiHashArr[numberOfElemInCrcMultiHashArr-1];
    }
    else
    {
        rc = GT_BAD_VALUE;
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "numberOfElemInCrcMultiHashArr is 0  - illegal - FAILED");
    }
    prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].exactMatchEntryIndex = index;

    /* save config */
    /* keep clientType configured for first lookup , for restore */
    rc = prvTgfExactMatchClientLookupGet(prvTgfDevNum, emUnitNum,
                                         lookupNum,
                                         &prvTgfRestoreCfg.lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupGet FAILED: %d", prvTgfDevNum);

    /* Save Profile Key Params , for restore */
    rc =  prvTgfExactMatchProfileKeyParamsGet(prvTgfDevNum, emUnitNum, profileId,
                                              &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChExactMatchProfileKeyParamsGet FAILED: %d", prvTgfDevNum);

    /* Save Exact Match Entry , for restore */
    rc =  prvTgfExactMatchPortGroupEntryGet(prvTgfDevNum,
                                            emUnitNum,
                                            lookupNum,
                                            portGroupsBmp,index,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].exactMatchEntryValid,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].exactMatchEntryActionType,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].exactMatchEntryAction,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].exactMatchEntry,
                                            &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].exactMatchEntryExpandedActionIndex);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryGet FAILED: %d", prvTgfDevNum);

    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntryGet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM2_E,
                                                               &prvTgfRestoreCfg.serialEmParams[PRV_TGF_EXACT_MATCH_SERIAL_EM2_E].serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntryGet FAILED: %d", prvTgfDevNum);

    /* Set port profile ID */
    rc = prvTgfExactMatchSerialPortProfileIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialPortProfileIdSet FAILED: %d", prvTgfDevNum);

    /* Populate params for Serial-EM2 */
    serialEmProfileParams.serialEmProfileId             = profileId;
    serialEmProfileParams.serialEmKeyType               = PRV_TGF_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E;
    serialEmProfileParams.serialEmEnableUDB28VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmEnableUDB29VlanTagKey = GT_FALSE;
    serialEmProfileParams.serialEmPclId                 = PRV_TGF_SERIAL_EM2_PCL_ID_CNS;
    rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet(prvTgfDevNum,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                               PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                               PRV_TGF_EXACT_MATCH_SERIAL_EM2_E,
                                                               &serialEmProfileParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet FAILED: %d", prvTgfDevNum);

    /* Set TTI Serial EM Client registration */
    rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, emUnitNum,
                                         lookupNum, lookupClientType);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    /* Set the Generic Range Source Port Profile ID for specific port */
    rc = cpssDxChTtiGenericRangeSrcPortProfileIdSet(prvTgfDevNum,
                                                    prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS],
                                                    PRV_TGF_SRC_PORT_PROFILE_ID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChTtiGenericRangeSrcPortProfileIdSet: %d", prvTgfDevNum);

    /* Set Expanded Action for Serial-EM2 lookup, profileId=2, ActionType=TTI, packet Command = Forward */
    rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, emUnitNum, profileId,
                                            actionType,&actionData,&expandedActionOrigin);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);

    /* Set Profile Key Params */
    rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum, emUnitNum,
                                              profileId, &keyParams);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);

    /* Set Exact Match Entry */
    rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,
                                            emUnitNum,
                                            lookupNum,
                                            portGroupsBmp,index,
                                            expandedActionIndex,
                                            &exactMatchEntry,
                                            actionType,/* same as configured in Expanded Entry */
                                            &actionData);/* same as configured in Expanded Entry */
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);

    return;
}

/**
* @internal prvTgfSerialExactMatchKeyAndActionConfigRestore function
* @endinternal
*
* @brief Restore Serial Exact Match Configurations
*
*/
GT_VOID prvTgfSerialExactMatchKeyAndActionConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    PRV_TGF_EXACT_MATCH_LOOKUP_ENT          ttiLookupNum;
    PRV_TGF_EXACT_MATCH_ACTION_TYPE_ENT     actionType;
    PRV_TGF_EXACT_MATCH_ACTION_UNT          zeroAction;
    GT_PORT_GROUPS_BMP                      portGroupsBmp;
    PRV_TGF_EXACT_MATCH_EXPANDED_ACTION_ORIGIN_UNT  zeroActionOrigin;
    PRV_TGF_EXACT_MATCH_UNIT_NUM_ENT                emUnitNum;
    PRV_TGF_EXACT_MATCH_SERIAL_EM_TYPE_ENT          serialEm;

    cpssOsMemSet(&zeroAction, 0, sizeof(zeroAction));
    cpssOsMemSet(&zeroActionOrigin, 0, sizeof(zeroActionOrigin));

    /* Restore profileId configured for first lookup
       need to be restored profileId before restore of client type */
    portGroupsBmp   = CPSS_PORT_GROUP_UNAWARE_MODE_CNS;
    actionType      = PRV_TGF_EXACT_MATCH_ACTION_TTI_E;

    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchClientLookupSet FAILED: %d", prvTgfDevNum);

    for (serialEm = PRV_TGF_EXACT_MATCH_SERIAL_EM0_E; serialEm < PRV_TGF_EXACT_MATCH_SERIAL_EM_LAST_E; serialEm++)
    {
        emUnitNum    = (serialEm == PRV_TGF_EXACT_MATCH_SERIAL_EM0_E)?PRV_TGF_EXACT_MATCH_UNIT_REDUCED_E:PRV_TGF_EXACT_MATCH_UNIT_0_E;
        ttiLookupNum = (serialEm == PRV_TGF_EXACT_MATCH_SERIAL_EM1_E)?PRV_TGF_EXACT_MATCH_LOOKUP_FIRST_E:PRV_TGF_EXACT_MATCH_LOOKUP_SECOND_E;

        if (serialEm > PRV_TGF_EXACT_MATCH_SERIAL_EM0_E)
        {
            /* Restore first lookup client type */
            rc = prvTgfExactMatchClientLookupSet(prvTgfDevNum, emUnitNum,
                                                ttiLookupNum, prvTgfRestoreCfg.lookupClientType);
        }

        rc = prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet(prvTgfDevNum,
                                                                   PRV_TGF_EXACT_MATCH_SERIAL_EM_PACKET_TYPE_UDB_ETHERNET_OTHER_E,
                                                                   PRV_TGF_SERIAL_EM_PORT_PROFILE_ID_CNS,
                                                                   serialEm,
                                                                   &prvTgfRestoreCfg.serialEmParams[serialEm].serialEmProfileParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchSerialEmProfileIdMappingTableEntrySet FAILED: %d", prvTgfDevNum);

        /* Restore for Serial EM1*/
        if (prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryValid == GT_TRUE)
        {
            /* Restore Exact Match Entry */
            rc =  prvTgfExactMatchPortGroupEntrySet(prvTgfDevNum,
                                                    emUnitNum,
                                                    ttiLookupNum,
                                                    portGroupsBmp,
                                                    prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryIndex,
                                                    prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryExpandedActionIndex,
                                                    &prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntry,
                                                    prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryActionType,/* same as configured in Extpanded Entry */
                                                    &prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryAction);/* same a sconfigured in Extpanded Entry */
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntrySet FAILED: %d", prvTgfDevNum);
        }
        else
        {
            /* invalidate the entry */
            rc = prvTgfExactMatchPortGroupEntryInvalidate(prvTgfDevNum, emUnitNum, ttiLookupNum, portGroupsBmp,
                                                          prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryIndex);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchPortGroupEntryInvalidate FAILED: %d", prvTgfDevNum);
        }

        /* Restore Expanded Action for TTI lookup */
        rc = prvTgfExactMatchExpandedActionSet(prvTgfDevNum, emUnitNum,
                                               prvTgfRestoreCfg.serialEmParams[serialEm].exactMatchEntryExpandedActionIndex,
                                               actionType, &zeroAction,
                                               &zeroActionOrigin);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchExpandedActionSet FAILED: %d", prvTgfDevNum);


        /* Restore Profile Key Params */
        rc =  prvTgfExactMatchProfileKeyParamsSet(prvTgfDevNum, emUnitNum,
                                                  prvTgfRestoreCfg.serialEmParams[serialEm].profileId,
                                                  &prvTgfRestoreCfg.serialEmParams[serialEm].keyParams);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfExactMatchProfileKeyParamsSet FAILED: %d", prvTgfDevNum);
    }
    return;
}
