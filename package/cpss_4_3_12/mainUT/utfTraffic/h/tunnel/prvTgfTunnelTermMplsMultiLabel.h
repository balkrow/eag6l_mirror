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
* @file prvTgfTunnelTermMplsMultiLabel.h
*
* @brief Test for popping 6 MPLS labels
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfTunnelTermMplsMultiLabelh
#define __prvTgfTunnelTermMplsMultiLabelh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfTunnelTermMplsMultiLabelBridgeConfigSet function
* @endinternal
*
* @brief   Set Bridge Configuration
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelBridgeConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMplsMultiLabelTtiConfigSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMplsMultiLabelTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermMplsMultiLabelConfigRestore function
* @endinternal
*
* @brief   Restore Bridge and TTI configurations
*/
GT_VOID prvTgfTunnelTermMplsMultiLabelConfigRestore
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermImplicitMplsPop1LabelTtiConfigSet function
* @endinternal
*
* @brief   Set TTI configurations
*/
GT_VOID prvTgfTunnelTermImplicitMplsPop1LabelTtiConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelTermImplicitMplsPop1LabelTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelTermImplicitMplsPop1LabelTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelMplsCutThroughConfigSet function
* @endinternal
*
* @brief   Set TTI Configuration
*/
GT_VOID prvTgfTunnelMplsCutThroughConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfTunnelMplsCutThroughTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*/
GT_VOID prvTgfTunnelMplsCutThroughTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfTunnelMplsCutThroughConfigRestore function
* @endinternal
*
* @brief   Restore Config
*/
GT_VOID prvTgfTunnelMplsCutThroughConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfTunnelTermMplsMultiLabelh */


