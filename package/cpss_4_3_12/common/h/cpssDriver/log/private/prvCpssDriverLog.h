/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDriverLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDriverLogh
#define __prvCpssDriverLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DRV_HW_RESOURCE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_DRV_HW_TRACE_TYPE_ENT);
PRV_CPSS_LOG_STC_ENUM_DECLARE_MAC(CPSS_MEMORY_DUMP_TYPE_ENT);


/********* structure fields log functions *********/

void prvCpssLogParamFuncStc_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR
(
    IN CPSS_LOG_LIB_ENT         contextLib,
    IN CPSS_LOG_TYPE_ENT        logType,
    IN GT_CHAR_PTR              namePtr,
    IN void                   * fieldPtr,
    INOUT PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC     * inOutParamInfoPtr
);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DRV_HW_RESOURCE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DRV_HW_TRACE_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_MEMORY_DUMP_TYPE_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_DRV_HW_ACCESS_OBJ_STC_PTR_hwAccessObjPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_DRV_HW_RESOURCE_TYPE_ENT_resourceType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_DRV_HW_TRACE_TYPE_ENT_traceType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_CPSS_MEMORY_DUMP_TYPE_ENT_dumpType;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_BOOL_bind;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_PTR_addrArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_PTR_data;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_PTR_dataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_addr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_fieldData;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_mask;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_regAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_startAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_HEX_value;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_arrLen;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_dumpLength;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_fieldLength;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_fieldOffset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_length;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_mgNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_portGroupId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_CPSS_INTERRUPT_SCAN_STC_PTR_PTR_treeRootPtrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_PTR_data;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_PTR_dataArr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_HEX_PTR_fieldData;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_PTR_notAccessibleBeforeStartInitPtrPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_numOfElementsPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_numOfInterruptRegistersNotAccessibleBeforeStartInitPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRegisterRead_E = (CPSS_LOG_LIB_APP_DRIVER_CALL_E << 16),
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRegisterWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRegFieldGet_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRegFieldSet_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRegBitMaskRead_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRegBitMaskWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRamRead_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRamWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwVectorRead_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwVectorWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwRamInReverseWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDrvHwPpResetAndInitControllerReadReg_E,
    PRV_CPSS_LOG_FUNC_cpssDrvHwPpResetAndInitControllerWriteReg_E,
    PRV_CPSS_LOG_FUNC_cpssDrvHwPpResetAndInitControllerGetRegField_E,
    PRV_CPSS_LOG_FUNC_cpssDrvHwPpResetAndInitControllerSetRegField_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwTraceEnable_E,
    PRV_CPSS_LOG_FUNC_cpssDrvHwAccessObjectBind_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwResourceReadRegister_E,
    PRV_CPSS_LOG_FUNC_cpssDrvPpHwResourceWriteRegister_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpReadRegister_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpWriteRegister_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpGetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpSetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpReadRegBitMask_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpWriteRegBitMask_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpReadRam_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpWriteRam_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpReadVec_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpWriteVec_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpWriteRamInReverse_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpReadInternalPciReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpWriteInternalPciReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpResetAndInitControllerReadReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpResetAndInitControllerGetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpResetAndInitControllerWriteReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpResetAndInitControllerSetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupReadRegister_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupWriteRegister_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupGetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupSetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupReadRegBitMask_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupWriteRegBitMask_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupReadRam_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupWriteRamWithoutSkip_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupWriteRamInReverse_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupReadVec_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupWriteVec_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupReadInternalPciReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpPortGroupWriteInternalPciReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpMgReadReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpMgWriteReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpMgGetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpMgSetRegField_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpBar0ReadReg_E,
    PRV_CPSS_LOG_FUNC_prvCpssDrvHwPpBar0WriteReg_E,
    PRV_CPSS_LOG_FUNC_cpssDrvInterruptsTreeGet_E,
    PRV_CPSS_LOG_FUNC_cpssPpDumpRegisters_E,
    PRV_CPSS_LOG_FUNC_cpssPpDumpMemory_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDriverLogh */
