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
* @file prvTgfStreamSgcExceptions.h
*
* @brief Stream Gate Control exception use cases testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamSgcExceptionsh
#define __prvTgfStreamSgcExceptionsh

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
 * @enum STREAM_SGC_EXCEPTION_TESTS_ENT
 *
 * @brief Stream gate state
 */
typedef enum{

    /** @brief :Gate Interval Max Octet Exceeded exception test
    */
    STREAM_SGC_EXCEPTION_TESTS_GATE_INTERVAL_MAX_OCTET_EXCEEDED_E,

    /** @brief :Gate Closed exception test
    */
    STREAM_SGC_EXCEPTION_TESTS_GATE_CLOSED_E,

    /** @brief :Gate byte count adjust test
    */
    STREAM_SGC_EXCEPTION_TESTS_GATE_BYTE_COUNT_ADJUST_E

} STREAM_SGC_EXCEPTION_TESTS_ENT;


/**
* internal prvTgfStreamSgcExceptionsConfigSet function
* @endinternal
*
* @brief   Stream Gate Control exceptions use case configurations
*/
GT_VOID prvTgfStreamSgcExceptionsConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcExceptionsTraffic function
* @endinternal
*
* @brief   Stream Gate Control exceptions use case traffic.
*/
GT_VOID prvTgfStreamSgcExceptionsTraffic
(
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType
);

/**
* @internal prvTgfStreamSgcExceptionsVerification function
* @endinternal
*
* @brief   Stream Gate Control exceptions use case verification.
*/
GT_VOID prvTgfStreamSgcExceptionsVerification
(
    STREAM_SGC_EXCEPTION_TESTS_ENT    testType
);

/**
* @internal prvTgfStreamSgcExceptionsConfigRestore function
* @endinternal
*
* @brief   Stream Gate Control exceptions configurations restore.
*/
GT_VOID prvTgfStreamSgcExceptionsConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamSgcExceptionsh */




