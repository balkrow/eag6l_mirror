
/*
 * (c), Copyright 2009-2013, Marvell International Ltd.  (Marvell)
 *
 * This code contains confidential information of Marvell.
 * No rights are granted herein under any patent, mask work right or copyright
 * of Marvell or any third party. Marvell reserves the right at its sole
 * discretion to request that this code be immediately returned to Marvell.
 * This code is provided "as is". Marvell makes no warranties, expressed,
 * implied or otherwise, regarding its accuracy, completeness or performance.
 */
/**
 * @brief utilities for calculation shaping parameters
 *
* @file tm_shaping_utils.c
*
* $Revision: 2.0 $
 */



#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedShapingUtils.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/core/prvSchedOsInterface.h>
#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSchedErrCodes.h>

#include <cpss/dxCh/dxChxGen/txq/private/Sched/prvSched.h>
#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>

#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <stdlib.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>



#define MINIMAL_PORT_SHAPER_ACCURACY    0.005
#define OPTIMAL_PORT_SHAPER_ACCURACY    0.0001


int prvSchedShapingUtilsCalculateNodeShapingNoBurstSizeAdaption
(
     GT_U8       devNum,
     PRV_DXCH_TXQ_SIP_6_SHAPER_INPUT_STC  *inParamsPtr,
     PRV_DXCH_TXQ_SIP_6_SHAPER_OUTPUT_STC *outParamsPtr
)
{
    uint16_t    token=0;

    double      next_bw_in_tokens,roundUpElement = 0.5,shaperAcceptableInaccuracy ;
    double      accuracy = 0;
    uint32_t    effective_token;
    uint8_t     divider_exp;
    unsigned int    maximal_token;
    CPSS_DXCH_TXQ_SHAPER_ROUND_MODE_ENT   roundMode;
    int rc;

    /*backward compatible*/
    if(inParamsPtr->hndl==NULL)
    {
        maximal_token = MAX_POSSIBLE_TOKEN;
    }

    roundMode = PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).shaperAlgCfg[inParamsPtr->tbNum].roundMode;
    rc = prvCpssTxqSchedulerGetMaximalToken(devNum,&maximal_token);
    shaperAcceptableInaccuracy = PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).shaperAlgCfg[inParamsPtr->tbNum].errorPercent/100.0;

    if(rc)return rc;

    if(CPSS_DXCH_TXQ_SHAPER_ROUND_MODE_NEG_E == roundMode)
    {
        roundUpElement =0;
    }

    outParamsPtr->resolution=0;
    /* check if necessary to use token resolution for max cir/eir */
    if ( (inParamsPtr->bw_in_tokens / ( 1 << inParamsPtr->divider_res_exp)) > maximal_token )
    {
        outParamsPtr->resolution=1;
        inParamsPtr->bw_in_tokens /= inParamsPtr->token_resolution_multiplyer;
    }

    /* find maximum accuracy solution ( regardless of burst) */
    for ( divider_exp = inParamsPtr->divider_res_exp;  divider_exp > 0 ; divider_exp--)
    {
        if (inParamsPtr->bw_in_tokens < maximal_token) break;
        inParamsPtr->bw_in_tokens /=2.0;
    }
    if ( inParamsPtr->bw_in_tokens > maximal_token ) return 1; /* bandwidth error - out of range  , failed to configure */

    /* try to decrease burst size - trading off accuracy  */
    while( divider_exp > 0 && (PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).shaperAlgCfg[inParamsPtr->tbNum].burstOptimization))
    {
        /* calculate accuracy in case of decreasing divider exponent */
        next_bw_in_tokens=inParamsPtr->bw_in_tokens /2.0;
        accuracy=(1.0*((int)(next_bw_in_tokens +roundUpElement))-next_bw_in_tokens)/next_bw_in_tokens;

        if(CPSS_DXCH_TXQ_SHAPER_ROUND_MODE_POS_E == roundMode)
        {
            if(accuracy<0.0)
             {
                roundUpElement = 1;
                accuracy=(1.0*((int)(next_bw_in_tokens +roundUpElement))-next_bw_in_tokens)/next_bw_in_tokens;
            }
        }

       if (schedFabsDb(accuracy) > shaperAcceptableInaccuracy) break;
        inParamsPtr->bw_in_tokens = next_bw_in_tokens;
        divider_exp--;
    }
    /* check if */
    token=(uint16_t )(inParamsPtr->bw_in_tokens + roundUpElement);
    /* check if it enough accurate*/
    accuracy=(1.0*token-inParamsPtr->bw_in_tokens)/inParamsPtr->bw_in_tokens;

    if(GT_FALSE==PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).permissiveShaperMode)
    {
        if (schedFabsDb(accuracy) > shaperAcceptableInaccuracy) return 3;
    }


    /* calculate effective token size in bytes and compare it to desired burst size */
    effective_token = token *  ((outParamsPtr->resolution) ? inParamsPtr->token_resolution_multiplyer : 1);
    if (effective_token > inParamsPtr->burst_size_bytes)
    {
        inParamsPtr->burst_size_bytes = effective_token;
        return 2;
    }

    /* here successfully configured */
    outParamsPtr->divider_exp=divider_exp;
    outParamsPtr->token = token;
    outParamsPtr->out_accuracy = accuracy;
    outParamsPtr->effectiveBurst = effective_token;
    outParamsPtr->token_resolution_multiplyer = inParamsPtr->token_resolution_multiplyer;


    if(token>0&&PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).logShapingResults )
    {
        schedPrintf("\nNode shaping profile result:\n");
        schedPrintf("token = %d \n",token);
        schedPrintf("burst_size_bytes = %d \n",effective_token);
        schedPrintf("resolution = %d \n",outParamsPtr->resolution);
        schedPrintf("divider_exp = %d \n",divider_exp);
        schedPrintf("accuracy = % lf \n",accuracy);
    }
    return 0;
}

int prvSchedShapingUtilsCalculateNodeShaping(
                            GT_U8       devNum,
                            double      cir_units,
                            double      eir_units,
                            uint16_t *  pcbs_kb,
                            uint16_t *  pebs_kb,
                            uint32_t    token_resolution_multiplyer,
                            uint8_t *   pcir_divider_exp,
                            uint8_t *   peir_divider_exp,
                            uint8_t *   pcir_resolution,
                            uint8_t *   peir_resolution,
                            uint16_t *  pcir_token,
                            uint16_t *  peir_token  ,
                            PRV_CPSS_SCHED_HANDLE hndl,
                            uint8_t     divider_res_exp,
                            double      * out_cir_accuracy_ptr,
                            double      * out_eir_accuracy_ptr
                            )
{
    uint32_t    burst;
    PRV_DXCH_TXQ_SIP_6_SHAPER_INPUT_STC  inParams;
    PRV_DXCH_TXQ_SIP_6_SHAPER_OUTPUT_STC outParams = {0,0,0,0,0,0};

    inParams.hndl = hndl;
    inParams.token_resolution_multiplyer = token_resolution_multiplyer;
    inParams.divider_res_exp = divider_res_exp;

    burst = *pcbs_kb * 1024;

    inParams.burst_size_bytes = burst;
    inParams.bw_in_tokens = cir_units;

    inParams.tbNum = PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.bucketUsedForShaping;

    switch (prvSchedShapingUtilsCalculateNodeShapingNoBurstSizeAdaption(devNum,
                                                &inParams,
                                                &outParams))
    {
        case 1: return  TM_BW_OUT_OF_RANGE;
        case 2:
            {
                *pcbs_kb =(uint16_t)((burst+512) /1024);
                burst = *pebs_kb * 1024;
                prvSchedShapingUtilsCalculateNodeShapingNoBurstSizeAdaption(devNum,&inParams,&outParams);
                *pebs_kb =(uint16_t) (burst+512) /1024;
                return  TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE;
            }
        case 3: return  TM_BW_UNDERFLOW;
        default: break;
    }

    *pcir_divider_exp =outParams.divider_exp;
    *pcir_resolution = outParams.resolution;
    *pcir_token =outParams.token;
    *out_cir_accuracy_ptr = outParams.out_accuracy;

    burst = *pebs_kb * 1024;

    inParams.burst_size_bytes = burst;
    inParams.bw_in_tokens = eir_units;
    inParams.tbNum = 1-PRV_CPSS_DXCH_PP_MAC(devNum)->pdqInfo.bucketUsedForShaping;

    switch (prvSchedShapingUtilsCalculateNodeShapingNoBurstSizeAdaption(devNum,&inParams,&outParams))
    {
        case 1: return TM_BW_OUT_OF_RANGE;
        case 2:
            {
                *pebs_kb = (uint16_t)((burst+512) /1024);
                return  TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE;
            }
        case 3: return TM_BW_UNDERFLOW;
        default: break;
    }


    *peir_divider_exp =outParams.divider_exp;
    *peir_resolution = outParams.resolution;
    *peir_token =outParams.token;
    *out_eir_accuracy_ptr = outParams.out_accuracy;

    return 0;
}


/* following constants are used for separation cir/cbs  & eir/ebs related errors in case of processing  max of them */
#define EXT_ERROR_SHIFT             10
#define SET_CIR_MAX                 error_shift=0;
#define SET_EIR_MAX                 error_shift=EXT_ERROR_SHIFT;

#define MAX_ERROR_SHIFT(error)      error_shift+error
#define MIN_ERROR_SHIFT(error)      (error+EXT_ERROR_SHIFT - error_shift)

int prvSchedShapingUtilsCalculatePortShaping(
                            GT_U8 devNum,
                            double cir_units,
                            double      eir_units,
                            uint32_t *  pcbs_kb,
                            uint32_t *  pebs_kb,
                            uint32_t    token_resolution_multiplyer,
                            uint16_t *  pperiods,
                            uint8_t *   pcir_resolution,
                            uint8_t *   peir_resolution,
                            uint16_t *  pcir_token,
                            uint16_t *  peir_token

                           )
{
    uint8_t     max_resolution=0;
    uint8_t     min_resolution=0;
    uint16_t    max_token=0;
    uint16_t    min_token=0;

    double      max_bw_in_tokens;
    double      min_bw_in_tokens;
    uint32_t    max_burst_size_bytes;
    uint32_t    min_burst_size_bytes;
    uint16_t    periods;
    uint16_t        port_max_periods;

    double      exact_ratio;
    double      actual_ratio;
    double      accuracy = 0;
    uint16_t    token;
    uint16_t    divider;
    uint32_t    effective_token;
    int         error_shift;

    double      best_accuracy;
    uint16_t    best_token;
    uint16_t    best_periods;
    uint32_t    updated_cbs;
    uint32_t    updated_ebs;
    int         rc;
    unsigned int    maximal_token;

    updated_cbs = 0;
    updated_ebs = 0;
    rc = 0;

    rc = prvCpssTxqSchedulerGetPortMaxPeriods(&port_max_periods);
    if(rc)return rc;

    rc = prvCpssTxqSchedulerGetMaximalToken(devNum,&maximal_token);
    if(rc)return rc;

    if ( cir_units >= eir_units)
    {
        max_bw_in_tokens=cir_units;
        max_burst_size_bytes=*pcbs_kb*1024;
        min_bw_in_tokens=eir_units;
        min_burst_size_bytes=*pebs_kb*1024;
        SET_CIR_MAX;
    }
    else
    {
        max_bw_in_tokens=eir_units;
        min_bw_in_tokens=cir_units;
        max_burst_size_bytes=*pebs_kb*1024;
        min_burst_size_bytes=*pcbs_kb*1024;
        SET_EIR_MAX
    }

    /* check if nesessary to use token resolution for max cir/eir */
    if (max_bw_in_tokens > ((double)(maximal_token) * port_max_periods))
    {
        max_resolution=1;
        max_bw_in_tokens /= token_resolution_multiplyer;
    }

    /*  find maximum accuracy solution ( regardless of burst)
      using periods value try to find maximal token in  range  1::MAX_POSSIBLE_TOKEN, so rounding error will be at most 0.5/token  */
    periods = port_max_periods;
    if (max_bw_in_tokens > maximal_token)
    {
        /* calculate desired periods - */
        periods=(uint16_t)(((double)maximal_token)/max_bw_in_tokens*(double)port_max_periods);
        if (periods==0) periods=1;
        /* calculate token */
        max_bw_in_tokens=max_bw_in_tokens*(double)(periods)/(double)port_max_periods;
    }

/*  if (max_bw_in_tokens > MAX_POSSIBLE_TOKEN) return MAX_ERROR_SHIFT(1); */ /* bandwidth error - overflow out of range  , failed to configure */
    if (max_bw_in_tokens > maximal_token) return TM_BW_OUT_OF_RANGE;

    /* here we found "exact" solution   :  ratio  max_bw_in_tokens/periods  gives us exact desired banwidth  (before rounding  of token value) */
    exact_ratio=max_bw_in_tokens/periods;
    /* rounded token value for best solution */
    max_token=(uint16_t)(max_bw_in_tokens + 0.5);


    /* now we will try to found  pair of integers <token,periods>  with minimal value of periods &   token/periods ratio  differs from exact ratio
     less than required accuracy */

   /*
      try to iterate between  pairs from 1..max_token  & 1..periods  to achieve  ratio with required accuracy
     in worst case  procedure below will return pair  <token,periods> unchanged
    */
    token=1;
    divider=1;
    best_accuracy=1.0;

    /* not necessary , prevent compilation warning */

    best_token=1;
    best_periods=1;

    while(1)
    {
        if ((divider>port_max_periods) || (token>max_token+1))  break;
        actual_ratio=(1.0*token)/divider;
        accuracy=(actual_ratio-exact_ratio)/exact_ratio;
        /* check if it is solution */
        if (schedFabsDb(accuracy) < best_accuracy)
        {
            best_token=token;
            best_periods=divider;
            best_accuracy=schedFabsDb(accuracy);
            if (best_accuracy < OPTIMAL_PORT_SHAPER_ACCURACY) break; /* achieve better accuracy causes bigger bursness */
        }
        /* iterate to next pair */
        if (actual_ratio > exact_ratio ) divider++;
        else token++;
    }

/*  if (best_accuracy > MINIMAL_PORT_SHAPER_ACCURACY) return MAX_ERROR_SHIFT(3); */ /* not enough accuracy , failed to configure */
    if (best_accuracy > MINIMAL_PORT_SHAPER_ACCURACY) return TM_BW_OUT_OF_RANGE;

    max_token=best_token;
    periods=best_periods;

    /*  here we have "optimal burstness" solution  */
    effective_token = max_token *  (max_resolution ? token_resolution_multiplyer : 1);
    if (effective_token > max_burst_size_bytes)
    {
/*          return MAX_ERROR_SHIFT(2); */
        if (error_shift)    updated_ebs = effective_token;
        else                updated_cbs = effective_token;
        rc = 1;
    }

    /* here successfully configured max of cir/eir  */
    if (min_bw_in_tokens) /* if necessary to configure min */
    {
        min_bw_in_tokens = min_bw_in_tokens*periods/port_max_periods;
        if ( min_bw_in_tokens > maximal_token )
        {
            min_resolution=1;
            min_bw_in_tokens /= token_resolution_multiplyer;
        }
        min_token=(uint16_t)(min_bw_in_tokens + 0.5);
/*      if (min_token==0)  return MAX_ERROR_SHIFT(4); */ /* failed to configure both cir/eir underflow occurred - too wide difference */
        if (min_token==0)   return TM_BW_UNDERFLOW;
        effective_token = min_token *  (min_resolution ? token_resolution_multiplyer : 1);
/*      if (effective_token > min_burst_size_bytes) return MIN_ERROR_SHIFT(2); */
        if (effective_token > min_burst_size_bytes)
        {
            if (error_shift)    updated_cbs = effective_token;
            else                updated_ebs = effective_token;
            rc = 1;
        }
    }

    if (rc)
    {
        if (updated_cbs) *pcbs_kb = updated_cbs/1024 + 1;
        if (updated_ebs) *pebs_kb = updated_ebs/1024 + 1;
        return TM_CONF_BURST_TOO_SMALL_FOR_GIVEN_RATE;
    }


    /*
    here we successfully configure  tokens
    let assign them
    */

    if(max_token>0|| min_token>0)
    {   schedPrintf("\nPort shaping result:\n");
        schedPrintf("max_token = %d  min_token = %d\n",max_token,min_token);
        schedPrintf("burst_size_bytes = %d \n",effective_token);
        schedPrintf("min_resolution = %d max_resolution = %d\n",min_resolution,max_resolution);
        schedPrintf("periods = %d \n",periods);
        schedPrintf("accuracy = % lf \n",accuracy);
    }

    *pperiods=periods;

    if(cir_units > eir_units)
    {
        *pcir_token = max_token;
        *peir_token = min_token;
        *pcir_resolution=max_resolution;
        *peir_resolution=min_resolution;
    }
    else
    {
        *pcir_token = min_token;
        *peir_token = max_token;
        *pcir_resolution=min_resolution;
        *peir_resolution=max_resolution;
    }
    return 0;
}



#define K_L_N_ACCURACY  0.00001
#define K_L_N_BIT_WIDTH 14

#define SIP_6_K_VALUE 0
#define SIP_6_L_VALUE 0
#define SIP_6_N_VALUE 0




/** NEW RevB: Calculate Periodic Rate Shaping L,K,N parameters for a given update
 * bank_upd_rate_shaped for a SINGLE level. If no values found, return an error code */
int prvSchedShapingUtilsFindLKN
(
    uint16_t *L_value,
    uint16_t *K_value,
    uint16_t *N_value
)
{
     /*SIP6 has predifined values. No calculation is required.*/
        *L_value=SIP_6_L_VALUE;
        *K_value=SIP_6_K_VALUE;
        *N_value=SIP_6_N_VALUE;
        return 1;
}


GT_STATUS prvSchedShapingLogResultSet
(
    GT_U32  devNum,
    GT_BOOL logOn
)
{
    PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).logShapingResults = logOn;
    return GT_OK;
}


GT_STATUS  prvSchedShapingBurstOptimizationSet
(
    GT_U32  devNum,
    CPSS_DXCH_TXQ_SHAPER_RATE_TYPE_ENT type,
    GT_BOOL optimizationOn
)
{   GT_U32 tbNum = prvCpssSip6RateTypeToTokenBucket(devNum,type);
    PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).shaperAlgCfg[tbNum].burstOptimization = optimizationOn;
    return GT_OK;
}


