/*******************************************************************************
*              (c), Copyright 2016, Marvell International Ltd.                 *
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
* @file prvTgfIpv6HeaderFailCheck.h
*
* @brief Unicast IP Header Check in bridge services
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv6HeaderFailCheckh
#define __prvTgfIpv6HeaderFailCheckh

#include <utf/utfMain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpv6HeaderFailCheckConfig function
* @endinternal
*
* @brief   AUTODOC: Set Configuration:
*         1. Create two VLANs SENDER and NEXTHOP
*         2. Save unicast routing state for sender port
*         3. Enable unicast routing on sender port
*         4. Assign virtual router to sender VLAN
*         5. Enable unicast routing in sender VLAN
*         6. Create FDB entries for DA/SA MACs
*         7. Save ARP table entries
*         8. Write ARP table entries for DA/SA MACs
*         9. Save route entries
*         10. Set up route entries to route packets between two VLANs
*         11. Add prefix rules for SIP and DIP which use the route entries
*         12. Save IP Header Fail check service state
*         13. Enable IP Header Fail check service
*/
GT_VOID prvTgfIpv6HeaderFailCheckConfig
(
    GT_VOID
);


/**
* @internal prvTgfIpv6HeaderCheckGenerate function
* @endinternal
*
* @brief   AUTODOC: Perform the test:
*         Ip Header check enabled,header correct - packet passed
*         Ip Header check enabled,incorrect header - packet dropped
*         Ip Header check disabled,header correct - packet passed
*         Ip Header check disabled,incorrect header- packet passed
*/
GT_VOID prvTgfIpv6HeaderFailCheckGenerate
(
    GT_VOID

);

/**
* @internal prvTgfIpv6HeaderFailCheckRestore function
* @endinternal
*
* @brief   AUTODOC: Restore previous configuration
*         1. Restore IP Header Fail check state
*         2. Delete prefix rules
*         3. Restore route entries
*         4. Restore ARP entries
*         5. Flush FDB
*         6. Disable routing in sender VLAN
*         7. Restore routing enabled state for sender port
*         8. Invalidate VLANs
*/
GT_VOID prvTgfIpv6HeaderFailCheckRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv6HeaderFailCheckh */


