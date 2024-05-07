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
* @file flexeAgent.h
*
* @brief FLEXE AGENT high level API definitions
*
* @version   1
********************************************************************************
*/
#ifndef FLEXE_AGENT_H
#define FLEXE_AGENT_H

#include <flexeAgentTypes.h>

#if 0
/**************************************************************************
* fwSdmaTxInit
*
* DESCRIPTION:
*    Allocates a single tx descriptor, that points to single buffer
*    for a specific queue, and init that descriptor
*    Supplied mem block should be big enough for the descriptors &
*    buffers to be allocated according to their alignment requirements
*
* INPUT:
*     mgNum     - mg number
*     qNum      - queue number
*     pBlock    - pointer to memory block in SDMA SRAM to alloc from
*     blockSize - size of tx block in pBlock
*     bufSize   - size of tx buffer to be allocated
*
* OUTPUT:
*     None.
*
* RETURN:
*     MV_OK - on success
*     MV_FAIL - otherwise
*
**************************************************************************/
MV_STATUS fwSdmaTxInit(
    MV_U8  mgNum,
    MV_U32 qNum,
    MV_U8  *pBlock,
    MV_U32 blockSize,
    MV_U32 bufSize
);

/**************************************************************************
* fwSdmaTx
*
* DESCRIPTION:
*    Transmit a buffer through specified queue
*
* INPUT:
*     mgNum     - mg number
*     qNum      - queue number
*     bufSize   - size of tx buffer to be allocated
*     msgBuf    - pointer to message buffer
*
* OUTPUT:
*     None.
*
* RETURN:
*     MV_OK - on success
*     MV_FAIL - otherwise
*
**************************************************************************/
MV_STATUS fwSdmaTx(
    MV_U8   mgNum,
    MV_U32  qNum,
    MV_U32  bufSize,
    MV_VOID *msgBuf
);
void ipcInit(void* ipcBase, int ipcSize);
void ipcMsgGet(void);
#endif

GT_VOID flexeAgentTaskCreate
(
    GT_VOID
);
#endif /* FLEXE_AGENT_H */
