/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*
*/
/**
********************************************************************************
* @file cpssGenCnMShMem.c
*
* @brief Generic CnM Shared Memory manager ApIs
*
* @version   1
********************************************************************************
*/

#include <cpss/common/cpssHwInit/cpssHwInit.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/CnM/cpssGenericCnMShMem.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <cpssCommon/private/prvCpssMath.h>

/* For cases in which internal CPU is used (a.k.a. CnM) part of the SRAM is
 * used for ATF and no allocation should be made from this range (addresses
 * listed below are relative to SRAM base):
 * 0x0 to 0xc000 (48KB) is left for FW to use
 * 0xc000 to 20000 (80KB) is used by ATF BL31
 * 0x20000 to 0x30000 (64KB) is unused. During boot up this region is used by
 * ATF BL1 but is no more in use once we reach Linux Kernel
*/
#define AC5_INTERNAL_RESERVED_SHARED_MEM_START 0x0C000
#define AC5_INTERNAL_RESERVED_SHARED_MEM_END   0x20000

#define RESERVED_SCPU_NUM 0xFFFFFFFF

/* CnM shared memory free list */
typedef struct PRV_CPSS_CNM_SHMEM_FREE_LIST_STCT{

    /* Offset in shared memory to free block */
    GT_U64 blockOffset;
    /* Size of free block */
    GT_U64 blockSize;

    struct PRV_CPSS_CNM_SHMEM_FREE_LIST_STCT *next;

} PRV_CPSS_CNM_SHMEM_FREE_LIST_STC;

/* CnM shared memory used list */
typedef struct PRV_CPSS_CNM_SHMEM_USED_LIST_STCT{

    struct CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STCT allocBlock;

    struct PRV_CPSS_CNM_SHMEM_USED_LIST_STCT *next;

} PRV_CPSS_CNM_SHMEM_USED_LIST_STC;

/* Free Free linked list */
GT_VOID prvCpssGenericCnMShMemMgrFreeFreeList
(
    IN GT_U8 devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList, *prevFreeList;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    tmpFreeList = devPtr->hwInfo.cnmRam[memType].cnmRamFreeList;

    while( tmpFreeList )
    {
        prevFreeList = tmpFreeList;
        tmpFreeList = tmpFreeList->next;
        cpssOsFree(prevFreeList);
    }

    devPtr->hwInfo.cnmRam[memType].cnmRamFreeList = NULL;

    return;
}

/* Free Used linked list */
GT_VOID prvCpssGenericCnMShMemMgrFreeUsedList
(
    IN GT_U8 devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList, *prevUsedList;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    tmpUsedList = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( tmpUsedList )
    {
        prevUsedList = tmpUsedList;
        tmpUsedList = tmpUsedList->next;
        cpssOsFree(prevUsedList);
    }

    devPtr->hwInfo.cnmRam[memType].cnmRamUsedList = NULL;

    return;
}

/* Create Free linked list */
GT_STATUS prvCpssGenericCnMShMemMgrCreateFreeList
(
    IN GT_U8 devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    tmpFreeList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
    if( tmpFreeList == NULL )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for free list creation");
    }

    /*
     *  To support memory address & size of more than 32 bits, the RAM size
     *  definition in device DB is a 64 bits value (i.e. GT_U64). However to
     *  ease the lists management which requires various mathematical operations
     *  32 bits parameter (i.e. GT_U32) are preferable. Therefore for each
     *  based on its size, we'll normalize the block offset & size parameters
     *  to be in such granularity so we can work wit 32 bit parameters.
     *  For non SIP7 no normalization is required as it is already 32 bits
     *  range. (This is the reason of the *.l[1]=0 in various places.
     */

    /* External CPU - all shared memory is available for allocation */
    tmpFreeList->blockOffset.l[0] = tmpFreeList->blockOffset.l[1] = 0;
    tmpFreeList->blockSize.l[0] = devPtr->hwInfo.cnmRam[memType].cnmRamSize.l[0];
    tmpFreeList->blockSize.l[1] = 0;
    tmpFreeList->next = NULL;
    devPtr->hwInfo.cnmRam[memType].cnmRamFreeList = tmpFreeList;

    if( CPSS_HW_INFO_BUS_TYPE_MBUS_E ==
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].busType)
    {
        /* Internal CPU - memory block reserved for BL3 */

        /* No internal CnM for these devices */
        if ( (devPtr->appDevFamily & CPSS_AC5P_E)    ||
             (devPtr->appDevFamily & CPSS_HARRIER_E) ||
             (devPtr->appDevFamily & CPSS_FALCON_E) )
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE,"Devices with no internal CnM");

        /* Internal CPU - split free before and after memory block reserved for BL3 */
        tmpFreeList->blockSize.l[0] = AC5_INTERNAL_RESERVED_SHARED_MEM_START;
        tmpFreeList->next = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
        if( tmpFreeList->next == NULL )
        {
            prvCpssGenericCnMShMemMgrFreeFreeList(devNum, memType);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for free list reserved entry");
        }

        tmpFreeList = tmpFreeList->next;
        tmpFreeList->blockOffset.l[0] = AC5_INTERNAL_RESERVED_SHARED_MEM_END;
        tmpFreeList->blockOffset.l[1] = 0;
        tmpFreeList->blockSize.l[0] =
            devPtr->hwInfo.cnmRam[memType].cnmRamSize.l[0] -
                                        AC5_INTERNAL_RESERVED_SHARED_MEM_END;
        tmpFreeList->blockSize.l[1] = 0;
        tmpFreeList->next = NULL;

        /* Internal CPU - memory block reserved for BL3 */
        tmpUsedList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_USED_LIST_STC));
        if( tmpUsedList == NULL )
        {
            prvCpssGenericCnMShMemMgrFreeFreeList(devNum, memType);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for used list creation");
        }

        tmpUsedList->allocBlock.blockOffset.l[0] =
                                        AC5_INTERNAL_RESERVED_SHARED_MEM_START;
        tmpUsedList->allocBlock.blockOffset.l[1] = 0;
        tmpUsedList->allocBlock.blockSize.l[0] =
                                   AC5_INTERNAL_RESERVED_SHARED_MEM_END -
                                        AC5_INTERNAL_RESERVED_SHARED_MEM_START;
        tmpUsedList->allocBlock.blockSize.l[1] = 0;
        tmpUsedList->allocBlock.allocType =
                                   CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_RESERVE_E;
        tmpUsedList->allocBlock.blockUse =
                                       CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_CODE_E;
        tmpUsedList->allocBlock.scpuNum = RESERVED_SCPU_NUM;
        tmpUsedList->next = NULL;
        devPtr->hwInfo.cnmRam[memType].cnmRamUsedList = tmpUsedList;
    }
    else if( CPSS_HW_INFO_BUS_TYPE_PEX_E !=
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].busType )
    {
        /* Nor external CPU, nor internal CPU */
        prvCpssGenericCnMShMemMgrFreeFreeList(devNum, memType);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Neither external CPU nor internal CPU");
    }

    return GT_OK;
}

/**
* @internal internal_cpssGenericCnMShMemMgrCreate function
* @endinternal
*
* @brief  This function creates a CnM shared memory (the coupled SRAM) manager
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum                - device number
* @param[in] mpuEnable             - use MPU
*                                    In non SIP7 devices, must be set to
*                                    GT_FALSE.
* @param[in] reinitEpb             - controls whether EPB (External Packet
*                                    Buffer) feature will have to be
*                                    reinitialized (with a smaller DRSAM area)
*                                    in case a new client requests a DRAM area
*                                    on runtime and there is no free DRAM segment
*                                    for it.
*                                    Meaningless for non SIP7 devices, must
*                                    be set to GT_FALSE;
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameter value.
* @retval GT_BAD_STATE             - nor internal or external CPU.
* @retval GT_BAD_VALUE             - internal CPU for unsupported device.
* @retval GT_FAIL
*         common/src/cpss/common/CnM/cpssGenericCnMShMem.c  -
*         fail to create manager.
* @retval GT_NO_RESOURCE           - failed to create manager DB
* @retval GT_ALREADY_EXIST         - manager alreday exists.
*/
GT_STATUS internal_cpssGenericCnMShMemMgrCreate
(
   IN GT_U8    devNum,
   IN GT_BOOL  mpuEnable,
   IN GT_BOOL  reinitEpb
)
{
    GT_STATUS rc;

    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT iiMemType, deviceMemType;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        /*  TODO:
         *  Currently non SIP7 devices are not supported.
         *  When they will several memory types will be supported within the
         *  same device and the current setting is here as placeholder\reminder
         */
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }
    else if( GT_TRUE == mpuEnable || GT_TRUE == reinitEpb )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Bad parameters for non SIP7 creation");
    }
    else
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E;
    }

    /* Create manager instance (if not already created) */
    if( GT_TRUE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
                                      "Shared Memory Manager already created");

    for( iiMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E ;
         iiMemType <= deviceMemType ;
         iiMemType++ )
    {
        rc = prvCpssGenericCnMShMemMgrCreateFreeList(devNum, iiMemType);
        if( GT_OK != rc )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc,
                  "Failed to create free list for memory type %d", iiMemType);
        }
    }

    devPtr->hwInfo.cnmRamManagerEnabled = GT_TRUE;

    /*  TODO:
     *  Scan SCPUs to see if existing FWs are running,
     *  retreive their allocations,
     *  remove them from free list and add to the used list.
     */
    return rc;
}

/**
* @internal cpssGenericCnMShMemMgrCreate function
* @endinternal
*
* @brief  This function creates a CnM shared memory (the coupled SRAM) manager
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum                - device number
* @param[in] mpuEnable             - use MPU
*                                    In non SIP7 devices, must be set to
*                                    GT_FALSE.
* @param[in] reinitEpb             - controls whether EPB (External Packet
*                                    Buffer) feature will have to be
*                                    reinitialized (with a smaller DRSAM area)
*                                    in case a new client requests a DRAM area
*                                    on runtime and there is no free DRAM segment
*                                    for it.
*                                    Meaningless for non SIP7 devices, must
*                                    be set to GT_FALSE;
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameter value.
* @retval GT_BAD_STATE             - nor internal or external CPU.
* @retval GT_BAD_VALUE             - internal CPU for unsupported device.
* @retval GT_FAIL                  - fail to create manager.
* @retval GT_NO_RESOURCE           - failed to create manager DB
* @retval GT_ALREADY_EXIST         - manager alreday exists.
*/
GT_STATUS cpssGenericCnMShMemMgrCreate
(
   IN GT_U8    devNum,
   IN GT_BOOL  mpuEnable,
   IN GT_BOOL  reinitEpb
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrCreate);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mpuEnable, reinitEpb));

    rc = internal_cpssGenericCnMShMemMgrCreate(devNum, mpuEnable, reinitEpb);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mpuEnable, reinitEpb));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrDelete function
* @endinternal
*
* @brief  This function deletes Shared Memory Manager and its DB (no impact on FWs)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum                - device number
*
* @retval GT_OK              - on success.
* @retval GT_BAD_PARAM       - on wrong input parameter value.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrDelete
(
   IN GT_U8    devNum
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT iiMemType, deviceMemType;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }
    else
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E;
    }

    /* Create manager instance (if not already created) */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,
                                      "Shared Memory Manager doesn't exist");

    for( iiMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E ;
         iiMemType <= deviceMemType ;
         iiMemType++ )
    {
        prvCpssGenericCnMShMemMgrFreeFreeList(devNum, iiMemType);
        prvCpssGenericCnMShMemMgrFreeUsedList(devNum, iiMemType);
    }
    devPtr->hwInfo.cnmRamManagerEnabled = GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssGenericCnMShMemMgrDelete function
* @endinternal
*
* @brief  This function deletes Shared Memory Manager and its DB (no impact on FWs)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum                - device number
*
* @retval GT_OK              - on success.
* @retval GT_BAD_PARAM       - on wrong input parameter value.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrDelete
(
   IN GT_U8    devNum
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrDelete);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssGenericCnMShMemMgrDelete(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Add a used list entry to the appropriate place in the used list.   */
/* Done after free list ordering and memory allocation for the entry. */
GT_STATUS prvCpssGenericCnMShMemMgrInsertUsedEntry
(
    IN GT_U8    devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr,
    IN PRV_CPSS_CNM_SHMEM_USED_LIST_STC *allocUsedList
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum);

    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList =
                                    devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;


    allocUsedList->allocBlock = *allocBlockPtr;

    if( NULL == tmpUsedList )
    {
        /* Used list is empty */
        devPtr->hwInfo.cnmRam[memType].cnmRamUsedList = allocUsedList;
        allocUsedList->next = NULL;
    }
    else
    {
        if( allocBlockPtr->blockOffset.l[0] < tmpUsedList->allocBlock.blockOffset.l[0] )
        {
            /* Insert as the first entry in the used list*/
            allocUsedList->next = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;
            devPtr->hwInfo.cnmRam[memType].cnmRamUsedList = allocUsedList;
        }
        else
        {
            while( tmpUsedList )
            {
                if( (NULL == tmpUsedList->next) /* Insert as last entry in list */ ||
                    (allocBlockPtr->blockOffset.l[0] < tmpUsedList->next->allocBlock.blockOffset.l[0]) )
                {
                    allocUsedList->next = tmpUsedList->next;
                    tmpUsedList->next = allocUsedList;
                    break;
                }

                tmpUsedList = tmpUsedList->next;
            }
        }
    }

    return GT_OK;
}

/* Hard allocation */
GT_STATUS prvCpssGenericCnMShMemMgrAllocBlockHard
(
    IN GT_U8    devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum);

    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList =
            devPtr->hwInfo.cnmRam[memType].cnmRamFreeList;

    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmp1FreeList, *prevFreeList = NULL;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *allocUsedList;

    if( NULL == tmpFreeList )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Free list is empty");
    }

    if( devPtr->hwInfo.cnmRam[memType].cnmRamSize.l[0] <
                                         allocBlockPtr->blockOffset.l[0] )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"Requested block offset above ram range");
    }

    while( tmpFreeList )
    {
        /* Find the free range the hard allocation start address reside in */
        if( (tmpFreeList->blockOffset.l[0] <= allocBlockPtr->blockOffset.l[0]) &&
            ((tmpFreeList->blockOffset.l[0] + tmpFreeList->blockSize.l[0]) >
                                              allocBlockPtr->blockOffset.l[0]) )
        {
            /* Check if the range is large enough to contain the block size */
            if( (allocBlockPtr->blockOffset.l[0] + allocBlockPtr->blockSize.l[0]) >
                      (tmpFreeList->blockOffset.l[0] + tmpFreeList->blockSize.l[0]) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CREATE_ERROR,"Hard allocate no free block with fitting size");
            }
            else
            {
                allocUsedList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_USED_LIST_STC));
                if( allocUsedList == NULL )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Hard allocate used entry allocation failed");
                }

                if( tmpFreeList->blockSize.l[0] == allocBlockPtr->blockSize.l[0] )
                {
                    /* Case 1: allocated block same as free block */
                    /* Remove the whole free block from the free list*/
                    if( tmpFreeList == devPtr->hwInfo.cnmRam[memType].cnmRamFreeList )
                    {
                        /* Case it is the first block in the list */
                        devPtr->hwInfo.cnmRam[memType].cnmRamFreeList = tmpFreeList->next;
                    }
                    else
                    {
                        /*
                         *  This code will not be reached for the first entry in list
                         *  during the first iteration of the while loop (for the
                         *  first entry, during the first iteration,  we enter the
                         *  "if" case and not the current "else" case), therefore
                         *  there is no risk of using prevFreeList uninitialized as
                         *  it is initialized at the end of the first iteration (done
                         *  at the end of the while loop code).
                         */

                        /* Still, just to be on the safe side */
                        if(!prevFreeList)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                        }

                        prevFreeList->next = tmpFreeList->next;
                    }
                    cpssOsFree(tmpFreeList);
                }
                else if( tmpFreeList->blockOffset.l[0] == allocBlockPtr->blockOffset.l[0] )
                {
                    /* Case 2: allocated block starts at same address as free block */
                    tmpFreeList->blockOffset.l[0] += allocBlockPtr->blockSize.l[0];
                    tmpFreeList->blockSize.l[0] -= allocBlockPtr->blockSize.l[0];
                } else if( tmpFreeList->blockOffset.l[0] + tmpFreeList->blockSize.l[0] ==
                           allocBlockPtr->blockOffset.l[0] + allocBlockPtr->blockSize.l[0])
                {
                    /* Case 3: allocated block ends at same address as free block */
                    tmpFreeList->blockSize.l[0] -= allocBlockPtr->blockSize.l[0];
                }
                else
                {
                    /* Case 4: allocated block at the middle of the free block */

                    tmp1FreeList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
                    if( tmp1FreeList == NULL )
                    {
                        cpssOsFree(allocUsedList);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Hard allocate option 4 additional free entry allocation failed");
                    }

                    tmp1FreeList->next = tmpFreeList->next;
                    tmpFreeList->next = tmp1FreeList;
                    tmp1FreeList->blockOffset.l[0] = allocBlockPtr->blockOffset.l[0] +
                                                      allocBlockPtr->blockSize.l[0];
                    tmp1FreeList->blockSize.l[0] = tmpFreeList->blockSize.l[0];
                    tmpFreeList->blockSize.l[0] = allocBlockPtr->blockOffset.l[0] -
                                                     tmpFreeList->blockOffset.l[0];
                    tmp1FreeList->blockSize.l[0] -= (tmpFreeList->blockSize.l[0] +
                                                     allocBlockPtr->blockSize.l[0]);
                }

                /* Used entry insertion */
                return prvCpssGenericCnMShMemMgrInsertUsedEntry(
                                    devNum, memType, allocBlockPtr, allocUsedList);
            }
        }

        prevFreeList = tmpFreeList;
        tmpFreeList = tmpFreeList->next;
    }

    /* the hard allocation start address is out of range */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CREATE_ERROR,"Hard allocate start address out of range");
}

/* Simple\dynamic allocation - Find the smallest hole at lowest address */
GT_STATUS prvCpssGenericCnMShMemMgrAllocBlockManager
(
    IN GT_U8    devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum);

    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList =
                                    devPtr->hwInfo.cnmRam[memType].cnmRamFreeList;

    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmp1FreeList = NULL;
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *prevFreeList = NULL;
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *prev1FreeList = NULL;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *allocUsedList;

    if( NULL == tmpFreeList )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Free list is empty");
    }

    while( tmpFreeList )
    {
        /* Find the smallest hole, at the lowest address */
        if( tmpFreeList->blockSize.l[0] >= allocBlockPtr->blockSize.l[0] )
        {
            if( NULL == tmp1FreeList ||
                (tmp1FreeList->blockSize.l[0] > tmpFreeList->blockSize.l[0]) )
            {
                tmp1FreeList = tmpFreeList;
                prev1FreeList = prevFreeList;
            }
        }

        prevFreeList = tmpFreeList;
        tmpFreeList = tmpFreeList->next;
    }

    if( NULL == tmp1FreeList )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_CREATE_ERROR,"Manager allocate no free block with fitting size");
    }

    allocUsedList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_USED_LIST_STC));
    if( allocUsedList == NULL )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Manager allocate used entry allocation failed");
    }

    allocBlockPtr->blockOffset.l[0] = tmp1FreeList->blockOffset.l[0];

    if( tmp1FreeList->blockSize.l[0] == allocBlockPtr->blockSize.l[0] )
    {
        /* option 1: allocated block same as free block */
        /* Remove the whole free block from the free list*/
        if( tmp1FreeList == devPtr->hwInfo.cnmRam[memType].cnmRamFreeList )
        {
            /* Case it is the first block in the list */
            devPtr->hwInfo.cnmRam[memType].cnmRamFreeList = tmp1FreeList->next;
        }
        else
        {
            /*
             *  If flow reached here, inner "if" in the above while loop must
             *  have been executed, therefore both prev1FreeList &
             *  tmp1FreeList have been initialized and there is no risk of
             *  using uninitialized pointers.
             */
            prev1FreeList->next = tmp1FreeList->next;
        }
        cpssOsFree(tmp1FreeList);
    }
    else
    {
        /* option 2: allocated block smaller than the free block */
        tmp1FreeList->blockOffset.l[0] += allocBlockPtr->blockSize.l[0];
        tmp1FreeList->blockSize.l[0] -= allocBlockPtr->blockSize.l[0];
    }

    /* Used entry insertion */
    return prvCpssGenericCnMShMemMgrInsertUsedEntry(
                                devNum, memType, allocBlockPtr, allocUsedList);
}

/* Check if the pair SCPU & Block Usage already allocated memory */
GT_STATUS prvCpssGenericCnMShMemMgrCheckScpuNum
(
    IN GT_U8    devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr = PRV_CPSS_PP_MAC(devNum);

    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList =
              devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( tmpUsedList )
    {
        if( (tmpUsedList->allocBlock.scpuNum == allocBlockPtr->scpuNum) &&
            (tmpUsedList->allocBlock.blockUse == allocBlockPtr->blockUse) )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST,
             "SCPU with the specified block usage already got memory allocation");
        }
        tmpUsedList = tmpUsedList->next;
    }

    return GT_OK;
}

/**
* @internal internal_cpssGenericCnMShMemMgrAllocBlock function
* @endinternal
*
* @brief  This function requests allocation of a memory block.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - memory type
* @param[in] allocBlockPtr   - (pointer to) block allocation request parameters.
*
* @param[out] allocBlockPtr  - (pointer to) block allocated settings.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_OUT_OF_RANGE   - requested offset (hard allocation) above ram range.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_ALREADY_EXIST  - scpu already goy allocation.
* @retval GT_NO_RESOURCE    - free list empty or failed to allocate list entry.
* @retval GT_CREATE_ERROR    - no available free block size.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrAllocBlock
(
   IN  GT_U8    devNum,
   IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
   INOUT  CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr
)
{
    GT_STATUS rc;

    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(allocBlockPtr);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }
    else if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E != memType ||
             CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_DATA_E != allocBlockPtr->blockUse ||
             0 != allocBlockPtr->numOfPartners ||
             2 != allocBlockPtr->reqNum ||
             CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_AUTO_E != allocBlockPtr->cacheability ||
             0 == allocBlockPtr->blockSize.l[0] )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Bad parameters for non SIP7 creation");
    }

    /* Verify block size is 4 bytes aligned */
    if( 0x0 != (allocBlockPtr->blockSize.l[0] & 0x3) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Block size must be 4 bytes aligned");
    }

    rc = prvCpssGenericCnMShMemMgrCheckScpuNum(devNum, memType, allocBlockPtr);
    if( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"SCPU number %d already exists",
                                                      allocBlockPtr->scpuNum);
    }

    if( CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_HARD_E == allocBlockPtr->allocType )
    {
        rc = prvCpssGenericCnMShMemMgrAllocBlockHard(devNum, memType, allocBlockPtr);
    }
    else if( CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_MANAGER_E == allocBlockPtr->allocType )
    {
        rc = prvCpssGenericCnMShMemMgrAllocBlockManager(devNum, memType, allocBlockPtr);
    }
    else
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Bad block allocation type %d", allocBlockPtr->allocType);

    return rc;
}

/**
* @internal cpssGenericCnMShMemMgrAllocBlock function
* @endinternal
*
* @brief  This function lists memory blocks allocated for a client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - memory type
* @param[in] allocBlockPtr   - (pointer to) block allocation request parameters.
*
* @param[out] allocBlockPtr  - (pointer to) block allocated settings.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_OUT_OF_RANGE   - requested offset (hard allocation) above ram range.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_ALREADY_EXIST  - scpu already goy allocation.
* @retval GT_NO_RESOURCE    - free list empty or failed to allocate list entry.
* @retval GT_CREATE_ERROR    - no available free block size.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrAllocBlock
(
   IN     GT_U8    devNum,
   IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
   INOUT  CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrAllocBlock);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, allocBlockPtr));

    rc = internal_cpssGenericCnMShMemMgrAllocBlock(devNum, memType, allocBlockPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, allocBlockPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Find blocks allocated for client on specified memory type */
GT_VOID prvGenericCnMShMemMgrClient2BlocksPerMemType
(
    IN    GT_U8   devNum,
    IN    GT_U32  scpuNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN    GT_U32  numOfAllocBlocks,
    INOUT GT_U32  *allocBlocksArrayUsed,
    OUT   CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC *allocBlocksPtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList;
    GT_U32 ii = *allocBlocksArrayUsed;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    tmpUsedList = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( (ii < numOfAllocBlocks) && tmpUsedList )
    {
        if( tmpUsedList->allocBlock.scpuNum == scpuNum )
        {
            allocBlocksPtr[ii].memType      = memType;
            allocBlocksPtr[ii].blockOffset  =
                                     tmpUsedList->allocBlock.blockOffset;
            allocBlocksPtr[ii].blockSize    =
                                     tmpUsedList->allocBlock.blockSize;
            allocBlocksPtr[ii].blockUse     =
                                     tmpUsedList->allocBlock.blockUse;
            allocBlocksPtr[ii].cacheability =
                                     tmpUsedList->allocBlock.cacheability;

            ii++;
        }
        tmpUsedList = tmpUsedList->next;
    }

    *allocBlocksArrayUsed = ii;

    return;
}

/**
* @internal internal_cpssGenericCnMShMemMgrClient2BlocksGetAll
* @endinternal
*
* @brief  This function lists memory blocks allocated for a client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] scpuNum         - the SCPU the blocks were allocated to
* @param[in] numOfAllocBlocksPtr - (pointer to) size of allocBlocksPtr array.
*
* @param[out] numOfAllocBlocksPtr - (pointer to) number of used entries in
*                                  allocBlocksPtr array.
* @param[out] allocBlockPtr  - (pointer to) client blocks data.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrClient2BlocksGetAll
(
    IN    GT_U8   devNum,
    IN    GT_U32  scpuNum,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT   CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC *allocBlocksPtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;

    GT_U32 allocBlocksArrayUsed = 0;
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT iiMemType, deviceMemType;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(allocBlocksPtr);
    CPSS_NULL_PTR_CHECK_MAC(numOfAllocBlocksPtr);
    if( 0 == *numOfAllocBlocksPtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Zero array size");
    }

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }
    else
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E;
    }

    for( iiMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E ;
         iiMemType <= deviceMemType ;
         iiMemType++ )
    {
        prvGenericCnMShMemMgrClient2BlocksPerMemType(devNum,
                                                     scpuNum,
                                                     iiMemType,
                                                     *numOfAllocBlocksPtr,
                                                     &allocBlocksArrayUsed,
                                                     allocBlocksPtr);
    }

    *numOfAllocBlocksPtr = allocBlocksArrayUsed;

    return GT_OK;
}

/**
* @internal cpssGenericCnMShMemMgrClient2BlocksGetAll
* @endinternal
*
* @brief  This function lists memory blocks allocated for a client.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] scpuNum         - the SCPU the blocks were allocated to
* @param[in] numOfAllocBlocksPtr - (pointer to) size of allocBlocksPtr array.
*
* @param[out] numOfAllocBlocksPtr - (pointer to) number of used entries in
*                                  allocBlocksPtr array.
* @param[out] allocBlockPtr  - (pointer to) client blocks data.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrClient2BlocksGetAll
(
    IN    GT_U8   devNum,
    IN    GT_U32  scpuNum,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT   CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC *allocBlocksPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrClient2BlocksGetAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, scpuNum, numOfAllocBlocksPtr, allocBlocksPtr));

    rc = internal_cpssGenericCnMShMemMgrClient2BlocksGetAll(devNum, scpuNum, numOfAllocBlocksPtr, allocBlocksPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, scpuNum, numOfAllocBlocksPtr, allocBlocksPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrBlock2ClientsGetAll
* @endinternal
*
* @brief  To get ALL the SCPUs (dummy or not) that own a Shared Memory block.
*         The block is identified by any of the memory line it covers in the
*         memType memory map.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for a used block at the
*                              specified offset.
* @param[in] memOffset       - offset in memory type to look for used block.
* @param[in] numOfScpusPtr   - (pointer to) scpus array size.
*
* @param[out] numOfScpusPtr  - (pointer to) number of used entries in scpus array.
* @param[out] scpusPtr       - (pointer to) scpus array.
* @param[out] blockOffsetPtr - (pointer to) block offset.
* @param[out] blockSizePtr   - (pointer to) block size.
* @param[out] reqNumPtr      - (pointer to) request number.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_OUT_OF_RANGE   - requested offset above ram range.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_BAD_STATE      - different block parameters for same search.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrBlock2ClientsGetAll
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN    GT_U64  memOffset,
    INOUT GT_U32  *numOfScpusPtr,
    OUT   GT_U32  *scpusPtr,
    OUT   GT_U64  *blockOffsetPtr,
    OUT   GT_U64  *blockSizePtr,
    OUT   GT_U32  *reqNumPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList;
    GT_U32 ii = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(numOfScpusPtr);
    if( 0 == *numOfScpusPtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Zero array size");
    }

    CPSS_NULL_PTR_CHECK_MAC(scpusPtr);
    CPSS_NULL_PTR_CHECK_MAC(blockOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(blockSizePtr);
    CPSS_NULL_PTR_CHECK_MAC(reqNumPtr);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }

    if( devPtr->hwInfo.cnmRam[memType].cnmRamSize.l[0] < memOffset.l[0] )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"Requested block offset above ram range");
    }

    tmpUsedList = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( (ii < *numOfScpusPtr) && tmpUsedList )
    {
        if( (-1 != prvCpssMathCompare64(memOffset,
                                        tmpUsedList->allocBlock.blockOffset)) &&
            (-1 == prvCpssMathCompare64(memOffset,
                       prvCpssMathAdd64(tmpUsedList->allocBlock.blockOffset,
                                        tmpUsedList->allocBlock.blockSize))) )
        {
            scpusPtr[ii]      = tmpUsedList->allocBlock.scpuNum;
            if( 0 == ii )
            {
                *blockOffsetPtr = tmpUsedList->allocBlock.blockOffset;
                *blockSizePtr   = tmpUsedList->allocBlock.blockSize;
                *reqNumPtr      = tmpUsedList->allocBlock.reqNum;
            }
            else if( (0 != prvCpssMathCompare64(*blockOffsetPtr,
                                      tmpUsedList->allocBlock.blockOffset)) ||
                     (0 != prvCpssMathCompare64(*blockSizePtr,
                                      tmpUsedList->allocBlock.blockSize)) ||
                     (*reqNumPtr != tmpUsedList->allocBlock.reqNum) )
            {
                rc = GT_BAD_STATE;
            }

            ii++;
        }
        tmpUsedList = tmpUsedList->next;
    }

    *numOfScpusPtr = ii;

    return rc;
}

/**
* @internal cpssGenericCnMShMemMgrBlock2ClientsGetAll
* @endinternal
*
* @brief  To get ALL the SCPUs (dummy or not) that own a Shared Memory block.
*         The block is identified by any of the memory line it covers in the
*         memType memory map.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for a used block at the
*                              specified offset.
* @param[in] memOffset       - offset in memory type to look for used block.
* @param[in] numOfScpusPtr   - (pointer to) scpus array size.
*
* @param[out] numOfScpusPtr  - (pointer to) number of used entries in scpus array.
* @param[out] scpusPtr       - (pointer to) scpus array.
* @param[out] blockOffsetPtr - (pointer to) block offset.
* @param[out] blockSizePtr   - (pointer to) block size.
* @param[out] reqNumPtr      - (pointer to) request number.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_OUT_OF_RANGE   - requested offset above ram range.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_BAD_STATE      - different block parameters for same search.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrBlock2ClientsGetAll
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN    GT_U64  memOffset,
    INOUT GT_U32  *numOfScpusPtr,
    OUT   GT_U32  *scpusPtr,
    OUT   GT_U64  *blockOffsetPtr,
    OUT   GT_U64  *blockSizePtr,
    OUT   GT_U32  *reqNumPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrBlock2ClientsGetAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, memOffset, numOfScpusPtr,
                            scpusPtr, blockOffsetPtr, blockSizePtr, reqNumPtr));

    rc = internal_cpssGenericCnMShMemMgrBlock2ClientsGetAll(devNum, memType,
       memOffset, numOfScpusPtr, scpusPtr, blockOffsetPtr, blockSizePtr, reqNumPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, memOffset,
            numOfScpusPtr, scpusPtr, blockOffsetPtr, blockSizePtr, reqNumPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/* Add free block */
GT_STATUS prvCpssGenericCnMShMemMgrInsertFreeEntry
(
    IN GT_U8   devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN PRV_CPSS_CNM_SHMEM_USED_LIST_STC *removedUsedList
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList, *prevFreeList = NULL, *allocFreeList;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    tmpFreeList = devPtr->hwInfo.cnmRam[memType].cnmRamFreeList;

    if( NULL == tmpFreeList )
    {
        /* Free list is empty */
        allocFreeList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
        if( allocFreeList == NULL )
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for free list creation");
        }
        allocFreeList->blockOffset = removedUsedList->allocBlock.blockOffset;
        allocFreeList->blockSize = removedUsedList->allocBlock.blockSize;
        allocFreeList->next = NULL;
        devPtr->hwInfo.cnmRam[memType].cnmRamFreeList = allocFreeList;
    }
    else
    {
        if( removedUsedList->allocBlock.blockOffset.l[0] < tmpFreeList->blockOffset.l[0] )
        {
            /* Insert as the first entry in the free list*/
            if( (removedUsedList->allocBlock.blockOffset.l[0] +
                   removedUsedList->allocBlock.blockSize.l[0]) ==
                                            tmpFreeList->blockOffset.l[0])
            {
                /* Case 1: Added free block end is the start of next free block */
                tmpFreeList->blockOffset.l[0] =
                                 removedUsedList->allocBlock.blockOffset.l[0];
                tmpFreeList->blockSize.l[0] +=
                                 removedUsedList->allocBlock.blockSize.l[0];
            }
            else
            {
                /* Case 2: Inserted free block at free list start */
                allocFreeList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
                if( allocFreeList == NULL )
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for free list creation");
                }
                allocFreeList->blockOffset = removedUsedList->allocBlock.blockOffset;
                allocFreeList->blockSize = removedUsedList->allocBlock.blockSize;
                allocFreeList->next = tmpFreeList;
                devPtr->hwInfo.cnmRam[memType].cnmRamFreeList = allocFreeList;
            }
        }
        else
        {
            while( tmpFreeList )
            {
                if( removedUsedList->allocBlock.blockOffset.l[0] < tmpFreeList->blockOffset.l[0] )
                {
                    /*  If we reached here, prevFreeList is initialized because:
                     *
                     *  This code cannot be reached at the first iteration of
                     *  the while loop because if the condition is true, it
                     *  will execute the previous "if" clause.
                     *  At the end of the first iteration prevFreeList is
                     *  initialized
                     *
                     *  Still, just sanity check:
                     */
                    if(!prevFreeList)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
                    }

                    if( ((prevFreeList->blockOffset.l[0] +
                            prevFreeList->blockSize.l[0]) ==
                                 removedUsedList->allocBlock.blockOffset.l[0]) &&
                        ((removedUsedList->allocBlock.blockOffset.l[0] +
                            removedUsedList->allocBlock.blockSize.l[0]) ==
                                                tmpFreeList->blockOffset.l[0]) )
                    {
                        /* Case 1: Added free block fits exactly to the gap */
                        prevFreeList->blockSize.l[0] +=
                            (removedUsedList->allocBlock.blockSize.l[0] +
                                                tmpFreeList->blockSize.l[0]);
                        prevFreeList->next = tmpFreeList->next;
                        cpssOsFree(tmpFreeList);
                    }
                    else if( (prevFreeList->blockOffset.l[0] +
                            prevFreeList->blockSize.l[0]) ==
                                 removedUsedList->allocBlock.blockOffset.l[0] )
                    {
                        /* Case 2: Added free block start is the end of previous free block */
                        prevFreeList->blockSize.l[0] +=
                            removedUsedList->allocBlock.blockSize.l[0];
                    }
                    else if( (removedUsedList->allocBlock.blockOffset.l[0] +
                               removedUsedList->allocBlock.blockSize.l[0]) ==
                                                tmpFreeList->blockOffset.l[0] )
                    {
                        /* Case 3: Added free block end is the start of next free block */
                        tmpFreeList->blockOffset.l[0] =
                                 removedUsedList->allocBlock.blockOffset.l[0];
                        tmpFreeList->blockSize.l[0] +=
                                 removedUsedList->allocBlock.blockSize.l[0];
                    }
                    else
                    {
                        /* Case 4: Insert free block to the list in the gap */
                        allocFreeList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
                        if( allocFreeList == NULL )
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for free list creation");
                        }
                        allocFreeList->blockOffset = removedUsedList->allocBlock.blockOffset;
                        allocFreeList->blockSize = removedUsedList->allocBlock.blockSize;
                        allocFreeList->next = tmpFreeList;
                        prevFreeList->next = allocFreeList;
                    }
                    break;
                }
                else if( NULL == tmpFreeList->next )
                {
                    /* Insert as last entry in list */
                    if ( (tmpFreeList->blockOffset.l[0] +
                            tmpFreeList->blockSize.l[0]) ==
                                 removedUsedList->allocBlock.blockOffset.l[0] )
                    {
                        /* Case 1: Added free block start is the end of previous free block */
                        tmpFreeList->blockSize.l[0] +=
                                   removedUsedList->allocBlock.blockSize.l[0];
                    }
                    else
                    {
                        /* Case 2: Inserted free block at free list end */
                        allocFreeList = cpssOsMalloc(sizeof(PRV_CPSS_CNM_SHMEM_FREE_LIST_STC));
                        if( allocFreeList == NULL )
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"No resource for free list creation");
                        }
                        allocFreeList->blockOffset = removedUsedList->allocBlock.blockOffset;
                        allocFreeList->blockSize = removedUsedList->allocBlock.blockSize;
                        allocFreeList->next = NULL;
                        tmpFreeList->next = allocFreeList;
                    }
                    break;
                }

                prevFreeList = tmpFreeList;
                tmpFreeList = tmpFreeList->next;
            }
        }
    }

    return GT_OK;
}

/* Remove the allocated block from the used list and retrun to the free list */
GT_STATUS prvCpssGenericCnMShMemMgrClientRemoveFromUsedAddToFree
(
    IN GT_U8   devNum,
    IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    IN GT_U32  scpuNum,
    IN    GT_U32  numOfAllocBlocks,
    INOUT GT_U32  *allocBlocksArrayUsed,
    OUT   CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC *allocBlocksPtr
)
{
    GT_STATUS rc;
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList, *prevUsedList, *toFreeUsedList;
    GT_U32 ii = *allocBlocksArrayUsed;

    devPtr = PRV_CPSS_PP_MAC(devNum);

    tmpUsedList = prevUsedList = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( tmpUsedList )
    {
        if( tmpUsedList->allocBlock.scpuNum == scpuNum )
        {
            rc = prvCpssGenericCnMShMemMgrInsertFreeEntry
                       (devNum,memType,tmpUsedList);
            if( GT_OK != rc )
            {
                return rc;
            }

            if ( ii < numOfAllocBlocks )
            {
                allocBlocksPtr[ii].memType      = memType;
                allocBlocksPtr[ii].blockOffset  =
                                         tmpUsedList->allocBlock.blockOffset;
                allocBlocksPtr[ii].blockSize    =
                                         tmpUsedList->allocBlock.blockSize;
                allocBlocksPtr[ii].reqNum       =
                                         tmpUsedList->allocBlock.reqNum;
                allocBlocksPtr[ii].freed        = GT_TRUE;
                ii++;
            }

            /* free the used entry */
            if( tmpUsedList == devPtr->hwInfo.cnmRam[memType].cnmRamUsedList )
            {
                /* the first entry*/
                devPtr->hwInfo.cnmRam[memType].cnmRamUsedList = tmpUsedList->next;
            }
            else
            {
                prevUsedList->next = tmpUsedList->next;
            }

            toFreeUsedList = tmpUsedList;
            tmpUsedList = tmpUsedList->next;
            cpssOsFree(toFreeUsedList);
        }
        else
        {
            prevUsedList = tmpUsedList;
            tmpUsedList = tmpUsedList->next;
        }
    }

    *allocBlocksArrayUsed = ii;

    return GT_OK;
}

/**
* @internal internal_cpssGenericCnMShMemMgrClientRemove
* @endinternal
*
* @brief  remove a SCPU (dummy or not) from the list of ALL its allocated
*         Shared Memory blocks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] scpuNum         - Service CPU number.
* @param[out] numOfAllocBlocksPtr  - (pointer to) length of alloc block array.

* @param[out] numOfAllocBlocksPtr  - (pointer to) number of used entries in alloc block array.
* @param[out] allocBlockPtr - (pointer to) lists all the Shared Memory blocks
*                              from where this SCPU was removed.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - failed to allocate free list entry.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrClientRemove
(
    IN GT_U8   devNum,
    IN GT_U32  scpuNum,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC *allocBlockPtr
)
{
    GT_STATUS rc = GT_OK;
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;/* pointer to common device info */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT iiMemType, deviceMemType;
    GT_U32 allocBlocksArrayUsed = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(numOfAllocBlocksPtr);
    CPSS_NULL_PTR_CHECK_MAC(allocBlockPtr);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }
    else
    {
        deviceMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E;
    }

    /* Create manager instance (if not already created) */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    for( iiMemType = CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E ;
         iiMemType <= deviceMemType ;
         iiMemType++ )
    {
        /* go over the Used list find allocated blocks and return them to the free list */
        rc = prvCpssGenericCnMShMemMgrClientRemoveFromUsedAddToFree(devNum,
                                                               iiMemType,
                                                               scpuNum,
                                                               *numOfAllocBlocksPtr,
                                                               &allocBlocksArrayUsed,
                                                               allocBlockPtr);
        if( GT_OK != rc )
        {
            break;
        }
    }

    *numOfAllocBlocksPtr = allocBlocksArrayUsed;

    return rc;
}

/**
* @internal cpssGenericCnMShMemMgrClientRemove
* @endinternal
*
* @brief  remove a SCPU (dummy or not) from the list of ALL its allocated
*         Shared Memory blocks
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] scpuNum         - Service CPU number.
* @param[out] numOfAllocBlocksPtr  - (pointer to) length of alloc block array.

* @param[out] numOfAllocBlocksPtr  - (pointer to) number of used entries in alloc block array.
* @param[out] allocBlockPtr - (pointer to) lists all the Shared Memory blocks
*                              from where this SCPU was removed.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - failed to allocate free list entry.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrClientRemove
(
    IN GT_U8   devNum,
    IN GT_U32  scpuNum,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC *allocBlockPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrClientRemove);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, scpuNum,
                            numOfAllocBlocksPtr, allocBlockPtr));

    rc = internal_cpssGenericCnMShMemMgrClientRemove(devNum, scpuNum,
                                           numOfAllocBlocksPtr, allocBlockPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, scpuNum,
                                      numOfAllocBlocksPtr, allocBlockPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrFreeMemTypeAll
* @endinternal
*
* @brief  Release ALL allocated Shared Memory blocks of a specific type to ALL
*         clients.
*         Using this API without removing/reloading ALL the FWs is NOT recommended.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to free.
* @param[in] numOfAllocBlocksPtr   - (pointer to) allocated blocks array length.
*
* @param[out] numOfAllocBlocksPtr  - (pointer to) number of used entries in array.
* @param[out] allocBlockArray      - (pointer to) allocated blocks array.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - array too small to contain all blocks data.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrFreeMemTypeAll
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT   GT_U64  allocBlockArray[][2]
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(numOfAllocBlocksPtr);
    if( 0 == *numOfAllocBlocksPtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Zero array size");
    }

    CPSS_NULL_PTR_CHECK_MAC(allocBlockArray);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }

    /* Get used list block members */
    rc = cpssGenericCnMShMemMgrMemTypeAllocMapGet(devNum,memType,numOfAllocBlocksPtr,allocBlockArray);
    if( GT_OK != rc )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,"Failed to get all list blocks");
    }

    /* Free the used list */
    prvCpssGenericCnMShMemMgrFreeUsedList(devNum, memType);

    /* Free the free list */
    prvCpssGenericCnMShMemMgrFreeFreeList(devNum, memType);

    /* Create the free list */
    rc = prvCpssGenericCnMShMemMgrCreateFreeList(devNum, memType);

    return rc;
}

/**
* @internal cpssGenericCnMShMemMgrFreeMemTypeAll
* @endinternal
*
* @brief  Release ALL allocated Shared Memory blocks of a specific type to ALL
*         clients.
*         Using this API without removing/reloading ALL the FWs is NOT recommended.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to free.
* @param[in] numOfAllocBlocksPtr   - (pointer to) allocated blocks array length.
*
* @param[out] numOfAllocBlocksPtr  - (pointer to) number of used entries in array.
* @param[out] allocBlockArray      - (pointer to) allocated blocks array.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - array too small to contain all blocks data.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrFreeMemTypeAll
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT   GT_U64  allocBlockArray[][2]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrFreeMemTypeAll);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, numOfAllocBlocksPtr,
                                                             allocBlockArray));

    rc = internal_cpssGenericCnMShMemMgrFreeMemTypeAll(
                       devNum, memType, numOfAllocBlocksPtr, allocBlockArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, numOfAllocBlocksPtr,
                                                             allocBlockArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrMemTypeAllocMapGet
* @endinternal
*
* @brief  get the map of ALL allocated Shared Memory blocks of a specific type
*         (to all clients).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the used blocks map.
* @param[in] numOfAllocBlocksPtr   - (pointer to) allocated blocks array length.
*
* @param[out] numOfAllocBlocksPtr  - (pointer to) number of used entries in array.
* @param[out] allocBlockArray      - (pointer to) allocated blocks array.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - array too small to contain all blocks data.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrMemTypeAllocMapGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT   GT_U64  allocBlockArray[][2]
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList;
    GT_U32 ii = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(numOfAllocBlocksPtr);
    if( 0 == *numOfAllocBlocksPtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Zero array size");
    }

    CPSS_NULL_PTR_CHECK_MAC(allocBlockArray);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }

    tmpUsedList = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( (ii < *numOfAllocBlocksPtr) && tmpUsedList )
    {
        if( CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_RESERVE_E !=
                                        tmpUsedList->allocBlock.allocType )
        {
            allocBlockArray[ii][0] = tmpUsedList->allocBlock.blockOffset;
            allocBlockArray[ii][1] = tmpUsedList->allocBlock.blockSize;

            ii++;
        }
        tmpUsedList = tmpUsedList->next;
    }

    if( ii == *numOfAllocBlocksPtr && tmpUsedList )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Array too small for all data");
    }

    *numOfAllocBlocksPtr = ii;

    return GT_OK;
}

/**
* @internal cpssGenericCnMShMemMgrMemTypeAllocMapGet
* @endinternal
*
* @brief  get the map of ALL allocated Shared Memory blocks of a specific type
*         (to all clients).
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the used blocks map.
* @param[in] numOfAllocBlocksPtr   - (pointer to) allocated blocks array length.
*
* @param[out] numOfAllocBlocksPtr  - (pointer to) number of used entries in array.
* @param[out] allocBlockArray      - (pointer to) allocated blocks array.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - array too small to contain all blocks data.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrMemTypeAllocMapGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    INOUT GT_U32  *numOfAllocBlocksPtr,
    OUT   GT_U64  allocBlockArray[][2]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrMemTypeAllocMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, numOfAllocBlocksPtr,
                                                             allocBlockArray));

    rc = internal_cpssGenericCnMShMemMgrMemTypeAllocMapGet(
                       devNum, memType, numOfAllocBlocksPtr, allocBlockArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, numOfAllocBlocksPtr,
                                                             allocBlockArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrMemTypeAllocSizeGet
* @endinternal
*
* @brief  get the overall size of all Shared Memory blocks of a specific type
*         that are currently owned by client
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the size of used blocks.
*
* @param[out] allocBlockSizePtr    - (pointer to) allocated blocks size.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrMemTypeAllocSizeGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    OUT   GT_U64  *allocBlockSizePtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_USED_LIST_STC *tmpUsedList;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(allocBlockSizePtr);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }

    allocBlockSizePtr->l[0] = 0;
    allocBlockSizePtr->l[1] = 0;

    tmpUsedList = devPtr->hwInfo.cnmRam[memType].cnmRamUsedList;

    while( tmpUsedList )
    {
        if( CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_RESERVE_E !=
                                        tmpUsedList->allocBlock.allocType )
        {
            *allocBlockSizePtr = prvCpssMathAdd64(*allocBlockSizePtr,
                                              tmpUsedList->allocBlock.blockSize);
        }
        tmpUsedList = tmpUsedList->next;
    }

    return GT_OK;
}

/**
* @internal cpssGenericCnMShMemMgrMemTypeAllocSizeGet
* @endinternal
*
* @brief  get the overall size of all Shared Memory blocks of a specific type
*         that are currently owned by client
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the size of used blocks.
*
* @param[out] allocBlockSizePtr    - (pointer to) allocated blocks size.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrMemTypeAllocSizeGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    OUT   GT_U64  *allocBlockSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrMemTypeAllocSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, allocBlockSizePtr));

    rc = internal_cpssGenericCnMShMemMgrMemTypeAllocSizeGet(
                       devNum, memType, allocBlockSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, allocBlockSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrMemTypeFreeMapGet
* @endinternal
*
* @brief  get the map of ALL free Shared Memory blocks of a specific type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the free blocks map.
* @param[in] numOfFreeBlocksPtr   - (pointer to) free blocks array length.
*
* @param[out] numOfFreeBlocksPtr  - (pointer to) number of used entries in array.
* @param[out] freeBlockArray      - (pointer to) free blocks array.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - array too small to contain all blocks data.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrMemTypeFreeMapGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    INOUT GT_U32  *numOfFreeBlocksPtr,
    OUT   GT_U64  freeBlockArray[][2]
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList;
    GT_U32 ii = 0;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(numOfFreeBlocksPtr);
    if( 0 == *numOfFreeBlocksPtr )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Zero array size");
    }

    CPSS_NULL_PTR_CHECK_MAC(freeBlockArray);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }

    tmpFreeList = devPtr->hwInfo.cnmRam[memType].cnmRamFreeList;

    while( (ii < *numOfFreeBlocksPtr) && tmpFreeList )
    {
        freeBlockArray[ii][0] = tmpFreeList->blockOffset;
        freeBlockArray[ii][1] = tmpFreeList->blockSize;

        ii++;
        tmpFreeList = tmpFreeList->next;
    }

    if( ii == *numOfFreeBlocksPtr && tmpFreeList )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE,"Array too small for all data");
    }

    *numOfFreeBlocksPtr = ii;

    return GT_OK;
}

/**
* @internal cpssGenericCnMShMemMgrMemTypeFreeMapGet
* @endinternal
*
* @brief  get the map of ALL free Shared Memory blocks of a specific type.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the free blocks map.
* @param[in] numOfFreeBlocksPtr   - (pointer to) free blocks array length.
*
* @param[out] numOfFreeBlocksPtr  - (pointer to) number of used entries in array.
* @param[out] freeBlockArray      - (pointer to) free blocks array.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NO_RESOURCE    - array too small to contain all blocks data.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrMemTypeFreeMapGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    INOUT GT_U32  *numOfFreeBlocksPtr,
    OUT   GT_U64  freeBlockArray[][2]
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrMemTypeFreeMapGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, numOfFreeBlocksPtr,
                                                             freeBlockArray));

    rc = internal_cpssGenericCnMShMemMgrMemTypeFreeMapGet(
                       devNum, memType, numOfFreeBlocksPtr, freeBlockArray);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, numOfFreeBlocksPtr,
                                                             freeBlockArray));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssGenericCnMShMemMgrMemTypeFreeSizeGet
* @endinternal
*
* @brief  get the overall size of all the free Shared Memory blocks of a specific type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the size of free blocks.
*
* @param[out] freeBlockSizePtr    - (pointer to) free blocks size.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS internal_cpssGenericCnMShMemMgrMemTypeFreeSizeGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    OUT   GT_U64  *freeBlockSizePtr
)
{
    PRV_CPSS_GEN_PP_CONFIG_STC *devPtr;
    PRV_CPSS_CNM_SHMEM_FREE_LIST_STC *tmpFreeList;

    PRV_CPSS_DEV_CHECK_MAC(devNum);

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_FALCON_E | CPSS_AC5_E | CPSS_AC5P_E
                                      | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(freeBlockSizePtr);

    devPtr = PRV_CPSS_PP_MAC(devNum);

    /* Verify that memory manager was created */
    if( GT_FALSE == devPtr->hwInfo.cnmRamManagerEnabled )
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED,"Manager not created");

    if( CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E < memType )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Wrong memory type");
    }

    if ( devPtr->appDevFamily & CPSS_AAS_E )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED,"AAS not yet supported");
    }

    freeBlockSizePtr->l[0] = 0;
    freeBlockSizePtr->l[1] = 0;

    tmpFreeList = devPtr->hwInfo.cnmRam[memType].cnmRamFreeList;

    while( tmpFreeList )
    {
        *freeBlockSizePtr = prvCpssMathAdd64(*freeBlockSizePtr,
                                              tmpFreeList->blockSize);
        tmpFreeList = tmpFreeList->next;
    }

    return GT_OK;
}

/**
* @internal cpssGenericCnMShMemMgrMemTypeFreeSizeGet
* @endinternal
*
* @brief  get the overall size of all the free Shared Memory blocks of a specific type
*
* @note   APPLICABLE DEVICES:      Falcon; AC5; AC5P; AC5X; Harrier; Ironman; AAS;
*
* @param[in] devNum          - device number
* @param[in] memType         - the memory type to look for the size of free blocks.
*
* @param[out] freeBlockSizePtr    - (pointer to) free blocks size.
*
* @retval GT_OK             - on success.
* @retval GT_BAD_PARAM      - on wrong input parameter value.
* @retval GT_BAD_PTR        - one of the parameters is NULL pointer.
* @retval GT_NOT_INITIALIZED - manager not created
*/
GT_STATUS cpssGenericCnMShMemMgrMemTypeFreeSizeGet
(
    IN    GT_U8   devNum,
    IN    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
    OUT   GT_U64  *freeBlockSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssGenericCnMShMemMgrMemTypeFreeSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, memType, freeBlockSizePtr));

    rc = internal_cpssGenericCnMShMemMgrMemTypeFreeSizeGet(
                       devNum, memType, freeBlockSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, memType, freeBlockSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

