/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* wraplCpssDxChBrgFdb.c
*
* DESCRIPTION:
*       A lua wrapper for bridge multicast entries.
*
* DEPENDENCIES:
*
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 2 $
*******************************************************************************/
#ifndef __wraplCpssDxChBrgFdb_h__
#define __wraplCpssDxChBrgFdb_h__


/**
* @internal pvrCpssFdbActionModeFromLuaGet
*
* @brief  Function Relevant mode : All modes
*         CPSS wrapper of fdb action mode getting from lua stack.
*
* @note APPLICABLE DEVICES:      DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
* @note NOT APPLICABLE DEVICES:  None.
*
* @param[in] L                   - lua state
* @param[in] L_index             - entry lua stack index
*
* @param[out] fdbActionModePtr   - fdb action mode
* @param[out] errorMessagePtr    - error message
*/
GT_STATUS pvrCpssFdbActionModeFromLuaGet
(
    IN  lua_State                   *L,
    IN  GT_32                       L_index,
    OUT CPSS_FDB_ACTION_MODE_ENT    *fdbActionModePtr,
    OUT GT_CHAR_PTR                 *errorMessagePtr
);

/*******************************************************************************
* prvCpssDxChBrgFdbTableSizeGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of FDB tables facility size.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum              - device number
*       fdbTableSizePrt     - pointer to FDB tables facility size
*
* OUTPUTS:
*       GT_OK
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChBrgFdbTableSizeGet
(
    IN  GT_U8               devNum,
    OUT GT_U32_PTR          fdbTableSizePrt,
    OUT GT_CHAR_PTR         *error_message
);

/*******************************************************************************
* prvCpssDxChBrgNextEntryIndexAndAgedAndMacEntryFromLuaGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next entry index and aged property and mac-entry from lua
*       stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       L                         - lua state
*       L_index                   - entry lua stack index
*       devNum                    - device number
*       searchOnlyValid           - property of searching only valid entries
*       macEntryIndexPtr          - next mac entry index
*       P_CALLING_FORMAL_DATA     - general debug information (environment
*                                   variables etc); could be empty
*
* OUTPUTS:
*       indexWasFoundPtr          - propety, that next mac entry was found
*       isSearchedAgedPropertyTakenPtr
*                                 - property, that searched pattern and mask of
*                                   aged property was found
*       patternAgedPtr            - searched pattern of aged property
*       isSearchedMacEntryTakenPtr
*                                 - property, that searched pattern and mask of
*                                   mac entry was found
*       patternMacEtnryPtr        - searched pattern of mac entry
*       maskMacEtnryPtr           - searched mask of mac entry
*       error_message             - error message
*
* RETURNS:
*       status and FDB entry index, if exists, to lua stack
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChBrgNextEntryIndexAndAgedAndMacEntryFromLuaGet
(
    IN    lua_State                 *L,
    IN    GT_32                     L_index,
    IN    GT_U8                     devNum,
    IN    GT_BOOL                   searchOnlyValid,
    INOUT GT_U32_PTR                macEntryIndexPtr,
    OUT   GT_BOOL                   *indexWasFoundPtr,
    OUT   GT_BOOL                   *isSearchedAgedPropertyTakenPtr,
    OUT   GT_BOOL                   *patternAgedPtr,
    OUT   GT_BOOL                   *maskAgedPtr,
    OUT   GT_BOOL                   *isSearchedMacEntryTakenPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *patternMacEtnryPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *maskMacEtnryPtr,
    OUT   GT_CHAR_PTR               *error_message
    P_CALLING_FORMAL_DATA
);


/*******************************************************************************
* prvCpssDxChBrgFdbNextEntryGet
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       Getting of next fdb entry index. Founded entry and its aged property
*       is masked and compared with pattern.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       devNum                      - device number
*       maskedSearch                - search with mask
*       searchOnlyValid             - property of searching only valid entries
*       patternAged                 - pattern of aged property
*       maskAged                    - mask of aged property
*       patternMacEntry             - pattern fdb mac entry
*       maskMacEntryPtr             - mask fdb mac entry
*       macEntryIndexPtr            - current FDB entry index
*       searchedValidPtr            - searched valid property
*       searchedSkipPtr             - searched skip property
*       searchedAgedSkipPtr         - searched aged property
*       searchedMacEntryPtr         - searched mac entry
*       entryWasFoundPtr            - mac entry founding property
*       error_message               - error message
*       P_CALLING_FORMAL_DATA       - general debug information (environment
*                                     variables etc); could be empty
*
* OUTPUTS:
*       status and FDB entry index, if exists, to lua stack
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
GT_STATUS prvCpssDxChBrgFdbNextEntryGet
(
    IN    GT_U8                     devNum,
    IN    GT_BOOL                   maskedSearch,
    IN    GT_BOOL                   searchOnlyValid,
    IN    GT_BOOL                   patternAged,
    IN    GT_BOOL                   maskAged,
    IN    CPSS_MAC_ENTRY_EXT_STC    *patternMacEntryPtr,
    IN    CPSS_MAC_ENTRY_EXT_STC    *maskMacEntryPtr,
    INOUT GT_U32_PTR                macEntryIndexPtr,
    OUT   GT_BOOL                   *searchedValidPtr,
    OUT   GT_BOOL                   *searchedSkipPtr,
    OUT   GT_BOOL                   *searchedAgedPtr,
    OUT   CPSS_MAC_ENTRY_EXT_STC    *searchedMacEntryPtr,
    OUT   GT_BOOL                   *entryWasFoundPtr,
    OUT   GT_CHAR_PTR               *error_message
    P_CALLING_FORMAL_DATA
);

/*******************************************************************************
* pvrCpssValidSkipAgedToLuaOnConditionPush
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of conditional pushing valid skip and aged values
*       to lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       status              - caller status
*       L                   - lua state
*       valid               - valid property
*       skip                - skipped property
*       aged                - aged property
*       condition           - pushing condition
*
* OUTPUTS:
*
* RETURNS:
*       1 if data was pushed to stack otherwise 0
*
* COMMENTS:
*
*******************************************************************************/
int pvrCpssValidSkipAgedToLuaOnConditionPush
(
    IN GT_STATUS                status,
    IN lua_State                *L,
    IN GT_BOOL                  valid,
    IN GT_BOOL                  skip,
    IN GT_BOOL                  aged,
    IN GT_BOOL                  condition
);


/*******************************************************************************
* pvrCpssMacEntryToLuaOnConditionPush
*
* DESCRIPTION:
*       Function Relevant mode : All modes
*
*       CPSS wrapper of conditional pushing CPSS_MAC_ENTRY_EXT_STC to lua stack.
*
* APPLICABLE DEVICES:
*        DxCh1; DxCh1_Diamond; DxCh2; DxCh3; xCat; xCat3; AC5; Lion; xCat2; Lion2.
*
* NOT APPLICABLE DEVICES:
*        None.
*
* INPUTS:
*       status              - caller status
*       L                   - lua state
*       macEntryPtr         - pushed mac entry
*       condition           - pushing condition
*
* OUTPUTS:
*
* RETURNS:
*       1 if data was pushed to stack otherwise 0
*
* COMMENTS:
*
*******************************************************************************/
int pvrCpssMacEntryToLuaOnConditionPush
(
    IN GT_STATUS                status,
    IN lua_State                *L,
    IN CPSS_MAC_ENTRY_EXT_STC   *macEntryPtr,
    IN GT_BOOL                  condition
);

#endif /* __wraplCpssDxChBrgFdb_h__ */
