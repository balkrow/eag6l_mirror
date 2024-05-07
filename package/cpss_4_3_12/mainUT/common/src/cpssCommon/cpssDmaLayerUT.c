/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
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
* @file cpssDmaLayerUT.c
*
* @brief UTs for DmaLayer
*
* @version   1
********************************************************************************
*/

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <common/tgfCommon.h>
#include <gtExtDrv/drivers/gtDmaLayer.h>

#define PRV_HELPER_SHARED_GLOBAL_VAR_GET(_var) osSharedGlobalVarsPtr->_var
#define PRV_SHARED_DEVICE_INFO_DB PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osLinuxMemSrc.dmaDeviceInfo)
#define MAX_BLOCKS  512
#define PAGE_SIZE   4096

#define BLOCK_SIZE (MAX_BLOCKS * PAGE_SIZE) /* each block holds 512 pages */
#define IF_NULL_ERROR_RETURN(p, s)                 \
    do {                                           \
        if (!p)                                    \
        {                                          \
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, s); \
            return;                                \
        }                                          \
    } while (0)
#define IF_NOT_NULL_ERROR_RETURN(p, s)             \
    do {                                           \
        if (p)                                     \
        {                                          \
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, s); \
            return;                                \
        }                                          \
    } while (0)
#define EXT_DRV_DMA_OPEN(h, flags)                                      \
    {                                                                   \
    GT_U8       domain, bus, device, func;                              \
    domain = PRV_SHARED_DEVICE_INFO_DB[prvTgfDevNum].pciDomainNum;      \
    bus = PRV_SHARED_DEVICE_INFO_DB[prvTgfDevNum].pciBusNum;            \
    device = PRV_SHARED_DEVICE_INFO_DB[prvTgfDevNum].pciDevNum;         \
    func = PRV_SHARED_DEVICE_INFO_DB[prvTgfDevNum].pciFunNum;           \
    do {                                                                \
        h = extDrvDmaOpen(domain, bus, device, func, flags);            \
        if (!h)                                                         \
        {                                                               \
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "extDrvDmaOpen failed"); \
            return;                                                     \
        }                                                               \
    } while (0); \
    }

#if defined(LINUX_NOKM)

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Allocate several DMA blocks (chunks of BLOCK_SIZE size)
*
*/
UTF_TEST_CASE_MAC(prvTgfAlloc2MbChunks)
{
#define MAX_NUM_OF_BLOCKS 20
    unsigned long long d[MAX_NUM_OF_BLOCKS];
    void *v[MAX_NUM_OF_BLOCKS];
    int num_of_blocks = 10;
    EXTDRV_DMA_HANDLE *h;
    int i;

    EXT_DRV_DMA_OPEN(h, EXTDRV_DMA_OPEN_GLOBAL);

    if ((PRV_SHARED_DEVICE_INFO_DB[prvTgfDevNum].pciDevNum == 0xFF) &&
        (PRV_SHARED_DEVICE_INFO_DB[prvTgfDevNum].pciFunNum == 0xFF))
    {
        /* TODO: read 4th byte from /proc/device-tree/reserved-memory/buffer@0x200000000/reg */
        num_of_blocks = 2;
    }

    for (i = 0; i < num_of_blocks; i++)
    {
        /* Making space for DMA header */
        v[i] = extDrvDmaAlloc(h, BLOCK_SIZE - PAGE_SIZE , &d[i]);
        /* printf("v[%d]=%p, d[%d]=0x%lx\n", i, v[i], i, d[i]); */
        IF_NULL_ERROR_RETURN(v[i], "extDrvDmaAlloc");
    }
    for (i = 0; i < num_of_blocks; i++)
    {
        extDrvDmaFree(v[i]);
    }

    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Allocate from global block from two handler instances
*
*/
UTF_TEST_CASE_MAC(prvTgfAllocGlobal)
{
    EXTDRV_DMA_HANDLE *h1, *h2;
    unsigned long long d[4];
    void *v[4];

    /* Instance #1 */
    EXT_DRV_DMA_OPEN(h1, EXTDRV_DMA_OPEN_GLOBAL);

    /* Shared memory is not supported with huge pages allocation */
    if (extDrvDmaIsHugePages(h1))
    {
        extDrvDmaClose(h1);
        return;
    }

    /* Allocate 2 chunks from instance #1 */
    v[0] = extDrvDmaAlloc(h1, 1024, &d[0]);
    IF_NULL_ERROR_RETURN(v[0], "extDrvDmaAlloc");
    v[1] = extDrvDmaAlloc(h1, 1024, &d[1]);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(d[0], d[1], "Non unique DMA addresses for the same device");

    /* Instance #2 */
    EXT_DRV_DMA_OPEN(h2, EXTDRV_DMA_OPEN_GLOBAL);

    /* Allocate 2 chunks from instance #2 */
    v[2] = extDrvDmaAlloc(h2, 1024, &d[2]);
    v[3] = extDrvDmaAlloc(h2, 1024, &d[3]);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(d[2], d[3], "Non unique DMA addresses for the same device");

    /* Expecting first two chunks from the two instances to be equal */
    UTF_VERIFY_EQUAL0_STRING_MAC(d[0], d[2], "First two allocs from global are not equal");
    UTF_VERIFY_EQUAL0_STRING_MAC(d[1], d[3], "Second two allocs from global are not equal");

    extDrvDmaFree(v[3]);
    extDrvDmaFree(v[2]);
    extDrvDmaFree(v[1]);
    extDrvDmaFree(v[0]);

    extDrvDmaClose(h2);
    extDrvDmaClose(h1);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Invalid PCI device
*
*/
UTF_TEST_CASE_MAC(prvTgfInvalidPciDevice)
{
    EXTDRV_DMA_HANDLE *h;

    h = extDrvDmaOpen(4, 5, 6, 7, EXTDRV_DMA_OPEN_GLOBAL);
    if (h && !extDrvDmaIsHugePages(h)) /* Device ID is ignord with Huge Pages */
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "extDrvDmaOpen");
    }
    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Close NULL handler
*
*/
UTF_TEST_CASE_MAC(prvTgfICloseNull)
{
    EXTDRV_DMA_HANDLE *h = NULL;

    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Alloc from the same block
*
*/
UTF_TEST_CASE_MAC(prvTgfAllocFromSameBlock)
{
    struct EXTDRV_DMA_BLOCK_DESC *b;
    unsigned long long d1, d2;
    EXTDRV_DMA_HANDLE *h;
    void *v1, *v2;
    int i;

    EXT_DRV_DMA_OPEN(h, EXTDRV_DMA_OPEN_GLOBAL);

    v1 = extDrvDmaAlloc(h, 4096, &d1);
    b = h->blocks_tail;

    /* When allocating 4K bytes DMA chunks, each chunks is prepended with 16 byte
     * header. Hence in a 2MB DMA block, max 510 4K chunks can be allocated
     * and additional 8,160 bytes will be consumed by the header
     *
     *  We are expecting the next 509 chunks to be allocated from the same block
     */
    for (i = 1; i <= 509; i++)
    {
        v2 = extDrvDmaAlloc(h, 4096, &d2);
        if (h->blocks_tail != b)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "extDrvDmaAlloc: tail was moved");
        }
        if (v2 != v1 + (i * 4096) + (i * sizeof(EXTDRV_DMA_BLOCK_HEADER)))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "extDrvDmaAlloc: unexpected virt address");
        }
        if (d2 != d1 + (i * 4096) + (i * sizeof(EXTDRV_DMA_BLOCK_HEADER)))
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "extDrvDmaAlloc: unexpected dma address");
        }
    }

    /* Next chunk cannot be satisfied from the first block, let's validate new block */
    v2 = extDrvDmaAlloc(h, 4096, &d2);
    if (h->blocks_tail == b)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "extDrvDmaAlloc: Expected new block allocation");
    }

    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Alloc too big buffer
*
*/
UTF_TEST_CASE_MAC(prvTgfAllocInvalidSize)
{
    EXTDRV_DMA_HANDLE *h;
    unsigned long long d;
    void *v;

    EXT_DRV_DMA_OPEN(h, EXTDRV_DMA_OPEN_GLOBAL);

    v = extDrvDmaAlloc(h, 4096 * 512 + 1, &d);
    IF_NOT_NULL_ERROR_RETURN(v, "extDrvDmaAlloc: Expecting NULL");

    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Alloc from pool
*
*/
UTF_TEST_CASE_MAC(prvTgfAllocFromPool1)
{
    unsigned long long d1, d2, d3;
    EXTDRV_DMA_HANDLE *h;
    void *v1, *v2, *v3;

    EXT_DRV_DMA_OPEN(h, EXTDRV_DMA_OPEN_GLOBAL);

    v1 = extDrvDmaAlloc(h, 2048, &d1);
    v2 = extDrvDmaAlloc(h, 2048, &d2);
    extDrvDmaFree(v1);
    v3 = extDrvDmaAlloc(h, 2048, &d3);
    /* Expecting to receive previously freed v1 address from the pool */
    UTF_VERIFY_EQUAL0_STRING_MAC(d1, d3, "extDrvDmaAlloc: Expecting 1 and 3 to be the same");

    extDrvDmaFree(v3);
    v3 = extDrvDmaAlloc(h, 4096, &d3);
    /* Expecting NOT to receive the previously freed v1 address from the pool b/c size is different */
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(d1, d3, "extDrvDmaAlloc: Expecting 1 and 3 to be not the same");

    /* Just to avoid compilation error "set but not used" */
    extDrvDmaFree(v2);

    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Alloc from pool with size granularity
*
*/
UTF_TEST_CASE_MAC(prvTgfAllocFromPool2)
{
    unsigned long long d1, d2, d3, d4;
    void *v1, *v2, *v3, *v4;
    EXTDRV_DMA_HANDLE *h;

    EXT_DRV_DMA_OPEN(h, EXTDRV_DMA_OPEN_GLOBAL);

    v1 = extDrvDmaAlloc(h, 4096, &d1);
    v2 = extDrvDmaAlloc(h, 1024, &d2);
    v3 = extDrvDmaAlloc(h, 2048, &d3);
    v4 = extDrvDmaAlloc(h, 2048, &d4);
    extDrvDmaFree(v4);
    extDrvDmaFree(v2);
    extDrvDmaFree(v3);
    extDrvDmaFree(v1);

    v2 = extDrvDmaAlloc(h, 2048, &d2);
    /* Expecting to receive the previously freed v4 address from the pool */
    UTF_VERIFY_EQUAL0_STRING_MAC(d2, d4, "extDrvDmaAlloc: Expecting to receive d4");

    extDrvDmaClose(h);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
*  Test description: Copy from two virtual addresses to the same DMA
*
*/
UTF_TEST_CASE_MAC(prvTgfSharedBuff)
{
#define BUFF_SZ 8
    char backup1[BUFF_SZ], backup2[BUFF_SZ];
    EXTDRV_DMA_HANDLE *h1, *h2;
    unsigned long long d1, d2;
    void *v1, *v2;
    int rc;

    EXT_DRV_DMA_OPEN(h1, EXTDRV_DMA_OPEN_GLOBAL);

    /* Shared memory is not supported with huge pages allocation */
    if (extDrvDmaIsHugePages(h1))
    {
        extDrvDmaClose(h1);
        return;
    }

    v1 = extDrvDmaAlloc(h1, 1024, &d1);

    EXT_DRV_DMA_OPEN(h2, EXTDRV_DMA_OPEN_GLOBAL);
    v2 = extDrvDmaAlloc(h2, 1024, &d2);
    UTF_VERIFY_EQUAL0_STRING_MAC(d1, d2, "Expecting the same dma address");

    /* Save content to be restored after the test */
    osMemCpy(backup1, v1, BUFF_SZ);
    osMemCpy(backup2, v2, BUFF_SZ); /* Need this in case test fails */

    osMemSet(v2, 0, BUFF_SZ);
    osMemCpy(v1, "10203040", BUFF_SZ);

    rc = osMemCmp(v1, v2, BUFF_SZ);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rc, "Expecting the same content");

    /* Restore memory from backup */
    osMemCpy(v1, backup1, BUFF_SZ);
    if (rc)
        osMemCpy(v2, backup2, BUFF_SZ);

    extDrvDmaClose(h2);
    extDrvDmaClose(h1);
}

/* test fails on multiple boards */
#if 0
/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
* Test description: Address translation virt to dma
*
*/
UTF_TEST_CASE_MAC(prvTgfVirtToDma)
{
    unsigned long long d1, d2, d3;
    EXTDRV_DMA_HANDLE *h1, *h2;
    void *v1, *v2;

    EXT_DRV_DMA_OPEN(h1, 0);
    v1 = extDrvDmaAlloc(h1, 1024, &d1);

    EXT_DRV_DMA_OPEN(h2, 0);
    v2 = extDrvDmaAlloc(h2, 1024, &d2);

    /* Conversion functions trims the upper 32 bits, we need to re-visit it,
     * for now let's do some hack
     */
    d1 &= 0xFFFFFFFF;
    d2 &= 0xFFFFFFFF;

    extDrvVirtToDma(h1, v1, (unsigned long *)&d3);
    UTF_VERIFY_EQUAL0_STRING_MAC(d3, d1, "Expecting same DMA address");

    extDrvVirtToDma(h2, v1, (unsigned long *)&d3);
    if (!extDrvDmaIsGlobal(h2) && d3)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Expecting not to find a match, unless shared");
    }

    extDrvVirtToDma(h2, v2, (unsigned long *)&d3);
    UTF_VERIFY_EQUAL0_STRING_MAC(d3, d2, "Expecting same DMA address");

    extDrvDmaFree(v2);
    extDrvDmaFree(v1);

    extDrvDmaClose(h2);
    extDrvDmaClose(h1);
}
#endif
/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
* Test description: Address translation dma to virt
*
*/
UTF_TEST_CASE_MAC(prvTgfDmaToVirt)
{
    EXTDRV_DMA_HANDLE *h1, *h2;
    unsigned long long d1, d2;
    void *v1, *v2, *v3;

    EXT_DRV_DMA_OPEN(h1, 0);
    v1 = extDrvDmaAlloc(h1, 1024, &d1);

    EXT_DRV_DMA_OPEN(h2, 0);
    v2 = extDrvDmaAlloc(h2, 1024, &d2);

    /* Conversion functions trims the upper 32 bits, we need to re-visit it,
     * for now let's do some hack
     */
    d1 &= 0xFFFFFFFF;
    d2 &= 0xFFFFFFFF;

    v3 = extDrvDmaToVirt(h1, d1);
    if (v3 != v1)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Expecting same virtual address");
    }

    v3 = extDrvDmaToVirt(h2, d1);
    if (!extDrvDmaIsGlobal(h2) && v3)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Expecting not to find a match, unless shared");
    }

    v3 = extDrvDmaToVirt(h2, d2);
    if (v3 != v2)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(0, 1, "Expecting same virtual address");
    }

    extDrvDmaClose(h2);
    extDrvDmaClose(h1);
}

/*----------------------------------------------------------------------------------*/
/* AUTODOC:
*
* Test description: Allocation from one of the previous blocks
*
*/
UTF_TEST_CASE_MAC(prvTgfAllocPrevBlock)
{
    unsigned long long d1, d2, d3;
    EXTDRV_DMA_HANDLE *h;
    void *v1, *v2, *v3;

    EXT_DRV_DMA_OPEN(h, 0);
    /* Let's leave a hole */
    v1 = extDrvDmaAlloc(h, BLOCK_SIZE - 1024, &d1);
    v2 = extDrvDmaAlloc(h, BLOCK_SIZE - sizeof(EXTDRV_DMA_BLOCK_HEADER), &d2);
    /* Attempting to consume the hole */
    v3 = extDrvDmaAlloc(h, 516, &d3);

    if (d3 != d1 + sizeof(EXTDRV_DMA_BLOCK_HEADER) + (BLOCK_SIZE - 1024))
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(d3, d1, "Expecting from block #1");
    }

    /* Just to avoid compilation error "set but not used" */
    extDrvDmaFree(v1);
    extDrvDmaFree(v2);
    extDrvDmaFree(v3);

    extDrvDmaClose(h);
}

#endif

/*
 * Configuration of cpssDmaLayer suit
 */
UTF_SUIT_BEGIN_TESTS_MAC(cpssDmaLayer)

#if defined(LINUX_NOKM)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAlloc2MbChunks)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAllocGlobal)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfInvalidPciDevice)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfICloseNull)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAllocFromSameBlock)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAllocInvalidSize)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAllocFromPool1)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAllocFromPool2)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfSharedBuff)
#if 0
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfVirtToDma)
#endif
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfDmaToVirt)
    UTF_SUIT_DECLARE_TEST_MAC(prvTgfAllocPrevBlock)
#endif

UTF_SUIT_END_TESTS_MAC(cpssDmaLayer)

