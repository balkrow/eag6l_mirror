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
* @file mvHwsAasMac400Units.c
*
* @brief AAS MTI MAC Global EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasMac400Db[] = {
    /* REVISION */
  /*0*/  {/*baseAddr*/ 0x9000,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CORE_REVISION */
  /*1*/  {/*baseAddr*/ 0x9000,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CORE_VERSION */
  /*2*/  {/*baseAddr*/ 0x9000,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CUSTOMER_REVISION */

    /* SCRATCH */
  /*3*/  {/*baseAddr*/ 0x9004,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SCRATCH */

    /* COMMAND_CONFIG */
  /*4*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*5*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*6*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PROMIS_EN */
  /*7*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*8*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*9*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*10*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*11*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*12*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*13*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*14*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CMD_FRAME_ENA */
  /*15*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*16*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*17*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*NO_LGTH_CHECK */
  /*18*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*19*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*20*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*21*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*22*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*23*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*24*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*SHORT_PREAMBLE */
  /*25*/  {/*baseAddr*/ 0x9008,  /*offsetFormula*/ 0x0, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*INV_LOOP */

    /* MAC_ADDR_0 */
  /*26*/  {/*baseAddr*/ 0x900c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* MAC_ADDR_1 */
  /*27*/  {/*baseAddr*/ 0x9010,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* FRM_LENGTH */
  /*28*/  {/*baseAddr*/ 0x9014,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*29*/  {/*baseAddr*/ 0x9014,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*FRM_LENGTH_TX_MTU */

    /* COMMAND_CONFIG_2 */
  /*30*/  {/*baseAddr*/ 0x9018,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*NO_TXFIFO_CLEAR_ON_LINK_FAIL */
  /*31*/  {/*baseAddr*/ 0x9018,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 3 }, /*DATA_FIFO_AF_THRESHOLD */
  /*32*/  {/*baseAddr*/ 0x9018,  /*offsetFormula*/ 0x0, /*fieldStart*/  11, /*fieldLen*/ 3 }, /*CTRL_FIFO_AF_THRESHOLD */
  /*33*/  {/*baseAddr*/ 0x9018,  /*offsetFormula*/ 0x0, /*fieldStart*/  14, /*fieldLen*/ 3 }, /*EOP_STATUS_FIFO_AF_THRESHOLD */

    /* RX_FIFO_SECTIONS */
  /*34*/  {/*baseAddr*/ 0x901c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_SECTION_AVAIL */
  /*35*/  {/*baseAddr*/ 0x901c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*RX_SECTION_EMPTY */

    /* TX_FIFO_SECTIONS */
  /*36*/  {/*baseAddr*/ 0x9020,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TX_SECTION_AVAIL_THRESHOLD */
  /*37*/  {/*baseAddr*/ 0x9020,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_SECTION_EMPTY_THRESHOLD */

    /* HASHTABLE_LOAD */
  /*38*/  {/*baseAddr*/ 0x902c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*HASH_TABLE_ADDRESS */
  /*39*/  {/*baseAddr*/ 0x902c,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*ENABLE_MULTICAST_FRAME */

    /* STATUS */
  /*40*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*41*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*42*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*43*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TS_AVAIL */
  /*44*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*TX_EMPTY */
  /*45*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RX_EMPTY */
  /*46*/  {/*baseAddr*/ 0x9040,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_ISIDLE */

    /* TX_IPG_LENGTH */
  /*47*/  {/*baseAddr*/ 0x9044,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*TXIPG */
  /*48*/  {/*baseAddr*/ 0x9044,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*MAC400_SHORT_IPG */
  /*49*/  {/*baseAddr*/ 0x9044,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 13 }, /*TXIPGCOMPENSATION */

    /* CL01_PAUSE_QUANTA */
  /*50*/  {/*baseAddr*/ 0x9054,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*51*/  {/*baseAddr*/ 0x9054,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* CL23_PAUSE_QUANTA */
  /*52*/  {/*baseAddr*/ 0x9058,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*53*/  {/*baseAddr*/ 0x9058,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* CL45_PAUSE_QUANTA */
  /*54*/  {/*baseAddr*/ 0x905c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*55*/  {/*baseAddr*/ 0x905c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* CL67_PAUSE_QUANTA */
  /*56*/  {/*baseAddr*/ 0x9060,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*57*/  {/*baseAddr*/ 0x9060,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* CL01_QUANTA_THRESH */
  /*58*/  {/*baseAddr*/ 0x9064,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*59*/  {/*baseAddr*/ 0x9064,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* CL23_QUANTA_THRESH */
  /*60*/  {/*baseAddr*/ 0x9068,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*61*/  {/*baseAddr*/ 0x9068,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* CL45_QUANTA_THRESH */
  /*62*/  {/*baseAddr*/ 0x906c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*63*/  {/*baseAddr*/ 0x906c,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* CL67_QUANTA_THRESH */
  /*64*/  {/*baseAddr*/ 0x9070,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*65*/  {/*baseAddr*/ 0x9070,  /*offsetFormula*/ 0x0, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* RX_PAUSE_STATUS */
  /*66*/  {/*baseAddr*/ 0x9074,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*RX_PAUSE_STATUS */

    /* TS_TIMESTAMP */
  /*67*/  {/*baseAddr*/ 0x907c,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TS_TIMESTAMP */

    /* XIF_MODE */
  /*68*/  {/*baseAddr*/ 0x9080,  /*offsetFormula*/ 0x0, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONE_STEP_ENA */
  /*69*/  {/*baseAddr*/ 0x9080,  /*offsetFormula*/ 0x0, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PFC_PULSE_MODE */
  /*70*/  {/*baseAddr*/ 0x9080,  /*offsetFormula*/ 0x0, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PFC_LP_MODE */
  /*71*/  {/*baseAddr*/ 0x9080,  /*offsetFormula*/ 0x0, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_LP_16PRI */

};

