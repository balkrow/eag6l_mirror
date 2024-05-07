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
* @file prvTgfTm17Ports.c
*
* @brief Traffic Manager functional testing
*
* @version   1
********************************************************************************
*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE) /*not supported in CAP - UTs have appDemo references */

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfTypes.h>
#include <cpss/generic/networkIf/cpssGenNetIfTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/common/systemRecovery/cpssGenSystemRecovery.h>
#include <cpss/generic/tm/prvCpssTmCtl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>

#include <utf/utfMain.h>
#include <utf/private/prvUtfHelpers.h>
#include <utf/private/prvUtfExtras.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfTrunkGen.h>
#include <trunk/prvTgfTrunk.h>
#include <common/tgfPacketGen.h>
#include <common/tgfConfigGen.h>
#include <gtOs/gtOsMem.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>
#include <appDemo/sysHwConfig/gtAppDemoTmCsRefConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <tm/prvTgfTmHaBasic.h>
#include <systemRecovery/prvCpssTmCatchup.h>
#include <systemRecovery/prvCpssTmDbgUtils.h>
#include <tm/prvTgfTm17Ports.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_1_CNS           5

/* default VLAN Id */
#define PRV_TGF_SEND_VLANID_2_CNS           6

/* port index to send traffic to */
#define PRV_TGF_SEND_PORT_IDX_CNS         1

/* number of ports */
#define PRV_TGF_PORT_COUNT_CNS            4

/* port index to next hop traffic to */
#define PRV_TGF_NEXTHOP_PORT_IDX_CNS         3

/* default number of packets to send */
#define PRV_TGF_BURST_COUNT_DEFAULT_CNS     1

/* values taken from gtAppDemoTmCsRefConfig.c */
#define VTCAM_PCL_MNG_ID         1
#define VTCAM_ID                 1

/******************************* Test packet **********************************/

/* L2 part of packet */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}                /* saMac */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag1Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    1, 0, PRV_TGF_SEND_VLANID_1_CNS                     /* pri, cfi, VlanId */
};

/* VLAN_TAG part */
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag2Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    2, 0, PRV_TGF_SEND_VLANID_2_CNS                     /* pri, cfi, VlanId */
};

/* packet's ethertype */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart = {TGF_ETHERTYPE_0800_IPV4_TAG_CNS};

/* packet's IPv4 */
static TGF_PACKET_IPV4_STC prvTgfPacketIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x2A,               /* totalLen */
    0,                  /* id */
    0,                  /* flags */
    0,                  /* offset */
    0x40,               /* timeToLive */
    TGF_PROTOCOL_UDP_E, /* protocol */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS,             /* csum */
    { 0x20, 0x21, 0x22, 0x23},   /* srcAddr */
    { 0x30, 0x31, 0x32, 0x33}    /* dstAddr */
};

/* packet's UDP part */
static TGF_PACKET_UDP_STC prvTgfPacketUdpPart =
{
    8,                  /* src port */
    0,                  /* dst port */
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,  /* length */
    TGF_PACKET_AUTO_CALC_CHECKSUM_CNS /* csum */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* PARTS of packet */
static TGF_PACKET_PART_STC prvTgfPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},  /* type, partPtr */
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag1Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag2Part},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_IPV4_E,      &prvTgfPacketIpv4Part},
    {TGF_PACKET_PART_UDP_E,       &prvTgfPacketUdpPart},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};


typedef struct
{
    GT_U32  bNode;
    GT_U32  aNode;
    GT_U32  cvlan;
    GT_U32  svlan;
} PRV_TM_HA_ADD_DELETE_FLOWS_STC;

static PRV_TM_HA_ADD_DELETE_FLOWS_STC addDeleteFlows[5];

/********************************************************************************/

extern GT_STATUS appDemoTmScenarioModeSet
(
    IN  CPSS_TM_SCENARIO  mode
);

extern GT_STATUS setDRAMIntNumber
(
    GT_U32 intNum
);

#define RECREATION_MAX_SIZE 256

typedef struct{
    GT_BOOL                         vaild;
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode;
    CPSS_PORT_SPEED_ENT             speed;
    CPSS_PORT_FEC_MODE_ENT          fecMode;
    GT_BOOL                         isMacLoopback;

} RECREATION_INFO;

static GT_U32 tgfcommonHighAvailabilityNumOfDevices
(
        GT_VOID
)
{
    GT_U8  tempDevNum = 0xFF ,ii = 0 ;
    while (GT_OK == cpssPpCfgNextDevGet(tempDevNum, &tempDevNum))
        ii++;
    return ii;
}

/*allocation of recreationRequired[numberOfDevices][RECREATION_MAX_SIZE] */
#define TGF_ARRAY_ALLOCATION                                                                                                    \
    do {                                                                                                                        \
        GT_U32 ii ,numberOfDevices = 0 ;                                                                                        \
        numberOfDevices = tgfcommonHighAvailabilityNumOfDevices();                                                              \
        recreationRequired =(RECREATION_INFO**)cpssOsMalloc(sizeof(RECREATION_INFO*)*numberOfDevices);                          \
        if (recreationRequired){                                                                                                \
            for (ii = 0; ii < numberOfDevices; ii++){                                                                           \
                recreationRequired[ii] = (RECREATION_INFO*)cpssOsMalloc(sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);           \
                if (recreationRequired[ii]==NULL){                                                                              \
                    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_OUT_OF_CPU_MEM,"\nError: recreationRequired[ii] allocation failed"); \
                    goto exit_cleanly_lbl;                                                                                      \
                }                                                                                                               \
                cpssOsMemSet(recreationRequired[ii], 0,sizeof(RECREATION_INFO)*RECREATION_MAX_SIZE);                            \
            }                                                                                                                   \
        }                                                                                                                       \
    } while (0);

/*free  recreationRequired*/
#define FREE_TGF_ARRAY_ALLOCATION                                               \
    do{                                                                         \
        GT_U32 ii ,numberOfDevices = 0 ;                                        \
        numberOfDevices = tgfcommonHighAvailabilityNumOfDevices();              \
        for (ii=0; ii<numberOfDevices; ii++){                                   \
            if(recreationRequired[ii])  cpssOsFree(recreationRequired[ii]);     \
        }                                                                       \
        if(recreationRequired)  cpssOsFree(recreationRequired);                 \
    } while (0);

/**
* @internal prvTgfHighAvailabilityInitPhase function
* @endinternal
*
* @brief   save all devices ports ,skip cpu and sdma ports
*
*/
extern GT_STATUS prvTgfHighAvailabilityInitPhase
(
    RECREATION_INFO *recreationRequiredPtr[]
);

extern GT_STATUS appDemoHaEmulateSwCrash
(
    GT_VOID
);

extern GT_STATUS prvTgHighAvailabilityStatusSet
(
        GT_U32  stage,
        GT_BOOL parallelMode
);

extern GT_STATUS appDemoEventRequestDrvnModeInit
(
    IN GT_VOID
);

extern GT_VOID tgfBasicTrafficSanity_extern
(
    GT_VOID
);

/**
* @internal prvTgfHighAvailabilityReplayAllPort function
* @endinternal
*
* @brief   configure all device port ,skip cpu and sdma ports
*
*/
extern GT_STATUS prvTgfHighAvailabilityReplayAllPort
(
    RECREATION_INFO *recreationRequiredPtr[]
);

/**
 * @internal prvTgfTm17PortsVlanConfigSet function
 * @endinternal
 *
 * @brief Set vlan configuration.
 *
 * @retval GT_VOID
 */
GT_VOID prvTgfTm17PortsVlanConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U8     tagArray[] = {1, 0, 0, 0};

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_1_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);

    /* AUTODOC: create VLAN 5 with tagged ports 0,1 */
    rc = prvTgfBrgDefVlanEntryWithPortsSet(PRV_TGF_SEND_VLANID_2_CNS, prvTgfPortsArray,
                                           NULL, tagArray, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWithPortsSet: %d", prvTgfDevNum);
}

GT_VOID prvTgfTm17PortsInitSystem
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      testPorts = 4;
    GT_U32      i;
    GT_U32      bNodeInd = 1;
    GT_U32      aNodeInd = 8;

    rc = appDemoTmNumOfTmPortSet(testPorts);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmNumOfTmPortSet");

    for (i = 0; i < testPorts; i++)
    {
        rc = appDemoTmPortIndexSet(i, prvTgfPortsArray[i]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmPortIndexSet");
    }

    rc = appDemoTmPrintOutEnable(GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "setDRAMIntNumber");

    for (i = 0; i < testPorts; i++)
    {
        rc = prvTgfPclDefPortInit(
                prvTgfPortsArray[i],
                CPSS_PCL_DIRECTION_EGRESS_E,
                CPSS_PCL_LOOKUP_0_E,
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*nonIpKey*/,
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E /*ipv4Key*/,
                PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E /*ipv6Key*/);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "prvTgfPclDefPortInit: %d", prvTgfDevNum);
    }

    rc = appDemoTmScenario17PortInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmScenarioModeSet");

    rc = appDemoTmScenario17PortConfigSet(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmScenario17PortConfigSet");

    bNodeInd = 1;
    aNodeInd = 8;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");
}

GT_STATUS prvTgfTm17PortsCncCounterCheck
(
    GT_U8 devNum,
    GT_U32 *hitIndexPtr,
    GT_U32 *hitNumPtr
)
{
    CPSS_DXCH_CNC_COUNTER_STC counter;
    GT_U32 totalEPCLCounter = 0;
    GT_STATUS rc;
    GT_U16  ii;
    GT_U16  block;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(0) == GT_TRUE)
    {
        for (block=0; block < 18 ; block++)
        {
            for(ii=0; ii< 1024;ii++)
            {
                rc = cpssDxChCncCounterGet(devNum, block, ii, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if( GT_OK != rc)
                   return rc;

                if(counter.packetCount.l[0] != 0)
                {
                    totalEPCLCounter = counter.packetCount.l[0] + totalEPCLCounter;
                    *hitIndexPtr = ii + block*1024;
                    *hitNumPtr   = counter.packetCount.l[0];
                    return GT_OK;
                }

            }
        }

    }

    return GT_OK;
}

GT_STATUS prvTgfTm17PortsCncCounterClear
(
    GT_U8 devNum
)
{
    CPSS_DXCH_CNC_COUNTER_STC counter;
    GT_STATUS rc;
    GT_U16  ii;
    GT_U16  block;

    if(PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(0) == GT_TRUE)
    {
        for (block=0; block < 18 ; block++)
        {
            for(ii=0; ii< 1024;ii++)
            {
                rc = cpssDxChCncCounterGet(devNum, block, ii, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
                if( GT_OK != rc)
                {
                   return rc;
                }
            }
        }
    }

    return GT_OK;
}

GT_VOID prvTgfTm17PortsTrafficGenerate
(
    IN GT_U32   sendPortIdx,
    IN GT_U32   burstCount
)
{
    GT_STATUS                       rc             = GT_OK;
    GT_U32                          sendPortNum    = prvTgfPortsArray[PRV_TGF_SEND_PORT_IDX_CNS];
    GT_U32                          partsCount     = 0;
    GT_U32                          packetSize     = 0;
    TGF_PACKET_PART_STC             *packetPartsPtr;
    TGF_PACKET_STC                  packetInfo;
    TGF_VFD_INFO_STC                vfdArray1[2];

    GT_U32                          hitIndex = 0, hitNum = 0;

    if (sendPortIdx < 4)
    {
        sendPortNum = prvTgfPortsArray[sendPortIdx];
    }

    /* -------------------------------------------------------------------------
     * 1. Setup counters and enable capturing
     */

    /* reset ETH counters */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfEthCountersReset");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");


    /* -------------------------------------------------------------------------
     * 2. Generating Traffic
     */

    partsCount = sizeof(prvTgfPacketPartArray) / sizeof(prvTgfPacketPartArray[0]);
    packetPartsPtr = prvTgfPacketPartArray;

    /* calculate packet size */
    rc = prvTgfPacketSizeGet(packetPartsPtr, partsCount, &packetSize);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPacketSizeGet:");

    /* build packet */
    packetInfo.totalLen   = packetSize;
    packetInfo.numOfParts = partsCount;
    packetInfo.partsArray = packetPartsPtr;

    cpssOsMemSet(vfdArray1, 0, sizeof(vfdArray1));
    vfdArray1[0].mode = TGF_VFD_MODE_INCREMENT_E;
    vfdArray1[0].modeExtraInfo = 0;
    vfdArray1[0].offset = 33;
    vfdArray1[0].cycleCount = 1;
    vfdArray1[0].incValue = 1;

    rc = prvTgfTm17PortsCncCounterClear(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTm17PortsCncCounterClear failed");

    /* setup packet */
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, burstCount, 1, vfdArray1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of SetTxSetup: %d", prvTgfDevNum);

    /* set transmit timings */
    rc = prvTgfSetTxSetup2Eth(1, 1);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* send packet -- send from specific port -- even if member of a trunk */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, sendPortNum);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR of StartTransmitting: %d, %d\n",
                                 prvTgfDevNum, sendPortNum);

    cpssOsTimerWkAfter(100);
#if 0
    rc = appDemoTcam2CncBlockGet(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of appDemoTcam2CncBlockGet: %d", prvTgfDevNum);
#endif
    rc = prvTgfTm17PortsCncCounterCheck(prvTgfDevNum, &hitIndex, &hitNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTm17PortsCncCounterCheck failed");
    UTF_VERIFY_EQUAL0_STRING_MAC(1, hitNum, "TCAM hit failed");
    PRV_UTF_LOG2_MAC("hitIndex = %d, hitNum = %d\n", hitIndex, hitNum);

    /* restore transmit timings */
    rc = prvTgfSetTxSetup2Eth(0, 0);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetup2Eth: %d", prvTgfDevNum);

    /* flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgFdbFlush");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

}

GT_VOID prvTgfTm17PortsConfigRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      aNodeInd = 8;
    GT_U32      bNodeInd = 1;

    rc = appDemoTmFlowRemove(prvTgfDevNum, PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS, bNodeInd, aNodeInd,
                          prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");

    rc = appDemoTmScenario17PortSchedulerDelete(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmScenario17PortSchedulerDelete failed");

    /* AUTODOC: invalidate vlan entries 5,6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SEND_VLANID_1_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_1_CNS);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_1_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate vlan entries 5,6 */
    rc = prvTgfBrgVlanEntryInvalidate(PRV_TGF_SEND_VLANID_2_CNS);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanEntryInvalidate: %d, %d",
                                 prvTgfDevNum, PRV_TGF_SEND_VLANID_1_CNS);

    /* AUTODOC: disable VLAN based MAC learning */
    rc = prvTgfBrgVlanLearningStateSet(PRV_TGF_SEND_VLANID_2_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanLearningStateSet: %d", prvTgfDevNum);

    rc = appDemoTmScenario17PortVTcamDelete(prvTgfDevNum);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "appDemoTmScenario17PortVTcamDelete: %d", prvTgfDevNum);

    /* restore default VLAN 1 */
    prvTgfBrgVlanEntryRestore(1);

    prvTgfPclRestore();

    /* Delete TM SW database */
    rc = cpssTmClose(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");

    /* Re-initialize TM sw database */
    rc = cpssTmInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");

    rc = appDemoTmDbReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmDbReset");

}

GT_VOID prvTgfTmHa17PortsReplay(GT_VOID)
{
    GT_STATUS rc = GT_OK;
    GT_U32      bNodeInd = 1;
    GT_U32      aNodeInd = 8;

    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");
}

/**
 * @brief
 *  1, Perform Play which does add and delete of flows. This
 *  involved adding and deleting of scheduler nodes
 *  2. Perform HA
 *  3  Replay the entires. During replay only play existing
 *  entries and ignore deleted entries
 *  4. Check validity
 */
GT_VOID prvTgfTmHa17Ports
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    CPSS_SYSTEM_RECOVERY_INFO_STC  systemRecoveryInfo;

    /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Pre HA");


    /* Change state to HA mode */
    cpssOsMemSet(&systemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* Delete TM SW database */
    rc = cpssTmClose(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");

    rc = appDemoTmDbReset(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmDbReset");

    /* Re-initialize TM sw database */
    rc = cpssTmInit(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");

    rc = appDemoTmScenario17PortInitTmAndPcl(prvTgfDevNum, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmScenario17PortInitTmAndPcl");

        /* Check sync up before catchup */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Before Catchup");

    /* Perform catchup to sync with hardware */
    rc = prvCpssTmCatchUp(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmCatchUp");

    /* Replay of additional settings */
    prvTgfTmHa17PortsReplay();

    /* Return to normal mode */
    systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc = cpssSystemRecoveryStateSet(&systemRecoveryInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssSystemRecoveryStateSet\n");

    /* check syncup after HA process is complete */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: After HA");
}

/**
 * Operation STag, CTag, Bnode, Anode
 * Add:         5,  6+1,     1,     9
 * Add:         5,  6+2,     1,    10
 * Add:         5,  6+3,     1,    11
 * Add:       5+1,    6,     2,    16
 * Add:       5+1,  6+1,     2,    17
 * Add:       5+1,  6+2,     2,    18
 */
GT_VOID prvTgfTmHa17PortsFlowsAddOrderChangePlay
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      bNodeInd, aNodeInd;

    bNodeInd = 1;
    aNodeInd = 9;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS+1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 10;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 11;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 3,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 16;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 17;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 18;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");
}

/**
 * Operation STag, CTag, Bnode, Anode
 * Add:         5,  6+1,     1,     9
 * Add:       5+1,  6+2,     2,    18
 * Add:         5,  6+3,     1,    11
 * Add:       5+1,  6+1,     2,    17
 * Add:         5,  6+2,     1,    10
 * Add:       5+1,    6,     2,    16
 */
GT_VOID prvTgfTmHa17PortsFlowsAddOrderChangeReplay
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      bNodeInd, aNodeInd;

    bNodeInd = 1;
    aNodeInd = 9;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");


    bNodeInd = 2;
    aNodeInd = 18;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 11;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 3,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 17;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 10;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 16;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

}

GT_VOID prvTgfTmHa17PortsFlowsAddOrderChangeRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      bNodeInd, aNodeInd;

    bNodeInd = 1;
    aNodeInd = 9;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");


    bNodeInd = 2;
    aNodeInd = 18;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");

    bNodeInd = 1;
    aNodeInd = 11;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 3,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");

    bNodeInd = 2;
    aNodeInd = 17;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");

    bNodeInd = 1;
    aNodeInd = 10;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");

    bNodeInd = 2;
    aNodeInd = 16;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed");

}

/**
 * Operation STag, CTag, Bnode, Anode
 * Add:         5,  6+1,     1,     9
 * Add:         5,  6+2,     1,    10
 * Add:         5,  6+3,     1,    11
 * Add:       5+1,    6,     2,    16
 * Add:       5+1,  6+1,     2,    17
 * Add:       5+1,  6+2,     2,    18
 * Delete:      5,  6+1,     1,     9
 * Delete:      5,  6+2,     1,    10
 * Delete:    5+1,    6,     2,    16
 * Delete:    5+1,  6+1,     2,    17
 * Add:         5,  6+2,     1,    10 (12) -> created anode at 12
 * Add:       5+1,  6+1,     2,    17 (19) -> created anode at 19
 */
GT_VOID prvTgfTmHa17PortsFlowsAddDeletePlay
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      bNodeInd, aNodeInd;

    bNodeInd = 1;
    aNodeInd = 9;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 10;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 11;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 3,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    addDeleteFlows[2].bNode = bNodeInd;
    addDeleteFlows[2].aNode = aNodeInd;
    addDeleteFlows[2].cvlan = PRV_TGF_SEND_VLANID_1_CNS;
    addDeleteFlows[2].svlan = PRV_TGF_SEND_VLANID_2_CNS + 3;

    bNodeInd = 2;
    aNodeInd = 16;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 17;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 18;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    addDeleteFlows[3].bNode = bNodeInd;
    addDeleteFlows[3].aNode = aNodeInd;
    addDeleteFlows[3].cvlan = PRV_TGF_SEND_VLANID_1_CNS + 1;
    addDeleteFlows[3].svlan = PRV_TGF_SEND_VLANID_2_CNS + 2;

    /* Delete Entries */
    bNodeInd = 1;
    aNodeInd = 9;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 10;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 16;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 17;
    rc = appDemoTmFlowRemove(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    /* Add entires */
    bNodeInd = 1;
    aNodeInd = 10;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    addDeleteFlows[0].bNode = bNodeInd;
    addDeleteFlows[0].aNode = aNodeInd;
    addDeleteFlows[0].cvlan = PRV_TGF_SEND_VLANID_1_CNS;
    addDeleteFlows[0].svlan = PRV_TGF_SEND_VLANID_2_CNS + 2;

    bNodeInd = 2;
    aNodeInd = 17;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    addDeleteFlows[1].bNode = bNodeInd;
    addDeleteFlows[1].aNode = aNodeInd;
    addDeleteFlows[1].cvlan = PRV_TGF_SEND_VLANID_1_CNS + 1;
    addDeleteFlows[1].svlan = PRV_TGF_SEND_VLANID_2_CNS + 1;
}

/**
 * Operation STag, CTag, Bnode, Anode
 * Add:         5,  6+3,     1,    11
 * Add:       5+1,  6+2,     2,    18
 * Add:         5,  6+2,     1,    12
 * Add:       5+1,  6+1,     2,    19
 */
GT_VOID prvTgfTmHa17PortsFlowsAddDeleteReplay
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      bNodeInd, aNodeInd;

    bNodeInd = 1;
    aNodeInd = 11;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 3,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 18;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 1;
    aNodeInd = 12;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS,
                          PRV_TGF_SEND_VLANID_2_CNS + 2,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");

    bNodeInd = 2;
    aNodeInd = 19;
    rc = appDemoTmFlowAdd(prvTgfDevNum,
                          PRV_TGF_SEND_VLANID_1_CNS + 1,
                          PRV_TGF_SEND_VLANID_2_CNS + 1,
                          bNodeInd, &aNodeInd, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmFlowAdd failed");
}

GT_VOID prvTgfTmHa17PortsFlowsAddDeleteRestore
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      vTcamPclMngId   = VTCAM_PCL_MNG_ID      ;
    GT_U32      vTcamId         = VTCAM_ID              ;

    for (i = 0; i<4; i++)
    {
        rc = appDemoTmFlowRemove(prvTgfDevNum,
                              addDeleteFlows[i].cvlan, addDeleteFlows[i].svlan,
                              addDeleteFlows[i].bNode, addDeleteFlows[i].aNode, prvTgfPortsArray[0]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "appDemoTmFlowRemove failed bNode:%d, aNode:%d",
                                     addDeleteFlows[i].bNode, addDeleteFlows[i].aNode);
    }

    for (i = 0; i < 8; i++)
    {
        /* deleting rule which is missed due to rule index mangaer */
        rc = cpssDxChVirtualTcamRuleDelete(vTcamPclMngId, vTcamId, 80+i);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChVirtualTcamRuleDelete failed");
    }
}

static GT_VOID prvTgfTmHa17PortsFlowsAddTestPlay
(
    PRV_TGF_TM_HA_CHECK_SCENARIO_ENT testScenario
)
{
    switch (testScenario)
    {
    case PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_ORDER_CHANGE_E:
        prvTgfTmHa17PortsFlowsAddOrderChangePlay();
        break;
    case PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_DELETE_E:
        prvTgfTmHa17PortsFlowsAddDeletePlay();
        break;
    default:
        break;
    }
}

static GT_VOID prvTgfTmHa17PortsFlowsAddTestReplay
(
    PRV_TGF_TM_HA_CHECK_SCENARIO_ENT testScenario
)
{
    switch (testScenario)
    {
    case PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_ORDER_CHANGE_E:
        prvTgfTmHa17PortsFlowsAddOrderChangeReplay();
        break;
    case PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_DELETE_E:
        prvTgfTmHa17PortsFlowsAddDeleteReplay();
        break;
    default:
        break;
    }
}

GT_VOID prvTgfTmHa17PortsFlowsAddTestRestore
(
    PRV_TGF_TM_HA_CHECK_SCENARIO_ENT testScenario
)
{
    switch (testScenario)
    {
    case PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_ORDER_CHANGE_E:
        prvTgfTmHa17PortsFlowsAddOrderChangeRestore();
        break;
    case PRV_TGF_TM_HA_CHECK_SCENARIO_ADD_DELETE_E:
        prvTgfTmHa17PortsFlowsAddDeleteRestore();
        break;
    default:
        break;
    }
}


/**
 * @brief
 *  1, Perform Play which does add and delete of flows. This
 *  involved adding and deleting of scheduler nodes
 *  2. Perform HA
 *  3  Replay the entires. During replay only play existing
 *  entries and ignore deleted entries
 *  4. Check validity
 */
GT_VOID prvTgfTmHa17PortsFlowsAddTest
(
    IN PRV_TGF_TM_HA_CHECK_SCENARIO_ENT testScenario,
    IN GT_BOOL       emulateCrash
)
{
    GT_STATUS rc = GT_OK;
    GT_BOOL                         portMng;
    RECREATION_INFO                 **recreationRequired=NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC   systemRecoveryInfo;

    prvTgfTmHa17PortsFlowsAddTestPlay(testScenario);

    rc = prvCpssTmDumpPort(prvTgfDevNum, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDumpPort: Pre HA");

    PRV_UTF_LOG0_MAC("====== HA Init State ==========\n");

    if (emulateCrash == GT_FALSE)
    {
        /* Change state to HA Init state */
        cpssOsMemSet(&systemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
        systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&systemRecoveryInfo);

        /* Delete TM SW database */
        rc = cpssTmClose(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmClose");

        rc = appDemoTmDbReset(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmDbReset");

        /* Re-initialize TM sw database */
        rc = cpssTmInit(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssTmInit");

        rc = appDemoTmScenario17PortInitTmAndPcl(prvTgfDevNum, GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmScenario17PortInitTmAndPcl");
    }
    else
    {
        rc = cpssDxChPortManagerEnableGet(prvTgfDevNum,&portMng);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : cpssPxPortManagerEnableGet FAILED:rc-%d",rc);
        if (portMng == GT_FALSE)
        {
            PRV_UTF_LOG0_MAC("tgfHighAvailabilitySimpleTest only support port manager ");
            SKIP_TEST_MAC
        }

        TGF_ARRAY_ALLOCATION

        rc = prvTgfHighAvailabilityInitPhase(recreationRequired);

        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR : prvTgfHighAvailabilityInitPhase FAILED:rc-%d devNum %d",rc,prvTgfDevNum);

        rc = appDemoHaEmulateSwCrash();
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "ERROR : appDemoHaEmulateSwCrash FAILED:rc-%d devNum %d",rc,prvTgfDevNum);

        rc = prvTgHighAvailabilityStatusSet(1,GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : prvTgHighAvailabilityStatusSet :rc-%d ",rc);

        PRV_UTF_LOG0_MAC("\nInit with port configuration \n");

        rc = cpssReInitSystem();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : cpssReInitSystem FAILED:rc-%d ", rc);
        prvUtfSetAfterSystemResetState();

        rc = appDemoTmScenario17PortInitTmAndPcl(prvTgfDevNum, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "appDemoTmScenario17PortInitTmAndPcl");
    }

    PRV_UTF_LOG0_MAC("====== HA Catchup State ==========\n");

    if (emulateCrash == GT_FALSE)
    {
        /* Change state to HA Init state */
        cpssOsMemSet(&systemRecoveryInfo, 0, sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
        systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&systemRecoveryInfo);

        /* Perform catchup to sync with hardware */
        rc = prvCpssTmCatchUp(prvTgfDevNum);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmCatchUp");
    }
    else
    {
        rc = prvTgHighAvailabilityStatusSet(2,GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : prvTgHighAvailabilityStatusSet :rc-%d ",rc);

        rc = prvTgfHighAvailabilityReplayAllPort(recreationRequired);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : tfgHighAvailabilityReplayAllPort :rc-%d ", rc);

    }

    /* Check sync up before replay */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: Before Replay");

    /* Replay of additional settings */
    prvTgfTmHa17PortsReplay();

    /* Replay of additional settings */
    prvTgfTmHa17PortsFlowsAddTestReplay(testScenario);

    PRV_UTF_LOG0_MAC("====== HA Completion State ==========\n");

    if (emulateCrash == GT_FALSE)
    {
        /* Return to normal mode */
        systemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
        systemRecoveryInfo.systemRecoveryState   = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        prvCpssSystemRecoveryStateUpdate(&systemRecoveryInfo);
    }
    else
    {
        rc = prvTgHighAvailabilityStatusSet(3,GT_FALSE);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : prvTgHighAvailabilityStatusSet :rc-%d ",rc);

        PRV_UTF_LOG0_MAC("\nStart event handlers.\n");
        rc = appDemoEventRequestDrvnModeInit();
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : appDemoEventRequestDrvnModeInit :rc-%d ",rc);

        rc = cpssDxChNetIfRestore(prvTgfDevNum);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "ERROR : cpssDxChNetIfRestore :rc-%d ",rc);
        PRV_UTF_LOG0_MAC("\n flush FDB ");
        /* flush FDB include static entries */
        prvTgfBrgFdbFlush(GT_TRUE);

        /*run test to verify system restore */
        tgfBasicTrafficSanity_extern();

        PRV_UTF_LOG0_MAC("\n flush FDB ");
        /* flush FDB include static entries */
        prvTgfBrgFdbFlush(GT_TRUE);

    }

    rc = prvCpssTmDumpPort(prvTgfDevNum, prvTgfPortsArray[0]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmDumpPort: Post HA");

    /* check syncup after HA process is complete */
    rc = prvCpssTmSyncCatchupValidityCheck(prvTgfDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvCpssTmSyncCatchupValidityCheck: After HA");

    prvTgfTmHa17PortsFlowsAddTestRestore(testScenario);

exit_cleanly_lbl:
    if (emulateCrash == GT_TRUE)
    {
        FREE_TGF_ARRAY_ALLOCATION
    }

}

#endif
