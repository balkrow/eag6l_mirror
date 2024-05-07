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
* @file mvHwsAasLpcsUnits.h
*
* @brief AAS LPCS port interface header file
*
* @version   1
********************************************************************************
*/
#ifndef __mvHwsAasLpcsUnits_H
#define __mvHwsAasLpcsUnits_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /*0*/  AAS_LPCS_CONTROL_P0_SPEED_6_E,
  /*1*/  AAS_LPCS_CONTROL_P0_DUPLEX_E,
  /*2*/  AAS_LPCS_CONTROL_P0_ANRESTART_E,
  /*3*/  AAS_LPCS_CONTROL_IP0_SOLATE_E,
  /*4*/  AAS_LPCS_CONTROL_P0_POWERDOWN_E,
  /*5*/  AAS_LPCS_CONTROL_P0_ANENABLE_E,
  /*6*/  AAS_LPCS_CONTROL_P0_SPEED_13_E,
  /*7*/  AAS_LPCS_CONTROL_P0_LOOPBACK_E,
  /*8*/  AAS_LPCS_CONTROL_P0_RESET_E,
  /*9*/  AAS_LPCS_STATUS_P0_EXTDCAPABILITY_E,
  /*10*/  AAS_LPCS_STATUS_P0_LINKSTATUS_E,
  /*11*/  AAS_LPCS_STATUS_P0_ANEGABILITY_E,
  /*12*/  AAS_LPCS_STATUS_P0_ANEGCOMPLETE_E,
  /*13*/  AAS_LPCS_PHY_ID_0_P0_PHYID0_E,
  /*14*/  AAS_LPCS_PHY_ID_1_P0_PHYID1_E,
  /*15*/  AAS_LPCS_DEV_ABILITY_P0_LD_ABILITY_RSV05_E,
  /*16*/  AAS_LPCS_DEV_ABILITY_P0_LD_FD_E,
  /*17*/  AAS_LPCS_DEV_ABILITY_P0_LD_HD_E,
  /*18*/  AAS_LPCS_DEV_ABILITY_P0_LD_PS1_E,
  /*19*/  AAS_LPCS_DEV_ABILITY_P0_LD_PS2_E,
  /*20*/  AAS_LPCS_DEV_ABILITY_P0_LD_ABILITY_RSV9_E,
  /*21*/  AAS_LPCS_DEV_ABILITY_P0_LD_RF1_E,
  /*22*/  AAS_LPCS_DEV_ABILITY_P0_LD_RF2_E,
  /*23*/  AAS_LPCS_DEV_ABILITY_P0_LD_ACK_E,
  /*24*/  AAS_LPCS_DEV_ABILITY_P0_LD_NP_E,
  /*25*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_PABILITY_RSV05_E,
  /*26*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_FD_E,
  /*27*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_HD_E,
  /*28*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_PS1_E,
  /*29*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_PS2_E,
  /*30*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_PABILITY_RSV9_E,
  /*31*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_PABILITY_RSV10_E,
  /*32*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_RF1_E,
  /*33*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_RF2_E,
  /*34*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_ACK_E,
  /*35*/  AAS_LPCS_PARTNER_ABILITY_P0_LP_NP_E,
  /*36*/  AAS_LPCS_AN_EXPANSION_P0_PAGERECEIVEDREALTIME_E,
  /*37*/  AAS_LPCS_AN_EXPANSION_P0_PAGERECEIVED_E,
  /*38*/  AAS_LPCS_AN_EXPANSION_P0_NEXTPAGEABLE_E,
  /*39*/  AAS_LPCS_NP_TX_P0_LD_NP_DATA_E,
  /*40*/  AAS_LPCS_NP_TX_P0_LD_NP_TOGGLE_E,
  /*41*/  AAS_LPCS_NP_TX_P0_LD_NP_ACK2_E,
  /*42*/  AAS_LPCS_NP_TX_P0_LD_NP_MP_E,
  /*43*/  AAS_LPCS_NP_TX_P0_LD_NP_LD_NP_ACK_E,
  /*44*/  AAS_LPCS_NP_TX_P0_LD_NP_NP_E,
  /*45*/  AAS_LPCS_LP_NP_RX_P0_LP_NP_DATA_E,
  /*46*/  AAS_LPCS_LP_NP_RX_P0_LP_NP_TOGGLE_E,
  /*47*/  AAS_LPCS_LP_NP_RX_P0_LP_NP_ACK2_E,
  /*48*/  AAS_LPCS_LP_NP_RX_P0_LP_NP_MP_E,
  /*49*/  AAS_LPCS_LP_NP_RX_P0_LP_NP_ACK_E,
  /*50*/  AAS_LPCS_LP_NP_RX_P0_LP_NP_NP_E,
  /*51*/  AAS_LPCS_SCRATCH_P0_SCRATCH_E,
  /*52*/  AAS_LPCS_REV_P0_REVISION_E,
  /*53*/  AAS_LPCS_LINK_TIMER_0_P0_TIMER0_E,
  /*54*/  AAS_LPCS_LINK_TIMER_0_P0_TIMER15_1_E,
  /*55*/  AAS_LPCS_LINK_TIMER_1_P0_TIMER20_16_E,
  /*56*/  AAS_LPCS_IF_MODE_P0_SGMII_ENA_E,
  /*57*/  AAS_LPCS_IF_MODE_P0_USE_SGMII_AN_E,
  /*58*/  AAS_LPCS_IF_MODE_P0_SGMII_SPEED_E,
  /*59*/  AAS_LPCS_IF_MODE_P0_SGMII_DUPLEX_E,
  /*60*/  AAS_LPCS_IF_MODE_P0_IFMODE_RSV5_E,
  /*61*/  AAS_LPCS_IF_MODE_P0_IFMODE_TX_PREAMBLE_SYNC_E,
  /*62*/  AAS_LPCS_IF_MODE_P0_IFMODE_RX_PREAMBLE_SYNC_E,
  /*63*/  AAS_LPCS_IF_MODE_P0_IFMODE_MODE_XGMII_BASEX_E,
  /*64*/  AAS_LPCS_IF_MODE_P0_IFMODE_SEQ_ENA_E,
  /*65*/  AAS_LPCS_IF_MODE_P0_RX_BR_DIS_E,
  /*66*/  AAS_LPCS_DECODE_ERRORS_P0_DECODEERRORS_E,
  /*67*/  AAS_LPCS_USXGMII_REP_USXGMIIREP_E,
  /*68*/  AAS_LPCS_USXGMII_REP_P0_USXGMII2_5_E,
  /*69*/  AAS_LPCS_TX_IPG_LENGTH_P0_TX_IPG_LENGTH_E,
  /*70*/  AAS_LPCS_GMODE_LPCS_ENABLE_E,
  /*71*/  AAS_LPCS_GMODE_QSGMII_0_ENABLE_E,
  /*72*/  AAS_LPCS_GMODE_QSGMII_4_ENABLE_E,
  /*73*/  AAS_LPCS_GMODE_USGMII8_ENABLE_E,
  /*74*/  AAS_LPCS_GMODE_USGMII_SCRAMBLE_ENABLE_E,
  /*75*/  AAS_LPCS_GSTATUS_GSYNC_STATUS_E,
  /*76*/  AAS_LPCS_GSTATUS_GAN_DONE_STATUS_E,
  /*77*/  AAS_LPCS_USXM0_PORTS_ENA_USXM0_PORTS_ENA_E,
  /*78*/  AAS_LPCS_USXM1_PORTS_ENA_USXM1_PORTS_ENA_E,
  /*79*/  AAS_LPCS_CFG_CLOCK_RATE_CFGCLOCKRATE_E,
  /*80*/  AAS_LPCS_USXGMII_ENABLE_INDICATION_USXGMII_ENABLE_IND_E,
          AAS_LPCS_REGISTER_LAST_E /* should be last */
} MV_HWS_AAS_LPCS_UNIT_FIELDS_ENT;

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsAasLpcsUnits_H */

