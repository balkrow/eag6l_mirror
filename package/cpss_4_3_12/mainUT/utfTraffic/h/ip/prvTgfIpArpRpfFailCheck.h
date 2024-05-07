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
* @file prvTgfIpArpRpfFailCheck.h
*
* @brief Unicast RPF Fail Check
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpArpRpfFailCheckh
#define __prvTgfIpArpRpfFailCheckh

#include <utf/utfMain.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfIpArpRpfFailCheckConfig function
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
*         12. Save UC RPF Fail check service state
*         13. Enable UC RPF Fail check service
*/
GT_VOID prvTgfIpArpRpfFailCheckConfig
(
    GT_VOID
);

/**
* @internal prvTgfIpArpRpfFailCheckGenerate function
* @endinternal
*
* @brief   AUTODOC: Perform the test:
*         UC RPF Fail check enabled, nexthopvlanID == packet vlan - packet passed
*         UC RPF Fail check enabled, nexthopvlanID != packet vlan  - packet dropped
*         UC RPF Fail check disabled, nexthopvlanID == packet vlan - packet passed
*         UC RPF Fail check disabled, nexthopvlanID != packet vlan - packet passed
*/
GT_VOID prvTgfIpArpRpfFailCheckGenerate
(
    GT_VOID
);

/**
* @internal prvTgfIpArpRpfFailCheckRestore function
* @endinternal
*
* @brief   AUTODOC: Restore previous configuration
*         1. Restore UC RPF Fail check state
*         2. Delete prefix rules
*         3. Restore route entries
*         4. Restore ARP entries
*         5. Flush FDB
*         6. Disable routing in sender VLAN
*         7. Restore routing enabled state for sender port
*         8. Invalidate VLANs
*/
GT_VOID prvTgfIpArpRpfFailCheckRestore
(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpArpRpfFailCheckh */


