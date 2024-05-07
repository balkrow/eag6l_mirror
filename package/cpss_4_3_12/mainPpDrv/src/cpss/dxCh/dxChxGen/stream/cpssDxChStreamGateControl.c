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
* @file cpssDxChStreamGateControl.c
*
* @brief CPSS DxCh Stream Gate Control (SGC) configuration APIs
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE

#include <cpss/generic/private/prvCpssCommonDefs.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/stream/private/prvCpssDxChStreamLog.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStreamGateControl.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/utils/prvCpssDxChTxqQbvUtils.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>



/*************************************************************************************************************
  Defines & Macros
 *************************************************************************************************************/

/* Maximum valid Gate ID value */
#define SGC_MAX_GATE_ID_NUM_CNS    510

/* Number of Table Sets */
#define SGC_MAX_TABLE_SETS_NUM_CNS    58

/* Number of Gates per Table Set */
#define SGC_MAX_GATES_NUM_PER_TABLE_SET_CNS    8

/* Total number of gates (58 Table Sets * 8 gates per Table Set = 464) */
#define SGC_MAX_GATES_NUM_CNS    SGC_MAX_TABLE_SETS_NUM_CNS * SGC_MAX_GATES_NUM_PER_TABLE_SET_CNS

/* Number of interval max profiles (as number of time slots) */
#define SGC_MAX_INTERVAL_MAX_PROFILES_NUM_CNS    CPSS_DXCH_STREAM_SGC_TIME_SLOTS_NUM_CNS

/* Maximum number of retry read attempts on re-configuration pending status bit */
#define SGC_MAX_NUM_OF_RETRY_COUNT_CNS    10

/* MACRO to check the SGC Gate ID index */
#define SMU_SGC_GATE_ID_INDEX_CHECK_MAC(_index)                                                      \
          if (_index > SGC_MAX_GATE_ID_NUM_CNS)                                                      \
          {                                                                                          \
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Gate ID [%d] is not in range of [0..%d]", \
                                            _index,                                                  \
                                            SGC_MAX_GATE_ID_NUM_CNS);                                \
          }

/* MACRO to check the SGC Gate Control List ID index */
#define SMU_SGC_GCL_ID_INDEX_CHECK_MAC(_index)                                                       \
          if (_index >= BIT_6)                                                                       \
          {                                                                                          \
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Gate ID [%d] is not in range of [0..%d]", \
                                            _index,                                                  \
                                            BIT_6-1);                                                \
          }

/* MACRO to check the SGC Table Set ID index */
#define SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(_index)                                                 \
          if (_index >= SGC_MAX_TABLE_SETS_NUM_CNS)                                                  \
          {                                                                                          \
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Gate ID [%d] is not in range of [0..%d]", \
                                            _index,                                                  \
                                            SGC_MAX_TABLE_SETS_NUM_CNS-1);                           \
          }

/* MACRO to check the time slots number */
#define SMU_SGC_TIME_SLOTS_NUMNER_CHECK_MAC(_timeSlotsNum)                                                    \
          if (_timeSlotsNum == 0 || _timeSlotsNum > CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS)              \
          {                                                                                                   \
              CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Time slots number[%d] is not in range of [1..%d]", \
                                            _timeSlotsNum,                                                           \
                                            CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS);                     \
          }

/* MACRO to check the interval max profiles number */
#define SMU_SGC_IMX_PROFILES_NUMNER_CHECK_MAC(_imxProfilesNum)                                                          \
          if (_imxProfilesNum == 0 || _imxProfilesNum > CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS)                  \
          {                                                                                                             \
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Interval max profiles number[%d] is not in range of [1..%d]", \
                                           _imxProfilesNum,                                                                      \
                                           CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS);                              \
          }


/*************************************************************************************************************
  Internal functions
 *************************************************************************************************************/

/**
* @internal streamSgcTableSetTimeConfigInfoEntrySet function
* @endinternal
*
* @brief  Set time parameters for specific table set
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum               - device number.
* @param[in] tableSetIndex        - index to entry in table. Represents specific Table Set.
* @param[in] tableParamsPtr       - (pointer to) the table's entry info
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcTableSetTimeConfigInfoEntrySet
(
    IN  GT_U8                                    devNum,
    IN  GT_U8                                    tableSetIndex,
    IN  CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC  *tableParamsPtr
)
{
    GT_STATUS rc;             /* return code status               */
    GT_U32    hwValueArr[4];  /* data to be written into HW table */

    /***** Check table parameters are valid *****/

    /* Check byteCountAdjust value is in range of 7 bits */
    if (tableParamsPtr->byteCountAdjust >= BIT_7)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "byteCountAdjust [%d] is out of range of [0..%d]",
                                      tableParamsPtr->byteCountAdjust,BIT_7-1);
    }

    /* Check remainingBitsFactor value is in range of 8 bits */
    if (tableParamsPtr->remainingBitsFactor >= BIT_8)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "remainingBitsFactor [%d] is out of range of [0..%d]",
                                      tableParamsPtr->remainingBitsFactor,BIT_8-1);
    }

    /* Check ingressTimestampOffset value is in range of 16 bits */
    if (tableParamsPtr->ingressTimestampOffset >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "ingressTimestampOffset [%d] is not in range of [0..%d]",
                                      tableParamsPtr->ingressTimestampOffset,BIT_16-1);
    }

    /* Check ingressTodOffset value is in range of 16 bits */
    if (tableParamsPtr->ingressTodOffset >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "ingressTodOffset [%d] is not in range of [0..%d]",
                                      tableParamsPtr->ingressTodOffset,BIT_16-1);
    }
    /* Check remainingBitsResolution value is in range of 2 bits */
    if (tableParamsPtr->remainingBitsResolution >= BIT_2)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "remainingBitsResolution [%d] is out of range of [0..%d]",
                                      tableParamsPtr->remainingBitsResolution,BIT_2-1);
    }

    /***** Build the entry to the table with the relevant fields *****/

    /* Clear local hwValueArr table */
    cpssOsMemSet(hwValueArr,0,sizeof(hwValueArr));

    /* Set ingressTodOffset */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TOD_OFFSET_E,
            tableParamsPtr->ingressTodOffset);

    /* Set ingressTimestampOffset */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TIMESTAMP_OFFSET_E,
            tableParamsPtr->ingressTimestampOffset);

    /* Set cycleTime */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_CYCLE_TIME_E,
            tableParamsPtr->cycleTime);

    /* Set remainingBitsFactor */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_FACTOR_E,
            tableParamsPtr->remainingBitsFactor);

    /* Set byteCountAdjust */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_E,
            tableParamsPtr->byteCountAdjust);

    /* Set byteCountAdjustPolarity */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_POLARITY_E,
            BOOL2BIT_MAC(tableParamsPtr->byteCountAdjustPolarity));

    /* Set remainingBitsResolution */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_RESOLUTION_E,
            tableParamsPtr->remainingBitsResolution);

    /* Write it into SGC Table Set Time Configurations table
       Entry to table is according to Table Set ID
     */
    rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E,
                                    tableSetIndex,
                                    hwValueArr);
    return rc;
}

/**
* @internal streamSgcTableSetTimeConfigInfoEntryGet function
* @endinternal
*
* @brief  Get time parameters for specific table set
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
* @param[in]  tableSetIndex        - index to specific entry in table. Represents specific Table Set.
* @param[out] tableParamsPtr       - (pointer to) the table's entry info
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcTableSetTimeConfigInfoEntryGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U8                                    tableSetIndex,
    OUT  CPSS_DXCH_STREAM_SGC_TABLE_SET_INFO_STC  *tableParamsPtr
)
{
    GT_STATUS rc;             /* return code status       */
    GT_U32    hwValueArr[4];  /* entry read from HW table */
    GT_U32    fieldValue;     /* value read from HW       */

    /* Clear local hwValueArr table */
    cpssOsMemSet(hwValueArr,0,sizeof(hwValueArr));

    /* Read entry from SGC Table Set Time Configurations table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E,
                                   tableSetIndex,
                                   hwValueArr);

    /* Parse the fields from the entry */

    /* Get ingressTodOffset */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TOD_OFFSET_E,
            fieldValue);
    tableParamsPtr->ingressTodOffset = fieldValue;

    /* Get ingressTimestampOffset */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TIMESTAMP_OFFSET_E,
            fieldValue);
    tableParamsPtr->ingressTimestampOffset = fieldValue;

    /* Get cycleTime */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_CYCLE_TIME_E,
            fieldValue);
    tableParamsPtr->cycleTime = fieldValue;

    /* Get remainingBitsFactor */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_FACTOR_E,
            fieldValue);
    tableParamsPtr->remainingBitsFactor = fieldValue;

    /* Get byteCountAdjust */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_E,
            fieldValue);
    tableParamsPtr->byteCountAdjust = fieldValue;

    /* Get byteCountAdjustPolarity */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_POLARITY_E,
            fieldValue);
    tableParamsPtr->byteCountAdjustPolarity = BIT2BOOL_MAC(fieldValue);

    /* Get remainingBitsResolution */
    SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
            devNum,
            hwValueArr,
            SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_RESOLUTION_E,
            fieldValue);
    tableParamsPtr->remainingBitsResolution = fieldValue;

    return rc;
}

/**
* @internal streamSgcTimeSlotInfoEntrySet function
* @endinternal
*
* @brief  Set Time slot information for specific time slot in selected Table Set and gate.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum            - device number.
* @param[in] tableIndex        - index to specific entry in table derived from table Set ID and time slot number
* @param[in] gateSelect        - gate number selected out of 8 options per entry
* @param[in] imxProfilesNum    - number of interval max profiles to use
* @param[in] tableParamsPtr    - (pointer to) the table's entry info
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcTimeSlotInfoEntrySet
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   tableIndex,
    IN  GT_U8                                    gateSelect,
    IN  GT_U32                                   imxProfilesNum,
    IN  CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC  *tableParamsPtr
)
{
    GT_STATUS rc;             /* return code status                             */
    GT_U32    hwValueArr[4];  /* data to be written into HW table               */
    GT_U32    gateOffset;     /* field offset according to gate select          */
    GT_U32    fieldId;        /* time slot field ID                             */
    GT_U8     ipv;            /* Internal Priority Value as written to register */

    /* Verify IPV value is in range of 3 bits in case IPV value set TC */
    if ( (tableParamsPtr->keepTc == GT_FALSE) && (tableParamsPtr->ipv >= BIT_3) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "ipv[%d] in timeSlot[%d] is out of range of [0..%d]",
                                      tableParamsPtr->ipv,(tableIndex & BIT_MASK_MAC(8)),BIT_3-1);
    }

    /* Check intervalMaxOctetProfile value is in valid range */
    if (tableParamsPtr->intervalMaxOctetProfile >= imxProfilesNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE,"intervalMaxOctetProfile[%d] index in timeSlot[%d] "
                                                      "is bigger than number of profiles that are set[%d]",
                                      tableParamsPtr->intervalMaxOctetProfile,
                                      (tableIndex & BIT_MASK_MAC(8)),
                                      imxProfilesNum);
    }

    /* Clear local hwValueArr table */
    cpssOsMemSet(hwValueArr,0,sizeof(hwValueArr));

    /***** Set time to advance value for specific time slot in selected tableSet  *****/

    /* Write to SGC Time To Advance table
       Each entry represents the delay (in Nano sec) from the Cycle start to transition to the next time slot
     */
    rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_TO_ADVANCE_E,
                                    tableIndex,
                                    &tableParamsPtr->timeToAdvance);
    if(rc != GT_OK)
    {
        return rc;
    }


    /***** Set Time Slot attributes values for specific time slot in selected gate *****/

    /* Start by read the entry of Time Slot Attributes table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_SLOT_ATTRIBUTES_E,
                                   tableIndex,
                                   hwValueArr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Build the entry to the table with the relevant fields

       Each entry includes 8 possible gates. For each gate there are 5 parameters:

           | Gate n + 0: newSlot0, streamGateState0, lengthAware0, ipv0, intervalMaxOctetProfile0
       1   | Gate n + 1: newSlot1, streamGateState1, lengthAware1, ipv1, intervalMaxOctetProfile1
     entry | -
           | -
           | Gate n + 7: newSlot7, streamGateState7, lengthAware7, ipv7, intervalMaxOctetProfile7
     */

    /* Calculate gate offset to point to right field taking into account 5 parameters for each gate */
    gateOffset = gateSelect * 5;

    /* Set newSlot */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                BOOL2BIT_MAC(tableParamsPtr->newSlot));

    /* Set streamGateState */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                tableParamsPtr->streamGateState);

    /* Set lengthAware */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                BOOL2BIT_MAC(tableParamsPtr->lengthAware));

    /* Set ipv as expected in table: 4b0XXX - set TC to XXX, 4b1*** - keep incoming TC */
    ipv = ( BOOL2BIT_MAC(tableParamsPtr->keepTc) << 3 | tableParamsPtr->ipv );
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                ipv);

    /* Set intervalMaxOctetProfile */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                tableParamsPtr->intervalMaxOctetProfile);

    /* Write the entry to SGC Time Slot Attributes table */
    rc = prvCpssDxChWriteTableEntry(devNum,
                                    CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_SLOT_ATTRIBUTES_E,
                                    tableIndex,
                                    hwValueArr);

    return rc;
}

/**
* @internal streamSgcTimeSlotInfoEntryGet function
* @endinternal
*
* @brief  Get Time slot information from specific time slot in selected Table Set and gate.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum            - device number.
* @param[in]  tableIndex        - index to specific entry in table derived from table Set ID and time slot number
* @param[in]  gateSelect        - gate number selected out of 8 options per entry
* @param[out] tableParamsPtr    - (pointer to) the table's entry info
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_FAIL                  - on failure.
* @retval GT_BAD_PARAM             - on wrong parameter
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcTimeSlotInfoEntryGet
(
    IN   GT_U8                                    devNum,
    IN   GT_U32                                   tableIndex,
    IN   GT_U8                                    gateSelect,
    OUT  CPSS_DXCH_STREAM_SGC_TIME_SLOT_INFO_STC  *tableParamsPtr
)
{
    GT_STATUS rc;             /* return code status                    */
    GT_U32    hwValueArr[4];  /* entry read from HW table              */
    GT_U32    fieldValue;     /* value read from HW                    */
    GT_U32    gateOffset;     /* field offset according to gate select */
    GT_U32    fieldId;        /* time slot field ID                    */

    /* Clear local hwValueArr table */
    cpssOsMemSet(hwValueArr,0,sizeof(hwValueArr));

    /***** Get time to advance value from specific time slot in selected tableSet  *****/

    /* Read SGC Time To Advance table
       Each entry represents the delay (in Nano sec) from the Cycle start to transition to the next time slot
     */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_TO_ADVANCE_E,
                                   tableIndex,
                                   &tableParamsPtr->timeToAdvance);
    if(rc != GT_OK)
    {
        return rc;
    }


    /***** Get Time Slot attributes values from specific time slot in selected Table Set and gate *****/

    /* Read the entry of Time Slot Attributes table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_SLOT_ATTRIBUTES_E,
                                   tableIndex,
                                   hwValueArr);

    /* Build the entry to the table with the relevant fields

       Each entry includes 8 possible gates. For each gate there are 5 parameters:

           | Gate n + 0: newSlot0, streamGateState0, lengthAware0, ipv0, intervalMaxOctetProfile0
       1   | Gate n + 1: newSlot1, streamGateState1, lengthAware1, ipv1, intervalMaxOctetProfile1
     entry | -
           | -
           | Gate n + 7: newSlot7, streamGateState7, lengthAware7, ipv7, intervalMaxOctetProfile7
     */

    /* Calculate gate offset to point to right field taking into account 5 parameters for each gate */
    gateOffset = gateSelect * 5;

    /* Get newSlot */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                fieldValue);
    tableParamsPtr->newSlot = BIT2BOOL_MAC(fieldValue);

    /* Get streamGateState */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                fieldValue);
    tableParamsPtr->streamGateState = (fieldValue == 0) ? CPSS_DXCH_STREAM_SGC_GATE_STATE_OPEN_E :
                                                          CPSS_DXCH_STREAM_SGC_GATE_STATE_CLOSE_E ;

    /* Get lengthAware */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                fieldValue);
    tableParamsPtr->lengthAware = BIT2BOOL_MAC(fieldValue);

    /* Get ipv + keepTc: 4’b0XXX - set TC to XXX, 4’b1*** - keep incoming TC */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                fieldValue);
    tableParamsPtr->keepTc = BIT2BOOL_MAC((fieldValue >> 3) & BIT_MASK_MAC(1));
    tableParamsPtr->ipv = fieldValue & BIT_MASK_MAC(3);

    /* Get intervalMaxOctetProfile */
    fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_0_E + gateOffset;
    SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                fieldId,
                fieldValue);
    tableParamsPtr->intervalMaxOctetProfile = fieldValue;

    return rc;
}

/**
* @internal streamSgcTableSetActiveSet function
* @endinternal
*
* @brief  Set Table Set to active/inactive.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] tableSetId     - the Table Set ID
*                             (APPLICABLE RANGES: 0..63)
* @param[in] enable         - Indication to activate/deactivate the specific Table Set.
*                             GT_TRUE  - set to active.
*                             GT_FALSE - set to inactive.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong tableSetId
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcTableSetActiveSet
(
    IN  GT_U8      devNum,
    IN  GT_U8      tableSetId,
    IN  GT_BOOL    enable
)
{
    GT_STATUS  rc;             /* return code status                           */
    GT_U32     regAddr;        /* the register's address to write to           */
    GT_U32     fieldOffset;    /* the start bit number in the register         */
    GT_U32     fieldLength;    /* the number of bits to be written to register */
    GT_U32     hwValue  ;      /* data to be written into the register         */

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Field size of 1 bit */
    fieldLength = 1;

    /* Bit per Table Set spread over 2 registers */
    fieldOffset = tableSetId % 32;

    /* Activate or deactivate this specific Table Set */
    hwValue  = BOOL2BIT_MAC(enable);

    /* Get the correct register address out of two according to TableSet ID
       - sgcTableSetIsActive[0] for the first 32 Table Set IDs
       - sgcTableSetIsActive[1] for the last 32 Table Set IDs
     */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcTableSetIsActive[tableSetId >> 5];

    /* Write it to SGC TableSet is active register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);

    return rc;
}

/**
* @internal streamSgcMapGateId2TableSetConfig function
* @endinternal
*
* @brief  Map Gate ID to Table Set ID configuration
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] gclId          - the 6 MS bits of Gate ID
* @param[in] tableSetId     - the Table Set ID
*                             (APPLICABLE RANGES: 0..57)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcMapGateId2TableSetConfig
(
    IN  GT_U8     devNum,
    IN  GT_U8     gclId,
    IN  GT_U8     tableSetId
)
{
    GT_STATUS  rc;             /* return code status                                */
    GT_U32     regAddr;        /* the register's address to write to                */
    GT_U32     fieldOffset;    /* the start bit number in the register              */
    GT_U32     fieldLength;    /* the number of bits to be written to register      */
    GT_U8      index;          /* index to SGC Gate ID 2 Table Set Config registers */

    /* Use it as an index to registers SGC Gate ID 2 Table Set Config */
    index = gclId;

    /* 6 bits field */
    fieldLength = 6;

    /* 5 fields per register spread over 13 registers */
    fieldOffset = (index % 5) * fieldLength;

    /* Get register address */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcGateId2TableSetConfig[index/5];

    /* Map Table Set to Gate ID
       - select the register that represents this Gate ID
       - write the desired Table Set ID into it
     */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, tableSetId);

    return rc;
}

/**
* @internal streamSgcReconfigurationSet function
* @endinternal
*
* @brief  Set re-configuration parameters
*         - set the time the re-configuration takes place
*         - map re-configuration Table Set ID to the Gate ID
*         - set pending status to '1'
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] gateID                - the Gate ID
*                                    (APPLICABLE RANGES: 0..510)
* @param[in] tableSetId            - the Table Set ID
*                                    (APPLICABLE RANGES: 0..57)
* @param[in] reconfigTimeParamsPtr - (pointer to) reconfiguration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcReconfigurationSet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       gateId,
    IN  GT_U8                                        tableSetId,
    IN  CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *reconfigTimeParamsPtr
)
{
    GT_STATUS  rc;             /* return code status                           */
    GT_U32     regAddr;        /* the register's address to write to           */
    GT_U32     fieldOffset;    /* the start bit number in the register         */
    GT_U32     fieldLength;    /* the number of bits to be written to register */
    GT_U32     hwValue = 0;    /* data to write to HW register                 */


    /***** Check re-configuration parameters are valid *****/

    /* Check TOD MSbits high value is in range of 16 bits */
    if (reconfigTimeParamsPtr->todMsb.l[1] >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "todMsb high value [%d] is out of range of [0..%d]",
                                      reconfigTimeParamsPtr->todMsb.l[1],BIT_16-1);
    }

    /* Check TOD LSbits value is in range of 30 bits */
    if (reconfigTimeParamsPtr->todLsb >= BIT_30)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "todLsb value [%d] is out of range of [0..%d]",
                                      reconfigTimeParamsPtr->todLsb,BIT_30-1);
    }

    /* Check reconfiguration time extension value is in range of 16 bits */
    if (reconfigTimeParamsPtr->configChangeTimeExtension >= BIT_16)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "configChangeTimeExtension value [%d] is out of range of [0..%d]",
                                      reconfigTimeParamsPtr->configChangeTimeExtension,BIT_16-1);
    }

    /***** Set re-configuration time of 48 bits TOD MSbits (represent seconds) *****/

    /* First set TOD 16 MSbits High part */

    /* Field size of 16 bits */
    fieldLength = 16;

    /* Field start bit */
    fieldOffset = 0;

    /* TOD 16 MS bits */
    hwValue  = reconfigTimeParamsPtr->todMsb.l[1];

    /* Get register address of SGC Reconfiguration TOD MSB High */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTodMsbHigh;

    /* Write it in the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Now set TOD 32 MSbits Low part */

    /* Field size of 32 bits */
    fieldLength = 32;

    /* Field start bit */
    fieldOffset = 0;

    /* TOD MSbits (32 MS bits) */
    hwValue  = reconfigTimeParamsPtr->todMsb.l[0];

    /* Get register address of SGC Reconfiguration TOD MSB Low */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTodMsbLow;

    /* Write it in the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }


    /***** Set re-configuration time of 30 bits TOD LSbits (represent nano seconds) *****/

    /* Field size of 30 bits */
    fieldLength = 30;

    /* Field start bit */
    fieldOffset = 0;

    /* TOD 30 LS bits */
    hwValue  = reconfigTimeParamsPtr->todLsb;

    /* Get register address of SGC Reconfiguration TOD LSB */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTodLsb;

    /* Write it in the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }


    /***** Set re-configuration time extension in ns *****/

    /* Field size of 16 bits */
    fieldLength = 16;

    /* Field start bit */
    fieldOffset = 12;

    /* Reconfiguration time extension value */
    hwValue  = reconfigTimeParamsPtr->configChangeTimeExtension;

    /* Get register address of SGC Reconfiguration Table Set Configurations */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTableSetConfig;

    /* Write it in the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }


    /***** Set re-configuration Table Set mapping to GCL ID *****/

    /* Combine 2 fields  (Table Set ID(6b) | GCL ID(6b) ) */
    hwValue  = ( (tableSetId << 6) | ( (gateId >> 3) & BIT_MASK_MAC(6) ) );

    /* Field size of 12 bits */
    fieldLength = 12;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration Table Set Configurations */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTableSetConfig;

    /* Write it in the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }


    /***********************************************************************
      Set reconfiguration pending status to '1'.
      Indicates HW Stream Gate Control (SGC) logic to start
      checking if current time matches re-configuration time
     ***********************************************************************/

    /* Field size of 1 bit */
    fieldLength = 1;

    /* Field start bit */
    fieldOffset = 0;

    /* Set pending bit to '1' */
    hwValue  = 1;

    /* Get register address of SGC Reconfiguration Pending Status */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigPendingStatus;

    /* Write it in the register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, hwValue);

    return rc;
}

/**
* @internal streamSgcReconfigurationGet function
* @endinternal
*
* @brief  Get re-configuration parameters
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                 - device number.
* @param[in] gateID                 - the Gate ID
*                                     (APPLICABLE RANGES: 0..510)
* @param[in] tableSetId             - the Table Set ID
*                                     (APPLICABLE RANGES: 0..57)
* @param[out] reconfigTimeParamsPtr - (pointer to) reconfiguration parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS streamSgcReconfigurationGet
(
    IN   GT_U8                                        devNum,
    OUT  CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *reconfigTimeParamsPtr
)
{
    GT_STATUS  rc;             /* return code status                           */
    GT_U32     regAddr;        /* the register's address to write to           */
    GT_U32     fieldOffset;    /* the start bit number in the register         */
    GT_U32     fieldLength;    /* the number of bits to be written to register */
    GT_U32     hwValue= 0;     /* data to write to HW register                 */

    /***** Get reconfiguration time of 48 bits TOD MSbits (represent seconds) *****/

    /* First get TOD 16 MSbits High part */

    /* Field size of 16 bits */
    fieldLength = 16;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration TOD MSB High */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTodMsbHigh;

    /* Read it from the register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Update output parameter */
    reconfigTimeParamsPtr->todMsb.l[1] = hwValue;

    /* Now get TOD 32 MSbits Low part */

    /* Field size of 32 bits */
    fieldLength = 32;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration TOD MSB Low */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTodMsbLow;

    /* Read it from the register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Update output parameter */
    reconfigTimeParamsPtr->todMsb.l[0] = hwValue;


    /***** Get reconfiguration time of 30 bits TOD LSbits (represent nano seconds) *****/

    /* Field size of 30 bits */
    fieldLength = 30;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration TOD LSB */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTodLsb;

    /* Read it from the register */
    hwValue = 0;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Update output parameter */
    reconfigTimeParamsPtr->todLsb = hwValue;


    /***** Get reconfiguration time extension in ns *****/

    /* Field size of 16 bits */
    fieldLength = 16;

    /* Field start bit */
    fieldOffset = 12;

    /* Get register address of SGC Reconfiguration Table Set Configurations */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTableSetConfig;

    /* Read it from the register */
    hwValue = 0;
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);

    /* Update output parameter */
    reconfigTimeParamsPtr->configChangeTimeExtension = hwValue;

    return rc;
}


/*************************************************************************************************************
  Private functions
 *************************************************************************************************************/

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
)
{
    GT_STATUS rc;         /* return code status                              */
    GT_U32    index;      /* index to the Octet Counters table               */
    GT_U32    hwValue;    /* value that is written into Octet Counters table */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Verify gateId is in valid range */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Calculate index to the Octet Counters table */
    index = (tableSetId << 3) + (gateId & BIT_MASK_MAC(3));

    /* Set HW value with octet counter */
    hwValue = octetCounter;

    /* Set octet counter value into SGC Octet Counters table */
    rc = prvCpssDxChWriteTableEntry(devNum,
                                   CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_OCTET_COUNTERS_E,
                                   index,
                                   &hwValue);
    return rc;
}

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
)
{
    GT_STATUS rc;         /* return code status                   */
    GT_U32    hwValue;    /* value read from Octet Counters table */
    GT_U32    index;      /* index to the Octet Counters table    */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(octetCounterPtr);

    /* Verify gateId is in valid range */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Calculate index to the Octet Counters table */
    index = (tableSetId << 3) + (gateId & BIT_MASK_MAC(3));

    /* Read octet counter value from SGC Octet Counters table */
    rc = prvCpssDxChReadTableEntry(devNum,
                                   CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_OCTET_COUNTERS_E,
                                   index,
                                   &hwValue);

    /* Update output parameter */
    *octetCounterPtr = hwValue;

    return rc;
}

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
)
{
    GT_STATUS  rc;             /* return code status                       */
    GT_U32     regAddr;        /* the register's address to read from      */
    GT_U32     fieldOffset;    /* the start bit number in the register     */
    GT_U32     fieldLength;    /* the number of bits to read from register */
    GT_U32     hwValue = 0;    /* data to be read from the register        */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(isActivePtr);

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Field size of 1 bit */
    fieldLength = 1;

    /* Bit per Table Set spreads over 2 registers */
    fieldOffset = tableSetId % 32;

    /* Get the correct register address out of two according to TableSet ID
       - sgcTableSetIsActive[0] for the first 32 Table Set IDs
       - sgcTableSetIsActive[1] for the last 32 Table Set IDs
     */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcTableSetIsActive[tableSetId >> 5];

    /* Read it from SGC TableSet is active register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);

    /* Update the output parameter */
    *isActivePtr  = BIT2BOOL_MAC(hwValue);

    return rc;
}

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
)
{
    GT_STATUS  rc;             /* return code status                                */
    GT_U32     regAddr;        /* the register's address to read from               */
    GT_U32     fieldOffset;    /* the start bit number in the register              */
    GT_U32     fieldLength;    /* the number of bits to read from register          */
    GT_U32     hwValue;        /* data to be read from HW register                  */
    GT_U8      index;          /* index to SGC Gate ID 2 Table Set Config registers */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(tableSetIdPtr);

    /* Verify gclId is in valid range */
    SMU_SGC_GCL_ID_INDEX_CHECK_MAC(gclId);

    /* Used as an index to registers SGC Gate ID 2 Table Set Config */
    index = gclId;

    /* 6 bits per gate2TableSet mapping */
    fieldLength = 6;

    /* 5 gateId2TableSet mapping per register spreads over 13 registers */
    fieldOffset = (index % 5) * fieldLength;

    /* Get register address. 5 gate2TableSet mapping per register spread over 13 registers */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcGateId2TableSetConfig[index/5];

    /* Read SGC Gate ID 2 Table Set Config register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);

    /* Update output parameter */
    *tableSetIdPtr = hwValue;

    return rc;
}

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
)
{
    GT_STATUS  rc;             /* return code status                       */
    GT_U32     regAddr;        /* the register's address to read from      */
    GT_U32     fieldOffset;    /* the start bit number in the register     */
    GT_U32     fieldLength;    /* the number of bits to read from register */
    GT_U32     hwValue;        /* data to be read from HW register         */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(pendingStatusPtr);
    CPSS_NULL_PTR_CHECK_MAC(validStatusPtr);

    /* Field size of 1 bit */
    fieldLength = 1;

    /* Field start bit */
    fieldOffset = 0;

    /**** Get Pending Status ****/

    /* Get register address of SGC Reconfiguration Valid Status */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigPendingStatus;

    /* Read it from register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    *pendingStatusPtr = BIT2BOOL_MAC(hwValue);


    /**** Get Valid Status ****/

    /* Get register address of SGC re-configuration Valid Status */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigValidStatus;

    /* Read it from register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    *validStatusPtr = BIT2BOOL_MAC(hwValue);

    return rc;
}

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
)
{
    GT_STATUS  rc;         /* return code status                      */
    GT_U32     hwValue;    /* data to be read from Interval Max table */
    GT_U32     i;          /* index to Interval Max table             */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Print out the Interval Max values */
    cpssOsPrintf("\n==== Interval Max Table ==== \n\n");

    /* Loop over all time slots. Each entry represents the maximum allowed number of bytes per time slot */
    for (i=0; i<CPSS_DXCH_STREAM_SGC_MAX_IMX_PROFILES_NUM_CNS; i++)
    {
        /* Read from SGC Interval Max table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E,
                                       i,
                                       &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Print out the Interval Max value */
        cpssOsPrintf("time slot[%3.3d]: 0x%8.8x  \n",i,hwValue);
    }

    return rc;
}

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
)
{
    GT_STATUS  rc;         /* return code status                        */
    GT_U32     hwValue;    /* data to be read from Octet Counters table */
    GT_U32     i;          /* index to Octet Counters table             */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Print out the Octet Counters values */
    cpssOsPrintf("\n==== Octet Counters Table ==== \n\n");

    /* Loop over all gates */
    for (i=0; i<SGC_MAX_GATES_NUM_CNS; i++)
    {
        /* Read from SGC Octet Counter table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_OCTET_COUNTERS_E,
                                       i,
                                       &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Print out the Octet Counter value */
        cpssOsPrintf("gate[%3.3d]: 0x%8.8x  \n",i,hwValue);
    }

    return rc;
}

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
)
{
    GT_STATUS  rc;           /* return code status                        */
    GT_U32     hwValue;      /* data to be read from Octet Counters table */
    GT_U32     i;            /* index to Octet Counters table             */
    GT_U32     tableSetOfst; /* Table Set start point entry in table      */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Print out the Time To Advance values */
    cpssOsPrintf("\n==== Time To Advance Table (for Table Set ID[%d]) ==== \n\n",tableSetId);

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Calculate table entry index for time slot tables per Table set */
    tableSetOfst = tableSetId * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

    /* Per Table set & Gate, go over all existing time slots and set parameters for each time slot */
    for (i=0; i<CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS; i++)
    {
        /* Read Time To Advance values from table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_TO_ADVANCE_E,
                                       tableSetOfst + i,
                                       &hwValue);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Print out the Time To Advance values */
        cpssOsPrintf("time slot[%3.3d]: 0x%8.8x  \n",i,hwValue);
    }

    return rc;
}

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
)
{
    GT_STATUS  rc;                      /* return code status                                                        */
    GT_U32     hwValueArr[4];           /* entry to be read from Octet Counters table                                */
    GT_U32     i;                       /* index to Octet Counters table                                             */
    GT_U32     ingressTodOffset;        /* negative Delay (in Nano sec) adjust from the port to the GCL unit         */
    GT_U32     ingressTimestampOffset;  /* delay (in Nano sec) offset of the ingress timestamp that the MAC assigned */
    GT_U32     cycleTime;               /* table set cycle time                                                      */
    GT_U32     remainingBitsFactor;     /* number of bit per 16 nsec                                                 */
    GT_U32     byteCountAdjust;         /* packet byte count adjust                                                  */
    GT_U32     byteCountAdjustPolarity; /* indicates the polarity of the Byte Counter Adjust                         */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Print out the Table Set Time Configuration values */
    cpssOsPrintf("\n==== Table Set Time Configuration Table ==== \n\n");

    cpssOsPrintf("                 ------------------------------------------------------------------------------------------------------------------------------------ \n");
    cpssOsPrintf("                 | ingressTodOffset, | ingressTimestampOffset, | cycleTime,     | remainingBitsFactor,  | byteCountAdjust, | byteCountAdjustPolarity |\n");
    cpssOsPrintf("                 ------------------------------------------------------------------------------------------------------------------------------------ \n");

    /* Go over all Table Sets */
    for (i=0; i<SGC_MAX_TABLE_SETS_NUM_CNS; i++)
    {
        /* Read entry from SGC Table Set Time Configurations table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E,
                                       i,
                                       hwValueArr);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* Parse the fields from the entry */

        /* Get ingressTodOffset */
        SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TOD_OFFSET_E,
                ingressTodOffset);

        /* Get ingressTimestampOffset */
        SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TIMESTAMP_OFFSET_E,
                ingressTimestampOffset);

        /* Get cycleTime */
        SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_CYCLE_TIME_E,
                cycleTime);

        /* Get remainingBitsFactor */
        SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_FACTOR_E,
                remainingBitsFactor);

        /* Get byteCountAdjust */
        SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_E,
                byteCountAdjust);

        /* Get byteCountAdjustPolarity */
        SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET_MAC(
                devNum,
                hwValueArr,
                SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_POLARITY_E,
                byteCountAdjustPolarity);


        /* Print out the Time To Advance values */
        cpssOsPrintf("[tableSetId %2.2d]: | 0x%8.8x,       | 0x%8.8x,             | 0x%8.8x,    | 0x%2.2x,                 | 0x%2.2x,            | 0x%1.1x                     | \n"
                     ,i
                     ,ingressTodOffset
                     ,ingressTimestampOffset
                     ,cycleTime
                     ,remainingBitsFactor
                     ,byteCountAdjust
                     ,byteCountAdjustPolarity);
    }

    cpssOsPrintf("                 ------------------------------------------------------------------------------------------------------------------------------------ \n");

    return rc;
}

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
)
{
    GT_STATUS  rc;                      /* return code status                                        */
    GT_U32     hwValueArr[4];           /* entry to be read from Octet Counters table                */
    GT_U32     i;                       /* index to Octet Counters table                             */
    GT_U32     newSlot;                 /* Indication whether to reset or continue the byte counting */
    GT_U32     streamGateState;         /* open/close gate state                                     */
    GT_U32     lengthAware;             /* packet length aware                                       */
    GT_U32     ipv;                     /* Internal Priority Value                                   */
    GT_U32     intervalMaxOctetProfile; /* packet byte count adjust                                  */
    GT_U32     gateSelect;              /* gate number selected out of 8 options per table Set       */
    GT_U32     tableSetOfst;            /* Table Set start point entry in table                      */
    GT_U32     gateOffset;              /* field offset according to gate select                     */
    GT_U32     fieldId;                 /* time slot field ID                                        */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Verify gateId is in valid range */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify tableSetId is in valid range */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Print out the Time To Advance values */
    cpssOsPrintf("\n==== Time Slot Attributes Table (for Gate ID[%d], Table Set ID[%d] ) ==== \n\n",gateId,tableSetId);

    /* gate_id[2:0] (3 LS bits) is used to select one of the 8 gates allocated per TableSet */
    gateSelect = gateId & BIT_MASK_MAC(3);

    /* Table Set start point entry in table */
    tableSetOfst = tableSetId * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

    /* Per Table set & Gate, go over all existing time slots and get parameters for each time slot */
    for (i=0; i<CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS; i++)
    {
        /* Read the entry of Time Slot Attributes table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_TIME_SLOT_ATTRIBUTES_E,
                                       tableSetOfst + i,
                                       hwValueArr);

        /* Parse the entry with the relevant fields */

        /* Calculate gate offset to point to right field taking into account 5 parameters for each gate */
        gateOffset = gateSelect * 5;

        /* Get newSlot */
        fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_0_E + gateOffset;
        SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                    devNum,
                    hwValueArr,
                    fieldId,
                    newSlot);

        /* Get streamGateState */
        fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_0_E + gateOffset;
        SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                    devNum,
                    hwValueArr,
                    fieldId,
                    streamGateState);

        /* Get lengthAware */
        fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_0_E + gateOffset;
        SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                    devNum,
                    hwValueArr,
                    fieldId,
                    lengthAware);

        /* Get ipv + keepTc: 4b0XXX - set TC to XXX, 4b1*** - keep incoming TC */
        fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_0_E + gateOffset;
        SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                    devNum,
                    hwValueArr,
                    fieldId,
                    ipv);

        /* Get intervalMaxOctetProfile */
        fieldId = SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_0_E + gateOffset;
        SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET_MAC(
                    devNum,
                    hwValueArr,
                    fieldId,
                    intervalMaxOctetProfile);

        /* Print out the Time slot attributes values per table set per gate */
        cpssOsPrintf("[time slot %3.3d]: newSlot %d, streamGateState %d, lengthAware %d, ipv %d, intervalMaxOctetProfile %3.3d \n"
                     ,i
                     ,newSlot
                     ,streamGateState
                     ,lengthAware
                     ,ipv
                     ,intervalMaxOctetProfile);
    }

    return rc;
}

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
)
{
    GT_STATUS  rc;            /* return code status                  */
    GT_U32     regAddr;       /* the register's address to read from */
    GT_U32     todSecMsb;     /* holds TOD seconds MSB value         */
    GT_U32     todSecLsb;     /* holds TOD seconds LSB value         */
    GT_U32     todNanoSec;    /* holds TOD nano seconds value        */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /* Get register address of debug TOD seconds MSB */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).debug.todSecMsb;

    /* AUTODOC: Read debug TOD seconds MSB */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &todSecMsb);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* Only 16 LS bits are valid */
    todSecMsb &= BIT_MASK_MAC(16);

    /* Get register address of debug TOD seconds LSB */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).debug.todSecLsb;

    /* AUTODOC: Read debug TOD seconds LSB */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &todSecLsb);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Get register address of debug TOD nano seconds */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).debug.todNanoSec;

    /* AUTODOC: Read debug TOD nano seconds */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &todNanoSec);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Print out TOD parameters */
    cpssOsPrintf("TOD seconds MSB: %d, TOD seconds LSB: %d, TOD nano seconds: %d \n"
                 ,todSecMsb
                 ,todSecLsb
                 ,todNanoSec);

    return rc;
}


/*************************************************************************************************************
  CPSS API functions
 *************************************************************************************************************/

/**
* @internal internal_cpssDxChStreamSgcGlobalConfigSet function
* @endinternal
*
* @brief  Set global configurations for Stream Gate Control (SGC) unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] gateParamsPtr  - (pointer to) global parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamSgcGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
)
{
    GT_STATUS                              rc;             /* return code                                  */
    GT_U32                                 regAddr;        /* the register's address to write to           */
    GT_U32                                 fieldOffset;    /* the start bit number in the register         */
    GT_U32                                 fieldLength;    /* the number of bits to be written to register */
    GT_U32                                 hwValue = 0;    /* data to be written into the register         */
    GT_U32                                 commandHwValue; /* command HW value                             */
    PRV_CPSS_DXCH_NET_DSA_TAG_CPU_CODE_ENT dsaCpuCode;     /* HW cpu code enumeration                      */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(globalParamsPtr);

    /**** Enable/disable SGC ****/
    /* Update bit#0 */
    fieldOffset = 0;
    fieldLength = 1;
    hwValue   = BOOL2BIT_MAC(globalParamsPtr->sgcGlobalEnable);

    /**** SGC gate closed exception parameters (command & drop code) ****/
    /* Update bits[1:11](include 2 fields: command(3b) + cpu code(8b) */
    fieldOffset = 1;
    fieldLength = 11;

    /* Verify packet command is valid and convert command into hardware value */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(commandHwValue, globalParamsPtr->gateClosedException.command);

    /* Convert generic cpu code into 'hw cpu code' */
    rc = prvCpssDxChNetIfCpuToDsaCode(globalParamsPtr->gateClosedException.cpuCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Combine exception command and cpu code */
    U32_SET_FIELD_MAC(hwValue,fieldOffset,fieldLength,((dsaCpuCode << 3) | commandHwValue));

    /**** SGC Interval Max Exceeded exception parameters (command & drop code) ****/
    /* Update bits[12:22](include 2 fields: command(3b) + cpu code(8b) */
    fieldOffset = 12;
    fieldLength = 11;

    /* Verify packet command is valid and convert command into hardware value */
    PRV_CPSS_CONVERT_PACKET_CMD_TO_HW_VAL_MAC(commandHwValue, globalParamsPtr->intervalMaxOctetExceededException.command);

    /* Convert generic cpu code into 'hw cpu code' */
    rc = prvCpssDxChNetIfCpuToDsaCode(globalParamsPtr->intervalMaxOctetExceededException.cpuCode, &dsaCpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Combine exception command and cpu code */
    U32_SET_FIELD_MAC(hwValue,fieldOffset,fieldLength,((dsaCpuCode << 3) | commandHwValue));

    /**** SGC counting mode ****/
    /* Update bit#23 */
    fieldOffset = 23;
    fieldLength = 1;
    U32_SET_FIELD_MAC(hwValue,fieldOffset,fieldLength,globalParamsPtr->sgcCountingModeL3);

    /**** SGC Timestamp/TOD mode ****/
    /* Update bit#24: SGC Ignore Timestamp (instead use the TOD) */
    fieldOffset = 24;
    fieldLength = 1;
    U32_SET_FIELD_MAC(hwValue,fieldOffset,fieldLength,globalParamsPtr->sgcIgnoreTimestampUseTod);

    /**** Write the 32 bits data into SGC Global Configuration register ****/
    /* Get SGC Global Configuration register address */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcGlobalConfig;

    /* Write the 32 bits data into the register */
    rc = prvCpssHwPpWriteRegister(devNum, regAddr, hwValue);

    return rc;
}

/**
* @internal cpssDxChStreamSgcGlobalConfigSet function
* @endinternal
*
* @brief  Set global configurations for Stream Gate Control (SGC) unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum         - device number.
* @param[in] gateParamsPtr  - (pointer to) global parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGlobalConfigSet
(
    IN  GT_U8                                   devNum,
    IN  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcGlobalConfigSet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalParamsPtr));

    rc = internal_cpssDxChStreamSgcGlobalConfigSet(devNum, globalParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSgcGlobalConfigGet function
* @endinternal
*
* @brief  Get global configurations for Stream Gate Control (SGC) unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum         - device number.
* @param[out] gateParamsPtr  - (pointer to) global parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamSgcGlobalConfigGet
(
    IN   GT_U8                                   devNum,
    OUT  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
)
{
    GT_STATUS                              rc;             /* return code                                  */
    GT_U32                                 regAddr;        /* the register's address to write to           */
    GT_U32                                 fieldOffset;    /* the start bit number in the register         */
    GT_U32                                 fieldLength;    /* the number of bits to be written to register */
    GT_U32                                 hwValue = 0;    /* data to be written into the register         */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(globalParamsPtr);

    /* Get SGC Global Configuration register address */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcGlobalConfig;

    /* Read the 32 bits data from the register */
    rc = prvCpssHwPpReadRegister(devNum, regAddr, &hwValue);
    if (rc != GT_OK)
    {
        return rc;
    }

    /**** Enable/disable SGC ****/
    /* Extract bit#0 */
    fieldOffset = 0;
    fieldLength = 1;
    globalParamsPtr->sgcGlobalEnable =  BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength));

    /**** SGC gate closed exception parameters (command & drop code) ****/
    /* Extract bits[1:3](command) */
    fieldOffset = 1;
    fieldLength = 3;
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(globalParamsPtr->gateClosedException.command,
                                              U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength) );

    /* Extract bits[4:11](drop code) */
    fieldOffset = 4;
    fieldLength = 8;
    /* Convert generic cpu code into 'hw cpu code' */
    rc = prvCpssDxChNetIfDsaToCpuCode(U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength),
                                      &globalParamsPtr->gateClosedException.cpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /**** SGC Interval Max Exceeded exception parameters (command & drop code) ****/
    /* Extract bits[12:14](command) */
    fieldOffset = 12;
    fieldLength = 3;
    PRV_CPSS_CONVERT_HW_VAL_TO_PACKET_CMD_MAC(globalParamsPtr->intervalMaxOctetExceededException.command,
                                              U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength) );

    /* Extract bits[15:22](drop code) */
    fieldOffset = 15;
    fieldLength = 8;
    /* Convert generic cpu code into 'hw cpu code' */
    rc = prvCpssDxChNetIfDsaToCpuCode(U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength),
                                      &globalParamsPtr->intervalMaxOctetExceededException.cpuCode);
    if (rc != GT_OK)
    {
        return rc;
    }

    /**** SGC counting mode ****/
    /* Extract bit#23 */
    fieldOffset = 23;
    fieldLength = 1;
    globalParamsPtr->sgcCountingModeL3 =  BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength));

    /**** SGC Timestamp/TOD mode ****/
    /* Extracts bit#24 */
    fieldOffset = 24;
    fieldLength = 1;
    globalParamsPtr->sgcIgnoreTimestampUseTod = BIT2BOOL_MAC(U32_GET_FIELD_MAC(hwValue,fieldOffset,fieldLength));
    return rc;
}

/**
* @internal cpssDxChStreamSgcGlobalConfigGet function
* @endinternal
*
* @brief  Get global configurations for Stream Gate Control (SGC) unit.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum         - device number.
* @param[out] gateParamsPtr  - (pointer to) global parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGlobalConfigGet
(
    IN   GT_U8                                   devNum,
    OUT  CPSS_DXCH_STREAM_SGC_GLOBAL_CONFIG_STC  *globalParamsPtr
)
{
    GT_STATUS rc;
    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcGlobalConfigGet);

    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum, globalParamsPtr));

    rc = internal_cpssDxChStreamSgcGlobalConfigGet(devNum, globalParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum, globalParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSgcGateConfigSet function
* @endinternal
*
* @brief  Set direct gate configurations to take place upon API complete
*
*         Note: it might have some traffic affect
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
* @param[in] slotsNum              - number of time slots to fill
*                                    (APPLICABLE RANGES: 1..256)
* @param[in] imxProfilesNum        - number of interval max profiles
*                                    (APPLICABLE RANGES: 1..256)
* @param[in] gateParamsPtr         - (pointer to) gate parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamSgcGateConfigSet
(
    IN GT_U8                               devNum,
    IN GT_U32                              gateId,
    IN GT_U32                              tableSetId,
    IN GT_U32                              slotsNum,
    IN GT_U32                              imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC  *gateParamsPtr
)
{
    GT_STATUS  rc;                   /* return code status                                                 */
    GT_U32     i;                    /* loop counter used for walk through all IMX profiles and time slots */
    GT_U32     timeSlotTablesEntry;  /* entry index to Time slot tables                                    */
    GT_U32     gateSelect;           /* gate number selected out of 8 options per entry                    */
    GT_U8      gclId;                /* 6 MS bits of Gate ID                                               */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(gateParamsPtr);

    /* Verify gate id number is valid */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify table set id number is valid */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Verify number of time slots is valid */
    SMU_SGC_TIME_SLOTS_NUMNER_CHECK_MAC(slotsNum);

    /* Verify number of interval max profiles is valid */
    SMU_SGC_IMX_PROFILES_NUMNER_CHECK_MAC(imxProfilesNum)


    /******* Set Interval Max values *******/
    /* Loop over all requested profiles. Each entry represents the maximum allowed number of bytes per time slot */
    for (i=0; i<imxProfilesNum; i++)
    {
        /* Write to SGC Interval Max table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E,
                                        i,
                                        &gateParamsPtr->intervalMaxArr[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******* Set Table Set Time configurations values *******/
    /* Set time parameters values for specific table set */
    rc = streamSgcTableSetTimeConfigInfoEntrySet(devNum,
                                                 tableSetId,
                                                 &gateParamsPtr->tableSetInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******* Set time slot parameters per table set per gate *******/
    /* gate_id[2:0] (3 LS bits) is used to select one of the 8 gates allocated per TableSet */
    gateSelect = gateId & BIT_MASK_MAC(3);

    /* Calculate table entry index for time slot tables per Table set */
    timeSlotTablesEntry = tableSetId * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

    /* Per Table set & Gate, go over all requested time slots and set parameters for each time slot */
    for (i=0; i<slotsNum; i++)
    {
        /* Set entry in time slot tables */
        rc = streamSgcTimeSlotInfoEntrySet(devNum,
                                           timeSlotTablesEntry + i,
                                           gateSelect,
                                           imxProfilesNum,
                                           &(gateParamsPtr->timeSlotInfoArr[i]));
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /* Extract 6 MSb from gateId (gate_id[8:3]) */
    gclId = (gateId >> 3) & BIT_MASK_MAC(6);

    /* Map Table Set to GCL ID */
    rc = streamSgcMapGateId2TableSetConfig(devNum, gclId, tableSetId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set Table Set to active */
    rc = streamSgcTableSetActiveSet(devNum, tableSetId, GT_TRUE);

    return rc;
}

/**
* @internal cpssDxChStreamSgcGateConfigSet function
* @endinternal
*
* @brief  Set direct gate configurations to take place upon API complete
*
*         Note: it might have some traffic affect
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
* @param[in] slotsNum              - number of time slots to fill
*                                    (APPLICABLE RANGES: 1..256)
* @param[in] imxProfilesNum        - number of interval max profiles
*                                    (APPLICABLE RANGES: 1..256)
* @param[in] gateParamsPtr         - (pointer to) gate parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGateConfigSet
(
    IN GT_U8                               devNum,
    IN GT_U32                              gateId,
    IN GT_U32                              tableSetId,
    IN GT_U32                              slotsNum,
    IN GT_U32                              imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC  *gateParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcGateConfigSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr));

    rc = internal_cpssDxChStreamSgcGateConfigSet(devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSgcGateConfigGet function
* @endinternal
*
* @brief  Get direct gate configurations
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - device number.
* @param[in]  gateId                - the Gate-ID assigned to the flow
*                                     (APPLICABLE RANGES: 0..510)
*                                     0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId            - the Table Set ID that is mapped to the gateId
*                                     (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum              - number of time slots to fill
*                                     (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum        - number of interval max profiles
*                                     (APPLICABLE RANGES: 1..256)
* @param[out] gateParamsPtr         - (pointer to) gate parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamSgcGateConfigGet
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                gateId,
    IN  GT_U32                                tableSetId,
    IN  GT_U32                                slotsNum,
    IN  GT_U32                                imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC    *gateParamsPtr
)
{
    GT_STATUS  rc;                     /* return code status                                                 */
    GT_U32     i;                      /* loop counter used for walk through all IMX profiles and time slots */
    GT_U32     timeSlotTablesEntry;    /* entry index to Time slot tables                                    */
    GT_U32     gateSelect;             /* gate number selected out of 8 options per entry                    */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(gateParamsPtr);

    /* Verify gate id number is valid */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify table set id number is valid */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Verify number of time slots is valid */
    SMU_SGC_TIME_SLOTS_NUMNER_CHECK_MAC(slotsNum);

    /* Verify number of interval max profiles is valid */
    SMU_SGC_IMX_PROFILES_NUMNER_CHECK_MAC(imxProfilesNum)


    /******* Get Interval Max values *******/
    /* Loop over all requested profiles. Each entry represents the maximum allowed number of bytes per time slot */
    for (i=0; i<imxProfilesNum; i++)
    {
        /* Read SGC Interval Max table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E,
                                       i,
                                       &gateParamsPtr->intervalMaxArr[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******* Get Table Set Time configurations values *******/
    /* Read time parameters values for specific table set */
    rc = streamSgcTableSetTimeConfigInfoEntryGet(devNum,
                                                 tableSetId,
                                                 &gateParamsPtr->tableSetInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******* Get time slot parameters per table set per gate *******/
    /* gate_id[2:0] (3 LS bits) is used to select one of the 8 gates allocated per TableSet */
    gateSelect = gateId & BIT_MASK_MAC(3);

    /* Calculate table entry index for time slot tables per Table set */
    timeSlotTablesEntry = tableSetId * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

    /* Per Table set & Gate, go over all requested time slots and read parameters from each time slot */
    for (i=0; i<slotsNum; i++)
    {
        /* Read entry from time slot tables */
        rc = streamSgcTimeSlotInfoEntryGet(devNum,
                                           timeSlotTablesEntry + i,
                                           gateSelect,
                                           &(gateParamsPtr->timeSlotInfoArr[i]));
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

/**
* @internal cpssDxChStreamSgcGateConfigGet function
* @endinternal
*
* @brief  Get direct gate configurations
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                - device number.
* @param[in]  gateId                - the Gate-ID assigned to the flow
*                                     (APPLICABLE RANGES: 0..510)
*                                     0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId            - the Table Set ID that is mapped to the gateId
*                                     (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum              - number of time slots to fill
*                                     (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum        - number of interval max profiles
*                                     (APPLICABLE RANGES: 1..256)
* @param[out] gateParamsPtr         - (pointer to) gate parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcGateConfigGet
(
    IN  GT_U8                                 devNum,
    IN  GT_U32                                gateId,
    IN  GT_U32                                tableSetId,
    IN  GT_U32                                slotsNum,
    IN  GT_U32                                imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC    *gateParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcGateConfigGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr));

    rc = internal_cpssDxChStreamSgcGateConfigGet(devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSgcTimeBasedGateReConfigSet function
* @endinternal
*
* @brief  Set time based gate configurations which takes place at specific time
*
* Note: in order to complete the reconfiguration process properly without affecting the
*       traffic and to enable next future reconfigurations, must call following CPSS API
*       cpssDxChStreamSgcTimeBasedGateReConfigComplete before reconfiguration time occurs.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                   - device number.
* @param[in]  gateId                   - the Gate-ID assigned to the flow
*                                        (APPLICABLE RANGES: 0..510)
*                                         0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId               - the Table Set ID that is mapped to the gateId
*                                        (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum                 - number of time slots to fill
*                                        (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum           - number of interval max profiles
*                                        (APPLICABLE RANGES: 1..256)
* @param[in] gateParamsPtr             - (pointer to) gate parameters
* @param[in] reconfigurationParamsPtr  - (pointer to) re-configuration parameters
*
* @retval GT_OK                        - on success
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_BAD_PARAM                 - one of the input parameters is not valid
* @retval GT_BAD_PTR                   - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamSgcTimeBasedGateReConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       gateId,
    IN GT_U32                                       tableSetId,
    IN GT_U32                                       slotsNum,
    IN GT_U32                                       imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    IN CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
)
{
    GT_STATUS  rc;                   /* return code status                                                 */
    GT_U32     i;                    /* loop counter used for walk through all IMX profiles and time slots */
    GT_U32     timeSlotTablesIndex;  /* index to Time slot tables                                          */
    GT_U32     gateSelect;           /* gate number selected out of 8 options per entry                    */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(gateParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(gateReconfigTimeParamsPtr);

    /* Verify gate id number is valid */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify table set id number is valid */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Verify number of time slots is valid */
    SMU_SGC_TIME_SLOTS_NUMNER_CHECK_MAC(slotsNum);

    /* Verify number of interval max profiles is valid */
    SMU_SGC_IMX_PROFILES_NUMNER_CHECK_MAC(imxProfilesNum)


    /******* Set Interval Max values *******/
    /* Loop over all requested profiles. Each entry represents the maximum allowed number of bytes per time slot */
    for (i=0; i<imxProfilesNum; i++)
    {
        /* Write to SGC Interval Max table */
        rc = prvCpssDxChWriteTableEntry(devNum,
                                        CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E,
                                        i,
                                        &gateParamsPtr->intervalMaxArr[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******* Set Table Set Time configurations values *******/
    /* Set time parameters values for specific table set */
    rc = streamSgcTableSetTimeConfigInfoEntrySet(devNum,
                                                 tableSetId,
                                                 &gateParamsPtr->tableSetInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******* Set time slot parameters per table set per gate *******/
    /* gate_id[2:0] (3 LS bits) is used to select one of the 8 gates allocated per TableSet */
    gateSelect = gateId & BIT_MASK_MAC(3);

    /* Calculate table index for time slot tables per Table set */
    timeSlotTablesIndex = tableSetId * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

    /* Per Table set & Gate, go over all requested time slots and set parameters for each time slot */
    for (i=0; i<slotsNum; i++)
    {
        /* Set entry in time slot tables */
        rc = streamSgcTimeSlotInfoEntrySet(devNum,
                                           timeSlotTablesIndex + i,
                                           gateSelect,
                                           imxProfilesNum,
                                           &(gateParamsPtr->timeSlotInfoArr[i]));
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******* Set re-configuration parameters  *******/
    rc = streamSgcReconfigurationSet(devNum, gateId, tableSetId, gateReconfigTimeParamsPtr);

    return rc;
}

/**
* @internal cpssDxChStreamSgcTimeBasedGateReConfigSet function
* @endinternal
*
* @brief  Set time based gate configurations which takes place at specific time
*
* Note: in order to complete the reconfiguration process properly without affecting the
*       traffic and to enable next future reconfigurations, must call following CPSS API
*       cpssDxChStreamSgcTimeBasedGateReConfigComplete before reconfiguration time occurs.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                   - device number.
* @param[in]  gateId                   - the Gate-ID assigned to the flow
*                                        (APPLICABLE RANGES: 0..510)
*                                         0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId               - the Table Set ID that is mapped to the gateId
*                                        (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum                 - number of time slots to fill
*                                        (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum           - number of interval max profiles
*                                        (APPLICABLE RANGES: 1..256)
* @param[in] gateParamsPtr             - (pointer to) gate parameters
* @param[in] reconfigurationParamsPtr  - (pointer to) re-configuration parameters
*
* @retval GT_OK                        - on success
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_BAD_PARAM                 - one of the input parameters is not valid
* @retval GT_BAD_PTR                   - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigSet
(
    IN GT_U8                                        devNum,
    IN GT_U32                                       gateId,
    IN GT_U32                                       tableSetId,
    IN GT_U32                                       slotsNum,
    IN GT_U32                                       imxProfilesNum,
    IN CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    IN CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcTimeBasedGateReConfigSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr,gateReconfigTimeParamsPtr));

    rc = internal_cpssDxChStreamSgcTimeBasedGateReConfigSet(devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr, gateReconfigTimeParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr,gateReconfigTimeParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSgcTimeBasedGateReConfigGet function
* @endinternal
*
* @brief  Get time based gate re-configurations parameters
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                    - device number.
* @param[in]  gateId                    - the Gate-ID assigned to the flow
*                                         (APPLICABLE RANGES: 0..510)
*                                          0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId                - the Table Set ID that is mapped to the gateId
*                                         (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum                  - number of time slots to fill
*                                         (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum            - number of interval max profiles
*                                         (APPLICABLE RANGES: 1..256)
* @param[out] gateParamsPtr             - (pointer to) gate parameters
* @param[out] reconfigurationParamsPtr  - (pointer to) re-configuration parameters
*
* @retval GT_OK                        - on success
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_BAD_PARAM                 - one of the input parameters is not valid
* @retval GT_BAD_PTR                   - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamSgcTimeBasedGateReConfigGet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       gateId,
    IN  GT_U32                                       tableSetId,
    IN  GT_U32                                       slotsNum,
    IN  GT_U32                                       imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    OUT CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
)
{
    GT_STATUS  rc;                     /* return code status                                                 */
    GT_U32     i;                      /* loop counter used for walk through all IMX profiles and time slots */
    GT_U32     timeSlotTablesIndex;    /* index to Time slot tables                                          */
    GT_U32     gateSelect;             /* gate number selected out of 8 options per entry                    */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(gateParamsPtr);
    CPSS_NULL_PTR_CHECK_MAC(gateReconfigTimeParamsPtr);

    /* Verify gate id number is valid */
    SMU_SGC_GATE_ID_INDEX_CHECK_MAC(gateId);

    /* Verify table set id number is valid */
    SMU_SGC_TABLE_SET_ID_INDEX_CHECK_MAC(tableSetId);

    /* Verify number of time slots is valid */
    SMU_SGC_TIME_SLOTS_NUMNER_CHECK_MAC(slotsNum);

    /* Verify number of interval max profiles is valid */
    SMU_SGC_IMX_PROFILES_NUMNER_CHECK_MAC(imxProfilesNum)


    /******* Get Interval Max values *******/
    /* Loop over all requested profiles. Each entry represents the maximum allowed number of bytes per time slot */
    for (i=0; i<imxProfilesNum; i++)
    {
        /* Read SGC Interval Max table */
        rc = prvCpssDxChReadTableEntry(devNum,
                                       CPSS_DXCH_SIP6_30_TABLE_SMU_SGC_INTERVAL_MAX_E,
                                       i,
                                       &gateParamsPtr->intervalMaxArr[i]);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******* Get Table Set Time configurations values *******/
    /* Read time parameters values for specific table set */
    rc = streamSgcTableSetTimeConfigInfoEntryGet(devNum,
                                                 tableSetId,
                                                 &gateParamsPtr->tableSetInfo);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******* Get time slot parameters per table set per gate *******/
    /* gate_id[2:0] (3 LS bits) is used to select one of the 8 gates allocated per TableSet */
    gateSelect = gateId & BIT_MASK_MAC(3);

    /* Calculate table index for time slot tables per Table set */
    timeSlotTablesIndex = tableSetId * CPSS_DXCH_STREAM_SGC_MAX_TIME_SLOTS_NUM_CNS;

    /* Per Table set & Gate, go over all requested time slots and read parameters from each time slot */
    for (i=0; i<slotsNum; i++)
    {
        /* Read entry from time slot tables */
        rc = streamSgcTimeSlotInfoEntryGet(devNum,
                                           timeSlotTablesIndex + i,
                                           gateSelect,
                                           &(gateParamsPtr->timeSlotInfoArr[i]));
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /******* Get re-configuration parameters  *******/
    rc = streamSgcReconfigurationGet(devNum, gateReconfigTimeParamsPtr);

    return rc;
}

/**
* @internal cpssDxChStreamSgcTimeBasedGateReConfigGet function
* @endinternal
*
* @brief  Get time based gate re-configurations parameters
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum                    - device number.
* @param[in]  gateId                    - the Gate-ID assigned to the flow
*                                         (APPLICABLE RANGES: 0..510)
*                                          0x1FF - indicates Gate Control List is disabled
* @param[in]  tableSetId                - the Table Set ID that is mapped to the gateId
*                                         (APPLICABLE RANGES: 0..57)
* @param[in]  slotsNum                  - number of time slots to fill
*                                         (APPLICABLE RANGES: 1..256)
* @param[in]  imxProfilesNum            - number of interval max profiles
*                                         (APPLICABLE RANGES: 1..256)
* @param[out] gateParamsPtr             - (pointer to) gate parameters
* @param[out] reconfigurationParamsPtr  - (pointer to) re-configuration parameters
*
* @retval GT_OK                        - on success
* @retval GT_HW_ERROR                  - on hardware error
* @retval GT_BAD_PARAM                 - one of the input parameters is not valid
* @retval GT_BAD_PTR                   - one of the parameters is NULL pointer
* @retval GT_NOT_APPLICABLE_DEVICE     - on not applicable device
*/
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigGet
(
    IN  GT_U8                                        devNum,
    IN  GT_U32                                       gateId,
    IN  GT_U32                                       tableSetId,
    IN  GT_U32                                       slotsNum,
    IN  GT_U32                                       imxProfilesNum,
    OUT CPSS_DXCH_STREAM_SGC_GATE_INFO_STC           *gateParamsPtr,
    OUT CPSS_DXCH_STREAM_SGC_GATE_RECONFIG_TIME_STC  *gateReconfigTimeParamsPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcTimeBasedGateReConfigGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr,gateReconfigTimeParamsPtr));

    rc = internal_cpssDxChStreamSgcTimeBasedGateReConfigGet(devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr,gateReconfigTimeParamsPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId,rc,devNum,gateId,tableSetId,slotsNum,imxProfilesNum,gateParamsPtr,gateReconfigTimeParamsPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamSgcTimeBasedGateReConfigComplete function
* @endinternal
*
* @brief  Read re-configuration pending bit to verify if re-configuration time was matched.
*         If matched continue with last configurations to enable future re-configurations.
*         - get re-configuration mapping of GCL ID to Table Set ID
*         - set new mapping according to re-configuration mapping
*         - clear Valid status bit to indicate process is ready
*         - Set the Table Set to active
*
*         Note:
*         - Must call this API after re-configuration time.
*           If API is called before time is matched it will return with failure status.
*         - Must call CPSS API cpssDxChStreamSgcTimeBasedGateReConfigSet first
*           to set reconfiguration parameters and reconfiguration time.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - if pending status bit was not cleared
*/
static GT_STATUS internal_cpssDxChStreamSgcTimeBasedGateReConfigComplete
(
    IN GT_U8     devNum
)
{
    GT_STATUS  rc;                  /* return code status                           */
    GT_U32     hwValue = 0;         /* data to be read from the register            */
    GT_U32     regAddr;             /* the register's address to write to           */
    GT_U32     fieldOffset;         /* the start bit number in the register         */
    GT_U32     fieldLength;         /* the number of bits to be written to register */
    GT_BIT     pendingStatus;       /* pending status bit                           */
    GT_U8      tableSetId;          /* the Table Set ID                             */
    GT_U8      gclId;               /* the GCL ID                                   */

    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    /*******************************************************************************
      Read re-configuration Pending status bit from HW register to verify if
      re-configuration time was indeed matched. If not return with error indication.
     *******************************************************************************/
    /* Field size of 1 bit */
    fieldLength = 1;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration Pending Status */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigPendingStatus;

    /* Read Pending status bit from the register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }
    pendingStatus = hwValue;

    /* Check if Pending bit was cleared by SGC HW unit which means re-configuration time was matched
       If Pending bit is still not cleared return with error indication */
    if (pendingStatus == 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,
                                      "Pending status bit was not cleared by Stream Gate Control (SGC) HW unit."
                                      "No match to re-configuration time");
    }

    /***********************************************************************************
      Update origin mapping of GCL ID to Table Set ID with re-configuration mapping
      At this point re-configuration mapping has strict priority over origin mapping
     ***********************************************************************************/
    /* Field size of 12 bits */
    fieldLength = 12;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration Table Set Configurations */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigTableSetConfig;

    /* Read it from the register */
    rc = prvCpssHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, &hwValue);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Extract re-configuration tableSetId bits[6:11] */
    tableSetId  = (hwValue >> 6) & BIT_MASK_MAC(6);

    /* Extract re-configuration gclId bits[5:0] */
    gclId  = hwValue & BIT_MASK_MAC(6);

    /* Update origin mapping with re-configuration GCL ID to Table Set ID */
    rc = streamSgcMapGateId2TableSetConfig(devNum, gclId, tableSetId);
    if(rc != GT_OK)
    {
        return rc;
    }

    /******************************************************************************
      Clear re-configuration Valid bit Status to '0'
      Cause the new mapping to have strict priority over re-configuration mapping
     ******************************************************************************/
    /* Field size of 1 bit */
    fieldLength = 1;

    /* Field start bit */
    fieldOffset = 0;

    /* Get register address of SGC Reconfiguration Valid Status */
    regAddr = PRV_DXCH_REG1_UNIT_SMU_MAC(devNum).streamGateControl.sgcReconfigValidStatus;

    /* Clear Valid bit to '0' */
    hwValue = 0;

    /* Write it into register */
    rc = prvCpssHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Set Table Set to active */
    rc = streamSgcTableSetActiveSet(devNum, tableSetId, GT_TRUE);

    return rc;
}

/**
* @internal cpssDxChStreamSgcTimeBasedGateReConfigComplete function
* @endinternal
*
* @brief  Read re-configuration pending bit to verify if re-configuration time was matched.
*         If matched continue with last configurations to enable future re-configurations.
*         - get re-configuration mapping of GCL ID to Table Set ID
*         - set new mapping according to re-configuration mapping
*         - clear Valid status bit to indicate process is ready
*         - Set the Table Set to active
*
*         Note:
*         - Must call this API after re-configuration time.
*           If API is called before time is matched it will return with failure status.
*         - Must call CPSS API cpssDxChStreamSgcTimeBasedGateReConfigSet first
*           to set reconfiguration parameters and reconfiguration time.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in]  devNum               - device number.
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - if pending status bit was not cleared
*/
GT_STATUS cpssDxChStreamSgcTimeBasedGateReConfigComplete
(
    IN GT_U8     devNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamSgcTimeBasedGateReConfigComplete);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamSgcTimeBasedGateReConfigComplete(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamEgressTableSetConfigSet function
* @endinternal
*
* @brief  802.1Qbv  - Configure tableset entries.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   -   physical device number
* @param[in] tableSet                    -  tableset index
* @param[in] slotsNum                    -number of slots to fill
* @param[in] timeSlotInfoArr           array of  configuration slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamEgressTableSetConfigSet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    IN CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotInfoArr
)
{
    GT_STATUS rc;
    /* Check parameters */
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(timeSlotInfoArr);

    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssSip6_30TxqQbvConfigureTableSet(devNum,tableSet,slotsNum,timeSlotInfoArr);

    return rc;
}

/**
* @internal cpssDxChStreamEgressTableSetConfigSet function
* @endinternal
*
* @brief  802.1Qbv  - Configure tableset entries.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   -   physical device number
* @param[in] tableSet                    -  tableset index
* @param[in] slotsNum                    -number of slots to fill
* @param[in] timeSlotInfoArr           array of  configuration slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressTableSetConfigSet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    IN CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotInfoArr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressTableSetConfigSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressTableSetConfigSet(devNum,tableSet,slotsNum,timeSlotInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,tableSet,slotsNum,timeSlotInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamEgressTableSetConfigGet function
* @endinternal
*
* @brief  802.1Qbv  - Get  tableset entries
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   -   physical device number
* @param[in] tableSet                    -  tableset index
* @param[in] slotsNum                    -number of slots to fill
* @param[out] timeSlotInfoArr           array of  configuration slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamEgressTableSetConfigGet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    OUT CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotInfoArr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(timeSlotInfoArr);

    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssSip6_30TxqQbvConfigureTableGet(devNum,tableSet,slotsNum,timeSlotInfoArr);

    return rc;
}


/**
* @internal cpssDxChStreamEgressTableSetConfigGet function
* @endinternal
*
* @brief  802.1Qbv  - Get  tableset entries
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   -   physical device number
* @param[in] tableSet                    -  tableset index
* @param[in] slotsNum                    -number of slots to fill
* @param[out] timeSlotInfoArr           array of  configuration slots
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressTableSetConfigGet
(
    IN GT_U8                devNum,
    IN GT_U32               tableSet,
    IN GT_U32               slotsNum,
    OUT CPSS_DXCH_STREAM_EGRESS_TIME_SLOT_INFO_STC  *timeSlotInfoArr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressTableSetConfigGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressTableSetConfigGet(devNum,tableSet,slotsNum,timeSlotInfoArr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,tableSet,slotsNum,timeSlotInfoArr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal cpssDxChStreamEgressPortBindSet function
* @endinternal
*
* @brief  Bind port to spesific table set using bind parameters.
*
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @param[in] tableSetId            - the Table Set ID that port  is mapped to
*                                    (APPLICABLE RANGES: 0..28)
* @param[in] bindEntryPtr         - (pointer to) bind entry  parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS internal_cpssDxChStreamEgressPortBindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
)
{
    GT_STATUS rc;
    GT_BOOL   qbvCapable;
    GT_U32    errorCode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(bindEntryPtr);

    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssSip6_30TxqPortQbvCapabilityGet(devNum,portNum,&qbvCapable,&errorCode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if(GT_FALSE==qbvCapable)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Port %d is not QBV capable.errorCode %d\n",portNum,errorCode);
    }

    rc = prvCpssSip6_30TxqQbvBindPortToTableSet(devNum,portNum,tableSet,bindEntryPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6_30TxqPortQbvEnableSet(devNum,portNum,GT_TRUE);

    return rc;
}


/**
* @internal cpssDxChStreamEgressPortBindSet function
* @endinternal
*
* @brief  Bind port to spesific table set using bind parameters.
*
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @param[in] tableSetId            - the Table Set ID that port  is mapped to
*                                    (APPLICABLE RANGES: 0..28)
* @param[in] bindEntryPtr         - (pointer to) bind entry  parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamEgressPortBindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32               tableSet,
    IN CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressPortBindSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressPortBindSet(devNum,portNum,tableSet,bindEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,,portNum,tableSet,bindEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_ cpssDxChStreamEgressPortBindGet function
* @endinternal
*
* @brief  Get  port  bind parameters.
*
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @param[in] tableSetId            - (pointer to) the Table Set ID that port  is mapped to
*                                    (APPLICABLE RANGES: 0..28)
* @param[out] bindEntryPtr         - (pointer to) bind entry  parameters
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/

static GT_STATUS internal_cpssDxChStreamEgressPortBindGet
(
    IN   GT_U8                devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32               *tableSetPtr,
    INOUT  CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
)
{
    GT_STATUS rc;
    GT_BOOL   qbvCapable;
    GT_U32    errorCode;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(tableSetPtr);
    CPSS_NULL_PTR_CHECK_MAC(bindEntryPtr);

    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssSip6_30TxqPortQbvCapabilityGet(devNum,portNum,&qbvCapable,&errorCode);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }


    if(GT_FALSE==qbvCapable)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM,"Port %d is not QBV capable.errorCode %d\n",portNum,errorCode);
    }

    return prvCpssSip6_30TxqQbvBindPortToTableGet(devNum,portNum,tableSetPtr,bindEntryPtr);
}

/**
* @internal cpssDxChStreamEgressPortBindGet function
* @endinternal
*
* @brief  Get  port  bind parameters.
*
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @param[out] tableSetPtr            - (pointer to) the Table Set ID that port  is mapped to
*                                    (APPLICABLE RANGES: 0..28)
* @param[inout] bindEntryPtr         - (pointer to) bind entry  parameters
*   Note that reconfiguration time is not updated .(i.e gateReconfigTimeParam)
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssDxChStreamEgressPortBindGet
(
    IN   GT_U8                devNum,
    IN   GT_PHYSICAL_PORT_NUM portNum,
    OUT  GT_U32               *tableSetPtr,
    INOUT  CPSS_DXCH_STREAM_EGREESS_PORT_BIND_ENTRY_STC  *bindEntryPtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressPortBindSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressPortBindGet(devNum,portNum,tableSetPtr,bindEntryPtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,,portNum,tableSetPtr,bindEntryPtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChStreamEgressPortBindComplete function
* @endinternal
*
* @brief  Polls on status of configuration pending bit.
*
*         Note: must call CPSS API cpssDxChStreamEgressPortBindSet first
*               to set reconfiguration parameters and reconfiguration time.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT - after max number of retries checking if pending status bit is cleared
*
*/
static GT_STATUS internal_cpssDxChStreamEgressPortBindComplete
(
    IN GT_U8     devNum
)
{
    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);


    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }


    return prvCpssSip6_30TxqQbvChangeReqCompleteGet(devNum);
}


/**
* @internal cpssDxChStreamEgressPortBindComplete function
* @endinternal
*
* @brief  Polls on status of configuration pending bit.
*
*         Note: must call CPSS API cpssDxChStreamEgressPortBindSet first
*               to set reconfiguration parameters and reconfiguration time.
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_TIMEOUT - after max number of retries checking if pending status bit is cleared
*
*/
GT_STATUS cpssDxChStreamEgressPortBindComplete
(
    IN GT_U8     devNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressPortBindComplete);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressPortBindComplete(devNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}
/**
* @internal cpssDxChStreamEgressPortUnbindSet function
* @endinternal
*
* @brief   Unbind port from  table set.
*
*         Note: Can not be performed under traffic.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE  - port under traffic
*/
static GT_STATUS internal_cpssDxChStreamEgressPortUnbindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;
    GT_U32    bufferNum;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);


    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssFalconTxqUtilsPortTxBufNumberGet(devNum,portNum,&bufferNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc,LOG_ERROR_NO_MSG);
    }

    if(0!=bufferNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE,"Unbind can not be done under traffic\n");
    }

    rc = prvCpssSip6_30TxqPortQbvEnableSet(devNum,portNum,GT_FALSE);

    return rc;
}


/**
* @internal cpssDxChStreamEgressPortUnbindSet function
* @endinternal
*
* @brief   Unbind port from  table set.
*
*         Note: Can not be performed under traffic.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier.
*
* @param[in] devNum                - device number.
* @param[in] portNum                - physical port number
*
* @retval GT_OK                    - on success
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - one of the input parameters is not valid
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE  - port under traffic
*/
GT_STATUS cpssDxChStreamEgressPortUnbindSet
(
    IN GT_U8                devNum,
    IN GT_PHYSICAL_PORT_NUM portNum
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressPortUnbindSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressPortUnbindSet(devNum,portNum);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,,portNum));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}


/**
* @internal internal_cpssDxChStreamEgressPortQueueGateSet function
* @endinternal
*
* @brief   Sets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[in] gate                       - gate  that this queue  is mapped to(APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
static GT_STATUS internal_cpssDxChStreamEgressPortQueueGateSet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    IN GT_U32 gate
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);

    PRV_CPSS_DXCH_COS_CHECK_TC_MAC(gate);
    PRV_CPSS_DXCH_SIP_6_QUEUE_OFFSET_VALIDATE_MAC(devNum,portNum,queueOffset);


    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssSip6_30TxqPortQueueGateSet(devNum,portNum,queueOffset,gate);

    return rc;
}

/**
* @internal cpssDxChStreamEgressPortQueueGateSet function
* @endinternal
*
* @brief   Sets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[in] gate                       - gate  that this queue  is mapped to(APPLICABLE RANGES: 0..7).
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressPortQueueGateSet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    IN GT_U32 gate
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressPortQueueGateSet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressPortQueueGateSet(devNum,portNum,queueOffset,gate);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,portNum,queueOffset,gate));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal internal_cpssDxChStreamEgressPortQueueGateGet function
* @endinternal
*
* @brief   Gets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[out] gatePtr                       - (pointer to)gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS internal_cpssDxChStreamEgressPortQueueGateGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    OUT GT_U32 *gatePtr
)
{
    GT_STATUS rc;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_DXCH_TSN_SUPPORTED_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum,
        CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E |
        CPSS_BOBCAT2_E | CPSS_BOBCAT3_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_AC3X_E |
        CPSS_ALDRIN2_E | CPSS_FALCON_E |  CPSS_AC5P_E |  CPSS_AC5X_E | CPSS_HARRIER_E);
    CPSS_NULL_PTR_CHECK_MAC(gatePtr);
    PRV_CPSS_DXCH_SIP_6_QUEUE_OFFSET_VALIDATE_MAC(devNum,portNum,queueOffset);

    if(GT_FALSE== TXQ_IS_QBV_CAPABLE_DEVICE(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED,"Device %d is not QBV capable\n",devNum);
    }

    rc = prvCpssSip6_30TxqPortQueueGateGet(devNum,portNum,queueOffset,gatePtr);

    return rc;

}



/**
* @internal cpssDxChStreamEgressPortQueueGateGet function
* @endinternal
*
* @brief   Gets gate  to queue map. Meaning this register field  define
*          which Q should be stoped  on closing of perticular gate
*
* @note   APPLICABLE DEVICES:     Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier;
*
* @param[in] devNum                   - physical device number
* @param[in] portNum               -physical port number
* @param[in] queueOffset              - queue offset within the port
* @param[out] gatePtr                       - (pointer to)gate  that this queue  is mapped to.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS cpssDxChStreamEgressPortQueueGateGet
(
    IN GT_U8  devNum,
    IN GT_PHYSICAL_PORT_NUM portNum,
    IN GT_U32 queueOffset,
    OUT GT_U32 *gatePtr
)
{
    GT_STATUS rc;

    CPSS_LOG_FUNC_VARIABLE_DECLARE_MAC(funcId, cpssDxChStreamEgressPortQueueGateGet);
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    CPSS_LOG_API_ENTER_MAC((funcId, devNum));

    rc = internal_cpssDxChStreamEgressPortQueueGateGet(devNum,portNum,queueOffset,gatePtr);

    CPSS_LOG_API_EXIT_MAC(funcId, rc);
    CPSS_APP_SPECIFIC_CB_MAC((funcId, rc, devNum,portNum,queueOffset,gatePtr));
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}





