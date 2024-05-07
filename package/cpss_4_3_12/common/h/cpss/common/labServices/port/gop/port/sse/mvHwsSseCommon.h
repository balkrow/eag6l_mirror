/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file mvHwsSseCommon.h
*
* @brief    Common definitions for all SSE components
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsSseCommon_H
#define __mvHwsSseCommon_H

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_SERDES_SEQ_ARGS             (5)
#define MAX_SERDES_OP_TYPES             (SSE_OP_LAST_E)

#define ADDR_ARG_POS                    (0)
#define VALUE_ARG_POS                   (ADDR_ARG_POS + 1)
#define DATA_ARG_POS                    (ADDR_ARG_POS + 1)
#define MASK_ARG_POS                    (ADDR_ARG_POS + 2)
#define TIMEOUT_ARG_POS                 (ADDR_ARG_POS + 3)
#define RD_DELAY_ARG_POS                (ADDR_ARG_POS + 4)

typedef GT_U8                           ATTR_IDX_T;
typedef GT_U8                           ARG_IDX_T;
typedef GT_U8                           SEQ_IDX_T;

typedef GT_PTR                          MV_HWS_LL_DESC;

typedef enum
{
    SSE_LUT_NA_E                     = 0,
    SSE_LUT_SERDES_E                 = 1,
    SSE_LUT_PCS_E                    = 2,
    SSE_LUT_MAC_E                    = 3,
    SSE_LUT_LAST_E
}SSE_LUT_TYPE;

typedef enum
{
    SSE_ORDER_NA_E                   = 0,
    SSE_ORDER_PRE_E                  = 1,
    SSE_ORDER_POST_E                 = 2,
    SSE_ORDER_LAST_E
}SSE_ORDER_TYPE;

typedef enum
{
    SSE_OPCODE_NA_E	                = 0,
    SSE_OPCODE_LOOP_E               = 1,
    SSE_OPCODE_END_LOOP_E           = 2,
    SSE_OPCODE_WRITE_E              = 3,
    SSE_OPCODE_POLL_E               = 4,
    SSE_OPCODE_DELAY_E              = 5,
    SSE_OPCODE_LAST_E
}SSE_OPCODE;

typedef enum
{
    SSE_ARG_CODE_NA_E                       = 0,
    SSE_ARG_CODE_SERDES_NUM_E               = 1,
    SSE_ARG_CODE_BAUD_RATE_E                = 2,
    SSE_ARG_CODE_PWR_UP_E                   = 3,
    SSE_ARG_CODE_ANALOG_RESET_E             = 4,
    SSE_ARG_CODE_DIGITAL_RESET_E            = 5,
    SSE_ARG_CODE_SYNC_ERESET_E              = 6,
    SSE_ARG_CODE_RX_TRAINING_E              = 7,
    SSE_ARG_CODE_SERDES_OPERATION_MODE_E    = 8,
    SSE_ARG_CODE_LAST_E
}SSE_ARG_CODE;

typedef enum
{
    SSE_OP_SERDES_RESET_E                   = 0,
    SSE_OP_SERDES_ARRAY_PWR_UP_CTRL_E       = 1,
    SSE_OP_SERDES_RX_AUTO_TUNE_START        = 2,
    SSE_OP_SERDES_SERDES_OPERATION_TYPE_E   = 3,
    SSE_OP_LAST_E
}SSE_OP_TYPE;

typedef enum
{
    SSE_KEY_ATTR_NA_E                = 0,
    SSE_KEY_ATTR_BOOL_E              = 1,
    SSE_KEY_ATTR_SIGN_DECIMAL_E      = 2,
    SSE_KEY_ATTR_UNSIGN_DECIMAL_E    = 3,
    SSE_KEY_ATTR_HEX_E               = 4,
    SSE_KEY_ATTR_STRING_E            = 5,
    SSE_KEY_ATTR_LAST_E
}SSE_KEY_ATTR_TYPES;

typedef struct
{
    MV_HWS_LL_DESC                  *pre;
    MV_HWS_LL_DESC                  *post;
}SSE_LUT_ENTRY;

typedef struct
{
    GT_U8                           lineNum;
    SSE_OPCODE                      opcode;
    GT_U32                          seqArgs[MAX_SERDES_SEQ_ARGS];
}SSE_SEQ_INST;

/**************************** APIs decleration***********************************
*/


#ifdef __cplusplus
}
#endif
#endif /* __mvHwsSseCommon_H */

