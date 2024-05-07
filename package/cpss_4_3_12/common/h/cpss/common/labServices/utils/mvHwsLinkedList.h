/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsLinkedList.h
*
* @brief Linked list function definition
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsLinkedList_H
#define __mvHwsLinkedList_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>

/**
* @internal mvLLlistCreate function
* @endinternal
*
* @brief  Create a list including specified assignments
* @param[in] dataSize             - size of node data
*
* @retval list decriptor          - on success
* @retval NULL                    - on error
*/
MV_HWS_LL_DESC mvLLlistCreate
(
    IN GT_U16  dataSize
);

/**
* @internal mvLLlistDestroy function
* @endinternal
*
* @brief  Frees all memory allocated to nodes of the specified
*         list
* @param[in] listDesc             - linked list descriptor
*
* @retval None
*/
GT_VOID mvLLlistDestroy
(
    IN MV_HWS_LL_DESC *listDesc
);

/**
* @internal mvLLHeadAdd function
* @endinternal
*
* @brief  Allocates memory for a new node and place it in the
*         head pf the list
*
* @param[in] listDesc             - linked list descriptor
* @param[in] data                 - pointer to
*                                   damvHwsLlAddHeadandRetriveTestta
*                                   to be stored in the new node
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLHeadAdd
(
    IN MV_HWS_LL_DESC   listDesc,
    IN GT_PTR           data
);

/**
* @internal mvLLTailAdd function
* @endinternal
*
* @brief  Allocates memory for a new node and place it at the
*         end of the list
*
* @param[in] listDesc             - linked list descriptor
* @param[in] data                 - pointer to data to be stored
*                                   in the new node
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLTailAdd
(
    IN MV_HWS_LL_DESC   listDesc,
    IN GT_PTR           data
);

/**
* @internal mvLLHeadRemove function
* @endinternal
*
* @brief  Frees all allocated memory for the first (Head) node
*         of the list
*
* @param[in] listDesc             - linked list descriptor
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLHeadRemove
(
    IN MV_HWS_LL_DESC   listDesc
);

/**
* @internal mvLLTailRemove function
* @endinternal
*
* @brief  Frees all allocated memory for last node of the list
*
* @param[in] listDesc             - linked list descriptor
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLTailRemove
(
    IN MV_HWS_LL_DESC   listDesc
);

/**
* @internal mvLLAllRemove function
* @endinternal
*
* @brief  Delete all elements from list
*
* @param[in] listDesc             - linked list descriptor
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLAllRemove
(
    IN MV_HWS_LL_DESC   listDesc
);

/**
* @internal mvLLHeadRetrive function
* @endinternal
*
* @brief  Returns a pointer to a data stored at the 1st node of
*         the list
*
* @param[in] listDesc             - linked list descriptor
*
* @retval pointer to data on success    - on success
* @retval NULL                          - on error
*/
GT_PTR mvLLHeadRetrive
(
    IN MV_HWS_LL_DESC   listDesc
);

/**
* @internal mvLLTailRetrive function
* @endinternal
*
* @brief  Returns a pointer to a data stored at the last node of
*         the list
*
* @param[in] listDesc             - linked list descriptor
*
* @retval pointer to data on success    - on success
* @retval NULL                          - on error
*/
GT_PTR mvLLTailRetrive
(
    IN MV_HWS_LL_DESC   listDesc
);

/**
* @internal mvLLNodeByIndexRetrive function
* @endinternal
*
* @brief  Returns a pointer to a specific node of the list
*         according to an index
*
* @param[in] listDesc             - linked list descriptor
*
* @param[in] nthEle               - number of element to retrive
*                                   its data
*                                   data
*
* @retval pointer to data on success    - on success
* @retval NULL                          - on error
*/
GT_PTR mvLLNodeByIndexRetrive
(
    IN MV_HWS_LL_DESC   listDesc,
    IN GT_U16           nthEle
);

/**
* @internal mvLLDataByNodeRetrive function
* @endinternal
*
* @brief  Returns a pointer to a data that is stored in a
*         specific node of the list (see aNode argument)
*
* @param[in] listDesc             - linked list descriptor
*
* @param[in] aNode                - pointer to a specific node
*                                   that contains the data
*
* @retval pointer to data on success    - on success
* @retval NULL                          - on error
*/
GT_PTR mvLLDataByNodeRetrive
(
    IN MV_HWS_LL_DESC   listDesc,
    IN GT_PTR           aNode
);

/**
* @internal mvLLDataByIndexRetrive function
* @endinternal
*
* @brief  Returns a pointer to a data that is stored in a
*         specific node of the list (according to an index)_
*
* @param[in] listDesc             - linked list descriptor
*
* @param[in] nthEle               - number of element to retrive
*                                   its data
*                                   data
*
* @retval pointer to data on success    - on success
* @retval NULL                          - on error
*/
GT_PTR mvLLDataByIndexRetrive
(
    IN MV_HWS_LL_DESC   listDesc,
    IN GT_U16           nthEle
);

/**
* @internal mvLLlistSizeGet function
* @endinternal
*
* @brief  Returns the total number of elements
*
* @param[in] listDesc             - linked list descriptor
*
* @param[out] listSize            - total number of elements
*
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLlistSizeGet
(
    IN  MV_HWS_LL_DESC   listDesc,
    OUT GT_U8_PTR        listSize
);

/**
* @internal mvLLNodeDelete function
* @endinternal
*
* @brief Delete specific node from list
*
* @param[in] listDesc             - linked list descriptor
*
* @param[out] node                - node to be deleted
*
* @retval 0                       - on success
* @retval 1                       - on error
*/
GT_STATUS mvLLNodeDelete
(
    IN  MV_HWS_LL_DESC   listDesc,
    IN  GT_PTR           node
);

#endif /* mvHwsLinkedList */



