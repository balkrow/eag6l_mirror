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
* @file prvTgfIpvxUcRoutingNonExistedVrf.h
*
* @brief Policy and LPM based IPV4 UC Routing.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpvxUcRoutingNonExistedVrfh
#define __prvTgfIpvxUcRoutingNonExistedVrfh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssHwInit/cpssHwInit.h>
#include <common/tgfLpmGen.h>
#include <common/tgfIpGen.h>
#include <common/tgfBridgeGen.h>

/**
* @internal prvTgfIpvxUcRoutingNonExistedVrfBaseConfigSet function
* @endinternal
*
* @brief   Set Base Configuration
*
* @param[in] prvUtfVrfId    - virtual router ID
*
*/
GT_VOID prvTgfIpvxUcRoutingNonExistedVrfConfigSet
(
    GT_U32  prvUtfVrfId
);

/**
* @internal prvTgfIpvxUcRoutingNonExistedVrfPclConfigSet function
* @endinternal
*
* @brief   Set PCL configuration
*/
GT_VOID prvTgfIpvxUcRoutingNonExistedVrfPclConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfIpvxUcRoutingNonExistedVrfIpConfigSet function
* @endinternal
*
* @brief   Set IP Configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*/
GT_VOID prvTgfIpvxUcRoutingNonExistedVrfIpConfigSet
(
    GT_U32      prvUtfVrfId
);

/**
* @internal prvTgfIpvxUcRoutingNonExistedVrfPclConfigSet function
* @endinternal
*
* @brief   Set test PCL configuration
*/
GT_VOID prvTgfIpvxUcRoutingNonExistedVrfPclConfigSet
(
    GT_VOID
);

/**
* @internal prvTgfIpvxUcRoutingNonExistedVrfTrafficGenerate function
* @endinternal
*
* @brief   Generate traffic
*
* @param[in] vlanId - vlan Id
*/
GT_VOID prvTgfIpvxUcRoutingNonExistedVrfTrafficGenerate
(
    GT_BOOL isIpv4
);

/**
* @internal prvTgfIpvxUcRoutingNonExistedVrfConfigRestore function
* @endinternal
*
* @brief   Restore configuration
*
* @param[in] prvUtfVrfId              - virtual router index
*
*/
GT_VOID prvTgfIpvxUcRoutingNonExistedVrfConfigRestore
(
    GT_U32   prvUtfVrfId
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpvxUcRoutingNonExistedVrfh */


