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
* @file cpssDxChTxHandling.c
*
* @brief This file implements all needed functions for sending packets of upper
* layer to the cpu port of the DXCH device (PP's Tx Queues).
*
* @version   46
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetworkIfLog.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>

#include <cpss/generic/networkIf/cpssGenNetIfMii.h>
#include <cpss/generic/networkIf/private/prvCpssGenNetworkIfMii.h>

#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdma.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwGdmaNetIfFunc.h>

#define _25MHZ_CLK_CNS       25000000 /* Sdma clock */
#define _12_5MHZ_CLK_CNS     12500000 /* Gdma clock */
#define GDMA_TMR_LAST_BIT    30

#define NSEC_PER_MSEC_CNS 1000000
#define NSEC_PER_SEC_CNS  1000000000
#define MAX_GT_U32      ((GT_U32)-1)

/* Local Queue 0..7 vs global 0..32 (CPSS_MAX_TX_QUEUE_CNS = NUM_OF_TX_QUEUES) */
#define _LQ_MAC(_GQueue)   ((_GQueue) & (CPSS_MAX_TX_QUEUE_CNS-1))

/*#define DBG_TRACE(_p) cpssOsPrintf _p */
#define DBG_TRACE(_p)

/**
* @struct PRV_QUEUE_TIMER_CFG_STC
*
* @brief   Struct containing Tx Queue Timer configuration parameters
*          used by TxGenerator for packet RateGet/Rate-calculate/RateSset
*
* @note   APPLICABLE DEVICES :     All.
* @note   NOT APPLICABLE DEVICES:  None.
*/
typedef struct {
    GT_BOOL gdma;   /* GT_TRUE the configuration for GDMA or GT_FALSE for SDMA unit & timer */
    GT_U32  unitId; /* GDMA unit number for GDMA or MG-unit number for SDMA */
    GT_U32  netIfNum;   /* "first"   netIf      index [0..8] in SW-DB cfg[8][8] */
    GT_U32  localQueue; /* "second" local queue index [0..8] in SW-DB cfg[8][8] */
    GT_U32  clkFreq;    /* Timer clock frequency MHz (HW hardcoded) */
    GT_U32  clkNanoSec; /* NanoSec time interval between 2 clocks (1/clkFreq) */
    GT_U32  enabled;    /* Timer is enabled=1 or disabled=0 (in a HW register) */
    GT_U32  clkVal;     /* Timer wrap clock value (in a HW register) */
} PRV_QUEUE_TIMER_CFG_STC;

/*******************************************************************************
* External usage environment parameters
*******************************************************************************/


/*******************************************************************************
* Internal usage environment parameters
*******************************************************************************/
#define PRV_CPSS_CHECK_xDMA_TX_Q_MAC(_devNum, _txQueue)         \
    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)    \
    {                                                           \
        PRV_CPSS_DXCH_GDMA_CHECK_SDMA_Q_MAC(devNum, /*ToCPU*/GT_FALSE, _txQueue); \
    }                                                           \
    else                                                        \
    {                                                           \
        PRV_CPSS_CHECK_SDMA_TX_Q_MAC(_devNum, _txQueue);        \
    }

/**
* @internal prvQueueIsTxGenerator function
* @endinternal
*
* @brief   Helper returning GT_TRUE if given TX Queue number is TxGenerator or GT_FALSE otherwise
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue.
*
* @retval   GT_TRUE if given TX Queue number is TxGenerator or GT_FALSE otherwise
*/
static GT_BOOL prvQueueIsTxGenerator
(
    IN  GT_U8     devNum,
    IN  GT_U8     txQueue
)
{
    GT_U32      netIfNum = txQueue / CPSS_MAX_TX_QUEUE_CNS;
    GT_U32      localQueue = _LQ_MAC(txQueue);
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;

    queueMode = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum)->multiNetIfCfg.
                        txSdmaQueuesConfig[netIfNum][localQueue].queueMode;

    return (queueMode == CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E);
}

/*
* @internal prvTxQueuePropertyCheckAnyTx function
* @brief    Basic TX-QueueId checker disregarding to TxGenerator or not
*/
static GT_STATUS prvTxQueuePropertyCheckAnyTx
(
    IN  GT_U8     devNum,
    IN  GT_U8     txQueue
)
{
    GT_U32  netIfNum;
    GT_U32  unitId;

    /* GDMA has single MG_UNIT only, but NetIf structs organized as [netIf=4][MAX_QUEUE=8] */
    netIfNum = txQueue / CPSS_MAX_TX_QUEUE_CNS;

    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_DXCH_GDMA_CHECK_SDMA_Q_MAC(devNum, /*ToCPU*/GT_FALSE, txQueue);
    }
    else
    {
        PRV_CPSS_DXCH_DEV_TX_SDMA_QUEUE_NUM_CHECK_MAC(txQueue);
        PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum, netIfNum, unitId);
    }
    return GT_OK;
}

/*
* @internal prvTxQueuePropertyCheck function
*/
static GT_STATUS prvTxQueuePropertyCheck
(
    IN  GT_U8     devNum,
    IN  GT_U8     txQueue
)
{
    GT_STATUS rc;

    rc = prvTxQueuePropertyCheckAnyTx(devNum, txQueue);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (prvQueueIsTxGenerator(devNum, txQueue) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    return GT_OK;
}

/*
* @internal prvTxQueuePropertyCheckAndConvert function
*/
static GT_STATUS prvTxQueuePropertyCheckAndConvert
(
    IN  GT_U8     devNum,
    IN  GT_U8     txQueue,
    OUT GT_U32    *netIfNumPtr,
    OUT GT_U32    *unitIdPtr  /* MG-unitId or GDMA-unitId */
)
{
    GT_U32      netIfNum;
    GT_U32      unitId;
    GT_U32      localQueue;
    GT_STATUS   rc;

    rc = prvTxQueuePropertyCheck(devNum, txQueue);
    if (rc != GT_OK)
        return rc;

    /* GDMA has only single NetIf, but structs are organized as [netIf=4][MAX_QUEUE=8] */
    netIfNum = txQueue / CPSS_MAX_TX_QUEUE_CNS;

    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        PRV_CPSS_GDMA_CONVERT_TX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum, txQueue,
                                                                       /*OUT*/ unitId, localQueue);
        GT_UNUSED_PARAM(localQueue);
    }
    else
    {
        PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum, netIfNum, unitId);
    }
    if (netIfNumPtr != NULL)
    {
        *netIfNumPtr = netIfNum;
    }
    if (unitIdPtr != NULL)
    {
        *unitIdPtr = unitId;
    }
    return GT_OK;
}

/*
* @internal prvQueueRatePropertyCheckAndInit function
*/
static GT_STATUS prvQueueRatePropertyCheckAndInit
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           txQueue,
    OUT PRV_QUEUE_TIMER_CFG_STC         *timerCfgPtr
)
{
    GT_U32      netIfNum;
    GT_U32      unitId;
    GT_U32      localQueue;
    GT_STATUS   rc;

    rc = prvTxQueuePropertyCheck(devNum, txQueue);
    if (rc != GT_OK)
        return rc;

    /* GDMA has only single NetIf, but structs are organized as [netIf=4][MAX_QUEUE=8] */
    netIfNum = txQueue / CPSS_MAX_TX_QUEUE_CNS;
    timerCfgPtr->netIfNum = netIfNum;
    timerCfgPtr->localQueue = txQueue % CPSS_MAX_TX_QUEUE_CNS;

    timerCfgPtr->gdma = PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum);
    if (timerCfgPtr->gdma)
    {
        PRV_CPSS_GDMA_CONVERT_TX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(devNum, txQueue,
                                                                       /*OUT*/ unitId, localQueue);
        GT_UNUSED_PARAM(localQueue);
        timerCfgPtr->clkFreq = _12_5MHZ_CLK_CNS;
    }
    else
    {
        PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum, netIfNum, unitId);
        timerCfgPtr->clkFreq = _25MHZ_CLK_CNS;
    }
    timerCfgPtr->unitId = unitId;
    timerCfgPtr->clkNanoSec = NSEC_PER_SEC_CNS / timerCfgPtr->clkFreq;
    /* Fields 'enabled' and 'clkVal' are initialized by prvQueueTimerGet */
    return GT_OK;
}

/*
* @internal prvQueueTimerGet function
*/
static GT_STATUS prvQueueTimerGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           txQueue,
    OUT PRV_QUEUE_TIMER_CFG_STC        *timerCfgPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      value;

    rc = prvQueueRatePropertyCheckAndInit(devNum, txQueue, timerCfgPtr);
    if (rc != GT_OK)
        return rc;

    if (timerCfgPtr->gdma)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueControl;
        rc = prvCpssHwPpGdmaGetRegField(devNum, timerCfgPtr->unitId, regAddr, 7, 1, &value);
        if (rc != GT_OK)
            return rc;
        timerCfgPtr->enabled = (value != 0);

        if (timerCfgPtr->enabled)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).gdmaTimerRegs[txQueue].timerValue;
            rc = prvCpssHwPpGdmaGetRegField(devNum, timerCfgPtr->unitId, regAddr,
                                            0, GDMA_TMR_LAST_BIT, &value);
            timerCfgPtr->clkVal = value;
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txSdmaPacketGeneratorConfigQueue[timerCfgPtr->localQueue];
        rc = prvCpssHwPpMgReadReg(devNum, timerCfgPtr->unitId, regAddr, &value);
        timerCfgPtr->enabled = ((value & 0x2) != 0);
        timerCfgPtr->clkVal = value >> 2; /* 30bits */
    }
    return rc;
}

/*
* @internal prvQueueTimerSet function
*/
static GT_STATUS prvQueueTimerSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           txQueue,
    IN  PRV_QUEUE_TIMER_CFG_STC         *timerCfgPtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;
    GT_U32      value;

    rc = prvQueueRatePropertyCheckAndInit(devNum, txQueue, timerCfgPtr);
    if (rc != GT_OK)
        return rc;

    if (timerCfgPtr->gdma)
    {
        regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueControl;
        rc = prvCpssHwPpGdmaSetRegField(devNum, timerCfgPtr->unitId, regAddr, 7, 1, timerCfgPtr->enabled);
        if (rc != GT_OK)
            return rc;

        if (timerCfgPtr->enabled)
        {
            regAddr = PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).gdmaTimerRegs[txQueue].timerValue;
            rc = prvCpssHwPpGdmaSetRegField(devNum, timerCfgPtr->unitId, regAddr,
                                            0, GDMA_TMR_LAST_BIT, timerCfgPtr->clkVal);
        }
    }
    else
    {
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txSdmaPacketGeneratorConfigQueue[timerCfgPtr->localQueue];
        value = (timerCfgPtr->clkVal << 2) | (timerCfgPtr->enabled << 1);
        rc = prvCpssHwPpMgWriteRegBitMask(devNum, timerCfgPtr->unitId, regAddr, 0xFFFFFFFE, value);
    }
    return rc;
}

/**
* @internal prvCpssDxChNetIfSdmaTxGeneratorNPacketIntervalsWait function
* @endinternal
*
* @brief   This function imposes a wait of 3 packet intervals.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on hardware error
*
* @note Waiting (sleeping) for 3 packet generator intervals before updating
*       descriptor & buffer is required because they might be in use. One (1)
*       interval might be not enough due to the descriptor prefetch mechanism.
*
*/
static GT_STATUS prvCpssDxChNetIfSdmaTxGeneratorNPacketIntervalsWait
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           txQueue,
    IN  GT_U8                           numPacketIntervals
)
{
    OUT PRV_QUEUE_TIMER_CFG_STC timerCfg;
    GT_U32                      sleepTime;  /* sleep time in ms */
    GT_STATUS                   rc;

    if (numPacketIntervals == 0)
        return GT_OK;

    rc = prvQueueTimerGet(devNum, txQueue, &timerCfg);
    if( GT_OK != rc )
        return rc;

    if (timerCfg.enabled == 0)
    {
        /* Tx without packet interval, use 1ms sleep */
        sleepTime = 1;
    }
    else
    {
        /* calculate sleep time in ms and add 1 due to rounding */
        if (timerCfg.clkVal < (MAX_GT_U32 / timerCfg.clkNanoSec))
        {
            /* Timer value is small - no GT_U32 overload */
            sleepTime = (timerCfg.clkVal * timerCfg.clkNanoSec) / NSEC_PER_MSEC_CNS;
        }
        else
        {
            /* Timer value is big - div/mul to avoid GT_U32 overload */
            sleepTime = (timerCfg.clkVal / 256) * timerCfg.clkNanoSec;
            sleepTime = (sleepTime / NSEC_PER_MSEC_CNS) * 256;
        }
        sleepTime += 1;
        sleepTime *= numPacketIntervals;
    }

    cpssOsTimerWkAfter(sleepTime);

    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfSdmaTxGeneratorRateCalc function
* @endinternal
*
* @brief   This function calculate the required number of clock ticks to configure
*         in HW for the packet interval.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] rateMode                 - transmission rate mode.
* @param[in] rateValue                - packets per second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
* @param[in] packetsInChain           - number of packets in the generator chain.
*
* @param[out] clkTicksPtr              - (pointer to) number of clock ticks to configure
*                                      in HW for packet gap.
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValue.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad rate mode or value.
* @retval GT_BAD_VALUE             - due to HW configuration limitation.
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
static GT_STATUS prvCpssDxChNetIfSdmaTxGeneratorRateCalc
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode,
    IN  GT_U64                                      rateValue,
    IN  GT_U32                                      packetsInChain,
    OUT GT_U32                                      *clkTicksPtr,
    OUT GT_U64                                      *actualRateValuePtr
)
{
    GT_U32  localPacketInChain = packetsInChain ;
    GT_U64  actualRateValue;
    GT_U64  tempVal;
    GT_U32  clkFreq;

    clkFreq = (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum)) ?
              _12_5MHZ_CLK_CNS : _25MHZ_CLK_CNS;

    switch(rateMode)
    {
        case CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E:
            /* cannot differentiate if required gap betwwen packets is less */
            /* then 1 clk tick. */
            if( rateValue.l[1] !=0 || rateValue.l[0] > clkFreq )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* division by zero */
            if( rateValue.l[0] == 0 )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            *clkTicksPtr = clkFreq/rateValue.l[0];
            actualRateValue.l[0] = clkFreq/(*clkTicksPtr);
            actualRateValue.l[1] = 0;
            *actualRateValuePtr = actualRateValue;
            break;

        case CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E:
            /* if there are no packets yet in the queue assume 1 packet to */
            /* intialize the configuration */
            if( 0 == localPacketInChain )
            {
                localPacketInChain = 1;
            }

            /* division by 1,000,000,000 since rate value in ns */
            tempVal = prvCpssMathMul64(
                        (BIT_30-1),
                        (localPacketInChain*(NSEC_PER_SEC_CNS/clkFreq)));
            if( (tempVal.l[1] < rateValue.l[1]) ||
                ((tempVal.l[1] == rateValue.l[1]) &&
                 (tempVal.l[0]  < rateValue.l[0])) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }

            tempVal = prvCpssMathDiv64By16(rateValue,
                                           (NSEC_PER_SEC_CNS/clkFreq));
            tempVal = prvCpssMathDiv64By16(tempVal,
                                           (GT_U16)localPacketInChain);

            *clkTicksPtr = tempVal.l[0];

            if(*clkTicksPtr == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
            }

            tempVal = prvCpssMathMul64(
                        (*clkTicksPtr),
                        (localPacketInChain*(NSEC_PER_SEC_CNS/clkFreq)));

            *actualRateValuePtr = tempVal;
            break;

        case CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E:
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfSdmaTxGeneratorHwRateSet function
* @endinternal
*
* @brief   This function sets Tx SDMA Generator HW related parameters for packet
*         interval.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupId              - Port Group ID.
* @param[in] txQueue                  - The Tx SDMA queue.
* @param[in] rateMode                 - transmission rate mode.
* @param[in] clkTicks                 - clock ticks to configure into HW (actual value).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad rate mode.
* @retval GT_FAIL                  - on hardware error
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
static GT_STATUS prvCpssDxChNetIfSdmaTxGeneratorHwRateSet
(
    IN  GT_U8                                       devNum,
    IN  GT_U8                                       txQueue,
    IN  CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode,
    IN  GT_U32                                      clkTicks
)
{
    GT_STATUS   rc;
    PRV_QUEUE_TIMER_CFG_STC timerCfg;

    /* Set packet interval enable - bit 1 & packet interval value - bits[31:2]*/
    switch(rateMode)
    {
        case CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E:
        case CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E:
            timerCfg.enabled = 1;
            timerCfg.clkVal = clkTicks;
            if (clkTicks == 0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            break;

        case CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E:
            timerCfg.enabled = 0;
            timerCfg.clkVal = 0;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    rc = prvQueueTimerSet(devNum, txQueue, &timerCfg);
    return rc;
}

/**
* @internal prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet function
* @endinternal
*
* @brief   This function calculated DSA tag bytes length and the packet data offset
*         based on tagging existence.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set into
*                                      the packet descriptors.
*
* @param[out] dsaTagNumOfBytesPtr      - (pointer to) DSA tag length in bytes.
* @param[out] dataOffsetPtr            - (pointer to) packet data offset.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - on wrong DSA tag type
*/
GT_STATUS prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet
(
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    OUT GT_U32                      *dsaTagNumOfBytesPtr,
    OUT GT_U32                      *dataOffsetPtr
)
{
    switch(packetParamsPtr->dsaParam.commonParams.dsaTagType)
    {
        case CPSS_DXCH_NET_DSA_1_WORD_TYPE_ENT:
            *dsaTagNumOfBytesPtr = 4;
            break;
        case CPSS_DXCH_NET_DSA_2_WORD_TYPE_ENT:
            *dsaTagNumOfBytesPtr = 8;
            break;
        case CPSS_DXCH_NET_DSA_4_WORD_TYPE_ENT:
            *dsaTagNumOfBytesPtr = 16;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Get the Data after Source MAC Address and Destination MAC Address */
    if( packetParamsPtr->packetIsTagged == GT_TRUE )
    {
        *dataOffsetPtr = PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS + /*12*/
                         PRV_CPSS_DXCH_ETHPRT_TAGGED_PCKT_FLAG_LEN_CNS;/* 4 */
    }
    else
    {
        *dataOffsetPtr = PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS; /*12*/
    }

    return GT_OK;
}

/**
* @internal dxChNetIfSdmaTxPacketSendFromSram function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from the different
*         core Tx functions. Prepares them for the transmit operation, and
*         enqueues the prepared descriptors to the PP's tx queues. -- SDMA relate
*         function activates Tx SDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      AC5X; Ironman; Falcon.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5P; Harrier; AAS.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufsPtr             - Length of buffList.
* @param[in] synchronicSend           - sent the packets synchronic (not return until the
*                                      buffers can be free)
*                                      GT_TRUE - send synchronic
*                                      GT_FALSE - send asynchronous (not wait for buffers to be
*                                      free)
* @param[in,out] txDescListPtr            - The Tx desc. list control structure
* @param[in,out] txDescListPtr            - The Tx desc. list control structure
*
* @param[out] txDescListUpdatedPtr     - (pointer to) a parameter indicating if the
*                                      txDescList was updated and data structure need
*                                      to be restored
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_ABORTED               - if command aborted (during shutDown operation)
* @retval GT_HW_ERROR              - when synchronicSend = GT_TRUE and after transmission
*                                       the last descriptor own bit wasn't changed for long time.
* @retval GT_BAD_PARAM             - the data buffer is longer than allowed.
*                                       buffer data can occupied up to the maximum
*                                       number of descriptors defined or queue assigned for
*                                       Packet Generator, unavailable for regular transmission.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
*
* @note 1. Each buffer should be at least 8 bytes long.
*       2. first buffer must be at least 24 bytes for tagged packet,
*          20 for untagged packet.
*       3. support both :
*          ===== regular DSA tag ====
*          ===== extended DSA tag ====
*       4. Packet's length should include 4 bytes for CRC.
*
*/
static GT_STATUS dxChNetIfSdmaTxPacketSendFromSram
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC      *pcktParamsPtr,
    IN GT_U8                            *buffList[],
    IN GT_U32                            buffLenList[],
    INOUT GT_U32                         *numOfBufsPtr,
    IN GT_BOOL                           synchronicSend,
    INOUT PRV_CPSS_TX_DESC_LIST_STC     *txDescListPtr,
    OUT GT_BOOL                         *txDescListUpdatedPtr
)
{
    GT_STATUS rc;
    GT_U8 txQueue;              /* The tx queue to send this packet to. */
    PRV_CPSS_TX_DESC_STC *firstDesc;  /* The first descriptor of the          */
                                /* currently transmitted packet.        */
    PRV_CPSS_SW_TX_DESC_STC  *currSwDesc;  /* The current handled descriptor of*/
                                /* the currently transmitted packet.    */
    PRV_CPSS_TX_DESC_STC  tmpTxDesc;  /* Temporary Tx descriptor used for     */
                                /* preparing the real descriptor data.  */
    PRV_CPSS_TX_DESC_STC  tmpFirstTxDesc; /* Temporary Tx descriptor used for */
                                /* preparing the real first descriptor  */
                                /* data.                                */
    PRV_CPSS_TX_DESC_STC  tmpTxSramDesc;  /* Temporary Tx descriptor used to     */
                                        /* read back real descriptor data.  */

    GT_U32      descWord1;          /* The first word of the Tx descriptor. */
    GT_UINTPTR  tmpBuffPtr;         /* Holds the real buffer pointer.       */
    GT_U32      txCmdMask;          /* Mask for Tx command register.        */
    GT_U32      i;
    GT_U8       numOfAddedBuffs;
    GT_U32      dataOffset = 0;
    GT_U32      dsaTagNumOfBytes = 0; /* extended DSA tag size */
    GT_U32      numOfBufs, netIfNum;    /* The network interface number */
    GT_U32      packetLen = 0;  /* packet length including the DSA tag, */
                                /* not including padding if needed */
    GT_U32      byteBlockMask;  /* mask used to calculate if padding needed due to */
                                /* troublesome packet length */
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/
    GT_U32      buffLen;    /* length of current buffer */
    GT_U32      buff0Len;   /* length of 0-th buffer */

    GT_U32 sramTxBuffOffset = 0; /* SRAM offset at which TX buffers start */
    GT_U8 shortBufferSize;  /* Size of short buffer */
    GT_U8 tmpShortBuffer[TX_SHORT_BUFF_SIZE_16_CNS]; /* Temp short buffer */
    GT_U8 *tmpSramBuffer;   /* Temp buffer used to handle our of bound access */
    GT_U32 wordCount;       /* No of words to read or write */

    /* must be first line before any chance that function may return
      indicate that the descriptors not changed yet */
    *txDescListUpdatedPtr = GT_FALSE;

    /* Set to 0, in the function body will be updated to real number of used buffers. */
    numOfBufs = *numOfBufsPtr;
    *numOfBufsPtr = 0;
    buff0Len = buffLenList[0];

    /* Set tx parameters */
    txQueue     = pcktParamsPtr->sdmaInfo.txQueue;
    netIfNum    = txQueue>>3;
    PRV_CPSS_DXCH_DEV_TX_SDMA_QUEUE_NUM_CHECK_MAC(txQueue);

    /************************************/
    /* convert netIfNum to mgUnitId     */
    /************************************/
    PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

    /* Queue is assigned for Packet Generator */
    if (prvQueueIsTxGenerator(devNum, txQueue) == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The (global) queue [%d] is associated with 'Packet Generator'",txQueue);
    }

    rc = prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet(pcktParamsPtr,
                                                         &dsaTagNumOfBytes,
                                                         &dataOffset);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(buff0Len <= dataOffset/*mac SA+DA+(optional)vlanTag*/)
    {
        if(buff0Len < dataOffset/*mac SA+DA*/)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "too few bytes in first buffer [%d] need to be at least [%d] for packetIsTagged = [%s]",
                buff0Len, dataOffset,
                pcktParamsPtr->packetIsTagged ? "true":"false");
        }

        /* the MAC SA+DA already split from the rest of the packet */
        /* so need only single buffer for the DSA */
        numOfAddedBuffs = 1;
    }
    else
    {
        /* Number of buffers added - 1 for the DSA Tag, 1 for split data after */
        /* the Source MAC Address and Destination MAC Address */
        numOfAddedBuffs = 2;
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E))
    {
        /* calc packet length */
        packetLen = dsaTagNumOfBytes + PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS - dataOffset;
        for( i = 0 ; i < numOfBufs ; i++ )
        {
            packetLen += buffLenList[i];
        }

        /* add 8 byte padding (another buffer is added, numOfAddedBuffs = 3) should */
        /* be done only if packets length is <byteblock>*n+k (1<=k<=8) (including   */
        /* DSA tag), where n and k are integers and <byteblock> is 256 for xCat */
        /* and 512 for DxCh                                 */
        /* otherwise (numOfAddedBuffs = 2) and previous flow is preserved.          */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            byteBlockMask = 0x1FF;
        }
        else /* xCat A1 */
        {
            byteBlockMask = 0xFF;
        }

        if( (1 <= (packetLen & byteBlockMask)) && (8 >= (packetLen & byteBlockMask)) )
        {
            numOfAddedBuffs = 3;
        }
    }



    if((numOfBufs + numOfAddedBuffs) > txDescListPtr->maxDescNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check available buffers in pool */
    if (cpssBmPoolBufFreeCntGet(txDescListPtr->poolId) < 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* Check if there are enough descriptors for this packet. */
    if((numOfBufs + numOfAddedBuffs) > txDescListPtr->freeDescNum)
    {
        if(PRV_CPSS_PP_MAC(devNum)->useIsr == GT_FALSE)
        {
            /* the device was NOT bound to the ISR ,
               and the EVENTs are not handled by the cpss */

            /* return 'Empty' to tell the application , to take care of the
               descriptors , because we have not enough descriptor to do the 'TX'
            */
            /* note : we must not call the
                prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle , because this
                function wants to use the EVENTs mechanism of the CPSS, by
                releasing the descriptors but also sending the 'cookie' info to
                the events queue handling
            */

            /* Application should call function cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
               to release the descriptors ,and get the 'cookie' indication */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, LOG_ERROR_NO_MSG);
        }
        else /* the device was bound to the ISR */
        {
            /* Call prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle to free buffers.
            Must be under interrupt disabled */
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle(devNum, txQueue);
            PRV_CPSS_INT_SCAN_UNLOCK();
            if(rc != GT_OK)
                return rc;
            /* Check again if there are enough descriptors for this packet. */
            if((numOfBufs + numOfAddedBuffs) > txDescListPtr->freeDescNum)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, LOG_ERROR_NO_MSG);
        }
    }

    /* the descriptors start to change here */
    *txDescListUpdatedPtr = GT_TRUE;

    /* Set descriptor parameters */
    currSwDesc              = txDescListPtr->next2Feed;
    firstDesc               = (PRV_CPSS_TX_DESC_STC *)((GT_UINTPTR)currSwDesc->sramTxDescOffset);
    /* Set the cookie in the userData - usually the packet pointer received by
       the application */
    currSwDesc->userData = pcktParamsPtr->cookie;
    currSwDesc->evReqHndl= pcktParamsPtr->sdmaInfo.evReqHndl;

    /***********************************************/
    /* start with the original first buffer's data */
    /***********************************************/
    descWord1 = 0;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* all packet must be 'recalc' ! and it comes from the 'per port' configuration. */
        /* done during : prvCpssDxChPortMappingRxDMA2LocalPhysSet(...) */
        /* here we MUST not set this bit , because it is connected in the design to 'CRC error' indication */
        /* JIRA : SWCNM-46 : wrong connection of recalc_crc to bad_crc (desc_out[14]) */
    }
    else
    if(pcktParamsPtr->sdmaInfo.recalcCrc == GT_TRUE)
    {
        descWord1 |= (1 << 12);
    }

    /* Set the first descriptor parameters. */
    TX_DESC_RESET_MAC(&tmpFirstTxDesc);

    tmpFirstTxDesc.word1 = descWord1;

    /* Set bit for first buffer of a frame for Temporary Tx descriptor */
    TX_DESC_SET_FIRST_BIT_MAC(&tmpFirstTxDesc,1);
    /* Add Source MAC Address and Destination MAC Address */
    TX_DESC_SET_BYTE_CNT_MAC(&tmpFirstTxDesc,PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS); /*12*/

    sramTxBuffOffset = txDescListPtr->sramTxBuffOffset;

    /* Checking if the space left in CnM SRAM is enough to store the buffer content */
    if(sramTxBuffOffset + PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS > txDescListPtr->sramTxBuffEnd)
    {
        txDescListPtr->sramTxBuffOffset = sramTxBuffOffset = txDescListPtr->sramTxBuffStart;
    }

    wordCount = PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS/sizeof(GT_U32);
    rc = prvCpssHwPpWriteRam(devNum, sramTxBuffOffset, wordCount, (GT_U32 *)buffList[0]);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    tmpBuffPtr = (GT_UINTPTR)sramTxBuffOffset;
    txDescListPtr->sramTxBuffOffset = (sramTxBuffOffset += PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS);

    tmpFirstTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

    /***************/
    /* Add DSA Tag */
    /***************/
    currSwDesc = currSwDesc->swNextDesc;

    TX_DESC_RESET_MAC(&tmpTxDesc);
    tmpTxDesc.word1 = descWord1;

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum)==GT_TRUE)
    {
        shortBufferSize = TX_SHORT_BUFF_SIZE_16_CNS;
    }
    else
    {
        shortBufferSize = TX_SHORT_BUFF_SIZE;
    }

    cpssOsMemSet(tmpShortBuffer, 0, sizeof(tmpShortBuffer));

    /* Use Short buffer in this case*/
    /* Build the DSA Tag info into the short buffer of the descriptor */
    rc = cpssDxChNetIfDsaTagBuild(devNum,
                                    &pcktParamsPtr->dsaParam,
                                    tmpShortBuffer);
    if(rc != GT_OK)
    {
        return rc;
    }

    wordCount = shortBufferSize/sizeof(GT_U32);
    rc = prvCpssHwPpWriteRam(devNum, (GT_U32)((GT_UINTPTR)currSwDesc->shortBuffer), wordCount, (GT_U32 *)tmpShortBuffer);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    tmpBuffPtr = (GT_UINTPTR)currSwDesc->shortBuffer;

    tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

    currSwDesc->userData = pcktParamsPtr->cookie;
    /* Set byte counter to DSA Tag size */
    TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc, dsaTagNumOfBytes);
    /* Set ownership bit - owned by DMA */
    TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

    tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
    tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);

    wordCount = 3;  /* Size of word1, word2 and buffPointer */
    rc = prvCpssHwPpWriteRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
    rc = prvCpssHwPpReadRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxSramDesc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    cpssOsMemCpy(currSwDesc->txDesc, &tmpTxSramDesc, sizeof(PRV_CPSS_TX_DESC_STC));

    /**************************************************/
    /* continue with the original first buffer's data */
    /**************************************************/
    if(buff0Len > dataOffset)
    {
        currSwDesc = currSwDesc->swNextDesc;

        TX_DESC_RESET_MAC(&tmpTxDesc);
        tmpTxDesc.word1 = descWord1;

        /*
         * prvCpssHwPpWriteRam writes 4 bytes (word) at a time and the number of
         * bytes to be written may not be a multiple of word. Hence allocating
         * memory that's multiple of word in size, memset the buffer to 0, copy
         * content into this buffer and passing it to prvCpssHwPpWriteRam
         *
         * This helps avoid out of bound access
         */
        tmpSramBuffer = (GT_U8 *)cpssOsMalloc(buff0Len);
        if(tmpSramBuffer == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        cpssOsMemSet(tmpSramBuffer, 0, buff0Len);
        cpssOsMemCpy(tmpSramBuffer, (buffList[0] + dataOffset), (buff0Len - dataOffset));

        wordCount = (buff0Len - dataOffset)/sizeof(GT_U32);
        if((buff0Len - dataOffset) % sizeof(GT_U32))
            wordCount++;

        /* Checking if the space left in CnM SRAM is enough to store the buffer content */
        if(sramTxBuffOffset + (wordCount * sizeof(GT_U32)) > txDescListPtr->sramTxBuffEnd)
        {
            txDescListPtr->sramTxBuffOffset = sramTxBuffOffset = txDescListPtr->sramTxBuffStart;
        }

        rc = prvCpssHwPpWriteRam(devNum, sramTxBuffOffset, wordCount, (GT_U32 *)(tmpSramBuffer));
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        cpssOsFree(tmpSramBuffer);

        tmpBuffPtr = (GT_UINTPTR)sramTxBuffOffset;
        txDescListPtr->sramTxBuffOffset = (sramTxBuffOffset += (wordCount * sizeof(GT_U32)));

        tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

        currSwDesc->userData = pcktParamsPtr->cookie;
        /* Set byte counter to buffer size minus first descriptor */
        TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc, (buff0Len - dataOffset ));
        /* Set ownership bit - owned by DMA */
        TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);
    }

    /* in case last descriptor don't swap
       need to set: Enable Interrupt bit, Last buffer of frame bit */
    if(numOfBufs != 1)
    {
        if(buff0Len > dataOffset)
        {
            tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
            tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);

            wordCount = 3;  /* Size of word1, word2 and buffPointer */
            rc = prvCpssHwPpWriteRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
            rc = prvCpssHwPpReadRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxSramDesc);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(currSwDesc->txDesc, &tmpTxSramDesc, sizeof(PRV_CPSS_TX_DESC_STC));
        }

        /*******************************************/
        /* continue with the original next buffers */
        /*******************************************/


        /* Add rest of the buffers (starting from second buffer) */
        for(i = 1; i < numOfBufs; i++)
        {
            currSwDesc = currSwDesc->swNextDesc;

            TX_DESC_RESET_MAC(&tmpTxDesc);
            tmpTxDesc.word1 = descWord1;

            buffLen = buffLenList[i];
            tmpSramBuffer = (GT_U8 *)cpssOsMalloc(buffLen);
            if(tmpSramBuffer == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemSet(tmpSramBuffer, 0, buffLen);
            cpssOsMemCpy(tmpSramBuffer, buffList[i], buffLen);

            /* Check if the buffers length is larger than (TX_SHORT_BUFF_SIZE)  */
            if(((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) &&
                (buffLen > TX_SHORT_BUFF_SIZE_16_CNS)) ||
                ((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE) &&
                 (buffLen > TX_SHORT_BUFF_SIZE)))
            {
                wordCount = buffLen/sizeof(GT_U32);
                if(buffLen % sizeof(GT_U32))
                    wordCount++;

                /* Checking if the space left in CnM SRAM is enough to store the buffer content */
                if(sramTxBuffOffset + (wordCount * sizeof(GT_U32)) > txDescListPtr->sramTxBuffEnd)
                {
                    txDescListPtr->sramTxBuffOffset = sramTxBuffOffset = txDescListPtr->sramTxBuffStart;
                }

                rc = prvCpssHwPpWriteRam(devNum, sramTxBuffOffset, wordCount, (GT_U32 *)tmpSramBuffer);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                tmpBuffPtr = (GT_UINTPTR)sramTxBuffOffset;
                txDescListPtr->sramTxBuffOffset = (sramTxBuffOffset += (wordCount * sizeof(GT_U32)));

            }
            else
            {
                wordCount = buffLen/sizeof(GT_U32);
                if(buffLen % sizeof(GT_U32))
                    wordCount++;

                rc = prvCpssHwPpWriteRam(devNum, (GT_U32)((GT_UINTPTR)currSwDesc->shortBuffer), wordCount, (GT_U32 *)tmpSramBuffer);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }

                tmpBuffPtr = (GT_UINTPTR)currSwDesc->shortBuffer;
            }

            cpssOsFree(tmpSramBuffer);

            tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);
            currSwDesc->userData = pcktParamsPtr->cookie;
            TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc,buffLen);
            TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

            /* in case last descriptor don't swap  */
            /* need to set: Enable Interrupt bit, Last buffer of frame bit */
            /* In case PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E is */
            /* to be done (3 == numOfAddedBuffs) the last buffer in the */
            /* received buffer list is not the last one.                */
            if((i != (numOfBufs - 1)) || (3 == numOfAddedBuffs) )
            {
                tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
                tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
                wordCount = 3;  /* Size of word1, word2 and buffPointer */
                rc = prvCpssHwPpWriteRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
                rc = prvCpssHwPpReadRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxSramDesc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                cpssOsMemCpy(currSwDesc->txDesc, &tmpTxSramDesc, sizeof(PRV_CPSS_TX_DESC_STC));
            }
        }
    }

    /* padded 8 bytes for PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E */
    if( 3 == numOfAddedBuffs )
    {
        if(numOfBufs == 1)
        {
            /* swapping the previous descriptor in case only one buffer was */
            /* supplied to the routine */
            tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
            tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);

            wordCount = 3;  /* Size of word1, word2 and buffPointer */
            rc = prvCpssHwPpWriteRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
            rc = prvCpssHwPpReadRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxSramDesc);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(currSwDesc->txDesc, &tmpTxSramDesc, sizeof(PRV_CPSS_TX_DESC_STC));
        }

        currSwDesc = currSwDesc->swNextDesc;

        TX_DESC_RESET_MAC(&tmpTxDesc);
        tmpTxDesc.word1 = descWord1;

        /* padding with zeros */
        cpssOsMemSet(tmpShortBuffer, 0, sizeof(tmpShortBuffer));

        wordCount = 2;
        rc = prvCpssHwPpWriteRam(devNum, (GT_U32)((GT_UINTPTR)currSwDesc->shortBuffer), wordCount, (GT_U32 *)tmpShortBuffer);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        tmpBuffPtr = (GT_UINTPTR)currSwDesc->shortBuffer;

        tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);
        currSwDesc->userData = pcktParamsPtr->cookie;
        TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc, 8);
        TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

        /* last descriptor don't swap */
    }

    /**************************************/
    /* Set the last descriptor parameters */
    /**************************************/
    /* Set the bit for Last buffer of a frame */
    TX_DESC_SET_LAST_BIT_MAC(&tmpTxDesc,1);

    /* check if caller want to get the event invocation ,
       NOTE : we must not invoke when sending "synchronic" */
    if((pcktParamsPtr->sdmaInfo.invokeTxBufferQueueEvent == GT_TRUE) &&
       (synchronicSend == GT_FALSE))
    {
        /* Set the bit for Enable Interrupt */
        TX_DESC_SET_INT_BIT_MAC(&tmpTxDesc,1);
    }
    else
        /* Set the bit for Disable Interrupt */
        TX_DESC_SET_INT_BIT_MAC(&tmpTxDesc,0);

    tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
    tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);

    wordCount = 3;  /* Size of word1, word2 and buffPointer */
    rc = prvCpssHwPpWriteRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
    rc = prvCpssHwPpReadRam(devNum, currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxSramDesc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    cpssOsMemCpy(currSwDesc->txDesc, &tmpTxSramDesc, sizeof(PRV_CPSS_TX_DESC_STC));

    /* protect modification of freeDescNum to avoid
     * race condition with prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle()
     * running in interrupt scan thread */
    PRV_CPSS_INT_SCAN_LOCK();
    txDescListPtr->freeDescNum -= (numOfBufs + numOfAddedBuffs);
    txDescListPtr->next2Feed    = currSwDesc->swNextDesc;
    txDescListPtr->currSwDesc   = currSwDesc;
    PRV_CPSS_INT_SCAN_UNLOCK();
    /* Set the sctual number of used buffers. */
    *numOfBufsPtr = numOfBufs + numOfAddedBuffs;

    /* Set the first descriptor own bit to start transmitting.  */
    TX_DESC_SET_OWN_BIT_MAC(&tmpFirstTxDesc,TX_DESC_DMA_OWN);
    tmpFirstTxDesc.word1  = CPSS_32BIT_LE(tmpFirstTxDesc.word1);
    tmpFirstTxDesc.word2  = CPSS_32BIT_LE(tmpFirstTxDesc.word2);

    wordCount = 3;  /* Size of word1, word2 and GT_UINTPTR */
    rc = prvCpssHwPpWriteRam(devNum, (GT_U32)((GT_UINTPTR)firstDesc), wordCount, (GT_U32 *)&tmpFirstTxDesc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
    rc = prvCpssHwPpReadRam(devNum, (GT_U32)((GT_UINTPTR)firstDesc), wordCount, (GT_U32 *)&tmpTxSramDesc);
    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }
    cpssOsMemCpy(currSwDesc->txDesc, &tmpTxSramDesc, sizeof(PRV_CPSS_TX_DESC_STC));

    /* The Enable DMA operation should be done only */
    /* AFTER all desc. operations where completed.  */
    GT_SYNC;

    if(pcktParamsPtr->sdmaInfo.bypassQueueEnable == GT_FALSE)
    {
        /* Enable the Tx DMA.   */
        txCmdMask = 1 << (_LQ_MAC(txQueue));
        rc = prvCpssHwPpMgWriteReg(devNum, mgUnitId, PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txQCmdReg, txCmdMask);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(synchronicSend == GT_TRUE)
    {
        GT_U32 loopIndex;
        GT_U32 doSleep;
        GT_U32 sleepTime;/*time to sleep , to allow the 'Asic simulation' process the packet */
        if(0 == CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
        {
            /* DMA procedure may take a long time for Jumbo packets.
              E.g. the TX of 10240 Byte packet may take 411 micro seconds for PEX Gen 1.1.
              PEX Gen 1.1 haz 250 MBytes per second maximal throughput. The TX of
              10240 bytes minimal time will take:
               (10240 + 16 (eDSA size) + 16 (descriptor size)) / 250 M = 41.088 microseconds
              The ARM v7 CPU with 1.3 Giga Core Clock and DDR3-1333 performs
              approximately 15 millions per second loops of OWN Bit check algorithm.
              It's required 41.088 * 15 = 613 loops for ARM CPU and maximal PEX 1.1 throughput.
              Choose approx 500 times more loops to be on safe side for high end CPUs - 312000.
              The "numOfBufs + numOfAddedBuffs"  is 3 or more.
              Use constant 104000 to get 312000 loop count.*/
            loopIndex = (numOfBufs + numOfAddedBuffs) * 104000;
            doSleep = 0;
        }
        else
        {
            /* ASIM on single image mode should also allow WM more time to handle
               the Tx descriptor from the CPU , before declaring fail */
            doSleep = 1;
            loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
        }

        if(cpssDeviceRunCheck_onEmulator())
        {
            doSleep = 1;
            loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
        }

        /* allow another task to process it if ready , without 1 millisecond penalty */
        /* this code of sleep 0 Vs sleep 1 boost the performance *20 in enhanced-UT !!! */
        sleepTime = 0;

        tryMore_lbl:

#ifndef ASIC_SIMULATION
        if(cpssDeviceRunCheck_onEmulator())
        {
            sleepTime +=5;
        }
#endif /*!ASIC_SIMULATION*/

        /* Wait until PP sent the packet. Or HW error if while block
                   run more than loopIndex times */
        do
        {
            wordCount = 3;  /* Size of word1, word2 and buffPointer */
            rc = prvCpssHwPpReadRam(devNum, (GT_U32)currSwDesc->sramTxDescOffset, wordCount, (GT_U32 *)currSwDesc->txDesc);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }

            if(doSleep)
            {
                /* do some sleep allow the simulation process the packet */
                cpssOsTimerWkAfter(sleepTime);
            }
            loopIndex--;
        } while (loopIndex && (TX_DESC_GET_OWN_BIT_MAC(devNum,currSwDesc->txDesc) == TX_DESC_DMA_OWN));

        if(loopIndex == 0)
        {
            if(doSleep)
            {
                if(sleepTime == 0)/* the TX was not completed ? we need to allow more retries with 'sleep (1)'*/
                {
                    loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
                    sleepTime = 1;
                    goto tryMore_lbl;
                }
            }

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR,
                "CPU send packet : The synchronic send failed to get ownership on the descriptor (timeout passed in which the device not released it)");
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChNetIfSdmaTxGeneratorPacketToBufferSet function
* @endinternal
*
* @brief   This function create the requested packet in the generator buffer,
*         updating the descriptor and adding the DSA tag.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length.
* @param[in] txDesc                   - (pointer to) The descriptor used for that packet.
* @param[in] bufferSize               - the buffer size for the created packet.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong DSA tag type the data buffer is longer than
*                                       allowed.
* @retval GT_FAIL                  - accessed GDMA descriptor has bad/wrong content
*/
static GT_STATUS prvCpssDxChNetIfSdmaTxGeneratorPacketToBufferSet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    IN  PRV_CPSS_TX_DESC_STC        *txDesc,
    IN  GT_U32                      bufferSize
)
{
    GT_STATUS   rc;

    GT_U32      dataOffset = 0;         /* packet data offset due to tagging */
    GT_U32      dsaTagNumOfBytes = 0;   /* DSA tag length */
    GT_U8       *buffAddr;
    GT_U32      pktLen, pktLenDummy;
    /* GDMA related */
    GT_BOOL                 isGdma;
    GT_BOOL                 isValid;

    /* Calculate DSA tag length and packet data offset */
    rc = prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet(packetParamsPtr,
                                                         &dsaTagNumOfBytes,
                                                         &dataOffset);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Starting from Ironman packet should be sent without CRC.
       For Ironman GM the packet sent with CRC as for older devices.   */
    if ((PRV_CPSS_PP_MAC(devNum)->isGmDevice == 0) &&
         PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        packetDataLength -= 4;
    }

    /* Check packet length does not excceed buffer length */
    pktLen = packetDataLength + dsaTagNumOfBytes +
             PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS - dataOffset;

    if (bufferSize < pktLen)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Fill new packet buffer */

    /* For GDMA: fetch Buffer address from the given SGD descriptor */
    isGdma = PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum);
    if (isGdma == GT_TRUE)
    {
        rc = prvCpssDxChNetIfGdmaTxGeneratorDescriptorGet(devNum,
                                    (PRV_CPSS_GDMA_DESC_STC *)txDesc,
                                    &buffAddr,
                                    &pktLenDummy,
                                    &isValid,
                                    NULL);
        if (rc != GT_OK)
            return rc;
    }
    else
    {
        buffAddr = (GT_U8*)txDesc + sizeof(PRV_CPSS_TX_DESC_STC);
    }

    /* Copy Ethernet MACs header */
    cpssOsMemCpy( buffAddr,
                  packetDataPtr,
                  PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS );

    /* Build & Add DSA tag */
    rc = cpssDxChNetIfDsaTagBuild( devNum,
                                   &packetParamsPtr->dsaParam,
                      (buffAddr + PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS) );
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Copy rest of packet data */
    cpssOsMemCpy( buffAddr + PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS + dsaTagNumOfBytes,
                  packetDataPtr + dataOffset,
                  packetDataLength - dataOffset);

    if (isGdma == GT_TRUE)
    {
        rc = prvCpssDxChNetIfGdmaTxGeneratorDescriptorSet(devNum,
                                    (PRV_CPSS_GDMA_DESC_STC *)txDesc,
                                    buffAddr,
                                    pktLen,
                                    GT_TRUE/*set valid*/);
        return rc;
    }

    /* Fill new descriptor */
    TX_DESC_RESET_MAC(txDesc);

    /* Set first and last bit */
    TX_DESC_SET_FIRST_BIT_MAC(txDesc,1);
    TX_DESC_SET_LAST_BIT_MAC(txDesc,1);

    /* Set packet byte count */
    TX_DESC_SET_BYTE_CNT_MAC(txDesc, pktLen);

    /* Set ownership bit - owned by DMA */
    TX_DESC_SET_OWN_BIT_MAC(txDesc,TX_DESC_DMA_OWN);

    txDesc->word1 = CPSS_32BIT_LE(txDesc->word1);
    txDesc->word2 = CPSS_32BIT_LE(txDesc->word2);

    return rc;
}

/**
* @internal dxChNetIfSdmaTxPacketSend function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from the different
*         core Tx functions. Prepares them for the transmit operation, and
*         enqueues the prepared descriptors to the PP's tx queues. -- SDMA relate
*         function activates Tx SDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufsPtr             - Length of buffList.
* @param[in] synchronicSend           - sent the packets synchronic (not return until the
*                                      buffers can be free)
*                                      GT_TRUE - send synchronic
*                                      GT_FALSE - send asynchronous (not wait for buffers to be
*                                      free)
* @param[in,out] txDescListPtr            - The Tx desc. list control structure
* @param[in,out] txDescListPtr            - The Tx desc. list control structure
*
* @param[out] txDescListUpdatedPtr     - (pointer to) a parameter indicating if the
*                                      txDescList was updated and data structure need
*                                      to be restored
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_ABORTED               - if command aborted (during shutDown operation)
* @retval GT_HW_ERROR              - when synchronicSend = GT_TRUE and after transmission
*                                       the last descriptor own bit wasn't changed for long time.
* @retval GT_BAD_PARAM             - the data buffer is longer than allowed.
*                                       buffer data can occupied up to the maximum
*                                       number of descriptors defined or queue assigned for
*                                       Packet Generator, unavailable for regular transmission.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
*
* @note 1. Each buffer should be at least 8 bytes long.
*       2. first buffer must be at least 24 bytes for tagged packet,
*          20 for untagged packet.
*       3. support both :
*          ===== regular DSA tag ====
*          ===== extended DSA tag ====
*       4. Packet's length should include 4 bytes for CRC.
*
*/
static GT_STATUS dxChNetIfSdmaTxPacketSend
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC      *pcktParamsPtr,
    IN GT_U8                            *buffList[],
    IN GT_U32                            buffLenList[],
    INOUT GT_U32                         *numOfBufsPtr,
    IN GT_BOOL                           synchronicSend,
    INOUT PRV_CPSS_TX_DESC_LIST_STC     *txDescListPtr,
    OUT GT_BOOL                         *txDescListUpdatedPtr
)
{
    GT_STATUS rc;
    GT_U8 txQueue;              /* The tx queue to send this packet to. */
    PRV_CPSS_TX_DESC_STC *firstDesc;  /* The first descriptor of the          */
                                /* currently transmitted packet.        */
    PRV_CPSS_SW_TX_DESC_STC  *currSwDesc;  /* The current handled descriptor of*/
                                /* the currently transmitted packet.    */
    PRV_CPSS_TX_DESC_STC  tmpTxDesc;  /* Temporary Tx descriptor used for     */
                                /* preparing the real descriptor data.  */
    PRV_CPSS_TX_DESC_STC  tmpFirstTxDesc; /* Temporary Tx descriptor used for */
                                /* preparing the real first descriptor  */
                                /* data.                                */
    GT_U32      descWord1;          /* The first word of the Tx descriptor. */
    GT_UINTPTR  tmpBuffPtr;         /* Holds the real buffer pointer.       */
    GT_U32      txCmdMask;          /* Mask for Tx command register.        */
    GT_U32      i;
    GT_U8       numOfAddedBuffs;
    GT_U32      dataOffset = 0;
    GT_U32      dsaTagNumOfBytes = 0; /* extended DSA tag size */
    GT_U32      numOfBufs, netIfNum;
    GT_U32      packetLen = 0;  /* packet length including the DSA tag, */
                                /* not including padding if needed */
    GT_U32      byteBlockMask;  /* mask used to calculate if padding needed due to */
                                /* troublesome packet length */
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/
    GT_U32      buffLen;    /* length of current buffer */
    GT_U32      buff0Len;   /* length of 0-th buffer */
    GT_U32      packetLenDec; /* length of packet decrement */

    /* must be first line before any chance that function may return
      indicate that the descriptors not changed yet */
    *txDescListUpdatedPtr = GT_FALSE;

    /* Set to 0, in the function body will be updated to real number of used buffers. */
    numOfBufs = *numOfBufsPtr;
    *numOfBufsPtr = 0;

    /* For Ironman HW and WM packet should be sent without CRC        */
    /* Assumed that the last 4 bytes of packet passed by caller is CRC */
    /* For Ironman GM the packet sent with CRC as for older devices   */
    packetLenDec = 0;
    if (PRV_CPSS_PP_MAC(devNum)->isGmDevice == 0)
    {
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
            packetLenDec = 4; /* size of CRC */
            for (i = numOfBufs; (i > 0); i--)
            {
                if ((buffLenList[i-1] > packetLenDec))
                {
                    break;
                }
                numOfBufs --;
                packetLenDec -= buffLenList[i-1];
            }
            if (numOfBufs == 0)
            {
                DBG_TRACE(("dxChNetIfSdmaTxPacketSend too small packet\n"));
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
    }

    buff0Len = buffLenList[0];
    if ((numOfBufs == 1) && (packetLenDec > 0))
    {
        /* no CRC for Ironman */
        buff0Len -= packetLenDec;
    }

    /* Set tx parameters */
    txQueue     = pcktParamsPtr->sdmaInfo.txQueue;
    netIfNum    = txQueue>>3;
    PRV_CPSS_DXCH_DEV_TX_SDMA_QUEUE_NUM_CHECK_MAC(txQueue);

    /************************************/
    /* convert netIfNum to mgUnitId     */
    /************************************/
    PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

    /* Queue is assigned for Packet Generator */
    if (prvQueueIsTxGenerator(devNum, txQueue) == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "The (global) queue [%d] is associated with 'Packet Generator'",txQueue);
    }

    rc = prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet(pcktParamsPtr,
                                                         &dsaTagNumOfBytes,
                                                         &dataOffset);
    if( GT_OK != rc )
    {
        return rc;
    }

    if(buff0Len <= dataOffset/*mac SA+DA+(optional)vlanTag*/)
    {
        if(buff0Len < dataOffset/*mac SA+DA*/)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,
                "too few bytes in first buffer [%d] need to be at least [%d] for packetIsTagged = [%s]",
                buff0Len, dataOffset,
                pcktParamsPtr->packetIsTagged ? "true":"false");
        }

        /* the MAC SA+DA already split from the rest of the packet */
        /* so need only single buffer for the DSA */
        numOfAddedBuffs = 1;
    }
    else
    {
        /* Number of buffers added - 1 for the DSA Tag, 1 for split data after */
        /* the Source MAC Address and Destination MAC Address */
        numOfAddedBuffs = 2;
    }

    if(GT_TRUE == PRV_CPSS_DXCH_ERRATA_GET_MAC(devNum, PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E))
    {
        /* calc packet length */
        packetLen = dsaTagNumOfBytes + PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS - dataOffset;
        for( i = 0 ; i < numOfBufs ; i++ )
        {
            packetLen += buffLenList[i];
        }

        /* add 8 byte padding (another buffer is added, numOfAddedBuffs = 3) should */
        /* be done only if packets length is <byteblock>*n+k (1<=k<=8) (including   */
        /* DSA tag), where n and k are integers and <byteblock> is 256 for xCat */
        /* and 512 for DxCh                                 */
        /* otherwise (numOfAddedBuffs = 2) and previous flow is preserved.          */
        if((PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E) ||
           (PRV_CPSS_SIP_5_CHECK_MAC(devNum)))
        {
            byteBlockMask = 0x1FF;
        }
        else /* xCat A1 */
        {
            byteBlockMask = 0xFF;
        }

        if( (1 <= (packetLen & byteBlockMask)) && (8 >= (packetLen & byteBlockMask)) )
        {
            numOfAddedBuffs = 3;
        }
    }



    if((numOfBufs + numOfAddedBuffs) > txDescListPtr->maxDescNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* check available buffers in pool */
    if (cpssBmPoolBufFreeCntGet(txDescListPtr->poolId) < 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* Check if there are enough descriptors for this packet. */
    if((numOfBufs + numOfAddedBuffs) > txDescListPtr->freeDescNum)
    {
        if(PRV_CPSS_PP_MAC(devNum)->useIsr == GT_FALSE)
        {
            /* the device was NOT bound to the ISR ,
               and the EVENTs are not handled by the cpss */

            /* return 'Empty' to tell the application , to take care of the
               descriptors , because we have not enough descriptor to do the 'TX'
            */
            /* note : we must not call the
                prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle , because this
                function wants to use the EVENTs mechanism of the CPSS, by
                releasing the descriptors but also sending the 'cookie' info to
                the events queue handling
            */

            /* Application should call function cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
               to release the descriptors ,and get the 'cookie' indication */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, LOG_ERROR_NO_MSG);
        }
        else /* the device was bound to the ISR */
        {
            /* Call prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle to free buffers.
            Must be under interrupt disabled */
            PRV_CPSS_INT_SCAN_LOCK();
            rc = prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle(devNum, txQueue);
            PRV_CPSS_INT_SCAN_UNLOCK();
            if(rc != GT_OK)
                return rc;
            /* Check again if there are enough descriptors for this packet. */
            if((numOfBufs + numOfAddedBuffs) > txDescListPtr->freeDescNum)
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, LOG_ERROR_NO_MSG);
        }
    }

    /* the descriptors start to change here */
    *txDescListUpdatedPtr = GT_TRUE;

    /* Set descriptor parameters */
    currSwDesc              = txDescListPtr->next2Feed;
    firstDesc               = currSwDesc->txDesc;
    /* Set the cookie in the userData - usually the packet pointer received by
       the application */
    currSwDesc->userData = pcktParamsPtr->cookie;
    currSwDesc->evReqHndl= pcktParamsPtr->sdmaInfo.evReqHndl;

    /***********************************************/
    /* start with the original first buffer's data */
    /***********************************************/
    descWord1 = 0;

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* all packet must be 'recalc' ! and it comes from the 'per port' configuration. */
        /* done during : prvCpssDxChPortMappingRxDMA2LocalPhysSet(...) */
        /* here we MUST not set this bit , because it is connected in the design to 'CRC error' indication */
        /* JIRA : SWCNM-46 : wrong connection of recalc_crc to bad_crc (desc_out[14]) */
    }
    else
    if(pcktParamsPtr->sdmaInfo.recalcCrc == GT_TRUE)
    {
        descWord1 |= (1 << 12);
    }

    /* Set the first descriptor parameters. */
    TX_DESC_RESET_MAC(&tmpFirstTxDesc);

    tmpFirstTxDesc.word1 = descWord1;

    /* Set bit for first buffer of a frame for Temporary Tx descriptor */
    TX_DESC_SET_FIRST_BIT_MAC(&tmpFirstTxDesc,1);
    /* Add Source MAC Address and Destination MAC Address */
    TX_DESC_SET_BYTE_CNT_MAC(&tmpFirstTxDesc,PRV_CPSS_DXCH_ETHPRT_HDR_MAC_SIZE_CNS); /*12*/

    /* update the packet header to the first descriptor */
    rc = cpssOsVirt2Phy((GT_UINTPTR)buffList[0],&tmpBuffPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (tmpBuffPtr & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    tmpFirstTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

    /***************/
    /* Add DSA Tag */
    /***************/
    currSwDesc = currSwDesc->swNextDesc;

    TX_DESC_RESET_MAC(&tmpTxDesc);
    tmpTxDesc.word1 = descWord1;

    /* Use Short buffer in this case*/
    /* Build the DSA Tag info into the short buffer of the descriptor */
    rc = cpssDxChNetIfDsaTagBuild(devNum,
                                     &pcktParamsPtr->dsaParam,
                                     &currSwDesc->shortBuffer[0]);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssOsVirt2Phy((GT_UINTPTR)(currSwDesc->shortBuffer),/*OUT*/&tmpBuffPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (tmpBuffPtr & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

    currSwDesc->userData = pcktParamsPtr->cookie;
    /* Set byte counter to DSA Tag size */
    TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc, dsaTagNumOfBytes);
    /* Set ownership bit - owned by DMA */
    TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

    tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
    tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
    TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);

    /**************************************************/
    /* continue with the original first buffer's data */
    /**************************************************/
    if(buff0Len > dataOffset)
    {
        currSwDesc = currSwDesc->swNextDesc;

        TX_DESC_RESET_MAC(&tmpTxDesc);
        tmpTxDesc.word1 = descWord1;

        /* Get the Data */
        rc = cpssOsVirt2Phy((GT_UINTPTR)( ((GT_U8*)buffList[0]) + dataOffset ),/*OUT*/&tmpBuffPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
            if (0 != (tmpBuffPtr & 0xffffffff00000000L))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        #endif

        tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);

        currSwDesc->userData = pcktParamsPtr->cookie;
        /* Set byte counter to buffer size minus first descriptor */
        TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc, (buff0Len - dataOffset ));
        /* Set ownership bit - owned by DMA */
        TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);
    }

    /* in case last descriptor don't swap
       need to set: Enable Interrupt bit, Last buffer of frame bit */
    if(numOfBufs != 1)
    {
        if(buff0Len > dataOffset)
        {
            tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
            tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);

            TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);
        }

        /*******************************************/
        /* continue with the original next buffers */
        /*******************************************/


        /* Add rest of the buffers (starting from second buffer) */
        for(i = 1; i < numOfBufs; i++)
        {
            currSwDesc = currSwDesc->swNextDesc;

            TX_DESC_RESET_MAC(&tmpTxDesc);
            tmpTxDesc.word1 = descWord1;

            buffLen = buffLenList[i];
            if ((numOfBufs == (i + 1)) && (packetLenDec > 0))
            {
                /* no CRC for Ironman */
                buffLen -= packetLenDec;
            }

            /* Check if the buffers length is larger than (TX_SHORT_BUFF_SIZE)  */
            if(((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE) &&
                (buffLen > TX_SHORT_BUFF_SIZE_16_CNS)) ||
                ((PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_FALSE) &&
                 (buffLen > TX_SHORT_BUFF_SIZE)))
            {
                rc = cpssOsVirt2Phy((GT_UINTPTR)(buffList[i]),/*OUT*/&tmpBuffPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
                    if (0 != (tmpBuffPtr & 0xffffffff00000000L))
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }
                #endif

            }
            else
            {
                cpssOsMemCpy(currSwDesc->shortBuffer,buffList[i], buffLen);
                rc = cpssOsVirt2Phy((GT_UINTPTR)(currSwDesc->shortBuffer),&tmpBuffPtr);
                if (rc != GT_OK)
                {
                    return rc;
                }

                #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
                    if (0 != (tmpBuffPtr & 0xffffffff00000000L))
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                    }
                #endif

            }

            tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);
            currSwDesc->userData = pcktParamsPtr->cookie;
            TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc,buffLen);
            TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

            /* in case last descriptor don't swap  */
            /* need to set: Enable Interrupt bit, Last buffer of frame bit */
            /* In case PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E is */
            /* to be done (3 == numOfAddedBuffs) the last buffer in the */
            /* received buffer list is not the last one.                */
            if((i != (numOfBufs - 1)) || (3 == numOfAddedBuffs) )
            {
                tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
                tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
                TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);
            }
        }
    }

    /* padded 8 bytes for PRV_CPSS_DXCH_XCAT_TX_CPU_CORRUPT_BUFFER_WA_E */
    if( 3 == numOfAddedBuffs )
    {
        if(numOfBufs == 1)
        {
            /* swapping the previous descriptor in case only one buffer was */
            /* supplied to the routine */
            tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
            tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
            TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);
        }

        currSwDesc = currSwDesc->swNextDesc;

        TX_DESC_RESET_MAC(&tmpTxDesc);
        tmpTxDesc.word1 = descWord1;

        /* padding with zeros */
        cpssOsMemSet(currSwDesc->shortBuffer, 0x0, 8);

        rc = cpssOsVirt2Phy((GT_UINTPTR)(currSwDesc->shortBuffer),/*OUT*/&tmpBuffPtr);
        if (rc != GT_OK)
        {
            return rc;
        }

        #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
            if (0 != (tmpBuffPtr & 0xffffffff00000000L))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        #endif

        tmpTxDesc.buffPointer = CPSS_32BIT_LE((GT_U32)tmpBuffPtr);
        currSwDesc->userData = pcktParamsPtr->cookie;
        TX_DESC_SET_BYTE_CNT_MAC(&tmpTxDesc, 8);
        TX_DESC_SET_OWN_BIT_MAC(&tmpTxDesc,TX_DESC_DMA_OWN);

        /* last descriptor don't swap */
    }

    /**************************************/
    /* Set the last descriptor parameters */
    /**************************************/
    /* Set the bit for Last buffer of a frame */
    TX_DESC_SET_LAST_BIT_MAC(&tmpTxDesc,1);

    /* check if caller want to get the event invocation ,
       NOTE : we must not invoke when sending "synchronic" */
    if((pcktParamsPtr->sdmaInfo.invokeTxBufferQueueEvent == GT_TRUE) &&
       (synchronicSend == GT_FALSE))
    {
        /* Set the bit for Enable Interrupt */
        TX_DESC_SET_INT_BIT_MAC(&tmpTxDesc,1);
    }
    else
        /* Set the bit for Disable Interrupt */
        TX_DESC_SET_INT_BIT_MAC(&tmpTxDesc,0);

    tmpTxDesc.word1 = CPSS_32BIT_LE(tmpTxDesc.word1);
    tmpTxDesc.word2 = CPSS_32BIT_LE(tmpTxDesc.word2);
    TX_DESC_COPY_MAC(currSwDesc->txDesc,&tmpTxDesc);

    /* protect modification of freeDescNum to avoid
     * race condition with prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle()
     * running in interrupt scan thread */
    PRV_CPSS_INT_SCAN_LOCK();
    txDescListPtr->freeDescNum -= (numOfBufs + numOfAddedBuffs);
    txDescListPtr->next2Feed    = currSwDesc->swNextDesc;
    txDescListPtr->currSwDesc   = currSwDesc;
    PRV_CPSS_INT_SCAN_UNLOCK();
    /* Set the sctual number of used buffers. */
    *numOfBufsPtr = numOfBufs + numOfAddedBuffs;

    /* Set the first descriptor own bit to start transmitting.  */
    TX_DESC_SET_OWN_BIT_MAC(&tmpFirstTxDesc,TX_DESC_DMA_OWN);
    tmpFirstTxDesc.word1  = CPSS_32BIT_LE(tmpFirstTxDesc.word1);
    tmpFirstTxDesc.word2  = CPSS_32BIT_LE(tmpFirstTxDesc.word2);
    TX_FIRST_DESC_COPY_MAC(firstDesc,&tmpFirstTxDesc);

    /* The Enable DMA operation should be done only */
    /* AFTER all desc. operations where completed.  */
    GT_SYNC;

    if(pcktParamsPtr->sdmaInfo.bypassQueueEnable == GT_FALSE)
    {
        /* Enable the Tx DMA.   */
        txCmdMask = 1 << (_LQ_MAC(txQueue));
        rc = prvCpssHwPpMgWriteReg(devNum, mgUnitId, PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txQCmdReg, txCmdMask);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(synchronicSend == GT_TRUE)
    {
        GT_U32 loopIndex;
        GT_U32 doSleep;
        GT_U32 sleepTime;/*time to sleep , to allow the 'Asic simulation' process the packet */
        if(0 == CPSS_DEV_IS_WM_NATIVE_OR_ASIM_MAC(devNum))
        {
            /* DMA procedure may take a long time for Jumbo packets.
              E.g. the TX of 10240 Byte packet may take 411 micro seconds for PEX Gen 1.1.
              PEX Gen 1.1 haz 250 MBytes per second maximal throughput. The TX of
              10240 bytes minimal time will take:
               (10240 + 16 (eDSA size) + 16 (descriptor size)) / 250 M = 41.088 microseconds
              The ARM v7 CPU with 1.3 Giga Core Clock and DDR3-1333 performs
              approximately 15 millions per second loops of OWN Bit check algorithm.
              It's required 41.088 * 15 = 613 loops for ARM CPU and maximal PEX 1.1 throughput.
              Choose approx 500 times more loops to be on safe side for high end CPUs - 312000.
              The "numOfBufs + numOfAddedBuffs"  is 3 or more.
              Use constant 104000 to get 312000 loop count.*/
            loopIndex = (numOfBufs + numOfAddedBuffs) * 104000;
            doSleep = 0;
        }
        else
        {
            /* ASIM on single image mode should also allow WM more time to handle
               the Tx descriptor from the CPU , before declaring fail */
            doSleep = 1;
            loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
        }

        if(cpssDeviceRunCheck_onEmulator())
        {
            doSleep = 1;
            loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
        }

        /* allow another task to process it if ready , without 1 millisecond penalty */
        /* this code of sleep 0 Vs sleep 1 boost the performance *20 in enhanced-UT !!! */
        sleepTime = 0;

        tryMore_lbl:

#ifndef ASIC_SIMULATION
        if(cpssDeviceRunCheck_onEmulator())
        {
            sleepTime +=5;
        }
#endif /*!ASIC_SIMULATION*/

        /* Wait until PP sent the packet. Or HW error if while block
                   run more than loopIndex times */
        while (loopIndex && (TX_DESC_GET_OWN_BIT_MAC(devNum,currSwDesc->txDesc) == TX_DESC_DMA_OWN))
        {
            if(doSleep)
            {
                /* do some sleep allow the simulation process the packet */
                cpssOsTimerWkAfter(sleepTime);
            }
            loopIndex--;
        }

        if(loopIndex == 0)
        {
            if(doSleep)
            {
                if(sleepTime == 0)/* the TX was not completed ? we need to allow more retries with 'sleep (1)'*/
                {
                    loopIndex = (numOfBufs + numOfAddedBuffs) * 500;
                    sleepTime = 1;
                    goto tryMore_lbl;
                }
            }

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR,
                "CPU send packet : The synchronic send failed to get ownership on the descriptor (timeout passed in which the device not released it)");
        }

    }


    return GT_OK;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxPacketSend function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from Application .
*         Prepares them for the transmit operation, and enqueues the prepared
*         descriptors to the PP's tx queues. -- SDMA relate.
*         function activates Tx SDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_BAD_PARAM             - on bad DSA params or the data buffer is longer
*                                       than allowed. Buffer data can occupied up to the
*                                       maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*
* @note 1. Support both regular DSA tag and extended DSA tag.
*       2. Each buffer must be at least 8 bytes long. First buffer must be at
*       least 24 bytes for tagged packet, 20 for untagged packet.
*       3. If returned status is GT_NO_RESOURCE then the application should free
*       the elements in the fifo (associated with the Handle) by calling
*       cpssDxChNetIfTxBufferQueueGet, and send the packet again.
*       4. If returned status is GT_EMPTY and CPSS handling the events
*       of the device then the application should wait and try to send the
*       packet again.
*       5. Packet's length should include 4 bytes for CRC.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    GT_STATUS rc = GT_OK;
    GT_U8                      txQueue;   /*The tx queue to send this packet to*/
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;/*The Tx desc. list control structure*/
    GT_BOOL   txDescListUpdated;/* do we need to restore descriptors -->
                        dummy parameter not used for 'asynch sending'*/

    GT_U32                          netIfNum;   /* The network interface number */
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;  /* TX queue mode of operation*/
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr; /* pointer to the module
                                                configure of the PP's database*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pcktParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(buffList);
    CPSS_NULL_PTR_CHECK_MAC(buffLenList);
    PRV_CPSS_DXCH_SDMA_USED_CHECK_MAC(devNum);

    /* Diamond Cut have not SDMA registers */
    if(!PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        /*check that the DB of networkIf library was initialized*/
        PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

        if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
        {
            /* handle GDMA device */
            /* a-synchronic sending */
            return prvCpssDxChGdmaNetIfTxPacketSend(devNum,
                    pcktParamsPtr, buffList, buffLenList,
                    numOfBufs, GT_FALSE);
        }

        txQueue     = pcktParamsPtr->sdmaInfo.txQueue;
        txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

        moduleCfgPtr = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum);
        netIfNum    = txQueue>>3;
        queueMode = moduleCfgPtr->multiNetIfCfg.txSdmaQueuesConfig[netIfNum][_LQ_MAC(txQueue)].queueMode;

        /* asynchronous sending */
        if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
        {
            rc = dxChNetIfSdmaTxPacketSendFromSram(devNum, pcktParamsPtr, buffList, buffLenList,
                                        &numOfBufs, GT_FALSE, txDescList ,
                                        &txDescListUpdated);
        }
        else
        {
            rc = dxChNetIfSdmaTxPacketSend(devNum, pcktParamsPtr, buffList, buffLenList,
                                        &numOfBufs, GT_FALSE, txDescList ,
                                        &txDescListUpdated);
        }
    }
    else
    {
        rc = prvCpssGenNetIfMiiTxModeSet(PRV_CPSS_GEN_NETIF_MII_TX_MODE_ASYNCH_E);
        if (rc != GT_OK)
            return rc;

        rc = prvCpssDxChNetIfMiiPacketTx(devNum, pcktParamsPtr, buffList,
                                         buffLenList, numOfBufs);
    }

    return rc;
}

/**
* @internal cpssDxChNetIfSdmaTxPacketSend function
* @endinternal
*
* @brief   This function receives packet buffers & parameters from Application .
*         Prepares them for the transmit operation, and enqueues the prepared
*         descriptors to the PP's tx queues. -- SDMA relate.
*         function activates Tx SDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            The internal packet params to be set into the packet
*                                      descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_BAD_PARAM             - on bad DSA params or the data buffer is longer
*                                       than allowed. Buffer data can occupied up to the
*                                       maximum number of descriptors defined.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*
* @note 1. Support both regular DSA tag and extended DSA tag.
*       2. Each buffer must be at least 8 bytes long. First buffer must be at
*       least 24 bytes for tagged packet, 20 for untagged packet.
*       3. If returned status is GT_NO_RESOURCE then the application should free
*       the elements in the fifo (associated with the Handle) by calling
*       cpssDxChNetIfTxBufferQueueGet, and send the packet again.
*       4. If returned status is GT_EMPTY and CPSS handling the events
*       of the device then the application should wait and try to send the
*       packet again.
*       5. Packet's length should include 4 bytes for CRC.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxPacketSend);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs));

    rc = internal_cpssDxChNetIfSdmaTxPacketSend(devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/*******************************************************************************
* internal_cpssDxChNetIfSdmaSyncTxPacketSend
*
*       This function receives packet buffers & parameters from Applications .
*       Prepares them for the transmit operation, and enqueues the prepared
*       descriptors to the PP's tx queues.
*       After transmition end all transmitted packets descriptors are freed.
*        -- SDMA relate.
*       function activates Tx SDMA , function wait for PP to finish processing
*       the buffers.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - Device number.
*       pcktParamsPtr- The internal packet params to be set into the packet
*                      descriptors.
*       buffList     - The packet data buffers list.
*       buffLenList  - A list of the buffers len in buffList.
*       numOfBufs    - Length of buffList.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success, or
*       GT_NO_RESOURCE - if there is not enough free elements in the fifo
*                        associated with the Event Request Handle.
*       GT_EMPTY       - if there are not enough descriptors to do the sending.
*       GT_HW_ERROR    - when after transmission last descriptor own bit wasn't
*                        changed for long time.
*       GT_BAD_PARAM   - on bad DSA params or the data buffer is longer
*                        than allowed. Buffer data can occupied up to the
*                        maximum number of descriptors defined.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_INITIALIZED       - the library was not initialized
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL otherwise.
*
* COMMENTS:
*    1. Support both regular DSA tag and extended DSA tag.
*    2. Each buffer must be at least 8 bytes long. First buffer must be at
*       least 24 bytes for tagged packet, 20 for untagged packet.
*    3. If returned status is GT_NO_RESOURCE then the application should free
*       the elements in the fifo (associated with the Handle) by calling
*       cpssDxChNetIfTxBufferQueueGet, and send the packet again.
*    4. If returned status is GT_EMPTY and CPSS handling the events
*       of the device then the application should wait and try to send the
*       packet again.
*    5. Packet's length should include 4 bytes for CRC.
*
*******************************************************************************/
static GT_STATUS internal_cpssDxChNetIfSdmaSyncTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    GT_STATUS rc0 = GT_OK;
    GT_STATUS rc1 = GT_OK;
    GT_BOOL   txDescListUpdated;/* do we need to restore descriptors */
    GT_U8 txQueue;              /* The tx queue to send this packet to. */
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;   /* The Tx desc. list control structure. */
    PRV_CPSS_SW_TX_DESC_STC    *firstDescPtr;
    GT_U32                      numOfUsedDesc, netIfNum;

    GT_UINTPTR                  phyNext2Feed; /* The physicat address of the next2Feed*/
                                              /* field.                               */
    PRV_CPSS_DXCH_PP_REGS_ADDR_STC *regsAddr;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr; /* pointer to the module
                                                configure of the PP's database*/
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;  /* TX queue mode of operation*/
    GT_U8                           wordCount;  /* No of words to read or write */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(pcktParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(buffList);
    CPSS_NULL_PTR_CHECK_MAC(buffLenList);
    PRV_CPSS_DXCH_SDMA_USED_CHECK_MAC(devNum);

    if(pcktParamsPtr->sdmaInfo.bypassQueueEnable == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Send Later option is not supported in Synchronous mode.");
    }

    /* Diamond Cut have not SDMA registers */
    if(!PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        /*check that the DB of networkIf library was initialized*/
        PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

        if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
        {
            /* handle GDMA device */
            /* synchronic sending */
            return prvCpssDxChGdmaNetIfTxPacketSend(devNum,
                    pcktParamsPtr, buffList, buffLenList,
                    numOfBufs, GT_TRUE);
        }

        txQueue     = pcktParamsPtr->sdmaInfo.txQueue;
        txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

        netIfNum = txQueue>>3;
        moduleCfgPtr = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum);
        queueMode = moduleCfgPtr->multiNetIfCfg.txSdmaQueuesConfig[netIfNum][_LQ_MAC(txQueue)].queueMode;

        /* dxChNetIfSdmaTxPacketSend changes txDescList->next2Feed, keep it here
        and return it back after cleaning. */
        firstDescPtr = txDescList->next2Feed;

        numOfUsedDesc = numOfBufs;

        /* synchronic sending */
        if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
        {
            rc0 =  dxChNetIfSdmaTxPacketSendFromSram(devNum,pcktParamsPtr,buffList,buffLenList,
                                            &numOfUsedDesc, GT_TRUE, txDescList ,
                                            &txDescListUpdated);
        }
        else
        {
            rc0 =  dxChNetIfSdmaTxPacketSend(devNum,pcktParamsPtr,buffList,buffLenList,
                                            &numOfUsedDesc, GT_TRUE, txDescList ,
                                            &txDescListUpdated);
        }

        if (txDescListUpdated == GT_TRUE)
        {
            /* Don't check rc here, need to restore data structure*/

            /* Restore next to feed descriptor. */
            txDescList->next2Feed = firstDescPtr;
            txDescList->freeDescNum += numOfUsedDesc;

            /* Returned the buffers to application. Return the state of descriptors as
             it was before calling internalTxPacketSend. Set txDescList->next2Feed
             to point to the same SW_DESC before calling internalTxPacketSend
             Then return the status to the caller (even in case of error). */
            while(numOfUsedDesc > 0)
            {
                firstDescPtr->txDesc->word1 = 0x0;
                firstDescPtr->txDesc->word2 = 0x0;

                if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
                {
                    wordCount = 2;  /* Size of word1 and word2 */
                    rc1 = prvCpssHwPpWriteRam(devNum, firstDescPtr->sramTxDescOffset, wordCount, (GT_U32 *)firstDescPtr->txDesc);
                    if(rc1 != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc1, LOG_ERROR_NO_MSG);
                    }
                }

                firstDescPtr = firstDescPtr->swNextDesc;
                numOfUsedDesc--;
            }

            /* Set/Restore Tx Current Desc Pointer Register. */
            regsAddr    = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum);

            if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
            {
                phyNext2Feed = (GT_UINTPTR)txDescList->next2Feed->sramTxDescOffset;
            }
            else
            {
                rc1 = cpssOsVirt2Phy((GT_UINTPTR)(txDescList->next2Feed->txDesc),/*OUT*/&phyNext2Feed);
                if (rc1 != GT_OK)
                {
                    return rc1;
                }
            }

            #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
                if (0 != (phyNext2Feed & 0xffffffff00000000L))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
            #endif

            /************************************/
            /* convert netIfNum to mgUnitId     */
            /************************************/
            PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

            rc1 = prvCpssHwPpMgWriteReg(
                devNum,  mgUnitId,
                regsAddr->sdmaRegs.txDmaCdp[_LQ_MAC(txQueue)], (GT_U32)phyNext2Feed);

            if(rc1 != GT_OK)
                return rc1;
        }
    }
    else
    {

        rc0 = prvCpssGenNetIfMiiTxModeSet(PRV_CPSS_GEN_NETIF_MII_TX_MODE_SYNCH_E);
        if (rc0 != GT_OK)
            return rc0;

        rc0 = prvCpssDxChNetIfMiiPacketTx(devNum, pcktParamsPtr, buffList,
                                         buffLenList, numOfBufs);
    }

    return rc0;
}

/*******************************************************************************
* @internal cpssDxChNetIfSdmaSyncTxPacketSend function
* @endinternal
*
* @brief This function receives packet buffers & parameters from Applications .
*       Prepares them for the transmit operation, and enqueues the prepared
*       descriptors to the PP's tx queues.
*       After transmition end all transmitted packets descriptors are freed.
*        -- SDMA relate.
*       function activates Tx SDMA , function wait for PP to finish processing
*       the buffers.
*
* APPLICABLE DEVICES:
*        xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum       - Device number.
*       pcktParamsPtr- The internal packet params to be set into the packet
*                      descriptors.
*       buffList     - The packet data buffers list.
*       buffLenList  - A list of the buffers len in buffList.
*       numOfBufs    - Length of buffList.
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK          - on success, or
*       GT_NO_RESOURCE - if there is not enough free elements in the fifo
*                        associated with the Event Request Handle.
*       GT_EMPTY       - if there are not enough descriptors to do the sending.
*       GT_HW_ERROR    - when after transmission last descriptor own bit wasn't
*                        changed for long time.
*       GT_BAD_PARAM   - on bad DSA params or the data buffer is longer
*                        than allowed. Buffer data can occupied up to the
*                        maximum number of descriptors defined.
*       GT_BAD_PTR               - one of the parameters is NULL pointer
*       GT_NOT_INITIALIZED       - the library was not initialized
*       GT_NOT_APPLICABLE_DEVICE - on not applicable device
*       GT_FAIL otherwise.
*
* COMMENTS:
*    1. Support both regular DSA tag and extended DSA tag.
*    2. Each buffer must be at least 8 bytes long. First buffer must be at
*       least 24 bytes for tagged packet, 20 for untagged packet.
*    3. If returned status is GT_NO_RESOURCE then the application should free
*       the elements in the fifo (associated with the Handle) by calling
*       cpssDxChNetIfTxBufferQueueGet, and send the packet again.
*    4. If returned status is GT_EMPTY and CPSS handling the events
*       of the device then the application should wait and try to send the
*       packet again.
*    5. Packet's length should include 4 bytes for CRC.
*
*******************************************************************************/
GT_STATUS cpssDxChNetIfSdmaSyncTxPacketSend
(
    IN GT_U8                                    devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC              *pcktParamsPtr,
    IN GT_U8                                    *buffList[],
    IN GT_U32                                   buffLenList[],
    IN GT_U32                                   numOfBufs
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaSyncTxPacketSend);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs));

    rc = internal_cpssDxChNetIfSdmaSyncTxPacketSend(devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, pcktParamsPtr, buffList, buffLenList, numOfBufs));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle function
* @endinternal
*
* @brief   This routine frees all transmitted packets descriptors. In addition, all
*         user relevant data in Tx End FIFO. -- SDMA relate.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - The device number the packet was transmitted from
* @param[in] queueIdx              - The global queue index.
*
* @retval GT_OK on success, or
*
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - the CPSS does not handle the ISR for the device
*                                       so function must not be called
*                                       GT_FAIL otherwise.
*
* @note Invoked by ISR routine !!
*       the function is called internally in the CPSS from the driver as a call
*       back , see bind in function hwPpPhase1Part1(...)
*
*/
GT_STATUS prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle
(
    IN GT_U8                devNum,
    IN GT_U8                queueIdx
)
{
    PRV_CPSS_TX_DESC_LIST_STC       *txDescListPtr;
    /* Counts the number of descriptors already freed. */
    GT_U32                          freeDescNum;
    PRV_CPSS_SW_TX_DESC_STC         *descPtr;
    /* Points to the first sw desc. to be sent to the callback function.*/
    PRV_CPSS_SW_TX_DESC_STC         *firstTxDescPtr;
    /* Number of descriptors this packet occupies to be sent to the callback
       function.*/
    GT_U32                          descNum;
    PRV_CPSS_TX_DESC_STC            tmpTxDesc;
    PRV_CPSS_SW_TX_FREE_DATA_STC    *txFreeDataPtr;
    PRV_CPSS_DRV_EVENT_HNDL_STC     *evHndlPtr;
    PRV_CPSS_TX_BUF_QUEUE_FIFO_STC  *fifoPtr;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr; /* pointer to the module
                                                configure of the PP's database*/
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;  /* TX queue mode of operation*/
    GT_U32                          netIfNum;   /* The network interface number */
    GT_U8                           wordCount;  /* No of words to read or write */
    GT_STATUS                       rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    /* Diamond Cut have not SDMA registers */

    /*check that the DB of networkIf library was initialized*/
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

    /* Check SDMA only, PRV_CPSS_DXCH_GDMA_CHECK_SDMA_Q_MAC(?To/FromCPU?) */
    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_FALSE)
    {
        PRV_CPSS_DXCH_DEV_TX_SDMA_QUEUE_NUM_CHECK_MAC(queueIdx);
    }

    if(PRV_CPSS_PP_MAC(devNum)->useIsr == GT_FALSE)
    {
        /* the CPSS not handling the ISR for the device so function must not be called */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
    {
        /* handle GDMA device */
        return prvCpssDxChGdmaNetIfTxBuffQueueEvIsrHandle(devNum,queueIdx);
    }

    txDescListPtr = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[queueIdx]);

    descPtr = txDescListPtr->next2Free;
    if (descPtr == NULL)
    {
        /* there is nothing to free so function must not be called */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
    }

    moduleCfgPtr = PRV_CPSS_DXCH_DEV_MODULE_CFG_MAC(devNum);
    netIfNum    = queueIdx>>3;
    queueMode = moduleCfgPtr->multiNetIfCfg.txSdmaQueuesConfig[netIfNum][_LQ_MAC(queueIdx)].queueMode;

    if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
    {
        wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
        rc = prvCpssHwPpReadRam(devNum, (GT_U32)descPtr->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
        cpssOsMemCpy(descPtr->txDesc, &tmpTxDesc, sizeof(PRV_CPSS_TX_DESC_STC));
    }
    else
    {
        tmpTxDesc.word1 = descPtr->txDesc->word1;
    }

    /* No more descriptors to release.  */
    while (TX_DESC_GET_OWN_BIT_MAC(devNum,&tmpTxDesc) != TX_DESC_DMA_OWN)
    {
        if (TX_DESC_GET_FIRST_BIT_MAC(devNum,&tmpTxDesc) == 0)
        {
            /* It may happen if all the Tx descriptors are free - this is legal case
               In this case - should not return GT_ERROR */
            if(txDescListPtr->freeDescNum == txDescListPtr->maxDescNum)
            {
                return GT_OK;
            }

            /* should not happen */
            CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_ERROR), LOG_ERROR_NO_MSG);
        }

        descNum     = 1;
        firstTxDescPtr = descPtr;
        evHndlPtr   = (PRV_CPSS_DRV_EVENT_HNDL_STC*)descPtr->evReqHndl;

        /* Get the packet's descriptors.        */
        while (TX_DESC_GET_LAST_BIT_MAC(devNum,&tmpTxDesc) == 0x0)
        {
            if (TX_DESC_GET_OWN_BIT_MAC(devNum,&tmpTxDesc) == TX_DESC_DMA_OWN)
            {
                return GT_OK;
            }
            descPtr = descPtr->swNextDesc;

            if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
            {
                wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
                rc = prvCpssHwPpReadRam(devNum, (GT_U32)descPtr->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
                cpssOsMemCpy(descPtr->txDesc, &tmpTxDesc, sizeof(PRV_CPSS_TX_DESC_STC));
            }
            else
            {
                tmpTxDesc.word1 = descPtr->txDesc->word1;
            }
            descNum++;
        }

        if (TX_DESC_GET_OWN_BIT_MAC(devNum,&tmpTxDesc) == TX_DESC_DMA_OWN)
        {
            return GT_OK;
        }

        /* for not control packets/master/stand alone/device that doesn't need trap2cpu WA */
        freeDescNum = descNum;

        if ((txFreeDataPtr = cpssBmPoolBufGet(txDescListPtr->poolId)) == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
        }

        /* save the Tx End data in FIFO node */
        txFreeDataPtr->devNum   = devNum;
        txFreeDataPtr->queueNum = queueIdx;
        txFreeDataPtr->userData = firstTxDescPtr->userData;
        txFreeDataPtr->nextPtr  = NULL;

        /* insert the new Tx end node into FIFO */
        if (evHndlPtr == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "evHndlPtr=NULL\n");
        }
        fifoPtr = (PRV_CPSS_TX_BUF_QUEUE_FIFO_STC*)evHndlPtr->extDataPtr;

        if (NULL == fifoPtr->tailPtr)
        {
            fifoPtr->tailPtr = txFreeDataPtr;
            fifoPtr->headPtr = txFreeDataPtr;

            /* notify the user process ,or        */
            /* call the application's CB function */
            /* only for first packet in fifo.     */
            prvCpssDrvEvReqNotify(evHndlPtr);
        }
        else
        {
            fifoPtr->tailPtr->nextPtr = txFreeDataPtr;
            fifoPtr->tailPtr = txFreeDataPtr;
        }

        while(freeDescNum > 0)
        {
            firstTxDescPtr->txDesc->word1 = 0x0;
            firstTxDescPtr->txDesc->word2 = 0x0;

            if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
            {
                wordCount = 2;  /* Size of word1 and word2*/
                rc = prvCpssHwPpWriteRam(devNum, firstTxDescPtr->sramTxDescOffset, wordCount, (GT_U32 *)firstTxDescPtr->txDesc);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
                }
            }

            firstTxDescPtr = firstTxDescPtr->swNextDesc;
            freeDescNum--;
        }

        txDescListPtr->freeDescNum += descNum;
        txDescListPtr->next2Free = firstTxDescPtr;
        if(txDescListPtr->next2Free == txDescListPtr->next2Feed)
            break;

        /* update the desc ponter for next iteration in while loop */
        descPtr = firstTxDescPtr;
        if(queueMode == CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E)
        {
            wordCount = 4;  /* Size of word1, word2, buffPointer and nextDescPointer */
            rc = prvCpssHwPpReadRam(devNum, (GT_U32)descPtr->sramTxDescOffset, wordCount, (GT_U32 *)&tmpTxDesc);
            if(rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(descPtr->txDesc, &tmpTxDesc, sizeof(PRV_CPSS_TX_DESC_STC));
        }
        else
        {
            tmpTxDesc.word1 = descPtr->txDesc->word1;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..31).
*                                      (APPLICABLE RANGES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
{
    GT_U32  regAddr;/* The register address to write to.    */
    GT_U32  data, netIfNum;
    GT_U32  mgUnitId;/*the MG to serve the SDMA operation*/
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    /* Diamond Cut have not SDMA registers */

    rc = prvTxQueuePropertyCheckAnyTx(devNum, queue);
    if (rc != GT_OK)
        return rc;

    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E &&
        !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
        {
            return prvCpssDxChGdmaNetIfTxQueueEnableSet(devNum,queue,enable);
        }

        netIfNum    = queue>>3;

        /************************************/
        /* convert netIfNum to mgUnitId     */
        /************************************/
        PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txQCmdReg;

        data = (1 << _LQ_MAC(queue));
        if (enable != GT_TRUE)
        {
            data <<= 8;
        }

        return prvCpssHwPpMgWriteReg (
            devNum,  mgUnitId, regAddr, data);
    }
    else if (PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E &&
             PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChNetIfSdmaTxQueueEnable function
* @endinternal
*
* @brief   Enable/Disable the specified traffic class queue for TX
*         on all packet processors in the system. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..31).
*                                      (APPLICABLE RANGES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
* @param[in] enable                   - GT_TRUE,  queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
*/
GT_STATUS cpssDxChNetIfSdmaTxQueueEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxQueueEnable);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queue, enable));

    rc = internal_cpssDxChNetIfSdmaTxQueueEnable(devNum, queue, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queue, enable));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS   rc;
    GT_U32      regAddr;/* The register address to write to.    */
    GT_U32      hwData, hwValue, netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/


    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_CHECK_xDMA_TX_Q_MAC(devNum, queue);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if(PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E &&
        !PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
        {
            return prvCpssDxChGdmaNetIfTxQueueEnableGet(devNum,queue,enablePtr);
        }

        netIfNum    = queue>>3;

        /************************************/
        /* convert netIfNum to mgUnitId     */
        /************************************/
        PRV_CPSS_DXCH_NETIF_CHECK_NETIF_NUM_AND_CONVERT_TO_MG_UNIT_ID_MAC(devNum,netIfNum,mgUnitId);

        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.txQCmdReg;

        rc = prvCpssHwPpMgReadReg (devNum, mgUnitId, regAddr, &hwValue);

        if(GT_OK != rc)
        {
            return rc;
        }

        hwData = hwValue & 0xFFFF;

        /* Need check only TxENQ bit to get Tx status.
          The TX SDMA is not active when TxENQ is 0. */
        if (hwData & (1 << _LQ_MAC(queue)))
        {
            *enablePtr = GT_TRUE;
        }
        else
        {
            *enablePtr = GT_FALSE;
        }

        return rc;
    }
    else if (PRV_CPSS_PP_MAC(devNum)->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E &&
             PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        *enablePtr = GT_FALSE;
        return GT_OK;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
}

/**
* @internal cpssDxChNetIfSdmaTxQueueEnableGet function
* @endinternal
*
* @brief   Get status of the specified traffic class queue for TX
*         packets from CPU. -- SDMA relate
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - device number
* @param[in] queue                    - traffic class queue
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - on CPU port is not SDMA
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS cpssDxChNetIfSdmaTxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           queue,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxQueueEnableGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, queue, enablePtr));

    rc = internal_cpssDxChNetIfSdmaTxQueueEnableGet(devNum, queue, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, queue, enablePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal prvCpssDxChNetSdmaCheckAndPoolGet function
* @endinternal
*
* @brief   This routine checks if SDMA is used and return SDMA pool ID if it used.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                     - device number
* @param[in] queue                      - SDMA queue
*
* @param[out] isSdmaPtr                - (pointer to) is SDMA used
* @param[out] poolIdPtr                - (pointer to) pool ID for queue
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_STATE             - the CPSS not handling the ISR for the device
*                                       so function must not be called
* @retval GT_NOT_INITIALIZED       - the library was not initialized
*/
static GT_STATUS prvCpssDxChNetSdmaCheckAndPoolGet
(
    IN  GT_U8               devNum,
    IN  GT_U8               queue,
    OUT GT_BOOL            *isSdmaPtr,
    OUT CPSS_BM_POOL_ID    *poolIdPtr
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);

    *isSdmaPtr = GT_FALSE;

    if(!PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        PRV_CPSS_DXCH_SDMA_USED_CHECK_MAC(devNum);

        /*check that the DB of networkIf library was initialized*/
        PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

        if(PRV_CPSS_PP_MAC(devNum)->useIsr == GT_FALSE)
        {
            /* this check can be done only after we have the 'device number' ,
               and now we can check that :
               the CPSS not handling the ISR for the device so function must not
               be called */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }

        *isSdmaPtr = GT_TRUE;

        if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
        {
            *poolIdPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr[queue].
                txEndFifo_poolId;
        }
        else
        {
            *poolIdPtr = PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[queue].poolId;
        }
    }

    return GT_OK;
}

/**
* @internal internal_cpssDxChNetIfTxBufferQueueGet function
* @endinternal
*
* @brief   This routine returns the caller the TxEnd parameters for a transmitted
*         packet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hndl                     - Handle got from gtEventBind.
*
* @param[out] devPtr                   - The device the packet was transmitted from.
* @param[out] cookiePtr                - The user cookie handed on the transmit request.
* @param[out] queuePtr                 - The queue from which this packet was transmitted
* @param[out] statusPtr                - GT_OK if packet transmission was successful, GT_FAIL on
*                                      packet reject.
*                                       GT_OK on success, or
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - no more packets
* @retval GT_BAD_STATE             - the CPSS not handling the ISR for the device
*                                       so function must not be called
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*/
static GT_STATUS internal_cpssDxChNetIfTxBufferQueueGet
(
    IN  GT_UINTPTR          hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
)
{
    PRV_CPSS_DRV_EVENT_HNDL_STC     *evHndlPtr;   /* The event handle pointer */
    PRV_CPSS_TX_BUF_QUEUE_FIFO_STC  *fifoPtr;     /* The Handle Tx-End FIFO   */
    PRV_CPSS_SW_TX_FREE_DATA_STC    *txEndDataPtr;/* The Tx shadow data pointer*/
    GT_BOOL                         isSdma;     /* is SDMA used */
    CPSS_BM_POOL_ID                 poolId;     /* pool ID for SDMA queue*/
    GT_STATUS                       rc;         /* return code */

    CPSS_NULL_PTR_CHECK_MAC(devPtr);
    CPSS_NULL_PTR_CHECK_MAC(cookiePtr);
    CPSS_NULL_PTR_CHECK_MAC(queuePtr);
    CPSS_NULL_PTR_CHECK_MAC(statusPtr);

    evHndlPtr = (PRV_CPSS_DRV_EVENT_HNDL_STC*)hndl;

    /* validate event handle data */
    if (NULL == evHndlPtr || NULL == evHndlPtr->extDataPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    fifoPtr = (PRV_CPSS_TX_BUF_QUEUE_FIFO_STC*)evHndlPtr->extDataPtr;

    if (NULL == fifoPtr->headPtr)
    {
        /* FIFO is empty */
        return /* it's not error for log */ GT_NO_MORE;
    }

    /* lock section from ISR preemption */
    PRV_CPSS_INT_SCAN_LOCK();

    txEndDataPtr = fifoPtr->headPtr;
    fifoPtr->headPtr  = txEndDataPtr->nextPtr;

    if (NULL == fifoPtr->headPtr)
    {
        /* last element in FIFO */
        fifoPtr->tailPtr = NULL;
    }

    /* extract data from FIFO element */
    *queuePtr   = txEndDataPtr->queueNum;
    *devPtr     = txEndDataPtr->devNum;
    *cookiePtr  = txEndDataPtr->userData;
    *statusPtr  = GT_OK;

    PRV_CPSS_INT_SCAN_UNLOCK();

    /* init to avoid warnings */
    poolId = NULL;

    CPSS_TRAFFIC_API_LOCK_MAC(*devPtr,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    rc = prvCpssDxChNetSdmaCheckAndPoolGet(*devPtr, *queuePtr, &isSdma, &poolId);
    CPSS_TRAFFIC_API_UNLOCK_MAC(*devPtr,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(isSdma)
    {
        /* return the FIFO element buffer to pool */
        PRV_CPSS_INT_SCAN_LOCK();
        if (cpssBmPoolBufFree(poolId, txEndDataPtr) != GT_OK)
        {
            PRV_CPSS_INT_SCAN_UNLOCK();
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        PRV_CPSS_INT_SCAN_UNLOCK();
    }
    else
    {
        /* return the packet header buffer to pool */
        if (prvCpssGenNetIfMiiPoolFreeBuf(PRV_CPSS_GEN_NETIF_MII_TX_POOL_E,
                                            txEndDataPtr) != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

/**
* @internal cpssDxChNetIfTxBufferQueueGet function
* @endinternal
*
* @brief   This routine returns the caller the TxEnd parameters for a transmitted
*         packet.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] hndl                     - Handle got from gtEventBind.
*
* @param[out] devPtr                   - The device the packet was transmitted from.
* @param[out] cookiePtr                - The user cookie handed on the transmit request.
* @param[out] queuePtr                 - The queue from which this packet was transmitted
* @param[out] statusPtr                - GT_OK if packet transmission was successful, GT_FAIL on
*                                      packet reject.
*                                       GT_OK on success, or
*
* @retval GT_BAD_PARAM             - one of the parameters value is wrong
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NO_MORE               - no more packets
* @retval GT_BAD_STATE             - the CPSS not handling the ISR for the device
*                                       so function must not be called
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*                                       GT_FAIL otherwise.
*/
GT_STATUS cpssDxChNetIfTxBufferQueueGet
(
    IN  GT_UINTPTR          hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfTxBufferQueueGet);

    CPSS_API_LOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, hndl, devPtr, cookiePtr, queuePtr, statusPtr));

    rc = internal_cpssDxChNetIfTxBufferQueueGet(hndl, devPtr, cookiePtr, queuePtr, statusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, hndl, devPtr, cookiePtr, queuePtr, statusPtr));
    CPSS_API_UNLOCK_DEVICELESS_MAC(PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet function
* @endinternal
*
* @brief   This routine frees all transmitted packets descriptors. In addition, all
*         user relevant data in Tx End FIFO.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - The device number the packet was transmitted from
* @param[in] txQueue                  - The queue the packet was transmitted upon
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..31).
*                                      (APPLICABLE RANGES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
*
* @param[out] cookiePtr                - (pointer to) the cookie attached to packet that was send
*                                      from this queue
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_MORE               - no more packet cookies to get
* @retval GT_ERROR                 - the Tx descriptor is corrupted
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. returns the caller the cookie attached to the frame that was sent from the
*       device on the specific queue.
*       2. put the descriptors of this sent packet back to the 'Free descriptors' list
*       --> SDMA relate
*       3. this function should be used only when the ISR of the CPSS is not in use,
*       meaning that the intVecNum (given in function cpssDxChHwPpPhase1Init) was
*       set to CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS.
*       NOTE: the caller is responsible to synchronize the calls to 'Tx send' and
*       this function , since both deal with the 'Tx descriptors'.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_PTR             *cookiePtr
)
{
    PRV_CPSS_TX_DESC_LIST_STC       *txDescListPtr;
    /* Counts the number of descriptors already freed. */
    GT_U32                          freeDescNum;
    PRV_CPSS_SW_TX_DESC_STC         *descPtr;
    /* Points to the first sw desc. to be sent to the callback function.*/
    PRV_CPSS_SW_TX_DESC_STC         *firstTxDescPtr;
    /* Number of descriptors this packet occupies to be sent to the callback
       function.*/
    GT_U32                          descNum;
    PRV_CPSS_TX_DESC_STC            tmpTxDesc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(cookiePtr);
    PRV_CPSS_DXCH_SDMA_USED_CHECK_MAC(devNum);

    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE,
            "GDMA not supported or not needed, txQueue=%d", txQueue);
    }

    if(!PRV_CPSS_DXCH_CHECK_SDMA_PACKETS_FROM_CPU_CORRUPT_WA_MAC(devNum))
    {
        PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
        PRV_CPSS_CHECK_xDMA_TX_Q_MAC(devNum, txQueue);

        /* the Tx descriptors for this Queue */
        txDescListPtr  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

        descPtr = txDescListPtr->next2Free;

        tmpTxDesc.word1 = descPtr->txDesc->word1;

        /* No more descriptors to release.  */
        if(TX_DESC_GET_OWN_BIT_MAC(devNum,&tmpTxDesc) == TX_DESC_DMA_OWN)
        {
            return /* it's not error for log */ GT_NO_MORE;
        }

        if(TX_DESC_GET_FIRST_BIT_MAC(devNum,&tmpTxDesc) == 0)
        {
            /*should not happen*/
            CPSS_LOG_ERROR_AND_RETURN_MAC(((GT_STATUS)GT_TX_RING_ERROR), LOG_ERROR_NO_MSG);
        }

        descNum     = 1;
        firstTxDescPtr = descPtr;

        /* Get the packet's descriptors.        */
        while(TX_DESC_GET_LAST_BIT_MAC(devNum,&tmpTxDesc) == 0x0)
        {
            if(TX_DESC_GET_OWN_BIT_MAC(devNum,&tmpTxDesc) == TX_DESC_DMA_OWN)
            {
                /*should not happen*/
                return /* it's not error for log */ GT_NO_MORE;
            }
            descPtr = descPtr->swNextDesc;
            tmpTxDesc.word1 = descPtr->txDesc->word1;
            descNum++;
        }

        if(TX_DESC_GET_OWN_BIT_MAC(devNum,&tmpTxDesc) == TX_DESC_DMA_OWN)
        {
            /*should not happen*/
            return /* it's not error for log */ GT_NO_MORE;
        }

        freeDescNum = descNum;

        /* we have a cookie to return to caller */
        *cookiePtr = firstTxDescPtr->userData;

        while(freeDescNum > 0)
        {
            firstTxDescPtr->txDesc->word1 = 0x0;

            firstTxDescPtr = firstTxDescPtr->swNextDesc;
            freeDescNum--;
        }

        txDescListPtr->freeDescNum += descNum;
        txDescListPtr->next2Free = firstTxDescPtr;
    }
    else
    {
        /* MII doesn't use descriptors */
        *cookiePtr = 0;
    }

    return GT_OK;
}

/**
* @internal cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet function
* @endinternal
*
* @brief   This routine frees all transmitted packets descriptors. In addition, all
*         user relevant data in Tx End FIFO.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  AAS.
*
* @param[in] devNum                   - The device number the packet was transmitted from
* @param[in] txQueue                  - The queue the packet was transmitted upon
*                                      (APPLICABLE RANGES: Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..31).
*                                      (APPLICABLE RANGES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X 0..7).
*
* @param[out] cookiePtr                - (pointer to) the cookie attached to packet that was send
*                                      from this queue
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NO_MORE               - no more packet cookies to get
* @retval GT_ERROR                 - the Tx descriptor is corrupted
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. returns the caller the cookie attached to the frame that was sent from the
*       device on the specific queue.
*       2. put the descriptors of this sent packet back to the 'Free descriptors' list
*       --> SDMA relate
*       3. this function should be used only when the ISR of the CPSS is not in use,
*       meaning that the intVecNum (given in function cpssDxChHwPpPhase1Init) was
*       set to CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS.
*       NOTE: the caller is responsible to synchronize the calls to 'Tx send' and
*       this function , since both deal with the 'Tx descriptors'.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_PTR             *cookiePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, txQueue, cookiePtr));

    rc = internal_cpssDxChNetIfSdmaTxBufferQueueDescFreeAndCookieGet(devNum, txQueue, cookiePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, txQueue, cookiePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet function
* @endinternal
*
* @brief   This function return the number of free Tx descriptors for given
*         device and txQueue
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue number.
*
* @param[out] numberOfFreeTxDescriptorsPtr - pointer to number of free
*                                      descriptors for the given queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_U32             *numberOfFreeTxDescriptorsPtr
)
{
    PRV_CPSS_TX_DESC_LIST_STC       *txDescListPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(numberOfFreeTxDescriptorsPtr);
    PRV_CPSS_DXCH_SDMA_USED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_CHECK_xDMA_TX_Q_MAC(devNum, txQueue);

    if(GT_TRUE == PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) &&
       prvQueueIsTxGenerator(devNum, txQueue) == GT_FALSE)
    {
        *numberOfFreeTxDescriptorsPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->gdmaInfo.
                    gdmaPerQueue_fromCpu_Arr[txQueue].fromCpu_descriptors.
                        fromCpu_free_numOfDescriptors;
        return GT_OK;
    }

    /* the Tx descriptors for this Queue */
    txDescListPtr  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    *numberOfFreeTxDescriptorsPtr = txDescListPtr->freeDescNum;

    return GT_OK;
}

/**
* @internal cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet function
* @endinternal
*
* @brief   This function return the number of free Tx descriptors for given
*         device and txQueue
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - Tx queue number.
*
* @param[out] numberOfFreeTxDescriptorsPtr - pointer to number of free
*                                      descriptors for the given queue.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad device or queue
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet
(
    IN GT_U8                devNum,
    IN GT_U8                txQueue,
    OUT GT_U32             *numberOfFreeTxDescriptorsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, txQueue, numberOfFreeTxDescriptorsPtr));

    rc = internal_cpssDxChNetIfSdmaTxFreeDescripotrsNumberGet(devNum, txQueue, numberOfFreeTxDescriptorsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, txQueue, numberOfFreeTxDescriptorsPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorPacketAdd function
* @endinternal
*
* @brief   This function adds a new packet to Tx SDMA working as Packet Generator.
*         This packet will be transmitted by the selected Tx SDMA with previous
*         packets already transmitted by this Packet generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @param[out] packetIdPtr              - (pointer to) packet identification number, used by
*                                      other Packet Generator functions which require access
*                                      to this packet.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_RESOURCE           - no available buffer or descriptor.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_VALUE             - addition of packet will violate the required
*                                       rate configuration.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be added while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorPacketAdd
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    OUT GT_U32                      *packetIdPtr
)
{
    GT_STATUS   rc;
    GT_U8       txQueue;        /* Tx queue */
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;   /* Tx desc. list */
    PRV_CPSS_TX_DESC_STC    *tmpTxDesc;
    GT_BOOL                 isGdma;
    GT_U32                  desc2descOffset;
    GT_UINTPTR  physicalMemoryPointer;

    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT rateMode; /* Generator rate mode */
    GT_U64                          rateValue;  /* Generator rate value */
    GT_U64                          actualRateValue;
                                            /* the real Generator rate value */
    GT_U32                          clkTicks; /* the HW configured clock ticks */
                                              /* to support the rate value */
    GT_U32          placeInQueue; /* the place (numerical) in the queue list */
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    CPSS_NULL_PTR_CHECK_MAC(packetParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetDataPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetIdPtr);

    cpssOsBzero((GT_VOID*)&actualRateValue, sizeof(actualRateValue));
    cpssOsBzero((GT_VOID*)&rateValue, sizeof(rateValue));


    txQueue = packetParamsPtr->sdmaInfo.txQueue;
    rc = prvTxQueuePropertyCheckAndConvert(devNum, txQueue, NULL, &mgUnitId);
    if( GT_OK != rc )
        return rc;

    txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    /* Check for available descriptor */
    if( 0 == txDescList->freeDescNum )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }

    /* Get current rate configuration */
    rc = cpssDxChNetIfSdmaTxGeneratorRateGet(devNum,
                                             portGroupsBmp,
                                             txQueue,
                                             &rateMode,
                                             &rateValue,
                                             &actualRateValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* To avoid warning */
    clkTicks = 0;

    /* Check availability of already configured rate can be supported due to */
    /* a new packet addition. */
    rc = prvCpssDxChNetIfSdmaTxGeneratorRateCalc(devNum,
                                                 rateMode,
                                                 rateValue,
                                                 txDescList->maxDescNum -
                                                    txDescList->freeDescNum + 1,
                                                 &clkTicks,
                                                 &actualRateValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    tmpTxDesc = txDescList->freeDescPtr;

    /* Configure and fill new descriptor & buffer */
    rc = prvCpssDxChNetIfSdmaTxGeneratorPacketToBufferSet(devNum,
                                                          packetParamsPtr,
                                                          packetDataPtr,
                                                          packetDataLength,
                                                          tmpTxDesc,
                                                    txDescList->actualBuffSize);
    if( GT_OK != rc )
    {
        return rc;
    }

    isGdma = PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum);

    desc2descOffset = (isGdma == GT_TRUE) ?
        sizeof(PRV_CPSS_GDMA_DESC_STC) :
        (sizeof(PRV_CPSS_TX_DESC_STC) + txDescList->actualBuffSize);

    if (isGdma == GT_TRUE)
    {
        /* Update ring chain size with '+1' (no change for Flow Rate Mode) */
        /* Note: may be special if(rateMode != CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E)*/
        rc = prvCpssDxChNetIfGdmaSgdChainSizeSet(devNum, mgUnitId, txQueue, GT_TRUE, 1);
        if (GT_OK != rc)
        {
            return rc;
        }

        rc = prvCpssDxChNetIfSdmaTxGeneratorHwRateSet(devNum, txQueue,
                                                      rateMode, clkTicks);
        if( GT_OK != rc )
        {
            return rc;
        }

        if ((txDescList->freeDescNum == txDescList->maxDescNum) &&
            (GT_TRUE == txDescList->userQueueEnabled))
        {
            /* An empty ring has got 1 buffer and ready-to-start - enable it */
            rc = cpssDxChNetIfSdmaTxQueueEnable(devNum, txQueue, GT_TRUE);
            if (GT_OK != rc)
            {
                return rc;
            }
        }
        /* Skip SDMA related part and go to common Sdma/Gdma */
        goto commonSdmaGdmaAdd;
    }

    /* --- SDMA only part ----------------------------------------------------- */
    /* Point to the first descriptor in the chain as the next one. */
    rc = cpssOsVirt2Phy((GT_UINTPTR)(txDescList->firstDescPtr),
                        /*OUT*/&physicalMemoryPointer);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    tmpTxDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)physicalMemoryPointer);

    /* Update HW rate configuration  */
    rc = prvCpssDxChNetIfSdmaTxGeneratorHwRateSet(devNum,
                                                  txQueue,
                                                  rateMode,
                                                  clkTicks);
    if( GT_OK != rc )
    {
        return rc;
    }

    if( txDescList->freeDescNum == txDescList->maxDescNum )
    {
        /* In case this is the first packet added to the queue, enable it */
        /* if previously explicitly enabled by application. */
        if( GT_TRUE == txDescList->userQueueEnabled )
        {
            /* Set Tx SDMA Current Descriptor Pointer to first descriptor in */
            /* queue chain */
            rc = cpssOsVirt2Phy((GT_UINTPTR)(txDescList->firstDescPtr),
                                /*OUT*/&physicalMemoryPointer);
            if (rc != GT_OK)
            {
                return rc;
            }

            #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
                if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
                }
            #endif

            rc = prvCpssHwPpMgWriteReg(devNum,mgUnitId,
                PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                     sdmaRegs.txDmaCdp[_LQ_MAC(txQueue)],(GT_U32)physicalMemoryPointer);

            if(GT_OK != rc)
            {
                return rc;
            }

            rc = cpssDxChNetIfSdmaTxQueueEnable(devNum, txQueue, GT_TRUE);

            if ( GT_OK != rc )
            {
                return rc;
            }
        }
    }
    else
    {
        /* Get physical address of new free descriptor, will be pointet */
        /* by next pointer field of the now last used descriptor. */
        rc = cpssOsVirt2Phy((GT_UINTPTR)tmpTxDesc, /*OUT*/&physicalMemoryPointer);
        if (rc != GT_OK)
        {
            return rc;
        }

        #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
            if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        #endif

        /* Get last used descriptor and point with its next to the new one*/
        tmpTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)tmpTxDesc -
                    (sizeof(PRV_CPSS_TX_DESC_STC) + txDescList->actualBuffSize));

        tmpTxDesc->nextDescPointer =
                            CPSS_32BIT_LE((GT_U32)physicalMemoryPointer);
    }

commonSdmaGdmaAdd:

    /* Advanced the next free descriptor */
    txDescList->freeDescPtr = (PRV_CPSS_TX_DESC_STC*)
                            ((GT_U8*)(txDescList->freeDescPtr) +
                                      desc2descOffset);

    placeInQueue = txDescList->maxDescNum - txDescList->freeDescNum;

    /* Decrease free descriptors number */
    txDescList->freeDescNum -= 1;

    /* Get next free packet ID */
    *packetIdPtr =
        PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.firstFree;

    /* Save queue number - bits [2:0] & place in queue - bits [31:3], */
    /* in packet ID DB */
    PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.cookie[*packetIdPtr] =
                              _LQ_MAC(txQueue) + (placeInQueue << 3);

    /* Set next free packet ID */
    PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.firstFree =
        PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.
                                                    freeLinkedList[*packetIdPtr];

    /* Update queue "reverse" packet ID DB, later used when moving last */
    /* packet during removal operation */
    txDescList->revPacketIdDb[placeInQueue] = *packetIdPtr;

    return rc;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorPacketAdd function
* @endinternal
*
* @brief   This function adds a new packet to Tx SDMA working as Packet Generator.
*         This packet will be transmitted by the selected Tx SDMA with previous
*         packets already transmitted by this Packet generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @param[out] packetIdPtr              - (pointer to) packet identification number, used by
*                                      other Packet Generator functions which require access
*                                      to this packet.
*
* @retval GT_OK                    - on success.
* @retval GT_NO_RESOURCE           - no available buffer or descriptor.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_VALUE             - addition of packet will violate the required
*                                       rate configuration.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be added while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketAdd
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength,
    OUT GT_U32                      *packetIdPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorPacketAdd);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, packetParamsPtr, packetDataPtr, packetDataLength, packetIdPtr));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorPacketAdd(devNum, portGroupsBmp, packetParamsPtr, packetDataPtr, packetDataLength, packetIdPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, packetParamsPtr, packetDataPtr, packetDataLength, packetIdPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorPacketUpdate function
* @endinternal
*
* @brief   This function updates already transmitted packet content and parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, packet is
*                                       not transmitted on this Tx SDMA, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be updated while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorPacketUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      packetId,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength

)
{
    GT_STATUS   rc;
    GT_U8       txQueue;        /* Tx queue */
    PRV_CPSS_TX_DESC_LIST_STC txDescList;  /* Tx desc. list */
    PRV_CPSS_TX_DESC_STC    *currTxDesc;    /* current updated descriptor */
    PRV_CPSS_TX_DESC_STC    *prevTxDesc;    /* descriptor previous to the */
                                            /* updated one */
    GT_U32                   desc2descOffset;
    GT_BOOL                  isGdma;

    GT_UINTPTR  physicalMemoryPointer;

    GT_U32        placeInQueue; /* the place (numerical) in the queue list */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    CPSS_NULL_PTR_CHECK_MAC(packetParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(packetDataPtr);

    txQueue = packetParamsPtr->sdmaInfo.txQueue;
    rc = prvTxQueuePropertyCheck(devNum, txQueue);
    if (rc != GT_OK)
        return rc;

    txDescList  = PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue];

    if (packetId >= PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.generatorsTotalDesc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Check the packet for update belongs to the txQueue */
    if( _LQ_MAC(PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.cookie[packetId]) !=
        _LQ_MAC(txQueue) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    placeInQueue = ((PRV_CPSS_PP_MAC(devNum)->
            intCtrl.txGeneratorPacketIdDb.cookie[packetId] >> 3) & 0x3FFFFFF);

    /* Check the packet within queue used packet range */
    if( placeInQueue >= (txDescList.maxDescNum - txDescList.freeDescNum) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    isGdma = PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum);

    desc2descOffset = (isGdma == GT_TRUE) ?
        sizeof(PRV_CPSS_GDMA_DESC_STC) :
        (sizeof(PRV_CPSS_TX_DESC_STC) + txDescList.actualBuffSize);

    currTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)(txDescList.firstDescPtr) +
                                                  placeInQueue * desc2descOffset);

    if(isGdma == GT_TRUE)
    {
        /* Invalidate the currTxDesc to excludes it from execution ring,
         * configure and fill new data to the buffer,
         * set VALID the currTxDesc to return it back to execution ring.
         */
        rc = prvCpssDxChNetIfSdmaTxGeneratorPacketToBufferSet(
                                                        devNum,
                                                        packetParamsPtr,
                                                        packetDataPtr,
                                                        packetDataLength,
                                                        currTxDesc,
                                                        txDescList.actualBuffSize);
        return rc;
    }

    if( currTxDesc == txDescList.firstDescPtr )
    {
        /* if the updated descriptor is the first, the previous is the last */
        prevTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)(txDescList.freeDescPtr) -
                                                      desc2descOffset);
    }
    else
    {
        prevTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)currTxDesc -
                                                     desc2descOffset);
    }

    /* Copy descriptor to be updated to the scratch pad descriptor */
    txDescList.scratchPadPtr->word1 = currTxDesc->word1;
    txDescList.scratchPadPtr->word2 = currTxDesc->word2;
    /* Point next from the scratch pad to the next of the updated descriptor */
    txDescList.scratchPadPtr->nextDescPointer = currTxDesc->nextDescPointer;
    /* Copy buffer data */
    cpssOsMemCpy( (GT_U8*)txDescList.scratchPadPtr + sizeof(PRV_CPSS_TX_DESC_STC),
                  (GT_U8*)currTxDesc + sizeof(PRV_CPSS_TX_DESC_STC),
                  TX_DESC_GET_BYTE_CNT_MAC(currTxDesc) );

    /* Physical address of the scratch pad */
    rc = cpssOsVirt2Phy((GT_UINTPTR)(txDescList.scratchPadPtr),
                        /*OUT*/&physicalMemoryPointer);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    /* Point with previous descriptor to the scratch pad */
    prevTxDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)physicalMemoryPointer);


    /* Wait 3 packet generator intervals */
    rc = prvCpssDxChNetIfSdmaTxGeneratorNPacketIntervalsWait(devNum, txQueue, 3);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Configure and fill new descriptor & buffer */
    rc = prvCpssDxChNetIfSdmaTxGeneratorPacketToBufferSet(
                                                    devNum,
                                                    packetParamsPtr,
                                                    packetDataPtr,
                                                    packetDataLength,
                                                    currTxDesc,
                                                    txDescList.actualBuffSize);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* Physical address of the current descriptor */
    rc = cpssOsVirt2Phy((GT_UINTPTR)(currTxDesc),
                        /*OUT*/&physicalMemoryPointer);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    /* Move previous next pointer back to current */
    prevTxDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)physicalMemoryPointer);

    return rc;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorPacketUpdate function
* @endinternal
*
* @brief   This function updates already transmitted packet content and parameters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set
*                                      into the packet descriptors.
* @param[in] packetDataPtr            - (pointer to) packet data.
* @param[in] packetDataLength         - packet data length. Buffer size configured during
*                                      initialization phase must be sufficient for packet
*                                      length and the DSA tag that will be added to it.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, packet is
*                                       not transmitted on this Tx SDMA, bad DSA
*                                       params or the data buffer is longer than
*                                       size configured.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note 1. Packet can be updated while Tx SDMA is enabled.
*       2. The packet buffer supplied by the application is copied to internal
*       prealloocated generator buffer, therefore application can free or reuse
*       that buffer as soon as function returns.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketUpdate
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      packetId,
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    IN  GT_U8                       *packetDataPtr,
    IN  GT_U32                      packetDataLength

)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorPacketUpdate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, packetId, packetParamsPtr, packetDataPtr, packetDataLength));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorPacketUpdate(devNum, portGroupsBmp, packetId, packetParamsPtr, packetDataPtr, packetDataLength);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, packetId, packetParamsPtr, packetDataPtr, packetDataLength));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorPacketRemove function
* @endinternal
*
* @brief   This function removes packet from Tx SDMA working as a Packet Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to remove the packet from.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or packet
*                                       is not transmitted on this Tx SDMA.
* @retval GT_BAD_VALUE             - removal of packet will violate the required
*                                       rate configuration.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Packet can be removed while Tx SDMA is enabled.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorPacketRemove
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U8               txQueue,
    IN  GT_U32              packetId
)
{
    GT_STATUS   rc;
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;  /* Tx desc. list */
    PRV_CPSS_TX_DESC_STC    *currTxDesc;    /* current descriptor of packet */
                                            /* remove */
    PRV_CPSS_TX_DESC_STC    *prevTxDesc;    /* descriptor previous to the */
                                            /* to removed one */
    PRV_CPSS_TX_DESC_STC    *lastTxDesc;    /* the last descriptor being used */
                                            /* in this queue */
    GT_U32        placeInQueue; /* the place (numerical) in the queue list */
    GT_BOOL                 enable;         /* queue enabling status */
    GT_UINTPTR  physicalMemoryPointer;
    GT_U32      movedPacketId; /* the packet ID of the moved buffer - */
                   /* from chain last place to the place of the removed one. */
    CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT rateMode; /* Generator rate mode */
    GT_U64                          rateValue;  /* Generator rate value */
    GT_U64                          actualRateValue;
                                            /* the real Generator rate value */
    GT_U32                          clkTicks; /* the HW configured clock ticks */
                                              /* to support the rate value */
    GT_BOOL                         isGdma;
    GT_U32                          gdmaUnit;
    GT_U32                          desc2descOffset;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    rc = prvTxQueuePropertyCheckAndConvert(devNum, txQueue, NULL, &gdmaUnit);
    if (rc != GT_OK)
        return rc;

    txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    if (packetId >= PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.generatorsTotalDesc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* Check the packet for update belongs to the txQueue */
    if( _LQ_MAC(PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.cookie[packetId]) !=
        _LQ_MAC(txQueue) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    placeInQueue = ((PRV_CPSS_PP_MAC(devNum)->
            intCtrl.txGeneratorPacketIdDb.cookie[packetId] >> 3) & 0x3FFFFFF);

    /* Check the packet within queue used packet range */
    if( placeInQueue >= (txDescList->maxDescNum - txDescList->freeDescNum) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    isGdma = PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum);

    desc2descOffset = (isGdma == GT_TRUE) ?
        sizeof(PRV_CPSS_GDMA_DESC_STC) :
        (sizeof(PRV_CPSS_TX_DESC_STC) + txDescList->actualBuffSize);

    currTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)(txDescList->firstDescPtr) +
                  (placeInQueue * desc2descOffset));

    lastTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)(txDescList->freeDescPtr) -
                   desc2descOffset);

   /* In case this is the last packet left in the queue chain */
   if( (txDescList->freeDescNum + 1) == txDescList->maxDescNum )
   {
        /* If queue is enabled, disable it */
        rc = cpssDxChNetIfSdmaTxQueueEnableGet(devNum, txQueue, &enable);
        if( GT_OK != rc )
        {
            return rc;
        }

        rc = cpssDxChNetIfSdmaTxQueueEnable(devNum, txQueue, GT_FALSE);
        if( GT_OK != rc )
        {
            return rc;
        }

        if (isGdma == GT_TRUE)
        {
            rc = prvCpssDxChNetIfGdmaSgdChainSizeSet(devNum, gdmaUnit, txQueue, GT_FALSE, 0);
            if (GT_OK != rc)
                return rc;

            /* Call SWAP to InValidate a descriptor */
            prvCpssDxChNetIfGdmaTxGeneratorDescriptorsSwap(
                                (void*)currTxDesc, GT_FALSE,  /*currTxDesc==lastTxDesc*/
                                (void*)lastTxDesc, GT_FALSE); /*became InVALID*/
        }
   } /* if(last packet left) .. else more than one descriptor in the queue chain */
   else
   { /* more than one descriptor in the queue chain */
    cpssOsBzero((GT_VOID*)&actualRateValue, sizeof(actualRateValue));
    cpssOsBzero((GT_VOID*)&rateValue, sizeof(rateValue));

    /* Get current rate configuration */
    rc = cpssDxChNetIfSdmaTxGeneratorRateGet(devNum,
                                             portGroupsBmp,
                                             txQueue,
                                             &rateMode,
                                             &rateValue,
                                             &actualRateValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    /* To avoid warning */
    clkTicks = 0;

    /* Check availability of already configured rate can be supported due to */
    /* packet removal. */
    rc = prvCpssDxChNetIfSdmaTxGeneratorRateCalc(devNum,
                                                 rateMode,
                                                 rateValue,
                                                 txDescList->maxDescNum -
                                                    txDescList->freeDescNum - 1,
                                                 &clkTicks,
                                                 &actualRateValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    if (isGdma == GT_TRUE)
    {
        /* Update ring chain size with '-1' */
        rc = prvCpssDxChNetIfGdmaSgdChainSizeSet(devNum, gdmaUnit, txQueue, GT_TRUE, -1);
        if (GT_OK != rc)
            return rc;
        /* Call SWAP even for (currTxDesc==lastTxDesc) to InValidate a descriptor */
        prvCpssDxChNetIfGdmaTxGeneratorDescriptorsSwap(
                            (void*)currTxDesc, GT_TRUE,   /*became VALID*/
                            (void*)lastTxDesc, GT_FALSE); /*became InVALID*/

        goto commonSdmaGdmaRemove;
    }

    /* --- SDMA only part ----------------------------------------------------- */
    if( currTxDesc == txDescList->firstDescPtr )
    {
        /* if the removed descriptor is the first, the previous is the last */
        prevTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)(txDescList->freeDescPtr) -
                                                      desc2descOffset);
    }
    else
    {
        prevTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)currTxDesc -
                                                     desc2descOffset);
    }

    /* Skip the descriptor to be romoved */
    prevTxDesc->nextDescPointer = currTxDesc->nextDescPointer;

    /* Wait 3 packet generator intervals */
    rc = prvCpssDxChNetIfSdmaTxGeneratorNPacketIntervalsWait(devNum, txQueue, 3);
    if( GT_OK != rc )
    {
        return rc;
    }

    if (currTxDesc != lastTxDesc)
    {

        /* Copy the last descriptor in chain to the place of the removed one */
        currTxDesc->word1 = lastTxDesc->word1;
        currTxDesc->word2 = lastTxDesc->word2;

        /* Copy the last buffer in chain to the place of the removed one */
        cpssOsMemCpy( (GT_U8*)currTxDesc + sizeof(PRV_CPSS_TX_DESC_STC),
                      (GT_U8*)lastTxDesc + sizeof(PRV_CPSS_TX_DESC_STC),
                      TX_DESC_GET_BYTE_CNT_MAC(lastTxDesc) );

        /* Physical address of the current descriptor */
        rc = cpssOsVirt2Phy((GT_UINTPTR)(currTxDesc),
                            /*OUT*/&physicalMemoryPointer);
        if (rc != GT_OK)
        {
            return rc;
        }

        #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
            if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
        #endif

        /* Return the previous descriptor pointing back to the current descriptor */
        prevTxDesc->nextDescPointer = CPSS_32BIT_LE((GT_U32)physicalMemoryPointer);

        /* Remove the last descriptor (already copied) from the chain */
        prevTxDesc = (PRV_CPSS_TX_DESC_STC*)((GT_U8*)(lastTxDesc) - desc2descOffset);

        prevTxDesc->nextDescPointer = lastTxDesc->nextDescPointer;

    }

commonSdmaGdmaRemove:

    /* Update HW rate configuration  */
    rc = prvCpssDxChNetIfSdmaTxGeneratorHwRateSet(devNum,
                                                  txQueue,
                                                  rateMode,
                                                  clkTicks);

   } /* END of else: more than one descriptor in the queue chain */

    movedPacketId =
     txDescList->revPacketIdDb[txDescList->maxDescNum-txDescList->freeDescNum-1];

    /* Update the moved last descriptor "reverse" packet ID DB*/
    txDescList->revPacketIdDb[placeInQueue] = movedPacketId;

    /* Update the moved last descriptor packet ID DB*/
    PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.
                                            cookie[movedPacketId] =
                                                (_LQ_MAC(txQueue)) + (placeInQueue << 3);
    /* Update the packet ID DB free list */
    PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.cookie[packetId] =
                                                                    0xFFFFFFFF;
    PRV_CPSS_PP_MAC(devNum)->
        intCtrl.txGeneratorPacketIdDb.freeLinkedList[packetId] =
               PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.firstFree;

    PRV_CPSS_PP_MAC(devNum)->intCtrl.txGeneratorPacketIdDb.firstFree = packetId;

    /* Increase queue free descriptors number */
    txDescList->freeDescNum += 1;

    /* Update next queue free descriptor pointer to previous last */
    txDescList->freeDescPtr = lastTxDesc;

    return rc;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorPacketRemove function
* @endinternal
*
* @brief   This function removes packet from Tx SDMA working as a Packet Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to remove the packet from.
* @param[in] packetId                 - packet identification number assign during packet
*                                      addition to the Generator.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or packet
*                                       is not transmitted on this Tx SDMA.
* @retval GT_BAD_VALUE             - removal of packet will violate the required
*                                       rate configuration.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Packet can be removed while Tx SDMA is enabled.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorPacketRemove
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U8               txQueue,
    IN  GT_U32              packetId
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorPacketRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, txQueue, packetId));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorPacketRemove(devNum, portGroupsBmp, txQueue, packetId);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, txQueue, packetId));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorRateSet function
* @endinternal
*
* @brief   This function sets Tx SDMA Generator transmission packet rate.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue.
* @param[in] rateMode                 - transmission rate mode.
* @param[in] rateValue                - packets per second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValue.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad rate
*                                       mode or value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_BAD_VALUE             - rate cannot be configured for Tx SDMA.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorRateSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    IN  CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode,
    IN  GT_U64                                      rateValue,
    OUT GT_U64                                      *actualRateValuePtr
)
{
    GT_STATUS   rc;
    PRV_CPSS_TX_DESC_LIST_STC txDescList;   /* Tx desc. list */
    GT_U32      clkTicks = 0;     /* number of clock ticks for HW configuration */
    OUT PRV_QUEUE_TIMER_CFG_STC  timerCfg;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    rc = prvQueueRatePropertyCheckAndInit(devNum, txQueue, &timerCfg);
    if (rc != GT_OK)
        return rc;

    txDescList  = PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue];

    if ( CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E != rateMode )
    {
        CPSS_NULL_PTR_CHECK_MAC(actualRateValuePtr);
        rc = prvCpssDxChNetIfSdmaTxGeneratorRateCalc(devNum,
                                                     rateMode,
                                                     rateValue,
                                                     txDescList.maxDescNum -
                                                     txDescList.freeDescNum,
                                                     &clkTicks,
                                                     actualRateValuePtr);
        if( GT_OK != rc )
        {
            return rc;
        }
    }

    /* HW rate configuration  */
    rc = prvCpssDxChNetIfSdmaTxGeneratorHwRateSet(devNum,
                                                  txQueue,
                                                  rateMode,
                                                  clkTicks);

    if( GT_OK != rc )
    {
        return rc;
    }

    /* Save configured rate parameters to DB */
    PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.rateMode[timerCfg.netIfNum][timerCfg.localQueue] =
                                                                      rateMode;
    PRV_CPSS_DXCH_PP_MAC(devNum)->netIf.rateValue[timerCfg.netIfNum][timerCfg.localQueue] =
                                                                      rateValue;

    return rc;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorRateSet function
* @endinternal
*
* @brief   This function sets Tx SDMA Generator transmission packet rate.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue.
* @param[in] rateMode                 - transmission rate mode.
* @param[in] rateValue                - (pointer to) packets per
*                                      second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode
*                                      ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValue.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode, bad rate
*                                       mode or value.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_BAD_VALUE             - rate cannot be configured for Tx SDMA.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note Rate can be set while Tx SDMA is enabled.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorRateSet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    IN  CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    rateMode,
    IN  GT_U64                                      *rateValue,
    OUT GT_U64                                      *actualRateValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorRateSet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, txQueue, rateMode, rateValue, actualRateValuePtr));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorRateSet(devNum, portGroupsBmp, txQueue, rateMode, *rateValue, actualRateValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, txQueue, rateMode, rateValue, actualRateValuePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorRateGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator transmission packet rate.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue.
*
* @param[out] rateModePtr              - (pointer to) transmission rate mode
* @param[out] rateValuePtr             - (pointer to)
*                                      packets per second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValuePtr.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorRateGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    *rateModePtr,
    OUT GT_U64                                      *rateValuePtr,
    OUT GT_U64                                      *actualRateValuePtr
)
{
    PRV_CPSS_TX_DESC_LIST_STC txDescList;   /* Tx desc. list */
    GT_U32      clkTicks;         /* number of clock ticks for HW configuration */
    GT_STATUS   rc;
    PRV_QUEUE_TIMER_CFG_STC  timerCfg;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    CPSS_NULL_PTR_CHECK_MAC(rateModePtr);

    rc = prvQueueRatePropertyCheckAndInit(devNum, txQueue, &timerCfg);
    if (rc != GT_OK)
        return rc;

    txDescList  = PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue];

    *rateModePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->
                                        netIf.rateMode[timerCfg.netIfNum][timerCfg.localQueue];

    if( CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E != *rateModePtr )
    {
        CPSS_NULL_PTR_CHECK_MAC(rateValuePtr);
        CPSS_NULL_PTR_CHECK_MAC(actualRateValuePtr);

        *rateValuePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->
                                        netIf.rateValue[timerCfg.netIfNum][timerCfg.localQueue];

        return prvCpssDxChNetIfSdmaTxGeneratorRateCalc(devNum,
                                                       *rateModePtr,
                                                       *rateValuePtr,
                                                       txDescList.maxDescNum -
                                                        txDescList.freeDescNum,
                                                       &clkTicks,
                                                       actualRateValuePtr);
    }

    return GT_OK;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorRateGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator transmission packet rate.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue.
*
* @param[out] rateModePtr              - (pointer to) transmission rate mode
* @param[out] rateValuePtr             - (pointer to)
*                                      packets per second for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_GLOBAL_THROUGHPUT_E,
*                                      nanoseconds for rateMode ==
*                                      CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_PER_FLOW_INTERVAL_E.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
* @param[out] actualRateValuePtr       - (pointer to) the actual configured rate value.
*                                      Same units as rateValuePtr.
*                                      Not relevant to CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_NO_GAP_E.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorRateGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_RATE_MODE_ENT    *rateModePtr,
    OUT GT_U64                                      *rateValuePtr,
    OUT GT_U64                                      *actualRateValuePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorRateGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, txQueue, rateModePtr, rateValuePtr, actualRateValuePtr));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorRateGet(devNum, portGroupsBmp, txQueue, rateModePtr, rateValuePtr, actualRateValuePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, txQueue, rateModePtr, rateValuePtr, actualRateValuePtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx SDMA Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
* @param[in] burstEnable              - GT_TRUE for packets burst generation,
*                                      - GT_FALSE for continuous packets generation.
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst
*                                       size.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW queue enabling the Tx SDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorEnable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue,
    IN  GT_BOOL                         burstEnable,
    IN  GT_U32                          burstPacketsNumber
)
{
    GT_STATUS   rc;
    GT_U32      regValue;       /* register value */
    PRV_CPSS_TX_DESC_LIST_STC *txDescList;   /* Tx desc. list */
    GT_BOOL     queueEnableState;   /* the current HW enabling queue status*/
    GT_UINTPTR  physicalMemoryPointer;
    GT_U32      netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    rc = prvTxQueuePropertyCheckAndConvert(devNum, txQueue, &netIfNum, &mgUnitId);
    if( GT_OK != rc )
        return rc;

    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return prvCpssDxChNetIfGdmaTxGeneratorEnable(devNum,
                                                     mgUnitId, txQueue,
                                                     burstEnable, burstPacketsNumber);
    }

    txDescList  = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    if( GT_TRUE == burstEnable )
    {
        if( burstPacketsNumber >= BIT_29 )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regValue = 1 + ( burstPacketsNumber << 1);
    }
    else
    {
        regValue = 0x0;
    }

    rc = prvCpssHwPpMgWriteRegBitMask(devNum, mgUnitId,
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.
                                    txSdmaPacketCountConfigQueue[_LQ_MAC(txQueue)],
            0x3FFFFFFF,
            regValue);

    if( GT_OK != rc )
    {
        return rc;
    }

    /* Indication for explicit enabling request */
    txDescList->userQueueEnabled = GT_TRUE;

    /* Enabling a queue is prohibited if the queue is empty, and will be done */
    /* later implicitly when the first packet is added to the queue. */
    if( txDescList->freeDescNum == txDescList->maxDescNum )
    {
        return GT_OK;
    }

    /* No need to re-enabled, already enabled queue */
    rc = cpssDxChNetIfSdmaTxQueueEnableGet(devNum, txQueue, &queueEnableState);
    if( GT_OK != rc || GT_TRUE == queueEnableState )
    {
        return rc;
    }

    /* Set Tx SDMA Current Descriptor Pointer to first descriptor in chain */
    rc = cpssOsVirt2Phy((GT_UINTPTR)(txDescList->firstDescPtr),
                        /*OUT*/&physicalMemoryPointer);
    if (rc != GT_OK)
    {
        return rc;
    }

    #if __WORDSIZE == 64    /* phyAddr must fit in 32 bit */
        if (0 != (physicalMemoryPointer & 0xffffffff00000000L))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    #endif

    rc = prvCpssHwPpMgWriteReg(devNum, mgUnitId,
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->
                     sdmaRegs.txDmaCdp[_LQ_MAC(txQueue)],(GT_U32)physicalMemoryPointer);

    if(GT_OK != rc)
    {
        return rc;
    }

    return cpssDxChNetIfSdmaTxQueueEnable(devNum, txQueue, GT_TRUE);
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx SDMA Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
* @param[in] burstEnable              - GT_TRUE for packets burst generation,
*                                      - GT_FALSE for continuous packets generation.
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst
*                                       size.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note If queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW queue enabling the Tx SDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorEnable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue,
    IN  GT_BOOL                         burstEnable,
    IN  GT_U32                          burstPacketsNumber
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorEnable);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, txQueue, burstEnable, burstPacketsNumber));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorEnable(devNum, portGroupsBmp, txQueue, burstEnable, burstPacketsNumber);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, txQueue, burstEnable, burstPacketsNumber));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorDisable function
* @endinternal
*
* @brief   This function disables selected Tx SDMA Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorDisable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    rc = prvTxQueuePropertyCheck(devNum, txQueue);
    if (rc != GT_OK)
        return rc;

    /* Indication for explicit disabling request */
    PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue].userQueueEnabled =
                                                                       GT_FALSE;

    return cpssDxChNetIfSdmaTxQueueEnable(devNum, txQueue, GT_FALSE);
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorDisable function
* @endinternal
*
* @brief   This function disables selected Tx SDMA Generator.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorDisable
(
    IN  GT_U8                           devNum,
    IN  GT_PORT_GROUPS_BMP              portGroupsBmp,
    IN  GT_U8                           txQueue
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorDisable);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, txQueue));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorDisable(devNum, portGroupsBmp, txQueue);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, txQueue));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChNetIfSdmaTxGeneratorBurstStatusGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator burst counter status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] burstStatusPtr           - (pointer to) Burst counter status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
static GT_STATUS internal_cpssDxChNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT *burstStatusPtr
)
{
    GT_STATUS   rc;
    GT_U32      fieldValue;     /* field value */
    GT_U32      netIfNum;
    GT_U32      mgUnitId;/*the MG to serve the SDMA operation*/

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);
    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum,portGroupsBmp);

    CPSS_NULL_PTR_CHECK_MAC(burstStatusPtr);

    rc = prvTxQueuePropertyCheckAndConvert(devNum, txQueue, &netIfNum, &mgUnitId);
    if( GT_OK != rc )
        return rc;

    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssHwPpGdmaGetRegField(devNum, mgUnitId,
                PRV_DXCH_REG1_UNIT_GDMA_MAC(devNum).queueStatusAndControlRegs[txQueue].queueEnable,
                1, 1, &fieldValue);
        if( GT_OK != rc )
        {
            return rc;
        }
        /* For backward compatibility use _LIMIT_ instead of _IDLE_ */
        *burstStatusPtr = (fieldValue == 0) ? CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_RUN_E : /*BUSY*/
                                              CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E;
        return GT_OK;
    }

    rc = prvCpssHwPpMgGetRegField(devNum, mgUnitId,
            PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->sdmaRegs.
                                    txSdmaPacketCountConfigQueue[_LQ_MAC(txQueue)],
            30, 2, &fieldValue);

    if( GT_OK != rc )
    {
        return rc;
    }

    switch(fieldValue)
    {
        case 0: *burstStatusPtr =
                            CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_IDLE_E;
                break;
        case 1: *burstStatusPtr =
                            CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_RUN_E;
                break;
        case 2: *burstStatusPtr =
                            CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_LIMIT_E;
                break;
        case 3: *burstStatusPtr =
                            CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_TERMINATE_E;
                break;
        default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                break;
    }

    return GT_OK;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorBurstStatusGet function
* @endinternal
*
* @brief   This function gets Tx SDMA Generator burst counter status.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman;AAS.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is
*                                      IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES None)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] burstStatusPtr           - (pointer to) Burst counter status.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorBurstStatusGet
(
    IN  GT_U8                                       devNum,
    IN  GT_PORT_GROUPS_BMP                          portGroupsBmp,
    IN  GT_U8                                       txQueue,
    OUT CPSS_DXCH_NET_TX_GENERATOR_BURST_STATUS_ENT *burstStatusPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChNetIfSdmaTxGeneratorBurstStatusGet);

    CPSS_TRAFFIC_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, txQueue, burstStatusPtr));

    rc = internal_cpssDxChNetIfSdmaTxGeneratorBurstStatusGet(devNum, portGroupsBmp, txQueue, burstStatusPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, txQueue, burstStatusPtr));
    CPSS_TRAFFIC_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_RXTX_CNS);

    return rc;
}
