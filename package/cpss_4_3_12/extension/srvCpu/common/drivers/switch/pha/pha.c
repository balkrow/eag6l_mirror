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
* @file mvMdio.c
*
* @brief MDIO driver for SMI and XSMI interfaces
*
*
* @version   1
********************************************************************************
*/
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <printf.h>
#include <pha.h>
#include <srvCpuServices.h>
#include <hw.h>
#include <fileOps.h>
#include <mvSFlash.h>
#include <FreeRTOS_CLI.h>

/* indication that thread is not valid/not implemented */
#define HA_FW_INSTRUCTION_POINTER_NOT_VALID_CNS   0xFFFFFFFF

/* ALL addresses of instruction pointers need to be with prefix 0x00400000 */
/* the PHA table need to hold only lower 16 bits (the prefix is added internally by the HW) */
#define HA_FW_INSTRUCTION_DOMAIN_ADDR_CNS      0x00400000
#define HA_FW_INSTRUCTION_DOMAIN_ADDR_MAX_CNS  0x0040FFFF
#define PHA_PPA_CTRL_REG 0x7eff00
#define PHA_CTRL_REG 0x7fff00
#define PHA_STATUS_REG (PHA_CTRL_REG + 4)
#define PHA_NULL_PROC_INST_REG (PHA_CTRL_REG + 0x20);
#define PHA_PPA_THREAD_CONF_REG 0x7f0000

/* max number of threads that the firmware supports */
#define MAX_THREADS_CNS 32
static MV_U32  miPhaDoNothing_PC = 0;

/**
 *
 * srvCpuPhaVersionGet
 *
 * @brief return current PIPE FW version
 *
 * @param devNum
 *
 * @return if success:32-bit PHA version from PPN space,
 *         0-otherwise
 */
MV_U32 srvCpuPhaVersionGet(IN MV_U8 devNum)
{
    /* Reading the current PIPE FW version */
    return srvCpuRegRead(0x0F000004);
}

/**
 *
 * miGetHaAddresses
 *
 * @param devNum
 * @param cptrHaThrPcArr
 *
 * @return GT_STATUS
 */
MV_STATUS miGetHaAddresses(
    IN MV_U8 devNum,
    OUT MV_U8* ptrHaThrPcArr)
{
    if (NULL != ptrHaThrPcArr) {
        memcpy(ptrHaThrPcArr,(const void *)&miPhaDoNothing_PC, sizeof(miPhaDoNothing_PC));
        return MV_OK;
    }
    return MV_FAIL;
}

extern MV_U32 remapWindow(MV_U32 offset);
/**
 *
 * srvCpuPhaFwDownload
 *
 * @return MV_STATUS
 */
static MV_STATUS srvCpuPhaFwDownload(void)
{
    MV_STATUS   rc = MV_ERROR;
    MV_U8  ppg;
    MV_U32 regAddr, fwData[4], size;
    MV_U32 ii = 0;
    file_params_STC file_params_ptr;
    MV_BOOL     restart = MV_TRUE;

    /* Search for PHA FW file in the super image */
    while (MV_TRUE == getNextFile(&file_params_ptr, restart))
    {
        restart = MV_FALSE;

        if (file_params_ptr.type == PHA_FW)
        {
            size = file_params_ptr.size / 4;
            if (size % 4 || size > 0x1000 ) {
                return MV_FAIL;
            }
            for(ppg = 0; ppg < devices_details[devIndex].ppgNum; ppg++) {
                regAddr = 0x40000 + 0x80000 * ppg;
                remapWindow((devices_details[devIndex].phaPpgBase << 24) + (regAddr & 0xfff00000));
                for(ii = 0 ; ii < size; ii += 4)
                {
                    rc = mvSFlashFastBlockRd(0, file_params_ptr.address + ii * 4, (MV_U8*)fwData, 16);
                    if (rc != MV_OK)
                        return rc;
                    /* Need to write 4 words in a row, without anything in between
                     * This is a temporary solution until such API is provided */
                    MV_MEMIO32_WRITE(SRV_CPU_SW_WIN_BASE | (regAddr + 0x0), fwData[0]);
                    MV_MEMIO32_WRITE(SRV_CPU_SW_WIN_BASE | (regAddr + 0x4), fwData[1]);
                    MV_MEMIO32_WRITE(SRV_CPU_SW_WIN_BASE | (regAddr + 0x8), fwData[2]);
                    MV_MEMIO32_WRITE(SRV_CPU_SW_WIN_BASE | (regAddr + 0xc), fwData[3]);
                    regAddr += 16;
                }
            }
        }
        if (file_params_ptr.type == HA_THR_PC)
        {
            rc = mvSFlashFastBlockRd(0, file_params_ptr.address, (MV_U8*)(&miPhaDoNothing_PC), 4);
            if (rc != MV_OK)
                return rc;
        }
    }

    if (ii == 0 || miPhaDoNothing_PC == 0)
    {
        printf("Pha FW file/s not found!\n");
        return MV_ERROR;
    }

    /* set field <ppa_imem_hold_off> to :
       0x0 = No HoldOff; IMEM responds with data */
    regAddr = (devices_details[devIndex].phaPpgBase << 24) + PHA_PPA_CTRL_REG;
    srvCpuRegWrite(regAddr, 0);
    return MV_OK;
}

/**
 *
 * srvCpuPhaTableInitDoNothing
 *
 * @brief Initialize HA Table to "Do Nothing" for all ports
 *
 * @return MV_STATUS
 */
MV_STATUS srvCpuPhaTableInitDoNothing(MV_VOID)
{
    MV_STATUS   rc = MV_ERROR;
    MV_U32 i,j;
    MV_U32 regAddr;

    /* Part 1: allow PHA processing by disabling Bypass */

    regAddr = (devices_details[devIndex].phaPpgBase << 24) + PHA_CTRL_REG;
    srvCpuRegWrite(regAddr, 0);

    /* Part 2: download PHA FW comes from super image */

    if((rc = srvCpuPhaFwDownload()) != MV_OK) {
        printf("Pha FW Download fail: rc = %d\n", rc);
        return MV_FAIL;
    }

    /* Part 3: initialize HA Thread PC array with values uploaded from Super Image config file
     * Not needed in MI, as configuration currently done using text file (trace file)
     */
#if 0
    for (i = 0; i < MAX_THREADS_CNS; i++) {
        /* The unsupported Thread PC's HA Address should be zeroed */
        miPhaTableThread_PC[i] =  (0 == threadPcDataArr[i]) ? HA_FW_INSTRUCTION_POINTER_NOT_VALID_CNS : threadPcDataArr[i];
    }
#endif
    /* Part 4: initialize HA Table to "DoNothing" - with thread 28 instruction pointer for all ports include CPU */

    regAddr = (devices_details[devIndex].phaPpgBase << 24) + PHA_PPA_THREAD_CONF_REG + 0x8000;

    /* skip entry #0 */
    regAddr += 32;
    for (i = 1; i < 256; i++)
    {
        /* 5 words per line */
        for (j = 0; j < 4; j++)
        {
            srvCpuRegWrite(regAddr, 0);
            regAddr += 4;
        }
        /* For 5-th word,- write the THR_28 DoNothing PC */
        srvCpuRegWrite(regAddr, (miPhaDoNothing_PC & (~HA_FW_INSTRUCTION_DOMAIN_ADDR_CNS)));
        regAddr += 16;
    }

    /* Write DoNothing PC to the Null Processing Instruction Pointer */
    regAddr = (devices_details[devIndex].phaPpgBase << 24) + PHA_NULL_PROC_INST_REG;
    srvCpuRegWrite(regAddr, (miPhaDoNothing_PC & (~HA_FW_INSTRUCTION_DOMAIN_ADDR_CNS)));

    printf("PHA init succeeded.\n");
    return MV_OK;
}


