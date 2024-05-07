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
* @file flexeAgentTransit.h
*
* @brief FLEXE AGENT transit definitions
*
* @version   1
********************************************************************************
*/

#ifndef FLEXE_AGENT_TRANSIT_H
#define FLEXE_AGENT_TRANSIT_H

#include <flexeAgentTypes.h>

#ifdef FLEXE_AGENT_IN_CPSS

#define FLEXE_AGENT_IPC_MSG_MAX_SIZE_IN_WORDS   16
#define FLEXE_AGENT_IPC_MSG_MAX_NUM             64

typedef struct {
    GT_U32  sizeInBytes;
    GT_U32  data[FLEXE_AGENT_IPC_MSG_MAX_SIZE_IN_WORDS];
} FLEXE_AGENT_IPC_MSG_DATA_STC;

typedef struct {
    /* device number */
    GT_U8       devNum;

    /* Primary to Agent IPC buffer */
    FLEXE_AGENT_IPC_MSG_DATA_STC ipcP2A[FLEXE_AGENT_IPC_MSG_MAX_NUM];

    /* Agent to Primary IPC buffer */
    FLEXE_AGENT_IPC_MSG_DATA_STC ipcA2P[FLEXE_AGENT_IPC_MSG_MAX_NUM];

    GT_U32      firstFreeP2A;
    GT_U32      firstActiveP2A;
    GT_U32      firstFreeA2P;
    GT_U32      firstActiveA2P;
    GT_U32      doorBellA2P;
    GT_U32      doorBellP2A;
} FLEXE_AGENT_TRANSIT_DATA_STC;

#define FLEXE_AGENT_TRANSIT_DATA_PTR_GET(_dataPtr) \
    _dataPtr = (FLEXE_AGENT_TRANSIT_DATA_STC *)PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.genericFlexeDir.flexeDbPtr)
#define FLEXE_AGENT_TRANSIT_DATA_PTR_CHECK_MAC(_dbPtr)                                  \
    do {                                                                         \
        if (_dbPtr == NULL)                                                      \
        {                                                                        \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG); \
        }                                                                        \
    } while (0)
#endif


#ifndef FLEXE_AGENT_IN_CPSS
/**
* @internal flexeAgentShmObjGet
* @endinternal
*
* @brief   Return the SHM object pointer
*
* @retval GT_STATUS
*/
IPC_SHM_STC *flexeAgentShmObjGet
(
    GT_VOID
);
#endif


GT_STATUS flexeAgentHwRegRead
(
    GT_U32  address,
    GT_U32  *data,
    GT_U32  mask
);

GT_STATUS flexeAgentHwRegWrite
(
    GT_U32  address,
    GT_U32  data,
    GT_U32  mask
);


#endif /*FLEXE_AGENT_TRANSIT_H*/
