/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtAppDemoTmUtils.c
*
* @brief Basic TM configurations.
*
*/

#include <appDemo/os/appOs.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBobcat2PhyConfig.h>

/* get the device info and common info */
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/config/cpssDxChCfgInit.h>

#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/generic/bridge/cpssGenBrgVlanTypes.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>


#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>

#include <cpss/common/port/cpssPortStat.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortStat.h>

#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/protection/cpssDxChProtection.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>

#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDram.h>
/*#include <cpss/dxCh/dxChxGen/tmGlue/private/prvCpssDxChTmGlue.h>*/
#include <cpss/generic/tm/cpssTmPublicDefs.h>
#include <cpss/generic/tm/cpssTmCtl.h>
#include <cpss/generic/tm/cpssTmSched.h>
#include <cpss/generic/tm/cpssTmDrop.h>
#include <cpss/generic/tm/cpssTmNodesCreate.h>
#include <cpss/generic/tm/cpssTmNodesCtl.h>
#include <cpss/generic/tm/cpssTmShaping.h>
#include <cpss/generic/tm/cpssTmNodesUpdate.h>
#include <cpss/generic/tm/cpssTmNodesRead.h>
#include <cpss/generic/tm/cpssTmNodesStatus.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortPfc.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGluePfc.h>

#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueAgingAndDelay.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueQueueMap.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueDrop.h>
#include <cpss/dxCh/dxChxGen/tmGlue/cpssDxChTmGlueFlowControl.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>

#include <cpss/dxCh/dxChxGen/private/routerTunnelTermTcam/prvCpssDxChRouterTunnelTermTcam.h>

#include <mvDdr3TrainingIp.h>
#include <mvDdr3TrainingIpBist.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/generic/tm/prvCpssTmCtl.h>
#include <cpss/generic/tm/prvCpssTmDrop.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define TM_QUEUE_POOL_SIZE 16384

extern GT_U32 appDemoDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
);


static char *tmTreelevelNamesArr[] =
{
    "Q_LEVEL",
    "A_LEVEL",
    "B_LEVEL",
    "C_LEVEL",
    "P_LEVEL"
};

static char *tmDropModeNamesArr[] =
{
    "CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E",
    "CPSS_TM_DROP_MODE_CB_TD_CA_TD_E",
    "CPSS_TM_DROP_MODE_CB_TD_ONLY_E"
};


/* packet CRC used for comaprison in the BIST test */
static GT_U8        CRC_arr[4] = {0xE8, 0x1F, 0x87, 0xA1};
/* TTI client CRC indexes, used in BIST test */
static const GT_U32 ttiCpuPortCounterIndex = 10240;
static const GT_U32 ttiCrcMatchCounterIndex = 10280;
static const GT_U32 ttiCrcNoMatchCounterIndex = 10288;
/* the first awailable TTI rule - for BIST test */
static GT_U32       firstTtiRule = 0;
/* the index of the Q-node on which BIST test is based */
static GT_U32       queueNum = 0;


/*
shell-execute appDemoTmTCAMCountersSet 0
*/
GT_STATUS appDemoTmTCAMCountersSet
(
    GT_U8 devNum
)
{
    GT_STATUS   rc=GT_OK;
    GT_U64      indexRangesBmp;
    GT_U32      i;

    cpssOsPrintf("==================> appDemoTmTCAMCountersSet is called <============\n");

    rc = cpssDxChCncCountingEnableSet(devNum, CPSS_DXCH_CNC_COUNTING_ENABLE_UNIT_TTI_E, GT_TRUE);
    if( GT_OK != rc)
    {
        cpssOsPrintf("cpssDxChCncCountingEnableSet failed for TTI rc = %d\n",rc);
        return rc;
    }

    /* each counter block contains 1024 counters; there are 32 blocks */

    for (i=10; i < 12 ; i++)
    {
        rc = cpssDxChCncBlockClientEnableSet(devNum, i, CPSS_DXCH_CNC_CLIENT_TTI_E, GT_TRUE);
        if( GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChCncBlockClientEnableSet[%d] failed rc = %d\n",i,rc);
            return rc;
        }
        indexRangesBmp.l[0] = 1 << i;
        indexRangesBmp.l[1] = 0;
        rc = cpssDxChCncBlockClientRangesSet(devNum,i,
                                             CPSS_DXCH_CNC_CLIENT_TTI_E,
                                             &indexRangesBmp);
        if( GT_OK != rc)
        {
            cpssOsPrintf("cpssDxChCncBlockClientRangesSet[%d] failed rc = %d\n",i,rc);
            return rc;
        }
    }
    return rc;
}


/* verify with which indexes nodes are created */
GT_STATUS appDemoTmNodeDeleteCreate(GT_U8 devNum, GT_U32 index, GT_U32 aNodeInd /* should be below 10*/)
{
    GT_STATUS                rc;
    GT_U32                   queueId;
    CPSS_TM_QUEUE_PARAMS_STC queueParams;
    GT_U8                    dev = SYSTEM_DEV_NUM_MAC(devNum);

    queueParams.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
    queueParams.quantum = CPSS_TM_NODE_QUANTUM_UNIT_CNS; /* in CPSS_TM_NODE_QUANTUM_UNIT_CNS */
    queueParams.dropProfileInd = 1;
    queueParams.eligiblePrioFuncId = CPSS_TM_ELIG_Q_PRIO0_E;


    rc = cpssTmNodesCtlNodeDelete(dev, CPSS_TM_LEVEL_Q_E, index);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = cpssTmQueueToAnodeCreate(dev, aNodeInd, &queueParams, &queueId);
    osPrintf("queueId = %d ",queueId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssTmQueueToAnodeCreate", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;

}

/* get TM aging and CNC TM counters*/
GT_STATUS appDemoTmQCountersGet
(
    IN GT_U8   devNum,
    IN GT_BOOL includeBytes
)
{
    GT_STATUS rc;
    GT_U32 i,j,queueId;
    CPSS_DXCH_TM_AGING_AND_DELAY_STATISTICS_STC agingCounters;
    CPSS_DXCH_CNC_COUNTER_STC         cncCounter;
    GT_BOOL Enable;
    GT_U32      counterBlocks;

/* each queue assigned two counters - one for pass, one for drop.  Ther are 16K Q-nodes in BobCat2 and 4K in BobK */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        counterBlocks = 8;
    else
        counterBlocks = 32;

    osPrintf("AgingAndDelay counters:\n");
    for (i=0; i<100; i++)
    {
        rc = cpssDxChTmGlueAgingAndDelayCountersGet(devNum, i, &agingCounters);
        if (rc != GT_OK)
            return rc;
        if(agingCounters.packetCounter > 0)
        {
            rc = cpssDxChTmGlueAgingAndDelayCounterQueueIdGet(devNum, i, &queueId);
            if (rc != GT_OK)
                return rc;
            osPrintf("counter %d: %d packets passed through queue %d on TM exit.  Letency min %d, max %d\n",i,
                     agingCounters.packetCounter, queueId, agingCounters.minLatencyTime, agingCounters.maxLatencyTime);
        }
    }
    osPrintf("TM CNC counters:\n");
    for (i=0; i<counterBlocks; i++)
    {
        rc = cpssDxChCncBlockClientEnableGet(devNum, i, CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E, &Enable);
        if( GT_OK != rc)
           return rc;

        if (Enable == GT_FALSE)
        {
            continue;
        }

        for (j=0; j<1024; j++)
        {
            rc = cpssDxChCncCounterGet(devNum,i,j,CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E,&cncCounter);
            if (rc != GT_OK)
                return rc;
            if( (cncCounter.packetCount.l[1]+cncCounter.packetCount.l[0]) > 0)
            {
                if( (j%2) == 0)
                    osPrintf("0x%X%X \t ( %8d )packets entered TM through queue %d\n",cncCounter.packetCount.l[1], cncCounter.packetCount.l[0],
                             (cncCounter.packetCount.l[1]*0xFFFFFFFF + cncCounter.packetCount.l[0]), i*512+j/2);
                else
                    osPrintf("0x%X%X \t ( %8d )packets dropped on queue %d\n",cncCounter.packetCount.l[1], cncCounter.packetCount.l[0],
                             (cncCounter.packetCount.l[1]*0xFFFFFFFF + cncCounter.packetCount.l[0]), i*512+j/2);
                if(includeBytes == GT_TRUE)
                {
                    if( (j%2) == 0)
                        osPrintf("0x%X%X ( %d )bytes entered TM through queue %d\n",cncCounter.byteCount.l[1], cncCounter.byteCount.l[0],
                                 (cncCounter.byteCount.l[1]*0xFFFFFFFF + cncCounter.byteCount.l[0]), i*512+j/2);
                    else
                        osPrintf("0x%X%X ( %d )bytes dropped on queue %d\n",cncCounter.byteCount.l[1], cncCounter.byteCount.l[0],
                                 (cncCounter.byteCount.l[1]*0xFFFFFFFF + cncCounter.byteCount.l[0]), i*512+j/2);
                }
            }

        }
    }
    return GT_OK;
}

/* get Summary of TM Q length counters */
GT_STATUS appDemoTmQLenCountersSumGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 from_q,
    IN  GT_U32 to_q
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 totalUse[2] = {0,0},
           qNodeIndex,
           qLength[2] = {0,0},
           maxQLenght[2] = {0,0},
           maxIndex[2] = {0,0};

    if (to_q == 0)
    {
        to_q = TM_QUEUE_POOL_SIZE - 1;
    }
    else
    {
        to_q = MIN(to_q, TM_QUEUE_POOL_SIZE - 1);
    }


    for(qNodeIndex = from_q;  qNodeIndex <= to_q; qNodeIndex++)
    {
        rc = cpssTmDropQueueLengthGet(devNum,
                                      CPSS_TM_LEVEL_Q_E,
                                      qNodeIndex,
                                      &(qLength[0]),
                                      &(qLength[1]));
        if(rc!=GT_OK)
            return rc;

        totalUse[0] += qLength[0];
        totalUse[1] += qLength[1];
        if(maxQLenght[0] < qLength[0])
        {
            maxQLenght[0]   = qLength[0];
            maxIndex[0]     = qNodeIndex;
        }
        if(maxQLenght[1] < qLength[1])
        {
            maxQLenght[1]   = qLength[1];
            maxIndex[1]     = qNodeIndex;
        }

    }

    osPrintf("Total queues length is %d bytes, max queue length is %d on queue %d\n",
             totalUse[0]*16,
             maxQLenght[0]*16,
             maxIndex[0]);

    osPrintf("Total average queues length is %d bytes, max queue average length is %d on queue %d\n",
             totalUse[1]*16,
             maxQLenght[1]*16,
             maxIndex[1]);

    return GT_OK;
}



/* get Summary of TM drop counters*/
GT_STATUS appDemoTmQCountersSumGet
(
    IN GT_U8   devNum,
    IN GT_BOOL includeBytes
)
{
    GT_STATUS rc;
    GT_U32 i,j;
    CPSS_DXCH_CNC_COUNTER_STC cncCounter;
    CPSS_DXCH_CNC_COUNTER_STC in_sum, drop_sum;
    GT_BOOL Enable;

    cpssOsMemSet(&in_sum, 0, sizeof(in_sum));
    cpssOsMemSet(&drop_sum, 0, sizeof(drop_sum));


    osPrintf("TM CNC counters Sum:\n");
    for (i=0; i<32; i++)
    {
        rc = cpssDxChCncBlockClientEnableGet(devNum, i, CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E, &Enable);
        if( GT_OK != rc)
           return rc;

        if (Enable == GT_FALSE)
        {
            continue;
        }

        for (j=0; j<1024; j++)
        {
            rc = cpssDxChCncCounterGet(devNum, i, j, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &cncCounter);
            if (rc != GT_OK)
                return rc;
            if( (cncCounter.packetCount.l[1]+cncCounter.packetCount.l[0]) > 0)
            {
                if( (j%2) == 0)
                    in_sum.packetCount = prvCpssMathAdd64(in_sum.packetCount, cncCounter.packetCount);
                else
                    drop_sum.packetCount = prvCpssMathAdd64(drop_sum.packetCount, cncCounter.packetCount);

                if(includeBytes == GT_TRUE)
                {
                    if( (j%2) == 0)
                        in_sum.byteCount = prvCpssMathAdd64(in_sum.byteCount, cncCounter.byteCount);
                    else
                        drop_sum.byteCount = prvCpssMathAdd64(drop_sum.byteCount, cncCounter.byteCount);
                }
            }
        }
    }

    if (in_sum.packetCount.l[1] + in_sum.packetCount.l[0] > 0)
    {
        osPrintf("0x%X%X ( %u )packets entered TM\n", in_sum.packetCount.l[1], in_sum.packetCount.l[0],
                 (in_sum.packetCount.l[1]*0xFFFFFFFF + in_sum.packetCount.l[0]));
    }

    if (in_sum.byteCount.l[1] + in_sum.byteCount.l[0] > 0)
    {
        osPrintf("0x%X%X ( %u )bytes entered TM\n", in_sum.byteCount.l[1], in_sum.byteCount.l[0],
                 (in_sum.byteCount.l[1]*0xFFFFFFFF + in_sum.byteCount.l[0]));

    }

    if (drop_sum.packetCount.l[1] + drop_sum.packetCount.l[0] > 0)
    {
        osPrintf("0x%X%X ( %u )packets dropped\n", drop_sum.packetCount.l[1], drop_sum.packetCount.l[0],
                 (drop_sum.packetCount.l[1]*0xFFFFFFFF + drop_sum.packetCount.l[0]));
    }

    if (drop_sum.byteCount.l[1] + drop_sum.byteCount.l[0] > 0)
    {
        osPrintf("0x%X%X ( %u )bytes dropped\n", drop_sum.byteCount.l[1], drop_sum.byteCount.l[0],
                 (drop_sum.byteCount.l[1]*0xFFFFFFFF + drop_sum.byteCount.l[0]));

    }

    return GT_OK;
}

GT_STATUS appDemoTmQCountersConfig
(
    IN GT_U8   devNum
)
{
    GT_STATUS   rc=GT_OK;
    GT_U32      i;
    GT_U32      agedPacketCouterQueueMask=0;
    GT_U32      agedPacketCouterQueue=0;
    GT_U64      indexRangesBmp;
    GT_U32      counterBlocks;

/* each queue assigned two counters - one for pass, one for drop.  Ther are 16K Q-nodes in BobCat2 and 4K in BobK */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        counterBlocks = 8;
    else
        counterBlocks = 32;

    /* enable counting for packets entering TM */

    rc = cpssDxChCncTmClientIndexModeSet(devNum, CPSS_DXCH_CNC_TM_INDEX_MODE_0_E);
    if( GT_OK != rc)
       return rc;
    for(i=0;i<counterBlocks;i++)
    {
        rc = cpssDxChCncBlockClientEnableSet(devNum,i,CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E,GT_TRUE);
        if( GT_OK != rc)
           return rc;
        indexRangesBmp.l[0] = 1 << i;
        indexRangesBmp.l[1] = 0;

        rc = cpssDxChCncBlockClientRangesSet(devNum,i,
                                             CPSS_DXCH_CNC_CLIENT_TM_PASS_DROP_E,
                                             &indexRangesBmp);
        if( GT_OK != rc)
           return rc;
    }

        /* enable counting for packets exiting TM */

    rc = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* count all queues for aged (dropped) packets*/
    agedPacketCouterQueueMask   = 0x3FFF;
    agedPacketCouterQueue       = 0x3FFF;

    rc = cpssDxChTmGlueAgingAndDelayAgedPacketCounterConfigSet(devNum,
                   agedPacketCouterQueueMask,
                   agedPacketCouterQueue);
    if (rc != GT_OK)
    {
        return rc;
    }

    return rc;

}

/* we have only 100 aging counters for 16K queues, we need to allocate them deligently */
GT_STATUS appDemoTmNQueuesCountSet
(
    IN GT_U8   devNum,
    IN GT_U32  q0,
    IN GT_U32  counter,
    IN GT_U32  numOfQueues
)
{

    GT_STATUS rc;
    GT_U32 i;

    for(i=0;i<numOfQueues;i++)
    {
        rc = cpssDxChTmGlueAgingAndDelayCounterQueueIdSet(devNum, counter+i, q0+i);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueAgingAndDelayCounterQueueIdSet", rc);
        if (rc != GT_OK)
            return rc;
    }
    return GT_OK;
}

/* block all TM quesues up to qnum, - check if traffic is forwarded by TM*/
GT_STATUS appDemoTmQueuesBlock
(
    IN GT_U8   devNum,
    IN GT_U32  qnum
)
{
    GT_STATUS rc;
    GT_U32 i;
    for(i=0; i<qnum; i++)
    {
        rc = cpssTmNodesCtlQueueUninstall(devNum,i);
        if (rc != GT_OK)
            return rc;
    }
    return GT_OK;
}

GT_32 appDemoTmQueueDropProfileGet
(
    IN GT_U8   devNum,
    IN GT_U32  qIndex
)
{

    GT_U32      profileIndex = 0;
    GT_U64      regAddr;
    GT_U64      data;
    GT_STATUS   rc = GT_OK;

    regAddr.l[0] = (0x2800000+8*(qIndex/4));
    regAddr.l[1] = 0x10;
    rc = cpssTmCtlReadRegister(devNum, &regAddr, &data);
    if (rc != GT_OK)
        return -1;


    switch (qIndex%4)
    {
    case 0:
                    profileIndex = (data.l[0] & 0x7ff);
                    break;
    case 1:
                    profileIndex = ((data.l[0] >> 16)  & 0x7ff);
                    break;
    case 2:
                    profileIndex = (data.l[1] & 0x7ff);
                    break;
    case 3:
                    profileIndex = ((data.l[1] >> 16)  & 0x7ff);
                    break;
    }

    return profileIndex;
}

/* get node parameters, CoS relevant for C and P levels only

use:
shell-execute appDemoTmNodeConfigGet 0, 0, 16, 0
*/
GT_STATUS appDemoTmNodeConfigGet
(
    IN GT_U8                devNum,
    IN CPSS_TM_LEVEL_ENT    level,
    IN GT_U32               index,
    IN GT_U32               CoS
)
{
    GT_STATUS rc=GT_OK;
    CPSS_TM_QUEUE_PARAMS_STC  paramsQ;
    CPSS_TM_A_NODE_PARAMS_STC paramsA;
    CPSS_TM_B_NODE_PARAMS_STC paramsB;
    CPSS_TM_C_NODE_PARAMS_STC paramsC;
    CPSS_TM_PORT_PARAMS_STC   paramsP;
    CPSS_TM_PORT_DROP_PER_COS_STC   cosParams;
    GT_32                     dpIndex;

    switch(level)
    {
    case CPSS_TM_LEVEL_Q_E:
        rc = cpssTmQueueConfigurationRead(devNum, index, &paramsQ);
        if (rc != GT_OK)
            return rc;

        dpIndex = appDemoTmQueueDropProfileGet(devNum, index);
        if (rc != GT_OK)
            return rc;

        osPrintf("Eligible Priority Function = %d, weight = %d\n Shaping Profile = %d\n TD profile = %d (in HW %u)\n",
                 paramsQ.eligiblePrioFuncId,
                 paramsQ.quantum,
                 paramsQ.shapingProfilePtr,
                 paramsQ.dropProfileInd,
                 dpIndex);
        break;

    case CPSS_TM_LEVEL_A_E:
        rc = cpssTmAnodeConfigurationRead(devNum, index, &paramsA);
        if (rc != GT_OK)
            return rc;
        osPrintf("Eligible Priority Function = %d, weight = %d\n Shaping Profile = %d\n TD profile = %d\n NumOfChildren = %d\n",
                 paramsA.eligiblePrioFuncId,
                 paramsA.quantum,
                 paramsA.shapingProfilePtr,
                 paramsA.dropProfileInd,
                 paramsA.numOfChildren);
        break;

    case CPSS_TM_LEVEL_B_E:
        rc = cpssTmBnodeConfigurationRead(devNum, index, &paramsB);
        if (rc != GT_OK)
            return rc;
        osPrintf("Eligible Priority Function = %d, weight = %d\n Shaping Profile = %d\n TD profile = %d\n NumOfChildren = %d\n",
                 paramsB.eligiblePrioFuncId,
                 paramsB.quantum,
                 paramsB.shapingProfilePtr,
                 paramsB.dropProfileInd,
                 paramsB.numOfChildren);
        break;

    case CPSS_TM_LEVEL_C_E:
        rc = cpssTmCnodeConfigurationRead(devNum, index, &paramsC);
        if (rc != GT_OK)
            return rc;
        osPrintf("Eligible Priority Function = %d, weight = %d\n Shaping Profile = %d\n TD profile = %d\n NumOfChildren = %d\n",
                 paramsC.eligiblePrioFuncId,
                 paramsC.quantum,
                 paramsC.shapingProfilePtr,
                 paramsC.dropProfileIndArr[CoS],
                 paramsC.numOfChildren);
        break;

    case CPSS_TM_LEVEL_P_E:
        rc = cpssTmPortConfigurationRead(devNum, index,  &paramsP, &cosParams);
        if (rc != GT_OK)
            return rc;
        osPrintf("Eligible Priority Function = %d, weight = %d\n Shaping: CIR %d KBit/Sec EIR %d KBit/Sec CBS %d Kbytes EBS %d Kbytes\n TD profile = %d\n NumOfChildren = %d\n",
                 paramsP.eligiblePrioFuncId,
                 paramsP.quantumArr[CoS],
                 paramsP.cirBw,
                 paramsP.eirBw,
                 paramsP.cbs,
                 paramsP.ebs,
                 paramsP.dropProfileInd,
                 paramsP.numOfChildren);
        break;

    default:
        osPrintf("Bad level\n");
        return rc;
    }

    return rc;

}


GT_STATUS appDemoTmNodePrioWeightUpdate
(
    IN GT_U8                    devNum,
    IN CPSS_TM_LEVEL_ENT        level,
    IN GT_U32                   index,
    IN CPSS_TM_ELIG_FUNC_NODE_ENT funcNum,
    IN GT_U32                   quantum
)
{
    CPSS_TM_PORT_PARAMS_STC       paramsP;
    CPSS_TM_A_NODE_PARAMS_STC     paramsA;
    CPSS_TM_B_NODE_PARAMS_STC     paramsB;
    CPSS_TM_C_NODE_PARAMS_STC     paramsC;
    CPSS_TM_QUEUE_PARAMS_STC      paramsQ;
    CPSS_TM_PORT_DROP_PER_COS_STC cosParams;
    CPSS_TM_ELIG_FUNC_NODE_ENT funcNumOld;
    CPSS_TM_ELIG_FUNC_QUEUE_ENT qFuncNumOld;
    GT_STATUS                  rc = GT_OK;
    GT_U32                     quantumArr[8];
    GT_U32                     i;

    osMemSet(&paramsP, 0, sizeof(paramsP));
    osMemSet(&paramsA, 0, sizeof(paramsA));
    osMemSet(&paramsB, 0, sizeof(paramsB));
    osMemSet(&paramsC, 0, sizeof(paramsC));
    osMemSet(&paramsQ, 0, sizeof(paramsQ));
    switch(level)
    {
    case CPSS_TM_LEVEL_Q_E:
        rc = cpssTmQueueConfigurationRead(devNum, index, &paramsQ);
        qFuncNumOld = paramsQ.eligiblePrioFuncId;
        paramsQ.eligiblePrioFuncId = (CPSS_TM_ELIG_FUNC_QUEUE_ENT)funcNum;
        paramsQ.quantum = quantum*CPSS_TM_NODE_QUANTUM_UNIT_CNS;
        rc = cpssTmQueueUpdate(devNum, index, &paramsQ);
        break;

    case CPSS_TM_LEVEL_A_E:
        rc = cpssTmAnodeConfigurationRead(devNum,index,&paramsA);
        funcNumOld = paramsA.eligiblePrioFuncId;
        paramsA.eligiblePrioFuncId = funcNum;
        paramsA.quantum = quantum*CPSS_TM_NODE_QUANTUM_UNIT_CNS;
        rc = cpssTmAnodeUpdate(devNum, index, &paramsA);
        break;

    case CPSS_TM_LEVEL_B_E:
        rc = cpssTmBnodeConfigurationRead(devNum, index, &paramsB);
        funcNumOld = paramsB.eligiblePrioFuncId;
        paramsB.eligiblePrioFuncId = funcNum;
        paramsB.quantum = quantum*CPSS_TM_NODE_QUANTUM_UNIT_CNS;
        rc = cpssTmBnodeUpdate(devNum, index, &paramsB);
        break;

    case CPSS_TM_LEVEL_C_E:
        rc = cpssTmCnodeConfigurationRead(devNum, index, &paramsC);
        funcNumOld = paramsC.eligiblePrioFuncId;
        paramsC.eligiblePrioFuncId = funcNum;
        paramsC.quantum = quantum*CPSS_TM_NODE_QUANTUM_UNIT_CNS;
        rc = cpssTmCnodeUpdate(devNum, index, &paramsC);
        break;

    case CPSS_TM_LEVEL_P_E:
        rc = cpssTmPortConfigurationRead(devNum, index, &paramsP, &cosParams);
        funcNumOld = paramsP.eligiblePrioFuncId;
        paramsP.eligiblePrioFuncId = funcNum;
        for(i=0;i<8;i++)
            quantumArr[i] = (GT_U16)quantum*CPSS_TM_PORT_QUANTUM_UNIT_CNS;
        rc = cpssTmPortSchedulingUpdate(devNum, index, funcNum, quantumArr, paramsP.schdModeArr);
        break;

    default:
        osPrintf("Bad level\n");
        return rc;
    }
    if (rc != GT_OK)
        return rc;
    if(level!=CPSS_TM_LEVEL_Q_E)
    {
     osPrintf("Changed from eligible priority function %d\n",funcNumOld);
    }
    else
    {
     osPrintf("Changed from eligible priority function %d\n",qFuncNumOld);
    }
    return rc;
}

/* for non-P levels update shaping profile identified by index parameter,
for P-level update the shaping on the P-node index*/
GT_STATUS appDemoTmShaperUpdate
(
    IN GT_U8             devNum,
    IN CPSS_TM_LEVEL_ENT level,
    IN GT_U32            index,
    IN GT_U32            cirBw,
    IN GT_U32            cbs,
    IN GT_U32            eirBw,
    IN GT_U32            ebs
)
{
    GT_STATUS rc = GT_OK;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC  profileStr;

    if(level != CPSS_TM_LEVEL_P_E )
    {
        profileStr.cirBw = cirBw;
        profileStr.cbs = cbs;
        profileStr.eirBw = eirBw;
        profileStr.ebs = ebs;

        rc = cpssTmShapingProfileUpdate(devNum, level, index, &profileStr);
        if(rc != GT_OK)
            return rc;
    }
    else
    {
        rc = cpssTmPortShapingUpdate(devNum, index, cirBw, eirBw, cbs, ebs);
    }
/*  rc = cpssTmShapingProfileRead(dev,level, index, &profileStr);
    if(rc != GT_OK)
        return rc;
    osPrintf("wrote shaper profile: CIR = %d, CBS = %d, EIR = %d, EBS = %d\n", profileStr.cirBw,
             profileStr.cbs, profileStr.eirBw, profileStr.ebs);*/
    return rc;
}


GT_STATUS appDemoTmNodeShaperEnable
(
    IN GT_U8             devNum,
    IN CPSS_TM_LEVEL_ENT level,
    IN GT_U32            index,
    IN GT_BOOL           enable,
    IN GT_U32            shaperIndex
)
{
    GT_STATUS rc=GT_OK;
    CPSS_TM_A_NODE_PARAMS_STC paramsA;
    CPSS_TM_B_NODE_PARAMS_STC paramsB;
    CPSS_TM_C_NODE_PARAMS_STC paramsC;
    CPSS_TM_QUEUE_PARAMS_STC  paramsQ;
    CPSS_TM_SHAPING_PROFILE_PARAMS_STC  profileStr;

    if(enable == GT_TRUE)
    {
        rc = cpssTmShapingProfileRead(devNum, level, shaperIndex, &profileStr);
        if(rc != GT_OK)
        {
            osPrintf("Cannot set such a shaper\n");
            return rc;
        }
        osPrintf("Set shaper profile: CIR = %d, CBS = %d, EIR = %d, EBS = %d\n", profileStr.cirBw,
                 profileStr.cbs, profileStr.eirBw, profileStr.ebs);

    }

    switch(level)
    {
    case CPSS_TM_LEVEL_Q_E:
        rc = cpssTmQueueConfigurationRead(devNum, index, &paramsQ);
        if (rc != GT_OK)
            return rc;
        if(enable == GT_FALSE)
            paramsQ.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        else
            paramsQ.shapingProfilePtr = shaperIndex;

        rc = cpssTmQueueUpdate(devNum, index, &paramsQ);
        break;

    case CPSS_TM_LEVEL_A_E:
        rc = cpssTmAnodeConfigurationRead(devNum, index,  &paramsA);
        if (rc != GT_OK)
            return rc;
        if(enable == GT_FALSE)
            paramsA.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        else
            paramsA.shapingProfilePtr = shaperIndex;
        rc = cpssTmAnodeUpdate(devNum, index, &paramsA);
        break;

    case CPSS_TM_LEVEL_B_E:
        rc = cpssTmBnodeConfigurationRead(devNum, index,  &paramsB);
        if (rc != GT_OK)
            return rc;
        if(enable == GT_FALSE)
            paramsB.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        else
            paramsB.shapingProfilePtr = shaperIndex;
        rc = cpssTmBnodeUpdate(devNum, index, &paramsB);
        break;

    case CPSS_TM_LEVEL_C_E:
        rc = cpssTmCnodeConfigurationRead(devNum, index,  &paramsC);
        if (rc != GT_OK)
            return rc;
        if(enable == GT_FALSE)
            paramsC.shapingProfilePtr = CPSS_TM_SHAPING_INFINITE_PROFILE_INDEX_CNS;
        else
            paramsC.shapingProfilePtr = shaperIndex;
        rc = cpssTmCnodeUpdate(devNum, index, &paramsC);
        break;

    default:
        osPrintf("Bad level\n");
        return rc;
    }
    if (rc != GT_OK)
        return rc;

    return rc;

}

GT_STATUS appDemoTmAllNodesShaperDisable
(
    IN GT_U8             devNum
)
{
    GT_U32  i;
    GT_STATUS rc = GT_OK;


    for (i=0;i<16000;i++)
    {
        rc = appDemoTmNodeShaperEnable(devNum, 0, i, GT_FALSE, 0);
    }

    for (i=0;i<2000;i++)
    {
        rc = appDemoTmNodeShaperEnable(devNum, 1, i, GT_FALSE, 0);
    }

    for (i=0;i<200;i++)
    {
        rc = appDemoTmNodeShaperEnable(devNum, 2, i, GT_FALSE, 0);
    }

    for (i=0;i<40;i++)
    {
        rc = appDemoTmNodeShaperEnable(devNum, 3, i, GT_FALSE, 0);
    }

    return rc;
}


/* Create a WRED curve to serve in TD profile on Q-level, for now - traditional curves only, maxProbability in percents from 1 to 100 */
/* Returns index of created curve or GT_ERROR, NOT REGULAR ERROR CODE */
GT_32 appDemoTmWREDCurveCreate
(
    IN GT_U8        devNum,
    IN GT_U32       maxProbability
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    curveInd;

    rc = cpssTmDropWredTraditionalCurveCreate(devNum, CPSS_TM_LEVEL_Q_E, 0, maxProbability, &curveInd);
    if(rc != GT_OK)
        return GT_ERROR;
    osPrintf("Max probability %d, curve number %d\n", maxProbability, curveInd);

    return curveInd;
}


/* set TD profile with bytesThreshold in MB, thresholdRatio same for all colors and modes
 typical use:
    shell-execute appDemoTmDropProfileCreate 0, 0, 2, 5, 5, 0, 0 or
    shell-execute appDemoTmDropProfileCreate 0, 0, 0, 7, 7, 1, 0
 Returns index of created profile or GT_ERROR, NOT REGULAR ERROR CODE
*/
GT_32 appDemoTmDropProfileCreate
(
    IN GT_U8                devNum,
    IN CPSS_TM_LEVEL_ENT    level,
    IN CPSS_TM_DROP_MODE_ENT dropMode,
    IN GT_U32               cbMBytesThreshold,
    IN GT_U32               caMBytesThreshold,
    IN GT_U32               curveInd,
    IN GT_U32               aqlExponent
)
{
    GT_STATUS                       rc=GT_OK;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profile;
    GT_U32 profileInd;
    GT_U32                          i;

    osMemSet(&profile, 0, sizeof(profile));

    profile.dropMode = dropMode;
    switch (dropMode)
    {
        case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
            profile.cbTdThresholdBytes = cbMBytesThreshold * 1024 * 1024;  /* make it big - to have no influence*/
            profile.caWredDp.aqlExponent  = (GT_U8)aqlExponent;
            for (i=0; i < 3 ; i++)
            {
                profile.caWredDp.curveIndex[i] = (GT_U8)curveInd;
                profile.caWredDp.dpCurveScale[i] = 0;
                profile.caWredDp.caWredTdMinThreshold[i] = 0;
                profile.caWredDp.caWredTdMaxThreshold[i] = caMBytesThreshold * 1024 * 1024;
            }
            break;
        case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
            profile.cbTdThresholdBytes = cbMBytesThreshold * 1024 * 1024 ;  /* make it big - to have no influence*/
            for (i=0; i < 3 ; i++)
            {
                profile.caTdDp.caTdThreshold[i]= caMBytesThreshold * 1024 * 1024;
            }
            break;
        case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
            profile.cbTdThresholdBytes = cbMBytesThreshold * 1024 * 1024 ;
            break;
        default:break;
    }

    rc = cpssTmDropProfileCreate(devNum, level, CPSS_TM_INVAL_CNS, &profile, &profileInd);
    if (rc != GT_OK)
        return GT_ERROR;

    osPrintf("created TD profile ID %d\n", profileInd);
    return profileInd;
}


/* set TD profile ID to a node on a level, also may disable TD*/
GT_STATUS appDemoTmNodeDropProfileSet
(
    IN GT_U8                devNum,
    IN CPSS_TM_LEVEL_ENT    level,
    IN GT_U32               index,
    IN GT_BOOL              enable,
    IN GT_U32               profileId
)
{
    GT_STATUS                 rc=GT_OK;
    CPSS_TM_A_NODE_PARAMS_STC paramsA;
    CPSS_TM_B_NODE_PARAMS_STC paramsB;
    CPSS_TM_C_NODE_PARAMS_STC paramsC;
    CPSS_TM_QUEUE_PARAMS_STC  paramsQ;
    GT_U32 i;

    if(enable == GT_FALSE)
        profileId = CPSS_TM_NO_DROP_PROFILE_CNS;

    switch(level)
    {
    case CPSS_TM_LEVEL_Q_E:
        rc = cpssTmQueueConfigurationRead(devNum, index, &paramsQ);
        if (rc != GT_OK)
            return rc;

        paramsQ.dropProfileInd = profileId;

        rc = cpssTmQueueUpdate(devNum, index, &paramsQ);
        break;

    case CPSS_TM_LEVEL_A_E:
        rc = cpssTmAnodeConfigurationRead(devNum, index, &paramsA);
        if (rc != GT_OK)
            return rc;

        paramsA.dropProfileInd = profileId;

        rc = cpssTmAnodeUpdate(devNum, index, &paramsA);
        break;

    case CPSS_TM_LEVEL_B_E:
        rc = cpssTmBnodeConfigurationRead(devNum, index, &paramsB);
        if (rc != GT_OK)
            return rc;

        paramsB.dropProfileInd = profileId;

        rc = cpssTmBnodeUpdate(devNum, index, &paramsB);
        break;

    case CPSS_TM_LEVEL_C_E:
        rc = cpssTmCnodeConfigurationRead(devNum, index, &paramsC);
        if (rc != GT_OK)
            return rc;

        for (i=0; i<8; i++)
            paramsC.dropProfileIndArr[i] = profileId;

        rc = cpssTmCnodeUpdate(devNum, index, &paramsC);
        break;

    case CPSS_TM_LEVEL_P_E:

        rc = cpssTmPortDropUpdate(devNum, index, profileId);
        break;


    default:
        osPrintf("Bad level\n");
        return rc;
    }

    if (rc != GT_OK)
        return rc;
    return GT_OK;
}



/* update an existing TD profile */
GT_STATUS appDemoTmDropProfileUpdate
(
    IN GT_U8                devNum,
    IN CPSS_TM_LEVEL_ENT    level,
    IN GT_U32               profileIndex,
    IN GT_U32               cbMBytesThreshold,
    IN GT_U32               caMBytesThreshold,
    IN GT_U32               curveInd,
    IN GT_U32               aqlExponent
)
{
    GT_STATUS             rc=GT_OK;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profile;
    GT_U32 i;

    rc = cpssTmDropProfileRead(devNum, level, 0, profileIndex, &profile);
    if (rc != GT_OK)
        return rc;

    switch (profile.dropMode)
    {
        case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
            profile.cbTdThresholdBytes = cbMBytesThreshold * 1024 * 1024 ;  /* make it big - to have no influence*/
            profile.caWredDp.aqlExponent  = (GT_U8)aqlExponent;
            for (i=0; i < 3 ; i++)
            {
                profile.caWredDp.curveIndex[i] = (GT_U8)curveInd;
                profile.caWredDp.dpCurveScale[i] = 0;
                profile.caWredDp.caWredTdMinThreshold[i] = 0;
                profile.caWredDp.caWredTdMaxThreshold[i] = caMBytesThreshold * 1024 * 1024 ;
            }
            break;
        case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
            profile.cbTdThresholdBytes = cbMBytesThreshold * 1024 ;  /* make it big - to have no influence*/
            for (i=0; i < 3 ; i++)
            {
                profile.caTdDp.caTdThreshold[i]= caMBytesThreshold * 1024 * 1024 ;
            }
            break;
        case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
            profile.cbTdThresholdBytes = cbMBytesThreshold * 1024 * 1024 ;
            break;
        default:break;
    }

    rc = cpssTmDropProfileUpdate(devNum, level, CPSS_TM_INVAL_CNS, profileIndex, &profile);
    if (rc != GT_OK)
        return GT_ERROR;

    return GT_OK;
}


/* dump all Drop Profiles Indexes per device  */
GT_STATUS appDemoTmDropProfilesIdsDump(GT_U8 devNum)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profileParams;
    GT_U32                          profIndex;
    GT_32                           cosArr[] = {0,1,2,3,4,5,6,7,-1};
    GT_32                           fromCos, toCos, cosArrInd, cos;
    CPSS_TM_LEVEL_ENT               level;

    for (level = CPSS_TM_LEVEL_Q_E; level <= CPSS_TM_LEVEL_P_E; level++)
    {
        if (level == CPSS_TM_LEVEL_P_E)
        {
            fromCos = 0;
            toCos = 8;
        }
        else if (level == CPSS_TM_LEVEL_C_E)
        {
            fromCos = 0;
            toCos = 7;
        }
        else
        {
            fromCos = 8;
            toCos = 8;
        }

        for (cosArrInd = fromCos; cosArrInd <= toCos; cosArrInd++)
        {
            profIndex = 0,
            cos = cosArr[cosArrInd];
            rc = prvCpssTmDropProfileGetNext(devNum, level, cos, &profIndex);

            if (rc != GT_OK)
                continue;

            cpssOsPrintf("Level: %s Cos %d\n", tmTreelevelNamesArr[level], cos);

            while(rc == GT_OK)
            {
                /* verify profiles exist */
                rc = cpssTmDropProfileRead(devNum,
                                           level,
                                           cos,
                                           profIndex,
                                           &profileParams);

                if (rc != GT_OK)
                {
                    cpssOsPrintf("tm_dump_drop_profiles: level: %d, profileIndex: %d, error: %d\n", level, profIndex, rc);
                    return rc;
                }

                cpssOsPrintf("\tprofileIndex: %d, dropMode: %s\n", profIndex, tmDropModeNamesArr[profileParams.dropMode]);

                rc = prvCpssTmDropProfileGetNext(devNum, level, cos, &profIndex);
            }
        }
    }

    return GT_OK;
}

/* Print Drop Profiles parameters */
static GT_STATUS appDemoTmDropProfilePrint
(
    CPSS_TM_LEVEL_ENT               level,
    GT_U32                          cos,
    GT_U32                          profileInd,
    CPSS_TM_DROP_PROFILE_PARAMS_STC *profilePtr
)
{
    char modeStr[64];
    GT_STATUS rc = GT_OK;

    switch(profilePtr->dropMode)
    {
    case CPSS_TM_DROP_MODE_CB_TD_CA_WRED_E:
        osSprintf(modeStr,"ColorBlind TD + WRED Color Aware");
        osPrintf("\nDrop Profile: index: %d, level: %s, cos: %d, mode: %s\n",
                profileInd,
                tmTreelevelNamesArr[level],
                cos,
                modeStr);

        osPrintf("  CB Threshold Bytes (BW) %u (%u)\n",
                profilePtr->cbTdThresholdBytes, cpssTmDropConvertBytesToBW(profilePtr->cbTdThresholdBytes));

        osPrintf("  aqlExponent %d\n",
                profilePtr->caWredDp.aqlExponent);

        osPrintf("  WRED Curves %d, %d, %d\n",
                 profilePtr->caWredDp.curveIndex[0],
                 profilePtr->caWredDp.curveIndex[1],
                 profilePtr->caWredDp.curveIndex[2]);

        osPrintf("  MinThreshold Bytes (BW) %u (%u), %u (%u), %u (%u)\n",
                 profilePtr->caWredDp.caWredTdMinThreshold[0], cpssTmDropConvertBytesToBW(profilePtr->caWredDp.caWredTdMinThreshold[0]),
                 profilePtr->caWredDp.caWredTdMinThreshold[1], cpssTmDropConvertBytesToBW(profilePtr->caWredDp.caWredTdMinThreshold[1]),
                 profilePtr->caWredDp.caWredTdMinThreshold[2], cpssTmDropConvertBytesToBW(profilePtr->caWredDp.caWredTdMinThreshold[2]));

        osPrintf("  MaxThreshold Bytes (BW) %u (%u), %u (%u), %u (%u)\n",
                 profilePtr->caWredDp.caWredTdMaxThreshold[0], cpssTmDropConvertBytesToBW(profilePtr->caWredDp.caWredTdMaxThreshold[0]),
                 profilePtr->caWredDp.caWredTdMaxThreshold[1], cpssTmDropConvertBytesToBW(profilePtr->caWredDp.caWredTdMaxThreshold[1]),
                 profilePtr->caWredDp.caWredTdMaxThreshold[2], cpssTmDropConvertBytesToBW(profilePtr->caWredDp.caWredTdMaxThreshold[2]));

        break;

    case CPSS_TM_DROP_MODE_CB_TD_CA_TD_E:
        osSprintf(modeStr,"ColorBlind TD + Color Aware TD");
        osPrintf("\nDrop Profile: index: %d, level: %s, cos: %d, mode: %s\n",
                profileInd,
                tmTreelevelNamesArr[level],
                cos,
                modeStr);

        osPrintf("  CB Threshold Bytes (BW) %u (%u)\n",
                profilePtr->cbTdThresholdBytes, cpssTmDropConvertBytesToBW(profilePtr->cbTdThresholdBytes));

        osPrintf("  CA Threshold Bytes (BW) %u (%u), %d (%u), %u (%u),\n",
                profilePtr->caTdDp.caTdThreshold[0], cpssTmDropConvertBytesToBW(profilePtr->caTdDp.caTdThreshold[0]),
                profilePtr->caTdDp.caTdThreshold[1], cpssTmDropConvertBytesToBW(profilePtr->caTdDp.caTdThreshold[1]),
                profilePtr->caTdDp.caTdThreshold[2], cpssTmDropConvertBytesToBW(profilePtr->caTdDp.caTdThreshold[2]));

        break;

    case CPSS_TM_DROP_MODE_CB_TD_ONLY_E:
        osSprintf(modeStr,"ColorBlind TD");
        osPrintf("\nDrop Profile: index: %d, level: %s, cos: %d, mode: %s\n",
                profileInd,
                tmTreelevelNamesArr[level],
                cos,
                modeStr);

        osPrintf("  CB Threshold Bytes (BW) %u(%u)\n",
                profilePtr->cbTdThresholdBytes, cpssTmDropConvertBytesToBW(profilePtr->cbTdThresholdBytes));
        break;
    default:
        osPrintf("Unknown drop mode %d\n", profilePtr->dropMode);
        rc = GT_BAD_VALUE;
    }

    return rc;
}

/* Dump Drop Profiles parameters per device for level, cos and range of Drop Profile Indexes */
GT_STATUS appDemoTmDropProfilesLevelDump
(
    GT_U8               devNum,
    CPSS_TM_LEVEL_ENT   level,
    GT_U32              cos,
    GT_U32              from,
    GT_U32              to
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profile;
    GT_U32                          profileInd;

    if (cos > 7)
    {
        cos = -1;
    }

    for (profileInd = from; profileInd <= to; profileInd++)
    {
        rc = cpssTmDropProfileRead(devNum, level, cos, profileInd, &profile);

        if (rc != GT_OK)
            return rc;

        rc = appDemoTmDropProfilePrint(level, cos, profileInd, &profile);
        if (rc != GT_OK)
            break;
    }

    return rc;
}

/* Dump Drop Profiles parameters per device for all Cos and All Levels for range of Drop Profile Indexes */
GT_STATUS appDemoTmDropProfilesDump
(
    GT_U8  devNum,
    GT_U32 fromInd,
    GT_U32 toInd
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_TM_DROP_PROFILE_PARAMS_STC profile;
    GT_U32                          profileInd;
    GT_32                           cosArr[] = {0,1,2,3,4,5,6,7,-1};
    GT_32                           fromCos, toCos, cosArrInd, cos;
    CPSS_TM_LEVEL_ENT               level;

    for (level = CPSS_TM_LEVEL_Q_E; level <= CPSS_TM_LEVEL_P_E; level++)
    {
        if (level == CPSS_TM_LEVEL_P_E)
        {
            fromCos = 0;
            toCos = 8;
        }
        else if (level == CPSS_TM_LEVEL_C_E)
        {
            fromCos = 0;
            toCos = 7;
        }
        else
        {
            fromCos = 8;
            toCos = 8;
        }

        for (cosArrInd = fromCos; cosArrInd <= toCos; cosArrInd++)
        {
            cos = cosArr[cosArrInd];
            for (profileInd = fromInd; profileInd <= toInd; profileInd++)
            {
                rc = cpssTmDropProfileRead(devNum, level, (GT_U32)cos, profileInd, &profile);

                if (rc != GT_OK)
                    continue;

                rc = appDemoTmDropProfilePrint(level, cos, profileInd, &profile);
                if (rc != GT_OK)
                    return rc;
            }
        }
    }
    return GT_OK;
}


GT_STATUS appDemoTmDropMaskSet
(
    IN GT_U8    devNum,
    IN GT_U32   profileId,
    IN GT_U32   dropBitMask,
    IN GT_BOOL  redDropByTM,
    IN GT_BOOL  redPaint
)
{

    GT_STATUS rc;
    GT_U32 i;
    CPSS_DXCH_TM_GLUE_DROP_MASK_STC dropMaskCfg;
    CPSS_DXCH_COS_PROFILE_STC       cosProfile;

    osPrintf("profileId %d, dropMask 0x%X, redDropByTM %d redPaint %d\n",profileId, dropBitMask, redDropByTM, redPaint);
    /* Drop profile 0 consider for drop P level Tail Drop only */
    osMemSet(&dropMaskCfg, 0, sizeof(dropMaskCfg));
    /* Set port tail drop recomenadtion for drop desision */
    dropMaskCfg.qTailDropUnmask = dropBitMask & 0x1;
    dropMaskCfg.qWredDropUnmask = (dropBitMask>>1) & 0x1;
    dropMaskCfg.aTailDropUnmask  = (dropBitMask>>2) & 0x1;
    dropMaskCfg.aWredDropUnmask = (dropBitMask>>3) & 0x1;
    dropMaskCfg.bTailDropUnmask = (dropBitMask>>4) & 0x1;
    dropMaskCfg.bWredDropUnmask = (dropBitMask>>5) & 0x1;
    dropMaskCfg.cTailDropUnmask = (dropBitMask>>6) & 0x1;
    dropMaskCfg.cWredDropUnmask = (dropBitMask>>7) & 0x1;
    dropMaskCfg.portTailDropUnmask  = (dropBitMask>>8) & 0x1;
    dropMaskCfg.portWredDropUnmask = (dropBitMask>>9) & 0x1;
    dropMaskCfg.outOfResourceDropUnmask = GT_TRUE;

    if(redDropByTM)
        dropMaskCfg.redPacketsDropMode = CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_ASK_TM_RESP_E;
    else
        dropMaskCfg.redPacketsDropMode = CPSS_DXCH_TM_GLUE_DROP_RED_PACKET_DROP_MODE_DROP_ALL_RED_E;

    /* Call CPSS API function */
    for (i=0 ; i<16; i++)
    {
        rc = cpssDxChTmGlueDropProfileDropMaskSet(devNum, profileId, i, &dropMaskCfg);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChTmGlueDropProfileDropMaskSet", rc);
        if(rc != GT_OK)
            return rc;
    }

    for (i=0 ; i<16383; i++)
    {
        rc = cpssDxChTmGlueDropQueueProfileIdSet(devNum, i, profileId);
        if(rc != GT_OK)
            osPrintf(" %d",i);
    }
    osPrintf(" \n");

    osMemSet(&cosProfile, 0, sizeof(cosProfile));

    for (i=0 ; i<8; i++)
    {
        if(redPaint)
            cosProfile.dropPrecedence = CPSS_DP_RED_E;
        else
            cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority = i;
        cosProfile.trafficClass = i;
        rc = cpssDxChCosProfileEntrySet(devNum, i+20, &cosProfile);
        if( GT_OK != rc)
           return rc;
    }


    return GT_OK;
}


/* if application relies on TCAM configuration -done in cpssInit, then tcamIndex should start with 18432
   in this example we deal with traffic arriving on port1 (in case of CPU TX, it is CPU port
   and not src port in DS tag), if it is VLAN = 1, it is assigned source ePort eport2,
   if it is untagged or from another VLAN, it is dropped */

static GT_STATUS TTIRuleSet
(
    IN GT_U8        devNum,
    IN GT_U32       tcamIndex1,
    IN GT_PORT_NUM  port1,
    IN GT_PORT_NUM  ingressEport1,
    IN GT_U32       step,
    IN GT_BOOL      drop,
    IN GT_U32       scenario
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_DXCH_TTI_RULE_UNT              pattern;
    CPSS_DXCH_TTI_RULE_UNT              mask;
    CPSS_DXCH_TTI_ACTION_STC            action;
    CPSS_DXCH_TTI_KEY_TYPE_ENT          keyType;
    CPSS_DXCH_TTI_RULE_TYPE_ENT         ruleType;
    GT_U8                               i,j;
    GT_U32                              startVlan = 1, startCVlan = 100;
    GT_U32                              startSVlan = 200;

    osMemSet(&(mask), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    osMemSet(&(pattern), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    osMemSet(&action, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));
    keyType  = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
    ruleType = CPSS_DXCH_TTI_RULE_UDB_10_E;
    rc = cpssDxChTtiPacketTypeKeySizeSet(devNum, keyType, CPSS_DXCH_TTI_KEY_SIZE_10_B_E);
    if(rc != GT_OK)
        return rc;

    /* UDB for source ePort*/
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 0,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 26);

    if(rc != GT_OK)
        return rc;
    /* UDB for Tag0 VLAN ID*/
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 1,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 24);

    if(rc != GT_OK)
        return rc;

/* for scenario 7 send packets with S-vlan 200 and C-vlan 100-107 */
    if(scenario == 7)
    {
        /* UDB for Tag1 VLAN ID*/
        rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 2,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E, 14);

        if(rc != GT_OK)
            return rc;

        /* UDB for Tag1 exists*/
        rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 3,
                                           CPSS_DXCH_TTI_OFFSET_METADATA_E, 16);

        if(rc != GT_OK)
            return rc;
        action.command = CPSS_PACKET_CMD_FORWARD_E;
        action.sourceEPortAssignmentEnable = GT_TRUE;
        action.sourceEPort = 9;
        action.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        action.keepPreviousQoS = GT_TRUE;
        action.bindToCentralCounter         = GT_TRUE;

        action.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

        /* inner tag exists */
        pattern.udbArray.udb[3] = 0x1;
        mask.udbArray.udb[3]    = 0x1;

        for(i=0;i<8;i++)
        {
            /* C-vlan (inner vlan tag) */
            pattern.udbArray.udb[2] = (GT_U8)(startCVlan+i);
            mask.udbArray.udb[2] = 0xFF;
            osPrintf("Match VLAN1 %d\n", pattern.eth.vid1);
            action.flowId = i;
            action.centralCounterIndex          = 10240 + i;

            rc = cpssDxChTtiRuleSet(devNum, tcamIndex1+i*3, ruleType, &pattern, &mask,
                                &action);
            if(rc != GT_OK)
                return rc;

            rc = cpssDxChTtiPortLookupEnableSet(devNum, port1+i, keyType, GT_TRUE);
            if(rc != GT_OK)
                return rc;
        }

        return GT_OK;
    }

    if(scenario == CPSS_TM_BIST)
    {
        rc = appDemoTmTCAMCountersSet(devNum);
        if (rc != GT_OK)
            return rc;

        /* enable the distination port for looped back traffic match */
        rc = cpssDxChTtiPortLookupEnableSet(devNum, port1, keyType, GT_TRUE);

        /* if a packet comes from CPU, it should be enabled lookup on CPU port, not on src port filed in DSA tag */
        rc = cpssDxChTtiPortLookupEnableSet(devNum, CPSS_CPU_PORT_NUM_CNS, keyType, GT_TRUE);
        if (rc != GT_OK)
            return rc;

        if(drop == GT_TRUE)
            action.command = CPSS_PACKET_CMD_DROP_HARD_E;
        else
            action.command = CPSS_PACKET_CMD_FORWARD_E /*CPSS_PACKET_CMD_DROP_HARD_E*/;
        action.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
        action.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;
        action.keepPreviousQoS = GT_TRUE;
        action.egressInterface.type             = CPSS_INTERFACE_PORT_E;
        action.egressInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum); /* the port is on local device*/;
        action.egressInterface.devPort.portNum  = port1;

        action.redirectCommand  = CPSS_DXCH_TTI_REDIRECT_TO_EGRESS_E;

        action.bridgeBypass       = GT_TRUE;
        action.ingressPipeBypass  = GT_FALSE;
        action.actionStop         = GT_TRUE;

        action.bindToCentralCounter = GT_TRUE;
        action.centralCounterIndex  = ttiCpuPortCounterIndex;
        action.sourceEPortAssignmentEnable = GT_TRUE;
        action.sourceEPort          = 9;
        action.flowId               = 1;

        /* catch packets on CPU port */
        pattern.udbArray.udb[0] = (GT_U8)CPSS_CPU_PORT_NUM_CNS;
        mask.udbArray.udb[0]    = 0xFF;
        rc = cpssDxChTtiRuleSet(devNum, tcamIndex1, ruleType, &pattern, &mask,
                                &action);
        osPrintf("write rule at index %d\n", tcamIndex1);
        return GT_OK;
    }

    /* UDB for Tag0[9..12] VLAN ID*/
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 2,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 25);

    if(rc != GT_OK)
        return rc;
    /* UDB for Tag1 VLAN ID*/
    rc = cpssDxChTtiUserDefinedByteSet(devNum, keyType, 3,
                                       CPSS_DXCH_TTI_OFFSET_METADATA_E, 14);

    if(rc != GT_OK)
        return rc;

    if(drop == GT_TRUE)
        action.command = CPSS_PACKET_CMD_DROP_HARD_E;
    else
        action.command = CPSS_PACKET_CMD_FORWARD_E /*CPSS_PACKET_CMD_DROP_HARD_E*/;
    action.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;
    action.userDefinedCpuCode = CPSS_NET_FIRST_USER_DEFINED_E + 1;
    action.keepPreviousQoS = GT_TRUE;
    /*action.trustUp = GT_TRUE; check how it works when keepPreviousQoS == false*/

    for(i=0;i<7;i=i+1)
    {
        rc = cpssDxChTtiPortLookupEnableSet(devNum, port1+i, keyType, GT_TRUE);
        if(rc != GT_OK)
            return rc;

        switch(scenario)
        {

        case 8:
            for(j=0;j<100;j++)
            {
                pattern.udbArray.udb[1] = (GT_U8)(startVlan + i*100 + j);
                mask.udbArray.udb[1] = 0xFF;
                action.flowId = i*100 + j + 1;
                rc = cpssDxChTtiRuleSet(devNum, tcamIndex1+i*100 + j, ruleType, &pattern, &mask,
                                        &action);
            }
            break;

        case 7:

            pattern.udbArray.udb[1] = (startSVlan+(i/2)*200) & 0xFF;
            mask.udbArray.udb[1] = 0xFF;
            pattern.udbArray.udb[2] = ((startSVlan+(i/2)*200)>>8) & 0xFF;
            mask.udbArray.udb[2] = 0x0F;

            /* inner VLAN match */
            /*
            pattern.udbArray.udb[3] = (GT_U8)(startCVlan+i);
            mask.udbArray.udb[3] = 0xFF;*/

            osPrintf("Match VLAN 0 %d, VLAN 1 %d\n",pattern.udbArray.udb[2]*256 + pattern.udbArray.udb[1], pattern.udbArray.udb[3]);
            action.sourceEPortAssignmentEnable = GT_TRUE;
            action.sourceEPort = 9;
            action.bindToCentralCounter         = GT_TRUE                               ;
            action.centralCounterIndex          = 10240 + i;

            action.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_ALL_E;

            action.tag1VlanId = 99;

            action.flowId = i;
            action.centralCounterIndex          = 10240 + i;
            rc = cpssDxChTtiRuleSet(devNum, tcamIndex1+i, ruleType, &pattern, &mask,
                                    &action);
            break;

        case 6:

            pattern.udbArray.udb[0] = (GT_U8)((port1+i)&0x00ff);
            mask.udbArray.udb[0] = 0xFF;
            pattern.udbArray.udb[1] = (GT_U8)((startVlan+i)&0x00ff);
            mask.udbArray.udb[1] = 0xFF;
            action.flowId = i+1;
            osPrintf("port %d, vlan %d, flowID = %d\n", port1+i, startVlan+i, action.flowId);
            action.keepPreviousQoS = GT_TRUE;
            action.sourceEPortAssignmentEnable = GT_TRUE;
            action.sourceEPort = (i+1)+200;

            rc = cpssDxChTtiRuleSet(devNum, tcamIndex1+i, ruleType, &pattern, &mask,
                                    &action);
        /* in order to see FDB learning on assigned port, make additional settings */
            rc = cpssDxChBrgFdbPortLearnStatusSet(devNum,(i+1)+200, GT_FALSE, CPSS_LOCK_FRWRD_E);
            if(rc != GT_OK)
                return rc;

            rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, (i+1)+200, GT_TRUE);
            if(rc != GT_OK)
                return rc;
#if 0
            /*In addition create a case for flow ID that maps into Q-node, connected to port 1, then we will
                move that Q-node to tree rooted in port 0 and verify that such operation is working */

            if(i==0)
            {
                pattern.udbArray.udb[0] = 0;
                mask.udbArray.udb[0] = 0;

                pattern.udbArray.udb[1] = 1;
                mask.udbArray.udb[1] = 0xFF;
                action.command = CPSS_PACKET_CMD_FORWARD_E;
                action.flowId = ingressEport1;
                action.sourceEPortAssignmentEnable = GT_TRUE;
                action.sourceEPort = 111;
                osPrintf("Set flow ID %d\n",action.flowId);
                rc = cpssDxChTtiRuleSet(dev, tcamIndex1+20, ruleType, &pattern, &mask,
                                        &action);
            /* in order to see FDB learning on assigned port, make additional settings */
                rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, 111, GT_FALSE, CPSS_LOCK_FRWRD_E);
                if(rc != GT_OK)
                    return rc;

                rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, 111, GT_TRUE);
                if(rc != GT_OK)
                    return rc;
            }
#endif
            break;

        case 2:

            pattern.udbArray.udb[0] = (GT_U8)(port1+i);
            mask.udbArray.udb[0] = 0xFF;
            action.sourceEPortAssignmentEnable = GT_TRUE;
            action.sourceEPort = ingressEport1 + i*step;
            rc = cpssDxChTtiRuleSet(devNum, tcamIndex1+i, ruleType, &pattern, &mask,
                                    &action);
            break;

        default:
            osPrintf("unsuported TM scenario\n");
            return 1;
        }


        if(rc != GT_OK)
            return rc;

    /* in order to see FDB learning on assigned port, make additional settings */
        rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, ingressEport1 + i*step, GT_FALSE, CPSS_LOCK_FRWRD_E);
        if(rc != GT_OK)
            return rc;

        rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, ingressEport1 + i*step, GT_TRUE);
        if(rc != GT_OK)
            return rc;

        /* make these ePorts enabled for policy */
        rc = cpssDxChPclPortIngressPolicyEnable(devNum, ingressEport1 + i*step, GT_TRUE);
        if(rc != GT_OK)
            return rc;


    }
    return rc;

}


/* use scenario5Set 0*/
GT_STATUS appDemoTmScenario5Set
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_PORT_NUM          ePort1,
    IN GT_BOOL              doDrop,
    IN GT_U32               marker
)
{
    GT_STATUS rc;
    GT_U32                          i,bitIndex;
    CPSS_MAC_ENTRY_EXT_STC          macEntry;
    /*CPSS_DXCH_COS_PROFILE_STC     cosProfile;*/
    CPSS_INTERFACE_INFO_STC         physicalInfoPtr;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;

    if(ePort1%4 != 1)
    {
        osPrintf ("ePort1 must be equal to 4*n+1\n");
        return GT_BAD_PARAM;
    }

    if(marker >25 )
    {
        osPrintf ("restrict marker to 24 \n");
        return GT_BAD_PARAM;
    }


    /*
    We create configuration for 4 flows, each mapped to a different dedicated Q-node.
    We fill FDB table with 4 MAC entries, all on VLAN 1.
    */
    /* set MAC entries for different ePorts, all ePorts here are mapped to port 1*/
    osMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));
    macEntry.daRoute = GT_FALSE;
    macEntry.isStatic = GT_TRUE;
    macEntry.key.key.macVlan.macAddr.arEther[0] = 0;
    macEntry.key.key.macVlan.macAddr.arEther[1] = 0;
    macEntry.key.key.macVlan.macAddr.arEther[2] = 0;
    macEntry.key.key.macVlan.macAddr.arEther[3] = 0x00;
    macEntry.key.key.macVlan.macAddr.arEther[4] = (GT_U8)marker;
    macEntry.key.key.macVlan.macAddr.arEther[5] = 0x11;
    macEntry.key.key.macVlan.vlanId = 1;
    macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.dstInterface.type = CPSS_INTERFACE_PORT_E;
    macEntry.dstInterface.devPort.hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);
    macEntry.dstInterface.devPort.portNum = ePort1;

    /* add 4 MAC entries */
    for(i=0;i<4;i++)
    {
        macEntry.key.key.macVlan.macAddr.arEther[5] = (GT_U8)(0x11+i);
        macEntry.dstInterface.devPort.portNum = ePort1+i; /* ePorts 200 to 399 are mapped to physical port 1*/

        do
        {
            rc = cpssDxChBrgFdbMacEntrySet(devNum, &macEntry);
            if(rc != GT_OK)
            {
                osPrintf("MAC creaton failed rc = %d\n",rc);
                if(rc != GT_BAD_STATE)
                    return rc;
                osTimerWkAfter(10);
            }
        }while(rc != GT_OK);
    }

    for (i=0; i<4; i++)
    {
        physicalInfoPtr.devPort.hwDevNum= PRV_CPSS_HW_DEV_NUM_MAC(devNum);
        physicalInfoPtr.devPort.portNum = portNum;
        physicalInfoPtr.type = CPSS_INTERFACE_PORT_E;
        rc= cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum ,ePort1+i ,&physicalInfoPtr);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    entry.queueIdBase = 0; /* will use it for full Q-remapping to a different physical port*/

    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        entry.bitSelectArr[bitIndex].selectType = CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TARGET_EPORT_E;
        entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
    }

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum, &entry);
    if(rc != GT_OK)
        return rc;

    if(doDrop == GT_TRUE)
        rc = appDemoTmDropMaskSet(devNum, 0, 0x3FF, GT_FALSE, GT_FALSE);
    else
        rc = appDemoTmDropMaskSet(devNum, 0, 0x3, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;

    if(rc != GT_OK)
        return rc;

    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, ePort1,1+marker*4,4);
    if( GT_OK != rc)
       return rc;

    return GT_OK;
}


/* use scenario6Set 0,3*/
/*
to set all 12 queues according to Scenario 6 requirements

    updateNodePrioWeight 0,9,0,2
    updateNodePrioWeight 0,10,0,3
    updateNodePrioWeight 0,11,0,5
    updateNodePrioWeight 0,12,5,2

    updateNodePrioWeight 0,13,0,1
    updateNodePrioWeight 0,14,0,2
    updateNodePrioWeight 0,15,0,3
    updateNodePrioWeight 0,16,5,2

    updateNodePrioWeight 0,17,0,5
    updateNodePrioWeight 0,18,0,3
    updateNodePrioWeight 0,19,0,2
    updateNodePrioWeight 0,20,5,2

to set all 3 A-nodes according to Scenario 6 requirements

    updateNodePrioWeight 1,2,18,1
    updateNodePrioWeight 1,3,10,2
    updateNodePrioWeight 1,4,11,2

to set all 2 B-nodes according to Scenario 6 requirements

    updateNodePrioWeight 2,1,10,1
    updateNodePrioWeight 2,2,19,2

set shapers
    appDemoTmNodeShaperEnable 0,0,12,1,1
    appDemoTmNodeShaperEnable 0, 0,16,1,1
    appDemoTmNodeShaperEnable 0, 0,20,1,1
    updateShaper 0,1,100000,8,0,0

    appDemoTmNodeShaperEnable 0, 1,2,1,11
    updateShaper 1,11,100000,16,0,0

    appDemoTmNodeShaperEnable 0, 2,2,1,21
    updateShaper 2,21,200000,26,0,0

    updateShaper 4,0,800000,46,0,0

*/
GT_STATUS appDemoTmScenario6Set
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                dropMask
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_VLAN_INFO_STC     cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U16                          i, bitIndex;
    GT_U16                          vlanId;
    CPSS_DXCH_COS_PROFILE_STC       cosProfile;
    /*CPSS_INTERFACE_INFO_STC         physicalInfoPtr;*/
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;

    /* Fill Vlan info for additonal VLAN */
    osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    osMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;
    cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    cpssVlanInfo.ucastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.mcastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.floodVidx              = 0xfff;
    cpssVlanInfo.unregIpmEVidx          = 0xfff;

    for(vlanId=1; vlanId<=10; vlanId++)
    {
        cpssVlanInfo.fidValue               = vlanId;
        rc = cpssDxChBrgVlanEntryWrite(devNum,vlanId,&portsMembers,&portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);
        if (rc != GT_OK)
            return rc;

/* for simplicity we add just one port to the VLAN, so we won;t get multiple copies of packet to this port */
        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, GT_TRUE,CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
        if (rc != GT_OK)
            return rc;
    }

/* create default FlowId for each vlan and ingress port */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        rc = TTIRuleSet(devNum, 9612, 0, 0, 1, GT_FALSE, 7);
    else
        rc = TTIRuleSet(devNum, 18432, 0, 0, 1, GT_FALSE, 6);
    if( GT_OK != rc)
      return rc;

    /* TC assignment will be based on UP*/
    for(i=1; i<10; i++)
    {
        rc = cpssDxChCosPortQosTrustModeSet(devNum, i, CPSS_QOS_PORT_TRUST_L2_E);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosPortTrustQosMappingTableIndexSet(devNum, i, GT_FALSE, 6);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosL2TrustModeVlanTagSelectSet(devNum, i, GT_TRUE, CPSS_VLAN_TAG0_E);
        if( GT_OK != rc)
           return rc;
    }

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    for(i=0; i<8; i++)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 0, 40+i);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 1, 40+i);
        if(rc != GT_OK)
            return rc;

        cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority = 5;
        cosProfile.trafficClass = i;
        rc = cpssDxChCosProfileEntrySet(devNum, 40+i, &cosProfile);
        if( GT_OK != rc)
           return rc;
/* just to be sure ...*/
/*      rc = cpssDxChCosProfileEntrySet(dev, i, &cosProfile);
        if( GT_OK != rc)
           return rc;*/
    }

    entry.queueIdBase = 1;
    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        GT_U8 bitOffset = 2;
        if(bitIndex < bitOffset)
        {
            /* Bits[0...1] - TM TC select type */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        else
        {
            /* Bits[2...13] - Flow ID */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_FLOW_ID_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex - bitOffset;
        }
    }

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum, &entry);
    if(rc != GT_OK)
        return rc;
/*
    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum+1, &entry);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum+2, &entry);
    if(rc != GT_OK)
        return rc;
*/
    rc = appDemoTmDropMaskSet(devNum, 0, dropMask, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;

    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, 0, 0, 80);
    if( GT_OK != rc)
       return rc;

    return GT_OK;
}

/* use scenario7Set 0,3*/
/* use the following set of commands to do the rest configuration of scenario 7

set TD profiles and WRED curves

    appDemoTmWREDCurveCreate 0,20
    appDemoTmWREDCurveCreate 0, 40
    appDemoTmWREDCurveCreate 0, 60
    appDemoTmWREDCurveCreate 0, 80

    appDemoTmDropProfileCreate 0,0,2,500,500,0,0
    appDemoTmDropProfileCreate 0, 0,0,500,500,1,0
    appDemoTmDropProfileCreate 0, 0,0,500,500,1,7
    appDemoTmDropProfileCreate 0, 0,0,500,500,2,0
    appDemoTmDropProfileCreate 0, 0,0,500,500,3,0
    appDemoTmDropProfileCreate 0, 0,0,500,500,4,0
    appDemoTmDropProfileCreate 0, 0,0,500,500,4,7
    appDemoTmDropProfileCreate 0, 0,0,500,500,4,14

check that setting worked as expected

    appDemoTmDropProfilePrint 0,0,7,0

assign TD profiles

    appDemoTmNodeDropProfileSet 0,0,24,1,8
    appDemoTmNodeDropProfileSet 0,0,23,1,7
    appDemoTmNodeDropProfileSet 0,0,22,1,6
    appDemoTmNodeDropProfileSet 0,0,21,1,5
    appDemoTmNodeDropProfileSet 0,0,20,1,4
    appDemoTmNodeDropProfileSet 0,0,19,1,3
    appDemoTmNodeDropProfileSet 0,0,18,1,2
    appDemoTmNodeDropProfileSet 0,0,17,1,1

demonstrate how WRED works - disable a queue node, send packets, read node counters
see how many packets are dropped, you may return on this several times, read DRAM
enable queue, see that all packets passed, read mode counter again

    appDemoTmNodePrioWeightUpdate 0,24,63,1

here send traffic burst - around 500 packets - to Q 24

    appDemoTmQCountersGet 0,0
    appDemoTmTMMemUseGet 0

    appDemoTmNodePrioWeightUpdate 0,24,0,1
*/
GT_STATUS appDemoTmScenario7Set
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                dropMask
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_VLAN_INFO_STC     cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U32                          i, bitIndex;
    GT_U16                          vlanId;
    CPSS_DXCH_COS_PROFILE_STC       cosProfile;
    /*CPSS_INTERFACE_INFO_STC       physicalInfoPtr;*/
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;

    /* Fill Vlan info for additonal VLAN */
    osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    osMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;
    cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    cpssVlanInfo.ucastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.mcastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.floodVidx              = 0xfff;
    cpssVlanInfo.unregIpmEVidx          = 0xfff;

    /* configure S-vlans (outer)*/
    for(vlanId=200; vlanId<=800; vlanId += 200)
    {
        cpssVlanInfo.fidValue               = vlanId;
        rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId,&portsMembers, &portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);
        if (rc != GT_OK)
            return rc;

/* for simplicity we add just one port to the VLAN, so we won't get multiple copies of packet to this port */
        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, GT_TRUE,CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E);
        if (rc != GT_OK)
            return rc;

    }

/* create default FlowId for each vlan and ingress port */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        rc = TTIRuleSet(devNum, 9612+30, portNum, 0, 1, GT_FALSE, 7);
    else
        rc = TTIRuleSet(devNum, 24000, portNum, 0, 1, GT_FALSE, 7);

    if( GT_OK != rc)
      return rc;

    /* TC assignment will be based on UP*/
    for(i=1; i<10; i++)
    {
/*      rc = cpssDxChBrgVlanPortVidSet(devNum, i, CPSS_DIRECTION_INGRESS_E, i+1);
        if (rc != GT_OK)
            return rc;
        rc = cpssDxChBrgVlanMemberAdd(devNum, i+1, i, GT_TRUE,CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
        if (rc != GT_OK)
            return rc;
*/
        rc = cpssDxChCosPortQosTrustModeSet(devNum, i, CPSS_QOS_PORT_TRUST_L2_E);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosPortTrustQosMappingTableIndexSet(devNum, i, GT_FALSE, 6);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosL2TrustModeVlanTagSelectSet(devNum, i, GT_TRUE, CPSS_VLAN_TAG0_E);
        if( GT_OK != rc)
           return rc;
    }

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    for(i=0; i<8; i++)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 0, 40+i);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 1, 40+i);
        if(rc != GT_OK)
            return rc;

        cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority = 5;
        cosProfile.trafficClass = i;
        rc = cpssDxChCosProfileEntrySet(devNum, 40+i, &cosProfile);
        if( GT_OK != rc)
           return rc;
/* just to be sure ...*/
/*      rc = cpssDxChCosProfileEntrySet(devNum, i, &cosProfile);
        if( GT_OK != rc)
           return rc;*/
    }

    entry.queueIdBase = 1;
    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        GT_U8 bitOffset = 3;
        if(bitIndex < bitOffset)
        {
            /* Bits[0...2] - TM TC select type */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        else
        {
            /* Bits[3...13] - Flow ID */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_FLOW_ID_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex - bitOffset;
        }
    }

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum, &entry);
    if(rc != GT_OK)
        return rc;

    rc = appDemoTmDropMaskSet(devNum, 0, dropMask, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;

    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, 0, 0, 80);
    if( GT_OK != rc)
       return rc;

    return GT_OK;
}


/* @internal appDemoTmScenarioBISTSet function
* @endinternal
*
* @brief   Configure TM related entities for external DRAM (BIST) test.
*
* @note   APPLICABLE DEVICES:      Caelum;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum       - device number
* @param[in] dstPort      - destination port, TM enabled.
* @param[in] qNodeIndex   - index of qNode to be used in teh BIST test
* @param[in] dropMask     - TM drop mask used in TM drop profile.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on general failure
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Used together with appDemoTmDramMemoryTestRun function

*/
GT_STATUS appDemoTmScenarioBISTSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM dstPort,
    IN GT_U32               qNodeIndex,
    IN GT_32                dropMask
)
{
    GT_STATUS                       rc;
    GT_U32                          bitIndex;
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;
    GT_U32                          testBit=0x1;
    CPSS_PORTS_BMP_STC              portsBmp;

    firstTtiRule = appDemoDxChTcamTtiBaseIndexGet(devNum, 0);
    osPrintf("Use TTI rule %d\n", firstTtiRule);


/* create default FlowId for each vlan and ingress port */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        rc = TTIRuleSet(devNum, firstTtiRule, dstPort, 0, 1, GT_FALSE, CPSS_TM_BIST);
    else
        rc = TTIRuleSet(devNum, 24000, dstPort, 0, 1, GT_FALSE, CPSS_TM_BIST);

    if( GT_OK != rc)
      return rc;

    entry.queueIdBase = 0;

    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        if((qNodeIndex & testBit) == 0)
        {
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        else
        {
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ONE_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        testBit = testBit<<1;
    }

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, dstPort, &entry);
    if(rc != GT_OK)
        return rc;

    rc = appDemoTmDropMaskSet(devNum, 0, dropMask, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;

    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, 0, 0, 80);
    if( GT_OK != rc)
       return rc;

    queueNum = qNodeIndex;


/* prepare for packet send from CPU */

    rc = cpssEventDeviceMaskSet(devNum, CPSS_PP_TX_BUFFER_QUEUE_E, CPSS_EVENT_UNMASK_E);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = cpssDxChBrgGenUcLocalSwitchingEnable(devNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }


    rc = cpssDxChCscdPortBridgeBypassEnableSet(devNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* in order to see FDB learning on CPU port, make additional settings */
    rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, CPSS_CPU_PORT_NUM_CNS, GT_FALSE, CPSS_LOCK_FRWRD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, CPSS_CPU_PORT_NUM_CNS, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* if we use own device for srcDev arguement we don't want it to be filtered out*/
    rc = cpssDxChCscdDsaSrcDevFilterSet(devNum, GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* end prepare for packet send from CPU */

    /* for Caelum we set ports 56-59, as the ports that can be set for loopback */
    cpssOsMemSet(&portsBmp, 0, sizeof(portsBmp));
    CPSS_PORTS_BMP_PORT_SET_MAC(&portsBmp, dstPort);

    rc = cpssDxChPortModeSpeedSet(devNum, &portsBmp, GT_TRUE, CPSS_PORT_INTERFACE_MODE_KR_E, CPSS_PORT_SPEED_10000_E);
    if (rc != GT_OK)
        return rc;

    /* in milliseconds */
    osTimerWkAfter(10);

    /* set loopback port, both to ensure that the port is up and enable loopback traffic */
    rc = cpssDxChPortInternalLoopbackEnableSet(devNum, dstPort, GT_TRUE);
    if (rc != GT_OK)
        return rc;
    /* alternative - cpssDxChPortSerdesLoopbackModeSet, cpssDxChSamplePortManagerLoopbackSet  */


    return GT_OK;
}


/* use scenario8Set 0,3*/
GT_STATUS appDemoTmScenario8Set
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                dropMask
)
{
    GT_STATUS rc;
    CPSS_DXCH_BRG_VLAN_INFO_STC     cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;
    GT_U32                          i, bitIndex;
    GT_U16                          vlanId;
    CPSS_DXCH_COS_PROFILE_STC       cosProfile;
    /*CPSS_INTERFACE_INFO_STC       physicalInfoPtr;*/
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;
    CPSS_DXCH_PORT_MAP_STC     portMap[3];
    GT_U32                      queueId;

    /* Fill Vlan info for additonal VLAN */
    osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    osMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;
    cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    cpssVlanInfo.ucastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.mcastLocalSwitchingEn  = GT_TRUE;
    cpssVlanInfo.floodVidx              = 0xfff;
    cpssVlanInfo.unregIpmEVidx          = 0xfff;

    /* configure vlans*/
    for(vlanId=2; vlanId<=1000; vlanId++)
    {
        cpssVlanInfo.fidValue               = vlanId;
        rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &portsMembers,&portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);
        if (rc != GT_OK)
            return rc;

/* for simplicity we add just one port to the VLAN, so we won;t get multiple copies of packet to this port */
        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, portNum, GT_TRUE,CPSS_DXCH_BRG_VLAN_PORT_TAG1_CMD_E);
        if (rc != GT_OK)
            return rc;

    }

/* create default FlowId for each vlan */
    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
        rc = TTIRuleSet(devNum, 9612, 0, 0, 1, GT_FALSE, 8);
    else
        rc = TTIRuleSet(devNum, 20000, 0, 0, 1, GT_FALSE, 8);

    if( GT_OK != rc)
      return rc;

    /* TC assignment will be based on UP*/
    for(i=1; i<10; i++)
    {
        rc = cpssDxChCosPortQosTrustModeSet(devNum, i, CPSS_QOS_PORT_TRUST_L2_E);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosPortTrustQosMappingTableIndexSet(devNum, i, GT_FALSE, 6);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosL2TrustModeVlanTagSelectSet(devNum, i, GT_TRUE, CPSS_VLAN_TAG0_E);
        if( GT_OK != rc)
           return rc;
    }

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    for(i=0; i<8; i++)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 0, 40+i);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 1, 40+i);
        if(rc != GT_OK)
            return rc;

        cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority = 5;
        cosProfile.trafficClass = i;
        rc = cpssDxChCosProfileEntrySet(devNum, 40+i, &cosProfile);
        if( GT_OK != rc)
           return rc;
    }

    entry.queueIdBase = 1;
    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        if(bitIndex < 3)
        {
            /* Bits[0...2] - TM TC select type */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        else
        {
            /* Bits[3...13] - Flow ID */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_FLOW_ID_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex - 3;
        }
    }

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum, &entry);
    if(rc != GT_OK)
        return rc;

    /* make 3 ports above portNum TM-disabled */
    osMemSet(portMap, 0, sizeof(portMap));
    for(i=0;i<3;i++)
    {

        portMap[i].tmEnable = GT_FALSE;
        portMap[i].physicalPortNumber = portNum + 1 + i;
        portMap[i].mappingType  = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
        portMap[i].tmPortInd        = 0; /* do we care? */
        portMap[i].interfaceNum     = portNum + 1 + i;/* it represents the mapping of TM port to physical port for egress*/
        portMap[i].txqPortNumber    = portNum + 1 + i;
        portMap[i].portGroup    = 0;

        /* check if the table still affect behavior */
        entry.queueIdBase = 1;
        for (bitIndex = 0; bitIndex < 14; bitIndex++)
        {
            if(bitIndex < 14)
            {
                entry.bitSelectArr[bitIndex].selectType =
                    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E;
                entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
            }

        }

        rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum + 1 + i, &entry);
        if(rc != GT_OK)
            return rc;
    }
/*  rc = cpssDxChPortPhysicalPortMapSet(devNum,3,portMap);
    if(rc != GT_OK)
        return rc;
*/

    rc = appDemoTmDropMaskSet(devNum, 0, dropMask, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;

/* configure attributes of TM tree - here not in script because of ammount of A-nodes */

    for(vlanId=1; vlanId<=1000; vlanId++)
    {
        for(i=0; i<8; i++)
        {
            queueId = 1+(vlanId-1)*8+i;

            rc = appDemoTmShaperUpdate(devNum, CPSS_TM_LEVEL_Q_E, queueId, (i+1)*10000,  20 + vlanId, 0, 0);

            if (rc != GT_OK)
                return rc;

/* queueId is equal to shaping profile ID */
            rc = appDemoTmNodeShaperEnable(devNum, CPSS_TM_LEVEL_Q_E, queueId, GT_TRUE, queueId);
            if (rc != GT_OK)
                return rc;

            rc = appDemoTmNodePrioWeightUpdate(devNum, CPSS_TM_LEVEL_Q_E, queueId, (CPSS_TM_ELIG_FUNC_NODE_ENT)CPSS_TM_ELIG_Q_SHP_SCHED00_PROP00, 1);
            if (rc != GT_OK)
                return rc;
        }
    }

    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, 0, 0, 80);
    if( GT_OK != rc)
       return rc;

    return GT_OK;
}

/*
demonstrate the aging and delay feature with simple configuration, use in conjunction with scenarios above, such as  scenario6Set 0,3
use as agingScenarioSet 16, 1000
*/
GT_STATUS appDemoTmAgingScenarioSet
(
    IN GT_U8            devNum,
    IN GT_U32           qIndex,
    IN GT_U32           highestBW
)
{
    GT_STATUS rc=GT_OK;
    GT_U32                                              resolution = 1000, i;
    CPSS_TM_DROP_PROFILE_PARAMS_STC                     profileArr[CPSS_TM_AGING_PROFILES_CNS];
    GT_U32                                              agingBlockIndex;
    GT_U32                                              agingProfileIndex = 1;
    CPSS_DXCH_TM_AGING_AND_DELAY_PROFILE_THRESHOLDS_STC thresholds;

    for(i = 0; i<CPSS_TM_AGING_PROFILES_CNS; i++)
    {
        osMemSet(&(profileArr[i]), 0, sizeof(CPSS_TM_DROP_PROFILE_PARAMS_STC));
        profileArr[i].dropMode =  CPSS_TM_DROP_MODE_CB_TD_ONLY_E;
        profileArr[i].cbTdThresholdBytes = cpssTmDropConvertBWToBytes(highestBW/(i+1));

    }
    rc = cpssTmDropProfileAgingBlockCreate(devNum, profileArr, &agingBlockIndex);
    if( GT_OK != rc)
       return rc;
    osPrintf("Aging Block created with index %d \n", agingBlockIndex);

    rc = appDemoTmNodeDropProfileSet(devNum, CPSS_TM_LEVEL_Q_E, qIndex, GT_TRUE, agingBlockIndex);
    if( GT_OK != rc)
       return rc;

    rc = cpssDxChTmGlueAgingAndDelayQueueAgingProfileIdSet(devNum, qIndex, agingProfileIndex);
    if( GT_OK != rc)
       return rc;


    rc = cpssTmAgingChangeStatus(devNum, 1);
    if( GT_OK != rc)
       return rc;

    rc = cpssDxChTmGlueAgingAndDelayTimerResolutionSet(devNum, &resolution);
    if( GT_OK != rc)
       return rc;

    osPrintf("Aging Timer resolution set to %d nanoseconds\n", resolution);

    thresholds.threshold0 = 1000;
    thresholds.threshold1 = 2000;
    thresholds.threshold2 = 3000;

    rc = cpssDxChTmGlueAgingAndDelayAgingProfileThresholdsSet(devNum, agingProfileIndex, &thresholds);
    if( GT_OK != rc)
       return rc;

    rc = cpssDxChTmGlueAgingAndDelayDropAgedPacketEnableSet(devNum, GT_TRUE);
    if( GT_OK != rc)
       return rc;


    rc = cpssDxChTmGlueAgingAndDelayAgedPacketsCountingEnableSet(devNum, GT_TRUE);
    if( GT_OK != rc)
       return rc;


    return GT_OK;
}

/* use scenario9Set 0,3 - scenario for PFC demo, pfcIngressPort=0 is where PFC packet enters the board, portNum is where we send TM traffic, that
    should stop egressing on port 0, when Xoff is received on port 0
    PFC send rate 100 packets/second
    */
GT_STATUS appDemoTmScenario9Set
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_32                dropMask
)
{
    GT_STATUS rc;
    GT_U32                          i, bitIndex;
    CPSS_DXCH_COS_PROFILE_STC       cosProfile;
    /*CPSS_INTERFACE_INFO_STC       physicalInfoPtr;*/
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;
    GT_PHYSICAL_PORT_NUM            pfcPortNum = 1, pfcIngressPort = 0;


    /* TC assignment will be based on UP*/
    for(i=1; i<10; i++)
    {
        rc = cpssDxChCosPortQosTrustModeSet(devNum, i, CPSS_QOS_PORT_TRUST_L2_E);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosPortTrustQosMappingTableIndexSet(devNum, i, GT_FALSE, 6);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosL2TrustModeVlanTagSelectSet(devNum, i, GT_TRUE, CPSS_VLAN_TAG0_E);
        if( GT_OK != rc)
           return rc;
    }

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    for(i=0; i<8; i++)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 0, 40+i);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 1, 40+i);
        if(rc != GT_OK)
            return rc;

        cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority = 5;
        cosProfile.trafficClass = i;
        rc = cpssDxChCosProfileEntrySet(devNum, 40+i, &cosProfile);
        if( GT_OK != rc)
           return rc;
    }

    entry.queueIdBase = 1;
    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        if(bitIndex < 3)
        {
            /* Bits[0...2] - TM TC select type */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        else
        {
            /* Bits[3...13] - Flow ID */
            entry.bitSelectArr[bitIndex].selectType = CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_LOCAL_TARGET_PHY_PORT_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex - 3;
        }
    }

    rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, portNum, &entry);
    if(rc != GT_OK)
        return rc;

    rc = appDemoTmDropMaskSet(devNum, 0, dropMask, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;

/* configure PFC TM glue layer */
    rc = cpssDxChTmGluePfcResponseModeSet(devNum, 0, CPSS_DXCH_TM_GLUE_PFC_RESPONSE_MODE_TM_E);
    if(rc != GT_OK)
        return rc;

/* we use just one port for demo */
    rc = cpssDxChTmGluePfcPortMappingSet(devNum, pfcIngressPort, pfcPortNum);
    if(rc != GT_OK)
        return rc;
/*  PFC port + tc   mapping to C-node should be in agreement with packet mapping to Q-node, where
    C-node is Q-node ancestor */
    /* C-nodes start from 0 */
    for(i=0;i<8;i++)
    {
        rc = cpssDxChTmGluePfcTmTcPort2CNodeSet(devNum, pfcPortNum, i, i);
        if(rc != GT_OK)
            return rc;
    }

/* configure PFC */
    rc = cpssDxChPortPfcEnableSet(devNum, CPSS_DXCH_PORT_PFC_ENABLE_TRIGGERING_AND_RESPONSE_E);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChPortPfcForwardEnableSet(devNum, pfcIngressPort, GT_TRUE);
    if(rc != GT_OK)
        return rc;
    rc = cpssDxChPortFlowControlModeSet(devNum, pfcIngressPort, CPSS_DXCH_PORT_FC_MODE_PFC_E);
    if(rc != GT_OK)
        return rc;

/* maybe should get the speed of the physical port, but that's just an example */
    rc = cpssDxChTmGlueFlowControlPortSpeedSet(devNum, pfcPortNum, CPSS_PORT_SPEED_1000_E);
    if(rc != GT_OK)
        return rc;

    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, 0, 0, 80);
    if( GT_OK != rc)
       return rc;

    return GT_OK;
}


GT_STATUS appDemoTmScenarioRev4Set
(
    IN GT_U8                devNum,
    IN GT_32                dropMask
)
{
    GT_STATUS rc;
    GT_U32                          i, bitIndex;
    GT_U16                          vlanId;
    CPSS_DXCH_COS_PROFILE_STC       cosProfile;
    /*CPSS_INTERFACE_INFO_STC       physicalInfoPtr;*/
    CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_ENTRY_STC    entry;
    CPSS_PORTS_BMP_STC              portsMembers;
    CPSS_PORTS_BMP_STC              portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC     cpssVlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC  portsTaggingCmd;

    /* create 3 VLANs, such that only two ports - 64 and another are in each VLAN */
    osMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    osMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    cpssVlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.floodVidx      = 0xFFF;
    cpssVlanInfo.floodVidxMode  = CPSS_DXCH_BRG_VLAN_FLOOD_VIDX_MODE_ALL_FLOODED_TRAFFIC_E;
    cpssVlanInfo.naMsgToCpuEn   = GT_TRUE;
    cpssVlanInfo.unregIpmEVidx = 0xFFF;

    for(i = 0; i < 4; i++)
    {
        vlanId = (GT_U16)(2+i);
        cpssVlanInfo.fidValue = vlanId; /* it's a MUST!!!*/

        rc = cpssDxChBrgVlanEntryWrite(devNum, vlanId, &portsMembers, &portsTagging, &cpssVlanInfo, &portsTaggingCmd);
        if( GT_OK != rc)
          return rc;

        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, 64, GT_TRUE, CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
        if( GT_OK != rc)
           return rc;

        rc = cpssDxChBrgVlanMemberAdd(devNum, vlanId, 64+i, GT_TRUE, CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E);
        if( GT_OK != rc)
           return rc;

        rc = cpssDxChBrgVlanPortVidSet(devNum, 64+i, CPSS_DIRECTION_INGRESS_E, vlanId);
        if( GT_OK != rc)
           return rc;
    }

    /* TC assignment will be based on UP*/
    for(i=0; i<4; i++)
    {
        rc = cpssDxChCosPortQosTrustModeSet(devNum, 64+i, CPSS_QOS_PORT_TRUST_L2_E);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosPortTrustQosMappingTableIndexSet(devNum, 64+i, GT_FALSE, 6);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosL2TrustModeVlanTagSelectSet(devNum, 64+i, GT_TRUE, CPSS_VLAN_TAG0_E);
        if( GT_OK != rc)
           return rc;
    }

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    for(i=0; i<8; i++)
    {
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 0, 40+i);
        if(rc != GT_OK)
            return rc;
        rc = cpssDxChCosUpCfiDeiToProfileMapSet(devNum, 6, 0, (GT_U8)i, 1, 40+i);
        if(rc != GT_OK)
            return rc;

        cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
        cosProfile.userPriority = 5;
        cosProfile.trafficClass = i;
        rc = cpssDxChCosProfileEntrySet(devNum, 40+i, &cosProfile);
        if( GT_OK != rc)
           return rc;
    }

    entry.queueIdBase = 1;
    for (bitIndex = 0; bitIndex < 14; bitIndex++)
    {
        if(bitIndex < 3)
        {
            /* Bits[0...2] - TM TC select type */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_TM_TC_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex;
        }
        else
        {
            /* Bits[3...13] - 0 */
            entry.bitSelectArr[bitIndex].selectType =
                CPSS_DXCH_TM_GLUE_QUEUE_MAP_BIT_SELECT_TYPE_ZERO_E;
            entry.bitSelectArr[bitIndex].bitSelector = bitIndex - 3;
        }
    }

    for(i=0;i<4;i++)
    {
        rc = cpssDxChTmGlueQueueMapBitSelectTableEntrySet(devNum, 64+i, &entry);
        if(rc != GT_OK)
            return rc;
    }

    rc = appDemoTmDropMaskSet(devNum, 0, dropMask, GT_FALSE, GT_FALSE);
    if(rc != GT_OK)
        return rc;


    /* at the end - configure counters*/
    rc = appDemoTmQCountersConfig(devNum);
    if( GT_OK != rc)
       return rc;

    rc = appDemoTmNQueuesCountSet(devNum, 0, 0, 80);
    if( GT_OK != rc)
       return rc;

    return GT_OK;
}




/* change LAD params settings (relevant before cpssInitSystem)
   and change number of active LAD Params. (relevant after cpssInitSystem) */

extern GT_STATUS setDRAMIntNumber(GT_U32 intNum);

GT_STATUS appDemoTmLadParamsSet
(
    IN GT_U8  devNum,
    IN GT_U8  numOfLads,
    IN GT_U32 minPkgSize,
    IN GT_U32 pagesPerBank,
    IN GT_U32 pkgesPerBank
)
{
    GT_STATUS rc = GT_OK;
    CPSS_TM_CTL_LAD_INF_PARAM_STC ladParams;

    osMemSet(&ladParams, 0, sizeof(ladParams));

    ladParams.minPkgSize = minPkgSize;
    ladParams.pagesPerBank = pagesPerBank;
    ladParams.pkgesPerBank = pkgesPerBank;

    rc = cpssTmCtlLadParamsSet(devNum, numOfLads, &ladParams);
    if( GT_OK != rc)
       return rc;

    setDRAMIntNumber(numOfLads);

    return rc;
}

GT_STATUS appDemoTmLadParamsDump
(
    IN GT_U8  devNum,
    IN GT_U8  numOfLads
)
{
    GT_STATUS rc = GT_OK;
    CPSS_TM_CTL_LAD_INF_PARAM_STC ladParams;

    osMemSet(&ladParams, 0, sizeof(ladParams));

    rc = cpssTmCtlLadParamsGet(devNum, numOfLads, &ladParams);

    if( GT_OK != rc)
       return rc;

    osPrintf("numOfLads:            0x%0x\n", numOfLads);
    osPrintf("minPkgSize:           0x%0x\n", ladParams.minPkgSize);
    osPrintf("pagesPerBank:         0x%0x\n", ladParams.pagesPerBank);
    osPrintf("pkgesPerBank:         0x%0x\n", ladParams.pkgesPerBank);
    osPrintf("portChunksEmitPerSel: 0x%0x\n", ladParams.portChunksEmitPerSel);

    return rc;
}

GT_STATUS appDemoTmLadParamsHWDump
(
    IN GT_U8  devNum
)
{
    GT_STATUS rc = GT_OK;
    CPSS_TM_CTL_LAD_INF_PARAM_STC ladParams;
    GT_U32 numOfLads;

    osMemSet(&ladParams, 0, sizeof(ladParams));

    rc = prvCpssTmCtlLadParamsGetHW(devNum, &numOfLads, &ladParams);

    if( GT_OK != rc)
        return rc;

    osPrintf("numOfLads:            0x%0x\n", numOfLads);
    osPrintf("minPkgSize:           0x%0x\n", ladParams.minPkgSize);
    osPrintf("pagesPerBank:         0x%0x\n", ladParams.pagesPerBank);
    osPrintf("pkgesPerBank:         0x%0x\n", ladParams.pkgesPerBank);
    osPrintf("portChunksEmitPerSel: 0x%0x\n", ladParams.portChunksEmitPerSel);

    return rc;
}

GT_STATUS appDemoTmUnitsErrorsGet
(
    IN GT_U8                               devNum,
    OUT CPSS_TM_UNITS_ERROR_STATUS_STC     *tmUnitsErrorPtr,
    OUT CPSS_TM_BAP_UNIT_ERROR_STATUS_STC  *tmBapErrorPtr,
    OUT GT_BOOL                            *errorOccuredPtr
)
{
    GT_STATUS   rc = GT_OK;
    int         i;

    rc = cpssTmUnitsErrorStatusGet(devNum, tmUnitsErrorPtr);
    if (rc != GT_OK)
    {
        osPrintf("cpssTmUnitsErrorStatusGet: error: %d\n", rc);
        return rc;
    }

    if (tmUnitsErrorPtr->qmngrStatus)
        *errorOccuredPtr = GT_TRUE;
    if (tmUnitsErrorPtr->dropStatus)
        *errorOccuredPtr = GT_TRUE;
    if (tmUnitsErrorPtr->schedStatus)
        *errorOccuredPtr = GT_TRUE;
    if (tmUnitsErrorPtr->rcbStatus)
        *errorOccuredPtr = GT_TRUE;

    for (i = 1; i < CPSS_TM_CTL_MAX_NUM_OF_BAPS_CNS; i++)
    {
        rc = cpssTmBapUnitErrorStatusGet(devNum, i, tmBapErrorPtr);
        if (rc != GT_OK)
        {
            if (rc == GT_NO_RESOURCE)
                continue;

            osPrintf("cpssTmBapUnitErrorStatusGet: bap: %d, error: %d\n", i, rc);
            return rc;
        }

        if (tmBapErrorPtr->bapStatus)
        {
            *errorOccuredPtr = GT_TRUE;
            break;
        }
    }

    return rc;
}

static void appDemoTmUnitsErrorsPrint
(
    CPSS_TM_UNITS_ERROR_STATUS_STC      *tmUnitsErrorPtr,
    CPSS_TM_BAP_UNIT_ERROR_STATUS_STC   *tmBapErrorPtr
)
{
    if (tmUnitsErrorPtr->qmngrStatus)
        osPrintf("qmngrStatus: 0x%0x\n", tmUnitsErrorPtr->qmngrStatus);
    if (tmUnitsErrorPtr->dropStatus)
        osPrintf("dropStatus:  0x%0x\n", tmUnitsErrorPtr->dropStatus);
    if (tmUnitsErrorPtr->schedStatus)
        osPrintf("schedStatus: 0x%0x\n", tmUnitsErrorPtr->schedStatus);
    if (tmUnitsErrorPtr->rcbStatus)
        osPrintf("rcbStatus:   0x%0x\n", tmUnitsErrorPtr->rcbStatus);

    if (tmBapErrorPtr->bapStatus)
        osPrintf("bap: %d bapStatus: 0x%0x\n", tmBapErrorPtr->bapNum, tmBapErrorPtr->bapStatus);

}

GT_STATUS appDemoTmUnitsErrorsDump(GT_U8 devNum)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_TM_UNITS_ERROR_STATUS_STC      tmUnitsError;
    CPSS_TM_BAP_UNIT_ERROR_STATUS_STC   tmBapError;
    GT_BOOL                             errorOccured;

    rc = appDemoTmUnitsErrorsGet(devNum,
                            &tmUnitsError,
                            &tmBapError,
                            &errorOccured);

    if (rc != GT_OK || errorOccured == GT_FALSE)
    {
        return rc;
    }

    appDemoTmUnitsErrorsPrint(&tmUnitsError, &tmBapError);

    return rc;
}

/* dump tm Units Errors
   only if error has occured or by dumpAlways - conditional flag */
GT_STATUS appDemoTmConditionalUnitsErrorsDump
(
    GT_U8 devNum,
    char *fileNamePtr,
    GT_32 line,
    GT_BOOL dumpAlways
)
{
    GT_STATUS                           rc = GT_OK;
    CPSS_TM_UNITS_ERROR_STATUS_STC      tmUnitsError;
    CPSS_TM_BAP_UNIT_ERROR_STATUS_STC   tmBapError;
    GT_BOOL                             errorOccured;

    rc = appDemoTmUnitsErrorsGet(devNum,
                            &tmUnitsError,
                            &tmBapError,
                            &errorOccured);

    if (rc != GT_OK)
    {
        return rc;
    }

    if (!errorOccured && !dumpAlways)
    {
        return rc;
    }

    osPrintf("%s: %d\n",
             (fileNamePtr != NULL) ? fileNamePtr : "",
             line);

    appDemoTmUnitsErrorsPrint(&tmUnitsError, &tmBapError);

    return rc;
}



GT_STATUS appDemoTmDramFreqDump(GT_U8 devNum)
{
    /* Read TM frequency */
    char    *freqNamePTR = 0;
    GT_U32  freqNum = 0;

    GT_STATUS rc = prvCpssDxChTMFreqGet(devNum, &freqNum);
    if (rc != GT_OK)
    {
        return rc;
    }

    switch(freqNum)
    {
        case 2:
            freqNamePTR = "CPSS_DRAM_FREQ_933_MHZ_E";
            break;
        case 3:
            freqNamePTR = "CPSS_DRAM_FREQ_667_MHZ_E";
            break;
        case 1:
            freqNamePTR = "CPSS_DRAM_FREQ_800_MHZ_E";
            break;
        case 0:
        default:
            freqNamePTR = "Unknown";
    }

    cpssOsPrintf("DRAM frequency: %d-%s\n", freqNum, freqNamePTR);

    return rc;
}


/* this function demonstrates CPU send of packet with a forward DSA tag.  The packet is directed to
ePort dstPort, also there is a hardcoded setting to physical port to 18, but also hardcoded isTrgPhyPortValid
to FALSE to ignore the physical port - relevant to BC2. For other devices packet directed to physical dstPort.
bypass arguement is to change the finality of egress interface as set by DSA tag, when bypass is FALSE
the forwarding decision is made by ingress pipe and not by DSA tag
use like:
shell-execute cpuTxFrwrd 0,56,200,0,100,1,0,0x55


*/
GT_STATUS cpuTxFrwrd(GT_U8 devNum, GT_PORT_NUM dstPort, GT_U32 vid, GT_BOOL tagged, GT_U32 pktSize, GT_U32 count, GT_U8 firstChar, GT_U8 fillChar)
{
    CPSS_DXCH_NET_DSA_FORWARD_STC *forwardPtr;
    GT_U8                       *buffList[1];
    CPSS_DXCH_NET_TX_PARAMS_STC dxChPcktParams;
    GT_U32                      buffLenList[1];
    GT_U32                      numOfBufs, j, i;
    GT_STATUS                   rc, rc0;
    GT_U32                      srcDev;
    GT_PORT_NUM                 srcPort=0;

    rc = cpssDxChBrgVlanLocalSwitchingEnableSet(devNum, vid, CPSS_DXCH_BRG_VLAN_LOCAL_SWITCHING_TRAFFIC_TYPE_KNOWN_UC_E,
        GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    srcDev = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    osMemSet(&dxChPcktParams, 0, sizeof(dxChPcktParams));
    dxChPcktParams.packetIsTagged                   = GT_FALSE;
    dxChPcktParams.sdmaInfo.recalcCrc               = GT_TRUE;
    dxChPcktParams.sdmaInfo.txQueue                 = 7;
    dxChPcktParams.sdmaInfo.evReqHndl               = 0;
    /* no sync - so no need */
    dxChPcktParams.sdmaInfo.invokeTxBufferQueueEvent = GT_FALSE;
    /* in eArch CPU port is cascade port of 4 word type, don't care for now about older devices */
    dxChPcktParams.dsaParam.commonParams.dsaTagType = CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT;

    dxChPcktParams.dsaParam.commonParams.vpt        = 0;
    dxChPcktParams.dsaParam.commonParams.cfiBit     = 0;
    dxChPcktParams.dsaParam.commonParams.vid        = vid;
    dxChPcktParams.dsaParam.commonParams.dropOnSource = GT_TRUE;
    dxChPcktParams.dsaParam.commonParams.packetIsLooped = GT_FALSE;
    dxChPcktParams.dsaParam.dsaType                 = CPSS_DXCH_NET_DSA_CMD_FORWARD_E;

    forwardPtr =                                      &dxChPcktParams.dsaParam.dsaInfo.forward;
    forwardPtr->dstInterface.type                   = CPSS_INTERFACE_PORT_E;
    forwardPtr->dstInterface.devPort.portNum        = dstPort;
    forwardPtr->dstInterface.devPort.hwDevNum       = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    if(tagged)
        forwardPtr->srcIsTagged         = CPSS_DXCH_BRG_VLAN_PORT_TAG0_CMD_E;
    else
        forwardPtr->srcIsTagged         = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;

    forwardPtr->srcHwDev            = srcDev;
    forwardPtr->srcIsTrunk          = GT_FALSE;
    forwardPtr->source.portNum      = srcPort;
    forwardPtr->srcId               = 0;
    forwardPtr->egrFilterRegistered = GT_FALSE;
    forwardPtr->wasRouted           = GT_FALSE;
    forwardPtr->qosProfileIndex     = 0;
    forwardPtr->isTrgPhyPortValid   = GT_TRUE;
    forwardPtr->dstEport            = dstPort;
    forwardPtr->tag0TpidIndex       = 0;
    forwardPtr->origSrcPhyIsTrunk   = GT_FALSE;
    forwardPtr->origSrcPhy.trunkId  = 0;
    forwardPtr->origSrcPhy.portNum  = 0;
    forwardPtr->phySrcMcFilterEnable= GT_FALSE;

    numOfBufs = 1;
    buffLenList[0] = pktSize;
    buffList[0] = cpssOsCacheDmaMalloc(buffLenList[0]*sizeof(GT_U8));
    buffList[0][0] = firstChar;
    for(i=1;i<pktSize;i++)
    {
        buffList[0][i] = fillChar;
    }

    for(j=0;j<count;j++)
    {
        rc = cpssDxChNetIfSdmaSyncTxPacketSend(devNum, &dxChPcktParams, buffList, buffLenList, numOfBufs);
        if(rc == GT_HW_ERROR)
        {
            /* in milliseconds, wait for own bit to be released, if it is the reason for the failure */
            osTimerWkAfter(10);
            rc0 = cpssDxChNetIfSdmaSyncTxPacketSend(devNum, &dxChPcktParams, buffList, buffLenList, numOfBufs);
            printf("\r send failed, try again: j = %d, rc = 0x%0X, rc0 = 0x%0X\n\r", j, rc, rc0);
            if (rc0 != GT_OK)
            {
                cpssOsCacheDmaFree(buffList[0]);
                return rc;
            }
        }
        else if(rc != GT_OK)
        {
            cpssOsCacheDmaFree(buffList[0]);
            return rc;
        }
    }

    cpssOsCacheDmaFree(buffList[0]);
    return rc;

}

static int time_gt(GT_U32 a_s, GT_U32 a_n, GT_U32 b_s, GT_U32 b_n)
{
    if (a_s > b_s)
        return 1;
    if (a_s < b_s)
        return 0;
    return (a_n > b_n) ? 1 : 0;
}
static void time_sub(GT_U32 a_s, GT_U32 a_n, GT_U32 b_s, GT_U32 b_n, GT_U32 *d_s, GT_U32 *d_n)
{
    if (time_gt(a_s, a_n, b_s, b_n))
    {
        *d_s = a_s - b_s;
        if (a_n >= b_n)
            *d_n = a_n - b_n;
        else
        {
            *d_n = 1000000000 + a_n - b_n;
            (*d_s)--;
        }
    } else
    {
        *d_s = 0;
        *d_n = 0;
    }
}

/*
* @internal appDemoTmTCAMCountersGet function
* @endinternal
*
* @brief   Read TTI counters relevant to the external DDR BIST test.
*
* @note   APPLICABLE DEVICES:      Caelum;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[out] crcCountPtr           - count of hits on CRC match TTI rule.
* @param[in] verbose                - if to print detailed information about the test run
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on general failure
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Used together with appDemoTmDramMemoryTestRun function

*/

GT_STATUS appDemoTmTCAMCountersGet
(
    IN GT_U8   devNum,
    OUT GT_U32 *crcCountPtr,
    IN GT_BOOL verbose
)
{
    CPSS_DXCH_CNC_COUNTER_STC counter;
    GT_STATUS rc;
    GT_U16  block=10;

    if(verbose)
        cpssOsPrintf("==================> appDemoTmTCAMCountersGet <============\n");
    rc = cpssDxChCncCounterGet(devNum, block, ttiCpuPortCounterIndex-block*1024, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    if( GT_OK != rc)
       return rc;

    if(verbose)
        cpssOsPrintf("TTI rule with counter %d hit %d times\n", ttiCpuPortCounterIndex,counter.packetCount.l[0] );
    rc = cpssDxChCncCounterGet(devNum, block, ttiCrcMatchCounterIndex-block*1024, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    if( GT_OK != rc)
       return rc;
    if(verbose)
        cpssOsPrintf("CRC match: TTI rule with counter %d hit %d times\n", ttiCrcMatchCounterIndex,counter.packetCount.l[0] );

    *crcCountPtr =  counter.packetCount.l[0];

    rc = cpssDxChCncCounterGet(devNum, block, ttiCrcNoMatchCounterIndex-block*1024, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &counter);
    if( GT_OK != rc)
       return rc;
    cpssOsPrintf("CRC NO match: TTI rule with counter %d hit %d times\n", ttiCrcNoMatchCounterIndex,counter.packetCount.l[0] );

    return GT_OK;
}



/* change CRC array to be used in the TM DRAM BIST test
   Use like shell-execute appDemoTmDramMemoryTestPrepare 0x55, 0x55, 0x55, 0x55 */

static GT_STATUS appDemoTmDramMemoryTestCRCSet(GT_U8 byte0, GT_U8 byte1, GT_U8 byte2, GT_U8 byte3)
{
    GT_STATUS rc=GT_OK;

    CRC_arr[0] = byte0;
    CRC_arr[1] = byte1;
    CRC_arr[2] = byte2;
    CRC_arr[3] = byte3;

    return rc;

}

/* @internal appDemoTmDramMemoryTestPrepare function
* @endinternal
*
* @brief   Configuration nessesary before running external DRAM verification test.
*
* @note   APPLICABLE DEVICES:      Caelum;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] dstPort                - destination port that is put in UP loopback state in order to redirect packets coming out of external DDR
*                                      back to TTI rul that checks packet's CRC.
* @param[in] CRCstartByte           - start byte of CRC in test packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on general failure
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Used together with appDemoTmDramMemoryTestRun function

*/
GT_STATUS appDemoTmDramMemoryTestPrepare(GT_U8 devNum, GT_PORT_NUM dstPort, GT_U32 CRCstartByte)
{
    GT_STATUS rc;
    GT_U32              i;
    CPSS_DXCH_TTI_RULE_UNT      pattern;
    CPSS_DXCH_TTI_RULE_UNT      mask;
    CPSS_DXCH_TTI_ACTION_STC    action;
    CPSS_PORT_EGRESS_CNTR_STC   egrCntr;

/* prepare TTI related stuff */

    /* UDB for CRC*/
    for(i=0;i<4;i++)
    {
        rc = cpssDxChTtiUserDefinedByteSet(devNum, CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E, 4+i,
                                               CPSS_DXCH_TTI_OFFSET_L2_E, CRCstartByte+i);

        if(rc != GT_OK)
            return rc;
    }

    /* set TTI rule to block all loopbacked packets and match on CRC bytes, it is located before the scenario rules */
    osMemSet(&(mask), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    osMemSet(&(pattern), 0, sizeof(CPSS_DXCH_TTI_RULE_UNT));
    osMemSet(&action, 0, sizeof(CPSS_DXCH_TTI_ACTION_STC));

    action.command              = CPSS_PACKET_CMD_DROP_HARD_E;
    action.userDefinedCpuCode   = CPSS_NET_FIRST_USER_DEFINED_E + 1;
    action.keepPreviousQoS      = GT_TRUE;
    action.bindToCentralCounter = GT_TRUE;
    action.centralCounterIndex  = ttiCrcMatchCounterIndex;

    action.tag1VlanCmd = CPSS_DXCH_TTI_VLAN_MODIFY_UNTAGGED_E;

    /* CRC pattern */
    osPrintf("configure CRC match 0x%X 0x%X 0x%X 0x%X\n", CRC_arr[0], CRC_arr[1], CRC_arr[2], CRC_arr[3]);

    for(i=0;i<4;i++)
    {
        pattern.udbArray.udb[4+i] = CRC_arr[i];
        mask.udbArray.udb[4+i]    = 0xff;
    }

    /* dstPort is the source port for looped back packet */
    pattern.udbArray.udb[0] = dstPort;
    mask.udbArray.udb[0]    = 0xff;

    rc = cpssDxChTtiRuleSet(devNum, firstTtiRule+1 /*9612*/, CPSS_DXCH_TTI_RULE_UDB_10_E, &pattern, &mask,
                        &action);
    if(rc != GT_OK)
        return rc;

    /* set next TTI rule to block all loopbacked packets and without match on CRC bytes, with the purpose of locating rules with wrong CRC*/
    for(i=0;i<4;i++)
    {
        pattern.udbArray.udb[4+i] = 0;
        mask.udbArray.udb[4+i]    = 0;
    }
    action.centralCounterIndex  = ttiCrcNoMatchCounterIndex;
    rc = cpssDxChTtiRuleSet(devNum, firstTtiRule+2 /*9613*/, CPSS_DXCH_TTI_RULE_UDB_10_E, &pattern, &mask,
                        &action);
    if(rc != GT_OK)
        return rc;

    rc = cpssDxChPortEgressCntrModeSet(devNum, 0, CPSS_EGRESS_CNT_PORT_E, dstPort, 0,0,0);
    if(rc != GT_OK)
        return rc;

    /* read to clear counters in HW */
    rc = cpssDxChPortEgressCntrsGet(devNum, 0, &egrCntr);
    if(rc != GT_OK)
        return rc;

/* ignore MAC address based exceptions */
    rc = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum, CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_MAC_SA_E, CPSS_PACKET_CMD_FORWARD_E);
    if(rc != GT_OK)
        return rc;


    /* make sure counters are clear on read */
        rc = cpssDxChPortMacCountersClearOnReadSet(devNum, dstPort, GT_TRUE);
        if(rc != GT_OK)
            return rc;

    return rc;

}


/* @internal appDemoTmDramMemoryTestGetCounters function
* @endinternal
*
* @brief   Read counters to verify success of the external DDR BIST test.
*
* @note   APPLICABLE DEVICES:      Caelum;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] dstPort                - destination port that is put in UP loopback state in order to redirect packets coming out of external DDR
*                                      back to TTI rul that checks packet's CRC.
* @param[out] counrtersMatchPtr     - true, if counters are as expected, false otherwise.
* @param[in] verbose               - if print detailed information about the test run.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                    - on general failure
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Used together with appDemoTmDramMemoryTestRun function

*/
GT_STATUS appDemoTmDramMemoryTestGetCounters
(
    IN GT_U8       devNum,
    IN GT_PORT_NUM dstPort,
    OUT GT_BOOL    *counrtersMatchPtr,
    IN GT_BOOL     verbose)
{
    CPSS_PORT_EGRESS_CNTR_STC   egrCntr;
    CPSS_PORT_MAC_COUNTER_SET_STC   portMacCounter;
    GT_STATUS                   rc;
    GT_U32                      checkBISTCountA, checkBISTCountB;

    /* read egress counters */
    rc = cpssDxChPortEgressCntrsGet(devNum, 0, &egrCntr);
    if(rc != GT_OK)
        return rc;

    /* read MIB counters */
    rc = cpssDxChPortMacCountersOnPortGet(devNum, dstPort, &portMacCounter);
    if(rc != GT_OK)
        return rc;
    if(verbose)
    {
        osPrintf("before entry to TM, egress counters: %u packets passed, %u packets dropped\n", egrCntr.outUcFrames,
            egrCntr.txqFilterDisc + egrCntr.brgEgrFilterDisc + egrCntr.egrFrwDropFrames + egrCntr.mcFifo3_0DropPkts);

        osPrintf("MIB counters on loopback port: %u packets received and sent - %u each direction\nFC packets %u, total %u drop events\n %u receive error %u CRC events",
                                                                                    portMacCounter.pkts64Octets.l[0] + portMacCounter.pkts65to127Octets.l[0],
                                                                                    (portMacCounter.pkts64Octets.l[0] + portMacCounter.pkts65to127Octets.l[0])/2,
                                                                                    portMacCounter.fcSent.l[0],
                                                                                    portMacCounter.dropEvents.l[0],
                                                                                    portMacCounter.macRcvError.l[0],
                                                                                    portMacCounter.badCrc.l[0]);
    }
    if((portMacCounter.dropEvents.l[0] + portMacCounter.macRcvError.l[0] + portMacCounter.badCrc.l[0]) != 0)
    {
        osPrintf("Error in packets transmission, %d failures\n", portMacCounter.dropEvents.l[0] + portMacCounter.macRcvError.l[0] + portMacCounter.badCrc.l[0]);
        *counrtersMatchPtr = GT_FALSE;
        return rc;
    }
    checkBISTCountA = (portMacCounter.pkts64Octets.l[0] + portMacCounter.pkts65to127Octets.l[0])/2;

    /* see if counters in DRAM and counters in TTI match */
    rc = appDemoTmTCAMCountersGet(devNum, &checkBISTCountB, verbose);
    if(rc != GT_OK)
        return rc;

    osPrintf("countA: %u countB: %u\n", checkBISTCountA, checkBISTCountB);
    if((checkBISTCountA == checkBISTCountB) && (checkBISTCountB != 0))
    {
        *counrtersMatchPtr = GT_TRUE;
    }
    else
    {
        *counrtersMatchPtr = GT_FALSE;
    }
    return rc;
}

/**
* @internal appDemoTmDramMemoryTestRun function
* @endinternal
*
* @brief   The function runs TM DDR memory test, as an alternative to traditional BIST test.  There is no need for an external traffic genearator.
*          In the course of the test CPU generated packets with predefined pattern
*          are sent to a TM port.  The relevant Q-node of the TM tree is set to block, but not drop the traffic.
*          Work of TD mechanism is disabled except for the full DDR threshold.  This way the DDR memory is filled to the maximum.
*          Then the node is enbaled for traffic and the packets leave the DDR.  The destination port is set to loopback and traffic is redirected
*          to a TTI rule that checks known CRC of packets and counts the hits, on assumption that if DDR emory was corrupted packet pattern and CRC were chenged.
*          Number of hits is compared with the number of packets that passed through TM.  In addition, we check that no errors were observed on the
*          MIB counter of the destination port.
*          There are two passes of the test - each with a different pattern.
*
* @note   APPLICABLE DEVICES:      Caelum;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] dstPort                  - destination port that is put in UP loopback state in order to redirect packets coming out of external DDR
*                                      back to TTI rul that checks packet's CRC.
* @param[in] pktSize                  - size of packets used in the teste.
* @param[in] pktCount                 - used for debug, if it is 0 - fill the DRAM with as many packets as needed, otherwise send the pktCount of packets
* @param[in] verbose               - if print detailed information about the test run.
*
* @retval GT_OK                    - test passed
* @retval GT_FAIL                  - test failed due to CRC mismatch - sign that teh external DDR memory is corrupted
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The test is based on CPSS_TM_BIST TM scenarion (scenario #18).  In order to run teh test use the following commands:
* shell-execute  appDemoTmScenarioModeSet 18
* shell-execute  appDemoTmOverrideRootSet 56
* cpssInitSystem 29,2,0 noPorts

* shell-execute  appDemoTmScenarioBISTSet 0,56,17,0
* shell-execute appDemoTmDramMemoryTestRun 0, 56, 100, 0, 0

*/
GT_STATUS appDemoTmDramMemoryTestRun
(
    IN GT_U8       devNum,
    IN GT_PORT_NUM dstPort,
    IN GT_U32      pktSize,
    IN GT_U32      pktCount,
    IN GT_BOOL     verbose
)
{
    CPSS_DXCH_CNC_COUNTER_STC   cncCounter;
    GT_U32                      dropCounterIndex;
    GT_U32                      passCounterIndex;
    GT_BOOL                     DRAMfull=GT_FALSE;
    GT_U32                      qLength[2] = {0,0};
    GT_U64                      storedPacketsNum;
    GT_STATUS                   rc;
    static GT_U32               start_s, start_n;
    static GT_U32               stop_s, stop_n;
    GT_U32                      delta_s, delta_n;
    GT_BOOL                     counrtersMatch;
    GT_U8                       firstBytes[2], fillBytes[2];
    GT_U32                      i;

    passCounterIndex = queueNum*2;
    dropCounterIndex = queueNum*2+1;

    if(PRV_CPSS_PP_MAC(devNum)->devSubFamily != CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E)
    {
        osPrintf("The BIST test does not run on BC2 devices yet\n");
        return GT_NOT_APPLICABLE_DEVICE;
    }

    firstBytes[0] = 0xFF;
    firstBytes[1] = 0x00;
    fillBytes[0]  = 0x55;
    fillBytes[1]  = 0xAA;

    rc = appDemoTmDramMemoryTestCRCSet(0xE8, 0x1F, 0x87, 0xA1);
    if(rc != GT_OK)
        return rc;

    if(pktCount > 0)
    {
        for(i=0;i<2;i++)
        {
            rc = appDemoTmDramMemoryTestPrepare(devNum, dstPort, pktSize-4);
            if (rc != GT_OK)
                return rc;

            rc = cpuTxFrwrd(devNum, dstPort,1,0, 100, pktCount, firstBytes[i], fillBytes[i]);
            if(rc != GT_OK)
                return rc;
            osTimerWkAfter(10);
            rc = appDemoTmDramMemoryTestGetCounters(devNum, dstPort, &counrtersMatch, verbose);
            if(rc != GT_OK)
                return rc;
            if(counrtersMatch == GT_TRUE)
            {
                osPrintf("reduced BIST test part %d PASSED\n", i+1);
            }
            else
            {
                osPrintf("reduced BIST test part %d FAILED\n", i+1);
            }

            rc = appDemoTmDramMemoryTestCRCSet(0x96, 0x66, 0xaa, 0x5d);
            if(rc != GT_OK)
                return rc;

        }
        return rc;
    }
    osTimeRT(&start_s, &start_n);

    /* repeate test with a different patterns */
    for(i=0;i<2;i++)
    {
        rc = appDemoTmDramMemoryTestPrepare(devNum, dstPort, pktSize-4);
        if (rc != GT_OK)
            return rc;
        /* block the node in order to fill the DRAM with packets */
        rc = appDemoTmNodePrioWeightUpdate(devNum, CPSS_TM_LEVEL_Q_E, queueNum, CPSS_TM_ELIG_N_DEQ_DIS_E, 1);
        if (rc != GT_OK)
            return rc;

        /* send packets */
        do
        {
            rc = cpuTxFrwrd(devNum, dstPort,1,0, 100, 1000, firstBytes[i], fillBytes[i]);
            if(rc != GT_OK)
                return rc;

            /* wait till the DRAM is full */
            rc = cpssDxChCncCounterGet(devNum, 0, dropCounterIndex, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &cncCounter);
            if (rc != GT_OK)
                return rc;

            /* check that there are dropped packets - meaning DRAM is full */
            if(cncCounter.packetCount.l[0] > 0)
            {
                if(verbose)
                {
                    osPrintf("0x%X%X \t ( %8u )packets dropped on queue 1\n",cncCounter.packetCount.l[1], cncCounter.packetCount.l[0],
                                   (cncCounter.packetCount.l[1]*0xFFFFFFFF + cncCounter.packetCount.l[0]));
                }
                DRAMfull=GT_TRUE;
                rc = cpssDxChCncCounterGet(devNum, 0, passCounterIndex, CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E, &cncCounter);
                if (rc != GT_OK)
                    return rc;
                if(verbose)
                {
                    osPrintf("0x%X%X \t ( %8u )packets entered TM through queue 1\n",cncCounter.packetCount.l[1], cncCounter.packetCount.l[0],
                                   (cncCounter.packetCount.l[1]*0xFFFFFFFF + cncCounter.packetCount.l[0]));
                    osPrintf("0x%X%X ( %u )bytes in DRAM\n",cncCounter.byteCount.l[1], cncCounter.byteCount.l[0],
                         (cncCounter.byteCount.l[1]*0xFFFFFFFF + cncCounter.byteCount.l[0]));
                    storedPacketsNum.l[1] = cncCounter.packetCount.l[1];
                    storedPacketsNum.l[0] = cncCounter.packetCount.l[0];
                    osPrintf("stored %d packets\n",storedPacketsNum.l[0]);
                }
                osPrintf("PASS %d: DRAM filled\n", i);
            }

        }while(DRAMfull == GT_FALSE);

        /* check how many bytes are now occupied in DRAM */
        rc = cpssTmDropQueueLengthGet(devNum,
                                  CPSS_TM_LEVEL_Q_E,
                                  queueNum,
                                  &(qLength[0]),
                                  &(qLength[1]));
    /* unblock the node */
        rc = appDemoTmNodePrioWeightUpdate(devNum, CPSS_TM_LEVEL_Q_E, queueNum, CPSS_TM_ELIG_N_PRIO1_E, 1);
        if (rc != GT_OK)
            return rc;

    /* wait till the DRAM is empty */
        do
        {
            if(verbose)
            {
                osPrintf("DRAM (average) instant use is %u (%u) bytes\n",
                         qLength[0]*16,
                         qLength[1]*16
                         );
            }
            osTimerWkAfter(100);
            rc = cpssTmDropQueueLengthGet(devNum,
                                      CPSS_TM_LEVEL_Q_E,
                                      queueNum,
                                      &(qLength[0]),
                                      &(qLength[1]));
            if( (qLength[0] +  qLength[1]) == 0)
            {
                DRAMfull = GT_FALSE;
                osPrintf("PASS %d: DRAM emptied, checking counters\n", i);
            }

        }while(DRAMfull == GT_TRUE);
/* count the received packets, comparing the expected CRC on TTI rule */
        rc = appDemoTmDramMemoryTestGetCounters(devNum, dstPort, &counrtersMatch, verbose);
        if(rc != GT_OK)
            return rc;
        if(counrtersMatch == GT_TRUE)
        {
            osPrintf("BIST test part %d PASSED\n", i+1);
        }
        else
        {
            osPrintf("BIST test FAILED\n");
            return GT_FAIL;
        }

        rc = appDemoTmDramMemoryTestCRCSet(0x96, 0x66, 0xaa, 0x5d);
        if(rc != GT_OK)
            return rc;

    }

    osTimeRT(&stop_s, &stop_n);
    time_sub(stop_s, stop_n, start_s, start_n, &delta_s, &delta_n);
    osPrintf("BIST test duration time=%d.%09d seconds\n",
            delta_s, delta_n);

    return rc;

}

/**
 * @internal readReg64 function
 * @endinternal
 *
 * @brief Read and print 64 bit TM registers
 *
 * @param[in] devNum      - device number
 * @param[in] regAddrMsb  - MSB of register address
 * @param[in] regAddrLsb  - LSB of register address
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - on null pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS readReg64
(
    IN GT_U8        devNum,
    IN GT_U32       regAddrMsb,
    IN GT_U32       regAddrLsb
)
{
    GT_STATUS       rc;
    GT_U64          hwData;
    GT_U64          regAddr;

    regAddr.l[0] = regAddrLsb;
    regAddr.l[1] = regAddrMsb;
    rc = cpssTmCtlReadRegister(devNum, &regAddr, &hwData);
    cpssOsPrintf("Read Reg: 0x%08x%08x Data:0x%08x%08x\n",
                 regAddrMsb, regAddrLsb, hwData.l[1], hwData.l[0]);
    return rc;
}

/**
 * @internal writeReg64 function
 * @endinternal
 *
 * @brief Write to HW 64 bit TM registers
 *
 * @param[in] devNum      - device number
 * @param[in] regAddrMsb  - MSB of register address
 * @param[in] regAddrLsb  - LSB of register address
 * @param[in] dataMSB     - MSB of 64bit data
 * @param[in] dataLSB     - LSB of 64bit data
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong parameters
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_BAD_PTR               - on null pointer
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS writeReg64
(
    IN GT_U8      devNum,
    IN GT_U32     regAddrMsb,
    IN GT_U32     regAddrLsb,
    IN GT_U32     dataMsb,
    IN GT_U32     dataLsb
)
{
    GT_STATUS   rc;
    GT_U64      hwData;
    GT_U64      regAddr;

    regAddr.l[0] = regAddrLsb; /* lower 32 bits */
    regAddr.l[1] = regAddrMsb;
    hwData.l[0]  = dataLsb;
    hwData.l[1]  = dataMsb;

    rc = cpssTmCtlWriteRegister(devNum, &regAddr, &hwData);
    return rc;
}


