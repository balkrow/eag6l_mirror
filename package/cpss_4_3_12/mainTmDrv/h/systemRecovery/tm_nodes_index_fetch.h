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
* @file tm_nodes_index_fetch.h
*
* @brief APIs to create nodes by given index
*
* @version   1
********************************************************************************
*/

#ifndef __tmNodesIndexFetchh
#define __tmNodesIndexFetchh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int tm_create_a_node_to_b_node_index_fetch
(
    tm_handle   hndl,
    uint32_t    parent_b_node_index,
    struct tm_a_node_params *a_params,
    uint32_t    *a_node_index_ptr,
    uint32_t    *first_queue_index_ptr
);

int tm_create_b_node_to_c_node_index_fetch
(
    tm_handle   hndl,
    uint32_t    parent_c_node_index,
    struct tm_b_node_params *b_params,
    uint32_t    *b_node_index_ptr,
    uint32_t    *first_a_node_index_ptr
);

int tm_create_c_node_to_port_index_fetch
(
    tm_handle   hndl,
    uint32_t    parent_c_node_index,
    struct tm_c_node_params *c_params,
    uint32_t    *c_node_index_ptr,
    uint32_t    *first_b_node_index_ptr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tmNodesIndexFetchh */


