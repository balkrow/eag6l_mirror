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
#include "srvCpuI2c_shell.h"
#include "srvCpuI2c.h"

/*******************************************************************************
* xParseI2cRdWrCommand -  This routine parse i2c read/write commands
*******************************************************************************/
static portBASE_TYPE xParseI2cRdWrCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{
    MV_U8 channel, address = 0, width = 0, size = 0, buff[32] = {0};
    MV_U32 offset = 0;
    MV_STATUS rc;
    enum {I2C_CMD_TYPE_READ, I2C_CMD_TYPE_WRITE, I2C_CMD_TYPE_PROBE} i2c_cmd_type = I2C_CMD_TYPE_READ;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );

    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "i2c") ) )
        goto parseI2cCommand_fail;

    /* Get the command type. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseI2cCommand_fail;
    if (!strcmp(value, "probe"))
        i2c_cmd_type = I2C_CMD_TYPE_PROBE;
    else if (!strcmp(value, "w"))
        i2c_cmd_type = I2C_CMD_TYPE_WRITE;
    else if (strcmp(value, "r"))
        goto parseI2cCommand_fail;

    /* Channel */
    value = strtok( NULL, DELIMETERS );
    if (i2c_cmd_type == I2C_CMD_TYPE_PROBE) {
        if (value == NULL)
            channel = 0;
        else if (isdigit(value[0]))
            channel = (MV_U8)strtoul(value, &value, 0);
        else
            goto parseI2cCommand_fail;
        goto parseI2cCommand_probe;
    }

    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    channel = (MV_U8)strtoul(value, &value, 0);
    /* Address */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    address = (MV_U8)strtoul(value, &value, 0);
    /* width */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    width = (MV_U8)strtoul(value, &value, 0);
    /* offset */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseI2cCommand_fail;
    offset = strtoul(value, &value, 0);

parseI2cCommand_probe:
    if (i2c_cmd_type == I2C_CMD_TYPE_PROBE) {
        buff[0] = 0;
        printf("Devices on channel %d:", channel);
        for (address = 1; address < 0x80; address++) {
            if (MV_OK == srvCpuI2cWrite( channel, address, 0, 0, buff, 1 ))
                printf(" %02x", address);
        }
    } else {

        value = strtok(NULL, DELIMETERS);
        if (i2c_cmd_type == I2C_CMD_TYPE_READ) {
            /* Amount of bytes to read */
            if ((value == NULL) || (!isdigit(value[0])))
                goto parseI2cCommand_fail;
            size = (MV_U8)strtoul(value, &value, 0);
            rc = srvCpuI2cRead( channel, address, offset, width, buff, size );
            printf("Read %d bytes from dev 0x%02x, channel %d, offset 0x%04x, width %d:", size, address, channel, offset, width);
        } else {
            /* Populate the buffer */
            while (value != NULL) {
                buff[size++] = (MV_U8)strtoul(value, &value, 16);
                value = strtok(NULL, DELIMETERS);
            }
            rc = srvCpuI2cWrite( channel, address, offset, width, buff, 1 );
            printf("write %d bytes to dev 0x%02x, channel %d, offset 0x%04x, width %d:", size, address, channel, offset, width);
        }
        if ( rc == MV_OK ) {
            width = 0;
            while (width < size)
                printf(" %02x", buff[width++]);
        } else
            printf("Failed");
    }
    printf("\n");

    return pdFALSE;
parseI2cCommand_fail:
    printf( "error: Wrong input. Usage:\ni2c r/w ch dev width offs size / bytes\ni2c probe [channel]\n" );
    return pdFALSE;
}

/*******************************************************************************
* xParseI2cInitCommand -  This routine initialize I2C
*******************************************************************************/
static portBASE_TYPE xParseI2cInitCommand( char *pcWriteBuffer,
                                       size_t xWriteBufferLen,
                                       const char *pcCommandString )
{
    MV_U32 frequency = 0, interface = 0;
    enum {I2C_MODE_TYPE_ISR, I2C_MODE_TYPE_POLLING} mode = I2C_MODE_TYPE_ISR;
    char *value;
    MV_STATUS rc;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );

    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "i2c_init") ) )
        goto parseI2cInitCommand_fail;

    /* Interface */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseI2cInitCommand_fail;
    interface = (MV_U32)strtoul(value, &value, 0);
    if ( (interface == 2) && (devices_details[devIndex].deviceType != SRV_CPU_DEVICE_TYPE_IRONMAN_E))
        goto parseI2cInitCommand_fail;
    else if (interface > 2)
        goto parseI2cInitCommand_fail;

    /* Frequency */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseI2cInitCommand_fail;
    frequency = (MV_U32)strtoul(value, &value, 0);

    /* Mode */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseI2cInitCommand_fail;
    if (!strcmp(value, "1"))
        mode = I2C_MODE_TYPE_POLLING;
    else if (strcmp(value, "0"))
        goto parseI2cInitCommand_fail;

    /* unmask I2C isr */
    srvCpuUnmaskI2cIrq(interface);

    rc = srvCpuTwsiInit(frequency, interface);
    if (rc != MV_OK)
        printf("I2C init failed! frequency is too high, please use up tp 400Khz. \n");
    else
        printf("I2C initialized successfully  with interface %d, frequency %d, mode %d \n", interface, frequency, mode);

    return pdFALSE;
parseI2cInitCommand_fail:
    printf( "error: Wrong input. Usage:\ni2c_init interface frequency mode\n");
    return pdFALSE;
}

static const CLI_Command_Definition_t xI2cInitCommand =
{
     ( const char * const ) "i2c_init",
    ( const char * const ) "i2c_init <ch> <freq> <mode>\t\t"
                        "Initialize the I2C interface, interface: 0-1, mode: 0 - ISR, 1 - Polling\n",
    xParseI2cInitCommand,
    -1
};

const CLI_Command_Definition_t xI2cRdWrCommand =
{
    ( const char * const ) "i2c",
    ( const char * const ) "i2c probe/r/w ch addr offst_width offset size/bytes\n"
                        "\t\t\t\t\tProbe/Read/Write from/to I2C device, bytes: xx xx xx\n",
    xParseI2cRdWrCommand,
    -1
};

void registerI2cCommands( void )
{
    FreeRTOS_CLIRegisterCommand( &xI2cInitCommand );
    FreeRTOS_CLIRegisterCommand( &xI2cRdWrCommand );
}
