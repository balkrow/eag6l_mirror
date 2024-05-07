
/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cpssDxChBrgHEcmp.c
*
* DESCRIPTION:
*       API implementations for Hierarchical ECMP.
*
* DEPENDENCIES:
*       None
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/

#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgHEcmp.h>

/**
 * @internal internal_cpssDxChBrgHEcmpTargetVPortMappingEntrySet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpTargetVPortMappingEntrySet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    IN  CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData[2];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (index >= CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Index %d exceeds limit %d\n",
                                      index, CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS);
    }

    /* format entry data into hardware data */
    cpssOsMemSet(hwData, 0, sizeof(hwData));
    U32_SET_FIELD_IN_ENTRY_MAC(hwData,  0,  2, (GT_U32)entryPtr->nextStage);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData,  2, 18, entryPtr->nextStagePtr);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 20,  2, (GT_U32)entryPtr->tablePtrTableId);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 22, 19, entryPtr->tablePtrBaseAddr);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 41,  2, (GT_U32)entryPtr->tablePtrEntrySize);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 43, 14, entryPtr->profile);

    /* write hardware data in the table */
    rc = prvCpssDxChWriteTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_VPORT_MAPPING_E,
                                    index, &hwData[0]);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpTargetVPortMappingEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChBrgHEcmpTargetVPortMappingEntrySet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChBrgHEcmpTargetVPortMappingEntryGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpTargetVPortMappingEntryGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      index,
    OUT CPSS_DXCH_BRG_HECMP_VPORT_MAPPING_ENTRY_STC *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    fieldVal = 0;
    GT_U32    hwData[2];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (index >= CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Index %d exceeds limit %d\n",
                                      index, CPSS_DXCH_VPORT_MAPPING_TABLE_SIZE_CNS);
    }

    cpssOsMemSet(hwData, 0, sizeof(hwData));

    /* read hardware data in the table */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_VPORT_MAPPING_E,
                                    index, &hwData[0]);
    if (rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Reading to VPORT mapping table failed");
    }

    /* read hwData and populate entry structure */
    U32_GET_FIELD_IN_ENTRY_MAC(hwData,  0,  2, fieldVal);
    entryPtr->nextStage = (CPSS_DXCH_BRG_HECMP_STAGE_ENT)fieldVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwData,  2, 18, fieldVal);
    entryPtr->nextStagePtr = fieldVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 20,  2, fieldVal);
    entryPtr->tablePtrTableId = (CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_ENT)fieldVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 22, 19, fieldVal);
    entryPtr->tablePtrBaseAddr = fieldVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 41,  2, fieldVal);
    entryPtr->tablePtrEntrySize = (CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT)fieldVal;
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 43, 14, fieldVal);
    entryPtr->profile = fieldVal;

    return GT_OK;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpTargetVPortMappingEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, entryPtr));

    rc = internal_cpssDxChBrgHEcmpTargetVPortMappingEntryGet(devNum, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpLttEntrySet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpLttEntrySet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage,
    IN  GT_U32                              index,
    IN  CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData[2];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }
    if (index >= CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "index %d out of range \n", index);
    }

    /* format entry data into hardware data */
    cpssOsMemSet(hwData, 0, sizeof(hwData));
    U32_SET_FIELD_IN_ENTRY_MAC(hwData,  0,  1, (GT_U32)entryPtr->ecmpEnable);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData,  1,  1, (GT_U32)entryPtr->ecmpRandomPathEnable);
    U32_SET_FIELD_IN_ENTRY_MAC(hwData,  2, 16, (entryPtr->ecmpStartIndex & 0xFFFF));
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 18, 12, (entryPtr->ecmpNumOfPaths & 0xFFF));
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 30,  4, (entryPtr->hashBitSelectionProfile & 0xF));
    U32_SET_FIELD_IN_ENTRY_MAC(hwData, 34,  1, (GT_U32)entryPtr->dualModeEnable);

    /* write hardware data in the table */
    rc = prvCpssDxChWriteTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_LTT_MEM_E,
                                    index, &hwData[0]);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpLttEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, index, entryPtr));

    rc = internal_cpssDxChBrgHEcmpLttEntrySet(devNum, stage, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
 * @internal internal_cpssDxChBrgHEcmpLttEntryGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpLttEntryGet
(
    IN  GT_U8                               devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT       stage,
    IN  GT_U32                              index,
    OUT CPSS_DXCH_BRG_HECMP_LTT_ENTRY_STC   *entryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    fieldVal = 0;
    GT_U32    hwData[2];

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(entryPtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }
    if (index >= CPSS_DXCH_HECMP_LTT_TABLE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "index %d out of range \n", index);
    }

    cpssOsMemSet(hwData, 0, sizeof(hwData));

    /* read hardware data in the table */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_LTT_MEM_E,
                                    index, &hwData[0]);
    if (rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Reading to VPORT mapping table failed");
    }

    /* read hwData and populate entry structure */
    U32_GET_FIELD_IN_ENTRY_MAC(hwData,  0,  1, fieldVal);
    entryPtr->ecmpEnable = (GT_BOOL) (fieldVal & 0x1);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData,  1,  1, fieldVal);
    entryPtr->ecmpRandomPathEnable = (GT_BOOL) (fieldVal & 0x1);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData,  2, 16, fieldVal);
    entryPtr->ecmpStartIndex = (fieldVal & 0xFFFF);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 18, 12, fieldVal);
    entryPtr->ecmpNumOfPaths = (fieldVal & 0xFFF);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 30,  4, fieldVal);
    entryPtr->hashBitSelectionProfile = (fieldVal & 0xF);
    U32_GET_FIELD_IN_ENTRY_MAC(hwData, 34,  1, fieldVal);
    entryPtr->dualModeEnable = (GT_BOOL) fieldVal;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpLttEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, index, entryPtr));

    rc = internal_cpssDxChBrgHEcmpLttEntryGet(devNum, stage, index, entryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, index, entryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpEcmpPairSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpEcmpPairSet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT        stage,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT   writeForm,
    IN  CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC    *pairEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData[5];
    GT_U32    offset = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(pairEntryPtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }
    if (index >= CPSS_DXCH_HECMP_ECMP_TABLE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "index %d out of range \n", index);
    }

    cpssOsMemSet(hwData, 0, sizeof(hwData));

    /* read original hardware data in the table */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_ECMP_MEM_E,
                                    index, &hwData[0]);


    U32_SET_FIELD_IN_ENTRY_MAC(hwData,  0,  15, pairEntryPtr->statusVectorPtr);

    offset = 15;
    if (writeForm != CPSS_DXCH_PAIR_READ_WRITE_SECOND_ONLY_E)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset +  0,  2, (GT_U32)pairEntryPtr->entry0.nextStage);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset +  2, 18, (pairEntryPtr->entry0.nextStagePtr & 0x3FFFF));
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 20,  2, (GT_U32)pairEntryPtr->entry0.tablePtrTableId);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 22, 20, (pairEntryPtr->entry0.tablePtrBaseAddr & 0xFFFFF));
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 42,  2, (GT_U32)pairEntryPtr->entry0.tablePtrEntrySize);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 44, 14, (pairEntryPtr->entry0.memberId & 0x3FFF));
    }

    offset += 58;
    if (writeForm != CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E)
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset +  0,  2, (GT_U32)pairEntryPtr->entry1.nextStage);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset +  2, 18, (pairEntryPtr->entry1.nextStagePtr & 0x3FFFF));
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 20,  2, (GT_U32)pairEntryPtr->entry1.tablePtrTableId);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 22, 20, (pairEntryPtr->entry1.tablePtrBaseAddr & 0xFFFFF));
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 42,  2, (GT_U32)pairEntryPtr->entry1.tablePtrEntrySize);
        U32_SET_FIELD_IN_ENTRY_MAC(hwData, offset + 44, 14, (pairEntryPtr->entry1.memberId & 0x3FFF));
    }

    /* write hardware data in the table */
    rc = prvCpssDxChWriteTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_ECMP_MEM_E,
                                    index, &hwData[0]);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpEcmpPairSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, index, writeForm, pairEntryPtr));

    rc = internal_cpssDxChBrgHEcmpEcmpPairSet(devNum, stage, index, writeForm, pairEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, index, writeForm, pairEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpEcmpPairGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpEcmpPairGet
(
    IN  GT_U8                                devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT        stage,
    IN  GT_U32                               index,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT   readForm,
    OUT CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC    *pairEntryPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    hwData[5];
    GT_U32    fieldVal = 0;
    GT_U32    offset = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(pairEntryPtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }
    if (index >= CPSS_DXCH_HECMP_ECMP_TABLE_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "index %d out of range \n", index);
    }

    cpssOsMemSet(hwData, 0, sizeof(hwData));

    /* read original hardware data in the table */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_ECMP_MEM_E,
                                    index, &hwData[0]);
    if (rc)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Reading to Ecmp table failed");
    }


    U32_GET_FIELD_IN_ENTRY_MAC(hwData,  0,  15, fieldVal);
    pairEntryPtr->statusVectorPtr = fieldVal & 0x7FFF;

    offset = 15;
    if (readForm != CPSS_DXCH_PAIR_READ_WRITE_SECOND_ONLY_E)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  0,  2, fieldVal);
        pairEntryPtr->entry0.nextStage = (CPSS_DXCH_BRG_HECMP_STAGE_ENT)(fieldVal & 0x3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  2, 18, fieldVal);
        pairEntryPtr->entry0.nextStagePtr = (fieldVal & 0x3FFFF);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 20,  2, fieldVal);
        pairEntryPtr->entry0.tablePtrTableId = (CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_ENT)(fieldVal & 0x3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 22, 20, fieldVal);
        pairEntryPtr->entry0.tablePtrBaseAddr = (fieldVal & 0xFFFFF);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 42,  2, fieldVal);
        pairEntryPtr->entry0.tablePtrEntrySize = (CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT)(fieldVal & 0x3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 44, 14, fieldVal);
        pairEntryPtr->entry0.memberId = (fieldVal & 0x3FFF);
    }

    offset += 58;
    if (readForm != CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E)
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  0,  2, fieldVal);
        pairEntryPtr->entry1.nextStage = (CPSS_DXCH_BRG_HECMP_STAGE_ENT)(fieldVal & 0x3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset +  2, 18, fieldVal);
        pairEntryPtr->entry1.nextStagePtr = (fieldVal & 0x3FFFF);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 20,  2, fieldVal);
        pairEntryPtr->entry1.tablePtrTableId = (CPSS_DXCH_BRG_HECMP_HA_TABLE_PTR_TABLE_ID_ENT)(fieldVal & 0x3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 22, 20, fieldVal);
        pairEntryPtr->entry1.tablePtrBaseAddr = (fieldVal & 0xFFFFF);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 42,  2, fieldVal);
        pairEntryPtr->entry1.tablePtrEntrySize = (CPSS_DXCH_BRG_HECMP_TABLE_PTR_ENTRY_SIZE_ENT)(fieldVal & 0x3);
        U32_GET_FIELD_IN_ENTRY_MAC(hwData, offset + 44, 14, fieldVal);
        pairEntryPtr->entry1.memberId = (fieldVal & 0x3FFF);
    }


    return GT_OK;
}

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
    OUT CPSS_DXCH_BRG_HECMP_ECMP_PAIR_STC    *pairEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpEcmpPairGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, index, readForm, pairEntryPtr));

    rc = internal_cpssDxChBrgHEcmpEcmpPairGet(devNum, stage, index, readForm, pairEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, index, readForm, pairEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpStatusVectorSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpStatusVectorSet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         index,
    IN  GT_U32                         statusVector
)
{
    GT_STATUS rc = GT_OK;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (index >= CPSS_DXCH_HECMP_STATUS_VECTOR_SIZE_IN_WORDS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Status Vector index %d out of range\n", index);
    }

    /* write hardware data in the table */
    rc = prvCpssDxChWriteTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_STATUS_VECTOR_E,
                                    index, &statusVector);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpStatusVectorSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusVector));

    rc = internal_cpssDxChBrgHEcmpStatusVectorSet(devNum, index, statusVector);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusVector));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpStatusVectorGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpStatusVectorGet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         index,
    OUT GT_U32                         *statusVectorPtr
)
{
    GT_STATUS rc = GT_OK;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(statusVectorPtr);

    if (index >= CPSS_DXCH_HECMP_STATUS_VECTOR_SIZE_IN_WORDS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Status Vector index %d out of range\n", index);
    }

    /* read hardware data from the table */
    rc = prvCpssDxChReadTableEntry(devNum, CPSS_DXCH_SIP7_TABLE_EQ_HECMP_STATUS_VECTOR_E,
                                    index, statusVectorPtr);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpStatusVectorGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, statusVectorPtr));

    rc = internal_cpssDxChBrgHEcmpStatusVectorGet(devNum, index, statusVectorPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, statusVectorPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPathSelectGcfEnableSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPathSelectGcfEnableSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];
    hwData = BOOL2BIT_MAC(enable);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwData);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPathSelectGcfEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChBrgHEcmpPathSelectGcfEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPathSelectGcfEnableGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPathSelectGcfEnableGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwData);
    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPathSelectGcfEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChBrgHEcmpPathSelectGcfEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          location,
    IN  GT_U32                          size
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }
    if (location > 0x7F)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid location value %d\n", location);
    }
    if (size > 7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid size value %d\n", size);
    }
    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];
    hwData = (location & 0x7F) | ((size & 0x7) << 7);

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 10, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, location, size));

    rc = internal_cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeSet(devNum, stage, location, size);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, location, size));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          *locationPtr,
    IN  GT_U32                          *sizePtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;


    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(locationPtr);
    CPSS_NULL_PTR_CHECK_MAC(sizePtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 10, &hwData);
    *locationPtr = hwData & 0x7F;
    *sizePtr     = (hwData >> 7) & 0x7;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, locationPtr, sizePtr));

    rc = internal_cpssDxChBrgHEcmpPathSelectGcfLocationAndSizeGet(devNum, stage, locationPtr, sizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, locationPtr, sizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPOrBSelectGcfEnableSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPOrBSelectGcfEnableSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_BOOL                         enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];
    hwData = BOOL2BIT_MAC(enable);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 11, 1, hwData);

    return rc;

}

/**
 * @internal cpssDxChBrgHEcmpPOrBSelectGcfEnableSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPOrBSelectGcfEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enable));

    rc = internal_cpssDxChBrgHEcmpPOrBSelectGcfEnableSet(devNum, stage, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPOrBSelectGcfEnableGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPOrBSelectGcfEnableGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    OUT GT_BOOL                         *enablePtr
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 11, 1, &hwData);
    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPOrBSelectGcfEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, enablePtr));

    rc = internal_cpssDxChBrgHEcmpPOrBSelectGcfEnableGet(devNum, stage, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPorBSelectGcfLocationSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPorBSelectGcfLocationSet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          location
)
{
    GT_STATUS rc      = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }
    if (location > 0x7F)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid location value %d\n", location);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];
    hwData = location & 0x7f;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 12, 7, hwData);

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPorBSelectGcfLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, location));

    rc = internal_cpssDxChBrgHEcmpPorBSelectGcfLocationSet(devNum, stage, location);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, location));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpPorBSelectGcfLocationGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpPorBSelectGcfLocationGet
(
    IN  GT_U8                           devNum,
    IN  CPSS_DXCH_BRG_HECMP_STAGE_ENT   stage,
    IN  GT_U32                          *locationPtr
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(locationPtr);

    if (stage > CPSS_DXCH_BRG_HECMP_STAGE_3_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "HECMP stage number %d is out of range\n", (GT_U32)stage);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.stagePathInfo[stage];

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 12, 7, &hwData);
    *locationPtr = hwData & 0x7F;

    return rc;
}

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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpPorBSelectGcfLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, stage, locationPtr));

    rc = internal_cpssDxChBrgHEcmpPorBSelectGcfLocationGet(devNum, stage, locationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, stage, locationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.isIngressDevice;
    hwData = BOOL2BIT_MAC(enable);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgHEcmpIsIngressDeviceGcfEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet
(
    IN  GT_U8               devNum,
    OUT GT_BOOL             *enablePtr
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.isIngressDevice;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwData);
    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}


/**
 * @internal cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgHEcmpIsIngressDeviceGcfEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              location
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    if (location > 0x7F)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid location value %d\n", location);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.isIngressDevice;
    hwData = location & 0x7F;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 7, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, location));

    rc = internal_cpssDxChBrgHEcmpIsIngressDeviceGcfLocationSet(devNum, location);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, location));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              *locationPtr
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(locationPtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.isIngressDevice;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 7, &hwData);
    *locationPtr = hwData & 0x7F;

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, locationPtr));

    rc = internal_cpssDxChBrgHEcmpIsIngressDeviceGcfLocationGet(devNum, locationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, locationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet
(
    IN  GT_U8               devNum,
    IN  GT_BOOL             enable
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.mcReplicationToRemoteDev;
    hwData = BOOL2BIT_MAC(enable);
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 1, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChBrgHEcmpIsMcReplicationsGcfEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet
(
    IN  GT_U8               devNum,
    OUT GT_BOOL             *enablePtr
)
{
    GT_STATUS   rc      = GT_OK;
    GT_U32      regAddr = 0;
    GT_U32      hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.mcReplicationToRemoteDev;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 1, &hwData);
    *enablePtr = BIT2BOOL_MAC(hwData);

    return rc;
}


/**
 * @internal cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChBrgHEcmpIsMcReplicationsGcfEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet
(
    IN  GT_U8               devNum,
    IN  GT_U32              location
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    if (location > 0x7F)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid location value %d\n", location);
    }

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.mcReplicationToRemoteDev;
    hwData = location & 0x7F;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 1, 7, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, location));

    rc = internal_cpssDxChBrgHEcmpIsMcReplicationsGcfLocationSet(devNum, location);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, location));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet
(
    IN  GT_U8               devNum,
    IN  GT_U32              *locationPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData  = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(locationPtr);

    regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).GCF.mcReplicationToRemoteDev;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 1, 7, &hwData);
    *locationPtr = hwData & 0x7F;

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, locationPtr));

    rc = internal_cpssDxChBrgHEcmpIsMcReplicationsGcfLocationGet(devNum, locationPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, locationPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpExceptionPktCmdSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpExceptionPktCmdSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    IN  CPSS_PACKET_CMD_ENT                    command
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if (command > CPSS_PACKET_CMD_DROP_SOFT_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid type");
    }

    if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.mllToOtherDevice ;
    }
    else if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.hEcmpOutOfSync;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid type");
    }

    hwData = ((GT_U32)command) & 0x7;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 3, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpExceptionPktCmdSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpExceptionPktCmdSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, type, command));

    rc = internal_cpssDxChBrgHEcmpExceptionPktCmdSet(devNum, type, command);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, command));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpExceptionPktCmdGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpExceptionPktCmdGet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    OUT CPSS_PACKET_CMD_ENT                    *commandPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(commandPtr);

    if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.mllToOtherDevice ;
    }
    else if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.hEcmpOutOfSync;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid type");
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 3, &hwData);
    *commandPtr = (CPSS_PACKET_CMD_ENT)hwData;

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpExceptionPktCmdGet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpExceptionPktCmdGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, commandPtr));

    rc = internal_cpssDxChBrgHEcmpExceptionPktCmdGet(devNum, type, commandPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, commandPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpExceptionCpuCodeSet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpExceptionCpuCodeSet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    IN  CPSS_NET_RX_CPU_CODE_ENT               cpuCode
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);

    if ((GT_U32)cpuCode > 255)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid cpu code");
    }

    if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.mllToOtherDevice ;
    }
    else if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.hEcmpOutOfSync;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid type");
    }

    hwData = ((GT_U32)cpuCode) & 0xFF;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 3, 8, hwData);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpExceptionCpuCodeSet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpExceptionCpuCodeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, cpuCode));

    rc = internal_cpssDxChBrgHEcmpExceptionCpuCodeSet(devNum, type, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
 * @internal internal_cpssDxChBrgHEcmpExceptionCpuCodeGet function
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
static GT_STATUS internal_cpssDxChBrgHEcmpExceptionCpuCodeGet
(
    IN  GT_U8                                  devNum,
    IN  CPSS_DXCH_BRG_HECMP_EXCEPTION_TYPE_ENT type,
    OUT CPSS_NET_RX_CPU_CODE_ENT               *cpuCodePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    regAddr = 0;
    GT_U32    hwData = 0;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E |
            CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E |
            CPSS_IRONMAN_E | CPSS_ALDRIN_E | CPSS_AC3X_E);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);

    if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_REPLICATED_TO_OTHER_DEVICE_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.mllToOtherDevice ;
    }
    else if (type == CPSS_DXCH_BRG_HECMP_EXCEPTION_OUT_OF_SYNC_E)
    {
        regAddr = PRV_DXCH_REG1_UNIT_EQ_MAC(devNum).E2PHY.hEcmpOutOfSync;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid type");
    }

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 3, 8, &hwData);
    *cpuCodePtr = (CPSS_NET_RX_CPU_CODE_ENT)(hwData & 0xff);

    return rc;
}

/**
 * @internal cpssDxChBrgHEcmpExceptionCpuCodeGet function
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
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChBrgHEcmpExceptionCpuCodeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, type, cpuCodePtr));

    rc = internal_cpssDxChBrgHEcmpExceptionCpuCodeGet(devNum, type, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, type, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
