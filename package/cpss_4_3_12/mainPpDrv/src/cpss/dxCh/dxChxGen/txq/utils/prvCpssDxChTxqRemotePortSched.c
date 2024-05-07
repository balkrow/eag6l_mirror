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
* @file prvCpssDxChTxqRemotePortSched.c
*
* @brief CPSS SIP6 TXQ remote port scheduling utilities.
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
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_TXQ_REMOTE_PORT_MIN_WEIGHT_CNS 1
#define PRV_TXQ_REMOTE_PORT_MAX_WEIGHT_CNS 256


#define PRV_TXQ_REMOTE_PORT_WEIGHT_VALIDATE(_weight) \
    do\
    {\
        if(_weight<PRV_TXQ_REMOTE_PORT_MIN_WEIGHT_CNS)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Weight %d too small.Should be more or equal then %d\n",\
                _weight,PRV_TXQ_REMOTE_PORT_MIN_WEIGHT_CNS);\
        }\
        if(_weight>PRV_TXQ_REMOTE_PORT_MAX_WEIGHT_CNS)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Weight %d too big.Should be less or equal then %d\n",\
                _weight,PRV_TXQ_REMOTE_PORT_MAX_WEIGHT_CNS);\
        }\
    }\
    while(0);


/**
* @internal prvCpssDxChTxqRemotePortDwrrWeightSet  function
* @endinternal
*
* @brief   Assign WRR weight to physical remote port.
*     This weight will be considered in case of port level DWRR.
*     The  weight will be  used in DWRR between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum -         physical port number
* @param[in] wrrWeight           -    wrr weight[Applicable range : 1..256]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortDwrrWeightSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               wrrWeight
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              tileNum;
    GT_U32              localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32              aNodeNum;
    GT_BOOL             isCascadePort = GT_FALSE;

    PRV_TXQ_REMOTE_PORT_WEIGHT_VALIDATE(wrrWeight);

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
    if(rc != GT_OK)
    {
         return rc;
    }
    if(GT_TRUE==isCascadePort)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Cascade port %d can not be configured with weight\n",portNum);
    }

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*only remote ports support DWRR*/
    if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        /*Find A level index*/
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &aNodeNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
        }
        /*Now we got the tile,A level  index and quantum.Let's normalize and set it*/
        rc = prvCpssSip6TxqPdqAlevelQuantumSet(devNum, tileNum, aNodeNum, wrrWeight*CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPdqAlevelQuantumSet  failed for A node  %d  ", aNodeNum);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Weight can be configured only on remote port.Wrong port type %d\n",mappingType);
    }
    return rc;
}

/**
* @internal prvCpssDxChTxqRemotePortDwrrWeightGet  function
* @endinternal
*
* @brief   Get assigned WRR weight of physical remote port.
*     This weight will be considered in case of port level DWRR.
*     The  weight will be  used in DWRR between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum -         physical port number
* @param[out] wrrWeightPtr           -    (pointer to)wrr weight[Applicable range : 1..256]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortDwrrWeightGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *wrrWeightPtr
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              tileNum;
    GT_U32              localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32              aNodeNum,hwData;
    GT_BOOL             isCascadePort = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(wrrWeightPtr);

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
    if(rc != GT_OK)
    {
         return rc;
    }

    if(GT_TRUE==isCascadePort)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Cascade port %d can not be configured with weight\n",portNum);
    }

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }

     /*only remote ports support DWRR*/
    if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
         /*Find A level index*/
         rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &aNodeNum);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
         }
         /*Now we got the tile,A level  index and quantum.Let's normalize and set it*/
         rc = prvCpssSip6TxqPdqAlevelQuantumGet(devNum, tileNum, aNodeNum, &hwData);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqPdqSetBlevelQuantum  failed for A node  %d  ", aNodeNum);
         }
         *wrrWeightPtr = hwData/CPSS_PDQ_SCHED_MIN_NODE_QUANTUM_CNS(devNum);
     }
     else
     {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Weight can be configured only on remote port.Wrong port type %d\n",mappingType);
     }

    return rc;
}
/**
* @internal prvCpssDxChTxqRemotePortPrioritySet function
* @endinternal
*
* @brief   Assign priority group to physical port. This group will be considered in case of port level scheduling.
* The priority group will be used in scheduling  between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum  -         physical port number
* @param[in] priorityGroup -    scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortPrioritySet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               priorityGroup
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              tileNum;
    GT_U32              localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32              aNodeNum;
    GT_BOOL             isCascadePort = GT_FALSE;

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
    if(rc != GT_OK)
    {
         return rc;
    }
    if(GT_TRUE==isCascadePort)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Cascade port %d can not be configured with priority\n",portNum);
    }

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*only remote ports support setting priority */
    if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        /*Find A level index*/
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &aNodeNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
        }
        /*Now we got the tile,A level  index and quantum.Let's normalize and set it*/
        rc = prvCpssSip6TxqPdqAlevelPrioritySet(devNum, tileNum, aNodeNum, priorityGroup);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPdqAlevelPrioritySet  failed for A node  %d  ", aNodeNum);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Priority can be set only on remote port.Wrong port type %d\n",mappingType);
    }
    return rc;
}

/**
* @internal prvCpssDxChTxqRemotePortPriorityGet function
* @endinternal
*
* @brief  Get  assigned priority group to physical port. This group will be considered in case of port level scheduling.
* The priority group will be used in scheduling  between ports that are mapped to the same DMA.
*
* @note   APPLICABLE DEVICES:Falcon; AC5P; AC5X;Harrier; Ironman,
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum  -         physical port number
* @param[out] priorityGroupPtr -    scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqRemotePortPriorityGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_U32               *priorityGroupPtr
)

{
    GT_STATUS           rc = GT_OK;
    GT_U32              tileNum;
    GT_U32              localdpPortNum, dpNum;
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mappingType;
    GT_U32              aNodeNum;
    GT_BOOL             isCascadePort = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(priorityGroupPtr);

    rc = prvCpssFalconTxqUtilsIsCascadePort(devNum, portNum,&isCascadePort,NULL);
    if(rc != GT_OK)
    {
         return rc;
    }
    if(GT_TRUE==isCascadePort)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Cascade port %d can not be configured with priority\n",portNum);
    }

    rc = prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber(devNum, portNum, &tileNum, &dpNum, &localdpPortNum, &mappingType);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsGetDpAndLocalPortByPhysicalPortNumber  failed for portNum  %d  ", portNum);
    }
    /*only remote ports support setting priority */
    if (mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
    {
        /*Find A level index*/
        rc = prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet(devNum, portNum, &tileNum, &aNodeNum);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsPhysicalNodeToAnodeMappingGet  failed for portNum  %d  ", portNum);
        }
        /*Now we got the tile,A level  index and quantum.Let's normalize and set it*/
        rc = prvCpssSip6TxqPdqAlevelPriorityGet(devNum, tileNum, aNodeNum, priorityGroupPtr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssSip6TxqPdqAlevelPriorityGet  failed for A node  %d  ", aNodeNum);
        }
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Priority can be set only on remote port.Wrong port type %d\n",mappingType);
    }
    return rc;
}

/**
* @internal prvCpssDxChTxqSubServicePrioritySet function
* @endinternal
*
* @brief   Assign priority group to sub service . This sub service priority  will be considered in case of service  level scheduling.
* The priority group will be used in scheduling  between subservices  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum            -physical device number
* @param[in] portNum           -physical port number (should be cascade port)
* @param[in] serviceId         -local service id
* @param[in] subServiceId      -local sub service id
* @param[in] priorityGroup      -scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND -sub service not found
*/
GT_STATUS prvCpssDxChTxqSubServicePrioritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               priorityGroup
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

   rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,portNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
   if (rc != GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

    rc = prvCpssSip6TxqPdqBlevelPrioritySet(devNum, tileIndex,bNodeIndex,priorityGroup);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChTxqSubServicePriorityGet function
* @endinternal
*
* @brief   Get asssigned  priority group to sub service . This sub service priority  will be considered in case of service  level scheduling.
* The priority group will be used in scheduling  between subservices  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[out] priorityGroupPtr-(pointer to)scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -sub service not found
*/

GT_STATUS prvCpssDxChTxqSubServicePriorityGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               *priorityGroupPtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(priorityGroupPtr);

    rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,portNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
   if (rc != GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

    rc = prvCpssSip6TxqPdqBlevelPriorityGet(devNum, tileIndex,bNodeIndex,priorityGroupPtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChTxqServicePrioritySet function
* @endinternal
*
* @brief   Assign priority group to  service . This service priority  will be considered in case of cascade port  level scheduling.
* The priority group will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum            -physical device number
* @param[in] portNum           -physical port number (should be cascade port)
* @param[in] serviceId         -local service id
* @param[in] priorityGroup      -scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND -sub service not found
*/
GT_STATUS prvCpssDxChTxqServicePrioritySet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               priorityGroup
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

   rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,portNum,serviceId,
                   &cNodeIndex,&tileIndex);
   if (rc != GT_OK)
   {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

    rc = prvCpssSip6TxqPdqClevelPrioritySet(devNum, tileIndex,cNodeIndex,priorityGroup);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChTxqServicePriorityGet function
* @endinternal
*
* @brief   Get asssigned  priority group to service . This service priority  will be considered in case of cascade port  level scheduling.
* The priority group will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[out] priorityGroupPtr-(pointer to)scheduling priority group [Applicable range : 0..7]

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -sub service not found
*/
GT_STATUS prvCpssDxChTxqServicePriorityGet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               *priorityGroupPtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(priorityGroupPtr);

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,portNum,serviceId,
                   &cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqClevelPriorityGet(devNum, tileIndex,cNodeIndex,priorityGroupPtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxqServiceDwrrWeightSet function
* @endinternal
*
* @brief   Set weight  to service . This weight  will be considered in case of service  level DWRR.
* The weight will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] wrrWeight        -wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -service not found
*/
GT_STATUS prvCpssDxChTxqServiceDwrrWeightSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               wrrWeight
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

    PRV_TXQ_REMOTE_PORT_WEIGHT_VALIDATE(wrrWeight);

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,portNum,serviceId,
                   &cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqClevelQuantumSet(devNum, tileIndex,cNodeIndex,wrrWeight);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChTxqServiceDwrrWeightGet function
* @endinternal
*
* @brief   Get weight  assigned to service . This weight  will be considered in case of service  level DWRR.
* The weight will be used in scheduling  between services  that are mapped to the same cascade port.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[out] wrrWeight       -(pointer to)wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - service not found
*/
GT_STATUS prvCpssDxChTxqServiceDwrrWeightGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    OUT GT_U32               *wrrWeightPtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,cNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(wrrWeightPtr);

    rc = prvCpssSip7TxqUtilsServiceIdToCnodeIndexGet(devNum,portNum,serviceId,
                   &cNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqClevelQuantumGet(devNum, tileIndex,cNodeIndex,wrrWeightPtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChTxqSubServiceDwrrWeightSet function
* @endinternal
*
* @brief   Set weight  to sub service . This weight  will be considered in case of suub service  level DWRR.
* The weight will be used in scheduling  between sub services  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[in] wrrWeight        -wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             -sub service not found
*/
GT_STATUS prvCpssDxChTxqSubServiceDwrrWeightSet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               wrrWeight
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    PRV_TXQ_REMOTE_PORT_WEIGHT_VALIDATE(wrrWeight);

    rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,portNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqBlevelQuantumSet(devNum, tileIndex,bNodeIndex,wrrWeight);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}
/**
* @internal prvCpssDxChTxqSubServiceDwrrWeightGet function
* @endinternal
*
* @brief   Get weight  assigned to sub service . This weight  will be considered in case of service  level DWRR.
* The weight will be used in scheduling  between sub services  that are mapped to the same service.
*
* @note   APPLICABLE DEVICES: Seahawk
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5;  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2,Falcon; AC5P; AC5X;Harrier; Ironman.
*
* @param[in] devNum           -physical device number
* @param[in] portNum          -physical port number (should be cascade port)
* @param[in] serviceId        -local service id
* @param[in] subServiceId     -local sub service id
* @param[out] wrrWeight       -(pointer to)wrr weight [Applicable range : 1..256]
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong device number, profile set or
*                                       traffic class
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_NOT_FOUND             - sub service not found
*/
GT_STATUS prvCpssDxChTxqSubServiceDwrrWeightGet
(
    IN GT_U8 devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               serviceId,
    IN GT_U32               subServiceId,
    IN GT_U32               *wrrWeightPtr
)
{
    GT_STATUS rc;
    GT_U32 tileIndex,bNodeIndex;

    CPSS_NULL_PTR_CHECK_MAC(wrrWeightPtr);

    rc = prvCpssSip7TxqUtilsSubServiceIdToBnodeIndexGet(devNum,portNum,serviceId,
                   subServiceId,&bNodeIndex,&tileIndex);
    if (rc != GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdqBlevelQuantumGet(devNum, tileIndex,bNodeIndex,wrrWeightPtr);
    if (rc)
    {
        return rc;
    }

    return GT_OK;
}


