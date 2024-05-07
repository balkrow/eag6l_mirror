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
* @file prvTgfBrgNestedDsaToAccess.h
********************************************************************************
*/
#ifndef __prvTgfBrgNestedDsaToAccess
#define __prvTgfBrgNestedDsaToAccess

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvTgfBrgNestedDsaToAccessConfig function
* @endinternal
*
* @brief   Config the ingress port as cascade and enable nested vlan.
*
*/
GT_VOID prvTgfBrgNestedDsaToAccessConfig();
/**
* @internal prvTgfBrgNestedDsaToAccessConfigRestore function
* @endinternal
*
* @brief  Restore the cascade and nested vlan config for the ingress port
*         and other cleanup
*
*/
GT_VOID prvTgfBrgNestedDsaToAccessConfigRestore();
/**
* @internal prvTgfBrgNestedDsaToAccessTrafficSend function
* @endinternal
*
* @brief  Traffic test for eDSA traffic over nested vlan port
*           For different vlan tag types setting in the eDSA tag for ingress port,
*           check the tag state of the egress access port
*/
GT_VOID prvTgfBrgNestedDsaToAccessTrafficSend();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif



