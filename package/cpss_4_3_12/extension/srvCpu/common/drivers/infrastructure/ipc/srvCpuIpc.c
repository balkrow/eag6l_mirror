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
* @file srvCpuIpc.c
*
* @brief wrapper to IPC mechanism
*
* @version   1
********************************************************************************
*/
#include <FreeRTOS.h>
#include <task.h>
#include <string.h>
#include <printf.h>
#include <mvShmIpc.h>
#include <srvCpuServices.h>
#include "srvCpuIpc.h"

/*By default, each SCPU SDK FW is an IPC server, and host CPU/ Other SCPU can connects as client */
static IPC_SHM_STC shmS; /* IPC_SHM_STC Server for initializing IPC shared memory and channel*/
#ifdef IPC_CLIENT
static IPC_SHM_STC shmC; /* IPC_SHM_STC Client, which connects to server */
#endif

static void wr_Ipc_sync(
    void* cookie,
    IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    void*   ptr,
    IPC_UINTPTR_T targetPtr,
    IPC_U32 size
)
{
    IPC_U32 i;
    (void)cookie;
    if (mode == IPC_SHM_SYNC_FUNC_MODE_READ_E)
    {
#if defined(IPC_CACHE_WB_INVALIDATE)
        armv7_dcache_wbinv_range(targetPtr, size);
#endif
        for (i = 0; i < size; i += 4)
        {
            *((IPC_U32*)(((IPC_UINTPTR_T)ptr)+i)) = *((volatile IPC_U32*)(targetPtr+i));
        }
        return;
    }
    /* mode == IPC_SHM_SYNC_FUNC_MODE_WRITE_E */
    for (i = 0; i < size; i += 4)
    {
        *((volatile IPC_U32*)(targetPtr+i)) = *((IPC_U32*)(((IPC_UINTPTR_T)ptr)+i));
    }
#if defined(IPC_CACHE_WB_INVALIDATE)
    armv7_dcache_wbinv_range(targetPtr, size);
#endif
}


/**
* @internal srvCpuIpcServerInit function
* @endinternal
*
* @brief   Initialize IPC struct, umask ISR
*
* @param[in] ipcBase  - virtual address of shared memory block
*
* @retval none
*/
void srvCpuIpcServerInit(void* ipcBase)
{
    int rc;

    shmIpcInit( &shmS, ipcBase, SRV_CPU_IPC_SIZE, 1 /* Server */, wr_Ipc_sync, NULL);

    /* Create channel #1 with 2 buffers of 0x50 bytes for RX and for TX */
    rc = shmIpcConfigChannel(
        &shmS, 1 /* chn */,
        2, IPC_MAX_MSG_SIZE,   /* rx */
        2, IPC_MAX_MSG_SIZE);  /* tx */
    if (rc) {
        printf("Failed to configure IPC: %d\n", rc);
        return;
    }
#ifdef IPC_CLIENT
    /* Unmask IPC ISR between cores */
    srvCpuUnmaskIpcIrq();
#endif
};

/* Shared with appl */
struct IPC_MSG {
    unsigned int msg_num;
    char    data[IPC_MAX_MSG_SIZE-4];
};

/* TODO: only for demo purpose - hook your response message/functioning here */
void ipcMessageResponse(unsigned char srcId)
{
    static struct IPC_MSG msg;
    static unsigned int msg_num = 0;

    int size;

    if (shmIpcRxChnReady(&shmS, 1)) {
        size = sizeof(struct IPC_MSG);
        if (shmIpcRecv(&shmS, 1, &msg, &size)==1)
        {
            msg.data[sizeof(msg.data)-1] = 0; /* Make sure the string ends with NULL */
            msg_num++;
            if (srcId == 0xff)
                printf("RTOS got message #%d from Host: %s\n", msg_num, msg.data);
            else /* message from other SCPU*/
                printf("RTOS got message #%d from Core %d: %s\n", msg_num, srcId, msg.data);
            size = sizeof(struct IPC_MSG);
            strcpy(msg.data, "hello to you too");
            msg.msg_num = msg_num;
            if (shmIpcSend(&shmS, 1, &msg, size))
                printf("Failed to send response to message #%d\n", msg.msg_num);
        }
    }
}

#ifdef IPC_CLIENT

/**
* @internal srvCpuIpcClientInit function
* @endinternal
*
* @brief   connect to server shared memory block, check magic
*
* @param[in] ipcBase  - virtual address of shared memory block
*
* @retval none
*/
void srvCpuIpcClientInit(void* ipcBase)
{
    shmIpcInit( &shmC, ipcBase, SRV_CPU_IPC_SIZE, 0 /* Client */, wr_Ipc_sync, NULL);
}

/**
* @internal srvCpuIpcClientSendMessage function
* @endinternal
*
* @brief   Send message to IPC channel
*
* @param[in] ipcBase   - virtual address of shared memory block
* @param[in] buf       - data to send
* @param[in] bufSize   - data size
* @param[in] tgtId     - send an interrupt received message to the server (target ID)
*
* @retval none
*/
void srvCpuIpcClientSendMessage(char* ipcBase, char* buf, unsigned int bufSize, unsigned char tgtId)
{
    struct IPC_MSG msg = {0};
    int size, i, rc;

    msg.msg_num = 1;
    memcpy(msg.data, buf, bufSize);

    rc = shmIpcSend(&shmC, 1, (const void*)&msg, sizeof(msg));
    if (rc) {
        printf("shmIpcSend failed %d\n", rc);
        return;
    }
    /* send INTERRUPT to server */
    srvCpuRegWrite( srvCpuGetMgBase(tgtId) | (0x440 + 4*mgOwnId), 0x1);
    for (i=0; i<10; i++) {
        vTaskDelay(configTICK_RATE_HZ / 10);
        if (shmIpcRxChnReady(&shmC, 1)) {
            memset(&msg, 0, sizeof(msg));
            size = sizeof(struct IPC_MSG);
            if (shmIpcRecv(&shmC, 1, &msg, &size)==1)
                printf("Appl got response #%d: %s\n", msg.msg_num, msg.data);
            else
                printf("Failed to get response on channel 1\n");

            return;
        }
    }

    printf("Failed to get response on channel 1\n");
}
#endif /* IPC_CLIENT */
