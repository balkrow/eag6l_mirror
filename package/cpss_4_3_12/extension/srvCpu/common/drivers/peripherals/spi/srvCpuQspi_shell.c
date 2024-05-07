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

#include <hw.h>
#include <FreeRTOS_CLI.h>
#include <printf.h>
#include <string.h>
#include <srvCpuServices.h>
#include <global.h>
#include "srvCpuQspi_shell.h"
#include "srvCpuQspi.h"
#include "srvCpuSpi.h"
#include "prvSpiCmnd.h"

/*******************************************************************************
* xParseQspiInitCommand -  This routine initialize QSPI controller
*******************************************************************************/
static portBASE_TYPE xParseQspiInitCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{
    MV_U32 baudRate = 0, mode = 0, ch = 0, prot = 0;
    enum SRV_CPU_SPI_MODE_E spiMode;
    MV_STATUS rc;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );

    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "qspi_init") ) )
        goto parseQspiInitCommand_fail;

    /* Channel */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    ch = (MV_U32)strtoul(value, &value, 0);
    if (ch > 1)
        goto parseQspiInitCommand_fail;

    /*Baud rate*/
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    baudRate = (MV_U32)strtoul(value, &value, 0);
#ifdef QSPI
    /* Mode */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    mode = (MV_U32)strtoul(value, &value, 0);
    if (mode > 3)
        goto parseQspiInitCommand_fail;
    spiMode = (enum SRV_CPU_SPI_MODE_E)mode;

    /* Protocol */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiInitCommand_fail;
    prot = (MV_U32)strtoul(value, &value, 0);
    if (prot > 2)
        goto parseQspiInitCommand_fail;

    rc = srvCpuQspiInit(ch, baudRate, spiMode, prot);
#elif SPI
    spiMode = spiMode; /* to avoid compilation error */
    rc = mvSpiInit(ch, baudRate, devices_details[devIndex].systclk * 100000);
#endif
    if (rc != 0)
        printf("QSPI init failed, device is not supported\n");
    else
        printf("QSPI initialized successfully  with ch %d, baudRate %d, mode %d protocol %d \n", ch, baudRate, mode, prot);

    return pdFALSE;
parseQspiInitCommand_fail:
    printf( "Error: Wrong input. Usage:\nqspi_init <ch> <baudrate> <mode> <prot>\n");
    return pdFALSE;
}

/*******************************************************************************
* xParseQspiReadWriteCommand -  This routine read/write from QSPI commands
*******************************************************************************/
static portBASE_TYPE xParseQspiReadWriteCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{
    MV_U8 cmd_buf[32] = {0}, addr_cycles = 0, dummy = 0, buf[128] = {0};
    MV_U32 addr = 0, size = 0, ch = 0, i;
    MV_STATUS rc;
    enum {QSPI_CMD_TYPE_READ, QSPI_CMD_TYPE_WRITE} qspi_cmd_type;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "qspi") ) )
        goto parseQspiCommand_fail;

    /* Get the command type. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiCommand_fail;
    if (!strcmp(value, "w"))
        qspi_cmd_type = QSPI_CMD_TYPE_WRITE;
    else if (!strcmp(value, "r"))
        qspi_cmd_type = QSPI_CMD_TYPE_READ;
    else
        goto parseQspiCommand_fail;

    /* Channel */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiCommand_fail;
    ch = (MV_U32)strtoul(value, &value, 0);
    if (ch > 1)
        goto parseQspiCommand_fail;

    /* Command */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseQspiCommand_fail;
    cmd_buf[0] = (MV_U8)strtoul(value, &value, 0);

    /* Address */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL)
        goto parseQspiCommand_fail;
    addr = (MV_U32)strtoul(value, &value, 0);

    /* Address cycles */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseQspiCommand_fail;
    addr_cycles = (MV_U8)strtoul(value, &value, 0);

    /* dummy */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseQspiCommand_fail;
    dummy = strtoul(value, &value, 0);

    /* Size / buffer  */
    value = strtok(NULL, DELIMETERS);

#ifdef SPI
    for(i = addr_cycles; i > 0; i--) {
        cmd_buf[addr_cycles + 1 - i] = (addr >> (addr_cycles-1)*8 ) & 0xFF; 
    }
#endif
    if (qspi_cmd_type == QSPI_CMD_TYPE_READ) {
    /* Amount of bytes to read */
        if (value == NULL)
            goto parseQspiCommand_fail;
        size = (MV_U32)strtoul(value, &value, 0);
#ifdef QSPI
        rc = srvCpuQspiRead(ch, cmd_buf[0], addr, addr_cycles, dummy, buf, size);
#elif SPI
        rc = mvSpiWriteThenRead(ch, &cmd_buf[0], addr_cycles+1, buf, size, dummy);
#endif
        if (rc != 0)
            printf("Read failed! returned with error 0x%x\n", rc);
        else {
            printf("read: ch %d, cmd %02x, addr %x, add_cyc %d, dummy %d, size %d\n", ch, cmd_buf[0], addr, addr_cycles, dummy, size);
            for(i = 0; i < size; i++)
                printf("%02x ", buf[i]);
            printf("\n");
        }
    }
    else { /* qspi_cmd_type == QSPI_CMD_TYPE_WRITE */
        /* Populate the buffer */
        while (value != NULL) {
            buf[size++] = (MV_U8)strtoul(value, &value, 16);
            value = strtok(NULL, DELIMETERS);
        }
#ifdef QSPI
        rc = srvCpuQspiWrite(ch, cmd_buf[0], addr, addr_cycles, dummy, buf, size);
#elif SPI
        rc = mvSpiWriteThenWrite(ch, &cmd_buf[0], addr_cycles+1+dummy, buf, size);
#endif
        if (rc != 0)
            printf("write failed! returned with error 0x%x\n", rc);
        else
            printf("write: ch %d, cmd %02x, addr %x, add_cyc %d, dummy %d, buf[0] %d\n",ch, cmd_buf[0], addr, addr_cycles, dummy, buf[0]);
    }

    return pdFALSE;
parseQspiCommand_fail:
    printf( "error: Wrong input. Usage:\n qspi r/w ch cmd addr addr_cycles dummy size/buf\n" );
    return pdFALSE;
}

static const CLI_Command_Definition_t xQspiInitCommand =
{
    ( const char * const ) "qspi_init",
    ( const char * const ) "qspi_init <ch> <baudrate> <mode> <prot>\t"
                        "Initialize QSPI interface. protocol: 0 - Single, 1 - Dual, 2 - Quad\n",
    xParseQspiInitCommand,
    -1
};

static const CLI_Command_Definition_t xQspiRdWrtCommand =
{
    ( const char * const ) "qspi",
    ( const char * const ) "qspi r/w <ch> <cmd> <addr> <addr_cycles> <dummy> <size>/<bytes>\n"
                        "\t\t\t\t\tRead <size> / Write <bytes> (format xx xx xx), using FIFO\n",
    xParseQspiReadWriteCommand,
    -1
};

void registerQspiCommands( void )
{
    FreeRTOS_CLIRegisterCommand( &xQspiInitCommand );
    FreeRTOS_CLIRegisterCommand( &xQspiRdWrtCommand );
}

