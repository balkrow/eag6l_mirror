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
* @file prvCpssTmDbgUtils.c
*
* @brief CPSS BobK TM  debug utilities.
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
#include <core/tm_os_interface.h>
#include <core/rm_internal_types.h>
#include <core/rm_ctl.h>
#include <systemRecovery/prvCpssTmCatchup.h>
#include <systemRecovery/sync_sw_registers.h>
#include <systemRecovery/cmp_sw_hw_registers.h>

#define TM_HA_RM_PREFIX_SIZE_CNS  (sizeof(uint32_t) * 3)
#define TM_HA_NODE_SHAPING_EXCLUDE_SIZE_CNS (sizeof(uint32_t)*2 + sizeof(uint16_t)*2)
#define TM_HA_SHAPING_PROFILE_EXCLUDE_SIZE_CNS (sizeof(uint32_t)*2)

#define TM_HA_RM_COPY(_ctl_dst, _ctl_src, prof, size) \
do {                                                                                                     \
    tm_memcpy(&(_ctl_dst->rm_profiles[prof]), &(_ctl_src->rm_profiles[prof]), TM_HA_RM_PREFIX_SIZE_CNS); \
    if((_ctl_src->rm_profiles[prof].used) && (_ctl_dst->rm_profiles[prof].used))                         \
    {                                                                                                    \
        tm_memcpy(_ctl_dst->rm_profiles[prof].used, _ctl_src->rm_profiles[prof].used, (size));           \
    }                                                                                                    \
} while (0)


#define TM_HA_RM_CMP(_ctl_dst, _ctl_src, prof, size) \
do {                                                                                                          \
    rc = tm_memcmp(&(_ctl_dst->rm_profiles[prof]), &(_ctl_src->rm_profiles[prof]), TM_HA_RM_PREFIX_SIZE_CNS); \
    __TM_CATCHUP_ERR_LOG(rc, "RM profile prefix didn't match : " #prof);                                      \
    if((_ctl_src->rm_profiles[prof].used) && (_ctl_dst->rm_profiles[prof].used))                              \
    {                                                                                                         \
        rc = tm_memcmp(_ctl_dst->rm_profiles[prof].used, _ctl_src->rm_profiles[prof].used, (size));           \
        __TM_CATCHUP_ERR_LOG(rc, "RM profile used didn't match : " #prof);                                    \
    }                                                                                                         \
} while (0)

#define TM_HA_MEM_CMP(src1, src2, size) \
do { \
    if((src1) && (src2)) \
    { \
        rc = tm_memcmp((src1), (src2), (size));\
        __TM_CATCHUP_ERR_LOG(rc, "Mem cmp failed : " #src1);  \
    } \
    else \
    {    \
        __TM_CATCHUP_ERR_LOG(GT_FAIL, "Mem cmp failed : NULL ponter\n"); \
    }    \
} while (0)

#define TM_HA_MEM_CMP_IN_LOOP(src1, src2, size, max_count, exclude_size)          \
do {                                                                \
    if((src1) && (src2))                                            \
    {                                                               \
        for(i=0; i<max_count; i++)                                  \
        {                                                           \
            rc = tm_memcmp(&(src1[i]), &(src2[i]), (size) - (exclude_size));         \
            __TM_CATCHUP_ERR_ARG1_LOG(rc, "Mem compare failed : " #src1 " i=%d ", i);    \
            __TM_BREAK_LOOP_ON_ERROR;                               \
        }                                                           \
    } \
    else \
    {    \
        __TM_CATCHUP_ERR_LOG(GT_FAIL, "Mem cmp failed : NULL ponter\n"); \
    }    \
} while (0)

#define TM_HA_MALLOC_AND_MEMCPY(arr,size)      \
do {                                            \
    ctl->arr = tm_malloc((size));               \
    if (!ctl->arr) {                            \
        __TM_CATCHUP_ERR_LOG(GT_OUT_OF_CPU_MEM, "Mem create failed : " #arr);  \
    }                                           \
    tm_memcpy(ctl->arr, ctl_orig->arr, (size)); \
} while (0)

GT_STATUS prvCpssTmDbgSwDbStoreRmCopy(uint16_t total_ports,
                    uint16_t total_c_nodes,
                    uint16_t total_b_nodes,
                    uint16_t total_a_nodes,
                    uint32_t total_queues, void *ctl_src, void *ctl_dst)
{
    struct rmctl *   ctl = (struct rmctl *) ctl_dst;
    struct rmctl *   ctl_orig = (struct rmctl *)ctl_src;

    if ((ctl) && (ctl_orig))
    {
        /* Fill in ctl structure */
        ctl->magic = ctl_orig->magic;

        TM_HA_RM_COPY(ctl, ctl_orig, RM_SHAPING_PRF,  total_queues + total_a_nodes + total_b_nodes + total_c_nodes);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_Q_CURVE,  TM_NUM_WRED_QUEUE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_A_CURVE,  TM_NUM_WRED_A_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_B_CURVE,  TM_NUM_WRED_B_NODE_CURVES);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_0,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_1,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_2,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_3,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_4,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_5,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_6,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_C_CURVE_COS_7,  TM_NUM_WRED_C_NODE_CURVES);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE,  TM_NUM_WRED_PORT_CURVES);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_0,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_1,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_2,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_3,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_4,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_5,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_6,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_WRED_P_CURVE_COS_7,  TM_NUM_WRED_PORT_CURVES);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_Q_DROP_PRF,  TM_NUM_QUEUE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_A_DROP_PRF,  TM_NUM_A_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_B_DROP_PRF,  TM_NUM_B_NODE_DROP_PROF);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_0,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_1,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_2,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_3,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_4,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_5,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_6,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_C_DROP_PRF_COS_7,  TM_NUM_C_NODE_DROP_PROF);

        /* for ports  drop profiles are per port -  drop profile count is equial to ports number */
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF,  total_ports);

        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_0,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_1,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_2,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_3,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_4,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_5,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_6,  total_ports);
        TM_HA_RM_COPY(ctl, ctl_orig, RM_P_DROP_PRF_COS_7,  total_ports);

    }
    return GT_OK;
}

GT_VOID prvCpssTmDbgSwDbStoreCopyTmCtl
(
    struct tm_ctl *ctl_src,
    struct tm_ctl *ctl_dst
)
{
    ctl_dst->magic            = ctl_src->magic;
    ctl_dst->tm_total_queues  = ctl_src->tm_total_queues;
    ctl_dst->tm_total_a_nodes = ctl_src->tm_total_a_nodes;
    ctl_dst->tm_total_b_nodes = ctl_src->tm_total_b_nodes;
    ctl_dst->tm_total_c_nodes = ctl_src->tm_total_c_nodes;
    ctl_dst->tm_total_ports   = ctl_src->tm_total_ports;
}

/**
 */
GT_STATUS prvCpssTmDbgSwDbStoreAllocateAndCopy(void *ctl_src, void **ctl_dst)
{
    struct tm_ctl *ctl = NULL;
    struct tm_ctl *ctl_orig = (struct tm_ctl *)ctl_src;
    uint32_t total_shap_profiles;

    int rc = 0;
    unsigned int i;
    int rc1 = 0;

    (void) i;
    (void) total_shap_profiles;
    (void) rc1;

    /* Allocate handle */
    ctl = (struct tm_ctl *)tm_malloc(sizeof(struct tm_ctl) * 1);
    if (!ctl)
    {
        rc =  -GT_OUT_OF_CPU_MEM;
        goto out;
    }
    tm_memset(ctl, 0, sizeof(*ctl));

    prvCpssTmDbgSwDbStoreCopyTmCtl(ctl_orig, ctl);
    /*tm_memcpy(ctl, ctl_orig, sizeof(*ctl));*/

    total_shap_profiles = ctl->tm_total_queues + ctl->tm_total_a_nodes + ctl->tm_total_b_nodes + ctl->tm_total_c_nodes;
    ctl->rm = rm_open(ctl->tm_total_ports,ctl->tm_total_c_nodes, ctl->tm_total_b_nodes, ctl->tm_total_a_nodes, ctl->tm_total_queues);
    if (!ctl->rm)
    {
        rc = -EINVAL;
        goto out;
    }

    prvCpssTmDbgSwDbStoreRmCopy(ctl_orig->tm_total_ports,   ctl_orig->tm_total_c_nodes,
                                 ctl_orig->tm_total_b_nodes, ctl_orig->tm_total_a_nodes,
                                 ctl_orig->tm_total_queues,  ctl_orig->rm, ctl->rm);

    /* allocate global arrays */
    TM_HA_MALLOC_AND_MEMCPY(tm_port_array,   sizeof(struct tm_port) * ctl->tm_total_ports);
    TM_HA_MALLOC_AND_MEMCPY(tm_c_node_array, sizeof(struct tm_c_node) * ctl->tm_total_c_nodes);
    TM_HA_MALLOC_AND_MEMCPY(tm_b_node_array, sizeof(struct tm_b_node) * ctl->tm_total_b_nodes);
    TM_HA_MALLOC_AND_MEMCPY(tm_a_node_array, sizeof(struct tm_b_node) * ctl->tm_total_a_nodes);
    TM_HA_MALLOC_AND_MEMCPY(tm_queue_array,  sizeof(struct tm_queue) * ctl->tm_total_queues);
    TM_HA_MALLOC_AND_MEMCPY(tm_shaping_profiles, sizeof(struct tm_shaping_profile) * total_shap_profiles);

    TM_HA_MALLOC_AND_MEMCPY(tm_q_lvl_drop_profiles, sizeof(struct tm_drop_profile) * TM_NUM_QUEUE_DROP_PROF);
    TM_HA_MALLOC_AND_MEMCPY(tm_a_lvl_drop_profiles, sizeof(struct tm_drop_profile) * TM_NUM_A_NODE_DROP_PROF);
    TM_HA_MALLOC_AND_MEMCPY(tm_b_lvl_drop_profiles, sizeof(struct tm_drop_profile) * TM_NUM_B_NODE_DROP_PROF);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MALLOC_AND_MEMCPY(tm_c_lvl_drop_profiles[i], sizeof(struct tm_drop_profile) * TM_NUM_C_NODE_DROP_PROF);
    }
    /* for ports  drop_profile & drop_profile_cos are per port  so drop profiles array sizes are equial to ports count */
    TM_HA_MALLOC_AND_MEMCPY(tm_p_lvl_drop_profiles, sizeof(struct tm_drop_profile) * ctl->tm_total_ports);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MALLOC_AND_MEMCPY(tm_p_lvl_drop_profiles_cos[i], sizeof(struct tm_drop_profile) * ctl->tm_total_ports);
    }

    TM_HA_MALLOC_AND_MEMCPY(tm_wred_q_lvl_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_QUEUE_CURVES);
    TM_HA_MALLOC_AND_MEMCPY(tm_wred_a_lvl_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_A_NODE_CURVES);
    TM_HA_MALLOC_AND_MEMCPY(tm_wred_b_lvl_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_B_NODE_CURVES);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MALLOC_AND_MEMCPY(tm_wred_c_lvl_curves[i], sizeof(struct tm_wred_curve) * TM_NUM_WRED_C_NODE_CURVES);
    }
    TM_HA_MALLOC_AND_MEMCPY(tm_wred_ports_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_PORT_CURVES);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MALLOC_AND_MEMCPY(tm_wred_ports_curves_cos[i], sizeof(struct tm_wred_curve) * TM_NUM_WRED_PORT_CURVES);
    }

    TM_HA_MALLOC_AND_MEMCPY(tm_q_lvl_drop_prof_ptr, sizeof(uint16_t) * ctl->tm_total_queues);
    TM_HA_MALLOC_AND_MEMCPY(tm_a_lvl_drop_prof_ptr, sizeof(uint16_t) * ctl->tm_total_a_nodes);
    TM_HA_MALLOC_AND_MEMCPY(tm_b_lvl_drop_prof_ptr, sizeof(uint16_t) * ctl->tm_total_b_nodes);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MALLOC_AND_MEMCPY(tm_c_lvl_drop_prof_ptr[i], sizeof(uint8_t) * ctl->tm_total_c_nodes);
    }

    TM_HA_MALLOC_AND_MEMCPY(tm_p_lvl_drop_prof_ptr, sizeof(uint8_t) * ctl->tm_total_ports);

    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MALLOC_AND_MEMCPY(tm_p_lvl_drop_prof_ptr_cos[i], sizeof(uint8_t) * ctl->tm_total_ports);
    }
    TM_HA_MALLOC_AND_MEMCPY(tm_port_sms_attr_pbase, sizeof(struct port_sms_attr_pbase) * ctl->tm_total_ports);
    TM_HA_MALLOC_AND_MEMCPY(tm_port_sms_attr_qmap_pars, sizeof(struct port_sms_attr_qmap_parsing) * ctl->tm_total_ports);

out:
    *ctl_dst = (void *)ctl;
    (void) ctl_dst;
    return rc;
}

GT_STATUS prvCpssTmDbgSwDbStore
(
    IN  GT_U8       devNum,
    OUT GT_VOID_PTR *swdbPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_VOID_PTR hndl = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

    rc = prvCpssTmDbgSwDbStoreAllocateAndCopy(hndl, swdbPtr);
    return rc;
}

GT_STATUS prvCpssTmDbgSwDbCmpRmCmp(uint16_t total_ports,
                    uint16_t total_c_nodes,
                    uint16_t total_b_nodes,
                    uint16_t total_a_nodes,
                    uint32_t total_queues, void *ctl_src, void **ctl_dst)
{
    GT_STATUS rc1 = GT_OK;
    int rc = 0;
    struct rmctl *   ctl = (struct rmctl *) *ctl_dst;
    struct rmctl *   ctl_orig = (struct rmctl *)ctl_src;

    if ((ctl) && (ctl_orig))
    {
        /* Fill in ctl structure */
        if(ctl->magic != ctl_orig->magic)
        {
            __TM_CATCHUP_ERR_LOG(GT_FAIL, "Magic number didn't match\n");
        }

        TM_HA_RM_CMP(ctl, ctl_orig, RM_SHAPING_PRF, total_queues + total_a_nodes + total_b_nodes + total_c_nodes);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_Q_CURVE,  TM_NUM_WRED_QUEUE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_A_CURVE,  TM_NUM_WRED_A_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_B_CURVE,  TM_NUM_WRED_B_NODE_CURVES);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_0,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_1,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_2,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_3,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_4,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_5,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_6,  TM_NUM_WRED_C_NODE_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_C_CURVE_COS_7,  TM_NUM_WRED_C_NODE_CURVES);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE,  TM_NUM_WRED_PORT_CURVES);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_0,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_1,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_2,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_3,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_4,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_5,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_6,  TM_NUM_WRED_PORT_CURVES);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_WRED_P_CURVE_COS_7,  TM_NUM_WRED_PORT_CURVES);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_Q_DROP_PRF,  TM_NUM_QUEUE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_A_DROP_PRF,  TM_NUM_A_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_B_DROP_PRF,  TM_NUM_B_NODE_DROP_PROF);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_0,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_1,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_2,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_3,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_4,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_5,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_6,  TM_NUM_C_NODE_DROP_PROF);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_C_DROP_PRF_COS_7,  TM_NUM_C_NODE_DROP_PROF);

        /* for ports  drop profiles are per port -  drop profile count is equial to ports number */
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF,  total_ports);

        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_0,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_1,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_2,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_3,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_4,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_5,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_6,  total_ports);
        TM_HA_RM_CMP(ctl, ctl_orig, RM_P_DROP_PRF_COS_7,  total_ports);
    }

out:
    return rc1;
}

GT_STATUS prvCpssTmDbgSwDbCmpWithOriginal
(
    void *ctl_src,
    void **ctl_dst
)
{
    GT_STATUS rc1 = GT_OK;
    int rc = 0;
    unsigned int i;
    struct tm_ctl *ctl = (struct tm_ctl *)*ctl_dst;
    struct tm_ctl *ctl_orig = (struct tm_ctl *)ctl_src;
    uint32_t total_shap_profiles;
    struct tm_shaping_profile shaping_prof;
    struct tm_shaping_profile shaping_prof_orig;

    total_shap_profiles = ctl->tm_total_queues + ctl->tm_total_a_nodes + ctl->tm_total_b_nodes + ctl->tm_total_c_nodes;

    prvCpssTmDbgSwDbCmpRmCmp(ctl_orig->tm_total_ports,   ctl_orig->tm_total_c_nodes,
                                 ctl_orig->tm_total_b_nodes, ctl_orig->tm_total_a_nodes,
                                 ctl_orig->tm_total_queues,  ctl_orig->rm, &ctl->rm);

    TM_HA_MEM_CMP(ctl->tm_port_array, ctl_orig->tm_port_array, sizeof(struct tm_port) * ctl->tm_total_ports);
    TM_HA_MEM_CMP_IN_LOOP(ctl->tm_c_node_array, ctl_orig->tm_c_node_array, sizeof(struct tm_c_node), ctl->tm_total_c_nodes, TM_HA_NODE_SHAPING_EXCLUDE_SIZE_CNS);
    TM_HA_MEM_CMP_IN_LOOP(ctl->tm_b_node_array, ctl_orig->tm_b_node_array, sizeof(struct tm_b_node), ctl->tm_total_b_nodes, TM_HA_NODE_SHAPING_EXCLUDE_SIZE_CNS);
    TM_HA_MEM_CMP_IN_LOOP(ctl->tm_a_node_array, ctl_orig->tm_a_node_array, sizeof(struct tm_a_node), ctl->tm_total_a_nodes, TM_HA_NODE_SHAPING_EXCLUDE_SIZE_CNS);
    TM_HA_MEM_CMP_IN_LOOP(ctl->tm_queue_array,  ctl_orig->tm_queue_array,  sizeof(struct tm_queue),  ctl->tm_total_queues,  TM_HA_NODE_SHAPING_EXCLUDE_SIZE_CNS);
    for (i=0; i<total_shap_profiles; i++)
    {
        tm_memcpy(&shaping_prof, &ctl->tm_shaping_profiles[i], sizeof(shaping_prof));
        tm_memcpy(&shaping_prof_orig, &ctl_orig->tm_shaping_profiles[i], sizeof(shaping_prof_orig));
        shaping_prof.min_token     &= 0x7ff;
        shaping_prof.max_token     &= 0x7ff;
        shaping_prof.min_token_res &= 0x1;
        shaping_prof.max_token_res &= 0x1;
        shaping_prof_orig.min_token     &= 0x7ff;
        shaping_prof_orig.max_token     &= 0x7ff;
        shaping_prof_orig.min_token_res &= 0x1;
        shaping_prof_orig.max_token_res &= 0x1;
        rc = tm_memcmp(&shaping_prof, &shaping_prof_orig, sizeof(shaping_prof) - TM_HA_SHAPING_PROFILE_EXCLUDE_SIZE_CNS);
        __TM_CATCHUP_ERR_ARG1_LOG(rc, "Mem cmp failed tm_shaping_profiles at idx = %d", i);  \
    }
    /*TM_HA_MEM_CMP_IN_LOOP(ctl->tm_shaping_profiles, ctl_orig->tm_shaping_profiles, sizeof(struct tm_shaping_profile), total_shap_profiles, TM_HA_SHAPING_PROFILE_EXCLUDE_SIZE_CNS); */
    TM_HA_MEM_CMP(ctl->tm_q_lvl_drop_profiles, ctl_orig->tm_q_lvl_drop_profiles, sizeof(struct tm_drop_profile) * TM_NUM_QUEUE_DROP_PROF);
    TM_HA_MEM_CMP(ctl->tm_a_lvl_drop_profiles, ctl_orig->tm_a_lvl_drop_profiles, sizeof(struct tm_drop_profile) * TM_NUM_A_NODE_DROP_PROF);
    TM_HA_MEM_CMP(ctl->tm_b_lvl_drop_profiles, ctl_orig->tm_b_lvl_drop_profiles, sizeof(struct tm_drop_profile) * TM_NUM_B_NODE_DROP_PROF);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MEM_CMP(ctl->tm_c_lvl_drop_profiles[i], ctl_orig->tm_c_lvl_drop_profiles[i], sizeof(struct tm_drop_profile) * TM_NUM_C_NODE_DROP_PROF);
    }
    TM_HA_MEM_CMP(ctl->tm_p_lvl_drop_profiles, ctl_orig->tm_p_lvl_drop_profiles, sizeof(struct tm_drop_profile) * ctl->tm_total_ports);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MEM_CMP(ctl->tm_p_lvl_drop_profiles_cos[i], ctl_orig->tm_p_lvl_drop_profiles_cos[i], sizeof(struct tm_drop_profile) * ctl->tm_total_ports);
    }
    TM_HA_MEM_CMP(ctl->tm_wred_q_lvl_curves, ctl_orig->tm_wred_q_lvl_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_QUEUE_CURVES);
    TM_HA_MEM_CMP(ctl->tm_wred_a_lvl_curves, ctl_orig->tm_wred_a_lvl_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_A_NODE_CURVES);
    TM_HA_MEM_CMP(ctl->tm_wred_b_lvl_curves, ctl_orig->tm_wred_b_lvl_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_B_NODE_CURVES);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MEM_CMP(ctl->tm_wred_c_lvl_curves[i], ctl_orig->tm_wred_c_lvl_curves[i], sizeof(struct tm_wred_curve) * TM_NUM_WRED_C_NODE_CURVES);
    }
    TM_HA_MEM_CMP(ctl->tm_wred_ports_curves, ctl_orig->tm_wred_ports_curves, sizeof(struct tm_wred_curve) * TM_NUM_WRED_PORT_CURVES);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MEM_CMP(ctl->tm_wred_ports_curves_cos[i], ctl_orig->tm_wred_ports_curves_cos[i], sizeof(struct tm_wred_curve) * TM_NUM_WRED_PORT_CURVES);
    }
    TM_HA_MEM_CMP(ctl->tm_q_lvl_drop_prof_ptr, ctl_orig->tm_q_lvl_drop_prof_ptr, sizeof(uint16_t) * ctl->tm_total_queues);
    TM_HA_MEM_CMP(ctl->tm_a_lvl_drop_prof_ptr, ctl_orig->tm_a_lvl_drop_prof_ptr, sizeof(uint16_t) * ctl->tm_total_a_nodes);
    TM_HA_MEM_CMP(ctl->tm_b_lvl_drop_prof_ptr, ctl_orig->tm_b_lvl_drop_prof_ptr, sizeof(uint16_t) * ctl->tm_total_b_nodes);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MEM_CMP(ctl->tm_c_lvl_drop_prof_ptr[i], ctl_orig->tm_c_lvl_drop_prof_ptr[i], sizeof(uint8_t) * ctl->tm_total_c_nodes);
    }
    TM_HA_MEM_CMP(ctl->tm_p_lvl_drop_prof_ptr, ctl_orig->tm_p_lvl_drop_prof_ptr, sizeof(uint8_t) * ctl->tm_total_ports);
    for (i=0; i<TM_WRED_COS; i++)
    {
        TM_HA_MEM_CMP(ctl->tm_p_lvl_drop_prof_ptr_cos[i], ctl_orig->tm_p_lvl_drop_prof_ptr_cos[i], sizeof(uint8_t) * ctl->tm_total_ports);
    }
    TM_HA_MEM_CMP(ctl->tm_port_sms_attr_pbase, ctl_orig->tm_port_sms_attr_pbase, sizeof(struct port_sms_attr_pbase) * ctl->tm_total_ports);
    TM_HA_MEM_CMP(ctl->tm_port_sms_attr_qmap_pars, ctl_orig->tm_port_sms_attr_qmap_pars, sizeof(struct port_sms_attr_qmap_parsing) * ctl->tm_total_ports);

out:
    return rc1;
}

GT_STATUS prvCpssTmDbgSwDbCmp
(
    IN  GT_U8       devNum,
    IN  GT_VOID_PTR *swdbPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_VOID_PTR hndl = PRV_CPSS_PP_MAC(devNum)->tmInfo.tmHandle;

    rc = prvCpssTmDbgSwDbCmpWithOriginal(hndl, swdbPtr);

    return rc;
}

GT_STATUS prvCpssTmDbgSwDbFree
(
    IN  GT_U8       devNum,
    IN  GT_VOID_PTR *swdbPtr
)
{
    GT_STATUS   rc = GT_OK;
    int ret = 0;

    ret = tm_lib_close_sw(*swdbPtr);
    rc = XEL_TO_CPSS_ERR_CODE(ret);

    return rc;
}

