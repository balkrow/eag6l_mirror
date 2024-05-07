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
* @file prvTgfStreamSgcMaxSduSizeProfileCheck.h
*
* @brief Stream Gate Control maximum SDU size profile check use cases testing
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamSgcMaxSduSizeProfileCheckh
#define __prvTgfStreamSgcMaxSduSizeProfileCheckh

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
* internal prvTgfStreamSgcMaxSduSizeProfileCheckConfigSet function
* @endinternal
*
* @param[in] direction - PCL direction
*
* @brief   Stream Gate Control maximum SDU size profile
*          check use case configurations
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckConfigSet
(
    IN GT_U32 direction
);

/**
* @internal prvTgfStreamSgcMaxSduSizeProfileCheckTraffic function
* @endinternal
*
* @brief   Stream Gate Control maximum SDU size profile check use case traffic.
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckTraffic
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcMaxSduSizeProfileCheckVerification function
* @endinternal
*
* @brief   Stream Gate Control maximum SDU size profile check use case verification.
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckVerification
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcMaxSduSizeProfileCheckConfigRestore function
* @endinternal
*
* @param[in] direction - PCL direction
*
* @brief   Stream Gate Control maximum SDU size profile check configurations restore.
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckConfigRestore
(
    IN GT_U32 direction
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamSgcMaxSduSizeProfileCheckh */




