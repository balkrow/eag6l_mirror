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
* @file prvCpssDxChTxqSdq.c
*
* @brief CPSS SIP6 TXQ Sdq low level configurations.
*
* @version   1
********************************************************************************
*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/pipe/prvCpssAc5pTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/pipe/prvCpssAc5xTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/pipe/prvCpssHarrierTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/seahawk/pipe/prvCpssAasTxqSdqRegFile.h>

#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqDebugUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define TBD_SIP7_SDQ_PROFILES

typedef float GT_FLOAT;

#define PRV_TH_GRANULARITY_SHIFT 8 /* 256*/
#define PRV_TXQ_UNIT_NAME "SDQ"
#define PRV_TXQ_LOG_REG GT_FALSE
#define PRV_TXQ_LOG_TABLE GT_FALSE


#define PRV_TXQ_LAST_VALID_BIT_INDEX 31
#define PRV_TXQ_REG_SIZE (PRV_TXQ_LAST_VALID_BIT_INDEX+1)
#define PRV_TXQ_TABLE_SET_NUM_CNS 29


static GT_STATUS prvCpssSip6TxqSdqSemiEligThresholdGet
(
     IN  GT_U8                      devNum,
     IN  CPSS_PORT_SPEED_ENT        speed,
     OUT GT_U32                     *thresholdPtr
);


/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   - port speed
* @param[out] queueAttributesPtr     - (pointer to) queue thresholds
*
* @retval GT_OK               - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/

static GT_STATUS prvCpssFalconTxqSdqInitQueueThresholdAttributes
(
    GT_U32                                              devNum,
    IN  CPSS_PORT_SPEED_ENT                             speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES     * queueAttributesPtr
);

static GT_STATUS prvCpssFalconTxqSdqInitPortAttributes
(
    IN  CPSS_PORT_SPEED_ENT                         speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
);

static GT_STATUS prvCpssSip6TxqSdqLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_U32                                   sdqNum,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_REG;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = sdqNum;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}
static GT_STATUS prvCpssSip6TxqSdqTableLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC * logDataPtr,
   IN GT_U32                                   tileNum,
   IN GT_U32                                   sdqNum,
   IN CPSS_DXCH_TABLE_ENT                      tableIdentity,
   IN GT_CHAR_PTR                              tableName,
   IN GT_U32                                   entryIndex
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    GT_UNUSED_PARAM(tableIdentity);

    logDataPtr->log = PRV_TXQ_LOG_TABLE;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->tileIndex = tileNum;
    logDataPtr->unitIndex = sdqNum;
    logDataPtr->tableName = tableName;
    logDataPtr->entryIndex = entryIndex;

    return GT_OK;
}



GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       startQ,
    IN GT_U32                                       endQ,
    IN GT_U32                                       qcnFactor
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    i,offset,size;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        for(i= startQ;i<=endQ;i++)
        {
          rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,i,
             PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QCN_DIV_FACTOR_E,&qcnFactor);

          if(rc != GT_OK)
          {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QCN_DIV_FACTOR_E set failed\n");
          }
        }

        return GT_OK;
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
        return rc;
    }


    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }

    for(i= startQ;i<=endQ;i++)
    {
        log.regIndex = i;

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
             tileNum,&log,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[i],
             offset,
             size,
             qcnFactor);

        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queue,
    IN GT_U32                                       *qcnFactorPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(qcnFactorPtr);

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queue,
             PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QCN_DIV_FACTOR_E,qcnFactorPtr);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QCN_DIV_FACTOR_E get failed\n");
        }

        return rc;
    }

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QCN_DIV_FACTOR_FIELD_SIZE;
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
             tileNum,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queue],
             offset,
             size,
             qcnFactorPtr);


    return rc;
}

GT_STATUS prvCpssSip6TxqSdqLocalPortQcnFactorDump
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queue,
    IN GT_U32                                       numberOfQueues
)
{
    GT_STATUS rc;
    GT_U32    i,data;

    for(i=0;i<numberOfQueues;i++)
    {
        rc =prvCpssSip6TxqSdqLocalPortQcnFactorGet(devNum,tileNum,sdqNum,queue+i,&data);
        if(rc!=GT_OK)
        {
          return rc;
        }

        cpssOsPrintf("Q %d QCN factor 0x%02x\n",queue+i,data);
    }

    return GT_OK;

}


/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeSet function
* @endinternal
*
* @brief   Map port to queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] range                    - the  of the queues(APPLICABLE RANGES:0..399).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE  range
)
{
    GT_U32 regValue = 0,i,offset,size;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    CPSS_DXCH_TABLE_ENT table;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);


    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if((range.hiQueueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)||
        (range.lowQueueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)||
        (range.lowQueueNumber>range.hiQueueNumber))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(GT_FALSE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Range_Low");
        if (rc != GT_OK)
        {
            return rc;
        }
        log.regIndex = localPortNum;

        if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
          offset = TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
          size = TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
        }
        else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
          offset = TXQ_HAWK_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
          size = TXQ_HAWK_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
        }
        else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
          offset = TXQ_PHOENIX_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
          size = TXQ_PHOENIX_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
        }
        else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
          offset = TXQ_HARRIER_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
          size = TXQ_HARRIER_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
        }
        else
        {
          offset = TXQ_IRONMAN_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
          size = TXQ_IRONMAN_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
        }

        U32_SET_FIELD_MASKED_MAC(regValue,offset,
                                size,
                                range.lowQueueNumber);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_Low[localPortNum],
                     0,
                     32,
                     regValue);

         if(rc!=GT_OK)
         {
            return rc;
         }

         regValue = 0;

        if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
          offset = TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
          size = TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
        }
        else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
        {
          offset = TXQ_HAWK_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
          size = TXQ_HAWK_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
        }
        else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
          offset = TXQ_PHOENIX_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
          size = TXQ_PHOENIX_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
        }
        else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
        {
          offset = TXQ_HARRIER_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
          size = TXQ_HARRIER_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
        }
        else
        {
          offset = TXQ_IRONMAN_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
          size = TXQ_IRONMAN_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
        }

         U32_SET_FIELD_MASKED_MAC(regValue,offset,
                                    size,
                                    range.hiQueueNumber);

         log.regName = "Port_Range_High";

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_High[localPortNum],
                     0,
                     32,
                     regValue);
    }
    else
    {
        for(i=range.lowQueueNumber;i<=range.hiQueueNumber;i++)
        {
           table = CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PORT_MAP_E + sdqNum + tileNum * CPSS_DXCH_SIP_7_MAX_LOCAL_SDQ_NUM_MAC;

           rc = prvCpssDxChWriteTableEntryField(devNum,
                                        table,
                                        i>>3 /*each line has 8 queues*/,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_AAS_SDQ_QUEUE_TO_PORT_MAP_QUEUE_PORT_FIELD_SIZE*(i&0x7),
                                        TXQ_AAS_SDQ_QUEUE_TO_PORT_MAP_QUEUE_PORT_FIELD_SIZE,
                                        localPortNum);
          if(rc!=GT_OK)
          {
             return rc;
          }
        }
    }

    return rc;
}
/**
* @internal prvCpssFalconTxqSdqLocalPortQueueRangeGet function
* @endinternal
*
* @brief   Read port to queue from specific SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortQueueRangeGet
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           tileNum,
    IN GT_U32                                           sdqNum,
    IN GT_U32                                           localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_QUEUE_RANGE     * rangePtr
)
{

    GT_STATUS rc;
    GT_U32 regValue,offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_Low[localPortNum],
                 0,
                 32,
                 &regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_PORT_RANGE_LOW_PORT_RANGE_LOW_0_FIELD_SIZE;
    }

    rangePtr->lowQueueNumber= U32_GET_FIELD_MAC(regValue,
        offset,
        size);


    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Range_High[localPortNum],
                 0,
                 32,
                 &regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
      size = TXQ_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_PORT_RANGE_HIGH_PORT_RANGE_HIGH_0_FIELD_SIZE;
    }

     rangePtr->hiQueueNumber = U32_GET_FIELD_MAC(regValue,
        offset,
        size);


     return rc;

}
/**
* @internal prvCpssFalconTxqSdqQueueAttributesSet function
* @endinternal
*
* @brief   Queue attributes set
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
* @param[in] speed                    - speed of port that contain the queue
* @param[in] enable                   - queue enable/disable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               queueNumber,
    IN CPSS_PORT_SPEED_ENT  speed,
    IN GT_BOOL              semiEligEnable
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size,offset,tmpSize;
    GT_U32 value;
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES  queueAttributes = { 0, 0, 0, 0, 0, 0 };

    GT_U32 semiEligThreshold;
    CPSS_DXCH_TABLE_ENT table;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber >= CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqSdqInitQueueThresholdAttributes(devNum,speed, &queueAttributes);

    if(rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
    }



    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {

        value = queueAttributes.agingThreshold >> PRV_TH_GRANULARITY_SHIFT;

        rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_AGING_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_AGING_E set failed\n");
        }

        value = queueAttributes.negativeCreditThreshold >> PRV_TH_GRANULARITY_SHIFT;

        rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_NEG_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_NEG_E set failed \n");
        }

        value = queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT;

        rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_LOW_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_LOW_E set failed \n");
        }

        value = queueAttributes.highCreditThreshold >> PRV_TH_GRANULARITY_SHIFT;

        rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_HIGH_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_HIGH_E set failed \n");
        }

        /*Now set semi-elig threshold*/

        value = queueAttributes.semiEligThreshold >> PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT;

        /*round up*/
        if(queueAttributes.semiEligThreshold % (1 << PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT) > ((1 << (PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT - 1))))
        {
            value++;
        }

        if(GT_FALSE == semiEligEnable)
        {
            value = 0;
        }

         rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_SEMI_ELIG_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_SEMI_ELIG_E set failed \n");
        }

        return GT_OK;
    }


    cpssOsMemSet(regValue, 0, sizeof(GT_U32) * 2);

    regValue[0] = 0;


    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
      size = TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
    }

    if(queueAttributes.agingThreshold >= (GT_U32)(1 << size))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue[0], offset,
                             size,
                             queueAttributes.agingThreshold >> PRV_TH_GRANULARITY_SHIFT);

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
      size = TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
    }
    else
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Field not supported for device %d \n",devNum);
    }

    if((queueAttributes.negativeCreditThreshold >> PRV_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << size))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue[0],offset,
                             size,
                             queueAttributes.negativeCreditThreshold>>PRV_TH_GRANULARITY_SHIFT);


    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
      size = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
    }

    if((queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << size))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    value = U32_GET_FIELD_MAC(queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT, 0, 32 - offset);
    tmpSize = 32 - offset;

    U32_SET_FIELD_MASKED_MAC(regValue[0], offset,
                             tmpSize,
                             value);

    value = U32_GET_FIELD_MAC(queueAttributes.lowCreditTheshold >> PRV_TH_GRANULARITY_SHIFT,
                              32 - offset,
                              size - (32 - offset));

    tmpSize = size - (32 - offset);

    U32_SET_FIELD_MASKED_MAC(regValue[1], 0,
                             tmpSize,
                             value);


    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }

    if((queueAttributes.highCreditThreshold >> PRV_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << size))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue[1], offset - 32,
                             size,
                             queueAttributes.highCreditThreshold >> PRV_TH_GRANULARITY_SHIFT);


    table= CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E + sdqNum + tileNum * CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC;

    rc = prvCpssSip6TxqSdqTableLogInit(&tableLog,tileNum,sdqNum,table,"TBL_queue_cfg",queueNumber);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                                    table,
                                    queueNumber,
                                    regValue);

    if(rc != GT_OK)
    {
        return rc;
    }

    /*Now set semi-elig threshold*/

    semiEligThreshold = queueAttributes.semiEligThreshold >> PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT;

    /*round up*/
    if(queueAttributes.semiEligThreshold % (1 << PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT) > ((1 << (PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT - 1))))
    {
        semiEligThreshold++;
    }

    if(GT_FALSE == semiEligEnable)
    {
        semiEligThreshold = 0;
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = queueNumber;

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }


    /*According to Cider : "This threshold is 256B granule and is signed number defined by the msb"*/




    if((queueAttributes.semiEligThreshold >> PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT) >= (GT_U32)(1 << size))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                                             tileNum,&log,
                                             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                                             offset,
                                             size,
                                             semiEligThreshold);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqQueueEnableSet function
* @endinternal
*
* @brief   Enable/disable queue
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] enable                - if GT_TRUE credit aging is enabled, otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNumber,
 IN GT_U32  tc,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(tc>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNumber);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Pause_TC");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNumber;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET)+(tc&0xF) /*only 16 traffic classes*/,
                 1,
                 /*reverse logic*/
                 enable?0:1);

     return rc;



}

/**
* @internal prvCpssFalconTxqSdqQueueEnableGet function
* @endinternal
*
* @brief   Get Enable/disable queue status
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..7).
*
* @param[out] enablePtr                - if GT_TRUE queue  is enabled , otherwise disabled
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueEnableGet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNumber,
 IN GT_U32  queueNumber,
 IN GT_BOOL * enablePtr
)
{
    GT_STATUS rc;
    GT_U32          regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PAUSE_TC_PAUSE_TC_0_FIELD_OFFSET)+(queueNumber&0xF)/*only 16 traffic classes*/,
                 1,
                 &regValue);

     if(rc==GT_OK)
     {
          /*reverse logic*/
        *enablePtr = (regValue == 1)?GT_FALSE:GT_TRUE;
     }

     return rc;



}

/**
* @internal prvCpssSip6TxqSdqPausedTcBmpSet function
* @endinternal
*
* @brief   Pause queues for local port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] localPortNum          - local port number of SDQ
* @param[in] pausedTcBmp           - bitmap of paused queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqPausedTcBmpSet
(
 IN  GT_U8   devNum,
 IN  GT_U32  tileNum,
 IN  GT_U32  sdqNum,
 IN  GT_U32  localPortNum,
 IN  GT_U32  pausedTcBmp
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Pause_TC");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNum],
                 0,
                 32,
                 pausedTcBmp);

   return rc;
}

/**
* @internal prvCpssSip6TxqSdqPausedTcBmpSet function
* @endinternal
*
* @brief   Pause queues for local port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] localPortNum          -local port number of SDQ
* @param[out] pausedTcBmpPtr       -(pointer to )bitmap of paused queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqPausedTcBmpGet
(
 IN  GT_U8   devNum,
 IN  GT_U32  tileNum,
 IN  GT_U32  sdqNum,
 IN  GT_U32  localPortNum,
 OUT GT_U32  *pausedTcBmpPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(pausedTcBmpPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Pause_TC[localPortNum],
                 0,
                 32,
                 pausedTcBmpPtr);

    return rc;
}



/**
* @internal prvCpssFalconTxqSdqQueueAttributesGet function
* @endinternal
*
* @brief   Read Queue attributes
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] queueNumber              - queue number(APPLICABLE RANGES:0..399).
*
* @param[out] queueAttributesPtr      - pointer to queue attributes
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqQueueAttributesGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       queueNumber,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size,value,offset,tmpSize;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_AGING_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_AGING_E get failed\n");
        }

        queueAttributesPtr->agingThreshold = value <<PRV_TH_GRANULARITY_SHIFT;


        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_NEG_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_NEG_E get failed \n");
        }

        queueAttributesPtr->negativeCreditThreshold = value <<PRV_TH_GRANULARITY_SHIFT;

        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_LOW_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_LOW_E get failed \n");
        }

        queueAttributesPtr->lowCreditTheshold= value <<PRV_TH_GRANULARITY_SHIFT;


        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_HIGH_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_HIGH_E get failed \n");
        }

        queueAttributesPtr->highCreditThreshold= value <<PRV_TH_GRANULARITY_SHIFT;

        /*Now set semi-elig threshold*/

        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_SEMI_ELIG_E,&value);

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_SEMI_ELIG_E get failed \n");
        }

        queueAttributesPtr->semiEligThreshold= value <<PRV_TH_GRANULARITY_SHIFT;

        rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TC_E,&(queueAttributesPtr->tc));

        if(rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TC_E get failed \n");
        }

        return GT_OK;
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 0,
                 32,
                 regValue);


     if(rc!=GT_OK)
     {
        return rc;
     }

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
     }

     queueAttributesPtr->tc = U32_GET_FIELD_MAC(regValue[0],
        offset,
        size);

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNumber,
                                        regValue);
     if(rc!=GT_OK)
     {
        return rc;
     }

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_AGING_TH_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CFG_AGING_TH_FIELD_SIZE;
     }

     queueAttributesPtr->agingThreshold = U32_GET_FIELD_MAC(regValue[0],
        offset,
        size);

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_NEG_TH_FIELD_SIZE;
     }

     queueAttributesPtr->negativeCreditThreshold = U32_GET_FIELD_MAC(regValue[0],
        offset,
        size);

     queueAttributesPtr->negativeCreditThreshold <<=PRV_TH_GRANULARITY_SHIFT;

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_HIGH_TH_FIELD_SIZE;
     }

     queueAttributesPtr->highCreditThreshold = U32_GET_FIELD_MAC(regValue[1],
        offset-32,
        size);

     queueAttributesPtr->highCreditThreshold <<=PRV_TH_GRANULARITY_SHIFT;

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CFG_CRDT_LOW_TH_FIELD_SIZE;
     }

     tmpSize = 32 - offset;

     queueAttributesPtr->lowCreditTheshold = 0;


     queueAttributesPtr->lowCreditTheshold = U32_GET_FIELD_MAC(regValue[0],
             offset,
             tmpSize);

     value =  U32_GET_FIELD_MAC(regValue[1],
             0,
             size - tmpSize);

     queueAttributesPtr->lowCreditTheshold|= ((value)<<tmpSize);

     queueAttributesPtr->lowCreditTheshold <<=PRV_TH_GRANULARITY_SHIFT;

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_SEMI_ELIG_TH_0_FIELD_SIZE;
     }


     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                                             tileNum,
                                             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                                             offset,
                                             size,
                                             &queueAttributesPtr->semiEligThreshold);

     if(rc==GT_OK)
     {
        queueAttributesPtr->semiEligThreshold<<= PRV_ELIG_PRIO_TH_GRANULARITY_SHIFT;
     }


     return rc;


}


/**
* @internal prvCpssFalconTxqSdqLocalPortEnableSet function
* @endinternal
*
* @brief   Set port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                   - If equal GT_TRUE port is enabled at SDQ,else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortEnableSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;
    GT_U32 selig4DryEnableBit;
    GT_U32 regValue = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(localPortNum>= CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d] must be less than [%d]",localPortNum,CPSS_DXCH_SIP_6_MAX_LOCAL_PORT_NUM_MAC(devNum));
    }
    /*reserved ports  can still be configured to disable*/
    if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E&&GT_TRUE==enable)
    {
        if(PRV_CPSS_TXQ_HARRIER_RESERVED_PORTS_BMP_MAC&(1<<localPortNum))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "local portNum[%d]considered reserved - can not be enabled.",localPortNum);\
        }
    }

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_OFFSET),
                                      TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_SIZE),
                                      enable?1:0);

     U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_BP_EN_FIELD_OFFSET),
                                      TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_BP_EN_FIELD_SIZE),
                                      enable?1:0);
    /*relevant only for Ironman*/
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
      U32_SET_FIELD_MASKED_MAC(regValue,TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_GLBL_SLCT_EN_0_FIELD_OFFSET,
                                      TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_GLBL_SLCT_EN_0_FIELD_SIZE,
                                      enable?1:0);
    }


    if(GT_TRUE == enable)
    {

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_OFFSET),
                                           TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_SIZE),
                                           0);

         if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
         {
            /*check if the feature is enabled*/

            rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                 PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_OFFSET),
                 PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_SIZE),
                 &selig4DryEnableBit);

            if(rc!=GT_OK)
            {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " Error at   prvCpssSip6TxqRegisterFieldRead \n");
            }


             U32_SET_FIELD_MASKED_MAC(regValue,PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH0_0_FIELD_OFFSET),
                                      PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH0_0_FIELD_SIZE),
                                      (selig4DryEnableBit==1)?PRV_QDRYER_SELIG_TH>>PRV_QDRYER_GRANULARITY_SHIFT:0);

             U32_SET_FIELD_MASKED_MAC(regValue,PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH1_0_FIELD_OFFSET),
                                      PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,PORT_CONFIG_QDRYER_SELIG_TH1_0_FIELD_SIZE),
                                      (selig4DryEnableBit==1)?PRV_QDRYER_SELIG_TH>>PRV_QDRYER_GRANULARITY_SHIFT:0);


         }
    }



    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Config");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                 0,32,regValue);

    return  rc;

}

/**
* @internal prvCpssFalconTxqSdqLocalPortEnableGet function
* @endinternal
*
* @brief   Get port to enable state in SDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] enablePtr               - If equal GT_TRUE port is enabled at SDQ,else otherwise
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqLocalPortEnableGet
(
 IN GT_U8       devNum,
 IN GT_U32      tileNum,
 IN GT_U32      sdqNum,
 IN GT_U32      localPortNum,
 OUT GT_BOOL    * enablePtr
)
{

    GT_U32 value;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_PORT_EN_0_FIELD_SIZE),
                 &value);

     if(rc==GT_OK)
     {
        *enablePtr = (value==1)?GT_TRUE:GT_FALSE;
     }

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   -  port speed
* @param[out] queueAttributesPtr     - (pointer to) queue thresholds
*
* @retval GT_OK               - on success.
* @retval GT_FAIL             - no thresholds are configured for this speed
*/
static GT_STATUS prvCpssFalconTxqSdqInitQueueThresholdAttributes
(
    GT_U32                                          devNum,
    IN  CPSS_PORT_SPEED_ENT                         speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ATTRIBUTES * queueAttributesPtr
)
{
    GT_STATUS rc = GT_OK;

    queueAttributesPtr->agingThreshold = 0x0;
    queueAttributesPtr->negativeCreditThreshold = 0;
    queueAttributesPtr->lowCreditTheshold = 149000;
    queueAttributesPtr->highCreditThreshold = 150000;

    if(speed == CPSS_PORT_SPEED_NA_E)
    {
         /*Default initialization on power up. The speed is undefined yet*/
         queueAttributesPtr->semiEligThreshold =  0;
         queueAttributesPtr->negativeCreditThreshold = 0;
         queueAttributesPtr->lowCreditTheshold = (0x1FF) << PRV_TH_GRANULARITY_SHIFT;
         queueAttributesPtr->highCreditThreshold = (0x3FF) << PRV_TH_GRANULARITY_SHIFT;
    }
    else
    {
        rc = prvCpssSip6TxqSdqSemiEligThresholdGet(devNum,speed,&(queueAttributesPtr->semiEligThreshold));
        if(rc !=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
        }
    }
    return rc;
}

/**
* @internal prvCpssFalconTxqSdqInitQueueThresholdAttributes function
* @endinternal
*
* @brief   Initialize port thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] speed                   - port speed
* @param[out] portAttributesPtr      -(pointer to) port thresholds
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - no thresholds are configured for this speed
*/
static GT_STATUS prvCpssFalconTxqSdqInitPortAttributes
(
    IN  CPSS_PORT_SPEED_ENT                         speed,
    IN  PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
)
{
    GT_STATUS rc = GT_OK;


    switch(speed)
    {
        case CPSS_PORT_SPEED_10_E:
        case CPSS_PORT_SPEED_100_E:
        case CPSS_PORT_SPEED_1000_E:
            portAttributesPtr->portBackPressureLowThreshold = 14020;
            break;
        case CPSS_PORT_SPEED_10000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14200;
            break;
        case CPSS_PORT_SPEED_12000_E:
        case CPSS_PORT_SPEED_11800_E:
            portAttributesPtr->portBackPressureLowThreshold =  14240;
            break;
        case CPSS_PORT_SPEED_25000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14500;
            break;
        case CPSS_PORT_SPEED_40000_E:
        case CPSS_PORT_SPEED_42000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14800;
            break;
        case CPSS_PORT_SPEED_47200_E:
        case CPSS_PORT_SPEED_50000_E:
        case CPSS_PORT_SPEED_53000_E:
            portAttributesPtr->portBackPressureLowThreshold = 15000;
            break;
        case CPSS_PORT_SPEED_100G_E:
            portAttributesPtr->portBackPressureLowThreshold = 16000;
            break;
        case CPSS_PORT_SPEED_102G_E:
        case CPSS_PORT_SPEED_106G_E:
        case CPSS_PORT_SPEED_107G_E:
            portAttributesPtr->portBackPressureLowThreshold = 16140;
            break;
        case CPSS_PORT_SPEED_200G_E:
            portAttributesPtr->portBackPressureLowThreshold = 18000;
            break;
        case CPSS_PORT_SPEED_212G_E:
            portAttributesPtr->portBackPressureLowThreshold = 18280;
            break;
        case CPSS_PORT_SPEED_400G_E:
            portAttributesPtr->portBackPressureLowThreshold =  22000;
            break;
        case CPSS_PORT_SPEED_424G_E:
            portAttributesPtr->portBackPressureLowThreshold =  22560;
            break;
        case CPSS_PORT_SPEED_2500_E:
            portAttributesPtr->portBackPressureLowThreshold =  14050;
            break;
        case CPSS_PORT_SPEED_5000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14100;
            break;
        case CPSS_PORT_SPEED_20000_E:
            portAttributesPtr->portBackPressureLowThreshold =  14400;
            break;

         case CPSS_PORT_SPEED_800G_E:
            portAttributesPtr->portBackPressureLowThreshold =  24000;
            break;

        default :
            rc = GT_FAIL;
            break;
    }

    portAttributesPtr->portBackPressureHighThreshold = portAttributesPtr->portBackPressureLowThreshold;

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPortAttributesSet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
*
* @param[in] speed                 - port speed
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqPortAttributesSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               localPortNum,
    IN CPSS_PORT_SPEED_ENT  speed
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  portAttributes;

    GT_STATUS rc ;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);


    rc = prvCpssFalconTxqSdqInitPortAttributes(speed,&portAttributes);

    if(rc !=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqSdqInitQueueAttributes failed\n");
    }


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Back_Pressure_Low_Threshold");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPortNum;


    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_Low_Threshold[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE),
                 portAttributes.portBackPressureLowThreshold);

    if(rc !=GT_OK)
    {
       return rc;
    }

    log.regName = "Port_Back_Pressure_High_Threshold";

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_High_Threshold[localPortNum],
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET),
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE),
                     portAttributes.portBackPressureHighThreshold);


      return rc;
}

/**
* @internal prvCpssFalconTxqSdqPortAttributesGet function
* @endinternal
*
* @brief   Get port thresholds
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
*
* @param[out] portAttributesPtr       - (pointer to) port thresholds
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssFalconTxqSdqPortAttributesGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    OUT PRV_CPSS_DXCH_SIP6_TXQ_SDQ_PORT_ATTRIBUTES  * portAttributesPtr
)
{
       GT_STATUS rc ;

      TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

       TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

       TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);


        rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_Low_Threshold[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_LOW_THRESHOLD_PORT_BP_LOW_TH_FIELD_SIZE),
                 &(portAttributesPtr->portBackPressureLowThreshold));

     if(rc !=GT_OK)
     {
        return rc;
     }

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Back_Pressure_High_Threshold[localPortNum],
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_OFFSET),
                     TXQ_SDQ_FIELD_GET(devNum,PORT_BACK_PRESSURE_HIGH_THRESHOLD_PORT_BP_HIGH_TH_FIELD_SIZE),
                     &(portAttributesPtr->portBackPressureHighThreshold));


      return rc;
}


GT_STATUS prvCpssFalconTxqSdqQueueStrictPrioritySet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  sp
)
{
     GT_STATUS rc;
     GT_U32 offset,size,value = sp?1:0;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
     }

     if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
     {
         rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
             PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_PRIO_E,&value);

          if(rc != GT_OK)
          {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_PRIO_E set failed\n");
          }

          return GT_OK;
     }

     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }


     rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
     if (rc != GT_OK)
     {
         return rc;
     }

     log.regIndex = queueNumber;

     rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 offset,
                 size,
                 sp?1:0);

     return rc;


}

/**
* @internal prvCpssFalconTxqSdqQueueStrictPriorityGet function
* @endinternal
*
* @brief   Get queue strict priority bit
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - Then number of tile (Applicable range 0..3)
* @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] queueNumber           - local queue number  (APPLICABLE RANGES:0..399).
*
* @param[out] spPtr                - (pointer to)strict priority bit
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqQueueStrictPriorityGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  *spPtr
)
{
     GT_STATUS rc;
     GT_U32 tmp,offset,size;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
     }

     if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
     {
         rc = prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
             PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_PRIO_E,spPtr);

         if(rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_PRIO_E get failed\n");
         }

         return rc;
     }


     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
       offset = TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_PRIO_0_FIELD_SIZE;
     }

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 offset,
                 size,
                 &tmp);

     *spPtr= tmp?GT_TRUE:GT_FALSE;

     return rc;


}


GT_STATUS prvCpssFalconTxqSdqSelectListSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE    * rangePtr
)
{
    GT_STATUS rc;
    GT_U32    offset,size;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if(GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }


    if(rangePtr->prio0LowLimit >=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(rangePtr->prio0HighLimit >=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if((rangePtr->prio0LowLimit >rangePtr->prio0HighLimit)||
       (rangePtr->prio1LowLimit >rangePtr->prio1HighLimit))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Sel_List_Range_Low_0");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNum;


    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[0][localPortNum],
                 offset,
                 size,
                 rangePtr->prio0LowLimit);

    if(rc!=GT_OK)
    {
       return rc;
    }


    log.regName ="Sel_List_Range_Low_1";

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[1][localPortNum],
                 offset,
                 size,
                 rangePtr->prio1LowLimit);

    if(rc!=GT_OK)
    {
       return rc;
    }

    log.regName ="Sel_List_Range_High_0";


    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
     offset = TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
     offset = TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
     offset = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
     offset = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else
    {
     offset = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[0][localPortNum],
                 offset,
                 size,
                 rangePtr->prio0HighLimit);

     if(rc!=GT_OK)
     {
        return rc;
     }



     log.regName ="Sel_List_Range_High_1";

     rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[1][localPortNum],
                 offset,
                 size,
                 rangePtr->prio1HighLimit);

     return rc;
}


/**
* @internal prvCpssFalconTxqSdqSelectListGet function
* @endinternal
*
* @brief   Initialize queue thresholds for given speed
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] tileNum           - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum            - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum      - local port (APPLICABLE RANGES:0..8).
* @param[in] range             - the  range of the queues
*
* @retval GT_OK                - on success.
* @retval GT_FAIL              - no thresholds are configured for this speed
*/
GT_STATUS prvCpssFalconTxqSdqSelectListGet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       sdqNum,
    IN GT_U32                                       localPortNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_RANGE    * rangePtr
)
{
    GT_STATUS rc;
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_LOW_0_SEL_LIST_RANGE_LOW_0_FIELD_SIZE;
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
         tileNum,
         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[0][localPortNum],
         offset,
         size,
         &(rangePtr->prio0LowLimit));

    if(rc!=GT_OK)
    {
     return rc;
    }

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,
         tileNum,
         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_Low[1][localPortNum],
         offset,
         size,
         &(rangePtr->prio1LowLimit));

    if(rc!=GT_OK)
    {
      return rc;
    }

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
     offset = TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
     offset = TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_HAWK_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
     offset = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_PHOENIX_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
     offset = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_HARRIER_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }
    else
    {
     offset = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_OFFSET;
     size = TXQ_IRONMAN_SDQ_SEL_LIST_RANGE_HIGH_0_SEL_LIST_RANGE_HIGH_0_FIELD_SIZE;
    }

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,
         tileNum,
         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[0][localPortNum],
         offset,
         size,
         &(rangePtr->prio0HighLimit));

    if(rc!=GT_OK)
    {
      return rc;
    }

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,
         tileNum,
         PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Range_High[1][localPortNum],
         offset,
         size,
         &(rangePtr->prio1HighLimit));

    return rc;
}


GT_STATUS prvCpssFalconTxqSdqQCreditBalanceGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileNum,
    IN GT_U32       sdqNum,
    IN GT_U32       queueNum,
    OUT  GT_32      *balancePtr
)
{
    GT_STATUS rc;
    GT_U32    value,mask;

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_OFFSET),
                                        TXQ_SDQ_FIELD_GET(devNum,QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_SIZE),
                                        &value);
    if(GT_OK==rc)
    {
        mask = 1<<(TXQ_SDQ_FIELD_GET(devNum,QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_SIZE)-1);
        if(value&mask)
        {
            mask = (1<<(TXQ_SDQ_FIELD_GET(devNum,QUEUE_CRDT_BLNC_CRDT_BLNC_FIELD_SIZE)-1))-1;
            *balancePtr = (-1)*(value&mask);
        }
        else
        {
             *balancePtr = value;
        }
    }

     return rc;


}

GT_STATUS prvCpssFalconTxqSdqPortCreditBalanceGet
(
    IN GT_U8     devNum,
    IN GT_U32    tileNum,
    IN GT_U32    sdqNum,
    IN GT_U32    portNum,
    OUT  GT_U32  * balancePtr,
    OUT  GT_U32  * qCountPtr
)
{
    GT_STATUS rc;

    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_BLNC_FIELD_OFFSET,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_BLNC_FIELD_SIZE,
                                        balancePtr);
    if(rc!=GT_OK)
    {
        return rc;
    }

   rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_PORT_CRDT_BLNC_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        portNum,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_QCNT_FIELD_OFFSET,
                                        TXQ_SDQ_PORT_CRDT_BLNC_PORT_QCNT_FIELD_SIZE,
                                        qCountPtr);


    return rc;


}


GT_STATUS prvCpssFalconTxqSdqEligStateGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileNum,
    IN GT_U32       sdqNum,
    IN GT_U32       queueNum,
    OUT  GT_U32     * eligStatePtr
)
{
    GT_STATUS rc;

    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_ELIG_STATE_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        queueNum,
                                        eligStatePtr);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enable                - port/tc PFC responce enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueOffset,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueOffset>=1<<TXQ_SDQ_PFC_CONTROL_TC_EN0_FIELD_SIZE)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNumber);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"PFC_Control");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNumber;


    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PFC_CONTROL_TC_EN0_FIELD_OFFSET)+queueOffset,
                 1,
                 enable?1:0);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableGet function
* @endinternal
*
* @brief   Get enable /Disable  PFC response per port/TC.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enablePtr             - (pointer to)port/tc PFC responce enable option.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  queueNumber,
    IN GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32          regValue;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=(GT_U32)1<<TXQ_SDQ_FIELD_GET(devNum,PFC_CONTROL_TC_EN0_FIELD_SIZE))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


   rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 TXQ_SDQ_FIELD_GET(devNum,PFC_CONTROL_TC_EN0_FIELD_OFFSET)+queueNumber,
                 1,
                 &regValue);

     if(rc==GT_OK)
     {
        *enablePtr = (regValue == 1)?GT_TRUE:GT_FALSE;
     }

     return rc;
}

/**
* @internal prvCpssFalconTxqSdqQueueTcSet function
* @endinternal
*
* @brief   Sets PFC TC to queue map. Meaning this table define
*          which Q should be paused on reception of perticular TC
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset              - queue offset[0..15]
* @param[in] tc                       - tc that is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueTcSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_U32 tc
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(tc>= (1<<TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
             PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TC_E,&tc);

          if(rc != GT_OK)
          {
              CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TC_E set failed\n");
          }

          return GT_OK;
    }


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = queueNumber;

    if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
      offset = TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
      size = TXQ_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
      offset = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
      offset = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
    }
    else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
      offset = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
      size = TXQ_HARRIER_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
    }
    else
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_TC_0_FIELD_SIZE;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 offset,
                 size,
                 tc);


}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapSet function
* @endinternal
*
* @brief   Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enableBtmp            - port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32  enableBtmp
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"PFC_Control");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNumber;

    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 0,
                 32,
                 enableBtmp);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqPfcControlEnableBitmapGet  function
* @endinternal
*
* @brief   Get Enable /Disable  PFC response per port
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset           - queue offset[0..15]
* @param[in] enableBtmpPtr         - (pointer to)port PFC responce enable bitmap option.(each bit represent queue)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqPfcControlEnableBitmapGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPortNumber,
    IN GT_U32 *enableBtmpPtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].PFC_Control[localPortNumber],
                 0,
                 16,
                 enableBtmpPtr);

    return rc;
}

/**
* @internal prvCpssFalconTxqSdqLocalPortFlushSet function
* @endinternal
*
* @brief  Set port to "credit ignore" mode.This make port to transmit whenever there is data to send ignoring credits.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - physical device number
* @param[in] tileNum               - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          - local port (APPLICABLE RANGES:0..8).
* @param[in] enable                - Enable/disable "credit ignore" mode
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqLocalPortFlushSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Config");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPortNum;

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,PORT_CONFIG_CRDT_IGN_0_FIELD_SIZE),
                 enable?1:0);

    return  rc;
}

/**
* @internal prvCpssFalconTxqSdqQueueStatusDump function
* @endinternal
*
* @brief  Dump queue eligeble status in parsed format
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum           - physical device number
* @param[in] tileNum          - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum           - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] startQ           - first queue to dump
* @param[in] size             - number of queues to dump.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconTxqSdqQueueStatusDump
(
    IN  GT_U8                   devNum,
    IN  GT_U32                  tileNumber,
    IN  GT_U32                  sdqNum,
    IN  GT_U32                  startQ,
    IN  GT_U32                  size,
    IN  CPSS_OS_FILE_TYPE_STC   * file
)
{
    GT_U32           i;
    GT_STATUS        rc;
    GT_U32 eligState,credit,qcn,dry,inPortCnt,InSel,enq,SemiElig;
    PRV_CPSS_TXQ_UTILS_CHECK_REDIRECT_MAC(file);


    cpssOsFprintf(file->fd,"\n queue_elig_state tile %d dp %d\n",tileNumber,sdqNum);

    cpssOsFprintf(file->fd,"\n+-------------+------+---+--------+---------+-------+----+--------+");
    cpssOsFprintf(file->fd,"\n| queueNumber |credit|QCN|  DRY   |inPortCnt|InSel  |ENQ |SemiElig|");
    cpssOsFprintf(file->fd,"\n+-------------+------+---+--------+---------+-------+----+--------+");


    for(i =startQ;i<(startQ+size);i++)
    {

       rc = prvCpssFalconTxqSdqEligStateGet(devNum,tileNumber,sdqNum,i,&eligState);

        if(rc!=GT_OK)
        {
            cpssOsPrintf("%d = prvCpssFalconTxqSdqQueueAttributesGet\n",rc );
            return rc;
        }

        credit = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_CRDT_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_CRDT_FIELD_SIZE));

        qcn = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_QCN_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_QCN_FIELD_SIZE));

        dry = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_DRY_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_DRY_FIELD_SIZE));

        inPortCnt = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INPORTCNT_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INPORTCNT_FIELD_SIZE));

        InSel =  U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INSEL_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_INSEL_FIELD_SIZE));

        enq = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_ENQ_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_ENQ_FIELD_SIZE));

        SemiElig = U32_GET_FIELD_MAC(eligState,
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_SELIG_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,QUEUE_ELIG_STATE_SELIG_FIELD_SIZE));

        cpssOsFprintf(file->fd,"\n|%13d|%6d|%2d|%8d|%10d|%7d|%4d|%8d|",i,credit,qcn,dry,inPortCnt,InSel,enq,SemiElig);
        cpssOsFprintf(file->fd,"\n+-------------+------+---+--------+---------+-------+----+--------+");

    }

    cpssOsFprintf(file->fd,"\n");

    return GT_OK;
}

/**
* @internal prvCpssFalconTxqSdqSemiEligEnableSet function
* @endinternal
*
* @brief   Enable/disable semi elig feature
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] tileNum                  - the tile number  (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the global number of SDQ (APPLICABLE RANGES:0..31).
* @param[in] seligEnable              - if GT_TRUE semi elig feature is enabled, otherwise disabled
* @param[in] selig4DryEnable          - if GT_TRUE Enable selection slow down when semi elig by entering queue to the qdryer after selection,
*                                       otherwise disabled
* @param[in] granularity1kEnable      - Change Semi Elig threshold granule to 1KB. In that case, Semi Elig Threshold range will be 1KB-128KB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssFalconTxqSdqSemiEligEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_BOOL seligEnable,
    IN GT_BOOL selig4DryEnable,
    IN GT_BOOL granularity1kEnable
)
{
    GT_STATUS rc;


    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"global_config");
    if (rc != GT_OK)
    {
      return rc;
    }


    rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                 TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_SELIG_EN_FIELD_OFFSET),
                 TXQ_SDQ_FIELD_GET(devNum,GLOBAL_CONFIG_SELIG_EN_FIELD_SIZE),
                 seligEnable?1:0);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqSemiEligEnableSet  failed \n");
    }

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
            PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_OFFSET),
            PRV_SDQ_SIP_6_10_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG4DRY_EN_FIELD_SIZE),
            selig4DryEnable?1:0);

       if(rc!=GT_OK)
       {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqSemiEligEnableSet  failed \n");
       }

       if (PRV_CPSS_SIP_6_15_CHECK_MAC(devNum) == GT_TRUE)
       {
           rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                tileNum,&log,
                PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG_1K_EN_FIELD_OFFSET),
                PRV_SDQ_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,GLOBAL_CONFIG_SELIG_1K_EN_FIELD_SIZE),
                granularity1kEnable?1:0);

           if(rc!=GT_OK)
           {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqSdqSemiEligEnableSet  failed \n");
           }
       }

     }

     return rc;
 }

 /**
 * @internal prvCpssSip6_10TxqSdqSelectListEnableSet function
 * @endinternal
 *
 * @brief   Enable/disable select list range
 *
 * @note   APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
 *
 * @param[in] devNum                - device number
 * @param[in] tileNum               - Then number of tile (Applicable range 0..3)
 * @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
 * @param[in] localPortNum          - local port number  (APPLICABLE RANGES:0..26).
 * @param[in] lowPriority           - Defines the range.If equal GT_TRUE then range=0 ,else range=1.
 * @param[in] enable                - Enable/Disable select list range(0,1)
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
 GT_STATUS prvCpssSip6_10TxqSdqSelectListEnableSet
 (
     IN GT_U8   devNum,
     IN GT_U32  tileNum,
     IN GT_U32  sdqNum,
     IN GT_U32  localPortNum,
     IN GT_BOOL lowPriority,
     IN GT_BOOL enable
 )
 {
     GT_STATUS rc;
     GT_U32    index0,offset,size;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     /*Supported only from SIP 6.10*/
     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     /*Supported only until  SIP 7*/
     if(GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
      offset = TXQ_HAWK_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
      offset = TXQ_PHOENIX_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }

     index0 =lowPriority?0:1;

     rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,lowPriority?"Sel_List_Enable_0":"Sel_List_Enable_1");
     if (rc != GT_OK)
     {
       return rc;
     }

     log.regIndex = localPortNum;

     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Enable[index0][localPortNum],
                 offset,
                 size,
                 BOOL2BIT_MAC(enable));

      return rc;
 }

 /**
  * @internal prvCpssFalconTxqSdqSelectListEnableGet function
  * @endinternal
  *
  * @brief   Get enable/disable select list range
  *
  * @note   APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman
  * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon.
  *
  * @param[in] devNum                - device number
  * @param[in] tileNum               - Then number of tile (Applicable range 0..3)
  * @param[in] sdqNum                - the global number of SDQ (APPLICABLE RANGES:0..31).
  * @param[in] localPortNum          - local port number  (APPLICABLE RANGES:0..26).
  * @param[in] lowPriority           - Defines the range.If equal GT_TRUE then range=0 ,else range=1.
  * @param[out] enablePtr            -(pointer to)Enable/Disable select list range(0,1)
  *
  * @retval GT_OK                    - on success.
  * @retval GT_BAD_PARAM             - wrong sdq number.
  * @retval GT_HW_ERROR              - on writing to HW error.
  * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
  */
 GT_STATUS prvCpssFalconTxqSdqSelectListEnableGet
 (
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  sdqNum,
     IN  GT_U32  localPortNum,
     IN  GT_BOOL lowPriority,
     OUT GT_BOOL *enablePtr
 )
 {
     GT_STATUS rc;
     GT_U32    value;
     GT_U32    index0,offset,size;

     CPSS_NULL_PTR_CHECK_MAC(enablePtr);

     /*Supported only from SIP 6.10*/
     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

     if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
      offset = TXQ_HAWK_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
      size = TXQ_HAWK_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
      offset = TXQ_PHOENIX_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
      size = TXQ_PHOENIX_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
       size = TXQ_HARRIER_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_OFFSET;
       size = TXQ_IRONMAN_SDQ_SEL_LIST_ENABLE_0_SEL_LIST_EN_0_FIELD_SIZE;
     }

     index0 =lowPriority?0:1;

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Sel_List_Enable[index0][localPortNum],
                 offset,
                 size,
                 &value);

     *enablePtr = BIT2BOOL_MAC(value);

     return rc;
 }


 GT_STATUS prvCpssFalconTxqSdqSelectListPtrGet
 (
     IN  GT_U8   devNum,
     IN  GT_U32  tileNum,
     IN  GT_U32  sdqNum,
     IN  GT_U32  ind,
     OUT GT_U32 *valuePtr
 )
 {
     GT_STATUS rc;

     CPSS_NULL_PTR_CHECK_MAC(valuePtr);

     /*Supported only from SIP 6.10*/
     if(GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     /*Supported only until  SIP 7*/
     if(GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     rc = prvCpssDxChReadTableEntry(devNum,
                                     CPSS_DXCH_SIP6_TXQ_SDQ_SEL_LIST_PTRS_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                     ind,
                                     valuePtr);


     return rc;
 }

/**
* @internal prvCpssSip6TxqRegisterFieldWrite function
* @endinternal
*
* @brief   Function write to SDQ register.Also check that register is defined add tile offset
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] regAddr               - Adress of the register in SDQ
* @param[in] fieldOffset           - The start bit number in the register.
* @param[in] fieldLength           - The number of bits to be written to register.
* @param[in] fieldData             - Data to be written into the register.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqRegisterFieldWrite
(
     IN  GT_U8    devNum,
     IN  GT_U32   tileNum,
     IN PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
     IN  GT_U32   regAddr,
     IN  GT_U32   fieldOffset,
     IN  GT_U32   fieldLength,
     IN  GT_U32   fieldData
)
{
    GT_U32 tileOffset;
    GT_U32 mask = 0xFFFFFFFF;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED==regAddr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Adress is unitnitialized. \n");
    }

    if(fieldLength==0||fieldLength>PRV_TXQ_REG_SIZE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field length %d \n",fieldLength);
    }
    if(fieldOffset>PRV_TXQ_LAST_VALID_BIT_INDEX)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Incorrect field offset  %d \n",fieldOffset);
    }

    if((fieldLength<PRV_TXQ_REG_SIZE)&&(fieldData>(GT_U32)(1<<fieldLength)-1))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "fieldData is too big  0x%x must be smaller then 0x%x\n",fieldData,(1<<fieldLength)-1);
    }


    regAddr+=tileOffset;

    if(NULL!=logDataPtr&&GT_TRUE==logDataPtr->log)
    {
        if(fieldLength<32)
        {
            mask=(1<<fieldLength)-1;
            mask<<=fieldOffset;
        }

         cpssOsPrintf("\nWR  [addr = 0x%08x] REG TILE %d  %s       ",
            regAddr,tileNum,logDataPtr->unitName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->unitIndex)
        {
            cpssOsPrintf("%d,",
            logDataPtr->unitIndex);
        }

        cpssOsPrintf(" %s",logDataPtr->regName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->regIndex)
        {
            cpssOsPrintf("_%d",logDataPtr->regIndex);
        }

        cpssOsPrintf(" 0x%08x mask 0x%08x\n",fieldData<<fieldOffset,mask);


    }

    return prvCpssHwPpSetRegField(devNum,regAddr,fieldOffset,fieldLength,fieldData);
}


/**
* @internal prvCpssSip6TxqRegisterFieldRead function
* @endinternal
*
* @brief   Function read from  SDQ register.Also check that register is defined add tile offset.
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] regAddr               - Adress of the register in SDQ
* @param[in] fieldOffset           - The start bit number in the register.
* @param[in] fieldLength           - The number of bits to be read.
*
* @param[out] fieldDataPtr         - (pointer to) Data to read from the register.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqRegisterFieldRead
(
     IN   GT_U8    devNum,
     IN   GT_U32   tileNum,
     IN   GT_U32   regAddr,
     IN   GT_U32   fieldOffset,
     IN   GT_U32   fieldLength,
     OUT  GT_U32   *fieldDataPtr
)
{
    GT_U32 tileOffset;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);

    if(PRV_CPSS_SW_PTR_ENTRY_UNUSED==regAddr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Adress is unitnitialized. \n");
    }

    if(fieldLength==0||fieldLength>32)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field length %d \n",fieldLength);
    }
    if(fieldOffset>31)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, " Incorrect field offset  %d \n",fieldOffset);
    }


    regAddr+=tileOffset;

    return prvCpssHwPpGetRegField(devNum,regAddr,fieldOffset,fieldLength,fieldDataPtr);
}
GT_STATUS prvCpssSip6TxqWriteTableEntry
(
    IN GT_U8                  devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC *logDataPtr,
    IN CPSS_DXCH_TABLE_ENT    tableType,
    IN GT_U32                 entryIndex,
    IN GT_U32                  *entryValuePtr

)
{
    GT_U32 i,numOfWords;
    PRV_CPSS_DXCH_TABLES_INFO_STC *tableInfoPtr; /* pointer to table info */
    PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC *tablePtr;

    tableInfoPtr = PRV_TABLE_INFO_PTR_GET_MAC(devNum,tableType);
    tablePtr = (PRV_CPSS_DXCH_TABLES_INFO_DIRECT_STC*)(tableInfoPtr->writeTablePtr);

    numOfWords = tableInfoPtr->entrySize;

    if(NULL!=logDataPtr&&GT_TRUE==logDataPtr->log)
    {
        cpssOsPrintf("\nWR  [addr = 0x%08x] MEM TILE %d  %s       ",
            tablePtr->baseAddress+(tablePtr->step*entryIndex),
            logDataPtr->tileIndex,logDataPtr->unitName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->unitIndex)
        {
            cpssOsPrintf("%d,",
            logDataPtr->unitIndex);
        }

        cpssOsPrintf(" %s",logDataPtr->tableName);

        if(PRV_TXQ_LOG_NO_INDEX != logDataPtr->entryIndex)
        {
            cpssOsPrintf(" %d",logDataPtr->entryIndex);
        }

        for(i=0;i<numOfWords;i++)
        {
            cpssOsPrintf(" 0x%08x",*(entryValuePtr+i));
        }

        cpssOsPrintf(" mask 0xFFFFFFFF\n");
    }
    return prvCpssDxChWriteTableEntry(devNum,tableType,entryIndex,
       entryValuePtr);
}


GT_STATUS prvCpssPhoenixTxqSdqPortPfcStateGet
(
    IN GT_U8    devNum,
    IN GT_U32   localPortNum,
    IN GT_U32   *statePtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
    CPSS_NULL_PTR_CHECK_MAC(statePtr);

     /*Supported only from SIP 6.15*/
     if(GT_FALSE == PRV_CPSS_TXQ_LIKE_PHOENIX_MAC(devNum))
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
     }

     rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_15_TXQ_SDQ_PORT_PFC_STATE_E,
                                        localPortNum,
                                        statePtr);
     return rc;

}


static GT_STATUS prvCpssSip6TxqSdqSemiEligThresholdGet
(
     IN  GT_U8                          devNum,
     IN  CPSS_PORT_SPEED_ENT            speed,
     OUT GT_U32                         *thresholdPtr
)
{
    GT_U32    minSemiElig = 0,speed_in_K;
    GT_FLOAT semiEligTime,speed_in_G;

    CPSS_NULL_PTR_CHECK_MAC(thresholdPtr);


    speed_in_K = prvCpssCommonPortSpeedEnumToMbPerSecConvert(speed);
    speed_in_G = speed_in_K/1000.0;


     if(GT_TRUE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
        semiEligTime = 0.6;
     }
     else if(GT_TRUE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
         semiEligTime = 0.7;
         minSemiElig =_8K;
     }
     else
     {
        semiEligTime = 0.6;
     }

     /*From architectural defenition:
                      SEMI_ELIGE_TIME = 0.6  (AC5X =0.7)

               [@[PDQ P Level Speed]]*SEMI_ELIGE_TIME/(8*1000)) + min semi elig*/

     *thresholdPtr = (GT_U32)((((speed_in_G*semiEligTime)/8)*1000) + minSemiElig);

     return GT_OK;

}

/**
* @internal prvCpssSip6TxqSdqDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in SDQ
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqDebugInterruptDisableSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"SDQ_Interrupt_Summary_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Interrupt_Summary_Mask,
                 0,32,0);
}
/**
* @internal prvCpssSip6TxqSdqDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for SDQ
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqSdqDebugInterruptGet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   sdqNum,
    OUT GT_U32  *functionalPtr,
    OUT GT_U32  *debugPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(functionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(debugPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Interrupt_Functional_Cause,
                 0,32,functionalPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Interrupt_Debug_Cause,
                 0,32,debugPtr);

    return rc;
}

GT_U32 prvCpssSip6TxqSdqUnMapPortErrorBmp
(
    IN GT_U8  devNum
)
{
   GT_U32 bitOffset;
   GT_U32 bmp=0;

   bitOffset = TXQ_SDQ_FIELD_GET(devNum,SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_ENQ_FIELD_OFFSET);
   bmp =  1<<bitOffset;
   bitOffset = TXQ_SDQ_FIELD_GET(devNum,SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UNMAP_PORT_DEQ_FIELD_OFFSET);
   bmp |= (1<<bitOffset);

   return bmp;
}

/**
* @internal prvCpssSip6TxqSdqErrorCaptureGet function
* @endinternal
*
* @brief  Capture SDQ errors
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               -  physical device number
* @param[in] tileNum              -  the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum               -  the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] selectErrorValidPtr  -  (pointer to)GT_TRUE if select error is valid,GT_FALSE otherwise
* @param[in] selectErrorPortPtr   -  (pointer to) port that caused select error
* @param[in] selectErrorPortPtr   -  (pointer to) elig error raw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6TxqSdqErrorCaptureGet
(
 IN  GT_U8      devNum,
 IN  GT_U32     tileNum,
 IN  GT_U32     sdqNum,
 OUT GT_BOOL    *selectErrorValidPtr,
 OUT GT_U32     *selectErrorPortPtr,
 OUT GT_U32     *eligPtr
)
{
    GT_STATUS rc;
    GT_U32    data;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(selectErrorValidPtr);
    CPSS_NULL_PTR_CHECK_MAC(selectErrorPortPtr);
    CPSS_NULL_PTR_CHECK_MAC(eligPtr);


    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Select_Func_Error_Capture,
                 0,
                 32,
                 &data);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

     *selectErrorPortPtr =  U32_GET_FIELD_MAC(data,
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_PORT_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_PORT_FIELD_SIZE));

     *selectErrorValidPtr =  U32_GET_FIELD_MAC(data,
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_VALID_FIELD_OFFSET),
            TXQ_SDQ_FIELD_GET(devNum,SELECT_FUNC_ERROR_CAPTURE_SEL_ERROR_VALID_FIELD_SIZE));

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Elig_Func_Error_Capture,
                 0,
                 32,
                 eligPtr);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    return  rc;
}

GT_STATUS prvCpssSip6TxqSdqErrorCaptureDump
(
 IN  GT_U8 devNum,
 IN  GT_U32 tileNum,
 IN  GT_U32 sdqNum
)
{
   GT_BOOL selectErrorValid;
   GT_U32  selectPort,eligError;
   GT_STATUS rc;

   TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
   TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

   rc = prvCpssSip6TxqSdqErrorCaptureGet (devNum,tileNum,sdqNum,&selectErrorValid,&selectPort,&eligError);
   if(rc!=GT_OK)
   {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
   }

   cpssOsPrintf("selectErrorValid %d , selectPort %d ,elig error 0x%08x\n",selectErrorValid,selectPort,eligError);

   return GT_OK;
}

/**
* @internal prvCpssSip6_30TxqSdqQbvScanEnableSet function
* @endinternal
*
* @brief   Set QBV scan
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] enable                -enable/disable qbv scan
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqQbvScanEnableSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_BOOL enable
)
{
     GT_STATUS rc = GT_OK;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
     {

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
         TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


         rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"global_config");
         if (rc != GT_OK)
         {
           return rc;
         }

         rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].global_config,
                     TXQ_IRONMAN_SDQ_GLOBAL_CONFIG_QBV_SCAN_EN_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_GLOBAL_CONFIG_QBV_SCAN_EN_FIELD_SIZE,
                     enable?1:0);

         if(rc!=GT_OK)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQbvScanEnableSet  failed \n");
         }

     }

     return rc;
 }
 /**
 * @internal prvCpssSip6TxqSdqLocalPortConfigGet function
 * @endinternal
 *
 * @brief   Get port configuration in raw format
 *
 * @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                - device number
 * @param[in] tileNum               - The number of tile (Applicable range 0..3)
 * @param[in] sdqNum                - data path index
 * @param[out] dataPtr                -(pointer to)value of port configuration register
 *
 * @retval GT_OK                    - on success.
 * @retval GT_BAD_PARAM             - wrong sdq number.
 * @retval GT_HW_ERROR              - on writing to HW error.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
 GT_STATUS prvCpssSip6TxqSdqLocalPortConfigGet
 (
  IN GT_U8       devNum,
  IN GT_U32      tileNum,
  IN GT_U32      sdqNum,
  IN GT_U32      localPortNum,
  OUT GT_U32    * dataPtr
 )
 {

     GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);
     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     CPSS_NULL_PTR_CHECK_MAC(dataPtr);

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                  tileNum,
                  PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                  0,
                  32,
                  dataPtr);

     return rc;
 }



/**
* @internal prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet function
* @endinternal
*
* @brief   Set preemption type for SDQ port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] preemptive                - act as preemptive channel
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqLocalPortPreemptionTypeSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL preemptive
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Config");
       if (rc != GT_OK)
       {
         return rc;
       }

       log.regIndex = localPortNum;

       rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                     TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_TYPE_PRMPT_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_TYPE_PRMPT_0_FIELD_SIZE,preemptive?1:0);
    }

    return rc;
}




/**
* @internal prvCpssSip6_30TxqSdqLocalPortPreemptionTypeGet function
* @endinternal
*
* @brief   Get preemption type for SDQ port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] *preemptivePtr                - (pointer to) act as preemptive channel
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqLocalPortPreemptionTypeGet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL *preemptivePtr
)
{
    GT_STATUS rc = GT_NOT_APPLICABLE_DEVICE;

    GT_U32  tmp;

    CPSS_NULL_PTR_CHECK_MAC(preemptivePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Config[localPortNum],
                     TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_TYPE_PRMPT_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_CONFIG_PORT_TYPE_PRMPT_0_FIELD_SIZE,&tmp);

       *preemptivePtr = (tmp==1)?GT_TRUE:GT_FALSE;
    }

    return rc;
}


/**
* @internal prvCpssSip6_30TxqSdqLocalPortQbvEnableSet function
* @endinternal
*
* @brief   Enable/disable  QVB for  SDQ port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] enable                - QVB enable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqLocalPortQbvEnableSet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL enable
)
{
    GT_STATUS rc = GT_NOT_APPLICABLE_DEVICE;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"QBV_Config");
       if (rc != GT_OK)
       {
         return rc;
       }

       log.regIndex = localPortNum;

       rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].QBV_Config[localPortNum],
                     TXQ_IRONMAN_SDQ_QBV_CONFIG_PORT_QBV_EN_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_QBV_CONFIG_PORT_QBV_EN_0_FIELD_SIZE,enable?1:0);
    }

    return rc;
}




/**
* @internal prvCpssSip6_30TxqSdqLocalPortQbvEnableGet function
* @endinternal
*
* @brief   Get enable/disable  QVB for  SDQ port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] enable                - QVB enable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqLocalPortQbvEnableGet
(
 IN GT_U8   devNum,
 IN GT_U32  tileNum,
 IN GT_U32  sdqNum,
 IN GT_U32  localPortNum,
 IN GT_BOOL *enablePtr
)
{
    GT_STATUS rc = GT_NOT_APPLICABLE_DEVICE;
    GT_U32    tmp;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
       rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].QBV_Config[localPortNum],
                     TXQ_IRONMAN_SDQ_QBV_CONFIG_PORT_QBV_EN_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_QBV_CONFIG_PORT_QBV_EN_0_FIELD_SIZE,&tmp);

       *enablePtr = (tmp==1)?GT_TRUE:GT_FALSE;
    }

    return rc;
}


/**
* @internal prvCpssSip6_30TxqSdqQueueGateSet function
* @endinternal
*
* @brief   Sets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset              - queue offset
* @param[in] gate                       - gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqQueueGateSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_U32 gate
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        return prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QBV_GATE_E,&gate);
    }

    /*gate range will be checked at prvCpssSip6TxqRegisterFieldWrite*/

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"queue_config");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = queueNumber;

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_GT_0_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_GT_0_FIELD_SIZE,
                 gate);


}


/**
* @internal prvCpssSip6_30TxqSdqQueueGateGet function
* @endinternal
*
* @brief   Gets gate  to queue map. Meaning this register field define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum             - local port (APPLICABLE RANGES:0..8).
* @param[in] queueOffset              - queue offset
* @param[in] gatePtr                       - (pointer to)gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqQueueGateGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNumber,
    IN GT_U32 *gatePtr
)
{

    CPSS_NULL_PTR_CHECK_MAC(gatePtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        return prvCpssSip7TxqSdqQueueProfileAttrGet(devNum,tileNum,sdqNum,queueNumber,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QBV_GATE_E,gatePtr);
    }

    return prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[queueNumber],
                 TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_GT_0_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_QUEUE_CONFIG_QUEUE_GT_0_FIELD_SIZE,
                 gatePtr);


}

/**
* @internal prvCpssSip6_30TxqSdqChangeTimeSet function
* @endinternal
*
* @brief   Set time for table to start working
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] cfgtimePtr             -  (pointer to )configuration time in HW format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqChangeTimeSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QBV_CONFIG_CHANGE_TIME_HW_FORMAT_CFG * cfgtimePtr
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS                           rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(cfgtimePtr);

    /* range will be checked at prvCpssSip6TxqRegisterFieldWrite*/

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_TOD_MSB_High");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_TOD_MSB_High,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_TOD_MSB_HIGH_QBV_CHANGE_TIME_TOD_MSB_HIGH_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_TOD_MSB_HIGH_QBV_CHANGE_TIME_TOD_MSB_HIGH_FIELD_SIZE,
                 cfgtimePtr->msbHi);
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_TOD_MSB_Low");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_TOD_MSB_Low,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_TOD_MSB_LOW_QBV_CHANGE_TIME_TOD_MSB_LOW_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_TOD_MSB_LOW_QBV_CHANGE_TIME_TOD_MSB_LOW_FIELD_SIZE,
                 cfgtimePtr->msbLow);
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_TOD_LSB");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_TOD_LSB,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_TOD_LSB_QBV_CHANGE_TIME_TOD_LSB_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_TOD_LSB_QBV_CHANGE_TIME_TOD_LSB_FIELD_SIZE,
                 cfgtimePtr->lsb);
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_Ext");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_Ext,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_EXT_QBV_CHANGE_TIME_EXT_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_EXT_QBV_CHANGE_TIME_EXT_FIELD_SIZE,
                 cfgtimePtr->extTime);

    if (rc != GT_OK)
    {
      return rc;
    }

    return GT_OK;
}


/**
* @internal prvCpssSip6_30TxqSdqCycleTimeSet function
* @endinternal
*
* @brief   Set cycle time for table
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] cycleTime             -  Cycle time in nano seconds
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqCycleTimeSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 cycleTime
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_Cycle_Time");
    if (rc != GT_OK)
    {
      return rc;
    }

    /*range validation of cycleTime  is done inside prvCpssSip6TxqRegisterFieldWrite*/

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_Cycle_Time,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_CYCLE_TIME_QBV_CHANGE_TIME_CYCLE_TIME_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_CYCLE_TIME_QBV_CHANGE_TIME_CYCLE_TIME_FIELD_SIZE,
                 cycleTime);

}


/**
* @internal prvCpssSip6_30TxqSdqCycleTimeGet function
* @endinternal
*
* @brief   Get cycle time for table
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] cycleTimePtr             -  (pointer to)Cycle time in nano seconds
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqCycleTimeGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32  localPort,
    IN GT_U32  *cycleTimePtr
)
{
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    CPSS_NULL_PTR_CHECK_MAC(cycleTimePtr);

    return prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_TableSet_Cycle_Time[localPort],
                 TXQ_IRONMAN_SDQ_PORT_TABLESET_CYCLE_TIME_PORT_TABLESET_CYC_TIME_0_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_PORT_TABLESET_CYCLE_TIME_PORT_TABLESET_CYC_TIME_0_FIELD_SIZE,
                 cycleTimePtr);

}


/**
* @internal prvCpssSip6_30TxqSdqEgressTodOffsetSet function
* @endinternal
*
* @brief   Set TX jitter
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[out] egressTodOffset    Egress TOD offset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqEgressTodOffsetSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 egressTodOffset
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_Egress_TOD_Offset");
    if (rc != GT_OK)
    {
      return rc;
    }

    /*range validation of egressTodOffset  is done inside prvCpssSip6TxqRegisterFieldWrite*/

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_Egress_TOD_Offset,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_EGRESS_TOD_OFFSET_QBV_CHANGE_TIME_EGRESS_TOD_OFFSET_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_EGRESS_TOD_OFFSET_QBV_CHANGE_TIME_EGRESS_TOD_OFFSET_FIELD_SIZE,
                 egressTodOffset);

}

/**
* @internal prvCpssSip6_30TxqSdqEgressTodOffsetGet function
* @endinternal
*
* @brief   Get TX jitter
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] localPortNum                   - the local port number of SDQ
* @param[out] egressTodOffsetPtr             -  (pointer to)Egress TOD offset
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqEgressTodOffsetGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPortNum,
    OUT GT_U32 *egressTodOffsetPtr
)
{

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    return prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Egress_TOD_Offset[localPortNum],
                 TXQ_IRONMAN_SDQ_PORT_EGRESS_TOD_OFFSET_PORT_EGRESS_TOD_OFFSET_0_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_PORT_EGRESS_TOD_OFFSET_PORT_EGRESS_TOD_OFFSET_0_FIELD_SIZE,
                 egressTodOffsetPtr);

}

/**
* @internal prvCpssSip6_30TxqSdqChangeReqSet function
* @endinternal
*
* @brief   Set change request parameters
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] configReqPtr             -  (pointer to) config request
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqChangeReqSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32  localPort,
    IN GT_U32  tableSet,
    IN CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_REQ_STC * configReqPtr
)
{
    GT_STATUS rc;
    GT_U32    data =0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    CPSS_NULL_PTR_CHECK_MAC(configReqPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);


    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Config_Change_Time_Req");
    if (rc != GT_OK)
    {
      return rc;
    }


   if(tableSet >PRV_TXQ_TABLE_SET_NUM_CNS)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Table set is too big %d ,must be smaller then %d\n",
        tableSet,PRV_TXQ_TABLE_SET_NUM_CNS);
   }

   if(configReqPtr->lastEntry >= (GT_U32)(1<<TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_LAST_ENTRY_FIELD_SIZE))
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Last entry index is  too big %d ,must be smaller then %d\n",
        configReqPtr->lastEntry,(GT_U32)(1<<TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_LAST_ENTRY_FIELD_SIZE));
   }

   if(configReqPtr->remainingBitsFactor >= (GT_U32)(1<<TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_REMAIN_PER_NS_FIELD_SIZE))
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "remainingBitsFactor is  too big %d ,must be smaller then %d\n",
        configReqPtr->remainingBitsFactor ,(GT_U32)(1<<TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_REMAIN_PER_NS_FIELD_SIZE));
   }

   /*Fill data*/

   U32_SET_FIELD_MASKED_MAC(data, TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_OFFSET,
                                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_SIZE,
                                 1);


   U32_SET_FIELD_MASKED_MAC(data, TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_PORT_FIELD_OFFSET,
                                  TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_PORT_FIELD_SIZE,
                                    localPort);

   U32_SET_FIELD_MASKED_MAC(data, TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TABLESET_FIELD_OFFSET,
                                  TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TABLESET_FIELD_SIZE,
                                    tableSet);

   U32_SET_FIELD_MASKED_MAC(data, TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_LAST_ENTRY_FIELD_OFFSET,
                                  TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_LAST_ENTRY_FIELD_SIZE,
                                   configReqPtr->lastEntry);

   U32_SET_FIELD_MASKED_MAC(data, TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_LENGTH_AWR_EN__FIELD_OFFSET,
                                  TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_LENGTH_AWR_EN__FIELD_SIZE,
                                  configReqPtr->lengthAware?1:0);

   U32_SET_FIELD_MASKED_MAC(data, TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_REMAIN_PER_NS_FIELD_OFFSET,
                                  TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_REMAIN_PER_NS_FIELD_OFFSET,
                                  configReqPtr->remainingBitsFactor);

   return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_Req,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_OFFSET,
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_REMAIN_PER_NS_FIELD_OFFSET+
                 TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_REMAIN_PER_NS_FIELD_SIZE,
                 data);

}


/**
* @internal prvCpssSip6_30TxqSdqQbvCalendarSet function
* @endinternal
*
* @brief   Set entry at specific tableset /specific index
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] tableSet             -  tabelset index
* @param[in] index             -        index within tableset
* @param[in] gateStateBmp    -  Gate state (1 -close /0-open). Bit 9 is hold/release (- -release/1- hold)
* @param[in] isCurrent             -  whether current part or next part should be updated at the entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqQbvCalendarSet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               tableSet,
    IN GT_U32               index,
    IN GT_U32               timeToAdvance,
    IN GT_U32               gateStateBmp,
    IN GT_BOOL              isCurrent
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32 size;
    GT_U32 value;

    CPSS_DXCH_TABLE_ENT table = CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E;

    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    /*no need to check timeToAdvance - it is 32 bit*/

    /*additional bit for hold/release*/
    if(gateStateBmp >= (GT_U32)(1 << (TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE+1)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(devNum,table,
                                        tableSet*PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS+index,
                                        regValue);
    if(rc!=GT_OK)
    {
       return rc;
    }

    if(GT_TRUE == isCurrent)
    {
        size = 32 - TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_OFFSET;
        value = timeToAdvance&((1<<size)-1);

        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_OFFSET,
                                 size,
                                 value);

        value = timeToAdvance>>size;
        size = TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_SIZE -size;
        U32_SET_FIELD_MASKED_MAC(regValue[1],0,
                                 size,
                                 value);

        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_OFFSET,
                                 TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE,
                                 gateStateBmp&((1<<TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE)-1));

        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_IRONMAN_SDQ_QBV_CFG_HOLD_RELEASE_CURRENT_FIELD_OFFSET,
                                 TXQ_IRONMAN_SDQ_QBV_CFG_HOLD_RELEASE_CURRENT_FIELD_SIZE,
                                 gateStateBmp >>TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE);
    }
    else
    {
        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_NEXT_FIELD_OFFSET,
                                 TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_NEXT_FIELD_SIZE,
                                 gateStateBmp&((1<<TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_NEXT_FIELD_SIZE)-1));

        U32_SET_FIELD_MASKED_MAC(regValue[0],TXQ_IRONMAN_SDQ_QBV_CFG_HOLD_RELEASE_NEXT_FIELD_OFFSET,
                                 TXQ_IRONMAN_SDQ_QBV_CFG_HOLD_RELEASE_NEXT_FIELD_SIZE,
                                 gateStateBmp >>TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_NEXT_FIELD_SIZE);
    }

    rc = prvCpssSip6TxqSdqTableLogInit(&tableLog,tileNum,sdqNum,table,"TBL_Port_QBV_Conf",tableSet*PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS+index);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                                    table,
                                    tableSet*PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS+index,
                                    regValue);


    return rc;
}


/**
* @internal prvCpssSip6_30TxqSdqQbvCalendarGet function
* @endinternal
*
* @brief   Get entry at specific tableset /specific index
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] tableSet             -  tabelset index
* @param[in] index             -        index within tableset
* @param[out] timeSlotPtr    -  (pointer to) entry data
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqQbvCalendarGet
(
    IN  GT_U8                devNum,
    IN  GT_U32               tileNum,
    IN  GT_U32               sdqNum,
    IN  GT_U32               tableSet,
    IN  GT_U32               index,
    OUT CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotPtr
)
{
    GT_STATUS rc;

    CPSS_DXCH_TABLE_ENT table = CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E;
    GT_U32 regValue[2],tmpValue,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(timeSlotPtr);

    rc = prvCpssDxChReadTableEntry(devNum,table,
                                        tableSet*PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS+index,
                                        regValue);
    if(rc!=GT_OK)
    {
       return rc;
    }

   timeSlotPtr->gateStateBmp = U32_GET_FIELD_MAC(regValue[0],
        TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_OFFSET,
        TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE);

   tmpValue = U32_GET_FIELD_MAC(regValue[0],
        TXQ_IRONMAN_SDQ_QBV_CFG_HOLD_RELEASE_CURRENT_FIELD_OFFSET,
        TXQ_IRONMAN_SDQ_QBV_CFG_HOLD_RELEASE_CURRENT_FIELD_SIZE);

   timeSlotPtr->hold = (tmpValue==0x1)?GT_TRUE:GT_FALSE;

   size = TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_SIZE -(32 - TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_OFFSET);
   tmpValue = U32_GET_FIELD_MAC(regValue[1],0, size);

   size = 32 - TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_OFFSET;

   timeSlotPtr->timeToAdvance= U32_GET_FIELD_MAC(regValue[0],
        TXQ_IRONMAN_SDQ_QBV_CFG_TIME_TO_ADVANCE_FIELD_OFFSET,
        size);

  timeSlotPtr->timeToAdvance |=(tmpValue<<size);

   return rc;
}

/**
* @internal prvCpssSip6_30TxqSdqQbvCalendarRawDataGet function
* @endinternal
*
* @brief   Get entry at specific tableset /specific index
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] tableSet             -  tabelset index
* @param[in] index             -        index within tableset
* @param[out] dataPtr    -  (pointer to) entry data in raw format
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS prvCpssSip6_30TxqSdqQbvCalendarRawDataGet
(
    IN GT_U8                devNum,
    IN GT_U32               tileNum,
    IN GT_U32               sdqNum,
    IN GT_U32               tableSet,
    IN GT_U32               index,
    OUT GT_U32              *dataPtr
)
{
    GT_STATUS rc;

    CPSS_DXCH_TABLE_ENT table = CPSS_DXCH_SIP6_30_TXQ_SDQ_QBV_CFG_E;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(dataPtr);

    rc = prvCpssDxChReadTableEntry(devNum,table,
                                        tableSet*PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS+index,
                                        dataPtr);
    if(rc!=GT_OK)
    {
       return rc;
    }

    return rc;
}



/**
* @internal prvCpssSip6_30TxqSdqChangeReqCompleteSet function
* @endinternal
*
* @brief  Reset  status of configuration pending bit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqChangeReqCompleteSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,NULL,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_Req,
                     TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_SIZE,
                     0x0);
    if(rc!=GT_OK)
    {
       return rc;
    }

    return rc;
}


/**
* @internal prvCpssSip6_30TxqSdqChangeReqCompleteGet function
* @endinternal
*
* @brief  Polls on status of configuration pending bit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[out] isCompletePtr    -  (pointer to) variable that reflect whether HW loaded the last request
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqChangeReqCompleteGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    OUT GT_BOOL *isCompletePtr
)
{
    GT_STATUS rc;
    GT_U32    data;

    CPSS_NULL_PTR_CHECK_MAC(isCompletePtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(isCompletePtr);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Config_Change_Time_Req,
                     TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_CONFIG_CHANGE_TIME_REQ_QBV_CHANGE_TIME_PEND_FIELD_SIZE,
                     &data);
    if(rc!=GT_OK)
    {
       return rc;
    }

    *isCompletePtr = (data==0)?GT_TRUE:GT_FALSE;

    return rc;
}
/**
* @internal prvCpssSip6_30TxqSdqPortTableSetGet function
* @endinternal
*
* @brief  Get tableset and last entry from HW (Read only debug registers)
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[out] tableSetIndexPtr    -  (pointer to)tableset index
* @param[out] tableSetLastEntryPtr    -  (pointer to)tableset last entry
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqPortTableSetGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPort,
    IN GT_U32  *tableSetIndexPtr,
    IN GT_U32  *tableSetLastEntryPtr
)
{
    GT_STATUS rc;
    GT_U32    data;

    CPSS_NULL_PTR_CHECK_MAC(tableSetIndexPtr);
    CPSS_NULL_PTR_CHECK_MAC(tableSetLastEntryPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc =  prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_QBV_TableSet[localPort],
                     0,
                     32,
                     &data);

    if(rc!=GT_OK)
    {
       return rc;
    }

    *tableSetIndexPtr = U32_GET_FIELD_MAC(data,
        TXQ_IRONMAN_SDQ_PORT_QBV_TABLESET_TABLE_SET_INDEX_0_FIELD_OFFSET,
        TXQ_IRONMAN_SDQ_PORT_QBV_TABLESET_TABLE_SET_INDEX_0_FIELD_SIZE);

     *tableSetLastEntryPtr = U32_GET_FIELD_MAC(data,
        TXQ_IRONMAN_SDQ_PORT_QBV_TABLESET_LAST_TBL_SET_ENTRY_0_FIELD_OFFSET,
        TXQ_IRONMAN_SDQ_PORT_QBV_TABLESET_LAST_TBL_SET_ENTRY_0_FIELD_SIZE);

     return GT_OK;


}


/**
* @internal prvCpssSip6_30TxqSdqPortLengthAwareGet function
* @endinternal
*
* @brief  Get port lentgth aware enable from HW (Read only debug registers)
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[out] lengthAwarePtr    -  (pointer to)length aware enable bit
* @param[out] remainingBitsFactorPtr    -  (pointer to)Reflects Configured Port speed: Number of bits per resolution.
*                                                Used to compute MaxByteCount that can be transmitted till the end of current QBV window.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqPortLengthAwareGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  localPort,
    OUT GT_BOOL  *lengthAwarePtr,
    OUT GT_U32   *remainingBitsFactorPtr
)
{
    GT_STATUS rc;
    GT_U32    data;

    CPSS_NULL_PTR_CHECK_MAC(lengthAwarePtr);
    CPSS_NULL_PTR_CHECK_MAC(remainingBitsFactorPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc =  prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Length_Awr_Attr[localPort],
                     0,
                     32,
                     &data);

    if(rc!=GT_OK)
    {
       return rc;
    }

     *remainingBitsFactorPtr = U32_GET_FIELD_MAC(data,
        TXQ_IRONMAN_SDQ_PORT_LENGTH_AWR_ATTR_PORT_REMAIN_PER_NS_0_FIELD_OFFSET,
        TXQ_IRONMAN_SDQ_PORT_LENGTH_AWR_ATTR_PORT_REMAIN_PER_NS_0_FIELD_SIZE);

     data  = U32_GET_FIELD_MAC(data,
        TXQ_IRONMAN_SDQ_PORT_LENGTH_AWR_ATTR_PORT_LENGTH_AWARE_EN_0_FIELD_OFFSET,
        TXQ_IRONMAN_SDQ_PORT_LENGTH_AWR_ATTR_PORT_LENGTH_AWARE_EN_0_FIELD_SIZE);

    *lengthAwarePtr = (data==0x1)?GT_TRUE:GT_FALSE;

     return GT_OK;


}

/**
* @internal prvCpssSip6_30TxqSdqBitsFactorResolutionSet function
* @endinternal
*
* @brief  Define ns resolution of remain_per_ns field in config_change_time_req register
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] bitsFactorResolution    - Bits resolution(APPLICABLE RANGES:0..2^16-1)
*                                                                   0 - defines port speed: bits in 1 ns
*                                                                   1- defines port speed: bits in 2 ns
*                                                                   2 - defines port speed: bits in 4 ns and so on.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqBitsFactorResolutionSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPort,
    IN GT_U32 bitsFactorResolution
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Remain_Per_NS_Resolution");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPort;

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Remain_Per_NS_Resolution[localPort],
                     TXQ_IRONMAN_SDQ_PORT_REMAIN_PER_NS_RESOLUTION_PORT_REMAIN_PER_NS_RES_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_REMAIN_PER_NS_RESOLUTION_PORT_REMAIN_PER_NS_RES_0_FIELD_SIZE,
                     bitsFactorResolution);

}

/**
* @internal prvCpssSip6_30TxqSdqBitsFactorResolutionGet function
* @endinternal
*
* @brief  Get definition of  ns resolution of remain_per_ns field in config_change_time_req register
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  - the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[out ] bitsFactorResolutionPtr    - (pointer to)Bits resolution(APPLICABLE RANGES:0..2^16-1)
*                                                                   0 - defines port speed: bits in 1 ns
*                                                                   1- defines port speed: bits in 2 ns
*                                                                   2 - defines port speed: bits in 4 ns and so on.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/

GT_STATUS prvCpssSip6_30TxqSdqBitsFactorResolutionGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPort,
    OUT GT_U32 *bitsFactorResolutionPtr
)
{

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);


    CPSS_NULL_PTR_CHECK_MAC(bitsFactorResolutionPtr);


    return prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Remain_Per_NS_Resolution[localPort],
                     TXQ_IRONMAN_SDQ_PORT_REMAIN_PER_NS_RESOLUTION_PORT_REMAIN_PER_NS_RES_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_REMAIN_PER_NS_RESOLUTION_PORT_REMAIN_PER_NS_RES_0_FIELD_SIZE,
                     bitsFactorResolutionPtr);

}
/**
* @internal prvCpssSip6_30TxqSdqBitsFactorResolutionSet function
* @endinternal
*
* @brief  Define Port MAX allowed BC Offset
*   Configured in Bytes
*   Amount of configured Byte will be reduced from computed Max allowed BC
*   Computed Max allowed BC is port speed and QBV time remain outcome
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  -   the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] maxAlwdBcOffset    -Port MAX allowed BC Offset
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqMaxAlwdBcOffsetSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPort,
    IN GT_U32 maxAlwdBcOffset
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"Port_Max_Alwd_BC_Offset");
    if (rc != GT_OK)
    {
        return rc;
    }

    log.regIndex = localPort;

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Max_Alwd_BC_Offset[localPort],
                     TXQ_IRONMAN_SDQ_PORT_MAX_ALWD_BC_OFFSET_PORT_MAX_ALWD_BC_OFFSET_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_MAX_ALWD_BC_OFFSET_PORT_MAX_ALWD_BC_OFFSET_0_FIELD_SIZE,
                     maxAlwdBcOffset);

}

/**
* @internal prvCpssSip6_30TxqSdqBMaxAlwdBcOffsetGet function
* @endinternal
*
* @brief  Get definition of  Port MAX allowed BC Offset
*   Configured in Bytes
*   Amount of configured Byte will be reduced from computed Max allowed BC
*   Computed Max allowed BC is port speed and QBV time remain outcome
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tileNum                  -   the tile numebr (APPLICABLE RANGES:0..3).
* @param[in] sdqNum                   - the local number of SDQ (APPLICABLE RANGES:0..7).
* @param[out] maxAlwdBcOffsetPtr    -Port MAX allowed BC Offset
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqSdqBMaxAlwdBcOffsetGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 localPort,
    OUT GT_U32 *maxAlwdBcOffsetPtr
)
{

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);


    CPSS_NULL_PTR_CHECK_MAC(maxAlwdBcOffsetPtr);


    return prvCpssSip6TxqRegisterFieldRead(devNum,
                     tileNum,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Port_Max_Alwd_BC_Offset[localPort],
                     TXQ_IRONMAN_SDQ_PORT_MAX_ALWD_BC_OFFSET_PORT_MAX_ALWD_BC_OFFSET_0_FIELD_OFFSET,
                     TXQ_IRONMAN_SDQ_PORT_MAX_ALWD_BC_OFFSET_PORT_MAX_ALWD_BC_OFFSET_0_FIELD_SIZE,
                     maxAlwdBcOffsetPtr);

}

/**
* @internal prvCpssSip6_30TxqSdqQbvScanEnableSet function
* @endinternal
*
* @brief    Set metal fix register
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] bitOffset                - bit offset
* @param[in] value                -bit value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqSdqMetalFixBitSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  bitOffset,
    IN GT_U32  value
)
{
     GT_STATUS rc = GT_OK;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
     {

         TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
         TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

         rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,"SDQ_Metal_Fix");
         if (rc != GT_OK)
         {
           return rc;
         }

         rc =prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].SDQ_Metal_Fix,
                     bitOffset,
                     1,
                     value);

         if(rc!=GT_OK)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqMetalFixSet  failed \n");
         }

     }

     return rc;
 }
/**
* @internal prvCpssSip7TxqSdqCopyQueueAttributes function
* @endinternal
*
* @brief    This function is used in case of queue number is changed.
*           New queues should use the same parameters as the old ones,hence the attributes copy should be performed.
*
* @note   APPLICABLE DEVICES: Seahawk;
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin;
*        AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier;Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - The local number of SDQ (APPLICABLE RANGES:0..7).
* @param[in] sourceBase            - Source first queue
* @param[in] sourceNumberOfQueues  - Old number of queues
* @param[in] targetBase            - Target  first queue
* @param[in] targetNumberOfQueues  - New number of queues
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqSdqCopyQueueAttributes
(
  IN GT_U8                                        devNum,
  IN GT_U32                                       tileNum,
  IN GT_U32                                       sdqNum,
  IN GT_U32                                       sourceBase,
  IN GT_U32                                       sourceNumberOfQueues,
  IN GT_U32                                       targetBase,
  IN GT_U32                                       targetNumberOfQueues
)
{
    GT_U32 regValue[2];
    GT_STATUS rc;
    GT_U32    i,queuesToCopy;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(sourceBase>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(targetBase>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        TBD_SIP7_SDQ_PROFILES
        return GT_OK;
    }

    /*the assumption is that all queues have same threshold configurations*/

    rc = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        sourceBase,
                                        regValue);
    if(rc!=GT_OK)
    {
       return rc;
    }

    for(i=0;i<targetNumberOfQueues;i++)
    {
         rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E+sdqNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_SDQ_NUM_MAC,
                                        targetBase+i,
                                        regValue);


         if(rc!=GT_OK)
         {
            return rc;
         }
    }

    queuesToCopy = targetNumberOfQueues<sourceNumberOfQueues?targetNumberOfQueues:sourceNumberOfQueues;
    for(i=0;i<queuesToCopy;i++)
    {
         rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[sourceBase+i],
                 0,
                 32,
                 regValue);


         if(rc!=GT_OK)
         {
            return rc;
         }

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     tileNum,NULL,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].queue_config[targetBase+i],
                     0,
                     32,
                     regValue[0]);


         if(rc!=GT_OK)
         {
            return rc;
         }
    }

    return GT_OK;
}

static GT_STATUS prvCpssSip7TxqSdqProfileAttrParamsGet
(
    IN  GT_U8        devNum,
    IN  GT_U32        sdqNum,
    IN  PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT  attr,
    IN  GT_U32        profile,
    OUT GT_U32       *offsetPtr,
    OUT GT_U32       *sizePtr,
    OUT GT_U32       *regAddrPtr,
    OUT GT_CHAR      **namePtrPtr
)
{

   if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AAS_E)
   {
     CPSS_LOG_ERROR_MAC(" prvCpssSip7TxqSdqProfileSet  not adapted for device family %d \n",PRV_CPSS_PP_MAC(devNum)->devFamily);
   }

   CPSS_NULL_PTR_CHECK_MAC(offsetPtr);
   CPSS_NULL_PTR_CHECK_MAC(sizePtr);
   CPSS_NULL_PTR_CHECK_MAC(regAddrPtr);

   switch(attr)
   {
       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TC_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Queue_TC[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_QUEUE_TC_QUEUE_TC_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_QUEUE_TC_QUEUE_TC_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Queue_TC";
        break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_SEMI_ELIG_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Q_Semi_Elig_Threshold[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_Q_SEMI_ELIG_THRESHOLD_SEMI_ELIG_TH_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_Q_SEMI_ELIG_THRESHOLD_SEMI_ELIG_TH_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Q_Semi_Elig_Threshold";
        break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QCN_DIV_FACTOR_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_QCN_Div_Factor[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_QCN_DIV_FACTOR_QCN_DIV_FACTOR_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_QCN_DIV_FACTOR_QCN_DIV_FACTOR_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_QCN_Div_Factor";
        break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_PRIO_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Queue_Prio[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_QUEUE_PRIO_QUEUE_PRIO_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_QUEUE_PRIO_QUEUE_PRIO_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Queue_Prio";
        break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_QBV_GATE_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].profile_queue_gate[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_QUEUE_GATE_QUEUE_GATE_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_QUEUE_GATE_QUEUE_GATE_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "profile_queue_gate";
         break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_AVB_EN_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Queue_AVB_en[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_QUEUE_AVB_EN_QUEUE_AVB_EN_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_QUEUE_AVB_EN_QUEUE_AVB_EN_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Queue_AVB_en";
         break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_AGING_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Q_Aging_Credit_Threshold[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_Q_AGING_CREDIT_THRESHOLD_AGING_CRDT_TH_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_Q_AGING_CREDIT_THRESHOLD_AGING_CRDT_TH_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Q_Aging_Credit_Threshold";
         break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_HIGH_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Q_High_Credit_Threshold[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_Q_HIGH_CREDIT_THRESHOLD_HIGH_CRDT_TH_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_Q_HIGH_CREDIT_THRESHOLD_HIGH_CRDT_TH_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Q_High_Credit_Threshold";
         break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_LOW_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Q_Low_Credit_Threshold[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_Q_LOW_CREDIT_THRESHOLD_LOW_CRDT_TH_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_Q_LOW_CREDIT_THRESHOLD_LOW_CRDT_TH_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Q_Low_Credit_Threshold";
         break;

       case PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_TH_NEG_E:
         *regAddrPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.sdq[sdqNum].Profile_Q_Neg_Credit_Threshold[profile];
         *offsetPtr = TXQ_AAS_SDQ_PROFILE_Q_NEG_CREDIT_THRESHOLD_NEG_CRDT_TH_PROFILE_0_FIELD_OFFSET;
         *sizePtr = TXQ_AAS_SDQ_PROFILE_Q_NEG_CREDIT_THRESHOLD_NEG_CRDT_TH_PROFILE_0_FIELD_SIZE;
         if(NULL!=namePtrPtr)*namePtrPtr = "Profile_Q_Neg_Credit_Threshold";
         break;

        default:
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong attr value %d \n",attr);
        }
        break;
   }

   return GT_OK;
}
GT_STATUS prvCpssSip7TxqSdqProfileSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT  attr,
    IN GT_U32  profile,
    IN GT_U32  value
)
{
   GT_STATUS rc = GT_OK;
   PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
   GT_U32   offset,size,regAddr;
   GT_CHAR * name;

   TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
   TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

   if(profile>=SDQ_PROFILE_MAX_MAC)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Profile index %d is bigger then max allowed %d\n",profile,SDQ_PROFILE_MAX_MAC-1);
   }


   rc = prvCpssSip7TxqSdqProfileAttrParamsGet(devNum,sdqNum,attr,profile,&offset,&size,&regAddr,&name);
   if (rc != GT_OK)
   {
     return rc;
   }

   log.regIndex = profile;

   rc = prvCpssSip6TxqSdqLogInit(&log,sdqNum,name);
   if (rc != GT_OK)
   {
     return rc;
   }

   rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
               tileNum,&log,
               regAddr,
               offset,
               size,
               value);

   if(rc!=GT_OK)
   {
     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip7TxqSdqProfileSet  failed \n");
   }


   return rc;
 }

GT_STATUS prvCpssSip7TxqSdqProfileGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  sdqNum,
    IN  PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT  attr,
    IN  GT_U32  profile,
    OUT GT_U32  *valuePtr
)
{
   GT_STATUS rc = GT_OK;
   GT_U32   offset,size,regAddr;

   TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
   TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);
   CPSS_NULL_PTR_CHECK_MAC(valuePtr);

   if(profile>=SDQ_PROFILE_MAX_MAC)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Profile index %d is bigger then max allowed %d\n",profile,SDQ_PROFILE_MAX_MAC-1);
   }


   rc = prvCpssSip7TxqSdqProfileAttrParamsGet(devNum,sdqNum,attr,profile,&offset,&size,&regAddr,NULL);
   if (rc != GT_OK)
   {
     return rc;
   }

   rc = prvCpssSip6TxqRegisterFieldRead(devNum,
               tileNum,
               regAddr,
               offset,
               size,
               valuePtr);

   if(rc!=GT_OK)
   {
     CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip7TxqSdqProfileGet  failed \n");
   }


   return rc;
 }


GT_STATUS prvCpssSip7TxqSdqMapQueueToProfileSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  queueNumber,
    IN GT_U32  profile
)
{
    CPSS_DXCH_TABLE_ENT table;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;
    GT_U32    data[2];

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(profile>=SDQ_PROFILE_MAX_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Profile index %d is bigger then max allowed %d\n",profile,SDQ_PROFILE_MAX_MAC-1);
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    table = CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E + sdqNum + tileNum * CPSS_DXCH_SIP_7_MAX_LOCAL_SDQ_NUM_MAC;

    rc = prvCpssSip6TxqSdqTableLogInit(&tableLog,tileNum,sdqNum,table,"TBL_queue_cfg",queueNumber);
    if (rc != GT_OK)
    {
        return rc;
    }


    rc = prvCpssDxChReadTableEntry(devNum,table,
                                    queueNumber>>3/*each entry 8 queues*/,
                                    data);
    if(rc!=GT_OK)
    {
       return rc;
    }

    /*low 4 queues*/
    if((queueNumber&0x7)<4)
    {
           U32_SET_FIELD_MASKED_MAC(data[0],(queueNumber&0x7)*TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                             TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                             profile);
    }
    else
    {
           U32_SET_FIELD_MASKED_MAC(data[1],((queueNumber&0x7)-4)*TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                             TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                             profile);
    }

    return prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                                        table,
                                        queueNumber>>3/*each entry 8 queues*/,
                                        data);

}


GT_STATUS prvCpssSip7TxqSdqMapQueueToProfileGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  queueNumber,
    IN GT_U32  *profilePtr
)
{
    CPSS_DXCH_TABLE_ENT table;
    GT_STATUS rc;
    GT_U32    data[2];

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    table = CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_TO_PROFILE_MAP_E + sdqNum + tileNum * CPSS_DXCH_SIP_7_MAX_LOCAL_SDQ_NUM_MAC;

    rc = prvCpssDxChReadTableEntry(devNum,table,
                                        queueNumber>>3/*each entry 8 queues*/,
                                        data);
    if(rc!=GT_OK)
    {
       return rc;
    }

    /*low 4 queues*/
    if((queueNumber&0x7)<4)
    {
           *profilePtr  = (GT_BOOL)U32_GET_FIELD_MAC(data[0],
                         (queueNumber&0x7)*TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                         TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE);
    }
    else
    {
          *profilePtr  = (GT_BOOL)U32_GET_FIELD_MAC(data[1],
                         ((queueNumber&0x7)-4)*TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                         TXQ_AAS_SDQ_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE);
    }

    return GT_OK;
}

GT_STATUS prvCpssSip7TxqSdqQueueFindMatchingProfile
(
    IN GT_U8   devNum,
    IN PRV_CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_PROFILE_STC  *profilePtr,
    OUT GT_U32 *indexPtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_PROFILE_STC * iterator;

    CPSS_NULL_PTR_CHECK_MAC(profilePtr);
    CPSS_NULL_PTR_CHECK_MAC(indexPtr);


    iterator = PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles;

    *indexPtr = CPSS_DXCH_SIP_7_MAX_SDQ_PROFILE_NUM_ALL_DEVICES;

    for(i=0;i<CPSS_DXCH_SIP_7_MAX_SDQ_PROFILE_NUM_ALL_DEVICES;i++,iterator++)
    {
        if(iterator->numberOfBindedQueues!=0)
        {
            if(0 == cpssOsMemCmp(iterator->data,profilePtr->data,sizeof(GT_U32)*PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_LAST_E))
            {
                *indexPtr = i;
            }
        }
    }

    return GT_OK;

}

GT_STATUS prvCpssSip7TxqSdqQueueFindFreeProfile
(
    IN GT_U8   devNum,
    OUT GT_U32 *indexPtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_PROFILE_STC * iterator;

    CPSS_NULL_PTR_CHECK_MAC(indexPtr);

    iterator = PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles;

    *indexPtr = CPSS_DXCH_SIP_7_MAX_SDQ_PROFILE_NUM_ALL_DEVICES;

    for(i=0;i<CPSS_DXCH_SIP_7_MAX_SDQ_PROFILE_NUM_ALL_DEVICES;i++,iterator++)
    {
        if(iterator->numberOfBindedQueues ==0)
        {
           *indexPtr = i;
           break;
        }
    }

    return GT_OK;

}

static GT_STATUS prvCpssSip7TxqSdqAllDpProfileSet
(
    IN GT_U8   devNum,
    IN PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT  attr,
    IN GT_U32  profile,
    IN GT_U32  value
)
{
    GT_U32 numDpPerTile,tileNumber;
    GT_U32 i,j;
    GT_STATUS rc;

    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
        case 0:
        case 1:
            tileNumber = 1;
            break;
        case 2:
        case 4:
            tileNumber = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
            break;
    }

    numDpPerTile = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.multiDataPath.maxDp / tileNumber;

    /*Duplicate to all tiles and PDS*/
    for (i = 0; i < tileNumber; i++)
    {
      for (j = 0; j < numDpPerTile; j++)
      {
         rc = prvCpssSip7TxqSdqProfileSet(devNum,i,j,attr,profile,value);
         if(rc!=GT_OK)
         {
            return rc;
         }
      }
    }

    return GT_OK;
}


GT_STATUS prvCpssSip7TxqSdqQueueProfileAttrSet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  queueNumber,
    IN  PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT  attr,
    IN  GT_U32  *valuePtr
)
{
     GT_STATUS rc;
     PRV_CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_PROFILE_STC newProfile;
     GT_U32  currentProfileIndex;
     GT_U32  newIndex;
     PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT i;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     }

     CPSS_NULL_PTR_CHECK_MAC(valuePtr);

     /*Get old profile*/
     rc = prvCpssSip7TxqSdqMapQueueToProfileGet(devNum,tileNum,sdqNum,queueNumber,&currentProfileIndex);
     if(rc!=GT_OK)
     {
        return rc;
     }

      if(PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles[currentProfileIndex].data[attr]!= *valuePtr)
      {
         /*Init new profile*/
       cpssOsMemCpy(&newProfile,&PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles[currentProfileIndex],
        sizeof(PRV_CPSS_DXCH_SIP7_TXQ_SDQ_QUEUE_PROFILE_STC));

        newProfile.data[attr] = *valuePtr;

        PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles[currentProfileIndex].numberOfBindedQueues--;

        rc = prvCpssSip7TxqSdqQueueFindMatchingProfile(devNum,&newProfile,&newIndex);
        if(rc!=GT_OK)
        {
           return rc;
        }

        if(CPSS_DXCH_SIP_7_MAX_SDQ_PROFILE_NUM_ALL_DEVICES== newIndex)
        {
            rc = prvCpssSip7TxqSdqQueueFindFreeProfile(devNum,&newIndex);
            if(rc!=GT_OK)
            {
               return rc;
            }

            if(CPSS_DXCH_SIP_7_MAX_SDQ_PROFILE_NUM_ALL_DEVICES == newIndex)
            {
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Out of SDQ profiles\n");
            }
            else
            {
                 valuePtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles[newIndex].data;

                /*sync full profile to HW*/
                for(i=PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_FIRST_E;
                    i<PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_LAST_E;i++,valuePtr++)
                {
                    if(*valuePtr !=newProfile.data[i])
                    {
                        /*duplicate to all DPs*/
                        rc = prvCpssSip7TxqSdqAllDpProfileSet(devNum,i,newIndex,newProfile.data[i]);
                        if(rc!=GT_OK)
                        {
                           return rc;
                        }
                     }
                }
            }

        }

        rc = prvCpssSip7TxqSdqMapQueueToProfileSet(devNum,tileNum,sdqNum,queueNumber,newIndex);
        if(rc!=GT_OK)
        {
           return rc;
        }

        PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles[newIndex].numberOfBindedQueues++;

        cpssOsMemCpy(PRV_CPSS_DXCH_PP_MAC(devNum)->port.txQDeviceConfig.sdqProfiles[newIndex].data,&newProfile.data,
            sizeof(GT_U32)*PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_LAST_E);


      }

      return GT_OK;
}

GT_STATUS prvCpssSip7TxqSdqQueueProfileAttrGet
(
    IN GT_U8   devNum,
    IN GT_U32  tileNum,
    IN GT_U32  sdqNum,
    IN GT_U32  queueNumber,
    IN  PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_ENT  attr,
    IN  GT_U32  *valuePtr
)
{
     GT_STATUS rc;
     GT_U32  currentProfileIndex;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

     if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     }

     CPSS_NULL_PTR_CHECK_MAC(valuePtr);

     /*Get current  profile*/
     rc = prvCpssSip7TxqSdqMapQueueToProfileGet(devNum,tileNum,sdqNum,queueNumber,&currentProfileIndex);
     if(rc!=GT_OK)
     {
        return rc;
     }

     /*Get the attribute*/
     rc = prvCpssSip7TxqSdqProfileGet(devNum,tileNum,sdqNum,attr,currentProfileIndex,valuePtr);
     if(rc!=GT_OK)
     {
        return rc;
     }

     return GT_OK;
}

GT_STATUS   prvCpssSip6TxqSdqShapingCreditAccumulationEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 sdqNum,
    IN GT_U32 queueNum,
    IN GT_BOOL enable
)
{
    GT_U32  value;
    GT_U32  offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    value = BOOL2BIT_MAC(enable);

    if(queueNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        return prvCpssSip7TxqSdqQueueProfileAttrSet(devNum,tileNum,sdqNum,queueNum,
            PRV_CPSS_DXCH_TXQ_SIP_7_SDQ_PROFILE_ATTRIBUTE_AVB_EN_E,&value);
    }
    else if (PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) == GT_TRUE)
    {
      offset = TXQ_IRONMAN_SDQ_QUEUE_CFG_QUEUE_AVB_EN_FIELD_OFFSET;
      size = TXQ_IRONMAN_SDQ_QUEUE_CFG_QUEUE_AVB_EN_FIELD_SIZE;

      return  prvCpssDxChWriteTableEntryField(devNum,
                                    CPSS_DXCH_SIP6_TXQ_SDQ_QUEUE_CFG_E,
                                    queueNum,PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,
                                    offset,size,
                                    value);
    }


    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, "Field not supported for device %d \n",devNum);

}

