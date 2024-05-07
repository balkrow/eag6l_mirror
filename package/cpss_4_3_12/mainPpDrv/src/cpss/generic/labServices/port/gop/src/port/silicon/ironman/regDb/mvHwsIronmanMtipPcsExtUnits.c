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
* @file mvHwsIronmanMtipPcsExtUnits.c
*
* @brief IronmanL PCS EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanPcsExtUnitsDb[] = {
    /* Port<0> Status */
  /*0*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_LINK_STATUS */
  /*1*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS */
  /*2*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_RX_SYNC */
  /*3*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE */
  /*4*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*5*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*CHANNELS_ENA_IND */
  /*6*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*USXG_ENA_IND */
  /*7*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CLEAN */
  /*8*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x4, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CLEAN */

    /* Port<0> Interrupt Cause */
  /*9*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*10*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CHANGE */
  /*11*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CHANGE */
  /*12*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_HI_BER_INT */
  /*13*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CLEAN_CHANGE */
  /*14*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CLEAN_CHANGE */
  /*15*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE_INT */

    /* Port<0> Interrupt Mask */
  /*16*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 6 }, /*P0_INTERRUPT_MASK */

    /* Global Last Violation */
  /*17*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Global Interrupt Mask */
  /*18*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*GLOBAL_INTERRUPT_MASK */

    /* Global Interrupt Cause */
  /*19*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*20*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*21*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ABU_BAD_ADDRESS_ERROR */
  /*22*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*ABU_WATCHDOG_ERROR */

    /* Global Interrupt Summary Cause */
  /*23*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*24*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*25*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*26*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*27*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*28*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*29*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*30*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*31*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*32*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*33*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SD0_100FX_INT_SUM */
  /*34*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SD1_100FX_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*35*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 11 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Global PCS Metal Fix */
  /*36*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* Global Status */
  /*37*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK_L_0 */
  /*38*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ALIGN_LOCK_0 */
  /*39*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ALIGN_ERR_IND_0 */
  /*40*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK_L_1 */
  /*41*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*ALIGN_LOCK_1 */
  /*42*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ALIGN_ERR_IND_1 */

    /* Global Control */
  /*43*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SD0_N2 */
  /*44*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SD1_N2 */
  /*45*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BER_TIMER_SHORT */
  /*46*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG */
  /*47*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*CYC_TO_STRETCH_MAC2REG */
  /*48*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 10 }, /*APB_WATCHDOG_TIMER_MAX_VAL */

    /* Global PCS Clock and Reset Control */
  /*49*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SD0_TX_RESET_ */
  /*50*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SD0_RX_RESET_ */
  /*51*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SD1_TX_RESET_ */
  /*52*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SD1_RX_RESET_ */
  /*53*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PCS_100FX_SD0_TX_RESET_ */
  /*54*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PCS_100FX_SD0_RX_RESET_ */
  /*55*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PCS_100FX_SD1_TX_RESET_ */
  /*56*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PCS_100FX_SD1_RX_RESET_ */
  /*57*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SD0_TX_CLK_EN */
  /*58*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SD0_RX_CLK_EN */
  /*59*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SD1_TX_CLK_EN */
  /*60*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SD1_RX_CLK_EN */
  /*61*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PCS_100FX_SD0_TX_CLK_EN */
  /*62*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*PCS_100FX_SD0_RX_CLK_EN */
  /*63*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*PCS_100FX_SD1_TX_CLK_EN */
  /*64*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PCS_100FX_SD1_RX_CLK_EN */
  /*65*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*PCS_SG_MAC_CLK_RESET_ */
  /*66*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PCS_SG_MAC_CLK_EN */
  /*67*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PCS_MAC_CLK_RESET_ */
  /*68*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PCS_MAC_CLK_EN */
  /*69*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PCS_100FX_CDR_SD0_RX_RESET_ */
  /*70*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*PCS_100FX_CDR_SD1_RX_RESET_ */

    /* Port<0> Control */
  /*71*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_STATUS_EN */
  /*72*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_FORCE_LINK_STATUS_DIS */
  /*73*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_FORCE_LPCS_LINK_STATUS_EN */
  /*74*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_FORCE_LPCS_LINK_STATUS_DIS */
  /*75*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_HD_ENA */
  /*76*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_100FX_MODE */
  /*77*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_LOOPBACK_EN */
  /*78*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x4, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P0_AN_LINK_FIX */

    /* Global Status2 */
  /*79*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*SD_BIT_SLIP_0 */
  /*80*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 6 }, /*SD_BIT_SLIP_1 */

    /* PCS_100FX<0> 100FX RX Control2 */
  /*81*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 18 }, /*SD0_STABILIZE_TIMER */
  /*82*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*SD0_TX_TFIFO_W_UPD */
  /*83*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*SD0_TX_TFIFO_R_UPD */
  /*84*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 22, /*fieldLen*/ 3 }, /*SD0_CDR_SPEED */
  /*85*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*SD0_START_ON_EVEN */
  /*86*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*SD0_ERR_START_MISALIGN */
  /*87*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*SD0_ERR_END_MISALIGN */
  /*88*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*SD0_MISSING_NIBBLE_DV_ON */
  /*89*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*SD0_SFD_MISALIGN_FIX */
  /*90*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x30, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*SD0_SMD_MISALIGN_FIX */

    /* PCS_100FX<0> Interrupt Mask */
  /*91*/  {/*baseAddr*/ 0xe0,  /*offsetFormula*/ 0x30, /*fieldStart*/  1, /*fieldLen*/ 13 }, /*SD0_INTERRUPT_MASK */

    /* PCS_100FX<0> 100FX Status2 */
  /*92*/  {/*baseAddr*/ 0xd8,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SD0_RESERVED_OUT */

    /* PCS_100FX<0> Interrupt Cause */
  /*93*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SD0_INTERRUPT_CAUSE_INT_SUM */
  /*94*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SD0_100FX_PCS_LINK_STATUS_CHANGE */
  /*95*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SD0_100FX_PCS_LINK_STATUS_CLEAN_CHANGE */
  /*96*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SD0_RX_ASYNC_FIFO_NOT_READY */
  /*97*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SD0_TX_TFIFO_FULL */
  /*98*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SD0_TX_TFIFO_EMPTY */
  /*99*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SD0_TX_TFIFO_R_ERR */
  /*100*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SD0_TX_TFIFO_W_ERR */
  /*101*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SD0_FE_FAULT */
  /*102*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SD0_ALIGNED_ON_FIRST_START_ */
  /*103*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SD0_START_MISALIGNMENT */
  /*104*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SD0_END_MISALIGNMENT */
  /*105*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SD0_FALSE_CARRIER_DETECTED */
  /*106*/  {/*baseAddr*/ 0xdc,  /*offsetFormula*/ 0x30, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SD0_LPI_DETECTED */

    /* PCS_100FX<0> 100FX RX Control1 */
  /*107*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*SD0_RX_NRZI_OPTION */
  /*108*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SD0_RX_INVERT */
  /*109*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SD0_RXERR_STAT_ENA */
  /*110*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SD0_CONFIRM_K_ERR_NO_DET */
  /*111*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SD0_DIS_SFD_ALIGN */
  /*112*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SD0_DIS_SMD_ALIGN */
  /*113*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  7, /*fieldLen*/ 2 }, /*SD0_CLKENA_DLY */
  /*114*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SD0_FORCE_RX_MII_OFF */
  /*115*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SD0_MASK_RX_MII */
  /*116*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SD0_IDLE_CHK_VLD */
  /*117*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SD0_IDLE_FLASE_CRS */
  /*118*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SD0_IDLE_ERR */
  /*119*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*SD0_NO_ESD1_STATE */
  /*120*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SD0_NO_ESD2_STATE */
  /*121*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SD0_NO_PRE_MAT_STATE */
  /*122*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 17, /*fieldLen*/ 2 }, /*SD0_RX_LL_OPTION */
  /*123*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*SD0_RX_FORCE_LINK_UP */
  /*124*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*SD0_RX_FORCE_LINK_DOWN */
  /*125*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*SD0_RX_FORCE_FAULT */
  /*126*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*SD0_FEFD_COUNT_NO_SIGDET */
  /*127*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*SD0_RX_FEFD_DIS */
  /*128*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*SD0_FEFD_CLEAR_ON_FAIL */
  /*129*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*SD0_OLD_FEFD */
  /*130*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*SD0_EVEN_ODD_SAWP */
  /*131*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*SD0_HALF_DUPLEX */
  /*132*/  {/*baseAddr*/ 0xc4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*100FX_CDR_TEST_MODE */

    /* PCS_100FX<0> 100FX TX Control */
  /*133*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*SD0_TX_NRZI_OPTION */
  /*134*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*SD0_TX_INVERT */
  /*135*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SD0_COL_TEST */
  /*136*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SD0_COL_BY_XMT */
  /*137*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SD0_USE_SYNCED_CRS */
  /*138*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SD0_TEST_ENA */
  /*139*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SD0_TX_FORCE_LINK_UP */
  /*140*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SD0_TX_FORCE_LINK_DOWN */
  /*141*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SD0_TEST_16_112_SEL */
  /*142*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SD0_FEFG_DIS */
  /*143*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SD0_ZERO_84_GEN */
  /*144*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SD0_FORCE_TX_FEF */
  /*145*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SD0_FORCE_ONFLY_DIS */
  /*146*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*SD0_IGNORE_LOAD */
  /*147*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SD0_NO_IDLE_REPLACE */
  /*148*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SD0_CLEAR_MASK_ON_EMPTY */
  /*149*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 17, /*fieldLen*/ 4 }, /*SD0_RD_THR */
  /*150*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*SD0_ERR_BYTE_TO_NIBBLE */
  /*151*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 22, /*fieldLen*/ 2 }, /*SD0_TX_LL_OPTION */
  /*152*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*SD0_FORCE_100FX_PCS_LINK_STATUS_EN */
  /*153*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*SD0_FORCE_100FX_PCS_LINK_STATUS_DIS */
  /*154*/  {/*baseAddr*/ 0xc0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*SD0_TX_START_ON_EVEN */

    /* PCS_100FX<0> 100FX Status0 */
  /*155*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*SD0_RX_FASTER_COUNTER */
  /*156*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x30, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*SD0_RX_SLOWER_COUNTER */
  /*157*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 16, /*fieldLen*/ 6 }, /*SD0_BIT_EDGE */
  /*158*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*P0_100FX_PCS_LINK_STATUS */
  /*159*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*P0_100FX_PCS_LINK_STATUS_CLEAN */
  /*160*/  {/*baseAddr*/ 0xd0,  /*offsetFormula*/ 0x30, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*100FX_CDR_TEST_RESULT */

    /* PCS_100FX<0> 100FX RX Control3 */
  /*161*/  {/*baseAddr*/ 0xcc,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SD0_RESERVED_INPUT */

    /* PCS_100FX<0> 100FX Status1 */
  /*162*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*SD0_TX_ASYNC_FIFO_WR_FILL_LEVEL */
  /*163*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*SD0_RX_ASYNC_FIFO_WR_FILL_LEVEL */
  /*164*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/  8, /*fieldLen*/ 5 }, /*SD0_TX_TFIFO_WRITE_FL */
  /*165*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SD0_RXERROR_STATUS */
  /*166*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*SD0_CARRIER_STATUS */
  /*167*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SD0_RCVING */
  /*168*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SD0_FALSE_CRS */
  /*169*/  {/*baseAddr*/ 0xd4,  /*offsetFormula*/ 0x30, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*SD0_FE_FAULT_INDICATION */
};

