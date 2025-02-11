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
* @file mvHwsAasPcs400Units.h
*
* @brief AAS LPCS port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsAasPcs400Units_H
#define __mvHwsAasPcs400Units_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  AAS_PCS400_CONTROL1_SPEED_SELECTION_E,
  /*1*/  AAS_PCS400_CONTROL1_SPEED_SELECTION_4_5_E,
  /*2*/  AAS_PCS400_CONTROL1_SPEED_SELECTION_6_E,
  /*3*/  AAS_PCS400_CONTROL1_LOW_POWER_E,
  /*4*/  AAS_PCS400_CONTROL1_SPEED_SELECTION_13_E,
  /*5*/  AAS_PCS400_CONTROL1_LOOPBACK_E,
  /*6*/  AAS_PCS400_CONTROL1_RESET_E,
  /*7*/  AAS_PCS400_STATUS1_PCS_RECEIVE_LINK_E,
  /*8*/  AAS_PCS400_STATUS1_FAULT_E,
  /*9*/  AAS_PCS400_DEVICE_ID0_IDENTIFIER0_E,
  /*10*/  AAS_PCS400_DEVICE_ID1_IDENTIFIER1_E,
  /*11*/  AAS_PCS400_SPEED_ABILITY_SPEEDABILITY_8_E,
  /*12*/  AAS_PCS400_SPEED_ABILITY_SPEEDABILITY_9_E,
  /*13*/  AAS_PCS400_DEVICES_IN_PKG1_DEVICES_PCSPRESENT_E,
  /*14*/  AAS_PCS400_DEVICES_IN_PKG2_DEVICES_PKG2_E,
  /*15*/  AAS_PCS400_CONTROL2_PCS_TYPE_E,
  /*16*/  AAS_PCS400_STATUS2_RECEIVE_FAULT_E,
  /*17*/  AAS_PCS400_STATUS2_TRANSMIT_FAULT_E,
  /*18*/  AAS_PCS400_STATUS2_DEVICE_PRESENT_E,
  /*19*/  AAS_PCS400_STATUS3_STATUS3_200_E,
  /*20*/  AAS_PCS400_STATUS3_STATUS3_400_E,
  /*21*/  AAS_PCS400_PKG_ID0_PACKAGE_IDENTIFIER0_E,
  /*22*/  AAS_PCS400_PKG_ID1_PACKAGE_IDENTIFIER1_E,
  /*23*/  AAS_PCS400_BASER_STATUS1_RECEIVE_LINK_E,
  /*24*/  AAS_PCS400_BASER_STATUS2_ERRORED_CNT_E,
  /*25*/  AAS_PCS400_BASER_TEST_CONTROL_SELECT_RANDOM_E,
  /*26*/  AAS_PCS400_BASER_TEST_ERR_CNT_COUNTER_E,
  /*27*/  AAS_PCS400_BER_HIGH_ORDER_CNT_BER_COUNTER_E,
  /*28*/  AAS_PCS400_ERR_BLK_HIGH_ORDER_CNT_ERRORED_BLOCKS_COUNTER_E,
  /*29*/  AAS_PCS400_ERR_BLK_HIGH_ORDER_CNT_HIGH_ORDER_PRESENT_E,
  /*30*/  AAS_PCS400_MULTILANE_ALIGN_STAT1_LANE_ALIGN_STATUS_E,
  /*31*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE0_MARKER_LOCK_E,
  /*32*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE1_MARKER_LOCK_E,
  /*33*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE2_MARKER_LOCK_E,
  /*34*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE3_MARKER_LOCK_E,
  /*35*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE4_MARKER_LOCK_E,
  /*36*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE5_MARKER_LOCK_E,
  /*37*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE6_MARKER_LOCK_E,
  /*38*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE7_MARKER_LOCK_E,
  /*39*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE8_MARKER_LOCK_E,
  /*40*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE9_MARKER_LOCK_E,
  /*41*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE10_MARKER_LOCK_E,
  /*42*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE11_MARKER_LOCK_E,
  /*43*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE12_MARKER_LOCK_E,
  /*44*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE13_MARKER_LOCK_E,
  /*45*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE14_MARKER_LOCK_E,
  /*46*/  AAS_PCS400_MULTILANE_ALIGN_STAT3_LANE15_MARKER_LOCK_E,
  /*47*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE16_MARKER_LOCK_E,
  /*48*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE17_MARKER_LOCK_E,
  /*49*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE18_MARKER_LOCK_E,
  /*50*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE19_MARKER_LOCK_E,
  /*51*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE20_MARKER_LOCK_E,
  /*52*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE21_MARKER_LOCK_E,
  /*53*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE22_MARKER_LOCK_E,
  /*54*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE23_MARKER_LOCK_E,
  /*55*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE24_MARKER_LOCK_E,
  /*56*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE25_MARKER_LOCK_E,
  /*57*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE26_MARKER_LOCK_E,
  /*58*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE27_MARKER_LOCK_E,
  /*59*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE28_MARKER_LOCK_E,
  /*60*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE29_MARKER_LOCK_E,
  /*61*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE30_MARKER_LOCK_E,
  /*62*/  AAS_PCS400_MULTILANE_ALIGN_STAT4_LANE31_MARKER_LOCK_E,
  /*63*/  AAS_PCS400_LANE_MAPPING_LANE0_MAPPING_E,
  /*64*/  AAS_PCS400_VENDOR_SCRATCH_SCRATCH_E,
  /*65*/  AAS_PCS400_VENDOR_CORE_REV_REVISION_E,
  /*66*/  AAS_PCS400_VENDOR_VL_INTVL_VL_INTVL_E,
  /*67*/  AAS_PCS400_VENDOR_TX_LANE_THRESH_TX_LANETHRESH_E,
  /*68*/  AAS_PCS400_VENDOR_TX_CDMII_PACE_TX_CDMII_PACE_E,
  /*69*/  AAS_PCS400_VENDOR_AM_0_CM0_E,
  /*70*/  AAS_PCS400_VENDOR_AM_0_CM1_E,
  /*71*/  AAS_PCS400_VENDOR_AM_1_CM2_E,
  /*72*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_0_E,
  /*73*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_1_E,
  /*74*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_2_E,
  /*75*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_3_E,
  /*76*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_4_E,
  /*77*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_5_E,
  /*78*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_6_E,
  /*79*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_8_E,
  /*80*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_9_E,
  /*81*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_10_E,
  /*82*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_11_E,
  /*83*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_12_E,
  /*84*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_13_E,
  /*85*/  AAS_PCS400_VENDOR_DBGINFO0_DBGINFO0_14_E,
  /*86*/  AAS_PCS400_VENDOR_DBGINFO1_DBGINFO1_0_E,
  /*87*/  AAS_PCS400_VENDOR_DBGINFO1_DBGINFO1_4_E,
  /*88*/  AAS_PCS400_VENDOR_DBGINFO1_DBGINFO1_5_E,
  /*89*/  AAS_PCS400_VENDOR_DBGINFO1_DBGINFO1_6_E,
  /*90*/  AAS_PCS400_VENDOR_DBGINFO2_DBGINFO2_E,
  /*91*/  AAS_PCS400_VENDOR_DBGINFO3_DBGINFO3_0_E,
  /*92*/  AAS_PCS400_VENDOR_DBGINFO3_DBGINFO3_1_E,
  /*93*/  AAS_PCS400_VENDOR_DBGINFO3_DBGINFO3_2_E,
  /*94*/  AAS_PCS400_VENDOR_DBGINFO3_DBGINFO3_4_E,
  /*95*/  AAS_PCS400_VENDOR_DBGINFO3_DBGINFO3_8_E,
          AAS_PCS400_REGISTER_LAST_E /* should be last */
} MV_HWS_AAS_PCS400_UNIT_FIELDS_ENT;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAasPcs400Units_H */

