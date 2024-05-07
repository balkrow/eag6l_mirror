/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
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
* @file prvCpssFlexeIpc.h
*
* @brief CPSS FlexE IPC communication definitions
*
* @version   1
********************************************************************************
*/

#include <cpss/generic/flexe/prvCpssFlexe.h>
#include <cpss/generic/flexe/prvCpssFlexeIpc.h>

/* set bits in input stream */
static GT_VOID prvCpssFlexeBitsSet
(
    IN    GT_U32   offset,
    IN    GT_U32   numBits,
    IN    GT_U32   val,
    INOUT GT_U8    *data
)
{
    GT_U32 ii, byte, bitShift;
    GT_U8 mask = 0;
    for(ii = 0; ii < numBits; ii++)
    {
        byte     = (offset + ii)/8;
        bitShift = (offset + ii) - byte*8;
        mask = ~(1 << bitShift);
        data[byte] |= (data[byte] & mask) | (((val >> ii) & 1) << bitShift);
    }
}



/**
 * @internal prvCpssFlexeIpcMessageSend function
 * @endinternal
 *
 * @brief send ipc mesage to service cpu
 *
 * @param[in] devNum   - device number
 * @param[in] ipcMsg  - IPC message
 *
 * @retVal    GT_OK     - on success
 */
GT_STATUS prvCpssFlexeIpcMessageSend
(
    IN GT_U8                           devNum,
    IN PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT *ipcMsg
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U8                           ipcData[PRV_CPSS_FLEXE_IPC_MSG_MAX_SIZE_CNS] = {0};
    CPSS_FLEXE_IPC_MSG_OPCODE_ENT   opcode;
    GT_U32                          startOffset = 0;
    GT_U32                          ipcMsgSize = 0;

    (void) devNum;
    opcode = ipcMsg->opcode;

    /* opcode */
    ipcData[0] = (GT_U8)opcode;
    startOffset = 8;

    switch (opcode)
    {
    case CPSS_FLEXE_IPC_MSG_OPCODE_HW_INIT_E:
        prvCpssFlexeBitsSet(startOffset, 2, (GT_U32)ipcMsg->hwInit.portGroupsBmp, ipcData);
        startOffset += 4;
        break;
    /* Entry Add */
    case CPSS_FLEXE_IPC_MSG_OPCODE_GROUP_CREATE_E:
        #if 0
        /* Group identifier Word0[11:8] (4 bits) */
        prvCpssFlexeBitsSet(startOffset, 4, (GT_U32)ipcMsg->groupCreate.groupId, ipcData);
        startOffset += 4;

        /* number of phys Word0[15:12] (4 bits) */
        prvCpssFlexeBitsSet(startOffset, 4, (GT_U32)ipcMsg->groupCreate.numPhys, ipcData);
        startOffset += 4;

        /* Physical port number Word0[23:16] (8 bits)*/
        prvCpssFlexeBitsSet(startOffset, 8, (GT_U32)ipcMsg->groupCreate.portNum, ipcData);
        startOffset += 8;

        /* first PHY Id Word0[31:24] (8 bits) */
        prvCpssFlexeBitsSet(startOffset, 8, (GT_U32)ipcMsg->groupCreate.firstPhyId, ipcData);
        startOffset += 8;
        #endif
        /* IPC message size is two words */
        ipcMsgSize = (startOffset + 7) / 8; /* in bytes */

        break;

    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, "illegal opcode");
    }

    (void) ipcMsgSize;

    rc = prvCpssGenericSrvCpuIpcMessageSendCpuId(devNum, 0/*scpuId*/,1/*channel*/,ipcData, ipcMsgSize);
    PRV_CPSS_FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "prvCpssGenericSrvCpuIpcMessageSendCpuId failed");
    return rc;
}

