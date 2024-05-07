/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *2
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvTgfSerialExactMatch.h
*
* @brief Test Serial Exact Match functionalities
*
* @version   1
********************************************************************************
*/

#ifndef __prvTgfSerialExactMatchh
#define __prvTgfSerialExactMatchh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfExactMatchGen.h>
#include <utf/private/prvUtfExtras.h>

/**
* @enum PRV_TGF_TAG_TYPE_ENT
 *
 * @brief VLAN Tag types
*/
typedef enum{
    PRV_TGF_UNTAGGED_E,
    PRV_TGF_SINGLE_TAGGED_E,
    PRV_TGF_DOUBLE_TAGGED_E
} PRV_TGF_TAG_TYPE_ENT;

/**
* @internal prvTgfSerialExactMatchTtiGenericConfigSet function
* @endinternal
*
* @brief   Set TTI generic test configuration, not related to Exact Match
*
*/
GT_VOID prvTgfSerialExactMatchTtiGenericConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchTtiGenericConfigRestore function
* @endinternal
*
* @brief   Restore TTI generic test configuration, not related to Exact Match
*/
GT_VOID prvTgfSerialExactMatchTtiGenericConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
* @param[in] expectTrapTraffic   - GT_FALSE: expect no traffic
*                                - GT_TRUE: expect traffic to be trapped
*
* @param[in] expectForwardTraffic - GT_FALSE: expect no traffic
*                                 - GT_TRUE: expect traffic to be forwarded
*
*/
GT_VOID prvTgfSerialExactMatchTrafficGenerate
(
    GT_U32      tag,
    GT_BOOL     expectTrapTraffic,
    GT_BOOL     expectForwardTraffic
);

/**
* @internal prvTgfSerialExactMatchBasicDefaultActionTtiCommandTrap function
* @endinternal
*
* @brief   set default action command to be trap with cpu code [502]
*
*/
GT_VOID prvTgfSerialExactMatchBasicDefaultActionTtiCommandTrap
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchBasicDefaultActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Serial Exact Match Default Action
*
*/
GT_VOID prvTgfSerialExactMatchBasicDefaultActionTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchBasicDefaultActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Serial Exact Match
*          Default Action
*/
GT_VOID prvTgfSerialExactMatchBasicDefaultActionTtiConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchBasicExpandedActionTtiConfigSet function
* @endinternal
*
* @brief Set TTI test configuration related to Serial Exact Match Expanded Action
*
*/
GT_VOID prvTgfSerialExactMatchBasicExpandedActionTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchBasicExpandedActionTtiConfigRestore function
* @endinternal
*
* @brief   Restore TTI test configuration related to Serial Exact Match
*          Expanded Action
*/
GT_VOID prvTgfSerialExactMatchBasicExpandedActionTtiConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchInvalidateEmEntry function
* @endinternal
*
* @brief   Invalidate Exact Match Entry
*/
GT_VOID prvTgfSerialExactMatchInvalidateEmEntry
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchKeyAndActionConfigSet function
* @endinternal
*
* @brief Serial Exact Match Configurations for Serial-EM0/EM1/EM2 handing
*
*/
GT_VOID prvTgfSerialExactMatchKeyAndActionConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfSerialExactMatchKeyAndActionConfigRestore function
* @endinternal
*
* @brief Restore Serial Exact Match Configurations
*
*/
GT_VOID prvTgfSerialExactMatchKeyAndActionConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvTgfSerialExactMatchh */


