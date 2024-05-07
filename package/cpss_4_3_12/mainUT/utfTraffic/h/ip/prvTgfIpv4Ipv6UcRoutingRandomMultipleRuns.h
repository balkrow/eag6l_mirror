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
* @file prvTgfIpv4Ipv6UcRoutingRandomMultipleRuns.h
*
* @brief IPV4 & IPV6 UC Routing when filling the Lpm and using many Add,
* Delete and flush operations.
*
* @version   1
********************************************************************************
*/
#ifndef __prvTgfIpv4Ipv6UcRandomMultipleRunsh
#define __prvTgfIpv4Ipv6UcRandomMultipleRunsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/******************************************************************************\
 *                            Test implementation                             *
\******************************************************************************/

/**
* @internal prvTgfIpLpmIpv4Ipv6UcPrefixAddMaxPrefix function
* @endinternal
*
* @brief  Flush all UC prefixes according to protocol
*
* @param[in] defragmentationEnable    - whether to enable performance costing de-fragmentation process
*                                      in the case that there is no place to insert the prefix.
*                                      To point of the process is just to make space for this prefix.
*                                      (For DEVICES: xCat3; AC5; Lion2:
*                                      relevant only if the LPM DB was created with partitionEnable = GT_FALSE)
* @param[out] addedNumIpv4Ptr  - (pointer to) the max number of ipv4 UC prefixes added
* @param[out] addedNumIpv6Ptr  - (pointer to) the max number of ipv6 UC prefixes added

*/
GT_STATUS prvTgfIpLpmIpv4Ipv6UcPrefixAddMaxPrefix
(
    GT_BOOL defragmentationEnable,
    GT_U32 *maxAddedNumIpv4Ptr,
    GT_U32 *maxAddedNumIpv6Ptr

);

/**
* @internal prvTgfIpLpmFlushUcPrefixes function
* @endinternal
*
* @brief  Flush all UC prefixes according to protocol
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
*/
GT_STATUS prvTgfIpLpmFlushUcPrefixes
(
    GT_BOOL isIpv6
);
/**
* @internal prvTgfIpLpmAddUcPrefixes function
* @endinternal
*
* @brief  Add UC prefixes according to parameters
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*                                      (For DEVICES: xCat3; AC5; Lion2: relevant only if
*                                      the LPM DB was created with partitionEnable = GT_FALSE)
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddUcPrefixes
(
    GT_BOOL is_ipv6,
    GT_BOOL defragmentationEnable,
    GT_U32 numOfPrefixesToAdd
);

GT_U32  prvTgfIpLpmUtilShiftLFSR19b(GT_U32 v);
GT_U32  prvTgfIpLpmUtilShiftLFSR10b(GT_U32 v);
GT_VOID prvTgfIpLpmUtilRibPrefixLenDbBuild(GT_BOOL is_ipv6, GT_U32 *prefix_len_db);

/**
* @internal prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulk function
* @endinternal
*
* @brief  Add ipv6 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulk(
   IN  GT_U32 ulPrefixBitmap,
   IN  GT_U32 numOfPrefixesToAdd
);
/**
* @internal prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulkDel function
* @endinternal
*
* @brief  Delete ipv6 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToDel - number of prefixes to deleted
*
*/
GT_STATUS prvTgfIpLpmAddIpv6UcEntriesbyPrefixStreamIncrBulkDel(
   IN GT_U32 ulPrefixBitmap,
   IN GT_U32 numOfPrefixesToDel
);

/**
* @internal prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulk function
* @endinternal
*
* @brief  Add ipv4 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulk(
   IN  GT_U32 ulPrefixBitmap,
   IN  GT_U32 numOfPrefixesToAdd
);
/**
* @internal prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulkDel function
* @endinternal
*
* @brief  Delete ipv4 UC prefixes according to parameters using bulk
*
* @param[in] ulPrefixBitmap - flag for different bulk parameters
* @param[in] numOfPrefixesToDel - number of prefixes to deleted
*
*/
GT_STATUS prvTgfIpLpmAddIpv4UcEntriesbyPrefixStreamIncrBulkDel(
   IN GT_U32 ulPrefixBitmap,
   IN GT_U32 numOfPrefixesToDel
);

/**
* @internal prvTgfIpv4Ipv6UcMcValidityCheck function
* @endinternal
*
* @brief   Check validity for LTT Route Configuration
*
*/
GT_VOID prvTgfIpv4Ipv6UcMcValidityCheck
(
    GT_VOID
);

/**
* @internal prvTgfIpLpmFlushMcPrefixes function
* @endinternal
*
* @brief  Flush all MC prefixes according to protocol
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
*/
GT_STATUS prvTgfIpLpmFlushMcPrefixes
(
    GT_BOOL isIpv6
);

/**
* @internal prvTgfIpLpmAddMcPrefixes function
* @endinternal
*
* @brief  Add MC prefixes according to parameters
*
* @param[in] isIpv6   - GT_TRUE: ipv6 protocol
*                       GT_FALSE: ipv4 protocol
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*                                      (For DEVICES: xCat3; AC5; Lion2: relevant only if
*                                      the LPM DB was created with partitionEnable = GT_FALSE)
* @param[in] numOfPrefixesToAdd - number of prefixes to add
*
*/
GT_STATUS prvTgfIpLpmAddMcPrefixes
(
    GT_BOOL is_ipv6,
    GT_BOOL defragmentationEnable,
    GT_U32 numOfPrefixesToAdd
);

/**
* @internal prvTgfIpLpmIpv4Ipv6McPrefixAddMaxPrefix function
* @endinternal
*
* @brief  Flush all MC prefixes according to protocol
*
* @param[in] defragmentationEnable    - whether to enable
*                                      performance costing
*                                      de-fragmentation process
*                                      in the case that there is
*                                      no place to insert the
*                                      prefix. To point of the
*                                      process is just to make
*                                      space for this prefix.
*                                      (For DEVICES: xCat3; AC5;
*                                      Lion2: relevant only if
*                                      the LPM DB was created
*                                      with partitionEnable =
*                                      GT_FALSE)
* @param[out] addedNumIpv4Ptr  - (pointer to) the max number of ipv4 MC prefixes added
* @param[out] addedNumIpv6Ptr  - (pointer to) the max number of ipv6 MC prefixes added

*/
GT_STATUS prvTgfIpLpmIpv4Ipv6McPrefixAddMaxPrefix
(
    GT_BOOL defragmentationEnable,
    GT_U32 *maxAddedNumIpv4Ptr,
    GT_U32 *maxAddedNumIpv6Ptr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfIpv4Ipv6UcRandomMultipleRunsh */


