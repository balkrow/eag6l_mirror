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
* @file prvCpssDxChTxqPsi.c
*
* @brief CPSS SIP6 TXQ Psi low level configurations.
*
* @version   1
********************************************************************************
*/

/*Synced to \Cider \EBU-IP \TXQ_IP \SIP7.0 (Falcon) \TXQ_PR_IP \TXQ_PR_IP {7.0.6}*/


#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/central/prvCpssFalconTxqPsiRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/central/prvCpssAc5pTxqPsiRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/central/prvCpssAc5xTxqPsiRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/central/prvCpssHarrierTxqPsiRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/central/prvCpssIronmanTxqPsiRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/seahawk/central/prvCpssAasTxqPsiRegFile.h>


#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_TXQ_UNIT_NAME "PSI"
#define PRV_TXQ_LOG_UNIT GT_FALSE
#define PRV_TXQ_LOG_TABLE GT_FALSE


#define PRV_PSI_SIP_6_30_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_7_CHECK_MAC(_dev)?TXQ_AAS_PSI_##_fieldName:TXQ_IRONMAN_PSI_##_fieldName)


#define PRV_PSI_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?PRV_PSI_SIP_6_30_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName):TXQ_HARRIER_PSI_##_fieldName)

#define PRV_PSI_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PSI_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_PSI_##_fieldName)


#define PRV_PSI_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PSI_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_PSI_##_fieldName)

#define TXQ_PSI_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PSI_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_PSI_##_fieldName))


#define PRV_PSI_FIELD_VALUE_GET_MAC(_dev,_name,_data,_value) _value = U32_GET_FIELD_MAC(_data,\
                                                     PRV_PSI_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_name##_FIELD_OFFSET),\
                                                     PRV_PSI_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_name##_FIELD_SIZE))\

GT_STATUS prvCpssSip6TxqPsiLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_UNIT;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = PRV_TXQ_LOG_NO_INDEX;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}



/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesSet function
* @endinternal
*
* @brief   Map port to pdq queues
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] psiNum                - current PSI id(APPLICABLE RANGES:0..3).
* @param[in] pNodeIndex            - index of Pnode representing the local port
* @param[in] firstQueueInDp        - the first pdq queue of DP
* @param[in] dp                    - data path index[0..3] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
* @param[in] localPort             - local DP port[0..26] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 pNodeIndex,
    IN  GT_U32 firstQueueInDp,
    IN  GT_U32 dp,
    IN  GT_U32 localPort
)
{
    GT_STATUS rc;
    GT_U32    regValue = 0,regAddr;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_U32 offset,size;

    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);

    if((firstQueueInDp>=CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)||(pNodeIndex>=CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(devNum)))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_OFFSET,
                            TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_SIZE,
                            firstQueueInDp);
    }
    else
    {
      if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
      {
        offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
        size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
      {
        offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
        size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
      {
        offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
        size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
      }
      else
      {
        offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
        size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
      }

      U32_SET_FIELD_MASKED_MAC(regValue,offset,size,1);


      if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
      {
        offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
        size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
      {
        offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
        size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
      {
        offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
        size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
      }
      else
      {
        offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
        size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
      }

      U32_SET_FIELD_MASKED_MAC(regValue,offset,size,firstQueueInDp);

      if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
      {
        offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
        size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
      {
        offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
        size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
      {
        offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
        size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
      }
      else
      {
        offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
        size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
      }


      U32_SET_FIELD_MASKED_MAC(regValue,offset,size,localPort);

      if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
      {
        offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
        size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
      {
        offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
        size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
      }
      else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
      {
        offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
        size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
      }
      else
      {
        offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
        size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
      }

      U32_SET_FIELD_MASKED_MAC(regValue,offset,size,dp);
    }

    rc = prvCpssSip6TxqPsiLogInit(&log,"pdq2sdq_Map");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = pNodeIndex;

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.pdq2sdq_Map[pNodeIndex];


    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,psiNum,&log,regAddr,0,32,regValue);

    return rc;

}

/**
* @internal prvCpssFalconPsiMapPortToPdqQueuesGet function
* @endinternal
*
* @brief   Read sdq to pdq queues mapping
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                -  device number
* @param[in] psiNum                -  current PSI id(APPLICABLE RANGES:0..3).
* @param[in] sdqNum                -  SDQ that being mapped number (APPLICABLE RANGES:0..7).
* @param[in] localPortNum          -  local port number (APPLICABLE RANGES:0..8).
*
* @param[out] pdqLowQueueNumPtr    - (pointer to)the first pdq queue
* @param[out] dpPtr                - (pointer to)data path index[0..3] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
* @param[out] localPortPtr         - (pointer to)local DP port[0..26] (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman).
* @param[out] validPtr             - (pointer to) GT_TRUE: the pNode is mapped to a queue
*                                                 GT_False: the pNode is NOT mapped to a queue
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssFalconPsiMapPortToPdqQueuesGet
(
    IN   GT_U8  devNum,
    IN   GT_U32 psiNum,
    IN   GT_U32 sdqNum,
    IN   GT_U32 localPortNum,
    OUT  GT_U32 *pdqLowQueueNumPtr,
    OUT  GT_U32 *dpPtr,
    OUT  GT_U32 *localPortPtr,
    OUT  GT_U32 *validPtr
)
{
    GT_U32 pNodeIndex;
    GT_STATUS rc;
    GT_U32    regValue = 0,regAddr,tileOffset;
    GT_U32 offset,size;

    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);

    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPortNum);

    CPSS_NULL_PTR_CHECK_MAC(pdqLowQueueNumPtr);

    CPSS_NULL_PTR_CHECK_MAC(dpPtr);

    CPSS_NULL_PTR_CHECK_MAC(localPortPtr);

    CPSS_NULL_PTR_CHECK_MAC(validPtr);


    pNodeIndex = localPortNum*MAX_DP_IN_TILE(devNum)+sdqNum;

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.pdq2sdq_Map[pNodeIndex];
    tileOffset = prvCpssSip6TileOffsetGet(devNum,psiNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) == GT_FALSE)
    {
        if(rc == GT_OK)
        {
            *pdqLowQueueNumPtr = U32_GET_FIELD_MAC(regValue,TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_OFFSET,
                                                     TXQ_PSI_PORT_QUEUE_OFFSET_PORT_Q_OFFSET_0_FIELD_SIZE);
        }
    }
    else
    {

        if(rc == GT_OK)
        {
            if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {
              offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
              size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
            {
              offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
              size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
            {
              offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
              size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
            }
            else
            {
              offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_OFFSET;
              size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_Q_OFFSET_0_FIELD_SIZE;
            }

            *pdqLowQueueNumPtr = U32_GET_FIELD_MAC(regValue,offset,size);

            if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {
              offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
              size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
            {
              offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
              size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
            {
              offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
              size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
            }
            else
            {
              offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_OFFSET;
              size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_SDQ_ID_0_FIELD_SIZE;
            }

            *dpPtr = U32_GET_FIELD_MAC(regValue,offset,size);

            if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {
              offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
              size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
            {
              offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
              size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
            {
              offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
              size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
            }
            else
            {
              offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_OFFSET;
              size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_0_FIELD_SIZE;
            }


            *localPortPtr = U32_GET_FIELD_MAC(regValue,offset,size);

            if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
            {
              offset = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
              size = TXQ_HAWK_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
            {
              offset = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
              size = TXQ_PHOENIX_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
            }
            else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
            {
              offset = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
              size = TXQ_HARRIER_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
            }
            else
            {
              offset = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_OFFSET;
              size = TXQ_IRONMAN_PSI_PDQ2SDQ_MAP_PORT_MAP_VALID_0_FIELD_SIZE;
            }

            *validPtr = U32_GET_FIELD_MAC(regValue,offset,size);

        }
    }


    return rc;

}

GT_STATUS  prvCpssSip7PsiCreditValueSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 creditValue
)
{   GT_U32 i,maxCreditValue,size;
    GT_U32 regAddr;
    GT_U32 regValue = 0;

    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    GT_STATUS rc;

    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_FALSE)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);


    size = TXQ_AAS_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE;
    maxCreditValue = (1<<size)-1;


    if(GT_TRUE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
       /*Credit Value 256B granule*/
       creditValue>>=8;
    }

    if(creditValue>maxCreditValue)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Credit value set to HW is too big %d .Maximal allowed is %d \n",creditValue,maxCreditValue);
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Credit_Value;

    for(i=0;i<4;i++)
    {
      U32_SET_FIELD_MASKED_MAC(regValue,i*size,size,creditValue);
    }


    rc = prvCpssSip6TxqPsiLogInit(&log,"Credit_Value");
    if (rc != GT_OK)
    {
      return rc;
    }

    for(i=0;i<CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(devNum)/4;i++)
    {
        log.regIndex = i;
        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,psiNum,&log,regAddr+4*i,0,32,regValue);
        if (rc != GT_OK)
        {
          return rc;
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssFalconPsiCreditValueSet function
* @endinternal
*
* @brief   Set credit value that is being given to SDQ from PDQ
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] psiNum                - current PSI id(APPLICABLE RANGES:0..3).
* @param[in] creditValue           - credit value
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Currently only one tile is supported
*
*/
GT_STATUS  prvCpssFalconPsiCreditValueSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 psiNum,
    IN  GT_U32 creditValue
)
{
     GT_U32 regAddr;
     GT_U32 regValue = 0;
     GT_U32 maxCreditValue,offset,size;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(psiNum);


    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

     rc = prvCpssSip6TxqPsiLogInit(&log,"Credit_Value");
     if (rc != GT_OK)
     {
       return rc;
     }
     if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
       offset = TXQ_HAWK_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET;
       size = TXQ_HAWK_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
     {
       offset = TXQ_PHOENIX_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET;
       size = TXQ_PHOENIX_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE;
     }
     else if(GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
       offset = TXQ_HARRIER_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET;
       size = TXQ_HARRIER_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE;
     }
     else
     {
       offset = TXQ_IRONMAN_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_OFFSET;
       size = TXQ_IRONMAN_PSI_CREDIT_VALUE_CREDIT_VALUE_FIELD_SIZE;
     }

     maxCreditValue = (1<<size)-1;

     if(GT_TRUE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
     {
        /*Credit Value 256B granule*/
        creditValue>>=8;
     }

     if(creditValue>maxCreditValue)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Credit value set to HW is too big %d .Maximal allowed is %d \n",creditValue,maxCreditValue);
     }

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Credit_Value;


     U32_SET_FIELD_MASKED_MAC(regValue,offset,size,creditValue);


     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,psiNum,&log,regAddr,0,32,regValue);

     return rc;


}

/**
* @internal prvCpssSip6PsiClockGatingEnableSet function
* @endinternal
*
* @brief   Enable/disable clock gating .(reducing dynamic power dissipation, by removing the clock signal when the circuit is not in use)
*
* @note   APPLICABLE DEVICES:      AC5X;Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P.
*
* @param[in] devNum               -  device number
* @param[in] clockGateEnable      -  Enable/disable clock gating
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS  prvCpssSip6PsiClockGatingEnableSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL clockGateEnable
)
{
     GT_U32 regAddr;
     GT_U32 regValue = 0;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     GT_STATUS rc;

     /*only for AC5X and above*/
     if (GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
     {
        return GT_OK;
     }

     rc = prvCpssSip6TxqPsiLogInit(&log,"Debug_CFG_Register");
     if (rc != GT_OK)
     {
       return rc;
     }

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.Debug_CFG_Register;

     U32_SET_FIELD_MASKED_MAC(regValue,PRV_PSI_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,DEBUG_CFG_REGISTER_PDQ_CG_EN_FIELD_OFFSET),
                            PRV_PSI_SIP_6_15_DEV_DEP_FIELD_GET_MAC(devNum,DEBUG_CFG_REGISTER_PDQ_CG_EN_FIELD_SIZE),
                            clockGateEnable);


     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,0,&log,regAddr,0,32,regValue);

     return rc;


}

/**
* @internal prvCpssSip6TxqPsiDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PSI
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPsiDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    rc = prvCpssSip6TxqPsiLogInit(&log,"PSI_Interrupt_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.PSI_Interrupt_Mask,
                 0,32,0);
}
/**
* @internal prvCpssSip6TxqSdqDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PSI
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
GT_STATUS prvCpssSip6TxqPsiDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    OUT GT_U32 *interruptPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(interruptPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    return  prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.PSI_Interrupt_Cause,
                 0,32,interruptPtr);
}


GT_U32 prvCpssSip6TxqPsiSchedErrorBmp
(
    IN GT_U8  devNum
)
{
   GT_U32 bitOffset;

   bitOffset = TXQ_PSI_FIELD_GET(devNum,PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_FIELD_OFFSET);
   return 1<<bitOffset;
}

static GT_STATUS prvCpssSip6TxqPsiTableLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC * logDataPtr,
   IN GT_U32                                   tileNum,
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
    logDataPtr->unitIndex = PRV_TXQ_LOG_NO_INDEX;
    logDataPtr->tableName = tableName;
    logDataPtr->entryIndex = entryIndex;

    return GT_OK;
}

/**
* @internal prvCpssSip7QueueToQueueTableSet function
* @endinternal
*
* @brief   Configure PSI Queue To Queue table
*
* @note   APPLICABLE DEVICES: AAS
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] sdqQNum               - SDQ Q index
* @param[in] valid                 - GT_TRUE in case entry valid ,GT_FALSE otherwise
* @param[in] pdqQNum               - PDQ Q index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssSip7QueueToQueueTableSet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  sdqNum,
    IN  GT_U32  sdqQNum,
    IN  GT_BOOL valid,
    IN  GT_U32  pdqQNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;
    GT_STATUS rc;
    GT_U32    index,value =0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(pdqQNum>=CPSS_DXCH_SIP_6_MAX_PDQ_QUEUE_NUM)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(sdqQNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*Queue To Queue mapping from sdqs Q to pdq Q.
    q2q line = {SDQNum[1:0], SDQQueue[14:0]}*/

    index = ((sdqNum&0x3)<<15)|(sdqQNum&0x7FFF);

    rc = prvCpssSip6TxqPsiTableLogInit(&tableLog,tileNum,
        CPSS_DXCH_SIP6_TXQ_PDX_DX_QGRPMAP_E,"Queue_To_Queue",index);
    if (rc != GT_OK)
    {
        return rc;
    }

    U32_SET_FIELD_MASKED_MAC(value,TXQ_AAS_PSI_Q2Q_PDQQ_FIELD_OFFSET,
                            TXQ_AAS_PSI_Q2Q_PDQQ_FIELD_SIZE,
                            pdqQNum);

    U32_SET_FIELD_MASKED_MAC(value,TXQ_AAS_PSI_Q2Q_VALID_FIELD_OFFSET,
                            TXQ_AAS_PSI_Q2Q_VALID_FIELD_SIZE,
                            valid);


    rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                    CPSS_DXCH_SIP7_TXQ_PSI_QUEUE_TO_QUEUE_E,
                    index,
                    &value);

    return GT_OK;

}

/**
* @internal prvCpssSip7QueueToQueueTableGet function
* @endinternal
*
* @brief   Get configuration of PSI Queue To Queue table
*
* @note   APPLICABLE DEVICES: AAS
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] sdqQNum               - SDQ Q index
* @param[out] validPtr             -(pointer to) GT_TRUE in case entry valid ,GT_FALSE otherwise
* @param[out] pdqQNumPtr           -(pointer to) PDQ Q index
* @param[out] indexPtr             - (pointer to) Calculated index in PSI Queue To Queue table.Ignored in case of NULL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssSip7QueueToQueueTableGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  tileNum,
    IN  GT_U32  sdqNum,
    IN  GT_U32  sdqQNum,
    OUT GT_BOOL *validPtr,
    OUT GT_U32  *pdqQNumPtr,
    OUT GT_U32  *indexPtr
)
{
    GT_STATUS rc;
    GT_U32    index,value =0;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    CPSS_NULL_PTR_CHECK_MAC(validPtr);
    CPSS_NULL_PTR_CHECK_MAC(pdqQNumPtr);

    if(sdqQNum>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /*Queue To Queue mapping from sdqs Q to pdq Q.
    q2q line = {SDQNum[1:0], SDQQueue[14:0]}*/

    index = ((sdqNum&0x3)<<15)|(sdqQNum&0x7FFF);

    rc = prvCpssDxChReadTableEntry(devNum,CPSS_DXCH_SIP7_TXQ_PSI_QUEUE_TO_QUEUE_E,
                                    index,
                                    &value);
    if(rc!=GT_OK)
    {
       return rc;
    }

    if(indexPtr)
    {
        *indexPtr = index;
    }

    *validPtr = (GT_BOOL)U32_GET_FIELD_MAC(value,
                             TXQ_AAS_PSI_Q2Q_VALID_FIELD_OFFSET,
                             TXQ_AAS_PSI_Q2Q_VALID_FIELD_SIZE);

    *pdqQNumPtr = U32_GET_FIELD_MAC(value,
                             TXQ_AAS_PSI_Q2Q_PDQQ_FIELD_OFFSET,
                             TXQ_AAS_PSI_Q2Q_PDQQ_FIELD_SIZE);


    return GT_OK;

}

/**
* @internal prvCpssSip7PsiBpPdqToSdqSet function
* @endinternal
*
* @brief  Set configuration of PSI bp_pdq_sdq bitmap
*
* @note   APPLICABLE DEVICES: AAS
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] pdqPort               - P node index
* @param[in] bmp                   - SDQ ports bitmap ,each bit represent SDQ port that  have queues connected to specific PDQ P node.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS  prvCpssSip7PsiBpPdqToSdqSet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 sdqNum,
    IN  GT_U32 pdqPort,
    IN  GT_U32 bmp
)
{
     GT_U32 regAddr;
     GT_U32 maxBmpValue;
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

     GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);

    if(pdqPort>=CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(devNum))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


     maxBmpValue = (1<<TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_SIZE)-1;

     if(bmp>maxBmpValue)
     {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Bmp is too big 0x%x .Maximal allowed is %0x%x \n",bmp,maxBmpValue);
     }

     rc = prvCpssSip6TxqPsiLogInit(&log,"bp_pdq_sdq");
     if (rc != GT_OK)
     {
       return rc;
     }

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.bp_pdq_sdq;
     /*
             BP PDQ Port 0 SDQ 0
             BP PDQ Port 0 SDQ 1
             BP PDQ Port 0 SDQ 2
             BP PDQ Port 0 SDQ 3

             BP PDQ Port 1 SDQ 0
             BP PDQ Port 1 SDQ 1
             BP PDQ Port 1 SDQ 2
             BP PDQ Port 1 SDQ 3

             ....
     */
     regAddr+=( 0x4*(sdqNum + pdqPort*4));

     log.regIndex = sdqNum + pdqPort*4;

     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,
        TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_OFFSET,TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_SIZE,bmp);

     return rc;


}

/**
* @internal prvCpssSip7PsiBpPdqToSdqGet function
* @endinternal
*
* @brief  Get configuration of PSI bp_pdq_sdq bitmap
*
* @note   APPLICABLE DEVICES: AAS
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
* @param[in] pdqPort               - P node index
* @param[out] bmpPtr               - (pointer to) SDQ ports bitmap ,each bit represent SDQ port that  have queues connected to
*                                     specific PDQ P node.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS  prvCpssSip7PsiBpPdqToSdqGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 sdqNum,
    IN  GT_U32 pdqPort,
    OUT GT_U32 *bmpPtr
)
{
     GT_U32 regAddr;
     GT_STATUS rc;

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
     TXQ_SIP_6_CHECK_LOCAL_SDQ_NUM_MAC(sdqNum);
     CPSS_NULL_PTR_CHECK_MAC(bmpPtr);

    if(pdqPort>=CPSS_DXCH_SIP_6_MAX_PDQ_PORT_NUM(devNum))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

     regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.psi_regs.bp_pdq_sdq;
     /*
             BP PDQ Port 0 SDQ 0
             BP PDQ Port 0 SDQ 1
             BP PDQ Port 0 SDQ 2
             BP PDQ Port 0 SDQ 3

             BP PDQ Port 1 SDQ 0
             BP PDQ Port 1 SDQ 1
             BP PDQ Port 1 SDQ 2
             BP PDQ Port 1 SDQ 3

             ....
     */
     regAddr+=( 0x4*(sdqNum + pdqPort*4));

     rc = prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,regAddr,
        TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_OFFSET,TXQ_AAS_PSI_BP_PDQ_PORT_FIELD_SIZE,bmpPtr);

     return rc;


}


