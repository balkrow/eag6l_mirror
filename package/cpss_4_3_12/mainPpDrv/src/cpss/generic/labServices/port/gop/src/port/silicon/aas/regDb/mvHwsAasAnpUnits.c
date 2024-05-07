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
* @file mvHwsAasAnpUnits.c
*
* @brief AAS ANP register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasAnpDb[] = {
    /* CONTROL_1 */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*AN_AP_TRAIN_TYPE */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 8 }, /*REG_TXCLK_SYNC_EN_WIDTH_S */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 2 }, /*REG_TRAIN_TYPE_SAMP */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 2 }, /*REG_TRAIN_TYPE_MX_SAMP */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 10 }, /*REG_RESET_PULSE_CONF_DELAY */
  /*5*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*REG_CH_SM_CNT_SATURATE */
  /*6*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*REG_TXSTR_RXSD_CLEAR_CNT */
  /*7*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*REG_TX_SM_CNT_SATURATE */
  /*8*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*REG_RX_SM_CNT_SATURATE */
  /*9*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*REG_TX_READY_LOSS_LATCH_EN */
  /*10*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*REG_DSP_SIGDET_LOSS_LATCH_EN */
  /*11*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*REG_DSP_LOCK_LOSS_LATCH_EN */
  /*12*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*PM_SD_PU_PLL_LATCH_EN */

    /* CONTROL_2 */
  /*13*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REG_PU_TX_CONF_DELAY */
  /*14*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*REG_PU_RX_CONF_DELAY */
  /*15*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 5 }, /*REG_DSP_ON_CONF_DELAY */
  /*16*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  13, /*fieldLen*/ 9 }, /*REG_TX_IDLE_CONF_DLY */
  /*17*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  22, /*fieldLen*/ 5 }, /*REG_PCS_TX_ON_CONF_DELAY */
  /*18*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  27, /*fieldLen*/ 5 }, /*REG_PCS_RX_ON_CONF_DELAY */

    /* CONTROL_3 */
  /*19*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG_NO_PRE_SELECTOR */
  /*20*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG_HCD_RESOLVED_CLEAN */
  /*21*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 6 }, /*REG_RESET_TO_CLOCK_CNT */
  /*22*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*REG_CLOCK_TO_RESET_CNT */
  /*23*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 6 }, /*REG_START_AP_MODE_CNT */
  /*24*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 6 }, /*REG_START_SELECTOR_CNT */
  /*25*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  26, /*fieldLen*/ 6 }, /*REG_START_UNGATE_CNT */

    /* CONTROL_4 */
  /*26*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*REG_SD_DFE_UPDATE_DIS_SAMP */
  /*27*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*REG_SD_DFE_PAT_DIS_SAMP */
  /*28*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*REG_SD_DFE_EN_SAMP */
  /*29*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*REG_SD_TX_IDLE_SAMP */
  /*30*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*REG_TX_TRAIN_ENABLE_SAMP */
  /*31*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 2 }, /*REG_RX_TRAIN_ENABLE_SAMP */
  /*32*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 2 }, /*REG_RX_INIT_SAMP */
  /*33*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*REG_RX_IDLE_COUNT */
  /*34*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*REG_TX_IDLE_COUNT */
  /*35*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*PHY_GEN_TX_DONE_OW */
  /*36*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*PHY_GEN_TX_DONE_OW_VAL */
  /*37*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*PHY_GEN_TX_LOAD_OW */
  /*38*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*PHY_GEN_TX_LOAD_OW_VAL */
  /*39*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*PHY_GEN_TX_START_CNT_DONE_OW */
  /*40*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*PHY_GEN_TX_START_CNT_DONE_OW_VAL */
  /*41*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*PHY_GEN_RX_DONE_OW */
  /*42*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*PHY_GEN_RX_DONE_OW_VAL */
  /*43*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*PHY_GEN_RX_LOAD_OW */
  /*44*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*PHY_GEN_RX_LOAD_OW_VAL */
  /*45*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*PHY_GEN_RX_START_CNT_DONE_OW */
  /*46*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*PHY_GEN_RX_START_CNT_DONE_OW_VAL */
  /*47*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*REG_PHY_GEN_DN_FRC */
  /*48*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*REG_TXSTR_REGRET_ENABLE */
  /*49*/  {/*baseAddr*/ 0xC,  /*offsetFormula*/ 0x0, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*REG_RXSTR_REGRET_ENABLE */

    /* CONTROL_5 */
  /*50*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PIN_SPD_CFG_FOR_R4_AND_R8 */
  /*51*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*PIN_SPD_CFG_FOR_THE_REST */
  /*52*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PIN_SPD_CFG_OW */
  /*53*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*PIN_SPD_CFG_OW_VALUE */

    /* CLOCK_AND_RESET */
  /*54*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PWM_SOFT_RESET_ */
  /*55*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PWM_CLK_EN */

    /* INGRESS_SD_MUX_CONTROL_ */
  /*56*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*SD_MUX_INGRESS_CONTROL0 */
  /*57*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SD_MUX_INGRESS_CONTROL0_ENABLE */
  /*58*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 7 }, /*SD_MUX_INGRESS_CONTROL1 */
  /*59*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SD_MUX_INGRESS_CONTROL1_ENABLE */
  /*60*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 7 }, /*SD_MUX_INGRESS_CONTROL2 */
  /*61*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*SD_MUX_INGRESS_CONTROL2_ENABLE */
  /*62*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 7 }, /*SD_MUX_INGRESS_CONTROL3 */
  /*63*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*SD_MUX_INGRESS_CONTROL3_ENABLE */

    /* LANE_DELAY */
  /*64*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 25 }, /*LANE_DELAY */

    /* TIMER_10MS */
  /*65*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_10MS */

    /* TIMER_35MS */
  /*66*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_35MS */

    /* TIMER_40MS */
  /*67*/  {/*baseAddr*/ 0x2C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_40MS */

    /* TIMER_50MS */
  /*68*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_50MS */

    /* TIMER_70MS */
  /*69*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_70MS */

    /* TIMER_100MS */
  /*70*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_100MS */

    /* TIMER_200MS */
  /*71*/  {/*baseAddr*/ 0x3C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_200MS */

    /* TIMER_300MS */
  /*72*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_300MS */

    /* TIMER_500MS */
  /*73*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_500MS */

    /* TIMER_1S */
  /*74*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_1S */

    /* TIMER_2S */
  /*75*/  {/*baseAddr*/ 0x4C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_2S */

    /* TIMER_3150MS */
  /*76*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_3150MS */

    /* TIMER_5S */
  /*77*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_5S */

    /* TIMER_6S */
  /*78*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_6S */

    /* TIMER_9S_LOW */
  /*79*/  {/*baseAddr*/ 0x5C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_9S_LOW */

    /* TIMER_12S_LOW */
  /*80*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_12S_LOW */

    /* TIMER_9S_12S_HIGH */
  /*81*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_9S_HIGH */
  /*82*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_12S_HIGH */

    /* TIMER_12600MS_LOW */
  /*83*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_12600MS_LOW */

    /* TIMER_20S_LOW */
  /*84*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_20S_LOW */

    /* TIMER_12600MS_20S_HIGH */
  /*85*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_12600MS_HIGH */
  /*86*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_20S_HIGH */

    /* TIMER_30S_LOW */
  /*87*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_30S_LOW */

    /* TIMER_40S_LOW */
  /*88*/  {/*baseAddr*/ 0x7C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RG_AN_TIMERS_40S_LOW */

    /* TIMER_30S_40S_HIGH */
  /*89*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_30S_HIGH */
  /*90*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 16 }, /*RG_AN_TIMERS_40S_HIGH */

    /* AN_ABILITIES */
  /*91*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RUNI_CFG_ABILITY1_A */
  /*92*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 16 }, /*RUNI_CFG_ABILITY2_A */

    /* AN_REVISION */
  /*93*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RUNI_OUI_NUM_3_18_A */
  /*94*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 4 }, /*RUNI_CFG_REV_ID_A */
  /*95*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 6 }, /*RUNI_CFG_MODEL_NUM_A */
  /*96*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x0, /*fieldStart*/  26, /*fieldLen*/ 6 }, /*RUNI_OUI_NUM_19_24_A */

    /* AN_TRAIN_TYPE_1 */
  /*97*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_1G */
  /*98*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_2P5G */
  /*99*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_5G */
  /*100*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_10G */
  /*101*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_25G */
  /*102*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_40GR4 */
  /*103*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_40GR2 */
  /*104*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_50GR2 */
  /*105*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_50G */
  /*106*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  18, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_100GR4 */
  /*107*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_100GR2 */
  /*108*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  22, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_200GR4 */
  /*109*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  24, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_400GR8 */
  /*110*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  26, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_200GR8 */
  /*111*/  {/*baseAddr*/ 0x8C,  /*offsetFormula*/ 0x0, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*TX_TRAIN_COUPLE_ENABLE */

    /* SPEED_TABLE_1 */
  /*112*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_1P25G */
  /*113*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_2P578125G */
  /*114*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_3P125G */

    /* SPEED_TABLE_2 */
  /*115*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_5G */
  /*116*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_5P15625G */
  /*117*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_10G */

    /* SPEED_TABLE_3 */
  /*118*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_10P3125G */
  /*119*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_20P625G */
  /*120*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_25P78125G */

    /* SPEED_TABLE_4 */
  /*121*/  {/*baseAddr*/ 0x9C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_26P5625G */
  /*122*/  {/*baseAddr*/ 0x9C,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_27P5G */
  /*123*/  {/*baseAddr*/ 0x9C,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_53P125G */

    /* SPEED_TABLE_5 */
  /*124*/  {/*baseAddr*/ 0xA0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_56G */
  /*125*/  {/*baseAddr*/ 0xA0,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_106P25G */
  /*126*/  {/*baseAddr*/ 0xA0,  /*offsetFormula*/ 0x0, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_12P375G */

    /* INTERRUPT_CAUSE */
  /*127*/  {/*baseAddr*/ 0xA4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*128*/  {/*baseAddr*/ 0xA4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*129*/  {/*baseAddr*/ 0xA4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ILLEGAL_TABLE_OVERLAP_INT */

    /* INTERRUPT_MASK */
  /*130*/  {/*baseAddr*/ 0xA8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*GLOBAL_INT_MASK */

    /* METAL_FIX */
  /*131*/  {/*baseAddr*/ 0xAC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*METAL_FIX */

    /* INTERRUPT_SUMMARY_CAUSE */
  /*132*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*INTERRUPT_CAUSE_INT_SUM */
  /*133*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GLOBAL_INT */
  /*134*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*135*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_INT2_SUM */
  /*136*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*137*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P1_INT2_SUM */
  /*138*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*139*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P2_INT2_SUM */
  /*140*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*141*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P3_INT2_SUM */
  /*142*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*143*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P4_INT2_SUM */
  /*144*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*145*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P5_INT2_SUM */
  /*146*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*147*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P6_INT2_SUM */
  /*148*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*149*/  {/*baseAddr*/ 0xB0,  /*offsetFormula*/ 0x0, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P7_INT2_SUM */

    /* INTERRUPT_SUMMARY_MASK */
  /*150*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*INTERRUPT_MASK */
  /*151*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_INTERRUPT1_MASK */
  /*152*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_INTERRUPT2_MASK */
  /*153*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P1_INTERRUPT1_MASK */
  /*154*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P1_INTERRUPT2_MASK */
  /*155*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P2_INTERRUPT1_MASK */
  /*156*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P2_INTERRUPT2_MASK */
  /*157*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P3_INTERRUPT1_MASK */
  /*158*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P3_INTERRUPT2_MASK */
  /*159*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P4_INTERRUPT1_MASK */
  /*160*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P4_INTERRUPT2_MASK */
  /*161*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P5_INTERRUPT1_MASK */
  /*162*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P5_INTERRUPT2_MASK */
  /*163*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P6_INTERRUPT1_MASK */
  /*164*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P6_INTERRUPT2_MASK */
  /*165*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P7_INTERRUPT1_MASK */
  /*166*/  {/*baseAddr*/ 0xB4,  /*offsetFormula*/ 0x0, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P7_INTERRUPT2_MASK */

    /* LAST_VIOLATION */
  /*167*/  {/*baseAddr*/ 0xB8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* SD_TX_IDLE_MIN_WAIT */
  /*168*/  {/*baseAddr*/ 0xBC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_TX_IDLE_MIN_WAIT_S */

    /* SD_RX_IDLE_MIN_WAIT */
  /*169*/  {/*baseAddr*/ 0xC0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_RX_IDLE_MIN_WAIT_S */

    /* PHY_GEN_PDN_TO_LOAD_TIMER */
  /*170*/  {/*baseAddr*/ 0xC4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 24 }, /*REG_PHY_GEN_PDN_TO_LOAD_TIMER */

    /* PHY_GEN_LOAD_TO_PUP_TIMER */
  /*171*/  {/*baseAddr*/ 0xC8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 24 }, /*REG_PHY_GEN_LOAD_TO_PUP_TIMER */

    /* RESET_TX_CORE */
  /*172*/  {/*baseAddr*/ 0xCC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_TX_RESET_MIN_WAIT_S */

    /* RESET_RX_CORE */
  /*173*/  {/*baseAddr*/ 0xD0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_RX_RESET_MIN_WAIT_S */

    /* SD_TX_RESET_WAIT_MAX_TIMER */
  /*174*/  {/*baseAddr*/ 0xD4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_TX_RESET_WAIT_TIME_OUT_S */
  /*175*/  {/*baseAddr*/ 0xD4,  /*offsetFormula*/ 0x0, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*REG_TX_RESET_WAIT_TIME_OUT_S_INF */

    /* SD_RX_RESET_WAIT_MAX_TIMER */
  /*176*/  {/*baseAddr*/ 0xD8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*REG_RX_RESET_WAIT_TIME_OUT_S */
  /*177*/  {/*baseAddr*/ 0xD8,  /*offsetFormula*/ 0x0, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*REG_RX_RESET_WAIT_TIME_OUT_S_INF */

    /* SPEED_MODULATION_ */
  /*178*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__0_MODULATION */
  /*179*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__1_MODULATION */
  /*180*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__2_MODULATION */
  /*181*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__3_MODULATION */
  /*182*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__4_MODULATION */
  /*183*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__5_MODULATION */
  /*184*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__6_MODULATION */
  /*185*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__7_MODULATION */
  /*186*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__8_MODULATION */
  /*187*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__9_MODULATION */
  /*188*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__10_MODULATION */
  /*189*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__11_MODULATION */
  /*190*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__12_MODULATION */
  /*191*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__13_MODULATION */
  /*192*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__14_MODULATION */
  /*193*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__15_MODULATION */
  /*194*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__16_MODULATION */
  /*195*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__17_MODULATION */
  /*196*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__18_MODULATION */
  /*197*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__19_MODULATION */
  /*198*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__20_MODULATION */
  /*199*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__21_MODULATION */
  /*200*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__22_MODULATION */
  /*201*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__23_MODULATION */
  /*202*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__24_MODULATION */
  /*203*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__25_MODULATION */
  /*204*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__26_MODULATION */
  /*205*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__27_MODULATION */
  /*206*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__28_MODULATION */
  /*207*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__29_MODULATION */
  /*208*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__30_MODULATION */
  /*209*/  {/*baseAddr*/ 0xDC,  /*offsetFormula*/ 0x4, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*PHY_GEN_VALUE__31_MODULATION */

    /* TRAIN_ENABLE_DEASSERTION_TIMER */
  /*210*/  {/*baseAddr*/ 0xE8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*TRAIN_EN_DEASSERT_TIMER */

    /* GLOBAL_STATUS */
  /*211*/  {/*baseAddr*/ 0xEC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*STAT_SPD_CFG_0 */
  /*212*/  {/*baseAddr*/ 0xEC,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*STAT_SPD_CFG_1 */

    /* FSM_DEBUG_CONTROL */
  /*213*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FSM_DEBUG_ENABLE */
  /*214*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*CHANNEL_AND_AN_FSMS_SELECTOR */
  /*215*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*LANE_SELECTOR */
  /*216*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*MANUAL_TRIGGER */
  /*217*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*CHANNEL_FSM_TRIGGER_ENABLE */
  /*218*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 5 }, /*CHANNEL_FSM_TRIGGER_STATE */
  /*219*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*AN_FSM_TRIGGER_ENABLE */
  /*220*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  15, /*fieldLen*/ 4 }, /*AN_FSM_TRIGGER_STATE */
  /*221*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  19, /*fieldLen*/ 11 }, /*LANE_TRIGGER_ENABLE */
  /*222*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*FSM_DEBUG_CLIENT_SELECT */
  /*223*/  {/*baseAddr*/ 0xF0,  /*offsetFormula*/ 0x0, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*FSM_DEBUG_RAM_POWER_DOWN */

    /* FSM_DEBUG_WRITE_ADDRESS */
  /*224*/  {/*baseAddr*/ 0xF4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*FSM_DEBUG_WRITE_ADDRESS */

    /* AN_TRAIN_TYPE_2 */
  /*225*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_100GR */
  /*226*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_200GR2 */
  /*227*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_400GR4 */
  /*228*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_800GR8 */
  /*229*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_12GR1 */
  /*230*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_53GR1 */
  /*231*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_106GR1 */
  /*232*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_106GR2 */
  /*233*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_212GR4 */
  /*234*/  {/*baseAddr*/ 0xF8,  /*offsetFormula*/ 0x0, /*fieldStart*/  18, /*fieldLen*/ 2 }, /*AN_TRAIN_TYPE_MODE_424GR8 */

    /* EGRESS_SD_MUX_CONTROL_ */
  /*228*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*SD_MUX_EGRESS_CONTROL0 */
  /*229*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SD_MUX_EGRESS_CONTROL0_ENABLE */
  /*230*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 7 }, /*SD_MUX_EGRESS_CONTROL1 */
  /*231*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SD_MUX_EGRESS_CONTROL1_ENABLE */
  /*232*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 7 }, /*SD_MUX_EGRESS_CONTROL2 */
  /*233*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*SD_MUX_EGRESS_CONTROL2_ENABLE */
  /*234*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 7 }, /*SD_MUX_EGRESS_CONTROL3 */
  /*235*/  {/*baseAddr*/ 0xFC,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*SD_MUX_EGRESS_CONTROL3_ENABLE */

    /* SPEED_TABLE_6 */
  /*243*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_56P25G */
  /*244*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*PM_SPEED_TABLE_112P5G */

    /* LANE_INTERRUPT_CAUSE */
  /*245*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PORT_INT_SUM */
  /*246*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AN_RESTART */
  /*247*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_HCD_FOUND */
  /*248*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AN_GOOD_CK */
  /*249*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_CH_SM_BP_REACHED_INT */
  /*250*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_SD_TX_SM_BP_REACHED_INT */
  /*251*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SD_RX_SM_BP_REACHED_INT */
  /*252*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_NORM_EN_MX_S */
  /*253*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_PWRUP_EN_MX_S */
  /*254*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_RXON_EN_MX_S */
  /*255*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_RXSD_EN_MX_S */
  /*256*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_TXON_EN_MX_S */
  /*257*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_TXRXON_EN_MX_S */
  /*258*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_INT_PROG_PWM_TXRXSD_EN_MX_S */
  /*259*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_INT_PM_DSP_RXUP_TIME_OUT */
  /*260*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_INT_PM_PCS_LINK_TIMER_OUT */
  /*261*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_INT_RXON_WAIT_TIME_OUT */
  /*262*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_INT_TXON_WAIT_TIME_OUT */
  /*263*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_INT_TXRX_START_WAIT_TIME_OUT */
  /*264*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_INT_PM_PROG_RX_INIT_S */
  /*265*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_INT_PM_PROG_RX_TRAIN_S */
  /*266*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_INT_PM_PROG_TX_TRAIN_S */
  /*267*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_INT_PROG_RX_TIME_OUT */
  /*268*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_INT_PROG_TX_TIME_OUT */
  /*269*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_INT_RX_INIT_TIME_OUT */
  /*270*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_INT_RX_PLL_UP_TIME_OUT */
  /*271*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_INT_RX_TRAIN_TIME_OUT */
  /*272*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_INT_TX_PLL_UP_TIME_OUT */
  /*273*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_INT_TX_TRAIN_TIME_OUT */
  /*274*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_INT_PROG_TIME_OUT */
  /*275*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_INT_WAIT_PWRDN_TIME_OUT */

    /* LANE_INTERRUPT_MASK */
  /*276*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 30 }, /*P0_PORT_INT_MASK */

    /* LANE_INTERRUPT2_CAUSE */
  /*277*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PORT_INT2_SUM */
  /*278*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_W_ERR */
  /*279*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_R_ERR */
  /*280*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_W_ERR */
  /*281*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_R_ERR */
  /*282*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_FULL */
  /*283*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_TX_TFIFO_EMPTY */
  /*284*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_FULL */
  /*285*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RX_TFIFO_EMPTY */
  /*286*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_INT_TX_RESET_WAIT_TIME_OUT */
  /*287*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_INT_RX_RESET_WAIT_TIME_OUT */
  /*288*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_INT_TX_SPEED_CHANGE_2_TIME_OUT */
  /*289*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_INT_RX_SPEED_CHANGE_2_TIME_OUT */

    /* LANE_INTERRUPT2_MASK */
  /*290*/  {/*baseAddr*/ 0x100C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 12 }, /*P0_PORT_INT2_MASK */

    /* LANE_CHANNEL_SM_CONTROL */
  /*291*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*P0_CH_SM_OVERRIDE_CTRL */
  /*292*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_CH_SM_AMDISAM */
  /*293*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_CH_SM_BP_REACHED */
  /*294*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 5 }, /*P0_CH_SM_STATE */

    /* LANE_SERDES_TX_SM_CONTROL */
  /*295*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*P0_SD_TX_SM_OVERRIDE_CTRL */
  /*296*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_SD_TX_SM_AMDISAM */
  /*297*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_SD_TX_SM_BP_REACHED */
  /*298*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*P0_SD_TX_SM_STATE */

    /* LANE_SERDES_RX_SM_CONTROL */
  /*299*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*P0_SD_RX_SM_OVERRIDE_CTRL */
  /*300*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_SD_RX_SM_AMDISAM */
  /*301*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_SD_RX_SM_BP_REACHED */
  /*302*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 5 }, /*P0_SD_RX_SM_STATE */

    /* LANE_AN_CONTROL_1 */
  /*303*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_AP_MODE_OW */
  /*304*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_AP_MODE_OW_VAL */
  /*305*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_RX_OW */
  /*306*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_RX_OW_VAL */
  /*307*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FR_OW */
  /*308*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FR_OW_VAL */
  /*309*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_PM_NORM_X_STATE_OW */
  /*310*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_PM_NORM_X_STATE_OW_VAL */
  /*311*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_SYS_OW */
  /*312*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_SYS_OW_VAL */
  /*313*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_TX_OW */
  /*314*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_PM_AP_RESET_TX_OW_VAL */
  /*315*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FT_OW */
  /*316*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_PM_ENCLK_AP_FT_OW_VAL */
  /*317*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_AN_PCS_SEL_OW */
  /*318*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_AN_PCS_SEL_OW_VAL */
  /*319*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_RG_ST_PCSLINK_MAX_TIME_AP_INF_S */
  /*320*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_REG_AP_FORCE_MODE_S */
  /*321*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_RG_BREAK_LINK_TIMER_FAST */
  /*322*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_RG_AUTONEG_WAIT_TIMER_FAST */
  /*323*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_RG_LINK_FAIL_INHIBIT_TIMER_FAST */
  /*324*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_RG_TIMER2_FAST */
  /*325*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_PM_TX_RX_DCLK_4X_EN_OW */
  /*326*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_PM_TXDCLK_4X_EN_OW_VAL */
  /*327*/  {/*baseAddr*/ 0x101C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_PM_RXDCLK_4X_EN_OW_VAL */

    /* LANE_AN_CONTROL_2 */
  /*328*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PHY_GEN_AP_OW */
  /*329*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 10 }, /*P0_PHY_GEN_AP_OW_VAL */

    /* LANE_AN_TIMERS_1 */
  /*330*/  {/*baseAddr*/ 0x1024,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RG_BREAK_LINK_TIMER_FAST_VAL */
  /*331*/  {/*baseAddr*/ 0x1024,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 16 }, /*P0_RG_AUTONEG_WAIT_TIMER_FAST_VAL */

    /* LANE_AN_TIMERS_2 */
  /*332*/  {/*baseAddr*/ 0x1028,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_RG_LINK_FAIL_INHIBIT_TIMER_FAST_VAL */
  /*333*/  {/*baseAddr*/ 0x1028,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 16 }, /*P0_RG_TIMER2_FAST_VAL */

    /* LANE_AN_TIED_IN */
  /*334*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_AP_ANEG_REMOTE_READY_S */
  /*335*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PRBS_MODE_S */
  /*336*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_GRG_3_0_15_S */
  /*337*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 5 }, /*P0_N_AG_MODE_S */
  /*338*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_N_AA_LINK_GOOD_S */
  /*339*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_FAR_CLEAR_RESET_ALL_S */
  /*340*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_FAR_SET_RESTART_ALL_S */
  /*341*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_PM_KR_ENABLE_S */
  /*342*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_PM_LOOPBACK_S */
  /*343*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_POR_FEC_ADV_S */
  /*344*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_RING_OSC_A */
  /*345*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 5 }, /*P0_Z80_AG_MODE_S */
  /*346*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_Z80_FEC_ENABLE_S */
  /*347*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_Z80_LLFEC_ENABLE_S */
  /*348*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_Z80_RESOLVED_S */
  /*349*/  {/*baseAddr*/ 0x102C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_Z80_RSFEC_ENABLE_S */

    /* LANE_AN_HANG_OUT */
  /*350*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AP_IRQ_S */
  /*351*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AP_RG_3_0_WR_S */
  /*352*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 5 }, /*P0_AG_MODE_S */
  /*353*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_ANEG_INT_S */
  /*354*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_AG_REG_1_6_S */
  /*355*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_AP_AG_LINK_DOWN_PLS_S */
  /*356*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_IEEE_AG_ANEG_ENABLE_PLS_S */
  /*357*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_BACKWARD_COMPATIBLE_EN */
  /*358*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_MP40GR1_S */
  /*359*/  {/*baseAddr*/ 0x1030,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_MP50GNRZ_S */

    /* LANE_AN_STATUS_1 */
  /*360*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_4X_S */
  /*361*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_G_S */
  /*362*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_2P5G_S */
  /*363*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_5GR_S */
  /*364*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_10GKR_S */
  /*365*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_IEEE25GS_S */
  /*366*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_IEEE25GR_S */
  /*367*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON25GKR_S */
  /*368*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON25GCR_S */
  /*369*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_40GKR4_S */
  /*370*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_40GCR4_S */
  /*371*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GKR2_S */
  /*372*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GCR2_S */
  /*373*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GCR4_S */
  /*374*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GCR10_S */
  /*375*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GKR4_S */
  /*376*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GKP4_S */
  /*377*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_200GKR8_S */
  /*378*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_25GKR2_S */
  /*379*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON40GR2_S */
  /*380*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GKR4_S */
  /*381*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_50GRCR_S */
  /*382*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GRCR2_S */
  /*383*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_200GRCR4_S */
  /*384*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_100GRCR_S */
  /*385*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_200GRCR2_S */
  /*386*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_400GRCR4_S */
  /*387*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_800GRCR8_S */
  /*388*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON400GRCR8_S */
  /*389*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_CON800GKCR8_S */
  /*390*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_12GR1_S */
  /*391*/  {/*baseAddr*/ 0x1034,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_53GR1_S */

    /* LANE_AN_STATUS_2 */
  /*392*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AP_AA_CLEAR_HCD_S */
  /*393*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_IEEE_AG_ANEG_ENABLE_S */
  /*394*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_AP_AG_RESTART_ANEG_S */
  /*395*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_AP_AG_LINK_S */
  /*396*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_AP_AG_HCD_RESOLVED_S */
  /*397*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_AA_LINK_GOOD_S */
  /*398*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_AP_AG_RX_PAUSE_ENABLE_S */
  /*399*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_AP_AG_TX_PAUSE_ENABLE_S */
  /*400*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_AP_FEC_ENABLE_S */
  /*401*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_AP_RSFEC_ENABLE_S */
  /*402*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_AG_LLFEC_ENABLE_S */
  /*403*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_AG_RSFEC_INT_S */
  /*404*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_106GR1_S */
  /*405*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_106GR2_S */
  /*406*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_212GR4_S */
  /*407*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_424GR8_S */
  /*408*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_AP_PWRUP_MP100GR4_RSFEC544_S */

    /* LANE_AN_RESTART_COUNTER */
  /*409*/  {/*baseAddr*/ 0x103C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_AN_RESTART_COUNTER */

    /* LANE_CH_PWRDN_MIN_TIMER */
  /*410*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_REG_PWRDN_MIN_TIME_S */

    /* LANE_CH_WAIT_PWRDN_MAX_TIMER */
  /*411*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_WAIT_PWRDN_MAX_TIMER */

    /* LANE_CH_WAIT_PWRDN_MIN_TIMER */
  /*412*/  {/*baseAddr*/ 0x1048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_WAIT_PWRDN_MIN_TIMER */

    /* LANE_CH_TXRX_MIN_TIMER */
  /*413*/  {/*baseAddr*/ 0x104C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_TXRX_MIN_TIMER */

    /* LANE_CH_TXRX_MAX_TIMER */
  /*414*/  {/*baseAddr*/ 0x1050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_TXRX_MAX_TIMER */

    /* LANE_CH_PCS_LOST_MIN_TIMER */
  /*415*/  {/*baseAddr*/ 0x1054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_PCS_LOST_MIN_TIMER */

    /* LANE_CH_ST_RESET_MIN_TIMER */
  /*416*/  {/*baseAddr*/ 0x1058,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_ST_RESET_MIN_TIME */

    /* LANE_CH_ST_MIN_TIMER */
  /*417*/  {/*baseAddr*/ 0x105C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_ST_WAIT_MIN_S */

    /* LANE_CH_RXON_MAX_TIMER */
  /*418*/  {/*baseAddr*/ 0x1060,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_RXON_MAX_TIMER */

    /* LANE_CH_TXON_MAX_TIMER */
  /*419*/  {/*baseAddr*/ 0x1064,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_RG_TXON_MAX_TIMER */

    /* LANE_CH_DSP_RXUP_MAX_TIMER */
  /*420*/  {/*baseAddr*/ 0x1068,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 15 }, /*P0_RG_DSP_RXUP_MAX_TIMER */
  /*421*/  {/*baseAddr*/ 0x1068,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_RG_DSP_RXUP_MAX_TIMER_INF */

    /* LANE_CH_PROG_MAX_TIMER */
  /*422*/  {/*baseAddr*/ 0x106C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_REG_PROG_MAX_TIME_S */

    /* LANE_PCS_LINK_MAX_TIMER_NORM */
  /*423*/  {/*baseAddr*/ 0x1070,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_PCS_LINK_MAX_TIMER_NORM */

    /* LANE_PCS_LINK_MAX_TIMER_AP */
  /*424*/  {/*baseAddr*/ 0x1074,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_PCS_LINK_MAX_TIMER_AP */

    /* LANE_SD_PLL_UP_MAX_TIMER */
  /*425*/  {/*baseAddr*/ 0x1078,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_PLL_UP_TIME_OUT_S */
  /*426*/  {/*baseAddr*/ 0x1078,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_PLL_UP_TIME_OUT_S_INF */

    /* LANE_SD_PROG_MAX_TIMER */
  /*427*/  {/*baseAddr*/ 0x107C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_SD_PROG_MAX_TIME_S */
  /*428*/  {/*baseAddr*/ 0x107C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_SD_PROG_MAX_TIME_S_INF */

    /* LANE_SD_RX_INIT_MIN_TIMER */
  /*429*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_INIT_MIN_WAIT_S */

    /* LANE_SD_RX_INIT_MAX_TIMER */
  /*430*/  {/*baseAddr*/ 0x1084,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_INIT_TIME_OUT_S */
  /*431*/  {/*baseAddr*/ 0x1084,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_RX_INIT_TIME_OUT_S_INF */

    /* LANE_SD_RX_PLL_UP_MIN_TIMER */
  /*432*/  {/*baseAddr*/ 0x1088,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_PLL_UP_MIN_WAIT_S */

    /* LANE_SD_RXT_MIN_TIMER */
  /*433*/  {/*baseAddr*/ 0x108C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_TRAIN_MIN_WAIT_S */

    /* LANE_SD_RXT_OK_MAX_TIMER */
  /*434*/  {/*baseAddr*/ 0x1090,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_TRAIN_OK_WAIT_TIMEOUT_S */

    /* LANE_SD_RXT_MAX_TIMER */
  /*435*/  {/*baseAddr*/ 0x1094,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_TRAIN_TIME_OUT_S */
  /*436*/  {/*baseAddr*/ 0x1094,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_TIME_OUT_S_INF */

    /* LANE_SD_IDLE_RM_MIN_TIMER */
  /*437*/  {/*baseAddr*/ 0x1098,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_COMPHY_IDLE_REMOVE_MIN_WAIT_S */

    /* LANE_SD_IDLE_AP_MIN_TIMER */
  /*438*/  {/*baseAddr*/ 0x109C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_IDLE_WAIT_TIME_AP_S */

    /* LANE_SD_IDLE_NAP_MIN_TIMER */
  /*439*/  {/*baseAddr*/ 0x10A0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_IDLE_WAIT_TIME_OTHER_S */

    /* LANE_SD_TX_PLL_UP_MIN_TIMER */
  /*440*/  {/*baseAddr*/ 0x10A4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_PLL_UP_MIN_WAIT_S */

    /* LANE_SD_TXT_MIN_TIMER */
  /*441*/  {/*baseAddr*/ 0x10A8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_TRAIN_MIN_WAIT_S */

    /* LANE_SD_TXT_OK_MAX_TIMER */
  /*442*/  {/*baseAddr*/ 0x10AC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_TRAIN_OK_WAIT_TIMEOUT_S */

    /* LANE_SD_TXT_MAX_TIMER */
  /*443*/  {/*baseAddr*/ 0x10B0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_TX_TRAIN_TIME_OUT_S */
  /*444*/  {/*baseAddr*/ 0x10B0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_TIME_OUT_S_INF */

    /* LANE_SD_RX_RETRAIN_MIN_TIMER */
  /*445*/  {/*baseAddr*/ 0x10B4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_RETRAIN_TIME_S */

    /* LANE_SD_RX_WAIT_PLUG_MIN_TIMER */
  /*446*/  {/*baseAddr*/ 0x10B8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_WAIT_PLUG_MIN_WAIT_S */

    /* LANE_SD_RX_WAIT_SQ_DET_MIN_TIMER */
  /*447*/  {/*baseAddr*/ 0x10BC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_RX_WAIT_SQ_DET_MIN_WAIT_S */

    /* LANE_CONTROL_1 */
  /*448*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_PORT_MODE */
  /*449*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_POWER_DOWN */
  /*450*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_FAST_SPEED_CHANGE_ENABLE */
  /*451*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*P0_FAST_SPEED_CHANGE_POTENTIAL_LANES */
  /*452*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_PCS_EN */
  /*453*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_RX_INIT_B4_TX_TRAIN */
  /*454*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 2 }, /*P0_CUSTOM_NUM_OF_LANES */
  /*455*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_REG_PWRUP_CNT_CLEAR_S */
  /*456*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_RG_ST_PCSLINK_MAX_TIME_NORM_INF_S */
  /*457*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_RG_MODE_1G_OW */
  /*458*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_RG_MODE_1G_OW_VAL */
  /*459*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_RG_MODE_2P5G_OW */
  /*460*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_RG_MODE_2P5G_OW_VAL */
  /*461*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_RG_MODE_5G_OW */
  /*462*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_RG_MODE_5G_OW_VAL */
  /*463*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_RG_MODE_10G_OW */
  /*464*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_RG_MODE_10G_OW_VAL */
  /*465*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_RG_MODE_25G_OW */
  /*466*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_RG_MODE_25G_OW_VAL */
  /*467*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR4_OW */
  /*468*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR4_OW_VAL */
  /*469*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR2_OW */
  /*470*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_RG_MODE_40GR2_OW_VAL */
  /*471*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR2_OW */
  /*472*/  {/*baseAddr*/ 0x10C0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR2_OW_VAL */

    /* LANE_CONTROL_2 */
  /*473*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR_OW */
  /*474*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_RG_MODE_50GR_OW_VAL */
  /*475*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR4_OW */
  /*476*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR4_OW_VAL */
  /*477*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_MODE_110GR4_OW */
  /*478*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RG_MODE_110GR4_OW_VAL */
  /*479*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR2_OW */
  /*480*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR2_OW_VAL */
  /*481*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RG_MODE_107GR2_OW */
  /*482*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RG_MODE_107GR2_OW_VAL */
  /*483*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR8_OW */
  /*484*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR8_OW_VAL */
  /*485*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR4_OW */
  /*486*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR4_OW_VAL */
  /*487*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_RG_MODE_400GR8_OW */
  /*488*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_RG_MODE_400GR8_OW_VAL */
  /*489*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_RG_MODE_428GR8_OW */
  /*490*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_RG_MODE_428GR8_OW_VAL */
  /*491*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_RG_MODE_QSGMII_OW */
  /*492*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_RG_MODE_QSGMII_OW_VAL */
  /*493*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_RG_MODE_USGMII_OW */
  /*494*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_RG_MODE_USGMII_OW_VAL */
  /*495*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX2P5G_OW */
  /*496*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX2P5G_OW_VAL */
  /*497*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX5G_OW */
  /*498*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX5G_OW_VAL */
  /*499*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX10G_OW */
  /*500*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX10G_OW_VAL */
  /*501*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX20G_OW */
  /*502*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_RG_MODE_USX20G_OW_VAL */
  /*503*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_RG_MODE_CUSTOM_OW */
  /*504*/  {/*baseAddr*/ 0x10C4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_RG_MODE_CUSTOM_OW_VAL */

    /* LANE_CONTROL_3 */
  /*505*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_SD_PHY_GEN_TX_OW */
  /*506*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_SD_PHY_GEN_RX_OW */
  /*507*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_SD_PHY_GEN_OW */
  /*508*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_NR_RX_RESET_OW */
  /*509*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_PM_NR_RX_RESET_OW_VAL */
  /*510*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_PM_AN_RESTART_OW */
  /*511*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_PM_AN_RESTART_OW_VAL */
  /*512*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_PM_NR_TX_RESET_OW */
  /*513*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_PM_NR_TX_RESET_OW_VAL */
  /*514*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_PLL_OW */
  /*515*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_PLL_OW_VAL */
  /*516*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_TX_OW */
  /*517*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_TX_OW_VAL */
  /*518*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_RX_OW */
  /*519*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_PM_SD_PU_RX_OW_VAL */
  /*520*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_PM_SD_SOFTRST_S_OW */
  /*521*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_PM_SD_SOFTRST_S_OW_VAL */
  /*522*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_PM_SD_COUPLE_MODE_EN_OW */
  /*523*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_PM_SD_COUPLE_MODE_EN_OW_VAL */
  /*524*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_PM_SD_TXCLK_SYNC_EN_PLL_OW */
  /*525*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_PM_SD_TXCLK_SYNC_EN_PLL_OW_VAL */
  /*526*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_PM_ST_EN_OW */
  /*527*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_PM_ST_EN_OW_VAL */
  /*528*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_PM_SOFTRST_S_OW */
  /*529*/  {/*baseAddr*/ 0x10C8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_PM_SOFTRST_S_OW_VAL */

    /* LANE_CONTROL_4 */
  /*530*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_PCS_COUPLE_OW */
  /*531*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PCS_COUPLE_OW_VAL */
  /*532*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_TRAIN_TYPE_MX_OW */
  /*533*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*P0_PM_TRAIN_TYPE_MX_OW_VAL */
  /*534*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_MAX_TIME_S_INF */
  /*535*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PM_PWRDN_OW */
  /*536*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_PWRDN_OW_VAL */
  /*537*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_CLEAR_OW */
  /*538*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_CLEAR_OW_VAL */
  /*539*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_PM_SD_TX_IDLE_OW */
  /*540*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_PM_SD_TX_IDLE_OW_VAL */
  /*541*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_RESOLVED_OW */
  /*542*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_PM_AN_HCD_RESOLVED_OW_VAL */
  /*543*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_UPDATE_DIS_OW */
  /*544*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_UPDATE_DIS_OW_VAL */
  /*545*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_PM_TX_TRAIN_POLY_SEL_OW */
  /*546*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 2 }, /*P0_PM_TX_TRAIN_POLY_SEL_OW_VAL */
  /*547*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_PM_DATA_GRAY_CODE_EN */
  /*548*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_PM_DATA_PRE_CODE_EN */
  /*549*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_PAT_DIS_OW */
  /*550*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_PAT_DIS_OW_VAL */
  /*551*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_EN_OW */
  /*552*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_PM_SD_DFE_EN_OW_VAL */
  /*553*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_REG_TX_TFIFO_W_UPD */
  /*554*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_REG_TX_TFIFO_R_UPD */
  /*555*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_REG_RX_TFIFO_W_UPD */
  /*556*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_REG_RX_TFIFO_R_UPD */
  /*557*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_ERROR_S_OW */
  /*558*/  {/*baseAddr*/ 0x10CC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 2 }, /*P0_TX_TRAIN_ERROR_S_OW_VAL */

    /* LANE_CONTROL_5 */
  /*559*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_AP_EN_S_OW */
  /*560*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_AP_EN_S_OW_VAL */
  /*561*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_OW */
  /*562*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_OW_VAL */
  /*563*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_OW */
  /*564*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_OW_VAL */
  /*565*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SQ_DETECTED_LPF_OW */
  /*566*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_SQ_DETECTED_LPF_OW_VAL */
  /*567*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RX_INIT_DONE_OW */
  /*568*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RX_INIT_DONE_OW_VAL */
  /*569*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_COMPLETE_OW */
  /*570*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_COMPLETE_OW_VAL */
  /*571*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_FAILED_OW */
  /*572*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_FAILED_OW_VAL */
  /*573*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_COMPLETE_OW */
  /*574*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_COMPLETE_OW_VAL */
  /*575*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_FAILED_OW */
  /*576*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_FAILED_OW_VAL */
  /*577*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_SD_TXCLK_SYNC_START_OUT_OW */
  /*578*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_SD_TXCLK_SYNC_START_OUT_OW_VAL */
  /*579*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_PM_DSP_TXDN_ACK_OW */
  /*580*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_PM_DSP_TXDN_ACK_OW_VAL */
  /*581*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_PM_DSP_RXDN_ACK_OW */
  /*582*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_PM_DSP_RXDN_ACK_OW_VAL */
  /*583*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_PM_DSP_TX_READY_OW */
  /*584*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_PM_DSP_TX_READY_OW_VAL */
  /*585*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_PM_DSP_SIGDET_OW */
  /*586*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_PM_DSP_SIGDET_OW_VAL */
  /*587*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_PM_DSP_LOCK_OW */
  /*588*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_PM_DSP_LOCK_OW_VAL */
  /*589*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_PM_RX_INIT_OW */
  /*590*/  {/*baseAddr*/ 0x10D0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_PM_RX_INIT_OW_VAL */

    /* LANE_CONTROL_6 */
  /*591*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_RX_TRAIN_ENABLE_OW */
  /*592*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_RX_TRAIN_ENABLE_OW_VAL */
  /*593*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_TX_TRAIN_ENABLE_OW */
  /*594*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PM_TX_TRAIN_ENABLE_OW_VAL */
  /*595*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_TXRX_MAX_TIMER_INF */
  /*596*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_NORMAL_STATE_LOCK */
  /*597*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_SD_RX_DTL_CLAMP_S_OW */
  /*598*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_SD_RX_DTL_CLAMP_S_OW_VAL */
  /*599*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_DSP_SIGDET_LOSS_PWRDN_INSTEAD_RXSTR */
  /*600*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_IGNORE_DSP_LOCK_LOSS */
  /*601*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_IGNORE_DSP_SIGDET_LOSS */
  /*602*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_IGNORE_TX_READY_LOSS */
  /*603*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_NO_DSP_LOCK_PWRDN_INSTEAD_RXSTR */
  /*604*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_RXSTR_PU_PLL_RX_VALUE */
  /*605*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_TX_READY_LOSS_PWRDN_INSTEAD_TXSTR */
  /*606*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_TXRX_ON_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*607*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_TXRX_SD_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*608*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_TXSTR_PU_PLL_TX_VALUE */
  /*609*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_NORM_EN_MX_S_OW */
  /*610*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_NORM_EN_MX_S_OW_VAL */
  /*611*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_PWRUP_EN_MX_S_OW */
  /*612*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_PWRUP_EN_MX_S_OW_VAL */
  /*613*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXON_EN_MX_S_OW */
  /*614*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXON_EN_MX_S_OW_VAL */
  /*615*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXSD_EN_MX_S_OW */
  /*616*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_RXSD_EN_MX_S_OW_VAL */
  /*617*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXON_EN_MX_S_OW */
  /*618*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXON_EN_MX_S_OW_VAL */
  /*619*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXON_EN_MX_S_OW */
  /*620*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXON_EN_MX_S_OW_VAL */
  /*621*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXSD_EN_MX_S_OW */
  /*622*/  {/*baseAddr*/ 0x10D4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_PM_PROG_PWM_TXRXSD_EN_MX_S_OW_VAL */

    /* LANE_CONTROL_7 */
  /*623*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_PCS_RX_CLK_ENA_OW */
  /*624*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PCS_RX_CLK_ENA_OW_VAL */
  /*625*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_PCS_TX_CLK_ENA_OW */
  /*626*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PM_PCS_TX_CLK_ENA_OW_VAL */
  /*627*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_RX_RESETN_OW */
  /*628*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_RX_RESETN_OW_VAL */
  /*629*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_TX_RESETN_OW */
  /*630*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_PCS_SD_TX_RESETN_OW_VAL */
  /*631*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_AN_PCS_CLKOUT_SEL_OW */
  /*632*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_AN_PCS_CLKOUT_SEL_OW_VAL */
  /*633*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_OW */
  /*634*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_OW_VAL */
  /*635*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_REG_INVERT_SD_RX_IN_S */
  /*636*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_REG_INVERT_SD_TX_OUT_S */
  /*637*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 2 }, /*P0_REG_SIGDET_MODE */
  /*638*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_RG_PWRDN_RDY_S */
  /*639*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_RG_SEL_LOS_SIG_S */
  /*640*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_TX_BUSY_NO_PWRDN */
  /*641*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE_HW_CLR */
  /*642*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_COMPLETE_LATCH */
  /*643*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_COMPLETE_LATCH */
  /*644*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_FAILED_LATCH */
  /*645*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_FAILED_LATCH */
  /*646*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_REG_AN_RESTART_CNT_EN */
  /*647*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_REG_DSP_LOCK_FAIL_CNT_EN */
  /*648*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_REG_LINK_FAIL_CNT_EN */
  /*649*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_RG_PROG_ENABLE_S */
  /*650*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_RG_FORCE_PG_START_S */
  /*651*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_CMEM_MASK */
  /*652*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_INT_VALID_OW */
  /*653*/  {/*baseAddr*/ 0x10D8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_INT_ENABLED_OW */

    /* LANE_CONTROL_8 */
  /*654*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_CLEAN_OW */
  /*655*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PLL_READY_RX_CLEAN_OW_VAL */
  /*656*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_CLEAN_OW */
  /*657*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PLL_READY_TX_CLEAN_OW_VAL */
  /*658*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_REG_LINK_SQ_DETECTED_MASK_S */
  /*659*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_NORM_EN_S */
  /*660*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_RXSTR_ABORT_RX_TRAIN_S */
  /*661*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_REG_PROG_PWRUP_EN_S */
  /*662*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_REG_PROG_RXON_EN_S */
  /*663*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_REG_PROG_RXSD_EN_S */
  /*664*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_REG_PROG_TXON_EN_S */
  /*665*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_REG_PROG_TXRXON_EN_S */
  /*666*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_REG_PROG_TXRXSD_EN_S */
  /*667*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_CNT_CLEAR_ON_SD_RST */
  /*668*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_DURATION_CLEAR_ON_SD_RST */
  /*669*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_RX_INIT_OK_CNT_CLEAR */
  /*670*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_RX_INIT_TIMEOUT_CLEAR */
  /*671*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_FAILED_CNT_CLEAR */
  /*672*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_OK_CNT_CLEAR */
  /*673*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_RX_TRAIN_TIMEOUT_CNT_CLEAR */
  /*674*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_FAILED_CNT_CLEAR */
  /*675*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_OK_CNT_CLEAR */
  /*676*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_TX_TRAIN_TIMEOUT_CNT_CLEAR */
  /*677*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_REG_RXSTR_ABORT_RX_INIT_S */
  /*678*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_DSP_RXSTR_ACK_S_OW */
  /*679*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_DSP_RXSTR_ACK_S_OW_VAL */
  /*680*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_DSP_TXSTR_ACK_S_OW */
  /*681*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_DSP_TXSTR_ACK_S_OW_VAL */
  /*682*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_REG_DFE_ADAPTATION_EN_S */
  /*683*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_REG_DFE_AUTO_CTRL_S */
  /*684*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_REG_DFE_FROZEN_S */
  /*685*/  {/*baseAddr*/ 0x10DC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_NORM_ST_SQ_DETECTED_MASK_S */

    /* LANE_CONTROL_9 */
  /*686*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_TRAIN_TYPE_OW */
  /*687*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*P0_PM_TRAIN_TYPE_OW_VAL */
  /*688*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RG_TXON_MAX_TIMER_INF */
  /*689*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_RXON_MAX_TIMER_INF */
  /*690*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_EN_RX_INIT_S */
  /*691*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_PROG_EN_RX_TRAIN_S */
  /*692*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RX_BUSY_NO_PWRDN */
  /*693*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_REG_PROG_EN_TX_TRAIN_S */
  /*694*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE_FIN_S_OW */
  /*695*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE_FIN_S_OW_VAL */
  /*696*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_REG_RX_AUTO_RE_TRAIN_S */
  /*697*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_REG_SEL_INIT_DONE_S */
  /*698*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_REG_TX_AUTO_RE_TRAIN_S */
  /*699*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_COMPHY_INT_ACK_MX_S_OW */
  /*700*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_COMPHY_INT_ACK_MX_S_OW_VAL */
  /*701*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_PROG_PWM_DONE_S_OW */
  /*702*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_PROG_PWM_DONE_S_OW_VAL */
  /*703*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_PROG_RX_DONE_S_OW */
  /*704*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_PROG_RX_DONE_S_OW_VAL */
  /*705*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_PROG_TX_DONE_S_OW */
  /*706*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_PROG_TX_DONE_S_OW_VAL */
  /*707*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_OPMODE_MATCH_MX_S_OW */
  /*708*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_OPMODE_MATCH_MX_S_OW_VAL */
  /*709*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_COUPLE_MATCH_MX_S_OW */
  /*710*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_COUPLE_MATCH_MX_S_OW_VAL */
  /*711*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_AP_MATCH_MX_S_OW */
  /*712*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_AP_MATCH_MX_S_OW_VAL */
  /*713*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_PG_EN_MX_S_OW */
  /*714*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_PG_EN_MX_S_OW_VAL */
  /*715*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_INT_DATA_OW */
  /*716*/  {/*baseAddr*/ 0x10E0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_INT_CODE_OW */

    /* LANE_CONTROL_10 */
  /*717*/  {/*baseAddr*/ 0x10E4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_INT_ENABLED_OW_VAL */
  /*718*/  {/*baseAddr*/ 0x10E4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_VALID_OW_VAL */
  /*719*/  {/*baseAddr*/ 0x10E4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 8 }, /*P0_INT_CODE_OW_VAL */
  /*720*/  {/*baseAddr*/ 0x10E4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 22 }, /*P0_INT_DATA_OW_VAL */

    /* LANE_CONTROL_11 */
  /*721*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PM_PU_RX_REQ_S_OW */
  /*722*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_PM_PU_RX_REQ_S_OW_VAL */
  /*723*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_PM_PU_TX_REQ_S_OW */
  /*724*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_PM_PU_TX_REQ_S_OW_VAL */
  /*725*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PM_ST_NORMAL_S_OW */
  /*726*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_PM_ST_NORMAL_S_OW_VAL */
  /*727*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_PM_ST_PWRDN_S_OW */
  /*728*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_PM_ST_PWRDN_S_OW_VAL */
  /*729*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PM_TX_IDLE_S_OW */
  /*730*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PM_TX_IDLE_S_OW_VAL */
  /*731*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_RESET_CORE_TX_ACK_ENABLE */
  /*732*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_RESET_CORE_RX_ACK_ENABLE */
  /*733*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_RESET_TX_CORE_ENABLE */
  /*734*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_RESET_RX_CORE_ENABLE */
  /*735*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_RESET_AN_TX_GEAR_BOX */
  /*736*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_RESET_AN_RX_GEAR_BOX */
  /*737*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_TXT_DSP_SIGDET_LOSS_PWRDN_INSTEAD_RXSTR */
  /*738*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_TXT_IGNORE_DSP_LOCK_LOSS */
  /*739*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_TXT_IGNORE_DSP_SIGDET_LOSS */
  /*740*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_TXT_IGNORE_TX_READY_LOSS */
  /*741*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_TXT_NO_DSP_LOCK_PWRDN_INSTEAD_RXSTR */
  /*742*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_TXT_RXSTR_PU_PLL_RX_VALUE */
  /*743*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_TXT_TX_READY_LOSS_PWRDN_INSTEAD_TXSTR */
  /*744*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_TXT_TXRX_ON_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*745*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_TXT_TXRX_SD_TIMEOUT_PWRDN_INSTEAD_RXSTR */
  /*746*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_TXT_TXSTR_PU_PLL_TX_VALUE */
  /*747*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 2 }, /*P0_REG_TX_TRAIN_DSP_SIGDET_SEL_S */
  /*748*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_TX_TRAIN_S */
  /*749*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_RX_TRAIN_S */
  /*750*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_RX_INIT_S */
  /*751*/  {/*baseAddr*/ 0x10E8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_RESET_EN_PU_PLL_S */

    /* LANE_CONTROL_12 */
  /*752*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_PU_RX_BOTH_IN_IDLE */
  /*753*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_REG_PM_SD_PU_RESET_ON_SFTRST_S */
  /*754*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RX_INIT_S */
  /*755*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RX_RESTR_S */
  /*756*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RX_TRAIN_S */
  /*757*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TX_RESTR_S */
  /*758*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TX_TRAIN_S */
  /*759*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_NORM_S */
  /*760*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_PWRUP_S */
  /*761*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RXON_S */
  /*762*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_RXSD_S */
  /*763*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TXON_S */
  /*764*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TXRXON_S */
  /*765*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_REG_PROG_MASK_TXRXSD_S */
  /*766*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_PU_TX_BOTH_IN_IDLE */
  /*767*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_SD_SW_RESETN_OW */
  /*768*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_SD_SW_RESETN_OW_VAL */
  /*769*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_PM_CLOCKOUT_GATER_OW */
  /*770*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_PM_CLOCKOUT_GATER_OW_VAL */
  /*771*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_RG_WAIT_PWRDN_MAX_TIMER_INF */
  /*772*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR_OW */
  /*773*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_RG_MODE_100GR_OW_VAL */
  /*774*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR2_OW */
  /*775*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_RG_MODE_200GR2_OW_VAL */
  /*776*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_RG_MODE_400GR4_OW */
  /*777*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_RG_MODE_400GR4_OW_VAL */
  /*778*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_RG_MODE_800GR8_OW */
  /*779*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_RG_MODE_800GR8_OW_VAL */
  /*780*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_RG_MODE_12GR1_OW */
  /*781*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_RG_MODE_12GR1_OW_VAL */
  /*782*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_RG_MODE_53GR1_OW */
  /*783*/  {/*baseAddr*/ 0x10EC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_RG_MODE_53GR1_OW_VAL */

    /* LANE_CONTROL_13 */
  /*784*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_AN_HW_SOFT_RESET_ */
  /*785*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_AN_SW_SOFT_RESET_ */
  /*786*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_RG_MODE_106GR1_OW */
  /*787*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_RG_MODE_106GR1_OW_VAL */
  /*788*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_RG_MODE_106GR2_OW */
  /*789*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_RG_MODE_106GR2_OW_VAL */
  /*790*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_RG_MODE_212GR4_OW */
  /*791*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_RG_MODE_212GR4_OW_VAL */
  /*792*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RG_MODE_424GR8_OW */
  /*793*/  {/*baseAddr*/ 0x10F0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_RG_MODE_424GR8_OW_VAL */

    /* LANE_CONTROL_14 */
  /*794*/  {/*baseAddr*/ 0x10F4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_PM_SD_PHY_GEN_TX_OW_VAL */
  /*795*/  {/*baseAddr*/ 0x10F4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*P0_PM_SD_PHY_GEN_RX_OW_VAL */
  /*796*/  {/*baseAddr*/ 0x10F4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*P0_CUSTOM_PHY_GEN */

    /* LANE_CONTROL_15 */
  /*797*/  {/*baseAddr*/ 0x10F8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_SD_PHY_GEN_OW_VAL */

    /* LANE_STATUS_1 */
  /*798*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_COMPLETE */
  /*799*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_COMPLETE */
  /*800*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_REG_TX_TRAIN_FAILED */
  /*801*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_REG_RX_TRAIN_FAILED */
  /*802*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_PCS_CFG_DONE */
  /*803*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_REG_PROG_PWM_DONE_S */
  /*804*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_REG_PROG_RX_DONE_S */
  /*805*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_REG_PROG_TX_DONE_S */
  /*806*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_PIN_RXDCLK_4X_EN_VAL */
  /*807*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_PIN_TXDCLK_4X_EN_VAL */
  /*808*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TXDATA_GRAY_CODE_EN */
  /*809*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_RXDATA_GRAY_CODE_EN */
  /*810*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TXDATA_PRE_CODE_EN */
  /*811*/  {/*baseAddr*/ 0x10FC,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_RXDATA_PRE_CODE_EN */

    /* LANE_STATUS_2 */
  /*812*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_DSP_RXSTR_REQ_MX_S */
  /*813*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_STAT_DSP_RXSTR_REQ_S */
  /*814*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_STAT_DSP_TXSTR_REQ_MX_S */
  /*815*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_DSP_TXSTR_REQ_S */
  /*816*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_EN_HCD_RESOLVED */
  /*817*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_HCD_CLEAR */
  /*818*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_HCD_RESOLVED */
  /*819*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_PCS_CLKOUT_SEL */
  /*820*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_PCS_SEL */
  /*821*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_PM_AN_RESTART */
  /*822*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_EN_S */
  /*823*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_MODE_S */
  /*824*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_RESET_RX_S */
  /*825*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_STAT_PM_AP_RESET_TX_S */
  /*826*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_STAT_PM_ENCLK_AP_FR_S */
  /*827*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_STAT_PM_ENCLK_AP_FT_S */
  /*828*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_STAT_PM_ENCLK_AP_SYS_S */
  /*829*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_100GR2 */
  /*830*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_100GR4 */
  /*831*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_107GR2 */
  /*832*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_10G */
  /*833*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_110GR4 */
  /*834*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_1G */
  /*835*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_200GR4 */
  /*836*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_200GR8 */
  /*837*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_25G */
  /*838*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_2P5G */
  /*839*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_400GR8 */
  /*840*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_40GR2 */
  /*841*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_40GR4 */
  /*842*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_428GR8 */
  /*843*/  {/*baseAddr*/ 0x1100,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_50GR */

    /* LANE_STATUS_3 */
  /*844*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_50GR2 */
  /*845*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_5G */
  /*846*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_CUSTOM */
  /*847*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_QSGMII */
  /*848*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USGMII */
  /*849*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX10G */
  /*850*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX20G */
  /*851*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX2P5G */
  /*852*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_USX5G */
  /*853*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_PM_NORM_X_STATE_S */
  /*854*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_STAT_PM_NR_RESET_TX_S */
  /*855*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_COUPLE_S */
  /*856*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_RX_CLK_ENA */
  /*857*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_SD_RX_RESET_ */
  /*858*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_SD_TX_RESET_ */
  /*859*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_STAT_PM_PCS_TX_CLK_ENA */
  /*860*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_STAT_PM_PWRDN_S */
  /*861*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_COUPLE_MODE_EN_S */
  /*862*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_PU_PLL_S */
  /*863*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_PU_RX_S */
  /*864*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_100GR */
  /*865*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_200GR2 */
  /*866*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_400GR4 */
  /*867*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_800GR8 */
  /*868*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_12GR1 */
  /*869*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_53GR1 */
  /*870*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_106GR1 */
  /*871*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_106GR2 */
  /*872*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_212GR4 */
  /*873*/  {/*baseAddr*/ 0x1104,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_STAT_PM_MODE_424GR8 */

    /* LANE_STATUS_4 */
  /*874*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_PU_TX_S */
  /*875*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_SOFTRST_S */
  /*876*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TXCLK_SYNC_EN_PLL_S */
  /*877*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_PM_SOFTRST_S */
  /*878*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_PM_ST_EN_S */
  /*879*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 2 }, /*P0_STAT_PM_TX_TRAIN_POLY_SEL_S */
  /*880*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_DSP_LOCK_S */
  /*881*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_STAT_DSP_PWRDN_ACK_S */
  /*882*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*P0_STAT_DSP_RXDN_ACK_S */
  /*883*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  12, /*fieldLen*/ 1 }, /*P0_STAT_DSP_SIGDET_S */
  /*884*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 1 }, /*P0_STAT_DSP_TXDN_ACK_S */
  /*885*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  14, /*fieldLen*/ 1 }, /*P0_STAT_PCS_LOCK_S */
  /*886*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_STAT_TX_READY_S */
  /*887*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_RX_CLEAN_S */
  /*888*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_RX_S */
  /*889*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_TX_CLEAN_S */
  /*890*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_STAT_PLL_READY_TX_S */
  /*891*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_LOCK_S */
  /*892*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_RXDN_ACK_S */
  /*893*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_RXSTR_ACK_S */
  /*894*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_SIGDET_S */
  /*895*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  24, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_TXDN_ACK_S */
  /*896*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  25, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_TX_READY_S */
  /*897*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 1 }, /*P0_STAT_PM_DSP_TXSTR_ACK_S */
  /*898*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  27, /*fieldLen*/ 1 }, /*P0_STAT_PM_RX_INIT_S */
  /*899*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  28, /*fieldLen*/ 1 }, /*P0_STAT_PM_RX_TRAIN_ENABLE_S */
  /*900*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  29, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_DFE_EN_S */
  /*901*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_DFE_PAT_DIS_S */
  /*902*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_DFE_UPDATE_DIS_S */

    /* LANE_STATUS_5 */
  /*903*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_STAT_PM_SD_TX_IDLE_S */
  /*904*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*P0_STAT_PM_TRAIN_TYPE_S */
  /*905*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_STAT_PM_TX_TRAIN_ENABLE_S */
  /*906*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_STAT_RX_INIT_DONE_S */
  /*907*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_STAT_RX_TRAIN_COMPLETE_S */
  /*908*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_STAT_RX_TRAIN_FAILED_S */
  /*909*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_STAT_SD_RX_DTL_CLAMP_S */
  /*910*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_STAT_SD_TXCLK_SYNC_START_OUT_S */
  /*911*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P0_STAT_SQ_DETECTED_LPF_S */
  /*912*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*P0_STAT_TX_TRAIN_COMPLETE_S */
  /*913*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 2 }, /*P0_STAT_TX_TRAIN_ERROR_L */
  /*914*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  13, /*fieldLen*/ 2 }, /*P0_STAT_TX_TRAIN_ERROR_S */
  /*915*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*P0_STAT_TX_TRAIN_FAILED_S */
  /*916*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*P0_STAT_PCS_CFG_DONE_FIN_S */
  /*917*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  17, /*fieldLen*/ 1 }, /*P0_STAT_SD_BUSY_RX_S */
  /*918*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  18, /*fieldLen*/ 1 }, /*P0_STAT_SD_BUSY_TX_S */
  /*919*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*P0_STAT_PM_PU_RX_REQ_S */
  /*920*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 1 }, /*P0_STAT_PM_PU_TX_REQ_S */
  /*921*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*P0_STAT_PM_ST_NORMAL_S */
  /*922*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  22, /*fieldLen*/ 1 }, /*P0_STAT_PM_ST_PWRDN_S */
  /*923*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  23, /*fieldLen*/ 1 }, /*P0_STAT_PM_TX_IDLE_S */
  /*924*/  {/*baseAddr*/ 0x110C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 2 }, /*P0_STAT_CMEM_STATE */

    /* LANE_STATUS_6 */
  /*925*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*P0_STAT_PM_CMEM_ADDR_S */
  /*926*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*P0_STAT_INT_STATE */
  /*927*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 3 }, /*P0_STAT_PROG_STATE_S */
  /*928*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 5 }, /*P0_REG_TX_TFIFO_UW_W */
  /*929*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  16, /*fieldLen*/ 5 }, /*P0_REG_TX_TFIFO_UW_R */
  /*930*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  21, /*fieldLen*/ 5 }, /*P0_REG_RX_TFIFO_UW_W */
  /*931*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  26, /*fieldLen*/ 5 }, /*P0_REG_RX_TFIFO_UW_R */
  /*932*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_STAT_PM_NR_RESET_RX_S */

    /* LANE_STATUS_7 */
  /*933*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_PM_SD_PHY_GEN_RX_S */
  /*934*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*P0_STAT_PM_SD_PHY_GEN_TX_S */
  /*935*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*P0_STAT_SD_PHY_GEN_S */

    /* LANE_COUNTER_1 */
  /*936*/  {/*baseAddr*/ 0x1118,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_RX_INIT_DURATION_L */

    /* LANE_COUNTER_2 */
  /*937*/  {/*baseAddr*/ 0x111C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_RX_TRAIN_DURATION_L */

    /* LANE_COUNTER_3 */
  /*938*/  {/*baseAddr*/ 0x1120,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_TX_TRAIN_DURATION_L */

    /* LANE_COUNTER_4 */
  /*939*/  {/*baseAddr*/ 0x1124,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_RX_INIT_OK_CNT */
  /*940*/  {/*baseAddr*/ 0x1124,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*P0_STAT_RX_INIT_TIMEOUT_CNT */
  /*941*/  {/*baseAddr*/ 0x1124,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*P0_STAT_RX_TRAIN_FAILED_CNT */
  /*942*/  {/*baseAddr*/ 0x1124,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 2 }, /*P0_STAT_RX_TRAIN_DURATION_L_HI */

    /* LANE_COUNTER_5 */
  /*943*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_RX_TRAIN_OK_CNT */
  /*944*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*P0_STAT_RX_TRAIN_TIMEOUT_CNT */
  /*945*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*P0_STAT_TX_TRAIN_FAILED_CNT */
  /*946*/  {/*baseAddr*/ 0x1128,  /*offsetFormula*/ 0x1000, /*fieldStart*/  30, /*fieldLen*/ 2 }, /*P0_STAT_TX_TRAIN_DURATION_L_HI */

    /* LANE_COUNTER_6 */
  /*947*/  {/*baseAddr*/ 0x112C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*P0_STAT_TX_TRAIN_OK_CNT */
  /*948*/  {/*baseAddr*/ 0x112C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  10, /*fieldLen*/ 10 }, /*P0_STAT_TX_TRAIN_TIMEOUT_CNT */
  /*949*/  {/*baseAddr*/ 0x112C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  20, /*fieldLen*/ 10 }, /*P0_PWRUP_CNT_S */

    /* LANE_COUNTER_7 */
  /*950*/  {/*baseAddr*/ 0x1130,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_LINK_UP_DURATION_L */

    /* LANE_DSP_LOCK_FAIL_COUNTER */
  /*951*/  {/*baseAddr*/ 0x1134,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_DSP_LOCK_FAIL_COUTNER */

    /* LANE_LINK_FAIL_COUNTER */
  /*952*/  {/*baseAddr*/ 0x1138,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_LINK_FAIL_COUNTER */

    /* LANE_CMD_LINE_LO */
  /*953*/  {/*baseAddr*/ 0x113C,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*P0_CMD_LINE%T_LO */

    /* LANE_CMD_LINE_HI */
  /*954*/  {/*baseAddr*/ 0x123C,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 20 }, /*P0_CMD_LINE_%T_HI */

    /* LANE_TX_FSM_DEBUG_CONTROL */
  /*955*/  {/*baseAddr*/ 0x133C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_0_FSM_TRIGGER_ENABLE */
  /*956*/  {/*baseAddr*/ 0x133C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 4 }, /*TX_0_FSM_TRIGGER_STATE */

    /* LANE_RX_FSM_DEBUG_CONTROL */
  /*957*/  {/*baseAddr*/ 0x1340,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_0_FSM_TRIGGER_ENABLE */
  /*958*/  {/*baseAddr*/ 0x1340,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 5 }, /*RX_0_FSM_TRIGGER_STATE */

    /* LANE_COUNTER_8 */
  /*959*/  {/*baseAddr*/ 0x1344,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*P0_STAT_HCD_TX_TRAIN_DURATION_L */

    /* LANE_PHYGEN_TO_PLL_NOT_READY_MAX_TIMER */
  /*960*/  {/*baseAddr*/ 0x1348,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_PHYGEN_TO_PLL_NOT_READY_TIME_OUT_S */
  /*961*/  {/*baseAddr*/ 0x1348,  /*offsetFormula*/ 0x1000, /*fieldStart*/  31, /*fieldLen*/ 1 }, /*P0_REG_PHYGEN_TO_PLL_NOT_READY_TIME_OUT_S_INF */

    /* LANE_GRAY_PRE_CODE_TO_PHYGEN_MIN_TIMER */
  /*962*/  {/*baseAddr*/ 0x134C,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*P0_REG_GRAY_PRE_CODE_TO_PHYGEN_MIN_WAIT_S */

};

