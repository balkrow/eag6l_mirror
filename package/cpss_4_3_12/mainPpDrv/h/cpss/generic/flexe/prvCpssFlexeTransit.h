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
* @file prvCpssFlexeTransit.h
*
* @brief FLEXE Primay transit definitions
*
* @version   1
********************************************************************************
*/

#ifndef _cpssFlexeTransith
#define _cpssFlexeTransith

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include <cpss/generic/flexe/cpssFlexeTypes.h>

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
} PRV_CPSS_FLEXE_TRANSIT_DATA_STC;

#endif


#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _cpssFlexeTransith */
