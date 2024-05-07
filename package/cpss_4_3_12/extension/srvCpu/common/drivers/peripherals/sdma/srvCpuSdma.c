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
* @file srvCpuSdma.c
*
* @brief This file includes the initialization and functions of the SDMA
* 
*
* @version   1
********************************************************************************
*/

#include <string.h>
#include <hw.h>
#include <task.h>
#include <FreeRTOS_CLI.h>
#include <printf.h>
#include <srvCpuServices.h>
#include <srvCpuTimer.h>
#include "srvCpuSdma.h"

#define DESC_OWNERSHIP_MASK 0x80000000
#define DESC_OWNERSHIP_SDMA 0x80000000
#define DESC_OWNERSHIP_CPU 0x0
#define SDMA_RX_DESC_SDMA_OWNER 0x8C000000
#define SDMA_TX_DESC_CPU_OWNER 0x300000
#define SDMA_TX_DESC_SDMA_OWNER 0x80301000
#define SDMA_BASE_ADDR(n) srvCpuSdmaGetMgBase(n / 8)
#define SDMA_RX_DESC_STATUS_AND_COMMAND(n) (SDMA_BASE_ADDR(n) + 0x2600 + (n % 8) * 0x10)
#define SDMA_RX_DESC_BUFFER_POINTER(n) (SDMA_BASE_ADDR(n) + 0x2604 + (n % 8) * 0x10)
#define SDMA_RX_NEXT_DESC_POINTER(n) (SDMA_BASE_ADDR(n) + 0x2608 + (n % 8) * 0x10)
#define SDMA_TX_DESC_CURRENT_TX_POINTER(n) (SDMA_BASE_ADDR(n) + 0x2684)
#define SDMA_TX_DESC(n) (SDMA_BASE_ADDR(n) + 0x26C0 + (n % 8) * 4)
#define SDMA_TX_Q_CTRL(n) (SDMA_BASE_ADDR(n) + 0x2868)
#define SDMA_RX_DESC(n) (SDMA_BASE_ADDR(n) + 0x260C + (n % 8) * 0x10)
#define SDMA_RX_Q_CTRL(n) (SDMA_BASE_ADDR(n) + 0x2680)
#define SDMA_RX_PACKET_COUNT(n) (SDMA_BASE_ADDR(n) + 0x2820 + (n % 8) * 4)
#define RX_BYTE_CNT_MASK 0x3FFF

/* Queues: 0-7 */
#define MAX_Q_NUM   24
/* Pointers alignment macro */
#define ALIGN_P(p, boundry)   (void*)(((MV_32)p+boundry-1) & ~(boundry-1))


#define DESC_SIZE   16
/* Although slightly different, RX & TX can use the same descriptor struct */
struct DESC {
    unsigned int    cmd_status;
    unsigned int    buf_size;
    unsigned int    pBuf;
    unsigned int    pNextDesc;
};

#pragma pack (1)
struct RX_PCKT {
    MV_U8   macDA[6];
    MV_U8   macSA[6];
    MV_U16  ethType;
    MV_U8   opcode;
    MV_U8   nofActions;
    MV_U16  seqId;
    MV_U8   unitId;
    MV_U32  addr;
    MV_U32  data;
};

struct TX_PCKT {
    MV_U8   macDA[6];
    MV_U8   macSA[6];
    MV_U16  ethType;
    MV_U8   opcode;
    MV_U8   nofActions;
    MV_U16  seqId;
    MV_32   status;
};
#pragma pack ()

static struct DESC* rx_desc_head[MAX_Q_NUM] = {NULL};
static struct DESC* rx_desc_tail[MAX_Q_NUM] = {NULL};
static struct DESC* tx_desc_list[MAX_Q_NUM] = {NULL};
static MV_U16 tx_buf_size[MAX_Q_NUM] = {0};
static MV_U32 mem_blk_rx_virt_base[MAX_Q_NUM]; /* RX Virtual base address of mem block for each queue */
static MV_U32 mem_blk_rx_phy_base[MAX_Q_NUM]; /* RX Physical base address of mem block for each queue */
static MV_U32 mem_blk_tx_virt_base[MAX_Q_NUM]; /* TX Virtual base address of mem block for each queue */
static MV_U32 mem_blk_tx_phy_base[MAX_Q_NUM]; /* TX Physical base address of mem block for each queue */

#define SDMA_CLI
#define DELIMETERS   " "
#define isdigit(_c)  (((_c)>='0' && (_c)<='9')?1:0)
static MV_U32  tx_queue;
static MV_U32  rx_queue;

/*
 * API to get an MG base address for SDMA purpose
 * Must be implemented by the FW if use SDMA
 */
extern MV_U32 srvCpuSdmaGetMgBase(MV_U32 mgNum);

static MV_STATUS prvCopyWords(void *dst, const void* src, MV_U32 size)
{
    MV_U8 i=0;
    MV_U32 *src_p=(MV_U32*)src, *dst_p=(MV_U32*)dst;

    if ( (dst == NULL) || (src == NULL) )
         return MV_FAIL;

    for (i=0; i<size; i++, src_p++, dst_p++)
       *dst_p = *src_p;

    return MV_OK;
}

static void prvReadModifyWriteBuffer(MV_U32 port, MV_U8 *packetBuffer, MV_U8 packetBufferIndex,
                         MV_U32 portMask, MV_U8 portShift, MV_U8 shiftDirection, MV_U32 bufferMask)
 {
    MV_U8 tempBuffer = 0, tempPort = 0;
    if (shiftDirection == 0 ) /* shift left*/
        tempPort = (MV_U8)((port & portMask) << portShift);
    else /* shift right */
        tempPort = (MV_U8)((port & portMask) >> portShift);
    tempBuffer = packetBuffer[packetBufferIndex];
    tempBuffer = (tempBuffer & bufferMask) | tempPort;
    packetBuffer[packetBufferIndex] = tempBuffer;
}

/******************************************************************************
 * srvCpuSdmaRxInit - Initialize an SDMA queue for RX
 *
 * Description:
 *              Provided a block of memory, create a list of buffers and
 *              cyclic descriptor chain
 *
 * Inputs:
 *         queue         - number of SDMA queue to use (0 - 7)
 *         pBlock_ph     - physical memory block pointer (address as seen by SDMA)
 *         pBlock_offset - offset in Physical memory block pointer for buffers/ descriptors allocation
 *         pBlock_vi     - virtual address of pBlock_ph(address as seen by CM3)
 *         block_size    - size of memory block
 *         num_of_elem   - requested number of buffers / descriptors to allocate
 *         buf_size      - requested size of buffers to allocate
 *
 *****************************************************************************/
MV_STATUS srvCpuSdmaRxInit(
    MV_U32 queue,
    MV_U8* pBlock_ph,
    MV_U8* pBlock_offset,
    MV_U8* pBlock_vi,
    MV_U32 block_size,
    MV_U32 num_of_elem,
    MV_U32 buf_size)
{
    MV_U32  i, start, end;
    struct DESC*    curr_desc_ptr;
    MV_U32          curr_buf;

    mem_blk_rx_virt_base[queue] = (MV_U32)pBlock_vi;
    mem_blk_rx_phy_base[queue] = (MV_U32)pBlock_ph;

    if (pBlock_offset == NULL) {
        pBlock_offset += 16; /* Descriptor in offset NULL = disable queue */
    }
    /* Check given block is big enough to contain desc & buffers */
    start = MV_ALIGN_UP((MV_U32)pBlock_vi + (MV_U32)pBlock_offset, 16);
    buf_size = MV_ALIGN_UP(buf_size, 128);
    end = start + num_of_elem * DESC_SIZE;
    end = MV_ALIGN_UP(end, 128);
    end += num_of_elem * buf_size;

    if (end - start > block_size)
        return MV_FAIL;

    rx_desc_head[queue] = rx_desc_tail[queue] = (struct DESC*)start;

    curr_desc_ptr = (struct DESC*)start;
    curr_buf  = start + num_of_elem * DESC_SIZE;
    curr_buf  = MV_ALIGN_UP(curr_buf, 128);
    for (i = 0; i < num_of_elem; i++, curr_desc_ptr++) {
        /* Configure Command /size  p. 206 */
        curr_desc_ptr->cmd_status = SDMA_RX_DESC_SDMA_OWNER; /* Owned by SDMA */
        /* Configure buf size */
        curr_desc_ptr->buf_size = buf_size;
        /* point to next desc */
        curr_desc_ptr->pNextDesc = (MV_U32)pBlock_ph + (MV_U32)(curr_desc_ptr + 1) - mem_blk_rx_virt_base[queue];
        curr_desc_ptr->pBuf = pBlock_ph + curr_buf - (MV_U8*)mem_blk_rx_virt_base[queue];
        curr_buf += buf_size;
    }
    (--curr_desc_ptr)->pNextDesc = (MV_U32)pBlock_ph + (MV_U32)pBlock_offset; /* Make it cyclic */

    /* Config Q Rx ptr to desc in Munit */
    srvCpuRegWrite(SDMA_RX_DESC(queue), (MV_U32)pBlock_ph + (MV_U32)pBlock_offset);

    return MV_OK;
}


/******************************************************************************
 * srvCpuSdmaRxEnable - Enable / Disable RX queue
 *
 *****************************************************************************/
MV_STATUS srvCpuSdmaRxEnable(
    MV_U32 queue,
    MV_BOOL enable)
{
    MV_U32 value;

    if (!rx_desc_head[queue] && enable)
        return MV_FAIL;

    value = queue % 8;
    if (!enable)
        value += 8;

    srvCpuRegWrite(SDMA_RX_Q_CTRL(queue), (1 << value)); /* Rx Go! Enable the Q */

    return MV_OK;
}


/******************************************************************************
 * srvCpuSdmaRxGet - Receive packet from SDMA queue
 *
 * Description:
 *              Check if new packet has been received and return pointer to it.
 *              If no new packet is waiting, return MV_NO_MORE
 *
 * Inputs:
 *         queue    - number of SDMA queue to receive from (0 - 7)
 *         buf_size - Pointer to size of buffer allocated
 *
 * Outputs:
 *         buf_size - Pointer to size of packet returned
 *         pBuf     - pointer to a returned buffer pointer
 *
 *
 *****************************************************************************/
MV_STATUS srvCpuSdmaRxGet(
    MV_U32 queue,
    OUT MV_U32* buf_size,
    OUT MV_U8** pBuf)
{
    struct DESC *curr_desc_ptr;

    if ( (queue >= MAX_Q_NUM) || !rx_desc_head[queue])
        return MV_FAIL;

    curr_desc_ptr = rx_desc_head[queue];

    if ((curr_desc_ptr->cmd_status & DESC_OWNERSHIP_MASK) == DESC_OWNERSHIP_CPU) {
        *buf_size = (curr_desc_ptr->buf_size >> 16) & RX_BYTE_CNT_MASK;
        *pBuf = mem_blk_rx_virt_base[queue] + (MV_U8*)curr_desc_ptr->pBuf;
        rx_desc_head[queue] = (struct DESC*)(mem_blk_rx_virt_base[queue] + curr_desc_ptr->pNextDesc) - mem_blk_rx_phy_base[queue];
        return MV_OK;
    }

    return MV_NO_MORE;
}


/******************************************************************************
 * srvCpuSdmaRxFreeBuf - Free a buffer by application to a queue
 *
 * Description:
 *              return a used buffer back to the descriptor chain
 *
 * Inputs:
 *         queue    - number of SDMA queue to receive from (0 - 7)
 *         msg_buf  - Virtual address of the buffer to free
 *
 *****************************************************************************/
MV_STATUS srvCpuSdmaRxFreeBuf(
    MV_U32 queue,
    MV_U8* msg_buf)
{
    struct DESC *curr_desc_ptr;
    MV_U32 tail = mem_blk_rx_virt_base[queue];

    if ( (queue > MAX_Q_NUM-1) || !rx_desc_head[queue])
        return MV_FAIL;

    curr_desc_ptr = (struct DESC*)rx_desc_tail[queue];

    if ((curr_desc_ptr->cmd_status & DESC_OWNERSHIP_MASK) == DESC_OWNERSHIP_CPU) {
        curr_desc_ptr->pBuf = (MV_U32)mem_blk_rx_phy_base[queue] + (MV_U32)(msg_buf - mem_blk_rx_virt_base[queue]);
        curr_desc_ptr->cmd_status |= DESC_OWNERSHIP_SDMA;
        curr_desc_ptr->buf_size &= 0xffff;
        tail = tail + (MV_U32)curr_desc_ptr->pNextDesc - mem_blk_rx_phy_base[queue];
        rx_desc_tail[queue] = (struct DESC*)tail;
        return MV_OK;
    }
    return MV_FAIL;
}

/**************************************************************************
* srvCpuSdmaTxInit
*
* DESCRIPTION:
*    Allocates a single TX descriptor, that points to single buffer
*    for a specific queue, and init that descriptor
*    Supplied mem block should be big enough for the descriptor &
*    buffer to be allocated
*
* INPUT:
*     queue         - queue number
*     pBlock_ph     - pointer to memory block in SDMA SRAM to alloc from (address as seen by SDMA)
*     pBlock_offset - offset in Physical memory block pointer for buffers/ descriptors allocation
*     pBlock_vi     - virtual address of pBlock_ph (address as seen by CM3)
*     block_size    - size of memory block
*     buf_size      - size of TX buffer to be allocated
*
* OUTPUT:
*     None.
*
* RETURN:
*     MV_OK - on success
*     MV_FAIL - otherwise
*
**************************************************************************/
MV_STATUS srvCpuSdmaTxInit(
    MV_U32 queue,
    MV_U8* pBlock_ph,
    MV_U8* pBlock_offset,
    MV_U8* pBlock_vi,
    MV_U32 block_size,
    MV_U32 buf_size)
{
    MV_U32          start, end;
    struct DESC*    curr_desc_ptr;
    MV_U32          curr_buf;

    if ( (queue > MAX_Q_NUM-1) || (buf_size <= 8) || (buf_size > _64K) )
        return MV_FAIL;

    mem_blk_tx_virt_base[queue] = (MV_U32)pBlock_vi;
    mem_blk_tx_phy_base[queue]  = (MV_U32)pBlock_ph;

    srvCpuRegWrite(SDMA_TX_Q_CTRL(queue), (1 << (queue % 8))); /* Disable queue */

    if (pBlock_offset == NULL) {
        pBlock_offset += 16; /* Descriptor in offset NULL = disable queue */
    }
    /* Check given block is big enough to contain desc & buffers */
    start = MV_ALIGN_UP((MV_U32)pBlock_vi + (MV_U32)pBlock_offset, 16);
    end = start + DESC_SIZE + buf_size;
    if (end - start > block_size)
        return MV_FAIL;

    tx_buf_size[queue] = buf_size;
    tx_desc_list[queue] = (struct DESC*)start;
    /* Config Q Tx ptr to desc in Munit */
    srvCpuRegWrite(SDMA_TX_DESC(queue), (MV_U32)pBlock_ph + (MV_U32)pBlock_offset);

    curr_desc_ptr = tx_desc_list[queue];
    /* buffer locates after the descriptors; */
    curr_buf  = start + DESC_SIZE;
    curr_buf  = MV_ALIGN_UP(curr_buf, 128);
    /* Configure Command /size  p. 206 */
    curr_desc_ptr->cmd_status = SDMA_TX_DESC_CPU_OWNER; /* Owned by CPU */
    /* not configuring buffer size */
    /* point to next descriptor */
    curr_desc_ptr->pNextDesc = 0; /* next descriptor is NULL  */
    curr_desc_ptr->pBuf = pBlock_ph + curr_buf -  (MV_U8*)mem_blk_tx_virt_base[queue];
    /* Configure buf size */
    curr_desc_ptr->buf_size = buf_size;

    return MV_OK;
}

/**************************************************************************
* srvCpuSdmaTx
*
* DESCRIPTION:
*     Transmit a buffer through specified queue
* INPUT:
*     queue      - queue number
*     buf_size   - size of buffer to transmit
*     pBuf       - pointer to buffer to transmit
*
* OUTPUT:
*     None.
*
* RETURN:
*     MV_OK - on success
*     MV_FAIL - otherwise
*
**************************************************************************/
MV_STATUS srvCpuSdmaTx(
    MV_U32 queue,
    MV_U32 buf_size,
    MV_VOID* pBuf)
{
    struct DESC* desc_ptr;
#ifdef SDK_DEBUG
    MV_U32 i, *desc_buf = NULL;
#endif

    if ((queue > MAX_Q_NUM-1) || !tx_desc_list[queue])
        return MV_FAIL;
    if (buf_size == 0)
        return MV_OK;

    desc_ptr = tx_desc_list[queue];
#ifdef SDK_DEBUG
    desc_buf = (MV_U32*)(mem_blk_tx_virt_base[queue] + desc_ptr->pBuf);
#endif
    /* Buffers can only be copied to SRAM in words - so round up to word */
    buf_size = MV_ALIGN_UP(buf_size, 4);
    if ( buf_size > tx_buf_size[queue])
        return MV_FAIL;

    prvCopyWords((MV_U32*)(mem_blk_tx_virt_base[queue] + desc_ptr->pBuf), pBuf, buf_size/4);

#ifdef SDK_DEBUG
    printf("\nTransmitting a packet in size: %d, data:\n", buf_size);
    for (i=0; i<(buf_size/4); i++)
    {
        printf("%x ", desc_buf[i]);
    }
    printf("\n");
#endif

    desc_ptr->buf_size = buf_size << 16;
    desc_ptr->cmd_status = SDMA_TX_DESC_SDMA_OWNER;

    /* reconfigure Q TX ptr to desc in Munit, as it got the next desc addr (NULL) after previous TX */
    srvCpuRegWrite(SDMA_TX_DESC(queue), (MV_U32)mem_blk_tx_phy_base[queue] + (MV_U32)tx_desc_list[queue] - mem_blk_tx_virt_base[queue]);
    srvCpuRegWrite(SDMA_TX_Q_CTRL(queue), (1 << (queue % 8))); /* Tx Go! Enable Q */
    srvCpuOsUsDelay(15);

    return MV_OK;
}

/**************************************************************************
* prvRxTxTask
*
* DESCRIPTION:
*     SDMA RX listening task
*
* handler - pointer to application hook for processing received buffers
*           Can be NULL
*
**************************************************************************/
#define SDMA_MAX_RX_FRAMES_TO_PROCESS 10
void prvRxTxTask(
    MV_FW_SDMA_RX_HANDLER handler)
{
    MV_U32          buf_size;
    MV_U8           count;
    MV_U8*          pBuf;
    MV_STATUS       st;

    srvCpuSdmaRxEnable(rx_queue, 1); /* Enable queue */
    for ( ;; ) {
        vTaskDelay(5 * portTICK_RATE_MS); /* wait for 5mSec */

        st = MV_OK;
        count = 0;

        /* Process at most SDMA_MAX_RX_FRAMES_TO_PROCESS per loop */
        while (st == MV_OK && count++ < SDMA_MAX_RX_FRAMES_TO_PROCESS) {
            st = srvCpuSdmaRxGet(rx_queue, &buf_size, &pBuf); /* Check if any buffer arrived */
            if (st == MV_FAIL)
                printf("srvCpuSdmaRxGet failed\n");
            else if (st == MV_OK) {

                if (buf_size >= 12) {
                    if (handler)
                        handler(buf_size, pBuf); /* Call application hook to process the buffer */
                }
                else
                    printf("Packet size too small: %d\n", buf_size);

                if (MV_OK != srvCpuSdmaRxFreeBuf(rx_queue, pBuf)) /* Free buffer back to ring */
                    printf("failed to free buf\n");
            }
        }
    }
}

static MV_STATUS prvSdmaRxHandler(MV_U32 buf_size, MV_VOID *pBuf)
{
    MV_U32 i = 0;
    char *buf = (char *)pBuf;
    printf("\nRX size: %d, data:\n", buf_size);
    while (buf_size--)
    {
        printf("%02x ", buf[i++]);
    }
    printf("\n");
    return MV_OK;
}

#ifdef SDMA_CLI
#ifdef SDK_DEBUG
static portBASE_TYPE prvSdmaPrintRegCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    MV_U32 txQueue = 0, rxQueue = 0;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "sdma-print-reg") ) )
        goto parseSdmaPrintRegCommand_fail;

    /* RX_QUEUE */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseSdmaPrintRegCommand_fail;
    rxQueue = (MV_U32)strtoul(value, &value, 0);

    /* TX_QUEUE */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseSdmaPrintRegCommand_fail;
    txQueue = (MV_U32)strtoul(value, &value, 0);

    printf("RX queue: %d, TX queue %d\n", rxQueue, txQueue);
    printf("RX pointer: %p\n",srvCpuRegRead( SDMA_RX_DESC(rxQueue)));
    printf("RX Next Desc: %p\n",srvCpuRegRead( SDMA_RX_NEXT_DESC_POINTER(rxQueue)));
    printf("RX Buffer p: %p\n",srvCpuRegRead( SDMA_RX_DESC_BUFFER_POINTER(rxQueue)));
    printf("RX queue command: %p\n",srvCpuRegRead( SDMA_RX_Q_CTRL(rxQueue)));
    printf("RX Desc Status/command %p\n",srvCpuRegRead( SDMA_RX_DESC_STATUS_AND_COMMAND(rxQueue)));
    printf("RX Packet Count: %p\n",srvCpuRegRead( SDMA_RX_PACKET_COUNT(rxQueue)));
    printf("TX current desc p: %p\n",srvCpuRegRead( SDMA_TX_DESC(txQueue)));
    printf("Last TX desc p: %p\n",srvCpuRegRead( SDMA_TX_DESC_CURRENT_TX_POINTER(txQueue)));
    return pdFALSE;
parseSdmaPrintRegCommand_fail:
    printf( "error: Wrong input. Usage:\n sdma-print-reg <RX Queue> <TX Queue>\n" );
    return pdFALSE;
}

static portBASE_TYPE prvSdmaTxSendDemoPacketCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    MV_U32 port = 0;
    char *value;
    MV_U8 packetBuffer[] =  {   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* DST_MAC*/
                                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, /* SRC_MAC*/
                                /* DSA TAG: FROM_CPU */
                                0x50, 0x02, 0x30, 0x00,
                                0x88, 0x08, 0x40, 0x00,
                                0xa0, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                /* Payload - Data example */
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa,
                                0xaa, 0xaa, 0xaa, 0xaa };

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "sdma_demo_packet") ) )
        goto sdmaTxSendDemoPacketCommand_fail;

    /* Port */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL){
        printf("SDMA Send Packet is failed!\n");
        return pdFALSE;
    }
    port = (MV_U32)strtoul(value, &value, 0);

    /* Calculate DSA-TAG
       port[9:0] = {Word3[24:23],Word2[28], Word1[11:10], Word0[23:19]} */
    prvReadModifyWriteBuffer(port,packetBuffer,13,0x001f,3,0 /* shift left */,0x007); /* Word0[23:19] - port[4:0] */
    prvReadModifyWriteBuffer(port,packetBuffer,18,0x0060,3,1 /* shift right*/,0xF3);  /* Word1[11:10] - port[6:5] */
    prvReadModifyWriteBuffer(port,packetBuffer,20,0x0080,3,1 /* shift right*/,0xEF);  /* Word2[28]    - port[7:7] */
    prvReadModifyWriteBuffer(port,packetBuffer,25,0x0100,1,1 /* shift right*/,0x7F);  /* Word3[23:23] - port[8:8] */
    prvReadModifyWriteBuffer(port,packetBuffer,24,0x0200,1,1 /* shift right*/,0xFE);  /* Word3[24:24] - port[9:9] */

    if (MV_OK !=  srvCpuSdmaTx(tx_queue, sizeof(packetBuffer), packetBuffer))
    {
        printf("SDMA Send Packet is failed!\n");
        return pdFALSE;
    }

    printf("Succeed send a packet via SDMA!\n");
        return pdFALSE;
sdmaTxSendDemoPacketCommand_fail:
    printf( "error: Wrong input. Usage:\nsdma_demo_packet <physical port>\n");
    return pdFALSE;
}
#endif /* SDK_DEBUG */

static portBASE_TYPE prvSdmaRxTxInitCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    MV_U32 queue;
    char *value;
    enum {SDMA_RX_INIT, SDMA_TX_INIT} sdmaEngineType = SDMA_RX_INIT;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "sdma_init") ) )
        goto parseSdmaInitCommand_fail;

    /* RX\TX. */
    value = strtok( NULL, DELIMETERS );
    if (value == NULL )
        goto parseSdmaInitCommand_fail;
    if (!strcmp(value, "tx"))
       sdmaEngineType = SDMA_TX_INIT;
    else if (strcmp(value, "rx"))
        goto parseSdmaInitCommand_fail;

    /* QUEUE */
    value = strtok( NULL, DELIMETERS );
    if ((value == NULL) || (!isdigit(value[0])))
        goto parseSdmaInitCommand_fail;
    queue = (MV_U32)strtoul(value, &value, 0);

    if (sdmaEngineType == SDMA_TX_INIT) {
        tx_queue = queue;
        /* Initialize TX descriptor ring and buffers. */
        if (MV_OK != srvCpuSdmaTxInit(tx_queue, (MV_U8 *)0x00000000, (MV_U8 *)0xc00, (MV_U8 *)0xa0500000, 0x250, 0x100))
            goto parseSdmaInitCommand_fail;
        printf("Succeed initial SDMA TX engine SDMA!\n");
        return pdFALSE;
    }
    else  /* sdmaEngineType == SDMA_RX_INIT */
    {
        rx_queue = queue;
        /* Initialize RX descriptor ring and buffers. */
        srvCpuSdmaRxEnable(rx_queue, 0);  /* Disable queue */
        if (MV_OK != srvCpuSdmaRxInit(rx_queue, (MV_U8 *)0x00000000, (MV_U8 *)0x0, (MV_U8 *)0xa0500000, 0x1000, 10, 0x100))
            goto parseSdmaInitCommand_fail;

        /* Create RX listening task */
        xTaskCreate( (pdTASK_CODE)prvRxTxTask, (const char * const)"SDMA_RX",
                    configMINIMAL_STACK_SIZE, prvSdmaRxHandler,
                    tskIDLE_PRIORITY + 2, NULL );
        printf("Succeed initial SDMA RX engine SDMA!\n");
        return pdFALSE;
    }

parseSdmaInitCommand_fail:
    printf( "SDMA init failed. Usage:\n sdma_init <rx/tx> <queue>\n" );
    return pdFALSE;
}

static portBASE_TYPE prvSdmaTxSendPacketCommand( char *pcWriteBuffer,
    size_t xWriteBufferLen, const char *pcCommandString )
{
    MV_U8 buf[256] = {0};
    MV_U32 size = 0, i;
    char *value;

    value = strtok( ( char * )&pcCommandString[0], DELIMETERS );
    /* Get the command type. */
    if ( (value == NULL) || ( strcmp(value, "sdma_send_packet") ) )
        goto sdmaTxSendPacketCommand_fail;

    printf("%s\n", pcCommandString + strlen(value) + 1);
    i = strlen(value) + 1;
    while (pcCommandString[i] != '\0')
    {
        if ((pcCommandString[i] >= '0') && (pcCommandString[i] <= '9'))
            buf[size] = (pcCommandString[i] - '0') << 4;
        else if ((pcCommandString[i] >= 'a') && (pcCommandString[i] <= 'f'))
            buf[size] = (pcCommandString[i] - 'a' + 10) << 4;
        else if ((pcCommandString[i] >= 'A') && (pcCommandString[i] <= 'F'))
            buf[size] |= pcCommandString[i] - 'A' + 10;
        else
            goto sdmaTxSendPacketCommand_fail;
        i++;
        if ((pcCommandString[i] >= '0') && (pcCommandString[i] <= '9'))
            buf[size] |= pcCommandString[i] - '0';
        else if ((pcCommandString[i] >= 'a') && (pcCommandString[i] <= 'f'))
            buf[size] |= pcCommandString[i] - 'a' + 10;
        else if ((pcCommandString[i] >= 'A') && (pcCommandString[i] <= 'F'))
            buf[size] |= pcCommandString[i] - 'A' + 10;
        else
            goto sdmaTxSendPacketCommand_fail;
        size++;
        i++;
    }

    if (MV_OK !=  srvCpuSdmaTx(tx_queue, size, (void*)buf))
    {
        printf("SDMA Send Packet is failed!\n");
        return pdFALSE;
    }

    printf("Succeed send a packet via SDMA!\n");
        return pdFALSE;
sdmaTxSendPacketCommand_fail:
    printf( "error: Wrong input. Usage:\nsdma_send_packet <buffer>\n");
    return pdFALSE;
}

static const CLI_Command_Definition_t xsdmaRxTxInitCommand =
{
    ( const char * const ) "sdma_init",
    ( const char * const ) "sdma_init <rx/tx> <queue>\t\t"
            "Inital SDMA RX/TX engine, Queue 0-7/15/23 depends on device\n",
    prvSdmaRxTxInitCommand,
    2
};

static const CLI_Command_Definition_t xsdmaTxSendPacketCommand =
{
    ( const char * const ) "sdma_send_packet",
    ( const char * const ) "sdma_send_packet <buffer>\t\t"
            "Send packet with buffer via SDMA engine\n",
    prvSdmaTxSendPacketCommand,
    -1
};

#ifdef SDK_DEBUG /* SDK_DEBUG */
static const CLI_Command_Definition_t xsdmaPrintRegCommand =
{
    ( const char * const ) "sdma-print-reg",
    ( const char * const ) "sdma-print-reg <RX Queue> <TX Queue>\t"
            "print SDMA registers, Queue 0-7/15/23 depends on device\n",
    prvSdmaPrintRegCommand,
    -1
};
static const CLI_Command_Definition_t xsdmaTxSendDemoPacketCommand =
{
    ( const char * const ) "sdma_demo_packet",
    ( const char * const ) "sdma_demo_packet <port>\t\t\t"
            "Send 64B packet: src/dst MAC 0xff, DSA FROM_CPU with physical port, Data 0xaa\n",
    prvSdmaTxSendDemoPacketCommand,
    -1
};
#endif /*SDK_DEBUG */

void registerSdmaCliCommands()
{
    #ifdef SDK_DEBUG /*SDK_DEBUG */
    FreeRTOS_CLIRegisterCommand( &xsdmaPrintRegCommand );
    FreeRTOS_CLIRegisterCommand( &xsdmaTxSendDemoPacketCommand );
    #endif
    /* Register tx_init and send_packet command */
    FreeRTOS_CLIRegisterCommand( &xsdmaRxTxInitCommand );
    FreeRTOS_CLIRegisterCommand( &xsdmaTxSendPacketCommand );
}
#endif /* SDMA_CLI */
