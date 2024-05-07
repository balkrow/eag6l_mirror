/*
 * (c), Copyright 2009-2014, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief  functions for set/get  platform specific registers
 *
* @file set_hw_registers_platform.c
*
* $Revision: 2.0 $
 */
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedCoreTypes.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedRegistersInterface.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/platform/prvSchedSetHwRegistersImp.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSetHwRegisters.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedSetLocalDbDefaults.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_CPSS_DXCH_SIP6_TXQ_ON_EMULATOR           cpssDeviceRunCheck_onEmulator



int prvSchedLowLevelSetPerLevelShapingParameters(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level)
{
    int rc = -ERANGE;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(CPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(APerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BPerCtlConf_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)return rc;

    if (PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E != PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
    {
        if(level >=SCHED_C_LEVEL&&level <=SCHED_P_LEVEL)
        {
             /*No need to configure shaping on those levels on Falcon.Only Q , A and B levels*/
            rc = 0;
        }
    }

    switch (level)
    {
    case SCHED_Q_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerCtlConf_Addr, DecEn,           ctl->level_data[SCHED_Q_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerCtlConf_Addr, PerInterval ,    ctl->level_data[SCHED_Q_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QPerCtlConf_Addr, PerEn ,          ctl->level_data[SCHED_Q_LEVEL].shaping_status)


        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerCtlConf_Addr,QPerCtlConf_Addr)
        break;
    case SCHED_A_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(APerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_A_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_A_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(APerCtlConf_Addr, PerEn ,       ctl->level_data[SCHED_A_LEVEL].shaping_status)



        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.APerCtlConf_Addr,APerCtlConf_Addr)
        break;

    case SCHED_B_LEVEL:
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(BPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_B_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_B_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(BPerCtlConf_Addr, PerEn ,       ctl->level_data[SCHED_B_LEVEL].shaping_status)



        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BPerCtlConf_Addr,BPerCtlConf_Addr)
        break;

    case SCHED_C_LEVEL:

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(CPerCtlConf_Addr)
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(CPerCtlConf_Addr,DecEn ,       ctl->level_data[SCHED_C_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(CPerCtlConf_Addr,PerInterval , ctl->level_data[SCHED_C_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(CPerCtlConf_Addr,PerEn ,       ctl->level_data[SCHED_C_LEVEL].shaping_status)


        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.CPerCtlConf_Addr, CPerCtlConf_Addr)

        break;
    case SCHED_P_LEVEL:

        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(PPerCtlConf_Addr)
        if (rc) break;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PPerCtlConf_Addr, DecEn ,       ctl->level_data[SCHED_P_LEVEL].shaper_dec)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PPerCtlConf_Addr, PerInterval , ctl->level_data[SCHED_P_LEVEL].per_interval)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(PPerCtlConf_Addr, PerEn ,       ctl->level_data[SCHED_P_LEVEL].shaping_status)

        /* write register */
        TXQ_SCHED_WRITE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PPerCtlConf_Addr,PPerCtlConf_Addr)

        break;

    default:
        break;
    }
    return rc;
}


int prvSchedLowLevelGetPerLevelShapingStatusFromHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    enum schedLevel level ,
    struct schedPerLevelPeriodicParams *periodic_params
)
{
    int rc = -ERANGE;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(PPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(CPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(BPerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(APerCtlConf_Addr)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QPerCtlConf_Addr)

    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    switch(level)
    {
    case SCHED_Q_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QPerCtlConf_Addr,QPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerCtlConf_Addr, DecEn,           periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerCtlConf_Addr, PerEn ,          periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QPerCtlConf_Addr, PerInterval ,    periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_A_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.APerCtlConf_Addr,APerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(APerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_B_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.BPerCtlConf_Addr,BPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(BPerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_C_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.CPerCtlConf_Addr,CPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerCtlConf_Addr ,PerEn ,       periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(CPerCtlConf_Addr, PerInterval ,          periodic_params->per_interval , (uint32_t))
        break;
    case SCHED_P_LEVEL:
        TXQ_SCHED_READ_REGISTER_DEVDEPEND(addressSpacePtr->Sched.PPerCtlConf_Addr,PPerCtlConf_Addr)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerCtlConf_Addr, DecEn ,       periodic_params->shaper_dec , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerCtlConf_Addr, PerEn ,       periodic_params->per_state , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(PPerCtlConf_Addr, PerInterval , periodic_params->per_interval , (uint32_t))
    }
out:
    return rc;
}




/* assume here that shaping profile is valid */
int prvSchedLowLevelSetNodeShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, enum schedLevel level, uint32_t node_ind, struct prvCpssDxChTxqSchedShapingProfile *profile)
{
    int rc = -EFAULT;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(AlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(BlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(ClvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(QueueTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(PortTokenBucketBurstSize)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_AlvlTokenBucketMapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_BlvlTokenBucketMapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_ClvlTokenBucketMapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_PlvlTokenBucketMapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueTokenBucketTokenEnDiv)


    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        if(level >=SCHED_C_LEVEL&&level <=SCHED_P_LEVEL)
        {
          if(PRV_CPSS_DXCH_TXQ_SCHEDULING_TREE_MODE_FULL_E != PRV_DXCH_TXQ_SCHED_TREE_MODE_GET(PDQ_ENV_DEVNUM_NUM(ctl)))
          {
             /*No need to configure shaping on those levels on Falcon.Only Q , A and B levels*/
            return 0;
          }
        }
    }


    /*  profile <--> level conformance test */
    if ((profile->level !=level) && (profile->level != ALL_LEVELS))
    {
        rc = -EFAULT;
        goto out;
    }

    switch (level)
    {
    case SCHED_Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;
        /* profile assignment */
        /* reset register reserved fields */
        TXQ_SCHED_REGISTER_DEVDEPEND_RESET(QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MinDivExp ,   profile->min_div_exp);
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MaxDivExp ,   profile->max_div_exp);

        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MinToken ,   profile->min_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MaxToken ,   profile->max_token )
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_Q_LEVEL].token_res_exp)
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET(QueueTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_Q_LEVEL].token_res_exp)

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_TRUE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_FROM_SIP_6_30(QueueTokenBucketTokenEnDiv, AbvEn, profile->credit_accumulation_disable)
        }


        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(QueueTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(QueueTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size);
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(QueueTokenBucketBurstSize , MinBurstSz , profile->min_burst_size);
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.QueueTokenBucketBurstSize, node_ind,  QueueTokenBucketBurstSize)
            if (rc) goto out;
        }
        else
        {   /*In SIP7 MinBurstSz and MaxBurstSz are part of QueueTokenBucketTokenEnDiv register*/
             TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_QueueTokenBucketTokenEnDiv, MaxBurstSz , profile->max_burst_size)
             TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_QueueTokenBucketTokenEnDiv, MinBurstSz , profile->min_burst_size)
        }


        /* write register */
        TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueTokenBucketTokenEnDiv, node_ind, QueueTokenBucketTokenEnDiv)
        if (rc) goto out;

        break;

    case SCHED_A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;
        /* profile assignment */

        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)


            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MinToken ,  profile->min_token )
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MinTokenRes ,   profile->min_token_res*ctl->level_data[SCHED_A_LEVEL].token_res_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv , MaxTokenRes ,   profile->max_token_res*ctl->level_data[SCHED_A_LEVEL].token_res_exp)



            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.AlvlTokenBucketTokenEnDiv, node_ind, AlvlTokenBucketTokenEnDiv)
            if (rc) goto out;

            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30( AlvlTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(AlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.AlvlTokenBucketBurstSize, node_ind, AlvlTokenBucketBurstSize)
            if (rc) goto out;
        }
        else
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(PDQ_SIP_7_AlvlTokenBucketMapping)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MinDivExp ,    profile->min_div_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MaxDivExp ,    profile->max_div_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MinToken ,  profile->min_token )
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MaxToken ,     profile->max_token )
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MinTokenRes ,   profile->min_token_res*ctl->level_data[SCHED_A_LEVEL].token_res_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MaxTokenRes ,   profile->max_token_res*ctl->level_data[SCHED_A_LEVEL].token_res_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_AlvlTokenBucketMapping , MinBurstSz , profile->min_burst_size)


            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlTokenBucketMapping,
                node_ind, PDQ_SIP_7_AlvlTokenBucketMapping)
            if (rc) goto out;


        }
        break;

    case SCHED_B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;
        /* profile assignment */
        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MinToken ,     profile->min_token)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_B_LEVEL].token_res_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_B_LEVEL].token_res_exp)

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.BlvlTokenBucketTokenEnDiv, node_ind, BlvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(BlvlTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(BlvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.BlvlTokenBucketBurstSize, node_ind,  BlvlTokenBucketBurstSize)
            if (rc) goto out;
        }
        else
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(PDQ_SIP_7_BlvlTokenBucketMapping)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MinDivExp ,    profile->min_div_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MaxDivExp ,    profile->max_div_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MinToken ,  profile->min_token )
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MaxToken ,     profile->max_token )
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MinTokenRes ,   profile->min_token_res*ctl->level_data[SCHED_B_LEVEL].token_res_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MaxTokenRes ,   profile->max_token_res*ctl->level_data[SCHED_B_LEVEL].token_res_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_BlvlTokenBucketMapping , MinBurstSz , profile->min_burst_size)


            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlTokenBucketMapping,
                node_ind, PDQ_SIP_7_BlvlTokenBucketMapping)
            if (rc) goto out;


        }
        break;


    case SCHED_C_LEVEL:

        NODE_VALIDATION(ctl->tm_total_c_nodes)
        if (rc) goto out;

        /* profile assignment */
        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MinDivExp ,    profile->min_div_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MaxDivExp ,    profile->max_div_exp)

            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MinToken ,     profile->min_token)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.ClvlTokenBucketTokenEnDiv, node_ind, ClvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(ClvlTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(ClvlTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.ClvlTokenBucketBurstSize, node_ind,  ClvlTokenBucketBurstSize)
            if (rc) goto out;
        }
        else
        {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_RESET(PDQ_SIP_7_ClvlTokenBucketMapping)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MinDivExp ,    profile->min_div_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MaxDivExp ,    profile->max_div_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MinToken ,  profile->min_token )
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MaxToken ,     profile->max_token )
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MinTokenRes ,   profile->min_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MaxTokenRes ,   profile->max_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_ClvlTokenBucketMapping , MinBurstSz , profile->min_burst_size)


            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlTokenBucketMapping,
                node_ind, PDQ_SIP_7_ClvlTokenBucketMapping)
            if (rc) goto out;


        }
        break;
    case SCHED_P_LEVEL:
          NODE_VALIDATION(ctl->tm_total_ports)
          if (rc) goto out;

          if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
          {
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , Periods ,    1<<profile->min_div_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , MinToken ,     profile->min_token)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , MaxToken ,     profile->max_token )
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , MinTokenRes ,profile->min_token_res*ctl->level_data[SCHED_C_LEVEL].token_res_exp)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv , MaxTokenRes ,profile->max_token_res*ctl->level_data[SCHED_P_LEVEL].token_res_exp)

            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.PortTokenBucketTokenEnDiv, node_ind, PortTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* reset register reserved fields */
            TXQ_SCHED_REGISTER_DEVDEPEND_RESET_UNTIL_SIP_6_30(PortTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketBurstSize , MaxBurstSz , profile->max_burst_size)
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_SET_UNTIL_SIP_6_30(PortTokenBucketBurstSize , MinBurstSz , profile->min_burst_size)
            /* write register */
            TXQ_SCHED_WRITE_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.PortTokenBucketBurstSize, node_ind,  PortTokenBucketBurstSize)
            if (rc) goto out;
          }
          else
          {
           /* reset register reserved fields */
              TXQ_SCHED_REGISTER_RESET(PDQ_SIP_7_PlvlTokenBucketMapping)
              if (rc) goto out;

              /* assign register fields */
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinDivExp , profile->min_div_exp)
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxDivExp , profile->max_div_exp)
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinToken ,  profile->min_token )
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxToken ,  profile->max_token )
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinTokenRes , profile->min_token_res*ctl->level_data[SCHED_P_LEVEL].token_res_exp)
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxTokenRes , profile->max_token_res*ctl->level_data[SCHED_P_LEVEL].token_res_exp)
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxBurstSz , profile->max_burst_size)
              TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinBurstSz , profile->min_burst_size)

              /* write register */
              TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PlvlTokenBucketMapping, node_ind , PDQ_SIP_7_PlvlTokenBucketMapping)
              if (rc) goto out;
         }

        break;
    default:
        rc = -EFAULT;
        break;
    }
out:
    return rc;
}



int prvSchedLowLevelSetPortShapingParametersToHw(PRV_CPSS_SCHED_HANDLE hndl, uint8_t port_ind)
{

    int rc =  -EFAULT;
    struct tm_port *port = NULL;
    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;

    TM_REGISTER_VAR(PDQ_SIP_7_PlvlTokenBucketMapping)
    TM_CTL(ctl, hndl)

    if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
    {
        /*no shaping on ports*/
          return 0;
    }

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    if (port_ind < ctl->tm_total_ports)
    {
        port = &(ctl->tm_port_array[port_ind]);

        /* reset register reserved fields */
        TM_RESET_TABLE_REGISTER(addressSpacePtr->Sched.PortTokenBucketTokenEnDiv , port_ind , PDQ_SIP_7_PlvlTokenBucketMapping)
        if (rc) goto out;
        /* assign register fields */
        /*TXQ_SCHED_REGISTER_SET(TM_Sched_PortTokenBucketTokenEnDiv , Periods ,      port->periods)*/


        TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinToken ,      port->cir_token)
        TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxToken ,      port->eir_token )
        TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinTokenRes ,   ctl->level_data[SCHED_P_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxTokenRes ,   ctl->level_data[SCHED_P_LEVEL].token_res_exp)
        TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MaxBurstSz , port->eir_burst_size)
        TXQ_SCHED_REGISTER_SET(PDQ_SIP_7_PlvlTokenBucketMapping , MinBurstSz , port->cir_burst_size)

        /*TBD MaxDivExp,MinDivExp*/

        /* write register */

        TXQ_SCHED_WRITE_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PlvlTokenBucketMapping, port_ind , PDQ_SIP_7_PlvlTokenBucketMapping)
        if (rc) goto out;

    }
out:
    return rc;

}

/* assume here that shaping profile is valid */
int prvSchedLowLevelGetNodeShapingParametersFromHw
(
    PRV_CPSS_SCHED_HANDLE hndl,
    enum schedLevel level,
    uint32_t node_ind,
    struct prvCpssDxChTxqSchedShapingProfile *profile
 )
{
     int rc = -EFAULT;

    struct prvCpssDxChTxqSchedAddressSpace * addressSpacePtr = NULL;
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(AlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(BlvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(ClvlTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(QueueTokenBucketBurstSize)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(PortTokenBucketBurstSize)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_AlvlTokenBucketMapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_BlvlTokenBucketMapping)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_ClvlTokenBucketMapping)
    TXQ_SCHED_DEVDEPEND_REGISTER_VAR_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv)
    TM_REGISTER_VAR(PDQ_SIP_7_PlvlTokenBucketMapping)

    TXQ_SCHED_DEVDEPEND_REGISTER_VAR(QueueTokenBucketTokenEnDiv)
    uint32_t              periods =0;


    TM_CTL(ctl, hndl)

    rc =prvSchedAdressSpacePointerGet(hndl,&addressSpacePtr);
    if (rc)goto out;

    switch (level)
    {
    case SCHED_Q_LEVEL:
        NODE_VALIDATION(ctl->tm_total_queues)
        if (rc) goto out;

        TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND(addressSpacePtr->Sched.QueueTokenBucketTokenEnDiv, node_ind,QueueTokenBucketTokenEnDiv)
        if (rc) goto out;
        /* assign register fields */
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))

        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MaxDivExp,   profile->max_div_exp , (uint8_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MaxToken , profile->max_token , (uint16_t))
        TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET(QueueTokenBucketTokenEnDiv, MaxTokenRes , profile->max_token_res , (uint8_t))

        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.QueueTokenBucketBurstSize, node_ind,QueueTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(QueueTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(QueueTokenBucketBurstSize, MaxBurstSz,   profile->max_burst_size , (uint16_t))
        }
        else
        {
            /*In SIP7 MinBurstSz and MaxBurstSz are part of QueueTokenBucketTokenEnDiv register*/
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_QueueTokenBucketTokenEnDiv,  MinBurstSz,   profile->min_burst_size , (uint16_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_QueueTokenBucketTokenEnDiv,  MaxBurstSz,   profile->min_burst_size , (uint16_t))
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_TRUE)
        {
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_FROM_SIP_6_30(QueueTokenBucketTokenEnDiv, AbvEn, profile->credit_accumulation_disable,(uint8_t))
        }

        break;

      case SCHED_A_LEVEL:
        NODE_VALIDATION(ctl->tm_total_a_nodes)
        if (rc) goto out;

        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.AlvlTokenBucketTokenEnDiv, node_ind,AlvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(AlvlTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.AlvlTokenBucketBurstSize, node_ind,AlvlTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(AlvlTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
        }
        else
        {
            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.AlvlTokenBucketMapping, node_ind ,PDQ_SIP_7_AlvlTokenBucketMapping)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_AlvlTokenBucketMapping, MinDivExp,   profile->min_div_exp , (uint8_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_AlvlTokenBucketMapping, MinToken , profile->min_token , (uint16_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_AlvlTokenBucketMapping, MinTokenRes , profile->min_token_res , (uint8_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_AlvlTokenBucketMapping, MinBurstSz,   profile->min_burst_size , (uint16_t))
        }

        break;
       case SCHED_B_LEVEL:
        NODE_VALIDATION(ctl->tm_total_b_nodes)
        if (rc) goto out;

        if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
        {
            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.BlvlTokenBucketTokenEnDiv, node_ind,BlvlTokenBucketTokenEnDiv)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(BlvlTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


            TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.BlvlTokenBucketBurstSize, node_ind,BlvlTokenBucketBurstSize)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(BlvlTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
        }
        else
        {
            TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.BlvlTokenBucketMapping, node_ind ,PDQ_SIP_7_BlvlTokenBucketMapping)
            if (rc) goto out;
            /* assign register fields */
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_BlvlTokenBucketMapping, MinDivExp,   profile->min_div_exp , (uint8_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_BlvlTokenBucketMapping, MinToken , profile->min_token , (uint16_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_BlvlTokenBucketMapping, MinTokenRes , profile->min_token_res , (uint8_t))
            TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_BlvlTokenBucketMapping, MinBurstSz,   profile->min_burst_size , (uint16_t))
        }
        break;

        case SCHED_C_LEVEL:
            NODE_VALIDATION(ctl->tm_total_c_nodes)
            if (rc) goto out;
            if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.ClvlTokenBucketTokenEnDiv, node_ind,ClvlTokenBucketTokenEnDiv)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv, MinDivExp,   profile->min_div_exp , (uint8_t))
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(ClvlTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.ClvlTokenBucketBurstSize, node_ind,ClvlTokenBucketBurstSize)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(ClvlTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
            }
            else
            {
                TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.ClvlTokenBucketMapping, node_ind ,PDQ_SIP_7_ClvlTokenBucketMapping)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_ClvlTokenBucketMapping, MinDivExp,   profile->min_div_exp , (uint8_t))
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_ClvlTokenBucketMapping, MinToken , profile->min_token , (uint16_t))
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_ClvlTokenBucketMapping, MinTokenRes , profile->min_token_res , (uint8_t))
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_ClvlTokenBucketMapping, MinBurstSz,   profile->min_burst_size , (uint16_t))
            }

        break;


        case SCHED_P_LEVEL:
            NODE_VALIDATION(ctl->tm_total_ports)
            if (rc) goto out;

            if (PRV_CPSS_SIP_7_CHECK_MAC(PDQ_ENV_DEVNUM_NUM(ctl)) == GT_FALSE)
            {
                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.PortTokenBucketTokenEnDiv, node_ind,PortTokenBucketTokenEnDiv)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv, Periods,  periods, (uint32_t))

                profile->min_div_exp=0;
                while(periods>1)
                {
                    profile->min_div_exp++;
                    periods>>=1;
                }
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv, MinToken , profile->min_token , (uint16_t))
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(PortTokenBucketTokenEnDiv, MinTokenRes , profile->min_token_res , (uint8_t))


                TXQ_SCHED_READ_TABLE_REGISTER_DEVDEPEND_UNTIL_SIP_6_30(addressSpacePtr->Sched.PortTokenBucketBurstSize, node_ind,PortTokenBucketBurstSize)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_DEVDEPEND_FIELD_PARSE_GET_UNTIL_SIP_6_30(PortTokenBucketBurstSize, MinBurstSz,   profile->min_burst_size , (uint16_t))
            }
            else
            {
                TXQ_SCHED_READ_TABLE_REGISTER(PDQ_ENV_DEVNUM_NUM(ctl),addressSpacePtr->Sched.PlvlTokenBucketMapping, node_ind ,PDQ_SIP_7_PlvlTokenBucketMapping)
                if (rc) goto out;
                /* assign register fields */
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_PlvlTokenBucketMapping, MinDivExp,   profile->min_div_exp , (uint8_t))
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_PlvlTokenBucketMapping, MinToken , profile->min_token , (uint16_t))
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_PlvlTokenBucketMapping, MinTokenRes , profile->min_token_res , (uint8_t))
                TXQ_SCHED_REGISTER_GET(PDQ_SIP_7_PlvlTokenBucketMapping, MinBurstSz,   profile->min_burst_size , (uint16_t))
            }
            break;


    default:
        rc = -EFAULT;
        break;
    }
out:
    return rc;
}

