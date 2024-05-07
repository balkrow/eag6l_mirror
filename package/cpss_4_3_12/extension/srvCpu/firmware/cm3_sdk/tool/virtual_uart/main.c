/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
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
* @file main.c
*
* @brief virtual uart application enable to support CLI without physical UART.
*        The CLI can be utilized for communicating with the FW.
*
* @version   1.0.2
********************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

#include "mvShmUart.h"
#include "mvShmUartSmi.h"
#include "slave_smi.h"
#include "common.h"

/* SHM_UART_BASE & SHM_UART_SIZE are passed from Makefile */

extern MV_VOID mvShmUartSmiInit(MV_U32 base, MV_U32 size, MV_U32 smiPhyAdd, MV_U32 smiPort);
extern MV_VOID mvShmUartSmiPutc(MV_U8 c );
extern MV_U8 mvShmUartSmiGetc();

/***********************************************************************
 * CM3 processor tool:
 *
 *   CM3 SRAM is located at offset 0x80000 of the PP address space.
 *   PP register 0x500 controls CM3:
 *      bit [28] - CM3_Enable
 *      bit [20] - init RAM
 *      bit [19] - CM3 clock disable
 **********************************************************************/

#define COMP_REG(_x) (0x120+(_x)*4)
#define BITS19_31(a) (a & 0xfff80000)
#define BITS0_18(a) (a & 0x7ffff)
#define COMP_REG_OFF(reg) (reg<<19)
#define COMP_REG_CTRL_REG 0x140

/* to simplify code Domain, bus,dev,func*/
#define DBDF pciDomain, pciBus, pciDev, pciFunc
#define DBDF_DECL        \
    uint32_t  pciDomain, \
    uint32_t  pciBus,    \
    uint32_t  pciDev,    \
    uint32_t  pciFunc

#define NUM_OF_TILES                   4
#define TILE_OFFSET_OFFSET             0x20000000

#define NUM_OF_RAVENS_PER_TILE         4
#define RAVEN_MEMORY_SPACE_SIZE        0x01000000

#define DEFAULT_CM3_OFFSET              0x80000
#define DEFAULT_CM3_SIZE                0x20000
#define CETUS_CM3_OFFSET                0x40000
#define CETUS_CM3_SIZE                  0x1F800
#define MG_MEMORY_SPACE_SIZE            0x00100000
#define NUM_OF_CM3_IN_HARRIER           2
#define NUM_OF_CM3_IN_AC5_AC5X          3
#define NUM_OF_CM3_IN_AC5P              4
#define AC5_MG_BASE_ADDRESS             0x7F900000
#define FALCON_TILE0_MG_BASE_ADDRESS    0x1D000000
#define FALCON_TILE2_MG_BASE_ADDRESS    0x5D000000
#define FALCON_MG_MEMORY_SIZE           0x00100000
#define NUM_OF_CM3_IN_FALCON_DUAL_TILE  2
#define AC5P_HARRIER_MG_BASE_ADDRESS    0x3C200000
#define AC5P_SSMI_XBAR_PORT_REMAP       0x3D013100
#define AC5_AC5X_SSMI_XBAR_PORT_REMAP   0x7F013100
#define SMI_DEFAULT_PHY_ADDR            0x6

enum device_type {
    DEVICE_TYPE_ALDRIN_E,
    DEVICE_TYPE_PIPE_E,
    DEVICE_TYPE_BC3_E,
    DEVICE_TYPE_CETUS_E,
    DEVICE_TYPE_ALDRIN2_E,
    DEVICE_TYPE_FALCON_E,
    DEVICE_TYPE_AC5_EXTERNAL_E,
    DEVICE_TYPE_AC5_INTERNAL_E,
    DEVICE_TYPE_AC5P_E,
    DEVICE_TYPE_HARRIER_E,
    DEVICE_TYPE_WM_SIM
};

#define VUART_VER     "1.0.2"

int terminal_init(void);

#ifdef WATCHDOG
volatile MV_U32  *wdExpired; /* Predefined: 1 word. Located at the end of the shared uart */
                              /* the WatchDog expired located after the RX and TX chains
                                 when receiving WatchDog interrupts, this pointer will gets TRUE.
                                 The VUART tool always checks (in vuart_rx_function()) if a WD interrupt has occurred,
                                 and if so it initializes the VUART, to align the markers and continue working with VUART. */
MV_32 smiWdExpired;
#endif
unsigned long long cm3_shm_buffs_phys = 0;

enum device_type dev_type;
void *pp_space_ptr2;
int fd;

void *pp_space_ptr = NULL;
void *dfx_space_ptr = NULL;
void *mg_space_ptr = NULL;
int atuNum;
bool smiFlag = 0;
int smiPortId = 0, smiPhyAddr = SMI_DEFAULT_PHY_ADDR;

static void pp_write_reg(int32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(pp_space_ptr + offset) = value;
}

static uint32_t pp_read_reg(int32_t offset)
{
    return *(volatile uint32_t *)(pp_space_ptr + offset);
}

int sysfs_pci_open(
    DBDF_DECL,
    const char *name,
    int     flags,
    int     *fd
)
{
    char fname[128];
    if (pciBus > 255 || pciDev > 31 || pciFunc > 7)
        return -1;
    sprintf(fname, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/%s",
            DBDF, name);
    *fd = open(fname, flags);
    if (*fd < 0) {
        perror(fname);
        return -1;
    }
    return 0;
}

int sysfs_pci_map(
    DBDF_DECL,
    const char *res_name,
    int flags,
    int   *fd,
    void **vaddr
)
{
    int rc;
    struct stat st;

    rc = sysfs_pci_open(DBDF, res_name, flags, fd);
    if (rc != 0) {
        perror(res_name);
        return rc;
    }

    if (fstat(*fd, &st) < 0) {
        close(*fd);
        return -1;
    }

    *vaddr = mmap(NULL,
                st.st_size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                *fd,
                (off_t)0);
    if (MAP_FAILED == *vaddr) {
        perror("mmap");
        close(*fd);
        return -1;
    }

    /* printf("%s mapped to %p, size=0x%x\n", res_name, *vaddr, (unsigned)st.st_size); */

    return 0;
}

void sysfs_pci_write(void* vaddr, int offset, uint32_t value)
{
    volatile uint32_t *addr;

    addr = (uint32_t *)((uintptr_t)vaddr + offset);
    *addr = (uint32_t)value;
}

uint32_t sysfs_pci_read(void* vaddr, int offset)
{
    volatile uint32_t *addr;

    addr = (uint32_t *)((uintptr_t)vaddr + offset);
    return *addr;
}

long getRavenBaseAddr(long ravenId)
{
    long base;

    base = (ravenId/NUM_OF_RAVENS_PER_TILE) * TILE_OFFSET_OFFSET;

    if( (ravenId % 8) < 4 )
    {
    base += (ravenId % NUM_OF_RAVENS_PER_TILE) * RAVEN_MEMORY_SPACE_SIZE;
    } else {
        base += (NUM_OF_RAVENS_PER_TILE - (ravenId % NUM_OF_RAVENS_PER_TILE) - 1) * RAVEN_MEMORY_SPACE_SIZE;
    }

    return base;
}

static void *vuart_rx_function(void *sram_add)
{
    int c;
    while (1)
    {
        usleep(10000);   /* sleep for 10ms */
#ifdef WATCHDOG
        if (*wdExpired == TRUE) {
            *wdExpired = FALSE;
            mvShmUartInit((MV_U32*)(sram_add + SHM_UART_BASE), SHM_UART_SIZE);
        }
#endif

        while ((c=mvShmUartGetc(0)))
            putchar(c);
    }

    return NULL;
}

static void SMI_CHK_RC(MV_STATUS rc)
{
    if (rc != MV_OK)
    { 
        slaveSmiDestroy(); 
        exit(1);
    }
}

static void *vuart_smi_rx_function(void* sram_add)
{
    int c;
    uint32_t ret_val = 0, value = TRUE, addr;
    uint64_t addr64 =  (uint64_t)sram_add;
    addr = (uint32_t)addr64;

    while (1)
    {
        usleep(10000);   /* sleep for 10ms */
#ifdef WATCHDOG
        SMI_CHK_RC( slaveSmiRead(0 /* BusId */, smiPhyAddr, smiPortId, smiWdExpired, &ret_val));
        if (ret_val == TRUE) {
            SMI_CHK_RC( slaveSmiWrite(0 /* BusId */, smiPhyAddr, smiPortId, smiWdExpired, &value));
            mvShmUartSmiInit(addr + SHM_UART_BASE, SHM_UART_SIZE, smiPhyAddr, smiPortId);
        }
#endif

        while ((c=mvShmUartSmiGetc()))
            putchar(c);
    }

    return NULL;
}

static void print_usage(char *name)
{
    printf("Usage:\n");
    printf("    %s  [DBDF]/[SMI] [PHY <phy address>] [ATU <atu window>] <dev> <cm3>\n", name);
    printf(" DBDF  - Domain Bus Device Function of PCIe device in DDDD:BB:DD.F, default 0000:01:00.0\n");
    printf(" SMI   - Interface configuration through SSMI\n");
    printf(" PHY   - Only in SMI, phy address, range [0..0x1f], default phy addr = 0x6\n");
    printf(" ATU   - add 1MB times number of window, if CPSS running in parallel, must use ATU > 3, default ATU = 0\n");
    printf(" dev   - device type - ac3, ac5ex, ac5in, ac5xex, ac5xin, ac5p, harrier, aldrin, falcon, imlex, imlin, imm\n");
    printf(" scpu  - Optional scpu instance id, range [0..19] , depends on device type\n");
    printf("         For Falcon, 0-15 for Ravens, 16-19 for tiles CM3\n");

}

int main(int argc, char* argv[])
{
    pthread_t rx_thread;
    int fd, i, mg_fd;
    FILE *f;
    int c, rc;
    unsigned int pciDomain, pciBus, pciDev, pciFunc;
    unsigned long long start, end, flags, res2, res4;
    uint32_t scpuId = 0, shift = 0;
    uint32_t reg, mgBase;
    void *scpuSramPtr = NULL;
    uint32_t smiScpuSramPtr = 0;
    uint32_t cm3_offset = DEFAULT_CM3_OFFSET;
    char fname[128];
    uint32_t ssmiXbarPortRemapAddr;

    /*
     * Check input arguments:
     * We expect, optionally, DBDF
     * Max is name + DBDF/SMI + ATU/PHY + <arg ATU/PHY> + longest command (2) = 6
     * Min is name + shortest command (1) = 2
     */
    if (argc > 6 || argc < 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* scan DBDF, expect format DDDD:BB:DD.F */
    rc = sscanf(argv[1], "%04x:%02x:%02x.%01x%n", &pciDomain, &pciBus, &pciDev, &pciFunc, &i);
    if (rc == 4 && i == 12) /* 4 numbers read, total length is 12 characters */
        shift++;
    else
    {
        if (!strcmp(argv[1], "SMI"))
        {
            smiFlag = 1;
            shift++;
            /* scan Phy address */
            if (!strcmp(argv[2], "PHY"))
            {
                smiPhyAddr = strtoul(argv[3], NULL, 0);
                shift += 2;
            }
        }
        else /* Default 0000:01:00.0 */
        {
            pciBus = 1;
            pciDomain = pciDev = pciFunc = 0;
        }
    }

    /* scan ATU NUM */
     if (!strcmp(argv[1 + shift], "ATU"))
    {
        atuNum = strtoul(argv[2 + shift], NULL, 0);
        shift += 2;
    }
    else
        atuNum  = 0;/* by default, used with ATU #0, If CPSS driver running in parallel, must use ATU > 3 */

    if (atuNum)
        printf ("ATU: 0x%x\n", atuNum);

    if (!smiFlag)
        printf ("DBDF: %04x:%02x:%02x.%01x\n", pciDomain, pciBus, pciDev, pciFunc);

    /* Shift argv[] back if DBDF present, so command starts at argv[1] */
    if (shift > 0)
    {
        argc -= shift;
        for (i = 1; i < argc; i++)
            argv[i] = argv[i + shift];
    }

    if (argc > 2)
        scpuId = strtoul(argv[2], NULL, 0);
    printf("SCPU Id: %d\n", scpuId);

    if (!strcmp(argv[1], "aldrin"))
    {
        dev_type = DEVICE_TYPE_ALDRIN_E;
    }
    else if ((!strcmp(argv[1], "ac5ex")) || (!strcmp(argv[1], "ac5xex")) || !strcmp(argv[1], "imlex"))
    {
        dev_type = DEVICE_TYPE_AC5_EXTERNAL_E;
        mgBase = AC5_MG_BASE_ADDRESS;
        mgBase += (scpuId % NUM_OF_CM3_IN_AC5_AC5X) * MG_MEMORY_SPACE_SIZE;
    }
    else if ((!strcmp(argv[1], "ac5in")) || (!strcmp(argv[1], "ac5xin")) || (!strcmp(argv[1], "imlin")) || !strcmp(argv[1], "imm"))
    {
        dev_type = DEVICE_TYPE_AC5_INTERNAL_E;
        mgBase = AC5_MG_BASE_ADDRESS;
        mgBase += (scpuId % NUM_OF_CM3_IN_AC5_AC5X) * MG_MEMORY_SPACE_SIZE;
    }
    else if (!strcmp(argv[1], "ac5p"))
    {
        dev_type = DEVICE_TYPE_AC5P_E;
        mgBase = AC5P_HARRIER_MG_BASE_ADDRESS;
        mgBase += (scpuId % NUM_OF_CM3_IN_AC5P) * MG_MEMORY_SPACE_SIZE;
    }
    else if (!strcmp(argv[1], "harrier"))
    {
        dev_type = DEVICE_TYPE_HARRIER_E;
        mgBase = AC5P_HARRIER_MG_BASE_ADDRESS;
        mgBase += (scpuId % NUM_OF_CM3_IN_HARRIER) * MG_MEMORY_SPACE_SIZE;
    }
    else if (!strcmp(argv[1], "cetus"))
    {
        dev_type = DEVICE_TYPE_CETUS_E;
        cm3_offset = CETUS_CM3_OFFSET;
    }
    else if (!strcmp(argv[1], "falcon"))
    {
        if (scpuId < 16) {
            mgBase = (getRavenBaseAddr(scpuId) + 0x00300000); /* scpuId parameter is actual used as raven id */
            printf("raven %d mgBase 0x%08x\n", scpuId, mgBase);
        } else {
            /* each tile has 4 MG units with only one MG having CM3 */
            scpuId -= 16;
            dev_type = DEVICE_TYPE_FALCON_E;
            mgBase = (scpuId > 1) ? FALCON_TILE2_MG_BASE_ADDRESS : FALCON_TILE0_MG_BASE_ADDRESS;
            /* each tile has 4 MG units where only one MG has CM3 unit */
            mgBase += ((scpuId % NUM_OF_CM3_IN_FALCON_DUAL_TILE)*4) * FALCON_MG_MEMORY_SIZE;
        }
    }
    else if (!strcmp(argv[1], "sim"))
    { /* simulation connects via Linux shared files, so
         we do not care about the specific device simulated: */
        char slname[128];
        int port_id = 14141;

        if (argc > 3)
            port_id = atoi(argv[3]);

        dev_type = DEVICE_TYPE_WM_SIM;
        if (!scpuId)
            sprintf(slname,"/tmp/UNIT_MG-%u", port_id);
        else
            sprintf(slname,"/tmp/UNIT_MG_0_%u-%u", scpuId, port_id);
        readlink(slname, fname, sizeof(fname) - 1); /* the above name is a symbolic soft link to the name derived from the process id of the WM simulation */
        mgBase = 0;
    } else
        print_usage(argv[0]);

    if (smiFlag) {
        printf("SMI Phy Addr:%x\n",smiPhyAddr);
        switch (dev_type)
        {
        /*  TODO: add AC5, AC5X Support
            case DEVICE_TYPE_AC5_EXTERNAL_E:
                ssmiXbarPortRemapAddr =  AC5_AC5X_SSMI_XBAR_PORT_REMAP;
                scpuId = (scpuId % NUM_OF_CM3_IN_AC5_AC5X);
                break;
        */
            case DEVICE_TYPE_AC5P_E:
                ssmiXbarPortRemapAddr = AC5P_SSMI_XBAR_PORT_REMAP;
                scpuId = (scpuId % NUM_OF_CM3_IN_AC5P);
                break;
            default:
                printf("VUART with SMI is not supported yet for %s\n",argv[1]);
                return -1;
        }

        if (MV_OK != slaveSmiInit(ssmiXbarPortRemapAddr))
        {
            printf("Failed to initialize SSMI\n");
            return -1;
        }
        smiPortId = scpuId + 4;
        /* Clear msb 3 nibbles, address will map according to port to correct MG */
        smiScpuSramPtr = (mgBase + DEFAULT_CM3_OFFSET) & 0x000fffff;
    }
    else /* !smiFlag */
    {
        if ( dev_type == DEVICE_TYPE_WM_SIM)
        { /* connect to the SRAM by memory mapping the shared file: */
            if ((fd = shm_open(fname, O_RDWR | O_SYNC, S_IRUSR | S_IWUSR) ) < 0)
            {
                printf("Could not open file %s!\n", fname);
                return -1;
            }

            scpuSramPtr = mmap(NULL, 0x60000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (scpuSramPtr == MAP_FAILED)
            {
                printf("Could not mmap\n");
                return -1;
            }

        }
        else if ( dev_type == DEVICE_TYPE_AC5_INTERNAL_E ) /* Internal CPU - using virtual address  */
        {
            if ((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
                return -1;
            pp_space_ptr = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mgBase);
            if (pp_space_ptr == MAP_FAILED)
            {
                printf("Could not mmap\n");
                return -1;
            }
        }
        else /* External CPU - using PCI*/
        {
            rc = sysfs_pci_open(DBDF, "resource", O_RDONLY, &fd);
            if (rc != 0)
                return rc;


            f = fdopen(fd, "r");
            if (f == NULL)
                return -1;
            i = 0;
            while (!feof(f))
            {
                if (fscanf(f, "%lli %lli %lli", &start, &end, &flags) != 3)
                    break;
                if (i == 2)
                    res2 = start;
                if (i == 4)
                    res4 = start;
                i++;
            }
            fclose(f);

            /* printf("res2 (pysical PP bar 2 addr): %llx\n", res2); */

            // Config Aldrin PCIe Window 0 to see Switching-Core
            rc = sysfs_pci_map(DBDF, "resource0", O_RDWR, &mg_fd, &mg_space_ptr);
            if (rc != 0)
                return rc;

            /* Init: create mappings for PP and XBAR */
            rc = sysfs_pci_map(DBDF, "resource2", O_RDWR | O_SYNC, &mg_fd, &pp_space_ptr);
            if (rc < 0)
                return -1;
        }


        if ( dev_type == DEVICE_TYPE_WM_SIM)
        {
            printf("Running in simulation mode...\n");
        }
        else if ( dev_type < DEVICE_TYPE_FALCON_E) /* Sip5 Devices */
        {
            sysfs_pci_write(mg_space_ptr, 0x41820, 0);
            sysfs_pci_write(mg_space_ptr, 0x41824, res2);
            sysfs_pci_write(mg_space_ptr, 0x41828, 0);
            sysfs_pci_write(mg_space_ptr, 0x4182C, 0);
            sysfs_pci_write(mg_space_ptr, 0x41820, 0x03ff0031);
            sysfs_pci_write(mg_space_ptr, 0x41830, 0);
            sysfs_pci_write(mg_space_ptr, 0x41834, res4);
            sysfs_pci_write(mg_space_ptr, 0x41838, 0);
            sysfs_pci_write(mg_space_ptr, 0x4183C, 0);
            sysfs_pci_write(mg_space_ptr, 0x41830, 0x001f0083);

            /* Init: create mappings for DFX */
            rc = sysfs_pci_map(DBDF, "resource4", O_RDWR | O_SYNC, &mg_fd, &dfx_space_ptr);
            if (rc < 0)
                return -1;

            /* Completion register */
            pp_write_reg(COMP_REG(7), htole32(BITS19_31(cm3_offset)>>19) );
            scpuSramPtr = pp_space_ptr + COMP_REG_OFF(7) + BITS0_18(cm3_offset)/* actually 0 */;
        }
        else  /* Sip6 Devices */
        {
            if( dev_type != DEVICE_TYPE_AC5_INTERNAL_E ) /* External CPU*/
            {
                /* add 1MB times number of window */
                res2 += atuNum * 0x100000;
                pp_space_ptr += atuNum * 0x100000;
                i = 0x1300 + 0x200 * atuNum;
                /* configure inbound window into the PP MG0, size 1MB, at start of res2 address space */
                /* take this window because it not used by other resources */
                *(unsigned*)(mg_space_ptr + i + 0x04) = 0x80000000; /* enable the window */
                *(unsigned*)(mg_space_ptr + i + 0x08) = (unsigned long)res2 & 0xFFFFFFFF; /* the start of the window */
                *(unsigned*)(mg_space_ptr + i + 0x0c) = 0x0;
                *(unsigned*)(mg_space_ptr + i + 0x10) = ((unsigned long)(res2 & 0xFFFFFFFF)) | 0xfffff; /* the end of the window (window size is 0xfffff) */
                *(unsigned*)(mg_space_ptr + i + 0x14) = mgBase; /* we can access mgBase address through this window */
                *(unsigned*)(mg_space_ptr + i + 0x1c) = 0x0;
            }

            /* Completion register */
            scpuSramPtr = pp_space_ptr + 0x80000;
            *(unsigned*)(pp_space_ptr + 0x124) = 0x1;
        }

        if (dev_type != DEVICE_TYPE_WM_SIM)
        {
            /* Set 8 region completion register mode */
            reg = pp_read_reg(COMP_REG_CTRL_REG);
            reg &= ~(1 << 16);
            pp_write_reg(COMP_REG_CTRL_REG, reg);
        }
    } /* !smiFlag */

    printf("SHM_UART_BASE: 0x%x, SHM_UART_SIZE %u\n", SHM_UART_BASE, SHM_UART_SIZE);

#ifdef WATCHDOG
    /* locate the watchdog expired word after the rx ant tx chains */
    /* use 2 SHM_UART_SIZE, one for RX and one for TX chain */
    if (smiFlag){
        smiWdExpired = smiScpuSramPtr + SHM_UART_BASE + 2*SHM_UART_SIZE;
        /* printf("wdExpired address: 0x%lx\n", smiWdExpired); */
    }
    else
    {
        wdExpired = scpuSramPtr + SHM_UART_BASE + 2*SHM_UART_SIZE;
        /* printf("wdExpired address: %p\n", wdExpired); */
    }
#endif
    if (terminal_init()) {
        if (errno == ENOTTY)
            fprintf(stderr, "This program requires a terminal.\n");
        else
            fprintf(stderr, "Cannot initialize terminal: %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (smiFlag)
        mvShmUartSmiInit(smiScpuSramPtr + SHM_UART_BASE, SHM_UART_SIZE, smiPhyAddr, smiPortId);
    else
        mvShmUartInit((MV_U32*)(scpuSramPtr + SHM_UART_BASE), SHM_UART_SIZE);

    /* create thread to read firmware characters if exist
       if watchdog enabled, check if WD interrupt is asserted */
    if (smiFlag)
        rc = pthread_create(&rx_thread, NULL, vuart_smi_rx_function, (void*)(unsigned long long)smiScpuSramPtr);
    else
        rc = pthread_create(&rx_thread, NULL, vuart_rx_function, scpuSramPtr);

    if (rc) {
        fprintf(stderr,"Error - pthread_create() return code: %d\n",rc);
        exit(EXIT_FAILURE);
    }
    printf("Press CTRL+shift+x or ` to quit\n");

    while ((c = getc(stdin)) != EOF)
    {
        if (c==0x18 || c=='`')
            break;
        else
        {
            if (smiFlag)
                mvShmUartSmiPutc(c);
            else
                mvShmUartPutc(0, c);
        }
    }

    fprintf(stderr, "\n");  /* Can't write yet to stdout */
    fflush(NULL);

    if(smiFlag)
        slaveSmiDestroy();
    else if( dev_type == DEVICE_TYPE_AC5_INTERNAL_E )
    {
        munmap(pp_space_ptr2, 0x100000);
        close(fd);
    }

    return EXIT_SUCCESS;
}

