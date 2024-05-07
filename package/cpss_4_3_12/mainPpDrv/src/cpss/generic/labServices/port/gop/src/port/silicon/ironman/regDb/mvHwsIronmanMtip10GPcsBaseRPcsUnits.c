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
* @file mvHwsIronmanMtip10GPcsBaseRPcsUnits.c
*
* @brief IronmanL 10G PCS BASE_R PCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanBaseRPcsUnitsDb[] = {
    /* CONTROL1 */
  /*0*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*SPEED_SELECTION */
  /*1*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_ALWAYS1 */
  /*2*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LOW_POWER */
  /*3*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_SELECT_ALWAYS1 */
  /*4*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*5*/  {/*baseAddr*/ 0x0,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS1 */
  /*6*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LOW_POWER_ABILITY */
  /*7*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*8*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */
  /*9*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RX_LPI_ACTIVE */
  /*10*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_LPI_ACTIVE */
  /*11*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_LPI */
  /*12*/  {/*baseAddr*/ 0x4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_LPI */

    /* DEVICE_ID0 */
  /*13*/  {/*baseAddr*/ 0x8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* DEVICE_ID1 */
  /*14*/  {/*baseAddr*/ 0xc,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* SPEED_ABILITY */
  /*15*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GETH */
  /*16*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10PASS_TS */
  /*17*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C40G */
  /*18*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C100G */
  /*19*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C25G */
  /*20*/  {/*baseAddr*/ 0x10,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C50G */

    /* DEVICES_IN_PKG1 */
  /*21*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*22*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMD_PMA */
  /*23*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRES */
  /*24*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRES */
  /*25*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS */
  /*26*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS */
  /*27*/  {/*baseAddr*/ 0x14,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRES */

    /* DEVICES_IN_PKG2 */
  /*28*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*29*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DEVICE1 */
  /*30*/  {/*baseAddr*/ 0x18,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*DEVICE2 */

    /* CONTROL2 */
  /*31*/  {/*baseAddr*/ 0x1c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*PCS_TYPE */

    /* STATUS2 */
  /*32*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GBASE_R */
  /*33*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10GBASE_X */
  /*34*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C10GBASE_W */
  /*35*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C10GBASE_T */
  /*36*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C40GBASE_R */
  /*37*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C100GBASE_R */
  /*38*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C25GBASE_R */
  /*39*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*C50GBASE_R */
  /*40*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*41*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*42*/  {/*baseAddr*/ 0x20,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* PKG_ID0 */
  /*43*/  {/*baseAddr*/ 0x38,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* PKG_ID1 */
  /*44*/  {/*baseAddr*/ 0x3c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* WAKE_ERR_COUNTER */
  /*45*/  {/*baseAddr*/ 0x58,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* BASER_STATUS1 */
  /*46*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK */
  /*47*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*48*/  {/*baseAddr*/ 0x80,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* BASER_STATUS2 */
  /*49*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */
  /*50*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*BER_COUNTER */
  /*51*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*52*/  {/*baseAddr*/ 0x84,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK */

    /* SEED_A0 */
  /*53*/  {/*baseAddr*/ 0x88,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A1 */
  /*54*/  {/*baseAddr*/ 0x8c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A2 */
  /*55*/  {/*baseAddr*/ 0x90,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A3 */
  /*56*/  {/*baseAddr*/ 0x94,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* SEED_B0 */
  /*57*/  {/*baseAddr*/ 0x98,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B1 */
  /*58*/  {/*baseAddr*/ 0x9c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B2 */
  /*59*/  {/*baseAddr*/ 0xa0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B3 */
  /*60*/  {/*baseAddr*/ 0xa4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* BASER_TEST_CONTROL */
  /*61*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DATA_PATTERN_SEL */
  /*62*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SELECT_SQUARE */
  /*63*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RX_TESTPATTERN */
  /*64*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_TESTPATTERN */
  /*65*/  {/*baseAddr*/ 0xa8,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* BASER_TEST_ERR_CNT */
  /*66*/  {/*baseAddr*/ 0xac,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* BER_HIGH_ORDER_CNT */
  /*67*/  {/*baseAddr*/ 0xb0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* ERR_BLK_HIGH_ORDER_CNT */
  /*68*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*69*/  {/*baseAddr*/ 0xb4,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* MULTILANE_ALIGN_STAT1 */
  /*70*/  {/*baseAddr*/ 0xc8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK_L */

    /* VENDORMX_SCRATCH */
  /*71*/  {/*baseAddr*/ 0xf0,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* VENDORMX_CORE_REV */
  /*72*/  {/*baseAddr*/ 0xf4,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* VENDORMX_PCS_MODE */
  /*73*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*ENA_CLAUSE49 */
  /*74*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DISABLE_MLD */
  /*75*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HI_BER25 */
  /*76*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*HI_BER5 */
  /*77*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ST_ENA_CLAUSE49 */
  /*78*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*ST_DISABLE_MLD */
  /*79*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*ST_HIBER25 */
  /*80*/  {/*baseAddr*/ 0xf8,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*ST_HIBER5 */

    /* VENDORMX_TXLANE_THRESH */
  /*81*/  {/*baseAddr*/ 0xfc,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*THRESHOLD */
};

