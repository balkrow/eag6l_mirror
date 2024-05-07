/*******************************************************************************
   Copyright (C) Marvell International Ltd. and its affiliates
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
*******************************************************************************/

#ifdef ASIC_SIMULATION
#include <unistd.h>
#endif
#include <hw.h>
#include <FreeRTOS_CLI.h>
#include <printf.h>
#include <string.h>
#include <srvCpuServices.h>
#include <global.h>
#include "srvCpuIpc.h"
#include "srvCpuIpc_shell.h"

unsigned char *wm_sim_get_sram_base_addr(unsigned scpu_id, unsigned size, int *fd_ptr);
#ifdef ASIC_SIMULATION
static int sram_fd = -1;
#endif

static unsigned long get_mg_sram_base(unsigned mg_id, unsigned sramSize)
{
    unsigned long base;

#ifdef ASIC_SIMULATION
    unsigned char *ptr;
    if (sram_fd > 0)
    {
	close(sram_fd);
	sram_fd = -1;
    }
    ptr = wm_sim_get_sram_base_addr(mg_id, sramSize, &sram_fd);
    base = (unsigned long)ptr;
#else
    base = 0xa0400000;
#endif

    return base;
}

/**
* @internal prvIpcClientInitCommand function
* @endinternal
*
* @brief Initialize IPC to connect to wanted core
*
* @Parameters from user: tgtId - the core to connect
*
* @retval pdFALSE
*/
static portBASE_TYPE prvIpcClientInitCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{
    MV_U8 tgtId = 0;
    int addr = 0, port = 0;
    char *value;


    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the core id */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseIpcClientInitCommand_fail;
    tgtId = (MV_U8)strtoul(value, &value, 0);
    /*if (deviceType == SRV_CPU_DEVICE_TYPE_FALCON_E)
        tgtId -= 16;*/

    /* Check that not connecting to self core */
    if( mgOwnId == tgtId )
        goto parseIpcClientInitCommand_fail;

    addr = srvCpuGetMgBase(tgtId) + CM3_SRAM_OFFSET;
    port = tgtId + 4;

#if 0 /* Falcon not supported yet. */
    if (deviceType == SRV_CPU_DEVICE_TYPE_FALCON_E)
    {
        addr = (tgtId > 1) ? FALCON_TILE2_MG_BASE_ADDRESS : FALCON_TILE0_MG_BASE_ADDRESS;
        /*
            each tile has 4 MG units where only one MG has CM3 unit
        */
        addr += ((tgtId % NUM_OF_CM3_IN_FALCON_DUAL_TILE)*4) * sramSize;
    }
#endif

    /* Configure window to access wanted CM3 SRAM */
    MV_MEMIO32_WRITE((MG_BASE | 0x4C0), 0x00300000 | port); /* Base addr + Target, accessed @ 0xa0400000 */
    MV_MEMIO32_WRITE((MG_BASE | 0x4C4), 0x70000); /* window Size: set to 384KB - which is suitable for 128KB-384KB */
    MV_MEMIO32_WRITE((MG_BASE | 0x4C8), 0x00000000); /* High base addr */
    MV_MEMIO32_WRITE((MG_BASE | 0x4CC), addr | 0xe); /* Remap base addr  */

    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_IRONMAN_E )
        srvCpuIpcClientInit((void*)((unsigned long)(get_mg_sram_base(tgtId, 0x8000) + (0x30000 - tgtId * 0x8000) - _2K)));
    else 
         srvCpuIpcClientInit((void*)((unsigned long)(get_mg_sram_base(tgtId, devices_details[devIndex].scpuSramSize * _1K) + (devices_details[devIndex].scpuSramSize * _1K) - _2K)));

    return pdFALSE;
parseIpcClientInitCommand_fail:
    printf( "Error: Wrong input. Usage:\n IPC_connect <core>\n");
    return pdFALSE;
}

/**
* @internal prvIpcSendMessageCommand function
* @endinternal
*
* @brief  parse IPC send message as Client command
*
* @Parameters from user: tgtId  - the core to connect
* @Parameters from user: message - data to send
*
* @retval pdFALSE
*/
static portBASE_TYPE prvIpcSendMessageCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{
    MV_U8 tgtId = 0;
    char *value;
    int len = 0;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the core id. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseIpcSendMessageCommand_fail;
    tgtId = (MV_U8)strtoul(value, &value, 0);

    /* Check that not connecting to self core */
    if( mgOwnId == tgtId)
        goto parseIpcSendMessageCommand_fail;

    /* get the message*/
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseIpcSendMessageCommand_fail;

    for(len = 0; value[len]!=0;len++);

    if (len >= IPC_MAX_MSG_SIZE - 4)
        goto parseIpcSendMessageCommand_fail;

    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_IRONMAN_E )
        srvCpuIpcClientSendMessage((void*)(unsigned long)(get_mg_sram_base(tgtId, 32*_1K) + (192*_1K - tgtId*32*_1K) - _2K), value, len, tgtId);
    else 
        srvCpuIpcClientSendMessage((void*)(unsigned long)(get_mg_sram_base(tgtId, devices_details[devIndex].scpuSramSize * _1K) + (devices_details[devIndex].scpuSramSize * _1K) - _2K), value, len, tgtId);

    return pdFALSE;
parseIpcSendMessageCommand_fail:
    printf( "Error: Wrong input. Usage:\n IPC_send_message <core> <message>\n");
    return pdFALSE;
}

static const CLI_Command_Definition_t xIpcClientInitCommand =
{
    ( const char * const ) "IPC_connect",
    ( const char * const ) "IPC_connect <core>\t\t\t"
            "Initialize IPC to connect to wanted core\n",
    prvIpcClientInitCommand,
    1
};

static const CLI_Command_Definition_t xIpcSentMessageCommand=
{
    ( const char * const ) "IPC_send_message",
    ( const char * const ) "IPC_send_message <core> <message>\t"
            "Sending IPC message to wanted core\n",
    prvIpcSendMessageCommand,
    2
};

void registerIpcCliCommands( void )
{
    FreeRTOS_CLIRegisterCommand( &xIpcClientInitCommand );
    FreeRTOS_CLIRegisterCommand( &xIpcSentMessageCommand );
}
