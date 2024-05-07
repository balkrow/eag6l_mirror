/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file noKmDrvCacheMng.c
*
* @brief Cache management functions
* Use mvDmaDrv or HUGETLB feature
* On Intel CPUs requires 'intel_iommu=off' kernel option
*
* @version   1
********************************************************************************
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>
#include <stdlib.h>
#include "prvNoKmDrv.h"
#undef   _64M
#undef   _4K
#undef   _16M
#undef   _8M
#undef   _4M
#undef   _2M
#undef   _64K
#undef   _1M
#undef   _256K
#undef   _512K
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#include <gtExtDrv/drivers/gtDmaLayer.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>


/**
* @internal extDrvMgmtCacheFlush function
* @endinternal
*
* @brief   Flush to RAM content of cache
*
* @param[in] type                     -  of cache memory data/intraction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvMgmtCacheFlush
(
    IN GT_MGMT_CACHE_TYPE_ENT   type,
    IN void                     *address_PTR,
    IN size_t                   size
)
{
    (void) type;
    (void) address_PTR;
    (void) size;
    return GT_OK;
}


/**
* @internal extDrvMgmtCacheInvalidate function
* @endinternal
*
* @brief   Invalidate current content of cache
*
* @param[in] type                     -  of cache memory data/intraction
* @param[in] address_PTR              - starting address of memory block to flush
* @param[in] size                     -  of memory block
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvMgmtCacheInvalidate
(
    IN GT_MGMT_CACHE_TYPE_ENT   type,
    IN void                     *address_PTR,
    IN size_t                   size
)
{
    (void) type;
    (void) address_PTR;
    (void) size;
    return GT_OK;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

GT_UINTPTR prvExtDrvDmaPhys = 0;
int prvExtDrvDmaFd = 0;

#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif

uint64_t linux_virt_to_phys(uintptr_t vaddr)
{
    int pagemap;
    int64_t paddr = 0;
    uintptr_t pagesize = getpagesize();
    uintptr_t offset;
    uint64_t e;

    offset = (vaddr / pagesize) * sizeof(uint64_t);
    /* https://www.kernel.org/doc/Documentation/vm/pagemap.txt */
    if ((pagemap = open("/proc/self/pagemap", O_RDONLY)) < 0)
        return 0;
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
    if (e & (1ULL << 63)) { /* page present ? */
        paddr = e & ((1ULL << 54) - 1); /* pfn mask */
        paddr = paddr * getpagesize();
        /* add offset within page */
        paddr |= (vaddr & (pagesize - 1));
    }
    return paddr;
}

#if defined(MAP_HUGETLB) && !defined(CONFIG_V2_DMA)

static const GT_U64 NULLU64 = {{0, 0}};

static GT_U64 uint64_to_GT_U64(uint64_t u64)
{
    GT_U64 ret;

    /* Make l[1] the high and l[0] the low */
    ret.l[0] = (GT_U32)u64;
    ret.l[1] = (GT_U32)(u64 >> 32);

    return ret;
}

/* try HUGETLB feature */
static long hugePagesTotal = -1;
static long hugePagesFree = -1;
static long hugePageSize = -1;
static int scan_proc_meminfo(void)
{
    FILE *f;
    char buf[BUFSIZ];
    unsigned long num;

    f = fopen("/proc/meminfo","r");
    if (!f)
        return 1;
    while (!feof(f))
    {
        if (!fgets(buf,sizeof(buf)-1,f))
            break;
        buf[sizeof(buf)-1] = 0;
        if (sscanf(buf, "HugePages_Total: %lu", &num) == 1)
        {
            hugePagesTotal = num;
            NOKMDRV_IPRINTF(("hugePagesTotal=%lu\n",hugePagesTotal));
        }
        if (sscanf(buf, "HugePages_Free: %lu", &num) == 1)
        {
            hugePagesFree = num;
            NOKMDRV_IPRINTF(("hugePagesFree=%lu\n",hugePagesFree));
        }
        if (sscanf(buf, "Hugepagesize: %lu", &num) == 1)
        {
            hugePageSize = num * 1024;
            NOKMDRV_IPRINTF(("hugePageSize=%lu\n",hugePageSize));
        }
    }
    fclose(f);
    if (hugePagesTotal < 0 || hugePagesFree < 0 || hugePageSize < 0)
        return 1;
    return 0;
}

static int alloc_hugetlb(void)
{
    void *h;
    if (scan_proc_meminfo())
        return 1;
    if (hugePagesFree == 0)
    {
        FILE *f;
        f = fopen("/proc/sys/vm/nr_hugepages","w");
        if (f == NULL)
            return 1;
        fprintf(f,"%lu\n", hugePagesTotal + 1);
        fclose(f);
        if (scan_proc_meminfo())
            return 1;
        if (hugePagesFree == 0)
            return 1;
    }

    h = mmap(
#ifndef SHARED_MEMORY
            NULL,
#else /* SHARED_MEMORY */
            (void*)((GT_UINTPTR)LINUX_VMA_DMABASE),
#endif
            hugePageSize,
            PROT_READ | PROT_WRITE,
#ifdef SHARED_MEMORY
            MAP_FIXED |
#endif
            MAP_ANONYMOUS | MAP_SHARED | MAP_HUGETLB,
            0, 0);
    if (h == MAP_FAILED)
    {
        perror("mmap(MAP_HUGETLB)");
        return 1;
    }

    PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPtr,h);
    PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaLen,hugePageSize);

    /* resolve phys address */
    mlock(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr), PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen));
    PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPhys64,
        uint64_to_GT_U64((linux_virt_to_phys((uintptr_t)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr)))));

    prvExtDrvDmaPhys = (GT_UINTPTR)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0];
    if ((GT_UINTPTR)(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1])
            && (getenv("MVPP_DENY64BITPHYS") != NULL))
    {
        /* Address above 32bit address space, not supported by Marvell PPs */
        munlock(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr), PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen));
        munmap(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr), PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen));

        PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPtr,NULL);
        PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaLen,0);
        prvExtDrvDmaPhys = (GT_UINTPTR)0L;
        PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPhys64, NULLU64);

    }
    else
    {
        NOKMDRV_IPRINTF(("hugePage mapped to %p\n", PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr)));
        /* reset high 32 bits */
        prvExtDrvDmaPhys = (GT_UINTPTR)(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0]);
    }

    return 0;
}
#endif



extern GT_STATUS traverse_sysfs_pci_devices(
    void (*cb)(DBDF_DECL, void *opaque),
    void *opaque
);
extern int sysfs_pci_detect_dev(
    DBDF_DECL,
    int print_found
);
extern GT_STATUS sysfs_pci_readNum(
    DBDF_DECL,
    IN  const char *name,
    OUT unsigned *val
);

GT_STATUS sysfs_mvDmaDrvOffset_set_and_open_new_window(IN DBDF_DECL);
void sysfs_sdma_window_ended(void);
/* Subset of code from : sysfs_pci_configure_pex
   Callback for 'traverse_sysfs_pci_devices' that determine if the visited device is our PP.
    Note that, in multi-device board, the final value of mvDmaDrvOffset will be the last visited device
*/
#if !defined(CONFIG_V2_DMA)
static void set_mvDmaDrvOffset(
    DBDF_DECL,
    void *opaque
)
{
    int i;

    GT_UNUSED_PARAM(opaque);

    i = sysfs_pci_detect_dev(DBDF, 1);
    if (i < 0)
        return; /* filtered as non-mavell-switch (non-supported-misl-device) */

    (void)sysfs_mvDmaDrvOffset_set_and_open_new_window(DBDF);

    return ;
}

static GT_STATUS dmaOverlapCheck(
    IN  GT_U64  dmaAddr,
    OUT GT_U32  *overlappedWindowPtr
)
{
    GT_U32  win;
    GT_U32  numOfActiveWindows = PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigNumOfActiveWindows);
    GT_U64  win_dma;
    GT_U32  win_dmaLen;

    NOKMDRV_IPRINTF(("check overlap of dmaAddr [0x%lx] \n",dmaAddr));

    for(win = 0 ; win < numOfActiveWindows ; win++)
    {
        win_dma    = osNonSharedGlobalVars.osNonVolatileDb.dmaConfig[win].prvExtDrvDmaPhys64;
        win_dmaLen = osNonSharedGlobalVars.osNonVolatileDb.dmaConfig[win].prvExtDrvDmaLen;

        if(0 == win_dmaLen)
        {
            /* not used yet */
            continue;
        }

	if ((dmaAddr.l[1] == win_dma.l[1]) &&
            dmaAddr.l[0] >= win_dma.l[0] && dmaAddr.l[0] < (win_dma.l[0]+win_dmaLen))
        {
            *overlappedWindowPtr = win;
            NOKMDRV_IPRINTF(("overlap of dmaAddr [0x%lx] in window [%d] \n",dmaAddr,win));
            return GT_ALREADY_EXIST;
        }
    }

    NOKMDRV_IPRINTF(("NO overlap of dmaAddr [0x%lx] windows [0..%d] \n",dmaAddr,numOfActiveWindows-1));
    /* no overlapping */
    *overlappedWindowPtr = 0;
    return GT_OK;
}
#endif
typedef struct{
    void*       virtMmap;
    int         myFd;
    GT_U64      dmaAddr;
}FILE_INFO_STC;

#define MAX_RETRY 3


#ifdef SHARED_MEMORY
#ifdef CONFIG_V2_DMA

/**
* @internal extDrvMapDmaBasePerDevice function
* @endinternal
*
* @brief   function update DMA handle per process(called for secondary process)
*
* INPUTS:
*       devNum      - Device number
*
*/
void extDrvMapDmaBasePerDevice
(
    GT_U8 devNum
)
{

    GT_U32 dmaSize = DMA_LEN;
    GT_UINTPTR physicalAddr;

    extDrvGetDeviceDmaBase(devNum, dmaSize, &physicalAddr);

    SHM_PRINT("Device %d DMA physical adress %p Virtual adress %p\n",devNum,
        (GT_UINTPTR)(((EXTDRV_DMA_HANDLE *)PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]))->blocks_head->dma),
        (GT_UINTPTR)(((EXTDRV_DMA_HANDLE *)PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]))->blocks_head->virt));
}
#endif
#endif


static void try_map_mvDmaDrv(void)
{
#if defined(CONFIG_V2_DMA)

    GT_U8 devNum = PRV_OS_DMA_CURR_DEVICE;
    GT_U32 dmaSize = DMA_LEN;
    GT_UINTPTR dmaBase;
    if(GT_TRUE ==PRV_OS_DMA_CURR_DEVICE_CONFIGURED)
    {
        extDrvGetDeviceDmaBase(devNum, dmaSize, &dmaBase);
    }
#else
    int fd;
    void*     prvExtDrvDmaLocalPtr;
    FILE_INFO_STC    retryArr[MAX_RETRY];/* array used for retry to get unique DMA memory address
                                    that not used by other device */
    GT_U32    retryIndex,ii,overlappedWindow;
    GT_U64  dmaAddr = NULLU64;

    /* to don't mix message with kernel messages
     * flush sdtout then wait till all output written */
    fflush(stdout);
    tcdrain(1);

    fd = open("/dev/mvDmaDrv", O_RDWR);
    if (fd >= 0)
    {
        /* mvDmaDrvOffset is ZERO for second process ,as no one set it
           if the 00:00:00.0 device     exists on the PCIe , the mmap will succeed
           if the 00:00:00.0 device NOT exists on the PCIe , the mmap will FAIL

           the code below added to support the ASIM that device 00:00:00.0 NOT exists on the PCIe.
        */
        if(0 == PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(mvDmaDrvOffset))
        {
            unsigned pciVendorId;
            if(sysfs_pci_readNum(0/*pciDomain*/,0/*pciBus*/,0/*pciDev*/,0/*pciFunc*/, "vendor", &pciVendorId) != GT_OK)
            {
                /* will set mvDmaDrvOffset with the last switch on the PCIe just to make the KO happy */
                traverse_sysfs_pci_devices(set_mvDmaDrvOffset, NULL);

                /* indicate that the SDMA window mapping ended */
                sysfs_sdma_window_ended();
            }
        }

        for(retryIndex = 0 ; retryIndex < MAX_RETRY; retryIndex ++)
        {
            retryArr[retryIndex].myFd = fd;
            lseek(fd, PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(mvDmaDrvOffset), 0);

            PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaLen,DMA_LEN);

            prvExtDrvDmaLocalPtr = mmap(
#if !defined(SHARED_MEMORY) && !defined(MTS_BUILD)
                NULL,
#else /* SHARED_MEMORY */
                (void*)((GT_UINTPTR)(LINUX_VMA_DMABASE + PRV_OS_DMA_CURR_WINDOW*DMA_LEN)),
#endif
                DMA_LEN,
                PROT_READ | PROT_WRITE,
#if defined(SHARED_MEMORY) || defined(MTS_BUILD)
                MAP_FIXED |
#endif
                MAP_32BIT | MAP_SHARED,
                fd, 0);
            if (prvExtDrvDmaLocalPtr == MAP_FAILED)
            {
                perror("mmap(mvDmaDrv)");
                PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPtr,NULL);
                close(fd);
                return;
            }

            retryArr[retryIndex].virtMmap = prvExtDrvDmaLocalPtr;

            /* to don't mix message with kernel messages
             * flush sdtout then wait till all output written */
            fflush(stdout);
            tcdrain(1);
            /* check if this address not fall into 'already used' address range by other device */
            if (read(fd, &dmaAddr, 8) != 8)
            {
                break;
            }

            retryArr[retryIndex].dmaAddr = dmaAddr;

            if(GT_OK == dmaOverlapCheck(dmaAddr,&overlappedWindow))
            {
                PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPtr,prvExtDrvDmaLocalPtr);
                NOKMDRV_IPRINTF(("mvDmaDrv mapped to virtAddr[%p] dmaAddr[0x%lx]\n",
                    prvExtDrvDmaLocalPtr,retryArr[retryIndex].dmaAddr));

                /* release the previous overlapped memories */
                for(ii = 0 ; ii < retryIndex ; ii++)
                {
                    NOKMDRV_IPRINTF(("release mvDmaDrv mapped to virtAddr[%p] (dmaAddr[0x%lx]) \n",
                        retryArr[ii].virtMmap,retryArr[ii].dmaAddr));

                    munmap(retryArr[ii].virtMmap,DMA_LEN);

                    NOKMDRV_IPRINTF(("close mvDmaDrv fd [%d] \n",
                        retryArr[ii].myFd));

                    close(retryArr[ii].myFd);
                }

                break;/* no more needed */
            }

            NOKMDRV_IPRINTF(("overlapped mvDmaDrv mapped to [%p] overlapped dmaAddr[0x%lx] by window[%d] \n",
                prvExtDrvDmaLocalPtr,retryArr[retryIndex].dmaAddr,overlappedWindow));

            fd = open("/dev/mvDmaDrv", O_RDWR);
            if (fd < 0)
            {
                NOKMDRV_IPRINTF(("FAILED to open the mvDmaDrv file on [retryIndex = %d] \n",
                    retryIndex));
                break;
            }
        }
    }

    prvExtDrvDmaFd = fd;
    prvExtDrvDmaPhys = 0;

    PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPhys64, NULLU64);
    if (read(fd, &(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64)), 8) == 8)
    {
        prvExtDrvDmaPhys = (GT_UINTPTR)(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0]);
    }
#endif
}

/**
* @internal check_dma function
* @endinternal
*
* @brief   Check if DMA block already allocated/mapped to userspace
*         If no then allocate/map
*/
static void check_dma(void)
{
#if !defined(CONFIG_V2_DMA)
    if (PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr) != NULL)
        return;
#endif
    /* try to map from mvDmaDrv */
    try_map_mvDmaDrv();
#if defined(MAP_HUGETLB) && !defined(CONFIG_V2_DMA)
    if (PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr) == NULL)
    {
        osPrintf("Allocating DMA buffer from huge pages (v1)\n");
        /* we get here since the 'mvDmaDrv.ko' is not running , and we will try
           to get mmap using : hugetlb */
#if defined(SHARED_MEMORY)
        if(GT_TRUE==osNonSharedGlobalVars.osNonVolatileDb.isFirst)
#endif/*SHARED_MEMORY*/
        {
            /* try to allocate hugetlb */
            alloc_hugetlb();
        }
#if defined(SHARED_MEMORY)
        else
        {
            osPrintf("INFO : DMA is not mapped .\n");
        }
#endif/*SHARED_MEMORY*/
    }
    else
    {
        osPrintf("Allocated DMA buffer from mvDmaDrv (v1)\n");
    }
#else /* MAP_HUGETLB */
    if (PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr) == NULL)
    {
#if !defined(CONFIG_V2_DMA)
        osPrintf("Fail to allocate DMA buffer from mvDmaDrv and no support for huge pages\n");
#endif
    /*For V2 this is ok.DMA buffer will be allocated later*/
    }
    else
    {
        osPrintf("Allocated DMA buffer from mvDmaDrv (v2)\n");
    }
#endif/*MAP_HUGETLB*/
NOKMDRV_IPRINTF(("dmaLen=%d\n", PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen)));
NOKMDRV_IPRINTF(("dmaPhys64[0]=0x%llx\n", (PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0])));
NOKMDRV_IPRINTF(("dmaPhys64[1]=0x%llx\n", (PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1])));
NOKMDRV_IPRINTF(("dmaPhys=%p\n", (void*)prvExtDrvDmaPhys));
}

#if defined(CONFIG_V2_DMA)
/**
* @internal extDrvGetDeviceDmaBase function
* @endinternal
*
* @brief   Get DMA address of a device required for Virt2Phys and Phys2Virt
            translation
*
* @param[in]    devNum             - device number
* @param[in]    dmaSize            - size of the DMA buffer to be allocated
* @param[out]   dmaBase            - the base address of the DMA buffer
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDeviceDmaBase
(
    IN  GT_U8        devNum,
    IN  GT_U32       dmaSize,
    OUT GT_UINTPTR  *dmaBase
)
{
    GT_U64_BIT  *prvExtDrvDmaPhysPtr = NULL;
    size_t          prvDmaBlockSize;
    GT_STATUS       prvStatus;
    GT_U8       pciDomainNum;
    GT_U8       pciBusNum;
    GT_U8       pciDevNum;
    GT_U8       pciFunNum;
    GT_U8       numDmaBlocks = (dmaSize)/extDrvDmaGetBlockSize();
    GT_STATUS   rc;

    rc = prvNoKmDrv_get_pci_info_by_devNum(devNum, &pciDomainNum, &pciBusNum, &pciDevNum, &pciFunNum);
    if(rc != GT_OK) {
        NOKMDRV_IPRINTF(("prvNoKmDrv_get_pci_info_by_devNum failed, rc = %d\n", rc));
        return GT_FAIL;
    }

    if (PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]) == NULL) {
        PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]) = extDrvDmaOpen(pciDomainNum,pciBusNum,pciDevNum,pciFunNum,EXTDRV_DMA_OPEN_GLOBAL);

        prvExtDrvDmaPhysPtr = (GT_U64_BIT *)malloc(numDmaBlocks * sizeof(GT_U64_BIT));
        if(prvExtDrvDmaPhysPtr == NULL)
            return GT_FAIL;
        else
            osMemSet(prvExtDrvDmaPhysPtr, 0, (numDmaBlocks * sizeof(GT_U64_BIT)));

        if (PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]) != NULL)
        {
                NOKMDRV_IPRINTF(("extDrvGetDeviceDmaBase: Opening mvDmaDrv for pre-allocation of %d MB with BDF = %x:%x:%x.%x\n", (dmaSize / (1024 * 1024)), pciDomainNum, pciBusNum, pciDevNum, pciFunNum));

		((EXTDRV_DMA_HANDLE *)(PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum])))->dev_id = devNum;
                prvStatus = extDrvDmaPreAllocBlocks(PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]), dmaSize, prvExtDrvDmaPhysPtr, numDmaBlocks, &prvDmaBlockSize);
                if (prvStatus != GT_OK)
                {
                    perror("extDrvDmaPreAllocBlocks");
                    PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPtr,NULL);
                    free(prvExtDrvDmaPhysPtr);
                    return GT_FAIL;
                }


    #if __WORDSIZE == 64
            PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0] =
                ((GT_U64_BIT)prvExtDrvDmaPhysPtr[0] & 0xFFFFFFFF);
            PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1] =
                (((GT_U64_BIT)prvExtDrvDmaPhysPtr[0] >> 32 ) & 0xFFFFFFFF);
    #else
            prvExtDrvDmaPhys = ((GT_U64_BIT)prvExtDrvDmaPhysPtr[0] & 0xFFFFFFFF);
    #endif
            PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaPtr, (void *)0xFFFFFFFF);
            PRV_NON_SHARED_DMA_GLOBAL_VAR_SET(prvExtDrvDmaLen, dmaSize);
        }

    #if __WORDSIZE == 64
        *dmaBase = (((GT_U64_BIT)prvExtDrvDmaPhysPtr[0] >> 32 ) & 0xFFFFFFFF) << 32;
        *dmaBase |= ((GT_U64_BIT)prvExtDrvDmaPhysPtr[0] & 0xFFFFFFFF);
    #else
        *dmaBase = (GT_U64_BIT)prvExtDrvDmaPhysPtr[0];
    #endif
        if (!(*dmaBase)) {
            free(prvExtDrvDmaPhysPtr);
            return GT_FAIL;
        }

        rc = prvNoKmDrv_set_dma_info_by_devNum(devNum, numDmaBlocks, extDrvDmaGetBlockSize(), prvExtDrvDmaPhysPtr);
        if(rc != GT_OK) {
            NOKMDRV_IPRINTF(("prvNoKmDrv_set_dma_info_by_devNum failed, rc = %d\n", rc));
            free(prvExtDrvDmaPhysPtr);
            return GT_FAIL;
        }

        free(prvExtDrvDmaPhysPtr);
    }

    return GT_OK;
}
#endif

/**
* @internal extDrvGetDmaBase function
* @endinternal
*
* @brief   Get the base address of the DMA area need for Virt2Phys and Phys2Virt
*         translaton
*
* @param[out] dmaBase                  - the base address of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaBase
(
    OUT GT_UINTPTR * dmaBase
)
{
    check_dma();

#if __WORDSIZE == 64
    *dmaBase = (GT_UINTPTR)(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1]) << 32;
    *dmaBase |= (GT_UINTPTR)(PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0]);
#else
    *dmaBase = (GT_UINTPTR)prvExtDrvDmaPhys;
#endif
    if (!(*dmaBase))
        return GT_ERROR;

    return GT_OK;
}

/**
* @internal extDrvGetDmaVirtBase function
* @endinternal
*
* @brief   Get the base address of the DMA area in userspace
*
* @param[out] dmaVirtBase              - the base address of the DMA area in userspace.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS extDrvGetDmaVirtBase
(
    OUT GT_UINTPTR *dmaVirtBase
)
{
    check_dma();
    *dmaVirtBase = (GT_UINTPTR)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr);
    if (!(*dmaVirtBase))
        return GT_ERROR;

    return GT_OK;
}

/**
* @internal extDrvGetDmaSize function
* @endinternal
*
* @brief   Get the size of the DMA area
*
* @param[out] dmaSize                  - The size of the DMA area.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
extern GT_STATUS extDrvGetDmaSize
(
    OUT GT_U32 * dmaSize
)
{
    *dmaSize = PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen);
    return GT_OK;
}


static GT_MUTEX testMutex = 0;
GT_STATUS noKmExtDrvPerformanceDmaTest(IN GT_U32 offset,IN GT_U32  numOfTimes , IN GT_U32   useMutex)
{
    GT_STATUS   rc;
    GT_UINTPTR  dmaVirtBase;
    GT_U32  data=0;
    GT_U32  ii;
    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    if(testMutex == 0)
    {
        osMutexCreate("testDmaMutex", &testMutex);
    }

    rc = extDrvGetDmaVirtBase(&dmaVirtBase);
    if(rc!=GT_OK)
    {
        return rc;
    }

    osTimeRT(&secondsStart,&nanoSecondsStart);

#define UNLOCK_AND_LOCK           \
    if(useMutex)\
    { \
       osMutexUnlock(testMutex); \
       osMutexLock(testMutex);   \
    }

    if(useMutex)osMutexLock(testMutex);
    for(ii = 0 ; ii < numOfTimes; ii++)
    {
        data = *((volatile GT_U32*)(dmaVirtBase+offset));
        UNLOCK_AND_LOCK;
    }
    if(useMutex)osMutexUnlock(testMutex);

    osTimeRT(&secondsEnd,&nanoSecondsEnd);

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }

    osPrintf("Performance of read DMA offset [0x%8.8x] for [%d] times in [%d] ms (dmaValue=[0x%8.8x]) \n",
        offset,numOfTimes,(seconds*1000+nanoSec/1000000),data);

    return GT_OK;
}

/* debug function to print physical memory (according to value written in the register)*/
GT_STATUS prvNoKmDrv_debug_print_physical_memory(
    IN GT_U32   phyAddr_0_31,
    IN GT_U32   numOfWords
)
{
    GT_U32  ii,offset_from_startDma;
    uint64_t phyAddr_64 = (uint64_t)phyAddr_0_31;
    uint64_t *memPtr;
    GT_VOID *dmaPhysPtr0 = (GT_VOID *)(GT_UINTPTR)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0];
    GT_VOID *dmaPhysPtr1 = (GT_VOID *)(GT_UINTPTR)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1];

    printf("physical prvExtDrvDmaPhysH [%p]\n"    ,dmaPhysPtr1 );
    printf("physical prvExtDrvDmaPhysL [%p]\n"    ,dmaPhysPtr0 );
    printf("VIRTUAL  prvExtDrvDmaPtr   [%p]\n"    ,PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr)   );

    if(phyAddr_64 < prvExtDrvDmaPhys || phyAddr_64 >= (prvExtDrvDmaPhys + PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen)))
    {
        printf("Requested physical address [0x%x] is out of range [%p .. %p]\n", phyAddr_0_31, dmaPhysPtr1,
                (GT_VOID *)((GT_UINTPTR)dmaPhysPtr1 + PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaLen)));
        return GT_BAD_PARAM;
    }

    offset_from_startDma = (phyAddr_64 - prvExtDrvDmaPhys);
    printf("offset_from_startDma [0x%x] \n"    ,offset_from_startDma   );

    memPtr = (uint64_t*)((GT_U8*)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr) + offset_from_startDma);

    for(ii = 0 ; ii < numOfWords/2; ii++,memPtr++)
    {
        printf("word[%2.2d] (low)  value[0x%8.8x] \n",
            ii*2,(GT_U32)(*memPtr));
        printf("word[%2.2d] (high) value[0x%8.8x] \n",
            1 + (ii*2),(GT_U32)((*memPtr) >> 32));
    }

    return GT_OK;
}

/**
* @internal extDrvMapDmaRenumber function
* @endinternal
*
*@brief  This function renumbers internal database to new device number.
         Must be called after cpssDxChCfgReNumberDevNum is used.
*
* @param[in] oldDevNum                 - old device number.
* @param[in] newDevNum                 - new device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS extDrvMapDmaRenumber
(
    IN GT_U8 oldDevNum,
    IN GT_U8 newDevNum
)
{
    PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[newDevNum]) = PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[oldDevNum]);
    PRV_SHARED_DEVICE_INFO_DB[newDevNum] = PRV_SHARED_DEVICE_INFO_DB[oldDevNum];

    return GT_OK;
}
