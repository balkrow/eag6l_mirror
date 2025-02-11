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
* @file cpssOsMem.h
*
* @brief Operating System CB definitions. Memory manipulation facility.
*
*
* @version   6
********************************************************************************
*/

#ifndef __cpssOsMemh
#define __cpssOsMemh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/************* Defines ********************************************************/
/******************************************************************************/
/* define 'osMalloc,osRealloc,osFree debug mode' to allow find memory leakage */
/* OS_MALLOC_MEMORY_LEAKAGE_DBG                                               */
/* logic:
    1. every allocation done by osMalloc,osRealloc will be registered with the
    file name + line number of the allocating source code.
    2. every free done by osFree will remove the registration of this info
    3. use osMallocMemoryLeakageDbgAction(start) to start the registration of the info
        every start clears the info about previous allocations
    4. use osMallocMemoryLeakageDbgAction(stop) to stop the registration of the info
    5. use osMallocMemoryLeakageDbgPrintStandingAllocated() to print the info about the
       allocated pointers that were not free in the time between 'start' and 'Stop'

    *********************************
    must be the same as in
    in file <../cpssEnabler/mainOs/h/gtOs/gtOsMem.h>
*/
/******************************************************************************/
/*#define OS_MALLOC_MEMORY_LEAKAGE_DBG*/

#if !defined(CHX_FAMILY) && !defined(PX_FAMILY)
    /* this mode supported only by the DXCH and PX */
    #undef OS_MALLOC_MEMORY_LEAKAGE_DBG
#endif /*EXMXPM_FAMILY*/

/* currently allow the leakage detection only in simulation */
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    #ifndef ASIC_SIMULATION
        #undef OS_MALLOC_MEMORY_LEAKAGE_DBG
    #endif /*!ASIC_SIMULATION*/
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

/*******************************************************************************
* CPSS_OS_BZERO_FUNC
*
* DESCRIPTION:
*       Fills the first n-bytes characters of the specified buffer with 0
*
* INPUTS:
*       start  - start address of memory block to be zeroed
*       nbytes - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID (*CPSS_OS_BZERO_FUNC)
(
    IN GT_CHAR *start,
    IN GT_U32  nbytes
);

/*******************************************************************************
* CPSS_OS_MEM_SET_FUNC
*
* DESCRIPTION:
*       Stores 'symbol' converted to an unsigned char in each of the elements
*       of the array of unsigned char beginning at 'start', with size 'size'.
*
* INPUTS:
*       start  - start address of memory block for setting
*       symbol - character to store, converted to an unsigned char
*       size   - size of block to be set
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to set memory block
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID * (*CPSS_OS_MEM_SET_FUNC)
(
    IN GT_VOID * start,
    IN int       symbol,
    IN GT_U32    size
);

/*******************************************************************************
* CPSS_OS_MEM_CPY_FUNC
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. If copying takes place between
*       objects that overlap, the behavior is undefined.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID * (*CPSS_OS_MEM_CPY_FUNC)
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32          size
);

/*******************************************************************************
* CPSS_OS_MEM_MOVE_FUNC
*
* DESCRIPTION:
*       Copies 'size' characters from the object pointed to by 'source' into
*       the object pointed to by 'destination'. The memory areas may overlap:
*       copying takes place as though the bytes in src are first copied into
*       a temporary array that does  not  overlap src or dest, and the bytes
*       are then copied from the temporary array to dest.
*
* INPUTS:
*       destination - destination of copy
*       source      - source of copy
*       size        - size of memory to copy
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Pointer to destination
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID * (*CPSS_OS_MEM_MOVE_FUNC)
(
    IN GT_VOID *       destination,
    IN const GT_VOID * source,
    IN GT_U32          size
);


/*******************************************************************************
* CPSS_OS_MEM_CMP_FUNC
*
* DESCRIPTION:
*       Compare 'size' characters from the object pointed to by 'str1' to
*       the object pointed to by 'str2'.
*
* INPUTS:
*       str1 - first memory area
*       str2 - second memory area
*       size - size of memory to compare
*
* OUTPUTS:
*       None
*
* RETURNS:
*       > 0  - if str1 is alphabetic bigger than str2
*       == 0 - if str1 is equal to str2
*       < 0  - if str1 is alphabetic smaller than str2
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_32 (*CPSS_OS_MEM_CMP_FUNC)
(
    IN const GT_VOID  *str1,
    IN const GT_VOID  *str2,
    IN GT_U32       size
);



/*******************************************************************************
* CPSS_OS_STATIC_MALLOC_FUNC
*
* DESCRIPTION:
*       Allocates memory block of specified size, this memory will not be free.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is only on FIRST initialization.
*
*******************************************************************************/
typedef GT_VOID * (*CPSS_OS_STATIC_MALLOC_FUNC)
(
    IN GT_U32 size
);

/*******************************************************************************
* CPSS_OS_MALLOC_FUNC
*
* DESCRIPTION:
*       Allocates memory block of specified size.
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
typedef GT_VOID * (*CPSS_OS_MALLOC_FUNC)
(
    IN GT_U32 size
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    ,IN const char*    fileNamePtr
    ,IN GT_U32   line
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/
);

/*******************************************************************************
* CPSS_OS_REALLOC_FUNC
*
* DESCRIPTION:
*       Reallocate memory block of specified size.
*
* INPUTS:
*       ptr  - pointer to previously allocated buffer
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Void pointer to the allocated space, or NULL if there is
*       insufficient memory available
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*       Recommended to avoid usage of this function.
*
*******************************************************************************/
typedef GT_VOID * (*CPSS_OS_REALLOC_FUNC)
(
    IN GT_VOID * ptr ,
    IN GT_U32    size
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    ,IN const char*    fileNamePtr
    ,IN GT_U32   line
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/
);

/*******************************************************************************
* CPSS_OS_FREE_FUNC
*
* DESCRIPTION:
*       De-allocates or frees a specified memory block.
*
* INPUTS:
*       memblock - previously allocated memory block to be freed
*
* OUTPUTS:
*       None
*
* RETURNS:
*       None
*
* COMMENTS:
*       Usage of this function is NEVER during initialization.
*
*******************************************************************************/
typedef GT_VOID (*CPSS_OS_FREE_FUNC)
(
    IN GT_VOID* const memblock
#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
    ,IN const char*    fileNamePtr
    ,IN GT_U32   line
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/
);

/*******************************************************************************
* CPSS_OS_CACHE_DMA_MALLOC_FUNC
*
* DESCRIPTION:
*       Allocate a cache-safe buffer  of specified size for DMA devices
*       and drivers
*
* INPUTS:
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID *(*CPSS_OS_CACHE_DMA_MALLOC_FUNC)
(
    IN GT_U32 size
);

/*******************************************************************************
* CPSS_OS_CACHE_DMA_MALLOC_BY_DEVICE_FUNC
*
* DESCRIPTION:
*       Allocate a cache-safe buffer  of specified size for DMA devices
*       and drivers by device number
*
* INPUTS:
*       devNum - device Number
*       size - bytes to allocate
*
* OUTPUTS:
*       None
*
* RETURNS:
*       A pointer to the cache-safe buffer, or NULL
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_VOID *(*CPSS_OS_CACHE_DMA_MALLOC_BY_DEVICE_FUNC)
(
    IN GT_U8  devNum,
    IN GT_U32 size
);

/*******************************************************************************
* CPSS_OS_CACHE_DMA_FREE_FUNC
*
* DESCRIPTION:
*       Free the buffer acquired with osCacheDmaMalloc()
*
* INPUTS:
*       ptr - pointer to malloc/free buffer
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_CACHE_DMA_FREE_FUNC)
(
    IN GT_VOID *ptr
);

/* allow to state CPSS_OS_PHY_TO_VIRT_FUNC that the devNum is 'unknown'
    and the function should find first device that hold this physical
    (for compilation of CONFIG_V2_DMA)
*/
#define CPSS_OS_DEV_NUM_UNKNOWN_CNS 0xFFFFFFFF
/*******************************************************************************
* CPSS_OS_PHY_TO_VIRT_FUNC
*
* DESCRIPTION:
*       Converts physical address to virtual.
*
* INPUTS:
*       devNum   - (for compilation of CONFIG_V2_DMA) the device that the DMA relate to.
*                   this needed as the SMMU/IOMMU can give same phyAddr for more than single device.
*                 NOTE: value of CPSS_OS_DEV_NUM_UNKNOWN_CNS (0xFFFFFFFF) ,
*                   to state that the function should find first device that hold this physical.
*       phyAddr  - physical address
*
* OUTPUTS:
*       virtAddr - virtual address
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_PHY_TO_VIRT_FUNC)
(
#ifdef CONFIG_V2_DMA
    IN  GT_U32     devNum,
#endif /*CONFIG_V2_DMA*/
    IN  GT_UINTPTR phyAddr,
    OUT GT_UINTPTR *virtAddr
);

/*******************************************************************************
* CPSS_OS_VIRT_TO_PHY_FUNC
*
* DESCRIPTION:
*       Converts virtual address to physical.
*
* INPUTS:
*       virtAddr - virtual address
*
* OUTPUTS:
*       phyAddr  - physical address
*
* RETURNS:
*       GT_OK   - on success
*       GT_FAIL - on error
*
* COMMENTS:
*
*******************************************************************************/
typedef GT_STATUS (*CPSS_OS_VIRT_TO_PHY_FUNC)
(
    IN  GT_UINTPTR virtAddr,
    OUT GT_UINTPTR *phyAddr
);

/**
* @internal CPSS_OS_MEM_GLOBAL_DB_SHMEM_INIT  function
* @endinternal
*
* @brief  Initialize process interface to shared lib:
*         Create a shared memory file, if one does not yet exist.
*         This shared memory file will be used for sharing global variables between processes.
*         Map shared memory to process address space
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] size - bytes to allocate
* @param[in] name -  shared memory  name
* @param[out] initDataSegmentPtr - (pointer to) whether init data
* @param[out] sharedGlobalVarsPtrPtr - (pointer to) pointer to shared memory
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR             - on NULL pointer
* @retval GT_OUT_OF_RANGE  -on wrong size
* @retval GT_FAIL - on allocation failure
*
*
*/
typedef GT_STATUS (*CPSS_OS_MEM_GLOBAL_DB_SHMEM_INIT)
(
    IN  unsigned int size,
    IN  GT_CHAR_PTR  name,
    OUT GT_BOOL * initDataSegmentPtr,
    OUT GT_VOID **sharedGlobalVarsPtrPtr
);

/**
* @internal CPSS_OS_MEM_GLOBAL_DB_SHMEM_UNLINK  function
* @endinternal
*
* @brief  Deletes a name from the file system.
*         If that name was the last link to a file and no processes have the file open the file is deleted and the space it was using is made available for reuse.
*         If the name was the last link to a file but any processes still have the file open the file will remain in existence until the last file descriptor referring to it is closed.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]name -  shared memory  name
*
* @retval GT_OK                    - on success
* @retval GT_FAIL -                 on unlink failure
*
*
*/
typedef GT_STATUS (*CPSS_OS_MEM_GLOBAL_DB_SHMEM_UNLINK)
(
    IN  GT_CHAR_PTR  name
);


/* phyAddr must fit in 32 bit */
#if __WORDSIZE == 64
    #define CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr)                        \
        if (0 != (phyAddr & 0xffffffff00000000L))                             \
        {                                                                     \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG); \
        }
#else
    #define CPSS_OS_PHY_ADDR_IN_32_BITS_CHECK(phyAddr)  /*empty*/
#endif


/* CPSS_OS_MEM_BIND_STC -
    structure that hold the "os memory" functions needed be bound to cpss.

*/
typedef struct{
    CPSS_OS_BZERO_FUNC                 osMemBzeroFunc;
    CPSS_OS_MEM_SET_FUNC               osMemSetFunc;
    CPSS_OS_MEM_CPY_FUNC               osMemCpyFunc;
    CPSS_OS_MEM_MOVE_FUNC              osMemMoveFunc;
    CPSS_OS_MEM_CMP_FUNC               osMemCmpFunc;
    CPSS_OS_STATIC_MALLOC_FUNC         osMemStaticMallocFunc;
    CPSS_OS_MALLOC_FUNC                osMemMallocFunc;
    CPSS_OS_REALLOC_FUNC               osMemReallocFunc;
    CPSS_OS_FREE_FUNC                  osMemFreeFunc;
    CPSS_OS_CACHE_DMA_MALLOC_FUNC      osMemCacheDmaMallocFunc;
    CPSS_OS_CACHE_DMA_MALLOC_BY_DEVICE_FUNC      osMemCacheDmaMallocByDeviceFunc;
    CPSS_OS_CACHE_DMA_FREE_FUNC        osMemCacheDmaFreeFunc;
    CPSS_OS_PHY_TO_VIRT_FUNC           osMemPhyToVirtFunc;
    CPSS_OS_VIRT_TO_PHY_FUNC           osMemVirtToPhyFunc;
    CPSS_OS_MEM_GLOBAL_DB_SHMEM_INIT   osMemGlobalDbShmemInit;
    CPSS_OS_MEM_GLOBAL_DB_SHMEM_UNLINK osMemGlobalDbShmemUnlink;
}CPSS_OS_MEM_BIND_STC;

#ifdef __cplusplus
}
#endif

#endif  /* __cpssOsMemh */



