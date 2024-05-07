/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChBrgHEcmp.h
*
* DESCRIPTION:
*       API declarations for Hierarchical ECMP.
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#ifndef __cpssDxChBrgHEcmph
#define __cpssDxChBrgHEcmph

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>

#define CPSS_DXCH_VPORTS_NUM_BITS_CNS                 18
#define CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS        4096
#define CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS            (2*_1K)
#define CPSS_DXCH_HECMP_ECMP_TABLE_SIZE_CNS           (2*_1K)
#define CPSS_DXCH_HECMP_STATUS_VECTOR_SIZE_IN_WORDS   (_1K)
#define CPSS_DXCH_HECMP_STATUS_VECTOR_NUM_BITS_CNS    15

/**
 * @enum CPSS_DXCH_BRG_HECMP_STAGE_ENT
 *
 * @brief Hierarchical ECMP stages enumerator
 */
typedef enum {
    /** Stage 1 */
    CPSS_DXCH_BRG_HECMP_STAGE_1_E,

    /** Stage 2 */
    CPSS_DXCH_BRG_HECMP_STAGE_2_E,

    /** Stage 3 */
    CPSS_DXCH_BRG_HECMP_STAGE_3_E,

    /** E2Phy */
    CPSS_DXCH_BRG_HECMP_E2PHY_E
} CPSS_DXCH_BRG_HECMP_STAGE_ENT;

/**
 *  @enum CPSS_DXCH_BRG_HECMP_TABLE_PTR_TABLE_ID_ENT
 *
 *  @brief Header Alteration table that is associated with the target vport
 */
typedef enum {
    /** No table is associated */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_NONE_E,

    /** Header Fields table 1 */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_HFT1_E,

    /** Header Fields table 2 */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_HFT2_E,

    /** Tunnel Start table */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_TS_E,
} CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_ENT;

/**
 * @enum CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT
 *
 * @brief The entry size of the Header Alteration table entry that is
 *         associated with the target H-ePort
 */
typedef enum {
    /** Single entry size */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_SINGLE_E,

    /** Double entry size */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_DOUBLE_E,

    /** Quadruple entry size */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_QUADRUPLE_E,

    /** Octuple entry size */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_ENTRY_SIZE_OCTUPLE_E,
} CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT;

/**
 * @enum CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT
 */
typedef enum {
    /** MLL replicated to other device exception */
    CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E,

    /** Out of sync exception. Target device extracted
     *  from E2Phy is not same as packet's incoming target device */
    CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E,
} CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT;


/**
 * @struct CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC
 *
 * @brief Hierarchical target vPort mapping table entry structure
 */
typedef struct {
    /** Specifies the Hierarchical ECMP entry point */
    CPSS_DXCH_BRG_HECMP_STAGE_ENT              nextStage;

    /** If <Next Stage> == Stage1/Stage2/Stage3 it is a pointer to the next stage LTT.
    *   If <Next Stage> == E2PHY this is the Trg Circuit. */
    GT_U32                                      nextStagePtr;

    /** Specifies the Header Alteration table that is associated with the target vPort */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_ENT      tablePtrTableId;

    /** The base address of the Header Alteration table entry that is associated with
     *  the target H-ePort. The table type is specified by <Table Ptr Table ID> */
    GT_U32                                      tablePtrBaseAddr;

    /** The entry size of the Header Alteration table entry that is associated with
     *  the target H-ePort */
    CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT    tablePtrEntrySize;

    /** Passenger packet attributes profile. Assigned to Desc<Trg ePort>. */
    GT_U32                                      profile;
} CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC;

/**
 * @struct CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC
 *
 * @brief Hierarchical ECMP LTT Entry structure
 */
typedef struct{

    /** @brief Determines the start index of the L2 ECMP block.
     *  (APPLICABLE RANGES: 0..8191)
     *  NOTE: When trunk mode is 'CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E' the CPSS allows
     *  to point to all 'L2 ECMP' table entries.
     *  Otherwise the CPSS allows to point ONLY to the lower half of 'L2 ECMP' table entries.
     */
    GT_U32 ecmpStartIndex;

    /** @brief The number of paths in the ECMP block:
     *  0x1 = 1 path, 0x2 = 2 paths and so on.
     *  (APPLICABLE RANGES: 1..4096)
     */
    GT_U32 ecmpNumOfPaths;

    /** Determines whether this packet is load balanced over an ECMP group. */
    GT_BOOL ecmpEnable;

    /** allows random load balance */
    GT_BOOL ecmpRandomPathEnable;

    /** @brief  Defines index of the Hash Bit Selection Profile
     */
    GT_U32 hashBitSelectionProfile;

    /** @brief If this field is set, then each ECMP table row includes the
     *  ECMP entry of the primary path and the ECMP entry of the backup path
     *  of the same ECMP group member */
    GT_BOOL dualModeEnable;

} CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC;


/**
 * @struct CPSS_DXCH_BRG_HECMP_ECMP_ENTRY_STC
 *
 * @brief Hierarchical-ECMP ECMP entry fields structure
 */
typedef struct {
    /** Specifies the next Hierarchical ECMP stage
        0: Reserved, 1: Stage2, 2: Stage3, 3: E2PHY  */
    CPSS_DXCH_BRG_HECMP_STAGE_ENT              nextStage;

    /** If <Next Stage> == Stage2/Stage3 it is a pointer to the next stage LTT.
     *  If <Next Stage> == E2PHY this is the Trg Circuit.*/
    GT_U32                                      nextStagePtr;

    /** Specifies the Header Alteration table that is associated with this ECMP entry */
    CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_ENT      tablePtrTableId;

    /** The base address of the Header Alteration table entry that is associated with
     *  this ECMP entry. The table type is specified by <Table Ptr Table ID> */
    GT_U32                                      tablePtrBaseAddr;

    /** The entry size of the Header Alteration table entry that is associated with this ECMP entry */
    CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT    tablePtrEntrySize;

    /** The member id of this entry in the ECMP group */
    GT_U32                                      memberId;
} CPSS_DXCH_BRG_HECMP_ECMP_ENTRY_STC;


/**
 * @struct CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC
 *
 * @brief ECMP table entry fields structure
 */
typedef struct {
    /** ECMP entry 0. In Hierarchical ECMP mode, it is primary entry */
    CPSS_DXCH_BRG_HECMP_ECMP_ENTRY_STC entry0;

    /** ECMP entry 1. In Hierarchical ECMP mode, it is backup entry */
    CPSS_DXCH_BRG_HECMP_ECMP_ENTRY_STC entry1;

    /** Pointer to bit vector that is relevant when <Protection switching mode>=0.
     *  If the pointed bit is 0 then <ECMP Entry 0 (Primary)> is selected,
     *  otherwise <ECMP Entry 1 (Backup)> is selected. */
    GT_U32                               statusVectorPtr;
} CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC;


/**
 * @internal cpssDxChBrgHEcmpTargetVPortMappingEntrySet function
 * @endinternal
 *
 * @brief  Set Hierarchical target vPort mapping table entry
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in] devNum   - device number
 * @param[in] index    - entry index
 * @param[in] entryPtr - (pointer to) stucture to table entry
 *
 * @retval GT_OK                    - On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpTargetVPortMappingEntrySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    IN  CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC *entryPtr
);

/**
 * @internal cpssDxChBrgHEcmpTargetVPortMappingEntryGet function
 * @endinternal
 *
 * @brief  Get Hierarchical target vPort mapping table entry
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum   - device number
 * @param[in]  index    - entry index
 * @param[out] entryPtr - (pointer to) stucture to table entry
 *
 * @retval GT_OK  -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpTargetVPortMappingEntryGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    OUT CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC *entryPtr
);

/**
 * @internal cpssDxChBrgHEcmpLttEntrySet function
 * @endinternal
 *
 * @brief  Set Hierarchical ECMP LTT entry
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in] devNum   - device number
 * @param[in] stage    - Hierarchical ECMP stage1/stage2/stage3
 * @param[in] index    - entry index
 * @param[in] entryPtr - (pointer to) stucture to ltt table entry
 *
 * @retval GT_OK  -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpLttEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   *entryPtr
);

/**
 * @internal cpssDxChBrgHEcmpLttEntryGet function
 * @endinternal
 *
 * @brief  Get Hierarchical ECMP LTT entry
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum   - device number
 * @param[in]  stage    - Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  index    - entry index
 * @param[out] entryPtr - (pointer to) stucture to ltt table entry
 *
 * @retval GT_OK  -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpLttEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   *entryPtr
);

/**
 * @internal cpssDxChBrgHEcmpEcmpPairSet function
 * @endinternal
 *
 * @brief Set Hierarchical ECMP table entry.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in] devNum       - device number
 * @param[in] stage        - Hierarchical ECMP stage1/stage2/stage3
 * @param[in] index        - entry index
 * @param[in] writeForm    - write first entry only or second entry only or both
 * @param[in] pairEntryPtr - (pointer to) stucture to ecmp pair entry
 *
 * @retval GT_OK  -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpEcmpPairSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT        stage,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT   writeForm,
    IN  CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC    *pairEntryPtr
);

/**
 * @internal cpssDxChBrgHEcmpEcmpPairGet function
 * @endinternal
 *
 * @brief  Get Hierarchical ECMP table entry.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum   - device number
 * @param[in]  stage    - Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  index    - entry index
 * @param[in]  readForm - read first entry only or second entry only or both
 * @param[out] entryPtr - (pointer to) stucture to table entry
 *
 * @retval GT_OK  -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpEcmpPairGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT        stage,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT   readForm,
    OUT CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC    *entryPtr
);

/**
 * @internal cpssDxChBrgHEcmpStatusVectorSet function
 * @endinternal
 *
 * @brief  Set status of Fast Reroute nodes.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum       - device number
 * @param[in]  index        - index
 * @param[out] statusVector - The status of Fast Reroute Nodes
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpStatusVectorSet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         index,
    IN  GT_U32                         statusVector
);

/**
 * @internal cpssDxChBrgHEcmpStatusVectorGet function
 * @endinternal
 *
 * @brief  Get status of Fast Reroute nodes.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum          - device number
 * @param[in]  index           - index
 * @param[out] statusVectorPtr - (pointer to) The status of Fast Reroute Nodes
 *
 * @retval GT_OK                    - On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
 */
GT_STATUS cpssDxChBrgHEcmpStatusVectorGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         index,
    OUT GT_U32                         *statusVectorPtr
);


/**
 * @internal cpssDxChBrgHEcmpPathSelectGcfEnableSet function
 * @endinternal
 *
 * @brief Set enable state of selecting the ECMP path of specific stage by
 *        setting bits in Generic Classification Field
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[in]  stage         -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  enable        -  GT_TRUE/GT_FALSE to enable/disable
 *
 * @retval GT_OK                    - On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfEnableSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_BOOL                         enable
);

/**
 * @internal cpssDxChBrgHEcmpPathSelectGcfEnableGet function
 * @endinternal
 *
 * @brief Get enable state of selecting the ECMP path of specific stage by
 *        setting bits in Generic Classification Field
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[in]  stage         -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  enablePtr     - (pointer to) enable state.
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfEnableGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    OUT GT_BOOL                     *enablePtr
);

/**
 * @internal  cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet function
 * @endinternal
 *
 * @brief Set General Classification Field LSB Location and Size of specific stage
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum   -  device number
 * @param[in]  stage    -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  location -  GCF LSB location
 * @param[in]  size     -  GCF size
 *
 * @retval GT_OK                    - On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          location,
    IN  GT_U32                          size
);

/**
 * @internal cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet function
 * @endinternal
 *
 * @brief Get General Classification Field LSB Location and Size of specific stage
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum  -  device number
 * @param[in]  stage   -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  locationPtr  -  (pointer to) GCF LSB location
 * @param[in]  sizePtr -  (pointer to) GCF size
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          *locationPtr,
    IN  GT_U32                          *sizePtr
);

/**
 * @internal  cpssDxChBrgHEcmpPOrBSelectGcfEnableSet function
 * @endinternal
 *
 * @brief Set enable state to force selection of the primary path or
 *        backup path of specific stage
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[in]  stage         -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  enable        -  GT_TRUE/GT_FALSE to enable/disable
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPOrBSelectGcfEnableSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_BOOL                         enable
);

/**
 * @internal cpssDxChBrgHEcmpPOrBSelectGcfEnableGet function
 * @endinternal
 *
 * @brief Get enable state to force selection of the primary path or
 *        backup path of specific stage
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[in]  stage         -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  enablePtr     - (pointer to) enable state.
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPOrBSelectGcfEnableGet
(
    IN  GT_U8                       devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    OUT GT_BOOL                     *enablePtr

);

/**
 * @internal cpssDxChBrgHEcmpPorBSelectGcfLocationSet function
 * @endinternal
 *
 * @brief Set location of General Classification field for primary or backup path selection.
 *        Two bits are used. 0 - active path (according to bit in status vector),
 *        1 - Primay path, 2 - secondary path
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum  -  device number
 * @param[in]  stage   -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  location     -  GCF LSB location
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPorBSelectGcfLocationSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          location
);

/**
 * @internal cpssDxChBrgHEcmpPorBSelectGcfLocationGet function
 * @endinternal
 *
 * @brief Get location of General Classification field for primary or backup path selection.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum  -  device number
 * @param[in]  stage   -  Hierarchical ECMP stage1/stage2/stage3
 * @param[in]  locationPtr  -  (pointer to) GCF LSB location
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpPorBSelectGcfLocationGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          *locationPtr
);

/**
 * @internal  cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet function
 * @endinternal
 *
 * @brief Set enable state of Generic Classification Field for
 *        "Is Ingress Device" Bit
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[out] enable        -  GT_TRUE/GT_FALSE to enable/disable
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             enable
);

/**
 * @internal  cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet function
 * @endinternal
 *
 * @brief Get enable state of Generic Classification Field for
 *        "Is Ingress Device" Bit
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[out] enablePtr     -  (pointer to) GT_TRUE/GT_FALSE to enable/disable
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet
(
    IN  GT_U8               devNum,
    OUT GT_BOOL             *enablePtr
);

/**
 * @internal  cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet function
 * @endinternal
 *
 * @brief Set General Classification Field Location for "Is Ingress Device" Bit
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum       -  device number
 * @param[in]  location     -  GCF LSB location
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              location
);

/**
 * @internal  cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet function
 * @endinternal
 *
 * @brief Get General Classification Field Location for "Is Ingress Device" Bit
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum       -  device number
 * @param[in]  locationPtr  -  (pointer to) GCF LSB location
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              *locationPtr
);

/**
 * @internal  cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet function
 * @endinternal
 *
 * @brief Set enable state for General Classification Field for
 *        "Is Multicast Replicated to Remote Device" Bit.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[out] enable        -  GT_TRUE/GT_FALSE to enable/disable
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             enable
);

/**
 * @internal  cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet function
 * @endinternal
 *
 * @brief Get enable state for General Classification Field for
 *        "Is Multicast Replicated to Remote Device" Bit.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[out] enablePtr     -  (pointer to) GT_TRUE/GT_FALSE to enable/disable
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet
(
    IN  GT_U8               devNum,
    OUT GT_BOOL             *enablePtr
);

/**
 * @internal  cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet function
 * @endinternal
 *
 * @brief Set location for General Classification Field for
 *        "Is Multicast Replicated to Remote Device" Bit.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum       -  device number
 * @param[in]  location     -  GCF LSB location
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              location
);

/**
 * @internal  cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet function
 * @endinternal
 *
 * @brief Get location for General Classification Field for
 *        "Is Multicast Replicated to Remote Device" Bit.
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        -  device number
 * @param[in]  locationPtr  -  (pointer to) GCF LSB location
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              *locationPtr
);

/**
 * @internal  cpssDxChBrgHEcmpExceptionPktCmdSet function
 * @endinternal
 *
 * @brief Set packet command for specific exception
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in] devNum        - device number
 * @param[in] type          - exception type
 * @param[in] command       - the packet command
 *                            (APPLICABLE VALUES:
 *                            CPSS_PACKET_CMD_FORWARD_E
 *                            CPSS_PACKET_CMD_MIRROR_TO_CPU_E
 *                            CPSS_PACKET_CMD_TRAP_TO_CPU_E
 *                            CPSS_PACKET_CMD_DROP_HARD_E
 *                            CPSS_PACKET_CMD_DROP_SOFT_E)
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpExceptionPktCmdSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    IN  CPSS_PACKET_CMD_ENT                    command
);

/**
 * @internal  cpssDxChBrgHEcmpExceptionPktCmdGet function
 * @endinternal
 *
 * @brief Get packet command for specific exception
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum       - device number
 * @param[in]  type         - exception type
 * @param[out] commandPtr   - (pointer to) the packet command
 *                            (APPLICABLE VALUES:
 *                            CPSS_PACKET_CMD_FORWARD_E
 *                            CPSS_PACKET_CMD_MIRROR_TO_CPU_E
 *                            CPSS_PACKET_CMD_TRAP_TO_CPU_E
 *                            CPSS_PACKET_CMD_DROP_HARD_E
 *                            CPSS_PACKET_CMD_DROP_SOFT_E)
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpExceptionPktCmdGet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    OUT CPSS_PACKET_CMD_ENT                    *commandPtr
);

/**
 * @internal  cpssDxChBrgHEcmpExceptionCpuCodeSet function
 * @endinternal
 *
 * @brief Set CPU Code for specific exceptions
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        - device number
 * @param[in]  type          - exception type
 * @param[in]  cpuCode       - the CPU/drop code
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpExceptionCpuCodeSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    IN  CPSS_NET_RX_CPU_CODE_ENT               cpuCode
);

/**
 * @internal  cpssDxChBrgHEcmpExceptionCpuCodeGet function
 * @endinternal
 *
 * @brief Get CPU Code for specific exceptions
 *
 * @note    APPLICABLE DEVICES:     AAS
 * @note    NON APPLICABLE DEVICES: xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X;
 *                                  Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman
 *
 * @param[in]  devNum        - device number
 * @param[in]  type          - exception type
 * @param[out] cpuCodePtr    - the CPU/drop code
 *
 * @retval GT_OK                    -  On Success
 * @retval GT_BAD_PARAM             - Illegal parameter
 * @retval GT_OUT_OF_RANGE          - out of range error
 * @retval GT_BAD_PTR               - NULL Pointer error
 * @retval GT_NOT_APPLICALBE_DEVICE - API not applicable for the device
*/
GT_STATUS cpssDxChBrgHEcmpExceptionCpuCodeGet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    OUT CPSS_NET_RX_CPU_CODE_ENT               *cpuCodePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChBrgHEcmph */

