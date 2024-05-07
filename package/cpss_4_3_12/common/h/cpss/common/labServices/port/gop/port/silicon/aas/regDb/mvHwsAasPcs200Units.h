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
* @file mvHwsAasPcs200Units.h
*
* @brief AAS PCS200 port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsAasPcs200Units_H
#define __mvHwsAasPcs200Units_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  AAS_PCS200_CONTROL1_SPEED_SELECTION_E,
  /*1*/  AAS_PCS200_CONTROL1_SPEED_SELECTION_6_E,
  /*2*/  AAS_PCS200_CONTROL1_LOW_POWER_E,
  /*3*/  AAS_PCS200_CONTROL1_SPEED_SELECTION_13_E,
  /*4*/  AAS_PCS200_CONTROL1_LOOPBACK_E,
  /*5*/  AAS_PCS200_CONTROL1_RESET_E,
  /*6*/  AAS_PCS200_STATUS1_PCS_RECEIVE_LINK_E,
  /*7*/  AAS_PCS200_STATUS1_FAULT_E,
  /*8*/  AAS_PCS200_DEVICE_ID0_IDENTIFIER0_E,
  /*9*/  AAS_PCS200_DEVICE_ID1_IDENTIFIER1_E,
  /*10*/  AAS_PCS200_SPEED_ABILITY_SPEEDABILITY_8_E,
  /*11*/  AAS_PCS200_DEVICES_IN_PKG1_DEVICES_PCSPRESENT_E,
  /*12*/  AAS_PCS200_DEVICES_IN_PKG2_DEVICES_PKG2_E,
  /*13*/  AAS_PCS200_CONTROL2_PCS_TYPE_E,
  /*14*/  AAS_PCS200_STATUS2_RECEIVE_FAULT_E,
  /*15*/  AAS_PCS200_STATUS2_TRANSMIT_FAULT_E,
  /*16*/  AAS_PCS200_STATUS2_DEVICE_PRESENT_E,
  /*17*/  AAS_PCS200_STATUS3_STATUS3_200_E,
  /*18*/  AAS_PCS200_PKG_ID0_PACKAGE_IDENTIFIER0_E,
  /*19*/  AAS_PCS200_PKG_ID1_PACKAGE_IDENTIFIER1_E,
  /*20*/  AAS_PCS200_BASER_STATUS1_RECEIVE_LINK_E,
  /*21*/  AAS_PCS200_BASER_STATUS2_ERRORED_CNT_E,
  /*22*/  AAS_PCS200_BASER_TEST_CONTROL_SELECT_RANDOM_E,
  /*23*/  AAS_PCS200_BASER_TEST_ERR_CNT_COUNTER_E,
  /*24*/  AAS_PCS200_BER_HIGH_ORDER_CNT_BER_COUNTER_E,
  /*25*/  AAS_PCS200_ERR_BLK_HIGH_ORDER_CNT_ERRORED_BLOCKS_COUNTER_E,
  /*26*/  AAS_PCS200_ERR_BLK_HIGH_ORDER_CNT_HIGH_ORDER_PRESENT_E,
  /*27*/  AAS_PCS200_MULTILANE_ALIGN_STAT1_LANE_ALIGN_STATUS_E,
  /*28*/  AAS_PCS200_LANE0_MAPPING_LANE0_MAPPING_E,
  /*29*/  AAS_PCS200_LANE1_MAPPING_LANE1_MAPPING_E,
  /*30*/  AAS_PCS200_LANE2_MAPPING_LANE2_MAPPING_E,
  /*31*/  AAS_PCS200_LANE3_MAPPING_LANE3_MAPPING_E,
  /*32*/  AAS_PCS200_LANE4_MAPPING_LANE4_MAPPING_E,
  /*33*/  AAS_PCS200_LANE5_MAPPING_LANE5_MAPPING_E,
  /*34*/  AAS_PCS200_LANE6_MAPPING_LANE6_MAPPING_E,
  /*35*/  AAS_PCS200_LANE7_MAPPING_LANE7_MAPPING_E,
          AAS_PCS200_REGISTER_LAST_E /* should be last */
} MV_HWS_AAS_PCS200_UNIT_FIELDS_ENT;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAasPcs200Units_H */

