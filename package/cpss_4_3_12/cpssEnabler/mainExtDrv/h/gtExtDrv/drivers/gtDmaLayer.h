/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file gtDmaLayer.h
*
* @brief DMA Layer API
*
* @author Yuval Shaia <yshaia@marvell.com>
*
* @version 1
********************************************************************************
*/

/*******************************************************************************
 * This module acts as a abstraction layer to DMA kernel driver.
 *
 * Background:
 * CPSS needs DMA buffers for 4 modules - RX, TX, AUQ and FUQA.
 * The current implementation request 2MB from the driver at load time and
 * then splits it across all 4 modules.
 * Each module then further splits its part according to its own needs so for
 * example, the RX module splits its share to RX descriptors and RX buffers.
 * While each descriptor and each buffer must be physically contiguous, the
 * entire block assigned to RX module, does not.
 *
 * Design:
 * The DMA layer hides the details of the blocks allocations and exposes
 * interface so caller is able to request physically contiguous chunks as much
 * as it needs (limited of course by the amount of memory in the system).
 * The blocks of DMA buffers are maintained by the DMA layer. Whenever a caller
 * request a new chunk, it first checks if there is already left overs in one of
 * the existing blocks or in free chunks list and only if not - allocate a new
 * block and satisfy the request from it. The new block is then added to the
 * blocks list.
 *
 * Following is a simple example of attaching to a 'shared' pool of blocks of
 * a specific PCI device and allocating chunk of size 4k from it.
 *
 * int do_some_alloc()
 * {
 *     EXTDRV_DMA_HANDLE *handle;
 *     unsigned long dma;
 *     void *virt;
 *
 *     handle = extDrvDmaOpen(0, 1, 0, 0, EXTDRV_DMA_OPEN_GLOBAL);
 *     if (!handle)
 *         return -1;
 *
 *     virt = extDrvDmaAlloc(handle, 4096, &dma);
 *     if (!virt)
 *         return -1;
 *
 *     extDrvDmaFree(handle, virt, 4096, dma);
 *
 *     extDrvDmaClose(handle);
 * }
*/

#ifndef __gtDmaLayerh
#define __gtDmaLayerh

#include <cpss/common/cpssTypes.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @brief Store DBDF in one u32 */
#define EXTDRV_DMA_DBDF_TO_U32(domain, bus, dev, func) \
    ((domain & 0xFF) << 16) | ((bus & 0xFF) << 8) | ((dev & 0x1F) << 3) | (func & 0x07)

/** @brief Valid flags for extDrvDmaOpen function */
#define EXTDRV_DMA_OPEN_GLOBAL (1 << 24)

/** @brief DMA block header magic number */
#define EXTDRV_DMA_BLOCK_HEADER_MAGIC   0x87654321

/**
 * struct EXTDRV_DMA_BLOCK_DESC
 *
 * @brief Descriptor of a DMA buffer allocated by the driver. The DMA Layer
 *        maintains a list of such descriptors for each DMA handle.
 */
typedef struct EXTDRV_DMA_BLOCK_DESC EXTDRV_DMA_BLOCK_DESC;
struct EXTDRV_DMA_BLOCK_DESC {
    /** @brief Virtual address of the block */
    void *virt;
    /** @brief DMA address of the block */
    unsigned long long dma;
    /** @brief Offset of the next free chunk in the block */
    int offset;
    /** @brief Next block in the list */
    EXTDRV_DMA_BLOCK_DESC *next;
};

/**
 * struct EXTDRV_DMA_CHUNK_DESC
 *
 * @brief Descriptor of a chunk of DMA buffer. The DMA Layer maintains list
 *        of such descriptors for each DMA handle to represent a pool of free
 *        buffers.
 */
typedef struct EXTDRV_DMA_CHUNK_DESC EXTDRV_DMA_CHUNK_DESC;
struct EXTDRV_DMA_CHUNK_DESC {
    /** @brief Virtual address of the chunk */
    void *virt;
    /** @brief DMA address of the chunk */
    unsigned long long dma;
    /** @brief Size of the chunk */
    size_t size;
    /** @brief Next chunk in the list */
    EXTDRV_DMA_CHUNK_DESC *next;
};

/**
 * struct EXTDRV_DMA_HANDLE
 *
 * @brief Handle represents the DMA session, created by extDrvDmaOpen function.
 */
typedef struct {
    /** @brief Device file descriptor */
    int fd;
    /** @brief Device ID */
    int dev_id;
    /** @brief Track the count of allocated blocks */
    int block_idx;
    /** @brief Is this global block (use for shared memory) */
    GT_BOOL global;
    /** @brief Pointer to first allocated DMA block */
    EXTDRV_DMA_BLOCK_DESC *blocks_head;
    /** @brief Pointer to last allocated DMA block */
    EXTDRV_DMA_BLOCK_DESC *blocks_tail;
    /** @brief Pointer to first free chunk in the free pool */
    EXTDRV_DMA_CHUNK_DESC *chunks_head;
} EXTDRV_DMA_HANDLE;

/**
 * struct EXTDRV_DMA_BLOCK_HEADER
 *
 * @brief Header of a DMA buffer allocated by the driver. Header info
 *        is used during free
 */
typedef struct EXTDRV_DMA_BLOCK_HEADER EXTDRV_DMA_BLOCK_HEADER;
struct EXTDRV_DMA_BLOCK_HEADER {
    /** @brief Header magic number */
    GT_U32 magic;
    /** @brief Size of this DMA buffer */
    GT_U32 size;
    /** @brief DMA driver handle */
    EXTDRV_DMA_HANDLE *handle;
};

/**
 * @brief Get size of each DMA block
 *
 * @retval Size of each DMA block
 */
size_t extDrvDmaGetBlockSize( void );

/**
 * @brief Creates DMA instance for the given device and option
 *
 * @param[in] domain - PCI Domain
 * @param[in] bus    - PCI Bus
 * @param[in] dev    - PCI Device
 * @param[in] func   - PCI Function
 * @param[in] flags  - Combination of the following
 *                       EXTDRV_DMA_OPEN_GLOBAL - Create or attach to device's
 *                                                global blocks
 *
 * @note For non-PCI device use -1, -1, -1 as bus, dev, func
 *
 * @retval EXTDRV_DMA_HANDLE pointer or NULL on error
 */
EXTDRV_DMA_HANDLE *extDrvDmaOpen
(
    IN int domain,
    IN int bus,
    IN int dev,
    IN int func,
    IN GT_U32 flags
);

/**
 * @brief Check if handle is global
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 *
 * @retval void
 */
GT_BOOL extDrvDmaIsGlobal
(
    IN EXTDRV_DMA_HANDLE *handle
);

/**
 * @brief Closes DMA instance
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 *
 * @retval void
 */
void extDrvDmaClose
(
    IN EXTDRV_DMA_HANDLE *handle
);

/**
 * @brief Check if Huge Pages is used (as apposed to kernel module)
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 *
 * @retval void
 */
GT_BOOL extDrvDmaIsHugePages
(
    IN EXTDRV_DMA_HANDLE *handle
);

/**
 * @brief Pre-allocate blocks for DMA allocations
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 * @param[in] size   - Requested size
 * @param[out] dma   - Array of DMA addresses of the allocated blocks
 * @param[in] asize  - Array size
 * @param[out] bsize - Size of each DMA block
 *
 * @retval
 *       GT_OK             - Success
 *       GT_BAD_VALUE      - Array size is too small
 *       GT_OUT_OF_CPU_MEM - Insufficient memory for block descriptor
 *       GT_NO_RESOURCE    - Fail to retrieve DMA address from driver
 *
 * @note This function is optional but needed in order for ATU and MG windows
 *       setup. Obviously, it will save extDrvDmaAlloc from lazy allocation of
 *       new blocks
 */
GT_STATUS extDrvDmaPreAllocBlocks
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN size_t size,
    OUT unsigned long long *dma,
    IN size_t asize,
    OUT size_t *bsize
);

/**
 * @brief Allocate buffer of DMAable memory
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 * @param[in] size   - Requested size of the buffer
 * @param[out] dma   - DMA address of the allocated buffer
 *
 * @retval Virtual address of the allocated buffer or NULL
 *
 * @note The function tries first to allocate from the free list, if no
 *       compatible entry was found then it tries from previously allocated
 *       blocks, if not enough place then it triggers call to mmap to allocate
 *       new block
 */
void *extDrvDmaAlloc
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN size_t size,
    OUT unsigned long long *dma
);

/**
 * @brief Free buffer allocated with extDrvDmaAlloc
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 * @param[in] virt   - Virtual address
 * @param[in] size   - Size of the buffer
 * @param[in] dma    - DMA address of the buffer
 *
 * @retval void
 *
 * @note The buffer is not really freed, it added to a pool and becomes a
 *       candidate to be selected by extDrvDmaAlloc
 * #note Caller can pass one of the two, virt or dma, but at leaset one
 */
void extDrvDmaFree
(
    IN void *virt
);

/**
 * @brief Retrieve the virtual address of a given DMA address
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 * @param[in] dma    - DMA address of a buffer
 *
 * @retval virtual address of the buffer or NULL
 */
void *extDrvDmaToVirt
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN unsigned long dma
);

/**
 * @brief Retrieve the 32 bit DMA address of a given virtaul address
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 * @param[in] virt  - Virtual address of a buffer
 * @param[out] dma  - DMA address of the buffer
 *
 * @retval
 *       GT_OK             - Success
 *       GT_BAD_PARAM      - If handle is NULL
 */
GT_STATUS extDrvVirtToDma
(
    IN  EXTDRV_DMA_HANDLE *handle,
    IN  void *virt,
    OUT unsigned long *dma
);

/**
 * @brief Retrieve the 64 bit DMA address of a given virtual address
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 * @param[in] virt  - Virtual address of a buffer
 * @param[out] dma  - 64 bit DMA address of the buffer
 *
 * @retval
 *       GT_OK             - Success
 *       GT_BAD_PARAM      - If handle is NULL
 */
GT_STATUS extDrvVirtToDma64
(
    IN  EXTDRV_DMA_HANDLE *handle,
    IN  void *virt,
    OUT unsigned long long *dma
);

/**
 * @brief Dump DMA to VIRT address mapping for debug purpose
 *
 * @param[in] handle - Handle of DMA instance returned from extDrvDmaOpen
 *
 * @retval
 *       GT_OK             - Success
 *       GT_BAD_PARAM      - If handle is NULL
 */
GT_STATUS extDrvDumpDmaMapping
(
    IN EXTDRV_DMA_HANDLE *handle
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __gtDmaLayerh */

