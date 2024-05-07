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
* @file prvTgfPolicerECNMarkingCheck.h
*
* @brief Header file for test of UP modification modes due to Egress policer.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfPolicerECNMarkingCheck
#define __prvTgfPolicerECNMarkingCheck

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPolicerGen.h>


/**
* @internal prvTgfPolicerECNMarkingConfigurationSet function
* @endinternal
*
* @brief   Test configurations
*/
GT_VOID prvTgfPolicerECNMarkingConfigurationSet();

/**
* @internal prvTgfPolicerECNMarkingRestore function
* @endinternal
*
* @brief   Test restore configurations
*/
GT_VOID prvTgfPolicerECNMarkingRestore();

/**
* @internal prvTgfPolicerECNMarkingGenerateTraffic function
* @endinternal
*
* @brief   Test generate traffic and check results
*/
GT_VOID prvTgfPolicerECNMarkingGenerateTraffic();

/**
* @internal prvTgfPolicerECNCountingPhaConfigurationSet function
* @endinternal
*
* @brief   PHA thread config
*           Set the FW thread id 80 with given statistical processing factor
*
*/
GT_VOID prvTgfPolicerECNCountingPhaConfigurationSet
(
    IN GT_U32    statisticalProcessingFactor
);

/**
* @internal prvTgfPolicerECNCountingPhaConfigurationRestore function
* @endinternal
*
* @brief PHA test configurations restore
*/
GT_VOID prvTgfPolicerECNCountingPhaConfigurationRestore();

/**
* @internal prvTgfPolicerECNCountingPhaCounterGetAndCheck function
* @endinternal
*
* @brief Read ECN counter entry in PHA shared memory and
*        verify counter is getting incremented.
*/
GT_VOID prvTgfPolicerECNCountingPhaCounterGetAndCheck
(
    IN GT_U32    queueNum
);

/**
* @internal prvTgfPolicerECNCountingPhaCounterReset function
* @endinternal
*
* @brief Clear ECN counter entry in PHA shared memory and
*        verify count is zero.
*/
GT_VOID prvTgfPolicerECNCountingPhaCounterReset
(
    IN GT_U32    queueNum
);

/**
* @internal prvTgfPolicerECNCountingQosConfigSet function
* @endinternal
*
* @brief Set QoS config to send packet to egress queue=queueNum
*/
GT_VOID prvTgfPolicerECNCountingQosConfigSet
(
    IN GT_U32    queueNum
);

/**
* @internal prvTgfPolicerECNCountingQosConfigReset function
* @endinternal
*
* @brief Reset QoS config
*/
GT_VOID prvTgfPolicerECNCountingQosConfigReset();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPolicerECNMarkingCheck */



