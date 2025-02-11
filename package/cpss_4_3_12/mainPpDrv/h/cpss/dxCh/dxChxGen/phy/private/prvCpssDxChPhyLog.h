/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssDxChPhyLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssDxChPhyLogh
#define __prvCpssDxChPhyLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* enums *********/

PRV_CPSS_LOG_STC_ENUM_MAP_DECLARE_MAC(CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT);


/********* parameters log functions *********/

void prvCpssLogParamFunc_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);
void prvCpssLogParamFunc_GT_STATUS_PTR(
    IN CPSS_LOG_LIB_ENT            contextLib,
    IN CPSS_LOG_TYPE_ENT           logType,
    IN GT_CHAR_PTR                 namePtr,
    IN va_list                   * argsPtr,
    IN GT_BOOL                     skipLog,
    IN PRV_CPSS_LOG_PARAM_ENTRY_INFO_STC   * inOutParamInfoPtr
);


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi0;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi1;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi2;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_autoPollNumOfPortsSmi3;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_ERRATA_WA_ENT_errataWa;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_SMI_INTERFACE_ENT_smiInterface;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_divisionFactor;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_smiInterface;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_PHY_XSMI_INTERFACE_ENT_xsmiInterface;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_VCT_ACTION_ENT_vctAction;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_VCT_CONFIG_STC_PTR_configPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_CPSS_VCT_GEN_BIND_FUNC_STC_PTR_genVctBindFuncPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_32_offset;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_enableXsmi;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_invertMDC;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_BOOL_useExternalPhy;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_HEX_data;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U16_phyReg;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_autoMediaSelect;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U32_smiInterfacePortGroup;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyAddr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyDev;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_IN_GT_U8_phyReg;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi0Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi1Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi2Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_DXCH_PHY_SMI_AUTO_POLL_NUM_OF_PORTS_ENT_PTR_autoPollNumOfPortsSmi3Ptr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PHY_SMI_INTERFACE_ENT_PTR_smiInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PHY_SMI_MDC_DIVISION_FACTOR_ENT_PTR_divisionFactorPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_PHY_XSMI_INTERFACE_ENT_PTR_smiInterfacePtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_VCT_CABLE_EXTENDED_STATUS_STC_PTR_extendedCableStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_CPSS_VCT_CABLE_STATUS_STC_PTR_cableStatusPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_32_PTR_offsetPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_enableXsmiPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_BOOL_PTR_invertMDCPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U16_HEX_PTR_dataPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_autoMediaSelectPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U32_PTR_smiInterfacePortGroupPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_phyAddFromDbPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC DX_OUT_GT_U8_PTR_phyAddFromHwPtr;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiInit_E = (CPSS_LOG_LIB_PHY_E << 16) | (1 << 24),
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiRegisterRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiRegisterWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPort10GSmiRegisterRead_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPort10GSmiRegisterWrite_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyAutonegSmiGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyAutonegSmiSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhySmiAutoMediaSelectSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhySmiAutoMediaSelectGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyXsmiMdcDivisionFactorSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyXsmiMdcDivisionFactorGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhySmiMdcDivisionFactorSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhySmiMdcDivisionFactorGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyBobcat2SmiXsmiMuxSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyBobcat2SmiXsmiMuxGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyXSmiMDCInvertSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyXSmiMDCInvertGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortAddrSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortAddrGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyAutoPollNumOfPortsSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyAutoPollNumOfPortsGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiInterfacePortGroupSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortSmiInterfacePortGroupGet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortXSmiInterfaceSet_E,
    PRV_CPSS_LOG_FUNC_cpssDxChPhyPortXSmiInterfaceGet_E,
    PRV_CPSS_LOG_FUNC_cpssPhyErrataWaExecute_E,
    PRV_CPSS_LOG_FUNC_cpssVctFuncObjectBind_E,
    PRV_CPSS_LOG_FUNC_cpssVctCableExtendedStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssVctCableStatusGet_E,
    PRV_CPSS_LOG_FUNC_cpssVctSetCableDiagConfig_E,
    PRV_CPSS_LOG_FUNC_cpssVctLengthOffsetSet_E,
    PRV_CPSS_LOG_FUNC_cpssVctLengthOffsetGet_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssDxChPhyLogh */
