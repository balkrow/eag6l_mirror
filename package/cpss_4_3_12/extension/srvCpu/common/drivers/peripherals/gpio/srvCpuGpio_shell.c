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
* @file srvCpuGpio_shell.c
*
* @brief This file includes implementation of GPIO shell
* 
*
* @version   1
********************************************************************************
*/
#include <string.h>
#include <printf.h>
#include <global.h>
#include "srvCpuGpio.h"
#include "srvCpuGpio_shell.h"
#include <FreeRTOS_CLI.h>

/************************************************************************************
* prvGpioGrpParseCommand -  This routine parse read/write/config gpio group commands
************************************************************************************/
static portBASE_TYPE prvGpioGrpParseCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{

    MV_U32 gpioGrp = 0, valueToWrite = 0;
    enum {GPIO_CMD_TYPE_READ, GPIO_CMD_TYPE_WRITE, GPIO_CMD_TYPE_CONFIG} gpio_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "gpio_grp") ) )
        goto parseGpioCommand_fail;

    /* Get the command type: r/w/c. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    if (!strcmp(value, "r"))
       gpio_cmd_type = GPIO_CMD_TYPE_READ;
    else if (!strcmp(value, "w"))
        gpio_cmd_type = GPIO_CMD_TYPE_WRITE;
    else if (!strcmp(value, "c"))
        gpio_cmd_type = GPIO_CMD_TYPE_CONFIG;
    else
        goto parseGpioCommand_fail;

    /* GPIO NUMBER */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    gpioGrp = (MV_U32)strtoul(value, &value, 0);
    if (gpioGrp > 1)
        goto parseGpioCommand_fail;

    if (gpio_cmd_type == GPIO_CMD_TYPE_READ)
    {
        printf("GPIO #%d: 0x%08x\n",gpioGrp, srvCpuGpioInRead(gpioGrp));
    }
    else
    {/* gpio_cmd_type == GPIO_CMD_TYPE_WRITE || GPIO_CMD_TYPE_CONFIG */
        /* value to write */
        value = strtok( NULL, DELIMETERS );
        if (value == NULL )
        goto parseGpioCommand_fail;
        valueToWrite = (MV_U32)strtoul(value, &value, 0);
        if(gpio_cmd_type == GPIO_CMD_TYPE_WRITE)
        {
            srvCpuGpioOutWrite(gpioGrp, valueToWrite);
            printf("Done set GPIO #%d pins in value %p\n", gpioGrp, valueToWrite);
        }
        else
        {/*gpio_cmd_type == GPIO_CMD_TYPE_CONFIG*/
            srvCpuGpioInOutConfig(gpioGrp, valueToWrite);
            printf("Done Configure GPIO #%d pins in value %p\n", gpioGrp, valueToWrite);
        }
    }

    return pdFALSE;
parseGpioCommand_fail:
    printf( "Error: Wrong input.\nUsage: gpio_grp r/w/c <GPIO Group> <value - only in w/c>\n");
    printf( "Read/Write/Config, GPIO Group: 0/1, value as bitmap, Configure Pins to IN/OUT as bitmap, where IN (1) / OUT (0)\n");
    return pdFALSE;
}

/************************************************************************************
* prvGpioPinParseCommand -  This routine parse read/write/config gpio pin commands
************************************************************************************/
static portBASE_TYPE prvGpioPinParseCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{

    MV_U32 gpioPin = 0, valueToWrite = 0;
    enum {GPIO_CMD_TYPE_READ, GPIO_CMD_TYPE_WRITE, GPIO_CMD_TYPE_CONFIG} gpio_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "gpio_pin") ) )
        goto parseGpioCommand_fail;

    /* Get the command type: r/w/c. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    if (!strcmp(value, "r"))
       gpio_cmd_type = GPIO_CMD_TYPE_READ;
    else if (!strcmp(value, "w"))
        gpio_cmd_type = GPIO_CMD_TYPE_WRITE;
    else if (!strcmp(value, "c"))
        gpio_cmd_type = GPIO_CMD_TYPE_CONFIG;
    else
        goto parseGpioCommand_fail;

    /* GPIO PIN */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseGpioCommand_fail;
    gpioPin = (MV_U32)strtoul(value, &value, 0);
    if (gpioPin > 48)
        goto parseGpioCommand_fail;

    if (gpio_cmd_type == GPIO_CMD_TYPE_READ)
    {
        printf("GPIO pin#%d: 0x%x\n",gpioPin, srvCpuGpioPinRead(gpioPin));
    }
    else
    {/* gpio_cmd_type == GPIO_CMD_TYPE_WRITE || GPIO_CMD_TYPE_CONFIG */
        /* value to write */
        value = strtok( NULL, DELIMETERS );
        if (value == NULL )
        goto parseGpioCommand_fail;
        valueToWrite = (MV_U32)strtoul(value, &value, 0);
        if(gpio_cmd_type == GPIO_CMD_TYPE_WRITE)
        {
            srvCpuGpioPinWrite(gpioPin, valueToWrite);
            printf("Done set GPIO pin#%d in value 0x%x\n", gpioPin, valueToWrite);
        }
        else
        {/*gpio_cmd_type == GPIO_CMD_TYPE_CONFIG*/
            srvCpuGpioPinConfig(gpioPin, valueToWrite);
            printf("Done Configure GPIO pin#%d in value 0x%x\n", gpioPin, valueToWrite);
        }
    }

    return pdFALSE;
parseGpioCommand_fail:
    printf( "Error: Wrong input.\nUsage: gpio_pin r/w/c <GPIO PIN> <value - only in w/c>\n");
    printf( "Read/Write/Config, GPIO PIN: 0-48, Configure Pin to IN (1) / OUT (0)\n\n");
    return pdFALSE;
}

static const CLI_Command_Definition_t xGpioGrpCommand =
{
    ( const char * const ) "gpio_grp",
    ( const char * const ) "gpio_grp r/w/c <GPIO Group> <value>\t"
            "Read/Write/Config, GPIO Group: 0-1, value as bmp, config as bmp IN(1)/OUT(0)\n",
    prvGpioGrpParseCommand,
    -1
};

static const CLI_Command_Definition_t xGpioPinCommand =
{
    ( const char * const ) "gpio_pin",
    ( const char * const ) "gpio_pin r/w/c <GPIO PIN> <value>\t"
            "Read/Write/Config single pin (0-48), value 0/1 or in:1 out:0\n",
    prvGpioPinParseCommand,
    -1
};

void registerGpioCliCommands()
{
    FreeRTOS_CLIRegisterCommand( &xGpioPinCommand);
    FreeRTOS_CLIRegisterCommand( &xGpioGrpCommand);
}