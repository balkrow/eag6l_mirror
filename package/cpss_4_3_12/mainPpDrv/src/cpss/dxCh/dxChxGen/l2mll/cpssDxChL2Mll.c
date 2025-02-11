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
* @file cpssDxChL2Mll.c
*
* @brief The CPSS DXCH L2 MLL APIs
*
* @version   32
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/l2mll/private/prvCpssDxChL2mllLog.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>
#include <cpss/dxCh/dxChxGen/diag/private/prvCpssDxChDiag.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* number of bits in AAS - MLL entry */
#define MAX_IP_MLL_ENTRY_BITS_SIZE_CNS       185

/* number of bits in the xCat C0; xCat3 - MLL entry */
#define PRV_CPSS_XCAT_L2_MLL_ENTRY_BITS_SIZE_CNS        128

/* size of MLL entry in word in xCat C0; xCat3 */
#define PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD BITS_TO_WORDS_MAC(PRV_CPSS_XCAT_L2_MLL_ENTRY_BITS_SIZE_CNS)

/* maximal multi-target port value */
#define PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VALUE_CNS     0x1ffff

/* maximal multi-target port VIDX base */
#define PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VIDX_BASE_CNS 0xffff

/* the LTT index can't exceed number of eVidx */
#define MLL_LTT_INDEX_CHECK_MAC(_devNum,_index)                             \
    if((_index) >                                                           \
        PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MLL_MAC(_devNum))     \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "MLL LTT index [%d] > max[%d]", \
        (_index),PRV_CPSS_DXCH_PP_HW_INFO_LAST_INDEX_IN_L2_LTT_MLL_MAC(_devNum));   \
    }

/* there are 2 L2MLL entries in each line of memory but the index
   given in the macro is the index of the line and not index of the entry */
#define MLL_NEXT_PTR_CHECK_MAC(_devNum,_index)                              \
    if((_index) >=                                                          \
        (PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.mllPairs))     \
    {                                                                       \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "MLL entry next PTR [%d] >= max[%d]",  \
            (_index),(PRV_CPSS_DXCH_PP_MAC(_devNum)->fineTuning.tableSize.mllPairs));     \
    }


static GT_STATUS sip5L2MllHw2StructFormat
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr,
    IN  GT_U32                                       *hwDataPtr
);

static GT_STATUS sip5L2MllStruct2HwFormat
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    IN  CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr,
    OUT GT_U32                                      *hwMaskPtr
);

static GT_STATUS xCatL2MllHw2StructFormat
(
    IN   GT_U32                       *hwDataPtr,
    OUT  CPSS_DXCH_L2_MLL_ENTRY_STC   *mllEntryPtr
);

static GT_STATUS xCatL2MllStruct2HwFormat
(
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairWriteForm,
    IN  CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr
);

static GT_STATUS xcatL2MllPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr
);

static GT_STATUS xcatL2MllPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr
);

extern GT_STATUS prvCpssDxChSip6GlobalEportMaskCheck
(
    IN  GT_U8   devNum,
    IN  GT_U32  mask,
    IN  GT_U32  pattern,
    OUT GT_U32  *minValuePtr,
    OUT GT_U32  *maxValuePtr
);
extern GT_STATUS prvCpssDxChSip6GlobalEportMaskGet
(
    IN  GT_U32  minValue,
    IN  GT_U32  maxValue,
    IN  GT_U32  *patternPtr,
    IN  GT_U32  *maskPtr
);


/**
* @internal internal_cpssDxChL2MllLookupForAllEvidxEnableSet function
* @endinternal
*
* @brief   Enable or disable MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: MLL lookup is performed for all multi-target packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllLookupForAllEvidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */
    GT_U32  fieldOffset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regData = BOOL2BIT_MAC(enable);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLLookupTriggerConfig;
    /*<Enable MLL Lookup For All eVIDX>*/
    fieldOffset = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 14 : 12;
    return prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, 1, regData);
}

/**
* @internal cpssDxChL2MllLookupForAllEvidxEnableSet function
* @endinternal
*
* @brief   Enable or disable MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE: MLL lookup is performed for all multi-target packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllLookupForAllEvidxEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChL2MllLookupForAllEvidxEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllLookupForAllEvidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup is performed for all multi-target
*                                      packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllLookupForAllEvidxEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */
    GT_U32  fieldOffset;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLLookupTriggerConfig;
    /*<Enable MLL Lookup For All eVIDX>*/
    fieldOffset = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 14 : 12;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, 1, &regData);

    if( rc != GT_OK )
    {
        return rc;
    }

    *enablePtr = BIT2BOOL_MAC(regData);

    return rc;
}

/**
* @internal cpssDxChL2MllLookupForAllEvidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for all multi-target packets.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup is performed for all multi-target
*                                      packets.
*                                      GT_FALSE: MLL lookup is performed only for multi-target
*                                      packets with eVIDX >= 4K. For packets with
*                                      eVidx < 4K L2 MLL is not accessed.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLookupForAllEvidxEnableGet
(
    IN GT_U8    devNum,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllLookupForAllEvidxEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChL2MllLookupForAllEvidxEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChL2MllLookupMaxVidxIndexSet function
* @endinternal
*
* @brief   Set the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] maxVidxIndex             - maximal VIDX value.
*                                      When cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*                                      L2 replication is performed to all eVidx > maxVidxIndex
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*
*/
static GT_STATUS internal_cpssDxChL2MllLookupMaxVidxIndexSet
(
    IN GT_U8   devNum,
    IN GT_U32  maxVidxIndex
)
{
    GT_U32  regAddr;    /* register address */
    GT_U32  fieldLength;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* check range */
    if (maxVidxIndex > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLLookupTriggerConfig;
    fieldLength = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 14 : 12;
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, fieldLength, maxVidxIndex);
}

/**
* @internal cpssDxChL2MllLookupMaxVidxIndexSet function
* @endinternal
*
* @brief   Set the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] maxVidxIndex             - maximal VIDX value.
*                                      When cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*                                      L2 replication is performed to all eVidx > maxVidxIndex
*                                      (APPLICABLE RANGES: 0..0xFFF)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableSet == FALSE
*
*/
GT_STATUS cpssDxChL2MllLookupMaxVidxIndexSet
(
    IN GT_U8   devNum,
    IN GT_U32  maxVidxIndex
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllLookupMaxVidxIndexSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxVidxIndex));

    rc = internal_cpssDxChL2MllLookupMaxVidxIndexSet(devNum, maxVidxIndex);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxVidxIndex));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllLookupMaxVidxIndexGet function
* @endinternal
*
* @brief   Get the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] maxVidxIndexPtr          - (pointer to) maximal VIDX value.
*                                      When <Enable MLL Lookup for All eVidx> == FALSE
*                                      L2 replication is performed to all eVidx > <Max VIDX Index>
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableGet== FALSE
*
*/
static GT_STATUS internal_cpssDxChL2MllLookupMaxVidxIndexGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *maxVidxIndexPtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */
    GT_U32  fieldLength;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(maxVidxIndexPtr);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLLookupTriggerConfig;
    fieldLength = PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) ? 14 : 12;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, fieldLength, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *maxVidxIndexPtr = regData;

    return rc;
}

/**
* @internal cpssDxChL2MllLookupMaxVidxIndexGet function
* @endinternal
*
* @brief   Get the maximal VIDX value that refers to a port distribution list(VIDX).
*         By default the value is 4K-1
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] maxVidxIndexPtr          - (pointer to) maximal VIDX value.
*                                      When <Enable MLL Lookup for All eVidx> == FALSE
*                                      L2 replication is performed to all eVidx > <Max VIDX Index>
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Relevant only when cpssDxChL2MllLookupForAllEvidxEnableGet== FALSE
*
*/
GT_STATUS cpssDxChL2MllLookupMaxVidxIndexGet
(
    IN  GT_U8    devNum,
    OUT GT_U32   *maxVidxIndexPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllLookupMaxVidxIndexGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, maxVidxIndexPtr));

    rc = internal_cpssDxChL2MllLookupMaxVidxIndexGet(devNum, maxVidxIndexPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, maxVidxIndexPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration for L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trapEnable               - GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL exception
*                                      in the MLL.
* @param[in] cpuCode                  - CPU code of packets that are trapped to CPU due to a
*                                      TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllTtlExceptionConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  trapEnable,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    rc = prvCpssDxChNetIfCpuToDsaCode(cpuCode,&dsaCpuCode);
    if( rc != GT_OK )
    {
        return rc;
    }

    regData =  (GT_U32)dsaCpuCode;/*bits 0..7*/
    U32_SET_FIELD_MAC(regData, 8, 1 ,BOOL2BIT_MAC(trapEnable));

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.globalTTLExceptionConfig;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 9, regData);

    return rc;
}

/**
* @internal cpssDxChL2MllTtlExceptionConfigurationSet function
* @endinternal
*
* @brief   Set configuration for L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] trapEnable               - GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL exception
*                                      in the MLL.
* @param[in] cpuCode                  - CPU code of packets that are trapped to CPU due to a
*                                      TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  trapEnable,
    IN CPSS_NET_RX_CPU_CODE_ENT cpuCode
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllTtlExceptionConfigurationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trapEnable, cpuCode));

    rc = internal_cpssDxChL2MllTtlExceptionConfigurationSet(devNum, trapEnable, cpuCode);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trapEnable, cpuCode));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllTtlExceptionConfigurationGet function
* @endinternal
*
* @brief   Get configuration of L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] trapEnablePtr            - (pointer to)
*                                      GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL
*                                      exception in the MLL.
* @param[out] cpuCodePtr               - (pointer to) CPU code of packets that are trapped to
*                                      CPU due to a TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllTtlExceptionConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *trapEnablePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;  /* DSA code */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(trapEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(cpuCodePtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.globalTTLExceptionConfig;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 9, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *trapEnablePtr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(regData,8,1));

    dsaCpuCode = U32_GET_FIELD_MAC(regData,0,8);

    return prvCpssDxChNetIfDsaToCpuCode(dsaCpuCode,cpuCodePtr);
}

/**
* @internal cpssDxChL2MllTtlExceptionConfigurationGet function
* @endinternal
*
* @brief   Get configuration of L2 MLL TTL Exceptions.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] trapEnablePtr            - (pointer to)
*                                      GT_TRUE: packet is trapped to the CPU with <cpuCode>
*                                      if packet's TTL is less than MLL entry field
*                                      <TTL Threshold>
*                                      GT_FALSE: no packet trap to CPU due to a TTL
*                                      exception in the MLL.
* @param[out] cpuCodePtr               - (pointer to) CPU code of packets that are trapped to
*                                      CPU due to a TTL exception in the MLL.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllTtlExceptionConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *trapEnablePtr,
    OUT CPSS_NET_RX_CPU_CODE_ENT *cpuCodePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllTtlExceptionConfigurationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, trapEnablePtr, cpuCodePtr));

    rc = internal_cpssDxChL2MllTtlExceptionConfigurationGet(devNum, trapEnablePtr, cpuCodePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, trapEnablePtr, cpuCodePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
{
    return cpssDxChL2MllPortGroupExceptionCountersGet(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            countersPtr);
}

/**
* @internal cpssDxChL2MllExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllExceptionCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, countersPtr));

    rc = internal_cpssDxChL2MllExceptionCountersGet(devNum, countersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, countersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPortGroupExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllPortGroupExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(countersPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
         /* read value from hardware register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->l2MllRegs.vplsDropCounter;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, &(countersPtr->skip));

        return rc;

    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLSkippedEntriesCntr;

    /* summary the counter from all port groups */
    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                0, 32, &(countersPtr->skip),
                                                NULL);
    if( rc != GT_OK )
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLTTLExceptionCntr;

    /* summary the counter from all port groups */
    return
         prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                0, 32, &(countersPtr->ttl),
                                                NULL);
}

/**
* @internal cpssDxChL2MllPortGroupExceptionCountersGet function
* @endinternal
*
* @brief   Get L2 MLL exception counters.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] countersPtr              - (pointer to) L2 MLL exception counters.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPortGroupExceptionCountersGet
(
    IN  GT_U8                                   devNum,
    IN GT_PORT_GROUPS_BMP                       portGroupsBmp,
    OUT CPSS_DXCH_L2_MLL_EXCEPTION_COUNTERS_STC  *countersPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPortGroupExceptionCountersGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, countersPtr));

    rc = internal_cpssDxChL2MllPortGroupExceptionCountersGet(devNum, portGroupsBmp, countersPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, countersPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - counter number. (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X)(APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  index,
    OUT GT_U32  *counterPtr
)
{
    return cpssDxChL2MllPortGroupCounterGet(devNum,
                                            CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                            index,
                                            counterPtr);
}

/**
* @internal cpssDxChL2MllCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - counter number. (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X)(APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllCounterGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  index,
    OUT GT_U32  *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, counterPtr));

    rc = internal_cpssDxChL2MllCounterGet(devNum, index, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPortGroupCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - counter number. (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X)(APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllPortGroupCounterGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              index,
    OUT GT_U32              *counterPtr
)
{
    GT_U32  regAddr;    /* register address */
    GT_STATUS rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(counterPtr);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);

    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
         /* read value from hardware register */
        regAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->l2MllRegs.vplsDropCounter;
        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 32, counterPtr);

        return rc;
    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if( index > 2 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.
        L2MLLValidProcessedEntriesCntr[index];

    return
        prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                0, 32, counterPtr, NULL);
}

/**
* @internal cpssDxChL2MllPortGroupCounterGet function
* @endinternal
*
* @brief   Get L2 MLL counter.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] index                    - counter number. (APPLICABLE DEVICES Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X)(APPLICABLE RANGES: 0..2)
*
* @param[out] counterPtr               - (pointer to) L2 MLL counter value.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPortGroupCounterGet
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp,
    IN  GT_U32              index,
    OUT GT_U32              *counterPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPortGroupCounterGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, index, counterPtr));

    rc = internal_cpssDxChL2MllPortGroupCounterGet(devNum, portGroupsBmp, index, counterPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, index, counterPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllSetCntInterfaceCfg function
* @endinternal
*
* @brief   Sets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] interfaceCfgPtr          - the L2 mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllSetCntInterfaceCfg
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllCntSet,
    IN CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
)
{
    GT_U32    regAddr;              /* register address                   */
    GT_U32    regAddr2;             /* register address                   */
    GT_STATUS rc;                   /* return code                        */
    GT_U32    data = 0;
    GT_U32    data2 = 0;
    GT_BOOL   setSecondReg;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfaceCfgPtr);

    /* DxCh supports 2 mll counters*/
    if(mllCntSet > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr  = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig[mllCntSet];
    regAddr2 = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig1[mllCntSet];

    data = 0;
    data2 = 0;
    setSecondReg = GT_FALSE;

    switch (interfaceCfgPtr->portTrunkCntMode)
    {
        case CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E:
            break;
        case CPSS_DXCH_L2MLL_PORT_CNT_MODE_E:
            data |= (0x1 << 30); /* <bits 30:31> = 1 indicates count port */
            if (interfaceCfgPtr->portTrunk.port > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_E_PORT_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            if (interfaceCfgPtr->hwDevNum > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_HW_DEV_NUM_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            data2 |= interfaceCfgPtr->portTrunk.port & 0xFFFFF;
            data2 |= ((interfaceCfgPtr->hwDevNum ) & 0xFFF) << 20;
            setSecondReg = GT_TRUE;
            break;
        case CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E:
            data |= (0x2 << 30); /* <bits 30:31> = 2 indicates count trunk */
            if (interfaceCfgPtr->portTrunk.trunk > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_TRUNK_ID_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            data2 |= interfaceCfgPtr->portTrunk.trunk & 0xFFF;
            setSecondReg = GT_TRUE;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (interfaceCfgPtr->vlanMode)
    {
        case CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E:
            break;
        case CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E:
            if (interfaceCfgPtr->vlanId > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VID_MAC(devNum))
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            data  |= (0x1 << 26); /* <bit 26> = 1 indicates count vlan */
            data  |= ((interfaceCfgPtr->vlanId & 0xFFFF) << 10);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (interfaceCfgPtr->ipMode)
    {
        case CPSS_IP_PROTOCOL_IPV4_E:
            data |= (0x1 << 27); /* <bits 27:28> = 1 indicates count ipv4 */
            break;
        case CPSS_IP_PROTOCOL_IPV6_E:
            data |= (0x2 << 27); /* <bits 27:28> = 2 indicates count ipv6 */
            break;
        case CPSS_IP_PROTOCOL_IPV4V6_E:
            data |= (0x0 << 27); /* <bits 27:28> = 0 indicates count all */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssHwPpWriteRegister(devNum,regAddr,data);
    if (rc != GT_OK)
        return rc;

    /* on some cases, there is no need to set second register */
    if(setSecondReg == GT_TRUE)
    {
        rc = prvCpssHwPpWriteRegister(devNum,regAddr2,data2);
        if (rc != GT_OK)
            return rc;
    }

    return rc;

}

/**
* @internal cpssDxChL2MllSetCntInterfaceCfg function
* @endinternal
*
* @brief   Sets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
* @param[in] interfaceCfgPtr          - the L2 mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSetCntInterfaceCfg
(
    IN GT_U8                                            devNum,
    IN GT_U32                                           mllCntSet,
    IN CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllSetCntInterfaceCfg);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllCntSet, interfaceCfgPtr));

    rc = internal_cpssDxChL2MllSetCntInterfaceCfg(devNum, mllCntSet, interfaceCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllCntSet, interfaceCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal internal_cpssDxChL2MllGetCntInterfaceCfg function
* @endinternal
*
* @brief   Gets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] interfaceCfgPtr          - the L2 mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllGetCntInterfaceCfg
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           mllCntSet,
    OUT CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
)
{
    GT_U32    regAddr;              /* register address                   */
    GT_U32    regAddr2;             /* register address                   */
    GT_STATUS rc;                   /* return code                        */
    GT_U32    data = 0;
    GT_U32    data2 = 0;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(interfaceCfgPtr);

    /* DxCh supports 2 mll counters*/
    if(mllCntSet > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(interfaceCfgPtr, 0, sizeof(interfaceCfgPtr));

    regAddr  = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig[mllCntSet];
    regAddr2 = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLOutInterfaceCntrConfig1[mllCntSet];

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&data);
    if (rc != GT_OK)
        return rc;

    rc = prvCpssHwPpReadRegister(devNum,regAddr2,&data2);
    if (rc != GT_OK)
       return rc;

    switch (U32_GET_FIELD_MAC(data,30,2))
    {
        case 0:
            interfaceCfgPtr->portTrunkCntMode = CPSS_DXCH_L2MLL_DISREGARD_PORT_TRUNK_CNT_MODE_E;
            break;
        case 1:
            interfaceCfgPtr->portTrunkCntMode = CPSS_DXCH_L2MLL_PORT_CNT_MODE_E;
            interfaceCfgPtr->portTrunk.port = (GT_PORT_NUM)U32_GET_FIELD_MAC(data2,0,20);
            interfaceCfgPtr->hwDevNum = (GT_HW_DEV_NUM)U32_GET_FIELD_MAC(data2,20,12);
            break;
        case 2:
            interfaceCfgPtr->portTrunkCntMode = CPSS_DXCH_L2MLL_TRUNK_CNT_MODE_E;
            interfaceCfgPtr->portTrunk.trunk = (GT_TRUNK_ID)U32_GET_FIELD_MAC(data2,0,12);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (U32_GET_FIELD_MAC(data,26,1))
    {
        case 0:
            interfaceCfgPtr->vlanMode = CPSS_DXCH_L2MLL_DISREGARD_VLAN_CNT_MODE_E;
            break;
        case 1:
            interfaceCfgPtr->vlanMode = CPSS_DXCH_L2MLL_USE_VLAN_CNT_MODE_E;
            interfaceCfgPtr->vlanId = (GT_U16)U32_GET_FIELD_MAC(data,10,16);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (U32_GET_FIELD_MAC(data,27,2))
    {
        case 0:
            interfaceCfgPtr->ipMode = CPSS_IP_PROTOCOL_IPV4_E;
            break;
        case 1:
            interfaceCfgPtr->ipMode = CPSS_IP_PROTOCOL_IPV6_E;
            break;
        case 2:
            interfaceCfgPtr->ipMode = CPSS_IP_PROTOCOL_IPV4V6_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return rc;

}

/**
* @internal cpssDxChL2MllGetCntInterfaceCfg function
* @endinternal
*
* @brief   Gets a L2 mll counter set's bounded inteface.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] interfaceCfgPtr          - the L2 mll counter interface configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllGetCntInterfaceCfg
(
    IN  GT_U8                                            devNum,
    IN  GT_U32                                           mllCntSet,
    OUT CPSS_DXCH_L2MLL_COUNTER_SET_INTERFACE_CFG_STC    *interfaceCfgPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllGetCntInterfaceCfg);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllCntSet, interfaceCfgPtr));

    rc = internal_cpssDxChL2MllGetCntInterfaceCfg(devNum, mllCntSet, interfaceCfgPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllCntSet, interfaceCfgPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPortGroupMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllPortGroupMcCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
)
{
    GT_STATUS rc;                   /* return code                        */
    GT_U32    regAddr;              /* register address                   */

    CPSS_NULL_PTR_CHECK_MAC(mllOutMCPktsPtr);

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
     PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if(mllCntSet > 1)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* NOTE: this is ROC counter -- 'read only , clear' so no ability to 'set' it */
    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLOutMcPktsCntr[mllCntSet];

    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                0, 32, mllOutMCPktsPtr, NULL);
    return rc;

}

/**
* @internal cpssDxChL2MllPortGroupMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllPortGroupMcCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    IN  GT_U32                  mllCntSet,
    OUT GT_U32                  *mllOutMCPktsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPortGroupMcCntGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, mllCntSet, mllOutMCPktsPtr));

    rc = internal_cpssDxChL2MllPortGroupMcCntGet(devNum, portGroupsBmp, mllCntSet, mllOutMCPktsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, mllCntSet, mllOutMCPktsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChL2MllMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllMcCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
{
    return cpssDxChL2MllPortGroupMcCntGet(devNum,
                                        CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                        mllCntSet, mllOutMCPktsPtr);
}

/**
* @internal cpssDxChL2MllMcCntGet function
* @endinternal
*
* @brief   Get the L2MLL MC counter
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number.
* @param[in] mllCntSet                - l2 mll counter set
*                                      (APPLICABLE RANGES: 0..1)
*
* @param[out] mllOutMCPktsPtr          - According to the configuration of this cnt set, The
*                                      number of Multicast packets Duplicated by the
*                                      L2 MLL Engine
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - Illegal parameter in function called
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllMcCntGet
(
    IN  GT_U8   devNum,
    IN  GT_U32  mllCntSet,
    OUT GT_U32  *mllOutMCPktsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMcCntGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllCntSet, mllOutMCPktsPtr));

    rc = internal_cpssDxChL2MllMcCntGet(devNum, mllCntSet, mllOutMCPktsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllCntSet, mllOutMCPktsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllSilentDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *silentDropPktsPtr
)
{
    return cpssDxChL2MllPortGroupSilentDropCntGet(devNum,
                                         CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
                                         silentDropPktsPtr);
}

/**
* @internal cpssDxChL2MllSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSilentDropCntGet
(
    IN  GT_U8  devNum,
    OUT GT_U32 *silentDropPktsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllSilentDropCntGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, silentDropPktsPtr));

    rc = internal_cpssDxChL2MllSilentDropCntGet(devNum, silentDropPktsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, silentDropPktsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPortGroupSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllPortGroupSilentDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *silentDropPktsPtr
)
{
    GT_STATUS rc;                   /* return code                        */
    GT_U32    regAddr;              /* register address                   */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_MULTI_PORT_GROUPS_BMP_CHECK_MAC(devNum, portGroupsBmp);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(silentDropPktsPtr);


    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLOutInterfaceCntrs.L2MLLSilentDropCntr;

    rc = prvCpssPortGroupsBmpCounterSummary(devNum, portGroupsBmp, regAddr,
                                                0, 32, silentDropPktsPtr, NULL);
    return rc;
}

/**
* @internal cpssDxChL2MllPortGroupSilentDropCntGet function
* @endinternal
*
* @brief   Get the silent drops in the L2 MLL replication block.
*         A drop occurs for a packet that was:
*         - Replicated in the TTI/IP MLL
*         AND
*         - All the elements of the linked list are filtered
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*
* @param[out] silentDropPktsPtr        - (pointer to) the number of counted silent dropped packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllPortGroupSilentDropCntGet
(
    IN  GT_U8                   devNum,
    IN  GT_PORT_GROUPS_BMP      portGroupsBmp,
    OUT GT_U32                  *silentDropPktsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPortGroupSilentDropCntGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portGroupsBmp, silentDropPktsPtr));

    rc = internal_cpssDxChL2MllPortGroupSilentDropCntGet(devNum, portGroupsBmp, silentDropPktsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portGroupsBmp, silentDropPktsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllLttEntrySet function
* @endinternal
*
* @brief   Set L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
* @param[in] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllLttEntrySet
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
{
    GT_U32 hwValue; /* L2 MLL LTT entry HW format */
    GT_U32  numBits_mllIndex;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(lttEntryPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    numBits_mllIndex = PRV_CPSS_SIP_7_CHECK_MAC(devNum) ? 17 : 15;

    /* check index */
    MLL_LTT_INDEX_CHECK_MAC(devNum,index);
    /* check 'mllPointer' */
    MLL_NEXT_PTR_CHECK_MAC(devNum, lttEntryPtr->mllPointer);

    if( (lttEntryPtr->mllMaskProfileEnable == GT_TRUE) &&
         (lttEntryPtr->mllMaskProfile > 14) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    hwValue = lttEntryPtr->mllPointer << 1;
    switch(lttEntryPtr->entrySelector)
    {
        case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E:
            hwValue += 1;/*bit 0*/
            break;
        case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E:
            /* do nothing - bit 0 shoudl be 0 */
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if( GT_FALSE == lttEntryPtr->mllMaskProfileEnable )
    {
        U32_SET_FIELD_MAC(hwValue , 1 + numBits_mllIndex, 4, 0);
    }
    else /* GT_TRUE == lttEntryPtr->mllMaskProfileEnable */
    {
        U32_SET_FIELD_MAC(hwValue , 1 + numBits_mllIndex, 4, (lttEntryPtr->mllMaskProfile + 1));
    }

    return prvCpssDxChWriteTableEntry(devNum,
                                      CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E,
                                      index,
                                      &hwValue);
}

/**
* @internal cpssDxChL2MllLttEntrySet function
* @endinternal
*
* @brief   Set L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
* @param[in] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - LTT entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLttEntrySet
(
    IN GT_U8                            devNum,
    IN GT_U32                           index,
    IN CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllLttEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, lttEntryPtr));

    rc = internal_cpssDxChL2MllLttEntrySet(devNum, index, lttEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, lttEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllLttEntryGet function
* @endinternal
*
* @brief   Get L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
*
* @param[out] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllLttEntryGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           index,
    OUT CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
{
    GT_STATUS rc;   /* return code */
    GT_U32 hwValue; /* L2 MLL LTT entry HW format */
    GT_U32  numBits_mllIndex;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(lttEntryPtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    /* check index */
    MLL_LTT_INDEX_CHECK_MAC(devNum,index);
    numBits_mllIndex = PRV_CPSS_SIP_7_CHECK_MAC(devNum) ? 17 : 15;

    rc =  prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP5_TABLE_L2_MLL_LTT_E,
                                    index,
                                    &hwValue);
    if( GT_OK != rc )
    {
        return rc;
    }

    switch(U32_GET_FIELD_MAC(hwValue , 0, 1))
    {
        case 0:
            lttEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
            break;
        case 1:
            lttEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
            break;
    }

    lttEntryPtr->mllPointer = U32_GET_FIELD_MAC(hwValue , 1, numBits_mllIndex);

    lttEntryPtr->mllMaskProfile = U32_GET_FIELD_MAC(hwValue , 1 + numBits_mllIndex, 4);

    if( 0 == lttEntryPtr->mllMaskProfile )
    {
        lttEntryPtr->mllMaskProfileEnable = GT_FALSE;
    }
    else
    {
        lttEntryPtr->mllMaskProfileEnable = GT_TRUE;
        lttEntryPtr->mllMaskProfile -= 1;
    }

    return GT_OK;
}

/**
* @internal cpssDxChL2MllLttEntryGet function
* @endinternal
*
* @brief   Get L2 MLL Lookup Translation Table (LTT) entry.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] index                    - the LTT index. eVidx range.
*
* @param[out] lttEntryPtr              - (pointer to) L2 MLL LTT entry.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllLttEntryGet
(
    IN  GT_U8                            devNum,
    IN  GT_U32                           index,
    OUT CPSS_DXCH_L2_MLL_LTT_ENTRY_STC   *lttEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllLttEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, index, lttEntryPtr));

    rc = internal_cpssDxChL2MllLttEntryGet(devNum, index, lttEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, index, lttEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal xcatL2MllPairWrite function
* @endinternal
*
* @brief   Write L2 MLL pair entry to L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS xcatL2MllPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr
)
{
    GT_U32    hwAddr;
    GT_U32    hwData[PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD];
    GT_U32    hwMaskArr[PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD];
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(mllPairEntryPtr);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E))
    {
        if(mllPairEntryPtr->nextPointer >= fineTuningPtr->tableSize.mllPairs)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    retVal = xCatL2MllStruct2HwFormat(mllPairWriteForm,mllPairEntryPtr,hwData);
    if(retVal != GT_OK)
        return retVal;

    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllTableBase +
             mllPairEntryIndex * 0x10;

    cpssOsMemSet(hwMaskArr,0,sizeof(hwMaskArr));
    switch(mllPairWriteForm)
    {
    case CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E:
        if(mllPairEntryPtr->firstMllNode.last)
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/2,
                                            &hwData[0]);
        }
        else
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 4,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[1]);
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[0]);
        }
        hwMaskArr[0] = hwMaskArr[1] = 0xFFFFFFFF;
        break;

    case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
        if(mllPairEntryPtr->secondMllNode.last)
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 8,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/2,
                                            &hwData[2]);
        }
        else
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 12,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[3]);
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 8,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[2]);
        }
        hwMaskArr[2] = hwMaskArr[3] = 0xFFFFFFFF;
        break;

    case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
        if(mllPairEntryPtr->firstMllNode.last)
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD,
                                            &hwData[0]);
        }
        else if(mllPairEntryPtr->secondMllNode.last)
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 8,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/2,
                                            &hwData[2]);
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/2,
                                            &hwData[0]);
        }
        else
        {
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 12,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[3]);
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 8,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[2]);
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr + 4,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[1]);
            retVal = prvCpssHwPpWriteRam(devNum,
                                            hwAddr,
                                            PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD/4,
                                            &hwData[0]);
        }
        cpssOsMemSet(hwMaskArr,1,sizeof(hwMaskArr));
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(retVal != GT_OK)
    {
        return retVal;
    }
    return prvCpssDxChPortGroupShadowLineUpdateMasked(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS,
            CPSS_DXCH_XCAT_TABLE_MLL_E, mllPairEntryIndex, &hwData[0], &hwMaskArr[0]);
}

/**
* @internal xcatL2MllPairRead function
* @endinternal
*
* @brief   Read L2 MLL pair entry from L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS xcatL2MllPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr
)
{
    GT_U32    hwAddr;
    GT_U32    hwData[PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD];
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    CPSS_NULL_PTR_CHECK_MAC(mllPairEntryPtr);

    cpssOsMemSet(mllPairEntryPtr,0,sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    hwAddr = PRV_CPSS_DXCH_DEV_REGS_MAC(devNum)->ipMtRegs.mllTableBase +
             mllPairEntryIndex * 0x10;

    retVal = prvCpssHwPpReadRam(devNum, hwAddr,
                                    PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD, hwData);
    if(retVal != GT_OK)
        return retVal;

    /* read generic data */
    switch(mllPairReadForm)
    {
         case CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E:
            retVal = xCatL2MllHw2StructFormat(&hwData[0], &mllPairEntryPtr->firstMllNode);
            if(retVal != GT_OK)
                return retVal;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E:
            retVal = xCatL2MllHw2StructFormat(&hwData[2], &mllPairEntryPtr->secondMllNode);
            if(retVal != GT_OK)
                return retVal;
            break;

        case CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E:
            retVal = xCatL2MllHw2StructFormat(&hwData[0], &mllPairEntryPtr->firstMllNode);
            if(retVal != GT_OK)
                return retVal;

            retVal = xCatL2MllHw2StructFormat(&hwData[2], &mllPairEntryPtr->secondMllNode);
            if(retVal != GT_OK)
                return retVal;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((mllPairReadForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E) ||
        (mllPairReadForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        /* Next MLL Pointer */
        mllPairEntryPtr->nextPointer = (GT_U16)U32_GET_FIELD_MAC(hwData[3], 20, 12);
    }

    return retVal;
}


/**
* @internal internal_cpssDxChL2MllPairWrite function
* @endinternal
*
* @brief   Write L2 MLL pair entry to L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr
)
{
    GT_U32    sip5HwData[BITS_TO_WORDS_MAC(MAX_IP_MLL_ENTRY_BITS_SIZE_CNS)];
                                                   /* L2 MLL entry HW format */
    GT_U32    sip5HwMask[BITS_TO_WORDS_MAC(MAX_IP_MLL_ENTRY_BITS_SIZE_CNS)];
                                                   /* mask for sip4HwData */
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(mllPairEntryPtr);

    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return xcatL2MllPairWrite(devNum,mllPairEntryIndex,mllPairWriteForm,mllPairEntryPtr);
    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    cpssOsMemSet(sip5HwData, 0, sizeof(sip5HwData));
    cpssOsMemSet(sip5HwMask, 0, sizeof(sip5HwMask));


    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E))
    {
        if(mllPairEntryPtr->nextPointer >= fineTuningPtr->tableSize.mllPairs)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     }

    /* update the HW entry */
    retVal = sip5L2MllStruct2HwFormat(devNum,mllPairWriteForm,mllPairEntryPtr,
                                      sip5HwData, sip5HwMask);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    retVal = prvCpssDxChWriteTableEntryMasked(devNum,
                                              CPSS_DXCH_SIP5_TABLE_L2_MLL_E,
                                              mllPairEntryIndex,
                                              sip5HwData,
                                              sip5HwMask);
    return retVal;

}

/**
* @internal cpssDxChL2MllPairWrite function
* @endinternal
*
* @brief   Write L2 MLL pair entry to L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairWriteForm         - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - MLL entry's parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPairWrite
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       mllPairEntryIndex,
    IN CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT           mllPairWriteForm,
    IN CPSS_DXCH_L2_MLL_PAIR_STC                    *mllPairEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPairWrite);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllPairEntryIndex, mllPairWriteForm, mllPairEntryPtr));

    rc = internal_cpssDxChL2MllPairWrite(devNum, mllPairEntryIndex, mllPairWriteForm, mllPairEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllPairEntryIndex, mllPairWriteForm, mllPairEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPairRead function
* @endinternal
*
* @brief   Read L2 MLL pair entry from L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr
)
{
    GT_U32    sip5HwData[BITS_TO_WORDS_MAC(MAX_IP_MLL_ENTRY_BITS_SIZE_CNS)];
    GT_STATUS retVal = GT_OK;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(mllPairEntryPtr);

    cpssOsMemSet(sip5HwData, 0, sizeof(sip5HwData));

    if(PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_MAC(devNum) == GT_TRUE)
    {
        return xcatL2MllPairRead(devNum,mllPairEntryIndex,mllPairReadForm,mllPairEntryPtr);
    }

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPairEntryIndex >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    retVal = prvCpssDxChReadTableEntry(devNum,
                                        CPSS_DXCH_SIP5_TABLE_L2_MLL_E,
                                        mllPairEntryIndex,
                                        sip5HwData);
    if(retVal != GT_OK)
    {
        return retVal;
    }

    /* convert HW entry to SW format*/
    retVal = sip5L2MllHw2StructFormat(devNum,mllPairReadForm,mllPairEntryPtr,sip5HwData);
    return retVal;
}

/**
* @internal cpssDxChL2MllPairRead function
* @endinternal
*
* @brief   Read L2 MLL pair entry from L2 MLL Table.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] mllPairEntryIndex        - the mll Pair entry index
* @param[in] mllPairReadForm          - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
*
* @param[out] mllPairEntryPtr          - the L2 link list pair entry
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllPairRead
(
    IN  GT_U8                                       devNum,
    IN  GT_U32                                      mllPairEntryIndex,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairReadForm,
    OUT CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPairRead);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, mllPairEntryIndex, mllPairReadForm, mllPairEntryPtr));

    rc = internal_cpssDxChL2MllPairRead(devNum, mllPairEntryIndex, mllPairReadForm, mllPairEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, mllPairEntryIndex, mllPairReadForm, mllPairEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip5L2MllStruct2HwFormatSubSection function
* @endinternal
*
* @brief   SIP5 : This function converts a given L2 Mll struct to the hardware format.
*         convert only sub section (0 or 1)
*         for section 1 , it also set the 'next MLL'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] mllPairEntryPtr          - (points to ) the entry to be converted.
* @param[in] sectionIndex             - section index 0 or 1.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
* @param[out] hwMaskPtr                - the mask for hwDataPtr. Bits of HW entry fields
*                                      updated in the function are raised. Other mask's
*                                      bits are not affected
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5L2MllStruct2HwFormatSubSection
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr,
    OUT GT_U32                                      *hwMaskPtr,
    IN  GT_U32                                       sectionIndex
)
{
    GT_U32  value;/*tmp value to set to HW*/
    CPSS_DXCH_L2_MLL_ENTRY_STC *currSwPtr;/*pointer to current SW entry (in each HW line there are 2 entries)*/
    SIP5_L2_MLL_TABLE_FIELDS_ENT fieldId; /* L2 MLL table field name */
    if(sectionIndex)
    {
        fieldId = SIP5_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR_E;
        value   = mllPairEntryPtr->nextPointer;
        SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

        fieldId = SIP5_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR_E;
        switch(mllPairEntryPtr->entrySelector)
        {
            case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E:
                value = 0;
                break;
            case CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E:
                value = 1;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    }

    /* pointer to current SW info */
    currSwPtr = sectionIndex ? &(mllPairEntryPtr->secondMllNode):
                               &(mllPairEntryPtr->firstMllNode);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_LAST_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_LAST_0_E);
    value = currSwPtr->last;
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0_E);
    value = BOOL2BIT_MAC(currSwPtr->unknownUcFilterEnable);
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0_E);
    value = BOOL2BIT_MAC(currSwPtr->unregMcFilterEnable);
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);


    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0_E);
    value = BOOL2BIT_MAC(currSwPtr->bcFilterEnable);
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0_E);
    value = BOOL2BIT_MAC(currSwPtr->mcLocalSwitchingEnable);
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0_E);
    value = BOOL2BIT_MAC(currSwPtr->maxHopCountEnable);
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    if(value)
    {
        value = currSwPtr->maxOutgoingHopCount;
        fieldId = (sectionIndex ?
                   SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1_E :
                   SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0_E);
        SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    }

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0_E);
    value = currSwPtr->maskBitmap;
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_E);
    value = currSwPtr->ttlThreshold;
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);


    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0_E);
    value = (currSwPtr->bindToMllCounterEnable == GT_TRUE) ?
                (currSwPtr->mllCounterIndex + 1) :
                0;
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_0_E);
    value = ((currSwPtr->egressInterface.type == CPSS_INTERFACE_VIDX_E) ||
             (currSwPtr->egressInterface.type == CPSS_INTERFACE_VID_E)) ? 1 : 0;
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    if(value)
    {
        if(currSwPtr->egressInterface.type == CPSS_INTERFACE_VID_E)
        {
            value = 0xFFF;
        }
        else
        {
            value = currSwPtr->egressInterface.vidx;
        }

        fieldId = (sectionIndex ?
                   SIP5_L2_MLL_TABLE_FIELDS_VIDX_1_E :
                   SIP5_L2_MLL_TABLE_FIELDS_VIDX_0_E);
        SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
    }
    else
    {
        fieldId = (sectionIndex ?
                   SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E :
                   SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E);
        value = (currSwPtr->egressInterface.type == CPSS_INTERFACE_TRUNK_E) ? 1 :0;
        SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

        if(currSwPtr->egressInterface.type == CPSS_INTERFACE_TRUNK_E)
        {
            fieldId = (sectionIndex ?
                       SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E :
                       SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E);
            value = currSwPtr->egressInterface.trunkId;
            SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
        }
        else
        if(currSwPtr->egressInterface.type == CPSS_INTERFACE_PORT_E)
        {
            if(PRV_CPSS_DXCH_IS_VPORT(devNum,currSwPtr->egressInterface.devPort.portNum))
            {
                fieldId = (sectionIndex ?
                           SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1_E :
                           SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0_E);
                value = 1;
                SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

                fieldId = (sectionIndex ?
                           SIP7_L2_MLL_TABLE_FIELDS_VPORT_1_E :
                           SIP7_L2_MLL_TABLE_FIELDS_VPORT_0_E);
                value = currSwPtr->egressInterface.devPort.portNum;
                SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
            }
            else
            {
                fieldId = (sectionIndex ?
                           SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_1_E :
                           SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_0_E);
                value = currSwPtr->egressInterface.devPort.portNum;
                SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

                fieldId = (sectionIndex ?
                           SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_1_E :
                           SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_0_E);
                value = currSwPtr->egressInterface.devPort.hwDevNum;
                SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);
            }
        }
        else
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

    }

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0_E);
    value = BOOL2BIT_MAC(currSwPtr->onePlusOneFilteringEnable);
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    fieldId = (sectionIndex ?
               SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_1_E :
               SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_0_E);
    value = currSwPtr->meshId;
    SIP5_L2_MLL_FIELD_AND_MASK_SET_MAC(devNum, hwDataPtr, hwMaskPtr, fieldId, value);

    return GT_OK;
}

/**
* @internal sip5L2MllHw2StructFormatSubSection function
* @endinternal
*
* @brief   SIP5 : This function converts a given L2 Mll hardware format to struct (SW) .
*         convert only sub section (0 or 1)
*         for section 1 , it also set the 'next MLL'
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; xCat2.
*
* @param[in] devNum                   - device number
* @param[in] hwDataPtr                - The entry in the HW format , the need to be converted to SW.
* @param[in] sectionIndex             - section index 0 or 1.
*
* @param[out] mllPairEntryPtr          - (points to ) the converted entry.
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5L2MllHw2StructFormatSubSection
(
    IN  GT_U8                                       devNum,
    OUT  CPSS_DXCH_L2_MLL_PAIR_STC                  *mllPairEntryPtr,
    IN GT_U32                                       *hwDataPtr,
    IN  GT_U32                                       sectionIndex
)
{
    GT_U32  value;/*tmp value to get from HW*/
    CPSS_DXCH_L2_MLL_ENTRY_STC *currSwPtr;/*pointer to current SW entry (in each HW line there are 2 entries)*/
    GT_U32  isVPort;/* sip7 : is vPort ? */

    if(sectionIndex)
    {
        SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            SIP5_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR_E
            ,value);
        mllPairEntryPtr->nextPointer = value;

        SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            SIP5_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR_E
            ,value);

        switch (value)
        {
            case 0:
                mllPairEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
                break;
            case 1:
                mllPairEntryPtr->entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_SECOND_MLL_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* pointer to current SW info */
    currSwPtr = sectionIndex ? &(mllPairEntryPtr->secondMllNode) :
                               &(mllPairEntryPtr->firstMllNode) ;

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_LAST_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_LAST_0_E
        ,value);
    currSwPtr->last = value;

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0_E
        ,value);
    currSwPtr->unknownUcFilterEnable = BIT2BOOL_MAC(value);

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0_E
        ,value);
    currSwPtr->unregMcFilterEnable = BIT2BOOL_MAC(value);

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0_E
        ,value);
    currSwPtr->bcFilterEnable = BIT2BOOL_MAC(value);

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0_E
        ,value);
    currSwPtr->mcLocalSwitchingEnable = BIT2BOOL_MAC(value);

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0_E
        ,value);
    currSwPtr->maxHopCountEnable = BIT2BOOL_MAC(value);

    if(value)
    {
        SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1_E :
                SIP5_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0_E
            ,value);
        currSwPtr->maxOutgoingHopCount = value;
    }

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0_E
        ,value);
    currSwPtr->maskBitmap = value;

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_E
        ,value);
    currSwPtr->ttlThreshold = value;

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0_E
        ,value);
    if(value)
    {
        currSwPtr->bindToMllCounterEnable = GT_TRUE;
        currSwPtr->mllCounterIndex = value - 1;
    }
    else
    {
        currSwPtr->bindToMllCounterEnable = GT_FALSE;
    }

    value = ((currSwPtr->egressInterface.type == CPSS_INTERFACE_VIDX_E) ||
             (currSwPtr->egressInterface.type == CPSS_INTERFACE_VID_E)) ? 1 : 0;
    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_USE_VIDX_0_E
        ,value);

    if(value)
    {
        SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_L2_MLL_TABLE_FIELDS_VIDX_1_E :
                SIP5_L2_MLL_TABLE_FIELDS_VIDX_0_E
            ,value);

        if(value == 0xFFF)
        {
            currSwPtr->egressInterface.type = CPSS_INTERFACE_VID_E;
            currSwPtr->egressInterface.vlanId = 0xFFF;
        }
        else
        {
            currSwPtr->egressInterface.type = CPSS_INTERFACE_VIDX_E;
            currSwPtr->egressInterface.vidx = value;
        }
    }
    else
    {
        SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
            sectionIndex ?
                SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1_E :
                SIP5_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0_E
            ,value);

        if(value)
        {
            currSwPtr->egressInterface.type = CPSS_INTERFACE_TRUNK_E;
            SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                sectionIndex ?
                    SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1_E :
                    SIP5_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0_E
                ,value);
            currSwPtr->egressInterface.trunkId = (GT_TRUNK_ID)value;
        }
        else
        {
            if(PRV_CPSS_SIP_7_CHECK_MAC(devNum))
            {
                SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                    sectionIndex ?
                        SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1_E :
                        SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0_E
                    ,value);
                isVPort = value;
            }
            else
            {
                isVPort = 0;
            }

            currSwPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
            if(isVPort)
            {
                /* vPort */
                SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                    sectionIndex ?
                        SIP7_L2_MLL_TABLE_FIELDS_VPORT_1_E :
                        SIP7_L2_MLL_TABLE_FIELDS_VPORT_0_E
                    ,value);
                currSwPtr->egressInterface.devPort.portNum = value;
                currSwPtr->egressInterface.devPort.hwDevNum = CPSS_PARAM_NOT_USED_CNS;
            }
            else
            {
                /* Port */
                SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                    sectionIndex ?
                        SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_1_E :
                        SIP5_L2_MLL_TABLE_FIELDS_TRG_EPORT_0_E
                    ,value);
                currSwPtr->egressInterface.devPort.portNum = value;

                SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
                    sectionIndex ?
                        SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_1_E :
                        SIP5_L2_MLL_TABLE_FIELDS_TRG_DEV_0_E
                    ,value);
                currSwPtr->egressInterface.devPort.hwDevNum = value;
            }
        }
    }

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0_E
        ,value);
    currSwPtr->onePlusOneFilteringEnable = BIT2BOOL_MAC(value);

    SIP5_L2_MLL_FIELD_GET_MAC(devNum,hwDataPtr,
        sectionIndex ?
            SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_1_E :
            SIP5_L2_MLL_TABLE_FIELDS_MESH_ID_0_E
        ,value);
    currSwPtr->meshId = value;

    return GT_OK;
}

/**
* @internal sip5L2MllStruct2HwFormat function
* @endinternal
*
* @brief   SIP5 : This function converts a given L2 Mll struct to the hardware format.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; xCat2.
*
* @param[in] mllPairForm              - the way to write the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] mllPairEntryPtr          - (points to ) the entry to be converted.
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
* @param[out] hwMaskPtr                - the mask for hwDataPtr. Bits of HW entry fields
*                                      updated in the function are raised. Other mask's
*                                      bits are not affected
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5L2MllStruct2HwFormat
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    IN  CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr,
    OUT GT_U32                                      *hwMaskPtr
)
{
    GT_STATUS rc = GT_BAD_PARAM;/* indication that none of the cases used */

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 0 */
        rc = sip5L2MllStruct2HwFormatSubSection(
            devNum, mllPairEntryPtr, hwDataPtr, hwMaskPtr, 0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 1 + next pointer */
        rc = sip5L2MllStruct2HwFormatSubSection(
            devNum, mllPairEntryPtr, hwDataPtr, hwMaskPtr, 1);
    }

    return rc;
}


/**
* @internal sip5L2MllHw2StructFormat function
* @endinternal
*
* @brief   SIP5 : This function converts a given L2 Mll hardware format to struct (SW) .
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; xCat2.
*
* @param[in] devNum                   - decvice number
* @param[in] mllPairForm              - the way to read the Mll pair, first part only/
*                                      second + next pointer only/ whole Mll pair
* @param[in] hwDataPtr                - The entry in the HW format , the need to be converted to SW.
*
* @param[out] mllPairEntryPtr          - (points to ) the converted entry.
*
* @retval GT_OK                    - on all valid parameters.
*/
static GT_STATUS sip5L2MllHw2StructFormat
(
    IN GT_U8                                        devNum,
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairForm,
    OUT  CPSS_DXCH_L2_MLL_PAIR_STC                  *mllPairEntryPtr,
    IN GT_U32                                       *hwDataPtr
)
{
    GT_STATUS rc = GT_BAD_PARAM;/* indication that none of the cases used */

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 0 */
        rc = sip5L2MllHw2StructFormatSubSection(
                devNum,mllPairEntryPtr,hwDataPtr,0);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    if(mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E ||
       mllPairForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E)
    {
        /* build section 1 + next pointer */
        rc = sip5L2MllHw2StructFormatSubSection(
                devNum,mllPairEntryPtr,hwDataPtr,1);
    }

    return rc;
}

/**
* @internal xCatL2MllStruct2HwFormat function
* @endinternal
*
* @brief   This function converts a given L2 Mll struct to the hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*
* @param[out] hwDataPtr                - The entry in the HW format representation.
*
* @retval GT_OK                    - on all valid parameters.
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS xCatL2MllStruct2HwFormat
(
    IN  CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT          mllPairWriteForm,
    IN  CPSS_DXCH_L2_MLL_PAIR_STC                   *mllPairEntryPtr,
    OUT GT_U32                                      *hwDataPtr
)
{
    /* clear hw data */
    cpssOsMemSet(hwDataPtr, 0, sizeof(GT_U32)*PRV_CPSS_XCAT_SIZE_OF_MLL_NODE_IN_WORD);

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 0, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.last));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 1, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.tunnelStartEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 2, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.unknownUcFilterEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 3, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.unregMcFilterEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 4, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.bcFilterEnable));
        if (mllPairEntryPtr->firstMllNode.meshId >= BIT_2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 5, 2, mllPairEntryPtr->firstMllNode.meshId);
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 7, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.onePlusOneFilteringEnable));
        switch(mllPairEntryPtr->firstMllNode.egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            if( (mllPairEntryPtr->firstMllNode.egressInterface.devPort.hwDevNum >= BIT_5) ||
                (mllPairEntryPtr->firstMllNode.egressInterface.devPort.portNum >= BIT_6) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 16, 1, GT_FALSE);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 17, 6, mllPairEntryPtr->firstMllNode.egressInterface.devPort.portNum);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 23, 5, mllPairEntryPtr->firstMllNode.egressInterface.devPort.hwDevNum);

            break;
        case CPSS_INTERFACE_TRUNK_E:
            if(mllPairEntryPtr->firstMllNode.egressInterface.trunkId >= BIT_7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 16, 1, GT_TRUE);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 21, 7, mllPairEntryPtr->firstMllNode.egressInterface.trunkId);
            break;
        case CPSS_INTERFACE_VIDX_E:
            if(mllPairEntryPtr->firstMllNode.egressInterface.vidx >= BIT_13)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 15, 1, GT_TRUE);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 16, 13, mllPairEntryPtr->firstMllNode.egressInterface.vidx);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (mllPairEntryPtr->firstMllNode.tunnelStartEnable)
        {
            if (mllPairEntryPtr->firstMllNode.tunnelStartPointer >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 32, 10, (mllPairEntryPtr->firstMllNode.tunnelStartPointer & 0x3FF));
            switch (mllPairEntryPtr->firstMllNode.tunnelStartPassengerType)
            {
            case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 42, 1, 0);
                break;
            case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 42, 1, 1);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 52, 2, ((mllPairEntryPtr->firstMllNode.tunnelStartPointer & 0xC00) >> 10));
        }
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 50, 1, BOOL2BIT_MAC(mllPairEntryPtr->firstMllNode.mcLocalSwitchingEnable));

    }

    if((mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E) ||
       (mllPairWriteForm == CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E))
    {
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 64, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.last));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 65, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.tunnelStartEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 66, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.unknownUcFilterEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 67, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.unregMcFilterEnable));
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 68, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.bcFilterEnable));

        if (mllPairEntryPtr->secondMllNode.meshId >= BIT_2)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 69, 2, mllPairEntryPtr->secondMllNode.meshId);

        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 71, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.onePlusOneFilteringEnable));
        switch(mllPairEntryPtr->secondMllNode.egressInterface.type)
        {
        case CPSS_INTERFACE_PORT_E:
            if( (mllPairEntryPtr->secondMllNode.egressInterface.devPort.hwDevNum >= BIT_5) ||
                (mllPairEntryPtr->secondMllNode.egressInterface.devPort.portNum >= BIT_6) )
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 80, 1, GT_FALSE);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 81, 6, mllPairEntryPtr->secondMllNode.egressInterface.devPort.portNum);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 87, 5, mllPairEntryPtr->secondMllNode.egressInterface.devPort.hwDevNum);

            break;
        case CPSS_INTERFACE_TRUNK_E:
            if(mllPairEntryPtr->secondMllNode.egressInterface.trunkId >= BIT_7)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 80, 1, GT_TRUE);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 85, 7, mllPairEntryPtr->secondMllNode.egressInterface.trunkId);
            break;
        case CPSS_INTERFACE_VIDX_E:
            if(mllPairEntryPtr->secondMllNode.egressInterface.vidx >= BIT_13)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 79, 1, GT_TRUE);
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 80, 13, mllPairEntryPtr->secondMllNode.egressInterface.vidx);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
        if (mllPairEntryPtr->secondMllNode.tunnelStartEnable)
        {
            if (mllPairEntryPtr->secondMllNode.tunnelStartPointer >= BIT_12)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 96, 10, (mllPairEntryPtr->secondMllNode.tunnelStartPointer & 0x3FF));
            switch (mllPairEntryPtr->secondMllNode.tunnelStartPassengerType)
            {
            case CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E:
                U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 106, 1, 0);
                break;
            case CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E:
                U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 106, 1, 1);
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
            U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 54, 2, ((mllPairEntryPtr->firstMllNode.tunnelStartPointer & 0xC00) >> 10));
        }
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 114, 1, BOOL2BIT_MAC(mllPairEntryPtr->secondMllNode.mcLocalSwitchingEnable));
        if (mllPairEntryPtr->nextPointer >= BIT_12)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
        U32_SET_FIELD_IN_ENTRY_MAC(hwDataPtr, 116, 12, mllPairEntryPtr->nextPointer);

    }

    return GT_OK;
}

/**
* @internal xCatL2MllHw2StructFormat function
* @endinternal
*
* @brief   This function converts a given ip Mll struct to the hardware format.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
*
* @param[out] mllEntryPtr              - The entry to be converted.
*
* @retval GT_OK                    - on all valid parameters.
* @retval GT_BAD_STATE             - on invalid hardware value read
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS xCatL2MllHw2StructFormat
(
    IN   GT_U32                       *hwDataPtr,
    OUT  CPSS_DXCH_L2_MLL_ENTRY_STC   *mllEntryPtr
)
{
    GT_U32 value;         /* field value */

    /* clear output data */
    cpssOsMemSet(mllEntryPtr, 0, sizeof(CPSS_DXCH_L2_MLL_ENTRY_STC));

    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 0, 1, value);
    mllEntryPtr->last = BIT2BOOL_MAC(value);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 1, 1, value);
    mllEntryPtr->tunnelStartEnable = BIT2BOOL_MAC(value);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 2, 1, value);
    mllEntryPtr->unknownUcFilterEnable = BIT2BOOL_MAC(value);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 3, 1, value);
    mllEntryPtr->unregMcFilterEnable = BIT2BOOL_MAC(value);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 4, 1, value);
    mllEntryPtr->bcFilterEnable = BIT2BOOL_MAC(value);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 5, 2, value);
    mllEntryPtr->meshId = value;
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 7, 1, value);
    mllEntryPtr->onePlusOneFilteringEnable = BIT2BOOL_MAC(value);
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 15, 1, value);
    if (value)
    {
        mllEntryPtr->egressInterface.type = CPSS_INTERFACE_VIDX_E;
        U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 16, 13, value);
        mllEntryPtr->egressInterface.vidx = value;
    }
    else
    {
        U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 16, 1, value);
        if (value)
        {
            mllEntryPtr->egressInterface.type = CPSS_INTERFACE_TRUNK_E;
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 21, 7, value);
            mllEntryPtr->egressInterface.trunkId = (GT_TRUNK_ID)value;
        }
        else
        {
            mllEntryPtr->egressInterface.type = CPSS_INTERFACE_PORT_E;
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 17, 6, value);
            mllEntryPtr->egressInterface.devPort.portNum = (GT_U8)value;
            U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 23, 5, value);
            mllEntryPtr->egressInterface.devPort.hwDevNum = (GT_U8)value;
        }
    }
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 32, 10, value);
    mllEntryPtr->tunnelStartPointer = value;
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 52, 2, value);
    mllEntryPtr->tunnelStartPointer |= value << 10;
    if (mllEntryPtr->tunnelStartPointer)
    {
        mllEntryPtr->tunnelStartEnable = GT_TRUE;
        U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 42, 1, value);
        switch (value)
        {
        case 0:
            mllEntryPtr->tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
            break;
        case 1:
            mllEntryPtr->tunnelStartPassengerType = CPSS_DXCH_TUNNEL_PASSENGER_OTHER_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
        }
    }
    U32_GET_FIELD_IN_ENTRY_MAC(hwDataPtr, 50, 1, value);
    mllEntryPtr->mcLocalSwitchingEnable = BIT2BOOL_MAC(value);

    return GT_OK;
}


/**
* @internal internal_cpssDxChL2MllMultiTargetPortEnableSet function
* @endinternal
*
* @brief   Enable multi-target port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - multi-target port mapping status:
*                                      GT_TRUE: enable multi-target port mapping
*                                      GT_FALSE: disable multi-target port mapping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_U32      startBit;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLReplicatedTrafficCtrl;
    startBit = PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) ? 14 : 11;

    hwData = BOOL2BIT_MAC(enable);
    return prvCpssHwPpSetRegField(devNum, regAddr, startBit, 1, hwData);
}

/**
* @internal cpssDxChL2MllMultiTargetPortEnableSet function
* @endinternal
*
* @brief   Enable multi-target port mapping
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - multi-target port mapping status:
*                                      GT_TRUE: enable multi-target port mapping
*                                      GT_FALSE: disable multi-target port mapping
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortEnableSet
(
    IN  GT_U8                       devNum,
    IN  GT_BOOL                     enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable));

    rc = internal_cpssDxChL2MllMultiTargetPortEnableSet(devNum, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllMultiTargetPortEnableGet function
* @endinternal
*
* @brief   Get the multi-target port mapping enabling status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) multi-target port mapping status:
*                                      GT_TRUE: multi-target port mapping is enabled
*                                      GT_FALSE: multi-target port mapping is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_U32      regAddr;
    GT_U32      hwData;
    GT_STATUS   rc;
    GT_U32      startBit;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.MLLReplicatedTrafficCtrl;
    startBit = PRV_CPSS_SIP_6_30_CHECK_MAC(devNum) ? 14 : 11;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, startBit, 1, &hwData);
    if (rc == GT_OK)
    {
        *enablePtr = BIT2BOOL_MAC(hwData);
    }
    return rc;
}

/**
* @internal cpssDxChL2MllMultiTargetPortEnableGet function
* @endinternal
*
* @brief   Get the multi-target port mapping enabling status
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to) multi-target port mapping status:
*                                      GT_TRUE: multi-target port mapping is enabled
*                                      GT_FALSE: multi-target port mapping is disabled
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortEnableGet
(
    IN  GT_U8                       devNum,
    OUT GT_BOOL                     *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr));

    rc = internal_cpssDxChL2MllMultiTargetPortEnableGet(devNum, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllMultiTargetPortRangeSet function
* @endinternal
*
* @brief   Set the multi-target port Range config
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portRangeConfigPtr    - (pointer to) multi-target port range config
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortRangeSet
(
    IN  GT_U8                                          devNum,
    IN  CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC  *portRangeConfigPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
            CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(portRangeConfigPtr);
    if ((portRangeConfigPtr->minValue > PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VALUE_CNS) ||
        (portRangeConfigPtr->maxValue > PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VALUE_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetGlobalEPortMinValue;
    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, portRangeConfigPtr->minValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetGlobalEPortMaxValue;
    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, portRangeConfigPtr->maxValue);
}

/**
* @internal cpssDxChL2MllMultiTargetPortRangeSet function
* @endinternal
*
* @brief   Set the multi-target port Range config
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] portRangeConfigPtr    - (pointer to) multi-target port range config
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - parameter is out of range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortRangeSet
(
    IN  GT_U8                                       devNum,
    IN  CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC  *portRangeConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortRangeSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portRangeConfigPtr));

    rc = internal_cpssDxChL2MllMultiTargetPortRangeSet(devNum, portRangeConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portRangeConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllMultiTargetPortRangeGet function
* @endinternal
*
* @brief   Get the multi-target port range config
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[out] portRangeConfigPtr   - (pointer to) multi-target port range config
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortRangeGet
(
    IN  GT_U8                                          devNum,
    OUT CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC  *portRangeConfigPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
            CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);
    CPSS_NULL_PTR_CHECK_MAC(portRangeConfigPtr);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetGlobalEPortMinValue;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, &portRangeConfigPtr->minValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetGlobalEPortMaxValue;
    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, &portRangeConfigPtr->maxValue);
}

/**
* @internal cpssDxChL2MllMultiTargetPortRangeGet function
* @endinternal
*
* @brief   Get the multi-target port range config
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[out] portRangeConfigPtr   - (pointer to) multi-target port range config
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortRangeGet
(
    IN  GT_U8                                          devNum,
    OUT CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC  *portRangeConfigPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortRangeGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portRangeConfigPtr));

    rc = internal_cpssDxChL2MllMultiTargetPortRangeGet(devNum, portRangeConfigPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portRangeConfigPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}




/**
* @internal sip6_cpssDxChL2MllMultiTargetPortSet function
* @endinternal
*
* @brief   Set the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - multi-target port value
*                                      (APPLICABLE RANGES: 0..0x1ffff)
* @param[in] mask                     - multi-target port mask
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range value or mask
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS sip6_cpssDxChL2MllMultiTargetPortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      value,
    IN  GT_U32                      mask
)
{
    GT_STATUS                                      rc;
    CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC   portRangeConfig;

    /* check that can convert mask to range */
    rc = prvCpssDxChSip6GlobalEportMaskCheck(devNum,mask,
        value,
        &portRangeConfig.minValue,
        &portRangeConfig.maxValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    return internal_cpssDxChL2MllMultiTargetPortRangeSet(devNum, &portRangeConfig);
}

/**
* @internal internal_cpssDxChL2MllMultiTargetPortSet function
* @endinternal
*
* @brief   Set the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - multi-target port value
*                                      (APPLICABLE RANGES: 0..0x1ffff)
* @param[in] mask                     - multi-target port mask
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range value or mask
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      value,
    IN  GT_U32                      mask
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if ((value > PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VALUE_CNS) ||
        (mask > PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VALUE_CNS))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return sip6_cpssDxChL2MllMultiTargetPortSet(devNum,value,mask);
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetEPortValue;

    rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, value);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetEPortMask;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, mask);
}

/**
* @internal cpssDxChL2MllMultiTargetPortSet function
* @endinternal
*
* @brief   Set the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] value                    - multi-target port value
*                                      (APPLICABLE RANGES: 0..0x1ffff)
* @param[in] mask                     - multi-target port mask
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range value or mask
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      value,
    IN  GT_U32                      mask
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, value, mask));

    rc = internal_cpssDxChL2MllMultiTargetPortSet(devNum, value, mask);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, value, mask));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal sip6_cpssDxChL2MllMultiTargetPortGet function
* @endinternal
*
* @brief   Get the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) multi-target port value
* @param[out] maskPtr                  - (pointer to) multi-target port mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS sip6_cpssDxChL2MllMultiTargetPortGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *valuePtr,
    OUT GT_U32                      *maskPtr
)
{
    GT_STATUS                                      rc;
    CPSS_DXCH_L2_MLL_MULTI_TARGET_PORT_RANGE_STC   portRangeConfig;

    rc = internal_cpssDxChL2MllMultiTargetPortRangeGet(devNum, &portRangeConfig);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* check that can convert mask to range */
    return prvCpssDxChSip6GlobalEportMaskGet(portRangeConfig.minValue,
                                 portRangeConfig.maxValue,
                                 valuePtr,
                                 maskPtr);
}

/**
* @internal internal_cpssDxChL2MllMultiTargetPortGet function
* @endinternal
*
* @brief   Get the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) multi-target port value
* @param[out] maskPtr                  - (pointer to) multi-target port mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *valuePtr,
    OUT GT_U32                      *maskPtr
)
{
    GT_U32      regAddr;
    GT_STATUS   rc;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(valuePtr);
    CPSS_NULL_PTR_CHECK_MAC(maskPtr);

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        return sip6_cpssDxChL2MllMultiTargetPortGet(devNum,valuePtr,maskPtr);
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetEPortValue;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, valuePtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetEPortMask;

    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, maskPtr);
}

/**
* @internal cpssDxChL2MllMultiTargetPortGet function
* @endinternal
*
* @brief   Get the multi-target port value and mask
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] valuePtr                 - (pointer to) multi-target port value
* @param[out] maskPtr                  - (pointer to) multi-target port mask
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *valuePtr,
    OUT GT_U32                      *maskPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, valuePtr, maskPtr));

    rc = internal_cpssDxChL2MllMultiTargetPortGet(devNum, valuePtr, maskPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, valuePtr, maskPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChL2MllMultiTargetPortBaseSet function
* @endinternal
*
* @brief   Set the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portBase                 - multi-target port base
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range port base
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portBase
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (portBase > PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VALUE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetEPortBase;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 17, portBase);
}

/**
* @internal cpssDxChL2MllMultiTargetPortBaseSet function
* @endinternal
*
* @brief   Set the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] portBase                 - multi-target port base
*                                      (APPLICABLE RANGES: 0..0x1ffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range port base
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portBase));

    rc = internal_cpssDxChL2MllMultiTargetPortBaseSet(devNum, portBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllMultiTargetPortBaseGet function
* @endinternal
*
* @brief   Get the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portBasePtr              - (pointer to) multi-target port base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllMultiTargetPortBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *portBasePtr
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(portBasePtr);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.multiTargetEPortBase;

    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 17, portBasePtr);
}

/**
* @internal cpssDxChL2MllMultiTargetPortBaseGet function
* @endinternal
*
* @brief   Get the multi-target port base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] portBasePtr              - (pointer to) multi-target port base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllMultiTargetPortBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *portBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllMultiTargetPortBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, portBasePtr));

    rc = internal_cpssDxChL2MllMultiTargetPortBaseGet(devNum, portBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, portBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPortToVidxBaseSet function
* @endinternal
*
* @brief   Set the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxBase                 - port VIDX base
*                                      (APPLICABLE RANGES: 0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range vidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllPortToVidxBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      vidxBase
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if (vidxBase > PRV_CPSS_MAX_NUM_OF_MULTI_TARGET_PORT_VIDX_BASE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.ePortToEVIDXBase;

    return prvCpssHwPpSetRegField(devNum, regAddr, 0, 16, vidxBase);
}

/**
* @internal cpssDxChL2MllPortToVidxBaseSet function
* @endinternal
*
* @brief   Set the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] vidxBase                 - port VIDX base
*                                      (APPLICABLE RANGES: 0..0xffff)
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_OUT_OF_RANGE          - on out of range vidx
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllPortToVidxBaseSet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      vidxBase
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPortToVidxBaseSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidxBase));

    rc = internal_cpssDxChL2MllPortToVidxBaseSet(devNum, vidxBase);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidxBase));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllPortToVidxBaseGet function
* @endinternal
*
* @brief   Get the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxBasePtr              - (pointer to) port VIDX base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
static GT_STATUS internal_cpssDxChL2MllPortToVidxBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *vidxBasePtr
)
{
    GT_U32      regAddr;

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);
    CPSS_NULL_PTR_CHECK_MAC(vidxBasePtr);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).multiTargetEPortMap.ePortToEVIDXBase;

    return prvCpssHwPpGetRegField(devNum, regAddr, 0, 16, vidxBasePtr);
}

/**
* @internal cpssDxChL2MllPortToVidxBaseGet function
* @endinternal
*
* @brief   Get the port to VIDX base
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] vidxBasePtr              - (pointer to) port VIDX base
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The function is useful when unicast traffic is forwarded over multiple
*       interfaces, for example, in 1+1 protection schemes where traffic is
*       forwarded to a logical unicast target that is represented by the target
*       eport but is physically sent over 2 paths: the working path and the
*       protection path
*
*/
GT_STATUS cpssDxChL2MllPortToVidxBaseGet
(
    IN  GT_U8                       devNum,
    OUT GT_U32                      *vidxBasePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllPortToVidxBaseGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidxBasePtr));

    rc = internal_cpssDxChL2MllPortToVidxBaseGet(devNum, vidxBasePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidxBasePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChL2MllSourceBasedFilteringConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[in] meshIdOffset             - The MESH ID assigned to a packet is conveyed in the SrcID
*                                      assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] meshIdSize               - Specifies the number of bits that are used for
*                                      the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..8)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on wrong meshIdOffset and meshIdSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllSourceBasedFilteringConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable,
    IN GT_U32                   meshIdOffset,
    IN GT_U32                   meshIdSize
)
{
    GT_STATUS   rc;         /* return code      */
    GT_U32      regAddr;    /* register address */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    if((meshIdOffset > 11) || (meshIdSize > 8))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.sourceBasedL2MLLFiltering;

   rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 4, meshIdOffset);
    if(rc != GT_OK)
        return rc;

    if (enable==GT_FALSE)
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 4, 0);
    }
    else
    {
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 4, 4, meshIdSize);
    }

    return rc;
}

/**
* @internal cpssDxChL2MllSourceBasedFilteringConfigurationSet function
* @endinternal
*
* @brief   Set configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[in] meshIdOffset             - The MESH ID assigned to a packet is conveyed in the SrcID
*                                      assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[in] meshIdSize               - Specifies the number of bits that are used for
*                                      the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..8)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_OUT_OF_RANGE          - on wrong meshIdOffset and meshIdSize
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSourceBasedFilteringConfigurationSet
(
    IN GT_U8                    devNum,
    IN GT_BOOL                  enable,
    IN GT_U32                   meshIdOffset,
    IN GT_U32                   meshIdSize
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllSourceBasedFilteringConfigurationSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enable, meshIdOffset, meshIdSize));

    rc = internal_cpssDxChL2MllSourceBasedFilteringConfigurationSet(devNum, enable, meshIdOffset, meshIdSize);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enable, meshIdOffset, meshIdSize));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllSourceBasedFilteringConfigurationGet function
* @endinternal
*
* @brief   Get configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[out] meshIdOffsetPtr          - (pointer to) The MESH ID assigned to a packet is conveyed
*                                      in the SrcID assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[out] meshIdSizePtr            - (pointer to) Specifies the number of bits that are used
*                                      for the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..8)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChL2MllSourceBasedFilteringConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_U32                   *meshIdOffsetPtr,
    OUT GT_U32                   *meshIdSizePtr
)
{
    GT_STATUS rc;       /* return code      */
    GT_U32  regAddr;    /* register address */
    GT_U32  regData;    /* register data    */

    /* check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    CPSS_NULL_PTR_CHECK_MAC(meshIdOffsetPtr);
    CPSS_NULL_PTR_CHECK_MAC(meshIdSizePtr);
    PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_SUPPORTED_CHECK_MAC(devNum);

    regAddr = PRV_DXCH_REG1_UNIT_MLL_MAC(devNum).MLLGlobalCtrl.sourceBasedL2MLLFiltering;

    rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 8, &regData);
    if( rc != GT_OK )
    {
        return rc;
    }

    *meshIdOffsetPtr = U32_GET_FIELD_MAC(regData,0,4);
    *meshIdSizePtr = U32_GET_FIELD_MAC(regData,4,4);
    if(*meshIdSizePtr==0)
    {
        *enablePtr = GT_FALSE;
    }
    else
    {
        *enablePtr = GT_TRUE;
    }

    return rc;
}

/**
* @internal cpssDxChL2MllSourceBasedFilteringConfigurationGet function
* @endinternal
*
* @brief   Get configuration for Source Based L2 MLL Filtering.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_FALSE: MESH ID filtering is not enabled
*                                      GT_TRUE:  MESH ID filtering is enabled
* @param[out] meshIdOffsetPtr          - (pointer to) The MESH ID assigned to a packet is conveyed
*                                      in the SrcID assigned to the packet.
*                                      This configuration specifies the location of the LSB of
*                                      the MESH ID in the SrcID field assigned to a packet
*                                      (APPLICABLE RANGES: 0..11)
* @param[out] meshIdSizePtr            - (pointer to) Specifies the number of bits that are used
*                                      for the MESH ID in the SrcID field.
*                                      Together with <MESH ID offset>, the MESH ID assigned to
*                                      a packet can be extracted:
*                                      Packet's MESH ID = <SST ID> (<MESH ID size> - 1 +
*                                      <MESH ID offset> : <MESH ID offset>)
*                                      (APPLICABLE RANGES: 1..8)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChL2MllSourceBasedFilteringConfigurationGet
(
    IN  GT_U8                    devNum,
    OUT GT_BOOL                  *enablePtr,
    OUT GT_U32                   *meshIdOffsetPtr,
    OUT GT_U32                   *meshIdSizePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllSourceBasedFilteringConfigurationGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, enablePtr, meshIdOffsetPtr, meshIdSizePtr));

    rc = internal_cpssDxChL2MllSourceBasedFilteringConfigurationGet(devNum, enablePtr, meshIdOffsetPtr, meshIdSizePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, enablePtr, meshIdOffsetPtr, meshIdSizePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet function
* @endinternal
*
* @brief   Set virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] targetDevNum             - target device number. (APPLICABLE RANGES: 0..15)
* @param[in] targetPortNum            - target port number.   (APPLICABLE RANGES: 0..63)
* @param[in] mllPointer               - pointer to L2 MLL entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note If the target supports APS 1+1, there are 2 L2MLL entries in this list, one
*       with a target logical port for the working path and one with a target
*       logical port for the protection path. It is required that these 2 target
*       logical ports be consecutive and differ only in the least significant bit of
*       the port number.
*       xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet
(
    IN GT_U8                             devNum,
    IN GT_U8                             targetDevNum,
    IN GT_U8                             targetPortNum,
    IN GT_U32                            mllPointer
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 entryIndex;
    PRV_CPSS_DXCH_PP_CONFIG_FINE_TUNING_STC *fineTuningPtr;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);
    if (targetDevNum > 15 || targetPortNum > 63)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    fineTuningPtr = &PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning;

    if(mllPointer >= fineTuningPtr->tableSize.mllPairs)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* Address this table by {TrgDev[3:0], TrgPort[5:0]} */
    entryIndex = targetDevNum << 6 | targetPortNum;

    /* set L2MLL Pointer Mapping table Entry */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_POINTER_MAPPING_E,
                                         entryIndex,                                        /* entryIndex   */
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,  /* fieldWordNum */
                                         0,                                                 /* fieldOffset  */
                                         12,                                                /* fieldLength  */
                                         mllPointer);                                       /* fieldValue   */
    return rc;
}

/**
* @internal cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet function
* @endinternal
*
* @brief   Set virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] targetDevNum             - target device number. (APPLICABLE RANGES: 0..15)
* @param[in] targetPortNum            - target port number.   (APPLICABLE RANGES: 0..63)
* @param[in] mllPointer               - pointer to L2 MLL entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note If the target supports APS 1+1, there are 2 L2MLL entries in this list, one
*       with a target logical port for the working path and one with a target
*       logical port for the protection path. It is required that these 2 target
*       logical ports be consecutive and differ only in the least significant bit of
*       the port number.
*       xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet
(
    IN GT_U8                             devNum,
    IN GT_U8                             targetDevNum,
    IN GT_U8                             targetPortNum,
    IN GT_U32                            mllPointer
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, targetDevNum, targetPortNum, mllPointer));

    rc = internal_cpssDxChL2MLLVirtualPortToMllMappingTableEntrySet(devNum, targetDevNum, targetPortNum, mllPointer);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, targetDevNum, targetPortNum, mllPointer));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet function
* @endinternal
*
* @brief   Get virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] targetDevNum             - target device number. (APPLICABLE RANGES: 0..15)
* @param[in] targetPortNum            - target port number.   (APPLICABLE RANGES: 0..63)
*
* @param[out] mllPointer               - (pointer to) L2 MLL entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                targetDevNum,
    IN  GT_U8                                targetPortNum,
    OUT GT_U32                              *mllPointer
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 entryIndex;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(mllPointer);
    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);
    if (targetDevNum > 15 || targetPortNum > 63)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* Address this table by {TrgDev[3:0], TrgPort[5:0]} */
    entryIndex = targetDevNum << 6 | targetPortNum;

    /* get L2MLL Pointer Mapping table Entry */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_POINTER_MAPPING_E,
                                        entryIndex,                                        /* entryIndex   */
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,  /* fieldWordNum */
                                        0,                                                 /* fieldOffset  */
                                        12,                                                /* fieldLength  */
                                        mllPointer);                                       /* fieldValue   */
    return rc;
}

/**
* @internal cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet function
* @endinternal
*
* @brief   Get virtual port mapping table entry. This entry maps
*         virtual target port to L2MLL entry.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number.
* @param[in] targetDevNum             - target device number. (APPLICABLE RANGES: 0..15)
* @param[in] targetPortNum            - target port number.   (APPLICABLE RANGES: 0..63)
*
* @param[out] mllPointer               - (pointer to) L2 MLL entry.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on wrong input parameters
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range.
* @retval GT_HW_ERROR              - on Hardware error.
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet
(
    IN  GT_U8                                devNum,
    IN  GT_U8                                targetDevNum,
    IN  GT_U8                                targetPortNum,
    OUT GT_U32                              *mllPointer
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, targetDevNum, targetPortNum, mllPointer));

    rc = internal_cpssDxChL2MLLVirtualPortToMllMappingTableEntryGet(devNum, targetDevNum, targetPortNum, mllPointer);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, targetDevNum, targetPortNum, mllPointer));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChL2MllVidxEnableSet function
* @endinternal
*
* @brief   Enable/disable MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
* @param[in] enable                   - GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllVidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_U16  vidx,
    IN GT_BOOL enable
)
{
    GT_STATUS rc = GT_OK;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    if (vidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    /* set L2MLL VIDX enable table Entry */
    rc = prvCpssDxChWriteTableEntryField(devNum,
                                         CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_VIDX_ENABLE_E,
                                         (GT_U32)(vidx / 32),                               /* entryIndex   */
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,  /* fieldWordNum */
                                         (GT_U32)(vidx % 32),                               /* fieldOffset  */
                                         1,                                                 /* fieldLength  */
                                         enable);                                           /* fieldValue   */
    return rc;
}

/**
* @internal cpssDxChL2MllVidxEnableSet function
* @endinternal
*
* @brief   Enable/disable MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
* @param[in] enable                   - GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllVidxEnableSet
(
    IN GT_U8   devNum,
    IN GT_U16  vidx,
    IN GT_BOOL enable
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllVidxEnableSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx, enable));

    rc = internal_cpssDxChL2MllVidxEnableSet(devNum, vidx, enable);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx, enable));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChL2MllVidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
static GT_STATUS internal_cpssDxChL2MllVidxEnableGet
(
    IN GT_U8     devNum,
    IN GT_U16    vidx,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    value; /* hardware value */

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
                                          CPSS_BOBCAT3_E | CPSS_ALDRIN2_E | CPSS_FALCON_E | CPSS_AC5P_E | CPSS_AC5X_E | CPSS_HARRIER_E | CPSS_IRONMAN_E | CPSS_AAS_E);

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    PRV_CPSS_DXCH_PP_HW_INFO_VPLS_MODE_SUPPORTED_CHECK_MAC(devNum);

    if (vidx > PRV_CPSS_DXCH_PP_HW_MAX_VALUE_OF_VIDX_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);

    /* get L2MLL VIDX enable table Entry */
    rc = prvCpssDxChReadTableEntryField(devNum,
                                         CPSS_DXCH_XCAT_TABLE_MLL_L2MLL_VIDX_ENABLE_E,
                                         (GT_U32)(vidx / 32),                               /* entryIndex   */
                                         PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS,  /* fieldWordNum */
                                         (GT_U32)(vidx % 32),                               /* fieldOffset  */
                                         1,                                                 /* fieldLength  */
                                         &value);                                           /* fieldValue   */

    if (rc != GT_OK)
        return rc;

    *enablePtr = (value != 0) ? GT_TRUE : GT_FALSE;

    return GT_OK;
}

/**
* @internal cpssDxChL2MllVidxEnableGet function
* @endinternal
*
* @brief   Get enabling status of MLL lookup for given vidx on the specified device.
*         When enabled the VIDX value is used as the L2MLL index.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.
*
* @param[in] devNum                   - device number
* @param[in] vidx                     - VIDX value. Valid range see datasheet for specific device.
*
* @param[out] enablePtr                - (pointer to)
*                                      GT_TRUE: MLL lookup per VIDX is enabled.
*                                      GT_FALSE: MLL lookup per VIDX is disabled.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong device
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note xCat device: applicable starting from revision C0
*
*/
GT_STATUS cpssDxChL2MllVidxEnableGet
(
    IN GT_U8     devNum,
    IN GT_U16    vidx,
    OUT GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChL2MllVidxEnableGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, vidx, enablePtr));

    rc = internal_cpssDxChL2MllVidxEnableGet(devNum, vidx, enablePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, vidx, enablePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

