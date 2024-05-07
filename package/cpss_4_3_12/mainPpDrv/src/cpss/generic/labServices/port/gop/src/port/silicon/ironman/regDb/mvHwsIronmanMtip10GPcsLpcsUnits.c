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
* @file mvHwsIronmanMtip10GPcsLpcsUnits.c
*
* @brief IronmanL 10G PCS LPCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanLpcsUnitsDb[] = {
    /* CONTROL */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_6 */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*DUPLEX */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*ANRESTART */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*ISOLATE */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*POWERDOWN */
  /*5*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*ANENABLE */
  /*6*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_13 */
  /*7*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*8*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS */
  /*9*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*EXTDCAPABILITY */
  /*10*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x80, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LINKSTATUS */
  /*11*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x80, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*ANEGABILITY */
  /*12*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ANEGCOMPLETE */

    /* PHY_ID_0 */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PHYID */

    /* PHY_ID_1 */
  /*14*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*PHYID */

    /* DEV_ABILITY */
  /*15*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*ABILITY_RSV05 */
  /*16*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FD */
  /*17*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*HD */
  /*18*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PS1 */
  /*19*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PS2 */
  /*20*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 3 }, /*ABILITY_RSV9 */
  /*21*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RF1 */
  /*22*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RF2 */
  /*23*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*ACK */
  /*24*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NP */

    /* PARTNER_ABILITY */
  /*25*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*PABILITY_RSV05 */
  /*26*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FD */
  /*27*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*HD */
  /*28*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PS1 */
  /*29*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PS2 */
  /*30*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*PABILITY_RSV9 */
  /*31*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*PABILITY_RSV10 */
  /*32*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RF1 */
  /*33*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RF2 */
  /*34*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*ACK */
  /*35*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NP */

    /* AN_EXPANSION */
  /*36*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x80, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PAGERECEIVED */
  /*37*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x80, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*NEXTPAGEABLE */

    /* NP_TX */
  /*38*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*DATA */
  /*39*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TOGGLE */
  /*40*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*ACK2 */
  /*41*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MP */
  /*42*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*ACK */
  /*43*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NP */

    /* LP_NP_RX */
  /*44*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*DATA */
  /*45*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TOGGLE */
  /*46*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*ACK2 */
  /*47*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x80, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*MP */
  /*48*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x80, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*ACK */
  /*49*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x80, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*NP */

    /* SCRATCH */
  /*50*/  {/*baseAddr*/ 0x40,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* REV */
  /*51*/  {/*baseAddr*/ 0x44,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* LINK_TIMER_0 */
  /*52*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TIMER0 */
  /*53*/  {/*baseAddr*/ 0x48,  /*offsetFormula*/ 0x80, /*fieldStart*/  1, /*fieldLen*/ 15 }, /*TIMER15_1 */

    /* LINK_TIMER_1 */
  /*54*/  {/*baseAddr*/ 0x4c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*TIMER20_16 */

    /* IF_MODE */
  /*55*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*SGMII_ENA */
  /*56*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*USE_SGMII_AN */
  /*57*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*SGMII_SPEED */
  /*58*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*SGMII_DUPLEX */
  /*59*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*IFMODE_RSV5 */
  /*60*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*IFMODE_TX_PREAMBLE_SYNC */
  /*61*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*IFMODE_RX_PREAMBLE_SYNC */
  /*62*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*IFMODE_MODE_XGMII_BASEX */
  /*63*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*IFMODE_SEQ_ENA */
  /*64*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_BR_DIS */
  /*65*/  {/*baseAddr*/ 0x50,  /*offsetFormula*/ 0x80, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SGMII_FIX_ENA */

    /* DECODE_ERRORS */
  /*66*/  {/*baseAddr*/ 0x54,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*DECODEERRORS */

    /* USXGMII_REP */
  /*67*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*USXGMIIREP */
  /*68*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x80, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*USXGMII2_5 */

    /* TX_IPG_LENGTH */
  /*69*/  {/*baseAddr*/ 0x5c,  /*offsetFormula*/ 0x80, /*fieldStart*/  0, /*fieldLen*/ 7 }, /*TXIPGLENGTH */
};

