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
* @file srvCpuWatchdog_shell.c
*
* @brief  This file includes implementation of Watchdog shell
* 
*
* @version   1
********************************************************************************
*/

#include <printf.h>
#include <global.h>
#include <FreeRTOS_CLI.h>
#include <srvCpuServices.h>
#include "srvCpuWatchdog.h"
#include "srvCpuWatchdog_shell.h"

static portBASE_TYPE prvTriggerResetCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    /* Reset cm3 */
    printf("Reset CM3 by triggering Watchdog mechanism\n");
    srvCpuWatchdogInit(1);
    return pdFALSE;
}
static portBASE_TYPE prvTriggerWatchdogInterruptCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    /* run infinity loop in order to get stuck and trigger WatchDog interrupt */
    while (0xa5a5a5a5 != srvCpuRegRead(0));
    return pdFALSE;
}

static portBASE_TYPE prvStopWatchdogInterruptCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    srvCpuWatchdogStop();
    return pdFALSE;
}

static portBASE_TYPE prvStartWatchdogInterruptCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    srvCpuWatchdogStart();
    return pdFALSE;
}

static const CLI_Command_Definition_t xTriggerResetCommand =
{
    ( const char * const ) "reset-cm3",
    ( const char * const ) "reset-cm3\t\t\t\t"
            "Run CM3 reset command\n",
    prvTriggerResetCommand,
    0
};
static const CLI_Command_Definition_t xTriggerWatchdogInterruptCommand =
{
    ( const char * const ) "trigger-watchdog",
    ( const char * const ) "trigger-watchdog\t\t\t"
            "Generates watchdog interrupt\n",
    prvTriggerWatchdogInterruptCommand,
    0
};

static const CLI_Command_Definition_t xStopWatchdogCommand =
{
    ( const char * const ) "stop-watchdog",
    ( const char * const ) "stop-watchdog\t\t\t\t"
            "Stop watchdog interrupt\n",
    prvStopWatchdogInterruptCommand,
    0
};

static const CLI_Command_Definition_t xStartWatchdogCommand =
{
    ( const char * const ) "start-watchdog",
    ( const char * const ) "start-watchdog\t\t\t\t"
            "Start watchdog interrupt\n",
    prvStartWatchdogInterruptCommand,
    0
};

void registerWatchdogCliCommands()
{
    FreeRTOS_CLIRegisterCommand( &xTriggerResetCommand);
    FreeRTOS_CLIRegisterCommand( &xTriggerWatchdogInterruptCommand);
    FreeRTOS_CLIRegisterCommand( &xStopWatchdogCommand);
    FreeRTOS_CLIRegisterCommand( &xStartWatchdogCommand);
}