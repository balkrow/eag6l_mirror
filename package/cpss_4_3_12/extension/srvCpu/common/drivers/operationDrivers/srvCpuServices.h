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
* @file srvCpuServices.h
*
* @brief srvCpu services declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __SRV_CPU_SERVICES_H__
#define __SRV_CPU_SERVICES_H__
#include "common.h"

/* MG base address */
#define SRV_CPU_EXT_MEM_BASE        0xA0100000
#define SRV_CPU_DFX_WIN_BASE        0xA0100000
#define SRV_CPU_DFX_WIN_SIZE        (0xF<<16)     /* 1M */

/* configuration parameters */
#define SRV_CPU_CM3_INTERNAL_WIN_SIZE   0x100000
#define SRV_CPU_CM3_INTERNAL_WIN_BASE   0x0

#define SRV_CPU_SOFT_RESET_REG_HARRIER  0xf8204 /* in DFX */
#define SRV_CPU_SOFT_RESET_BIT_HARRIER  1

#define SRV_CPU_SW_WIN_BASE         0xa0300000

#define SRV_CPU_MG_SOURCE_ID         0x604

#define FALCON_TILE0_MG_BASE_ADDRESS    0x1D000000
#define FALCON_TILE2_MG_BASE_ADDRESS    0x5D000000
#define NUM_OF_CM3_IN_FALCON_DUAL_TILE  2
#define RUNIT_SIZE                      0x100000
#define CM3_SRAM_OFFSET                 0x80000

/* Switching core interrupt registers */
#define GLOB_IRQ_SUM_MASK_REG_0     (MG_BASE + 0x030)
#define GLOB_IRQ_SUM_CAUSE_REG_0    (MG_BASE + 0x034)
#define MG_INT_IRQ_CAUSE_REG_0      (MG_BASE + 0x038)
#define MG_INT_IRQ_MASK_REG_0       (MG_BASE + 0x03c)
#define GENERAL_CONFIGURATION_1     (MG_BASE + 0x110)
#define MG1_INT_IRQ_CAUSE_REG_3     (MG_BASE + 0x118)
#define MG1_INT_IRQ_MASK_REG_3      (MG_BASE + 0x11c)
#define MG_PORTS1_IRQ_CAUSE_REG_3   (MG_BASE + 0x2d0)
#define MG_PORTS1_IRQ_MASK_REG_3    (MG_BASE + 0x2d4)
#define MG_PORTS2_IRQ_CAUSE_REG_3   (MG_BASE + 0x2d8)
#define MG_PORTS2_IRQ_MASK_REG_3    (MG_BASE + 0x2dc)
#define GLOB_IRQ_SUM_CAUSE_REG_3    (MG_BASE + 0x2e0)
#define GLOB_IRQ_SUM_MASK_REG_3     (MG_BASE + 0x2e4)
#define MG_INT_IRQ_CAUSE_REG_3      (MG_BASE + 0x2e8)
#define MG_INT_IRQ_MASK_REG_3       (MG_BASE + 0x2ec)
#define PORTS_IRQ_SUM_CAUSE_REG3    (MG_BASE + 0x2F0)
#define PORTS_IRQ_SUM_MASK_REG3     (MG_BASE + 0x2F4)
#define UNITS_INIT_DONE_STATUS7     (MG_BASE + 0x3BC)
#define DOORBELL_1_REG              (MG_BASE + 0x440)
#define HOST_TO_CM3_DOORBELL_REG    (MG_BASE + 0x534)
#define CM3_TO_MSYS_DOORBELL_REG    (MG_BASE + 0x538)
#define GLOB_IRQ_SUM_CAUSE_REG_1    (MG_BASE + 0x610)
#define GLOB_IRQ_SUM_MASK_REG_1     (MG_BASE + 0x614)
#define MG_INT_IRQ_CAUSE_REG_1      (MG_BASE + 0x618)
#define MG_INT_IRQ_MASK_REG_1       (MG_BASE + 0x61c)
#define GLOB_IRQ_SUM_CAUSE_REG_2    (MG_BASE + 0x670)
#define GLOB_IRQ_SUM_MASK_REG_2     (MG_BASE + 0x674)
#define MG_INT_IRQ_CAUSE_REG_2      (MG_BASE + 0x678)
#define MG_INT_IRQ_MASK_REG_2       (MG_BASE + 0x67c)

/* Switching core interrupt registers - AAS: */
#define CNM_INTERRUPT_UNIT_BASE 0x5C4C0000
#define AAS_GLOB_IRQ_SUM_CAUSE_REG_3 (CNM_INTERRUPT_UNIT_BASE + 0x100 + 0x300 + 0x0)
#define AAS_GLOB_IRQ_SUM_MASK_REG_3 (CNM_INTERRUPT_UNIT_BASE + 0x100 + 0x300 + 0x4)
#define AAS_CNM0_INT_SUM_CAUSE_REG_3 (CNM_INTERRUPT_UNIT_BASE + 0x100 + 0x300 + 0x8)
#define AAS_CNM0_INT_SUM_MASK_REG_3 (CNM_INTERRUPT_UNIT_BASE + 0x100 + 0x300 + 0xC)
#define CNM_AGGREGATION_IRQ_SUMMARY_CAUSE_REG_3 (CNM_INTERRUPT_UNIT_BASE + 0x100 + 0x300 + 0xA0)
#define CNM_AGGREGATION_IRQ_SUMMARY_MASK_REG_3 (CNM_INTERRUPT_UNIT_BASE + 0x100 + 0x300 + 0xA4)
#define CNM_CM7_CLUSTER_RFU_CPU_BASE 0x5C760000
#define CM7_0_OWN_INTERRUPT_CAUSE (CNM_CM7_CLUSTER_RFU_CPU_BASE + 0x1010) /* CM7 instance #0 */
#define CM7_0_OWN_INTERRUPT_MASK (CNM_CM7_CLUSTER_RFU_CPU_BASE + 0x1014) /* CM7 instance #0 */
#define CM7_0_EXT_INTERRUPT_CAUSE (CNM_CM7_CLUSTER_RFU_CPU_BASE + 0x1018) /* CM7 instance #0 */
#define CM7_0_EXT_INTERRUPT_MASK (CNM_CM7_CLUSTER_RFU_CPU_BASE + 0x101C) /* CM7 instance #0 */
#define AAS_HOST_TO_CM3_DOORBELL_REG (CNM_CM7_CLUSTER_RFU_CPU_BASE + 0x1070) /* for CM7 #0 */

/* GLOB_IRQ_SUM_MASK_REG_3 fields */
#define MG_PORTS1_SUM_MASK          (1 << 16) /* Eagle */
#define MG1_INTERNAL_SUM_MASK       (1 << 18)
#define PORTS_INTERRUPTS_SUM_MASK   (1 << 5)

/* PORTS_IRQ_SUM MASK_REG3 fields */
#define PORTS_COMIRA_MASK       (3 <<  9)
#define PORTS_COMIRA0_MASK      (1 << 10)
#define PORTS_COMIRA1_MASK      (1 <<  9)

/* MG1_INT_IRQ_MASK_REG_3 fields */
#define MG1_DOORBELL_FROM_MSYS       (1 << 1)
#define SCPU_TO_SCPU_DOORBELL0       (1 << 7)
#define SCPU_TO_SCPU_DOORBELL1       (1 << 8)
#define SCPU_TO_SCPU_DOORBELL2       (1 << 9)
#define SCPU_TO_SCPU_DOORBELL3       (1 << 10)

/* AAS */
#define HOST_TO_CM7_DOORBELL           (1 << 9)
#define CM7_IRQ_SUMMARY_CAUSE_BIT_BASE (1 << 13)
#define GLBL_CNM_AGGR_BIT              (1 << 1)
#define AAS_UART0_CAUSE_BIT            (1 << 22)
#define CNM0_INT_SUM_CAUSE_SUM_BIT     (1 << 1)

/* MG_PORTS2_IRQ_MASK_REG_3 fields*/
#define I2C_INTR_0                      (1 << i2cIntr)
#define I2C_INTR_1                      (1 << (i2cIntr+1))

/**
* @enum DEVICE_TYPE
 *
 * @brief Device type enum
*/
enum SRV_CPU_DEVICE_TYPE_E {
    SRV_CPU_DEVICE_TYPE_CAELUM_E,
    SRV_CPU_DEVICE_TYPE_CETUS_E,
    SRV_CPU_DEVICE_TYPE_ALDRIN_E,
    SRV_CPU_DEVICE_TYPE_PIPE_E,
    SRV_CPU_DEVICE_TYPE_BC3_E,
    SRV_CPU_DEVICE_TYPE_ALDRIN2_E,
    SRV_CPU_DEVICE_TYPE_RAVEN_E,
    SRV_CPU_DEVICE_TYPE_FALCON_E,
    SRV_CPU_DEVICE_TYPE_AC5_E,
    SRV_CPU_DEVICE_TYPE_AC5X_E,
    SRV_CPU_DEVICE_TYPE_AC5P_E,
    SRV_CPU_DEVICE_TYPE_HARRIER_E,
    SRV_CPU_DEVICE_TYPE_IRONMAN_E,
    SRV_CPU_DEVICE_TYPE_AAS_E
};

struct SRV_CPU_DEVICES_TYPES_STC {
    enum SRV_CPU_DEVICE_TYPE_E deviceType;
    MV_U16 deviceId;
    MV_U8  cores;
    MV_U8  sdmaEngine;
    MV_U8  sdmaPort;
    MV_U8  phaPpgBase;   /* bits [32:24] */
    MV_U8  ppgNum;
    MV_U8  mppMax;
    MV_U16 mppBaseAddr;  /* bits [32:16] of size */
    MV_U16 systclk;      /* in units of 0.1MHz */
    MV_U32 baseAddress;
    MV_U16 scpuSramSize; /* in units of KB */
    MV_U16 cnmSramSize;  /* in units of KB */
    MV_U16 cnmRemapSize; /* bits [32:16] of size */
    MV_U16 cnmAddress;   /* bits [32:16] of size */
    MV_U32 dfxBase;
    MV_U32 runit;
};

/**
* @enum DOORBELL_FROM_HOST
 *
 * @brief doorbell from host enum
*/
enum SRV_CPU_DOORBELL_FROM_HOST_E {
    SRV_CPU_DOORBELL_FROM_HOST_GENERAL_E = 0x1,              /* BIT 0 */
    SRV_CPU_DOORBELL_FROM_HOST_IPC_MESSAGE_E = 0x2,          /* BIT 1 */
    SRV_CPU_DOORBELL_FROM_HOST_SW_RESET_PREPARATION_E = 0x4, /* BIT 2 */
    SRV_CPU_DOORBELL_FROM_HOST_RESERVE1_E = 0x8,             /* BIT 3 */
    SRV_CPU_DOORBELL_FROM_HOST_RESERVE2_E = 0x10,            /* BIT 4 */
    SRV_CPU_DOORBELL_FROM_HOST_RESERVE3_E = 0x20,            /* BIT 5 */
};

extern struct SRV_CPU_DEVICES_TYPES_STC devices_details[];
extern MV_U8 devIndex;
extern MV_U8 mgOwnId;

/**
* @internal srvCpuGetMgBase function
* @endinternal
*
* @brief   get base address of MG unit by number
*
* @param mgNum          - MG number
*
* @retval MV_U32        - MG base address
*/
MV_U32 srvCpuGetMgBase(MV_U32 mgNum);

/**
* @internal srvCpuScpuToMsysInterRegs function
* @endinternal
*
* @brief   set base and size to configure internal access window to RUnit
*
* @param base           - base
* @param size           - size
*
* @retval void          - none
*/
void   srvCpuScpuToMsysInterRegs(unsigned long base, unsigned long size);

extern MV_U32 remapWindow(MV_U32 offset);
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
MV_U32 srvCpuRegRead(MV_U32 offset);

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
MV_U32 srvCpuRegWrite(MV_U32 offset, MV_U32 value);

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
MV_U32 srvCpuQspiMapInit(MV_U32 address);

/**
* @internal setDeviceParameters function
* @endinternal
*
* @brief   Set device parameters accoring to device type
*
* @retval MV_TRUE          - succeed
*         MV_FALSE         - failed
*/
MV_BOOL setDeviceParameters(void);

/**
* @internal srvCpuMandatoryInit function
* @endinternal
*
* @brief   Initialize SCPU features
*
* @retval void          - none
*/
void srvCpuMandatoryInit(void);

/**
* @internal srvCpuSysMapInit function
* @endinternal
*
* @brief   Setup mapping windows
*          This routine sets up mapping windows for various units such as DFX, SPI.
*
* @retval void          - none
*/
void   srvCpuSysMapInit(void);

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
void srvCpuUnmaskI2cIrq(MV_U8 interface);

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
void srvCpuUnmaskIpcIrq( void);

/**
* @internal prvUtilityTask function
* @endinternal
*
* @brief utility task, handles when doorbell is asserted
*
* @param  none
*
* @retval none
*/
void prvUtilityTask( void *pvParameters );

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
void srvCpuInterruptInit(void);

#endif /*__SRV_CPU_SERVICES_H__*/

