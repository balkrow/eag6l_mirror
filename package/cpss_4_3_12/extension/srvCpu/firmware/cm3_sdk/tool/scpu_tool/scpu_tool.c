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
* @file scpu_tool.c
*
* @brief  scpu_tool enables loading the Firmware .bin file into the SCPU SRAM.
*         In addition, you can use the scpu_tool to read registers and write from switch device.
*         as well as for sending IPC messages.
*         This tool runs on the Linux user space of the host CPU.
*
* @version   1.0.2
********************************************************************************
*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <i2c_lib.h>
#include <mvTwsi.h>
#include <crc32.h>
#include <slave_smi.h>

extern int  send_ipc_message(char* ipcBase, char* str);
extern void receive_ipc_message(char* ipcBase, char* str);

/***********************************************************************
 * SCPU processor tool:
 *
 *   SCPU SRAM is located at offset 0x80000 of the PP address space.
 *   PP register 0x500 controls SCPU:
 *      bit [28] - SCPU_Enable
 *      bit [20] - init RAM
 *      bit [19] - SCPU clock disable
 **********************************************************************/

#define COMP_REG(_x) (0x120+(_x)*4)
#define BITS19_31(a) (a & 0xfff80000)
#define BITS0_18(a) (a & 0x7ffff)
#define COMP_REG_OFF(reg) (reg<<19)
#define COMP_REG_CTRL_REG 0x140
/* 32bit byte swap. For example 0x11223344 -> 0x44332211                    */
#define MV_BYTE_SWAP_32BIT(X) ((((X)&0xff)<<24) |                       \
                               (((X)&0xff00)<<8) |                      \
                               (((X)&0xff0000)>>8) |                    \
                               (((X)&0xff000000)>>24))

/* to simplify code Domain, bus,dev,func*/
#define DBDF pciDomain, pciBus, pciDev, pciFunc
#define DBDF_DECL        \
    uint32_t  pciDomain, \
    uint32_t  pciBus,    \
    uint32_t  pciDev,    \
    uint32_t  pciFunc

#define _1K     1024
#define _2K     2048

#define DEFAULT_SCPU_OFFSET             0x80000
#define CETUS_SCPU_OFFSET               0x40000
#define CETUS_SCPU_SIZE                 0x1F800
#define MG_MEMORY_SPACE_SIZE            0x00100000
#define NUM_OF_SCPU_IN_HARRIER          2
#define NUM_OF_SCPU_IN_AC5_AC5X         3
#define NUM_OF_SCPU_IN_AC5P             4
#define MG0_OFFSET_AC5                  0x7f900000
#define MG1_OFFSET_AC5                  0x7fA00000
#define MG2_OFFSET_AC5                  0x7fB00000
#define MG0_OFFSET_AC5P                 0x3C200000
#define MG1_OFFSET_AC5P                 0x3C300000
#define MG2_OFFSET_AC5P                 0x3C400000
#define MG3_OFFSET_AC5P                 0x3C500000
#define EXTERNAL_MG_OFFSET              0x00000000
#define FALCON_TILE0_MG_BASE_ADDRESS    0x1D000000
#define FALCON_TILE2_MG_BASE_ADDRESS    0x5D000000
#define FALCON_MG_MEMORY_SIZE           0x00100000
#define NUM_OF_SCPU_IN_FALCON_DUAL_TILE 2
#define AC5P_HARRIER_MG_BASE_ADDRESS    0x3C200000
#define AC5P_SSMI_XBAR_PORT_REMAP       0x3D013100
#define AC5_AC5X_SSMI_XBAR_PORT_REMAP   0x7F013100
#define SMI_DEFAULT_PHY_ADDR            0x6
#define HOST_TO_SCPU_DOORBELL_REG       0x534       /* Offset in MG */
#define USER_DEFINED_REGISTER           0x70
#define AC5_SCPU_SIZE                   (128*_1K)
#define AC5P_SCPU_SIZE                  (384*_1K)
#define FALCON_SCPU_SIZE                (384*_1K)
#define HARRIER_SCPU_SIZE               (256*_1K)
#define IRONMAN_SCPU_SIZE(n)            (192*_1K - n*32*_1K)

#define SIP6_WINDOW_OFFSET              (0x000FFFFF)
#define SIP6_WINDOW_MASK                (0xFFFFFFFF^SIP6_WINDOW_OFFSET)
#define CRC_MAGIC                       0x78787878

#define DOORBELL_FROM_HOST_IPC_MESSAGE  0x2

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
    DEVICE_TYPE_HARRIER_E
};

#define SCPU_TOOL_VER     "1.0.2"

void *pp_space_ptr = NULL;
void *pp2_space_ptr = NULL;
void *dfx_space_ptr = NULL;
void *mg_space_ptr = NULL;
bool verbose = 0;
bool smiFlag = 0;

uint32_t direct_0_or_through_pci_1, atuNum, smiPhyAddr = SMI_DEFAULT_PHY_ADDR;


static void pp_write_reg(int32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(pp_space_ptr + offset) = value;
}

static uint32_t pp_read_reg(int32_t offset)
{
    return *(volatile uint32_t *)(pp_space_ptr + offset);
}


static void enable_scpu(unsigned int dev_id, bool enable)
{
    uint32_t reg;

    reg = pp_read_reg(0x500);
    if (enable) {
        reg |= (1 << 28); /* Enable SCPU */
        if (0xbe00 != dev_id) {
            reg |= (1 << 29); /* Enable SCPU */
        } else {
            reg |= (1 << 19); /* bit 19: SCPU_PROC_CLK_DIS */
        }
    } else {
        reg &= ~(1 << 28); /* disable SCPU */
        if (0xbe00 != dev_id) {
            reg &= ~(1 << 29); /* disable SCPU */
        } else {
            reg |= 0x00100000; /* bit 20: init ram */
            reg &= 0xfff7ffff; /* bit 19: SCPU_PROC_CLK_DIS */
        }
    }

    pp_write_reg(0x500, reg);

    if (0xbe00 == dev_id && !enable) {
        reg = pp_read_reg(0x54);
        reg |= 1; // bit 0: SCPU_METAL_FIX_JTAG_EN
        pp_write_reg(0x54, reg);
    }
}

int sysfs_pci_open(
    DBDF_DECL,
    const char *name,
    int     flags,
    int     *fd
)
{
    char fname[128];
    FILE *f;
    if (pciBus > 255 || pciDev > 31 || pciFunc > 7)
        return -1;

    sprintf(fname, "/sys/bus/pci/devices/%04x:%02x:%02x.%x/enable", DBDF);
    f = fopen(fname, "w");
    if (f == NULL)
    {
        printf("Error enabling PCI device at %s\n", fname);
        return -1;
    }

    fputs("1",f);
    fclose(f);

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

uint32_t getPortId(enum device_type dev_type, uint32_t *offset)
{
    int port = 0xD;
    int reg = *offset;
    /* in case of address in one of the MG's, clear msb 3 nibbles,
       because address (0x000fffff) will map according to port to correct MG */
    *offset = *offset & 0x000fffff;
    if (dev_type == DEVICE_TYPE_AC5_EXTERNAL_E) {
       switch( reg & 0xFFF00000) {
            case MG0_OFFSET_AC5:
                port = 4; /*MG 0 */
            break;
            case MG1_OFFSET_AC5:
                port = 5; /*MG 1 */
            break;
            case MG2_OFFSET_AC5:
                port = 6; /*MG 2 */
            break;
            case EXTERNAL_MG_OFFSET:
                port = 9; /* External MG */
            break;
            default:
                *offset = reg;
        }
    } else if (dev_type ==  DEVICE_TYPE_AC5P_E) {
       switch( reg & 0xFFF00000) {
            case MG0_OFFSET_AC5P:
                port = 4; /*MG 0 */
                break;
            case MG1_OFFSET_AC5P:
                port = 5; /*MG 1 */
                break;
            case MG2_OFFSET_AC5P:
                port = 6; /*MG 2 */
                break;
            case MG3_OFFSET_AC5P:
                port = 7; /*MG 3 */
                break;
            case EXTERNAL_MG_OFFSET:
                port = 9; /* External MG */
                break;
            default:
                *offset = reg;
        }
    }
    return port;
}

uint32_t read_register(enum device_type dev_type, uint32_t address)
{
    uint32_t ret_val = 0, port;
    MV_STATUS rc;

    if(!smiFlag)
    {
        *(volatile uint32_t *)(mg_space_ptr + 0x1314 + atuNum * 0x200) = address & 0xfff00000;
        ret_val = *(volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff));
        if (verbose) {
            printf("PCIE read: address:0x%08x(%p) value:0x%08x\n", address, (volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff)), ret_val);
        }
    }
    else
    {
        port = getPortId( dev_type, &address);
        rc = slaveSmiRead(0 /* BusId */, smiPhyAddr, port, address, &ret_val);
        if (verbose) {
            if ( rc != MV_OK)
            {
                printf("returned with error: %x\n",rc);
                slaveSmiDestroy(); 
                exit(1);
            }
            else
                printf("SMI read: address:0x%08x value:0x%08x\n",address, ret_val);
        }
    }
    return  ret_val;
}

void write_register( enum device_type dev_type, uint32_t address, uint32_t value)
{
    uint32_t port;
    MV_STATUS rc;

    if(!smiFlag)
    {
        *(volatile uint32_t *)(mg_space_ptr + 0x1314 + atuNum * 0x200) = address & 0xfff00000;
        *(volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff)) = value;
        if (verbose) {
            printf("PCIE write: address:0x%08x(%p) value:0x%08x\n", address, (volatile uint32_t *)(pp_space_ptr + (address & 0x000fffff)), value);
        }
    }
    else
    {
        port = getPortId(dev_type, &address);
        rc = slaveSmiWrite(0 /* BusId */, smiPhyAddr, port, address, &value);
        if (verbose) {
            if ( rc != MV_OK)
            {
                printf("returned with error: %x\n",rc);
                slaveSmiDestroy(); 
                exit(1);
            }
            else
                printf("SMI write: address:0x%08x value:0x%08x\n",address, value);
        }
    }

}

static void ssmi_enable_scpu(enum device_type dev_type, uint32_t baseAddress, bool enable)
{
    uint32_t reg;

    reg = read_register(dev_type, baseAddress + 0x500);
    if (enable)
    {
        reg |= (1 << 28); /* Enable SCPU */
        reg |= (1 << 29); /* Enable SCPU */
    } else
    {
        reg &= ~(1 << 28); /* disable SCPU */
        reg &= ~(1 << 29); /* disable SCPU */
    }
    write_register( dev_type, baseAddress + 0x500, reg);
}

int eeprom_write_2_words(unsigned char i2c_addr, int offset, uint32_t data0, uint32_t data1) {
    char buffer[8];

    *(uint32_t *)(buffer) = MV_BYTE_SWAP_32BIT(data0);
    *(uint32_t *)(buffer + 4) = MV_BYTE_SWAP_32BIT(data1);
    if( 0 == direct_0_or_through_pci_1 ) {
        if (0 < i2c_transmit_with_offset(i2c_addr, 8, 0, i2c_offset_type_16_E, offset, buffer)) {
            printf("w 0x%08x 0x%08x\n", data0, data1);
            return 0;
        } else {
            printf("i2c write to 0x%02x offset 0x%08x failed\n", i2c_addr, offset);
            return -1;
        }
    }
    else /* 1 == direct_0_or_through_pci_1 */
    {
        MV_TWSI_SLAVE mv_twsi_slave;

        mv_twsi_slave.slaveAddr.address = i2c_addr << 1;
            mv_twsi_slave.slaveAddr.type = SRV_CPU_TWSI_ADDR_TYPE_ADDR7_BIT_E;
            mv_twsi_slave.validOffset = 1;
            mv_twsi_slave.offset = offset;
            mv_twsi_slave.offset_length = 2;

        if ( MV_OK == mv_twsi_master_transceive( buffer, 8, &mv_twsi_slave, MV_TWSI_TRANSMIT) ) {
            printf("w 0x%08x 0x%08x\n", data0, data1);
            return 0;
        } else { /* MV_FAIL */
            printf("i2c write to 0x%02x offset 0x%08x failed\n", i2c_addr, offset);
            return -1;
        }
    }
}

static void print_usage(char *name)
{
    printf("Usage:\n");
    printf("    %s  [DBDF]/[SMI] [PHY <phy address>] [ATU <atu window>] <command> parameters\n", name);
    printf(" DBDF - Domain Bus Device Function of PCIe device in DDDD:BB:DD.F, default 0000:01:00.0\n");
    printf(" SMI  - Interface configuration through SSMI\n");
    printf(" PHY  - Only in SMI, phy address, range [0..0x1f], default phy addr = 0x6\n");
    printf(" ATU  - add 1MB times number of window, if CPSS running in parallel, must use ATU > 3, default ATU = 0\n");
    printf(" dev  - device type - ac3, ac5ex, ac5in, ac5xex, ac5xin, ac5p, harrier, aldrin, falcon, imlex, imlin, imm\n");
    printf(" scpu - scpu instance id, range [0..19] , depends on device type\n\n");
    printf(" Commands:\n");
    printf("       %s <dev> <scpu> <file> - load FW from binary image file to SCPU SRAM and run it\n", name);
    printf("       %s <dev> <scpu> -i <string> - sends IPC message to FW\n", name);
    printf("       %s <dev> <scpu> -c - clear entire SCPU SRAM memory\n", name);
    printf("       %s <dev> -w <offset> <value> - write PP register\n", name);
    printf("       %s <dev> -r <offset> - read PP register\n", name);
    printf("       %s <dev> -e <i2c address> <file> - Load FW to EEPROM at specified I2C address\n", name);
    printf("       %s <dev> -p <i2c address> <file> - Load FW to EEPROM at specified I2C address through PCIe\n", name);
}

int main(int argc, char *argv[])
{
    char copy_buf[256], shift = 0;
    struct stat fw_stat;
    uint32_t reg;
    unsigned int pciDomain, pciBus, pciDev, pciFunc;
    int fd, fd2;
    FILE *f;
    int i, i2cOffset;
    unsigned long long start, end, flags;
    int rc;

    int fw_fd = 0, xbar_fd, mg_fd;
    uint32_t *scpu_sram_ptr = NULL,*scpu_sram_ptr_temp = NULL, *xbar_space_ptr = NULL;

    // sysfs_read_resource physical
    unsigned long long res2, res4;
    uint8_t i2c_addr;
    uint32_t dev_id, scpu_id;

    uint32_t scpu_offset = DEFAULT_SCPU_OFFSET;
    uint32_t scpu_size = AC5_SCPU_SIZE;

    void *buffer;
    uint32_t *ptr;

    uint32_t sram_base, offset, valueToWrite, ssmiXbarPortRemapAddr;
    enum device_type dev_type;
    uint32_t valid_input = 1;

    uint32_t calc_crc, image_crc;

    /*
     * Check input arguments:
     * We expect, optionally, DBDF
     * Max is name + DBDF/SMI + ATU/PHY + <arg ATU/PHY> + longest command (4) = 8
     * Min is name + shortest command (3) = 4
     */
    if (argc > 8 || argc < 4)
    {
        print_usage(argv[0]);
        return 0;
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

    /* Shift argv[] back if DBDF\SMI\ATU present, so command starts at argv[1] */
    if (shift > 0)
    {
        argc -= shift;
        for (i = 1; i < argc; i++)
            argv[i] = argv[i + shift];
    }

    /* Check inputs */
    if (argc >= 4)
    {
        scpu_id = strtoul(argv[2], NULL, 0);
        if (!strcmp(argv[1], "aldrin"))
        {
            dev_type = DEVICE_TYPE_ALDRIN_E;
        }
        else if ((!strcmp(argv[1], "ac5ex")) || (!strcmp(argv[1], "ac5xex")) || !strcmp(argv[1], "imlex"))
        {
            dev_type = DEVICE_TYPE_AC5_EXTERNAL_E;
            sram_base = MG0_OFFSET_AC5;
            sram_base += (scpu_id % NUM_OF_SCPU_IN_AC5_AC5X) * MG_MEMORY_SPACE_SIZE;
            if ( !strcmp(argv[1], "imlex"))
                scpu_size = IRONMAN_SCPU_SIZE(scpu_id);
        }
        else if ((!strcmp(argv[1], "ac5in")) || (!strcmp(argv[1], "ac5xin")) || (!strcmp(argv[1], "imlin")) || !strcmp(argv[1], "imm"))
        {
            dev_type = DEVICE_TYPE_AC5_INTERNAL_E;
            sram_base = MG0_OFFSET_AC5;
            sram_base += (scpu_id % NUM_OF_SCPU_IN_AC5_AC5X) * MG_MEMORY_SPACE_SIZE;
            if ( !strcmp(argv[1], "imlin") || !strcmp(argv[1], "imm"))
                scpu_size = IRONMAN_SCPU_SIZE(scpu_id);
        }
        else if (!strcmp(argv[1], "ac5p"))
        {
            dev_type = DEVICE_TYPE_AC5P_E;
            sram_base = AC5P_HARRIER_MG_BASE_ADDRESS;
            sram_base += (scpu_id % NUM_OF_SCPU_IN_AC5P) * MG_MEMORY_SPACE_SIZE;
            scpu_size = AC5P_SCPU_SIZE;
        }
        else if (!strcmp(argv[1], "harrier"))
        {
            dev_type = DEVICE_TYPE_HARRIER_E;
            sram_base = AC5P_HARRIER_MG_BASE_ADDRESS;
            sram_base += (scpu_id % NUM_OF_SCPU_IN_HARRIER) * MG_MEMORY_SPACE_SIZE;
            scpu_size = HARRIER_SCPU_SIZE;
        }
        else if (!strcmp(argv[1], "cetus"))
        {
            dev_type = DEVICE_TYPE_CETUS_E;
            scpu_offset = CETUS_SCPU_OFFSET;
            scpu_size = CETUS_SCPU_SIZE;
        }
        else if (!strcmp(argv[1], "falcon"))
        {
            if ((!strcmp(argv[2], "-r")) || (!strcmp(argv[2], "-w")))
                scpu_id = 16;
            if ((scpu_id < 16) || (scpu_id > 19)) {
                printf("scpu ID 16..19 (Eagles) only are currently supported\n");
                return -1;
            }
            scpu_id -= 16;
            dev_type = DEVICE_TYPE_FALCON_E;
            sram_base = (scpu_id > 1) ? FALCON_TILE2_MG_BASE_ADDRESS : FALCON_TILE0_MG_BASE_ADDRESS;
            /* each tile has 4 MG units where only one MG has SCPU unit
             */
            sram_base += ((scpu_id % NUM_OF_SCPU_IN_FALCON_DUAL_TILE)*4) * FALCON_MG_MEMORY_SIZE;
            scpu_size = FALCON_SCPU_SIZE;
        }
        else
            valid_input = 0;
    }
    else {
        valid_input = 0;
    }


    if ( !valid_input )
    {
        print_usage(argv[0]);
        return 0;
    }

    if (smiFlag) {
        printf("SMI Phy Addr:%x\n",smiPhyAddr);
        switch (dev_type)
        {
        /*  TODO: add AC5, AC5X Support
            case DEVICE_TYPE_AC5_EXTERNAL_E:
                ssmiXbarPortRemapAddr =  AC5_AC5X_SSMI_XBAR_PORT_REMAP;
                break;
        */
            case DEVICE_TYPE_AC5P_E:
                ssmiXbarPortRemapAddr = AC5P_SSMI_XBAR_PORT_REMAP;
                break;
            default:
                printf("scpu_tool SMI is not supported yet for %s\n",argv[1]);
                return -1;
        }

        if (MV_OK != slaveSmiInit( ssmiXbarPortRemapAddr))
        {
            printf("Failed to initialize SSMI\n");
            return MV_FAIL;
        }

       /* TODO: merge commands supported by SMI into the general commands,
          by moving to function to each command, according to SMI/PCIe    */
        if (!strcmp(argv[2], "-w")) {
            offset = strtoul(argv[3], NULL, 0);
            valueToWrite = strtoul(argv[4], NULL, 0);
            write_register(dev_type, offset, valueToWrite);
            printf("0x%08x: 0x%08x\n", offset, read_register(dev_type, offset));
            slaveSmiDestroy();
            return MV_OK;
        }
        if (!strcmp(argv[2], "-r")) {
            offset = strtoul(argv[3], NULL, 0);
            printf("0x%08x: 0x%08x\n", offset, read_register(dev_type, offset));
            slaveSmiDestroy();
            return MV_OK;
        }
        if (!strcmp(argv[3], "-c")) {
            for(i = 0; i < scpu_size; i += 4)
                write_register(dev_type, sram_base + 0x80000 + i, 0x0);

            printf("Successfully cleared SCPU SRAM through SSMI.\n");
            slaveSmiDestroy();
            return MV_OK;
        }
        /* File load command */
        fw_fd = open(argv[3], O_RDONLY);
        if (fw_fd <= 0) {
            fprintf(stderr, "Cannot open %s file.\n", argv[3]);
            return -1;
        }

        ssmi_enable_scpu(dev_type, sram_base, 0); /* Disable SCPU */

        fstat(fw_fd, &fw_stat);
        if (fw_stat.st_size > scpu_size) {
            printf("Error - file too large (%ld), we have only %dKB space\n", fw_stat.st_size, scpu_size/1024);
            close(fw_fd);
            return -1;
        }

        buffer = malloc(fw_stat.st_size);
        if (buffer == NULL)
            return -1;

        ptr = buffer;
        read(fw_fd, buffer, fw_stat.st_size);

        for (i = 0; (void*)ptr < (buffer + fw_stat.st_size); i +=4 )
        {
            write_register(dev_type, sram_base + 0x80000 + i, *ptr);
            ptr++;
        }

        free(buffer);
        close(fw_fd);

        printf("successfully loaded file %s, size %ld\n", argv[3], fw_stat.st_size);

        ssmi_enable_scpu(dev_type, sram_base, 1); /* Enable SCPU */
        return MV_OK;
    }

    if ( dev_type == DEVICE_TYPE_AC5_INTERNAL_E ) /* Internal CPU - using virtual address  */
    {
        if ((fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
            return -1;
        pp_space_ptr = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, sram_base);
        if (pp_space_ptr == MAP_FAILED)
        {
            printf("Could not mmap\n");
            return -1;
        }
    }
    else /* External CPU - using PCI*/
    {
        /*
        * Map switch device over the PCIe to access its registers
        * BAR 0 provides access to ATUs configuration.
        * BAR 2 provides access to switch registers through ATUs
        * We set ATU size to 1MB each. By default, BAR 2 size is 4MB
        * CPSS driver uses 4 ATUs, therefore consuming all 4MB
        *
        * If running in parallel to CPSS driver,
        *  BAR 2 must be set to 8MB and ATU > 3 must be used
        * BAR 2 size should be set before Linux starts,
        * preferably by u-boot cli commands
        */
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

        // Config Aldrin PeX Window 0 to see Switching-Core
        rc = sysfs_pci_map(DBDF, "resource0", O_RDWR, &mg_fd, &mg_space_ptr);
        if (rc != 0)
            return rc;

        /* Init: create mappings for PP and XBAR */
        rc = sysfs_pci_map(DBDF, "resource2", O_RDWR | O_SYNC, &mg_fd, &pp_space_ptr);
        if (rc < 0)
            return -1;
    }

    if ( dev_type < DEVICE_TYPE_FALCON_E) /* Sip5 Devices */
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
        pp_write_reg(COMP_REG(7), htole32(BITS19_31(scpu_offset)>>19) );
        scpu_sram_ptr = pp_space_ptr + COMP_REG_OFF(7) + BITS0_18(scpu_offset)/* actually 0 */;
        dev_id = (pp_read_reg(0x4c) >> 4) & 0xffff;
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
            *(unsigned*)(mg_space_ptr + i + 0x10) = ((unsigned long)(res2 & 0xFFFFFFFF)) | SIP6_WINDOW_OFFSET; /* the end of the window (window size is 0xfffff) */
            *(unsigned*)(mg_space_ptr + i + 0x14) = sram_base; /* we can access sram_base address through this window */
            *(unsigned*)(mg_space_ptr + i + 0x1c) = 0x0;
        }

        scpu_sram_ptr =(uint32_t*)(pp_space_ptr);
        scpu_sram_ptr += 0x80000/sizeof(uint32_t);

        dev_id = (pp_read_reg(0x4c) >> 4) & 0xffff;
        /* Completion register */
        *(unsigned*)(pp_space_ptr + 0x124) = 0x1;
    }

    /* Set 8 region completion register mode */
    reg = pp_read_reg(COMP_REG_CTRL_REG);
    reg &= ~(1 << 16);
    pp_write_reg(COMP_REG_CTRL_REG, reg);

    /* PP write command */
    if (!strcmp(argv[2], "-w")) {
        if ( dev_type < DEVICE_TYPE_FALCON_E) /* Sip5 Devices */
        {
            printf("scpu_tool -w is not supported in %s\n",argv[1]);
            return 0;
        }
        else if ( dev_type == DEVICE_TYPE_AC5_INTERNAL_E ) /* Internal CPU - using virtual address  */
        {
            if ((fd2 = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
                return -1;
            pp2_space_ptr = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE , MAP_SHARED, fd2, strtoul(argv[3], NULL, 0) & 0xfffff000);
            if (pp_space_ptr == MAP_FAILED)
            {
                printf("Could not mmap\n");
                return -1;
            }
            *(volatile uint32_t *)(pp2_space_ptr + (strtoul(argv[3], NULL, 0) & 0x00000fff)) = strtoul(argv[4], NULL, 0);
            printf("0x%08x: 0x%08x\n", strtoul(argv[3], NULL, 0),
                               *(volatile uint32_t *)(pp2_space_ptr + (strtoul(argv[3], NULL, 0) & 0x00000fff)));
            if ( munmap(pp2_space_ptr, 0x100000) == -1) {
              perror("munmap failed with error:");
            }
            close(fd2);
            return 0;
        }
        else /* Sip6 Devices - External CPU - using PCI*/
        {
            *(unsigned*)(mg_space_ptr + 0x1314 + atuNum * 0x200) = strtoul(argv[3], NULL, 0) & SIP6_WINDOW_MASK;
            *(volatile uint32_t *)(pp_space_ptr + ((strtoul(argv[3], NULL, 0) & SIP6_WINDOW_OFFSET))) = strtoul(argv[4], NULL, 0);
            printf("0x%08x\n", *(volatile uint32_t *)(pp_space_ptr + ((strtoul(argv[3], NULL, 0) & SIP6_WINDOW_OFFSET))));
            close(mg_fd);
            close(fd);
            return 0;
        }
    }

    /* PP read command */
    if (!strcmp(argv[2], "-r")) {
        if ( dev_type < DEVICE_TYPE_FALCON_E) /* Sip5 Devices */
        {
            printf("scpu_tool -r is not supported in %s\n",argv[1]);
            return 0;
        }
        else if ( dev_type == DEVICE_TYPE_AC5_INTERNAL_E ) /* Internal CPU - using virtual address  */
        {
            if ((fd2 = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
                return -1;
            pp2_space_ptr = mmap(NULL, 0x100000, PROT_READ | PROT_WRITE , MAP_SHARED, fd2, strtoul(argv[3], NULL, 0) & 0xfffff000);
            if (pp_space_ptr == MAP_FAILED)
            {
                printf("Could not mmap\n");
                return -1;
            }
            printf("0x%08x: 0x%08x\n", strtoul(argv[3], NULL, 0),
                               *(volatile uint32_t *)(pp2_space_ptr + (strtoul(argv[3], NULL, 0) & 0x00000fff)));
            if ( munmap(pp2_space_ptr, 0x100000) == -1) {
              perror("munmap failed with error:");
            }
            close(fd2);
            return 0;
        }
        else /* External CPU - using PCI*/
        {
            *(unsigned*)(mg_space_ptr + 0x1314 + atuNum * 0x200) = strtoul(argv[3], NULL, 0) & SIP6_WINDOW_MASK;
            printf("0x%08x: 0x%08x\n", strtoul(argv[3], NULL, 0),
                                *(volatile uint32_t *)(pp_space_ptr + ((strtoul(argv[3], NULL, 0) & SIP6_WINDOW_OFFSET))));
            close(mg_fd);
            close(fd);
            return 0;
        }
    }

    if (!strcmp(argv[3], "-i")) {
        if (!send_ipc_message((char*)scpu_sram_ptr + scpu_size - _2K, argv[4]))
        {
            pp_write_reg(HOST_TO_SCPU_DOORBELL_REG, DOORBELL_FROM_HOST_IPC_MESSAGE); /* Send Interrupt to SCPU */
            receive_ipc_message((char*)scpu_sram_ptr + scpu_size - _2K, argv[4]);
        }
        return 0;
    }

    /* EEPROM program command */
    if (!strcmp(argv[2], "-e") || !strcmp(argv[2], "-p")) {

        if (!strcmp(argv[2], "-e")) {
            direct_0_or_through_pci_1 = 0;
        } else { /* (!strcmp(argv[2], "-p")) */
            printf("Don't forget to disable EEPROM\n");
            direct_0_or_through_pci_1 = 1;
        }

        if (argc < 5) {
            if (0 == direct_0_or_through_pci_1) {
                printf("Usage: %s <dev> -e <i2c address> <file> - Load FW to EEPROM at specified I2C address\n", argv[0]);
            } else { /* (1 == direct_0_or_through_pci_1) */
                printf("Usage: %s <dev> -p <i2c address> <file> - Load FW to EEPROM at specified I2C address through PCIe\n", argv[0]);
            }
            return -1;
        }

        i2c_addr = strtoul(argv[3], NULL, 0);

        fw_fd = open(argv[4], O_RDONLY);
        if (fw_fd <= 0) {
            fprintf(stderr, "Cannot open %s file.\n", argv[4]);
            return -1;
        }

        if (0 == direct_0_or_through_pci_1) {
            i2c_init();
            if (0 < i2c_receive_with_offset (i2c_addr, 0x20, 0, i2c_offset_type_16_E, 0, copy_buf)) {
                printf("i2c read from %x offset 0: 0x%08x\n", i2c_addr, *(uint32_t *)copy_buf);
            } else {
                printf("i2c failed\n");
            }
        } else { /* (1 == direct_0_or_through_pci_1) */
            uint32_t mv_rc;
            mv_rc = mv_twsi_init(100000, 250000000 /*250MHZ*/);
            if ( 90000 < mv_rc ) {
                printf("mv_twsi_init passed, actual freq %d\n", mv_rc);
            } else {
                printf("mv_twsi_init failed, actual freq %d\n", mv_rc);
                return -1;
            }
        }

        i = 0x30080002;
        i2cOffset= 0;
        eeprom_write_2_words(i2c_addr, i2cOffset, 0x8001320c, 0x00010000);
        while (true) {
            rc = read(fw_fd, &reg, 4);
            offset += 8;
            if (rc < 4) {  /* Handle last bytes of file */
                while (rc < 4)
                    *(uint8_t *)(&reg + rc++) = 0;
                rc = 0;
            }
            eeprom_write_2_words(i2c_addr, i2cOffset, i, reg);
            if (rc < 4)
                break;
            i += 4;
        }
        eeprom_write_2_words(i2c_addr, i2cOffset + 8,  0x30000502, 0x300e1a80); /* Take SCPU out of reset */
        eeprom_write_2_words(i2c_addr, i2cOffset + 16, 0xffffffff, 0xffffffff); /* Mark end of data */

        return 0;
    }

    /* Clear SCPU SRAM memory */
    for(scpu_sram_ptr_temp = scpu_sram_ptr; scpu_sram_ptr_temp < scpu_sram_ptr + scpu_size/sizeof(uint32_t); scpu_sram_ptr_temp++)
    {
        *scpu_sram_ptr_temp = 0;
    }
    printf("Successfully cleared SCPU SRAM.\n");

    if (!strcmp(argv[3], "-c")) {
        return 0;
    }

    /* File load command */
    fw_fd = open(argv[3], O_RDONLY);
    if (fw_fd <= 0) {
        fprintf(stderr, "Cannot open %s file.\n", argv[3]);
        return -1;
    }

    enable_scpu(dev_id, 0); /* Disable SCPU */

    fstat(fw_fd, &fw_stat);
    if (fw_stat.st_size > scpu_size) {
        printf("Error - file too large (%ld), we have only %dKB space\n", fw_stat.st_size, scpu_size/1024);
        close(fw_fd);
        return -1;
    }

    buffer = malloc(fw_stat.st_size);
    if (buffer == NULL)
        return -1;

    ptr = buffer;
    read(fw_fd, buffer, fw_stat.st_size);

    while ((void*)ptr < (buffer + fw_stat.st_size))
    {
        *scpu_sram_ptr = *ptr;
        ptr++;
        scpu_sram_ptr++;
    }

    /* crc validation */
    ptr = buffer;
    scpu_sram_ptr =(uint32_t*)(pp_space_ptr) + 0x80000/sizeof(uint32_t);
    calc_crc = mvFtlCrc32Calc((unsigned char*)ptr, fw_stat.st_size, CRC_MAGIC);
    image_crc = mvFtlCrc32Calc((unsigned char*)scpu_sram_ptr, fw_stat.st_size, CRC_MAGIC);
    if (calc_crc != image_crc) {
        printf("Image validation failed!\n");
        return -1;
    }
    printf("Firmware image integrity check is successful\n");
    if( dev_type == DEVICE_TYPE_FALCON_E ) /* to identify Tiles in Falcon, if Tile1 - write 0x1 */
         pp_write_reg(USER_DEFINED_REGISTER, ((scpu_id > 1) ? 0x1 : 0x0));

    free(buffer);
    close(fw_fd);

    printf("successfully loaded file %s, size %ld\n", argv[3], fw_stat.st_size);

    enable_scpu(dev_id, 1); /* Enable SCPU */

    if( dev_type == DEVICE_TYPE_AC5_INTERNAL_E )
    {
        if ( munmap(pp_space_ptr, 0x100000) == -1)
            perror("munmap failed with error:");
        close(fd);
    }

    return 0;
}
