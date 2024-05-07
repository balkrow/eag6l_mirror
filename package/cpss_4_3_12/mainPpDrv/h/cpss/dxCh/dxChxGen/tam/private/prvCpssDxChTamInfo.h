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
* @file prvCpssDxChTamInfo.h
*
* @brief CPSS definitions for holding the global variables
*        for the TAM (Telemetry Analytics and Monitoring) feature
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChTamInfoh
#define __prvCpssDxChTamInfoh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/tam/cpssDxChTam.h>

/**
 * @struct PRV_CPSS_DXCH_TAM_INFO_STC
 *
 * @brief structure for holding TAM Global Variables
 */
typedef struct PRV_CPSS_DXCH_TAM_INFO_STC {

    /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_SNAPSHOT_DB_STC */
    GT_U32 tamSnapshotDbStartAddr;

    /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_COUNTER_DB_STC */
    GT_U32 tamCountersDbStartAddr;

    /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_MICROBURST_DB_STC */
    GT_U32 tamMicroburstDbStartAddr;

    /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_HISTOGRAM_DB_STC */
    GT_U32 tamHistogramDbStartAddr;

    /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_THRESHOLD_DB_STC */
    GT_U32 tamThresholdDbStartAddr;

    /* Starting address (in the fw sram) of Threshold Notification DB */
    GT_U32 tamThresholdNotifStartAddr;

    /* Starting address (in the fw sram) of Threshold Notification Bitmap */
    GT_U32 tamThresholdNotifBmpStartAddr;

    /* address (in the fw sram) of the Debug Counters */
    GT_U32 tamFwDebugCounterAddr;

    /* address (in the fw sram) of the Debug Statistics Counters */
    GT_U32 tamFwDebugStatCounterAddr;

    /* address (in the fw sram) of the location of the PRV_CPSS_DXCH_TAM_PID_STC */
    GT_U32 tamFwPidAddr;

    /* address (in the fw sram) of the location of the TAM enable flag */
    GT_BOOL tamFwEnable;

    /* starting address (in the fw sram) of the free counter */
    GT_U32 freeCounterFirstAddr;

    /* number of free counters */
    GT_U32 freeCountersNum;

    /* Parameters for TAM Feature */
    CPSS_DXCH_TAM_PARAMS_STC    tamParams;

    CPSS_DXCH_TAM_TRESHOLD_BREACH_EVENT_FUNC eventNotifyFuncPtr;

} PRV_CPSS_DXCH_TAM_INFO_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __prvCpssDxChTamInfoh */

