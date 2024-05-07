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
* @file cpssDxChNetIfDbg.c
*
* @brief Debug utilities for network interface.
*
* @version   2
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/networkIf/private/prvCpssDxChNetIfDbg.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorMemCheck function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory are arranged
*         to support optimal performance.
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
*
* @param[in] devNum              - Device number.
* @param[in] txQueue             - The Tx SDMA queue to enable.
*
* @param[out] checkResultPtr     - (pointer to) memory check result status
*                                      CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E
*                                      CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E
*                                if Ptr=1 the OK/FAIL is returned over retval (for shell-exec checking)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_FAIL    - if checkResultPtr=1 the result is "added" into retval as OK or FAIL
*
* @note memory check can be performed while queue is enabled.
* @note  Differences between GDMA vs SDMA:
*        - SGD struct is different,
*        - SGD descriptors are not linked into a list,
*        - not all PRV_CPSS_TX_DESC_LIST_STC fields ar in use,
*        - mem-area is (consecutive(N * SGDs) + consecutive(N * Buffers)),
*        - may/should be checked for full and empty queue as well.
*        This API checks:
*        - every SGD points to valid Buffer in the consecutive(N * Buffers) area,
*        - every Buffer is pointed by one only SGD (e.g. no duplication).
*/
static GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorMemCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   txQueue,
    OUT CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT *checkResultPtr
)
{
    GT_STATUS                   rc;
    PRV_CPSS_TX_DESC_LIST_STC   *txDescList;    /* SW-DB Tx desc. list */
    PRV_CPSS_GDMA_DESC_STC      *sgd;
    GT_U32                      maxDescNum;     /* number of SGDs */
    GT_U32                      idx;            /* index 0..maxDescNum */
    GT_U32                      dup;            /* index to check duplication */
    GT_UINTPTR                  phyAddrBegin;   /* physical address of Buffers' area */
    GT_U32                      bufAddrBegin;   /* bits 2..31 of phyAddrBegin */
    GT_U32                      bufAddrEnd;
    GT_U32                      bufAddr;
    CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT result;

    result = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E;

    txDescList = &(PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);
    maxDescNum = txDescList->maxDescNum;
    sgd = (void *)txDescList->firstDescPtr;

    rc = cpssOsVirt2Phy((GT_UINTPTR)(sgd + maxDescNum), &phyAddrBegin);
    if (rc != GT_OK)
        return rc;
    phyAddrBegin |= 0x200000000;
    bufAddrBegin = (GT_U32)(phyAddrBegin >> 2); /* only bits 2..31 */
    bufAddrEnd = bufAddrBegin + maxDescNum * txDescList->actualBuffSize;

    for (idx = 0; idx < maxDescNum; idx++)
    {
        bufAddr = sgd[idx].word1;
        if (bufAddr < bufAddrBegin || bufAddr >= bufAddrEnd)
        {
            CPSS_LOG_ERROR_MAC("txGen Queue=%d sgdIdx=%d - buffer address is out of area",
                                txQueue, idx);
            goto end;
        }
        for (dup = idx + 1; dup < maxDescNum; dup++)
        {
            if (bufAddr == sgd[dup].word1)
            {
                CPSS_LOG_ERROR_MAC("txGen Queue=%d sgdIdx=%d and sgdIdx=%d - buffer duplication",
                                    txQueue, idx, dup);
                goto end;
            }
        }
    }
    result = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E;
end:
    if ((GT_UINTPTR)checkResultPtr == 1)
    {
        /* trick for "shell-execute cpssDxChNetIfSdmaTxGeneratorMemCheck 0,q,1"
         * GT_FAIL value is equal to CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E
         */
        rc = (result == CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E) ? GT_OK : GT_FAIL;
    }
    else
    {
        *checkResultPtr = result;
    }
    return rc;
}

/**
* @internal cpssDxChNetIfSdmaTxGeneratorMemCheck function
* @endinternal
*
* @brief   This function verify that descriptors and buffers memory is arranged
*         to support optimal performance.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - Device number.
* @param[in] txQueue                  - The Tx SDMA queue to enable.
*
* @param[out] checkResultPtr           - (pointer to) memory check result status
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
*
* @note memory check can be performed while queue is enabled.
*
*/
GT_STATUS cpssDxChNetIfSdmaTxGeneratorMemCheck
(
    IN  GT_U8                   devNum,
    IN  GT_U8                   txQueue,
    OUT CPSS_TX_SDMA_GENERATOR_MEM_CHECK_ENT *checkResultPtr
)
{
    GT_STATUS                   rc;
    PRV_CPSS_TX_DESC_LIST_STC   txDescList;   /* Tx desc. list */
    PRV_CPSS_TX_DESC_STC        *currDescPtr;   /* current descriptor pointer */
    PRV_CPSS_TX_DESC_STC        *nextDescPtr;   /* next descriptor pointer */
    GT_UINTPTR  virtAddr;       /* virtual address */
    GT_U32 sizeOfDescAndBuff;   /* The amount of memory (in bytes) that a     */
                                /* single descriptor and adjacent buffer will */
                                /* occupy, including the alignment.           */
    GT_U32   loopCounter;       /* to avoid infinite loop */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_NETIF_INITIALIZED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(checkResultPtr);

    PRV_CPSS_DXCH_DEV_TX_SDMA_QUEUE_NUM_CHECK_MAC(txQueue);

    PRV_CPSS_DXCH_DEV_TX_SDMA_GENERATOR_CHECK_MAC(devNum,txQueue);

    if (PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        rc = prvCpssDxChNetIfGdmaTxGeneratorMemCheck(devNum, txQueue, checkResultPtr);
        return rc;
    }

    txDescList  = (PRV_CPSS_PP_MAC(devNum)->intCtrl.txDescList[txQueue]);

    /* Nothing to check if queue is empty */
    if( txDescList.freeDescNum == txDescList.maxDescNum )
    {
        *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_EMPTY_QUEUE_E;
        return GT_OK;
    }

    sizeOfDescAndBuff = sizeof(PRV_CPSS_TX_DESC_STC) +
                                                    txDescList.actualBuffSize;

    currDescPtr = txDescList.firstDescPtr;

    rc = perDev_cpssOsPhy2Virt(devNum,(GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->nextDescPointer)),
                                &virtAddr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Physical address %p to virtual conversion failed\n",
            (GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->nextDescPointer)));
    }

    nextDescPtr = (PRV_CPSS_TX_DESC_STC*)virtAddr;

    /* In case only one buffer in chain */
    if( txDescList.maxDescNum - txDescList.freeDescNum == 1 )
    {
        rc = perDev_cpssOsPhy2Virt(devNum,(GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->buffPointer)),
                                    &virtAddr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Physical address %p to virtual conversion failed\n",
            (GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->buffPointer)));
        }

        /* Check buffer pointer for coupling descriptor */
        if( (GT_U8*)virtAddr != (GT_U8*)(currDescPtr) + sizeof(PRV_CPSS_TX_DESC_STC) )
        {
            *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E;
            return GT_OK;
        }

        /* Check correct next descriptor address -         */
        /* for only one descriptor it must point to itself */
        if( nextDescPtr != currDescPtr )
        {
            *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_NEXT_DESC_E;
            return GT_OK;
        }

        *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E;
        return GT_OK;
    }

    loopCounter = 0;
    while( nextDescPtr != txDescList.firstDescPtr )
    {
        rc = perDev_cpssOsPhy2Virt(devNum,(GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->buffPointer)),
                                    &virtAddr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Physical address %p to virtual conversion failed\n",
                (GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->buffPointer)));
        }

        /* Check buffer pointer for coupling descriptor */
        if( (GT_U8*)virtAddr != (GT_U8*)(currDescPtr) + sizeof(PRV_CPSS_TX_DESC_STC) )
        {
            *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_BUFF_PTR_E;
            return GT_OK;
        }

        /* Check correct next descriptor address */
        if( nextDescPtr != (PRV_CPSS_TX_DESC_STC*)
                                ((GT_U8*)(currDescPtr) + sizeOfDescAndBuff) )
        {
            *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_BAD_NEXT_DESC_E;
            return GT_OK;
        }

        currDescPtr = nextDescPtr;
        rc = perDev_cpssOsPhy2Virt(devNum,(GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->nextDescPointer)),
                                &virtAddr);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Physical address %p to virtual conversion failed\n",
                (GT_UINTPTR)(CPSS_32BIT_LE(currDescPtr->nextDescPointer)));
        }

        nextDescPtr = (PRV_CPSS_TX_DESC_STC*)virtAddr;

        loopCounter++;
        if( loopCounter > txDescList.maxDescNum )
        {
            *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_DESC_EXCEED_E;
            return GT_OK;
        }
    }

    *checkResultPtr = CPSS_TX_SDMA_GENERATOR_MEM_CHECK_PASS_E;
    return GT_OK;
}


