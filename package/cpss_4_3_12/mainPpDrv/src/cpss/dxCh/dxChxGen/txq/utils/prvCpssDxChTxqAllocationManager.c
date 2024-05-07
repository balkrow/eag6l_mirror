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
* @file prvCpssDxChTxqAllocationManager.c
*
* @brief CPSS SIP6 TXQ  dynamic resource allocations  operation functions
*
* @version   1
********************************************************************************
*/
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqAllocationManager.h>

static GT_VOID prvDxChTxqSip7AllocManagerResourceMerge
(
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC *headPtr
)
{
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC *iterator,*tmp;

    while(headPtr)
    {
        iterator =headPtr;
        while(iterator->next && (iterator->last ==(iterator->next->first-1)))
        {
            tmp = iterator->next;
            iterator->last = iterator->next->last;
            iterator->next = iterator->next->next;
            cpssOsFree(tmp);
        }
        headPtr=headPtr->next;
    }
}


GT_STATUS prvDxChTxqSip7AllocManagerAddResourceRange
(
    INOUT PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr,
    IN GT_U32  first,
    IN GT_U32  last
)
{
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC *current;
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC *iterator;

    current = (PRV_CPSS_TXQ_RESOURCE_RANGE_STC *)cpssOsMalloc(sizeof(PRV_CPSS_TXQ_RESOURCE_RANGE_STC));
    if(NULL==current)
    {
         CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    current->first=first;
    current->last = last;
    current->next = NULL;

    if(NULL == *headPtr)
    {
       *headPtr =  current;
       return GT_OK;
    }

    iterator = *headPtr;

    while(iterator)
    {
        if(iterator->first >current->first)
        {
            /*insert after and then swap fields*/
            current->next = iterator->next;
            iterator->next = current;
            first = iterator->first;
            last= iterator->last;
            iterator->first = current->first;
            iterator->last = current->last;
            current->first = first;
            current->last = last;
            break;

        }
        if(NULL==iterator->next)
        {
            iterator->next = current;
            break;
        }
        iterator=iterator->next;
    }

    prvDxChTxqSip7AllocManagerResourceMerge(*headPtr);

    return GT_OK;
}

GT_STATUS prvDxChTxqSip7AllocManagerRangeGet
(
    IN PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr,
    IN  GT_U32    size,
    OUT GT_U32  * firstPtr
)
{
    GT_U32 delta = 0xFFFFFFFF;
    GT_U32 rangeSize;
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC * candidatePtr=NULL,**candidatePrevPtr = NULL;
    GT_STATUS rc = GT_NOT_FOUND;
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC **prevPtr,*currentPtr;

    prevPtr= headPtr;
    currentPtr=*headPtr;

    while(currentPtr)
    {
        rangeSize = currentPtr->last - currentPtr->first+1;
        if(rangeSize>=size)
        {
            if(rangeSize - size <delta)
            {
                delta =rangeSize - size ;
                candidatePtr = currentPtr;
                candidatePrevPtr = prevPtr;
                if(0==delta)
                {   /*best option*/
                    break;
                }
            }
        }
        prevPtr = &(currentPtr->next);
        currentPtr =currentPtr->next;
    }

    if(candidatePtr)
    {
        *firstPtr = candidatePtr->first;
        if(0 == delta)
        {
            *candidatePrevPtr = candidatePtr->next;
            cpssOsFree(candidatePtr);
        }
        else
        {
            candidatePtr->first+=size;
        }


        rc = GT_OK;
    }

    return rc;
}


GT_STATUS prvDxChTxqSip7AllocManagerRemoveResourceRange
(
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr,
    IN GT_U32  first,
    IN GT_U32  last
)
{

    PRV_CPSS_TXQ_RESOURCE_RANGE_STC **prevPtr,*currentPtr,* splitPtr;

    prevPtr= headPtr;
    currentPtr=*headPtr;

    while(currentPtr)
    {
        if(currentPtr->first==first)
        {
            if(currentPtr->last==last)
            {
                *prevPtr=currentPtr->next;
                cpssOsFree(currentPtr);
                return GT_OK;
            }

            if(currentPtr->last<last)
            {
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "currentPtr->last %d < last %d",
                currentPtr->last,last);
            }

            currentPtr->first = last+1;
            return GT_OK;
        }
        if(currentPtr->last==last)
        {
            if(currentPtr->first >first )
            {
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "currentPtr->first %d < first %d",
                currentPtr->first,first);
            }
            currentPtr->last= first-1;
            return GT_OK;
        }

        if((currentPtr->last>last)&&
            (currentPtr->first<first))
        {

            /*we have to split*/
            splitPtr = (PRV_CPSS_TXQ_RESOURCE_RANGE_STC *)cpssOsMalloc(sizeof(PRV_CPSS_TXQ_RESOURCE_RANGE_STC));
            if(NULL==splitPtr)
            {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            splitPtr->first=last+1;
            splitPtr->last=currentPtr->last;
            currentPtr->last =first-1;
            splitPtr->next = currentPtr->next;
            currentPtr->next = splitPtr;

            return GT_OK;
         }

        prevPtr = &(currentPtr->next);
        currentPtr = currentPtr->next;
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

}

static GT_VOID prvDxChTxqSip7AllocManagerFreeRange
(
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC **headPtr
)
{
    PRV_CPSS_TXQ_RESOURCE_RANGE_STC*tmp;

    while(*headPtr)
    {
        tmp =*headPtr;
        *headPtr = (*headPtr)->next;
        cpssOsFree(tmp);
    }
}


GT_STATUS prvDxChTxqSip7AllocManagerFreeAllRanges
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{

    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION * mappingPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_U32 i;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);

    mappingPtr = &(tileConfigsPtr->mapping);

    prvDxChTxqSip7AllocManagerFreeRange(&PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).cncResources);

    for(i=0;i<PRV_CPSS_MAX_LEVELS_NUM_CNS;i++)
    {
        prvDxChTxqSip7AllocManagerFreeRange(&mappingPtr->pdqResources[i]);
    }

        for(i=0;i<MAX_DP_IN_TILE(devNum);i++)
    {
        prvDxChTxqSip7AllocManagerFreeRange(&mappingPtr->sdqResources[i]);
    }

    return GT_OK;

}



GT_STATUS prvDxChTxqSip7AllocManagerInitAllRanges
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum
)
{

    PRV_CPSS_DXCH_TXQ_SIP_6_MAPPING_CONFIGURATION * mappingPtr;
    PRV_CPSS_DXCH_TXQ_SIP_6_CONFIGURATION * tileConfigsPtr;
    GT_U32 i;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    tileConfigsPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->port.tileConfigsPtr[tileNum]);

    mappingPtr = &(tileConfigsPtr->mapping);

    PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).cncResources = NULL;

    for(i=0;i<PRV_CPSS_MAX_LEVELS_NUM_CNS;i++)
    {
        mappingPtr->pdqResources[i] = NULL;
    }

    for(i=0;i<MAX_DP_IN_TILE(devNum);i++)
    {
       mappingPtr->sdqResources[i] = NULL;
    }

    return GT_OK;

}






