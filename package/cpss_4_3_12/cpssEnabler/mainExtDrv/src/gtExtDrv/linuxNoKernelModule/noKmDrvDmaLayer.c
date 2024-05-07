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
* @file noKmDrvDmaLayer.c
*
* @brief DMA Layer API implementation
*
* @author Yuval Shaia <yshaia@marvell.com>
*
* @version 1
********************************************************************************
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <asm-generic/mman.h>

#include <gtExtDrv/drivers/gtDmaLayer.h>
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>
#include <gtOs/gtOsMem.h>
#include "prvNoKmDrv.h"

static const char *dma_dev_file = "/dev/mvdma";
/*
 * Block size is 2MB, each block holds 512 pages.
 * For huge-pages, the assumption is that page size is 2MB
 */
static const size_t BLOCK_SIZE = CONFIG_DMA_BLK_SIZE_NUM_MB * 0x100000;

/*
 * fd = -1 indicates the use of Huge Pages
 */
static const int HUGE_PAGES_FD = -1;

/**
 * @brief Get size of each DMA block
 *
 * @retval Size of each DMA block
 */
size_t extDrvDmaGetBlockSize( void )
{
    return BLOCK_SIZE;
}

EXTDRV_DMA_HANDLE *extDrvDmaOpen
(
    IN int domain,
    IN int bus,
    IN int dev,
    IN int func,
    IN GT_U32 flags
)
{
    EXTDRV_DMA_HANDLE *handle;
    int rc;
    GT_U32 off;

    handle = osMalloc(sizeof(*handle));
    if (!handle)
    {
        return NULL;
    }
    osMemSet(handle, 0, sizeof(*handle));

    handle->block_idx = -1;

    /* Platform device allocations are always 'shared' cross clients. Callers
     * requesting PCI device allocations can force 'shared' with a flag
     */
    handle->global = (((bus == 255) && (dev == 255) && (func == 255)) ||
                      (flags & EXTDRV_DMA_OPEN_GLOBAL) == EXTDRV_DMA_OPEN_GLOBAL); 

    handle->fd = open(dma_dev_file, O_RDWR);
    if (handle->fd > 0)
    {
        off = EXTDRV_DMA_DBDF_TO_U32(domain, bus, dev, func);
        off |= flags;
        rc = lseek(handle->fd, off, 0);
        if (rc < 0)
        {
            goto err_close;
        }
        osPrintf("dmaLayer: Allocating from KM\n");
    }
    else
    {
#if !defined(MAP_HUGETLB)
        goto err_free;
#else
#if defined(SHARED_MEMORY)
    /* TODO: No support for shared memory, re-visit later */
    if (handle->global)
        osPrintf("%s: Warning: Shared memory with huge pages is not yet supported\n",
                 __func__);
#endif
    handle->fd = HUGE_PAGES_FD;
    osPrintf("dmaLayer: Allocating from Huge Pages\n");
#endif
    }

    return handle;

err_close:
    close(handle->fd);

#if !defined(MAP_HUGETLB)
err_free:
#endif
    osFree(handle);

    return NULL;
}

GT_BOOL extDrvDmaIsGlobal
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    return handle->global;
}

GT_BOOL extDrvDmaIsHugePages
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    return handle->fd == HUGE_PAGES_FD;
}

/**
 * @brief Deallocate all blocks of a given DMA handle
 *
 * @param[in] handle - Pointer to DMA handle
 */
static void extDrvDmaDeAllocBlocks
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    EXTDRV_DMA_BLOCK_DESC *b1, *b2;

    b1 = handle->blocks_head;
    while (b1)
    {
        b2 = b1;
        /* For huge-pages, unlock first */
        if (extDrvDmaIsHugePages(handle))
        {
            munlock(b2->virt, BLOCK_SIZE);
        }
        munmap(b2->virt, BLOCK_SIZE);
        b1 = b1->next;
        osFree(b2);
    }
}

/**
 * @brief Deallocate free chunks pool of a given DMA handle
 *
 * @param[in] handle - Pointer to DMA handle
 */
static void extDrvDmaDeAllocFreePool
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    EXTDRV_DMA_CHUNK_DESC *c1, *c2;

    c1 = handle->chunks_head;
    while (c1)
    {
        c2 = c1;
        c1 = c1->next;
        osFree(c2);
    }
}

void extDrvDmaClose
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    if (!handle)
    {
        return;
    }

    extDrvDmaDeAllocBlocks(handle);
    extDrvDmaDeAllocFreePool(handle);

    if (!extDrvDmaIsHugePages(handle))
    {
        lseek(handle->fd, 0xFFFFFFFF, 0);
        close(handle->fd);
    }

    osFree(handle);
}

/**
 * @brief Request one more huge page
 */
static GT_STATUS extDrvDmaHugePagesReservePage
(
)
{
    unsigned long free = 0, total = -1;
    char buf[BUFSIZ];
    FILE *f;

    f = fopen("/proc/meminfo","r");
    if (!f)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    while (!feof(f))
    {
        if (!fgets(buf, sizeof(buf) - 1, f))
            break;
        buf[sizeof(buf) - 1] = 0;

        sscanf(buf, "HugePages_Total: %lu", &total);
        sscanf(buf, "HugePages_Free: %lu", &free);
    }

    fclose(f);

    /* Do we need to add one more page to the pool */
    if (free <= 0)
    {
        f = fopen("/proc/sys/vm/nr_hugepages","w");
        if (!f)
        {
            return GT_OUT_OF_CPU_MEM;
        }

        fprintf(f,"%lu\n", total + 1);

        fclose(f);
    }

    return GT_OK;
}

/**
 * @brief Retrieve physical address of a given virtual address
 */
static unsigned long prvDrvDmaVirtToPhys(void *virt)
{
    uintptr_t pagesize = sysconf(_SC_PAGESIZE);
    uintptr_t vaddr = (uintptr_t)virt;
    int64_t paddr = 0;
    uintptr_t offset;
    int pagemap;
    uint64_t e;

    offset = (vaddr / pagesize) * sizeof(uint64_t);
    /* https://www.kernel.org/doc/Documentation/vm/pagemap.txt */
    if ((pagemap = open("/proc/self/pagemap", O_RDONLY)) < 0)
    {
        return 0;
    }

    if (lseek(pagemap, offset, SEEK_SET) != (intptr_t)offset)
    {
        close(pagemap);
        return 0;
    }

    if (read(pagemap, &e, sizeof(uint64_t)) != sizeof(uint64_t))
    {
        close(pagemap);
        return 0;
    }

    close(pagemap);

    if (e & (1ULL << 63)) /* page present ? */
    {
        paddr = e & ((1ULL << 54) - 1); /* pfn mask */
        paddr = paddr * pagesize;
        /* add offset within page */
        paddr |= (vaddr & (pagesize - 1));
    }

    return paddr;
}

/**
 * @brief Request DMA block allocation from driver
 *
 * @param[in] handle - Pointer to DMA handle
 */
static GT_STATUS extDrvDmaAllocNewBlock
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    EXTDRV_DMA_BLOCK_DESC *b;
    int rc;

    b = osMalloc(sizeof(*b));
    if (!b)
    {
        return GT_OUT_OF_CPU_MEM;
    }

    osMemSet(b, 0, sizeof(*b));

    if (!extDrvDmaIsHugePages(handle))
    {
#if !defined(SHARED_MEMORY) && !defined(MTS_BUILD)
        b->virt = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
                       handle->fd, (handle->block_idx + 1) *
                       sysconf(_SC_PAGE_SIZE));
#else
	GT_UINTPTR fixedVa;

        fixedVa = ((GT_UINTPTR)((LINUX_VMA_DMABASE)+
                  (handle->block_idx + 1)*BLOCK_SIZE)+
                  (handle->dev_id*(BLOCK_SIZE*4)));
        osPrintf("dmaLayer: Allocating from KM (Fixed VA %p)\n", fixedVa);
        b->virt = mmap((void*)fixedVa, BLOCK_SIZE,
                       PROT_READ | PROT_WRITE, MAP_FIXED | MAP_SHARED, handle->fd,
                       (handle->block_idx + 1) * sysconf(_SC_PAGE_SIZE));
#endif
        if (b->virt == MAP_FAILED)
        {
            goto err_free_block;
        }

        rc = read(handle->fd, &b->dma, sizeof(b->dma));
        if (rc < 0)
        {
            rc = GT_NO_RESOURCE;
            goto err_unmap;
        }
    }
    else
    {
        extDrvDmaHugePagesReservePage();
#if !defined(SHARED_MEMORY) && !defined(MTS_BUILD)
        b->virt = mmap(NULL, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED |
                       MAP_ANONYMOUS | MAP_HUGETLB, 0, 0);
#else
        osPrintf("dmaLayer: Allocating from Huge Pages (Fixed VA 0x%x)\n",
                 LINUX_VMA_DMABASE);
        b->virt = mmap((void*)((GT_UINTPTR)(LINUX_VMA_DMABASE)), BLOCK_SIZE,
                       PROT_READ | PROT_WRITE, MAP_FIXED |MAP_SHARED |
                       MAP_ANONYMOUS | MAP_HUGETLB, 0, 0);
#endif
        if (b->virt == MAP_FAILED)
        {
            goto err_free_block;
        }
        mlock(b->virt, BLOCK_SIZE);
        b->dma = prvDrvDmaVirtToPhys(b->virt);
        if (!b->dma)
        {
            rc = GT_NO_RESOURCE;
            goto err_unmap;
        }
    }

    handle->block_idx++;

    if (!handle->blocks_head)
    {
        handle->blocks_head = b;
    }
    else
    {
        handle->blocks_tail->next = b;
    }
    handle->blocks_tail = b;

    return GT_OK;

err_unmap:
    /* For huge-pages, unlock first */
    if (extDrvDmaIsHugePages(handle))
    {
        munlock(b->virt, BLOCK_SIZE);
    }
    munmap(b->virt, BLOCK_SIZE);

err_free_block:
    osFree(b);

    NOKMDRV_IPRINTF("DMA: Fail to allocate new block\n");

    return ENOMEM;
}

GT_STATUS extDrvDmaPreAllocBlocks
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN size_t size,
    OUT unsigned long long *dma,
    IN size_t asize,
    OUT size_t *bsize
)
{
    size_t num_of_blocks = size / BLOCK_SIZE;
    size_t i;
    int rc;

    if (!handle)
    {
        return GT_BAD_VALUE;
    }

    if (size < num_of_blocks)
    {
        return GT_BAD_VALUE;
    }

    *bsize = BLOCK_SIZE;

    osMemSet(dma, 0, asize * sizeof(*dma));

    for (i = 0; i < num_of_blocks; i++)
    {
        rc = extDrvDmaAllocNewBlock(handle);
        if (rc != GT_OK)
        {
            return rc;
        }
        dma[i] = handle->blocks_tail->dma;
    }

    return GT_OK;
}

/**
 * @brief Try to allocate chunk from free list
 *
 * @param[in] handle - Handle of DMA instance returned from dma_open
 * @param[in] size   - Requested size of the buffer
 * @param[out] dma   - DMA address of the allocated buffer
 *
 * @retval Virtual address or NULL
 */
static void *extDrvDmaPoolAlloc
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN size_t size,
    OUT unsigned long long *dma
)
{
    EXTDRV_DMA_CHUNK_DESC *c1, *c2;
    EXTDRV_DMA_BLOCK_HEADER *hdr;
    void *v = NULL;

    c1 = handle->chunks_head;
    c2 = NULL;
    while (c1)
    {
        if (c1->size >= size)
        {
            break;
        }
        c2 = c1;
        c1 = c1->next;
    }

    if (c1) {
        if (c2)
        {
            c2->next = c1->next;
        }
        else
        {
            handle->chunks_head = c1->next;
        }
        *dma = c1->dma;
        v = c1->virt;

        hdr = (EXTDRV_DMA_BLOCK_HEADER *)v;
        hdr->magic = EXTDRV_DMA_BLOCK_HEADER_MAGIC;
        hdr->size = size - sizeof(EXTDRV_DMA_BLOCK_HEADER);
        hdr->handle = handle;

        *dma = *dma + sizeof(EXTDRV_DMA_BLOCK_HEADER);
        v = v + sizeof(EXTDRV_DMA_BLOCK_HEADER);

        osFree(c1);
    }

    return v;
}

void *extDrvDmaAlloc
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN size_t size,
    OUT unsigned long long *dma
)
{
    EXTDRV_DMA_BLOCK_DESC *block;
    EXTDRV_DMA_BLOCK_HEADER *hdr;

    void *v;
    int rc;

    if (!handle)
    {
        return NULL;
    }

    if (!size || (size + sizeof(EXTDRV_DMA_BLOCK_HEADER) > BLOCK_SIZE))
    {
        return NULL;
    }

    *dma = 0;
    size += sizeof(EXTDRV_DMA_BLOCK_HEADER);

    /* Can we satisfy the request from the pool? */
    v = extDrvDmaPoolAlloc(handle, size, dma);
    if (v)
    {
        return v;
    }

    if (!handle->blocks_head)
    {
        rc = extDrvDmaAllocNewBlock(handle);
        if (rc)
        {
            return NULL;
        }
    }
retry:
    /* Can we satisfy the request from previous blocks? */
    block = handle->blocks_head;
    while (block)
    {
            if (block->offset + size <= BLOCK_SIZE)
            {
                *dma = (block->dma + block->offset);
                v = block->virt + block->offset;
                block->offset += size;

                hdr = (EXTDRV_DMA_BLOCK_HEADER *)v;
                hdr->magic = EXTDRV_DMA_BLOCK_HEADER_MAGIC;
                hdr->size = size - sizeof(EXTDRV_DMA_BLOCK_HEADER);
                hdr->handle = handle;

                *dma = *dma + sizeof(EXTDRV_DMA_BLOCK_HEADER);
                v = v + sizeof(EXTDRV_DMA_BLOCK_HEADER);

                return v;
            }
            block = block->next;
    }

    /* No alternative, let's allocate new block */
    rc = extDrvDmaAllocNewBlock(handle);
    if (rc)
    {
        return NULL;
    }

    goto retry;
}

void extDrvDmaFree
(
    IN void *virt
)
{
    EXTDRV_DMA_CHUNK_DESC *c, *p1, *p2;
    EXTDRV_DMA_BLOCK_HEADER *hdr;
    EXTDRV_DMA_HANDLE *handle;
    unsigned long long d = 0;
    size_t size;

    if (!virt)
    {
        return;
    }

    hdr = virt - sizeof(EXTDRV_DMA_BLOCK_HEADER);
    if(hdr->magic != EXTDRV_DMA_BLOCK_HEADER_MAGIC)
    {
        fprintf(stderr, "extDrvDmaFree failed\n");
        return;
    }

    handle = hdr->handle;
    size = hdr->size + sizeof(EXTDRV_DMA_BLOCK_HEADER);

    if (!handle)
    {
        return;
    }

    if (!size || (size > BLOCK_SIZE))
    {
        return;
    }

    c = osMalloc(sizeof(*c));
    if (!c)
    {
        return;
    }

    c->virt = virt - sizeof(EXTDRV_DMA_BLOCK_HEADER);
    extDrvVirtToDma64(handle, virt, &d);
    c->dma = d - sizeof(EXTDRV_DMA_BLOCK_HEADER);
    c->size = size;
    c->next = NULL;

    /* If first chunk */
    if (!handle->chunks_head)
    {
        handle->chunks_head = c;
        return;
    }

    p1 = handle->chunks_head;
    p2 = NULL;
    while (p1)
    {
        if (p1->size > c->size)
            break;

        p2 = p1;
        p1 = p1->next;
    }
    if (p2)
    {
        p2->next = c;
    }
    else
    {
        handle->chunks_head = c;
    }
    c->next = p1;
}

void *extDrvDmaToVirt
(
    IN EXTDRV_DMA_HANDLE *handle,
    IN unsigned long dma
)
{
    EXTDRV_DMA_BLOCK_DESC *b;

    /* TODO:
     * Should we maintain list of handlers so we can accept NULL handler?
     */

    if (!handle)
    {
        return NULL;
    }

    if (!dma)
    {
        return NULL;
    }

    b = handle->blocks_head;
    while (b)
    {
        if (((dma + (b->dma & 0xFFFFFFFF00000000)) >= b->dma) &&
            ((dma + (b->dma & 0xFFFFFFFF00000000)) < b->dma + BLOCK_SIZE))
        {
            return b->virt + (dma - (b->dma & 0xFFFFFFFF));
        }
        b = b->next;
    }
    return NULL;
}

GT_STATUS extDrvVirtToDma
(
    IN  EXTDRV_DMA_HANDLE *handle,
    IN  void *virt,
    OUT unsigned long *dma
)
{
    EXTDRV_DMA_BLOCK_DESC *b;

    if (!handle)
    {
        return GT_BAD_PARAM;
    }

    if (!virt || !dma)
    {
        return GT_BAD_PARAM;
    }

    b = handle->blocks_head;
    while (b)
    {
        if ((virt >= b->virt) && (virt < b->virt + BLOCK_SIZE))
        {
            *dma = (b->dma & 0xFFFFFFFF) + (virt - b->virt);
            return GT_OK;
        }
        b = b->next;
    }

    *dma = 0x0;
    return GT_FAIL;
}

GT_STATUS extDrvVirtToDma64
(
    IN  EXTDRV_DMA_HANDLE *handle,
    IN  void *virt,
    OUT unsigned long long *dma
)
{
    EXTDRV_DMA_BLOCK_DESC *b;

    if (!handle)
    {
        return GT_BAD_PARAM;
    }

    if (!virt || !dma)
    {
        return GT_BAD_PARAM;
    }

    b = handle->blocks_head;
    while (b)
    {
        if ((virt >= b->virt) && (virt < b->virt + BLOCK_SIZE))
        {
            *dma = (b->dma) + (virt - b->virt);
            return GT_OK;
        }
        b = b->next;
    }

    *dma = 0x0;
    return GT_FAIL;
}

GT_STATUS extDrvDumpDmaMapping
(
    IN EXTDRV_DMA_HANDLE *handle
)
{
    EXTDRV_DMA_BLOCK_DESC *b;

    if (!handle)
    {
        return GT_BAD_PARAM;
    }

    b = handle->blocks_head;
    while (b)
    {
        fprintf(stderr, "DMA = 0x%llx VIRT = 0x%p\n", b->dma, b->virt);
        b = b->next;
    }
    return GT_OK;
}

