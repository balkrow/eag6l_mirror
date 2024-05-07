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
* @file mvHwsIronmanMtip10GPcsFcfecUnits.c
*
* @brief IronmanL 10G PCS LPCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanPcsFcFecUnitsDb[] = {
    /* CONTROL */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS */
  /*1*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LINK_STATUS_REALTIME */
  /*2*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LINK_STATUS */
  /*3*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*SD_BIT_SLIP */

    /* PORTS_ENA */
  /*4*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*NUMPORTS */

    /* VL_INTVL */
  /*5*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MARKER_COUNTER */

    /* ALIGN_MATCH_ERR */
  /*6*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ALIGNMATCHERR */

    /* VL0_BYTE3 */
  /*7*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL0BYTE3 */

    /* VL_INTVL_HI */
  /*8*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MARKER_COUNTERHI */

    /* SDCONFIG */
  /*9*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_THRESHOLD */
  /*10*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TOGGLE_EN */

    /* VL0_0 */
  /*11*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*12*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* VL0_1 */
  /*13*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */
  /*14*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*PAD */

    /* VL1_0 */
  /*15*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*16*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* VL1_1 */
  /*17*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */
  /*18*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*PAD */

    /* VL2_0 */
  /*19*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*20*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* VL2_1 */
  /*21*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */
  /*22*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*PAD */

    /* VL3_0 */
  /*23*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M0 */
  /*24*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*M1 */

    /* VL3_1 */
  /*25*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*M2 */
  /*26*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*PAD */

    /* FC_FEC_ABILITY */
  /*27*/  {/*baseAddr*/ 0x300,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*FC_ABILITY */

    /* FC_FEC_CONTROL */
  /*28*/  {/*baseAddr*/ 0x304,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FC_ENABLE */
  /*29*/  {/*baseAddr*/ 0x304,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FC_ERROR_ENABLE */

    /* FC_FEC_STATUS */
  /*30*/  {/*baseAddr*/ 0x308,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FC_LOCKED */

    /* FC_FEC_CCW_LO */
  /*31*/  {/*baseAddr*/ 0x30c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* FC_FEC_NCCW_LO */
  /*32*/  {/*baseAddr*/ 0x310,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* FC_FEC_COUNTER_HI */
  /*33*/  {/*baseAddr*/ 0x314,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */
};

