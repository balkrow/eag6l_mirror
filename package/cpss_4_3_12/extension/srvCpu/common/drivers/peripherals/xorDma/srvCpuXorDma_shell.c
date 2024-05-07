/*******************************************************************************
Copyright (C) 2016 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Marvell nor the names of its contributors may be
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

#include <string.h>
#include <printf.h>
#include <FreeRTOS.h>
#include <FreeRTOS_CLI.h>
#include <global.h>
#include "srvCpuXorDma_shell.h"
#include "srvCpuXorDma.h"

static MV_STATUS convert64bitTo32(char* value, MV_U32 *data)
{
    int i, index = 2, len = 0;
    char buf[20] = {0}, buf32_l[10] = {0}, buf32_h[10] ={0};

    for(len = 0; value[len]!=0;len++);
    memcpy(buf,value,len);

    /* check if number is in Hex */
    if((buf[0]=='0') && (buf[1]=='x' || buf[1]=='X'))
    {
        memcpy(buf32_l,"0x0",3);
        memcpy(buf32_h,"0x0",3);

        if(len > 10)
        {
            for(i=index;i<len-8;i++)
                buf32_h[i]=buf[i];

            for(i=len-8; i<len; i++,index++)
                buf32_l[index] = buf[i];
        }
        else
        {
            for(i=index;i<len;i++)
            {
                buf32_l[i]=buf[i];
            }
        }

        /* convert strings to unsigned long */
        data[0] = (MV_U32)strtoul(buf32_h, NULL, 0);
        data[1] = (MV_U32)strtoul(buf32_l, NULL, 0);
        return MV_OK;
    }

    return MV_BAD_VALUE;
}

/*******************************************************************************
* prvXorDmaCommand -  This routine parse cpy/cmp (using xor dma) commands
*******************************************************************************/
static portBASE_TYPE prvXorDmaParseCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{

    /* src - in cpy/cmp is used as source, in set_pattern is used as pattern*/
    MV_U32 src[2] = {0}, dst[2] = {0}, size = 0;
    enum {XOR_DMA_CMD_TYPE_COPY, XOR_DMA_CMD_TYPE_COMPARE, XOR_DMA_CMD_TYPE_SET_PATTERN} xorDmaCmdType;
    char *value;
    MV_STATUS ret;
    int compResult;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "xor_dma") ) )
        goto parseXorDmaCommand_fail;

    /* Get the command type: cpy/cmp. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseXorDmaCommand_fail;
    if (!strcmp(value, "cpy"))
       xorDmaCmdType = XOR_DMA_CMD_TYPE_COPY;
    else if (!strcmp(value, "cmp"))
        xorDmaCmdType = XOR_DMA_CMD_TYPE_COMPARE;
    else if (!strcmp(value, "set_pattern"))
        xorDmaCmdType = XOR_DMA_CMD_TYPE_SET_PATTERN;
    else
        goto parseXorDmaCommand_fail;

    /* Source / Pattern */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseXorDmaCommand_fail;
    /* getting number in 64 bit and convert it to two 32 bits variables */
    ret = convert64bitTo32(value, src);
    if (ret)
        goto parseXorDmaCommand_fail;

    /* Destination */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseXorDmaCommand_fail;
    /* getting number in 64 bit and convert it to two 32 bits variables */
    ret = convert64bitTo32(value, dst);
    if (ret)
        goto parseXorDmaCommand_fail;

    /* Size */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseXorDmaCommand_fail;
    size = (MV_U32)strtoul(value, &value, 0);

    if (xorDmaCmdType == XOR_DMA_CMD_TYPE_COPY)
    {
        ret = srvCpuXorDmaOperation(SRV_CPU_DESC_OP_MODE_MEMCPY_E, src, dst, size, 0x7FE00000, NULL);
        if (!ret)
            printf("Succeed copy using XOR_DMA engine\n");
        else
            printf("Failed to copy using XOR_DMA engine\n",src, dst);
    }
    else if(xorDmaCmdType == XOR_DMA_CMD_TYPE_COMPARE)
    {
        ret = srvCpuXorDmaOperation(SRV_CPU_DESC_OP_MODE_MEMCMP_E, src, dst, size, 0x7FE00000, &compResult);
        if (ret != MV_OK)
            goto parseXorDmaCommand_fail;
        else  
        {
            if (!compResult)
                printf("XOR DMA Result:\nThe two memory addresses are equal\n");
            else
                printf("XOR DMA Result:\n There are %d differences between the two memory addresses\n",compResult);
        }
    }
    else
    {/* xorDmaCmdType == XOR_DMA_CMD_TYPE_SET_PATTERN  */
        ret = srvCpuXorDmaOperation(SRV_CPU_DESC_OP_MODE_MEMSET_E, src, dst, size, 0x7FE00000, NULL);
        if (!ret)
            printf("Succeed to set pattern using XOR_DMA engine\n");
        else
            printf("Failed to set pattern using XOR_DMA engine\n");
    }
    return pdFALSE;
parseXorDmaCommand_fail:
    printf( "error: Wrong input. Usage:\n xor_dma cpy/cmp/set_pattern <src|pattern> <dst> <size>\n" );
    return pdFALSE;
}

static const CLI_Command_Definition_t xXorDmaCommand =
{
    ( const char * const ) "xor_dma",
    ( const char * const ) "xor_dma cpy/cmp/set_pattern <src|pattern> <dst> <size>\n"
            "\t\t\t\t\tCopy/Compare/Set Pattern (64bit in Hex), using XOR DMA engine\n",
    prvXorDmaParseCommand,
    -1
};

void registerXorDmaCommand()
{
    FreeRTOS_CLIRegisterCommand( &xXorDmaCommand);
}