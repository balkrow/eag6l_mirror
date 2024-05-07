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
* @file srvCpuSdma.h
*
* @brief SDMA driver services declarations.
*
* @version   1
********************************************************************************
*/

#ifndef __SRV_CPU_SDMA_H__
#define __SRV_CPU_SDMA_H__


/* Prototype for application RX handler hook */
typedef MV_STATUS (*MV_FW_SDMA_RX_HANDLER)(MV_U32, MV_VOID*);

 /**
* @internal srvCpuSdmaRxInit function
* @endinternal
*
* @brief    Initialize a SDMA queue for RX.
*           Provided a block of memory, create a list of buffers and
*           cyclic descriptor chain
*
* @param MV_U32 queue         - Number of SDMA queue to use (0 - 7)
* @param MV_U8* pBlock_ph     - Physical memory block pointer
* @param MV_U8* pBlock_offset - offset in Physical memory block pointer for buffers/ descriptors allocation
* @param MV_U8* pBlock_vi     - Virtual address of pBlock_ph
* @param MV_U32 block_size    - Size of memory block
* @param MV_U32 num_of_elem   - Requested number of buffers / descriptors to allocate
* @param MV_U32 buf_size      - Requested size of buffers to allocate
*
* @retval MV_OK           - on success.
* @retval MV_FAIL         - on fail
*
*/
extern MV_STATUS srvCpuSdmaRxInit(MV_U32 queue, MV_U8* pBlock_ph, MV_U8* pBlock_offset, MV_U8* pBlock_vi, MV_U32 block_size, MV_U32 num_of_elem, MV_U32 buf_size);

 /**
* @internal srvCpuSdmaRxEnable function
* @endinternal
*
* @brief  Enable / Disable RX queue
*
* @param MV_U32  queue    - number of SDMA queue(0 - 7)
* @param MV_BOOL enable   - 1 - Enable, 0 - Disable
*
* @retval MV_OK           - on success.
* @retval MV_FAIL         - on fail
*
*/
extern MV_STATUS srvCpuSdmaRxEnable(MV_U32 queue, MV_BOOL enable);

 /**
* @internal srvCpuSdmaRxGet function
* @endinternal
*
* @brief  Receive packet from SDMA queue
*         Check if new packet has been received and return pointer to it.
*         If no new packet is waiting, return MV_NO_MORE
*
* @param MV_U32  queue    - Number of SDMA queue to receive from (0 - 7)
* @param MV_U32* buf_size - Pointer to size of packet returned
* @param MV_U8** pBuf     - pointer to a returned buffer pointer
*
* @retval MV_OK           - on success.
* @retval MV_FAIL         - on fail
*
*/
extern MV_STATUS srvCpuSdmaRxGet(MV_U32 queue, OUT MV_U32* buf_size, OUT MV_U8** pBuf);

 /**
* @internal srvCpuSdmaRxFreeBuf function
* @endinternal
*
* @brief  Free a buffer by application to a queue
*         return a used buffer back to the descriptor chain
*
* @param MV_U32  queue    - Number of SDMA queue to receive from (0 - 7)
* @param MV_U8* msg_buf   - Virtual address of the buffer to free
*
* @retval MV_OK           - on success.
* @retval MV_FAIL         - on fail
*
*/
extern MV_STATUS srvCpuSdmaRxFreeBuf(MV_U32 queue, MV_U8* msg_buf);

 /**
* @internal srvCpuSdmaTxInit function
* @endinternal
*
* @brief    Initialize a SDMA queue for TX.
*           Allocates a single TX descriptor, that points to single buffer
*           for a specific queue, and init that descriptor
*           Supplied mem block should be big enough for the descriptor &
*           buffer to be allocated
*
* @param MV_U32 queue          - Number of SDMA queue to use (0 - 7)
* @param MV_U8* pBlock_ph      - pointer to memory block in SDMA SRAM to alloc from (address as seen by SDMA)
* @param MV_U8* pBlock_offset  - offset in Physical memory block pointer for buffers/ descriptors allocation
* @param MV_U8* pBlock_vi      - virtual address of pBlock_ph (address as seen by CM3)
* @param MV_U32 block_size     - Size of memory block
* @param MV_U32 buf_size       - Size of TX buffer to be allocated
*
* @retval MV_OK           - on success.
* @retval MV_FAIL         - on fail
*
*/
extern MV_STATUS srvCpuSdmaTxInit(MV_U32 queue,  MV_U8* pBlock_ph, MV_U8* pBlock_offset, MV_U8* pBlock_vi, MV_U32 block_size, MV_U32 buf_size);

 /**
* @internal srvCpuSdmaTx function
* @endinternal
*
* @brief  Transmit a buffer through specified queue
*
* @param MV_U32   queue    - Number of SDMA queue to transmit from (0 - 7)
* @param MV_U32   buf_size - Size of buffer to transmit
* @param MV_VOID* pBuf     - Pointer to buffer to transmit
*
* @retval MV_OK            - on success.
* @retval MV_ERROR         - on error
*
*/
extern MV_STATUS srvCpuSdmaTx(MV_U32 queue, MV_U32 buf_size, MV_VOID* pBuf);

 /**
* @internal srvCpuSdmaBaseAddressInit function
* @endinternal
*
* @brief  set base address
*
* @param MV_U32 baseAddress  - set base address depends on device type
*
* @retval MV_OK            - on success.
* @retval MV_ERROR         - on error
*
*/
MV_STATUS srvCpuSdmaBaseAddressInit(MV_U32 baseAddress);

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
extern void prvRxTxTask(MV_FW_SDMA_RX_HANDLER handler);

void registerSdmaCliCommands( void );

#endif /* __SRV_CPU_SDMA_H__ */