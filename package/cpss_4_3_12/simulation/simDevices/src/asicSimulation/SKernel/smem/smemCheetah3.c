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
* @file smemCheetah3.c
*
* @brief Cheetah3 memory mapping implementation.
*
* @version   128
********************************************************************************
*/
#include <os/simTypes.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah3.h>
#include <asicSimulation/SKernel/smem/smemAc5.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahL2.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Pcl.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <common/Utils/Math/sMath.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SLog/simLog.h>

#define XCAT_A1_VLAN_NUM_WORDS_CNS  6

/* Policer counting entry size in words for xCat and above devices */
#define XCAT_POLICER_CNTR_ENTRY_NUM_WORDS_CNS 8

/* 1 Billion nanoseconds in a second */
#define NANO_SEC_MAX_VALUE_CNS  1000000000

/* memory units */

/* calc sign of bit in table 'Metering Conformance Level Sign Memory' (without byte count considerations)*/
#define SIP_5_15_CONF_SIGN_CALC(bucket_size,mru)  \
        ((bucket_size > mru) ? 1 : 0)

/* policer xcat 2 size */
#define POLICER_XCAT2_SIZE_CNS         0x237F

static void smemCht3WritePolicyTcamData
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  lineNumber ,
    IN         GT_U32  tcamDataType ,
    IN         GT_U32  tcamCompMode
);

static void smemCht3ReadWritePolicyTcam
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  lineNumber,
    IN         GT_U32  tcamDataType,
    IN         GT_U32  rdWr,
    IN         GT_U32  tcamCompMode
);

static void smemCht3ReadWriteRouterTTTcam
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_BOOL  writeAction,
    IN         GT_U32  rtLineNumber,
    IN         GT_U32  rtDataType,
    IN         GT_U32  rtBankEnBmp,
    IN         GT_U32  rtValidBit ,
    IN         GT_U32  rtCompMode,
    IN         GT_U32  rtSpare
);
static void smemCht3ReadPolicyTcamData
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type
);

static void smemCht3ReadWritePclAction
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
);

/* non valid address to cause no match when looking for valid address */
#define NON_VALID_ADDRESS_CNS           0x00000001

#define     PCL_ACTION_TBL_REGS_NUM(dev)        (4 * (dev->pclTcamInfoPtr->bankEntriesNum *  dev->pclTcamInfoPtr->bankNumbers))

/* sampled at reset register */
#define SAMPLED_AT_RESET_ADDR  0x00000028

/* MAC Counters address mask */
#define     XG_MIB_COUNT_MSK_CNS              0xff81ff00



/**
* @internal smemCht3Init2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemCht3Init2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  port;
    GT_U32  currAddr;

    /* read the registers - as was set during load of the 'Registers defaults file' */
    if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /*Bridge Global Configuration2*/
        currAddr = SMEM_CHT_BRIDGE_GLOBAL_CONF2_REG(devObjPtr);
        smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);

        /*Pre-Egress Engine Global Configuration*/
        currAddr = SMEM_CHT_PRE_EGR_GLB_CONF_REG(devObjPtr);
        smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);
    }

    /*Ingress Policy Global Configuration*/
    currAddr = SMEM_CHT_PCL_GLOBAL_REG(devObjPtr);
    smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);
    /*TTI Global Configuration*/
    currAddr = SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr);
    smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);
    /*HA Global Configuration*/
    currAddr = SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr);
    smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);

    if(devObjPtr->policerSupport.supportPolicerMemoryControl)
    {
        /* Policer Memory Control */
        if (SKERNEL_IS_LION2_DEV(devObjPtr))
        {
            currAddr = SMEM_XCAT_PLR_HIERARCHICAL_POLICER_CTRL_REG(devObjPtr,  0);  /* Hierarchical Policer control register */
        }
        else
        {
            currAddr = SMEM_CHT_POLICER_GLB_CONF_REG(devObjPtr, 0 /* cycle */);
        }
        /* update the register , using the SCIB interface so the
          'Active memory' will be called --> meaning that function
           smemXCatActiveWritePolicerMemoryControl(...) will be called */
        smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);
    }

    if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* check the port registers to see what ports uses GE/XG registers */
        for(port = 0 ; port < devObjPtr->portsNumber  ; port++)
        {
            if(
               (0 == IS_CHT_HYPER_GIGA_PORT(devObjPtr,port)))
            {
                /* no XG mac for this port */
                continue;
            }

            currAddr = SMEM_XCAT_XG_MAC_CONTROL3_REG(devObjPtr, port);
            /* update the register back , using the SCIB interface so the
              'Active memory' will be called --> meaning that function
               smemXcatActiveWriteMacModeSelect(...) will be called */
            smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);
        }
    }

    if (devObjPtr->errata.supportSrcTrunkToCpuIndicationEnable)
    {
        currAddr = SMEM_XCAT_TTI_INTERNAL_METAL_FIX_REG(devObjPtr);
        /* write the register back , using the SCIB interface so the
          'Active memory' will be called --> meaning that function
           smemXCatActiveWriteTtiInternalMetalFix(...) will be called */
        smemRegUpdateAfterRegFile(devObjPtr,currAddr,1);
    }

    return;
}


/**
* @internal smemCht3ActiveWriteRouterAction function
* @endinternal
*
* @brief   The Tcam router engine table and TT write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemCht3ActiveWriteRouterAction
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     rtLineNumber;    /* Router Tcam Line Number */
    GT_U32     rtWr;            /* Write operation */
    GT_U32     rtDataType;      /* Router Tcam data type */
    GT_U32     rtValid;          /* TCAM valid entry bit */
    GT_U32     rtBankEnBmp;     /* bmp of enabled banks */
    GT_U32     rtCompMode;      /* TCAM compare mode , IPv4 ... IPv6 */
    GT_U32     rtSpare;         /* Entry spare bit */
    GT_U32     actionTarget;    /*Target Data Structure.
                                  0x0 = Router TCAM; RouterTCAM Write.
                                  0x1 = LTT; Action Table access for read or write.
                                  0x2 = RouterECC Table;*/

    GT_U32     fldValue;

    /* Read router and TT tcam Access Control Register 1*/
    rtLineNumber = (*inMemPtr >> 2) & 0x3FFF;
    rtDataType = (*inMemPtr >> 21) & 0x1;
    rtBankEnBmp = (*inMemPtr >> 16) & 0xf;
    rtWr = (*inMemPtr & 0x1);    /* write action */
    actionTarget = (*inMemPtr >> 22) & 0x3;

    /* Read router and TT tcam Access Control Register 0*/
    smemRegGet(devObjPtr, SMEM_CHT3_ROUTER_ACCESS_DATA_CTRL_REG(devObjPtr) , &fldValue);
    rtCompMode = (fldValue >> 16) & 0xff;
    rtValid    = (fldValue & 0xf);
    rtSpare    = (fldValue >> 8) & 0xf;

    switch(actionTarget)
    {
        case 0:
            smemCht3ReadWriteRouterTTTcam(devObjPtr, rtWr ?  GT_TRUE : GT_FALSE,rtLineNumber, rtDataType,
                                        rtBankEnBmp, rtValid, rtCompMode,rtSpare);
            break;
        case 1:
            /* accessing the LLT  */

            /* not implemented -- see CQ# 105927 */
            break;
        case 2:
            /* accessing the ECC */

            /* not need to be implemented , since no ECC meaning in simulation */
            break;
        case 3:
        default:
            /* should not happen */

            /* simulation ignore it */
            break;
    }


    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemCht3ActiveWritePclAction function
* @endinternal
*
* @brief   The action table and policy TCAM write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemCht3ActiveWritePclAction
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     lineNumber;      /* line number in TCAM and policy action table*/
    GT_U32     rdWr;            /* Read or write operation */
    GT_U32     rdWrTarget;      /* Read or Write Target Data Structure(Tcam/Action) */
    GT_U32     tcamDataType;    /* TCAM data type */
    GT_U32     tcamCompMode;    /* TCAM compare mode , IPv4 ... IPv6 */

    /* Read Action Table and Policy TCAM Access Control Register */
    rdWrTarget = (*inMemPtr >> SMEM_CHT_GROUP_INDEX_FIRST_BIT_CNS) & 0x3;
    tcamDataType = (*inMemPtr >> 15) & 0x1;
    lineNumber = ((*inMemPtr >> 2) & 0x1FFF) | (((*inMemPtr >> 18) & 1 ) << 13);
    rdWr = (*inMemPtr >> 1) & 0x1;    /* 0 - read , 1 - write*/
    tcamCompMode = (*inMemPtr >> 19) & 0x1;

    /* stored as is to be retrieved in called functions */
    *memPtr = *inMemPtr ;

    if(SKERNEL_IS_AC5_BASED_DEV(devObjPtr))
    {
        /* state that TCAM access is in context according to the address that CPU access */
        smemAc5PclTcamByAddrContextSet(devObjPtr,address);
    }


    if (rdWrTarget == 0) /* Read or Write the TCAM policy table */
    {
        smemCht3ReadWritePolicyTcam(devObjPtr, lineNumber,
                                    tcamDataType, rdWr,tcamCompMode);
    }
    else if(rdWrTarget == 1)
    {   /* Read or Write the PCL action table */
        smemCht3ReadWritePclAction(devObjPtr, lineNumber, rdWr);
    }
    else
    {
        /* ECC not supported */
    }

    if(SKERNEL_IS_AC5_BASED_DEV(devObjPtr))
    {
        /* state that TCAM access is out of the client context */
        smemAc5PclTcamClientContextSet(devObjPtr,SIP5_TCAM_CLIENT_LAST_E);
    }


    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}


/**
* @internal smemCht3ActiveReadTcamBistConfigAction function
* @endinternal
*
* @brief   Policy/Router TCAM BIST config read access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] outMemPtr                - Pointer to the memory to get register's content.
*/
void smemCht3ActiveReadTcamBistConfigAction
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * outMemPtr
)
{
    /* Simulate BIST done */
    *outMemPtr = 0x1028B;
}

/**
* @internal smemCht3ActiveWritePolicerTbl function
* @endinternal
*
* @brief   The Policers table write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemCht3ActiveWritePolicerTbl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 trigCmd;             /* type of triggered action that the Policer
                                   needs to perform */
    GT_U32 entryOffset;         /* the offset from the base address of the
                                   accessed entry */
                                /* device memory pointer */
    GT_U32 * regPtr;            /* table entry pointer */
    GT_U32 regAddr;             /* table base address */
    GT_U32 cycle = param;       /* ingress policer unit (could be 0 or 1) */

    trigCmd = SMEM_U32_GET_FIELD(*inMemPtr, 1, 3);
    entryOffset = SMEM_U32_GET_FIELD(*inMemPtr, 16, 16);

    switch (trigCmd)
    {
    /* Counter entry will be read to the Ingress Policer Table Access Data Registers */
    case 0:
    case 1:
        regAddr = SMEM_CHT_POLICER_CNT_TBL_MEM(devObjPtr, cycle, entryOffset);
        regPtr = smemMemGet(devObjPtr, regAddr);

        regAddr = SMEM_CHT3_INGRESS_POLICER_TBL_DATA_TBL_MEM(devObjPtr, cycle, 0);
        smemMemSet(devObjPtr, regAddr, regPtr, 8);
        if (trigCmd == 0)
        {
            /* The counter entry will be reset and written back to memory. */
            memset(regPtr, 0, 8 * sizeof(GT_U32));
        }
        break;

    /* Sets metering entry fields, according to the information placed in the
       Ingress Policer Table Access Data Registers. */
    case 4:
        regAddr = SMEM_CHT3_INGRESS_POLICER_TBL_DATA_TBL_MEM(devObjPtr, cycle, 0);
        regPtr = smemMemGet(devObjPtr, regAddr);

        regAddr = SMEM_CHT_POLICER_TBL_MEM(devObjPtr, cycle, entryOffset);
        smemMemSet(devObjPtr, regAddr, regPtr, 8);
        break;

    /* Refresh the metering entry status fields */
    case 5:
        break;
    default:
        return;
    }

    /* Cleared by the device when the read or write action is completed. */
    SMEM_U32_SET_FIELD(*inMemPtr, 0, 1, 0);

    *memPtr = *inMemPtr;
}

/**
* @internal smemXCatPolicerIPFixTimeStampUpload function
* @endinternal
*
* @brief   Upload Policer IPFix timestamp
*       read the memory of : SMEM_XCAT_POLICER_TIMER_TBL_MEM
*/
static GT_VOID smemXCatPolicerIPFixTimeStampUpload
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN  GT_U32 cycle,
    INOUT GT_U32 * inMemPtr
)
{
    GT_U32 clocks;              /* Current clock value */
    GT_U32 seconds;             /* seconds in clocks */
    GT_U32 nanoSeconds;         /* nanoseconds in clocks */
    GT_U64 added_seconds64;     /* 64 bits seconds */
    GT_U64 orig_seconds64;      /* 64 bits seconds in inMemPtr*/
    GT_U64 new_seconds64;       /* 64 bits seconds new value to write to inMemPtr */
    GT_U32 orig_nanoSeconds;    /* nanoseconds in orig memory */
    GT_U32 new_nanoSeconds;     /* nanoseconds in inMemPtr */
    GT_U32 currTicks;

    /* Current clock value */
    currTicks = SIM_OS_MAC(simOsTickGet)();

    /* we need to add to SMEM_XCAT_POLICER_TIMER_TBL_MEM the time that elapsed
        since the time that the application configured the clock

        so calc the time that elapse
    */
    clocks = currTicks - devObjPtr->clockWhenPolicerTimerTblWasSet[cycle];

    /* Convert tick clocks to seconds and nanoseconds */
    SNET_TOD_CLOCK_FORMAT_MAC(clocks, seconds, nanoSeconds);

    /* NOTE: inMemPtr is pointing to :  SMEM_XCAT_POLICER_TIMER_TBL_MEM(devObjPtr, cycle, 6);*/

    /* IPfix nano timer is incremented by the value of time stamp upload registers */
    orig_nanoSeconds = inMemPtr[0];

    new_nanoSeconds = nanoSeconds + orig_nanoSeconds;

    if(new_nanoSeconds >= NANO_SEC_MAX_VALUE_CNS)
    {
        new_nanoSeconds -= NANO_SEC_MAX_VALUE_CNS;
        seconds += 1;
    }

    /* Convert timer data to 64 bit value*/
    added_seconds64.l[0] = seconds;
    added_seconds64.l[1] = 0;

    /* Second timer low word [31:0] */
    orig_seconds64.l[0] = inMemPtr[1];
    /* Second timer hi word [63:32] */
    orig_seconds64.l[1] = inMemPtr[2];

    /* IPfix timer is incremented by the value of time stamp upload registers */
    new_seconds64 =
        prvSimMathAdd64(added_seconds64, orig_seconds64);

    /* write the updated values back to the memory */
    inMemPtr[0] = new_nanoSeconds;
    inMemPtr[1] = new_seconds64.l[0];
    inMemPtr[2] = new_seconds64.l[1];

    /* as we updated the memory , we need to update the time that we did that */
    devObjPtr->clockWhenPolicerTimerTblWasSet[cycle] = currTicks;
}

#define   READ_AND_UPDATE_BILLING_ENTRY_CNS 0x80
enum{
    CounterReadAndReset = 0 ,/* Counter entry is read to the Ingress Policer Table Access Data Registers.The counter entry is reset and written back to memory.*/
    CounterReadOnly = 1 , /*Counter entry is read to the Ingress Policer Table Access Data Registers. No write-back is performed.*/
    CounterBufferFlush = 2, /*Counter write-back buffer is flushed.*/
    MeteringIndirectRead = 3, /*Metering entry is read to the Ingress Policer Table Access Data Registers.*/
    MeteringUpdate = 4,/*Sets metering entry fields, according to the information placed in the Ingress Policer Table Access Data registers excluding LastUpdateTime/ByteCount value that is calculated by the pipe.; in BOBK was internal (since didnt work). fixed.*/
    MeteringRefresh = 5,/*Refresh metering entry status fields.*/
    MeteringIndirectWrite = 7, /*Sets metering entry fields, according to the information placed in the Ingress Policer Table Access Data registers.*/

    /*sip 5.20*/
    CounterWrite = CounterBufferFlush,/*2*/ /*CounterBufferFlush; Counter write-back buffer is written.*/
    ManagementCountersRead = 8,
    ManagementCountersReset = 9,
    ManagementCountersReadAndReset = 10,
};

/**
* @internal smemXCatActiveWritePolicerTbl function
* @endinternal
*
* @brief   The Policers table write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWritePolicerTbl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    DECLARE_FUNC_NAME(smemXCatActiveWritePolicerTbl);
    GT_U32 trigCmd;             /* type of triggered action that the Policer
                                   needs to perform */
    GT_U32 entryOffset;         /* the offset from the base address of the
                                   accessed entry */
                                /* device memory pointer */
    GT_U32 * regPtr;            /* table entry pointer */
    GT_U32 regAddr;             /* table base address */
    GT_U32 cntMode;             /* counter mode: Billing/IPfix */
    GT_U32 direction;           /* Ingress/Egress policer engine */
    GT_U32 cycle;               /* Ingress policer cycle */
    GT_U32 clockVal;            /* Number of clock ticks */
    GT_U32 goodPckts, timeStamp, dropPckts; /* good packets, time stamp, drop packets value */
    GT_U64 bytesCnt;            /* bytes counter value */
    GT_U32*entryPtr;            /* pointer to entry in memory */
    GT_U32 entryAddr;           /* address of counting entry */
    GT_U32 entry[XCAT_POLICER_CNTR_ENTRY_NUM_WORDS_CNS]; /* memory to store counting entry */
    GT_U32 countingEntryFormat; /* Long or short format of counting entry */
    GT_U32 wordsArr[4];
    GT_U32 meterEntryAddr;
    GT_U32 meterCfgEntryAddr;
    GT_U32 counterEntryAddr;
    GT_U32 max_bucket_size0, max_bucket_size1; /* maximal bucket sizes configured by application */
    GT_U32 bucket_size0, bucket_size1;         /* internal current sizes of leaky  buckets       */
    GT_U32 rate_type0, rate_type1;             /* rate types (maximal bucket size resolution)    */
    GT_U32 bytesFactor0, bytesFactor1;         /* calculated maximal bucket size resolution      */
    GT_U32 tokenBucket0Sign , tokenBucket1Sign;/* calculated TB 0,1 sign */
    GT_U32 policerMru;
    GT_U32 policer_mode;                      /* policer mode */
    GT_U32 meterBaseAddr = 0;    /* metering table base address */
    GT_U32 countingBaseAddr =0;  /* counting table base address */
    GT_U32 meterMemoryCycle;            /* policer stage for meter table access */
    GT_U32 meterConfigMemoryCycle=0;    /* policer stage for meter config table access */
    GT_U32 counterMemoryCycle;/* policer stage for counting tables access */
    GT_U32 counterIndex;/*management counter index */
    GT_U32 counterSetId;  /*management counter set Id */
    GT_U32 countingMode;/* the global (per PLR unit) counting Mode */
    GT_U32 supportAction = GT_TRUE;
    GT_U32 sampleMode;  /* IPFIX Sampling mode */

    if(0 == SMEM_U32_GET_FIELD(*inMemPtr, 0, 1))
    {
        /* the Trigger bit is '0' --> no action triggered */

        /* just update the control register content */
        *memPtr = *inMemPtr;
        return;
    }

    /* Ingress/Egress Policer engine */
    direction = (param == 2) ?
            SMAIN_DIRECTION_EGRESS_E : SMAIN_DIRECTION_INGRESS_E;

    /* Ingress Policer cycle number */
    meterMemoryCycle = cycle = param;

    if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        trigCmd = SMEM_U32_GET_FIELD(*inMemPtr, 1, 3);
    }
    else
    {
        trigCmd = SMEM_U32_GET_FIELD(*inMemPtr, 1, 4);
    }

    if(devObjPtr->errata.plrIndirectReadCountersOnDisabledMode &&
        (trigCmd == CounterReadAndReset ||
         trigCmd == CounterReadOnly))
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_POLICER_GLB_CONF_REG(devObjPtr,cycle),
            1,2,&countingMode);
        /* get <Counting Mode> */

        if(countingMode == 0) /* counting disabled */
        {
            /* do not start operation */
            /* do not clear the bit   */
            __LOG(("Errata : indirect action [%s] not triggering action (and not reset the 'Trigger/Status' bit) \n",
                 (char*)((trigCmd == CounterReadAndReset) ? "CounterReadAndReset" :
                                                            "CounterReadOnly")));

            /* just update the control register content (without clearing the trigger bit) */
            *memPtr = *inMemPtr;

            return;
        }
    }


    entryOffset = SMEM_U32_GET_FIELD(*inMemPtr, 16, 16);

    /* check Update Billing Entry Counter Mode bit for Lion B0 and above
       but only when Trigger Command  is 0x0 = CounterReadAndReset.
       Value 1 of this bit means read entry to Policer Table Access Data Registers
       and update entry by content of Policer Table Access Data Registers. */
    if (SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr) &&
        (CounterReadAndReset == trigCmd) &&
        (1 == SMEM_U32_GET_FIELD(*inMemPtr, 7, 1)))
    {
        /* read and update command */
        trigCmd = READ_AND_UPDATE_BILLING_ENTRY_CNS;
    }

    meterConfigMemoryCycle = cycle;
    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
    {

        if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* read metering base address */
            smemRegFldGet(devObjPtr,
                          SMEM_LION3_POLICER_METERING_BASE_ADDR_REG(devObjPtr, cycle),
                          0, 24, &meterBaseAddr);
        }
        else
        {
            meterBaseAddr = SMEM_LION3_POLICER_METERING_BASE_ADDR____FROM_DB(devObjPtr,cycle);
        }

        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* meter memories became shared between ALL PLR starting from SIP 5.20 (Bobcat3)
               Direct access done through IPLR0. And base address is used to get absolute address in memory.  */
            meterMemoryCycle = 0;
        }
        else
        {
            /* meter memories became shared between IPLR0 and IPLR1 starting from SIP 5.15 (BobK)
               Direct access done through IPLR0. And base address is used to get absolute address in memory.  */
            meterMemoryCycle = (cycle == 1) ? 0 : cycle;
        }
        if (devObjPtr->policerSupport.isMeterConfigTableShared[cycle] == GT_TRUE)
        {
            meterConfigMemoryCycle = meterMemoryCycle;
        }
        else
        {
            meterConfigMemoryCycle = cycle;
        }

        meterEntryAddr   = SMEM_XCAT_POLICER_METER_ENTRY_TBL_MEM(devObjPtr, direction, meterMemoryCycle, entryOffset + meterBaseAddr);

        __LOG(("In case of meter update write data to IPLR[%d] index %d\n", meterMemoryCycle, (entryOffset + meterBaseAddr)));
    }
    else
    {
        meterEntryAddr   = SMEM_XCAT_POLICER_METER_ENTRY_TBL_MEM(devObjPtr, direction, cycle, entryOffset);
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        countingBaseAddr = SMEM_LION3_POLICER_COUNTING_BASE_ADDR____FROM_DB(devObjPtr,cycle);

        /* counting memories became shared between ALL PLR starting from SIP 5.20 (Bobcat3)
           Direct access done through IPLR0. And base address is used to get absolute address in memory.  */
        counterMemoryCycle = 0;
        counterEntryAddr   = SMEM_XCAT_POLICER_CNT_ENTRY_TBL_MEM(devObjPtr, direction, counterMemoryCycle, entryOffset + countingBaseAddr);
        __LOG(("In case of meter update write data to IPLR[%d] index %d\n", counterMemoryCycle, (entryOffset + countingBaseAddr)));
    }
    else
    {
        counterEntryAddr = SMEM_XCAT_POLICER_CNT_ENTRY_TBL_MEM  (devObjPtr, direction, cycle, entryOffset);
    }

    if(counterEntryAddr == SMAIN_NOT_VALID_CNS ||
       meterEntryAddr == SMAIN_NOT_VALID_CNS)
    {
        skernelFatalError( "smemXCatActiveWritePolicerTbl: plr[%d] have no memory. Check Policer Memory Control configuration \n",
            cycle);
    }


    switch (trigCmd)
    {
    /* Counter entry will be read to the Policer Table Access Data Registers */
    case CounterReadAndReset:
    case CounterReadOnly:
        __LOG(("PLR[%d] : action [%s] triggered \n",
            cycle,
             (char*)((trigCmd == CounterReadAndReset) ? "CounterReadAndReset" :
                                                        "CounterReadOnly")));
        entryOffset += countingBaseAddr;

        regAddr = counterEntryAddr;
        regPtr = smemMemGet(devObjPtr, regAddr);
        /* pointer to the registers that CPU read/write for 'indirect access' */
        regAddr = SMEM_XCAT_POLICER_INDIRECT_ACCESS_DATA_ENTRY_REG(devObjPtr, cycle, 0);
        smemMemSet(devObjPtr, regAddr, regPtr,  (devObjPtr->tablesInfo.policer.paramInfo[0].step / 4));
        if (trigCmd == CounterReadAndReset)
        {
            countingEntryFormat =
                snetXCatPolicerCountingEntryFormatGet(devObjPtr, direction, cycle);
            if (countingEntryFormat)    /* Short entry format */
            {
                cntMode = 0;  /* set Billing mode in "short" format, IPFix not supported */
            }
            else    /* Full entry format */
            {

                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    cntMode =
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_GET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_COUNTER_MODE_E);
                }
                else
                {
                    /* Counter Mode */
                    cntMode = SMEM_U32_GET_FIELD(regPtr[7], 4, 1);
                }
            }
            /* IPfix */
            if(cntMode)
            {
                goodPckts = dropPckts = 0;
                bytesCnt.l[0] = bytesCnt.l[1] = 0;
                clockVal = SNET_XCAT_IPFIX_TOD_CLOCK_GET_MAC(devObjPtr, cycle);

                /* Convert clock value in ticks to time stamp format */
                snetXcatIpfixTimestampFormat(devObjPtr, clockVal, &timeStamp);

                if(!devObjPtr->errata.plrIndirectReadAndResetIpFixCounters)
                {
                    /* The <Packet Counter>, <Byte Counter>, and <Dropped Packet Counter> fields are cleared */
                    snetXcatIpfixCounterWrite(devObjPtr, NULL/*no descriptor to give*/ , regPtr, &bytesCnt, goodPckts,
                                              timeStamp, dropPckts);

                    /* The <Last Sampled Value> is cleared */
                    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                    {
                        sampleMode = SMEM_LION3_PLR_IPFIX_ENTRY_FIELD_GET(devObjPtr,regPtr,
                                                                          entryOffset,
                                                                          SMEM_LION3_PLR_IPFIX_TABLE_FIELDS_SAMPLING_MODE_E);

                        /* field is multiplexed with new fields in Hawk those need not be cleared */
                        if(!SMEM_CHT_IS_SIP6_10_GET(devObjPtr) || (sampleMode != 0))
                        {
                            wordsArr[0] = wordsArr[1] = 0;
                            SMEM_LION3_PLR_IPFIX_ENTRY_LARGE_FIELD_SET(devObjPtr,regPtr,
                                entryOffset,
                                SMEM_LION3_PLR_IPFIX_TABLE_FIELDS_LAST_SAMPLED_VALUE_E,
                                wordsArr);
                        }
                    }
                    else
                    {
                        SMEM_U32_SET_FIELD(regPtr[4], 2, 30, 0);
                        SMEM_U32_SET_FIELD(regPtr[5], 0, 2, 0);
                        SMEM_U32_SET_FIELD(regPtr[5], 2, 4, 0);
                    }
                }
                else
                {
                    /* time stamp is updated as usual */
                    SMEM_LION3_PLR_IPFIX_ENTRY_FIELD_SET(devObjPtr,regPtr,
                                    SMAIN_NOT_VALID_CNS,
                                    SMEM_LION3_PLR_IPFIX_TABLE_FIELDS_TIME_STAMP_E,
                                    timeStamp);

                    __LOG(("Errata : indirect action CounterReadAndReset does not reset <Packet Counter>, <Byte Counter>,  <Dropped Packet Counter> and <Last Sampled Value>.\n"));
                }
            }
            else
            {
                /* Bobcat2 A0/B0 devices (SIP 5.0, 5.10) resets all fields besides counter mode
                   but not only counters.
                   This misbehaviour was fixed in BobK (SIP 5.15) */
                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /* reset all fields , exclude : SMEM_LION3_PLR_BILLING_TABLE_FIELDS_COUNTER_MODE_E */

                    wordsArr[0] = wordsArr[1] = 0;
                    /* Green Counter */
                    SMEM_LION3_PLR_BILLING_ENTRY_LARGE_FIELD_SET(devObjPtr,regPtr,
                        entryOffset,
                        SMEM_LION3_PLR_BILLING_TABLE_FIELDS_GREEN_COUNTER_E,
                        wordsArr);
                    /* Yellow Counter */
                    SMEM_LION3_PLR_BILLING_ENTRY_LARGE_FIELD_SET(devObjPtr,regPtr,
                        entryOffset,
                        SMEM_LION3_PLR_BILLING_TABLE_FIELDS_YELLOW_COUNTER_E,
                        wordsArr);
                    /* Red Counter */
                    SMEM_LION3_PLR_BILLING_ENTRY_LARGE_FIELD_SET(devObjPtr,regPtr,
                        entryOffset,
                        SMEM_LION3_PLR_BILLING_TABLE_FIELDS_RED_COUNTER_E,
                        wordsArr);

                    if (!SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
                    {
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_BILLING_COUNTERS_MODE_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_BILLING_COUNT_ALL_EN_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_LM_COUNTER_CAPTURE_MODE_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_LARGE_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_GREEN_COUNTER_SNAPSHOT_E,
                            wordsArr);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_GREEN_COUNTER_SNAPSHOT_VALID_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_PACKET_SIZE_MODE_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_TUNNEL_TERMINATION_PACKET_SIZE_MODE_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_INCLUDE_LAYER1_OVERHEAD_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_DSA_TAG_COUNTING_MODE_E,
                            0);
                        SMEM_LION3_PLR_BILLING_ENTRY_FIELD_SET(devObjPtr,regPtr,
                            entryOffset,
                            SMEM_LION3_PLR_BILLING_TABLE_FIELDS_TIMESTAMP_TAG_COUNTING_MODE_E,
                            0);
                    }
                }
                else
                {
                    /* Green Counter */
                    regPtr[0] = 0;
                    SMEM_U32_SET_FIELD(regPtr[1], 0, 10, 0);
                    /* Yellow Counter */
                    SMEM_U32_SET_FIELD(regPtr[1], 10, 22, 0);
                    SMEM_U32_SET_FIELD(regPtr[2], 0, 20, 0);
                    /* Red Counter */
                    SMEM_U32_SET_FIELD(regPtr[2], 20, 12, 0);
                    SMEM_U32_SET_FIELD(regPtr[3], 0, 30,  0);
                }
            }
        }

        break;

    case CounterBufferFlush: /* CounterWrite in sip 5.20 */
        if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            __LOG(("PLR[%d] : action [%s] triggered \n",
                cycle,
                 "CounterBufferFlush"));
            /* simulation hold no 'cache' ... */

            /* in sip5.20 this action moved to bit 13 in 'policerCtrl1' */

            break;
        }

        __LOG(("PLR[%d] : action [%s] triggered \n",
            cycle,
             "CounterWrite"));

        /*this is case of CounterWrite :*/
        /* content of Policer Table Access Data Registers will be written to counting entry .*/
        entryAddr = counterEntryAddr;
        entryPtr = smemMemGet(devObjPtr, entryAddr);

        /* pointer to the registers that CPU write for 'indirect access' */
        regAddr = SMEM_XCAT_POLICER_INDIRECT_ACCESS_DATA_ENTRY_REG(devObjPtr, cycle, 0);
        regPtr = smemMemGet(devObjPtr, regAddr);

        /* write content of Policer Table Access Data Registers to counting entry */
        smemMemSet(devObjPtr, entryAddr, regPtr, (devObjPtr->tablesInfo.policerCounters.paramInfo[0].step / 4));

        break;
    /* Sets metering entry fields, according to the information placed in the
       Policer Table Access Data Registers. */
    case MeteringUpdate:
        __LOG(("PLR[%d] : action [%s] triggered \n",
            cycle,
             "MeteringUpdate"));
        /* pointer to the registers that CPU read/write for 'indirect access' */
        regAddr = SMEM_XCAT_POLICER_INDIRECT_ACCESS_DATA_ENTRY_REG(devObjPtr, cycle, 0);
        regPtr = smemMemGet(devObjPtr, regAddr);

        /* copy to table entry */
        entryOffset += meterBaseAddr;
        regAddr = meterEntryAddr;
        smemMemSet(devObjPtr, regAddr, regPtr, (devObjPtr->tablesInfo.policer.paramInfo[0].step / 4));

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddr = SMEM_XCAT_POLICER_MRU_REG(devObjPtr,cycle);
            smemRegFldGet(devObjPtr, regAddr, 0, 32, &policerMru);

            /* update bucket_size0-1 fields in table entry */
            regPtr = smemMemGet(devObjPtr, meterEntryAddr);

            if(0 == SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                devObjPtr, regPtr, entryOffset,
                SMEM_LION3_PLR_METERING_TABLE_FIELDS_RATE0))
            {
                __LOG_NO_LOCATION_META_DATA(("since the rate0 is 0 do not auto fill bucket0 (with burst size)\n"));
                /* the rate is 0 so no one going to update the bucket */
                bucket_size0 = SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0);
            }
            else  /* fill the bucket to match the burst size */
            {
                __LOG_NO_LOCATION_META_DATA(("fill the bucket 0 to match the burst size\n"));
                max_bucket_size0 = SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE0);

                rate_type0 = SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_RATE_TYPE0);

                if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    bytesFactor0 = (rate_type0 == 0) ? 4 : 1 << (3*rate_type0);/*8^rate_type0;*/ /*4,8,64,512,4K,32K */
                }
                else
                {
                    bytesFactor0 = 1 << (3*rate_type0);/*8^rate_type0;*/ /*1,8,64,512,4K,32K */
                }

                bucket_size0 = max_bucket_size0 * bytesFactor0;
                if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
                {
                    /* the max_bucket_size0 is actually treated as value that need to add MRU to it */
                    bucket_size0 += policerMru;
                }

                SMEM_LION3_PLR_METERING_ENTRY_FIELD_SET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE0,
                    bucket_size0);
            }

            if(0 == SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                devObjPtr, regPtr, entryOffset,
                SMEM_LION3_PLR_METERING_TABLE_FIELDS_RATE1))
            {
                __LOG_NO_LOCATION_META_DATA(("since the rate1 is 0 do not auto fill bucket1 (with burst size)\n"));
                bucket_size1 = SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1);
            }
            else  /* fill the bucket to match the burst size */
            {
                __LOG_NO_LOCATION_META_DATA(("fill the bucket 1 to match the burst size\n"));

                max_bucket_size1 = SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_MAX_BURST_SIZE1);

                rate_type1 = SMEM_LION3_PLR_METERING_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_RATE_TYPE1);

                if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    bytesFactor1 = (rate_type1 == 0) ? 4 : 1 << (3*rate_type1);/*8^rate_type1;*/ /*4,8,64,512,4K,32K */
                }
                else
                {
                    bytesFactor1 = 1 << (3*rate_type1);/*8^rate_type1;*/ /*1,8,64,512,4K,32K */
                }

                bucket_size1 = max_bucket_size1 * bytesFactor1;
                if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
                {
                    /* the max_bucket_size0 is actually treated as value that need to add MRU to it */
                    bucket_size1 += policerMru;
                }

                SMEM_LION3_PLR_METERING_ENTRY_FIELD_SET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_LION3_PLR_METERING_TABLE_FIELDS_BUCKET_SIZE1,
                    bucket_size1);
            }

            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                meterCfgEntryAddr   = SMEM_SIP5_15_POLICER_CONFIG_ENTRY_TBL_MEM(
                    devObjPtr, meterConfigMemoryCycle, entryOffset);
                regPtr = smemMemGet(devObjPtr, meterCfgEntryAddr);
                /* envelope entry has at least one of these values not zero */
                policer_mode  = SMEM_SIP5_15_PLR_METERING_CONFIG_ENTRY_FIELD_GET(
                    devObjPtr, regPtr, entryOffset,
                    SMEM_SIP5_15_PLR_METERING_CONFIG_TABLE_FIELDS_POLICER_MODE_E);

                if ((policer_mode != SNET_LION3_POLICER_METERING_MODE_MEF10_3_START_E) &&
                    (policer_mode != SNET_LION3_POLICER_METERING_MODE_MEF10_3_NOT_START_E))
                {
                    /* not member of metering envelope */

                    /* update metering conformance level sign table fields in table entry */
                    regPtr = smemMemGet(devObjPtr,
                            SMEM_SIP5_15_POLICER_METERING_CONFORMANCE_LEVEL_SIGN_TBL_MEM(
                                devObjPtr, cycle, entryOffset));
                    tokenBucket0Sign = SIP_5_15_CONF_SIGN_CALC(bucket_size0,policerMru);
                    tokenBucket1Sign = SIP_5_15_CONF_SIGN_CALC(bucket_size1,policerMru);
                    SMEM_U32_SET_FIELD(*regPtr, 0, 1, tokenBucket0Sign);
                    SMEM_U32_SET_FIELD(*regPtr, 1, 1, tokenBucket1Sign);

                    __LOG_NO_LOCATION_META_DATA((
                        "update metering conformance level sign table fields in table entry\n"));
                    __LOG_NO_LOCATION_META_DATA((
                        "cycle[%d], entryOffset[%d], tokenBucket0Sign[%d], tokenBucket1Sign[%d] \n",
                        cycle, entryOffset, tokenBucket1Sign,tokenBucket0Sign));
                }
            }
        }
        break;

    /* Refresh the metering entry status fields */
    case MeteringRefresh:
        __LOG(("PLR[%d] : action [%s] triggered \n",
            cycle,
             "MeteringRefresh"));
        break;

    case ManagementCountersRead:/* 8 =  Management counters are read to the Policer Table Access Data Registers*/
    case ManagementCountersReset:/* 9 = Management counters are reset */
    case ManagementCountersReadAndReset:/*10 = read+reset */
        if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            __LOG(("PLR[%d] : management counters are not supported (removed) from all PLR units \n",
                cycle));
            goto caseNotValid_lbl;
        }
        __LOG(("PLR[%d] : action [%s] triggered \n",
            cycle,
             (char*)((trigCmd == ManagementCountersRead)  ? "ManagementCountersRead" :
                     (trigCmd == ManagementCountersReset) ? "ManagementCountersReset" :
                                                            "ManagementCountersReadAndReset")));

        entryOffset &= 0xF;/*lower 4 bits*/

        counterIndex = entryOffset & 0x3;
        counterSetId = entryOffset >> 2;

        entryAddr = SMEM_XCAT_POLICER_MNG_CNT_TBL_MEM(devObjPtr,cycle,counterSetId);
        entryAddr += counterIndex * 0x10;

        entryPtr = smemMemGet(devObjPtr, entryAddr);

        if(trigCmd != ManagementCountersReset)
        {
            __LOG_NO_LOCATION_META_DATA(("Read management counter table index [%d] \n",
                entryOffset));

            /* pointer to the registers that CPU read for 'indirect access' */
            regAddr = SMEM_XCAT_POLICER_INDIRECT_ACCESS_DATA_ENTRY_REG(devObjPtr, cycle, 0);
            smemMemSet(devObjPtr, regAddr, entryPtr,  4/* update 4 words (like GM do) */);
        }

        if(trigCmd == ManagementCountersRead)

        {
            break;
        }

        __LOG_NO_LOCATION_META_DATA(("Reset management counter table index [%d] \n",
            entryOffset));

        /* do the reset  */
        wordsArr[0] = wordsArr[1] = wordsArr[2] = wordsArr[3] = 0;
        entryPtr = &wordsArr[0];

        smemMemSet(devObjPtr, entryAddr, entryPtr, 4/* update 4 words (like GM do) */);

        break;


    case READ_AND_UPDATE_BILLING_ENTRY_CNS:
        __LOG(("PLR[%d] : action [%s] triggered \n",
            cycle,
             "Update Counting Entry"));
        /* Counter entry will be copied to the Policer Table Access Data
           Registers and content of Policer Table Access Data Registers
           will be written to counting entry .*/
        entryAddr = counterEntryAddr;
        entryPtr = smemMemGet(devObjPtr, entryAddr);
        /* pointer to the registers that CPU read/write for 'indirect access' */
        regAddr = SMEM_XCAT_POLICER_INDIRECT_ACCESS_DATA_ENTRY_REG(devObjPtr, cycle, 0);
        regPtr = smemMemGet(devObjPtr, regAddr);

        /* store content of Policer Table Access Data Registers */
        memcpy(entry, regPtr, sizeof(entry));

        /* write counting entry to Policer Table Access Data Registers */
        smemMemSet(devObjPtr, regAddr, entryPtr, (devObjPtr->tablesInfo.policerCounters.paramInfo[0].step / 4));

        /* write content of Policer Table Access Data Registers to counting entry */
        smemMemSet(devObjPtr, entryAddr, entry, (devObjPtr->tablesInfo.policerCounters.paramInfo[0].step / 4));
        break;
    default:
        caseNotValid_lbl :
        __LOG(("PLR[%d] : unsupported by simulation action [%d] triggered \n",
            cycle,trigCmd));
        supportAction = GT_FALSE;
        break;
    }

    if(supportAction == GT_TRUE)
    {
        /* Cleared by the device when the read or write action is completed. */
        SMEM_U32_SET_FIELD(*inMemPtr, 0, 1, 0);
    }

    *memPtr = *inMemPtr;
}

/**
* @internal smemXCatActiveReadIPFixNanoTimeStamp function
* @endinternal
*
* @brief   The IPFix Nano Time Of Day register read access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIPFixNanoTimeStamp
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    smemXCatPolicerIPFixTimeStampUpload(devObjPtr, param, memPtr);

    /* the smemXCatPolicerIPFixTimeStampUpload will update the content of the 3 words
        that hold the timer */
    *outMemPtr = * memPtr;
}

/**
* @internal smemXCatActiveReadIPFixSecMsbTimeStamp function
* @endinternal
*
* @brief   The IPFix Seconds MSB Time Of Day register read access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIPFixSecMsbTimeStamp
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* it used to be active memory , but when the smemXCatActiveReadIPFixNanoTimeStamp is activated
       the function will update also the memory of this register */
    *outMemPtr = * memPtr;
}

/**
* @internal smemXCatActiveReadIPFixSecLsbTimeStamp function
* @endinternal
*
* @brief   The IPFix Seconds LSB Time Of Day register read access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIPFixSecLsbTimeStamp
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* it used to be active memory , but when the smemXCatActiveReadIPFixNanoTimeStamp is activated
       the function will update also the memory of this register */
    *outMemPtr = * memPtr;
}

/**
* @internal smemXCatActiveReadIPFixSampleLog function
* @endinternal
*
* @brief   The IPFix IPFIX Sample Entries Log read access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIPFixSampleLog
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* Copy registers content to the output memory */
    *outMemPtr = *memPtr;
    /* Clear register after read */
    *memPtr = 0;
}

/**
* @internal smemXCatActiveReadPolicerManagementCounters function
* @endinternal
*
* @brief   Policer : when Read a Data Unit counter part(Entry LSB) of Management Counters Entry.
*         then value of LSB and MSB copied to Shadow registers
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadPolicerManagementCounters
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{

    GT_U32 regAddr;             /* memory address */

    /* Copy registers content to the output memory */
    *outMemPtr = *memPtr;

    if((address & 0xF0) > (0xB0))
    {

        /* address match the mask of : POLICER_MANAGEMENT_COUNTER_MASK_CNS
           but it is not part of the management counters active memory */
        return;
    }

    /* get register Policer Shadow<n> Register (n=01) */
    regAddr = SMEM_XCAT_POLICER_MANAGEMENT_COUNTERS_POLICER_SHADOW_REG(devObjPtr,param);

    /* copy the content of the next 2 word into the memory of :
       Policer Shadow<n> Register (n=01) */
    smemMemSet(devObjPtr,regAddr , &memPtr[1] , 2);

    return;
}


/**
* @internal smemXCatActiveReadIeeeMcConfReg function
* @endinternal
*
* @brief   IEEE Reserved Multicast Configuration register reading.
*         Reading of IEEE Reserved Multicast Configuration registers
*         should be performed from:
*         0x200X820 instead of 0x200X810 (table 2)
*         0x200X828 instead of 0x200X818 (table 3) - for xCat A1 and above
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIeeeMcConfReg
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32 regAddr;             /* memory address */

    if (devObjPtr->errata.ieeeReservedMcConfigRegRead)
    {
        /* get value of IEEE Reserved Multicast Configuration register */
        regAddr = address - 0x10;
        smemRegGet(devObjPtr,regAddr,outMemPtr);
    }
    else
    {
        *outMemPtr =  *memPtr;
    }
}

/**
* @internal smemChtGetIplrMemoryStep function
* @endinternal
*
* @brief   PLR function return the width in bytes of the memory of 'address'
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
*
* @param[out] plrMemoryTypePtr         - Pointer to the type of PLR memory
*                                      set only if not NULL
*/
static GT_U32 smemChtGetIplrMemoryStep
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    OUT        SMEM_CHT_PLR_MEMORY_TYPE_ENT    *plrMemoryTypePtr
)
{
    GT_BIT          isMeter = 0 , isCounter = 0 , isMeterConfig = 0 /*, isConfLevelSign = 0*/;
    GT_U32          unitMask = SMEM_CHT_IS_SIP5_GET(devObjPtr) ? 0x00FFFFFF : 0x007FFFFF;
    GT_U32          relativeAddr = address & unitMask;
    GT_U32          masked_meterTblBaseAddr = devObjPtr->policerSupport.meterTblBaseAddr & unitMask;
    GT_U32          masked_countTblBaseAddr = devObjPtr->policerSupport.countTblBaseAddr & unitMask;
    GT_U32          masked_meterConfigTblBaseAddr = devObjPtr->policerSupport.meterConfigTblBaseAddr & unitMask;
    GT_U32          masked_policerConformanceLevelSignTblBaseAddr = devObjPtr->policerSupport.policerConformanceLevelSignTblBaseAddr & unitMask;
    GT_U32      memoryStep;
    SMEM_CHT_PLR_MEMORY_TYPE_ENT    plrMemoryType = SMEM_CHT_PLR_MEMORY_TYPE_OTHER_E;

    if(relativeAddr >= masked_meterTblBaseAddr &&
        relativeAddr < masked_countTblBaseAddr)
    {
        isMeter = 1;
        plrMemoryType = SMEM_CHT_PLR_MEMORY_TYPE_METERING_E;
    }
    else
    if(relativeAddr >= masked_countTblBaseAddr)
    {
        if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) &&
           relativeAddr >= masked_meterConfigTblBaseAddr)
        {
            if(relativeAddr >= masked_policerConformanceLevelSignTblBaseAddr)
            {
                plrMemoryType = SMEM_CHT_PLR_MEMORY_TYPE_CONF_LEVEL_SIGN_E;
                /*isConfLevelSign = 1;*/
            }
            else
            {
                isMeterConfig = 1;
                plrMemoryType = SMEM_CHT_PLR_MEMORY_TYPE_METERING_CONFIG_E;
            }
        }
        else
        {
            plrMemoryType = SMEM_CHT_PLR_MEMORY_TYPE_COUNTING_E;
            isCounter = 1;
        }
    }

    if(isMeter)
    {
        memoryStep = devObjPtr->tablesInfo.policer.paramInfo[0].step;
    }
    else
    if(isCounter)
    {
        memoryStep = devObjPtr->tablesInfo.policerCounters.paramInfo[0].step;
    }
    else
    if(isMeterConfig)
    {
        memoryStep = devObjPtr->tablesInfo.policerConfig.paramInfo[0].step;
    }
    else /*isConfLevelSign*/
    {
        /* NOTE: this table is local to the unit and NOT shared in memory of PLR[0] */
        memoryStep = devObjPtr->tablesInfo.policerConformanceLevelSign.paramInfo[0].step;
    }

    if(plrMemoryTypePtr)
    {
        *plrMemoryTypePtr = plrMemoryType;
    }


    return memoryStep;
}

/**
* @internal smemBobKIsPlrMemoryShared function
* @endinternal
*
* @brief   Returns isShared flag for Policer memories.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
*/
static GT_U32 smemBobKIsPlrMemoryShared
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address
)
{
    DECLARE_FUNC_NAME(smemBobKIsPlrMemoryShared);
    GT_U32          isSharedMemory;   /* is memory shared between IPLR0 and IPLR1 */
    SMEM_CHT_PLR_MEMORY_TYPE_ENT  plrMemoryType;

    if(!SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
    {
        return 0;
    }
    smemChtGetIplrMemoryStep(devObjPtr,address,&plrMemoryType);

    switch(plrMemoryType)
    {
        case SMEM_CHT_PLR_MEMORY_TYPE_METERING_E:
        case SMEM_CHT_PLR_MEMORY_TYPE_COUNTING_E:
        case SMEM_CHT_PLR_MEMORY_TYPE_METERING_CONFIG_E:
            isSharedMemory = 1;
            break;
        default:
            isSharedMemory = 0;
            break;
    }
    /* log only for relevant devices */
    __LOG(("PLR memory access isSharedMemory flag is %d\n", isSharedMemory));

    return isSharedMemory;
}

/**
* @internal smemXCatActiveReadIplr0Tables function
* @endinternal
*
* @brief   Read iplr0 metering, counting tables (tablesInfo.policer, tablesInfo.policerCounters).
*         Redirected to "end of" iplr0 tables (lower part) if needed. See Policer Memory Control
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIplr0Tables
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* Active memory of IPLR-0. This code covers metering and counting
    because offsets are 'relative', even using only 'policer' table */
    GT_U32         newAddr;
    GT_U32        *regPtr;
    GT_U32          memoryStep;
    GT_U32          isSharedMemory;   /* is memory shared between IPLR0 and IPLR1 */

    isSharedMemory = smemBobKIsPlrMemoryShared(devObjPtr, address);

    newAddr = address;
    if (devObjPtr->policerSupport.iplr1EntriesFirst && (0 == isSharedMemory))
    {
        memoryStep = smemChtGetIplrMemoryStep(devObjPtr,address,NULL);

        /* policer0 entries are after policer1 entries - do shift addr by length of policer1 part */
        newAddr += devObjPtr->policerSupport.iplr1TableSize * memoryStep;

        /* pointer to simulation memory of iplr0 table (policer or policerCounters) */
        regPtr = smemMemGet(devObjPtr, newAddr);
        /* copy iplr0 table contents to the output memory */
        memcpy(outMemPtr, regPtr, memSize * sizeof(GT_U32));
    }
    else
    {
        memcpy(outMemPtr, memPtr, memSize * sizeof(GT_U32));
    }

}

/**
* @internal smemXCatActiveWriteIplr0Tables function
* @endinternal
*
* @brief   Write to iplr0 metering, counting tables (tablesInfo.policer, tablesInfo.policerCounters).
*         Redirected to "end of" iplr0 tables (lower part) if needed. See Policer Memory Control
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteIplr0Tables
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* Active memory of IPLR-0. This code covers metering and counting
    because offsets are 'relative', even using only 'policer' table */
    GT_U32         newAddr;
    GT_U32          *targetMemoryPtr;
    GT_U32          memoryStep;
    GT_U32          isSharedMemory;   /* is memory shared between IPLR0 and IPLR1 */

    isSharedMemory = smemBobKIsPlrMemoryShared(devObjPtr, address);

    newAddr = address;
    if (devObjPtr->policerSupport.iplr1EntriesFirst && (0 == isSharedMemory))
    {
        memoryStep = smemChtGetIplrMemoryStep(devObjPtr,address,NULL);

        /* policer0 entries are after policer1 entries - do shift addr by length of policer1 part */
        newAddr += devObjPtr->policerSupport.iplr1TableSize * memoryStep;

        /* pointer to simulation memory of iplr0 table (policer or policerCounters) */
        targetMemoryPtr = smemMemGet(devObjPtr, newAddr);
        /* write input memory to the iplr0 table */
    }
    else
    {
        targetMemoryPtr = memPtr;
    }

    memcpy(targetMemoryPtr, inMemPtr, memSize * sizeof(GT_U32));

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemLion3ActiveWriteMeterEntry(devObjPtr,targetMemoryPtr,newAddr,0/*plr0*/);
    }
}

/**
* @internal smemXCatActiveReadIplr1Tables function
* @endinternal
*
* @brief   Read iplr1 metering, counting tables (tablesInfo.policer, tablesInfo.policerCounters).
*         Redirected to iplr0 tables. See Policer Memory Control
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemXCatActiveReadIplr1Tables
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    /* Active memory of IPLR-1. This code covers metering and counting
    because offsets are 'relative', even using only 'policer' table */
    GT_U32         addrOffset, newAddr;
    GT_U32        *regPtr;
    GT_U32          memoryStep;

    if (devObjPtr->tablesInfo.policer.paramInfo[1].step == SMAIN_NOT_VALID_CNS)
    {
        skernelFatalError( "smemXCatActiveReadIplr1Tables: iplr1 have no memory. Check Policer Memory Control configuration \n");
    }

    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) &&
       !smemBobKIsPlrMemoryShared(devObjPtr, address))
    {
        /* Read from PLR1 */
        newAddr = address;
    }
    else
    {
        addrOffset  = address - devObjPtr->memUnitBaseAddrInfo.policer[1];
        if (!devObjPtr->policerSupport.iplr1EntriesFirst)
        {
            memoryStep = smemChtGetIplrMemoryStep(devObjPtr,address,NULL);
            /* policer1 entries are after policer0 entries - do shift addr by length of policer0 part */
            addrOffset += devObjPtr->policerSupport.iplr0TableSize * memoryStep;
        }
        newAddr = addrOffset + devObjPtr->memUnitBaseAddrInfo.policer[0];
    }

    /* pointer to simulation memory of iplr0 table (policer or policerCounters) */
    regPtr = smemMemGet(devObjPtr, newAddr);
    /* copy iplr0 table contents to the output memory */
    memcpy(outMemPtr, regPtr, memSize * sizeof(GT_U32));
}

/**
* @internal smemXCatActiveWriteIplr1Tables function
* @endinternal
*
* @brief   Write to iplr1 metering, counting tables (tablesInfo.policer, tablesInfo.policerCounters).
*         Redirected to iplr0 tables. See Policer Memory Control
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteIplr1Tables
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* Active memory of IPLR-1. This code covers metering and counting
       because offsets are 'relative', even using only 'policer' table */
    GT_U32         addrOffset, newAddr;
    GT_U32        *regPtr;
    GT_U32          memoryStep;

    if (devObjPtr->tablesInfo.policer.paramInfo[1].step == SMAIN_NOT_VALID_CNS)
    {
         skernelFatalError( "smemXCatActiveWriteIplr1Tables: iplr1 have no memory. Check Policer Memory Control configuration \n");
    }

    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) &&
       !smemBobKIsPlrMemoryShared(devObjPtr, address))
    {
        /* write to PLR1 */
        newAddr = address;
    }
    else
    {
        addrOffset  = address - devObjPtr->memUnitBaseAddrInfo.policer[1];
        if (!devObjPtr->policerSupport.iplr1EntriesFirst)
        {
            memoryStep = smemChtGetIplrMemoryStep(devObjPtr,address,NULL);
            /* policer1 entries are after policer0 entries - do shift addr by length of policer0 part */
            addrOffset += devObjPtr->policerSupport.iplr0TableSize * memoryStep;
        }
        newAddr = addrOffset + devObjPtr->memUnitBaseAddrInfo.policer[0];
    }

    /* pointer to simulation memory of iplr0 table (policer or policerCounters) */
    regPtr = smemMemGet(devObjPtr, newAddr);
    /* write input memory to the iplr0 table */
    memcpy(regPtr, inMemPtr, memSize * sizeof(GT_U32));


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemLion3ActiveWriteMeterEntry(devObjPtr,regPtr,newAddr,1/*plr1*/);
    }

}

/**
* @internal smemXCatActiveWritePolicerMemoryControl function
* @endinternal
*
* @brief   Set the Policer Control0 register in iplr0, and update Policer Memory Control
*         configuration
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWritePolicerMemoryControl
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 fldValue;
    GT_U32 meterBase;        /* metering table start addr in iplr0 (tablesInfo.policer) */
    GT_U32 countBase;        /* counters table start addr in iplr0 (tablesInfo.policerCounters) */
    GT_U32 meterEntrySize, countEntrySize;        /* table entry size 0x20, equal for both tables */

    /* set register content */
    *memPtr = *inMemPtr ;

    meterBase = devObjPtr->policerSupport.meterTblBaseAddr;
    countBase = devObjPtr->policerSupport.countTblBaseAddr;
    meterEntrySize = devObjPtr->tablesInfo.policer.paramInfo[0].step;        /* size of entry */
    countEntrySize = devObjPtr->tablesInfo.policerCounters.paramInfo[0].step;        /* size of entry */

    /* get "Policer Memory Control" field - bits 12:11 */
    fldValue = SMEM_U32_GET_FIELD(*inMemPtr, 11, 2);
    switch (fldValue)
    {
        case 0: /* PLR0_UP_PLR1_LW:Policer 0 uses the upper memory (1792) Policer 1 uses the lower memory (256) */
            devObjPtr->tablesInfo.policer.commonInfo.baseAddress = meterBase;
            devObjPtr->tablesInfo.policer.paramInfo[1].step = POLICER_MEMORY_1792_CNS * meterEntrySize;
            devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = countBase;
            devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = POLICER_MEMORY_1792_CNS * countEntrySize;
            devObjPtr->policerSupport.iplr0TableSize = POLICER_MEMORY_1792_CNS;
            devObjPtr->policerSupport.iplr1TableSize = POLICER_MEMORY_256_CNS;
            devObjPtr->policerSupport.iplr1EntriesFirst = 0;
            break;
        case 1: /* PLR0_UP_AND_LW:Policer 0 uses both memories */
            devObjPtr->tablesInfo.policer.commonInfo.baseAddress = meterBase;
            devObjPtr->tablesInfo.policer.paramInfo[1].step = SMAIN_NOT_VALID_CNS;
            devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = countBase;
            devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = SMAIN_NOT_VALID_CNS;
            devObjPtr->policerSupport.iplr0TableSize = 2048;
            devObjPtr->policerSupport.iplr1TableSize = 0;
            devObjPtr->policerSupport.iplr1EntriesFirst = 0;
            break;
        case 2: /* PLR1_UP_AND_LW:Policer 1 uses both memories */
            devObjPtr->tablesInfo.policer.commonInfo.baseAddress = SMAIN_NOT_VALID_CNS;
            devObjPtr->tablesInfo.policer.paramInfo[1].step = meterBase - SMAIN_NOT_VALID_CNS;
            devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = SMAIN_NOT_VALID_CNS;
            devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = countBase - SMAIN_NOT_VALID_CNS;
            /* Note: (commonInfo.baseAddress + paramInfo[1].step) now points to start of table */
            devObjPtr->policerSupport.iplr0TableSize = 0;
            devObjPtr->policerSupport.iplr1TableSize = 2048;
            devObjPtr->policerSupport.iplr1EntriesFirst = 0;
            break;
        case 3: /* PLR1_UP_PLR0_LW:Policer 1 uses the upper memory (1792) Policer 0 uses the lower memory (256) */
            devObjPtr->tablesInfo.policer.commonInfo.baseAddress = meterBase + POLICER_MEMORY_1792_CNS * meterEntrySize;
            devObjPtr->tablesInfo.policer.paramInfo[1].step = 0 - (POLICER_MEMORY_1792_CNS * meterEntrySize);
            devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = countBase + POLICER_MEMORY_1792_CNS * countEntrySize;
            devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = 0 - (POLICER_MEMORY_1792_CNS * countEntrySize);
            devObjPtr->policerSupport.iplr0TableSize = POLICER_MEMORY_256_CNS;
            devObjPtr->policerSupport.iplr1TableSize = POLICER_MEMORY_1792_CNS;
            devObjPtr->policerSupport.iplr1EntriesFirst = 1;
            break;
        default:
            break;
    }
}
#define __LOG_PARAM_u(param)                                    \
    __LOG(("[%s] = [%u] \n",                                    \
        #param ,/* name of the parameter (field name)   */      \
        (param)))/*value of the parameter (field value) */
/**
* @internal smemXCatActiveWriteIPFixTimeStamp function
* @endinternal
*
* @brief   The Time Of Day register write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteIPFixTimeStamp
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    DECLARE_FUNC_NAME(smemXCatActiveWriteIPFixTimeStamp);
    GT_U32 * regPtr;            /* memory data pointer */
    GT_U32 regAddr;             /* memory address */
    GT_U32 cycle;               /* policer cycle */
    GT_U32 cycleStart,cycleEnd; /* start/end policer cycle */
    GT_U32 increment;           /* Time stamp upload mode */
    GT_U32 timerData[3];        /* Upload timer data */
    GT_U32 uploadTrigger;       /* Upload timer trigger */
    GT_U32 nanoSeconds = 0;     /* IPfix nano timer value */
    GT_U32 seconds = 0;         /* IPfix second timer */
    GT_U32 * ctrlRegDataPtr;    /* pointer IPFIX Control data */
    GT_U64 seconds64;           /* 64 bits seconds value */
    SNET_TOD_TIMER_STC ipFixTimer; /* IPFix timer */
    GT_U32 clocks;              /* TOD value in clocks */
    GT_U32 newTimeInMilli;      /* new time in milliseconds */
    GT_U32 currTicks;           /* current time in ticks (milliseconds)*/

    *memPtr = *inMemPtr;

    seconds64.l[0] = seconds64.l[1] = 0;

    uploadTrigger = SMEM_U32_GET_FIELD(*inMemPtr, 6, 1);
    /* Update the IPfix time stamp according to the time stamp upload mode */
    if(uploadTrigger)
    {
        /* Get current TOD clock value and convert seconds to 64 bits value */
        clocks = SNET_XCAT_IPFIX_TOD_CLOCK_GET_MAC(devObjPtr, 0);
        SNET_TOD_CLOCK_FORMAT_MAC(clocks, seconds, nanoSeconds);

        currTicks = SIM_OS_MAC(simOsTickGet)();

        seconds64.l[0] = seconds;
        seconds64.l[1] = 0;

        cycle = param;
        if(cycle == 0)
        {
            /* NOTE:
               This bit in IPLR0 controls the IPLR1 and EPLR as well.
               The upload mode and upload values are taken in each PLR from its local RegFile */
            cycleStart = 0;
            cycleEnd = 3;
        }
        else
        {
            /* impact only local unit */
            cycleStart = cycle;
            cycleEnd   = cycle + 1;
        }


        for(cycle = cycleStart; cycle < cycleEnd; cycle++)
        {
            /* Policer IPFIX Control */
            regAddr =
                SMEM_XCAT_POLICER_IPFIX_CTRL_REG(devObjPtr, cycle);

            /* IPLR0/IPLR1/EPLR control */
            ctrlRegDataPtr = smemMemGet(devObjPtr, regAddr);

            /* Time stamp upload mode: 0 - absolute, 1 - incremental */
            increment = SMEM_U32_GET_FIELD(*ctrlRegDataPtr, 7, 1);

            /* IPFIX nano timer stamp upload */
            regAddr =
                SMEM_XCAT_POLICER_IPFIX_NANO_TS_UPLOAD_REG(devObjPtr,
                                                                    cycle);
            regPtr = smemMemGet(devObjPtr, regAddr);

            /* Copy IPFix timer data: nano timer, second LSb timer, second MSb timer */
            memcpy(timerData, regPtr, sizeof(timerData));

            /* Policer Timer Memory */
            regAddr = SMEM_XCAT_POLICER_TIMER_TBL_MEM(devObjPtr,  cycle, 6);
            regPtr = smemMemGet(devObjPtr, regAddr);

            /* Nano timer */
            ipFixTimer.nanoSecondTimer = timerData[0];

            /* Convert second timer data to 64 bit value */
            ipFixTimer.secondTimer.l[0] = timerData[1];
            ipFixTimer.secondTimer.l[1] = timerData[2];

            if(increment)
            {
                /* value of LS word before the update */
                GT_U32  oldLsValue = ipFixTimer.secondTimer.l[0];
                /* value of MS word before the update */
                GT_U32  oldMsValue = ipFixTimer.secondTimer.l[1];

                __LOG(("do increment PLR time \n"));

                ipFixTimer.nanoSecondTimer += nanoSeconds;

                if(ipFixTimer.nanoSecondTimer >= NANO_SEC_MAX_VALUE_CNS)
                {
                    GT_U64  oneSecond;

                    oneSecond.l[0] = 1;
                    oneSecond.l[1] = 0;

                    /* the nano seconds wrap ...
                        1. reduce the 10^9 nanos from the value.
                        2. give carry to the seconds
                    */
                    ipFixTimer.nanoSecondTimer -= NANO_SEC_MAX_VALUE_CNS;

                    ipFixTimer.secondTimer =
                        prvSimMathAdd64(ipFixTimer.secondTimer, oneSecond);
                }

                ipFixTimer.secondTimer =
                    prvSimMathAdd64(ipFixTimer.secondTimer, seconds64);

                if(devObjPtr->errata.ipfixTodIncrementModeWraparoundIssues)
                {
                    /* check for LS word wraparound */
                    if(ipFixTimer.secondTimer.l[0] < oldLsValue)
                    {
                        /* the LS word did wrap around */
                        /* but due to the erratum the carry should not go
                           to the MS word  !!! */
                        ipFixTimer.secondTimer.l[1] --;

                        /* the LS word get the carry instead of MS word !!! */
                        ipFixTimer.secondTimer.l[0] ++;
                    }
                    else
                    /* check for MS word wraparound (when LS word did not wraparound) */
                    if(ipFixTimer.secondTimer.l[1] < oldMsValue)
                    {
                        /* the MS word did wrap around */
                        /* but due to the erratum the wrap around should be
                           added 1 !!! */
                        ipFixTimer.secondTimer.l[1] ++;
                    }
                }
            }
            else
            {
                __LOG(("do absolute PLR time setting \n"));
            }

            __LOG_PARAM(ipFixTimer.nanoSecondTimer);
            __LOG_PARAM(ipFixTimer.secondTimer.l[0]);
            __LOG_PARAM(ipFixTimer.secondTimer.l[1]);

            /* IPfix timer is uploaded to the absolute/incremented value of time stamp upload registers */
            regPtr[0] = ipFixTimer.nanoSecondTimer;
            regPtr[1] = ipFixTimer.secondTimer.l[0];
            regPtr[2] = ipFixTimer.secondTimer.l[1];

/*          next calc gives 'bizar' values !
            the 'ipFixTimer.secondTimer.l[0] * 1000' gives value that not ended with 000 decimal.

            newTimeInMilli = ipFixTimer.nanoSecondTimer / 1000000 +
                             ipFixTimer.secondTimer.l[0] * 1000;

            so we need to 'allow' the *1000 to not overflow !
            so use only 22 bits of ipFixTimer.secondTimer.l[0] (that leaves 1024 multiple on it)

            newTimeInMilli = ipFixTimer.nanoSecondTimer / 1000000 +
                             (ipFixTimer.secondTimer.l[0] & 0x003FFFFF) * 1000;
*/
            newTimeInMilli = ipFixTimer.nanoSecondTimer / 1000000 +
                             (ipFixTimer.secondTimer.l[0] & 0x003FFFFF) * 1000;
            /* Cleared by the device when the write action is completed */
            SMEM_U32_SET_FIELD(*ctrlRegDataPtr, 6, 1, 0);

            __LOG_PARAM_u(currTicks);
            __LOG_PARAM_u(newTimeInMilli);

            /* Set IPFix timestamp TOD offset */
            /* do the 'currTicks - newTimeInMilli' because the MACRO SNET_XCAT_IPFIX_TOD_CLOCK_GET_MAC
               is doing :  SIM_OS_MAC(simOsTickGet)() - (dev)->ipFixTimeStampClockDiff[(cycle)]

               that will become:
               SIM_OS_MAC(simOsTickGet)() - (currTicks - newTimeInMilli)

               that will become:
               'new current time' - ('old current time' - newTimeInMilli) =

               newTimeInMilli + ('new current time' - 'old current time') =

               newTimeInMilli + ('diff between 'now' and time that the application
                set the clock in the register)  --> time that consider the clock in the registers
            */
            devObjPtr->ipFixTimeStampClockDiff[cycle] = currTicks - newTimeInMilli;
            __LOG_PARAM_u(devObjPtr->ipFixTimeStampClockDiff[cycle]);

            /* save the current time (needed by smemXCatPolicerIPFixTimeStampUpload()) */
            devObjPtr->clockWhenPolicerTimerTblWasSet[cycle] = currTicks;
            __LOG_PARAM_u(devObjPtr->clockWhenPolicerTimerTblWasSet[cycle]);
        }
    }
}

/**
* @internal smemXCatActiveWriteLogTargetMap function
* @endinternal
*
* @brief   The Logical Target Mapping Table write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteLogTargetMap
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     lineNumber;      /* line number in Logical Target Mapping Table */
    GT_U32     rdWr;            /* read/write operation */
    GT_U32     regAddr;         /* register address */
    GT_U32    *regPtr;          /* register pointer */
    GT_U32    *logTrgMapPtr;    /* pointer to logical target mapping table entry */

    /* Action 0 - read , 1 - write */
    rdWr = SMEM_U32_GET_FIELD(*inMemPtr, 1, 1);
    /* Line Number */
    if(devObjPtr->supportLogicalMapTableInfo.supportFullRange)
    {
        /* the device supports all 32 (0..31 not only 24..31) devices for this table */
        lineNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 11);/*6 bits port , 5 bits device */
    }
    else
    {
        /* the device supports only 8 (24..31) devices for the table */
        lineNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 9);
    }

    /* Logical Target Mapping Table */
    regAddr = SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_MEM(devObjPtr,
                                                   (lineNumber >> 6),
                                                   (lineNumber & 0x3f));
    logTrgMapPtr = smemMemGet(devObjPtr, regAddr);

    /* Logical Target Mapping Table Data Access */
    regAddr = SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_DATA_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    if (rdWr == 0)
    {
        /* Read Logical Target Mapping entry */
        *regPtr = *logTrgMapPtr;
    }
    else
    {
        /* Write Logical Target Mapping entry */
        *logTrgMapPtr = *regPtr;
    }

    if(devObjPtr->tablesInfo.logicalTrgMappingTable.paramInfo[1].step >= 8)
    {
        /* there is second register for the indirect to use */
        /* Logical Target Mapping Table Data Access2 */
        regAddr = SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_DATA2_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);

        if (rdWr == 0)
        {
            /* Read Logical Target Mapping entry */
            *regPtr = logTrgMapPtr[1];
        }
        else
        {
            /* Write Logical Target Mapping entry */
            logTrgMapPtr[1] = *regPtr;
        }
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemXCatActiveWritePolicyTcamConfig_0 function
* @endinternal
*
* @brief   set the register of PCL TCAM configuration
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWritePolicyTcamConfig_0
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     tcamSize;  /* */
    GT_U32     reduceNumOfEntries;/* number of entries to reduce from the max support */

    /* set register content */
    *memPtr = *inMemPtr ;

    tcamSize = SMEM_U32_GET_FIELD(*inMemPtr, 1, 4);

    switch(tcamSize)
    {
        case 0:/*Full Tcam Used*/
            reduceNumOfEntries = 0;
            break;
        case 1:/*Full Tcam Minus 1K entries*/
            reduceNumOfEntries = 1*1024;
            break;
        case 2:/*Full Tcam Minus 2K entries*/
            reduceNumOfEntries = 2*1024;
            break;
        default:/* not described in the documentation */
            /*simulation will treat it as Full Tcam Minus 3K entries*/
            reduceNumOfEntries = 3*1024;
            break;
    }

    if(reduceNumOfEntries > devObjPtr->pclTcamMaxNumEntries)
    {
        reduceNumOfEntries = devObjPtr->pclTcamMaxNumEntries;
    }

    devObjPtr->pclTcamInfoPtr->bankEntriesNum =
            (devObjPtr->pclTcamMaxNumEntries - reduceNumOfEntries) / 4;


    return;

}

/**
* @internal smemXCatActiveWriteTtiInternalMetalFix function
* @endinternal
*
* @brief   Set the register of TTI Internal, Metal Fix
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteTtiInternalMetalFix
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 fldValue;

    /* set register content */
    *memPtr = *inMemPtr ;

    if (devObjPtr->errata.supportSrcTrunkToCpuIndicationEnable)
    {
        /* Disable Routed Bug Fix */
        fldValue = SMEM_U32_GET_FIELD(*inMemPtr, 17, 1);
        /* Enable/Disable simulation of erratum:
        "Wrong Trunk-ID/Source Port Information of Packet to CPU" */
        devObjPtr->errata.srcTrunkToCpuIndication = (fldValue) ? 0 : 1;
        /*If enable, the port isolation take its members from the source port and not from the source trunk */
        devObjPtr->errata.srcTrunkPortIsolationAsSrcPort =
            !devObjPtr->errata.srcTrunkToCpuIndication;
    }
}

/**
* @internal smemCht3ReadWriteRouterTTTcam function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory.
*
* @param[in] devObjPtr                -
* @param[in] writeAction              - GT_TRUE - write action , GT_FALSE - read action
* @param[in] rtLineNumber             -
* @param[in] rtDataType               -
* @param[in] rtBankEnBmp              -  bmp of valid banks
* @param[in] rtValidBit               -
* @param[in] rtCompMode               -
*/
static void smemCht3ReadWriteRouterTTTcam
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_BOOL  writeAction,
    IN         GT_U32  rtLineNumber,
    IN         GT_U32  rtDataType,
    IN         GT_U32  rtBankEnBmp,
    IN         GT_U32  rtValidBit ,
    IN         GT_U32  rtCompMode,
    IN         GT_U32  rtSpare
)
{
    GT_U32  *regPtr;         /* register's entry pointer */
    GT_U32  tcamRegAddr0;    /* register's address group 0-2 */
    GT_U32  ctrlRegAddr0;
    GT_U32  word2;
    GT_U32  word1 = 0;
    GT_U32  validBit = 0 ;
    GT_U32  compModeBit = 0 ;
    GT_U32  ii;
    GT_U32  spareBit;

    if(writeAction == GT_TRUE)
    {
        if (rtDataType == 0)
        {
            /* Write is done to the data part of the Router TCAM and CTRL entries */
            tcamRegAddr0 = SMEM_CHT3_TCAM_X_DATA_TBL_MEM(devObjPtr,rtLineNumber);
            ctrlRegAddr0 = SMEM_CHT3_TCAM_ROUTER_TT_X_CONTROL_TBL_MEM(devObjPtr,rtLineNumber);
        }
        else
        {
            tcamRegAddr0 = SMEM_CHT3_TCAM_Y_DATA_TBL_MEM(devObjPtr,rtLineNumber);
            ctrlRegAddr0 = SMEM_CHT3_TCAM_ROUTER_TT_Y_CONTROL_TBL_MEM(devObjPtr,rtLineNumber);
        }

/* ROUTER TCAM DATA */
#define SMEM_CHT3_ROUTER_TCAM_ACCESS_DATA_TBL_MEM(dev,entry_indx) \
        (0x0D800400 + ( 0x04 * (entry_indx)))

        /* Read TCAM data register word 0 - long key */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT3_ROUTER_TCAM_ACCESS_DATA_TBL_MEM(devObjPtr,0));

        for (ii = 0; (ii < 4) ; ii++)
        {
           switch (ii)
           {
                case 0x0: /* Write tcam word1 from data 0 and word2 from data2 16 LSB*/
                    word1 =  regPtr[ii];
                    word2 = (regPtr[ii + 1] ) & 0xFFFF;
                break;

           case 0x1: /* Write tcam word1 from data 2 and 3 and word2 from data4 16 MSB*/
                    word1 = 0;
                    word1 =   (((regPtr[ii ] & 0xffff0000) >> 16)
                            |    (((regPtr[ii + 1]) & 0xffff)) << 16);


                    word2 = regPtr[ii + 1 ] >> 16 & 0x0000FFFF;
                break;
                case 0x2: /* Write tcam word1 from data 0 32 bit*/
                    word1 =  regPtr[ii + 1];
                    word2 = (regPtr[ii + 2]) & 0xFFFF;
                break;
           case 0x3: /* Write tcam word3 from data 4 ,5*/
                    word1 = 0;
                    word1 =   (((regPtr[ii + 1] & 0xffff0000) >> 16)
                            |    (((regPtr[ii + 2]) & 0xffff)) << 16);

                    word2 = regPtr[ii + 2] >> 16 & 0x0000FFFF;
                break;

           }
           if ((1 << ii) & rtBankEnBmp)
           {

                smemRegSet(devObjPtr, (tcamRegAddr0 + (devObjPtr->routeTcamInfo.bankWidth * ii)) , word1);
                /*compareBit  = ((rtCompare & (1 << ii)) ? 1 : 0) << 17; */
                validBit    = ((rtValidBit & (1 << ii)) ? 1 : 0);
                compModeBit = (rtCompMode >> (2 * ii)) & 0x3;
                spareBit    = ((rtSpare & (1 << ii)) ? 1 : 0);
                word2 |=  spareBit  << 16 |validBit  << 17 | (compModeBit << 18);

                smemRegSet(devObjPtr , (ctrlRegAddr0 + (devObjPtr->routeTcamInfo.bankWidth * ii)) , word2);
            }
        }
    }
    else
    {
        /* not implemented -- see CQ# 105927 */
    }
}

/**
* @internal smemCht3ReadWritePolicyTcam function
* @endinternal
*
* @brief   Handler for read/write router TT TCAM memory.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] lineNumber               - table line number to be read or write
* @param[in] tcamDataType             - Mask/Data part of entry.
* @param[in] rdWr                     - read/write action. (1 = for write)
* @param[in] tcamCompMode             - compare mode
*/
static void smemCht3ReadWritePolicyTcam
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32  lineNumber,
    IN GT_U32  tcamDataType,
    IN GT_U32  rdWr,
    IN GT_U32  tcamCompMode
)
{
    if (rdWr)
    {
        smemCht3WritePolicyTcamData(devObjPtr, lineNumber, tcamDataType,
                                    tcamCompMode);
    }
    else
    {
        smemCht3ReadPolicyTcamData(devObjPtr, lineNumber, tcamDataType);
    }
}

/**
* @internal smemCht3WritePolicyTcamData function
* @endinternal
*
* @brief   Handler for write policy TCAM data memory.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table line number to be read or write.
*                                      tcamDataType        - TCAM data(=0) or mask(=1) part of entry
* @param[in] tcamCompMode             - compare mode
*/
static void smemCht3WritePolicyTcamData
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line ,
    IN         GT_U32  type ,
    IN         GT_U32  tcamCompMode
)
{
    GT_U32                  regAddr,*regPtr;
    GT_U32                  data[12];/* 12 words from the user*/
    GT_U32                  ii;
    GT_U32                  regParam1,regParam2;/*registers of extra parameters*/
    GT_U32                  mode=1;/* always - long key */


    smemRegGet(devObjPtr,
        SMEM_CHT3_PCL_ACTION_TCAM_DATA_PARAM1_REG(devObjPtr),
        &regParam1);
    smemRegGet(devObjPtr,
        SMEM_CHT3_PCL_ACTION_TCAM_DATA_PARAM2_REG(devObjPtr),
        &regParam2);
    /*
        The data that is written to the policy TCAM or the action table,
            or the data read from the action table, as follows:
        The following registers are used for a write access of an extended rule
            (48B) to the Policy TCAM as follows:

        Data0 [31:0] <= Array0Word0[31:0] Entry<LineNumber>
        Data1 [15:0] <= Array0Word0[47:32]Entry<LineNumber>

        Data1[31:16] <= Array0Word1[15:0] Entry<LineNumber>
        Data2[31:0] <= Array0Word1[47:16]Entry<LineNumber>

        Data3 [31:0] <= Array0Word2[31:0] Entry<LineNumber>
        Data4 [15:0] <= Array0Word2[47:32]Entry<LineNumber>

        Data4[31:16] <= Array0Word3[15:0] Entry<LineNumber>
        Data5[31:0] <= Array0Word3[47:16]Entry<LineNumber>

        Data6 [31:0] <= Array1Word0[31:0] Entry<LineNumber>
        Data7 [15:0] <= Array1Word0[47:32]Entry<LineNumber>

        Data7[31:16] <= Array1Word1[15:0] Entry<LineNumber>
        Data8[31:0] <= Array1Word1[47:16]Entry<LineNumber>

        Data9 [31:0] <= Array1Word2[31:0] Entry<LineNumber>
        Data10 [15:0] <= Array1Word2[47:32]Entry<LineNumber>

        Data10[31:16] <= Array1Word3[15:0] Entry<LineNumber>
        Data11[31:0] <= Array1Word3[47:16]Entry<LineNumber>
    */

    /* Read TCAM data register word 0 - long key, 6 - short */
    regPtr = smemMemGet(devObjPtr, SMEM_CHT3_PCL_ACTION_TCAM_DATA_TBL_MEM(devObjPtr,
        ((mode == 0) ? 6 : 0)));

    for(ii=0;ii<12;ii++)
    {
        data[ii] = regPtr[ii];
    }

    /* TCAM data or mask / x or y */
    if(type == 1)   /*data*/
        regAddr = SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(devObjPtr,line, 0);
    else            /*mask*/
        regAddr = SMEM_CHT3_PCL_TCAM_MASK_TBL_MEM(devObjPtr,line, 0);

    regPtr = smemMemGet(devObjPtr, regAddr);

    for(ii = 0 ; ii < 2; ii++)
    {
        if(0 == SMEM_U32_GET_FIELD(regParam1,ii*4,4))
        {
            /* check fields of Enable array number */
            continue;
        }

        regPtr[(ii*4)+0]   = data[(ii*3)+0] ;

        regPtr[(ii*4)+1]   = data[(ii*3)+1] & 0xFFFF;
        /* set control bits */
        regPtr[(ii*4)+1]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+0)+ 8,1))<<16; /*spare*/
        regPtr[(ii*4)+1]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+0)+ 0,1))<<17; /*valid*/
        regPtr[(ii*4)+1]  |= (SMEM_U32_GET_FIELD(regParam1,(ii*4+0)+16,2))<<18; /*compare mode*/

        regPtr[(ii*4)+2]   = data[(ii*3)+1] >> 16;
        regPtr[(ii*4)+2]  |= data[(ii*3)+2] << 16;

        regPtr[(ii*4)+3]   = data[(ii*3)+2] >> 16;

        /* set control bits */
        regPtr[(ii*4)+3]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+1)+ 8,1))<<16; /*spare*/
        regPtr[(ii*4)+3]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+1)+ 0,1))<<17; /*valid*/
        regPtr[(ii*4)+3]  |= (SMEM_U32_GET_FIELD(regParam1,(ii*4+1)+16,2))<<18; /*compare mode*/
    }

    /* A long key */
    if (mode)
    {
        /* TCAM data or mask / x or y */
        if(type == 1)   /*data*/
            regAddr = SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(devObjPtr,line+512, 0);
        else            /*mask*/
            regAddr = SMEM_CHT3_PCL_TCAM_MASK_TBL_MEM(devObjPtr,line+512, 0);

        regPtr = smemMemGet(devObjPtr, regAddr);

        for(ii = 0 ; ii < 2; ii++)
        {
            if(0 == SMEM_U32_GET_FIELD(regParam1,(ii+2)*4,4))
            {
                /* check fields of Enable array number */
                continue;
            }

            regPtr[(ii*4)+0+6]   = data[(ii*3)+0] ;

            regPtr[(ii*4)+1+6]   = data[(ii*3)+1] & 0xFFFF;
            /* set control bits */
            regPtr[(ii*4)+1+6]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+2)+ 8,1))<<16; /*spare*/
            regPtr[(ii*4)+1+6]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+2)+ 0,1))<<17; /*valid*/
            regPtr[(ii*4)+1+6]  |= (SMEM_U32_GET_FIELD(regParam1,(ii*4+2)+16,2))<<18; /*compare mode*/

            regPtr[(ii*4)+2+6]   = data[(ii*3)+1] >> 16;
            regPtr[(ii*4)+2+6]  |= data[(ii*3)+2] << 16;

            regPtr[(ii*4)+3+6]   = data[(ii*3)+2] >> 16;
            /* set control bits */
            regPtr[(ii*4)+3+6]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+3)+ 8,1))<<16; /*spare*/
            regPtr[(ii*4)+3+6]  |= (SMEM_U32_GET_FIELD(regParam2,(ii*4+3)+ 0,1))<<17; /*valid*/
            regPtr[(ii*4)+3+6]  |= (SMEM_U32_GET_FIELD(regParam1,(ii*4+3)+16,2))<<18; /*compare mode*/
        }
    }
}


/**
* @internal smemCht3ReadPolicyTcamData function
* @endinternal
*
* @brief   Handler for read/write policy TCAM memory .
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] line                     - table  number to be read or write
* @param[in] type                     - TCAM data or mask
*/
static void smemCht3ReadPolicyTcamData
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  type
)
{
    GT_U32                  regAddr,*regPtr;
    GT_U32                  data[12];
    GT_U32                  ii;
    GT_U32                  mode=1;/* always - long key */

    /*
        The data that is written to the policy TCAM or the action table,
            or the data read from the action table, as follows:
        The following registers are used for a write access of an extended rule
            (48B) to the Policy TCAM as follows:

        Data0 [31:0] <= Array0Word0[31:0] Entry<LineNumber>
        Data1 [15:0] <= Array0Word0[47:32]Entry<LineNumber>

        Data1[31:16] <= Array0Word1[15:0] Entry<LineNumber>
        Data2[31:0] <= Array0Word1[47:16]Entry<LineNumber>

        Data3 [31:0] <= Array0Word2[31:0] Entry<LineNumber>
        Data4 [15:0] <= Array0Word2[47:32]Entry<LineNumber>

        Data4[31:16] <= Array0Word3[15:0] Entry<LineNumber>
        Data5[31:0] <= Array0Word3[47:16]Entry<LineNumber>

        Data6 [31:0] <= Array1Word0[31:0] Entry<LineNumber>
        Data7 [15:0] <= Array1Word0[47:32]Entry<LineNumber>

        Data7[31:16] <= Array1Word1[15:0] Entry<LineNumber>
        Data8[31:0] <= Array1Word1[47:16]Entry<LineNumber>

        Data9 [31:0] <= Array1Word2[31:0] Entry<LineNumber>
        Data10 [15:0] <= Array1Word2[47:32]Entry<LineNumber>

        Data10[31:16] <= Array1Word3[15:0] Entry<LineNumber>
        Data11[31:0] <= Array1Word3[47:16]Entry<LineNumber>
    */

    /* TCAM data or mask / x or y */
    if(type == 1)   /*data*/
        regAddr = SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(devObjPtr,line, 0);
    else            /*mask*/
        regAddr = SMEM_CHT3_PCL_TCAM_MASK_TBL_MEM(devObjPtr,line, 0);

    regPtr = smemMemGet(devObjPtr, regAddr);

    for(ii = 0 ; ii < 2; ii++)
    {
        data[(ii*3)+0]     = regPtr[(ii*4)+0] ;

        data[(ii*3)+1]     = regPtr[(ii*4)+1] & 0xFFFF;
        data[(ii*3)+1]    |= regPtr[(ii*4)+2] << 16;

        data[(ii*3)+2]     = regPtr[(ii*4)+2] >> 16;
        data[(ii*3)+2]    |= regPtr[(ii*4)+3] << 16;
    }

    /* A long key */
    if (mode)
    {
        /* TCAM data or mask / x or y */
        if(type == 1)   /*data*/
            regAddr = SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(devObjPtr,line+512, 0);
        else            /*mask*/
            regAddr = SMEM_CHT3_PCL_TCAM_MASK_TBL_MEM(devObjPtr,line+512, 0);

        regPtr = smemMemGet(devObjPtr, regAddr);

        for(ii = 0 ; ii < 2; ii++)
        {
            data[(ii*3)+0+6]     = regPtr[(ii*4)+0] ;

            data[(ii*3)+1+6]     = regPtr[(ii*4)+1] & 0xFFFF;
            data[(ii*3)+1+6]    |= regPtr[(ii*4)+2] << 16;

            data[(ii*3)+2+6]     = regPtr[(ii*4)+2] >> 16;
            data[(ii*3)+2+6]    |= regPtr[(ii*4)+3] << 16;
        }

        /* Write TCAM data register word 0 */
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA0_REG(devObjPtr), data, 12);
    }
    else
    {
        /* Write TCAM data register word 6*/
        smemMemSet(devObjPtr, SMEM_CHT_PCL_ACTION_TCAM_DATA6_REG(devObjPtr), data, 6);
    }
}


/**
* @internal smemCht3ReadWritePclAction function
* @endinternal
*
* @brief   Handler write for Cheetah3 PCL indirect action table.
*
* @param[in] devObjPtr                - device object PTR.
*                                      address     - Address for ASIC memory.
*                                      memPtr      - Pointer to the register's memory in the simulation.
*                                      param       - Registers' specific parameter.
*                                      inMemPtr    - Pointer to the memory to get register's content.
*/
static void smemCht3ReadWritePclAction (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32  line,
    IN         GT_U32  rdWr
)
{
    GT_U32     *regPtr;           /* register's entry pointer   */
    GT_U32      regAddr;          /* register's address         */
    GT_U32      accParam1Data;    /* ACCESS param1 reg data     */
    GT_U32      dataRegAddr;      /* address of data  register  */
    GT_U32      i;                /* loop index                 */
    GT_U32      bankSize;         /* loop index                 */

    /* amount of actions in each of 4 banks (action is 4 registers) */
    bankSize = PCL_ACTION_TBL_REGS_NUM(devObjPtr) / (4 /*banks*/ * 4 /*regs-of-action*/);

    /* indirect write parameters1 data */
    regPtr = smemMemGet(devObjPtr, SMEM_CHT3_PCL_ACTION_TCAM_DATA_PARAM1_REG(devObjPtr));
    accParam1Data = *regPtr;

    for (i = 0; (i < 4); i++)
    {
        if ((accParam1Data & (0x0F << (i * 4))) == 0)
        {
            /* all 4 words copying to the current bank masked */
            continue;
        }

        /* copy all 4 words to/from action table line     */
        /* partial line copy not supported bu simulation */

        /* PCL action table address */
        regAddr = SMEM_CHT_PCL_ACTION_TBL_MEM(devObjPtr, (line + (bankSize * i)));
        dataRegAddr =  SMEM_CHT3_PCL_ACTION_TCAM_DATA_TBL_MEM(devObjPtr,0) + 0x18 /* word6 */;

        if (rdWr)
        {
            /* Write Words 6-9 to Action Table and Policy TCAM Access Data */
            regPtr = smemMemGet(devObjPtr, dataRegAddr);
            smemMemSet(devObjPtr, regAddr, regPtr, 4);
        }
        else
        {
            /* Read words 6-9 from Action Table and Policy TCAM Access Data */
            regPtr = smemMemGet(devObjPtr, regAddr);
            smemMemSet(devObjPtr, dataRegAddr, regPtr, 4);
        }
    }
}

#define IS_IN_CNC_CHECK(address,cncUnit,_cncUnitSize)                                              \
    if(devObjPtr->memUnitBaseAddrInfo.CNC[cncUnit] &&/*is valid*/             \
       address >= devObjPtr->memUnitBaseAddrInfo.CNC[cncUnit] &&              \
       address <  (devObjPtr->memUnitBaseAddrInfo.CNC[cncUnit] + _cncUnitSize))\
    {                                                                         \
        return cncUnit;                                                       \
    }                                                                         \
/**
* @internal smemCht3CncUnitIndexFromAddrGet function
* @endinternal
*
* @brief   get the CNC unit index from the address. (if address is not in CNC unit --> fatal error)
*         NOTE: sip5 support 2 CNC units (legacy devices supports only 1)
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address in one of the CNC units.
*/
static GT_U32  smemCht3CncUnitIndexFromAddrGet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address
)
{
    GT_U32  pipeOffset,pipeIndex = 0;
    GT_U32  cncUnitSize;/* the size of the CNC unit */

    if(devObjPtr->cncNumOfUnits <= 1)
    {
        /* single CNC unit .. so must be CNC[0] */
        return 0;
    }

    cncUnitSize =  devObjPtr->cncUnitSize ? devObjPtr->cncUnitSize :
            SMEM_CHT_IS_SIP6_GET(devObjPtr)  ? (256 * _1K)/* support device without devObjPtr->memUnitBaseAddrInfo.CNC[1] */ :
                0x01000000;

    pipeOffset = SMEM_UNIT_PIPE_OFFSET_GET_MAC(devObjPtr, address , &pipeIndex);
    if(pipeIndex)
    {
        /* ignore pipe indication ! */
        address -= pipeOffset;
    }

    IS_IN_CNC_CHECK(address,0,cncUnitSize);
    IS_IN_CNC_CHECK(address,1,cncUnitSize);
    IS_IN_CNC_CHECK(address,2,cncUnitSize);
    IS_IN_CNC_CHECK(address,3,cncUnitSize);

    /* should not happen */
    skernelFatalError("smemCht3CncUnitIndexFromAddrGet: address [0x%8.8x] not belong to CNC0[0x%8.8x] and not to CNC1[[0x%8.8x]] \n",
        address,
        devObjPtr->memUnitBaseAddrInfo.CNC[0],
        devObjPtr->memUnitBaseAddrInfo.CNC[1]);

    return 0;
}


/**
* @internal smemCht3ActiveWriteCncFastDumpTrigger function
* @endinternal
*
* @brief   Trigger for start of Centralized Counter Block Upload.
*         Called from Application task context by write to
*         CNC Fast Dump Trigger Register
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*
* @note relevant only for Cheetah3 packet processor
*
*/
void smemCht3ActiveWriteCncFastDumpTrigger
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    SBUF_BUF_ID bufferId;               /* Buffer ID */
    GT_U32 SelCNCOrFU;                 /* Type of updates to send to the CPU Fu/Cnc */
    GT_U8  * dataPtr;                   /* Pointer to the data in the buffer */
    GT_U32 dataSize;                    /* Data size */
    GT_U32 cncUnitIndex = 0;            /* index of one of the CNC units*/
    GT_U32 pipeId = 0,localPipeId;
    GT_U32 cnc_pipeId;
    GT_U32 bitIndex;

    *memPtr = *inMemPtr;

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        GT_CHAR* unitName = smemUnitNameByAddressGet(devObjPtr,address);
        if(0 == strcmp(unitName,STR(UNIT_CNC_1)))
        {
            cncUnitIndex = 1;
        }
        else
        if(0 == strcmp(unitName,STR(UNIT_CNC_2)))
        {
            cncUnitIndex = 2;
        }
        else
        if(0 == strcmp(unitName,STR(UNIT_CNC_3)))
        {
            cncUnitIndex = 3;
        }
    }
    else
    if(SMEM_CHT_IS_SIP6_15_GET(devObjPtr))
    {
        cncUnitIndex = 0;/* single CNC unit ! (do not use 'deviceCtrl14' of the DFX) */
    }
    else
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* get the unit name */
        GT_CHAR* unitName = smemUnitNameByAddressGet(devObjPtr,address);
        GT_U32   isCnc2 = 0;

        if(0 == strcmp(unitName,STR(UNIT_CNC_2)))
        {
            isCnc2 = 1;
        }
        else
        if(0 == strcmp(unitName,STR(UNIT_CNC_3)))
        {
            isCnc2 = 1;
        }

        bitIndex = 11 + isCnc2;

        /* get which one of the CNC units was triggered by the CPU */
        /* <CNC Dump Selector> */
        smemDfxRegFldGet(devObjPtr, SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl14,
            bitIndex, 1, &cncUnitIndex);

        cncUnitIndex += isCnc2*2;
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* the device always allowed to send CNC messages (when triggered) */
        pipeId = smemGetCurrentPipeId(devObjPtr);
        localPipeId = smemConvertGlobalPipeIdToTileAndLocalPipeIdInTile(devObjPtr,pipeId,NULL);

        bitIndex = 28 + localPipeId;/* bit 28 or 29 */

        /* get which one of the CNC units was triggered by the CPU */
        /* <CNC Dump Selector> */
        smemDfxRegFldGet(devObjPtr, SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl15,
            bitIndex, 1, &cncUnitIndex);
    }
    else
    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        /* cnc_pipe_select */
        smemDfxRegFldGet(devObjPtr, SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl16,
            20, 1, &pipeId);

        cnc_pipeId = smemGetCurrentPipeId(devObjPtr);
        if(pipeId != cnc_pipeId)
        {
            /* The CNC unit belong to pipeId but the 'DFX selector' point to other pipe*/
            /* so ignore the operation ?! */
            simWarningPrintf("smemCht3ActiveWriteCncFastDumpTrigger: The CNC unit belong to pipeId[%d] but the 'DFX selector' point to other pipe --> CNC DUMP operation ignored \n",
                cnc_pipeId);
            return;
        }

        /* the device always allowed to send CNC messages (when triggered) */
        bitIndex = 18 + pipeId;/* bit 18 or 19 */

        /* get which one of the CNC units was triggered by the CPU */
        /* <CNC Dump Selector> */
        smemDfxRegFldGet(devObjPtr, SMEM_CHT_MAC_REG_DB_DFX_SERVER_GET(devObjPtr)->resetAndInitCtrller.DFXServerUnitsBC2SpecificRegs.deviceCtrl16,
            bitIndex, 1, &cncUnitIndex);
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the device always allowed to send CNC messages (when triggered) */

        /* get which one of the CNC units was triggered by the CPU */
        /* <CNC Dump Selector> */
        smemRegFldGet(devObjPtr, SMEM_LION3_MG_GENERAL_CONFIG_REG(devObjPtr),28 , 1, &cncUnitIndex);
    }
    else
    {

        if (!SKERNEL_IS_LION2_DEV(devObjPtr))
        {
            smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 14, 1, &SelCNCOrFU );
            /* the bit must be reset to be 0 to permit CNC dump. */
            if (SelCNCOrFU)
            {
                return;
            }

            if(SKERNEL_IS_AC5_BASED_DEV(devObjPtr))
            {
                /* always check register in the CNC[0] ! ... and not CNC[1] */
                smemRegFldGet(devObjPtr,
                    SMEM_CHT_MAC_REG_DB_GET(devObjPtr)->centralizedCntrs[0].globalRegs.CNCMetalFixRegister,
                    16,1,
                    &cncUnitIndex);
                cncUnitIndex = 1 - cncUnitIndex;/* unit 0 : value = 1 , unit 1 : value = 0 */
            }
        }
        else
        {
            /* the device always allowed to send CNC messages (when triggered) */
        }

    }

    /* Get buffer */
    bufferId = sbufAlloc(devObjPtr->bufPool, 2 * sizeof(GT_U32));

    if (bufferId == NULL)
    {
        simWarningPrintf("smemCht3ActiveWriteCncFastDumpTrigger: no buffers to upload counters in CNC unit [%d], bmp of client[0x%8.8x] \n",
            cncUnitIndex,memPtr[0]);
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, &dataPtr, &dataSize);

    /* Copy CNC Fast Dump Trigger Register to buffer */
    memcpy(dataPtr, memPtr, sizeof(GT_U32));

    /* Copy cncUnitIndex to buffer */
    memcpy(dataPtr + sizeof(GT_U32), &cncUnitIndex, sizeof(GT_U32));

    /* Set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* Set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_CNC_FAST_DUMP_E;

    bufferId->pipeId = pipeId;

    /* Put buffer to queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/**
* @internal smemCht3ActiveCncBlockRead function
* @endinternal
*
* @brief   Centralized Counter Block Read.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
void smemCht3ActiveCncBlockRead
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32 wordIndex;               /* CNC block word index */
    GT_U32 start;                   /* CNC entry index */
    GT_U32 block;                   /* CNC block number */
    GT_U32 cncUnitIndex = smemCht3CncUnitIndexFromAddrGet(devObjPtr,address);
    GT_U32 mask;
    GT_U32 blockMask;
    GT_U32 numBits;
    GT_U32 baseAddressBlock0;/*base address of block 0*/
    GT_U32 addressToUse;/*address to use */

    *outMemPtr = *memPtr;

    blockMask = 0xF;/*support for 16 blocks*/

    if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
    {
        /* support 1K entries of 8 bytes --> 13 bits */
        numBits = 13;
        baseAddressBlock0 = param;/*base address of block 0*/
        blockMask = 0x1; /* Only two blocks */
    }
    else
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        numBits = 17;/* support 16K entries of 8 bytes --> 17 bits */
        baseAddressBlock0 = param;/*base address of block 0*/
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        numBits = 13;/* support 1K entries of 8 bytes --> 13 bits */
        baseAddressBlock0 = param;/*base address of block 0*/
    }
    else
    {
        /* support 2K entries of 8 bytes --> 14 bits */
        numBits = 14;
        baseAddressBlock0 = 0;
    }

    addressToUse = address - baseAddressBlock0;

    mask = SMEM_BIT_MASK(numBits);

    if(baseAddressBlock0 & mask) /* just sanity check */
    {
        /* the address of the unit must be aligned to the size of the block */
        skernelFatalError("smemCht3ActiveCncBlockRead: baseAddressBlock0[0x%8.8x] not aligned to mask[0x%8.8x]\n",
            baseAddressBlock0 , mask);
    }


    /* Calculate CNC block word index  */
    wordIndex = (addressToUse & mask) / 4;


    if (wordIndex % 2)
    {
        /* Calculate block and entry index */
        block = (addressToUse >> numBits) & blockMask;
        start = (addressToUse & mask) / 8;

        if(block < 16)
        {
            /* Reset CNC block entry after two words read */
            snetCht3CncBlockReset(devObjPtr, block, start, 1,cncUnitIndex);
        }
    }

}

/**
* @internal smemCht3ActiveCncWrapAroundStatusRead function
* @endinternal
*
* @brief   CNC Block Wraparound Status Register Read.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*
* @note relevant only for Cheetah3 packet processor
*
*/
void smemCht3ActiveCncWrapAroundStatusRead
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
)
{
    GT_U32 * regPtr;     /* register data pointer */
    GT_U32 cncBlock;     /* CNC Block index */
    GT_U32 cncUnitIndex; /* CNC Unit index  */
    GT_U32 firstBlockRegAddr; /* address of first block in unit */
    GT_U32 ii;                /* iterator */

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && !SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        /* formula for wraparound status registers in CNC unit is
           0x00001498 + (n * 0x10), where n is cncBlock */
        if ((address & 0xF) != 8)
        {
            *outMemPtr = 0xBADAD;
            return;
        }

        cncUnitIndex = smemCht3CncUnitIndexFromAddrGet(devObjPtr,address);

        firstBlockRegAddr = SMEM_CHT3_CNC_BLOCK_WRAP_AROUND_STATUS_REG(devObjPtr, 0, 0, cncUnitIndex);
        cncBlock = (address - firstBlockRegAddr) >> 4;

        if((cncBlock >= MAX_CNC_BLOCKS_IN_UNIT_CNS) ||
           (cncUnitIndex >= MAX_CNC_UNITS_CNS))
        {
            /* should not happen */
            skernelFatalError("smemCht3ActiveCncWrapAroundStatusRead: wrong CNC Unit [%d] or block[%d] for address [0x%8.8x]\n",
                cncUnitIndex, cncBlock, address);
        }

        /* AC5P/X and Harrier use FIFO to store status */
        regPtr = SMEM_SIP_6_10_CNC_WRAP_FIFO_MEM_GET(devObjPtr,cncUnitIndex, cncBlock);

        *outMemPtr = *regPtr;
        if (*regPtr & 1)
        {
            /* pop entry from FIFO */
            for (ii = 0; ii < (CNC_WRAP_FIFO_REGS_NUM_CNS - 1); ii++)
            {
                regPtr[ii] = regPtr[ii + 1];
            }

            /* invalidate last entry */
            regPtr[ii] = 0;
        }
    }
    else
    {
        *outMemPtr = *memPtr;

        *memPtr &= ~0x10001;
    }
}

/**
* @internal smemCht3ActiveWriteFDBGlobalCfgReg function
* @endinternal
*
* @brief   The function updates FDB table size according to FDB_Global_Configuration[bit24]
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemCht3ActiveWriteFDBGlobalCfgReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 fdbSize;

    /* data to be written */
    *memPtr = *inMemPtr;

    switch(devObjPtr->deviceFamily)
    {
        case SKERNEL_XCAT3_FAMILY:
        case SKERNEL_AC5_FAMILY:
            /* bit[24],[25]  - FDBSize */
            fdbSize = (*memPtr >> 24) & 0x3;

            switch(fdbSize)
            {
                default:
                    /* not support 32K*/
                    break;
                case 1:
                    devObjPtr->fdbNumEntries = SMEM_MAC_TABLE_SIZE_16KB;
                    break;
                case 2:
                    devObjPtr->fdbNumEntries = SMEM_MAC_TABLE_SIZE_8KB;
                    break;
                case 3:
                    devObjPtr->fdbNumEntries = SMEM_MAC_TABLE_SIZE_4KB;
                    break;
            }
            break;
        default: /* cheetah 3 , Lion */
            skernelFatalError("smemCht3ActiveWriteFDBGlobalCfgReg: CH3/Lion not supported \n");
    }

    if(devObjPtr->fdbMaxNumEntries < devObjPtr->fdbNumEntries)
    {
        skernelFatalError("smemCht3ActiveWriteFDBGlobalCfgReg: FDB num entries in HW [0x%4.4x] , but set to use[0x%4.4x] \n",
            devObjPtr->fdbMaxNumEntries , devObjPtr->fdbNumEntries);
    }

    return;
}

/**
* @internal smemXCatA1UnitPex function
* @endinternal
*
* @brief   Allocate address type specific memories -- for the PEX/MBus unit
*
* @param[in] devObjPtr                - pointer to device memory object.
* @param[in,out] unitPtr                  - pointer to the unit chunk
* @param[in] pexBaseAddr              - PCI/PEX/MNus unit base address
*/
void smemXCatA1UnitPex
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SMEM_UNIT_CHUNKS_STC  * unitPtr,
    IN GT_U32 pexBaseAddr
)
{
    {
        GT_U32 i;

        SMEM_CHUNK_BASIC_STC  chunksMem[]=
        {
             {SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001820 ,0x000018c4 )}  /* PCI Express Address Window Control */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001804 ,0x0000180c )}  /* PCI Express BAR Control */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x000018f8 ,0x000018fc )}  /* PCI Express Configuration Cycles Generation */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00000000 ,0x00000128 )}  /* PCI Express Configuration Header  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001a00 ,0x00001B00 )}  /* PCI Express Control and Status  */
            ,{SET_SMEM_CHUNK_BASIC_STC_ENTRY_BY_END_ADDR_MAC (0x00001900 ,0x00001910  )} /* PCI Express Interrupt  */
        };
        GT_U32  numOfChunks = sizeof(chunksMem)/sizeof(SMEM_CHUNK_BASIC_STC);

        for(i = 0; i < numOfChunks; i++)
        {
            chunksMem[i].memFirstAddr |= pexBaseAddr;
        }

        smemInitMemChunk(devObjPtr,chunksMem,numOfChunks,unitPtr);
    }
}



/**
* @internal smemCht3ActiveWriteCncInterruptsMaskReg function
* @endinternal
*
* @brief   the application changed the value of the interrupts mask register.
*         check if there is waiting interrupt for that.
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemCht3ActiveWriteCncInterruptsMaskReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32 dataArray[3];                     /* Data array to put into buffer */
    GT_U32 cncUnitIndex = smemCht3CncUnitIndexFromAddrGet(devObjPtr,address);

    /* Address of CNC interrupt cause register */
    dataArray[0] = SMEM_CHT3_CNC_INTR_CAUSE_REG(devObjPtr,cncUnitIndex);
    /* Address of CNC  interrupt mask register */
    dataArray[1] = SMEM_CHT3_CNC_INTR_MASK_REG(devObjPtr,cncUnitIndex);
    /* Summary bit in global interrupt cause register */
    dataArray[2] = SMEM_CHT_CNC_ENGINE_INT(devObjPtr,cncUnitIndex);

    /* Call for common interrupt mask active write function */
    smemChtActiveWriteInterruptsMaskReg(devObjPtr, address, memSize, memPtr,
                                        (GT_UINTPTR)dataArray, inMemPtr);
}

/**
* @internal smemXcatActiveWriteMacModeSelect function
* @endinternal
*
* @brief   The function check if the mac mode of the port changed , and due to that the
*         'mode' of this port , from working with the XG registers or the GIGA registers
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXcatActiveWriteMacModeSelect (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32  port;
    GT_U32  xgMode;
    GT_U32  numBits;

    /* data to be written */
    *memPtr = *inMemPtr;

    port = CHT_FROM_ADDRESS_GET_PORT_CNS(devObjPtr,address);

    if(port >= devObjPtr->portsNumber)
    {
        /* the XLG port registers are in memory space that match the 'port 12' .
        about the 'XLG port' - the port in Lion B that can be in the
        40G speed is port 10 but the actual memory space is as if the
        port is 12 */
        if(devObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_XLG_40G_E)
            return;
        else
            skernelFatalError("smemXcatActiveWriteMacModeSelect: port [%d] out of range \n",port);
    }

    if(devObjPtr->portsArr[port].supportMultiState != GT_TRUE)
    {
        return;
    }

    if(CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,port))
    {
        return; /* the 100G MAC used ... don't change it's mode */
    }

    if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
    {
        numBits = 3;
    }
    else
    {
        numBits = 2;
    }

    if(SMEM_U32_GET_FIELD((*inMemPtr), 13, numBits))
    {
        /* mode for 10,20,40 G*/
        /* for lion b also HGL */
        devObjPtr->portsArr[port].state = SKERNEL_PORT_STATE_XG_E;

        xgMode = 1;
    }
    else
    {
        /* mode for 1,2.5G */
        devObjPtr->portsArr[port].state = SKERNEL_PORT_STATE_GE_STACK_A1_E;

        xgMode = 0;
    }

    if(!SKERNEL_IS_LION2_PORT_GROUP_ONLY_DEV_MAC(devObjPtr))/* only for xCat3,5 */
    {
        /* set the 'Sampled at reset' register */

        /* set bits 26..29 in the 'Sampled at reset' to be sync with the
           SERDESs */
        smemRegFldSet(devObjPtr, SAMPLED_AT_RESET_ADDR, (port+2), 1, xgMode);
    }

    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
    {
        /* trigger smemBobKActiveWriteMacXlgCtrl4(...) to be called */
        smemRegUpdateAfterRegFile(devObjPtr,
            SMEM_XCAT_XG_MAC_CONTROL4_REG(devObjPtr, port),
            1);
    }



    return;
}



/*write active memory*/
void smemXCatActiveWriteBridgeGlobalConfig2Reg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* save the new value */
    *memPtr = *inMemPtr;

    if(devObjPtr->supportVpls)
    {
        devObjPtr->vplsModeEnable.bridge = SMEM_U32_GET_FIELD(*inMemPtr,17,1);
    }
}

void smemXCatActiveWriteEqGlobalConfigReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* save the new value */
    *memPtr = *inMemPtr;

    if(devObjPtr->supportVpls)
    {
        devObjPtr->vplsModeEnable.eq = SMEM_U32_GET_FIELD(*inMemPtr,24,1);
    }
}

void smemXCatActiveWriteIpclGlobalConfigReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* save the new value */
    *memPtr = *inMemPtr;

    if(devObjPtr->supportVpls)
    {
        devObjPtr->vplsModeEnable.ipcl = SMEM_U32_GET_FIELD(*inMemPtr,15,1);
    }
}

void smemXCatActiveWriteTtiGlobalConfigReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* save the new value */
    *memPtr = *inMemPtr;

    if(devObjPtr->supportVpls)
    {
        devObjPtr->vplsModeEnable.tti = SMEM_U32_GET_FIELD(*inMemPtr,29,1);
    }
}

void smemXCatActiveWriteHaGlobalConfigReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* save the new value */
    *memPtr = *inMemPtr;

    if(devObjPtr->supportVpls)
    {
        devObjPtr->vplsModeEnable.ha = SMEM_U32_GET_FIELD(*inMemPtr,16,1);
    }
}

void smemXCatActiveWriteMllGlobalConfigReg (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    /* save the new value */
    *memPtr = *inMemPtr;

    if(devObjPtr->supportVpls)
    {
        devObjPtr->vplsModeEnable.mll = SMEM_U32_GET_FIELD(*inMemPtr,14,1);
    }
}

/**
* @internal smemXCatActiveWriteEgressFilterVlanMap function
* @endinternal
*
* @brief   The EQ Egress Filter Vlan Map Table write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteEgressFilterVlanMap
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     lineNumber;      /* line number in the Table */
    GT_U32     rdWr;            /* read/write operation */
    GT_U32     regAddrTbl,regAddrDataAccess;         /* register address */
    GT_U32    *dataAccessRegPtr;/* register pointer */
    GT_U32    *tableEntryPtr;    /* pointer to table entry */

    if(devObjPtr->tablesInfo.eqVlanMapping.commonInfo.baseAddress == 0 || /* this device not support active memory on this register */
        ((*inMemPtr & 1) == 0))/* the triggering was not operated*/
    {
        *memPtr = *inMemPtr;
        return;
    }

    /* Action 0 - read , 1 - write */
    rdWr = SMEM_U32_GET_FIELD(*inMemPtr, 1, 1);
    /* Line Number */
    lineNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 12);

    /* the table entry */
    regAddrTbl = SMEM_XCAT_EQ_VLAN_MAPPING_TABLE_TBL_MEM(devObjPtr,
                                                   lineNumber);
    tableEntryPtr = smemMemGet(devObjPtr, regAddrTbl);

    /* Table Data Access */
    regAddrDataAccess = SMEM_XCAT_EQ_EGRESS_FILTER_VLAN_MAP_TBL_DATA_REG(devObjPtr);
    dataAccessRegPtr = smemMemGet(devObjPtr, regAddrDataAccess);

    if (rdWr == 0)
    {
        /* Read the entry --> copy info from the table */
        smemMemSet(devObjPtr,
            regAddrDataAccess,
            tableEntryPtr,
            devObjPtr->tablesInfo.eqVlanMapping.paramInfo[0].step/4);
    }
    else
    {
        /* Write to the entry --> copy info into the table */
        smemMemSet(devObjPtr,
            regAddrTbl,
            dataAccessRegPtr,
            devObjPtr->tablesInfo.eqVlanMapping.paramInfo[0].step/4);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemXCatActiveWriteEgressFilterVlanMember function
* @endinternal
*
* @brief   The EQ Egress Filter Vlan Member Table write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteEgressFilterVlanMember
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
)
{
    GT_U32     lineNumber;      /* line number in the Table */
    GT_U32     rdWr;            /* read/write operation */
    GT_U32     regAddrTbl,regAddrDataAccess;         /* register address */
    GT_U32    *dataAccessRegPtr;/* register pointer */
    GT_U32    *tableEntryPtr;    /* pointer to table entry */

    if(devObjPtr->tablesInfo.eqLogicalPortEgressVlanMember.commonInfo.baseAddress == 0 || /* this device not support active memory on this register */
        ((*inMemPtr & 1) == 0))/* the triggering was not operated*/
    {
        *memPtr = *inMemPtr;
        return;
    }

    /* Action 0 - read , 1 - write */
    rdWr = SMEM_U32_GET_FIELD(*inMemPtr, 1, 1);
    /* Line Number */
    lineNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 12);

    /* the table entry */
    regAddrTbl = SMEM_XCAT_EQ_LOGICAL_PORT_EGRESS_VLAN_MEMBER_TBL_MEM(devObjPtr,
                                                   lineNumber);
    tableEntryPtr = smemMemGet(devObjPtr, regAddrTbl);

    /* Table Data Access */
    regAddrDataAccess = SMEM_XCAT_EQ_EGRESS_FILTER_VLAN_MEMBER_TABLE_TBL_DATA_REG(devObjPtr);
    dataAccessRegPtr = smemMemGet(devObjPtr, regAddrDataAccess);

    if (rdWr == 0)
    {
        /* Read the entry --> copy info from the table */
        smemMemSet(devObjPtr,
            regAddrDataAccess,
            tableEntryPtr,
            devObjPtr->tablesInfo.eqLogicalPortEgressVlanMember.paramInfo[0].step/4);
    }
    else
    {
        /* Write to the entry --> copy info into the table */
        smemMemSet(devObjPtr,
            regAddrTbl,
            dataAccessRegPtr,
            devObjPtr->tablesInfo.eqLogicalPortEgressVlanMember.paramInfo[0].step/4);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemXCatActiveWriteL2MllVidxEnable function
* @endinternal
*
* @brief   The L2 MLL VIDX Enable Table write access.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteL2MllVidxEnable
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 address,
    IN GT_U32                 memSize,
    IN GT_U32                *memPtr,
    IN GT_UINTPTR             param,
    IN GT_U32                *inMemPtr
)
{
    GT_U32  lineNumber;                   /* line number in the Table */
    GT_U32  rdWr;                         /* read/write operation */
    GT_U32  regAddrTbl,regAddrDataAccess; /* register address */
    GT_U32 *dataAccessRegPtr;             /* register pointer */
    GT_U32 *tableEntryPtr;                /* pointer to table entry */

    if(devObjPtr->tablesInfo.l2MllVidxToMllMapping.commonInfo.baseAddress == 0 || /* this device not support active memory on this register */
        ((*inMemPtr & 1) == 0))/* the triggering was not operated*/
    {
        *memPtr = *inMemPtr;
        return;
    }

    /* Action 0 - read , 1 - write */
    rdWr = SMEM_U32_GET_FIELD(*inMemPtr, 1, 1);

    /* Line Number */
    lineNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 12);

    /* the table entry */
    regAddrTbl = SMEM_XCAT_L2_MLL_VIDX_TO_MLL_MAPPING_TBL_MEM(devObjPtr, lineNumber);
    tableEntryPtr = smemMemGet(devObjPtr, regAddrTbl);

    /* Table Data Access */
    regAddrDataAccess = SMEM_XCAT_L2_MLL_VIDX_ENABLE_TBL_DATA_REG(devObjPtr);
    dataAccessRegPtr = smemMemGet(devObjPtr, regAddrDataAccess);

    if (rdWr == 0)
    {
        /* Read the entry --> copy info from the table */
        smemMemSet(devObjPtr,
            regAddrDataAccess,
            tableEntryPtr,
            devObjPtr->tablesInfo.l2MllVidxToMllMapping.paramInfo[0].step/4);
    }
    else
    {
        /* Write to the entry --> copy info into the table */
        smemMemSet(devObjPtr,
            regAddrTbl,
            dataAccessRegPtr,
            devObjPtr->tablesInfo.l2MllVidxToMllMapping.paramInfo[0].step/4);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);

}

/**
* @internal smemXCatActiveWriteL2MllPointerMap function
* @endinternal
*
* @brief   The L2 MLL Pointer Map.
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] memSize                  - Size of memory to be written
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemXCatActiveWriteL2MllPointerMap
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 address,
    IN GT_U32                 memSize,
    IN GT_U32                *memPtr,
    IN GT_UINTPTR             param,
    IN GT_U32                *inMemPtr
)
{
    GT_U32  lineNumber;                   /* line number in the Table */
    GT_U32  rdWr;                         /* read/write operation */
    GT_U32  regAddrTbl,regAddrDataAccess; /* register address */
    GT_U32 *dataAccessRegPtr;             /* register pointer */
    GT_U32 *tableEntryPtr;                /* pointer to table entry */

    if(devObjPtr->tablesInfo.l2MllVirtualPortToMllMapping.commonInfo.baseAddress == 0 || /* this device not support active memory on this register */
        ((*inMemPtr & 1) == 0))/* the triggering was not operated*/
    {
        *memPtr = *inMemPtr;
        return;
    }

    /* Action 0 - read , 1 - write */
    rdWr = SMEM_U32_GET_FIELD(*inMemPtr, 1, 1);

    /* Line Number */
    lineNumber = SMEM_U32_GET_FIELD(*inMemPtr, 2, 12);

    /* the table entry */
    regAddrTbl = SMEM_XCAT_L2_MLL_VIRTUAL_PORT_TO_MLL_MAPPING_TBL_MEM(devObjPtr, lineNumber);
    tableEntryPtr = smemMemGet(devObjPtr, regAddrTbl);

    /* Table Data Access */
    regAddrDataAccess = SMEM_XCAT_L2_MLL_POINTER_MAP_TBL_DATA_REG(devObjPtr);
    dataAccessRegPtr = smemMemGet(devObjPtr, regAddrDataAccess);

    if (rdWr == 0)
    {
        /* Read the entry --> copy info from the table */
        smemMemSet(devObjPtr,
            regAddrDataAccess,
            tableEntryPtr,
            devObjPtr->tablesInfo.l2MllVirtualPortToMllMapping.paramInfo[0].step/4);
    }
    else
    {
        /* Write to the entry --> copy info into the table */
        smemMemSet(devObjPtr,
            regAddrTbl,
            dataAccessRegPtr,
            devObjPtr->tablesInfo.l2MllVirtualPortToMllMapping.paramInfo[0].step/4);
    }

    /* Cleared by the device when the read or write action is completed. */
    *memPtr = *inMemPtr & (~1);
}

/**
* @internal smemCht3TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*         override values set by smemCht2TableInfoSet
* @param[in] devObjPtr                - device object PTR.
*/
void smemCht3TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    smemCht2TableInfoSet(devObjPtr);

    devObjPtr->globalInterruptCauseRegister.pexInt     = 1;
    devObjPtr->globalInterruptCauseRegister.pexErr     = 2;
    devObjPtr->globalInterruptCauseRegister.cnc[0]     = 3;
    devObjPtr->globalInterruptCauseRegister.global1    = 20;/* AKA 'xg ports' */
    devObjPtr->globalInterruptCauseRegister.port24     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.port25     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.port26     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.port27     = SMAIN_NOT_VALID_CNS;
    devObjPtr->globalInterruptCauseRegister.iplr0      = 24;

    if(devObjPtr->policerSupport.tablesBaseAddrSetByOrigDev == 0)
    {
        devObjPtr->tablesInfo.policer.commonInfo.baseAddress = 0x0C040000;
        devObjPtr->tablesInfo.policer.paramInfo[0].step = 0x20; /*entry*/
        devObjPtr->tablesInfo.policer.paramInfo[1].step = 0; /* not supported*/

        devObjPtr->tablesInfo.policerReMarking.commonInfo.baseAddress = 0x0C080000;

        devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress = devObjPtr->tablesInfo.policer.commonInfo.baseAddress + (1024 * 0x20);
        devObjPtr->tablesInfo.policerCounters.paramInfo[0].step = 0x20;/*entry*/
        devObjPtr->tablesInfo.policerCounters.paramInfo[1].step = 0; /* not supported*/

        devObjPtr->tablesInfo.policerTblAccessData.commonInfo.baseAddress = 0xC000030;
        devObjPtr->tablesInfo.policerTblAccessData.paramInfo[0].step = 0x4; /*entry*/
    }

    devObjPtr->tablesInfo.pclConfig.commonInfo.baseAddress = 0x0B840000;
    devObjPtr->tablesInfo.pclConfig.paramInfo[0].step = 0x8; /*entry*/

    devObjPtr->tablesInfo.pclAction.commonInfo.baseAddress = 0x0B880000;

    devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress = 0x0BA00000;
    devObjPtr->tablesInfo.pclTcam.paramInfo[0].step = 0x40;  /*entry*/
    devObjPtr->tablesInfo.pclTcam.paramInfo[1].step = 0x8;   /*word*/

    devObjPtr->tablesInfo.pclTcamMask.commonInfo.baseAddress = devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress + 0x20;
    devObjPtr->tablesInfo.pclTcamMask.paramInfo[0].step = devObjPtr->tablesInfo.pclTcam.paramInfo[0].step;   /*entry*/
    devObjPtr->tablesInfo.pclTcamMask.paramInfo[1].step = devObjPtr->tablesInfo.pclTcam.paramInfo[1].step;   /*word*/

    devObjPtr->tablesInfo.ttiAction.commonInfo.baseAddress = 0x02900000;
    devObjPtr->tablesInfo.ttiAction.paramInfo[0].step = 0x4;     /*entry*/
    devObjPtr->tablesInfo.ttiAction.paramInfo[1].step = 0x5000;  /*word*/


    devObjPtr->tablesInfo.vrfId.commonInfo.baseAddress = 0x0A300000;
    devObjPtr->tablesInfo.vrfId.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.ingressVlanTranslation.commonInfo.baseAddress = SMEM_CHT3_INGRESS_VLAN_TRANSLATION_TBL_MEM;
    devObjPtr->tablesInfo.ingressVlanTranslation.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.egressVlanTranslation.commonInfo.baseAddress = SMEM_CHT3_EGRESS_VLAN_TRANSLATION_TBL_MEM;
    devObjPtr->tablesInfo.egressVlanTranslation.paramInfo[0].step = 0x4;

    devObjPtr->tablesInfo.macToMe.commonInfo.baseAddress = 0x0B800700;
    devObjPtr->tablesInfo.macToMe.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.ieeeRsrvMcCpuIndex.commonInfo.baseAddress = 0x02000804;
    devObjPtr->tablesInfo.ieeeRsrvMcCpuIndex.paramInfo[0].step = 0x1000;
    devObjPtr->tablesInfo.ieeeRsrvMcCpuIndex.paramInfo[0].divider = 16;

    devObjPtr->tablesInfo.routeNextHopAgeBits.commonInfo.baseAddress = 0x2801000; /*Router Next Hop Table Age Bits*/
    devObjPtr->tablesInfo.routeNextHopAgeBits.paramInfo[0].step = 0x4;

    if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        devObjPtr->tablesInfo.xgPortMibCounters.commonInfo.baseAddress = 0x09000000;
        devObjPtr->tablesInfo.xgPortMibCounters.paramInfo[0].step = 0x20000;
    }

    devObjPtr->tablesInfo.secondTargetPort.commonInfo.baseAddress = 0x01E40000;
    devObjPtr->tablesInfo.secondTargetPort.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.upToQoSProfile.commonInfo.baseAddress   = 0x0B8004A0;
    devObjPtr->tablesInfo.upToQoSProfile.paramInfo[1].step = 0x10;   /* CFI */


    /* note those values may be override by the xcat A1 device -->
        see function smemXcatA1TableInfoSet */

    devObjPtr->tablesInfo.pclActionTcamData.commonInfo.baseAddress = 0x0d000100;

    devObjPtr->tablesInfo.pclAction.commonInfo.baseAddress = 0x0D0B0000;

    devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress = 0x0D040000;
    devObjPtr->tablesInfo.pclTcamMask.commonInfo.baseAddress = devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress + 0x20;

    devObjPtr->tablesInfo.routerTcam.commonInfo.baseAddress = 0x0D840000;
    devObjPtr->tablesInfo.routerTcam.paramInfo[0].step = devObjPtr->routeTcamInfo.entryWidth;  /*entry*/
    devObjPtr->tablesInfo.routerTcam.paramInfo[1].step = 0x4;                                  /*word*/

    devObjPtr->tablesInfo.ttiAction.commonInfo.baseAddress = 0x0d880000;
    devObjPtr->tablesInfo.ttiAction.paramInfo[0].step = 0x10;   /*entry*/
    devObjPtr->tablesInfo.ttiAction.paramInfo[1].step = 0x4;    /*word*/

    if(devObjPtr->xgCountersStepPerPort == 0)
    {
        devObjPtr->xgCountersStepPerPort = 0x20000;
    }

    devObjPtr->tablesInfo.cncMemory.commonInfo.baseAddress = 0x08080000;
    devObjPtr->tablesInfo.cncMemory.paramInfo[0].step = 0x4000;/* block*/
    devObjPtr->tablesInfo.cncMemory.paramInfo[1].step = 8;/*index*/

}

/**
* @internal smemXcatA1TableInfoSet function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*         override values set by smemCht3TableInfoSet
* @param[in] devObjPtr                - device object PTR.
*/
void smemXcatA1TableInfoSet
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  iplr0ToIplr1Offset;
    GT_BIT  updatePlr = devObjPtr->policerSupport.tablesBaseAddrSetByOrigDev ? 0 : 1;

    smemCht3TableInfoSet(devObjPtr);

    if(devObjPtr->memUnitBaseAddrInfo.policer[0] == 0)
    {
        /* DON'T OVERRIDE ADDRESS !!!
           ALLOW MORE ADVANCED DEVICE TO SET OTHER ADDRESSES */
        devObjPtr->memUnitBaseAddrInfo.policer[0] = 0x0C800000;
        devObjPtr->memUnitBaseAddrInfo.policer[1] = 0x0D000000;
        devObjPtr->memUnitBaseAddrInfo.policer[2] = 0x03800000;
    }


    devObjPtr->globalInterruptCauseRegister.tti        = 4;
    devObjPtr->globalInterruptCauseRegister.tccLower   = 5;
    devObjPtr->globalInterruptCauseRegister.tccUpper   = 6;
    devObjPtr->globalInterruptCauseRegister.bcn        = 7;
    devObjPtr->globalInterruptCauseRegister.misc       = 9;
    devObjPtr->globalInterruptCauseRegister.mem        = 10;
    devObjPtr->globalInterruptCauseRegister.txq        = 11;
    devObjPtr->globalInterruptCauseRegister.l2i        = 12;
    devObjPtr->globalInterruptCauseRegister.bm0        = 13;
    devObjPtr->globalInterruptCauseRegister.bm1        = 14;
    devObjPtr->globalInterruptCauseRegister.fdb        = 15;
    devObjPtr->globalInterruptCauseRegister.ports      = 16;
    devObjPtr->globalInterruptCauseRegister.cpuPort    = 17;
    devObjPtr->globalInterruptCauseRegister.global1    = 18;/* AKA 'xg ports' */
    devObjPtr->globalInterruptCauseRegister.txSdma     = 19;
    devObjPtr->globalInterruptCauseRegister.rxSdma     = 20;
    devObjPtr->globalInterruptCauseRegister.pcl        = 21;
    devObjPtr->globalInterruptCauseRegister.iplr0      = 22;
    devObjPtr->globalInterruptCauseRegister.iplr1      = 23;
    devObjPtr->globalInterruptCauseRegister.eq         = 24;
    devObjPtr->globalInterruptCauseRegister.eplr       = 8;

    devObjPtr->tablesInfo.stp.commonInfo.baseAddress = 0x0a080000;

    devObjPtr->tablesInfo.mcast.commonInfo.baseAddress = 0x0a100000;

    devObjPtr->tablesInfo.vlan.commonInfo.baseAddress = 0x0a200000;
    devObjPtr->tablesInfo.vlan.paramInfo[0].step = 0x20;

    devObjPtr->tablesInfo.portVlanQosConfig.commonInfo.baseAddress = 0x0c001000;
    devObjPtr->tablesInfo.portVlanQosConfig.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.arpTable.commonInfo.baseAddress = 0x0F040000;

    devObjPtr->tablesInfo.vlanPortMacSa.commonInfo.baseAddress = 0x0F008000;

    devObjPtr->tablesInfo.pclConfig.commonInfo.baseAddress = 0x0b810000;
    devObjPtr->tablesInfo.pclConfig.paramInfo[0].step = 0x8; /*entry*/
    devObjPtr->tablesInfo.pclConfig.paramInfo[1].step = 0x10000; /* IPCL0/IPCL1 */

    devObjPtr->tablesInfo.pclAction.commonInfo.baseAddress = 0x0e0b0000;

    devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress = 0x0e040000;

    devObjPtr->tablesInfo.pclTcamMask.commonInfo.baseAddress = devObjPtr->tablesInfo.pclTcam.commonInfo.baseAddress + 0x20;

    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
    {
        /* next table instance per PLR */
        devObjPtr->tablesInfo.policerConformanceLevelSign.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerConformanceLevelSign.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        /* next tables SINGLE instance in the IPLR 0 only */
        devObjPtr->tablesInfo.policer.commonInfo.multiInstanceInfo.numBaseAddresses = 0;
        devObjPtr->tablesInfo.policerCounters.commonInfo.multiInstanceInfo.numBaseAddresses = 0;
        devObjPtr->tablesInfo.policerConfig.commonInfo.multiInstanceInfo.numBaseAddresses = 0;
        /*the EPLR not share this table with IPLR0/1*/
    }
    else
    {
        /* eplr tables */
        {
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.egressPolicerCounters.commonInfo.baseAddress,
                        SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,2,0x00060000));
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.egressPolicerCounters.paramInfo[0].step , 0x20);

            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.egressPolicerMeters.commonInfo.baseAddress ,
                        SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,2,0x00040000));
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.egressPolicerMeters.paramInfo[0].step , 0x20);
        }

        /* iplr tables */
        {
            iplr0ToIplr1Offset = devObjPtr->memUnitBaseAddrInfo.policer[1] -
                                 devObjPtr->memUnitBaseAddrInfo.policer[0];

            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policer.commonInfo.baseAddress ,
                        SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00040000));
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policer.paramInfo[0].step , 0x20); /*entry*/
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policer.paramInfo[1].step , iplr0ToIplr1Offset);/*cycle*/

            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress ,
                        SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00060000));
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerCounters.paramInfo[0].step , 0x20);/*entry*/
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerCounters.paramInfo[1].step , iplr0ToIplr1Offset);/*cycle*/

            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerConfig.paramInfo[1].step , iplr0ToIplr1Offset);/*cycle*/
                SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerConformanceLevelSign.paramInfo[1].step , iplr0ToIplr1Offset);/*cycle*/
            }
        }
    }

    /* policer tables that are 'duplicated' for IPLR0,1,EPLR */
    {
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerReMarking.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00080000));
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerReMarking.paramInfo[0].step , 0x8); /*entry*/

        devObjPtr->tablesInfo.policerReMarking.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerReMarking.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];

        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerTblAccessData.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00000074));
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerTblAccessData.paramInfo[0].step , 0x4); /*entry*/

        devObjPtr->tablesInfo.policerTblAccessData.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerTblAccessData.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];

        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerMeterPointer.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00001800));
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerMeterPointer.paramInfo[0].step , 0x4); /*entry*/

        devObjPtr->tablesInfo.policerMeterPointer.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerMeterPointer.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];
        if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            devObjPtr->tablesInfo.policerMeterPointer.paramInfo[0].divider = 2;
        }

        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerIpfixWaAlert.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00000800));
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerIpfixWaAlert.paramInfo[0].step , 0x4); /*entry*/

        devObjPtr->tablesInfo.policerIpfixWaAlert.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerIpfixWaAlert.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];
        devObjPtr->tablesInfo.policerIpfixWaAlert.paramInfo[0].divider = 32;

        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerIpfixAgingAlert.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00000900));
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerIpfixAgingAlert.paramInfo[0].step , 0x4); /*entry*/

        devObjPtr->tablesInfo.policerIpfixAgingAlert.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerIpfixAgingAlert.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];
        devObjPtr->tablesInfo.policerIpfixAgingAlert.paramInfo[0].divider = 32;

        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerTimer.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00000200));
        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerTimer.paramInfo[0].step , 0x4); /* entry */

        devObjPtr->tablesInfo.policerTimer.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerTimer.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];

        SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerManagementCounters.commonInfo.baseAddress ,
                    SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00000500));
        devObjPtr->tablesInfo.policerManagementCounters.paramInfo[0].step = 0x10; /* entry */ /* 4 entries are 'counter set'*/
        devObjPtr->tablesInfo.policerManagementCounters.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
        devObjPtr->tablesInfo.policerManagementCounters.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];

        devObjPtr->policerSupport.meterTblBaseAddr = devObjPtr->tablesInfo.policer.commonInfo.baseAddress;
        devObjPtr->policerSupport.countTblBaseAddr = devObjPtr->tablesInfo.policerCounters.commonInfo.baseAddress;

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* new in SIP_6 */
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerQosAttributes.commonInfo.baseAddress ,
                        SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00005000));
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerQosAttributes.paramInfo[0].step , 0x4); /*entry*/

            devObjPtr->tablesInfo.policerQosAttributes.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
            devObjPtr->tablesInfo.policerQosAttributes.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];
        }
        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            /* new in SIP_6 */
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerIpfix1StNPackets.commonInfo.baseAddress ,
                        SMEM_XCAT_POLICER_OFFSET_MAC(devObjPtr,0,0x00070000));
            SET_IF_CONDITION_TRUE_MAC(updatePlr,devObjPtr->tablesInfo.policerIpfix1StNPackets.paramInfo[0].step , 0x4); /*entry*/

            devObjPtr->tablesInfo.policerIpfix1StNPackets.commonInfo.multiInstanceInfo.numBaseAddresses = 3;
            devObjPtr->tablesInfo.policerIpfix1StNPackets.commonInfo.multiInstanceInfo.multiUnitsBaseAddressPtr = &devObjPtr->memUnitBaseAddrInfo.policer[0];
            devObjPtr->tablesInfo.policerIpfix1StNPackets.paramInfo[0].divider = 32;
        }
    }

    devObjPtr->tablesInfo.ipclUserDefinedBytesConf.commonInfo.baseAddress = 0x0C010000;
    devObjPtr->tablesInfo.ipclUserDefinedBytesConf.paramInfo[0].step = 0x20;

    devObjPtr->tablesInfo.tunnelStartTable.commonInfo.baseAddress = devObjPtr->tablesInfo.arpTable.commonInfo.baseAddress;
    devObjPtr->tablesInfo.tunnelStartTable.paramInfo[0].step = devObjPtr->tablesInfo.arpTable.paramInfo[0].step;

    devObjPtr->tablesInfo.routerTcam.commonInfo.baseAddress = 0x0e840000;
    devObjPtr->tablesInfo.routerTcam.paramInfo[0].step = devObjPtr->routeTcamInfo.entryWidth;/*entry*/
    devObjPtr->tablesInfo.routerTcam.paramInfo[1].step = 0x4;                                /*word*/


    devObjPtr->tablesInfo.ttiAction.commonInfo.baseAddress = 0x0e8c0000;
    devObjPtr->tablesInfo.ttiAction.paramInfo[0].step = 0x20;   /*entry*/
    devObjPtr->tablesInfo.ttiAction.paramInfo[1].step = 0x4;

    devObjPtr->tablesInfo.vrfId.commonInfo.baseAddress = 0x0a180000;

    devObjPtr->tablesInfo.ingressVlanTranslation.commonInfo.baseAddress = 0x0c00c000;

    devObjPtr->tablesInfo.egressVlanTranslation.commonInfo.baseAddress = 0x0f010000;

    devObjPtr->tablesInfo.macToMe.commonInfo.baseAddress = 0x0C001600;


    devObjPtr->tablesInfo.mllTable.commonInfo.baseAddress = 0x0d880000;

    devObjPtr->tablesInfo.mllOutInterfaceConfig.commonInfo.baseAddress = 0x0d800980;

    devObjPtr->tablesInfo.mllOutInterfaceCounter.commonInfo.baseAddress = 0x0d800900;

    devObjPtr->memUnitBaseAddrInfo.mllRegBase = 0x0d800000;

    devObjPtr->tablesInfo.logicalTrgMappingTable.commonInfo.baseAddress = 0x0b600000;
    devObjPtr->tablesInfo.logicalTrgMappingTable.paramInfo[0].step = 0x100; /* target device */
    devObjPtr->tablesInfo.logicalTrgMappingTable.paramInfo[1].step = 0x4; /* target port */

    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.commonInfo.baseAddress = 0x02000800;
    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.paramInfo[0].step = 0x1000;
    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.paramInfo[0].divider = 16;
    devObjPtr->tablesInfo.ieeeRsrvMcConfTable.paramInfo[1].step = 0x8;

    devObjPtr->tablesInfo.ieeeTblSelect.commonInfo.baseAddress = 0x0200080C;
    devObjPtr->tablesInfo.ieeeTblSelect.paramInfo[0].step = 0x10;
    devObjPtr->tablesInfo.ieeeTblSelect.paramInfo[0].divider = 16;

    devObjPtr->tablesInfo.dscpToDscpMap.commonInfo.baseAddress    = 0x0C000900;
    devObjPtr->tablesInfo.dscpToQoSProfile.commonInfo.baseAddress = 0x0C000400;

    /* CFI UP -> QosProfile */
    devObjPtr->tablesInfo.upToQoSProfile.commonInfo.baseAddress   = 0x0C000440;
    devObjPtr->tablesInfo.expToQoSProfile.commonInfo.baseAddress  = 0x0c000460;

    devObjPtr->tablesInfo.l2PortIsolation.commonInfo.baseAddress = 0x01E40004;
    devObjPtr->tablesInfo.l2PortIsolation.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.l3PortIsolation.commonInfo.baseAddress = 0x01E40008;
    devObjPtr->tablesInfo.l3PortIsolation.paramInfo[0].step = 0x10;

    devObjPtr->tablesInfo.portProtocolVidQoSConf.commonInfo.baseAddress = 0x0C004000;
    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[0].step = 0x80;
    devObjPtr->tablesInfo.portProtocolVidQoSConf.paramInfo[1].step = 0x8;

}

/**
* @internal smemXcatA1TableInfoSetPart2 function
* @endinternal
*
* @brief   set the table info for the device --> fill devObjPtr->tablesInfo
*         AFTER the bound of memories (after calling smemBindTablesToMemories)
* @param[in] devObjPtr                - device object PTR.
*/
void smemXcatA1TableInfoSetPart2
(
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
}


