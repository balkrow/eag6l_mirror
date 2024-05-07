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
* @file prvCpssDxChTamIfa.h
*
* @brief CPSS definitions for configuring, gathering info and statistics
*        for the TAM IFA (Telemetry Analytics and Monitoring Inband Flow Analyzer)
*        feature
*
* @version   1
********************************************************************************
*/
#ifndef __prvCpssDxChTamIfah
#define __prvCpssDxChTamIfah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/tam/cpssDxChTamIfa.h>

/** Maximum number of IFA Global MACs in the system */
#define PRV_CPSS_DXCH_MAX_IFA_GLOBAL_MAC_INDEX_CNS 256

/** Maximum number of CNC Blocks in the system */
#define PRV_CPSS_DXCH_MAX_IFA_CNC_BLOCK_INDEX_CNS  32

/** Maximum number of CNC indexes per blocks in the system */
#define PRV_CPSS_DXCH_MAX_IFA_CNC_COUNTER_INDEX_CNS  _1K

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChTamIfah */

