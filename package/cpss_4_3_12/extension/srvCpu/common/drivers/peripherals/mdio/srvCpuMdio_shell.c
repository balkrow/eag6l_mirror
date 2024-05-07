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
* @file srvCpuMdio_shell.c
*
* @brief  This file includes implementation of MDIO shell
* 
*
* @version   1
********************************************************************************
*/

#include <string.h>
#include <printf.h>
#include <global.h>
#include <FreeRTOS_CLI.h>
#include "srvCpuMdio.h"
#include "srvCpuMdio_shell.h"

/************************************************************************************
* prvMdioParseCommand -  This routine parse init/read/write mdio commands
************************************************************************************/
static portBASE_TYPE prvMdioParseCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{

    MV_U8 ifNum = 0, device, reg, speed, phy = 0;
    MV_U16 valueToWrite = 0, addr, readValue = 0;
    MV_STATUS rc;

    enum {IF_TYPE_SMI, IF_TYPE_XSMI} if_type = IF_TYPE_SMI;
    enum {SMI_CMD_TYPE_INIT, SMI_CMD_TYPE_READ, SMI_CMD_TYPE_WRITE} smi_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the interface type. */
    if (value != NULL)
    {
        if (!strcmp(value, "smi"))
            if_type = IF_TYPE_SMI;
        else if (!strcmp(value, "xsmi"))
            if_type = IF_TYPE_XSMI;
        else
            goto parseSmiCommand_fail;
    }
    else
        goto parseSmiCommand_fail;

    /* Get the command type: init/read/write */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseSmiCommand_fail;
    if (!strcmp(value, "init"))
       smi_cmd_type = SMI_CMD_TYPE_INIT;
    else if (!strcmp(value, "read"))
        smi_cmd_type = SMI_CMD_TYPE_READ;
    else if (!strcmp(value, "write"))
        smi_cmd_type = SMI_CMD_TYPE_WRITE;
    else
        goto parseSmiCommand_fail;

    /* Channel number */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseSmiCommand_fail;
    ifNum = (MV_U8)strtoul(value, &value, 0);
    if (ifNum > 1)
        goto parseSmiCommand_fail;

    /* Speed | device number */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseSmiCommand_fail;
    speed = (MV_U8)strtoul(value, &value, 0);
    if (smi_cmd_type == SMI_CMD_TYPE_INIT)
    {
        if (if_type == IF_TYPE_SMI)
        {
            if (speed > 6)
                goto parseSmiCommand_fail;
            rc = srvCpuSmiInit(ifNum, speed);
        }
        else /* xsmi */
        {
            if (speed > 3)
                goto parseSmiCommand_fail;
            rc = srvCpuXsmiInit(ifNum, speed);
        }
        if (rc != MV_OK)
            printf("SMI configuration failed! returned with error 0x%x\n", rc);
        return pdFALSE;
    }
    else
    {
        if (if_type == IF_TYPE_SMI)
            device = speed;
        else
        {
            phy = speed;
            /* Device number */
            value = strtok( NULL, DELIMETERS );
            if (value == NULL )
                goto parseSmiCommand_fail;
            device = (MV_U8)strtoul(value, &value, 0);
        }
    }

    /* reg */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL)
        goto parseSmiCommand_fail;
    reg = (MV_U8)strtoul(value, &value, 0);
    addr = (MV_U16)strtoul(value, &value, 0);

    if (device > 0x1f || reg > 0x1f)
        goto parseSmiCommand_fail;

    if (smi_cmd_type == SMI_CMD_TYPE_READ)
    {
        if (if_type == IF_TYPE_SMI) {
            rc = srvCpuSmiRead(ifNum, device, reg, &readValue);
            if (rc == MV_OK)
                printf("SMI ch %d, device %d 0x%02x: 0x%04x\n", ifNum, device, reg, readValue);
        }
        else {
            rc = srvCpuXsmiRead(ifNum, phy, device, addr, &readValue);
            if (rc == MV_OK)
                printf("XSMI ch %d, phy %d device %d 0x%04x: 0x%04x\n", ifNum, phy, device, reg, readValue);
        }
        if (rc != MV_OK)
            goto parseSmiCommand_fail;
    }

    else
    {/* SMI_CMD_TYPE_WRITE */
        /* value to write */
        value = strtok(NULL, DELIMETERS);
        if (value == NULL )
            goto parseSmiCommand_fail;
        valueToWrite = (MV_U16)strtoul(value, &value, 0);
        if (if_type == IF_TYPE_SMI)
        {
            rc = srvCpuSmiWrite(ifNum, device, reg, valueToWrite);
            if (rc != MV_OK)
                goto parseSmiCommand_fail;
        }
        else
        {
            rc = srvCpuXsmiWrite(ifNum, phy, device, reg, valueToWrite);
            if (rc != MV_OK)
                goto parseSmiCommand_fail;
        }
    }

    return pdFALSE;
parseSmiCommand_fail:
    if (if_type == IF_TYPE_SMI)
        printf( "Error: Wrong input. Usage:\nsmi init <ch> <speed (0-6)>\nsmi read / write <ch> <device (0-0x1f)> <reg (0-0x1f)> [value (0-0xffff)]\n");
    else
        printf( "Error: Wrong input. Usage:\nxsmi init <ch> <speed (0-3)>\nxsmi read / write <ch> <phy (0-0x1f) <device (0-0x1f)> <reg (0-0x1f)> [value (0-0xffff)]\n");
    return pdFALSE;
}

static const CLI_Command_Definition_t xSmiCommand =
{
    ( const char * const ) "smi",
    ( const char * const ) "smi init <ch> <speed>\t\t\tSet smi channel (0-1) speed: 0-6\n"
                           "smi read/write <ch> <device> <reg> <val>\tRead/Write smi channel (0-1)\n",
    prvMdioParseCommand,
    -1
};

static const CLI_Command_Definition_t xXsmiCommand =
{
    ( const char * const ) "xsmi",
    ( const char * const ) "xsmi init <ch> <speed>\t\t\tSet xsmi channel (0-2) speed: 0-3\n"
                           "xsmi read/write <ch> <phy> <device> <reg> <val>\n\t\t\t\t\tRead/Write xsmi channel (0-2)\n",
    prvMdioParseCommand,
    -1
};

void registerMdioCliCommands()
{
    FreeRTOS_CLIRegisterCommand( &xSmiCommand);
    FreeRTOS_CLIRegisterCommand( &xXsmiCommand);
}