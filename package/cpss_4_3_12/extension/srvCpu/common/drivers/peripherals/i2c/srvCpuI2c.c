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
* @file srvCpuI2c.c
*
* @brief I2C capabilities implementation.
*
* @version   1
********************************************************************************
*/

#include <FreeRTOS.h>
#include <semphr.h>
#include "string.h"
#include "srvCpuI2c.h"
#include "printf.h"
#include <srvCpuTimer.h>
#include <srvCpuServices.h>
#ifndef MICRO_INIT
#include <prvSrvCpuServices.h>
#endif

#undef MV_DEBUG
//#define MV_DEBUG

/* This macro returns absolute value                                        */
#ifndef MV_ABS
#define MV_ABS( number )    ((( MV_32 )( number ) < 0 ) ? -( MV_32 )( number ) : ( MV_32 )( number ))
#endif

static void mv_twsi_isr( MV_U8 interface );
#ifdef MV_DEBUG
static void mv_twsi_add_dbg_str(char *str);
#endif /*MV_DEBUG*/

/* GLOBALS */
static SRV_CPU_TWSI_CTRL_STATE_E  mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_IDLE_E;
static MV_U8         mv_twsi_slave_address;
static MV_U8         mv_twsi_buffer[SRV_CPU_TWSI_EXTERNAL_BUFFER_SIZE];
static MV_U32        mv_twsi_length;
static MV_U32        mv_twsi_buffer_offset;
static MV_U8*        mv_twsi_user_buffer_ptr;
static MV_U32        mv_twsi_offset_length;
static MV_STATUS     mv_twsi_status;
static MV_U8         interfaceLockFlag = 0;

#ifndef MICRO_INIT
/* Semaphore to block calling srvCpuTwsiMasterTransceive before ISR is finish */
static xSemaphoreHandle isrSemaphore = NULL;
#endif
/* mutex for critical section between 2 threads */
static xSemaphoreHandle twsiMutex = NULL;

#ifdef MV_DEBUG
static MV_U32        mv_twsi_reset_counter = 0;
char                 mv_twsi_dbg_buf[0x5000];
static char          dbg_str[0x100];
#endif /*MV_DEBUG*/

void sysfs_pci_write(void* vaddr, int offset, MV_U32 value);
MV_U32 sysfs_pci_read(void* vaddr, int offset);

#define MV_TWSI_DELAY  3*configTICK_RATE_HZ / 1000

/* Macros */
#define SRV_CPU_TWSI_WRITE_REG(interface,addr,data) \
        *(volatile MV_U32 *)(unsigned long)(SRV_CPU_TWSI_BASE_ADDR(interface) + addr)=data

#define SRV_CPU_TWSI_READ_REG(interface,addr) \
        (*(volatile MV_U32 *)(unsigned long)(SRV_CPU_TWSI_BASE_ADDR(interface) + addr))


#define MV_TWSI_BAUD(M, N)  ((M << 3) | (N & 0x7))

#define TICKS_TO_WAIT    100

SET_I2C_CHANNEL_FUNC_PTR i2cSetChannelFunc = NULL;

/***********************************************************************
 * registerSetI2cChannelFunc()                                         *
 *    register I2C set channel function pointer                        *
 ***********************************************************************/
MV_VOID registerSetI2cChannelFunc( SET_I2C_CHANNEL_FUNC_PTR funcPtr )
{
    i2cSetChannelFunc = funcPtr;
}

/***********************************************************************
 * MV_TWSI_RESET()                                                     *
 *    Reset controller in case of error: calls init as well            *
 ***********************************************************************/
static void mv_twsi_reset( MV_U8 interface )
{
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_RESET_E, 0);
}

/***********************************************************************
 * MV_TWSI_RELINQUISH()                                                *
 *    When called from ISR - abandon transaction and quit              *
 ***********************************************************************/
static void mv_twsi_relinquish(void)
{
    srvCpuOsUsDelay(2);
    mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_IDLE_E;
#ifndef MICRO_INIT
    xSemaphoreGiveFromISR( isrSemaphore, 0 );
#endif

/* DEBUG - remove */
#ifdef MV_DEBUG
    if (mv_twsi_status != MV_FAIL) {
        mv_twsi_reset_counter = 0;
        /*printf("TWSI_DBG %s\n",dbg_str);*/
        mv_twsi_add_dbg_str(dbg_str);
    } else {
        if (mv_twsi_reset_counter < 10) {
            mv_twsi_reset_counter++;
            /*printf("TWSI_DBG %s\n",dbg_str); */
            mv_twsi_add_dbg_str(dbg_str);
        }
    }
#endif
/* End of DEBUG */

}

/**
* @internal srvCpuTwsiInit function
* @endinternal
*
* @brief  Initialize controller and global registers
*
* @param MV_U32 frequency  - set frequency
* @param MV_U8 interface   - select between first interface and second interface
*
* @retval MV_STATUS   MV_OK        - For successful operation
* @retval             MV_BAD_VALUE - Illegal value
*
*/
MV_STATUS srvCpuTwsiInit(MV_U32 frequency, MV_U8 interface)
{
    MV_32  power, n, actualN, m, actualM, freq, minMargin = 0x0fffffff;
    MV_STATUS rc = MV_OK;
    /* Create mutex before starting tasks */
    twsiMutex = xSemaphoreCreateMutex();
#ifndef MICRO_INIT
    vSemaphoreCreateBinary(isrSemaphore);
    xSemaphoreTake( isrSemaphore, 1); /* take semaphore and release when ISR is finish */
#endif

#ifndef MICRO_INIT
    /* config MPP to I2C according to interface*/
    rc = prvI2cMppConfig(interface);
    if (rc != MV_OK)
        return rc;
#endif
    /* Set state machine to IDLE */
    mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_IDLE_E;

    if (frequency > 400000)
        return MV_BAD_VALUE;
    /* Calculate N and M for the TWSI clock baud rate */
    actualN = 0;
    actualM = 0;

    for (n = 0 ; n < 8 ; n++)
        for (m = 0 ; m < 16 ; m++) {
              power = 2 << n; /* power = 2^(n+1) */
              freq = (devices_details[devIndex].systclk * 100000) / (10 * (m + 1) * power);
              if (MV_ABS(frequency - freq) < minMargin) {
                    minMargin   = MV_ABS(frequency - freq);
                    actualN     = n;
                    actualM     = m;
              }
        }

    /* Reset controller */
    mv_twsi_reset( interface );
    /* Set the baud rate */
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_BAUD_E, MV_TWSI_BAUD(actualM, actualN));

#ifdef MICRO_INIT
    /* Enable TWSI*/
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_CTRL_E, SRV_CPU_TWSI_CTRL_BIT_SLAVE_E);
#else /* ifndef MICRO_INIT */
    /* Enable TWSI and TWSI interrupt */
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_CTRL_E, SRV_CPU_TWSI_CTRL_BIT_IE_E | SRV_CPU_TWSI_CTRL_BIT_SLAVE_E);
#endif

    /* DEBUG */
#ifdef MV_DEBUG
      memset(mv_twsi_dbg_buf, 0, sizeof(mv_twsi_dbg_buf));
#endif /*MV_DEBUG*/
    /* DEBUG */

    return rc;
}

/**
* @internal
*           srvCpuTwsiMasterTransceive function
* @endinternal
*
* @brief   Transmit / receive BUFFER on I2C bus to / from SLAVE_ADDRESS
*
* @param MV_U8* BUFFER:              - pre allocated buffer with length bytes
* @param MV_U32 LENGTH:              - How many bytes to read
* @param SRV_CPU_TWSI_SLAVE SLAVE    - slave structure pre filled with address & offset
* @param MV_U32 TX_RX                - 1 for transmit, 0 for receive.
* @param MV_U8 INTERFACE             - I2C interface 
*
* @retval 0                          - on success
* @retval 1                          - if not ready
*/
MV_U32 srvCpuTwsiMasterTransceive( MV_U8 *buffer,
                                  MV_U32 length,
                                  SRV_CPU_TWSI_SLAVE_STC *slave,
                                  MV_U32 tx_rx, MV_U8 interface)
{
    MV_U8 temp, i=0;

#ifndef MICRO_INIT
    unsigned char ucLocalTickCount = 0;
#endif

    if (length > SRV_CPU_TWSI_EXTERNAL_BUFFER_SIZE)
        return MV_FAIL;

    /* Set global values for ISR, copy buffer, set START signal */
    temp = mv_twsi_slave_address = slave->slaveAddr.address | tx_rx;
    mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_STA_E;
    mv_twsi_length = length;
    mv_twsi_buffer_offset = 0;

    /* Copy the slave register offset bytes to transmit buffer */
    if (slave->validOffset) {
        mv_twsi_offset_length = slave->offset_length;
        /* Copy offset bytes to transmit buffer */
        for (i = 0; i < mv_twsi_offset_length; i++)
            mv_twsi_buffer[i] = (MV_U8)(((slave->offset) >> ((mv_twsi_offset_length - 1 - i) * 8)) & 0xFF);
    }

    if (SRV_CPU_TWSI_TRANSMIT == tx_rx) {
        memcpy(mv_twsi_buffer + i, buffer, length);
        if (slave->validOffset) {
            length += mv_twsi_offset_length;
            mv_twsi_length = length;
        }
    } else {/* Receiver mode */
                if (slave->validOffset)
                    temp = slave->slaveAddr.address | SRV_CPU_TWSI_TRANSMIT;
                mv_twsi_user_buffer_ptr = buffer;
    }/* Receiver mode */

    /* Load slave address + W/R bit */
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_DATA_E, temp);

    /* Set START signal on bus */
    temp = SRV_CPU_TWSI_READ_REG(interface, SRV_CPU_TWSI_REG_CTRL_E);
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_CTRL_E, temp | SRV_CPU_TWSI_CTRL_BIT_STA_E);

    mv_twsi_status = MV_FAIL;

    /* DEBUG - remove */
#ifdef MV_DEBUG
    memset(dbg_str, 0, sizeof(dbg_str));
    sprintf(dbg_str, "Addr 0x%02x %d bytes, offs 0x%x - ",
                      mv_twsi_slave_address,
                      length,
                      slave->offset);
#endif /*MV_DEBUG*/

#ifdef MICRO_INIT
    while (SRV_CPU_TWSI_CTRL_STATE_IDLE_E != mv_twsi_controller_state) {
        vTaskDelay( MV_TWSI_DELAY );
        mv_twsi_isr( interface );
    }
#else /* ifndef MICRO_INIT */
    while( xSemaphoreTake( isrSemaphore, 1) != pdTRUE )
    {
        /* if transaction is not completed within TICKS_TO_WAIT msec then release isrSemaphore to avoid deadlock */
        if( ucLocalTickCount++ >= TICKS_TO_WAIT )
        {
            /* Unblock the task by releasing the semaphore, and set state machine to IDLE */
            mv_twsi_relinquish();
            /* Reset controller */
            mv_twsi_reset( interface );
            SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_CTRL_E, SRV_CPU_TWSI_CTRL_BIT_IE_E | SRV_CPU_TWSI_CTRL_BIT_SLAVE_E);
            xSemaphoreTake( isrSemaphore, 1); /* take semaphore for next transaction */
            break;
        }
    }
#endif
    return mv_twsi_status;
}



/***********************************************************************
 * MV_TWSI_ISR()                                                       *
 ***********************************************************************/
static void mv_twsi_isr( MV_U8 interface )
{
    SRV_CPU_TWSI_CONTROLLER_STATUS_E status;
    MV_U8 ctrl;

    /* Read ctrl & status registers */
    status = SRV_CPU_TWSI_READ_REG(interface, SRV_CPU_TWSI_REG_STATUS_E);
    ctrl  = SRV_CPU_TWSI_READ_REG(interface, SRV_CPU_TWSI_REG_CTRL_E) & ~SRV_CPU_TWSI_CTRL_BIT_IF_E;
    /* DEBUG - remove */
#ifdef MV_DEBUG
    sprintf(dbg_str + strlen(dbg_str), "%x 0x%02x %02x, ", mv_twsi_controller_state, ctrl, status);
    if ((mv_twsi_controller_state != SRV_CPU_TWSI_CTRL_STATE_STA_E) && (mv_twsi_controller_state != SRV_CPU_TWSI_CTRL_STATE_DONE_E))
    sprintf(dbg_str + strlen(dbg_str), "data 0x%02x, ", mv_twsi_buffer[mv_twsi_buffer_offset]);
#endif /*MV_DEBUG*/

    switch(mv_twsi_controller_state) {

/*--------------  Master start transmitted  --------------*/
/* This iteration will only clear the interrupt flag
 * so slave address can be transmitted
 */
    case SRV_CPU_TWSI_CTRL_STATE_STA_E:
    /* Start signal transmitted */
        switch(status) {
        case SRV_CPU_TWSI_STATUS_MSTR_STA_E:
        case SRV_CPU_TWSI_STATUS_MSTR_REP_STA_E:
            /*
             * Start signal transmitted successfully.
             * Set next state to master tx or rx.
             */
            if (mv_twsi_slave_address & SRV_CPU_TWSI_RECEIVE) {
                if(mv_twsi_offset_length == 0){
                    mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_REC_E;
                }
                else{
                    mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_OFFS_E;
                }
            }
            else {  /* Master transmitter */
                mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_TRAN_E;
            }
            break;

            default:
              /* Error or Bytes have been transmitted and nacked
               * - end of transmition
               */
              mv_twsi_reset( interface );
              mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
              break;
        }
        break;


/*--------------  Master transmitter  --------------*/
    case SRV_CPU_TWSI_CTRL_STATE_TRAN_E:

        switch (status) {
        case SRV_CPU_TWSI_STATUS_MSTR_TRAN_SLA_ACK_E:
        case SRV_CPU_TWSI_STATUS_MSTR_TRAN_DATA_ACK_E:
              /*
               * Either slave acked it's address or data byte/s
               * transmitted successfully. Load next byte/s or stop.
               */
              if(mv_twsi_length <= mv_twsi_buffer_offset)
              { /* No more bytes - end transmition */
                    ctrl |= SRV_CPU_TWSI_CTRL_BIT_STO_E;
                    mv_twsi_status = MV_OK;
                    mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
              } else {  /* Transmit next byte if any left */
                    /* Load bytes to data for TX */
                    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_DATA_E,
                                      mv_twsi_buffer[mv_twsi_buffer_offset++]);
              }
              break;

        case SRV_CPU_TWSI_STATUS_MSTR_TRAN_SLA_NACK_E:
              /* No Slave acked it's address. */
              ctrl |= SRV_CPU_TWSI_CTRL_BIT_STO_E;
              mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
              break;

        default:
              /* Error or Bytes have been transmitted and nacked
               * - end of transmition
               */
              mv_twsi_reset( interface );
              mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
              break;
      }
      break;


/*--------------  Transmit offset  --------------*/

    case SRV_CPU_TWSI_CTRL_STATE_OFFS_E:

        switch(status) {
        case SRV_CPU_TWSI_STATUS_MSTR_TRAN_SLA_ACK_E:
        case SRV_CPU_TWSI_STATUS_MSTR_TRAN_DATA_ACK_E:
            /*
             * Either slave acked it's address or offset byte/s
             * transmitted successfully. Load next byte/s or restart.
             */
            if (mv_twsi_offset_length > 0) {
                mv_twsi_offset_length--;
                /* Load bytes to data for TX */
                SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_DATA_E,
                                  mv_twsi_buffer[mv_twsi_buffer_offset++]);
            } else {
                /* Offset has been transmitted - set repeated start, go to receiver mode */
                mv_twsi_buffer_offset = 0;
                ctrl |= SRV_CPU_TWSI_CTRL_BIT_STA_E;
                /* Load slave address + W/R bit */
                SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_DATA_E, mv_twsi_slave_address);
                mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_STA_E;
            }
            break;
        case SRV_CPU_TWSI_STATUS_MSTR_TRAN_SLA_NACK_E:
              /* No Slave acked it's address. */
              ctrl |= SRV_CPU_TWSI_CTRL_BIT_STO_E;
              mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
              break;

        default:
              /* Error or Bytes have been transmitted and nacked
               * - end of transmition
               */
              mv_twsi_reset( interface );
              mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
              break;
        }
        break;


/*--------------  Master receiver  --------------*/
    case SRV_CPU_TWSI_CTRL_STATE_REC_E:

        switch(status) {
        case SRV_CPU_TWSI_STATUS_MSTR_RECV_DATA_NACK_E:
            /* We're done receiving. Read byte/s, set stop bit, return to IDLE */
        case SRV_CPU_TWSI_STATUS_MSTR_RECV_DATA_ACK_E:
            /*
             * Slave sent byte/s, we acked. Read byte/s to external buffer.
             * Don't ack last byte if no more space in external buffer.
             */

              mv_twsi_buffer[mv_twsi_buffer_offset++] = SRV_CPU_TWSI_READ_REG(interface, SRV_CPU_TWSI_REG_DATA_E);

            if (SRV_CPU_TWSI_STATUS_MSTR_RECV_DATA_NACK_E == status) {
                /* End of communication - copy buffer, stop bit and IDLE */
                memcpy(mv_twsi_user_buffer_ptr, mv_twsi_buffer, mv_twsi_length);
                ctrl |= SRV_CPU_TWSI_CTRL_BIT_STO_E;
                mv_twsi_status = MV_OK;
                mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
            } else /* Continue to receive next byte */
                if (mv_twsi_length - mv_twsi_buffer_offset == 1)
                    ctrl &= ~SRV_CPU_TWSI_CTRL_BIT_ACK_E; /* Don't acknowledge next byte */
                else
                    ctrl |= SRV_CPU_TWSI_CTRL_BIT_ACK_E; /* Acknowledge next byte */

            break;

        case SRV_CPU_TWSI_STATUS_MSTR_RECV_SLA_ACK_E:
            /* Slave acked it's address. Set ack if needed, and receive first byte */
            if (mv_twsi_length > 1)
                ctrl |= SRV_CPU_TWSI_CTRL_BIT_ACK_E; /* Acknowledge next byte */
            break;

        case SRV_CPU_TWSI_STATUS_MSTR_RECV_SLA_NACK_E:
            /* Slave nacked it's address. */
            ctrl |= SRV_CPU_TWSI_CTRL_BIT_STO_E;
            mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
            break;

        default:
            /* Error or Bytes have been transmitted and nacked
             * - end of transmission
             */
            mv_twsi_reset( interface );
            mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
            break;
        }

        break;

    default:
      mv_twsi_reset( interface );
      mv_twsi_controller_state = SRV_CPU_TWSI_CTRL_STATE_DONE_E;
    }
    /* Clear interrupt */
    SRV_CPU_TWSI_WRITE_REG(interface, SRV_CPU_TWSI_REG_CTRL_E, ctrl);
    if (mv_twsi_controller_state == SRV_CPU_TWSI_CTRL_STATE_DONE_E)
    {
        mv_twsi_relinquish();
    }
}

/**
* @internal
*           srvCpuI2cRead function
* @endinternal
*
* @brief   Receive BUFFER on I2C bus from SLAVE_ADDRESS
*          The same read interface, but implement with locks semaphore to avoid simultaneous contact to i2c
*          Receive BUFFER on I2C bus from SLAVE_ADDRESS
*          this wrapper fills the slave structure and using semaphore lock to prevent simulations access to i2c
*
* @param MV_U8* channel  - I2C channel
*                          (while channel == 0xFF - i2c set channel will be ignored)
* @param MV_U32 address  - i2c address (expected 7 Bit address)
* @param MV_U32 offset   - offset in slave
* @param MV_U32 width    - number of bytes that compose the offset ('0' - for no offset)
* @param MV_U8* buffer   - pre allocated buffer with length bytes
* @param MV_U32 LENGTH   - How many bytes to read
*
* @retval 0                          - on success
* @retval 1                          - if not ready
*/
MV_STATUS srvCpuI2cRead(MV_U8 channel, MV_U32 address, MV_U32 offset, MV_U8 width, MV_U8 *buffer, MV_U32 length)
{
    SRV_CPU_TWSI_SLAVE_STC slave;
    MV_STATUS rc;
    MV_U8 interface = 0;

    if (interfaceLockFlag != 0xff)
    {
        slave.slaveAddr.address = address << 1;
        slave.slaveAddr.type = SRV_CPU_TWSI_ADDR_TYPE_ADDR7_BIT_E;
        slave.validOffset = width > 0 ? MV_TRUE : MV_FALSE;
        slave.offset = offset;
        slave.offset_length = width;

        /* take semaphore before critical section */
        while( xSemaphoreTake( twsiMutex, 1) != pdTRUE);
        if (i2cSetChannelFunc == NULL)
            interface = channel;
        else
            interface = i2cSetChannelFunc(channel);
        rc = srvCpuTwsiMasterTransceive(buffer, length, &slave, SRV_CPU_TWSI_RECEIVE, interface);
        xSemaphoreGive( twsiMutex );

        return rc;
    }
    else
        return MV_BUSY;
}

/**
* @internal
*           srvCpuI2cWrite function
* @endinternal
*
* @brief   Transmit BUFFER on I2C bus to SLAVE_ADDRESS
*          The same write interface, but implement with locks semaphore to avoid simultaneous contact to i2c
*          this wrapper fills the slave structure and using semaphore lock to prevent simulations access to i2c
*
* @param MV_U8* channel  - I2C channel
*                          (while channel == 0xFF - i2c set channel will be ignored)
* @param MV_U32 address  - i2c address
* @param MV_U32 offset   - offset in slave
* @param MV_U32 width    - number of bytes that compose the offset ('0' - for no offset)
* @param MV_U8* buffer   - pre allocated buffer with length bytes
* @param MV_U32 length   - How many bytes to write
*
* @retval 0                          - on success
* @retval 1                          - if not ready
*/
MV_STATUS srvCpuI2cWrite(MV_U8 channel, MV_U32 address, MV_U32 offset, MV_U8 width, MV_U8 *buffer, MV_U32 length)
{
    SRV_CPU_TWSI_SLAVE_STC slave;
    MV_STATUS rc;
    MV_U8 interface;

    if (interfaceLockFlag != 0xff)
    {
        slave.slaveAddr.address = address << 1;
        slave.slaveAddr.type = SRV_CPU_TWSI_ADDR_TYPE_ADDR7_BIT_E;
        slave.validOffset = width > 0 ? MV_TRUE : MV_FALSE;
        slave.offset = offset;
        slave.offset_length = width;

        /* take semaphore before critical section */
        while( xSemaphoreTake( twsiMutex, 1) != pdTRUE);
        if (i2cSetChannelFunc == NULL)
            interface = channel;
        else
            interface = i2cSetChannelFunc(channel);
        rc = srvCpuTwsiMasterTransceive(buffer, length, &slave, SRV_CPU_TWSI_TRANSMIT, interface);
        xSemaphoreGive( twsiMutex );

        return rc;
    }
    else
        return MV_BUSY;
}

/* function used mostly when requested to prepare to SW reset,
 lock the I2C interface after current transaction */
MV_STATUS srvCpuI2cSetInterfaceLock()
{
    interfaceLockFlag = 0xff;
    return MV_OK;
}

/* function used mostly after SW reset has been executed,
 unlock the I2C interface in order to back to normal mode */
MV_STATUS srvCpuI2cSetInterfaceUnLock()
{
    interfaceLockFlag = 0x0;
    return MV_OK;
}

/* function used mostly when requested to prepare to SW reset,
   checking if i2c CONTROLLER_STATUS is idle, to prevent collision */
MV_STATUS srvCpuI2cIsIdleState(MV_U8 interface)
{
    SRV_CPU_TWSI_CONTROLLER_STATUS_E statusInterface;

    /* Read status register */
    statusInterface = SRV_CPU_TWSI_READ_REG( interface, SRV_CPU_TWSI_REG_STATUS_E);
    if ((statusInterface == SRV_CPU_TWSI_STATUS_IDLE_E))
        return MV_OK;

    return MV_NOT_READY;
}

void cm3_i2c_handler(MV_U8 interface)
{
    mv_twsi_isr(interface);
}


/* DEBUG - remove */
#ifdef MV_DEBUG
static void mv_twsi_add_dbg_str(char *str)
{
    static MV_U32 index = 0, current_pos = 0;
    MV_U32 size, space_left;

    space_left = sizeof(mv_twsi_dbg_buf) - current_pos;

    /* Roll over if not enough space left */
    size = strlen(str);
    /*If not enough space left - set space to zero and roll over */
    if (space_left < size + 12) {
        memset(mv_twsi_dbg_buf + current_pos, 0, space_left);
        current_pos = 0;
    }

    /* Start new line with serial number. */
    sprintf(mv_twsi_dbg_buf + current_pos, "\n%08x: ", index++);
    current_pos += 11;

    /* Copy first part of string to current position */
    memcpy(mv_twsi_dbg_buf + current_pos, str, size);

    /* update positions */
    current_pos += size;
}
#endif /*MV_DEBUG*/
