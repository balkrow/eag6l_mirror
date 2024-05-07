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
* @file smemAas.h
*
* @brief Aas memory mapping implementation.
*
* @version   1
********************************************************************************
*/
#ifndef __smemAash
#define __smemAash

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemIronman.h>

ACTIVE_WRITE_FUNC_PROTOTYPE_MAC(smemAasActiveWriteCnmGdmaAddGdmaSgdOwn);

#define LPM_MAX_NUM_OF_BLOCKS                        48
#define LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_CNS (256*_1K)
#define LPM_RAM_CHILD_POINTER_NUMBER_OF_BITS_CNS     24

#define  SIP7_MLL_ACTIVE_MEM_MAC                                                    \
/*MLL*/                                                                        \
    /* MLL counters */                                                         \
    /*L2MLLValidProcessedEntriesCntr[0]*/                                      \
    {0x00460e20       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLValidProcessedEntriesCntr[1]*/                                      \
    {0x00460e24       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLValidProcessedEntriesCntr[2]*/                                      \
    {0x00460e28       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLSilentDropCntr*/                                                    \
    {0x00460e00  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLSkippedEntriesCntr*/                                                \
    {0x00460e04  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLTTLExceptionCntr*/                                                  \
    {0x00460e08  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLOutMcPktsCntr[0]*/                                                  \
    {0x00460c00       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*L2MLLOutMcPktsCntr[1]*/                                                  \
    {0x00460d00       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*MLLSilentDropCntr*/                                                      \
    {0x00460800  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*MLLMCFIFODropCntr*/                                                      \
    {0x00460804  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*IPMLLSkippedEntriesCntr*/                                                \
    {0x00460b04  , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*IPMLLOutMcPktsCntr[0]*/                                                  \
    {0x00460900       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*IPMLLOutMcPktsCntr[1]*/                                                  \
    {0x00460a00       , SMEM_FULL_MASK_CNS, smemChtActiveReadCntrs, 0, NULL,0},     \
    /*MLL Interrupt Cause Registers*/                                          \
    {0x00460030, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, 0, smemChtActiveWriteIntrCauseReg, 0}, \
    {0x00460034, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0}

/**
* @enum SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENT
 *
 * @brief enumeration to hold fields of EXACT_MATCH table in SIP7 devices.
*/
typedef enum {
     SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_VALID
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE

    /*EM ACTION*/
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_83_64

    /*EM KEY*/
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64
    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_127_96

    ,SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E

}SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENT;

/* macro to shorten the calling code of (EM) 'Exact match' table fields - for GET field */
#define SMEM_SIP7_EXACT_MATCH_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)       \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName,SKERNEL_TABLE_FORMAT_EXACT_MATCH_ENTRY_E)

/* macro to shorten the calling code of (EM) 'Exact match' table fields - for SET field */
#define SMEM_SIP7_EXACT_MATCH_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value)       \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value,SKERNEL_TABLE_FORMAT_EXACT_MATCH_ENTRY_E)

#define SMEM_SIP7_HA_TUNNEL_START_MPLS_LABEL_STACK_GET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                                                     \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].formatNamePtr,                      \
        _index,/* the index */                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_TUNNEL_START_E].fieldsNamePtr, \
        SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_MPLS_LABEL_STACK_E,_valueArr)

#define SMEM_SIP7_HA_HFT_MPLS_LABEL_STACK_GET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                                                     \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_HEADER_FIELDS_E].formatNamePtr,                      \
        _index,/* the index */                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_HEADER_FIELDS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_HEADER_FIELDS_E].fieldsNamePtr, \
        SMEM_SIP7_HA_HEADER_FIELDS_MPLS_LABELS_E,_valueArr)

#define SMEM_SIP7_HA_HFT_GCF_BITS_GET(_devObjPtr,_memPtr,_index,_valueArr)       \
    snetFieldFromEntry_Any_Get(_devObjPtr,                           \
        _memPtr,                                                     \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_HEADER_FIELDS_E].formatNamePtr,                      \
        _index,/* the index */                                       \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_HEADER_FIELDS_E].fieldsInfoPtr, \
        _devObjPtr->tableFormatInfo[SKERNEL_TABLE_FORMAT_HA_HEADER_FIELDS_E].fieldsNamePtr, \
        SMEM_SIP7_HA_HEADER_FIELDS_GCF_BITS_5_47_E,_valueArr)

extern GT_BOOL smemAasActiveCncBlockReadIsMatch_64bits
(
    IN        SKERNEL_DEVICE_OBJECT * deviceObjPtr,
    IN        GT_U32 address,
    IN        GT_UINTPTR param
);

#define ACTIVE_MEM_SIP7_CNC_COMMON_MAC                                                                      \
    /* CPU direct read from the 16 blocks counters */                                                       \
    {0x00220000,SMEM_FULL_MASK_CNS, smemCht3ActiveCncBlockRead, 0x00220000/*base address of block 0*/, NULL, 0 , \
                    smemAasActiveCncBlockReadIsMatch_64bits}, \
    /* CNC Fast Dump Trigger Register Register */                                                           \
    {0x00000030,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncFastDumpTrigger, 0},                    \
    /* CNC Block Wraparound Status Register */                                                              \
    {0x00001400,0xFFFFFE00, smemCht3ActiveCncWrapAroundStatusRead, 0, NULL, 0},                             \
    /* read interrupts cause registers CNC -- ROC register */                                               \
    {0x00000100,SMEM_FULL_MASK_CNS,smemChtActiveReadIntrCauseReg, 3, smemChtActiveWriteIntrCauseReg, 0},    \
    /* Write Interrupt Mask CNC Register */                                                                 \
    {0x00000104,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncInterruptsMaskReg, 0},                  \
    /* Wraparound Functional Interrupt Cause Register */                                                    \
    {0x00000190,SMEM_FULL_MASK_CNS,smemChtActiveReadIntrCauseReg, 3, smemChtActiveWriteIntrCauseReg, 0},    \
    /* Wraparound Functional Interrupt Mask Register */                                                     \
    {0x000001A4,SMEM_FULL_MASK_CNS, NULL, 0 , smemCht3ActiveWriteCncInterruptsMaskReg, 0},


/**
* @internal smemAasInit function
* @endinternal
*
* @brief   Init memory module for a AAS device.
*/
void smemAasInit
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/**
* @internal smemAasInit2 function
* @endinternal
*
* @brief   Init memory module for a device - after the load of the default
*         registers file
* @param[in] devObjPtr                - pointer to device object.
*/
void smemAasInit2
(
    IN SKERNEL_DEVICE_OBJECT * deviceObj
);

/*******************************************************************************
*   smemAasInterruptTreeInit
*
* DESCRIPTION:
*       Init the interrupts tree for the AAS device
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
GT_VOID smemAasInterruptTreeInit
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr
);

/**
* @internal smemAasActiveReadLpm function
* @endinternal
*
* @brief   redirect the read of LPM memory to do the read of another table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter
*
* @param[out] outMemPtr                - Pointer to the memory to copy register's content.
*/
 void smemAasActiveReadLpm (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    OUT        GT_U32 * outMemPtr
);

/**
* @internal smemAasActiveWriteLpm function
* @endinternal
*
* @brief   redirect the write of LPM memory to do the wrtite from another table
*
* @param[in] devObjPtr                - device object PTR.
* @param[in] address                  - Address for ASIC memory.
* @param[in] memSize                  - size of the requested memory
* @param[in] memPtr                   - Pointer to the register's memory in the simulation.
* @param[in] param                    - Registers' specific parameter.
* @param[in] inMemPtr                 - Pointer to the memory to get register's content.
*/
void smemAasActiveWriteLpm (
    IN         SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN         GT_U32   address,
    IN         GT_U32   memSize,
    IN         GT_U32 * memPtr,
    IN         GT_UINTPTR   param,
    IN         GT_U32 * inMemPtr
);

#define AAS_ACTIVE_MEM_POLICER_COMMON_MAC(policerId)   \
    /* Policers Table Access Control Register */                                                            \
    {0x00000100, SMEM_FULL_MASK_CNS,  NULL,policerId, smemXCatActiveWritePolicerTbl, policerId},            \
                                                                                                            \
    /* Policer IPFIX  */                                                                                    \
    {0x00000074, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSampleLog, policerId, NULL,policerId},          \
    {0x00000078, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSampleLog, policerId, NULL,policerId},          \
    {0x0000007c, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSampleLog, policerId, NULL,policerId},          \
    {0x00000080, SMEM_FULL_MASK_CNS, smemXCatActiveReadIPFixSampleLog, policerId, NULL,policerId},          \
                                                                                                            \
    /* PLR interrupt cause register */                                                                      \
    {0x00000200, SMEM_FULL_MASK_CNS, smemChtActiveReadIntrCauseReg, policerId, smemChtActiveWriteIntrCauseReg,policerId}, \
    {0x00000204, SMEM_FULL_MASK_CNS, NULL, 0, smemChtActiveGenericWriteInterruptsMaskReg, 0},               \
    /*SMEM_LION3_POLICER_METERING_BASE_ADDR_REG*/                                                           \
    /*Policer Metering Base Address*/                                                                       \
    {0x0000001C, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerMeteringBaseAddr,  policerId},     \
    /*SMEM_LION3_POLICER_COUNTING_BASE_ADDR_REG*/                                                           \
    /*Policer Counting Base Address*/                                                                       \
    {0x00000018, SMEM_FULL_MASK_CNS, NULL, 0 , smemBobkActiveWritePolicerCountingBaseAddr,  policerId}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __smemsmemAash */


