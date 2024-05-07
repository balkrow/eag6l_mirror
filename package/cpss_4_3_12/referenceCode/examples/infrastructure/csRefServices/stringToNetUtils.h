/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* stringToNetUtils.h
*
* DESCRIPTION:
*  Convert presentation-level(string) Mac/IPv6/IPv4 address to network order
*  binary form.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef __stringToNetUtils_h
#define __stringToNetUtils_h

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */


#include <cpss/generic/cpssTypes.h>

typedef enum{
  INET_ADDRESS_IPV4,
  INET_ADDRESS_IPV6,
  INET_ADDRESS_MAC
}INET_ADDRESS_TYPE;


/**
* @internal csRefInetStringToNet function
* @endinternal
*
* @brief   This function convert presentation-level(string)
*          Mac/IPv6/IPv4 address to network order binary form.
*
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  addressTypeaf - type of address to convert MAC, IPV4 or IPv6.
* @param[in] src            - String that contains the address to convert.
                              Address format should be according to standard
* @param[out] dst           - Array to store the address's binary format.
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS csRefInetStringToNet
(
  IN  INET_ADDRESS_TYPE addressTypeaf,
  IN  char             *src,
  OUT GT_U8            *dst
);

/**
* @internal csRefIncAddress function
* @endinternal
*
* @brief   This function increment Mac/IPv6/IPv4 addrress by one.
*
*
* @note   APPLICABLE DEVICES:  All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in]  addressType - type of address to convert MAC, IPV4 or IPv6.
* @param[in]  netAddress  - Array of bytes contains the address to convert.
                            Address format is network order.
*
*
* @retval GT_OK                     - on success
* @retval GT_FAIL                  - otherwise
*
* @note
*/
GT_STATUS csRefIncAddress
(
  IN     INET_ADDRESS_TYPE addressType,
  INOUT  GT_U8            *netAddress,
  IN     GT_U32            prefix
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPolicerh */

