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
* @file prvTgfStreamSgcQosProfileUpdate.h
*
* @brief SGC Gate QoS profile update use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamSgcQosProfileUpdateh
#define __prvTgfStreamSgcQosProfileUpdateh

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
* internal prvTgfStreamSgcQosProfileUpdateConfigSet function
* @endinternal
*
* @brief   Stream Gate Control gate QoS profile update use case configurations
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcQosProfileUpdateTraffic function
* @endinternal
*
* @brief   Stream Gate Control gate QoS profile update use case traffic.
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateTraffic
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcQosProfileUpdateVerification function
* @endinternal
*
* @brief   Stream Gate Control gate QoS profile update use case verification.
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateVerification
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcQosProfileUpdateConfigRestore function
* @endinternal
*
* @brief   Stream Gate Control gate QoS profile update configurations restore.
*/
GT_VOID prvTgfStreamSgcQosProfileUpdateConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamSgcQosProfileUpdateh */




