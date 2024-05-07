/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/**
********************************************************************************
* @file srvCpuServices.c
*
* @brief srvCpu services implementation.
*
* @version   1
********************************************************************************
*/
#ifdef ASIC_SIMULATION
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#endif

#include <printf.h>
#include "srvCpuServices.h"
#include "prvSrvCpuServices.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include "global.h"
#include <cli.h>
#include <ic.h>
#include <srvCpuI2c.h>
#include <srvCpuIpc.h>
#include <srvCpuWatchdog.h>
#include <srvCpuGpio.h>
#include <srvCpuTimer.h>

#ifndef cm3_internal_win_conf_write
#define cm3_internal_win_conf_write( val )  \
    (*(( volatile unsigned long * )(MG_BASE | REG_MBUS_SYSTEM_INT_WINDOW_CONFIG ))) = (val)
#endif

#define CM3_BASE_ADDR_VAL(addr,attr,target)     ( (addr-EXT_MEM_BASE) | attr<<8 | target)

/* Globals */
MV_U8 mgOwnId;
MV_U8 devIndex;
static xSemaphoreHandle xRegLock = NULL;
#ifndef SHM_UART
static int uartIntr;
#endif
#ifdef TWSI_SUPPORT
static int i2cIntr;
#endif
#ifdef ASIC_SIMULATION
unsigned char *g_cnm_sram_ptr;
unsigned char *g_scpu_sram_ptr = NULL;
#endif

/* Semaphore to block Utility Task until doorbell is asserted */
static xSemaphoreHandle isrUtilityTaskSemaphore = NULL;
/* Queue containing the list of doorbell interrupt callers */
QueueHandle_t xUtilityQueue;

#define ARRAY_SIZE(a) ((sizeof a) / (sizeof a[0]))

#define CM7_AD_REG_OFS_WIN_ATTR            0x0
#define CM7_AD_REG_OFS_WIN_SIZE_LOW        0x4
#define CM7_AD_REG_OFS_WIN_BASE_REMAP_HIGH 0x8
#define CM7_AD_REG_OFS_WIN_BASE_LOW        0xC
#define CM7_AD_REG_OFS_WIN_REMAP_LOW       0x10
#define CM7_AD_REG_OFS_TO_32_IDX(ofs) (ofs/sizeof(int))

#define CM7_AHBP_BASE         0x40000000
#define AAS_AHBP_AD_BASE      0x5C717100
#define AAS_AHBP_AD_CPU_STEP  0x00001000
#define AAS_AD_ENTRY_STEP     0x00000018
#define AAS_DFX_SERVER_ADDR   0x3D000000
#define MG0_OFFSET_AC5        0x7f900000
#define MG1_OFFSET_AC5        0x7fA00000
#define MG2_OFFSET_AC5        0x7fB00000

#define MG0_OFFSET_AC5P       0x3C200000
#define MG1_OFFSET_AC5P       0x3C300000
#define MG2_OFFSET_AC5P       0x3C400000
#define MG3_OFFSET_AC5P       0x3C500000

#define EXTERNAL_MG_OFFSET    0x00000000

#define MG_MEMORY_SPACE_SIZE   0x00100000
#define MG_ID_OFFSET           4
#define USER_DEFINED_REG       0x70
#define FALCON_TILE_1_OFFSET   0x40000000
#define SCPU_READY_TO_SW_RESET 0x1

#define MPP_REG(base ,n) ((base) + 0x100 + (n / 8) * 4)

struct SRV_CPU_DEVICES_TYPES_STC devices_details[] = {
    /*Device_type                       Device_ID   cores sdmaEngine sdmaPort phaPpgBase ppgNum mppMax mppBaseAddr SYS_TCLK  baseAddress scpuSram   cnmSram  cnmRemap cnmAddr dfxBase     RUNIT    */
    { SRV_CPU_DEVICE_TYPE_ALDRIN_E,     0xc800,     1,    0,         0,       0/*NA*/,   0,     0,     0/*TBD*/,   2580,     0/*TBD*/,   0/*TBD*/,  0,       0,       0,      0,          0/*TBD*/  },
    { SRV_CPU_DEVICE_TYPE_PIPE_E,       0xc400,     1,    0,         0,       0x0f   ,   4,     0,     0/*TBD*/,   2580,     0/*TBD*/,   0/*TBD*/,  0,       0,       0,      0,          0/*TBD*/  },
    { SRV_CPU_DEVICE_TYPE_BC3_E,        0xd400,     1,    0,         0,       0/*NA*/,   0,     0,     0/*TBD*/,   2580,     0/*TBD*/,   0/*TBD*/,  0,       0,       0,      0,          0/*TBD*/  },
    { SRV_CPU_DEVICE_TYPE_ALDRIN2_E,    0xcc00,     2,    0,         0,       0/*NA*/,   0,     0,     0/*TBD*/,   2580,     0/*TBD*/,   0/*TBD*/,  0,       0,       0,      0,          0/*TBD*/  },
    { SRV_CPU_DEVICE_TYPE_RAVEN_E,      0x8800,     1,    0,         0,       0/*NA*/,   0,     0,     0/*TBD*/,   2580,     0/*TBD*/,   0/*TBD*/,  0,       0,       0,      0,          0/*TBD*/  },
    { SRV_CPU_DEVICE_TYPE_FALCON_E,     0x8400,     4,    1,         3,       0x0c,      3,     32,    0x1C10,     3125,     0x1d000000, 384,       0,       0x1,     0x1c00, 0x1be0000e, 0/*TBD*/  },
    { SRV_CPU_DEVICE_TYPE_AC5_E,        0xb400,     3,    1,         3,       0/*NA*/,   0,     64,    0x8002,     3250,     0x7f900000, 128,       192,     0x3,     0x7fe0, 0x1be0000e, 0x7F000000},
    { SRV_CPU_DEVICE_TYPE_AC5X_E,       0x9800,     3,    2,         3,       0x8c,      2,     64,    0x8002,     3250,     0x7f900000, 128,       192,     0x3,     0x7fe0, 0x1be0000e, 0x7F000000},
    { SRV_CPU_DEVICE_TYPE_AC5P_E,       0x9400,     4,    4,         3,       0x0d,      4,     48,    0x3C00,     3125,     0x3C200000, 384,       64,      0x0,     0x3c07, 0x0b80000e, 0x3D000000},
    { SRV_CPU_DEVICE_TYPE_HARRIER_E,    0x9000,     2,    2,         2,       0x08,      2,     48,    0x3C00,     3125,     0x3C200000, 256,       64,      0x0,     0x3c07, 0x0,        0x3D000000},
    { SRV_CPU_DEVICE_TYPE_HARRIER_E,    0x2100,     2,    2,         2,       0x08,      2,     48,    0x3C00,     3125,     0x3C200000, 256,       64,      0x0,     0x3c07, 0x0,        0x3D000000},
    { SRV_CPU_DEVICE_TYPE_IRONMAN_E,    0xa000,     3,    3,         3,       0/*NA*/,   0,     64,    0x8002,     3330,     0x7f900000, 192,       512,     0x7,     0x7fe0, 0x1be0000e, 0x7F000000},
    { SRV_CPU_DEVICE_TYPE_IRONMAN_E,    0xa200,     3,    3/* 2? */, 3,       0/*NA*/,   0,     64,    0x8002,     3330,     0x7f900000, 192,       512,     0x7,     0x7fe0, 0x1be0000e, 0x7F000000},
#ifdef CONFIG_AAS
    { SRV_CPU_DEVICE_TYPE_AAS_E,        0x4000,     7,    2,         1,       0/*NA*/,   0,     32,    0x7C4E,     6000,     0x5c4d0000,  32,      2048,     0x0,     0x0100, 0x3D000000, 0x5C610000}
#endif
};

#ifdef CONFIG_AAS
MV_32 srvCpuSysMapAASAddressDecoder(MV_U32 cpu_num,
                                    MV_U32 ad_idx,
                                    MV_U32 offset_low32,
                                    MV_U32 offset_high32,
                                    MV_U32 cm7_addr32,
                                    MV_U32 size);
#endif

#if 0
static void prvISR(void)
{
    /* remote int */
    if (MV_MEMIO32_READ(0xa0018114) & BIT_15 ) {
        MSS_REG_BIT_RESET(0x18114, BIT_15); // will also clear cause reg 0x3F8
        printf("Remote interrupt\n");
        return;
    }

    if (prvPpReadRegister(0x9C)) {  // // cause bit is clear-on-read. Will be cleared if was asserted
        printf("Doorbell interrupt\n");
    } else
        printf("Unknown interrupt\n");
}

static void intInit(void)
{
    if (iICRegisterHandler(NVIC_GLOBAL_IRQ_0, 0, prvISR, IRQ_ENABLE, 0) <0)
        printf( "iICRegisterHandler failed\n");
    // Remote interrupt
    // MV_MEMIO32_WRITE(0x60000034, 8); // Enable global int - not enabled by SW, as board does not load with int-line connected, so line is
    // only connected after it loads
    MV_MEMIO32_WRITE(0xa0018118, BIT_15); // unmask GPIO_15 level interrupt
    MV_MEMIO32_WRITE(0xa0018114, 0);      // clear, in order to fresh-start
    MV_MEMIO32_WRITE(0x600003fc, BIT_21); // unmask int tree: GPIO_15_8_INT

    // Doorbell interrupt
    // MG-0
    prvPpWriteRegister(0xa0, BIT_7);
    prvPpWriteRegister(0x34, BIT_18);
    // MG-1
    MV_MEMIO32_WRITE(0x600003fc, BIT_21 | BIT_1); // add bit 1
    MV_MEMIO32_WRITE(0x60000034, BIT_3);
}
#endif

#ifdef ASIC_SIMULATION
unsigned char *wm_sim_get_sram_base_addr(unsigned scpu_id, unsigned size, int *fd_ptr);

/*
 * Memory map the simulation shared SRAM file
 * into memory and return the pointer to the
 * shared memory. The name of the file is
 * derived from the triplet base name + peer pid + device id.
 * Currently only device id 0 is supported.
 * Finally, create a soft symbolic in /tmp/
 * starting with the base name and ending with
 * the port number and linking to the triplet
 * filename above. This allows the vuart utility
 * to use this symbol link to easily open the
 * shared SRAM with the user only having to pass
 * the same parameter as the fw application which
 * is constant per invocation id - instead of passing the
 * pid which changes between runs.
*/
unsigned char *wm_sim_get_sram_base_addr(unsigned scpu_id, unsigned size, int *fd_ptr)
{
    int fd;
    char base_fname[32], fname[128];
    unsigned char *scpu_sram_ptr;

    if (!scpu_id)
        sprintf(base_fname,"UNIT_MG");
    else
    {
        if (scpu_id < 0xffffffff)
            sprintf(base_fname,"UNIT_MG_0_%u", scpu_id);
        else
            sprintf(base_fname,"CnmSram");
    }
    sprintf(fname, "%s_%u_%u", base_fname, getPeerPid(), 0 /* currently only a single device id is supported */);

    if ((fd = shm_open(fname, O_RDWR | O_SYNC,
                       S_IRUSR | S_IWUSR) ) < 0)
    {
        char errmsg[128];

        sprintf(errmsg, "Could not open file %s!\n", fname);
        write(0, errmsg, strlen(errmsg));
        return NULL;
    }

    scpu_sram_ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (scpu_sram_ptr == MAP_FAILED)
    {
        char errmsg[128];

        sprintf(errmsg, "Could not mmap SRAM using file %s! size %x errno %d devtype %d!\n", fname, size, errno, devices_details[devIndex].deviceType);
        write(0, errmsg, strlen(errmsg));
        close(fd);
        return NULL;
    }
    softlink_shm(base_fname, fname); /* make a shorter soft link name, easier to vuart to generate */

    if (fd_ptr != NULL)
        *fd_ptr = fd;
    return scpu_sram_ptr;
}
#endif

/**
* @internal srvCpuGetMgBase function
* @endinternal
*
* @brief   get base address of MG unit by number, or own
*
* @param mgNum          - MG number, or 0xffffffff for own
*
* @retval MV_U32        - MG base address
*/
MV_U32 srvCpuGetMgBase(MV_U32 mgNum)
{
    if (devices_details[devIndex].deviceType < SRV_CPU_DEVICE_TYPE_FALCON_E)
        return 0xffffffff;
    return mgNum * MG_MEMORY_SPACE_SIZE + devices_details[devIndex].baseAddress;
}

#ifdef SDMA_SUPPORT
/*
 * API to get an MG base address for SDMA purpose
 */
MV_U32 srvCpuSdmaGetMgBase(MV_U32 mgNum);
MV_U32 srvCpuSdmaGetMgBase(MV_U32 mgNum)
{
    /* AC5 use External MG in order to access SDMA */
    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5_E)
        return 0;

   return srvCpuGetMgBase(mgNum);
}
#endif

/**
* @internal srvCpuScpuToMsysInterRegs function
* @endinternal
*
* @brief   set base and size to configure internal access window to RUnit
*
* @param base           - base
* @param size           - size
*
* @retval void              - none
*/
void srvCpuScpuToMsysInterRegs(unsigned long base, unsigned long size)
{
    if (size < _128K)
        size = _128K;

    if (base >= _128K)
        base = _128K - 1;

    cm3_internal_win_conf_write( base | size);
}

/**************************************************************************
 * Map window[2] into switch address space, return offset from window
 *
 * offset - absolute offset of register
 *************************************************************************/
MV_U32 remapWindow(MV_U32 offset)
{
    int port = 0xD, value;
    if (devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5_E ||
        devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5X_E ||
        devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_IRONMAN_E) {
       switch( offset & 0xFFF00000) {
            case MG0_OFFSET_AC5:
                port = 4; /*MG 0 */
            break;
            case MG1_OFFSET_AC5:
                port = 5; /*MG 1 */
            break;
            case MG2_OFFSET_AC5:
                port = 6; /*MG 2 */
            break;
            case 0x07000000:
            case 0x09000000:
                if (devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5_E)
                    port = 9; /* External MG */
                break;
            case EXTERNAL_MG_OFFSET:
                port = 9; /* External MG */
            break;
        }
    } else if (devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5P_E) {
       switch( offset & 0xFFF00000) {
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
        }
    }

    if ((devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5_E) ||
        (devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5X_E) ||
        (devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_IRONMAN_E) ||
        (devices_details[devIndex].deviceType ==  SRV_CPU_DEVICE_TYPE_AC5P_E) ) {
            value = MV_MEMIO32_READ(MG_BASE | 0x4a0);
            MV_MEMIO32_WRITE(MG_BASE | 0x4a0, (value  & 0xfffffff0) | port);
    }

    MV_MEMIO32_WRITE(MG_BASE | 0x4ac, (offset & 0xfff00000) | 0xe);
    return offset & 0xfffff;
}

static MV_BOOL isRunitOffset(MV_U32 offset)
{
    if ( devices_details[devIndex].runit == 0 )
        return MV_FALSE;

    if ((offset >= devices_details[devIndex].runit) && (offset < (devices_details[devIndex].runit + RUNIT_SIZE)))
        return MV_TRUE;

    return MV_FALSE;
}

#ifndef ASIC_SIMULATION
/**
* @internal srvCpuRegRead function
* @endinternal
*
* @brief   Read a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of register
*
* @retval MV_U32          - the value of the wanted register
*/
MV_U32 srvCpuRegRead(MV_U32 offset)
{
    MV_U32 retValue = 0;

#ifndef CONFIG_AAS
    if (MV_TRUE == isRunitOffset(offset))
        retValue = MV_MEMIO32_READ(0xa0000000 | (offset & 0x00ffffff));
    else
#endif
    {
        /*  See if can obtain the semaphore prior to critical section. If the semaphore is not
        available wait 1 ticks to see if it becomes free. */
        if( xSemaphoreTake( xRegLock, 1) == pdTRUE )
        {
#ifndef CONFIG_AAS
            retValue =  MV_MEMIO32_READ(SRV_CPU_SW_WIN_BASE | remapWindow(offset));
#else
            retValue = MV_MEMIO32_READ(offset); /* no remap windows yet for CM7 */
#endif
            /* finish accessing the critical section. Release the
            semaphore. */
            xSemaphoreGive( xRegLock );
        }
    }
    return retValue;
}

/**
* @internal srvCpuRegWrite function
* @endinternal
*
* @brief   Write a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of register
* @param value            - value to write
*
* @retval none
*/
MV_U32 srvCpuRegWrite(MV_U32 offset, MV_U32 value)
{
    MV_U32 retValue = 0;

    if (MV_TRUE == isRunitOffset(offset))
        retValue =  MV_MEMIO32_WRITE(0xa0000000 | (offset & 0x00ffffff), value);
    else
    {
        /*  See if can obtain the semaphore prior to critical section. If the semaphore is not
        available wait 1 ticks to see if it becomes free. */
        if( xSemaphoreTake( xRegLock, 1) == pdTRUE )
        {
#ifndef CONFIG_AAS
            retValue =  MV_MEMIO32_WRITE(SRV_CPU_SW_WIN_BASE | remapWindow(offset), value);
#else
            retValue =  MV_MEMIO32_WRITE(offset, value); /* no remap windows yet for CM7 */
#endif
            /* finish accessing the critical section. Release the
            semaphore. */
            xSemaphoreGive( xRegLock );
        }
    }
    return retValue;
}
#endif

#if 0
/**************************************************************************
 * Reset the device
 *************************************************************************/
static void reset(void)
{
    switch (devices_details[devIndex].deviceType) {
    case SRV_CPU_DEVICE_TYPE_HARRIER_E:
        MV_REG_BIT_RESET(SRV_CPU_DFX_WIN_BASE | SOFT_RESET_REG_HARRIER, SOFT_RESET_BIT_HARRIER);
        break;
    default:
        MV_REG_BIT_SET( SYSREG_RSTOUT, 1 << SYSBIT_RSTOUT_GLOBAL_EN );
        MV_REG_BIT_SET( SYSREG_SOFT_RESET, 1 << SYSBIT_SOFT_RESET_GLOB );
    }
}
#endif

/**
* @internal prvSrvCpuMppConfig function
* @endinternal
*
* @brief   config MPP with given number
*
* @param mppNum - MPP number
* @param value  - value to set MPP nibble
* @param count  - amount of MPPs to set, starting from mppNum
*
* @retval MV_STATUS
*/
MV_STATUS prvSrvCpuMppConfig(MV_U8 mppNum, MV_U8 value, MV_U8 count)
{
    MV_U32 temp = 0, i, m, offset = 0;

     /* Falcon contains 2 tiles, each tile has 4 MPP's registers; MPP num: 0-31 Tile0, 32-63 Tile1 */
    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_FALCON_E)
    {
        if (mppNum > 31)
        {
            offset = FALCON_TILE_1_OFFSET;
            mppNum = mppNum % 32;
        }
    }

    if (( mppNum >= devices_details[devIndex].mppMax ) ||
                 (value > 0xf)  ||
                    (mppNum + count > devices_details[devIndex].mppMax))
        return MV_FAIL;

    for ( i = 0; i < count; i++, mppNum++)
    {
        m = mppNum % 8;
        temp = srvCpuRegRead(MPP_REG(((devices_details[devIndex].mppBaseAddr << 16) + offset), mppNum));
        temp &= ~(0xf << (m * 4));
        temp |= (value << (m * 4));
        srvCpuRegWrite(MPP_REG(((devices_details[devIndex].mppBaseAddr << 16) + offset), mppNum), temp);
    }
    return MV_OK;
}

/**
* @internal prvSrvCpuMppDisplay function
* @endinternal
*
* @brief   List all MPPs and values
*
* @param none
*
* @retval none
*/
void prvSrvCpuMppDisplay()
{
    MV_32 temp = 0, i;

    printf("         76543210\n");
    for ( i = 0; i < devices_details[devIndex].mppMax; i += 8)
    {
        temp = srvCpuRegRead(MPP_REG(devices_details[devIndex].mppBaseAddr << 16 , i));
        printf("%02d-%02d: 0x%08x\n",i + 7, i, temp);
    }
    /* if deviceType == Falcon, print also Tile1 MPP's */
    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_FALCON_E)
    {
        printf("\nTile1\n         76543210\n");
        for ( i = 0; i < devices_details[devIndex].mppMax; i += 8)
        {
            temp = srvCpuRegRead(MPP_REG(((devices_details[devIndex].mppBaseAddr << 16 ) + FALCON_TILE_1_OFFSET) , i));
            printf("%02d-%02d: 0x%08x\n",i + 7, i, temp);
        }
    }
}

/**
* @internal srvCpuQspiMapInit function
* @endinternal
*
* @brief   Setup mapping windows
*          This routine sets up mapping windows for QSPI driver
*
* @param address - QSPI base address to be mapped to
*
* @retval MV_U32        - configured window
*/
MV_U32 srvCpuQspiMapInit(MV_U32 address)
{
    /* configure window to qspi */
    MV_MEMIO32_WRITE((MG_BASE | 0x4d0), 0x0010000D); /* Base addr=0xa0100000 + Target=0x10000D, accessed @ 0xa0200000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4d4), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4d8), 0x00000000); /* High base addr - this window is for general switch access */
    MV_MEMIO32_WRITE((MG_BASE | 0x4dc), (address & 0xfff00000) | 0xe); /* Low Remap address */
    return 0xa0200000;
}

/**
* @internal setDeviceParameters function
* @endinternal
*
* @brief   Set device parameters accoring to device type
*
* @retval MV_TRUE          - succeed
*         MV_FALSE         - failed
*/
MV_BOOL setDeviceParameters(void)
{
    MV_U32 deviceId;
    MV_U8  i;

#ifndef CONFIG_AAS
    mgOwnId = MV_MEMIO32_READ(MG_BASE | SRV_CPU_MG_SOURCE_ID) - MG_ID_OFFSET;
    deviceId = (MV_MEMIO32_READ(MG_BASE | 0x4c) >> 4) & 0xff00;
#else
    mgOwnId = 0;
    deviceId = (MV_MEMIO32_READ(MG_BASE | 0x28) >> 0) & 0xff00;
#endif

    /* Set struct SRV_CPU_DEVICES_TYPES_STC dev according to device ID */
    for ( i = 0; i < ARRAY_SIZE(devices_details); i++)
    {
        if ((deviceId == devices_details[i].deviceId) ||
                /* Check if device is Falcon - using different mask */
                ((deviceId & 0xfc00) == devices_details[i].deviceId))
        {
            devIndex = i;
            break;
        }
    }
    /* No device is found */
    if (i == ARRAY_SIZE(devices_details))
        return MV_FALSE;

    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_IRONMAN_E)
        devices_details[devIndex].scpuSramSize = 192 - mgOwnId*32;

    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_FALCON_E)
    {
        /* to identify Tiles in Falcon, if USER_DEFINED_REG == 0x1: Tile1; Default - Tile0 */
        if ( MV_MEMIO32_READ(MG_BASE | USER_DEFINED_REG ) == 0x1)
            devices_details[devIndex].baseAddress = 0x5d000000;
    }
    /* CNM = case 0, dfx_base = 0x1be0000e*/

    return MV_TRUE;
}

/**
* @internal srvCpuMandatoryInit function
* @endinternal
*
* @brief   Initialize SCPU features
*
* @retval void          - none
*/
void srvCpuMandatoryInit(void)
{
    int clock_divisor;

    /* Setup interrupts */
    vICInit();

    /* Initialize CM3 UART */
    clock_divisor = ((devices_details[devIndex].systclk * 100000) / 16 ) / CLIUartBaud;

#ifdef ASIC_SIMULATION
    g_scpu_sram_ptr = wm_sim_get_sram_base_addr(mgOwnId, devices_details[devIndex].scpuSramSize * _1K, NULL);
    if (!g_scpu_sram_ptr)
        exit(3);
    g_cnm_sram_ptr = wm_sim_get_sram_base_addr(0xffffffff, 0x10000, NULL);
#endif

#ifdef SHM_UART
    clock_divisor++;    /* dummy - avoid compilation error */
    mvShmUartInit((MV_U32*)(SHM_UART_BASE + (unsigned long)SCPU_SRAM_BASE), SHM_UART_SIZE);
#else
    /* Use UART0 in external CPU, UART2 in internal CPU - in order ro prevent Console conflict */
    CLIUartPort = (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5_E ||
                   devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5X_E ||
                   devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_IRONMAN_E ) ? 2 : 0;
    mvUartInit(CLIUartPort, clock_divisor, mvUartBase(CLIUartPort));
#endif
    /* Initialize the timer to be used before scheduler is up.
     * Scheduler init will reconfigure the timer anyway */
    srvCpuOsTickInit();
#ifndef DISABLE_CLI
    /* Setup CLI task */
    vCLISetup();
#endif

#if (!defined RAVEN_DEV_SUPPORT)
    srvCpuInterruptInit();
#endif

#if defined CM3_SDK || defined HARRIER_DEV_SUPPORT
    /* Initialize IPC as server. By default, each SCPU SDK FW is an IPC server */
    srvCpuIpcServerInit((void*)((unsigned long)SCPU_SRAM_BASE + (devices_details[devIndex].scpuSramSize * _1K) - _2K));
#endif

#ifdef WATCHDOG
    srvCpuWatchdogInit(1000); /* set 1000 milliseconds (= 1 second) to Watchdog timer expiration */
    srvCpuWatchdogStart();
    /*Create the WD task */
    watchdogCreateTask(50); /* set 50 milliseconds delay in the task, in order to clear Watchdog timer before timer expired */
#endif

#ifdef CM3_SDK
    srvCpuGpioInit();
    /* Create Task, which will function various type of tasks, thats gets doorbell ISR to perform:
    *  IPC: respond the IPC message receive
    *  SW Reset: prepare interfaces to soft reset */
    vSemaphoreCreateBinary(isrUtilityTaskSemaphore);
    xSemaphoreTake( isrUtilityTaskSemaphore, 1); /* take semaphore and release when ISR is arrived */
    /* Create a queue capable of containing 5 unsigned long values. */
    xUtilityQueue = xQueueCreate( 5, sizeof( unsigned long) );
    xTaskCreate( prvUtilityTask, ( const char * const ) "UTILITIES",
            configMINIMAL_STACK_SIZE, NULL,
            tskIDLE_PRIORITY + 2, NULL );
#ifdef AAS_EMULATOR /* until next CVoE emulator release after CC1 which should have UART interrupt connected, poll for UART RX: */
/*    xTaskCreate( prvPollCliTask, ( const char * const ) "Poll_CLI",
            configMINIMAL_STACK_SIZE, NULL,
            tskIDLE_PRIORITY + 5, NULL ); */
#endif

#endif
}

#ifdef CONFIG_AAS
/**
* @internal srvCpuSysMapAASAddressDecoder
* @endinternal
*
* @brief    Setup mapping of address decoder on the AHBP (ARM Advanced High Performance Peripheral bus)
*
* @param cpu_num          - CM7 cpu number 0..6
* @param ad_idx           - Address decoder entry index 0..15
*                           Entries 1,2 are used to access CnM registers,
*                           initialized by HW. Should not be overwritten
*                           in normal operation or else access to CnM
*                           registers will be lost!
* @param offset_low32     - absolute lower 32-bit offset of register as accessed from external (PCI-e)
* @param offset_high32    - absolute upper 32-bit offset of register as accessed from external (PCI-e)
* @param cm7_addr32       - address in CM7 address space
* @param size             - size to remap
*
* @retval MV_32           - zero if successful, negative on error
*
*/
MV_32 srvCpuSysMapAASAddressDecoder(MV_U32 cpu_num,
                                    MV_U32 ad_idx,
                                    MV_U32 offset_low32,
                                    MV_U32 offset_high32,
                                    MV_U32 cm7_addr32,
                                    MV_U32 size)
{
 MV_U32 *ad_base = (MV_U32 *)(unsigned long)(AAS_AHBP_AD_BASE + (cpu_num * AAS_AHBP_AD_CPU_STEP) + (ad_idx * AAS_AD_ENTRY_STEP));

 if (size < 0x10000)
     size = 0x10000;

 ad_base[CM7_AD_REG_OFS_TO_32_IDX(CM7_AD_REG_OFS_WIN_ATTR)] = 0x0; /* Disable window before setting it. If you target entry 1 or 2, and you run from the matching tile, you will fault on the next write: */
 ad_base[CM7_AD_REG_OFS_TO_32_IDX(CM7_AD_REG_OFS_WIN_SIZE_LOW)] = (size >> 16) - 1; /* Window size low bits [47:16] */
 ad_base[CM7_AD_REG_OFS_TO_32_IDX(CM7_AD_REG_OFS_WIN_BASE_REMAP_HIGH)] = offset_high32 & 0xffff0000;
 ad_base[CM7_AD_REG_OFS_TO_32_IDX(CM7_AD_REG_OFS_WIN_BASE_LOW)] = cm7_addr32 >> 16;
 ad_base[CM7_AD_REG_OFS_TO_32_IDX(CM7_AD_REG_OFS_WIN_REMAP_LOW)] = (offset_low32 >> 16) | (offset_high32 << 16);
 /* ad_base[5] = Leave at defaults */
 ad_base[CM7_AD_REG_OFS_TO_32_IDX(CM7_AD_REG_OFS_WIN_ATTR)] = 0x1; /* High size is not used since the window size is smaller. Slave target is not used in the AHBP address decoder. bit 0 is window enable */

 return 0;
}
#endif

/**
* @internal srvCpuSysMapInit function
* @endinternal
*
* @brief   Setup mapping windows
*          This routine sets up mapping windows for various units such as DFX, SPI.
*
* @retval void          - none
*/
void srvCpuSysMapInit(void)
{
    MV_U32 data;
#ifdef SDMA_SUPPORT
    int win = 0; /*  SDMA address decoding window number - for all MGs */
    MV_U8  i;
#endif

    /* Create mutex before starting tasks */
    vSemaphoreCreateBinary(xRegLock);

#ifdef CONFIG_AAS
    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AAS_E)
    {
        srvCpuSysMapAASAddressDecoder(0 /* TODO, currently only supports CPU #0 */,
                                      3,
                                      AAS_DFX_SERVER_ADDR,
                                      0x0,
                                      CM7_AHBP_BASE,
                                      0x100000);
        return;
    }
#endif

    /* configure internal access window to RUnit */
    srvCpuScpuToMsysInterRegs(SRV_CPU_CM3_INTERNAL_WIN_BASE, SRV_CPU_CM3_INTERNAL_WIN_SIZE);

    /* configure illegal access default target */
    MV_MEMIO32_WRITE((MG_BASE | 0x200), 0);
    MV_MEMIO32_WRITE((MG_BASE | 0x204), 0xe1);
    MV_MEMIO32_WRITE((MG_BASE | 0x340), 0);

    /* configure window 0 to DFX server */
    MV_MEMIO32_WRITE((MG_BASE | 0x484), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x488), 0x00000000); /* High base addr */
    if (devices_details[devIndex].deviceType >= SRV_CPU_DEVICE_TYPE_FALCON_E) {
        MV_MEMIO32_WRITE((MG_BASE | 0x480), 0x0000000D); /* Base addr + Target, accessed @ 0xa0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x48c), devices_details[devIndex].dfxBase);   /* Remap */
    } else {
        MV_MEMIO32_WRITE((MG_BASE | 0x480), 0x00000008); /* Base addr + Target, accessed @ 0xa0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x48c), 0x0000000e); /* Remap */
    }
    MV_MEMIO32_WRITE((MG_BASE | 0x4a0), 0x0020000D); /* Base addr + Target, accessed @ 0xa0300000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a4), 0x000f0000); /* Size = 1MB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4a8), 0x00000000); /* High base addr - this window is for general switch access */
    if (devices_details[devIndex].deviceType <= SRV_CPU_DEVICE_TYPE_FALCON_E) {
        MV_MEMIO32_WRITE((MG_BASE | 0x490), 0x10001e01); /* Base addr + Target, accessed @ 0xb0100000 */
        MV_MEMIO32_WRITE((MG_BASE | 0x494), 0x00ff0000); /* Size = 16MB */
        MV_MEMIO32_WRITE((MG_BASE | 0x498), 0x00000000); /* High base addr */
        MV_MEMIO32_WRITE((MG_BASE | 0x49c), 0x0000000e); /* Remap */
    }

    /* set other windows and enable debug */
    switch (devices_details[devIndex].deviceType) {
    case SRV_CPU_DEVICE_TYPE_ALDRIN_E:
        MV_MEMIO32_WRITE((INTER_REGS_BASE | 0x18000), 0x00222222); /* Set MPP[5:0] to SPI */
        MV_MEMIO32_WRITE((SRV_CPU_DFX_WIN_BASE | 0xf8288), 0x067FC108); /* Enable CM3 JTAG */
        break;
    case SRV_CPU_DEVICE_TYPE_PIPE_E:
        /* PUP enable */
        MV_MEMIO32_WRITE((SRV_CPU_DFX_WIN_BASE | 0xf828c), 0x00000006);
        /* set console owner to 1 in case only core 1 is active
        if ((MV_MEMIO32_READ(SRV_CPU_DFX_WIN_BASE | PIPE_SAR) & (0x03 << 9)) == (0x02 << 9))
            mvUartSetOwner(1); */
        break;
    case SRV_CPU_DEVICE_TYPE_BC3_E:
        break;
    case SRV_CPU_DEVICE_TYPE_ALDRIN2_E:
        MV_MEMIO32_WRITE((INTER_REGS_BASE | 0x18008),0x11111000);
        break;
    case SRV_CPU_DEVICE_TYPE_FALCON_E:
        data = MV_MEMIO32_READ(SRV_CPU_DFX_WIN_BASE | 0xf828c);
        MV_MEMIO32_WRITE((SRV_CPU_DFX_WIN_BASE | 0xf828c), (data | 0x00000001)); /* Enable CM3 JTAG */
        break;
    case SRV_CPU_DEVICE_TYPE_AC5_E:
        srvCpuRegWrite(0x8002010c, (srvCpuRegRead(0x8002010c) & 0x0000ffff) | 0x22220000); /* set MPP 28-31 to enable SMI */
        srvCpuRegWrite(0x840F8288, (srvCpuRegRead(0x840F8288) & 0xFFFFFF1F) | 0xe0); /* Enable CM3 JTAG  0x840F8288 bits 5-7 */
        break;
    case SRV_CPU_DEVICE_TYPE_AC5X_E:
        srvCpuRegWrite(0x944F829C, (srvCpuRegRead(0x944F829C) & 0xFFFFF1FF) | 0xe00); /* Enable CM3 JTAG  0x944F829C bits 9-11 */
        break;
    case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
        srvCpuRegWrite(0x840F82A0, (srvCpuRegRead(0x840F82A0) & 0xFE3FFFFF) | 0x1C00000); /* Enable CM3 JTAG - 0x840F82A0 bits 22-24*/
        break;
    case SRV_CPU_DEVICE_TYPE_AC5P_E:
        data = MV_MEMIO32_READ(SRV_CPU_DFX_WIN_BASE | 0xf8284);
        MV_MEMIO32_WRITE((SRV_CPU_DFX_WIN_BASE | 0xf8284), (data | 0x00003c00)); /* Enable CM3 JTAG */
        break;
    case SRV_CPU_DEVICE_TYPE_HARRIER_E:
#ifndef MICRO_INIT
        data = MV_MEMIO32_READ(SRV_CPU_DFX_WIN_BASE | 0xf8280);
        MV_MEMIO32_WRITE((SRV_CPU_DFX_WIN_BASE | 0xf8280), (data | 0x00000003)); /* Enable CM3 JTAG - 0x000F8280 bit 0-1 */
#endif /* !MICRO_INIT */
        break;
    default:
        break;
    }
#ifdef SDMA_SUPPORT
    /* Configure window to access CnM SRAM */
    MV_MEMIO32_WRITE(MG_BASE | 0x4b0, 0x00400000 | devices_details[devIndex].sdmaPort); /* Base addr + Target, accessed @ 0xa0500000 */
    MV_MEMIO32_WRITE(MG_BASE | 0x4b4, (devices_details[devIndex].cnmRemapSize * 0x10000)); /* Size */
    MV_MEMIO32_WRITE(MG_BASE | 0x4b8, 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE(MG_BASE | 0x4bc, (devices_details[devIndex].cnmAddress << 16) | 0xe); /* Remap base addr  */

    if (devices_details[devIndex].deviceType != SRV_CPU_DEVICE_TYPE_AC5_E) {
        /*
         * Setup address decoder window to access CnM SRAM
         * set base = 0, target = (AMB2/3)
         * set size = (size of SRAM)
         * set remap = (base of shared SRAM)
         */
        for ( i = 0; i < devices_details[devIndex].sdmaEngine; i++)
        {
            srvCpuRegWrite(srvCpuSdmaGetMgBase(i) | (0x20c + win * 8), 0x0 | devices_details[devIndex].sdmaPort);
            srvCpuRegWrite(srvCpuSdmaGetMgBase(i) | (0x210 + win * 8), (devices_details[devIndex].cnmRemapSize * 0x10000));
            srvCpuRegWrite(srvCpuSdmaGetMgBase(i) | (0x254 + win * 4), (devices_details[devIndex].cnmAddress << 16) | 0xe);
        }
    } else { /* devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5_E */
        srvCpuRegWrite(0x20c + win * 8, 0x00000003); /* set base = 0, target = 3 (AMB3) */
        srvCpuRegWrite(0x210 + win * 8, 0x000f0000); /* set size = 0x000f0000 (1 MB) */
        srvCpuRegWrite(0x254 + win * 4, 0x00000006); /* set Window Control Register = 0x00000006 (Enable remap)*/

        /*  Because of missing the remap configuration for the lower part of the address (bits [31:16] of the address),
         *  decide to leave the address generated by the SDMA(base address 0x00000000) and to do the remap in one of the AMB address decoders.
         *  So activate window 0 to do the remap to CNM SRAM.
         *  Create new window(one of the AMB address decoders) to access CnM SRAM with address base zero.
         */
        srvCpuRegWrite(0x804301F0,0x13); /*       Window Attributes and Size High register
                                                  bit[5:1] Target - 0x9 - configured to CnM SRAM
                                                  bit[0] Window Enable - 0x1 - Enable
                                                  {1001,1} = 0x13  */
        srvCpuRegWrite(0x804301F4,0xf);        /* Size - 1 MB */
        srvCpuRegWrite(0x804301FC,0x00000000); /* Window Base Low */
        srvCpuRegWrite(0x80430200,0x00007fe0); /* Window Remap Low  - defined to CNM SRAM */
    }
#endif  /* SDMA_SUPPORT */
}

static void prvScpuMaskIrq(void)
{
    switch (devices_details[devIndex].deviceType) {
#ifdef CONFIG_AAS
    case SRV_CPU_DEVICE_TYPE_AAS_E:
        /* mask all MG interrupts for SCPU #0 (IRQ TREE3) */
        MV_MEMIO32_WRITE(AAS_GLOB_IRQ_SUM_MASK_REG_3, 0x0);
    break;
#endif

    default:
        /* mask all MG interrupts for SCPU (IRQ3) */
        MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, 0x0);
    }
}

/* IRQ3 of SCPU is connected to MG3 interrupt tree
 * This function enables uart0 interrupt through MG3 for all device types */
static void prvScpuUnmaskIrq(void)
{
    MV_U32 data;

#ifndef SHM_UART
    switch (devices_details[devIndex].deviceType) {
    case SRV_CPU_DEVICE_TYPE_ALDRIN_E:
    case SRV_CPU_DEVICE_TYPE_PIPE_E:
    case SRV_CPU_DEVICE_TYPE_BC3_E:
    case SRV_CPU_DEVICE_TYPE_ALDRIN2_E:
        data = MV_MEMIO32_READ(GLOB_IRQ_SUM_MASK_REG_3);
        MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, data | (1 << 7)); /* unmask MG Internal IRQ Summary */
        data = MV_MEMIO32_READ(MG_INT_IRQ_MASK_REG_3);
        uartIntr = 6 + CLIUartPort;
        MV_MEMIO32_WRITE(MG_INT_IRQ_MASK_REG_3, data | (1 << uartIntr)); /* unmask uart interrupt */
        break;
    case SRV_CPU_DEVICE_TYPE_AC5_E:
    case SRV_CPU_DEVICE_TYPE_AC5X_E:
    case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
        data = MV_MEMIO32_READ(GLOB_IRQ_SUM_MASK_REG_3);
        MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, data | (1 << 17)); /* unmask ports_2_int_sum */
        data = MV_MEMIO32_READ(MG_PORTS2_IRQ_MASK_REG_3);
        uartIntr = 9 + CLIUartPort;
        MV_MEMIO32_WRITE(MG_PORTS2_IRQ_MASK_REG_3, data | (1 << uartIntr)); /* unmask uart_intr_2 */
        break;
    case SRV_CPU_DEVICE_TYPE_FALCON_E:
    case SRV_CPU_DEVICE_TYPE_AC5P_E:
    case SRV_CPU_DEVICE_TYPE_HARRIER_E:
        data = MV_MEMIO32_READ(GLOB_IRQ_SUM_MASK_REG_3);
        MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, data | (1 << 16)); /* unmask ports_1_int_sum */
        data = MV_MEMIO32_READ(MG_PORTS1_IRQ_MASK_REG_3);
        uartIntr = 7 + CLIUartPort;
        MV_MEMIO32_WRITE(MG_PORTS1_IRQ_MASK_REG_3, data | (1 << uartIntr)); /* unmask uart0 */
        break;
#ifdef CONFIG_AAS
    case SRV_CPU_DEVICE_TYPE_AAS_E:
        /* Update the appropriate irq summary bits: */
        data = MV_MEMIO32_READ(AAS_CNM0_INT_SUM_MASK_REG_3);
        MV_MEMIO32_WRITE(AAS_CNM0_INT_SUM_MASK_REG_3, (data | AAS_UART0_CAUSE_BIT));
        data = MV_MEMIO32_READ(CNM_AGGREGATION_IRQ_SUMMARY_MASK_REG_3);
        MV_MEMIO32_WRITE(CNM_AGGREGATION_IRQ_SUMMARY_MASK_REG_3, (data | CNM0_INT_SUM_CAUSE_SUM_BIT) ); /* This is where CM7 own interrupt mask is aggerrgated into  */
        break;
#endif

    default:
        break;
    }
#endif
    /* unmask doorbell interrupt */
    switch (devices_details[devIndex].deviceType) {
#ifdef CONFIG_AAS
    case SRV_CPU_DEVICE_TYPE_AAS_E:
        data = MV_MEMIO32_READ(CM7_0_EXT_INTERRUPT_MASK); /* must unmask the external interrupt, otherwise it does not propagate down the interrupt tree */
        MV_MEMIO32_WRITE(CM7_0_EXT_INTERRUPT_MASK, data | HOST_TO_CM7_DOORBELL); /* Host to SCPU doorbell IRQ */
        data = MV_MEMIO32_READ(CNM_AGGREGATION_IRQ_SUMMARY_MASK_REG_3);
        MV_MEMIO32_WRITE(CNM_AGGREGATION_IRQ_SUMMARY_MASK_REG_3, data | (CM7_IRQ_SUMMARY_CAUSE_BIT_BASE << 0 /* TODO: CPU ID */) ); /* This is where CM7 own interrupt mask is aggerrgated into  */
        data = MV_MEMIO32_READ(AAS_GLOB_IRQ_SUM_MASK_REG_3);
        MV_MEMIO32_WRITE(AAS_GLOB_IRQ_SUM_MASK_REG_3, data | (1<<1 /* bit 1 in the global summary is GDMA aggergation interrupt summary. This is actually the CNM aggregation IRQ summary */ ));
        break;
#endif
    default:
        data = MV_MEMIO32_READ(MG1_INT_IRQ_MASK_REG_3);
        MV_MEMIO32_WRITE(MG1_INT_IRQ_MASK_REG_3, data | MG1_DOORBELL_FROM_MSYS); /* Host to SCPU doorbell IRQ */
        data = MV_MEMIO32_READ(GLOB_IRQ_SUM_MASK_REG_3);
        MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, data | (1<<18 ));
    }
}

#ifdef TWSI_SUPPORT
/**
* @internal prvI2cMppConfig function
* @endinternal
*
* @brief   configure I2C MPP's according to interface
*
* @param interface - select between first interface and second interface
*
* @retval MV_STATUS
*/
MV_STATUS prvI2cMppConfig(MV_U8 interface)
{
    MV_STATUS rc = MV_OK;
    switch (devices_details[devIndex].deviceType) {
    case SRV_CPU_DEVICE_TYPE_AC5_E:
    case SRV_CPU_DEVICE_TYPE_AC5X_E:
        if (interface == 0)
            rc = prvSrvCpuMppConfig(26, 0x1, 2); /* MPPs 26, 27 */
        else /* interface = 1 */
            rc = prvSrvCpuMppConfig(20, 0x2, 2); /* MPPs 20, 21 */
        break;
    case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
        if (interface == 0)
            rc = prvSrvCpuMppConfig(21, 0x1, 2); /* MPPs 21, 22 */
        else if (interface == 1)
            rc = prvSrvCpuMppConfig(43, 0x1, 2); /* MPPs 43, 44 */
        else /* interface = 2 */
            rc = prvSrvCpuMppConfig(36, 0x2, 2); /* MPPs 36, 37 */
        break;
    default:
        break;
    }
    return rc;
}

/**
* @internal srvCpuUnmaskI2cIrq function
* @endinternal
*
* @brief unmask i2c irq (according to interface)
*
* @param MV_U8 interface - select between first interface and second interface
*
* @retval none
*/
void srvCpuUnmaskI2cIrq(MV_U8 interface)
{
    MV_U32 data;
    switch (devices_details[devIndex].deviceType) {
        case SRV_CPU_DEVICE_TYPE_AC5_E:
        case SRV_CPU_DEVICE_TYPE_AC5X_E:
        case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
            data = MV_MEMIO32_READ(GLOB_IRQ_SUM_MASK_REG_3);
            MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, data | (1 << 17)); /* unmask ports_2_int_sum */
            data = MV_MEMIO32_READ(MG_PORTS2_IRQ_MASK_REG_3);
            i2cIntr = (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_IRONMAN_E)? 17 : 13;
            MV_MEMIO32_WRITE(MG_PORTS2_IRQ_MASK_REG_3, data | (interface? I2C_INTR_1 : I2C_INTR_0)); /* unmask i2c */
            break;
        case SRV_CPU_DEVICE_TYPE_FALCON_E:
        case SRV_CPU_DEVICE_TYPE_AC5P_E:
        case SRV_CPU_DEVICE_TYPE_HARRIER_E:
            data = MV_MEMIO32_READ(GLOB_IRQ_SUM_MASK_REG_3);
            MV_MEMIO32_WRITE(GLOB_IRQ_SUM_MASK_REG_3, data | (1 << 16)); /* unmask ports_1_int_sum */
            data = MV_MEMIO32_READ(MG_PORTS1_IRQ_MASK_REG_3);
            i2cIntr = 9;
            MV_MEMIO32_WRITE(MG_PORTS1_IRQ_MASK_REG_3, data | (interface? I2C_INTR_1 : I2C_INTR_0)); /* unmask i2c */
            break;
        default:
            break;
    }
}
#endif

/**
* @internal srvCpuUnmaskIpcIrq function
* @endinternal
*
* @brief unmask IPC irq
*
* @param  none
*
* @retval none
*/
#ifdef IPC_CLIENT
void srvCpuUnmaskIpcIrq()
{
    MV_U32 data;
    switch (devices_details[devIndex].deviceType) {
        case SRV_CPU_DEVICE_TYPE_AC5_E:
        case SRV_CPU_DEVICE_TYPE_AC5X_E:
        case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
        case SRV_CPU_DEVICE_TYPE_AC5P_E:
        case SRV_CPU_DEVICE_TYPE_HARRIER_E:
            /* unmask scpu_to_scpu_Doorbell_reg interrupt */
            data = MV_MEMIO32_READ(MG1_INT_IRQ_MASK_REG_3);
            MV_MEMIO32_WRITE(MG1_INT_IRQ_MASK_REG_3, data | SCPU_TO_SCPU_DOORBELL0 | SCPU_TO_SCPU_DOORBELL1 |
                                                            SCPU_TO_SCPU_DOORBELL2 | SCPU_TO_SCPU_DOORBELL3 );
            break;
        case SRV_CPU_DEVICE_TYPE_FALCON_E:
            /* not supported yet. */
            break;
        default:
            break;
    }
}
#endif /* IPC_CLIENT */

void prvUtilityTask( void *pvParameters )
{
    MV_U32* queueValue = 0;
#ifdef TWSI_SUPPORT
    MV_U32  regValue = 0, stateMachine, interfaceConfig = 0;
#endif
    for ( ;; ) {

        vTaskDelay(5 * portTICK_RATE_MS); /* wait for 5mSec */

        while( xSemaphoreTake( isrUtilityTaskSemaphore, 1) != pdTRUE );
        /* read the values send in queue, end until queue is empty */
        if( xUtilityQueue != NULL )
        {
            while (xQueueReceive( xUtilityQueue,( void * ) &queueValue, 5) == pdTRUE)
            {
                if ((long)queueValue >= 128) /* Doorbell interrupt triggered by other SCPU due to IPC message */
                    ipcMessageResponse((long)queueValue - 128);
                else if ((enum SRV_CPU_DOORBELL_FROM_HOST_E)queueValue == SRV_CPU_DOORBELL_FROM_HOST_IPC_MESSAGE_E)
                    ipcMessageResponse(0xff);
                else if ((enum SRV_CPU_DOORBELL_FROM_HOST_E)queueValue == SRV_CPU_DOORBELL_FROM_HOST_SW_RESET_PREPARATION_E)
                {
#ifdef TWSI_SUPPORT
                    /*--------------------------start of SRV_CPU_DOORBELL_FROM_HOST_SW_RESET_PREPARATION_E ----------------------------*/
                    /* the following stages are mandatory insure sw reset done right */
                    /*stage 1 - set interface flag as lock, and wait until interface finish current transaction  */
                    srvCpuI2cSetInterfaceLock();
                    while (srvCpuI2cIsIdleState(0 /* interface 0 */) != MV_OK) {
                        vTaskDelay(2 * portTICK_RATE_MS); /* wait for 2mSec */
                    }

                    /* stage 2 - interface ready: Store the defined configuration and send Doorbell interrupt to host */
                    interfaceConfig = srvCpuRegRead(devices_details[devIndex].runit + 0x1100c + ((0/*interface*/)*0x100));
                    MV_MEMIO32_WRITE(CM3_TO_MSYS_DOORBELL_REG, SCPU_READY_TO_SW_RESET);

                    /* stage 3 - polling on register that indicate that the device reset-init is done */
                    /* state machine: 1. Before sw reset been triggered. (reg = 0x0)
                                      2. SW reset is executing. (reg = 0x1)
                                      3. SW reset ends (reg = 0x0) */
                    stateMachine = 0;
                    while (stateMachine < 2) {
                        vTaskDelay(2 * portTICK_RATE_MS); /* wait for 2mSec */
                        /* Bit 31 is used for PoE resume indication, interfaces can continue to work after soft reset was performed.
                           0x0 - init stage done, 0x1 - init stage not done */
                        regValue = (MV_MEMIO32_READ(UNITS_INIT_DONE_STATUS7) >> 31);
                        if (regValue != stateMachine)
                            stateMachine++;
                        if (regValue == 1)
                        {
                            /* Write 1 and then 0 to this bit (31) to reset the HW indication in register UNITS_INIT_DONE_STATUS7 bit 31 */
                            MV_MEMIO32_WRITE(GENERAL_CONFIGURATION_1, 0x1 << 31);
                            MV_MEMIO32_WRITE(GENERAL_CONFIGURATION_1, 0x0 << 31);
                        }
                    }

                    /* stage 4 - reconfigure interfaces, set flag as free */
                    srvCpuRegWrite(devices_details[devIndex].runit + 0x1100c + ((0/*interface*/)*0x100), interfaceConfig);
                    srvCpuI2cSetInterfaceUnLock();
                    /*--------------------------End of SRV_CPU_DOORBELL_FROM_HOST_SW_RESET_PREPARATION_E ------------------------------*/
#endif /*TWSI_SUPPORT*/
                }
            }
        }
        xSemaphoreTake( isrUtilityTaskSemaphore, 1); /* take semaphore for next transaction */
    }
}

static void prvDoorbellFromHost(MV_BOOL is_AAS)
{
    /* Read the value of Doorbell interrupt HOST has transmit */
    enum SRV_CPU_DOORBELL_FROM_HOST_E value = MV_MEMIO32_READ((is_AAS ? AAS_HOST_TO_CM3_DOORBELL_REG : HOST_TO_CM3_DOORBELL_REG));

    switch (value)
    {
        case SRV_CPU_DOORBELL_FROM_HOST_GENERAL_E:
            /* TODO: only for demo purpose - hook your interrupt handler here */
            /* printf("Doorbell from HOST\n"); */
            break;
        case SRV_CPU_DOORBELL_FROM_HOST_IPC_MESSAGE_E:
        case SRV_CPU_DOORBELL_FROM_HOST_SW_RESET_PREPARATION_E:
            xQueueSendFromISR( xUtilityQueue, &value, NULL );
            xSemaphoreGiveFromISR( isrUtilityTaskSemaphore, 0 );
            break;
        default:
       break;
    }
}

static void cm3_mg_internal_irq_dispatcher3(void)
{
    int reg = 0;
#ifdef IPC_CLIENT
    int scpuTrig = 0, i;
#endif
#if (!defined(DISABLE_CLI) && !defined(SHM_UART)) || defined(TWSI_SUPPORT)
    int regC = 0, regM = 0;

    switch (devices_details[devIndex].deviceType) {
        case SRV_CPU_DEVICE_TYPE_ALDRIN_E:
        case SRV_CPU_DEVICE_TYPE_PIPE_E:
        case SRV_CPU_DEVICE_TYPE_BC3_E:
        case SRV_CPU_DEVICE_TYPE_ALDRIN2_E:
            regC = MV_MEMIO32_READ(MG_INT_IRQ_CAUSE_REG_3);
            regM = MV_MEMIO32_READ(MG_INT_IRQ_MASK_REG_3);
            break;
        case SRV_CPU_DEVICE_TYPE_AC5_E:
        case SRV_CPU_DEVICE_TYPE_AC5X_E:
        case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
            regC = MV_MEMIO32_READ(MG_PORTS2_IRQ_CAUSE_REG_3);
            regM = MV_MEMIO32_READ(MG_PORTS2_IRQ_MASK_REG_3);
            break;
        case SRV_CPU_DEVICE_TYPE_FALCON_E:
        case SRV_CPU_DEVICE_TYPE_AC5P_E:
        case SRV_CPU_DEVICE_TYPE_HARRIER_E:
            regC = MV_MEMIO32_READ(MG_PORTS1_IRQ_CAUSE_REG_3);
            regM = MV_MEMIO32_READ(MG_PORTS1_IRQ_MASK_REG_3);
            break;
        default:
            break;
    }
#endif
#if !defined(DISABLE_CLI) && !defined(SHM_UART)
    if (regC & (1 << uartIntr))
        iCLIHandler();
    if (regM) {} /* dummy - avoid compilation error */
#endif /* DISABLE_CLI */

#ifdef TWSI_SUPPORT
    if (regC & regM & I2C_INTR_0)
        cm3_i2c_handler(0);
    if (regC & regM & I2C_INTR_1)
        cm3_i2c_handler(1);
#endif
    /* Doorbell interrupt */
    reg = MV_MEMIO32_READ(MG1_INT_IRQ_CAUSE_REG_3);
    if (reg & MG1_DOORBELL_FROM_MSYS) {
        prvDoorbellFromHost(MV_FALSE);
    }
#ifdef IPC_CLIENT
    if (reg & (SCPU_TO_SCPU_DOORBELL0 | SCPU_TO_SCPU_DOORBELL1 |
               SCPU_TO_SCPU_DOORBELL2 | SCPU_TO_SCPU_DOORBELL3)) {
        /* Add to queue the core ID's of the interrupts that were raised */
        for ( i = 0; i < 4; i++)
        {
            if (reg & (SCPU_TO_SCPU_DOORBELL0 << i)) {
                scpuTrig = 128 + i; 
                xQueueSendFromISR( xUtilityQueue, &scpuTrig, NULL );
            }
        }
        /* Release semaphore will cause utility task to perform */
        xSemaphoreGiveFromISR( isrUtilityTaskSemaphore, 0 );
    }
#endif
}

#ifdef CONFIG_AAS
static void cm7_cnm_internal_irq_dispatcher3(void)
{
    unsigned glbl, aggr, ext;

    glbl = MV_MEMIO32_READ(AAS_GLOB_IRQ_SUM_CAUSE_REG_3);

    if (glbl & GLBL_CNM_AGGR_BIT)
    {
        aggr = MV_MEMIO32_READ(CNM_AGGREGATION_IRQ_SUMMARY_CAUSE_REG_3);

        if (aggr & CM7_IRQ_SUMMARY_CAUSE_BIT_BASE) /* currently only CM7 #0 is supported */
        {

            ext = MV_MEMIO32_READ(CM7_0_EXT_INTERRUPT_CAUSE);

            if ( ext & HOST_TO_CM7_DOORBELL)
            {
                prvDoorbellFromHost(MV_TRUE);
            }

        }

        if (aggr & CNM0_INT_SUM_CAUSE_SUM_BIT)
        {
            ext = MV_MEMIO32_READ(AAS_CNM0_INT_SUM_CAUSE_REG_3);

            if ( ext & AAS_UART0_CAUSE_BIT)
            {
#if !defined(DISABLE_CLI) && !defined(SHM_UART)
                iCLIHandler();
#endif
            }
        }

    }
}
#endif

/* Handler for IRQ3 */
static void cm3_irq_dispatcher3(void)
{
    int reg;

    ( void ) portSET_INTERRUPT_MASK_FROM_ISR();
    {
        switch (devices_details[devIndex].deviceType) {
#ifdef CONFIG_AAS
            case SRV_CPU_DEVICE_TYPE_AAS_E: /* CM7 for AAS */
                reg = MV_MEMIO32_READ(AAS_GLOB_IRQ_SUM_CAUSE_REG_3);
                if (reg & 0x1) /* bit[0] is summary */
                {
                    cm7_cnm_internal_irq_dispatcher3();
                }
            break;
#endif
            default:
                reg = MV_MEMIO32_READ(GLOB_IRQ_SUM_CAUSE_REG_3);
                if (reg & 0x1) /* bit[0] is summary */
                {
                    cm3_mg_internal_irq_dispatcher3();
                }
        }
    }
    portCLEAR_INTERRUPT_MASK_FROM_ISR( 0 );
}

/**
* @internal srvCpuInterruptInit function
* @endinternal
*
* @brief register interrupt tree, unmask interrupt
*
* @param none
*
* @retval none
*/
void srvCpuInterruptInit(void)
{
    /* maks all interrupts */
    prvScpuMaskIrq();

    switch (devices_details[devIndex].deviceType) {
#ifdef CONFIG_AAS
        case SRV_CPU_DEVICE_TYPE_AAS_E: /* Interrupt #1 in CM7 for AAS */
            iICRegisterHandler( NVIC_GLOBAL_IRQ(1), 1,
                cm3_irq_dispatcher3, IRQ_ENABLE, 0xe0 ); /* Priority same as kernel to prevent nesting */
        break;
#endif

        default:
            iICRegisterHandler( NVIC_GLOBAL_IRQ(3), 3,
                cm3_irq_dispatcher3, IRQ_ENABLE, 0xe0 ); /* Priority same as kernel to prevent nesting */
    }

    /* Unmask used scpu interrupts in the switch core interrupt tree */
    prvScpuUnmaskIrq();
}

void cm3_to_msys_doorbell(void)
{
    MV_MEMIO32_WRITE(CM3_TO_MSYS_DOORBELL_REG, 0x1);
}

