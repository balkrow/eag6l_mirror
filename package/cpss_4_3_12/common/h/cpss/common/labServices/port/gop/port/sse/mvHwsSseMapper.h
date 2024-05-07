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
* @file mvHwsSseMapper.h
*
* @brief
*
* @version   1
********************************************************************************
*/

#ifndef _mvHwsSseMapper_H
#define _mvHwsSseMapper_H

#include <cpss/common/labServices/port/gop/port/sse/mvHwsSseCommon.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************** APIs decleration***********************************
*/

/**
* @internal  mvHwsSseMapperInit function
* @endinternal
*
* @brief  Initialize Sub Sequence Mapper
*
* @retval 0     - on success
* @retval 1     - on error
*/
GT_STATUS mvHwsSseMapperInit
(
    IN GT_VOID
);

/**
* @internal  mvHwsSseMapperDestroy function
* @endinternal
*
* @brief  Free Sub Seqence Mapper internal resources
*
*/
GT_VOID mvHwsSseMapperDestroy
(
    IN GT_VOID
);

/**
* @internal  mvHwsSseMapperLUTsClear  function
* @endinternal
*
* @brief    Delete all LUT's elements
*
*/
GT_VOID mvHwsSseMapperLUTsClear
(
    IN GT_VOID
);

/**
* @internal  mvHwsSseMapperOpTypeValidate function
* @endinternal
*
* @brief  Search for a given operation type to verify if it is
*         supported.This function may be called during script
*         file validation process
*
* @param[in] lutType              - Look Up Table type
*                                   (i.e. SERDES\PCS\MAC)
*
* @param[in] opType               - Specifies the operation
*                                   type as string
*
* @param[in] opTypeLen            - Specifies the operation type
*                                   string size (in bytes)
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperOpTypeValidate
(
    IN SSE_LUT_TYPE     lutType,
    IN GT_CHAR_PTR      opType,
    IN GT_U16           opTypeLen
);

/**
* @internal  mvHwsSseMapperKeyAttrValidate function
* @endinternal
*
* @brief  Search for a given key attribute argument to verify if
*         it is supported. This function may be called during
*         script file validation process
*
* @param[in] argName              - Specifies the argument name
*                                   as string
*
* @param[in] argLen               - Specifies the argument name
*                                   string size (in bytes)
*
* @retval on success              - _ARG_SERDES_NUM_CODE_up
*                                   to_LAST_ARG_CODE_
* @retval on error                - _ARG_CODE_NA
*/
SSE_ARG_CODE mvHwsSseMapperKeyAttrValidate
(
    IN GT_CHAR_PTR          argName,
    IN GT_U16               argNameLen
);

/**
* @internal  mvHwsSseMapperArgSeqListByLUTentryCreate function
* @endinternal
*
* @brief  Create argument and sequence list for specific LUT
*         entry. This function may be called several times for a
*         specific operation and order types for different
*         sequences
*
* @param[in] lutType              - Look Up Table type
*                                   (i.e. SERDES\PCS\MAC)
*
* @param[in] opType               - Specifies the operation
*                                   type as string
*
* @param[in] opTypeLen            - Specifies the operation type
*                                   string size (in bytes)
*
* @param[in] orderType            - Specifies the order type
*                                   (i.e. pre or post)
*
* @retval pointer to created key attribute entry    - on success
* @retval NULL                                      - on error
*/
GT_VOID_PTR mvHwsSseMapperArgSeqListByLUTentryCreate
(
    IN SSE_LUT_TYPE        lutType,
    IN GT_CHAR_PTR         opType,
    IN GT_U16              opTypeLen,
    IN SSE_ORDER_TYPE      orderType
);


/**
* @internal  mvHwsSseMapperArgsByKeyAttrFill function
* @endinternal
*
* @brief  Fill entry key attributes with arguments . This
*         function may be called several times for a specific
*         operation and order types for different sequences
*
* @param[in] keyAttrEntry         - pointer to key attributes
*                                   entry (i.e. return value of
*                                   mvHwsSseSsmCreateArgSeqListByLUTentry
*                                   function)
*
* @param[in] argName              - Specifies the argument name
*                                   as string
*
* @param[in] argLen               - Specifies the argument name
*                                   string size (in bytes)
*
* @param[in] argValue             - Specifies value of the
*                                   argName
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperArgsByKeyAttrFill
(
    IN GT_VOID_PTR      keyAttrEntry,
    IN GT_CHAR_PTR      argName,
    IN GT_U16           argLen,
    IN GT_U32           argValue,
    IN SEQ_IDX_T        seqNum
);

/**
* @internal  mvHwsSseMapperSeqByKeyAttrFill function
* @endinternal
*
* @brief  Fill entry sequence list with instructions . This
*         function may be called several times for a specific
*         operation and order types for different sequences
*
* @param[in] keyAttrEntry          - pointer to key attributes
*                                   entry (i.e. return value of
*                                   mvHwsSseSsmCreateArgSeqListByLUTentry
*                                   function)
*
* @param[in] seqInst              - pointer to single sequence
*                                   instruction
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperSeqByKeyAttrFill
(
    IN GT_VOID_PTR          keyAttrEntry,
    IN SSE_SEQ_INST         *seqInst
);

/**
* @internal  mvHwsSseMapperKeyAttrListGet function
* @endinternal
*
* @brief  Return total amount of key attributes for a specific
*         operation type, order and LUT type.
*
* @param[in] lutType              - look Up Table type
*                                   (i.e. SERDES\PCS\MAC)
*
* @param[in] opTypeIdx            - index to secific LUT entry
*
* @param[in] orderType            - specifies the order type
*                                   (i.e. pre or post)
*
* @param[out] keyAttrsNum         - total number of key
*                                   attributes per operation and
*                                   order type
*
* @param[out] keyAttrList         - pointer to key attributes
*                                   list that refers to
*                                   operation and order type
*
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperKeyAttrListGet
(
    IN  SSE_LUT_TYPE       lutType,
    IN  SSE_OP_TYPE        opTypeIdx,
    IN  SSE_ORDER_TYPE     orderType,
    OUT ATTR_IDX_T         *keyAttrsSize,
    OUT GT_VOID_PTR        *keyAttrList
);

/**
* @internal  mvHwsSseMapperArgListGet function
* @endinternal
*
* @brief  Return argument list of specific key attributes list's
*         node for a specific operation type, order and LUT
*         type.
*
* @param[in] keyAttrList          - pointer of key attributes
*                                   list
*
* @param[in] KeyAttrNodeNum       - index of specific key
*                                   attribute
*
* @param[out] argListSize         - size of argument list
*
* @param[out] argList             - pointer to requested
*                                   argument list
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperArgListGet
(
    IN GT_VOID_PTR          keyAttrList,
    IN ATTR_IDX_T           KeyAttrNodeNum,
    OUT ARG_IDX_T           *argListSize,
    OUT GT_VOID_PTR         *argList
);

/**
* @internal  mvHwsSseMapperSeqListGet function
* @endinternal
*
* @brief  Return sequence list of specific key attribute list's
*         node for a specific operation type, order and LUT
*         type.
*
* @param[in] keyAttrList          - pointer of key attributes
*                                   list
*
* @param[in] KeyAttrNodeNum       - index of specific key
*                                   attribute
*
* @param[out] seqListSize         - size of sequence list
*
* @param[out] seqList             - pointer to requested
*                                   sequence list
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperSeqListGet
(
    IN GT_VOID_PTR          keyAttrList,
    IN ATTR_IDX_T           KeyAttrNodeNum,
    OUT SEQ_IDX_T           *seqListSize,
    OUT GT_PTR              *seqList
);

/**
* @internal  mvHwsSseMapperArgsPairGet function
* @endinternal
*
* @brief  Retrive argument pair (argument core and value)
*         according to key attribute list's node, for a specific
*         operation type, order and LUT type.
*
* @param[in] keyAttrList          - pointer of key attributes
*                                   list
*
* @param[in] KeyAttrNodeNum       - index of specific key
*                                   attribute
*
* @param[out] seqListSize         - size of sequence list
*
* @param[out] seqList             - pointer to requested
*                                   sequence list
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvHwsSseMapperArgsPairGet
(
    IN GT_VOID_PTR          keyAttrArgList,
    IN GT_U8                pairNum,
    OUT SSE_ARG_CODE        *argCode,
    OUT GT_U32              *argValue
);

/**
* @internal  mvHwsSseMapperSeqNumMatchFind function
* @endinternal
*
* @brief  Search for sequence number in key attribute list for a
*         specific operation and order type . If there us a
*         match, a pointer to key attribute list will be return.
*
*
* @param[in] lutType                - look Up Table type
*                                     (i.e. SERDES\PCS\MAC)
*
* @param[in] opType                 - Specifies the operation
*                                     type as string
*
* @param[in] opTypeLen              - Specifies the operation
*                                     type string size (in
*                                     bytes)
* @param[in] orderType              - specifies the order type
*                                     (i.e. pre or post)
*
* @param[in] SeqNum                 - specifies the sequence
*                                     number to search
*
* @retval pointer to key attribute  - on success
*           entry that holds the seqNum
* @retval NULL                      - on no match
*/
GT_VOID_PTR mvHwsSseMapperSeqNumMatchFind
(
    IN  SSE_LUT_TYPE       lutType,
    IN  GT_CHAR_PTR        opType,
    IN  GT_U16             opTypeLen,
    IN  SSE_ORDER_TYPE     orderType,
    IN  SEQ_IDX_T          seqNum
);

/**
* @internal  mvHwsSseMapperKeyAttrObjDelete function
* @endinternal
*
* @brief  Deletes key attribute object from a LUT of a specific
*         type (i.e. SERDES\PCS\MAC).
*
*
* @param[in] lutType                - look Up Table type
*                                     (i.e. SERDES\PCS\MAC)
*
* @param[in] opType                 - Specifies the operation
*                                     type as string
*
* @param[in] opTypeLen              - Specifies the operation
*                                     type string size (in
*                                     bytes)
* @param[in] orderType              - specifies the order type
*                                     (i.e. pre or post)
*
* @param[in] keyAttrArgNode         - specifies the key
*                                     attribute node to be
*                                     deleted
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsSseMapperKeyAttrObjDelete
(
    IN  SSE_LUT_TYPE       lutType,
    IN  GT_CHAR_PTR        opType,
    IN  GT_U16             opTypeLen,
    IN  SSE_ORDER_TYPE     orderType,
    IN  GT_VOID_PTR        keyAttrArgNode
);

/******************************************************************************
*                       Sanity Test section                                   *
*******************************************************************************/

GT_VOID mvHwsSseMapperLUTdump
(
    IN SSE_LUT_TYPE        lutType,
    IN SSE_ORDER_TYPE      orderType
);


#ifdef __cplusplus
}
#endif
#endif /* _mvHwsSseMapper_H */

