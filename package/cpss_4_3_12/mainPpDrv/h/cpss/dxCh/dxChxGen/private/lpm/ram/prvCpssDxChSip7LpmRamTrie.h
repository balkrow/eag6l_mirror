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
* @file prvCpssDxChSip7LpmRamTrie.h
*
* @brief This file includes implementation of a binary trie, which is used for
* keeping the history of the insertions to the MC tree structure and to
* LPM buckets (for UC and MC).
* The pData field in the trie node structure will include
* pointers to the following structures:
*
* 1. When used for UC LPM purpose:
* - pData holds a pointer to the route entry pointer inserted into the
* LPM structure.
*
* 2. When used for MC LPM purpose:
* - pData holds a pointer to the route entry pointer inserted into the
* LPM structure.
*
* 3. When used for MC Tree purpose:
* - pData holds mc src bucket which holds a pointer to the
* shadow bucket in CPU memory holding the source prefixes and the
* HW address in PP's RAM of the source LPM structure.
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChSip7LpmRamTrieh
#define __prvCpssDxChSip7LpmRamTrieh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/extServices/os/gtOs/gtGenTypes.h>

/**
* @internal prvCpssDxChLpmTrieInsertSip7 function
* @endinternal
*
* @brief   Insert a new NEXT_POINTER_PTR to the trie structure.
*
* @note   APPLICABLE DEVICES:       AAS.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] rootPtr                  - Root of the trie to insert the entry.
* @param[in] addrPtr                  - address associated with the given entry.
* @param[in] prefix                   -  of addrPtr.
* @param[in] trieDepth                - The maximum depth of the trie.
* @param[in] entry                    - The  to be inserted.
*
* @retval GT_OK                    - if succeeded
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate memory
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmTrieInsertSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC  *rootPtr,
    IN GT_U8                                *addrPtr,
    IN GT_U32                               prefix,
    IN GT_U32                               trieDepth,
    IN GT_PTR                               entry
);

/*******************************************************************************
* prvCpssDxChLpmTrieSearchSip7
*
* DESCRIPTION:
*       Search for the entry which is associated with address & prefix .
*
* @note   APPLICABLE DEVICES:       AAS.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* INPUTS:
*       root    - Root of the trie to search for the entry.
*       addr    - the address associated with the given entry.
*       prefix  - prefix of addr.
*       trieDepth - The maximum depth of the trie.
*       entry   - Indicates whether to return the entry in the searched node
*                 or not.
*
* OUTPUTS:
*       entry   - A pointer to the searched entry if found, NULL otherwise.
*
* RETURNS:
*       A pointer to the node in the trie including the searched entry.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC* prvCpssDxChLpmTrieSearchSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC  *root,
    IN GT_U8                                *addr,
    IN GT_U32                               prefix,
    IN GT_U32                               trieDepth,
    INOUT GT_PTR                            *entry
);

/**
* @internal prvCpssDxChLpmTrieDelSip7 function
* @endinternal
*
* @brief   Deletes a previously inserted entry.
*
* @note   APPLICABLE DEVICES:       AAS.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] rootPtr                  - Root of the trie to delete from.
* @param[in] addrPtr                  - the address associated with the entry to be deleted.
* @param[in] prefix                   - the  of the address.
* @param[in] trieDepth                - The maximum depth of the trie.
*
* @param[out] entryPtr                 - A pointer to the entry of the deleted node if found, NULL
*                                      otherwise.
*
* @retval GT_OK                    - if succeeded
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note This function does not free the pointers to the data stored in the
*       trie node, it's the responsibility of the caller to do that.
*
*/
GT_STATUS prvCpssDxChLpmTrieDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC  *rootPtr,
    IN GT_U8                                *addrPtr,
    IN GT_U32                               prefix,
    IN GT_U32                               trieDepth,
    OUT GT_PTR                              *entryPtr
);

/**
* @internal prvCpssDxChLpmTrieGetNextSip7 function
* @endinternal
*
* @brief   This function searchs the trie for the next valid node. The search is
*         started from the given address & prefix.
*
* @note   APPLICABLE DEVICES:       AAS.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] rootPtr                  - Root of the trie to be searched.
* @param[in] trieDepth                - The maximum depth of the trie.
* @param[in] force                    - Indicates whether to find the next valid entry even if the
*                                      given address,prefix are not valid.
* @param[in,out] addrPtr                  - the address associated with the search start point.
* @param[in,out] prefixPtr                - prefix of ipAddr.
* @param[in,out] addrPtr                  - The address associated with the next node.
* @param[in,out] prefixPtr                - The prefix associated with ipAddr.
*
* @param[out] entryPtr                 - A pointer to the entry if found, NULL otherwise.
*                                       GT_OK if succeeded, GT_FAIL otherwise.
*
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. If the given node is the last node in the trie then the values of
*       ipAddr and prefix will not be changed, and entry will get NULL.
*       2. If force == GT_TRUE and (ipAddr,ipPrefix) is invalid, then this
*       function will find the first valid node.
*
*/
GT_STATUS prvCpssDxChLpmTrieGetNextSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_TRIE_NODE_STC  *rootPtr,
    IN GT_U32                               trieDepth,
    IN GT_BOOL                              force,
    INOUT GT_U8                             *addrPtr,
    INOUT GT_U32                            *prefixPtr,
    OUT GT_PTR                              *entryPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip7LpmRamTrieh */


