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
* @file prvCpssDxChSip7LpmRamDbg.h
*
* @brief Private CPSS DXCH LPM RAM debug functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChSip7LpmRamDbgh
#define __prvCpssDxChSip7LpmRamDbgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

/**
* @internal prvCpssDxChLpmRamDbgDumpSip7 function
* @endinternal
*
* @brief   This function is intended to do hardware LPM dump
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgDumpSip7
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType
);

/**
* @internal prvCpssDxChLpmRamDbgHwValidationSip7 function
* @endinternal
*
* @brief   This function is intended to do hardware LPM validation
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - physical device number.
* @param[in] vrId                     - virtual router ID
* @param[in] protocol                 - the protocol
* @param[in] prefixType               - the prefix type (Unicast or Multicast)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong devNum or vrId.
* @retval GT_HW_ERROR              - on Hardware error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       For every LPM LINE
*       Struct type (bits 129-131)
*       if bucket type = 0 Regular --> check childPointerand bit_vector 0-43 legality
*       if bucket type = 1 Embedded 1_Leaf --> check childPointer, childType/offset and defaultLeaf legality
*       if bucket type = 2 Embedded 2_Leaf --> check childPointer, childType/offset, defaultLeaf and leaf legality
*       if bucket type = 3 Embedded 3_Leaf --> check offset1/offset2 and leaf_0/leaf_1/leaf_2 legality
*       if bucket type = 4 Compress --> check childPointer and childType/offset legality
*       if bucket type = 5 not_supported
*       if bucket type = 6 Last_Skip_Node_Type --> TBD: check match_Leaf, deafault_leaf and match_prefix legality
*       if bucket type = 7 Skip_Node_Type --> TBD: check match_Leaf, deafault_leaf and match_prefix legality
*
*       -----------------
*       IPv4 UC
*       Max tree depth is 4 so from depth 4 all LPM lines must be NH or ECMP/QoS bucket type
*       Illegal to point to a source tree.
*       IPv6 UC
*       Same but depth is 16
*       IPv4 MC SRC
*       Max MC group depth is 4 and each group points to root of sources tree (with max depth of 4). So total max depth is 8.
*       Illegal to point to another source tree.
*       If you reach depth 8, then all LPM lines must be pointing to NH or ECMP/QoS.
*       IPv6 MC
*       Same but 8 instead of 4 and 16 instead of 8
*
*/
GT_STATUS prvCpssDxChLpmRamDbgHwValidationSip7
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_UNICAST_MULTICAST_ENT              prefixType
);

/**
* @internal prvCpssDxChLpmRamDbgDbHwMemPrintSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function print LPM debug information
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr              - The LPM DB.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgDbHwMemPrintSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmRamDbgShadowValidityCheckSip7 function
* @endinternal
*
* @brief   Validation function for the LPM shadow
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - LPM DB
* @param[in] vrId                     - virtual router id, 4096 means "all vrIds"
* @param[in] protocolBitmap           - protocols bitmap
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note IPv6 MC validation is not implemented yet
*
*/
GT_STATUS prvCpssDxChLpmRamDbgShadowValidityCheckSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP           protocolBitmap,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  returnOnFailure
);

/**
* @internal prvCpssDxChLpmRamDbgHwShadowSyncValidityCheckSip7 function
* @endinternal
*
* @brief   This function validates synchronization between the SW and HW of the LPM
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbPtr                 - LPM DB
* @param[in] vrId                     - virtual router id, 256 means "all vrIds"
* @param[in] protocolBitmap           - protocols bitmap
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgHwShadowSyncValidityCheckSip7
(
    IN GT_U8                                    devNum,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP           protocolBitmap,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN GT_BOOL                                  returnOnFailure
);

/**
* @internal prvCpssDxChLpmRamDbgHwOctetsToBlockMappingInfoPrintSip7 function
* @endinternal
*
* @brief   Print Octet to Block mapping debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
* @retval GT_NOT_FOUND             - if can't find the lpm DB
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamDbgHwOctetsToBlockMappingInfoPrintSip7
(
    IN GT_U32                           lpmDbId
);

/**
* @internal prvCpssDxChLpmDbgHwBlockInfoPrintSip7 function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
GT_VOID prvCpssDxChLpmDbgHwBlockInfoPrintSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerBlockPrintSip7 function
* @endinternal
*
* @brief   Print Octet per Block debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerBlockPrintSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal prvCpssDxChIpLpmDbgLastNeededMemInfoPrintSip7 function
* @endinternal
*
* @brief   Print Needed memory information for last prefix addition
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr  - (pointer to ) LPM shadow db
*
*/
GT_VOID prvCpssDxChIpLpmDbgLastNeededMemInfoPrintSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC  *shadowPtr
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrintSip7 function
* @endinternal
*
* @brief   Print Octet per Protocol debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrintSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

/**
* @internal prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCountersSip7 function
* @endinternal
*
* @brief   Print Octet per Protocol LPM lines debug information for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*/
GT_VOID prvCpssDxChLpmDbgHwOctetPerProtocolPrintLpmLinesCountersSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr
);

GT_U32 getBaseAddrFromHWSip7(CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT type,
                         GT_U32 index,
                         GT_U32 *hwData,GT_U32 bucketPtr);
GT_STATUS validateBitVectorOfRegularBucketSip7(GT_U8 devNum, GT_U32 *bucketDataPtr,
                                               GT_U32 hwAddr, GT_U32 basePrefix);

GT_STATUS getNumOfRangesFromHWSip7(PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol,
                                   GT_U32 depth,
                                   PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT nodeType,
                                   GT_U32 *hwData,
                                   GT_U32 hwAddr,
                                   GT_U32 nodeChildAddressesArr[6],
                                   GT_U32 nodeTotalChildTypesArr[6][3],
                                   GT_U32 nodeTotalLinesPerTypeArr[6][3],
                                   GT_U32 nodeTotalBucketPerTypesArr[6][3],
                                   GT_U32 *numOfRangesPtr,
                                   GT_U32 *rangesPtr,
                                   GT_U32 *rangesTypePtr,
                                   GT_U32 *rangesTypeIndexPtr,
                                   GT_U32 *rangesBitLinePtr);
/**
* @internal
*           prvCpssDxChLpmRamDbgGetLeafDataSip7 function
* @endinternal
*
* @brief   get the leaf data from HW
*
* @param[in] devNum                   - The device number
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] hwBucketDataArr          - array holding hw data.
*                                       in case of root this is
*                                       a null pointer
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
* @param[out] nextNodeTypePtr         - pointer to the next node
*                                       entry type
* @param[out] nhAdditionalDataPtr     - pointer to a set of UC
*                                       security check enablers
*                                       and IPv6 MC scope level
* @param[out] nextBucketPointerPtr    - pointer to the next
*                                       bucket on the (G,S)
*                                       lookup
* @param[out] nhPointerPtr            - Pointer to the Next Hop
*                                       Table or the ECMP Table,
*                                       based on the Leaf Type
* @param[out] leafTypePtr             - pointer to The leaf
*                                       entry type
* @param[out] entryTypePtr            - pointer to entry type:
*                                       Leaf ot Trigger
* @param[out] lpmOverEmPriorityPtr    - pointer to the resolution
*                                       priority between LPM and
*                                       Exact Match results
* @param[out]                         - the HW address of the leaf line
* @param[out] offsetOfLeafInLine      - pointer to the offset in HW were the 20 bits for
*                                       the pointer to the next bucket on the (G,S) lookup
*                                       starts this value can be:
*                                       for a line leaf structure: 2 for Leaf0, 25 for Leaf1,
*                                       48 for Leaf2, 71 for Leaf3, 94 for Leaf4 (92-23*leafOffsetInLine)+2
*                                       for a line embedded leaf structure:
*                                       91, 68, 45  (89 - leafOffsetInLine*23)+2
*  @param[out] applyPbrPtr            - pointer to applyPbr flag, Valid if EntryType ="Leaf"
*                                       0 = OFF, 1 = ON
*  @param[out] epgAssignedToLeafNodePtr -pointer to  EPG Assigned To Leaf Node, Valid if EntryType="Leaf"
*                                       EPG assigned to the leaf node
*                                       On SIP search - assigns source EPG
*                                       On DIP search - assigns target EPG
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbgGetLeafDataSip7
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      hwBucketGonAddr,
    IN  GT_U32                                      hwBucketDataArr[],
    IN  GT_U32                                      hwRangeType,
    IN  GT_U32                                      hwRangeTypeIndex,
    IN  GT_U32                                      nodeChildAddress,
    IN  GT_U32                                      nodeTotalChildTypesArr[],
    IN  GT_BOOL                                     returnOnFailure,
    OUT GT_U32                                      *nextNodeTypePtr,
    OUT GT_U32                                      *nhAdditionalDataPtr,
    OUT GT_U32                                      *nextBucketPointerPtr,
    OUT GT_U32                                      *nhPointerPtr,
    OUT GT_U32                                      *leafTypePtr,
    OUT GT_U32                                      *entryTypePtr,
    OUT GT_U32                                      *lpmOverEmPriorityPtr,
    OUT GT_U32                                      *leafLineHwAddr,
    OUT GT_U32                                      *offsetOfLeafInLine,
    OUT GT_BOOL                                     *applyPbrPtr,
    OUT GT_U32                                      *epgAssignedToLeafNodePtr
);

/**
* @internal
*           prvCpssDxChLpmRamDbgGetSrcBucketPointerSip7 function
* @endinternal
*
* @brief   get the head of SRC tree Bucket pointer
*
* @param[in] bucketPtr   - pointer to the bucket
* @param[in] rangeIndex  - index of the range we are looking for
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops on
*                                      first failure
*                                      GT_FALSE: continue with the test on
*                                      failure
*
* @param[out]rangeSrcBucketPointerPtr - the root of the SRS tree
*                                       pointer address
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbgGetSrcBucketPointerSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC     *bucketPtr,
    IN  GT_U32                                      rangeIndex,
    OUT GT_UINTPTR                                  *rangeSrcBucketPointerPtr
);

/**
* @internal
*           prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip7
*           function
* @endinternal
*
* @brief  function that goes over the range list and count non
*         real ranges. relevant for regular buckets
*
* @param[in] bucketPtr            - pointer to the bucket
*
* @param[out] totalNumOfRangesPtr  - pointer to all
*                             ranges includded the hidden ones
*            totalRangesPtr       - pointer to the ranges values
*            totalRangesTypesPtr  - pointer to the ranges types
*            totalRangesIsHiddenPtr - pointer to flags for
*                                     hidden ranges
*
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    OUT GT_U32                                    *totalNumOfRangesPtr,
    OUT GT_U32                                    *totalRangesPtr,
    OUT GT_U32                                    *totalRangesTypesPtr,
    OUT GT_BOOL                                   *totalRangesIsHiddenPtr
);

/* the child address calculation differs according to the child type.
   The group of child nodes are organized so that all child nodes
   of the same type are grouped together*/
GT_STATUS getFromTheGonOneNodeAddrAndSizeSip7
(
    GT_U32 gonBaseAddr,
    GT_U32 *totalChildsTypeArr,
    GT_U32 rangeType,
    GT_U32 rangeTypeIndex,
    GT_U32 *gonNodeAddrPtr,
    GT_U32 *gonNodeSizePtr
);


/**
* @internal prvCpssDxChLpmRamDbgValidityCheckSip7 function
* @endinternal
*
* @brief   This function validates LPM shadow, LPM hw, and sync between LPM shadow and LPM hw.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number
* @param[in] lpmDbPtr                 - LPM DB ptr
* @param[in] vrId                     - virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - UC/MC/both prefix type
* @param[in] validityAction           - defines which validity operation is done.
* @param[in] returnOnFailure          - GT_TRUE: the validation check stops at first failure
*                                      GT_FALSE: continue with the test on failure
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal input parameter/s
* @retval GT_NOT_FOUND             - LPM DB was not found
* @retval GT_NOT_INITIALIZED       - LPM DB is not initialized
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgValidityCheckSip7
(
    IN GT_U8                                    devNum,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT               protocol,
    IN CPSS_UNICAST_MULTICAST_ENT               prefixType,
    IN CPSS_DXCH_IP_LPM_VALIDITY_TYPE_ENT       validityAction,
    IN GT_BOOL                                  returnOnFailure
);


/**
* @internal prvCpssDxChLpmRamDbgPrintLpmNodesStatisticsSip7 function
* @endinternal
*
* @brief   print statistics of LPM nodes for given vrf, protocol, prefix type.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual router id
* @param[in] protocol                 - The IP protocol
* @param[in] prefixType               - Unicast or multicast
* @param[in] lpmDbPtr                 - pointer to LPM db
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgPrintLpmNodesStatisticsSip7
(
    IN GT_U32                             vrId,
    IN CPSS_IP_PROTOCOL_STACK_ENT         protocol,
    IN CPSS_UNICAST_MULTICAST_ENT         prefixType,
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC* lpmDbPtr
);

/**
* @internal prvCpssDxChLpmRamDbgLpmTreeTraverseRunSip7 function
* @endinternal
*
* @brief   run over lpm tree for given protocol
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmEngineMemPtrPtr       - points to a an array of PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC
*                                      which holds all the memory information needed for where and
*                                      how to allocate search memory for each of the lpm levels
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] protocolStack            - the protocol Stack
* @param[in] vrId                     - VR Id
* @param[in] prefixType               - shows prefix type
*
* @retval GT_OK                    - If there is enough memory for the insertion.
* @retval GT_OUT_OF_PP_MEM         - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbgLpmTreeTraverseRunSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC           **lpmEngineMemPtrPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN GT_U32                                       vrId,
    IN CPSS_UNICAST_MULTICAST_ENT                   prefixType
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChSip7LpmRamDbgh */


