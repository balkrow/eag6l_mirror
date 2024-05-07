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
* @file prvCpssDxChTxqShapingUtils.c
*
* @brief CPSS SIP6 TXQ  shaping operation functions
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>


/**
* @internal prvCpssSip6TxqUtilsShaperPermissiveModeEnableSet function
* @endinternal
*
* @brief Set operation mode of egress shaper.
*  If enable equal GT_TRUE then  shaper rate accuracy error is ignored,
*  Otherwise  GT_SHAPER_GRANULARITY_OUT_OF_RANGE will be returned in
*  case accuracy error exceed threshold.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier;Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum               - physical device number
* @param[in] enable                  -if equals GT_TRUE shaper permissive mode activated,GT_FALSE otherwise.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqUtilsShaperPermissiveModeEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 enable
)
{
    PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).permissiveShaperMode = enable;
    return GT_OK;
}

/**
* @internal prvCpssSip6TxqUtilsShaperPermissiveModeEnableGet function
* @endinternal
*
* @brief Get operation mode of egress shaper.
*  If enable equal GT_TRUE then  shaper rate accuracy error is ignored,
*  Otherwise  GT_SHAPER_GRANULARITY_OUT_OF_RANGE will be returned in
*  case accuracy error exceed threshold.
*
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier;Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in]  devNum               - physical device number
* @param[out] enablePtr                  -(pointer to) shaper permissive mode activation.
*
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssSip6TxqUtilsShaperPermissiveModeEnableGet
(
    IN GT_U8                        devNum,
    OUT GT_BOOL                      *enablePtr
)
{

    *enablePtr = PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).permissiveShaperMode;
    return GT_OK;
}




/**
 * @internal prvCpssFalconTxqUtilsShapingParametersGet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and read shaping parameters from specific node(SW)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          - physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 *
 * @param[in] burstSize                - burst size in units of 4K bytes
 *                                       max value is 1K-1 which results in 4M-1 burst size
 * @param[out] maxRatePtr              - (pointer to) the requested shaping Rate value in Kbps.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsShapingParametersGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN PRV_QUEUE_SHAPING_ACTION_ENT rateType,
    OUT GT_U16                              *burstSizePtr,
    OUT GT_U32                              *maxRatePtr
)
{
    GT_U32                  tileNum;
    GT_U32                  nodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;
    GT_STATUS               rc = GT_OK;
    PRV_CPSS_PDQ_LEVEL_ENT level;


   rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
   if (rc!=GT_OK)
   {
       return rc;
   }


    if(GT_FALSE == isCascade)
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &nodeIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        level =PRV_CPSS_PDQ_LEVEL_A_E;
    }
    else
    {
        nodeIndex = pNodePtr->pNodeIndex;
        /*P[i]=C[i]=B[i]*/
        level =PRV_CPSS_PDQ_LEVEL_B_E;
        tileNum = pNodePtr->aNodelist[0].queuesData.tileNum;
    }

    if (perQueue == GT_FALSE)
    {
        rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileNum, nodeIndex, level, rateType, burstSizePtr, maxRatePtr);
        if (rc)
        {
            return rc;
        }
    }
    else
    {
        if(GT_TRUE==isCascade)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
        }

        /*Find the queue index*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
        if (rc)
        {
            return rc;
        }
        qNodeIndex  = aNodePtr->queuesData.pdqQueueFirst + queueNumber;
        rc      = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, rateType, burstSizePtr, maxRatePtr);
    }
    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsShapingParametersSet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and write shaping parameters to specific node
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          - physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 * @param[in] shapingActionType        - token bucket type to configure
 * @param[in] burstSize                - burst size in units of 4K bytes
 *                                       max value is 1K-1 which results in 4M-1 burst size
 * @param[in,out] maxRatePtr               - (pointer to)Requested Rate in Kbps
 * @param[in,out] maxRatePtr               -(pointer to) the actual Rate value in Kbps.
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsShapingParametersSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN PRV_QUEUE_SHAPING_ACTION_ENT shapingActionType,
    IN GT_U16 burstSize,
    INOUT GT_U32                              *maxRatePtr
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled = GT_FALSE;
    GT_BOOL                 actAsPreemptiveChannel = GT_FALSE;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;
    PRV_CPSS_PDQ_LEVEL_ENT level;

    /*check  if this is cascade port*/
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_FALSE==isCascade)
    {
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
        }

        if(GT_TRUE==actAsPreemptiveChannel)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
        }
    }
    else
    {
       if(GT_TRUE== perQueue)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
       }
        tileNum = pNodePtr->aNodelist[0].queuesData.tileNum;
    }
    if (perQueue == GT_FALSE)
    {
        if(GT_TRUE==preemptionEnabled)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Shaping can not be configured on port %d due to preemption\n", physicalPortNum);
        }

        level = PRV_CPSS_PDQ_LEVEL_A_E;

        if(GT_TRUE==isCascade)
        {
            if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E != PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
            {
                level = PRV_CPSS_PDQ_LEVEL_B_E;/*Bnode index == pNode index*/
            }
            else
            {
                level = PRV_CPSS_PDQ_LEVEL_P_E;
            }
        }


        rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileNum,GT_TRUE==isCascade?pNodePtr->pNodeIndex:aNodeIndex,level,
               burstSize, maxRatePtr);
        if (rc)
        {
            return rc;
        }
    }
    else
    {
        /*Find the queue index*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &(aNodePtr[0]));
        if (rc)
        {
            return rc;
        }
        if(GT_TRUE==preemptionEnabled)
        {
            rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
            if (rc != GT_OK)
            {
                return rc;
            }
        }


         /*Duplicate to preemptive channel*/
            for(i=0;i<2;i++)
            {
                if(NULL!=aNodePtr[i])
                {
                    qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;
                    if (shapingActionType == PRV_QUEUE_SHAPING_ACTION_PRIORITY_DOWNGRADE_ENT)
                    {
                        rc = prvCpssFalconTxqPdqMinBwOnNodeSet(devNum, tileNum, qNodeIndex, burstSize, maxRatePtr);
                        if(rc==GT_OK)
                        {
                            if(i==0)
                            {
                                    /*save only 8 queues*/
                                if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                                {
                                    /*save for restore*/
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].cir = *maxRatePtr;
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].burstCir= burstSize;
                                }
                             }
                        }
                    }
                    else
                    {
                        rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, burstSize, maxRatePtr);
                        if(rc==GT_OK)
                        {
                            if(i==0)
                            {
                                    /*save only 8 queues*/
                                if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                                {
                                    /*save for restore*/
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].eir = *maxRatePtr;
                                    aNodePtr[i]->queuesData.restoreData[queueNumber].burstEir= burstSize;
                                }
                             }
                        }
                    }
                    if (rc)
                    {
                        return rc;
                    }
                }
            }

    }
    return rc;
}
/**
 * @internal prvCpssSip6TxqUtilsShapingEnableSet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 * @param[in] enable                   - GT_TRUE,  Shaping
 *                                      GT_FALSE, disable Shaping
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssSip6TxqUtilsShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN GT_BOOL enable
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled = GT_FALSE;
    GT_BOOL                 actAsPreemptiveChannel = GT_FALSE;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;
    PRV_CPSS_PDQ_LEVEL_ENT level;


    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
    }


    if(GT_TRUE==actAsPreemptiveChannel)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
    }

    /*check  if this is cascade port*/
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_TRUE==isCascade)
    {
       if(GT_TRUE== perQueue)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
       }

      if(GT_TRUE==isCascade)
      {
          if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E != PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(devNum))
          {
              level = PRV_CPSS_PDQ_LEVEL_B_E;/*P node index is equal to B node idex*/
          }
          else
          {
              level = PRV_CPSS_PDQ_LEVEL_P_E;
          }
      }

       rc = prvCpssFalconTxqPdqShapingEnableSet(devNum, pNodePtr->aNodelist[0].queuesData.tileNum,
                pNodePtr->pNodeIndex, level, enable);
       if (rc != GT_OK)
       {
           return rc;
       }
    }
    else
    {
        /*chek that port is mapped*/
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
        if (rc != GT_OK)
        {
            return rc;
        }
        if (perQueue == GT_FALSE)
        {
         if(GT_TRUE==preemptionEnabled)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Shaping can not be configured on port %d due to preemption\n", physicalPortNum);
         }

         rc = prvCpssFalconTxqPdqShapingEnableSet(devNum, tileNum, aNodeIndex, PRV_CPSS_PDQ_LEVEL_A_E, enable);
         if (rc != GT_OK)
         {
             return rc;
         }
        }
        else
        {
            /*Find the queue index*/
            rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &(aNodePtr[0]));
            if (rc)
            {
                return rc;
            }
            /*Duplicate to preemptive channel*/
            if(GT_TRUE==preemptionEnabled)
            {
                rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
            for(i=0;i<2;i++)
            {
               if(NULL!=aNodePtr[i])
               {
                   qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;
                   rc      = prvCpssFalconTxqPdqShapingEnableSet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, enable);
                   if(rc==GT_OK)
                   {
                       if(i==0)
                       {
                                  /*save only 8 queues*/
                          if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                          {
                               /*save for restore*/
                               aNodePtr[i]->queuesData.restoreData[queueNumber].eirEnable = enable;
                          }
                        }
                   }

                   if (rc)
                   {
                       return rc;
                   }
               }
             }
        }
     }
    return rc;
}

/**
 * @internal prvCpssSip6TxqUtilsShapingEnableGet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and get Enable/Disable Token Bucket rate shaping on specified port or queue of specified device.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
 * @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port,relevant only if perQueue is GT_TRUE
 * @param[in] perQueue                 - if equal GT_TRUE then it is node at Q level ,if  if equal GT_TRUE then it is node at A level
 *
 * @param[out] enablePtr                - GT_TRUE, enable Shaping
 *                                      GT_FALSE, disable Shaping
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssSip6TxqUtilsShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 queueNumber,
    IN GT_BOOL perQueue,
    IN GT_BOOL  *  enablePtr
)
{
    GT_U32                  tileNum;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 isCascade = GT_FALSE;
    PRV_CPSS_DXCH_TXQ_SIP_6_P_NODE *pNodePtr = NULL;

    /*check  if this is cascade port*/
    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum,physicalPortNum,&isCascade,&pNodePtr);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if(GT_TRUE==isCascade)
    {
       if(GT_TRUE== perQueue)
       {
           CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Q level shaping can not be configured on cascade port\n");
       }
       /*P node index is equal to B node idex*/
       rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, pNodePtr->aNodelist[0].queuesData.tileNum,
                pNodePtr->pNodeIndex, PRV_CPSS_PDQ_LEVEL_B_E, enablePtr);

       return rc;
    }

    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (perQueue == GT_FALSE)
    {
        rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileNum, aNodeIndex, PRV_CPSS_PDQ_LEVEL_A_E, enablePtr);
        if (rc)
        {
            return rc;
        }
    }
    else
    {
        /*Find the queue index*/
        rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
        if (rc)
        {
            return rc;
        }
        qNodeIndex  = aNodePtr->queuesData.pdqQueueFirst + queueNumber;
        rc      = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileNum, qNodeIndex, PRV_CPSS_PDQ_LEVEL_Q_E, enablePtr);
        if (rc)
        {
            return rc;
        }
    }
    return rc;
}

/**
 * @internal prvCpssFalconTxqUtilsMinimalBwEnableGet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and get
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
 *
 * @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port
 * @param[in] enablePtr                   - (pointer to)GT_TRUE,  Set lowest  priority once token bucket is empty
 *                                                       GT_FALSE, otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableGet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       queueNumber,
    OUT GT_BOOL     *enablePtr,
    OUT  GT_BOOL    *keepPriorityGroupPtr
)
{
    GT_U32                  tileNum;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr;
    GT_U32                  minBwType;
    GT_STATUS               rc = GT_OK;
    /*check that port is mapped*/
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }
    /*Find the queue index*/
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc)
    {
        return rc;
    }
    qNodeIndex  = aNodePtr->queuesData.pdqQueueFirst + queueNumber;
    rc      = prvCpssFalconTxqPdqMinBwEnableGet(devNum, tileNum, qNodeIndex, enablePtr,&minBwType);
    if (rc)
    {
        return rc;
    }

    if(0==minBwType)
    {
        *keepPriorityGroupPtr = GT_FALSE;
    }
    else
    {
        *keepPriorityGroupPtr = GT_TRUE;
    }

    return rc;
}
/**
 * @internal prvCpssFalconTxqUtilsMinimalBwEnableSet function
 * @endinternal
 *
 * @brief   Convert physicalPortNum into scheduler node and
 *             Enable/Disable Token Bucket rate shaping on specified port or queue of  specified device.
 *             Once token bucket is empty the priority is lowered to lowest.
 *
 * @note   APPLICABLE DEVICES:        Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:    xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   - Device number.
 * @param[in] physicalPortNum          -physical port number        -
 * @param[in] queueNumber              - Queue offset from base queue mapped to the current port
 * @param[in] enable                   - GT_TRUE,  Set lowest  priority once token bucket is empty
 *                                      GT_FALSE, otherwise
 *
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - on hardware error.
 * @retval GT_NO_RESOURCE           - on out of memory space.
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
 * @retval GT_BAD_STATE             - on burst mode or init doesnt succeed
 */
GT_STATUS prvCpssFalconTxqUtilsMinimalBwEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPortNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  enable,
    IN  GT_BOOL keepPriorityGroup
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled,actAsPreemptiveChannel;

    /*chek that port is mapped*/
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
    }

    if(GT_TRUE==actAsPreemptiveChannel)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
    }
    /*Find the A node */
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr[0]);
    if (rc)
    {
        return rc;
    }

    /*Duplicate to preemptive channel*/
    if(GT_TRUE==preemptionEnabled)
    {
        rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    for(i=0;i<2;i++)
    {
       if(NULL!=aNodePtr[i])
       {
           qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;

           rc      = prvCpssFalconTxqPdqMinBwEnableSet(devNum, tileNum, qNodeIndex,enable,keepPriorityGroup);
           if (rc)
           {
               return rc;
           }
           if(rc==GT_OK)
           {
               if(i==0)
               {
                      /*save only 8 queues*/
                  if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                  {
                    /*save for restore*/
                    aNodePtr[i]->queuesData.restoreData[queueNumber].cirEnable = enable;
                   }
                }
           }
       }
     }

    return rc;
}

/**
* @internal prvCpssSip6TxqUtilsShapingCreditAccumulationEnableSet function
* @endinternal
*
* @brief   This function also handle preemptive channel
                Enable/Disable credit accumulation. Required for 802.1.Q 8.6.8.2 (previously 802.1Qav)
*              A frame is only allowed to transmit when the corresponding credit is not negative.
*              If credit accumulation is disabled ,then positive credit is set to zero if there is no frame of the corresponding class.
*              Note : Default behavior is enabling credit accumulation
*
* @note   APPLICABLE DEVICES:          Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] queueOffset              - Egress queue offset
* @param[in] enable                   -  GT_TRUE,  Credit is accumulated until max burst size.
*                                                          GT_FALSE, Credit is not accumulated .Positive credit is set to zero if there is no frame of the corresponding class.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

GT_STATUS prvCpssSip6TxqUtilsShapingCreditAccumulationEnableSet
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPortNum,
    IN GT_U32   queueNumber,
    IN GT_BOOL  enable
)
{
    GT_U32                  tileNum,i;
    GT_U32                  aNodeIndex, qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr[2]={NULL,NULL};
    GT_STATUS               rc = GT_OK;
    GT_BOOL                 preemptionEnabled,actAsPreemptiveChannel;

    /*chek that port is mapped*/
    rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, physicalPortNum, &tileNum, &aNodeIndex);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssSip6TxqUtilsPreemptionStatusGet(devNum,physicalPortNum,&preemptionEnabled,&actAsPreemptiveChannel,NULL,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc , "prvCpssSip6TxqUtilsPreemptionStatusGet failed for port %d\n", physicalPortNum);
    }

    if(GT_TRUE==actAsPreemptiveChannel)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Port %d act as preemptive channel.\n", physicalPortNum);
    }
    /*Find the A node */
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr[0]);
    if (rc)
    {
        return rc;
    }

    /*Duplicate to preemptive channel*/
    if(GT_TRUE==preemptionEnabled)
    {
        rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum, aNodePtr[0],&(aNodePtr[1]));
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    for(i=0;i<2;i++)
    {
       if(NULL!=aNodePtr[i])
       {
           qNodeIndex = aNodePtr[i]->queuesData.pdqQueueFirst + queueNumber;

           rc      = prvCpssSip6TxqPdqNodeShapingCreditAccumulationEnableSet(devNum, tileNum, qNodeIndex,enable);
           if (rc)
           {
               return rc;
           }
           if(rc==GT_OK)
           {
               if(i==0)
               {
                      /*save only 8 queues*/
                  if(queueNumber<PRV_CPSS_DXCH_SIP_6_MAX_TC_QUEUE_TO_RESTORE_NUM_MAC)
                  {
                    /*save for restore*/
                    aNodePtr[i]->queuesData.restoreData[queueNumber].creditAccumulation= enable;
                   }
                }
           }

           qNodeIndex = aNodePtr[i]->queuesData.queueBase + queueNumber;

           rc      = prvCpssSip6TxqSdqShapingCreditAccumulationEnableSet(devNum, tileNum,aNodePtr[i]->queuesData.dp,qNodeIndex,enable);
           if (rc)
           {
               return rc;
           }
       }
     }

    return rc;
}


/**
* @internal prvCpssSip6TxqUtilsShapingCreditAccumulationEnableGet function
* @endinternal
*
* @brief
                Get Enable/Disable credit accumulation. Required for 802.1.Q 8.6.8.2 (previously 802.1Qav)
*              A frame is only allowed to transmit when the corresponding credit is not negative.
*              If credit accumulation is disabled ,then positive credit is set to zero if there is no frame of the corresponding class.
*              Note : Default behavior is enabling credit accumulation
*
* @note   APPLICABLE DEVICES:          Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; .
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical or CPU port number
* @param[in] queueOffset              - Egress queue offset
* @param[in] enable                   -  GT_TRUE,  Credit is accumulated until max burst size.
*                                                          GT_FALSE, Credit is not accumulated .Positive credit is set to zero if there is no frame of the corresponding class.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqUtilsShapingCreditAccumulationEnableGet
(
    IN GT_U8    devNum,
    IN GT_U32   physicalPortNum,
    IN GT_U32   queueNumber,
    OUT GT_BOOL  *enablePtr
)
{
    GT_U32                  qNodeIndex;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE *    aNodePtr= NULL;
    GT_STATUS               rc = GT_OK;


     /*Find the A node */
    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, physicalPortNum, &aNodePtr);
    if (rc)
    {
        return rc;
    }

    qNodeIndex = aNodePtr->queuesData.pdqQueueFirst + queueNumber;

    rc      = prvCpssSip6TxqPdqNodeShapingCreditAccumulationEnableGet(devNum, aNodePtr->queuesData.tileNum, qNodeIndex,enablePtr);
    if (rc)
    {
        return rc;
    }
    return rc;
}


GT_STATUS prvCpssSip7TxqUtilsServiceShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,physicalPortNum,serviceId,&cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqPdqShapingEnableSet(devNum, tileIndex,
                cNodeIndex, PRV_CPSS_PDQ_LEVEL_C_E, enable);

    return rc;
}

GT_STATUS prvCpssSip7TxqUtilsServiceShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;


    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,physicalPortNum,serviceId,&cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileIndex,
                cNodeIndex, PRV_CPSS_PDQ_LEVEL_C_E, enablePtr);

    return rc;


}

GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingEnableSet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_U32 subServiceId,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,physicalPortNum,serviceId,
                subServiceId,&bNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqPdqShapingEnableSet(devNum, tileIndex,
                            bNodeIndex, PRV_CPSS_PDQ_LEVEL_B_E, enable);

    return rc;
}


GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingEnableGet
(
    IN GT_U8 devNum,
    IN GT_U32 physicalPortNum,
    IN GT_U32 serviceId,
    IN GT_U32 subServiceId,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

   rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,physicalPortNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
   if (rc != GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

   rc = prvCpssFalconTxqPdqShapingEnableGet(devNum, tileIndex,
                               bNodeIndex, PRV_CPSS_PDQ_LEVEL_B_E, enablePtr);

    return rc;
}


GT_STATUS prvCpssSip7TxqUtilsServiceShapingParametersSet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U16       burstSize,
    INOUT GT_U32    *maxRatePtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,physicalPortNum,serviceId,&cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileIndex,cNodeIndex,
            PRV_CPSS_PDQ_LEVEL_C_E, burstSize, maxRatePtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}


GT_STATUS prvCpssSip7TxqUtilsServiceShapingParametersGet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U16       *burstSizePtr,
    OUT GT_U32    *maxRatePtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);
    CPSS_NULL_PTR_CHECK_MAC(burstSizePtr);

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,physicalPortNum,serviceId,&cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileIndex,cNodeIndex,
            PRV_CPSS_PDQ_LEVEL_C_E, PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT,burstSizePtr, maxRatePtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}




GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingParametersSet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U32       subServiceId,
    IN GT_U16       burstSize,
    INOUT GT_U32    *maxRatePtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

   rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,physicalPortNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
   if (rc != GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

    rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileIndex,bNodeIndex,
            PRV_CPSS_PDQ_LEVEL_B_E, burstSize, maxRatePtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}



GT_STATUS prvCpssSip7TxqUtilsSubServiceShapingParametersGet
(
    IN GT_U8        devNum,
    IN GT_U32       physicalPortNum,
    IN GT_U32       serviceId,
    IN GT_U32       subServiceId,
    OUT GT_U16      *burstSizePtr,
    OUT GT_U32      *maxRatePtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

    rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,physicalPortNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileIndex,bNodeIndex,
            PRV_CPSS_PDQ_LEVEL_B_E, PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT,burstSizePtr, maxRatePtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}

GT_STATUS prvCpssSip7TxqUtilsPnodeShapingParametersSet
(
    IN GT_U8        devNum,
    IN GT_U32       tileIndex,
    IN GT_U32       pNodeIndex,
    IN GT_U16       burstSize,
    INOUT GT_U32    *maxRatePtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

    rc = prvCpssFalconTxqPdqShapingOnNodeSet(devNum, tileIndex,pNodeIndex,
            PRV_CPSS_PDQ_LEVEL_P_E, burstSize, maxRatePtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}


GT_STATUS prvCpssSip7TxqUtilsPnodeShapingParametersGet
(
    IN GT_U8        devNum,
    IN GT_U32       tileIndex,
    IN GT_U32       pNodeIndex,
    OUT GT_U16      *burstSizePtr,
    OUT GT_U32      *maxRatePtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(maxRatePtr);

    rc = prvCpssSip6TxqPdqShapingRateFromHwGet(devNum, tileIndex,pNodeIndex,
            PRV_CPSS_PDQ_LEVEL_P_E, PRV_QUEUE_SHAPING_ACTION_STOP_TRANSMITION_ENT,burstSizePtr, maxRatePtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}


