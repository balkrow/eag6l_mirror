/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file mvHwsRsfecStatisticsUnits.h
*
* @brief Hawk port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsRsfecStatisticsUnits_H
#define __mvHwsRsfecStatisticsUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  RSFEC_STATISTICS_UNITS_RSFEC_FAST_DATA_HI_LATCHED_COUNTER_HI_FAST,
  /*1*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_STATUS_STAT_MODUlE_BUSY,
  /*2*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONFIG_SATURATE,
  /*3*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONFIG_CLEAR_ON_READ,
  /*4*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONFIG_SOFT_RESET,
  /*5*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL,
  /*6*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL_CHANMASK,
  /*7*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL_CMD_CAPTURE,
  /*8*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL_CLEAR_PRE,
  /*9*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CONTROL_CMD_CLEAR,
  /*10*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CLEARVALUE_LO_STATM_LOWER_32B,
  /*11*/  RSFEC_STATISTICS_UNITS_RSFEC_STATN_CLEARVALUE_HI_STATM_HIGH_32B,
  /*12*/  RSFEC_STATISTICS_UNITS_RSFEC_SLOW_DATA_HI_LATCHED_COUNTER_HI_SLOW,
  /*13*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_TOTAL_CW_RECEIVED,
  /*14*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CORRECT,
  /*15*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CORRECTED,
  /*16*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_UNCORRECTABLE,
  /*17*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_1SE,
  /*18*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_2SE,
  /*19*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_3SE,
  /*20*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_4SE,
  /*21*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_5SE,
  /*22*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_6SE,
  /*23*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_7SE,
  /*24*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_8SE,
  /*25*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_9SE,
  /*26*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_10SE,
  /*27*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_11SE,
  /*28*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_12SE,
  /*29*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_13SE,
  /*30*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_14SE,
  /*31*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_15SE,
  /*32*/  RSFEC_STATISTICS_UNITS_RSFEC_COUNTER_CAPTURE_COUNTER_LOW_COUNTER_CAPTURE_CW_CORRECTED_TOTAL_SYMBOL_ERROR_CORRECTED,
  /*33*/  RSFEC_STATISTICS_UNITS_RSFEC_TOTAL_CODEWORDS_RECIVED_TOTAL_CW_RECEIVED,
  /*34*/  RSFEC_STATISTICS_UNITS_RSFEC_TOTAL_CODEWORDS_CORRECT_TOTAL_CW_CORRECT,
  /*35*/  RSFEC_STATISTICS_UNITS_RSFEC_TOTATL_CODEWORDS_CORRECTED_TOTAL_CW_CORRECTED,
  /*36*/  RSFEC_STATISTICS_UNITS_RSFEC_TOTATL_CODEWORDS_UNCORRECTABLE_TOTATL_CW_UNCORRECTABLE,
  /*37*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_1SE_CW_CORRECTED_1SE,
  /*38*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_2SE_CW_CORRECTED_2SE,
  /*39*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_3SE_CW_CORRECTED_3SE,
  /*40*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_4SE_CW_CORRECTED_4SE,
  /*41*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_5SE_CW_CORRECTED_5SE,
  /*42*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_6SE_CW_CORRECTED_6SE,
  /*43*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_7SE_CW_CORRECTED_7SE,
  /*44*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_8SE_CW_CORRECTED_8SE,
  /*45*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_9SE_CW_CORRECTED_9SE,
  /*46*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_10SE_CW_CORRECTED_10SE,
  /*47*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_11SE_CW_CORRECTED_11SE,
  /*48*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_12SE_CW_CORRECTED_12SE,
  /*49*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_13SE_CW_CORRECTED_13SE,
  /*50*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_14SE_CW_CORRECTED_14SE,
  /*51*/  RSFEC_STATISTICS_UNITS_RSFEC_CODEWORDS_CORRECTED_15SE_CW_CORRECTED_15SE,
  /*52*/  RSFEC_STATISTICS_UNITS_RSFEC_TOTAL_SYMBOLS_ERROR_CORRECTED,

    RSFEC_STATISTICS_REGISTER_LAST_E /* should be last */
} MV_HWS_RSFEC_STATISTICS_UNITS_FIELDS_E;



#ifdef __cplusplus
}
#endif

#endif /* __mvHwsRsfecStatisticsUnits_H */

