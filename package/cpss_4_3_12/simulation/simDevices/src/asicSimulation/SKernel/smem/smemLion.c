/******************************************************************************
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
* @file smemLion.c
*
* @brief Lion memory mapping implementation: the 'port group' and the shared memory
*
* @version   79
********************************************************************************
*/
#include <asicSimulation/SInit/sinit.h>
#include <asicSimulation/SKernel/smem/smemLion.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Pcl.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahTxQ.h>

/* the port group# that used by CPU to access the shared memory -->
   the PEX address is the same one  */
#define PORT_GROUP_ID_FOR_PORT_GROUP_SHARED_ACCESS_CNS  0

/**
* @internal smemLionTableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemLionTableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    /* Override Lion B0 tables info */
    if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
    {
        /* must be set before calling smemXcatA1TableInfoSet(...) */
        /* use xcat A1 table addresses */
        smemXcatA1TableInfoSet(devObjPtr);
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* no more */
            return;
        }
        devObjPtr->globalInterruptCauseRegister.txqSht     = 5;
        devObjPtr->globalInterruptCauseRegister.txqDq      = 6;
        devObjPtr->globalInterruptCauseRegister.tti        = 7;
        devObjPtr->globalInterruptCauseRegister.tccLower   = 8;
        devObjPtr->globalInterruptCauseRegister.tccUpper   = 9;
        devObjPtr->globalInterruptCauseRegister.bcn        = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.ha         = 10;
        devObjPtr->globalInterruptCauseRegister.misc       = 11;
        devObjPtr->globalInterruptCauseRegister.mem        = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.txq        = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.txqll      = 12;
        devObjPtr->globalInterruptCauseRegister.txqQueue   = 13;
        devObjPtr->globalInterruptCauseRegister.l2i        = 14;
        devObjPtr->globalInterruptCauseRegister.bm0        = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.bm1        = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.txqEgr     = 15;
        devObjPtr->globalInterruptCauseRegister.xlgPort    = 16;
        devObjPtr->globalInterruptCauseRegister.fdb        = 17;
        devObjPtr->globalInterruptCauseRegister.ports      = 18;
        devObjPtr->globalInterruptCauseRegister.cpuPort    = 19;
        devObjPtr->globalInterruptCauseRegister.global1    = 20;
        devObjPtr->globalInterruptCauseRegister.txSdma     = 21;
        devObjPtr->globalInterruptCauseRegister.rxSdma     = 22;
        devObjPtr->globalInterruptCauseRegister.pcl        = 23;
        devObjPtr->globalInterruptCauseRegister.port24     = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.port25     = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.port26     = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.port27     = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.iplr0      = 24;
        devObjPtr->globalInterruptCauseRegister.iplr1      = 25;
        devObjPtr->globalInterruptCauseRegister.rxDma      = 26;
        devObjPtr->globalInterruptCauseRegister.txDma      = 27;
        devObjPtr->globalInterruptCauseRegister.eq         = 28;
        devObjPtr->globalInterruptCauseRegister.bma        = SMAIN_NOT_VALID_CNS;
        devObjPtr->globalInterruptCauseRegister.eplr       = 31;

        /* like in xcat A0 */
        {/* ipcl TCC */
            devObjPtr->tablesInfo.pclAction.commonInfo.baseAddress = 0x0D0B0000;
            devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress = 0x0D040000;
            devObjPtr->tablesInfo.pclTcamMask.commonInfo.baseAddress = devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress + 0x20;
        }

        devObjPtr->tablesInfo.routerTcam.commonInfo.baseAddress = 0x0D840000;

        {/* mll */
            devObjPtr->tablesInfo.mllTable.commonInfo.baseAddress = 0x0C880000;
            devObjPtr->tablesInfo.mllOutInterfaceConfig.commonInfo.baseAddress = 0x0C800980;
            devObjPtr->tablesInfo.mllOutInterfaceCounter.commonInfo.baseAddress = 0x0C800900;
            devObjPtr->memUnitBaseAddrInfo.mllRegBase = 0x0C800000;
        }

        devObjPtr->tablesInfo.stp.commonInfo.baseAddress = 0x118D0000;

        devObjPtr->tablesInfo.mcast.commonInfo.baseAddress = 0x11860000;

        devObjPtr->tablesInfo.vlan.commonInfo.baseAddress = 0x118A0000;
        devObjPtr->tablesInfo.vlan.paramInfo[0].step = 0x20;

        /* not valid table any more -- the vrfId is part of the vlan entry */
        devObjPtr->tablesInfo.vrfId.commonInfo.baseAddress = 0;

        /* override with values that like xCat-A1 for next tables */
        devObjPtr->tablesInfo.arpTable.commonInfo.baseAddress = 0x0E840000;
        devObjPtr->tablesInfo.tunnelStartTable.commonInfo.baseAddress = devObjPtr->tablesInfo.arpTable.commonInfo.baseAddress;
        devObjPtr->tablesInfo.tunnelStartTable.paramInfo[0].step = devObjPtr->tablesInfo.arpTable.paramInfo[0].step;

        devObjPtr->tablesInfo.vlanPortMacSa.commonInfo.baseAddress = 0x0E808000;
        devObjPtr->tablesInfo.ingressVlanTranslation.commonInfo.baseAddress = 0x0100C000;/*TTI*/
        devObjPtr->tablesInfo.egressVlanTranslation.commonInfo.baseAddress = 0x0E810000;/*HA*/

        devObjPtr->tablesInfo.ttiAction.commonInfo.baseAddress = 0x0D8C0000;

        devObjPtr->tablesInfo.deviceMapTable.commonInfo.baseAddress = 0x11870000;
        devObjPtr->tablesInfo.deviceMapTable.paramInfo[0].step       = 0x4;
        devObjPtr->tablesInfo.deviceMapTable.paramInfo[1].step       = 0;

        devObjPtr->tablesInfo.dscpToDscpMap.commonInfo.baseAddress    = 0x01000900;
        devObjPtr->tablesInfo.dscpToQoSProfile.commonInfo.baseAddress = 0x01000400;
        /* CFI UP -> QosProfile - table 0 */
        devObjPtr->tablesInfo.upToQoSProfile.commonInfo.baseAddress   = 0x01000440;
        devObjPtr->tablesInfo.upToQoSProfile.paramInfo[1].step  = 0x8;    /* CFI */
        devObjPtr->tablesInfo.upToQoSProfile.paramInfo[2].step = 0x10;    /*upProfile*/

        devObjPtr->tablesInfo.expToQoSProfile.commonInfo.baseAddress  = 0x01000460;

        devObjPtr->tablesInfo.cfiUpQoSProfileSelect.commonInfo.baseAddress = 0x01000470;
        devObjPtr->tablesInfo.cfiUpQoSProfileSelect.paramInfo[0].step = 0x4;

        devObjPtr->tablesInfo.macToMe.commonInfo.baseAddress = 0x01001600;

        devObjPtr->tablesInfo.portVlanQosConfig.commonInfo.baseAddress = 0x01001000;
        devObjPtr->tablesInfo.portVlanQosConfig.paramInfo[0].step = 0x10;

        devObjPtr->tablesInfo.portProtocolVidQoSConf.commonInfo.baseAddress = 0x01004000;

        devObjPtr->tablesInfo.ipclUserDefinedBytesConf.commonInfo.baseAddress = 0x01010000;

        devObjPtr->tablesInfo.pearsonHash.commonInfo.baseAddress = 0x0B8005C0;
        devObjPtr->tablesInfo.pearsonHash.paramInfo[0].step = 0x4;

        devObjPtr->tablesInfo.crcHashMask.commonInfo.baseAddress = 0x0B800400;
        devObjPtr->tablesInfo.crcHashMask.paramInfo[0].step = 0x10;

        devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress = 0x0D040000;
        devObjPtr->tablesInfo.pclTcamMask.commonInfo.baseAddress = devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress + 0x20;

        devObjPtr->tablesInfo.tcamBistArea.commonInfo.baseAddress = 0x0D000400;
        devObjPtr->tablesInfo.tcamBistArea.paramInfo[0].step = 0x4;

        devObjPtr->tablesInfo.tcamArrayCompareEn.commonInfo.baseAddress = 0x0D001000;
        devObjPtr->tablesInfo.tcamArrayCompareEn.paramInfo[0].step = 0x4;

        devObjPtr->tablesInfo.haUp0PortKeepVlan1.commonInfo.baseAddress = 0x0E800440;
        devObjPtr->tablesInfo.haUp0PortKeepVlan1.paramInfo[0].step = 0x4;

        devObjPtr->tablesInfo.egressVlan.commonInfo.baseAddress = 0x11800000;
        devObjPtr->tablesInfo.egressVlan.paramInfo[0].step = 0x40;

        devObjPtr->tablesInfo.egressStp.commonInfo.baseAddress = 0x11840000;
        devObjPtr->tablesInfo.egressStp.paramInfo[0].step = 0x10;

        devObjPtr->tablesInfo.l2PortIsolation.commonInfo.baseAddress = 0x11880000;
        devObjPtr->tablesInfo.l2PortIsolation.paramInfo[0].step = 0x10;

        devObjPtr->tablesInfo.l3PortIsolation.commonInfo.baseAddress = 0x11890000;
        devObjPtr->tablesInfo.l3PortIsolation.paramInfo[0].step = 0x10;

        devObjPtr->tablesInfo.nonTrunkMembers.commonInfo.baseAddress = 0x01810800;
        devObjPtr->tablesInfo.nonTrunkMembers.paramInfo[0].step = 0x8;

        devObjPtr->tablesInfo.designatedPorts.commonInfo.baseAddress = 0x01810200;
        devObjPtr->tablesInfo.designatedPorts.paramInfo[0].step = 0x8;

        devObjPtr->tablesInfo.sst.commonInfo.baseAddress = 0x01810500;
        devObjPtr->tablesInfo.sst.paramInfo[0].step = 0x8;

        devObjPtr->tablesInfo.secondTargetPort.commonInfo.baseAddress = 0x01801300;
        devObjPtr->tablesInfo.secondTargetPort.paramInfo[0].step = 0x4;

        devObjPtr->tablesInfo.egressStc.commonInfo.baseAddress = 0x11004400;
        devObjPtr->tablesInfo.egressStc.paramInfo[0].step = 0x10;/*local port*/
    }
    else
    {
        /* use xcat A0 table addresses */
        smemCht3TableInfoSet(devObjPtr);

        devObjPtr->globalInterruptCauseRegister.bma        = 29;

        /* restore override table */
        devObjPtr->tablesInfo.portProtocolVidQoSConf.commonInfo.baseAddress = 0x0B810800;
        devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[0].step = 0x20;
        devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[1].step = 0x4;

        /* override with values that like xCat-A1 for next tables */
        devObjPtr->tablesInfo.stp.commonInfo.baseAddress = 0x0A080000;

        devObjPtr->tablesInfo.mcast.commonInfo.baseAddress = 0x0A100000;

        devObjPtr->tablesInfo.vlan.commonInfo.baseAddress = 0x0A200000;
        devObjPtr->tablesInfo.vlan.paramInfo[0].step = 0x20;

        devObjPtr->tablesInfo.vrfId.commonInfo.baseAddress = 0x0A180000;

        devObjPtr->tablesInfo.pclConfig.commonInfo.baseAddress = 0x0B840000;
        devObjPtr->tablesInfo.pclConfig.paramInfo[0].step = 0x8; /*entry*/
        devObjPtr->tablesInfo.pclConfig.paramInfo[1].step = 0;

        devObjPtr->tablesInfo.deviceMapTable.paramInfo[0].step       = 0x80;/*per device*/
        devObjPtr->tablesInfo.deviceMapTable.paramInfo[1].step       = 0x10;/*per port>>3*/
    }

}



/**
* @internal smemLionActiveWriteSchedulerConfigReg function
* @endinternal
*
* @brief   Set the <UpdateSchedVarTrigger> field in the Scheduler Configuration
*         Register which loads the new profile configuration.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter -
*                                      global interrupt bit number.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemLionActiveWriteSchedulerConfigReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 reg_value;

    /* data to be written */
    reg_value = *inMemPtr;

    /* Clear bit[6] in Scheduler Configuration Register in order to flag that updates were performed */
    reg_value &= ~(1<<6);

    /* write to simulation memory */
    *memPtr = reg_value;

    return;
}

/**
* @internal smemLionActiveReadPtpMessage function
* @endinternal
*
* @brief   Read PTP Message to CPU Register - simulate read from timestamp fifo.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemLionActiveReadPtpMessage
(
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32 * fifoBufferPtr;                 /* fifo memory pointer */
    GT_U32 i;

    /* Set pointer to the common memory */
    SMEM_CHT_DEV_COMMON_MEM_INFO  * memInfoPtr =
        (SMEM_CHT_DEV_COMMON_MEM_INFO *)(deviceObjPtr->deviceMemory);

    /* Get pointer to the start of timestamp fifo */
    if(param == SMAIN_DIRECTION_INGRESS_E)
    {
        fifoBufferPtr = &memInfoPtr->gtsIngressFifoMem.gtsFifoRegs[0];
    }
    else
    {
        fifoBufferPtr = &memInfoPtr->gtsEgressFifoMem.gtsFifoRegs[0];
    }

    /* Output value */
    *outMemPtr = *fifoBufferPtr;

    if (*fifoBufferPtr == 0xffffffff)
    {
        /* Buffer is empty, there is no new message */
        return;
    }

    /* Shift other words up */
    for (i = 1; i < GTS_FIFO_REGS_NUM; i++)
    {
        fifoBufferPtr[i - 1] = fifoBufferPtr[i];
    }

    /* Invalidate last word */
    fifoBufferPtr[GTS_FIFO_REGS_NUM - 1] = 0xffffffff;
}

/**
* @internal smemLionActiveWriteTodGlobalReg function
* @endinternal
*
* @brief   Global configuration settings and TOD function triggering
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter: 0  ingress GTS , 1  egress GTS
* @param[in,out] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemLionActiveWriteTodGlobalReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    INOUT      GT_U32 * inMemPtr
)
{
    GT_U32 fieldVal;
    SNET_LION_PTP_TOD_EVENT_ENT event;

    /* Output value */
    *memPtr = *inMemPtr;

    fieldVal = SMEM_U32_GET_FIELD(*inMemPtr, 6, 1);
    /* TOD Counter Function not triggered */
    if(fieldVal == 0)
    {
        return;
    }

    fieldVal = SMEM_U32_GET_FIELD(*inMemPtr, 4, 2);
    /* TOD Counter Function  */
    switch(fieldVal)
    {
        case 0:
            event = SNET_LION_PTP_TOD_UPDATE_E;
            break;
        case 1:
            event = SNET_LION_PTP_TOD_INCREMENT_E;
            break;
        case 2:
            event = SNET_LION_PTP_TOD_CAPTURE_E;
            break;
        case 3:
            event = SNET_LION_PTP_TOD_GENERATE_E;
            break;
        default:
            return;
    }

    snetLionPtpTodCounterApply(devObjPtr, param, event);

    if( (event != SNET_LION_PTP_TOD_CAPTURE_E) &&
        (event != SNET_LION_PTP_TOD_GENERATE_E) )
    {
        /* Update clock reference value for TOD calculation */
        devObjPtr->todTimeStampClockDiff[param] = SIM_OS_MAC(simOsTickGet)();
    }

    /* Clear TOD Counter Function Trigger bit once the action is done */
    SMEM_U32_SET_FIELD(*memPtr, 6, 1, 0);
}

/**
* @internal smemLionActiveReadPtpTodSeconds function
* @endinternal
*
* @brief   Read PTP TOD seconds register.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemLionActiveReadPtpTodSeconds
(
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U64 seconds64;

    /* Read TOD counter 64 bit seconds */
    snetLionPtpTodCounterSecondsRead(deviceObjPtr, param, &seconds64);

    if((address & 0xff) == 0x14)
    {
        /* Low word of second field of the TOD Counter */
        *outMemPtr = seconds64.l[0];
    }
    else
    {
        /* High word of second field of the TOD Counter */
        *outMemPtr = seconds64.l[1];
    }

}

/**
* @internal smemLionActiveReadPtpTodNanoSeconds function
* @endinternal
*
* @brief   Read PTP TOD nano seconds register.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemLionActiveReadPtpTodNanoSeconds
(
    IN         SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32 nanoseconds;

    /* Read TOD counter nanoseconds */
    snetLionPtpTodCounterNanosecondsRead(deviceObjPtr, param, &nanoseconds);

    *outMemPtr = nanoseconds;
}

/**
* @internal smemLionActiveWriteDequeueEnableReg function
* @endinternal
*
* @brief   Process Dequeue Enable Register which loads the new
*         tc configuration (tc enable/disable).
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
GT_VOID smemLionActiveWriteDequeueEnableReg
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   address,
    IN GT_U32   memSize,
    IN GT_U32 * memPtr,
    IN GT_UINTPTR   param,
    IN GT_U32 * inMemPtr
)
{
    GT_U32  port;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* exact match for all 72 registers is achieved */
        /* get port num */
        port = param/*the start port from 0x00*/ + ((address & 0xFF) / 0x4);
        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* convert local TxQ port to global one */
            port = smemSip5_20_GlobalTxQPortForDqRegisterGet(devObjPtr, address, port);
        }

        smemTxqSendDequeueMessages(devObjPtr, *inMemPtr, port, 0);
    }
    else
    /* check address first byte range 0x14...0x58 */
    if((address & 0xFF) >= 0x14 && (address & 0xFF) <= 0x58)
    {
        /* get port num */
        port = ((address & 0xFF) - 0x14) / 0x4;

        smemTxqSendDequeueMessages(devObjPtr, *inMemPtr, port, 0);
    }

    /* Update the register value */
    *memPtr = *inMemPtr;
    return;
}


