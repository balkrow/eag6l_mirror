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
* @file prvCpssDxChPortPizzaArbiterProfileStorage.c
*
* @brief CPSS user profile storage
*
* @version   10
********************************************************************************
*/

#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/port/PizzaArbiter/TablePizzaArbiter/prvCpssDxChPortPizzaArbiterProfileStorage.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal pizzaProfileStorageInit function
* @endinternal
*
* @brief   init pizza profile storage
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in,out] storagePtr               - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - storagePtr is NULL
*/
GT_STATUS pizzaProfileStorageInit
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC *storagePtr
)
{
    GT_STATUS rc;
    if (NULL == storagePtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    rc = ActivePortGroupListInit(&storagePtr->portGroupList);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = pizzaProfileInit(&storagePtr->pizzaProfile);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portPizzaCfgSliceListBySlicesNumInit(&storagePtr->regModePizza,0,0);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portPizzaCfgSliceListBySlicesNumInit(&storagePtr->extModePizza,0,0);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portPizzaCfgSlicesListPoolInit      (&storagePtr->portPizzaCfgSliceListPool);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portPrioritiesMatrixInit            (&storagePtr->portPriorityMatr);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}


/**
* @internal pizzaProfileStorageCopy function
* @endinternal
*
* @brief   copy pizza for devNum, portGroupId into storage
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in] devNum                   - dev number
* @param[in] portGroupListPtr         - port group list for which the profile is used
* @param[in] pizzaProfilePtr          - (pointer to ) profile
* @param[in,out] storagePtr               - pointer to iterator
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS pizzaProfileStorageCopy
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC *storagePtr,
       IN GT_U8 devNum,
       IN PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr,
       IN CPSS_DXCH_PIZZA_PROFILE_STC              *pizzaProfilePtr
)
{
    GT_STATUS rc;
    GT_U32 i;

    /*
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    */

    if (NULL == pizzaProfilePtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == storagePtr || NULL == portGroupListPtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }


    rc = portPizzaCfgSlicesListPoolInit(&storagePtr->portPizzaCfgSliceListPool);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portPrioritiesMatrixCopy(/*OUT*/&storagePtr->portPriorityMatr,/*IN*/pizzaProfilePtr->pPortPriorityMatrPtr);
    if (GT_OK != rc)
    {
        return rc;
    }

    rc = portPizzaCfgSliceListBySlicesNumCopy_wAlloc(&storagePtr->regModePizza,
                                                     /*IN*/pizzaProfilePtr->regModePizzaPtr,
                                                     /*IN*/&storagePtr->portPizzaCfgSliceListPool);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (NULL != pizzaProfilePtr->extModePizzaPtr)
    {
        rc = portPizzaCfgSliceListBySlicesNumCopy_wAlloc(&storagePtr->extModePizza,
                                                         /*IN*/pizzaProfilePtr->extModePizzaPtr,
                                                         /*IN*/&storagePtr->portPizzaCfgSliceListPool);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    rc = pizzaProfileInit(&storagePtr->pizzaProfile);
    if (GT_OK != rc)
    {
        return rc;
    }
    storagePtr->pizzaProfile.portNum         = pizzaProfilePtr->portNum;
    storagePtr->pizzaProfile.maxSliceNum     = pizzaProfilePtr->maxSliceNum;
    storagePtr->pizzaProfile.txQDef.pizzaRepetitionNum    = pizzaProfilePtr->txQDef.pizzaRepetitionNum;
    storagePtr->pizzaProfile.txQDef.cpuPortDef.type       = pizzaProfilePtr->txQDef.cpuPortDef.type;
    storagePtr->pizzaProfile.txQDef.cpuPortDef.cpuPortNum = pizzaProfilePtr->txQDef.cpuPortDef.cpuPortNum;
    for (i = 0;
         i < sizeof(pizzaProfilePtr->txQDef.cpuPortDef.cpuPortSlices)/sizeof(pizzaProfilePtr->txQDef.cpuPortDef.cpuPortSlices[0]);
         i ++)
    {
        storagePtr->pizzaProfile.txQDef.cpuPortDef.cpuPortSlices[i] = pizzaProfilePtr->txQDef.cpuPortDef.cpuPortSlices[i];
    }

    storagePtr->pizzaProfile.regModePizzaPtr = &storagePtr->regModePizza;
    if (NULL != pizzaProfilePtr->extModePizzaPtr)
    {
        storagePtr->pizzaProfile.extModePizzaPtr = &storagePtr->extModePizza;
    }
    storagePtr->pizzaProfile.pPortPriorityMatrPtr = &storagePtr->portPriorityMatr;
    storagePtr->devNum       = devNum;
    rc = ActivePortGroupListCopy(&storagePtr->portGroupList,portGroupListPtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}


/**
* @internal pizzaProfileStoragePoolInit function
* @endinternal
*
* @brief   init pizza profile storage pool
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS pizzaProfileStoragePoolInit
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC *poolPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    poolPtr->profileStorageCur = 0;
    poolPtr->profileStorageRemains = sizeof(poolPtr->pizzaProfileStorageArr)/sizeof(poolPtr->pizzaProfileStorageArr[0]);
    for ( i = 0 ; i < poolPtr->profileStorageRemains ; i++)
    {
        rc = pizzaProfileStorageInit(&poolPtr->pizzaProfileStorageArr[i]);
        if (rc != GT_OK)
        {
            return rc;
        }

    }
    return GT_OK;
}


/**
* @internal pizzaProfileStoragePoolFind1 function
* @endinternal
*
* @brief   find pizza profile for devNum portGroupIdx
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port grouop id
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] ppPizzaStoragePtrPtr     - (pointer to) pointer to storage found or NULL
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_FAIL                  - not find
*/
GT_STATUS pizzaProfileStoragePoolFind1
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC *poolPtr,
       IN GT_U8 devNum,
       IN GT_U32 portGroupId,
      OUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC **ppPizzaStoragePtrPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC * pPizzaStoragePtr;
    GT_BOOL isFound;

    /*
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    */

    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == ppPizzaStoragePtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }


    * ppPizzaStoragePtrPtr = NULL;
    pPizzaStoragePtr = &poolPtr->pizzaProfileStorageArr[0];
    for (i = 0 ; i < poolPtr->profileStorageCur; i++,pPizzaStoragePtr++)
    {
        if (pPizzaStoragePtr->devNum == devNum )
        {
            rc = ActivePortGroupListSearch(&pPizzaStoragePtr->portGroupList,portGroupId, /*OUT*/&isFound);
            if (rc != GT_OK)
            {
                return rc;
            }
            if (GT_TRUE == isFound)
            {
                * ppPizzaStoragePtrPtr = pPizzaStoragePtr;
                return GT_OK;
            }
        }
    }
    /* not found */
    return/* this is not an error for LOG */GT_FAIL;
}



/**
* @internal pizzaProfileStoragePoolAlloc function
* @endinternal
*
* @brief   alloc place for pizza profile for devNum portGroupIdx
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] pizzaStoragePtrPtr       - pointer to where the allocated address is stored
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_NO_MORE               - no place in pool
*/
GT_STATUS pizzaProfileStoragePoolAlloc
(
    INOUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC *poolPtr,
      OUT PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_STC     **pizzaStoragePtrPtr
)
{
    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == pizzaStoragePtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (poolPtr->profileStorageRemains == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_MORE, LOG_ERROR_NO_MSG);
    }
    *pizzaStoragePtrPtr = &poolPtr->pizzaProfileStorageArr[poolPtr->profileStorageCur];
    poolPtr->profileStorageCur ++ ;
    poolPtr->profileStorageRemains--;
    return GT_OK;
}


/*----------------------------------------------------------------------*/
/*  storage for speed conv table                                        */
/*----------------------------------------------------------------------*/
/**
* @internal portSpeed2SliceNumPoolInit function
* @endinternal
*
* @brief   init speed 2 slice convertion table storage pool
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
*/
GT_STATUS portSpeed2SliceNumPoolInit
(
    INOUT PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC *poolPtr
)
{
    GT_STATUS rc;
    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    poolPtr->cur = 0;
    poolPtr->remains = sizeof(poolPtr->arr)/sizeof(poolPtr->arr[0]);
    rc = portSpeed2SliceNumListInit(&poolPtr->arr[0],poolPtr->remains);
    return rc;
}

/**
* @internal portSpeed2SliceNumPoolAlloc function
* @endinternal
*
* @brief   alloc speed 2 slice convertion table from storage pool
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in] size                     - how many elements to be allocated
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] itemListPtrPtr           - (pointer to) pointer on allocated items or NULL
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - pPizza or ppSliceList is NULL
* @retval GT_NO_RESOURCE           - no place in pool
*/
GT_STATUS portSpeed2SliceNumPoolAlloc
(
    INOUT PRV_CPSS_DXCH_PORT_SPEED_TO_SLICENUM_CONV_POOL_STC *poolPtr,
       IN GT_U32 size,
      OUT CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC **itemListPtrPtr
)
{
    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == itemListPtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    *itemListPtrPtr = NULL;
    if (poolPtr->remains < size )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    *itemListPtrPtr = &poolPtr->arr[poolPtr->cur];
    poolPtr->cur+=size;
    poolPtr->remains-=size;
    return GT_OK;
}


/**
* @internal speedConvTableStorageInit function
* @endinternal
*
* @brief   init speed comnversion table storage (PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC)
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in,out] storagePtr               - pointer to Profile
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - storagePtr is NULL
*/
GT_STATUS speedConvTableStorageInit
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC *storagePtr
)
{
    GT_STATUS rc;
    if (NULL == storagePtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    storagePtr->devNum = 0;
    rc = ActivePortGroupListInit(&storagePtr->portGroupList);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portGroupSpeed2SliceNumListInit(&storagePtr->portSpeed2SlicesConvTbl);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = portSpeed2SliceNumPoolInit(&storagePtr->portSpeed2SlicesConvPool);
    if (GT_OK != rc)
    {
        return rc;
    }
    return GT_OK;
}



/**
* @internal speedConvTableStorageCopy function
* @endinternal
*
* @brief   copy the speed conversion table for <dev,port group >
*         into the storage place (PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC)
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in,out] storagePtr               - pointer to Profile
* @param[in] devNum                   - device Id
* @param[in] portGroupListPtr         - port group list
* @param[in] portSpeed2SlicesConvTblPtr - speed conversion table it self
* @param[in,out] storagePtr               - pointer to Profile
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - storagePtr or portSpeed2SlicesConvTblPtr is NULL
* @retval GT_NO_RESOURCE           - not enough resources , consider increase constants
*/
GT_STATUS speedConvTableStorageCopy
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC *storagePtr,
       IN GT_U8 devNum,
       IN PRV_CPSS_DXCH_ACTIVE_PORT_GROUP_LIST_STC *portGroupListPtr,
       IN CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *portSpeed2SlicesConvTblPtr
)
{
    GT_U32 i;
    GT_U32 len;
    GT_STATUS rc;
    CPSS_DXCH_PORT_SPEED_TO_SLICENUM_LIST_ITEM_STC * pArrPtr;
    CPSS_DXCH_PORT_SPEED_TO_SLICE_NUM_STC * pPortSpeed2SliceNumListPtr;

    /*
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);
    */

    if (NULL == storagePtr || NULL == portGroupListPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == portSpeed2SlicesConvTblPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    rc = portSpeed2SliceNumPoolInit(&storagePtr->portSpeed2SlicesConvPool);
    if (GT_OK != rc )
    {
        return rc;
    }

    rc = portGroupSpeed2SliceNumListInit(&storagePtr->portSpeed2SlicesConvTbl);
    if (GT_OK != rc )
    {
        return rc;
    }

    storagePtr->devNum       = devNum;
    rc = ActivePortGroupListCopy(&storagePtr->portGroupList,portGroupListPtr);
    if (GT_OK != rc )
    {
        return rc;
    }

    pArrPtr = &portSpeed2SlicesConvTblPtr->arr[0];

    for (i = 0 ; pArrPtr->portN != CPSS_PA_INVALID_PORT ; i++, pArrPtr++)
    {
        rc = PortSpeed2SliceNumListLen(pArrPtr->portSpeed2SliceNumListPtr, /*OUT*/&len);
        if (GT_OK != rc )
        {
            return rc;
        }

        rc = portSpeed2SliceNumPoolAlloc(&storagePtr->portSpeed2SlicesConvPool,
                                          /*IN*/len,
                                          /*OUT*/&pPortSpeed2SliceNumListPtr);
        if (GT_OK != rc )
        {
            return rc;
        }

        rc = portSpeed2SliceNumListCopy(/*OUT*/pPortSpeed2SliceNumListPtr,
                                             /*IN*/pArrPtr->portSpeed2SliceNumListPtr);
        if (GT_OK != rc )
        {
            return rc;
        }

        rc = portGroupSpeed2SliceNumListAppend(&storagePtr->portSpeed2SlicesConvTbl,
                                                     /*IN*/pArrPtr->portN,
                                                     /*IN*/pPortSpeed2SliceNumListPtr);
        if (GT_OK != rc )
        {
            return rc;
        }
    }
    return GT_OK;
}


/*----------------------------------------------------------------------*/
/*  storage Pool for speed conv table                                        */
/*----------------------------------------------------------------------*/

/**
* @internal speedConvTableStoragePoolInit function
* @endinternal
*
* @brief   init pool speed conversion table storages to empty one
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS speedConvTableStoragePoolInit
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC *poolPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    poolPtr->storageCur = 0;
    poolPtr->storageRemains = sizeof(poolPtr->speedConvTableStorageArr)/sizeof(poolPtr->speedConvTableStorageArr[0]);
    for ( i = 0 ; i < poolPtr->storageRemains ; i++)
    {
        rc = speedConvTableStorageInit(&poolPtr->speedConvTableStorageArr[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;

}

/**
* @internal speedConvTableStoragePoolFind function
* @endinternal
*
* @brief   search in pool speed conversion table storages for table
*         for specific <dev , port group>
*
* @note   APPLICABLE DEVICES:      Lion2.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in] devNum                   - device Id
* @param[in] portGroupId              - port group idx
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] ppSpeedConvTblStoragePtrPtr - pointer to place where the pointer to found table is stored;
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS speedConvTableStoragePoolFind
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC *poolPtr,
       IN GT_U8 devNum,
       IN GT_U32 portGroupId,
      OUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC **ppSpeedConvTblStoragePtrPtr
)
{
    GT_STATUS rc;
    GT_U32 i;
    GT_BOOL isFound;
    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC * speedConvTableStoragePtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E);

    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == ppSpeedConvTblStoragePtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    *ppSpeedConvTblStoragePtrPtr = NULL;
    speedConvTableStoragePtr = &poolPtr->speedConvTableStorageArr[0];
    for (i = 0 ; i < poolPtr->storageCur; i++,speedConvTableStoragePtr++)
    {
        if (speedConvTableStoragePtr->devNum == devNum)
        {
            rc = ActivePortGroupListSearch(&speedConvTableStoragePtr->portGroupList,portGroupId,/*OUT*/&isFound);
            if (GT_OK != rc)
            {
                return rc;
            }
            if (GT_TRUE == isFound)
            {
                *ppSpeedConvTblStoragePtrPtr = speedConvTableStoragePtr;
                return GT_OK;
            }
        }
    }
    /* not found */
    return/* this is not an error for LOG */GT_FAIL;
}

/**
* @internal speedConvTableStoragePoolAlloc function
* @endinternal
*
* @brief   alloc place from pool of speed conversion table storages
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] poolPtr                  - pointer to pool
* @param[in,out] poolPtr                  - pointer to pool
*
* @param[out] ppSpeedConvTblStoragePtrPtr - pointer to place where the pointer to alloced
*                                      storage is stored;
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
* @retval GT_NO_RESOURCE           - no place at pool
*/
GT_STATUS speedConvTableStoragePoolAlloc
(
    INOUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC *poolPtr,
      OUT PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC     **ppSpeedConvTblStoragePtrPtr
)
{
    if (NULL == poolPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (NULL == ppSpeedConvTblStoragePtrPtr)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }
    if (poolPtr->storageRemains == 0)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
    }
    *ppSpeedConvTblStoragePtrPtr = &poolPtr->speedConvTableStorageArr[poolPtr->storageCur];
    poolPtr->storageCur ++ ;
    poolPtr->storageRemains--;
    return GT_OK;

}


/*----------------------------------------------------------------------*/
/*  storage for speed conv table                                        */
/*----------------------------------------------------------------------*/

/**
* @internal userProfileSpeedConvTableStorageInit function
* @endinternal
*
* @brief   init pools of user profile storage and speed conversion table storages to empty one
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in,out] storagePtr               - pointer to pool
* @param[in,out] storagePtr               - pointer to pool
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - poolPtr is NULL
*/
GT_STATUS userProfileSpeedConvTableStorageInit
(
    INOUT PRV_CPSS_DXCH_PROFILE_SPEED_CONV_TBL_STORAGE_STC *storagePtr
)
{
    GT_STATUS rc;

    rc = pizzaProfileStoragePoolInit(&storagePtr->pizzaProfilePool);
    if (GT_OK != rc)
    {
        return rc;
    }
    rc = speedConvTableStoragePoolInit(&storagePtr->speedConvTablePool);
    if (GT_OK != rc)
    {
        return rc;
    }
    storagePtr->isInit = GT_TRUE;
    return GT_OK;
}

/*----------------------------------------------------------------------*/
/*  storage for speed conv table                                        */
/*----------------------------------------------------------------------*/

/*******************************************************************************
* pizzaProfileStoragePoolGet
*
* DESCRIPTION:
*      get user profile storage pool from global variable
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*    PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC* pointer to storage
*
* COMMENTS:
*    if global storage g_usedProfileSpeedConvTableStorage (invisible for user)
*         is not initialized already, it is initilized first
*
*******************************************************************************/
PRV_CPSS_DXCH_PORT_PIZZA_PROFILE_STORAGE_POOL_STC * pizzaProfileStoragePoolGet(void)
{
    if (PRV_SHARED_PA_DB_VAR(g_usedProfileSpeedConvTableStorage.isInit) == GT_FALSE)
    {
        userProfileSpeedConvTableStorageInit(
            &PRV_SHARED_PA_DB_VAR(g_usedProfileSpeedConvTableStorage));
    }
    return &PRV_SHARED_PA_DB_VAR(
        g_usedProfileSpeedConvTableStorage.pizzaProfilePool);
}

/*******************************************************************************
* speedConvTableStoragePoolGet
*
* DESCRIPTION:
*      get user spped conversion storage pool from global variable
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC* pointer to storage
*
* COMMENTS:
*    if global storage g_usedProfileSpeedConvTableStorage (invisible for user)
*         is not initialized already, it is initilized first
*
*******************************************************************************/
PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC * speedConvTableStoragePoolGet
(
    void
)
{
    if (PRV_SHARED_PA_DB_VAR(g_usedProfileSpeedConvTableStorage.isInit) == GT_FALSE)
    {
        userProfileSpeedConvTableStorageInit(
            &PRV_SHARED_PA_DB_VAR(g_usedProfileSpeedConvTableStorage));
    }
    return &PRV_SHARED_PA_DB_VAR(
        g_usedProfileSpeedConvTableStorage.speedConvTablePool);
}



/**
* @internal prvCpssDxChPizzaArbiterSelectSlicesBySpeed function
* @endinternal
*
* @brief   selection slice number by speed
*         shall be used in cpss blocks that uses Pizza Arbiter
*
* @note   APPLICABLE DEVICES:     xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES: None.
*
* @param[in] devNum                   - device number
* @param[in] portGroupId              - port group
* @param[in] localPort                - local port
* @param[in] portSpeed                - port speed
*
* @param[out] sliceNumUsedPtr          - number of slices to be used if speed is supported,  NULL othewise
*                                       GT_OK.       if speed is supported
*                                       GT_NOT_SUPPORTED  otherwise
*/
GT_STATUS prvCpssDxChPizzaArbiterSelectSlicesBySpeed
(
    IN GT_U8               devNum,
    IN GT_U32              portGroupId,
    IN GT_U32              localPort,
    IN CPSS_PORT_SPEED_ENT portSpeed,
   OUT GT_U32             *sliceNumUsedPtr
)
{
    GT_STATUS rc;
    CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC * table2UsePtr;
    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_STC       * pSpeedConvTblStoragePtr;
    PRV_CPSS_DXCH_SPEED_CONV_TBL_STORAGE_POOL_STC   * pPoolPtr;


    pPoolPtr = speedConvTableStoragePoolGet();

    table2UsePtr =
        (CPSS_DXCH_PORT_PIZZA_PORTGROUP_SPEED_TO_SLICENUM_STC *)
        &PRV_SHARED_PA_DB_VAR(prvPortGroupPortSpeed2SliceNumDefault);

    rc = speedConvTableStoragePoolFind(pPoolPtr,devNum,portGroupId,/*OUT*/&pSpeedConvTblStoragePtr);
    if (rc == GT_OK)  /* Found user Table !!!! */
    {
        table2UsePtr = &pSpeedConvTblStoragePtr->portSpeed2SlicesConvTbl;
    }
    return portGroupSpeed2SliceNumListFind(table2UsePtr,
                                           localPort,
                                           portSpeed,
                                           /*OUT*/sliceNumUsedPtr);
}



