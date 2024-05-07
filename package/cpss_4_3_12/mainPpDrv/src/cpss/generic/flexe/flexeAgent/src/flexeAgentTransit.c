/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file flexeAgentTransit.c
*
* @brief FLEXE AGENT transit APIs Implementation
*
* @version   1
********************************************************************************
*/

#include <flexeAgentTypes.h>
#include <flexeAgent.h>
#include <flexeAgentIpc.h>
#include <flexeAgentTransit.h>

#ifdef FLEXE_AGENT_IN_CPSS
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#else
/* IPC object */
static IPC_SHM_STC flexeIpcShmObj;

IPC_SHM_STC *flexeAgentShmObjGet
(
    GT_VOID
)
{
    return &flexeIpcShmObj;
}
#endif

/* [TBD] Device number is hardcoded need to fetch it in another way */
#define FLEXE_AGENT_DEFAULT_DEV_NUM 0

static FLEXE_TASK prvFlexeAgentTask
(
    GT_VOID *paramsPtr
)
{
    (void) paramsPtr;
    while (1)
    {
         /* IPC treatment: During "idle" time or every PACKETS_PER_IPC_CHECK packets */
         flexeAgentIpcMsgFetchAndExecute();
    }

#ifdef FLEXE_AGENT_IN_CPSS
    return 0;
#endif
}

GT_VOID flexeAgentTaskCreate
(
    GT_VOID
)
{
#ifndef FLEXE_AGENT_IN_CPSS
    xTaskCreate( prvFlexeAgentTask, ( const char * ) "FLEXE_AGENT",
                configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 1, NULL );
#else
    GT_STATUS rc = GT_OK;
    GT_TASK   taskId;

    rc = cpssOsTaskCreate("flexeAgent",
                          255, /* priority */
                          _1KB, /* stack */
                          prvFlexeAgentTask,
                          NULL /* arg */,
                          &taskId);
    FLEXE_ERR_CHECK_AND_PRINT_MAC(rc, "cpssOsTaskCreate failed rc=%d");
#endif
}

GT_STATUS flexeAgentHwRegRead
(
    GT_U32  address,
    GT_U32  *data,
    GT_U32  mask
)
{
    GT_STATUS rc = GT_OK;
    if(mask == 0)
        mask = 0xFFFFFFFF;

#ifndef FLEXE_AGENT_IN_CPSS
    *data   = (srvCpuRegRead(address) & mask);
#else
    rc = prvCpssHwPpReadRegBitMask(FLEXE_AGENT_DEFAULT_DEV_NUM, address, mask, data);
#endif
    return rc;
}

GT_STATUS flexeAgentHwRegWrite
(
    GT_U32  address,
    GT_U32  data,
    GT_U32  mask
)
{
    GT_STATUS   rc = GT_OK;
    if(mask == 0)
        mask = 0xFFFFFFFF;

#ifndef FLEXE_AGENT_IN_CPSS
    if (mask != 0xFFFFFFFF)
    {
        srvCpuRegWrite(address, (srvCpuRegRead(address) & ~mask) | (data & mask));
    }
    else
    {
        srvCpuRegWrite(address, data);
    }
#else
    rc = prvCpssHwPpWriteRegBitMask(FLEXE_AGENT_DEFAULT_DEV_NUM, address, mask, data);
#endif
    return rc;
}

GT_STATUS flexeAgentIpcMsgSendData
(
    IN  FLEXE_IPC_MSG_DATA_STC  *msgDataPtr,
    IN  GT_32                   sizeInBytes
)
{
    GT_STATUS rc = GT_OK;

#ifndef FLEXE_AGENT_IN_CPSS
    shmIpcSend(&flexeIpcShmObj, 1, msgDataPtr, sizeInBytes);
#else
    FLEXE_AGENT_TRANSIT_DATA_STC    *transitDataPtr;

    FLEXE_AGENT_TRANSIT_DATA_PTR_GET(transitDataPtr);
    FLEXE_AGENT_TRANSIT_DATA_PTR_CHECK_MAC(transitDataPtr);

    if (transitDataPtr->firstFreeA2P == transitDataPtr->firstActiveA2P)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    transitDataPtr->ipcP2A[transitDataPtr->firstFreeA2P].sizeInBytes = sizeInBytes;
    memcpy(transitDataPtr->ipcP2A[transitDataPtr->firstFreeA2P].data, msgDataPtr->data, sizeof(msgDataPtr->data));
    transitDataPtr->firstFreeA2P ++;
    if (transitDataPtr->firstFreeA2P >= FLEXE_AGENT_IPC_MSG_MAX_NUM)
    {
        transitDataPtr->firstFreeA2P = 0;
    }
#endif
    return rc;
}

GT_STATUS flexeAgentIpcMsgFetchData
(
    OUT FLEXE_IPC_MSG_DATA_STC  *msgDataPtr,
    OUT GT_32                   *sizeInBytesPtr
)
{
    GT_STATUS rc = GT_OK;

#ifndef FLEXE_AGENT_IN_CPSS
    if (shmIpcRxChnReady(&flexeIpcShmObj, 1))
    {
        flexeAgentOsMemSet(msgDataPtr, 0, sizeof(FLEXE_IPC_MSG_DATA_STC));
        *sizeInBytesPtr = sizeof(FLEXE_IPC_MSG_DATA_STC);
        if (shmIpcRecv(&flexeIpcShmObj, 1, msgDataPtr, sizeInBytesPtr)!=1)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }
#else
    FLEXE_AGENT_TRANSIT_DATA_STC    *transitDataPtr;

    FLEXE_AGENT_TRANSIT_DATA_PTR_GET(transitDataPtr);
    FLEXE_AGENT_TRANSIT_DATA_PTR_CHECK_MAC(transitDataPtr);

    if (transitDataPtr->firstFreeP2A == transitDataPtr->firstActiveP2A)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_EMPTY, LOG_ERROR_NO_MSG);
    }
    memcpy(msgDataPtr->data, transitDataPtr->ipcP2A[transitDataPtr->firstActiveP2A].data, sizeof(msgDataPtr->data));
    *sizeInBytesPtr = transitDataPtr->ipcP2A[transitDataPtr->firstActiveP2A].sizeInBytes;
    if (transitDataPtr->firstActiveP2A >= FLEXE_AGENT_IPC_MSG_MAX_NUM)
    {
        transitDataPtr->firstActiveP2A = 0;
    }
#endif
    return rc;
}

