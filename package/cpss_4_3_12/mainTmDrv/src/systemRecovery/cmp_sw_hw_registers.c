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
* @file cmp_sw_hw_registers.c
*
* @brief APIs for Restoring Software DB by reading from hardware
*
* @version   1
********************************************************************************
*/

#include "tm_core_types.h"
#include "set_hw_registers_imp.h"

#include "set_hw_registers.h"
#include "tm_os_interface.h"
#include "rm_status.h"
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <systemRecovery/cmp_sw_hw_registers.h>

#define ENOMATCH 1

#define TM_HA_CMP_DROP_PROFILE_SIZE_CNS  28
#define TM_HA_CMP_SHAPING_PROFILE_SIZE_CNS  16

/**
 */
int cmp_sw_hw_max_dp_mode(tm_handle hndl)
{
    int rc = 0;
    int i;
    uint8_t port = 0;
    uint8_t c_lvl = 0;
    uint8_t b_lvl = 0;
    uint8_t a_lvl = 0;
    uint8_t queue = 0;
    int cmp_res;

    TM_REGISTER_VAR(TM_Drop_WREDMaxProbModePerColor)

    TM_CTL(ctl, hndl)

    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.WREDMaxProbModePerColor,TM_Drop_WREDMaxProbModePerColor)
    if (rc) goto out;

    TM_REGISTER_GET(TM_Drop_WREDMaxProbModePerColor,Port, port ,(uint8_t))
    TM_REGISTER_GET(TM_Drop_WREDMaxProbModePerColor,Clvl, c_lvl,(uint8_t))
    TM_REGISTER_GET(TM_Drop_WREDMaxProbModePerColor,Blvl, b_lvl,(uint8_t))
    TM_REGISTER_GET(TM_Drop_WREDMaxProbModePerColor,Alvl, a_lvl,(uint8_t))
    TM_REGISTER_GET(TM_Drop_WREDMaxProbModePerColor,Queue,queue,(uint8_t))

    cmp_res = 0;

    /* assign register fields */
    for (i=0; i<3; i++)
    {
        cmp_res |= (ctl->dp_unit.local[P_LEVEL].max_p_mode[i] != ((port  >> (i*2)) & 0x3));
        cmp_res |= (ctl->dp_unit.local[C_LEVEL].max_p_mode[i] != ((c_lvl >> (i*2)) & 0x3));
        cmp_res |= (ctl->dp_unit.local[B_LEVEL].max_p_mode[i] != ((b_lvl >> (i*2)) & 0x3));
        cmp_res |= (ctl->dp_unit.local[A_LEVEL].max_p_mode[i] != ((a_lvl >> (i*2)) & 0x3));
        cmp_res |= (ctl->dp_unit.local[Q_LEVEL].max_p_mode[i] != ((queue >> (i*2)) & 0x3));

        if (cmp_res) return ENOMATCH;

    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_drop_aqm_mode(tm_handle hndl)
{
    int     rc  =  -EFAULT;
    uint8_t val = 0;
    enum tm_dp_source tmp_dp_src = 0;

    TM_REGISTER_VAR(TM_Drop_WREDDropProbMode)
    TM_REGISTER_VAR(TM_Drop_DPSource)

    TM_CTL(ctl, hndl)

    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.WREDDropProbMode, TM_Drop_WREDDropProbMode)
    if (rc) goto out;

    /* assign register fields */
    TM_REGISTER_GET(TM_Drop_WREDDropProbMode, Port , val, (uint8_t))
    if (val == 1)
    {
        if(ctl->dp_unit.local[P_LEVEL].color_num != TM_3_COLORS)
        {
            return ENOMATCH;
        }
    }

    TM_REGISTER_GET(TM_Drop_WREDDropProbMode, Clvl , val, (uint8_t))
    if (val == 1)
    {
        if(ctl->dp_unit.local[C_LEVEL].color_num != TM_3_COLORS)
        {
            return ENOMATCH;
        }
    }

    TM_REGISTER_GET(TM_Drop_WREDDropProbMode, Blvl , val, (uint8_t))
    if (val == 1)
    {
        if(ctl->dp_unit.local[B_LEVEL].color_num != TM_3_COLORS)
        {
            return ENOMATCH;
        }
    }

    TM_REGISTER_GET(TM_Drop_WREDDropProbMode, Alvl , val, (uint8_t))
    if (val == 1)
    {
        if(ctl->dp_unit.local[A_LEVEL].color_num != TM_3_COLORS)
        {
            return ENOMATCH;
        }
    }

    TM_REGISTER_GET(TM_Drop_WREDDropProbMode, Queue, val, (uint8_t))
    if (val == 1)
    {
        if(ctl->dp_unit.local[Q_LEVEL].color_num != TM_3_COLORS)
        {
            return ENOMATCH;
        }
    }

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.DPSource, TM_Drop_DPSource)
    if (rc) goto out;

    /* assign register fields */
    TM_REGISTER_GET(TM_Drop_DPSource , PortSrc , val, (uint8_t));
    if (ctl->dp_unit.local[P_LEVEL].dp_src[0] != ((enum tm_dp_source)(val & 0x1)))
    {
        return ENOMATCH;
    }

    if (ctl->dp_unit.local[P_LEVEL].dp_src[1] != ((enum tm_dp_source)((val >> 1) & 0x1)))
    {
        return ENOMATCH;
    }

    if (ctl->dp_unit.local[P_LEVEL].dp_src[2] != ((enum tm_dp_source)((val >> 2) & 0x1)))
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Drop_DPSource , ClvlSrc , tmp_dp_src, (enum tm_dp_source));
    if (ctl->dp_unit.local[C_LEVEL].dp_src[0] != tmp_dp_src)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Drop_DPSource, BlvlSrc, tmp_dp_src, (enum tm_dp_source));
    if (ctl->dp_unit.local[B_LEVEL].dp_src[0] != tmp_dp_src)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Drop_DPSource , AlvlSrc , tmp_dp_src, (enum tm_dp_source));
    if (ctl->dp_unit.local[A_LEVEL].dp_src[0] != tmp_dp_src)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Drop_DPSource , QueueSrc , tmp_dp_src, (enum tm_dp_source));
    if (ctl->dp_unit.local[Q_LEVEL].dp_src[0] != tmp_dp_src)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_queues_wred_curve(tm_handle hndl,uint8_t curve_ind)
{
    int rc = -ERANGE;
    uint8_t *prob_array;
    uint8_t tmp_prob_array[32];
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (curve_ind < TM_NUM_WRED_QUEUE_CURVES)
    {
        tm_memset(tmp_prob_array, 0, sizeof(tmp_prob_array));

        prob_array = (uint8_t *)(ctl->tm_wred_q_lvl_curves[curve_ind].prob);
        rc = tm_get_hw_wred_curve(hndl, Q_LEVEL, 0, curve_ind, tmp_prob_array);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)prob_array, (void *)tmp_prob_array, sizeof(tmp_prob_array));
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE;
    return rc;
}

int cmp_sw_hw_a_nodes_wred_curve(tm_handle hndl,  uint8_t curve_ind)
{
    int rc = -ERANGE;
    uint8_t *prob_array;
    uint8_t tmp_prob_array[32];
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (curve_ind < TM_NUM_WRED_A_NODE_CURVES)
    {
        tm_memset(tmp_prob_array, 0, sizeof(tmp_prob_array));

        prob_array = (uint8_t *)(ctl->tm_wred_a_lvl_curves[curve_ind].prob);
        rc = tm_get_hw_wred_curve(hndl, A_LEVEL, 0, curve_ind, tmp_prob_array);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)prob_array, (void *)tmp_prob_array, sizeof(tmp_prob_array));
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_b_nodes_wred_curve(tm_handle hndl,   uint8_t curve_ind)
{
    int rc = -ERANGE;
    uint8_t *prob_array;
    uint8_t tmp_prob_array[32];
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (curve_ind < TM_NUM_WRED_B_NODE_CURVES)
    {
        tm_memset(tmp_prob_array, 0, sizeof(tmp_prob_array));

        prob_array = (uint8_t *)(ctl->tm_wred_b_lvl_curves[curve_ind].prob);
        rc = tm_get_hw_wred_curve(hndl, B_LEVEL, 0, curve_ind, tmp_prob_array);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)prob_array, (void *)tmp_prob_array, sizeof(tmp_prob_array));
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_c_nodes_wred_curve(tm_handle hndl,  uint8_t cos, uint8_t curve_ind)
{
    uint8_t *prob_array;
    int rc =  -EFAULT;
    uint8_t tmp_prob_array[32];
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (curve_ind < TM_NUM_WRED_C_NODE_CURVES)
    {
        tm_memset(tmp_prob_array, 0, sizeof(tmp_prob_array));

        prob_array = (uint8_t *)(ctl->tm_wred_c_lvl_curves[cos][curve_ind].prob);
        rc = tm_get_hw_wred_curve(hndl, C_LEVEL, 0, curve_ind, tmp_prob_array);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)prob_array, (void *)tmp_prob_array, sizeof(tmp_prob_array));
        if (ret)
        {
            return ENOMATCH;
        }
    }

    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_ports_wred_curve(tm_handle hndl, uint8_t curve_ind)
{
    int rc =  -EFAULT;
    uint8_t *prob_array;
    uint8_t tmp_prob_array[32];
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (curve_ind < TM_NUM_WRED_PORT_CURVES)
    {
        tm_memset(tmp_prob_array, 0, sizeof(tmp_prob_array));

        prob_array = (uint8_t *)(ctl->tm_wred_ports_curves[curve_ind].prob);
        rc = tm_get_hw_wred_curve(hndl, P_LEVEL, 0, curve_ind, tmp_prob_array);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)prob_array, (void *)tmp_prob_array, sizeof(tmp_prob_array));
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_ports_wred_curve_cos(tm_handle hndl, uint8_t cos, uint8_t curve_ind)
{
    int rc = -EFAULT;
    uint8_t *prob_array;
    uint8_t tmp_prob_array[32];
    int ret = 0;

    TM_CTL(ctl, hndl);
    if (curve_ind < TM_NUM_WRED_PORT_CURVES)
    {
        tm_memset(tmp_prob_array, 0, sizeof(tmp_prob_array));

        prob_array = (uint8_t *)(ctl->tm_wred_ports_curves_cos[cos][curve_ind].prob);
        rc = tm_get_hw_wred_curve(hndl, P_LEVEL, cos, curve_ind, tmp_prob_array);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)prob_array, (void *)tmp_prob_array, sizeof(tmp_prob_array));
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_queue_drop_profile(tm_handle hndl, uint32_t prof_ind)
{
    int rc =  -EFAULT;
    struct tm_drop_profile *profile;
    struct tm_drop_profile tmp_profile;
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (prof_ind < TM_NUM_QUEUE_DROP_PROF)
    {
        tm_memset(&tmp_profile, 0, sizeof(tmp_profile));

        profile = &(ctl->tm_q_lvl_drop_profiles[prof_ind]);
        rc = tm_get_hw_queue_drop_profile(hndl, prof_ind, &tmp_profile);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)profile, (void *)&tmp_profile, TM_HA_CMP_DROP_PROFILE_SIZE_CNS);
        if (ret)
        {
            return ENOMATCH;
        }

    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_a_nodes_drop_profile(tm_handle hndl, uint32_t prof_ind)
{
    int rc =  -EFAULT;
    struct tm_drop_profile *profile;
    struct tm_drop_profile tmp_profile;
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (prof_ind < TM_NUM_A_NODE_DROP_PROF)
    {
        tm_memset(&tmp_profile, 0, sizeof(tmp_profile));

        profile = &(ctl->tm_a_lvl_drop_profiles[prof_ind]);
        rc = tm_get_hw_a_nodes_drop_profile(hndl, prof_ind, &tmp_profile);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)profile, (void *)&tmp_profile, TM_HA_CMP_DROP_PROFILE_SIZE_CNS);
        if (ret)
        {
            return ENOMATCH;
        }

    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_b_nodes_drop_profile(tm_handle hndl, uint32_t prof_ind)
{
    int rc =  -EFAULT;
    struct tm_drop_profile *profile;
    struct tm_drop_profile tmp_profile;
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (prof_ind < TM_NUM_B_NODE_DROP_PROF)
    {
        tm_memset(&tmp_profile, 0, sizeof(tmp_profile));

        profile = &(ctl->tm_b_lvl_drop_profiles[prof_ind]);
        rc = tm_get_hw_b_nodes_drop_profile(hndl, prof_ind, &tmp_profile);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)profile, (void *)&tmp_profile, TM_HA_CMP_DROP_PROFILE_SIZE_CNS);
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_c_nodes_drop_profile(tm_handle hndl,  uint8_t cos, uint32_t prof_ind)
{
    int rc =  -EFAULT;
    struct tm_drop_profile *profile;
    struct tm_drop_profile tmp_profile;
    int ret = 0;

    TM_CTL(ctl, hndl)

    if (prof_ind < TM_NUM_C_NODE_DROP_PROF)
    {
        tm_memset(&tmp_profile, 0, sizeof(tmp_profile));

        profile = &(ctl->tm_c_lvl_drop_profiles[cos][prof_ind]);
        rc = tm_get_hw_c_nodes_drop_profile(hndl, cos, prof_ind, &tmp_profile);
        if (rc)
        {
            return rc;
        }

        ret = tm_memcmp((void *)profile, (void *)&tmp_profile, TM_HA_CMP_DROP_PROFILE_SIZE_CNS);
        if (ret)
        {
            return ENOMATCH;
        }
    }
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_aging_status(tm_handle hndl)
{
    int rc;
    uint8_t val = 0;
    TM_REGISTER_VAR(TM_Drop_AgingUpdEnable)

    TM_CTL(ctl, hndl)

    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.AgingUpdEnable , TM_Drop_AgingUpdEnable)
    if (rc) goto out;

    TM_REGISTER_GET(TM_Drop_AgingUpdEnable, En , val , (uint8_t))  /* casting to uint8_t */

    if (ctl->aging_status != val)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_tree_deq_status(tm_handle hndl)
{
    int rc =  -EFAULT;
    uint8_t val = 0;
    TM_REGISTER_VAR(TM_Sched_TreeDeqEn)

    TM_CTL(ctl, hndl)

    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.TreeDeqEn, TM_Sched_TreeDeqEn)
    if (rc)  goto out;

    TM_REGISTER_GET(TM_Sched_TreeDeqEn, En , val, (uint8_t) ) /* no casting */
    if (ctl->tree_deq_status != val)
    {
        return ENOMATCH;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_tree_dwrr_priority(tm_handle hndl)
{
    int rc =  -EFAULT;
    uint8_t val = 0;
    TM_REGISTER_VAR(TM_Sched_TreeDWRRPrioEn)

    TM_CTL(ctl, hndl)

    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.TreeDWRRPrioEn, TM_Sched_TreeDWRRPrioEn)
    if (rc) goto out;

    TM_REGISTER_GET(TM_Sched_TreeDWRRPrioEn , PrioEn , ctl->tree_dwrr_enable_per_priority_mask , (uint8_t) )
    if (ctl->tree_dwrr_enable_per_priority_mask != val)
    {
        return ENOMATCH;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_shaping_status(tm_handle hndl, enum tm_level level)
{
    int rc = -ERANGE;
    struct level_config tmp_level_data;
    struct level_config *level_data_ptr;

    TM_REGISTER_VAR(TM_Sched_PortPerConf)
    TM_REGISTER_VAR(TM_Sched_ClvlPerConf)
    TM_REGISTER_VAR(TM_Sched_BlvlPerConf)
    TM_REGISTER_VAR(TM_Sched_AlvlPerConf)
    TM_REGISTER_VAR(TM_Sched_QueuePerConf)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_level_data, 0, sizeof(tmp_level_data));

    switch(level)
    {
    case Q_LEVEL:
        TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueuePerConf, TM_Sched_QueuePerConf)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_QueuePerConf, DecEn,       tmp_level_data.shaper_dec ,   (uint8_t))
        TM_REGISTER_GET(TM_Sched_QueuePerConf, PerInterval, tmp_level_data.per_interval,  (uint32_t))
        TM_REGISTER_GET(TM_Sched_QueuePerConf, PerEn ,      tmp_level_data.shaping_status,(uint8_t))
        TM_REGISTER_GET(TM_Sched_QueuePerConf, TokenResExp, tmp_level_data.token_res_exp, (uint8_t))
        break;
    case A_LEVEL:
        TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlPerConf, TM_Sched_AlvlPerConf)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_AlvlPerConf, DecEn,       tmp_level_data.shaper_dec ,   (uint8_t))
        TM_REGISTER_GET(TM_Sched_AlvlPerConf, PerInterval, tmp_level_data.per_interval,  (uint32_t))
        TM_REGISTER_GET(TM_Sched_AlvlPerConf, PerEn ,      tmp_level_data.shaping_status,(uint8_t))
        TM_REGISTER_GET(TM_Sched_AlvlPerConf, TokenResExp, tmp_level_data.token_res_exp, (uint8_t))
        break;
    case B_LEVEL:
        TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlPerConf, TM_Sched_BlvlPerConf)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BlvlPerConf, DecEn,       tmp_level_data.shaper_dec ,   (uint8_t))
        TM_REGISTER_GET(TM_Sched_BlvlPerConf, PerInterval, tmp_level_data.per_interval,  (uint32_t))
        TM_REGISTER_GET(TM_Sched_BlvlPerConf, PerEn ,      tmp_level_data.shaping_status,(uint8_t))
        TM_REGISTER_GET(TM_Sched_BlvlPerConf, TokenResExp, tmp_level_data.token_res_exp, (uint8_t))
        break;
    case C_LEVEL:
        TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlPerConf, TM_Sched_ClvlPerConf)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvlPerConf, DecEn,       tmp_level_data.shaper_dec ,   (uint8_t))
        TM_REGISTER_GET(TM_Sched_ClvlPerConf, PerInterval, tmp_level_data.per_interval,  (uint32_t))
        TM_REGISTER_GET(TM_Sched_ClvlPerConf, PerEn ,      tmp_level_data.shaping_status,(uint8_t))
        TM_REGISTER_GET(TM_Sched_ClvlPerConf, TokenResExp, tmp_level_data.token_res_exp, (uint8_t))
        break;
    case P_LEVEL:
        TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortPerConf, TM_Sched_PortPerConf)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortPerConf, DecEn,       tmp_level_data.shaper_dec ,   (uint8_t))
        TM_REGISTER_GET(TM_Sched_PortPerConf, PerInterval, tmp_level_data.per_interval,  (uint32_t))
        TM_REGISTER_GET(TM_Sched_PortPerConf, PerEn ,      tmp_level_data.shaping_status,(uint8_t))
        TM_REGISTER_GET(TM_Sched_PortPerConf, TokenResExp, tmp_level_data.token_res_exp, (uint8_t))
        break;
    default:
            goto out;
    }

    level_data_ptr = &ctl->level_data[level];
    if (level_data_ptr->shaper_dec != tmp_level_data.shaper_dec)
    {
        return ENOMATCH;
    }
    if (level_data_ptr->per_interval != tmp_level_data.per_interval)
    {
        return ENOMATCH;
    }
    if (level_data_ptr->shaping_status != tmp_level_data.shaping_status)
    {
        return ENOMATCH;
    }
    if (level_data_ptr->token_res_exp != tmp_level_data.token_res_exp)
    {
        return ENOMATCH;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_periodic_scheme(tm_handle hndl)
{
    int rc =  -EFAULT;
    struct level_config tmp_level_data;

    TM_REGISTER_VAR(TM_Sched_PortPerRateShpPrms)
    TM_REGISTER_VAR(TM_Sched_ClvlPerRateShpPrms)
    TM_REGISTER_VAR(TM_Sched_BlvlPerRateShpPrms)
    TM_REGISTER_VAR(TM_Sched_AlvlPerRateShpPrms)
    TM_REGISTER_VAR(TM_Sched_QueuePerRateShpPrms)
    TM_REGISTER_VAR(TM_Sched_ScrubSlotAlloc)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_level_data, 0, sizeof(tmp_level_data));

    rc = cmp_sw_hw_shaping_status(hndl,P_LEVEL);
    if (rc)
        goto out;

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortPerRateShpPrms, TM_Sched_PortPerRateShpPrms)
    if (rc) goto out;
    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_PortPerRateShpPrms , L , tmp_level_data.L, (uint16_t))
    TM_REGISTER_GET(TM_Sched_PortPerRateShpPrms , K , tmp_level_data.K, (uint16_t))
    TM_REGISTER_GET(TM_Sched_PortPerRateShpPrms , N , tmp_level_data.N, (uint16_t))
    if ((ctl->level_data[P_LEVEL].L != tmp_level_data.L) ||
        (ctl->level_data[P_LEVEL].K != tmp_level_data.K) ||
        (ctl->level_data[P_LEVEL].N != tmp_level_data.N))
    {
        return ENOMATCH;
    }

    rc=cmp_sw_hw_shaping_status(hndl,C_LEVEL);
    if (rc) goto out;

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlPerRateShpPrms, TM_Sched_ClvlPerRateShpPrms)
    if (rc) goto out;
    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_ClvlPerRateShpPrms , L , tmp_level_data.L, (uint16_t))
    TM_REGISTER_GET(TM_Sched_ClvlPerRateShpPrms , K , tmp_level_data.K, (uint16_t))
    TM_REGISTER_GET(TM_Sched_ClvlPerRateShpPrms , N , tmp_level_data.N, (uint16_t))
    if ((ctl->level_data[C_LEVEL].L != tmp_level_data.L) ||
        (ctl->level_data[C_LEVEL].K != tmp_level_data.K) ||
        (ctl->level_data[C_LEVEL].N != tmp_level_data.N))
    {
        return ENOMATCH;
    }

    rc=cmp_sw_hw_shaping_status(hndl,B_LEVEL);
    if (rc) goto out;

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlPerRateShpPrms, TM_Sched_BlvlPerRateShpPrms)
    if (rc) goto out;
    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_BlvlPerRateShpPrms , L , tmp_level_data.L, (uint16_t))
    TM_REGISTER_GET(TM_Sched_BlvlPerRateShpPrms , K , tmp_level_data.K, (uint16_t))
    TM_REGISTER_GET(TM_Sched_BlvlPerRateShpPrms , N , tmp_level_data.N, (uint16_t))
    if ((ctl->level_data[B_LEVEL].L != tmp_level_data.L) ||
        (ctl->level_data[B_LEVEL].K != tmp_level_data.K) ||
        (ctl->level_data[B_LEVEL].N != tmp_level_data.N))
    {
        return ENOMATCH;
    }

    rc=cmp_sw_hw_shaping_status(hndl,A_LEVEL);
    if (rc) goto out;

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlPerRateShpPrms, TM_Sched_AlvlPerRateShpPrms)
    if (rc) goto out;
    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_AlvlPerRateShpPrms , L , tmp_level_data.L, (uint16_t))
    TM_REGISTER_GET(TM_Sched_AlvlPerRateShpPrms , K , tmp_level_data.K, (uint16_t))
    TM_REGISTER_GET(TM_Sched_AlvlPerRateShpPrms , N , tmp_level_data.N, (uint16_t))
    if ((ctl->level_data[A_LEVEL].L != tmp_level_data.L) ||
        (ctl->level_data[A_LEVEL].K != tmp_level_data.K) ||
        (ctl->level_data[A_LEVEL].N != tmp_level_data.N))
    {
        return ENOMATCH;
    }

    rc=cmp_sw_hw_shaping_status(hndl,Q_LEVEL);
    if (rc) goto out;

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueuePerRateShpPrms, TM_Sched_QueuePerRateShpPrms)
    if (rc) goto out;
    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_QueuePerRateShpPrms , L , tmp_level_data.L, (uint16_t))
    TM_REGISTER_GET(TM_Sched_QueuePerRateShpPrms , K , tmp_level_data.K, (uint16_t))
    TM_REGISTER_GET(TM_Sched_QueuePerRateShpPrms , N , tmp_level_data.N, (uint16_t))
    if ((ctl->level_data[Q_LEVEL].L != tmp_level_data.L) ||
        (ctl->level_data[Q_LEVEL].K != tmp_level_data.K) ||
        (ctl->level_data[Q_LEVEL].N != tmp_level_data.N))
    {
        return ENOMATCH;
    }

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ScrubSlotAlloc, TM_Sched_ScrubSlotAlloc)
    if (rc) goto out;
    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_ScrubSlotAlloc , PortSlots , tmp_level_data.scrub_slots, (uint8_t))
    if (ctl->level_data[P_LEVEL].scrub_slots != tmp_level_data.scrub_slots)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Sched_ScrubSlotAlloc, ClvlSlots,   tmp_level_data.scrub_slots, (uint8_t))
    if (ctl->level_data[C_LEVEL].scrub_slots != tmp_level_data.scrub_slots)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Sched_ScrubSlotAlloc , BlvlSlots , tmp_level_data.scrub_slots, (uint8_t))
    if (ctl->level_data[B_LEVEL].scrub_slots != tmp_level_data.scrub_slots)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Sched_ScrubSlotAlloc , AlvlSlots , tmp_level_data.scrub_slots, (uint8_t))
    if (ctl->level_data[A_LEVEL].scrub_slots != tmp_level_data.scrub_slots)
    {
        return ENOMATCH;
    }

    TM_REGISTER_GET(TM_Sched_ScrubSlotAlloc , QueueSlots ,tmp_level_data.scrub_slots, (uint8_t))
    if (ctl->level_data[Q_LEVEL].scrub_slots != tmp_level_data.scrub_slots)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_gen_conf(tm_handle hndl)
{
    int rc = 0;
    uint8_t val = 0;
    TM_REGISTER_VAR(TM_Sched_PortExtBPEn)

    TM_CTL(ctl, hndl)

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortExtBPEn, TM_Sched_PortExtBPEn)
    if (rc) goto out;

    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_PortExtBPEn , En , val, (uint8_t));
    if (ctl->port_ext_bp_en != val)
    {
        return ENOMATCH;
    }
out:
    COMPLETE_HW_WRITE;
    return rc;
}

/**
 */
int cmp_sw_hw_dwrr_limit(tm_handle hndl)
{
    int rc = 0;
    uint8_t val = 0;
    TM_REGISTER_VAR(TM_Sched_PortDWRRBytesPerBurstsLimit)

    TM_CTL(ctl, hndl)

    /* read register */
    TM_READ_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortDWRRBytesPerBurstsLimit,TM_Sched_PortDWRRBytesPerBurstsLimit)
    if (rc) goto out;

    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_PortDWRRBytesPerBurstsLimit, limit , val, (uint8_t))
    if (ctl->dwrr_bytes_burst_limit != val)
    {
        return ENOMATCH;
    }
out:
    COMPLETE_HW_WRITE;
    return rc;
}

/**
 *  Configure user Q level Eligible Priority Function
 */
int cmp_sw_hw_q_elig_prio_func_entry(tm_handle hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    struct tm_elig_prio_func_queue *params;
    struct tm_elig_prio_func_queue tmp_params;
    int ret = 0;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFunc)

    TM_CTL(ctl, hndl)

    params = &(ctl->tm_elig_prio_q_lvl_tbl[func_offset]);

    tm_memset(&tmp_params, 0, sizeof(tmp_params));

    /* read register */
    TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueueEligPrioFunc, func_offset, TM_Sched_QueueEligPrioFunc)
    if (rc)
        goto out;

    /* assign register fields */
    TM_REGISTER_GET(TM_Sched_QueueEligPrioFunc , FuncOut0 ,  tmp_params.tbl_entry.func_out[0], (uint16_t))
    TM_REGISTER_GET(TM_Sched_QueueEligPrioFunc , FuncOut1 ,  tmp_params.tbl_entry.func_out[1], (uint16_t))
    TM_REGISTER_GET(TM_Sched_QueueEligPrioFunc , FuncOut2 ,  tmp_params.tbl_entry.func_out[2], (uint16_t))
    TM_REGISTER_GET(TM_Sched_QueueEligPrioFunc , FuncOut3 ,  tmp_params.tbl_entry.func_out[3], (uint16_t))

    ret = tm_memcmp((void *)params, (void *)&tmp_params, sizeof(tmp_params));
    if (ret)
    {
        return ENOMATCH;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 *  Configure user Node level Eligible Priority Function
 */
int cmp_sw_hw_a_lvl_elig_prio_func_entry(tm_handle hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;
    struct tm_elig_prio_func_node *params;
    struct tm_elig_prio_func_node tmp_params;
    int ret = 0;

    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_params, 0, sizeof(tmp_params));

    params = &ctl->tm_elig_prio_a_lvl_tbl[func_offset];
    for (i = 0; i < 8; i++)
    {
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_AlvlEligPrioFunc_Entry)
        if (rc) goto out;

        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut0 , tmp_params.tbl_entry[i].func_out[0], (uint16_t))
        TM_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut1 , tmp_params.tbl_entry[i].func_out[1], (uint16_t))
        TM_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut2 , tmp_params.tbl_entry[i].func_out[2], (uint16_t))
        TM_REGISTER_GET(TM_Sched_AlvlEligPrioFunc_Entry , FuncOut3 , tmp_params.tbl_entry[i].func_out[3], (uint16_t))
    }

    ret = tm_memcmp((void *)params, (void *)&tmp_params, sizeof(tmp_params));
    if (ret)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_b_lvl_elig_prio_func_entry(tm_handle hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;
    struct tm_elig_prio_func_node *params;
    struct tm_elig_prio_func_node tmp_params;
    int ret = 0;

    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_params, 0, sizeof(tmp_params));

    params = &ctl->tm_elig_prio_b_lvl_tbl[func_offset];

    for (i = 0; i < 8; i++)
    {
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_BlvlEligPrioFunc_Entry)
        if (rc) goto out;

        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut0 , tmp_params.tbl_entry[i].func_out[0], (uint16_t))
        TM_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut1 , tmp_params.tbl_entry[i].func_out[1], (uint16_t))
        TM_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut2 , tmp_params.tbl_entry[i].func_out[2], (uint16_t))
        TM_REGISTER_GET(TM_Sched_BlvlEligPrioFunc_Entry , FuncOut3 , tmp_params.tbl_entry[i].func_out[3], (uint16_t))
    }

    ret = tm_memcmp((void *)params, (void *)&tmp_params, sizeof(tmp_params));
    if (ret)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_c_lvl_elig_prio_func_entry(tm_handle hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;
    struct tm_elig_prio_func_node *params;
    struct tm_elig_prio_func_node tmp_params;
    int ret = 0;

    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_params, 0, sizeof(tmp_params));

    params = &ctl->tm_elig_prio_c_lvl_tbl[func_offset];
    for (i = 0; i < 8; i++)
    {
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlEligPrioFunc_Entry, func_offset + i*64 ,TM_Sched_ClvlEligPrioFunc_Entry);
        if (rc) goto out;

        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut0 , tmp_params.tbl_entry[i].func_out[0], (uint16_t));
        TM_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut1 , tmp_params.tbl_entry[i].func_out[1], (uint16_t));
        TM_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut2 , tmp_params.tbl_entry[i].func_out[2], (uint16_t));
        TM_REGISTER_GET(TM_Sched_ClvlEligPrioFunc_Entry , FuncOut3 , tmp_params.tbl_entry[i].func_out[3], (uint16_t));
    }

    ret = tm_memcmp((void *)params, (void *)&tmp_params, sizeof(tmp_params));
    if (ret)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE;
    return rc;
}

int cmp_sw_hw_p_lvl_elig_prio_func_entry(tm_handle hndl, uint16_t func_offset)
{
    int rc =  -EFAULT;
    int i;
    struct tm_elig_prio_func_node *params;
    struct tm_elig_prio_func_node tmp_params;
    int ret = 0;

    TM_REGISTER_VAR(TM_Sched_PortEligPrioFunc_Entry)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_params, 0, sizeof(tmp_params));

    params = &ctl->tm_elig_prio_p_lvl_tbl[func_offset];
    for (i = 0; i < 8; i++)
    {
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortEligPrioFunc_Entry , func_offset + i*64 ,TM_Sched_PortEligPrioFunc_Entry);
        if (rc) goto out;

        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry , FuncOut0 , tmp_params.tbl_entry[i].func_out[0], (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry , FuncOut1 , tmp_params.tbl_entry[i].func_out[1], (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry , FuncOut2 , tmp_params.tbl_entry[i].func_out[2], (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortEligPrioFunc_Entry , FuncOut3 , tmp_params.tbl_entry[i].func_out[3], (uint16_t))
    }

    ret = tm_memcmp((void *)params, (void *)&tmp_params, sizeof(tmp_params));
    if (ret)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}


/**
 */
int cmp_sw_hw_elig_prio_func_tbl_q_level(tm_handle hndl)
{
    int j;
    int rc ;

    for (j = 0; j < TM_ELIG_FUNC_TABLE_SIZE; j++)
    {
        rc = cmp_sw_hw_q_elig_prio_func_entry(hndl, j);
        if (rc) break;
    }
    return rc;
}


/**
 */
int cmp_sw_hw_elig_prio_func_tbl_a_level(tm_handle hndl)
{
    int rc =  -EFAULT;
    int j;
    for (j=0; j< TM_ELIG_FUNC_TABLE_SIZE; j++)
    {
        rc = cmp_sw_hw_a_lvl_elig_prio_func_entry(hndl,j);
        if (rc) break;
    }
    return rc;
}


/**
 */
int cmp_sw_hw_elig_prio_func_tbl_b_level(tm_handle hndl)
{
    int rc =  -EFAULT;
    int j;
    for (j=0; j<TM_ELIG_FUNC_TABLE_SIZE; j++)
    {
        rc = cmp_sw_hw_b_lvl_elig_prio_func_entry(hndl,j);
        if (rc) break;
    }
    return rc;
}


/**
 */
int cmp_sw_hw_elig_prio_func_tbl_c_level(tm_handle hndl)
{
    int rc =  -EFAULT;
    int j;
    for (j=0; j<TM_ELIG_FUNC_TABLE_SIZE; j++)
    {
        rc = cmp_sw_hw_c_lvl_elig_prio_func_entry(hndl,j);
        if (rc) break;
    }
    return rc;
}


/**
 */
int cmp_sw_hw_elig_prio_func_tbl_p_level(tm_handle hndl)
{
    int rc =  -EFAULT;
    int j;
    for (j=0; j<TM_ELIG_FUNC_TABLE_SIZE; j++)
    {
        rc = cmp_sw_hw_p_lvl_elig_prio_func_entry(hndl,j);
        if (rc) break;
    }
    return rc;
}


/**
 */
int cmp_sw_hw_node_mapping(tm_handle hndl, enum tm_level lvl, uint32_t index)
{
    int rc =  -EFAULT;
    int tmp_childLo = 0;
    int tmp_childHi = 0;
    uint16_t tmp_nodeParent = 0;

    TM_REGISTER_VAR(TM_Sched_PortRangeMap)
    TM_REGISTER_VAR(TM_Sched_ClvltoPortAndBlvlRangeMap)
    TM_REGISTER_VAR(TM_Sched_BLvltoClvlAndAlvlRangeMap)
    TM_REGISTER_VAR(TM_Sched_ALvltoBlvlAndQueueRangeMap)
    TM_REGISTER_VAR(TM_Sched_QueueAMap)

    TM_CTL(ctl, hndl)

    switch (lvl)
    {
    case P_LEVEL:
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortRangeMap , index , TM_Sched_PortRangeMap)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortRangeMap , Lo , tmp_childLo, (int))
        if (ctl->tm_port_array[index].mapping.childLo != tmp_childLo)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_PortRangeMap, Hi, tmp_childHi, (int))
        if (ctl->tm_port_array[index].mapping.childHi != tmp_childHi)
        {
            return ENOMATCH;
        }

        break;
    case C_LEVEL:
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvltoPortAndBlvlRangeMap , index , TM_Sched_ClvltoPortAndBlvlRangeMap)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap , Port , tmp_nodeParent, (uint16_t))
        if (ctl->tm_c_node_array[index].mapping.nodeParent != tmp_nodeParent)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap, BlvlLo, tmp_childLo, (int))
        if (ctl->tm_c_node_array[index].mapping.childLo != tmp_childLo)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_ClvltoPortAndBlvlRangeMap , BlvlHi , tmp_childHi , (int))
        if (ctl->tm_c_node_array[index].mapping.childHi != tmp_childHi)
        {
            return ENOMATCH;
        }

        break;
    case B_LEVEL:
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BLvltoClvlAndAlvlRangeMap , index , TM_Sched_BLvltoClvlAndAlvlRangeMap)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap , Clvl , tmp_nodeParent, (uint16_t))
        if (ctl->tm_b_node_array[index].mapping.nodeParent != tmp_nodeParent)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlLo , tmp_childLo , (int))
        if (ctl->tm_b_node_array[index].mapping.childLo != tmp_childLo)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_BLvltoClvlAndAlvlRangeMap , AlvlHi , tmp_childHi , (int))
        if (ctl->tm_b_node_array[index].mapping.childHi != tmp_childHi)
        {
            return ENOMATCH;
        }
        break;
    case A_LEVEL:
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ALvltoBlvlAndQueueRangeMap , index , TM_Sched_ALvltoBlvlAndQueueRangeMap)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap , Blvl , tmp_nodeParent, (uint16_t))
        if (ctl->tm_a_node_array[index].mapping.nodeParent != tmp_nodeParent)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap , QueueLo , tmp_childLo , (int))
        if (ctl->tm_a_node_array[index].mapping.childLo != tmp_childLo)
        {
            return ENOMATCH;
        }

        TM_REGISTER_GET(TM_Sched_ALvltoBlvlAndQueueRangeMap , QueueHi , tmp_childHi , (int))
        if (ctl->tm_a_node_array[index].mapping.childHi != tmp_childHi)
        {
            return ENOMATCH;
        }

        break;
    case Q_LEVEL:
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueueAMap , index , TM_Sched_QueueAMap)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_QueueAMap , Alvl , tmp_nodeParent, (uint16_t))
        if (ctl->tm_queue_array[index].mapping.nodeParent != tmp_nodeParent)
        {
            return ENOMATCH;
        }
        break;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_port_shaping(tm_handle hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    struct tm_port *orig_port = NULL;
    struct tm_port tmp_port;
    struct tm_port *port = &tmp_port;
    uint32_t val = 0;

    TM_REGISTER_VAR(TM_Sched_PortTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_PortTokenBucketBurstSize)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_port, 0, sizeof(tmp_port));

    if (port_ind < ctl->tm_total_ports)
    {
        orig_port = &(ctl->tm_port_array[port_ind]);

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortTokenBucketTokenEnDiv , port_ind , TM_Sched_PortTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortTokenBucketTokenEnDiv , Periods ,  port->periods, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortTokenBucketTokenEnDiv , MinToken , val, (uint32_t))
        port->cir_token = (uint16_t)(val & 0x7FF);
        port->min_token_res = (uint16_t)(val >> 11);
        val = 0;
        TM_REGISTER_GET(TM_Sched_PortTokenBucketTokenEnDiv , MaxToken , val, (uint32_t))
        port->eir_token = (uint16_t)(val & 0x7FF);
        port->max_token_res = (uint16_t)(val >> 11);

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortTokenBucketBurstSize , port_ind , TM_Sched_PortTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortTokenBucketBurstSize , MaxBurstSz , port->eir_burst_size, (uint32_t))
        TM_REGISTER_GET(TM_Sched_PortTokenBucketBurstSize , MinBurstSz , port->cir_burst_size, (uint32_t))

        if ((orig_port->periods != port->periods) ||
            (orig_port->cir_token != port->cir_token) ||
            (orig_port->min_token_res != port->min_token_res) ||
            (orig_port->eir_token != port->eir_token) ||
            (orig_port->max_token_res != port->max_token_res) ||
            (orig_port->eir_burst_size != port->eir_burst_size) ||
            (orig_port->cir_burst_size != port->cir_burst_size))

        {
            return ENOMATCH;
        }
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}


/**
 */
int cmp_sw_hw_port_scheduling(tm_handle hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    struct tm_port *orig_port = NULL;
    struct tm_port tmp_port;
    struct tm_port *port = &tmp_port;
    int ret = 0;

    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosLo)
    TM_REGISTER_VAR(TM_Sched_PortQuantumsPriosHi)
    TM_REGISTER_VAR(TM_Sched_PortDWRRPrioEn)

    TM_CTL(ctl, hndl);

    tm_memset(&tmp_port, 0, sizeof(tmp_port));

    if (port_ind < ctl->tm_total_ports)
    {
        orig_port = &(ctl->tm_port_array[port_ind]);

        /* DWRR for Port */
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortQuantumsPriosLo , port_ind , TM_Sched_PortQuantumsPriosLo)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosLo , Quantum0 , port->dwrr_quantum_per_level[0].quantum, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosLo , Quantum1 , port->dwrr_quantum_per_level[1].quantum, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosLo , Quantum2 , port->dwrr_quantum_per_level[2].quantum, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosLo , Quantum3 , port->dwrr_quantum_per_level[3].quantum, (uint16_t))

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortQuantumsPriosHi , port_ind , TM_Sched_PortQuantumsPriosHi)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosHi , Quantum4 , port->dwrr_quantum_per_level[4].quantum, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosHi , Quantum5 , port->dwrr_quantum_per_level[5].quantum, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosHi , Quantum6 , port->dwrr_quantum_per_level[6].quantum, (uint16_t))
        TM_REGISTER_GET(TM_Sched_PortQuantumsPriosHi , Quantum7 , port->dwrr_quantum_per_level[7].quantum, (uint16_t))

        ret = tm_memcmp((void *)orig_port->dwrr_quantum_per_level, (void *)port->dwrr_quantum_per_level,
                        sizeof(port->dwrr_quantum_per_level));
        if(ret)
        {
            return ENOMATCH;
        }

        /* DWRR for C-nodes in Port's range */
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortDWRRPrioEn , port_ind , TM_Sched_PortDWRRPrioEn)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_PortDWRRPrioEn ,En , port->dwrr_enable_per_priority_mask, (uint8_t))
        if (orig_port->dwrr_enable_per_priority_mask != port->dwrr_enable_per_priority_mask)
        {
            return ENOMATCH;
        }
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_ports_drop_profile(tm_handle hndl,  uint32_t prof_ind, uint8_t port_ind)
{
    int rc =  -EFAULT;
    struct tm_drop_profile *profile;
    struct tm_drop_profile tmp_profile;
    int ret = 0;

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_profile, 0, sizeof(tmp_profile));

    profile = &(ctl->tm_p_lvl_drop_profiles[prof_ind]);
    rc  = tm_get_hw_ports_drop_profile(hndl, port_ind, &tmp_profile);
    if (rc)
    {
        return rc;
    }

    ret = tm_memcmp((void *)profile, (void *)&tmp_profile, TM_HA_CMP_DROP_PROFILE_SIZE_CNS);
    if (ret)
    {
        return ENOMATCH;
    }

    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_port_drop_global(tm_handle hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    struct tm_port *port = NULL;

    TM_CTL(ctl, hndl)

    if (port_ind < ctl->tm_total_ports)
    {
        port = &(ctl->tm_port_array[port_ind]);
        rc = cmp_sw_hw_ports_drop_profile(hndl, port->wred_profile_ref, port_ind);
    }
    return rc;
}

/**
 */
int cmp_sw_hw_node_elig_prio_function_ex(tm_handle hndl, enum tm_level node_level, uint32_t node_index,uint8_t *elig_prio_func_ptr)
{
    int rc =  -EFAULT;

    TM_REGISTER_VAR(TM_Sched_QueueEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_PortEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_AlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_BlvlEligPrioFuncPtr)
    TM_REGISTER_VAR(TM_Sched_ClvlEligPrioFuncPtr)

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
    case P_LEVEL:
        if (node_index < ctl->tm_total_ports)
        {
            /* read register */
            TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.PortEligPrioFuncPtr , node_index , TM_Sched_PortEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TM_REGISTER_GET(TM_Sched_PortEligPrioFuncPtr , Ptr , *elig_prio_func_ptr, (uint8_t))
        }
        break;
    case C_LEVEL:
        if (node_index < ctl->tm_total_c_nodes)
        {
            /* read register */
            TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlEligPrioFuncPtr , node_index , TM_Sched_ClvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TM_REGISTER_GET(TM_Sched_ClvlEligPrioFuncPtr , Ptr , *elig_prio_func_ptr, (uint8_t))
        }
        break;
    case B_LEVEL:
        if (node_index < ctl->tm_total_b_nodes)
        {
            /* read register */
            TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlEligPrioFuncPtr , node_index , TM_Sched_BlvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TM_REGISTER_GET(TM_Sched_BlvlEligPrioFuncPtr , Ptr , *elig_prio_func_ptr, (uint8_t))
        }
        break;
    case A_LEVEL:
        if (node_index < ctl->tm_total_a_nodes)
        {
            /* read register */
            TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlEligPrioFuncPtr , node_index , TM_Sched_AlvlEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TM_REGISTER_GET(TM_Sched_AlvlEligPrioFuncPtr , Ptr , *elig_prio_func_ptr, (uint8_t))
        }
        break;
    case Q_LEVEL:
        if (node_index < ctl->tm_total_queues)
        {
            /* read register */
            TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueueEligPrioFuncPtr , node_index , TM_Sched_QueueEligPrioFuncPtr)
            if (rc) break;
            /* assign register fields */
            TM_REGISTER_GET(TM_Sched_QueueEligPrioFuncPtr , Ptr , *elig_prio_func_ptr, (uint8_t))
        }
        break;
    }
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_node_elig_prio_function(tm_handle hndl, enum tm_level node_level, uint32_t node_index)
{
    int rc =  -EFAULT;
    uint8_t tmp_elig_prio_func;

    TM_CTL(ctl, hndl)

    switch (node_level)
    {
        case P_LEVEL:
        {
            rc = cmp_sw_hw_node_elig_prio_function_ex(hndl, node_level, node_index, &tmp_elig_prio_func);
            if (rc)
            {
                return rc;
            }

            if (ctl->tm_port_array[node_index].elig_prio_func != tmp_elig_prio_func)
            {
                return ENOMATCH;
            }

            break;
        }
        case C_LEVEL:
        {
            rc = cmp_sw_hw_node_elig_prio_function_ex(hndl, node_level, node_index, &tmp_elig_prio_func);
            if (rc)
            {
                return rc;
            }
            if (ctl->tm_c_node_array[node_index].elig_prio_func != tmp_elig_prio_func)
            {
                return ENOMATCH;
            }
            break;
        }
        case B_LEVEL:
        {
            rc = cmp_sw_hw_node_elig_prio_function_ex(hndl, node_level, node_index, &tmp_elig_prio_func);
            if (rc)
            {
                return rc;
            }
            if (ctl->tm_b_node_array[node_index].elig_prio_func != tmp_elig_prio_func)
            {
                return ENOMATCH;
            }
            break;
        }
        case A_LEVEL:
        {
            rc = cmp_sw_hw_node_elig_prio_function_ex(hndl, node_level, node_index, &tmp_elig_prio_func);
            if (rc)
            {
                return rc;
            }
            if (ctl->tm_a_node_array[node_index].elig_prio_func != tmp_elig_prio_func)
            {
                return ENOMATCH;
            }
            break;
        }
        case Q_LEVEL:
        {
            rc = cmp_sw_hw_node_elig_prio_function_ex(hndl, node_level, node_index, &tmp_elig_prio_func);
            if (rc)
            {
                return rc;
            }
            if (ctl->tm_queue_array[node_index].elig_prio_func != tmp_elig_prio_func)
            {
                return ENOMATCH;
            }
            break;
        }
        default:
            break;
    }
    COMPLETE_HW_WRITE;
    return rc;
}

/**
 */
int cmp_sw_hw_port(tm_handle hndl, uint8_t port_ind)
{
    int rc =  -EFAULT;
    TM_CTL(ctl, hndl);
    if (port_ind < ctl->tm_total_ports)
    {
        rc = cmp_sw_hw_node_mapping(hndl, P_LEVEL, port_ind);
        if (rc)
            goto out;

        rc = cmp_sw_hw_port_shaping(hndl, port_ind);
        if (rc)
            goto out;

        rc = cmp_sw_hw_port_scheduling(hndl, port_ind);
        if (rc)
            goto out;

        rc = cmp_sw_hw_port_drop_global(hndl, port_ind);
        if (rc)
            goto out;

        rc = cmp_sw_hw_node_elig_prio_function(hndl, P_LEVEL, port_ind);
        if (rc)
            goto out;
    }
out:
    /*   reset_hw_connection(ctl); already done  */
    return rc;
}

int cmp_sw_hw_c_node_drop_profile_ptr(tm_handle hndl, uint32_t node_ind , uint8_t cos)
{
    int rc =  -EFAULT;
    int entry;
    int base_ind;
    uint16_t tmp_drop_prof_ptr[8];
    uint32_t i;

    TM_REGISTER_VAR(TM_Drop_ClvlDropProfPtr_CoS)

    TM_CTL(ctl, hndl)

    return 0; /* [TBD] remove later */

    if ((node_ind < ctl->tm_total_c_nodes) && (cos < TM_WRED_COS))
    {
        /* Set drop profile pointer entry with data exisitng in SW image
         * to avoid read-modify-write from HW */
        /* Entry in the table */
        entry = node_ind/TM_C_DRP_PROF_PER_ENTRY;
        base_ind = entry*TM_C_DRP_PROF_PER_ENTRY;

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.ClvlDropProfPtr_CoS[cos] , entry , TM_Drop_ClvlDropProfPtr_CoS)
        if (rc) goto out;

        /* assign register fields */
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr0 , tmp_drop_prof_ptr[0], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr1 , tmp_drop_prof_ptr[1], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr2 , tmp_drop_prof_ptr[2], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr3 , tmp_drop_prof_ptr[3], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr4 , tmp_drop_prof_ptr[4], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr5 , tmp_drop_prof_ptr[5], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr6 , tmp_drop_prof_ptr[6], (uint16_t))
        TM_REGISTER_GET(TM_Drop_ClvlDropProfPtr_CoS , ProfPtr7 , tmp_drop_prof_ptr[7], (uint16_t))

        for (i=0; i<8; i++)
        {
            if (ctl->tm_c_lvl_drop_prof_ptr[cos][base_ind+i] != tmp_drop_prof_ptr[i])
            {
                return ENOMATCH;
            }
        }
    }

out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_c_node_params(tm_handle hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;
    int i;
    struct tm_c_node *node = NULL;
    struct tm_c_node tmp_node;

    TM_REGISTER_VAR(TM_Sched_ClvlQuantum)
    TM_REGISTER_VAR(TM_Sched_ClvlDWRRPrioEn)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_node, 0, sizeof(tmp_node));

    if (node_ind < ctl->tm_total_c_nodes)
    {
        node = &(ctl->tm_c_node_array[node_ind]);

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlQuantum , node_ind , TM_Sched_ClvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvlQuantum , Quantum , tmp_node.dwrr_quantum, (uint16_t))
        if (node->dwrr_quantum != tmp_node.dwrr_quantum)
        {
            return ENOMATCH;
        }

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlDWRRPrioEn , node_ind , TM_Sched_ClvlDWRRPrioEn)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvlDWRRPrioEn , En , tmp_node.dwrr_enable_per_priority_mask, (uint8_t))
        if (node->dwrr_enable_per_priority_mask != tmp_node.dwrr_enable_per_priority_mask)
        {
            return ENOMATCH;
        }

        for (i=0; i<TM_WRED_COS; i++)
        {
            /* Set drop profile pointer entry with data exisitng in SW image */
            rc = cmp_sw_hw_c_node_drop_profile_ptr(hndl, node_ind , i);
            if (rc) goto out;
        }
        if (rc) goto out;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_b_node_drop_profile_ptr(tm_handle hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;
    int entry;
    int base_ind;
    uint16_t tmp_drop_prof_ptr[8];
    uint32_t i;

    TM_REGISTER_VAR(TM_Drop_BlvlDropProfPtr)

    TM_CTL(ctl, hndl)

    return 0; /* [TBD] remove later */

    if (node_ind < ctl->tm_total_b_nodes)
    {
        /* Set drop profile pointer entry with data exisitng in SW image
         * to avoid read-modify-write from HW */
        /* Entry in the table */
        entry = node_ind/TM_B_DRP_PROF_PER_ENTRY;
        base_ind = entry*TM_B_DRP_PROF_PER_ENTRY;

        /* read register*/
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.BlvlDropProfPtr , entry , TM_Drop_BlvlDropProfPtr);
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr0 , tmp_drop_prof_ptr[0], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr1 , tmp_drop_prof_ptr[1], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr2 , tmp_drop_prof_ptr[2], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr3 , tmp_drop_prof_ptr[3], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr4 , tmp_drop_prof_ptr[4], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr5 , tmp_drop_prof_ptr[5], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr6 , tmp_drop_prof_ptr[6], (uint16_t))
        TM_REGISTER_GET(TM_Drop_BlvlDropProfPtr , ProfPtr7 , tmp_drop_prof_ptr[7], (uint16_t))

        for (i=0; i<8; i++)
        {
            if (ctl->tm_b_lvl_drop_prof_ptr[base_ind+i] != tmp_drop_prof_ptr[i])
            {
                return ENOMATCH;
            }
        }
        (void) base_ind;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_b_node_params(tm_handle hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;
    struct tm_b_node *node = NULL;
    struct tm_b_node tmp_node;

    TM_REGISTER_VAR(TM_Sched_BlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_BlvlDWRRPrioEn)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_node, 0, sizeof(tmp_node));
    if (node_ind < ctl->tm_total_b_nodes)
    {
        node = &(ctl->tm_b_node_array[node_ind]);

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlQuantum , node_ind , TM_Sched_BlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BlvlQuantum , Quantum , tmp_node.dwrr_quantum, (uint16_t))
        if (node->dwrr_quantum != tmp_node.dwrr_quantum)
        {
            return ENOMATCH;
        }

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlDWRRPrioEn , node_ind , TM_Sched_BlvlDWRRPrioEn)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BlvlDWRRPrioEn , En , tmp_node.dwrr_enable_per_priority_mask, (uint8_t))
        if (node->dwrr_enable_per_priority_mask != tmp_node.dwrr_enable_per_priority_mask)
        {
            return ENOMATCH;
        }

        /* Set drop profile pointer entry with data exisitng in SW image */
        rc = cmp_sw_hw_b_node_drop_profile_ptr(hndl, node_ind);
        if (rc) goto out;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */
int cmp_sw_hw_a_node_drop_profile_ptr(tm_handle hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;

    int entry;
    int base_ind;
    uint16_t tmp_drop_prof_ptr[4];
    uint32_t i;
    TM_REGISTER_VAR(TM_Drop_AlvlDropProfPtr)

    TM_CTL(ctl, hndl)

    return 0; /* [TBD] remove later */
    if (node_ind < ctl->tm_total_a_nodes)
    {
        /* Set drop profile pointer entry with data exisitng in SW image
         * to avoid read-modify-write from HW */
        /* Entry in the table */
        entry = node_ind/TM_A_DRP_PROF_PER_ENTRY;
        base_ind = entry*TM_A_DRP_PROF_PER_ENTRY;

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.AlvlDropProfPtr , entry , TM_Drop_AlvlDropProfPtr)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Drop_AlvlDropProfPtr , ProfPtr0 , tmp_drop_prof_ptr[base_ind+0], (uint16_t))
        TM_REGISTER_GET(TM_Drop_AlvlDropProfPtr , ProfPtr1 , tmp_drop_prof_ptr[base_ind+1], (uint16_t))
        TM_REGISTER_GET(TM_Drop_AlvlDropProfPtr , ProfPtr2 , tmp_drop_prof_ptr[base_ind+2], (uint16_t))
        TM_REGISTER_GET(TM_Drop_AlvlDropProfPtr , ProfPtr3 , tmp_drop_prof_ptr[base_ind+3], (uint16_t))
        for (i=0; i<4; i++)
        {
            if (ctl->tm_a_lvl_drop_prof_ptr[base_ind+i] != tmp_drop_prof_ptr[i])
            {
                return ENOMATCH;
            }
        }

    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_a_node_params(tm_handle hndl, uint32_t node_ind)
{
    int rc =  -EFAULT;

    struct tm_a_node *node = NULL;
    struct tm_a_node tmp_node;

    TM_REGISTER_VAR(TM_Sched_AlvlQuantum)
    TM_REGISTER_VAR(TM_Sched_AlvlDWRRPrioEn)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_node, 0, sizeof(tmp_node));
    if (node_ind < ctl->tm_total_a_nodes)
    {
        node = &(ctl->tm_a_node_array[node_ind]);

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlQuantum , node_ind , TM_Sched_AlvlQuantum)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_AlvlQuantum , Quantum , tmp_node.dwrr_quantum, (uint16_t))
        if (node->dwrr_quantum != tmp_node.dwrr_quantum)
        {
            return ENOMATCH;
        }

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlDWRRPrioEn , node_ind , TM_Sched_AlvlDWRRPrioEn)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_AlvlDWRRPrioEn , En , tmp_node.dwrr_enable_per_priority_mask, (uint8_t))
        if (node->dwrr_enable_per_priority_mask != tmp_node.dwrr_enable_per_priority_mask)
        {
            return ENOMATCH;
        }

        /* Set drop profile pointer entry with data exisitng in SW image */
        rc = cmp_sw_hw_a_node_drop_profile_ptr(hndl, node_ind);
        if (rc) goto out;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

/**
 */

int cmp_sw_hw_queue_drop_profile_ptr(tm_handle hndl, uint32_t queue_ind)
{
    int rc =  -EFAULT;

    int entry;
    int base_ind;
    uint16_t tmp_drop_prof_ptr[4];
    uint32_t i;

    TM_REGISTER_VAR(TM_Drop_QueueDropProfPtr)

    TM_CTL(ctl, hndl)

    return 0; /* [TBD] remove later */
    if (queue_ind < ctl->tm_total_queues)
    {

        /* Set drop profile pointer entry with data exisitng in SW image
         * to avoid read-modify-write from HW */
        /* Entry in the table */
        entry = queue_ind/TM_Q_DRP_PROF_PER_ENTRY;
        base_ind = entry*TM_Q_DRP_PROF_PER_ENTRY;

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Drop.QueueDropProfPtr , entry , TM_Drop_QueueDropProfPtr)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Drop_QueueDropProfPtr , ProfPtr0 , tmp_drop_prof_ptr[base_ind+0], (uint16_t))
        TM_REGISTER_GET(TM_Drop_QueueDropProfPtr , ProfPtr1 , tmp_drop_prof_ptr[base_ind+1], (uint16_t))
        TM_REGISTER_GET(TM_Drop_QueueDropProfPtr , ProfPtr2 , tmp_drop_prof_ptr[base_ind+2], (uint16_t))
        TM_REGISTER_GET(TM_Drop_QueueDropProfPtr , ProfPtr3 , tmp_drop_prof_ptr[base_ind+3], (uint16_t))
        for (i=0; i<4; i++)
        {
            if (ctl->tm_q_lvl_drop_prof_ptr[base_ind+i] != tmp_drop_prof_ptr[i])
            {
                return ENOMATCH;
            }
        }

    }
out:
    COMPLETE_HW_WRITE
    return rc;
}

int cmp_sw_hw_queue_params(tm_handle hndl, uint32_t queue_ind)
{
    int rc =  -EFAULT;

    struct tm_queue *queue = NULL;
    struct tm_queue tmp_queue;

    TM_REGISTER_VAR(TM_Sched_QueueQuantum)

    TM_CTL(ctl, hndl)

    tm_memset(&tmp_queue, 0, sizeof(tmp_queue));
    if (queue_ind < ctl->tm_total_queues)
    {
        queue = &(ctl->tm_queue_array[queue_ind]);

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueueQuantum , queue_ind , TM_Sched_QueueQuantum)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_QueueQuantum , Quantum , tmp_queue.dwrr_quantum, (uint16_t))
        if (queue->dwrr_quantum != tmp_queue.dwrr_quantum)
        {
            return ENOMATCH;
        }

        /* Set drop profile pointer entry with data exisitng in SW image */
        rc = cmp_sw_hw_queue_drop_profile_ptr(hndl,queue_ind);
        if (rc) goto out;
    }
out:
    COMPLETE_HW_WRITE
    return rc;
}


/* assume here that shaping profile is valid */
int cmp_sw_hw_node_shaping_ex(tm_handle hndl, enum tm_level level, uint32_t node_ind, struct tm_shaping_profile *profile)
{
    int rc = 0;
    uint32_t val = 0;

    TM_REGISTER_VAR(TM_Sched_ClvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_BlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_AlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_QueueTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(TM_Sched_ClvlTokenBucketBurstSize)
    TM_REGISTER_VAR(TM_Sched_BlvlTokenBucketBurstSize)
    TM_REGISTER_VAR(TM_Sched_AlvlTokenBucketBurstSize)
    TM_REGISTER_VAR(TM_Sched_QueueTokenBucketBurstSize)

    TM_CTL(ctl, hndl)

    switch (level)
    {
    case Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;
        /* profile assignment */

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueueTokenBucketTokenEnDiv, node_ind, TM_Sched_QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp, (uint8_t));
        TM_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp, (uint8_t));

        TM_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv , MinToken , val, (uint32_t))
        profile->min_token = (uint16_t)(val & 0x7FF);
        profile->min_token_res = (uint16_t)(val >> 11);
        TM_REGISTER_GET(TM_Sched_QueueTokenBucketTokenEnDiv , MaxToken , val, (uint32_t))
        profile->max_token = (uint16_t)(val & 0x7FF);
        profile->max_token_res = (uint16_t)(val >> 11);

#if 0
        TM_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , PerEn ,       is_queue_elig_fun_uses_shaper(ctl->tm_elig_prio_q_lvl_tbl,ctl->tm_queue_array[node_ind].elig_prio_func))
/*
        let prevent case that  eligible function core is changed (in elig_function table)   ,but clients will not be updated
        set PerEn  always enabled for nodes
        TM_REGISTER_SET(TM_Sched_QueueTokenBucketTokenEnDiv , PerEn ,       1)
*/
#endif

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.QueueTokenBucketBurstSize, node_ind,  TM_Sched_QueueTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_QueueTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size, (uint16_t));
        TM_REGISTER_GET(TM_Sched_QueueTokenBucketBurstSize , MinBurstSz , profile->min_burst_size, (uint16_t));
        break;

    case A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlTokenBucketTokenEnDiv, node_ind, TM_Sched_AlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp, (uint8_t));
        TM_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp, (uint8_t));

        TM_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv , MinToken , val, (uint32_t))
        profile->min_token = (uint16_t)(val & 0x7FF);
        profile->min_token_res = (uint16_t)(val >> 11);
        TM_REGISTER_GET(TM_Sched_AlvlTokenBucketTokenEnDiv , MaxToken , val, (uint32_t))
        profile->max_token = (uint16_t)(val & 0x7FF);
        profile->max_token_res = (uint16_t)(val >> 11);

#if 0
        TM_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , PerEn ,       is_queue_elig_fun_uses_shaper(ctl->tm_elig_prio_q_lvl_tbl,ctl->tm_queue_array[node_ind].elig_prio_func))
/*
        let prevent case that  eligible function core is changed (in elig_function table)   ,but clients will not be updated
        set PerEn  always enabled for nodes
        TM_REGISTER_SET(TM_Sched_AlvlTokenBucketTokenEnDiv , PerEn ,       1)
*/
#endif

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.AlvlTokenBucketBurstSize, node_ind, TM_Sched_AlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_AlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size, (uint16_t));
        TM_REGISTER_GET(TM_Sched_AlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size, (uint16_t));
        break;

    case B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlTokenBucketTokenEnDiv, node_ind, TM_Sched_BlvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BlvlTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp, (uint8_t));
        TM_REGISTER_GET(TM_Sched_BlvlTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp, (uint8_t));

        TM_REGISTER_GET(TM_Sched_BlvlTokenBucketTokenEnDiv , MinToken , val, (uint32_t))
        profile->min_token = (uint16_t)(val & 0x7FF);
        profile->min_token_res = (uint16_t)(val >> 11);
        TM_REGISTER_GET(TM_Sched_BlvlTokenBucketTokenEnDiv , MaxToken , val, (uint32_t))
        profile->max_token = (uint16_t)(val & 0x7FF);
        profile->max_token_res = (uint16_t)(val >> 11);

#if 0
        TM_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , PerEn ,       is_queue_elig_fun_uses_shaper(ctl->tm_elig_prio_q_lvl_tbl,ctl->tm_queue_array[node_ind].elig_prio_func))
/*
        let prevent case that  eligible function core is changed (in elig_function table)   ,but clients will not be updated
        set PerEn  always enabled for nodes
        TM_REGISTER_SET(TM_Sched_BlvlTokenBucketTokenEnDiv , PerEn ,       1)
*/
#endif

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.BlvlTokenBucketBurstSize, node_ind,  TM_Sched_BlvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_BlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size, (uint16_t));
        TM_REGISTER_GET(TM_Sched_BlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size, (uint16_t));
        break;

    case C_LEVEL:
        NODE_VALIDATION(ctl->tm_total_c_nodes)
        if (rc) goto out;
        /* profile assignment */
        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlTokenBucketTokenEnDiv, node_ind, TM_Sched_ClvlTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvlTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp, (uint8_t));
        TM_REGISTER_GET(TM_Sched_ClvlTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp, (uint8_t));

        TM_REGISTER_GET(TM_Sched_ClvlTokenBucketTokenEnDiv , MinToken , val, (uint32_t))
        profile->min_token = (uint16_t)(val & 0x7FF);
        profile->min_token_res = (uint16_t)(val >> 11);
        TM_REGISTER_GET(TM_Sched_ClvlTokenBucketTokenEnDiv , MaxToken , val, (uint32_t))
        profile->max_token = (uint16_t)(val & 0x7FF);
        profile->max_token_res = (uint16_t)(val >> 11);

#if 0
        TM_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , PerEn ,       is_queue_elig_fun_uses_shaper(ctl->tm_elig_prio_q_lvl_tbl,ctl->tm_queue_array[node_ind].elig_prio_func))
/*
        let prevent case that  eligible function core is changed (in elig_function table)   ,but clients will not be updated
        set PerEn  always enabled for nodes
        TM_REGISTER_SET(TM_Sched_ClvlTokenBucketTokenEnDiv , PerEn ,       1)
*/
#endif

        /* read register */
        TM_READ_TABLE_REGISTER(PRV_SHARED_TM_PLATFORM_DIR_GLOBAL_VAR_GET(TM).Sched.ClvlTokenBucketBurstSize, node_ind, TM_Sched_ClvlTokenBucketBurstSize)
        if (rc) goto out;
        /* assign register fields */
        TM_REGISTER_GET(TM_Sched_ClvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size, (uint16_t));
        TM_REGISTER_GET(TM_Sched_ClvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size, (uint16_t));
        break;

    default:
        rc = -EFAULT;
        break;
    }
out:
    COMPLETE_HW_WRITE;
    return rc;
}

int cmp_sw_hw_node_shaping(tm_handle hndl, enum tm_level level, uint32_t node_ind)
{
    int rc = 0;
    uint32_t    sh_profile_index;
    struct tm_shaping_profile *profile = NULL;
    struct tm_shaping_profile tmp_profile;
    struct tm_shaping_profile tmp_profile_orig;
    int ret = 0;

    TM_CTL(ctl, hndl)

    /* [TBD] Set rm_shaping_profile_status */

    tm_memset(&tmp_profile, 0, sizeof(tmp_profile));

    /* get shaper parameters from attached shaping profile */
    switch (level)
    {
    case Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;
        sh_profile_index=ctl->tm_queue_array[node_ind].shaping_profile_ref;
        break;
    case A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;
        sh_profile_index=ctl->tm_a_node_array[node_ind].shaping_profile_ref;
        break;
    case B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;
        sh_profile_index=ctl->tm_b_node_array[node_ind].shaping_profile_ref;
        break;
    case C_LEVEL:
        NODE_VALIDATION(ctl->tm_total_c_nodes)
        if (rc) goto out;
        sh_profile_index=ctl->tm_c_node_array[node_ind].shaping_profile_ref;
        break;
    default:
        rc =  -EFAULT;
        goto out;
        break;
    }

    profile = &(ctl->tm_shaping_profiles[sh_profile_index]);
    tm_memcpy(&tmp_profile_orig, profile, sizeof(tmp_profile));
    rc = cmp_sw_hw_node_shaping_ex(hndl,level,node_ind, &tmp_profile);
    if (rc)
    {
        return rc;
    }

    tmp_profile_orig.level = tmp_profile.level; /* No need to check */
    tmp_profile_orig.min_token = tmp_profile_orig.min_token & 0x7FF;
    tmp_profile_orig.max_token = tmp_profile_orig.max_token & 0x7FF;
    tmp_profile_orig.min_token_res = tmp_profile_orig.min_token_res & 0x1;
    tmp_profile_orig.max_token_res = tmp_profile_orig.max_token_res & 0x1;

    ret = tm_memcmp((void *)&tmp_profile_orig, (void *)&tmp_profile, TM_HA_CMP_SHAPING_PROFILE_SIZE_CNS);
    if (ret)
    {
        return ENOMATCH;
    }

out:
    COMPLETE_HW_WRITE;
    return rc;
}
