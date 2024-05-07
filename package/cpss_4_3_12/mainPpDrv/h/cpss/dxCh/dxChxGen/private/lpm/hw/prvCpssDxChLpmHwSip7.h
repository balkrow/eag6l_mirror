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
* @file prvCpssDxChLpmHwSip7.h
*
* @brief Sip7 HW memory read/write internal functions
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChLpmHwSip7h
#define __prvCpssDxChLpmHwSip7h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>

#ifdef GM_USED
/* childPointer propertiers: bank number of bits and lpmOffset number of bits  */
#define PRV_CPSS_DXCH_LPM_RAM_BANK_NUMBER_OF_BITS_CNS                        6
#define PRV_CPSS_DXCH_LPM_RAM_BANK_MASK_CNS                                  0x3f     /* 6 bits mask */
#define PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS                      14       /* using only 14 bits out of 18 - since each block has only 16K lines*/
#define PRV_CPSS_DXCH_LPM_RAM_OFFSET_MASK_CNS                                0x3FFF   /* 14 bits mask */
#define PRV_CPSS_DXCH_LPM_RAM_CHILD_POINTER_NUMBER_OF_BITS_CNS               20       /* using only 20 bits out of 24 - since each block has only 16K lines*/
#define PRV_CPSS_DXCH_LPM_RAM_CHILD_POINTER_MASK_CNS                         0xFFFFF  /* 20 bits mask */
#else
/* childPointer propertiers: bank number of bits and lpmOffset number of bits  */
#define PRV_CPSS_DXCH_LPM_RAM_BANK_NUMBER_OF_BITS_CNS                        6
#define PRV_CPSS_DXCH_LPM_RAM_BANK_MASK_CNS                                  0x3f     /* 6 bits mask */
#define PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS                      18
#define PRV_CPSS_DXCH_LPM_RAM_OFFSET_MASK_CNS                                0x3ffff  /* 18 bits mask */
#define PRV_CPSS_DXCH_LPM_RAM_CHILD_POINTER_NUMBER_OF_BITS_CNS               24
#define PRV_CPSS_DXCH_LPM_RAM_CHILD_POINTER_MASK_CNS                         0xFFFFFF /* 24 bits mask */
#endif

#define PRV_CPSS_DXCH_LPM_RAM_CHILD_POINTER_NUMBER_OF_BITS_IN_HW_CNS         24       /* 24 bits are defined in HW for childPtr (offset+bankId) */
/**
* @internal prvCpssDxChLpmHwVrfEntryWriteSip7 function
* @endinternal
*
* @brief   Write a VRF table entry
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] shadowType               - the shadow type
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the protocol
* @param[in] nodeType                 - the type of the node
* @param[in] rootNodeAddr             - the line number of the root node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_OUT_OF_RANGE          - rootNodeAddr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryWriteSip7
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    IN  GT_U32                                  rootNodeAddr
);

/**
* @internal prvCpssDxChLpmHwVrfEntryReadSip7 function
* @endinternal
 *
* @brief   Read a VRF table entry
 *
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeTypePtr can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E
*       CPSS_DXCH_LPM_LEAFE_NODE_PTR_TYPE_E,
*       or CPSS_DXCH_LPM_ONE_COMPRESSED_NODE_PTR_TYPE_E.
*
* @param[in] devNum                   - the device number
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the
* @param[out] nodeTypePtr              - the type of the node
* @param[out] rootNodeAddrPtr          - the line number of the root node
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryReadSip7
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    OUT PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT        *nodeTypePtr,
    OUT GT_U32                                  *rootNodeAddrPtr
);

/**
* @internal prvCpssDxChLpmHwEcmpEntryWriteSip7 function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryWriteSip7
(
    IN GT_U8                                devNum,
    IN GT_U32                               ecmpEntryIndex,
    IN PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC     *ecmpEntryPtr
);

/**
* @internal prvCpssDxChLpmHwEcmpEntryReadSip7 function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryReadSip7
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ecmpEntryIndex,
    OUT PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC    *ecmpEntryPtr
);

/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndexSip7 function
* @endinternal
*
* @brief   Calculate HW Index for an LPM leaf entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] lpmIndexPtr              - (pointer to) leaf index within the LPM PBR block in HW representation
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndexSip7
(
    IN GT_U8            devNum,
    IN GT_U32           leafIndex,
    OUT GT_U32          *lpmIndexPtr
);

/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndexSip7 function
* @endinternal
*
* @brief   Calculate SW Index for an LPM leaf HW index
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] leafHwIndex              - leaf HW index within the LPM PBR block in HW representation
*
* @param[out] leafSwIndexPtr           - (pointer to)leaf index within the LPM PBR block SW representation
*
* @retval GT_OK                    - on success
*/
GT_VOID prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndexSip7
(
    IN GT_U8            devNum,
    IN GT_U32           leafHwIndex,
    OUT GT_U32          *leafSwIndexPtr
);

/**
* @internal prvCpssLpmFillPrvLeafValueSip7 function
* @endinternal
*
* @brief   translate CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf into PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_VOID prvCpssLpmFillPrvLeafValueSip7
(
    IN  CPSS_DXCH_LPM_LEAF_ENTRY_STC      *leafEntryPtr,
    OUT PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *prvLeafEntryPtr
);

/**
* @internal prvCpssLpmBuildLeafValueSip7 function
* @endinternal
*
* @brief   Build leaf data for single leaf
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssLpmBuildLeafValueSip7
(
    IN  GT_U8                             devNum,
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *leafNodePtr,
    OUT GT_U32                            *valuePtr
);

/**
* @internal prvCpssDxChLpmLeafNodeDataBuildSip7 function
* @endinternal
*
* @brief   Build hw format for LPM leaves nodes
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                     - device number
* @param[in] leafNodePtr                - node data
* @param[in] leavesCounter              - number of leaves so far
* @param[in] startNewGonLeaves          - leaves from the new GON started to process
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmLeafNodeDataBuildSip7
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafNodePtr,
    IN  GT_U32                                      leavesCounter,
    IN  GT_BOOL                                     startNewGonLeaves,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
);

/**
* @internal prvCpssDxChLpmCompressedNodeDataBuildSip7 function
* @endinternal
*
* @brief   Build hw format for LPM compressed nodes
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                     - the device number
* @param[in] compressedNodePtr          - node data
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmCompressedNodeDataBuildSip7
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_LPM_COMPRESSED_STC          *compressedNodePtr,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
);

/**
* @internal prvCpssLpmDecodeLeafValueSip7 function
* @endinternal
*
* @brief   Decode leaf entry from HW value
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] value                  - leaf data in HW format
*
* @param[out] leafNodePtr           - pointer to leaf entry.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on any parameter is wrong
*/
GT_STATUS prvCpssLpmDecodeLeafValueSip7
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          *value,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC    *leafNodePtr
);

/**
* @internal prvCpssLpmPbrIndexConvertSip7 function
* @endinternal
*
* @brief   Convert leafIndex to address space index
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                  - device number
* @param[in] leafIndexPtr            - pointer to leaf index.
*
* @param[out] leafIndexPtr           - pointer to leaf index in address space format.
*
* @retval GT_OK                     - on success
*/
GT_STATUS prvCpssLpmPbrIndexConvertSip7
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *leafIndexPtr
);

/**
* @internal prvCpssDxChLpmPbrBankSizeSetSip7 function
* @endinternal
*
* @brief  Sip7 : set PBR bank size.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - the device number
* @param[in] pbrBankSize            - 0 is small , 1 is medium , 2 is big
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*/
GT_STATUS prvCpssDxChLpmPbrBankSizeSetSip7
(
    IN GT_U8          devNum,
    IN GT_U32         pbrBankSize
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChLpmHwSip7h */


