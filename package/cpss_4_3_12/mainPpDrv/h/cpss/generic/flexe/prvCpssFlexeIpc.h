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

#ifndef _pvCpssFlexeIpch
#define _prvCpssFlexeIPch

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include <cpss/generic/config/private/prvCpssConfigTypes.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/flexe/cpssFlexe.h>

/**
 * IPC Message Formats
 * Name             Words format
 * HW_INIT          W0[7:0]=opcode,  W0[8]=tileId
 * Hw_INIT_DONE     W0[7:0]=opcode,  W0[8]=tileId
 * GROUP_CREATE     W0[7:0]=opcode,  W0[8]=tileId,  W0[23:16] = instBmp, W0[31:24]=bondMaskInst0,
 *                  W1[31:0]=bondMaskInst[4,3,2,1],
 *                  W2[23:0]=bondMaskInst[7,6,5], W2[25:24]= interleaveConfig
 */

#define PRV_CPSS_FLEXE_IPC_MSG_MAX_SIZE_CNS 64


/**
 * @enum CPSS_FLEXE_IPC_MSG_OPCODE_ENT
 *
 * @brief enumerator for IPC message opcodes
 */
typedef enum {
    /** Invalid */
    CPSS_FLEXE_IPC_MSG_OPCODE_NONE_E,

    CPSS_FLEXE_IPC_MSG_OPCODE_HW_INIT_E,

    /** Group Create */
    CPSS_FLEXE_IPC_MSG_OPCODE_GROUP_CREATE_E,

} CPSS_FLEXE_IPC_MSG_OPCODE_ENT;


/**
 * @struct CPSS_FLEXE_IPC_MSG_OPCODE_GET_STC
 *
 * @brief dummy structure to read opcode of IPC message
 */
typedef struct {

    /** Opcode Word0[7:0] (8 bits) */
    CPSS_FLEXE_IPC_MSG_OPCODE_ENT      opcode;

} CPSS_FLEXE_IPC_MSG_OPCODE_GET_STC;


/**
 * @struct PRV_CPSS_FLEXE_IPC_MSG_GROUP_CREATE_STC
 *
 * @brief structure for Group Create IPC message
 */
typedef struct {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_FLEXE_IPC_MSG_OPCODE_ENT      opcode;

    /* tileId Word0[8] (4 bits) */
    GT_U8                              tileId;

    /* shim Id */
    GT_U8                              shimId;

    /* instance bitmap Word0[23:16] (4 bits) */
    GT_U8                              instanceBmp;

    /* bondMaskArr Word0[34:24] (8 bits)*/
    GT_U8                              bondMaskArr[CPSS_FLEXE_GROUP_INSTANCES_MAX_CNS];

    /* interlieve confug Word2[25:24] (8 bits) */
    GT_U8                              interleaveCfg;

} PRV_CPSS_FLEXE_IPC_MSG_GROUP_CREATE_STC;


typedef struct {

    /* Opcode Word0[7:0] (8 bits) */
    CPSS_FLEXE_IPC_MSG_OPCODE_ENT      opcode;

    /* portGroupsBmp Word0[8] (2 bits) */
    GT_U8                              portGroupsBmp;

} PRV_CPSS_FLEXE_IPC_MSG_HW_INIT_STC;

/**
 * @union PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT
 *
 * @brief union of IPC messages
 */
typedef union {

    /* Opcode of any IPC message */
    CPSS_FLEXE_IPC_MSG_OPCODE_ENT      opcode;

    /* GroupCreate IPC message */
    PRV_CPSS_FLEXE_IPC_MSG_GROUP_CREATE_STC        groupCreate;

    /* hwInit IPC message */
    PRV_CPSS_FLEXE_IPC_MSG_HW_INIT_STC  hwInit;

} PRV_CPSS_FLEXE_IPC_MSG_SEND_UNT;

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
);

/* [TBD] Comments */
GT_STATUS   prvCpssFlexeIpcFwInit
(
    IN  GT_U8       devNum,
    IN  GT_U8       sCpuId
);

#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _prvCpssFlexeIpch */
