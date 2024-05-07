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
* @file prvCpssDxChTxqQbvUtils.c
*
* @brief CPSS SIP6 TXQ  qvb  high level functions
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqSearchUtils.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqPreemptionUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqSdqRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/seahawk/pipe/prvCpssAasTxqSdqRegFile.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqQbvUtils.h>



extern GT_STATUS cpssDxChStreamEgressPortBindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
);

/* Maximum number of retry read attempts on re-configuration pending status bit */
#define PRV_QBV_MAX_NUM_OF_RETRY_COUNT_CNS    10

#define PRV_QBV_CAPABILITY_ERROR_CODE_WRONG_MAPPING_TYPE   0x0
#define PRV_QBV_CAPABILITY_ERROR_CODE_CSCD_PORT   0x1
#define PRV_QBV_CAPABILITY_ERROR_NOT_EXP_CHANNEL   0x2





/**
* @internal prvCpssSip6_30TxqSdqLocalPortQbvEnableSet function
* @endinternal
*
* @brief   Enable/disable  QVB for  physical  port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               -physical port number
* @param[in] enable                - QVB enable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqPortQbvEnableSet
(
 IN GT_U8   devNum,
 IN GT_PHYSICAL_PORT_NUM portNum,
 IN GT_BOOL enable
)
{
    GT_STATUS rc;
    GT_U32 tileNum, dpNum,localdpPortNum[2];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT preemptionPortType;
    GT_BOOL isPreemptionEnabled;

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &(localdpPortNum[0]), &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    if (mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Wrong mapping type %d for portNum  %d  ",mappingType, portNum);
    }

    /*double check that this is express channel*/
    rc = prvCpssDxChTxqSip6_10LocalPortTypeGet(devNum,0,localdpPortNum[0],&preemptionPortType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqSip6_10LocalPortTypeGet failed  ");
    }
    if (preemptionPortType != PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Wrong preemption type %d for portNum  %d  ",preemptionPortType, portNum);
    }


    rc = prvCpssSip6_30TxqSdqLocalPortQbvEnableSet(devNum,tileNum,dpNum,localdpPortNum[0],enable);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqLocalPortQbvEnableSet failed  ");
    }

    /*if preemption is enabled ,then also handle preemptive channel*/
    rc = prvCpssDxChTxqSip6_10PreemptionEnableGet(devNum, portNum, 0, &isPreemptionEnabled);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"prvCpssDxChTxqSip6_10PreemptionEnableGet: error for portNum = %d\n", portNum);
    }

    if(GT_TRUE == isPreemptionEnabled)
    {
        rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum,0,localdpPortNum[0], &(localdpPortNum[1]));
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssSip6_30TxqSdqLocalPortQbvEnableSet(devNum,tileNum,dpNum,localdpPortNum[1],enable);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqLocalPortQbvEnableSet failed  ");
        }
    }

    return rc;


}


/**
* @internal prvCpssSip6_30TxqSdqLocalPortQbvEnableGet function
* @endinternal
*
* @brief   Get enable/disable  QVB for  physical  port(required for QVB feature configuration)
*
* @note   APPLICABLE DEVICES: Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number
* @param[in] portNum               -physical port number
* @param[out] enable                - QVB enable
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6_30TxqPortQvbEnableGet
(
 IN GT_U8   devNum,
 IN GT_PHYSICAL_PORT_NUM portNum,
 OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32 tileNum, dpNum,localdpPortNum;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum,NULL);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqLocalPortQbvEnableGet(devNum,tileNum,dpNum,localdpPortNum,enablePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqLocalPortQbvEnableSet failed  ");
    }

    return rc;


}


/**
* @internal prvCpssSip6_30TxqPortQueueGateSet function
* @endinternal
*
* @brief   Sets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[in] gate                       - gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqPortQueueGateSet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    IN GT_U32 gate
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * aNodePtr[2];
    GT_U32   iterationNum = 1;
    GT_U32   i;

    PRV_CPSS_DXCH_SIP_6_QUEUE_OFFSET_VALIDATE_MAC(devNum,portNum,queueOffset);

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &aNodePtr[0]);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    /*if preemption is enabled ,then also handle preemptive channel*/

    if(GT_TRUE == aNodePtr[0]->preemptionActivated)
    {
       rc = prvCpssDxChTxqSip6PreeptivePartnerAnodeGet(devNum,aNodePtr[0],&(aNodePtr[1]));
       if (rc != GT_OK)
       {
           return rc;
       }

       iterationNum = 2;
    }

    for(i=0;i<iterationNum;i++)
    {
        rc = prvCpssSip6_30TxqSdqQueueGateSet(devNum,aNodePtr[i]->queuesData.tileNum,aNodePtr[i]->queuesData.dp,aNodePtr[i]->queuesData.queueBase + queueOffset,gate);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQueueGateSet failed  ");
        }
    }

    return rc;


}



/**
* @internal prvCpssSip6_30TxqPortQueueGateGet function
* @endinternal
*
* @brief   Gets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[out] gatePtr                       - gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqPortQueueGateGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    OUT GT_U32 *gatePtr
)
{
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr;

    CPSS_NULL_PTR_CHECK_MAC(gatePtr);

    PRV_CPSS_DXCH_SIP_6_QUEUE_OFFSET_VALIDATE_MAC(devNum,portNum,queueOffset);

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqQueueGateGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,anodePtr->queuesData.queueBase +queueOffset,gatePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQueueGateGet failed  ");
    }

    return rc;


}

/**
* @internal prvCpssSip6_30TxqQbvChangeTimeSet function
* @endinternal
*
* @brief   Set time for table to start working
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] seconds                  - second value of the time stamping TOD counter
*                                                          48 LSBits are significant.
* @param[in] nanoSeconds            nanosecond value of the time stamping TOD counter. 29 LSBits are significant.
* @param[in] delay                           nanosecond value of start delay .16 LSBits are significant.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS prvCpssSip6_30TxqQbvChangeTimeSet
(
    IN GT_U8   devNum,
    IN GT_U64  seconds,
    IN GT_U32  nanoSeconds,
    IN GT_U32  delay
)
{
    PRV_CPSS_DXCH_SIP6_TXQ_SDQ_QBV_CONFIG_CHANGE_TIME_HW_FORMAT_CFG cfgtime;

    if(nanoSeconds >= (GT_U32)1<<29)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "nanoSeconds value is too big %d ,should be smaller then %d",nanoSeconds,(GT_U32)1<<29);
    }
    if(seconds.l[1]>0xFFFF)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "seconds msb is too big %d ,should be smaller then %d",seconds.l[1],0xFFFF);
    }


    if(delay >= (GT_U32)1<<16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,
            "execution delay value is too big %d ,should be smaller then %d",delay,(GT_U32)1<<16);
    }

    cfgtime.lsb = nanoSeconds;
    cfgtime.lsb|=((seconds.l[0]&0x3)<<30);
    cfgtime.msbLow =((seconds.l[0])>>2);
    cfgtime.msbLow|=((seconds.l[1]&0x3)<<30);
    cfgtime.msbHi=((seconds.l[1])>>2);
    cfgtime.extTime = delay;

    return prvCpssSip6_30TxqSdqChangeTimeSet(devNum,0,0,&cfgtime);

}

GT_STATUS prvCpssSip6_30TxqPortQbvCapabilityGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL             *qbvCapablePtr,
    OUT GT_U32              *errCodePtr
)
{

    GT_STATUS rc;
    CPSS_DXCH_DETAILED_PORT_MAP_STC *   portMapShadowPtr;
    PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_ENT preemptionPortType;
    GT_BOOL isCascade ;

    *qbvCapablePtr = GT_FALSE;

    rc = prvCpssDxChPortPhysicalPortMapShadowDBGet(devNum, portNum, /*OUT*/ &portMapShadowPtr);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (GT_FALSE == portMapShadowPtr->valid)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    if (portMapShadowPtr->portMap.mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E)
    {
        rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum, &isCascade, NULL);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsIsCascadePort  failed for portNum  %d  ", portNum);
        }

        if(GT_FALSE == isCascade )
        {
            /*double check that this is express channel*/
            rc = prvCpssDxChTxqSip6_10LocalPortTypeGet(devNum,0,portMapShadowPtr->extPortMap.localPortInDp, &preemptionPortType);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssDxChTxqSip6_10LocalPortTypeGet failed  ");
            }

            /*QBV can be set only on express channel (Even if preemption is no activated .This is design limitation due to reduced table size.)*/
            if (preemptionPortType == PRV_DXCH_TXQ_SIP6_PREEMTION_PORT_TYPE_EXP_E)
            {
                *qbvCapablePtr = GT_TRUE;
            }
            else
            {
                *errCodePtr = PRV_QBV_CAPABILITY_ERROR_NOT_EXP_CHANNEL;;
            }
        }
        else
        {
            *errCodePtr = PRV_QBV_CAPABILITY_ERROR_CODE_CSCD_PORT;
        }
    }
    else
    {
        *errCodePtr = PRV_QBV_CAPABILITY_ERROR_CODE_WRONG_MAPPING_TYPE;
    }

    return GT_OK;
}


/**
* @internal prvCpssSip6_30TxqQbvBindPortToTableSet function
* @endinternal
*
* @brief   Bind port to table set
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -    physical port number

* @param[in] bindEntryPtr            (pointer to ) bind parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvBindPortToTableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
)
{
    GT_STATUS rc;
    GT_U32    localPort;
    GT_BOOL   preemptionAllowed;

    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr;

    CPSS_NULL_PTR_CHECK_MAC(bindEntryPtr);

    if(tableSet>=PRV_CPSS_DXCH_STREAM_EGRESS_TABLESET_NUM_CNS(devNum))
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " tableSet  too big  %d must be smaller then %d ",
          tableSet,PRV_CPSS_DXCH_STREAM_EGRESS_TABLESET_NUM_CNS(devNum));
    }

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    if(CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_EXP_E == bindEntryPtr->gateReconfigRequestParam.channelType)
    {
        localPort = anodePtr->queuesData.localPort;
    }
    else
    {
        rc = prvCpssFalconTxqUtilsPortPreemptionAllowedGet(devNum,portNum,&preemptionAllowed);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(GT_FALSE==preemptionAllowed)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Port  %d  does not have preemptive channel mapped", portNum);
        }

        rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum,0,anodePtr->queuesData.localPort, &localPort);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = prvCpssSip6_30TxqQbvChangeTimeSet(devNum,bindEntryPtr->gateReconfigTimeParam.todMsb,bindEntryPtr->gateReconfigTimeParam.todLsb,
        bindEntryPtr->gateReconfigTimeParam.configChangeTimeExtension);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqQbvChangeTimeSet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqCycleTimeSet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        bindEntryPtr->gateReconfigRequestParam.cycleTime);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqQbvChangeTimeSet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqEgressTodOffsetSet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        bindEntryPtr->gateReconfigRequestParam.egressTodOffset);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqEgressTodOffsetSet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqBitsFactorResolutionSet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,bindEntryPtr->gateReconfigRequestParam.bitsFactorResolution);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqBitsFactorResolutionSet  failed for portNum  %d  ", portNum);
    }

     rc =prvCpssSip6_30TxqSdqMaxAlwdBcOffsetSet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,bindEntryPtr->gateReconfigRequestParam.maxAlwdBcOffset);

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqMaxAlwdBcOffsetSet  failed for portNum  %d  ", portNum);
    }

    /*should be  last since trigger is set here*/
    rc = prvCpssSip6_30TxqSdqChangeReqSet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,tableSet,&(bindEntryPtr->gateReconfigRequestParam));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqChangeReqSet  failed for portNum  %d  ", portNum);
    }

    return GT_OK;
}

/**
* @internal prvCpssSip6_30TxqQbvBindPortToTableGet function
* @endinternal
*
* @brief   Get  port to tableset binding info
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -    physical port number
* @param[out] tableSetPtr            (pointer to ) tablseset index
* @param[out] bindEntryPtr            (pointer to ) bind parameters
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvBindPortToTableGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               * tableSetPtr,
    INOUT CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_TXQ_SIP_6_A_NODE * anodePtr;
    GT_BOOL   preemptionAllowed;
    GT_U32    localPort;

    CPSS_NULL_PTR_CHECK_MAC(tableSetPtr);
    CPSS_NULL_PTR_CHECK_MAC(bindEntryPtr);

    rc = prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber(devNum, portNum, &anodePtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssFalconTxqUtilsFindAnodeByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

    if(CPSS_DXCH_STREAM_EGRESS_PORT_RECONFIG_CHANNEL_TYPE_EXP_E == bindEntryPtr->gateReconfigRequestParam.channelType)
    {
        localPort = anodePtr->queuesData.localPort;
    }
    else
    {
        rc = prvCpssFalconTxqUtilsPortPreemptionAllowedGet(devNum,portNum,&preemptionAllowed);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(GT_FALSE==preemptionAllowed)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " Port  %d  does not have preemptive channel mapped", portNum);
        }

        rc = prvCpssDxChTxqSip6_10PreChannelGet(devNum,0,anodePtr->queuesData.localPort, &localPort);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc =  prvCpssSip6_30TxqSdqPortTableSetGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,tableSetPtr,&(bindEntryPtr->gateReconfigRequestParam.lastEntry));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqPortTableSetGet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqEgressTodOffsetGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,&(bindEntryPtr->gateReconfigRequestParam.egressTodOffset));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqEgressTodOffsetGet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqCycleTimeGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,&(bindEntryPtr->gateReconfigRequestParam.cycleTime));

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqCycleTimeGet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqPortLengthAwareGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,&(bindEntryPtr->gateReconfigRequestParam.lengthAware),
        &(bindEntryPtr->gateReconfigRequestParam.remainingBitsFactor));

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqPortLengthAwareGet  failed for portNum  %d  ", portNum);
    }

    rc = prvCpssSip6_30TxqSdqBitsFactorResolutionGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,&(bindEntryPtr->gateReconfigRequestParam.bitsFactorResolution));
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqBitsFactorResolutionGet  failed for portNum  %d  ", portNum);
    }

    rc =prvCpssSip6_30TxqSdqBMaxAlwdBcOffsetGet(devNum,anodePtr->queuesData.tileNum,anodePtr->queuesData.dp,
        localPort,&(bindEntryPtr->gateReconfigRequestParam.maxAlwdBcOffset));

    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqBMaxAlwdBcOffsetGet  failed for portNum  %d  ", portNum);
    }

    return rc;
}


/**
* @internal prvCpssSip6_30TxqQbvConfigureTableSet function
* @endinternal
*
* @brief  Configure tableset
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tableSet                    -   tableset index
* @param[in] slotsNum                    -   number of slots to fill
* @param[in] timeSlots           array of slots configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvConfigureTableSet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    IN CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlots
)
{
     GT_U32 i;
     GT_U32 rc = GT_OK,gatesBmp,nextIndex;

     if(tableSet>=PRV_CPSS_DXCH_STREAM_EGRESS_TABLESET_NUM_CNS(devNum))
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " tableSet  too big  %d must be smaller then %d ",
           tableSet,PRV_CPSS_DXCH_STREAM_EGRESS_TABLESET_NUM_CNS(devNum));
     }

     if(slotsNum >PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " slotsNum  too big  %d must be smaller then %d ",
           slotsNum,PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS);
     }


     for(i=0;i<slotsNum;i++)
     {
        if(timeSlots[i].gateStateBmp >=(1<<TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE))
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " gateStateBmp  too big  %d must be smaller then 0x%x ",
              slotsNum,(1<<TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE));
        }
        gatesBmp = timeSlots[i].gateStateBmp;
        if(GT_TRUE==timeSlots[i].hold)
        {
            gatesBmp|=(1<<TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE);
        }


        rc = prvCpssSip6_30TxqSdqQbvCalendarSet(devNum,0,0,tableSet,i,timeSlots[i].timeToAdvance,
            gatesBmp,GT_TRUE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQbvCalendarSet failed for tableSet %d index %d\n",tableSet,i);
        }

        nextIndex = (i+1)%slotsNum;

        gatesBmp = timeSlots[nextIndex].gateStateBmp;
        if(GT_TRUE==timeSlots[nextIndex].hold)
        {
            gatesBmp|=(1<<TXQ_IRONMAN_SDQ_QBV_CFG_GATE_STATE_CURRENT_FIELD_SIZE);
        }

        rc = prvCpssSip6_30TxqSdqQbvCalendarSet(devNum,0,0,tableSet,i,timeSlots[i].timeToAdvance /*don't care*/,
            gatesBmp,GT_FALSE);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQbvCalendarSet failed for tableSet %d index %d\n",tableSet,i);
        }
     }

     return rc;
}

/**
* @internal prvCpssSip6_30TxqQbvConfigureTableGet function
* @endinternal
*
* @brief  Get tableset entries
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] tableSet                    -   tableset index
* @param[in] slotsNum                    -   number of slots to fill
* @param[out] timeSlots           array of slots configurations
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssSip6_30TxqQbvConfigureTableGet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    OUT CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlots
)
{
     GT_U32 i;
     GT_U32 rc = GT_OK;

     if(tableSet>=PRV_CPSS_DXCH_STREAM_EGRESS_TABLESET_NUM_CNS(devNum))
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " tableSet  too big  %d must be smaller then %d ",
           tableSet,PRV_CPSS_DXCH_STREAM_EGRESS_TABLESET_NUM_CNS(devNum));
     }

     if(slotsNum >PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS)
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, " slotsNum  too big  %d must be smaller then %d ",
           slotsNum,PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS);
     }

     for(i=0;i<slotsNum;i++)
     {
        rc = prvCpssSip6_30TxqSdqQbvCalendarGet(devNum,0,0,tableSet,i,timeSlots+i);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqSdqQbvCalendarGet failed for tableSet %d index %d\n",tableSet,i);
        }
     }

    return GT_OK;
}

/**
* @internal prvCpssSip6_30TxqQbvChangeReqCompleteGet function
* @endinternal
*
* @brief  Polls on status of configuration pending bit.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT - after max number of retries checking if pending status bit is cleared
*
*/
GT_STATUS prvCpssSip6_30TxqQbvChangeReqCompleteGet
(
    IN GT_U8  devNum
)
{
    /* Set the number of read attempts */
    GT_STATUS rc;
    GT_U32 retryReadCounter= PRV_QBV_MAX_NUM_OF_RETRY_COUNT_CNS;
    GT_BOOL isComplete = GT_FALSE;

    /* Polls Pending status bit */
    while (retryReadCounter)
    {
        /* Read Pending status bit from the register */
        rc = prvCpssSip6_30TxqSdqChangeReqCompleteGet(devNum, 0, 0, &isComplete);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Check if Pending bit was cleared to '0' by SGC HW unit which means re-configuration time was matched */
        if (GT_TRUE == isComplete)
        {
            break;
        }

        /* Sleep for 1 msec */
        cpssOsTimerWkAfter(1);

        /* Reduce number of read attempts by 1 */
        retryReadCounter--;
    }

    /* If Pending bit is still not cleared return with time out error indication */
    if (GT_FALSE == isComplete)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT,
                                      "Pending status bit was not cleared by Stream Gate Control (SGC) HW unit."
                                      "No match to re-configuration time");
    }

    return GT_OK;
}

GT_STATUS prvCpssSip6_30TxqQbvInstantBindPortToTableSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN GT_U32               numOfEntries
)
{
        GT_STATUS rc;

        CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  bindEntry;
        CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC timeSlots[PRV_CPSS_DXCH_STREAM_EGRESS_TIME_SLOTS_NUM_CNS];

        cpssOsMemSet(&bindEntry,0,sizeof(CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC));

        rc =prvCpssSip6_30TxqQbvConfigureTableGet(devNum,tableSet,numOfEntries,timeSlots);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6_30TxqQbvConfigureTableGet failed for tableSet %d \n",tableSet);
        }

        bindEntry.gateReconfigRequestParam.cycleTime = timeSlots[numOfEntries-1].timeToAdvance;

        return cpssDxChStreamEgressPortBindSet(devNum,portNum,tableSet,&bindEntry);


}

