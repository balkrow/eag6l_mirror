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
* @file smemIronman.h
*
* @brief Ironman memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemIronmanh
#define __smemIronmanh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemHarrier.h>

ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemSip6_30ActiveReadFdbHsrPrpTimerReg);
ACTIVE_READ_FUNC_PROTOTYPE_MAC(smemSip6_30ActiveReadMacGopPtpTxTsQueueReg2Reg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSip6_30ActiveWriteFdbHsrPrpTimerReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSip6_30ActiveWriteFdbHsrPrpGlobalConfigReg);
ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemSip6_30ActiveWritePntReadyReg);

/**
* @internal smemIronmanInit function
* @endinternal
*
* @brief   Init memory module for a Falcon device.
*/
void smemIronmanInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemIronmanInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemIronmanInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/*******************************************************************************
*   smemIronmanInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the Hawk device
*
* INPUTS:
*       devObjPtr   - pointer to device object.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*
* COMMENTS:
*
*
*******************************************************************************/
GT_VOID smemIronmanInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**************************************************/
/********** SMU UNIT START ************************/
/**************************************************/
/**
* @enum SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU IRF SNG table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_INDIVIDUAL_RECOVERY_ENABLED_E
    ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E
    ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_TAKE_ANY_E
    ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E
    ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E
    ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E

    ,SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_IRF_SNG_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E].formatNamePtr, \
        _index,/* the streamId 0..(2K-1)*/                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E].fieldsNamePtr, \
        fieldName)

#define SMEM_SIP6_30_SMU_IRF_SNG_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E].formatNamePtr, \
        _index,/* the streamId 0..(2K-1)*/                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E].fieldsNamePtr, \
        fieldName,_value)

/**
* @enum SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU IRF counters table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E
    ,SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E
    ,SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_OUT_OF_ORDER_PACKETS_COUNTER_E
    ,SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_NUMBER_OF_TAGLESS_PACKETS_COUNTER_E
    ,SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E

    ,SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_IRF_COUNTERS_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E)

#define SMEM_SIP6_30_SMU_IRF_COUNTERS_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E)

/* GET field of 48 bits */
#define SMEM_SIP6_30_SMU_IRF_COUNTERS_BYTES_FIELD_GET(_devObjPtr,_memPtr,_index,_valueArr)  \
    snetFieldFromEntry_Any_Get(_devObjPtr,                              \
        _memPtr,                                                        \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E].formatNamePtr, \
        _index,/* the streamId 0..(2K-1)*/                                                  \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E].fieldsNamePtr, \
        SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E,_valueArr)

/* SET field of 48 bits */
#define SMEM_SIP6_30_SMU_IRF_COUNTERS_BYTES_FIELD_SET(_devObjPtr,_memPtr,_index,_valueArr)  \
    snetFieldFromEntry_Any_Set(_devObjPtr,                              \
        _memPtr,                                                        \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E].formatNamePtr, \
        _index,/* the streamId 0..(2K-1)*/                                                  \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E].fieldsNamePtr, \
        SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E,_valueArr)

/**
* @enum SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Interval Max table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_INTERVAL_MAX_E

    ,SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E].fieldsNamePtr, \
        fieldName)

#define SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E].fieldsNamePtr, \
        fieldName,_value)

/**
* @enum SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Octet Counters table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_OCTET_COUNTER_E

    ,SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E].fieldsNamePtr, \
        fieldName)

#define SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E].fieldsNamePtr, \
        fieldName,_value)

/**
* @enum SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Table Set Time Configurations table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TOD_OFFSET_E
    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_INGRESS_TIMESTAMP_OFFSET_E
    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_CYCLE_TIME_E
    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_FACTOR_E
    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_E
    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_BYTE_COUNT_ADJUST_POLARITY_E
    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_REMAINING_BITS_RESOLUTION_E

    ,SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].fieldsNamePtr, \
        fieldName)

#define SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E].fieldsNamePtr, \
        fieldName,_value)

/**
* @enum SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Time Slot Attributes table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_0_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_0_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_0_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_0_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_0_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_1_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_1_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_1_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_1_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_1_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_2_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_2_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_2_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_2_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_2_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_3_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_3_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_3_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_3_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_3_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_4_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_4_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_4_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_4_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_4_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_5_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_5_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_5_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_5_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_5_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_6_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_6_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_6_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_6_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_6_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NEW_SLOT_7_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_STREAM_GATE_STATE_7_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_LENGTH_AWARE_7_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IPV_7_E
    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_IMX_PROFILE_7_E

    ,SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].fieldsNamePtr, \
        fieldName)

#define SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E].fieldsNamePtr, \
        fieldName,_value)

/**
* @enum SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of SMU SGC Time To Advance table.
*/
typedef enum{
     SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_NEW_SLOT_0_E

    ,SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    snetFieldFromEntry_GT_U32_Get(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E].fieldsNamePtr, \
        fieldName)

#define SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    snetFieldFromEntry_GT_U32_Set(_devObjPtr,                                          \
        _memPtr,                                                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E].formatNamePtr, \
        _index,                                             \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E].fieldsNamePtr, \
        fieldName,_value)


/**************************************************/
/********** PREQ UNIT START ************************/
/**************************************************/
/**
* @enum SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream Mapping table.
*/
typedef enum{
     SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_ENABLED_E
    ,SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_STREAM_TYPE_E
    ,SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_SRF_NUMBER_E
    ,SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_0_E
    ,SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_EGRESS_PORTS_BITMAP_1_E

    ,SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_PREQ_SRF_MAPPING_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_PREQ_SRF_MAPPING_E)

#define SMEM_SIP6_30_PREQ_SRF_MAPPING_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_PREQ_SRF_MAPPING_E)


/**
* @enum SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream config table.
*/
typedef enum{
     SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_SEQ_HIST_LENGTH_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_RECOV_SEQ_NUM_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_TAKE_NO_SEQUENCE_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ROGUE_PKTS_CNT_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_FIRST_BUFFER_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_NUMBER_OF_BUFFERS_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BUFFER_E
    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_CUR_BIT_E

    ,SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_PREQ_SRF_CONFIG_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_PREQ_SRF_CONFIG_E)

#define SMEM_SIP6_30_PREQ_SRF_CONFIG_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_PREQ_SRF_CONFIG_E)

/**
* @enum SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream counters table.
*/
typedef enum{
     SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_PASSED_PKTS_CNT_E
    ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_DISCARDED_PKTS_CNT_E
    ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_OUT_OF_ORDER_PKTS_CNT_E
    ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_CLEARED_ZERO_HIST_CNT_E
    ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_COUNTER_E
    ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_RESTART_THRESHOLD_E

    ,SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_PREQ_SRF_COUNTERS_TABLE_FIELDS_ENT;

#define SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_PREQ_SRF_COUNTERS_E)

#define SMEM_SIP6_30_PREQ_SRF_COUNTERS_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_PREQ_SRF_COUNTERS_E)


/**
* @enum SMEM_SIP6_30_PREQ_SRF_CONFIG_DAEMON_FIELDS_ENT
 *
 * @brief enumeration to hold fields of PREQ Stream daemon table.
*/
typedef enum{
     SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_NUMBER_OF_RESETS_E
    ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_ENABLE_E
    ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_BASE_DIFFERENCE_E
    ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_DIFFERENCE_E
    ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_RESET_TIME_E
    ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_LE_TIME_SINCE_LAST_RESET_E

    ,SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS___LAST_VALUE___E /* used for array size */
}SMEM_SIP6_30_PREQ_SRF_DAEMON_TABLE_FIELDS_ENT;

/* macro to get value to field of (PREQ) SRF daemon entry format in buffer */
#define SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)        \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_PREQ_SRF_DAEMON_E)

/* macro to set value to field of (PREQ) SRF daemon entry format in buffer */
#define SMEM_SIP6_30_PREQ_SRF_DAEMON_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value) \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_PREQ_SRF_DAEMON_E)

/*******************************************************************************
*   smemSip6_30FdbHsrPrpTimerGet
*
* DESCRIPTION:
*       Function to get the FDB HSR/PRP running timer.
*
* INPUTS:
*       devObjPtr - (pointer to) the device object
*       useFactor - the timer need to apply factor or not
*           GT_TRUE   - the timer is in 22 bits , without the 'timer factor'.
*                       (as exists in the timer register)
*           GT_FALSE  - the timer is in 32 bits 'micro seconds' , after applying
*                       the 'timer factor'
*        NOTE: the 'factor' is (80/40/20/10 micro-sec , from the <Timer Tick Time>
*               field in register 'HSR PRP Global Configuration')
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       the timer (with/without factor).
*       if using factor --> the value is in micro seconds.
*
* COMMENTS:
*
*******************************************************************************/
GT_U32 smemSip6_30FdbHsrPrpTimerGet
(
    IN struct SKERNEL_DEVICE_OBJECT_T * devObjPtr,
    IN GT_BOOL  useFactor
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemsmemIronmanh */


