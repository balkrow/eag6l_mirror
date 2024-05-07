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
* @file flexeAgentIpc.c
*
* @brief FLEXE AGENT IPC related API implementation
*
* @version   1
********************************************************************************
*/

#ifndef FLEXE_AGENT_IN_CPSS
    #include <FreeRTOS.h>
    #include <task.h>
    #include <string.h>
    /* Demo app includes. */
    #include <global.h>
#else

#endif

#include <flexeAgent.h>
#include <flexeAgentIpc.h>
#include <flexeAgentHwConfig.h>
#include <flexeAgentTransit.h>

#ifndef FLEXE_AGENT_IN_CPSS
/* TODO: Sram size should be retrieve from somewhere... */
#define AAS_CM3_SRAM_SIZE           0x60000
#define LINK_MNG_CTRL_IPC_CHANNEL   0


#if 0
static void sync_ipc_memory(
    void* cookie,
    IPC_SHM_SYNC_FUNC_MODE_ENT mode,
    void*   ptr,
    IPC_UINTPTR_T targetPtr,
    IPC_U32 size
)
{
    IPC_U32 i;
    /* direct access only */
    (void)cookie;
    if (mode == IPC_SHM_SYNC_FUNC_MODE_READ_E)
    {
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
}

void ipcInit(void* ipcBase, int ipcSize)
{
    shmIpcInit(&shm, ipcBase, ipcSize, 0 /* slave */, sync_ipc_memory, NULL);
}
#endif
#endif


/** [TBD] comments */
static GT_STATUS flexeAgentIpcMsgFetch
(
    OUT FLEXE_IPC_MSG_UNT      *ipcMsgPtr
)
{
    GT_STATUS rc = GT_OK;
    FLEXE_IPC_MSG_DATA_STC   msgData;
    GT_32                    size;

    FLEXE_NULL_PTR_CHECK_MAC(ipcMsgPtr);

    rc = flexeAgentIpcMsgFetchData(&msgData, &size);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentIpcMsgFetchData failed");

    ipcMsgPtr->opCode = (FLEXE_IPC_MSG_OPCODE_ENT)msgData.data[0] & 0xF;
    switch (ipcMsgPtr->opCode)
    {
    case FLEXE_IPC_MSG_OPCODE_HW_INIT_E:
        ipcMsgPtr->hwInit.portGroupsBmp = (msgData.data[0] >> 8) & 3;
        break;
    default:
        rc = GT_BAD_VALUE;
        FLEXE_DEBUG_PRINT_MAC("Invalid Opcode: %d", ipcMsgPtr->opCode);
        break;
    }

    return rc;
}

static GT_STATUS flexeAgentIpcMsgSend
(
    IN FLEXE_IPC_MSG_UNT      *ipcMsgPtr
)
{
    GT_STATUS rc = GT_OK;
    FLEXE_IPC_MSG_DATA_STC  msgData;
    GT_32                   sizeInBytes;

    FLEXE_NULL_PTR_CHECK_MAC(ipcMsgPtr);

    msgData.data[0] = (GT_U32)(ipcMsgPtr->opCode & 0xFF);

    switch (ipcMsgPtr->opCode)
    {
    case FLEXE_IPC_MSG_OPCODE_HW_INIT_DONE_E:
        msgData.data[0] |= ((ipcMsgPtr->hwInit.portGroupsBmp & 0x3) << 8);
        sizeInBytes = 2;
        break;
    default:
        rc = GT_BAD_VALUE;
        FLEXE_DEBUG_PRINT_MAC("Invalid Opcode: %d", ipcMsgPtr->opCode);
        break;
    }

    rc = flexeAgentIpcMsgSendData(&msgData,sizeInBytes);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentIpcMsgSendData failed");
    return rc;
}

/** [TBD] comments */
static GT_STATUS flexeAgentIpcMsgExecute
(
    IN  FLEXE_IPC_MSG_UNT      *ipcMsgPtr
)
{
    GT_STATUS rc = GT_OK;
    FLEXE_IPC_MSG_UNT       ipcMsgSend;
    GT_U32                  i;
    FLEXE_AGENT_GROUP_CONFIG_STC groupConfig;

    flexeAgentOsMemSet(&ipcMsgSend, 0, sizeof(ipcMsgSend));

    FLEXE_NULL_PTR_CHECK_MAC(ipcMsgPtr);
    switch (ipcMsgPtr->opCode)
    {
    case FLEXE_IPC_MSG_OPCODE_HW_INIT_E:
        for (i=0; i<FLEXE_NUM_TIlES_MAX; i++)
        {
            if (ipcMsgPtr->hwInit.portGroupsBmp & (1<<i))
            {
                /* Perform Hw Init */
                rc = flexeAgentHwInit(i);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentHwInit failed");
            }
        }

        /* Create and send IPC message */
        ipcMsgSend.opCode = FLEXE_IPC_MSG_OPCODE_HW_INIT_DONE_E;
        ipcMsgSend.hwInitDone.portGroupsBmp = ipcMsgPtr->hwInit.portGroupsBmp;
        rc = flexeAgentIpcMsgSend(&ipcMsgSend);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentIpcMsgSend failed");
        break;

    case FLEXE_IPC_MSG_OPCODE_GROUP_CREATE_E:
        flexeAgentOsMemSet(&groupConfig, 0, sizeof(groupConfig));
        /* [TBD] Fill other params */
        groupConfig.interleaveCfg = ipcMsgPtr->groupCreate.interleaveCfg;
        for (i=0; i<FLEXE_NUM_INSTANCES_MAX; i++)
        {
            groupConfig.instance[i].bondMask = ipcMsgPtr->groupCreate.bondMaskArr[i];
        }

        rc = flexeAgentGroupCreate(ipcMsgPtr->groupCreate.tileId,
                                   &groupConfig);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentGroupCreate failed");
        break;
    case FLEXE_IPC_MSG_OPCODE_GROUP_DELETE_E:
        rc = flexeAgentGroupDelete(ipcMsgPtr->groupCreate.tileId,
                                   ipcMsgPtr->groupCreate.instanceBmp,
                                   ipcMsgPtr->groupCreate.bondMaskArr,
                                   ipcMsgPtr->groupCreate.interleaveCfg);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentGroupCreate failed");
        break;
    default:
        rc = GT_BAD_VALUE;
        FLEXE_DEBUG_PRINT_MAC("Invalid Opcode: %d", ipcMsgPtr->opCode);
        break;
    }
    return rc;
}

/** [TBD] comments */
GT_STATUS flexeAgentIpcMsgFetchAndExecute
(
    GT_VOID
)
{
    GT_STATUS               rc;
    FLEXE_IPC_MSG_UNT       ipcMsg;

    flexeAgentOsMemSet(&ipcMsg, 0, sizeof(ipcMsg));

    rc = flexeAgentIpcMsgFetch(&ipcMsg);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeAgentIpcMsgFetch failed");

    rc = flexeAgentIpcMsgExecute(&ipcMsg);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "flexeMagentIpcMsgExecute failed");

    return GT_OK;
}
