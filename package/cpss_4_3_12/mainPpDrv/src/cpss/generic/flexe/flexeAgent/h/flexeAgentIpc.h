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
* @file flexeAgentIpc.h
*
* @brief FLEXE AGENT IPC related API definitions
*
* @version   1
********************************************************************************
*/

#ifndef FLEXE_AGENT_IPC_H
#define FLEXE_AGENT_IPC_H

#include <flexeAgentTypes.h>

#ifndef FLEXE_AGENT_IN_CPSS
#include <mvShmIpc.h>
#endif

/**
 * IPC Message Formats
 * Name             Words format
 * HW_INIT          W0[7:0]=opcode,  W0[9:8]=portGroupBmp
 * Hw_INIT_DONE     W0[7:0]=opcode,  W0[9:8]=portGroupBmp
 * GROUP_CREATE     W0[7:0]=opcode,  W0[8]=tileId,  W0[23:16] = instBmp, W0[31:24]=bondMaskInst0,
 *                  W1[31:0]=bondMaskInst[4,3,2,1],
 *                  W2[23:0]=bondMaskInst[7,6,5], W2[25:24]= interleaveConfig
 */

typedef struct {
    GT_U32    data[15];
} FLEXE_IPC_MSG_DATA_STC;

/**
 * @enum FLEXE_IPC_MSG_OPCODE_ENT
 *
 * @brief enum defining all IPC requests from client (primary) to server (agent)
 */
typedef enum
{
    FLEXE_IPC_MSG_OPCODE_HW_INIT_E,
    FLEXE_IPC_MSG_OPCODE_HW_INIT_DONE_E,
    FLEXE_IPC_MSG_OPCODE_GROUP_CREATE_E,
    FLEXE_IPC_MSG_OPCODE_GROUP_DELETE_E,
    FLEXE_IPC_MSG_OPCODE_CALENDAR_SWITCH_E,
    FLEXE_IPC_MSG_OPCODE_CALENDAR_SWITCH_DONE_E,
    FLEXE_IPC_MSG_OPCODE_LAST_E
} FLEXE_IPC_MSG_OPCODE_ENT;

/** [TBD] comments */
typedef struct {
    GT_U8       opcode;
    GT_U8       portGroupsBmp;
} FLEXE_IPC_MSG_HW_INIT_STC;

typedef struct {
    GT_U8       opcode;
    GT_U8       tileId;
    GT_U8       instanceBmp;
    GT_U8       bondMaskArr[FLEXE_NUM_INSTANCES_MAX];
    GT_U8       interleaveCfg;
} FLEXE_IPC_MSG_GROUP_CREATE_STC;

/** [TBD] comments */
typedef struct
{
    GT_U8                           opCode;
    FLEXE_IPC_MSG_HW_INIT_STC       hwInit;
    FLEXE_IPC_MSG_HW_INIT_STC       hwInitDone;
    FLEXE_IPC_MSG_GROUP_CREATE_STC  groupCreate;
}FLEXE_IPC_MSG_UNT;


/**
 * @structure FLEXE_IPC_MSG_PARAMS_STC
 *
 * @brief message request
 */
typedef struct
{
    FLEXE_IPC_MSG_UNT   *msgReqDataPtr;
    GT_U32              msgReqDataLen;
}FLEXE_IPC_MSG_PARAMS_STC;

GT_STATUS flexeAgentIpcMsgSendData
(
    FLEXE_IPC_MSG_DATA_STC  *msgDataPtr,
    GT_32                   sizeInBytes
);

GT_STATUS flexeAgentIpcMsgFetchData
(
    OUT FLEXE_IPC_MSG_DATA_STC  *msgDataPtr,
    OUT GT_32                   *sizeInBytesPtr
);

GT_STATUS flexeAgentIpcMsgFetchAndExecute
(
    GT_VOID
);

#endif /* FLEXE_AGENT_IPC_H */
