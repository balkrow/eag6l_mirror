/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvCpssGenSrvCpuLog.h
*       WARNING!!! this is a generated file, please don't edit it manually
* COMMENTS:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*******************************************************************************/
#ifndef __prvCpssGenSrvCpuLogh
#define __prvCpssGenSrvCpuLogh
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <cpss/generic/log/cpssLog.h>
#include <cpss/generic/log/prvCpssLog.h>


/********* API fields DB *********/

extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_bufBmp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_bufNum;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_bufSize;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U32_fw_id;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_PTR_bufPtr;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_resetOp;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_scpuId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_GT_U8_srvCpuId;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC IN_const_char;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC OUT_GT_U32_PTR_status;
extern const PRV_CPSS_LOG_FUNC_PARAM_STC _IN_GT_CHAR;


/********* lib API DB *********/

enum {
    PRV_CPSS_LOG_FUNC_cpssSrvCpuSetOptionalFW_E = (CPSS_LOG_LIB_SRV_CPU_E << 16),
    PRV_CPSS_LOG_FUNC_cpssSrvCpuSetUsrPath_E,
    PRV_CPSS_LOG_FUNC_cpssGenericSrvCpuResetUnreset_E,
    PRV_CPSS_LOG_FUNC_cpssGenericSrvCpuFWUpgrade_E
};

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __prvCpssGenSrvCpuLogh */
