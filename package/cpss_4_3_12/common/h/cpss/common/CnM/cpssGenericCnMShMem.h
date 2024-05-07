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
* @file cpssGenericCnMShMem.h
*
* @brief Includes generic Shared Memory definitions.
*
*
* @version   1
********************************************************************************
*/

#ifndef __cpssGenericCnMShMemh
#define __cpssGenericCnMShMemh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT
*
*  @brief Memory type and place to use for allocation.
*
*  @note In non SIP7 devices, this parameter must be set
*  @note to CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E.
*/
typedef enum{

    /* SRAM on the same tile the SCPU reside */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE0_E = 0,

    /* SRAM on the other tile the SCPU reside */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_SRAM_TILE1_E = 1,

    /* DRAM on the same tile the SCPU reside */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE0_E = 2,

    /* DRAM on the other tile the SCPU reside */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_DRAM_TILE1_E = 3

} CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT;

/**
* @enum CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT
*
*  @brief block usage for code, data, intensive data or GDMA
*
*  @note This parameter is used by the SharedMem Manager, in conjunction with
*  @note numOfPartners and the memType, to select the proper memory attributes
*  @note and region index.
*  @note In non SIP7 devices, this parameter must be set to
*  @note CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_DATA_E.
*/
typedef enum{

    /* Memory used for FW code segment, e.g. the FW binary file loaded. */
    CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_CODE_E = 0,

    /* Memory for DB the Fw\application is using */
    CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_DATA_E = 1,

    /* In case more than one client access this memory for its DB, but the */
    /* one allocating will use it more intensively - this will influence the */
    /* cacheability requirements. */
    CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_INTENSIVE_DATA_E = 2,

} CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT;

/**
* @enum CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT
*
*  @brief specific cacheability type
*
*  @note In non SIP7 devices, this parameter must be set to
*  @note CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_AUTO_E.
*/
typedef enum{

    /* Cache disable */
    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_NONE_E = 0,

    /* Cache Policy Write Through */
    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_WT_E = 1,

    /* Cache Policy Write Back, Write Allocate */
    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_WB_WA_E = 2,

    /* Cache Policy Write Back, No Write Allocate */
    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_WB_nWA_E = 3,

    /* Cache Policy is not defined explicitly, but depends on other */
    /* allocation request parameters. */
    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_AUTO_E = 4

} CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT;

/**
* @enum CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT
*
*  @brief allocation method used.
*
*/
typedef enum{

    /* Dynamic allocation by the manager*/
    CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_MANAGER_E = 0,

    /* static allocation due to hard allocation request */
    CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_HARD_E = 1,

    /* range reserved and not available */
    CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_RESERVE_E = 2

} CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT;

/**
* @struct CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
 *
*  @brief parameters for the requested block allocation.
*
*  @note  Input values - the request.
*  @note  Output value - the allocation.
*
*/
typedef struct CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STCT {


    /*
     * The same callerId may call this API multiple times
     * If the memory block is used by a FW, the callerId must be
     * [ FW ID, Device ID ]
     * If the memory block is used only by a host app (no FW involved),
     * then a dummy FW ID (in a dedicated range) shall be used here,
     * while using the proper Device ID
     * FW ID MS-bit: 1 - customer client, 0 - MRVL client
     * FW ID (MS-bit - 1): 1 - host app client, 0 - true FW client
     * Multiple instantiations of the same host app or of the same FW will use
     * the same callerId. From the time a client has requested a Shared Memory
     * block, it will be uniquely identified by the scpuNum returned.
     */
    GT_U32 callerId;

    CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT blockUse;

    /*
     * Length of the partnerArray
     * In non SIP7 devices, this parameter must be set to 0.
     */
    GT_U32 numOfPartners;

    /*
     * Array of partner IDs where partner ID is:
     * - [ FW ID, Device ID ] of a partner with which the Shared Memory block
     *   is shared.
     * - It may use a dummy FW ID (in a dedicated range) that represents a host
     *   app partner.
     * - The array may be empty if the requested block is for the exclusive use
     *   of the caller.
     * - A partnerId may be the same as the callerId if the block is shared by
     *   the same two (or more) FWs that run on multiple CM7 cores.
     * - The same partnerId may appear more than once in the array if it
     *   represents a FW that runs on multiple CM7 cores.
     * In non SIP7 devices, this parameter must be set to the empty array,
     * i.e. it is ignored.
     */
    GT_U32 *partnerArray;

    /*
     * This request identifier is hardcoded in the client code, and it is used
     * by the client to uniquely identify the target use for this block.
     * E.g., one reqNum for an IPC block between the FW and the host, and
     * another reqNum for an IPC block between the FW and another FW.
     * ReqNum = 0 is reserved for the shared SRAM block used for the FW image.
     * ReqNum = 1 is reserved for the shared SRAM block used for IPC with the host.
     * The same ReqNum shall be invoked by all the clients that share this block.
     * SharedMem Manager uses the reqNum in conjunction with the list of clients
     * (a.k.a. the partner array - the previous parameter, which is the list)
     * concerned to uniquely identify a request.
     * In non SIP7 devices, only reqNum = 2 is supported, for allocating
     * a single block per client (since the list\partner array in this case is
     * empty).
     */
    GT_U32 reqNum;

    /*
     * As input parameter - the requested block size.
     * As output parameter - the size of the allocated block.
     * It may be larger than the requested blockSize because of
     * MPU region/sub-region considerations. For a FW client, it is the sum of
     * the sizes of all enabled sub-regions.
     * Its granularity is derived from the memory size by taking into account
     * only the 32 LSbits. E.g. if memory size is 36 bits range, than
     * granularity is on 4 bits boundaries, i.e. 16 bytes.
     * For memory size equal or less than 32 bits, 4 bytes granularity
     * is maintained.
    */
    GT_U64 blockSize;

    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT cacheability;

    /* As input parameter -
     * Optional parameter used to request a specific SCPU number (dummy or not).
     * This can be useful for selecting the tile to which the SCPU is mapped.
     * If this parameter is not relevant for the client, it must be set to all 1s.
     * In non SIP7 devices, this parameter is mandatory and must be set
     * to the SCPU related to the request or to a dummy index that represents
     * a host app.
     * As output parameter -
     * For a callerId that represents a FW, this is the SCPU number allocated to it.
     * For a callerId that represents a host app, it will return a dummy SCPU
     * number, outside the range of SCPU indexes in the device.
     * In non SIP7 devices, the same input value is returned.
     */
    GT_U32 scpuNum;

    /*
     * CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_HARD_E - it notifies the SharedMem
     * Manager that the size, offset and cacheability attribute (if not set to
     * auto) for this block are hardcoded and shall be registered 'as is' from
     * the input parameters provided to the cpssGenericCnMShMemMgrAllocBlock.
     * Since blocks are allocated from the lowest to the highest addresses in a
     * Shared Memory, it is recommended to map such hardcoded blocks to the high
     * address range. Like in the automatic allocation scheme, if part of the
     * block area requested is already allocated to another block,
     * GT_CREATE_ERROR error status is returned.
     * blockSize and blockAddress parameter values that are submitted for
     * such hardcoded blocks shall be aligned to an MPU sub-region's boundaries.
     * Otherwise, BAD_PARAM error status is returned.
     */
    CPSS_GENERIC_CNM_SHMEM_ALLOC_TYPE_ENT allocType;

    /*
     * The memory offset allocated.
     * If hardAlloc == GT_TRUE, it is an input parameter of the memory offset
     * for the requested hardcoded block.
     * Its granularity is derived from the memory size by taking into account
     * only the 32 LSbits. E.g. if memory size is 36 bits range, than
     * granularity is on 4 bits boundaries, i.e. 16 bytes.
     */
    GT_U64 blockOffset;

} CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC;

/**
* @struct CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC
 *
*  @brief parameters of blocks allocated for a client.
*
*/

typedef struct CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STCT {

    /*
     * See description for field blockOffset in
     * CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
     */
    GT_U64 blockOffset;

    /*
     * See description for field blockSize in
     * CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
     */
    GT_U64 blockSize;

    /* The memory used for allocation */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType;

    /*
     * See description for field blockUse in
     * CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
     */
    CPSS_GENERIC_CNM_SHMEM_BLOCK_USE_ENT blockUse;

    CPSS_GENERIC_CNM_SHMEM_CACHEABILITY_ENT cacheability;

} CPSS_GENERIC_CNM_SHMEM_MEM_CLIENT_BLOCK_STC;

/**
* @struct CPSS_GENERIC_CNM_SHMEM_MEM_REMOVED_BLOCK_STC
 *
*  @brief parameters of ablock allocated that was removed.
*
*/
typedef struct CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STCT {

    /*
     * See description for field blockOffset in
     * CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
     */
    GT_U64 blockOffset;

    /*
     * See description for field blockSize in
     * CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
     */
    GT_U64 blockSize;

    /* The memory used for allocation */
    CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType;

    /*
     * See description for field blockUse in
     * CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC
     */
    GT_U32 reqNum;

    /* Currently always set as GT_TRUE since the memory block has been returned
     * to the free shared memory area because the removed SCPU was the last and
     * only owner of this block */
    GT_BOOL freed;

} CPSS_GENERIC_CNM_SHMEM_MEM_REMOVE_BLOCK_STC;

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
*                                    reinitialized (with a smaller DRAM area)
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
);

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
);

/**
* @internal cpssGenericCnMShMemMgrAllocBlock function
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
GT_STATUS cpssGenericCnMShMemMgrAllocBlock
(
   IN     GT_U8    devNum,
   IN CPSS_GENERIC_CNM_SHMEM_MEM_TYPE_ENT memType,
   INOUT  CPSS_GENERIC_CNM_SHMEM_MEM_ALLOC_BLOCK_STC    *allocBlockPtr
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssGenericCnMShMemh */

