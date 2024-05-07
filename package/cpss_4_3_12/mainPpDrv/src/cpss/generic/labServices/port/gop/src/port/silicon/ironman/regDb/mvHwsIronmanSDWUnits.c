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
* @file mvHwsIronmanSDWUnits.c
*
* @brief IronmanL SerDes register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanSdwUnitsDb[] = {
    /** SDW common units fields */
    /* General Control 0 */
  /*0*/   {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PU_IVREF */
  /*1*/   {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SIF_SEL */
  /*2*/   {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ASYNC_RESET */
  /*3*/   {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*REFCLK_DIS */
  /*4*/   {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 8 }, /*CID_REV */
  /*5*/   {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RESERVED_INPUT */

    /* General Control 1 */
  /*6*/   {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*RCVRD_CLK0_DIV */
  /*7*/   {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*RCVRD_CLK1_DIV */
  /*8*/   {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SYNC_E0_MUX_SEL */
  /*9*/   {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SYNC_E1_MUX_SEL */
  /*10*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*SYNC_E0_LANE_SEL */
  /*11*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 2 }, /*SYNC_E1_LANE_SEL */
  /*12*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*RCVRD_CLK0_AUTO_MASK_EN */
  /*13*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RCVRD_CLK1_AUTO_MASK_EN */
  /*14*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*RCVRD_CLK0_LOCAL_CLK_DISABLED */
  /*15*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*RCVRD_CLK1_LOCAL_CLK_DISABLED */
  /*16*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*RCVRD_CLK0_LOCAL_CLK_DIV_RESET_ */
  /*17*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*RCVRD_CLK1_LOCAL_CLK_DIV_RESET_ */
  /*18*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TX_CLK_ON_RCVRD_CLK0 */
  /*19*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TX_CLK_ON_RCVRD_CLK1 */

    /* General Mux Control */
  /*20*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PU_IVREF_SELECT */
  /*21*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ASYNC_RESET_SELECT */
  /*22*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*CID_REV_SELECT */
  /*23*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RESERVED_INPUT_SELECT */

    /* Watchdog Timer Control */
  /*24*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 31 }, /*WATCHDOG_TIMER_VALUE */
  /*25*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*WATCHDOG_TIMER_ENABLE */

    /* SDW Metal Fix Register */
  /*26*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*METALFIXBITS */

    /* General Status 0 */
  /*27*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RESERVED_OUTPUT */
  /*28*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*REFCLK_DIS_ACK */

    /* SDW Interrupt Cause */
  /*29*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SUMMARY */
  /*30*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SERDES_INTERRUPT */
  /*31*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*32*/  {/*baseAddr*/ 0x200,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SQ_DETECT_0 */

    /* SDW Interrupt Mask */
  /*33*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*INTERRUPT_MASK */

    /* SDW Invalid Address Latch */
  /*34*/  {/*baseAddr*/ 0x208,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*INVALID_ADDR_LATCH */

  /** SDW lane units fields */
  /* Lane lane Control 0 */
  /*35*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PU_PLL */
  /*36*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PU_RX */
  /*37*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PU_TX */
  /*38*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 4 }, /*PHY_GEN_RX */
  /*39*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*PHY_GEN_TX */
  /*40*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*TX_IDLE */
  /*41*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*RX_INIT */
  /*42*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 5 }, /*REF_FREF_SEL */
  /*43*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*DFE_UPDATE_DIS */
  /*44*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*DFE_PAT_DIS */
  /*45*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*DFE_EN */
  /*46*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*BACKCHANNEL_WINDOW */
  /*47*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*SQ_DETECT_MASK */
  /*48*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*SQ_DETECT_ACTIVE_LOW */
  /*49*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*SW_RESET_TO_PCS_MASK */
  /*50*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*REFCLK_SEL */
  /*51*/  {/*baseAddr*/ 0x500,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*RX_DC_TERM_EN */

    /* Lane lane Control 1 */
  /*52*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SSC_EN */
  /*53*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SYNC_DET_EN */
  /*54*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RESET_CORE */
  /*55*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RF_RESET_IN */
  /*56*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TXCLK_SYNC_EN_PLL_IN */
  /*57*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_TRAIN_ENABLE */
  /*58*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*TX_TRAIN_ENABLE */
  /*59*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_TRAIN_FRAME_LOCK_ENABLE */
  /*60*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LOCAL_CTRL_FIELD_READY */
  /*61*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TXDATA_SAMP_SEL */
  /*62*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*RXDATA_SAMP_SEL */
  /*63*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*RX_NEGEDGE_SAMPLE_EN */
  /*64*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*RX_CLK_ENABLE */
  /*65*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*TX_CLK_ENABLE */
  /*66*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*RX_CLK_4X_ENABLE */
  /*67*/  {/*baseAddr*/ 0x504,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*TX_CLK_4X_ENABLE */

    /* Lane lane Control 2 */
  /*68*/  {/*baseAddr*/ 0x508,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REMOTE_STATUS_FIELD_VALID */
  /*69*/  {/*baseAddr*/ 0x508,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 6 }, /*REMOTE_STATUS_FIELD */
  /*70*/  {/*baseAddr*/ 0x508,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*REMOTE_CTRL_FIELD_VALID */
  /*71*/  {/*baseAddr*/ 0x508,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*REMOTE_CTRL_FIELD */
  /*72*/  {/*baseAddr*/ 0x508,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 4 }, /*REMOTE_CTRL_FIELD_RESET */

    /* Lane lane Control 3 */
  /*73*/  {/*baseAddr*/ 0x50c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RESERVED_INPUT_TX */
  /*74*/  {/*baseAddr*/ 0x50c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RESERVED_INPUT_RX */

    /* Lane lane Mux Control */
  /*75*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PU_PLL_SELECT */
  /*76*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PU_RX_SELECT */
  /*77*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PU_TX_SELECT */
  /*78*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PHY_GEN_RX_SELECT */
  /*79*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_GEN_TX_SELECT */
  /*80*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RX_INIT_SELECT */
  /*81*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TX_IDLE_SELECT */
  /*82*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*COUPLE_MODE_EN_SELECT */
  /*83*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TXCLK_SYNC_EN_PLL_IN_SELECT */
  /*84*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RX_TRAIN_ENABLE_SELECT */
  /*85*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*TX_TRAIN_ENABLE_SELECT */
  /*86*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*RESET_CORE_SELECT */
  /*87*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RESERVED_INPUT_RX_SELECT */
  /*88*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RESERVED_INPUT_TX_SELECT */
  /*89*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*RESERVE_ANP_IN_RX_SELECT */
  /*90*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESERVE_ANP_IN_TX_SELECT */
  /*91*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*DFE_EN_SELECT */
  /*92*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*DFE_PAT_DIS_SELECT */
  /*93*/  {/*baseAddr*/ 0x510,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*DFE_UPDATE_DIS_SELECT */

    /* Lane lane Fix Register */
  /*94*/  {/*baseAddr*/ 0x514,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PRBS_PATTERN_ENABLE */

    /* Lane lane Status 0 */
  /*95*/  {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SYNC_FOUND */
  /*96*/  {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SQ_DETECTED */
  /*97*/  {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PLL_READY_TX */
  /*98*/  {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PLL_READY_RX */
  /*99*/  {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_INIT_DONE */
  /*100*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REALIGN_SYNC_FOUND */
  /*101*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SQ_DETECTED_LPF */
  /*102*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*CDR_LOCK */
  /*103*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TXCLK_SYNC_START_OUT */
  /*104*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RX_TRAIN_COMPLETE */
  /*105*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_TRAIN_FAILED */
  /*106*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_TRAIN_COMPLETE */
  /*107*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*TX_TRAIN_FAILED */
  /*108*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*TX_TRAIN_FRAME_LOCK_DETECTED */
  /*109*/ {/*baseAddr*/ 0x600,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*TX_TRAIN_ERROR */

    /* Lane lane Status 1 */
  /*110*/ {/*baseAddr*/ 0x604,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REMOTE_CTRL_FIELD_READY */
  /*111*/ {/*baseAddr*/ 0x604,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LOCAL_STATUS_FIELD_VALID */
  /*112*/ {/*baseAddr*/ 0x604,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 6 }, /*LOCAL_STATUS_FIELD */
  /*113*/ {/*baseAddr*/ 0x604,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LOCAL_CTRL_FIELD_VALID */
  /*114*/ {/*baseAddr*/ 0x604,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 4 }, /*LOCAL_CTRL_FIELD_RESET */
  /*115*/ {/*baseAddr*/ 0x604,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 6 }, /*LOCAL_CTRL_FIELD */

    /* Lane lane Status 2 */
  /*116*/ {/*baseAddr*/ 0x608,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RESERVED_OUTPUT_TX */
  /*117*/ {/*baseAddr*/ 0x608,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RESERVED_OUTPUT_RX */
};

