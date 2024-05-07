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
* @file smemCheetah2.c
*
* @brief Cheetah2 memory mapping implementation.
*
* @version   48
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah2.h>
#include <asicSimulation/SKernel/smem/smemCheetah3.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>

/**
* @internal smemCht2ActiveFuqBaseWrite function
* @endinternal
*
* @brief   FDB Upload Queue Base Address active write.
*/
void smemCht2ActiveFuqBaseWrite
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32         fuqSize;
    CHT2_FUQ_MEM  * fuqMemPtr;

    *memPtr = *inMemPtr;

    fuqMemPtr = SMEM_CHT_MAC_FUQ_MEM_GET(devObjPtr);

    smemRegFldGet(devObjPtr, SMEM_CHT_FU_QUE_BASE_ADDR_REG(devObjPtr), 0, 30, &fuqSize);

    /* First time */
    if(fuqMemPtr->baseInit ==  GT_FALSE)
    {
        fuqMemPtr->fuqBase = *inMemPtr;
        fuqMemPtr->baseInit = GT_TRUE;
        fuqMemPtr->fuqBaseSize = fuqSize;
        fuqMemPtr->fuqBaseValid = GT_TRUE;
    }
    else
    {
        if (fuqMemPtr->fuqBaseValid == GT_TRUE)
        {
            fuqMemPtr->fuqShadow = *inMemPtr;
            fuqMemPtr->fuqShadowSize = fuqSize;
            fuqMemPtr->fuqShadowValid = GT_TRUE;
        }
        else
        {
            fuqMemPtr->fuqBase = *inMemPtr;
            fuqMemPtr->fuqOffset = 0;
            fuqMemPtr->fuqNumMessages = 0;
            fuqMemPtr->fuqBaseSize = fuqSize;
            fuqMemPtr->fuqBaseValid = GT_TRUE;

            /* Set Queue Full bit (bit 30) to zero */
            smemRegFldSet(devObjPtr, SMEM_CHT_FU_QUE_BASE_ADDR_REG(devObjPtr), 30, 1, 0);
        }
    }
}



/**
* @internal smemCht2TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*
* @param[in] devObjPtr                - device object PTR.
*/
void smemCht2TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    smemChtTableInfoSet(devObjPtr);

    /* override the ch1 values */

    devObjPtr->globalInterruptCauseRegister.port27     = 27;

    devObjPtr->tablesInfo.stp.commonInfo.baseAddress = 0x0A100000;
    devObjPtr->tablesInfo.stp.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.mcast.commonInfo.baseAddress = 0x0A200000;
    devObjPtr->tablesInfo.mcast.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.vlan.commonInfo.baseAddress = 0x0A400000;
    devObjPtr->tablesInfo.vlan.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.ingrStc.commonInfo.baseAddress = 0x0B040000;
    devObjPtr->tablesInfo.ingrStc.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.statisticalRateLimit.commonInfo.baseAddress = 0x0B100000;

    devObjPtr->tablesInfo.cpuCode.commonInfo.baseAddress = 0x0B200000;

    devObjPtr->tablesInfo.qosProfile.commonInfo.baseAddress = 0x0B300000;

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        devObjPtr->tablesInfo.fdbTable.commonInfo.baseAddress = 0x06400000;
    }

    devObjPtr->tablesInfo.portVlanQosConfig.commonInfo.baseAddress = 0x0B810000;
    devObjPtr->tablesInfo.portVlanQosConfig.paramInfo[0].step = 0x20;

/*  xxx  devObjPtr->tablesInfo.pclId.commonInfo.baseAddress = 0x0B800200;*/

    if(devObjPtr->policerSupport.tablesBaseAddrSetByOrigDev == 0)
    {
        devObjPtr->tablesInfo.policer.commonInfo.baseAddress = 0x0C100000;
        devObjPtr->tablesInfo.policerReMarking.commonInfo.baseAddress = 0x0C200000;

        devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = 0x0C300000;
        devObjPtr->tablesInfo.policerCounters.paramInfo[0].step = 0x8;
        devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = 0; /* not supported*/
    }

    devObjPtr->tablesInfo.arpTable.commonInfo.baseAddress = 0x07E80000;
    devObjPtr->tablesInfo.arpTable.paramInfo[0].step = 0x20;
    devObjPtr->tablesInfo.arpTable.paramInfo[0].divider = 4;/* 4 sub entries in each line */

    devObjPtr->tablesInfo.vlanPortMacSa.commonInfo.baseAddress = 0x07F80000;

    devObjPtr->tablesInfo.pclConfig.commonInfo.baseAddress = 0x0B840000;
    devObjPtr->tablesInfo.pclConfig.paramInfo[0].step = 0x4; /*entry*/


    devObjPtr->tablesInfo.pclAction.commonInfo.baseAddress = 0x0B8C0000;
    devObjPtr->tablesInfo.pclAction.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress = 0x0B880000;
    devObjPtr->tablesInfo.pclTcam.paramInfo[0].step = 0x4;  /*entry*/
    devObjPtr->tablesInfo.pclTcam.paramInfo[1].step = 0x10; /*word*/

    devObjPtr->tablesInfo.pclTcamMask.commonInfo.baseAddress = 0x0B880008;
    devObjPtr->tablesInfo.pclTcamMask.paramInfo[0].step = 0x4; /*entry*/
    devObjPtr->tablesInfo.pclTcamMask.paramInfo[1].step = 0x10;/*word*/

    devObjPtr->tablesInfo.tcpUdpDstPortRangeCpuCode.commonInfo.baseAddress = 0x0B007000;
    devObjPtr->tablesInfo.tcpUdpDstPortRangeCpuCode.paramInfo[0].step = 0x4;      /*entry*/
    devObjPtr->tablesInfo.tcpUdpDstPortRangeCpuCode.paramInfo[1].step = 0x40;     /*word*/


    devObjPtr->tablesInfo.ipProtCpuCode.commonInfo.baseAddress = 0x0B008000;
    devObjPtr->tablesInfo.ipProtCpuCode.paramInfo[0].step = 0x4;
    devObjPtr->tablesInfo.ipProtCpuCode.paramInfo[0].divider = 2;

    devObjPtr->tablesInfo.tunnelStartTable.commonInfo.baseAddress = devObjPtr->tablesInfo.arpTable.commonInfo.baseAddress;
    devObjPtr->tablesInfo.tunnelStartTable.paramInfo[0].step = devObjPtr->tablesInfo.arpTable.paramInfo[0].step;


    devObjPtr->tablesInfo.routerTcam.commonInfo.baseAddress = 0x02A00000;
    devObjPtr->tablesInfo.routerTcam.paramInfo[0].step = devObjPtr->routeTcamInfo.entryWidth; /*entry*/
    devObjPtr->tablesInfo.routerTcam.paramInfo[1].step = 0x4;                                 /*word*/

    devObjPtr->tablesInfo.ttiAction.commonInfo.baseAddress = 0x02900000;
    devObjPtr->tablesInfo.ttiAction.paramInfo[0].step = 0x10;   /*entry*/
    devObjPtr->tablesInfo.ttiAction.paramInfo[1].step = 0x4;    /*word*/

    devObjPtr->tablesInfo.mllTable.commonInfo.baseAddress = 0x0C880000;
    devObjPtr->tablesInfo.mllTable.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.mllOutInterfaceConfig.commonInfo.baseAddress = 0x0C800980;
    devObjPtr->tablesInfo.mllOutInterfaceConfig.paramInfo[0].step = 0x100;

    devObjPtr->tablesInfo.mllOutInterfaceCounter.commonInfo.baseAddress = 0x0C800900;
    devObjPtr->tablesInfo.mllOutInterfaceCounter.paramInfo[0].step = 0x100;

    devObjPtr->memUnitBaseAddrInfo.mllRegBase = 0x0C800000;

    devObjPtr->tablesInfo.trunkNumOfMembers.commonInfo.baseAddress = 0x0b009000;
    devObjPtr->tablesInfo.trunkNumOfMembers.paramInfo[0].step = 0x4;
    devObjPtr->tablesInfo.trunkNumOfMembers.paramInfo[0].divider = 8;

    devObjPtr->tablesInfo.trunkMembers.commonInfo.baseAddress = 0x0b400000;
    devObjPtr->tablesInfo.trunkMembers.paramInfo[0].step       = 0x4 ;/*member*/
    devObjPtr->tablesInfo.trunkMembers.paramInfo[1].step       = 0x20;/*trunkId*/

    devObjPtr->tablesInfo.ieeeRsrvMcCpuIndex.commonInfo.baseAddress = 0x02000804;
    devObjPtr->tablesInfo.ieeeRsrvMcCpuIndex.paramInfo[0].step = 0x1000;
    devObjPtr->tablesInfo.ieeeRsrvMcCpuIndex.paramInfo[0].divider = 16;

    devObjPtr->tablesInfo.routeNextHopAgeBits.commonInfo.baseAddress = 0x0280100C; /*Router Next Hop Table Age Bits*/
    devObjPtr->tablesInfo.routeNextHopAgeBits.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.portProtocolVidQoSConf.commonInfo.baseAddress = 0x0B810800;
    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[0].step = 0x20;
    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[1].step = 0x4;
}


