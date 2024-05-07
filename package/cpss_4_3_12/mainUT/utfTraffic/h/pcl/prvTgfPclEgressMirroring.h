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
* @file prvTgfPclEgressMirroring.h
*
* @brief Pcl Egress Packet cmd Mirroring tests
*
* @version   10
********************************************************************************
*/
#ifndef __prvTgfPclEgressMirroringh
#define __prvTgfPclEgressMirroringh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfPclGen.h>

GT_VOID prvTgfPclEgressMirroringCfgSet
(
    IN GT_BOOL actionLatencyMonitor,
    IN GT_BOOL actionTxMirror,
    IN GT_BOOL notActionTxMirror
);

GT_VOID prvTgfPclEgressMirroringLMClear
(
    GT_VOID
);

GT_VOID prvTgfPclEgressMirroringLMCheck
(
    GT_BOOL notZeroLM_egressPort,
    GT_BOOL notZeroLM_egressAnalyserPort
);

GT_VOID prvTgfPclEgressMirroringTrafficGenerate
(
    GT_VOID
);

GT_VOID prvTgfPclEgressMirroringCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfEpclReTrapPacketConfig function
* @endinternal
*
* @brief Configuration for EPCL ReTrap packet test
*
*/
GT_VOID prvTgfEpclReTrapPacketConfig
(
    CPSS_DXCH_PHA_RE_TRAP_MODE_ENT    reTrapMode
);

/**
* @internal prvTgfEpclReTrapPacketTrafficGenerate function
* @endinternal
*
* @brief Traffic generate and verify for EPCL ReTrap test
*
*/
GT_VOID prvTgfEpclReTrapPacketTrafficGenerate
(
    CPSS_DXCH_PHA_RE_TRAP_MODE_ENT    reTrapMode
);

/**
* @internal prvTgfEpclReTrapPacketRestore function
* @endinternal
*
* @brief Restore for EPCL ReTrap test
*
*/
GT_VOID prvTgfEpclReTrapPacketRestore
(
    GT_VOID
);

/**
* @internal prvTgfEpclDropPacketToErepTrapTest function
* @endinternal
*
* @brief Configuration for EPCL Drop Erep Trap packet test
*
*/
GT_VOID prvTgfEpclDropPacketToErepTrapTest
(
    GT_VOID
);

/**
* @internal prvTgfEpclDropPacketToErepTrapTrafficGenerate function
* @endinternal
*
* @brief Traffic generate and verify for EPCL Drp Erep Trap test
*
*/
GT_VOID prvTgfEpclDropPacketToErepTrapTrafficGenerate
(
    GT_VOID
);

/**
* @internal prvTgfEpclDropPacketToErepTrapTestRestore function
* @endinternal
*
* @brief Restore for EPCL Drop Erep Trap test
*
*/
GT_VOID prvTgfEpclDropPacketToErepTrapTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfPclEgressMirroringNotPclLmCfgSet function
* @endinternal
*
* @brief  Configure Egress mirroring and Latency measurment by 
*   mirror and LM API without PCL rule.
*/
GT_VOID prvTgfPclEgressMirroringNotPclLmCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPclEgressMirroringCfgSetWithLM function
* @endinternal
*
* @brief  Restore Egress mirroring and Latency measurment by 
*   mirror and LM API without PCL rule.
*/
GT_VOID prvTgfPclEgressMirroringNotPclLmCfgRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
