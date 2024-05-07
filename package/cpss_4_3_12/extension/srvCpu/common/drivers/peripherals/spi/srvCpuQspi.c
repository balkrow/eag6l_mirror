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
* @file srvCpuQspi.c
*
* @brief Qspi capabilities implementation.
*
* @version   1
********************************************************************************
*/

#include <global.h>
#include <srvCpuServices.h>
#include <srvCpuTimer.h>
#include "srvCpuQspi.h"
#include <printf.h>

#define AC5_MPP_MUX(n)            (0x80020100 + 4 * n) /* Each register controls 8 pins */
#define HARRIER_MPP_MUX(n)        (0x3C000100 + 4 * n) /* Each register controls 8 pins */
#define AC5_QSPI_BASE_ADDRESS     0x805a0000
#define IRONMAN_QSPI_BASE_ADDRESS 0x806b0000
#define HARRIER_QSPI_BASE_ADDRESS 0x3c190000
#define CHANNEL_1_DEFAULT_OFFSET  0x8000

#define QSPI_CTRL_REG              (0x00)
#define QSPI_CFG_REG               (0x04)
#define QSPI_DATA_OUT_REG          (0x08)
#define QSPI_DATA_IN_REG           (0x0c)
#define QSPI_INSTRUCTION_REG       (0x10)
#define QSPI_ADDR_REG              (0x14)
#define QSPI_READ_MODE_REG         (0x18)
#define QSPI_HDR_COUNT_REG         (0x1c)
#define QSPI_DATA_IN_CNT_REG       (0x20)
#define QSPI_TIMING_REG            (0x24)
#define QSPI_INT_STATUS_REG        (0x28)
#define QSPI_INT_ENABLE_REG        (0x2c)
#define QSPI_START_ADDR_REG        (0x30)
#define QSPI_END_ADDR_REG          (0x34)
#define QSPI_PHYS_ADDR_REG         (0x38)

/* QSPI_CTRL_REG */
#define SPI_EN_3        19
#define SPI_EN_2        18
#define SPI_EN_1        17
#define SPI_EN_0        16
#define ADDR_ERR        12
#define WFIFO_OVRFLW    11
#define WFIFO_UNDRFLW   10
#define RFIFO_OVRFLW    9
#define RFIFO_UNDRFLW   8
#define WFIFO_FULL      7
#define WFIFO_EMPTY     6
#define RFIFO_FULL      5
#define RFIFO_EMPTY     4
#define WFIFO_RDY       3
#define RFIFO_RDY       2
#define SPI_RDY         1
#define SPI_XFER_DONE   0

/* QSPI_CFG_REG */
#define WFIFO_THRS_OFFSET       28
#define WFIFO_THRS_MASK         7
#define RFIFO_THRS_OFFSET       24
#define RFIFO_THRS_MASK         7
#define AUTO_CS_EN              20
#define DMA_WR_EN               19
#define DMA_RD_EN               18
#define FIFO_MODE               17
#define SRST                    16
#define XFER_START              15
#define XFER_STOP               14
#define INSTR_PIN               13
#define ADDR_PIN                12
#define DATA_PIN                10
#define DATA_PIN_MASK           3
#define FIFO_FLUSH              9
#define RW_EN                   8
#define SPI_CLK_POL             7
#define SPI_CLK_PHASE           6
#define SPI_MODE_MASK           0xc
#define BYTE_LEN                5
#define SPI_CLK_PRESCALE_OFFSET 0
#define SPI_CLK_PRESCALE_MASK   0x1f

/* QSPI_HDR_COUNT_REG */
#define DUMMY_CNT     12
#define RM_CNT        8
#define ADDR_CNT      4
#define INSTR_CNT     0

/* QSPI_TIMING_REG */
#define SPI_SIG_MON_SEL_OFFSET 28
#define SPI_SIG_MON_SEL_MASK   7
#define CS_SETUP_HOLD_OFFSET   12
#define CS_SETUP_HOLD_MASK     3
#define CLK_OUT_DLY_OFFSET     8
#define CLK_OUT_DLY_MASK       3
#define CLK_CAPT_EDGE          7
#define CLK_IN_DLY_OFFSET      4
#define CLK_IN_DLY_MASK        3
#define DATA_IN_DLY_OFFSET     0
#define DATA_IN_DLY_MASK       3

/* QSPI_INT_*_REG */
#define ADDR_ERR       12
#define WFIFO_OVRFLW   11
#define WFIFO_UNDRFLW  10
#define RFIFO_OVRFLW   9
#define RFIFO_UNDRFLW  8
#define WFIFO_FULL     7
#define WFIFO_EMPTY    6
#define RFIFO_FULL     5
#define RFIFO_EMPTY    4
#define WFIFO_RDY      3
#define RFIFO_RDY      2
#define SPI_RDY        1
#define XFER_DONE      0

/* Delay*/
/* one bit(~100nsec) * 8 bits =~1mSec for one byte*/
#define DELAY_32_BITS_IN_MILLISEC 4
#define DELAY_8_BITS_IN_MILLISEC  1

MV_U32 deviceQspiRegBaseAddr[2];
MV_U32 configuredWindow;

/**
* @internal srvCpuQspiInit function
* @endinternal
*
* @brief  Initialize the QSPI interface for single pin.
*
* @param MV_U32 ch             - QSPI unit number
* @param MV_U32 baudRate       - set baud rate
* @param SRV_CPU_SPI_MODE_E mode - set SPI mode
* @param MV_U32 protocol       - SPI protocol: 0 - Single, 1 - Dual, 2 - Quad
*
* @retval MV_STATUS
*
*/
MV_STATUS srvCpuQspiInit(MV_U32 ch, MV_U32 baudrate, enum SRV_CPU_SPI_MODE_E mode,  MV_U32 protocol)
{
    MV_U32 value, intRes, baseAddress = 0;
    double resDiv1,resDiv2;

    /* Set MPPs to enable QSPI for supported devices */
#ifdef BOOT_FW
/* TODO: temporarily copying this from boot init, need to use a flInfo structure for QSPI too */
    value = (MV_MEMIO32_READ(MG_BASE | 0x4c) >> 4) & 0xff00;
    switch (value) {
    case DEVICE_ID_AC5:
    case DEVICE_ID_AC5X:
        value = SRV_CPU_DEVICE_TYPE_AC5_E;
        break;
    case DEVICE_ID_HARRIER:
        value = SRV_CPU_DEVICE_TYPE_HARRIER_E;
        break;
    case DEVICE_ID_IRONMAN_M:
    case DEVICE_ID_IRONMAN_L:
        value = SRV_CPU_DEVICE_TYPE_IRONMAN_E;
        break;
    default:
        return MV_NOT_SUPPORTED;
    }

    switch (value)
#else
    switch (devices_details[devIndex].deviceType)
#endif
    {
        case SRV_CPU_DEVICE_TYPE_AC5_E:
            baseAddress = AC5_QSPI_BASE_ADDRESS;
            if (ch == 0) {
                srvCpuRegWrite(AC5_MPP_MUX(1), (srvCpuRegRead(AC5_MPP_MUX(1)) & 0x0000ffff) | 0x11110000);
                srvCpuRegWrite(AC5_MPP_MUX(2), (srvCpuRegRead(AC5_MPP_MUX(2)) & 0xffffff00) | 0x00000011);
            }
            else /*ch == 1*/
                srvCpuRegWrite(AC5_MPP_MUX(2), (srvCpuRegRead(AC5_MPP_MUX(2)) & 0x0000ffff) | 0x33330000);
            break;
        case SRV_CPU_DEVICE_TYPE_IRONMAN_E:
            baseAddress = IRONMAN_QSPI_BASE_ADDRESS;
            if (ch == 0) {
                srvCpuRegWrite(AC5_MPP_MUX(1), (srvCpuRegRead(AC5_MPP_MUX(1)) & 0x0000ffff) | 0x11110000);
                srvCpuRegWrite(AC5_MPP_MUX(2), (srvCpuRegRead(AC5_MPP_MUX(2)) & 0xffffff00) | 0x00000011);
            }
            else /*ch == 1*/
            {
                srvCpuRegWrite(AC5_MPP_MUX(2), (srvCpuRegRead(AC5_MPP_MUX(2)) & 0x0fffffff) | 0x10000000);
                srvCpuRegWrite(AC5_MPP_MUX(3), (srvCpuRegRead(AC5_MPP_MUX(3)) & 0xfffff000) | 0x00000111);
            }
            break;
        case SRV_CPU_DEVICE_TYPE_HARRIER_E:
            baseAddress = HARRIER_QSPI_BASE_ADDRESS;
            if(ch == 0) {
                srvCpuRegWrite(HARRIER_MPP_MUX(0), (srvCpuRegRead(HARRIER_MPP_MUX(0)) & 0xfff00000) | 0x11111);
                srvCpuRegWrite(HARRIER_MPP_MUX(1), (srvCpuRegRead(HARRIER_MPP_MUX(1)) & 0xf0ffffff) | 0x1000000);
            }
            else {
                srvCpuRegWrite(HARRIER_MPP_MUX(2), (srvCpuRegRead(HARRIER_MPP_MUX(2)) & 0x0fffffff) | 0x10000000);
                srvCpuRegWrite(HARRIER_MPP_MUX(3), (srvCpuRegRead(HARRIER_MPP_MUX(3)) & 0xfffff000) | 0x111);
            }
            break;
        default:
            return MV_NOT_SUPPORTED;
    }

    deviceQspiRegBaseAddr[ch] = baseAddress + ch*CHANNEL_1_DEFAULT_OFFSET;
#ifdef BOOT_FW
    configuredWindow = 0xa0200000;
#else
    configuredWindow = srvCpuQspiMapInit(baseAddress);
#endif

    /* Reset the qspi controller to defaults */
    srvCpuRegWrite(deviceQspiRegBaseAddr[ch] | QSPI_CFG_REG, (1 << SRST));
    srvCpuRegWrite(deviceQspiRegBaseAddr[ch] | QSPI_CFG_REG, 0);

    value = srvCpuRegRead(deviceQspiRegBaseAddr[ch] | QSPI_CFG_REG) & ~(SPI_CLK_PRESCALE_MASK); /* mask SPI_CLK_PRESCALE [4:0], SPI_CLK_PHASE [6] and SPI_CLK_POL [7] */
    /* Baud-rate */
    if (ch == 0)
        value |=  (500000000 / baudrate) & 0x1f;
    else /* ch == 1 */
    {
        /* Note that the available frequencies for SPI-1 are 25M/n (n is integer)
           Since the core clk is 25MHz, and divisions are full numbers,
           the valid options are 25Mhz, 12.5MHz, 8.33MHz, 6.25, etc.
        */
        if (baudrate == 25000000)
        {
            value  = 0; /* When the user requests a frequency that is equal to the core clk
                           than value should be equal to 0x0, and not 0x1 */
        }
        else
        {
            intRes = 25000000/baudrate;
            resDiv1 = 25000000/(double)intRes;
            resDiv2 = 25000000/(double)(intRes + 1);
            intRes = (ABS_SDK((baudrate - resDiv2)) > ABS_SDK((baudrate - resDiv1)))? intRes : intRes + 1;
            value |=  intRes & 0x1f;
        }
    }

    value |=  ((MV_U32)mode << SPI_CLK_PHASE) & (SPI_MODE_MASK << SPI_CLK_PHASE); /* mode */
    value |=  (protocol << DATA_PIN) & (DATA_PIN_MASK << DATA_PIN); /* protocol Q/D/SPI*/
    value |= (1 << BYTE_LEN);
    value &=  ~(0x3 << ADDR_PIN); /* set ADDR_PIN and INSTR_PIN as 0 [13:12]*/

    /* Mode */
    value |= 1 << FIFO_MODE; /* Enable FIFO */
    srvCpuRegWrite(deviceQspiRegBaseAddr[ch] | QSPI_CFG_REG, value);

    return MV_OK;
}

/**
* @internal srvCpuQspiRead function
* @endinternal
*
* @brief   send a read command with offset & dummy bytes, using FIFO
*
* @param MV_U32 ch          - select between first QSPI and second QSPI
* @param MV_U8  cmd         - 8bit instruction value (assumed to be single byte), 0x00 means no instruction
* @param MV_U32 addr        - value to send for address, up to 4 bytes long
* @param MV_U8  addr_cycles - 1-4 bytes, representing address length
* @param MV_U8  dummy       - number of dummy bytes to send after address, before collecting received bytes
* @param void*  buf         - pointer to buffer for received bytes
* @param MV_U32 size        - size of buffer
*
* @retval MV_STATUS
*/
MV_STATUS srvCpuQspiRead(MV_U32 ch, MV_U8 cmd, MV_U32 addr, MV_U8 addr_cycles, MV_U8 dummy, void *buf, MV_U32 size)
{

    MV_U32 i = 0, length = size;
    MV_U32 *buffer32 = (MV_U32 *)buf;
    MV_U32 base_addr = configuredWindow + (deviceQspiRegBaseAddr[ch] & 0x000fffff);

    /* Flush FIFO by writing '1' to FIFO_FLUSH */
    MV_MEMIO32_WRITE(base_addr | QSPI_CFG_REG , MV_MEMIO32_READ(base_addr | QSPI_CFG_REG) | (1 << FIFO_FLUSH));
    /* Wait until FIFO is flushed (FIFO_FLUSH = '0') */
    while (MV_MEMIO32_READ(base_addr | QSPI_CFG_REG) & (1 << FIFO_FLUSH));
    /* Assert SPI_CS0 */
    MV_MEMIO32_WRITE(base_addr , MV_MEMIO32_READ(base_addr) | (1 << SPI_EN_0));
    /* Program SPI_INSTR */
    if (cmd) {
        MV_MEMIO32_WRITE(base_addr | QSPI_INSTRUCTION_REG , cmd);
        i = 1;
    }

    /* Program SPI Header count */
    MV_MEMIO32_WRITE(base_addr | QSPI_HDR_COUNT_REG , i + (addr_cycles << ADDR_CNT) + (dummy << DUMMY_CNT));
    /* Program SPI_ADDR */
    MV_MEMIO32_WRITE(base_addr | QSPI_ADDR_REG , addr);
    /* Program SPI_RM */
    MV_MEMIO32_WRITE(base_addr | QSPI_READ_MODE_REG , 0);
    /* Program SPI_IN_CNT */
    MV_MEMIO32_WRITE(base_addr | QSPI_DATA_IN_CNT_REG , length);
    /* Program SPI Cfg reg FIFO_MODE = 1, RW_EN = 0, XFER_START = 1 */
    MV_MEMIO32_WRITE(base_addr  | QSPI_CFG_REG , (MV_MEMIO32_READ(base_addr | QSPI_CFG_REG) & ~(1 << RW_EN)) | (1 << XFER_START));

    size >>= 2; /* We read bytes in 32bit words */
    while (size >= 8) {
        i = 200;
        while (!(MV_MEMIO32_READ(base_addr) & (1 << RFIFO_FULL))) /*while FIFO not full, continue*/
        {
            if (i-- < 1)
                return MV_TIMEOUT;
        }

        for (i = 0; i < 8; i++, size--){
            *buffer32++ = MV_MEMIO32_READ(base_addr | QSPI_DATA_IN_REG);
        }
    }

    /* Wait until XFER DONE before de-asserting CS */
    i = 100;
    while (!(MV_MEMIO32_READ(base_addr) & (1 << SPI_XFER_DONE)))
    {
        srvCpuOsUsDelay(DELAY_32_BITS_IN_MILLISEC);
        if (i-- < 1)
            return MV_TIMEOUT;
    }

    /*while FIFO not empty and there is more to read , continue */
    while (!(MV_MEMIO32_READ(base_addr) & (1 << RFIFO_EMPTY)) && size > 0) {
        *buffer32++ = MV_MEMIO32_READ(base_addr | QSPI_DATA_IN_REG);
        size--;
    }

    /* De-assert SPI_CS0 */
    MV_MEMIO32_WRITE(base_addr , MV_MEMIO32_READ(base_addr) & ~(1 << SPI_EN_0));
    return MV_OK;
}

/**
* @internal srvCpuQspiWrite function
* @endinternal
*
* @brief   send a write command with offset & dummy bytes, using FIFO
*
* @param MV_U32 ch          - select between first QSPI and second QSPI
* @param MV_U8  cmd         - 8bit instruction value (assumed to be single byte), 0x00 means no instruction
* @param MV_U32 addr        - value to send for address, up to 4 bytes long
* @param MV_U8  addr_cycles - 1-4 bytes, representing address length
* @param MV_U8  dummy       - number of dummy bytes to send after address, before collecting received bytes
* @param void*  buf         - pointer to buffer for transmitting
* @param MV_U32 size        - size of buffer
*
* @retval MV_STATUS
*/
MV_STATUS srvCpuQspiWrite(MV_U32 ch, MV_U8 cmd, MV_U32 addr, MV_U8 addr_cycles, MV_U8 dummy, void *buf, MV_U32 size)
{

    MV_U32 i = 0;
    MV_U32 *buffer32 = (MV_U32 *)buf;
    MV_U32 base_addr = configuredWindow + (deviceQspiRegBaseAddr[ch] & 0x000fffff);

    /* Flush FIFO by writing '1' to FIFO_FLUSH */

    MV_MEMIO32_WRITE(base_addr  | QSPI_CFG_REG , MV_MEMIO32_READ( base_addr | QSPI_CFG_REG) | (1 << FIFO_FLUSH));
    /* Wait until FIFO is flushed (FIFO_FLUSH = '0') */

    while (MV_MEMIO32_READ(base_addr  | QSPI_CFG_REG) & (1 << FIFO_FLUSH));

    /* Assert SPI_CS0 */
    MV_MEMIO32_WRITE(base_addr, MV_MEMIO32_READ(base_addr) | (1 << SPI_EN_0));

    /* Program SPI_INSTR */
    if (cmd) {
        MV_MEMIO32_WRITE(base_addr | QSPI_INSTRUCTION_REG , cmd);
        i = 1;
    }
    /* Program SPI Header count */
    MV_MEMIO32_WRITE(base_addr | QSPI_HDR_COUNT_REG , i + (addr_cycles << ADDR_CNT) + (dummy << DUMMY_CNT));
    /* Program SPI_ADDR */
    MV_MEMIO32_WRITE(base_addr | QSPI_ADDR_REG  , addr);
    /* Program SPI Cfg reg FIFO_MODE = 1, RW_EN = 1, XFER_START = 1 */
    MV_MEMIO32_WRITE(base_addr | QSPI_CFG_REG  , MV_MEMIO32_READ(base_addr | QSPI_CFG_REG) | (1 << RW_EN) | (1 << XFER_START));

    /* if buffer is empty, just wait for header transmit done */
    if (size == 0)
        while (!(MV_MEMIO32_READ(base_addr) & (1 << SPI_RDY)));

    /* send bytes from buffer to FIFO */

    if (size % 4)
        size += 4;
    size >>= 2; /* We send bytes in 32bit words */

    while (size-- > 0) { /* Fill Fifo */
        i = 100;
        while (MV_MEMIO32_READ(base_addr) & (1 << WFIFO_FULL))
        {
            srvCpuOsUsDelay(DELAY_32_BITS_IN_MILLISEC);
            if (i-- < 1)
                return MV_TIMEOUT;
        }
        MV_MEMIO32_WRITE(base_addr | QSPI_DATA_OUT_REG , *buffer32++);
    }
    /* Wait until Fifo is empty, transaction is done */
    i = 200;
    while (!(MV_MEMIO32_READ(base_addr) & (1 << WFIFO_EMPTY)))
    {
        srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
            if (i-- < 1)
                return MV_TIMEOUT;
    }

    /* Program SPI Cfg reg XFER_STOP = 1 */
    MV_MEMIO32_WRITE(base_addr | QSPI_CFG_REG , MV_MEMIO32_READ(base_addr | QSPI_CFG_REG) | (1 << XFER_STOP));
    /* Wait until XFER DONE before de-asserting CS */
    i = 100;
    while (!(MV_MEMIO32_READ(base_addr) & (1 << SPI_XFER_DONE)))
    {
        srvCpuOsUsDelay(DELAY_8_BITS_IN_MILLISEC);
        if (i-- < 1)
            return MV_TIMEOUT;
    }
    /* De-assert SPI_CS0 */
    MV_MEMIO32_WRITE(base_addr , MV_MEMIO32_READ(base_addr) & ~(1 << SPI_EN_0));
    return MV_OK;
}
