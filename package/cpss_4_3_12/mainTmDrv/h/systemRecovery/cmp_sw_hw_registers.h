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
* @file cmp_sw_hw_registers.h
*
* @brief API declarations for Restoring Software DB by reading from hardware
*
* @version   1
********************************************************************************
*/

#ifndef _CMP_SW_HW_REGISTERS_H_
#define _CMP_SW_HW_REGISTERS_H_

#include "tm_core_types.h"

int cmp_sw_hw_max_dp_mode(tm_handle hndl);

int cmp_sw_hw_drop_aqm_mode(tm_handle hndl);

int cmp_sw_hw_queues_wred_curve(tm_handle hndl,uint8_t curve_ind);

int cmp_sw_hw_a_nodes_wred_curve(tm_handle hndl,  uint8_t curve_ind);

int cmp_sw_hw_b_nodes_wred_curve(tm_handle hndl,   uint8_t curve_ind);

int cmp_sw_hw_c_nodes_wred_curve(tm_handle hndl,  uint8_t cos, uint8_t curve_ind);

int cmp_sw_hw_ports_wred_curve(tm_handle hndl, uint8_t curve_ind);

int cmp_sw_hw_ports_wred_curve_cos(tm_handle hndl, uint8_t cos, uint8_t curve_ind);

int cmp_sw_hw_queue_drop_profile(tm_handle hndl, uint32_t prof_ind);

int cmp_sw_hw_a_nodes_drop_profile(tm_handle hndl, uint32_t prof_ind);

int cmp_sw_hw_b_nodes_drop_profile(tm_handle hndl, uint32_t prof_ind);

int cmp_sw_hw_c_nodes_drop_profile(tm_handle hndl,  uint8_t cos, uint32_t prof_ind);

int cmp_sw_hw_aging_status(tm_handle hndl);

int cmp_sw_hw_tree_deq_status(tm_handle hndl);

int cmp_sw_hw_tree_dwrr_priority(tm_handle hndl);

int cmp_sw_hw_shaping_status(tm_handle hndl, enum tm_level level);

int cmp_sw_hw_periodic_scheme(tm_handle hndl);

int cmp_sw_hw_gen_conf(tm_handle hndl);

int cmp_sw_hw_dwrr_limit(tm_handle hndl);

int cmp_sw_hw_elig_prio_func_tbl_q_level(tm_handle hndl);

int cmp_sw_hw_elig_prio_func_tbl_a_level(tm_handle hndl);

int cmp_sw_hw_elig_prio_func_tbl_b_level(tm_handle hndl);

int cmp_sw_hw_elig_prio_func_tbl_c_level(tm_handle hndl);

int cmp_sw_hw_elig_prio_func_tbl_p_level(tm_handle hndl);

int cmp_sw_hw_node_mapping(tm_handle hndl, enum tm_level lvl, uint32_t index);

int cmp_sw_hw_port(tm_handle hndl, uint8_t port_ind);

int cmp_sw_hw_ports_drop_profile(tm_handle hndl,  uint32_t prof_ind, uint8_t port_ind);

int cmp_sw_hw_port_drop_global(tm_handle hndl, uint8_t port_ind);

int cmp_sw_hw_node_elig_prio_function(tm_handle hndl, enum tm_level node_level, uint32_t node_index);

int cmp_sw_hw_c_node_params(tm_handle hndl, uint32_t node_ind);

int cmp_sw_hw_b_node_params(tm_handle hndl, uint32_t node_ind);

int cmp_sw_hw_a_node_params(tm_handle hndl, uint32_t node_ind);

int cmp_sw_hw_queue_params(tm_handle hndl, uint32_t queue_ind);

int cmp_sw_hw_node_shaping(tm_handle hndl, enum tm_level level, uint32_t node_ind);

#endif /* _CMP_SW_HW_REGISTERS_H_ */
