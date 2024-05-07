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
* @file prvCpssTmCatchUp.c
*
* @brief CPSS BobK TM  catch up utilities.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/generic/tm/cpssTmServices.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <core/tm_ctl_internal.h>
#include <core/set_hw_registers.h>
#include <systemRecovery/prvCpssTmCatchup.h>
#include <systemRecovery/sync_sw_registers.h>
#include <systemRecovery/cmp_sw_hw_registers.h>

#define __TM_MISMATCH_HANDLE(_set_func)                             \
    if ((rc != 0)  && (updateHw == GT_TRUE))                        \
    {                                                               \
        CPSS_LOG_INFORMATION_MAC("Fixing mismatch: " #_set_func);   \
        rc = _set_func;                                             \
    }                                                               \
    __TM_CATCHUP_ERR_LOG(rc, "Fixing mismatch failed: " #_set_func)

/**
 */
static GT_STATUS prvCpssTmCmpSwHw(GT_U8 devNum, GT_BOOL updateHw)
{
    GT_STATUS   ret = GT_OK;
    uint32_t    i;
    uint8_t     j;
    int rc = 0;
    int rc1 = 0;

    GT_VOID_PTR hndl = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

    TM_CTL(ctl,hndl)

    rc = cmp_sw_hw_max_dp_mode(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_max_dp_mode failed")
    __TM_MISMATCH_HANDLE(set_hw_max_dp_mode(hndl))

    rc = cmp_sw_hw_drop_aqm_mode(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_drop_aqm_mode failed")
    __TM_MISMATCH_HANDLE(set_hw_drop_aqm_mode(hndl))

    for (i=0; i<TM_NUM_WRED_QUEUE_CURVES; i++)
    {
        rc = cmp_sw_hw_queues_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_queues_wred_curve failed. idx = %d", i)
        __TM_MISMATCH_HANDLE(set_hw_queues_wred_curve(hndl, (uint8_t)i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_A_NODE_CURVES; i++)
    {
        rc = cmp_sw_hw_a_nodes_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_a_nodes_wred_curve failed. idx = %d", i)
        __TM_MISMATCH_HANDLE(set_hw_a_nodes_wred_curve(hndl, (uint8_t)i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_B_NODE_CURVES; i++)
    {
        rc = cmp_sw_hw_b_nodes_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_b_nodes_wred_curve failed. idx = %d", i)
        __TM_MISMATCH_HANDLE(set_hw_b_nodes_wred_curve(hndl, (uint8_t)i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_C_NODE_CURVES; i++)
    {
        for (j=0; j<TM_WRED_COS; j++)
        {
            rc = cmp_sw_hw_c_nodes_wred_curve(hndl, j, (uint8_t)i);
            __TM_CATCHUP_ERR_ARG2_LOG(rc, "cmp_sw_hw_c_nodes_wred_curve failed. i=%d, j=%d", i, j)
            __TM_MISMATCH_HANDLE(set_hw_c_nodes_wred_curve(hndl, j, (uint8_t)i))
            __TM_BREAK_LOOP_ON_ERROR
        }
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_PORT_CURVES; i++)
    {
        rc = cmp_sw_hw_ports_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_ports_wred_curve failed. idx = %d", i)
        __TM_MISMATCH_HANDLE(set_hw_ports_wred_curve(hndl, (uint8_t)i))
        __TM_BREAK_LOOP_ON_ERROR
        for (j=0; j<TM_WRED_COS; j++)
        {
            rc = cmp_sw_hw_ports_wred_curve_cos(hndl, j, (uint8_t)i);
            __TM_CATCHUP_ERR_ARG2_LOG(rc, "cmp_sw_hw_ports_wred_curve_cos failed. i=%d, j=%d", i, j)
            __TM_MISMATCH_HANDLE(set_hw_ports_wred_curve_cos(hndl, j, (uint8_t)i))
            __TM_BREAK_LOOP_ON_ERROR
        }
        __TM_BREAK_LOOP_ON_ERROR
    }

    /* No need to set Port Drop porfiles, it's done inside
     * cmp_sw_hw_port later */

    rc = cmp_sw_hw_aging_status(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_aging_status failed")
    __TM_MISMATCH_HANDLE(set_hw_aging_status(hndl))

    rc = cmp_sw_hw_tree_deq_status(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_tree_deq_status failed")
    __TM_MISMATCH_HANDLE(set_hw_tree_deq_status(hndl))

    rc = cmp_sw_hw_tree_dwrr_priority(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_tree_deq_status failed")
    __TM_MISMATCH_HANDLE(set_hw_tree_dwrr_priority(hndl))

    /* other registers */
    rc = cmp_sw_hw_gen_conf(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_gen_conf failed")
    __TM_MISMATCH_HANDLE(set_hw_gen_conf(hndl))

    /* Need to test this when scheduling is tested */
    for (i=0; i<TM_NUM_QUEUE_DROP_PROF; i++)
    {
        rc = cmp_sw_hw_queue_drop_profile(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_queue_drop_profile failed at idx=%d", i)
        __TM_MISMATCH_HANDLE(set_hw_queue_drop_profile(hndl, i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_A_NODE_DROP_PROF; i++)
    {
        rc = cmp_sw_hw_a_nodes_drop_profile(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_a_nodes_drop_profile failed at idx=%d", i)
        __TM_MISMATCH_HANDLE(set_hw_a_nodes_drop_profile(hndl, i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_B_NODE_DROP_PROF; i++)
    {
        rc = cmp_sw_hw_b_nodes_drop_profile(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_b_nodes_drop_profile failed at idx=%d", i)
        __TM_MISMATCH_HANDLE(set_hw_b_nodes_drop_profile(hndl, i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_C_NODE_DROP_PROF; i++)
    {
        for (j=0; j<TM_WRED_COS; j++)
        {
            rc = cmp_sw_hw_c_nodes_drop_profile(hndl, j, i);
            __TM_CATCHUP_ERR_ARG2_LOG(rc, "cmp_sw_hw_c_nodes_drop_profile failed at i=%d j=%d", i, j)
            __TM_MISMATCH_HANDLE(set_hw_c_nodes_drop_profile(hndl, j, i))
            __TM_BREAK_LOOP_ON_ERROR
        }
        __TM_BREAK_LOOP_ON_ERROR
    }

    rc = cmp_sw_hw_periodic_scheme(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_periodic_scheme failed")
    __TM_MISMATCH_HANDLE(set_hw_periodic_scheme(hndl))


    for (i=0; i<P_LEVEL; i++)
    {
        rc = cmp_sw_hw_shaping_status(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_shaping_status failed at idx = %d", i)
        __TM_MISMATCH_HANDLE(set_hw_shaping_status(hndl, i))
        __TM_BREAK_LOOP_ON_ERROR
    }

    /* This is not configured during TM init */
    /* TM SW DB holds default value which is not written to hardware */
    /* Expect mismatch if cpssTmSchedPortLvlDwrrBytesPerBurstLimitSet() not called */
#if 0
    rc = cmp_sw_hw_dwrr_limit(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_dwrr_limit failed")
#endif
    /* Download Eligable Prio Table */
    rc = cmp_sw_hw_elig_prio_func_tbl_q_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_elig_prio_func_tbl_q_level failed")
    __TM_MISMATCH_HANDLE(set_hw_elig_prio_func_tbl_q_level(hndl))

    rc = cmp_sw_hw_elig_prio_func_tbl_a_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_elig_prio_func_tbl_a_level failed")
    __TM_MISMATCH_HANDLE(set_hw_elig_prio_func_tbl_a_level(hndl))

    rc = cmp_sw_hw_elig_prio_func_tbl_b_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_elig_prio_func_tbl_b_level failed")
    __TM_MISMATCH_HANDLE(set_hw_elig_prio_func_tbl_b_level(hndl))

    rc = cmp_sw_hw_elig_prio_func_tbl_c_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_elig_prio_func_tbl_c_level failed")
    __TM_MISMATCH_HANDLE(set_hw_elig_prio_func_tbl_c_level(hndl))

    rc = cmp_sw_hw_elig_prio_func_tbl_p_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_elig_prio_func_tbl_p_level failed")
    __TM_MISMATCH_HANDLE(set_hw_elig_prio_func_tbl_p_level(hndl))

    /* Download Nodes */
    for (i=0; i < ctl->tm_total_ports; i++)
    {
        if (ctl->tm_port_array[i].mapping.nodeStatus == TM_NODE_USED)
        {
            rc = cmp_sw_hw_port(hndl, (uint8_t)i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_port failed at idx=%d", i)
            __TM_MISMATCH_HANDLE(set_hw_port(hndl, (uint8_t)i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_node_elig_prio_function(hndl, P_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_elig_prio_function failed at idx=%d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_elig_prio_function(hndl, P_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR
        }

    }

    for (i=0; i < ctl->tm_total_c_nodes; i++)
    {
        if (ctl->tm_c_node_array[i].mapping.nodeStatus == TM_NODE_USED)
        {
            rc = cmp_sw_hw_node_mapping(hndl, C_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_mapping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_mapping(hndl, C_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_c_node_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_c_node_params failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_c_node_params(hndl, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_node_elig_prio_function(hndl, C_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_elig_prio_function failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_elig_prio_function(hndl, C_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the c-node shaping params base on the shaping profile ptr */
            rc = cmp_sw_hw_node_shaping(hndl, C_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_shaping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_shaping(hndl, C_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR
        }
    }

    for (i=0; i < ctl->tm_total_b_nodes; i++)
    {
        if (ctl->tm_b_node_array[i].mapping.nodeStatus == TM_NODE_USED)
        {
            rc = cmp_sw_hw_node_mapping(hndl, B_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_mapping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_mapping(hndl, B_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_b_node_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_b_node_params failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_b_node_params(hndl, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_node_elig_prio_function(hndl, B_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_elig_prio_function failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_elig_prio_function(hndl, B_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the b-node shaping params base on the shaping profile ptr */
            rc = cmp_sw_hw_node_shaping(hndl, B_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_shaping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_shaping(hndl, B_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

        }
    }

    for (i=0; i < ctl->tm_total_a_nodes; i++)
    {
        if (ctl->tm_a_node_array[i].mapping.nodeStatus == TM_NODE_USED)
        {
            rc = cmp_sw_hw_node_mapping(hndl, A_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_mapping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_mapping(hndl, A_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_a_node_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_a_node_params failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_a_node_params(hndl, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_node_elig_prio_function(hndl, A_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_elig_prio_function failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_elig_prio_function(hndl, A_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the a-node shaping params base on the shaping profile ptr */
            rc = cmp_sw_hw_node_shaping(hndl, A_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_shaping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_shaping(hndl, A_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR
        }
    }
    for (i=0; i < ctl->tm_total_queues; i++)
    {
        if (ctl->tm_queue_array[i].mapping.nodeStatus == TM_NODE_USED)
        {
            rc = cmp_sw_hw_node_mapping(hndl, Q_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_mapping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_mapping(hndl, Q_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_queue_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_queue_params failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_queue_params(hndl, i))
            __TM_BREAK_LOOP_ON_ERROR

            rc = cmp_sw_hw_node_elig_prio_function(hndl, Q_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_elig_prio_function failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_elig_prio_function(hndl, Q_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the queue shaping params base on the shaping profile
             * ptr */
            rc = cmp_sw_hw_node_shaping(hndl, Q_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "cmp_sw_hw_node_shaping failed. idx = %d", i)
            __TM_MISMATCH_HANDLE(set_hw_node_shaping(hndl, Q_LEVEL, i))
            __TM_BREAK_LOOP_ON_ERROR

            #if 0
            if (ctl->tm_queue_array[i].installed == TM_ENABLE)
            {
                rc |= cmp_sw_hw_install_queue(hndl, i);
            }
            else
            {
                rc |= cmp_sw_hw_uninstall_queue(hndl, i);
            }
            __TM_CATCHUP_ERR_LOG(rc, "cmp_sw_hw_install/uninstall_queue failed")
            #endif

            __TM_BREAK_LOOP_ON_ERROR
        }
    }

out:

    ret = XEL_TO_CPSS_ERR_CODE(rc1);

    return ret;
}

/**
 */
static GT_STATUS prvCpssTmSyncSw(GT_U8 devNum)
{
    GT_STATUS   ret = GT_OK;
    uint32_t    i;
    uint8_t     j;
    int rc = 0;
    int rc1 = 0;

    GT_VOID_PTR hndl = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

    TM_CTL(ctl,hndl)

    rc = sync_sw_max_dp_mode(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_max_dp_mode failed")

    rc = sync_sw_drop_aqm_mode(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_drop_aqm_mode failed")

    for (i=0; i<TM_NUM_WRED_QUEUE_CURVES; i++)
    {
        rc = sync_sw_queues_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_queues_wred_curve failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_A_NODE_CURVES; i++)
    {
        rc = sync_sw_a_nodes_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_a_nodes_wred_curve failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_B_NODE_CURVES; i++)
    {
        rc = sync_sw_b_nodes_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_b_nodes_wred_curve failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_C_NODE_CURVES; i++)
    {
        for (j=0; j<TM_WRED_COS; j++)
        {
            rc = sync_sw_c_nodes_wred_curve(hndl, j, (uint8_t)i);
            __TM_CATCHUP_ERR_ARG2_LOG(rc, "sync_sw_c_nodes_wred_curve failed. i=%d j=%d", i, j)
            __TM_BREAK_LOOP_ON_ERROR
        }
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_WRED_PORT_CURVES; i++)
    {
        rc = sync_sw_ports_wred_curve(hndl, (uint8_t)i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_ports_wred_curve failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
        for (j=0; j<TM_WRED_COS; j++)
        {
            rc = sync_sw_ports_wred_curve_cos(hndl, j, (uint8_t)i);
            __TM_CATCHUP_ERR_ARG2_LOG(rc, "sync_sw_ports_wred_curve_cos failed. i=%d, j=%d", i, j)
            __TM_BREAK_LOOP_ON_ERROR
        }
        __TM_BREAK_LOOP_ON_ERROR
    }

    /* No need to set Port Drop porfiles, it's done inside
     * sync_sw_port later */

    rc = sync_sw_aging_status(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_aging_status failed")

    rc = sync_sw_tree_deq_status(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_tree_deq_status failed")

    rc = sync_sw_tree_dwrr_priority(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_tree_dwrr_priority failed")

    /* other registers */
    rc = sync_sw_gen_conf(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_gen_conf failed")

    for (i=0; i<TM_NUM_QUEUE_DROP_PROF; i++)
    {
        rc = sync_sw_queue_drop_profile(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_queue_drop_profile failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_A_NODE_DROP_PROF; i++)
    {
        rc = sync_sw_a_nodes_drop_profile(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_a_nodes_drop_profile failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_B_NODE_DROP_PROF; i++)
    {
        rc = sync_sw_b_nodes_drop_profile(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_b_nodes_drop_profile failed. idx = %d", i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    for (i=0; i<TM_NUM_C_NODE_DROP_PROF; i++)
    {
        for (j=0; j<TM_WRED_COS; j++)
        {
            rc = sync_sw_c_nodes_drop_profile(hndl, j, i);
            __TM_CATCHUP_ERR_ARG2_LOG(rc, "sync_sw_c_nodes_drop_profile failed. i=%d j=%d", i, j)
            __TM_BREAK_LOOP_ON_ERROR
        }
        __TM_BREAK_LOOP_ON_ERROR
    }

    rc = sync_sw_periodic_scheme(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_periodic_scheme failed")

    for (i=0; i<P_LEVEL; i++)
    {
        rc = sync_sw_shaping_status(hndl, i);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_shaping_status failed at idx = %d",i)
        __TM_BREAK_LOOP_ON_ERROR
    }

    rc = sync_sw_dwrr_limit(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_dwrr_limit failed")

    /* Download Eligable Prio Table */
    rc = sync_sw_elig_prio_func_tbl_q_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_elig_prio_func_tbl_q_level failed")

    rc = sync_sw_elig_prio_func_tbl_a_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_elig_prio_func_tbl_a_level failed")

    rc = sync_sw_elig_prio_func_tbl_b_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_elig_prio_func_tbl_b_level failed")

    rc = sync_sw_elig_prio_func_tbl_c_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_elig_prio_func_tbl_c_level failed")

    rc = sync_sw_elig_prio_func_tbl_p_level(hndl);
    __TM_CATCHUP_ERR_LOG(rc, "sync_sw_elig_prio_func_tbl_p_level failed")

    /* Download Nodes */
    for (i=0; i < ctl->tm_total_ports; i++)
    {
        if (ctl->tm_port_array[i].mapping.nodeStatus != TM_NODE_FREE)
        {
            rc = sync_sw_port(hndl, (uint8_t)i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_port failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_node_elig_prio_function(hndl, P_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_elig_prio_function failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR
        }
    }

    for (i=0; i < ctl->tm_total_c_nodes; i++)
    {
        if (ctl->tm_c_node_array[i].mapping.nodeStatus != TM_NODE_FREE)
        {
            rc = sync_sw_node_mapping(hndl, C_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_mapping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_c_node_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_c_node_params failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_node_elig_prio_function(hndl, C_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_elig_prio_function failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the c-node shaping params base on the shaping profile ptr */
            rc = sync_sw_node_shaping(hndl, C_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_shaping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR
        }
    }

    for (i=0; i < ctl->tm_total_b_nodes; i++)
    {
        if (ctl->tm_b_node_array[i].mapping.nodeStatus != TM_NODE_FREE)
        {
            rc = sync_sw_node_mapping(hndl, B_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_mapping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_b_node_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_b_node_params failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_node_elig_prio_function(hndl, B_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_elig_prio_function failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the b-node shaping params base on the shaping profile ptr */
            rc = sync_sw_node_shaping(hndl, B_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_shaping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR
        }
    }

    for (i=0; i < ctl->tm_total_a_nodes; i++)
    {
        if (ctl->tm_a_node_array[i].mapping.nodeStatus != TM_NODE_FREE)
        {
            rc = sync_sw_node_mapping(hndl, A_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_mapping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_a_node_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_a_node_params failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_node_elig_prio_function(hndl, A_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_elig_prio_function failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the a-node shaping params base on the shaping profile ptr */
            rc = sync_sw_node_shaping(hndl, A_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_shaping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR
        }
    }
    for (i=0; i < ctl->tm_total_queues; i++)
    {
        if (ctl->tm_queue_array[i].mapping.nodeStatus != TM_NODE_FREE)
        {
            rc = sync_sw_node_mapping(hndl, Q_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_mapping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_queue_params(hndl, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_queue_params failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            rc = sync_sw_node_elig_prio_function(hndl, Q_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_elig_prio_function failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            /* sets the queue shaping params base on the shaping profile
             * ptr */
            rc = sync_sw_node_shaping(hndl, Q_LEVEL, i);
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "sync_sw_node_shaping failed. idx = %d", i)
            __TM_BREAK_LOOP_ON_ERROR

            #if 0
            if (ctl->tm_queue_array[i].installed == TM_ENABLE)  rc = sync_sw_install_queue(hndl, i);
            else                                                rc = sync_sw_uninstall_queue(hndl, i);
            __TM_CATCHUP_ERR_LOG(rc, "sync_sw_install/uninstall_queue failed")
            #endif
            __TM_BREAK_LOOP_ON_ERROR
        }
    }

  out:
    ret = XEL_TO_CPSS_ERR_CODE(rc1);

    return ret;
}

/**
* @internal  prvCpssTmCatchUp  function
* @endinternal
*
* @brief   Perform catch up of TM SW database for specific
*          device
*
* @note   APPLICABLE DEVICES:     Caelum;
* @note  NOT APPLICABLE DEVICES:   Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                  AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               PP's device number.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssTmCatchUp
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc=GT_OK;

    rc = prvCpssTmSyncSw(devNum);

    return rc;
}


/**
* @internal  prvCpssTmSyncSwHwForHa  function
* @endinternal
*
* @brief   Loop over all devices and perform catch up of TM
*          configuration for all devices
*
* @note   APPLICABLE DEVICES:     Caelum
* @note  NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    -  on success
* @retval GT_BAD_PTR               -  on NULL ptr
* @retval GT_HW_ERROR              -  if write failed
*/
GT_STATUS prvCpssTmSyncSwHwForHa
(
    GT_VOID
)
{
    GT_U8 devNum;
    GT_STATUS rc;

    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        if (PRV_CPSS_PP_CONFIG_ARR_MAC[devNum] == NULL)
        {
            CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
            continue;
        }

        if ((PRV_CPSS_PP_MAC(devNum)->devFamily ==  CPSS_PP_FAMILY_DXCH_BOBCAT2_E) &&
            (PRV_CPSS_PP_MAC(devNum)->devSubFamily == CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E) &&
            (PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle != NULL))
        {
            rc = prvCpssTmSyncCatchupValidityCheck(devNum);
            if (rc != GT_OK)
            {
                CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                return rc;
            }
        }

        CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    return GT_OK;
}

/**
* @internal  prvCpssTmSyncCatchupValidityCheck  function
* @endinternal
*
* @brief   Check synchronization of hardware and software database
*          of TM confguration.
*
* @note   APPLICABLE DEVICES:     Caelum
* @note  NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    -  on success
*/
GT_STATUS prvCpssTmSyncCatchupValidityCheck
(
    GT_U32  devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = prvCpssTmCmpSwHw(devNum, GT_FALSE /*updateHw*/);

    return rc;
}

/**
* @internal  prvCpssTmSyncMismatchHandle  function
* @endinternal
*
* @brief   Check synchronization between hardware and software database
*          of TM confguration and update Hw accordingly.
*
* @note   APPLICABLE DEVICES:     Caelum
* @note  NOT APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman; xCat3;
*                                AC5; Lion2; Bobcat2; Aldrin; AC3X; Bobcat3;
*                                Aldrin2; AC5P; AC5X; Harrier; Ironman.
*
*
* @retval GT_OK                    -  on success
*/
GT_STATUS prvCpssTmSyncMismatchHandle
(
    GT_U32  devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = prvCpssTmCmpSwHw(devNum, GT_TRUE /*updateHw*/);

    return rc;
}

