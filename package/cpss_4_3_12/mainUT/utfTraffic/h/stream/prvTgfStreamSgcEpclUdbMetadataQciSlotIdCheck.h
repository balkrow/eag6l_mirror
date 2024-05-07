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
* @file prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheck.h
*
* @brief Egress mirroring of Qci stream with time slot added to the mirrored copy
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckh
#define __prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckh

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
* internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigSet
* function
* @endinternal
*
* @brief   EPCL UDB Metadata Qci Slot Id Check configurations
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckTraffic function
* @endinternal
*
* @brief   EPCL UDB Metadata Qci Slot Id Check traffic.
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckTraffic
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckVerification function
* @endinternal
*
* @brief   EPCL UDB Metadata Qci Slot Id Check verification.
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckVerification
(
    GT_VOID
);

/**
* @internal prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigRestore function
* @endinternal
*
* @brief   EPCL UDB Metadata Qci Slot Id Check configurations restore.
*/
GT_VOID prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckConfigRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfStreamSgcEpclUdbMetadataQciSlotIdCheckh */




