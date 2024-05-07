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
* @file mvHwsAasPcs100Units.c
*
* @brief AAS MTI PCS100 register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasPcs100Db[] = {
    /* CONTROL1 */
  /*0*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 4 }, /*SPEED_SELECTION */
  /*1*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x2000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SPEED_ALWAYS1 */
  /*2*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LOW_POWER */
  /*3*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SPEED_SELECT_ALWAYS1 */
  /*4*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LOOPBACK */
  /*5*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RESET */

    /* STATUS1 */
  /*6*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LOW_POWER_ABILITY */
  /*7*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PCS_RECEIVE_LINK */
  /*8*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FAULT */
  /*9*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RX_LPI_ACTIVE */
  /*10*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_LPI_ACTIVE */
  /*11*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_LPI */
  /*12*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_LPI */

    /* DEVICE_ID0 */
  /*13*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER0 */

    /* DEVICE_ID1 */
  /*14*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER1 */

    /* SPEED_ABILITY */
  /*15*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GETH */
  /*16*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10PASS_TS */
  /*17*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C40G */
  /*18*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C100G */
  /*19*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C25G */
  /*20*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C50G */

    /* DEVICES_IN_PKG1 */
  /*21*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*22*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMD_PMA */
  /*23*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRES */
  /*24*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRES */
  /*25*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS */
  /*26*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS */
  /*27*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x2000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRES */

    /* DEVICES_IN_PKG2 */
  /*28*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE22 */
  /*29*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DEVICE1 */
  /*30*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*DEVICE2 */

    /* CONTROL2 */
  /*31*/  {/*baseAddr*/ 0x101c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PCS_TYPE */

    /* STATUS2 */
  /*32*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C10GBASE_R */
  /*33*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C10GBASE_X */
  /*34*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C10GBASE_W */
  /*35*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C10GBASE_T */
  /*36*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C40GBASE_R */
  /*37*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C100GBASE_R */
  /*38*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C25GBASE_R */
  /*39*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*C50GBASE_R */
  /*40*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RECEIVE_FAULT */
  /*41*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TRANSMIT_FAULT */
  /*42*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*DEVICE_PRESENT */

    /* PKG_ID0 */
  /*43*/  {/*baseAddr*/ 0x1038,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* PKG_ID1 */
  /*44*/  {/*baseAddr*/ 0x103c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*IDENTIFIER */

    /* BASER_STATUS1 */
  /*45*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*BLOCK_LOCK */
  /*46*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*47*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RECEIVE_LINK */

    /* BASER_STATUS2 */
  /*48*/  {/*baseAddr*/ 0x1084,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*ERRORED_CNT */
  /*49*/  {/*baseAddr*/ 0x1084,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 6 }, /*BER_COUNTER */
  /*50*/  {/*baseAddr*/ 0x1084,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*HIGH_BER */
  /*51*/  {/*baseAddr*/ 0x1084,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*BLOCK_LOCK */

    /* SEED_A0 */
  /*52*/  {/*baseAddr*/ 0x1088,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A1 */
  /*53*/  {/*baseAddr*/ 0x108c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A2 */
  /*54*/  {/*baseAddr*/ 0x1090,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_A3 */
  /*55*/  {/*baseAddr*/ 0x1094,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* SEED_B0 */
  /*56*/  {/*baseAddr*/ 0x1098,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B1 */
  /*57*/  {/*baseAddr*/ 0x109c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B2 */
  /*58*/  {/*baseAddr*/ 0x10a0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SEED */

    /* SEED_B3 */
  /*59*/  {/*baseAddr*/ 0x10a4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 10 }, /*SEED */

    /* BASER_TEST_CONTROL */
  /*60*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*DATA_PATTERN_SEL */
  /*61*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*SELECT_SQUARE */
  /*62*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RX_TESTPATTERN */
  /*63*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_TESTPATTERN */
  /*64*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SELECT_RANDOM */

    /* BASER_TEST_ERR_CNT */
  /*65*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*COUNTER */

    /* BER_HIGH_ORDER_CNT */
  /*66*/  {/*baseAddr*/ 0x10b0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*BER_COUNTER */

    /* ERR_BLK_HIGH_ORDER_CNT */
  /*67*/  {/*baseAddr*/ 0x10b4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*ERRORED_BLOCKS_COUNTER */
  /*68*/  {/*baseAddr*/ 0x10b4,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*HIGH_ORDER_PRESENT */

    /* MULTILANE_ALIGN_STAT1 */
  /*69*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE0_BLOCK_LOCK */
  /*70*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE1_BLOCK_LOCK */
  /*71*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE2_BLOCK_LOCK */
  /*72*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE3_BLOCK_LOCK */
  /*73*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LANE4_BLOCK_LOCK */
  /*74*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LANE5_BLOCK_LOCK */
  /*75*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LANE6_BLOCK_LOCK */
  /*76*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LANE7_BLOCK_LOCK */
  /*77*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LANE_ALIGN_STATUS */

    /* MULTILANE_ALIGN_STAT2 */
  /*78*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE8_BLOCK_LOCK */
  /*79*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE9_BLOCK_LOCK */
  /*80*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE10_BLOCK_LOCK */
  /*81*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE11_BLOCK_LOCK */
  /*82*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LANE12_BLOCK_LOCK */
  /*83*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LANE13_BLOCK_LOCK */
  /*84*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LANE14_BLOCK_LOCK */
  /*85*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LANE15_BLOCK_LOCK */
  /*86*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LANE16_BLOCK_LOCK */
  /*87*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LANE17_BLOCK_LOCK */
  /*88*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LANE18_BLOCK_LOCK */
  /*89*/  {/*baseAddr*/ 0x10cc,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LANE19_BLOCK_LOCK */

    /* MULTILANE_ALIGN_STAT3 */
  /*90*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE0_ALIGN_MLOCK */
  /*91*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE1_ALIGN_MLOCK */
  /*92*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE2_ALIGN_MLOCK */
  /*93*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE3_ALIGN_MLOCK */
  /*94*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LANE4_ALIGN_MLOCK */
  /*95*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LANE5_ALIGN_MLOCK */
  /*96*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LANE6_ALIGN_MLOCK */
  /*97*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LANE7_ALIGN_MLOCK */

    /* MULTILANE_ALIGN_STAT4 */
  /*98*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LANE8_ALIGN_MLOCK */
  /*99*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LANE9_ALIGN_MLOCK */
  /*100*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LANE10_ALIGN_MLOCK */
  /*101*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LANE11_ALIGN_MLOCK */
  /*102*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LANE12_ALIGN_MLOCK */
  /*103*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LANE13_ALIGN_MLOCK */
  /*104*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LANE14_ALIGN_MLOCK */
  /*105*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LANE15_ALIGN_MLOCK */
  /*106*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LANE16_ALIGN_MLOCK */
  /*107*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LANE17_ALIGN_MLOCK */
  /*108*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LANE18_ALIGN_MLOCK */
  /*109*/  {/*baseAddr*/ 0x10d4,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LANE19_ALIGN_MLOCK */

    /* BIP_ERR_CNT_LANE0 */
  /*110*/  {/*baseAddr*/ 0x1320,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE1 */
  /*111*/  {/*baseAddr*/ 0x1324,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE2 */
  /*112*/  {/*baseAddr*/ 0x1328,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE3 */
  /*113*/  {/*baseAddr*/ 0x132c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE4 */
  /*114*/  {/*baseAddr*/ 0x1330,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE5 */
  /*115*/  {/*baseAddr*/ 0x1334,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE6 */
  /*116*/  {/*baseAddr*/ 0x1338,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE7 */
  /*117*/  {/*baseAddr*/ 0x133c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE8 */
  /*118*/  {/*baseAddr*/ 0x1340,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE9 */
  /*119*/  {/*baseAddr*/ 0x1344,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE10 */
  /*120*/  {/*baseAddr*/ 0x1348,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE11 */
  /*121*/  {/*baseAddr*/ 0x134c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE12 */
  /*122*/  {/*baseAddr*/ 0x1350,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE13 */
  /*123*/  {/*baseAddr*/ 0x1354,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE14 */
  /*124*/  {/*baseAddr*/ 0x1358,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE15 */
  /*125*/  {/*baseAddr*/ 0x135c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE16 */
  /*126*/  {/*baseAddr*/ 0x1360,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE17 */
  /*127*/  {/*baseAddr*/ 0x1364,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE18 */
  /*128*/  {/*baseAddr*/ 0x1368,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* BIP_ERR_CNT_LANE19 */
  /*129*/  {/*baseAddr*/ 0x136c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ERROR_COUNTER */

    /* LANE0_MAPPING */
  /*130*/  {/*baseAddr*/ 0x1640,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE1_MAPPING */
  /*131*/  {/*baseAddr*/ 0x1644,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE2_MAPPING */
  /*132*/  {/*baseAddr*/ 0x1648,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE3_MAPPING */
  /*133*/  {/*baseAddr*/ 0x164c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE4_MAPPING */
  /*134*/  {/*baseAddr*/ 0x1650,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE5_MAPPING */
  /*135*/  {/*baseAddr*/ 0x1654,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE6_MAPPING */
  /*136*/  {/*baseAddr*/ 0x1658,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE7_MAPPING */
  /*137*/  {/*baseAddr*/ 0x165c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE8_MAPPING */
  /*138*/  {/*baseAddr*/ 0x1660,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE9_MAPPING */
  /*139*/  {/*baseAddr*/ 0x1664,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE10_MAPPING */
  /*140*/  {/*baseAddr*/ 0x1668,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE11_MAPPING */
  /*141*/  {/*baseAddr*/ 0x166c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE12_MAPPING */
  /*142*/  {/*baseAddr*/ 0x1670,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE13_MAPPING */
  /*143*/  {/*baseAddr*/ 0x1674,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE14_MAPPING */
  /*144*/  {/*baseAddr*/ 0x1678,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE15_MAPPING */
  /*145*/  {/*baseAddr*/ 0x167c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE16_MAPPING */
  /*146*/  {/*baseAddr*/ 0x1680,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE17_MAPPING */
  /*147*/  {/*baseAddr*/ 0x1684,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE18_MAPPING */
  /*148*/  {/*baseAddr*/ 0x1688,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* LANE19_MAPPING */
  /*149*/  {/*baseAddr*/ 0x168c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAPPING */

    /* VENDOR_SCRATCH */
  /*150*/  {/*baseAddr*/ 0x1800,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*SCRATCH */

    /* VENDOR_CORE_REV */
  /*151*/  {/*baseAddr*/ 0x1804,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*REVISION */

    /* VENDOR_VL_INTVL */
  /*152*/  {/*baseAddr*/ 0x1808,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MARKER_COUNTER */

    /* VENDOR_TXLANE_THRESH */
  /*153*/  {/*baseAddr*/ 0x180c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*THRESHOLD_0_3 */

    /* VENDOR_MIRROR_VL0_0 */
  /*154*/  {/*baseAddr*/ 0x1820,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*155*/  {/*baseAddr*/ 0x1820,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL0_1 */
  /*156*/  {/*baseAddr*/ 0x1824,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_MIRROR_VL1_0 */
  /*157*/  {/*baseAddr*/ 0x1828,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*158*/  {/*baseAddr*/ 0x1828,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL1_1 */
  /*159*/  {/*baseAddr*/ 0x182c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_MIRROR_VL2_0 */
  /*160*/  {/*baseAddr*/ 0x1830,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*161*/  {/*baseAddr*/ 0x1830,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL2_1 */
  /*162*/  {/*baseAddr*/ 0x1834,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_MIRROR_VL3_0 */
  /*163*/  {/*baseAddr*/ 0x1838,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M0 */
  /*164*/  {/*baseAddr*/ 0x1838,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*MIRROR_M1 */

    /* VENDOR_MIRROR_VL3_1 */
  /*165*/  {/*baseAddr*/ 0x183c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*MIRROR_M2 */

    /* VENDOR_PCS_MODE */
  /*166*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*ENA_CLAUSE49 */
  /*167*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*DISABLE_MLD */
  /*168*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*HI_BER25 */
  /*169*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*HI_BER5 */
  /*170*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ST_ENA_CLAUSE49 */
  /*171*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*ST_DISABLE_MLD */
  /*172*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*ST_HI_BER25 */
  /*173*/  {/*baseAddr*/ 0x1840,  /*offsetFormula*/ 0x2000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*ST_HI_BER5 */

    /* VL0_0 */
  /*174*/  {/*baseAddr*/ 0x1900,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL0_0 */

    /* VL0_1 */
  /*175*/  {/*baseAddr*/ 0x1904,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL0_1 */

    /* VL1_0 */
  /*176*/  {/*baseAddr*/ 0x1908,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL1_0 */

    /* VL1_1 */
  /*177*/  {/*baseAddr*/ 0x190c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL1_1 */

    /* VL2_0 */
  /*178*/  {/*baseAddr*/ 0x1910,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL2_0 */

    /* VL2_1 */
  /*179*/  {/*baseAddr*/ 0x1914,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL2_1 */

    /* VL3_0 */
  /*180*/  {/*baseAddr*/ 0x1918,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL3_0 */

    /* VL3_1 */
  /*181*/  {/*baseAddr*/ 0x191c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL3_1 */

    /* VL4_0 */
  /*182*/  {/*baseAddr*/ 0x1920,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL4_0 */

    /* VL4_1 */
  /*183*/  {/*baseAddr*/ 0x1924,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL4_1 */

    /* VL5_0 */
  /*184*/  {/*baseAddr*/ 0x1928,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL5_0 */

    /* VL5_1 */
  /*185*/  {/*baseAddr*/ 0x192c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL5_1 */

    /* VL6_0 */
  /*186*/  {/*baseAddr*/ 0x1930,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL6_0 */

    /* VL6_1 */
  /*187*/  {/*baseAddr*/ 0x1934,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL6_1 */

    /* VL7_0 */
  /*188*/  {/*baseAddr*/ 0x1938,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL7_0 */

    /* VL7_1 */
  /*189*/  {/*baseAddr*/ 0x193c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL7_1 */

    /* VL8_0 */
  /*190*/  {/*baseAddr*/ 0x1940,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL8_0 */

    /* VL8_1 */
  /*191*/  {/*baseAddr*/ 0x1944,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL8_1 */

    /* VL9_0 */
  /*192*/  {/*baseAddr*/ 0x1948,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL9_0 */

    /* VL9_1 */
  /*193*/  {/*baseAddr*/ 0x194c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL9_1 */

    /* VL10_0 */
  /*194*/  {/*baseAddr*/ 0x1950,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL10_0 */

    /* VL10_1 */
  /*195*/  {/*baseAddr*/ 0x1954,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL10_1 */

    /* VL11_0 */
  /*196*/  {/*baseAddr*/ 0x1958,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL11_0 */

    /* VL11_1 */
  /*197*/  {/*baseAddr*/ 0x195c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL11_1 */

    /* VL12_0 */
  /*198*/  {/*baseAddr*/ 0x1960,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL12_0 */

    /* VL12_1 */
  /*199*/  {/*baseAddr*/ 0x1964,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL12_1 */

    /* VL13_0 */
  /*200*/  {/*baseAddr*/ 0x1968,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL13_0 */

    /* VL13_1 */
  /*201*/  {/*baseAddr*/ 0x196c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL13_1 */

    /* VL14_0 */
  /*202*/  {/*baseAddr*/ 0x1970,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL14_0 */

    /* VL14_1 */
  /*203*/  {/*baseAddr*/ 0x1974,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL14_1 */

    /* VL15_0 */
  /*204*/  {/*baseAddr*/ 0x1978,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL15_0 */

    /* VL15_1 */
  /*205*/  {/*baseAddr*/ 0x197c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL15_1 */

    /* VL16_0 */
  /*206*/  {/*baseAddr*/ 0x1980,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL16_0 */

    /* VL16_1 */
  /*207*/  {/*baseAddr*/ 0x1984,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL16_1 */

    /* VL17_0 */
  /*208*/  {/*baseAddr*/ 0x1988,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL17_0 */

    /* VL17_1 */
  /*209*/  {/*baseAddr*/ 0x198c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL17_1 */

    /* VL18_0 */
  /*210*/  {/*baseAddr*/ 0x1990,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL18_0 */

    /* VL18_1 */
  /*211*/  {/*baseAddr*/ 0x1994,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL18_1 */

    /* VL19_0 */
  /*212*/  {/*baseAddr*/ 0x1998,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*VL19_0 */

    /* VL19_1 */
  /*213*/  {/*baseAddr*/ 0x199c,  /*offsetFormula*/ 0x2000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*VL19_1 */
};

