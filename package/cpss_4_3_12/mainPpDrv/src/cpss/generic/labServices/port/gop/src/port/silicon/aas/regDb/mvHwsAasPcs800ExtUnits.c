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
* @file mvHwsAasPcs800ExtUnits.c
*
* @brief AAS MTI PCS Port EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasPcs800ExtDb[] = {
    /* Global PMA Control */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*GC_SD_8X */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*GC_SD_16X */

    /* Global FEC Control */
  /*2*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*GC_FEC_ENA */
  /*3*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*GC_FEC_ERR_ENA */

    /* Global FEC Control 2 */
  /*4*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*GC_FEC91_ENA_IN */
  /*5*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*GC_KP_MODE_IN */

    /* Global Channel Control */
  /*6*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*GC_PCS400_ENA_IN */
  /*7*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_0 */
  /*8*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_3 */
  /*9*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_2 */
  /*10*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*GC_MLG_ENA_IN_1 */
  /*11*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*MODE_800G */
  /*12*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PCS_ABU_WATCHDOG */

    /* Global Channel Control 2 */
  /*13*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN0 */
  /*14*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN4 */
  /*15*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN8 */
  /*16*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*GC_MODE40_ENA_IN12 */
  /*17*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN0 */
  /*18*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN2 */
  /*19*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN4 */
  /*20*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN6 */
  /*21*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN8 */
  /*22*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN10 */
  /*23*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN12 */
  /*24*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*GC_PCS100_ENA_IN14 */
  /*25*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN0 */
  /*26*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN2 */
  /*27*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN4 */
  /*28*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN6 */
  /*29*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN8 */
  /*30*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN10 */
  /*31*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN12 */
  /*32*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*GC_FEC91_1LANE_IN14 */
  /*33*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN0 */
  /*34*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN2 */
  /*35*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN4 */
  /*36*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN6 */
  /*37*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN8 */
  /*38*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN10 */
  /*39*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN12 */
  /*40*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*GC_RXLAUI_ENA_IN14 */

    /* Global Clock Control */
  /*41*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*GC_FAST_1LANE_MODE */
  /*42*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*GC_PACER_10G */

    /* Global Clock Control 2 */
  /*43*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GC_CFG_CLOCK_RATE */

    /* Global Clock Enable */
  /*44*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_CMN_CLK_EN */
  /*45*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 2 }, /*FEC91_CLK_EN */
  /*46*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 4 }, /*XPCS_CLK_EN */
  /*47*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 2 }, /*PCS000_CLK_EN */
  /*48*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*SGREF_CLK_EN */

    /* Global Reset Control */
  /*49*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*GC_SD_RX_RESET_ */
  /*50*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*GC_SD_TX_RESET_ */

    /* Global Reset Control 2 */
  /*51*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*GC_PCS000_RESET_ */
  /*52*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*GC_XPCS_RESET_ */
  /*53*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*GC_F91_RESET_ */
  /*54*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 2 }, /*GC_SG_RESET_ */
  /*55*/  {/*baseAddr*/ 0x24,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*GC_REF_RESET_ */

    /* Global Amps Lock Status */
  /*56*/  {/*baseAddr*/ 0x28,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS000_AMPS_LOCK */

    /* Global Amps Lock Status 2 */
  /*57*/  {/*baseAddr*/ 0x2c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*AMPS_LOCK */

    /* Global FEC Status */
  /*58*/  {/*baseAddr*/ 0x30,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*FEC_LOCKED */

    /* Global FEC Error Status */
  /*59*/  {/*baseAddr*/ 0x34,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*FEC_NCERR */

    /* Global FEC Error Status 2 */
  /*60*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*FEC_CERR */

    /* Global Block Lock Status0 */
  /*61*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK0 */
  /*62*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK1 */
  /*63*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK2 */
  /*64*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK3 */
  /*65*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK4 */
  /*66*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK5 */
  /*67*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK6 */
  /*68*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK7 */
  /*69*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK8 */
  /*70*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK9 */
  /*71*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK10 */
  /*72*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK11 */
  /*73*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK12 */
  /*74*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK13 */
  /*75*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK14 */
  /*76*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK15 */
  /*77*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK16 */
  /*78*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK17 */
  /*79*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK18 */
  /*80*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK19 */

    /* Global Block Lock Status1 */
  /*81*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK20 */
  /*82*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK21 */
  /*83*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK22 */
  /*84*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK23 */
  /*85*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK24 */
  /*86*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK25 */
  /*87*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK26 */
  /*88*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK27 */
  /*89*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK28 */
  /*90*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK29 */
  /*91*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK30 */
  /*92*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK31 */
  /*93*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK32 */
  /*94*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK33 */
  /*95*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK34 */
  /*96*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK35 */
  /*97*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK36 */
  /*98*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK37 */
  /*99*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK38 */
  /*100*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK39 */

    /* Global Block Lock Status 2 */
  /*101*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK40 */
  /*102*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK41 */
  /*103*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK42 */
  /*104*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK43 */
  /*105*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK44 */
  /*106*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK45 */
  /*107*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK46 */
  /*108*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK47 */
  /*109*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK48 */
  /*110*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK49 */
  /*111*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK50 */
  /*112*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK51 */
  /*113*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK52 */
  /*114*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK53 */
  /*115*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK54 */
  /*116*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK55 */
  /*117*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK56 */
  /*118*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK57 */
  /*119*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK58 */
  /*120*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK59 */

    /* Global Block Lock Status 3 */
  /*121*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK60 */
  /*122*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*BLOCK_LOCK61 */
  /*123*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*BLOCK_LOCK62 */
  /*124*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*BLOCK_LOCK63 */
  /*125*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*BLOCK_LOCK64 */
  /*126*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*BLOCK_LOCK65 */
  /*127*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*BLOCK_LOCK66 */
  /*128*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*BLOCK_LOCK67 */
  /*129*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*BLOCK_LOCK68 */
  /*130*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*BLOCK_LOCK69 */
  /*131*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*BLOCK_LOCK70 */
  /*132*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*BLOCK_LOCK71 */
  /*133*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*BLOCK_LOCK72 */
  /*134*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*BLOCK_LOCK73 */
  /*135*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*BLOCK_LOCK74 */
  /*136*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK75 */
  /*137*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*BLOCK_LOCK76 */
  /*138*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*BLOCK_LOCK77 */
  /*139*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*BLOCK_LOCK78 */
  /*140*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*BLOCK_LOCK79 */

    /* Global Interrupt Cause */
  /*141*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_CAUSE_INT_SUM */
  /*142*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*143*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ABU_BAD_ADDRESS_ERROR */
  /*144*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*ABU_WATCHDOG_ERROR */

    /* Global Interrupt Mask */
  /*145*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 3 }, /*GLOBAL_INTERRUPT_MASK */

    /* Global FEC CE Interrupt Cause */
  /*146*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_CAUSE_INT_SUM */
  /*147*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_0 */
  /*148*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_CE_1 */
  /*149*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_CE_2 */
  /*150*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_CE_3 */
  /*151*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_CE_4 */
  /*152*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_CE_5 */
  /*153*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_CE_6 */
  /*154*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_CE_7 */
  /*155*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_CE_8 */
  /*156*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_CE_9 */
  /*157*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_CE_10 */
  /*158*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_CE_11 */
  /*159*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_CE_12 */
  /*160*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_CE_13 */
  /*161*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_CE_14 */
  /*162*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_CE_15 */

    /* Global FEC CE Interrupt Cause 1 */
  /*163*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_CE_INTERRUPT_CAUSE_1_INT_SUM */
  /*164*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_CE_16 */
  /*165*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_CE_17 */
  /*166*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_CE_18 */
  /*167*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_CE_19 */
  /*168*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_CE_20 */
  /*169*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_CE_21 */
  /*170*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_CE_22 */
  /*171*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_CE_23 */
  /*172*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_CE_24 */
  /*173*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_CE_25 */
  /*174*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_CE_26 */
  /*175*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_CE_27 */
  /*176*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_CE_28 */
  /*177*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_CE_29 */
  /*178*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_CE_30 */
  /*179*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_CE_31 */

    /* Global FEC CE Interrupt Mask */
  /*180*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_CE_INTERRUPT_MASK */

    /* Global FEC CE Interrupt Mask 1 */
  /*181*/  {/*baseAddr*/ 0x60,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_CE_INTERRUPT_MASK_1 */

    /* Global FEC NCE Interrupt Cause */
  /*182*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_CAUSE_INT_SUM */
  /*183*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_0 */
  /*184*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_NCE_1 */
  /*185*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_NCE_2 */
  /*186*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_NCE_3 */
  /*187*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_NCE_4 */
  /*188*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_NCE_5 */
  /*189*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_NCE_6 */
  /*190*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_NCE_7 */
  /*191*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_NCE_8 */
  /*192*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_NCE_9 */
  /*193*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_NCE_10 */
  /*194*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_NCE_11 */
  /*195*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_NCE_12 */
  /*196*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_NCE_13 */
  /*197*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_NCE_14 */
  /*198*/  {/*baseAddr*/ 0x64,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_NCE_15 */

    /* Global FEC NCE Interrupt Cause 1 */
  /*199*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC_NCE_INTERRUPT_CAUSE_1_INT_SUM */
  /*200*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FEC_NCE_16 */
  /*201*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FEC_NCE_17 */
  /*202*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FEC_NCE_18 */
  /*203*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FEC_NCE_19 */
  /*204*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FEC_NCE_20 */
  /*205*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FEC_NCE_21 */
  /*206*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FEC_NCE_22 */
  /*207*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FEC_NCE_23 */
  /*208*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FEC_NCE_24 */
  /*209*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FEC_NCE_25 */
  /*210*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FEC_NCE_26 */
  /*211*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FEC_NCE_27 */
  /*212*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FEC_NCE_28 */
  /*213*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FEC_NCE_29 */
  /*214*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FEC_NCE_30 */
  /*215*/  {/*baseAddr*/ 0x68,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FEC_NCE_31 */

    /* Global FEC NCE Interrupt Mask */
  /*216*/  {/*baseAddr*/ 0x6c,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_NCE_INTERRUPT_MASK */

    /* Global FEC NCE Interrupt Mask 1 */
  /*217*/  {/*baseAddr*/ 0x70,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 16 }, /*FEC_NCE_INTERRUPT_MASK_1 */

    /* Global Interrupt Summary Cause */
  /*218*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*GLOBAL_INTERRUPT_SUMMARY_INT_SUM */
  /*219*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_INT_SUM */
  /*220*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P1_INT_SUM */
  /*221*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P2_INT_SUM */
  /*222*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P3_INT_SUM */
  /*223*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P4_INT_SUM */
  /*224*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P5_INT_SUM */
  /*225*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*P6_INT_SUM */
  /*226*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P7_INT_SUM */
  /*227*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*P8_INT_SUM */
  /*228*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*P9_INT_SUM */
  /*229*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*P10_INT_SUM */
  /*230*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*P11_INT_SUM */
  /*231*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*P12_INT_SUM */
  /*232*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*P13_INT_SUM */
  /*233*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*P14_INT_SUM */
  /*234*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*P15_INT_SUM */
  /*235*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*GLOBAL_INT_SUM */
  /*236*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*FEC_CE_INT_SUM */
  /*237*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*FEC_CE_1_INT_SUM */
  /*238*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*FEC_NCE_INT_SUM */
  /*239*/  {/*baseAddr*/ 0x74,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*FEC_NCE_1_INT_SUM */

    /* Global Interrupt Summary Mask */
  /*240*/  {/*baseAddr*/ 0x78,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 21 }, /*GLOBAL_INTERRUPT_SUMMARY_MASK */

    /* Global Last Violation */
  /*241*/  {/*baseAddr*/ 0x7c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*LAST_VIOLATION */

    /* Port<0> Marker insertion counters */
  /*242*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*P0_MARKER_INS_CNT_25_50 */
  /*243*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x18, /*fieldStart*/ 16, /*fieldLen*/ 15 }, /*P0_MARKER_INS_CNT_100 */

    /* Port<0> Interrupt Cause */
  /*244*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT0_INTERRUPT_CAUSE_INT_SUM */
  /*245*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LINK_STATUS_CHANGE */
  /*246*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS_CHANGE */
  /*247*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SEG_P0_HI_SER_INT */
  /*248*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SEG_P0_DEGRADE_SER_INT */
  /*249*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_HI_BER_INT */

    /* Port<0> Interrupt Mask */
  /*250*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 5 }, /*P0_INTERRUPT_MASK */

    /* Port<0> Control */
  /*251*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*SEG_P0_TX_AM_SF */
  /*252*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LATCH_DESK_RLEVEL */
  /*253*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_SXGMII_MODE */

    /* Port<0> Status */
  /*254*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*P0_LINK_STATUS */
  /*255*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*P0_LPCS_LINK_STATUS */
  /*256*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*P0_LPCS_RX_SYNC */
  /*257*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*P0_LPCS_AN_DONE */
  /*258*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*P0_ALIGN_DONE */
  /*259*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*P0_BER_TIMER_DONE */
  /*260*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*P0_HI_BER */
  /*261*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SEG_P0_DEGRADE_SER */
  /*262*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*P0_RSFEC_ALIGNED */
  /*263*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEG_P0_HI_SER */
  /*264*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/ 10, /*fieldLen*/ 3 }, /*SEG_P0_RX_AM_SF */
  /*265*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/ 13, /*fieldLen*/ 6 }, /*SEG_P0_SD_BIT_SLIP */
  /*266*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*P0_QUAD_LINK_STATUS */
  /*267*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*P0_PCS000_LINK_STATUS */
  /*268*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x18, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*P0_PCS000_ALIGN_LOCK */

    /* Global PCS Metal Fix */
  /*269*/  {/*baseAddr*/ 0x204,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PCS_METAL_FIX */

    /* PCS ABU watchdog max timer */
  /*270*/  {/*baseAddr*/ 0x208,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*PCS_ABU_WATCHDOG_MAX_TIMER */

    /* PMA MUX */
  /*271*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_0 */
  /*272*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_1 */
  /*273*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_2 */
  /*274*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_3 */
  /*275*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_4 */
  /*276*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_5 */
  /*277*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_6 */
  /*278*/  {/*baseAddr*/ 0x20c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_7 */

    /* PMA MUX 1 */
  /*279*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_8 */
  /*280*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_9 */
  /*281*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_10 */
  /*282*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_11 */
  /*283*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_12 */
  /*284*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_13 */
  /*285*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/ 24, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_14 */
  /*286*/  {/*baseAddr*/ 0x210,  /*offsetFormula*/ 0x0, /*fieldStart*/ 28, /*fieldLen*/ 4 }, /*RF_SD_MUX_SEL_LANE_15 */

    /* Flexe mode */
  /*287*/  {/*baseAddr*/ 0x214,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FLEXE_EN */

    /* VL manual sel */
  /*288*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_0 */
  /*289*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_1 */
  /*290*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_2 */
  /*291*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_3 */
  /*292*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_4 */
  /*293*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_5 */
  /*294*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_6 */
  /*295*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_7 */
  /*296*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_8 */
  /*297*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_9 */
  /*298*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_10 */
  /*299*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_11 */
  /*300*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_12 */
  /*301*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_13 */
  /*302*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_14 */
  /*303*/  {/*baseAddr*/ 0x218,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*VL_MANUAL_MARK_EN_15 */

    /* port<0> VL &lt;0&gt; */
  /*304*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x50, /*fieldStart*/  0, /*fieldLen*/ 24 }, /*VL */
};

