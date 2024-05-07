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
* @file prvCpssDxChHwGdmaDbg.c
*
* @brief Debug and Reg-Dump for GDMA unit
*
* @version   1
********************************************************************************
*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdma.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdmaNetIfFunc.h>

GT_STATUS dbgGdmaTxQueueNumOfFreeDescs(GT_U8 devNum, GT_U32 txQueue)
{
    GT_U32    num = 0xffffffff;
    GT_STATUS rc = cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(devNum, txQueue, &num);
    cpssOsPrintf(" txQueue=%d:  number of free TxDescriptors = %d\n", num);
    return rc;
}

GT_STATUS dbgGdmaQueueRegDump(GT_U8 devNum, GT_U32 gdmaUnitId, GT_U32 queue, GT_U32 mod)
{
    GT_U32      regAddr, regAdd0;
    GT_U32      value;
    GT_STATUS   rc = GT_OK;

    if (mod > 3)
        mod = 0;
    if (mod == 1) {
        cpssOsPrintf("DUMP [%u:%d]  generalRegs ----------\n", gdmaUnitId, queue);
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).generalRegs.gdmaBuffersSize;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X]=0x%08x   gdmaBuffersSize           \n", regAddr, value);
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).generalRegs.gdmaPayloadBufferThreshold;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X]=0x%08x   gdmaPayloadBufferThreshold\n", regAddr, value);
    }
    if (mod == 0 || mod == 2) {
        cpssOsPrintf("DUMP [%u:%d]  sgdAxiControlRegs ----------\n", gdmaUnitId, queue);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[  0  ].sgdChainBaseAddressLow & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[queue].sgdChainBaseAddressLow;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   sgdChainBaseAddressLow    \n", regAddr, regAdd0, value);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[  0  ].sgdBufferAllocationControl & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[queue].sgdBufferAllocationControl;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   sgdBufferAllocationControl\n", regAddr, regAdd0, value);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[  0  ].sgdChainSize & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).sgdAxiControlRegs[queue].sgdChainSize;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   sgdChainSize              \n", regAddr, regAdd0, value);
    }
    if (mod == 0 || mod == 3) {
        cpssOsPrintf("DUMP [%u:%d]  queueStatusAndControlRegs ----------\n", gdmaUnitId, queue);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[  0  ].queueEnable & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].queueEnable;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   queueEnable     \n", regAddr, regAdd0, value);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[  0  ].queueControl & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].queueControl;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   queueControl    \n", regAddr, regAdd0, value);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[  0  ].wruSGDPointer & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].wruSGDPointer;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   wruSGDPointer(Current SGD Status)\n", regAddr, regAdd0, value);
        regAdd0 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[  0  ].freeSGDsStatusRegister & 0xffff;
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].freeSGDsStatusRegister;
        rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &value);   cpssOsPrintf("      [0x%X](%X)=0x%08x   freeSGDsStatusRegister\n", regAddr, regAdd0, value);
    }
    if (mod == 0) {
        cpssOsPrintf("DUMP [%u:%d]  interruptAndDebugRegs --- NA ---\n", gdmaUnitId, queue);
    }
    return rc;
}

GT_STATUS dbgGdmaQueueStatsGet(GT_U8 devNum, GT_U32 gdmaUnitId, GT_U32 queue)
{
    GT_U32      regAddr;
    GT_U32      pkts, bytes;
    GT_STATUS   rc;

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].stat_counter[0];
    rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &pkts);
    if (rc != GT_OK)
        return rc;
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].stat_byte_counter[0];
    rc = prvCpssHwPpGdmaReadReg(devNum, gdmaUnitId, regAddr, &bytes);
    if (rc != GT_OK)
        return rc;
    if (pkts == 0)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    cpssOsPrintf("[%u:%d] STATS  pkts=[0x%X]=%-3u  bytes=%u\n", gdmaUnitId, queue, regAddr, pkts, bytes);
    return GT_OK;
}

GT_STATUS dbgGdmaUnitStatsGet(GT_U8 devNum, GT_U32 gdmaUnitId)
{
    GT_U32 queue;
    for (queue = 0; queue < 32; queue++)
        dbgGdmaQueueStatsGet(devNum, gdmaUnitId, queue);
    return GT_OK;
}

GT_STATUS dbgGdmaStatsResetAll(void)
{
    GT_U8       devNum = 0;
    GT_U32      gdmaUnitId;
    GT_U32      queue;
    GT_U32      regAddr1, regAddr2;
    GT_STATUS   rc;

    for (gdmaUnitId = 0; gdmaUnitId < 4; gdmaUnitId++)
    {
        for (queue = 0; queue < 32; queue++)
        {
            regAddr1 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].stat_counter[0];
            regAddr2 = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[queue].stat_byte_counter[0];
            rc  = prvCpssHwPpGdmaWriteReg(devNum, gdmaUnitId, regAddr1, 0);
            rc |= prvCpssHwPpGdmaWriteReg(devNum, gdmaUnitId, regAddr2, 0);
            if (rc != GT_OK)
            {
                cpssOsPrintf(" ERROR on %u:%d\n", devNum, gdmaUnitId);
                return rc;
            }
        }
    }
    return GT_OK;
}

GT_STATUS dbgGdmaTxGeneratorStatusGet(GT_U8 devNum, GT_U32 txQueue, GT_U32 traceId)
{
#define GDMA_TMR_LAST_BIT    30
    GT_STATUS   rc;
    GT_U32      gdmaUnitId = 3;
    GT_BOOL     qEnable;
    GT_U32      qIdle;
    GT_U32      sgdChainSize;
    GT_U32      sgdRecycleChain;
    GT_U32      regAddr;
    GT_U32      tmrClocks;
    GT_U32      tmrEnable;
    PRV_CPSS_TX_DESC_LIST_STC   *txDescList;

    /* This Debug API could be called from temporary modified run-time functions,
     * so the GDMA-cheking is required for generalization
     */
    if (!PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);

    txDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    rc = cpssDxChNetIfSdmaTxQueueEnableGet(devNum, txQueue, &qEnable);
    if (rc != GT_OK)
        goto rc_err;
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueEnable;
    rc = prvCpssHwPpGdmaGetRegField(devNum, gdmaUnitId, regAddr, 1, 1, &qIdle);
    if (rc != GT_OK)
        goto rc_err;

    rc = prvCpssDxChNetIfGdmaSgdChainSizeGet(devNum, gdmaUnitId, txQueue, &sgdChainSize);
    if (rc != GT_OK)
        goto rc_err;
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueControl;
    rc = prvCpssHwPpGdmaGetRegField(devNum, gdmaUnitId, regAddr, 4, 1, &sgdRecycleChain);
    if (rc != GT_OK)
        return rc;

    /* Get timer clocks value (125MHz) */
    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).gdmaTimerRegs[txQueue].timerValue;
    rc = prvCpssHwPpGdmaGetRegField(devNum, gdmaUnitId, regAddr,
                                    0, GDMA_TMR_LAST_BIT, &tmrClocks);
    if (rc != GT_OK)
        goto rc_err;

    regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueControl;
    rc = prvCpssHwPpGdmaGetRegField(devNum, gdmaUnitId, regAddr, 7, 1, &tmrEnable);
    if (rc != GT_OK)
        return rc;

    cpssOsPrintf("  TxGen %2u q[%d]: SGD free %d + %d in chain %s; q%s %s; tmrClocks=%u:%s\n",
                 traceId, txQueue, txDescList->freeDescNum, sgdChainSize,
                 (sgdRecycleChain) ? "Loop" : "Singl",
                 (qEnable) ? "ENA" : "DIS",
                 (qIdle) ? "IDLE" : "RUN",
                 tmrClocks,
                 (tmrEnable) ? "ENA" : "DIS");
    return GT_OK;

rc_err:
    cpssOsPrintf("  TxGen %2u StatusGet q[%d] failed rc=%d\n", traceId, txQueue, rc);
    return rc;
}
