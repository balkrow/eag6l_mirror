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
* @file prvTgfStreamSgcGateTimeSlotsCheck.h
*
* @brief SGC gate time slots functionality testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamSgcGateTimeSlotsCheckh
#define __prvTgfStreamSgcGateTimeSlotsCheckh

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
* internal prvTgfStreamSgcGateTimeSlotsCheckConfigSet function
* @endinternal
*
* @brief   SGC gate time slots configurations
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcGateTimeSlotsCheckTraffic function
* @endinternal
*
* @brief   SGC gate time slots traffic.
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckTraffic
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcGateTimeSlotsCheckVerification function
* @endinternal
*
* @brief   SGC gate time slots verification.
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckVerification
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcGateTimeSlotsCheckConfigRestore function
* @endinternal
*
* @brief   SGC gate time slots configurations restore.
*/
GT_VOID prvTgfStreamSgcGateTimeSlotsCheckConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamSgcGateTimeSlotsCheckh */




