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
* @file prvTgfStreamQch.h
*
* @brief 802.1Qch Cyclic Queuing and Forwarding feature use case testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamQchh
#define __prvTgfStreamQchh

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
* internal prvTgfStreamQchConfigSet function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding feature use case configurations
*/
GT_VOID prvTgfStreamQchConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfStreamQchTraffic function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding feature use case traffic.
*/
GT_VOID prvTgfStreamQchTraffic
(
    GT_VOID
);

/**
* @internal prvTgfStreamQchVerification function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding feature use case verification.
*/
GT_VOID prvTgfStreamQchVerification
(
    GT_VOID
);

/**
* @internal prvTgfStreamQchConfigRestore function
* @endinternal
*
* @brief   802.1Qch Cyclic Queuing and Forwarding feature configurations restore.
*/
GT_VOID prvTgfStreamQchConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamQchh */




