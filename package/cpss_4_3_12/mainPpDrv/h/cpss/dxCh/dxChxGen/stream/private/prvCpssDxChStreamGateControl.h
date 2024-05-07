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
* @file prvCpssDxChStreamGateControl.h
*
* @brief CPSS DxCh Stream Gate Control (SGC) private APIs.
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChStreamGateControlh
#define __prvCpssDxChStreamGateControlh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @internal prvCpssStreamSgcOctetCounterSet function
* @endinternal
*
* @brief  Set counter value in Octet Counter Table for specific gate
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] gateId                - the Gate-ID assigned to the flow
*                                    (APPLICABLE RANGES: 0..510)
*                                    0x1FF - indicates Gate Control List is disabled
* @param[in] tableSetId            - the Table Set ID that is mapped to the gateId
*                                    (APPLICABLE RANGES: 0..57)
* @param[in] octetCounter          - byte count value
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcOctetCountersSet
(
    IN  GT_U8     devNum,
    IN  GT_U32    gateId,
    IN  GT_U32    tableSetId,
    IN  GT_U32    octetCounter
);

/**
* @internal prvCpssStreamSgcOctetCounterGet function
* @endinternal
*
* @brief  Get counter value from Octet Counters Table
*         This field contains the sum of byte count per gate
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] gateId                - the Gate-ID assigned to the flow
*                                    (APPLICABLE RANGES: 0..510)
*                                    0x1FF - indicates Gate Control List is disabled
* @param[in] tableSetId            - the Table Set ID that is mapped to the gateId
*                                    (APPLICABLE RANGES: 0..57)
* @param[out] octetCounterPtr      - (pointer to) sum of byte count
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PTR               - on null pointer
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcOctetCountersGet
(
    IN  GT_U8     devNum,
    IN  GT_U32    gateId,
    IN  GT_U32    tableSetId,
    OUT GT_U32    *octetCounterPtr
);

/**
* @internal prvCpssStreamSgcTableSetStatusGet function
* @endinternal
*
* @brief  Get the status of active/inactive of specific Table Set.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum        - device number.
* @param[in]  tableSetId    - the Table Set ID
*                              (APPLICABLE RANGES: 0..63)
* @param[out] isActivePtr   - (pointer to) indication to active/inactive of the specific Table Set
*                             GT_TRUE  - Table Set is active.
*                             GT_FALSE - Table Set is not active.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcTableSetStatusGet
(
    IN   GT_U8      devNum,
    IN   GT_U8      tableSetId,
    OUT  GT_BOOL    *isActivePtr
);

/**
* @internal prvCpssStreamSgcMapGateId2TableGetConfig function
* @endinternal
*
* @brief  Get the Table Set ID that is mapped to the GCL ID
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
* @param[in]  gclId                - the 6 MS bits of Gate ID
*                                    (APPLICABLE RANGES: 0..63)
* @param[out] tableSetIdPtr        - (pointer to) Table Set ID that is mapped to gclId.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcGateId2TableSetConfigGet
(
    IN   GT_U8     devNum,
    IN   GT_U32    gclId,
    OUT  GT_U8     *tableSetIdPtr
);

/**
* @internal prvCpssStreamSgcReconfigurationStatusGet function
* @endinternal
*
* @brief  Get re-configuration status parameters
*         - Pending status
*         - Valid status
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
* @param[out] pendingStatusPtr     - (pointer to) pending status.
* @param[out] validStatusPtr       - (pointer to) valid status.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcReconfigurationStatusGet
(
    IN   GT_U8     devNum,
    OUT  GT_BOOL   *pendingStatusPtr,
    OUT  GT_BOOL   *validStatusPtr
);

/**
* @internal prvCpssStreamSgcIntervalMaxTableDump function
* @endinternal
*
* @brief  Dump Interval Max Table values
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcIntervalMaxTableDump
(
    IN   GT_U8     devNum
);

/**
* @internal prvCpssStreamSgcOctetCountersTableDump function
* @endinternal
*
* @brief  Dump Octet Counters Table values
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcOctetCountersTableDump
(
    IN   GT_U8     devNum
);

/**
* @internal prvCpssStreamSgcTimeToAdvanceTableDump function
* @endinternal
*
* @brief  Dump Time To Advance Table values for specific Table Set ID
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] tableSetId            - the Table Set ID
*                                    (APPLICABLE RANGES: 0..57)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcTimeToAdvanceTableDump
(
    IN   GT_U8     devNum,
    IN   GT_U8     tableSetId
);

/**
* @internal prvCpssStreamSgcTableSetTimeConfigurationTableDump function
* @endinternal
*
* @brief  Dump Table Set Time Configuration Table values for all Table Set IDs
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcTableSetTimeConfigurationTableDump
(
    IN   GT_U8     devNum
);

/**
* @internal prvCpssStreamSgcTimeSlotAttributesTableDump function
* @endinternal
*
* @brief  Dump Time slot attributes Table values for specific Table Set ID
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] gateID                - the Gate ID
*                                    (APPLICABLE RANGES: 0..510)
* @param[in] tableSetId            - the Table Set ID
*                                    (APPLICABLE RANGES: 0..57)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcTimeSlotAttributesTableDump
(
    IN   GT_U8     devNum,
    IN   GT_U32    gateId,
    IN   GT_U8     tableSetId
);

/**
* @internal prvCpssStreamSgcDebugTodDump function
* @endinternal
*
* @brief  Dump debug TOD parameters
*         - TOD seconds MSB
*         - TOD seconds LSB
*         - TOD nano seconds
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssStreamSgcDebugTodDump
(
    IN   GT_U8     devNum
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChStreamGateControlh */



