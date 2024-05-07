/*
    FreeRTOS V7.3.0 - Copyright (C) 2012 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest versions, license
    and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

#ifdef ASIC_SIMULATION
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#endif

/* Scheduler includes. */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

/* Demo app includes. */
#include <stdint.h>
#include <string.h>
#include <FreeRTOS_CLI.h>
#include <portmacro.h>
#include <hw.h>
#include <ic.h>
#include <uart.h>
#include <global.h>

/* Demo app includes drivers */
#include <srvCpuServices.h>
#include <prvSrvCpuServices.h>
#include <srvCpuIpc.h>
#include <srvCpuWatchdog.h>
#include <srvCpuSdma.h>
/* shell */
#include <srvCpuMdio_shell.h>
#include <srvCpuGpio_shell.h>
#include <srvCpuI2c_shell.h>
#include <srvCpuIpc_shell.h>
#include <srvCpuQspi_shell.h>
#include <srvCpuWatchdog_shell.h>
#include <srvCpuDoorbell_shell.h>
#include <srvCpuXorDma_shell.h>
#include <cli.h>
#include <printf.h>

#define SDK_VER     "1.0.8"

/* needed by mvShmUart */
const MV_BOOL enableUart = MV_TRUE;
extern unsigned char *g_scpu_sram_ptr, *g_cnm_sram_ptr;

void iICSetSimulationtreeId(unsigned treeId);

void iICSetSimulationMGId(unsigned mgId);
unsigned char *wm_sim_get_sram_base_addr(unsigned scpu_id, unsigned size, int *fd_ptr);
int sim_update_mg_base(char *devname);

/* Memory Dump and Write commands */
#define GETMEM( x, o )      (*(( volatile unsigned * )( x + o )))
static portBASE_TYPE prvDumpCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    static unsigned long base, size = 0;
    char * ptr;

    if ( size == 0 ) {
        ptr = ( char * )&pcCommandString[ 2 ];
        base = strtoul(( const char * )ptr, &ptr, 0 );
        size = strtoul(( const char * )ptr, &ptr, 0 );
    }

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "%p: %08x %08x %08x %08x  %08x %08x %08x %08x\n",
            base, GETMEM( base, 0 ), GETMEM( base, 4 ), GETMEM( base, 8 ), GETMEM( base, 12 ),
            GETMEM( base, 16 ), GETMEM( base, 20 ), GETMEM( base, 24 ), GETMEM( base, 28 ));
    base += 32;
    size = ( size >= 32 ? size - 32 : 0 );
    if ( size )
        return pdTRUE;
    else
        return pdFALSE;
}

static portBASE_TYPE prvWriteCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    unsigned long base, value;
    char * ptr;

    ptr = ( char * )&pcCommandString[ 2 ];
    base = strtoul(( const char * )ptr, &ptr, 0 );
    value = strtoul(( const char * )ptr, &ptr, 0 );

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "Write value %p to memory %p.\n", value, base );
    GETMEM( base, 0 ) = value;

    return pdFALSE;
}

static portBASE_TYPE prvRegReadCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    static unsigned long base;
    char * ptr;

    ptr = ( char * )&pcCommandString[ 5 ];
    base = strtoul(( const char * )ptr, &ptr, 0 );

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "%p: %08x \n", base, srvCpuRegRead( base));

    return pdFALSE;
}

static portBASE_TYPE prvRegWriteCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    unsigned long base, value;
    char * ptr;

    ptr = ( char * )&pcCommandString[ 5 ];
    base = strtoul(( const char * )ptr, &ptr, 0 );
    value = strtoul(( const char * )ptr, &ptr, 0 );

    ptr = ( char * )pcWriteBuffer;
    sprintf( ptr, "Write value %p to memory %p.\n", value, base );
    srvCpuRegWrite(base, value);

    return pdFALSE;
}

static portBASE_TYPE prvMppDisplayCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    prvSrvCpuMppDisplay();
    return pdFALSE;
}

static const CLI_Command_Definition_t xDumpCommand =
{
    ( const char * const ) "md",
    ( const char * const ) "md <address> <byte count>\t\t"
            "Dump memory region to console\n",
    prvDumpCommand,
    -1
};

static const CLI_Command_Definition_t xWriteCommand =
{
    ( const char * const ) "mw",
    ( const char * const ) "mw <address> <value>\t\t\t"
            "Write value to memory\n",
    prvWriteCommand,
    2
};

static const CLI_Command_Definition_t xRegReadCommand =
{
    ( const char * const ) "reg r",
    ( const char * const ) "reg r <address>\t\t\t\t"
            "Read register\n",
    prvRegReadCommand,
    2
};

static const CLI_Command_Definition_t xRegWriteCommand =
{
    ( const char * const ) "reg w",
    ( const char * const ) "reg w <address> <value>\t\t\t"
            "Write register\n",
    prvRegWriteCommand,
    3
};

static const CLI_Command_Definition_t xMppDisplayCommand =
{
    ( const char * const ) "MPP_Display",
    ( const char * const ) "MPP_Display\t\t\t\t"
            "Display MPP status\n",
    prvMppDisplayCommand,
    0
};

/*-----------------------------------------------------------*/
#ifdef ASIC_SIMULATION
/*
  Update the simulation MG base address
  according to the device type:
*/
int sim_update_mg_base(char *devname)
{
    if (!strcmp(devname, "ac5"))
        sim_mg_base = 0x7F900000;
    else if (!strcmp(devname, "ac5p"))
        sim_mg_base = 0x3C200000;
    else if (!strcmp(devname, "falcon"))
        sim_mg_base = 0x3C200000;
    else return -1;

    return 0;
}

int main(int argc, char *argv[])
#else
int main( void )
#endif
{
#ifdef ASIC_SIMULATION
    unsigned mg_id = 0,
             sync_tcp_port = DEFAULT_TCP_PORT_SYNC,
             async_tcp_port = DEFAULT_TCP_PORT_ASYNC;

    if (argc < 2)
    {
        char *errmsg = "Syntax: <progname> <devname> <irq_treeid> <mg_id> <sync tcp port> <async tcp port>\n"
                       "Where <devname> can be: ac5 ac5p falcon\n";
        write(0, errmsg, strlen(errmsg));
        return -1;
    }

    /* Update the simulation parameters according to
       the command line parameters: */
    if (sim_update_mg_base(argv[1]) < 0)
    {
        char *errmsg = "Invalid devname.\n"
                       "Syntax: <progname> <devname> <irq_treeid> <mg_id> <sync tcp port> <async tcp port>\n"
                       "Where <devname> can be: ac5 ac5p falcon\n";
        write(0, errmsg, strlen(errmsg));
        return -2;
    }

    if (argc > 2)
        iICSetSimulationtreeId(atoi(argv[2]));
    if (argc > 5)
    {
        sync_tcp_port = atoi(argv[4]);
        async_tcp_port = atoi(argv[5]);
    }

    if (argc > 3)
    {
        mg_id = atoi(argv[3]);
        vICSimInit(sync_tcp_port, async_tcp_port); /* must initialize sockets to simulation before we potentially set the MG Source ID register in the simulation */

        /* update the mg base address to reflect the md id
         * and also write the mg id to the MG source ID
         * register for detection as the simulation does
         * not update this field. */
        sim_mg_base += (0x100000 * mg_id);
        writeRegister(iICGetSimulationSocket(), (sim_mg_base + SRV_CPU_MG_SOURCE_ID), (4 + mg_id));
        iICSetSimulationMGId(mg_id);
    }
    else
        vICSimInit(sync_tcp_port, async_tcp_port); /* must initialize sockets to simulation before we potentially set the MG Source ID register in the simulation */
#endif

    /* Set parameters according to device type */
    if (!setDeviceParameters())
        return -1;

    srvCpuSysMapInit();

    /* Initialize SCPU features */
    srvCpuMandatoryInit();
    printf( "\n\nFreeRTOS %s - cm3 SDK %s, %s\n\n",
            tskKERNEL_VERSION_NUMBER, SDK_VER, __DATE__);

#ifdef ASIC_SIMULATION
    printf("CM3 WM simulated. Syntax: %s devname irq_tree_id mg_id\n", argv[0]);
    printf("min stack size is %u, CLI size is %d\n",
		    configMINIMAL_STACK_SIZE, CLI_MAX_CMD);

    printf(" SCPU SRAM ptr is %p, CnM SRAM ptr is %p\n",
           g_scpu_sram_ptr, g_cnm_sram_ptr);
#endif

#ifndef DISABLE_CLI
    FreeRTOS_CLIRegisterCommand( &xDumpCommand );
    FreeRTOS_CLIRegisterCommand( &xWriteCommand );
    FreeRTOS_CLIRegisterCommand( &xRegReadCommand );
    FreeRTOS_CLIRegisterCommand( &xRegWriteCommand );
    FreeRTOS_CLIRegisterCommand( &xMppDisplayCommand );
#ifdef TWSI_SUPPORT
    /* Register 'i2c_init' and 'i2c r/w/probe' commands */
    registerI2cCommands();
#endif

#ifdef SDK_DEBUG
    /* Register invoke doorbell interrupt command */
    registerInvokeDoorbellCommand();
    #ifdef WATCHDOG /* SDK_DEBUG && WATCHDOG */
    /* Register watchdog commands: start, stop, reset etc. */
    registerWatchdogCliCommands();
    #endif
#endif /* SDK_DEBUG */

    /* Register 'gpio r', 'gpio w' and 'gpio c' commands */
    registerGpioCliCommands();

#if defined(QSPI) || defined(SPI)
    /* Register 'qspi_init' and 'qspi r/w' commands */
    registerQspiCommands();
#endif

#ifdef XOR_DMA
    /* Register 'xor_dma' commands */
    registerXorDmaCommand();
#endif

#ifdef SDMA_SUPPORT
    /* Register sdma commands: sdma-init, send-packet, etc. */
    registerSdmaCliCommands();
#endif

#ifdef IPC_CLIENT
    /* Register IPC Client commands */
    registerIpcCliCommands();
#endif
    /* Register SMI XSMI commands */
    registerMdioCliCommands();

#endif /* DISABLE CLI */

    /* Start the scheduler */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient heap to start the
    scheduler. */

    return 0;
}

