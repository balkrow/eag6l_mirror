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
* @file mvHwsAasCpuPcsExtUnits.c
*
* @brief AAS CPU PCS EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasCpuPcsExtDb[] = {
    /* MTIP_GLOBAL_PMA_CONTROL */
  /*0*/  {/*baseAddr*/ 0x00000000,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_SD_8X */

    /* MTIP_GLOBAL_FEC_CONTROL */
  /*1*/  {/*baseAddr*/ 0x00000004,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_FEC_ENA */
  /*2*/  {/*baseAddr*/ 0x00000004,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_FEC_ERR_ENA */
  /*3*/  {/*baseAddr*/ 0x00000004,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GC_FEC91_ENA_IN */
  /*4*/  {/*baseAddr*/ 0x00000004,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*GC_KP_MODE_IN */

    /* MTIP_GLOBAL_CHANNEL_CONTROL */
  /*5*/  {/*baseAddr*/ 0x00000008,  /*offsetFormula*/ 0x0, /*fieldStart*/  19, /*fieldLen*/ 1 }, /*BER_TIMER_SHORT */
  /*6*/  {/*baseAddr*/ 0x00000008,  /*offsetFormula*/ 0x0, /*fieldStart*/  21, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG_ENA */
  /*7*/  {/*baseAddr*/ 0x00000008,  /*offsetFormula*/ 0x0, /*fieldStart*/  25, /*fieldLen*/ 3 }, /*CYC_TO_STRETCH_MAC2REG */

    /* MTIP_GLOBAL_CLOCK_CONTROL */
  /*8*/  {/*baseAddr*/ 0x0000000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_FAST_1LANE_MODE */
  /*9*/  {/*baseAddr*/ 0x0000000C,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_PACER_10G */

    /* GLOBAL_CLOCK_ENABLE */
  /*10*/  {/*baseAddr*/ 0x00000010,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*MAC_CLK_EN */
  /*11*/  {/*baseAddr*/ 0x00000010,  /*offsetFormula*/ 0x0, /*fieldStart*/  10, /*fieldLen*/ 1 }, /*MAC_CMN_CLK_EN */
  /*12*/  {/*baseAddr*/ 0x00000010,  /*offsetFormula*/ 0x0, /*fieldStart*/  11, /*fieldLen*/ 1 }, /*FEC91_CLK_EN */
  /*13*/  {/*baseAddr*/ 0x00000010,  /*offsetFormula*/ 0x0, /*fieldStart*/  12, /*fieldLen*/ 2 }, /*XPCS_CLK_EN */
  /*14*/  {/*baseAddr*/ 0x00000010,  /*offsetFormula*/ 0x0, /*fieldStart*/  15, /*fieldLen*/ 1 }, /*SGREF_CLK_EN */
  /*15*/  {/*baseAddr*/ 0x00000010,  /*offsetFormula*/ 0x0, /*fieldStart*/  16, /*fieldLen*/ 1 }, /*MAC_MAC_CMN_CLK_EN */

    /* GLOBAL_RESET_CONTROL */
  /*16*/  {/*baseAddr*/ 0x00000014,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_SD_RX_RESET_ */
  /*17*/  {/*baseAddr*/ 0x00000014,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_SD_TX_RESET_ */
  /*18*/  {/*baseAddr*/ 0x00000014,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GC_XPCS_RESET_ */
  /*19*/  {/*baseAddr*/ 0x00000014,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*GC_F91_RESET_ */
  /*20*/  {/*baseAddr*/ 0x00000014,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*GC_SG_RESET_ */

    /* GLOBAL_RESET_CONTROL2 */
  /*21*/  {/*baseAddr*/ 0x00000018,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MAC_MAC_CMN_RESET_ */

    /* MTIP_GLOBAL_AMPS_LOCK_STATUS */
  /*22*/  {/*baseAddr*/ 0x0000001C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*AMPS_LOCK */

    /* MTIP_GLOBAL_FEC_STATUS */
  /*23*/  {/*baseAddr*/ 0x00000020,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_LOCKED */

    /* MTIP_GLOBAL_FEC_ERROR_STATUS */
  /*24*/  {/*baseAddr*/ 0x00000024,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CERR */
  /*25*/  {/*baseAddr*/ 0x00000024,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCERR */

    /* MTIP_GLOBAL_BLOCK_LOCK_STATUS0 */
  /*26*/  {/*baseAddr*/ 0x00000028,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK0 */

    /* GLOBAL_INTERRUPT_CAUSE */
  /*27*/  {/*baseAddr*/ 0x0000002C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*28*/  {/*baseAddr*/ 0x0000002C,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */

    /* GLOBAL_INTERRUPT_MASK */
  /*29*/  {/*baseAddr*/ 0x00000030,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_MASK */

    /* GLOBAL_FEC_CE_INTERRUPT_CAUSE */
  /*30*/  {/*baseAddr*/ 0x00000034,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_CAUSE_INT_SUM */
  /*31*/  {/*baseAddr*/ 0x00000034,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_0 */

    /* GLOBAL_FEC_CE_INTERRUPT_MASK */
  /*32*/  {/*baseAddr*/ 0x00000038,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_MASK */

    /* GLOBAL_FEC_NCE_INTERRUPT_CAUSE */
  /*33*/  {/*baseAddr*/ 0x0000003C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_CAUSE_INT_SUM */
  /*34*/  {/*baseAddr*/ 0x0000003C,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_0 */

    /* GLOBAL_FEC_NCE_INTERRUPT_MASK */
  /*35*/  {/*baseAddr*/ 0x00000040,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_MASK */

    /* GLOBAL_INTERRUPT_SUMMARY_CAUSE */
  /*36*/  {/*baseAddr*/ 0x00000044,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*37*/  {/*baseAddr*/ 0x00000044,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*38*/  {/*baseAddr*/ 0x00000044,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*39*/  {/*baseAddr*/ 0x00000044,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_CE_INT_SUM */
  /*40*/  {/*baseAddr*/ 0x00000044,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_NCE_INT_SUM */

    /* GLOBAL_INTERRUPT_SUMMARY_MASK */
  /*41*/  {/*baseAddr*/ 0x00000048,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 4 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* LAST_VIOLATION */
  /*42*/  {/*baseAddr*/ 0x0000004C,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* GLOBAL_PCS_METAL_FIX */
  /*43*/  {/*baseAddr*/ 0x00000050,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* MTIP_PORT_STATUS */
  /*44*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XL_LINK_STATUS */
  /*45*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LPCS_LINK_STATUS */
  /*46*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LPCS_RX_SYNC */
  /*47*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LPCS_AN_DONE */
  /*48*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*XL_HI_BER */
  /*49*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RSFEC_ALIGNED */
  /*50*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*XL_LINK_STATUS_CLEAN */
  /*51*/  {/*baseAddr*/ 0x00000054,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LPCS_LINK_STATUS_CLEAN */

    /* MTIP_PORT_INTERRUPT_CAUSE */
  /*52*/  {/*baseAddr*/ 0x00000058,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*INTERRUPT_CAUSE_INT_SUM */
  /*53*/  {/*baseAddr*/ 0x00000058,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*XL_LINK_STATUS_CHANGE */
  /*54*/  {/*baseAddr*/ 0x00000058,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LPCS_LINK_STATUS_CHANGE */
  /*55*/  {/*baseAddr*/ 0x00000058,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*HI_BER */

    /* MTIP_PORT_INTERRUPT_MASK */
  /*56*/  {/*baseAddr*/ 0x0000005C,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*INTERRUPT_MASK */

    /* MTIP_PORT_CONTROL1 */
  /*57*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*RF_TX_LANE_THRESH */
  /*58*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FORCE_LINK_STATUS_EN */
  /*59*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FORCE_LINK_STATUS_DIS */
  /*60*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FORCE_LPCS_LINK_STATUS_EN */
  /*61*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FORCE_LPCS_LINK_STATUS_DIS */
  /*62*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FORCE_XL_LINK_STATUS_EN */
  /*63*/  {/*baseAddr*/ 0x00000060,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FORCE_XL_LINK_STATUS_DIS */

    /* PCS_ABU_WATCHDOG_MAX_TIMER */
  /*64*/  {/*baseAddr*/ 0x00000064,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PCS_ABU_WATCHDOG_MAX_TIMER */

    /* MTIP_PORT_STATUS_2 */
  /*65*/  {/*baseAddr*/ 0x00000068,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 6 }, /*SD_BIT_SLIP */

};

