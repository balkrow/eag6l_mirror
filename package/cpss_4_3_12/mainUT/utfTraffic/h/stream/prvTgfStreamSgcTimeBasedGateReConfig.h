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
* @file prvTgfStreamSgcTimeBasedGateReConfig.h
*
* @brief SGC time based gate re-configuration functionality testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamSgcTimeBasedGateReConfigh
#define __prvTgfStreamSgcTimeBasedGateReConfigh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <common/tgfTunnelGen.h>

#define PRV_UTF_LOG_VERIFY_GT_OK(rc, name)                                   \
    if (GT_OK != (rc))                                                       \
    {                                                                        \
        PRV_UTF_LOG2_MAC("\n[TGF]: %s FAILED, rc = [%d]\n", name, rc);       \
    }


/**
* internal prvTgfStreamSgcTimeBasedGateReConfigConfigSet function
* @endinternal
*
* @brief   SGC time based gate re-configuration configurations
*/
GT_VOID prvTgfStreamSgcTimeBasedGateReConfigConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcTimeBasedGateReConfigTraffic function
* @endinternal
*
* @brief   SGC time based gate re-configuration traffic.
*/
GT_VOID prvTgfStreamSgcTimeBasedGateReConfigTraffic
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcTimeBasedGateReConfigVerification function
* @endinternal
*
* @brief   SGC time based gate re-configuration verification.
*/
GT_VOID prvTgfStreamSgcTimeBasedGateReConfigVerification
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcTimeBasedGateReConfigConfigRestore function
* @endinternal
*
* @brief   SGC time based gate re-configuration configurations restore.
*/
GT_VOID prvTgfStreamSgcTimeBasedGateReConfigConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamSgcTimeBasedGateReConfigh */




