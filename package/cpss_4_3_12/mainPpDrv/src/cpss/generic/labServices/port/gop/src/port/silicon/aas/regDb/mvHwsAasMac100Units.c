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
* @file mvHwsAasMac100Units.c
*
* @brief AAS MTI MAC Global EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasMac100Db[] = {
    /* REVISION */
  /*0*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*CORE_REVISION */
  /*1*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*CORE_VERSION */
  /*2*/  {/*baseAddr*/ 0x1000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CUSTOMER_REVISION */

    /* SCRATCH */
  /*3*/  {/*baseAddr*/ 0x1004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*SCRATCH */

    /* COMMAND_CONFIG */
  /*4*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*5*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*6*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*CRC_FWD */
  /*7*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*8*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*9*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*10*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LOOPBACK_EN */
  /*11*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*TX_PAD_EN */
  /*12*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*13*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CNTL_FRAME_ENA */
  /*14*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PHY_TXENA */
  /*15*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*SEND_IDLE */
  /*16*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*PMAC_PAUSE_MASK_P */
  /*17*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*PMAC_PAUSE_MASK_E */
  /*18*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*19*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*20*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*RX_SFD_ANY */
  /*21*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*22*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TX_LOWP_ENA */
  /*23*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*REG_LOWP_RXEMPTY */
  /*24*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*FLT_TX_STOP */
  /*25*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*26*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 27, /*fieldLen*/ 1 }, /*FLT_HDL_DIS */
  /*27*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*PMAC_TX_PAUSE_DIS */
  /*28*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*PMAC_RX_PAUSE_DIS */
  /*29*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 30, /*fieldLen*/ 1 }, /*SHORT_PREAMBLE */
  /*30*/  {/*baseAddr*/ 0x1008,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*NO_PREAMBLE */

    /* PMAC_MAC_ADDR_0 */
  /*31*/  {/*baseAddr*/ 0x100c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* PMAC_MAC_ADDR_1 */
  /*32*/  {/*baseAddr*/ 0x1010,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* PMAC_FRM_LENGTH */
  /*33*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*34*/  {/*baseAddr*/ 0x1014,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* COMMAND_CONFIG_2 */
  /*35*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*NO_TXFIFO_CLEAR_ON_LINK_FAIL */
  /*36*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 3 }, /*DATA_FIFO_AF_THRESHOLD */
  /*37*/  {/*baseAddr*/ 0x1018,  /*offsetFormula*/ 0x1000, /*fieldStart*/  11, /*fieldLen*/ 3 }, /*CTRL_FIFO_AF_THRESHOLD */

    /* PMAC_RX_FIFO_SECTIONS */
  /*38*/  {/*baseAddr*/ 0x101c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*RX_SECTION_FULL */
  /*39*/  {/*baseAddr*/ 0x101c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*RX_SECTION_EMPTY */

    /* PMAC_TX_FIFO_SECTIONS */
  /*40*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_SECTION_FULL */
  /*41*/  {/*baseAddr*/ 0x1020,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*TX_SECTION_EMPTY */

    /* STATUS */
  /*42*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RX_LOC_FAULT */
  /*43*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_REM_FAULT */
  /*44*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PHY_LOS */
  /*45*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TS_AVAIL */
  /*46*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_LOWP */
  /*47*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PMAC_TX_EMPTY */
  /*48*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_RX_EMPTY */
  /*49*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RX_LINT_FAULT */
  /*50*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_IS_IDLE */
  /*51*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_MERGE_ISIDLE */
  /*52*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*EMAC_TX_EMPTY */
  /*53*/  {/*baseAddr*/ 0x1040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*EMAC_RX_EMPTY */

    /* TX_IPG_LENGTH */
  /*54*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*ENABLE_SHORT_IPG */
  /*55*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*CMP_ALLOW_SHORT_IPG */
  /*56*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TXIPG_RESERVED2 */
  /*57*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*TXIPG */
  /*58*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*COMPENSATION_HI */
  /*59*/  {/*baseAddr*/ 0x1044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*COMPENSATION */

    /* CRC_MODE */
  /*60*/  {/*baseAddr*/ 0x1048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*DISABLE_RX_CRC_CHECK */
  /*61*/  {/*baseAddr*/ 0x1048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*CRC_1BYTE */
  /*62*/  {/*baseAddr*/ 0x1048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*CRC_2BYTE */
  /*63*/  {/*baseAddr*/ 0x1048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*CRC_0BYTE */
  /*64*/  {/*baseAddr*/ 0x1048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*CRC_INV */

    /* TS_TIMESTAMP */
  /*65*/  {/*baseAddr*/ 0x107c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TS_TIMESTAMP */

    /* XIF_MODE */
  /*66*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XGMII */
  /*67*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PAUSETIMERX8 */
  /*68*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*ONESTEPENA */
  /*69*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*EMAC_RX_PAUSE_BYPASS */
  /*70*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_RX_PAUSE_BYPASS */
  /*71*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_MAC_RS_ERR */
  /*72*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*CFG_1STEP_DELTA_MODE */
  /*73*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*CFG_1STEP_DELAY_MODE */
  /*74*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*CFG_1STEP_BINARY_MODE */
  /*75*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*CFG_1STEP_64UPD_MODE */
  /*76*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*CFG_PFC_PULSE_MODE */
  /*77*/  {/*baseAddr*/ 0x1080,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TS_SFD_ENA */

    /* BR_CONTROL */
  /*78*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_PREEMPT_EN */
  /*79*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*TX_VERIF_DIS */
  /*80*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*TX_ADDFRAGSIZE */
  /*81*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 7 }, /*TX_VERIFY_TIME */
  /*82*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*TX_PAUSE_PRI_EN */
  /*83*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*TX_FRAG_PAUSE_EN */
  /*84*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*TX_NON_EMPTY_PREEMPT_EN */
  /*85*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*TX_ALLOW_PMAC_IF_NVERIF */
  /*86*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TX_ALLOW_EMAC_IF_NVERIF */
  /*87*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*BR_COUNT_CLR_ON_RD */
  /*88*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*BR_COUNT_SAT */
  /*89*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*RX_STRICT_PREAMBLE */
  /*90*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PMAC_TO_EMAC_STATS */
  /*91*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*BR_RX_SMD_DIS */
  /*92*/  {/*baseAddr*/ 0x10a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*RX_BR_STRICT_FRM_ENA */

    /* BR_STATUS */
  /*93*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*TX_VERIFY_STATUS */
  /*94*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TX_PREEMPT_STATUS */
  /*95*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*ASSY_ERR_LH */
  /*96*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SMD_ERR_LH */
  /*97*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*VERIF_SEEN_LH */
  /*98*/  {/*baseAddr*/ 0x10ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RESP_SEEN_LH */

    /* BR_TX_FRAG_COUNT */
  /*99*/  {/*baseAddr*/ 0x10b0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TX_FRAG_COUNT */

    /* BR_RX_FRAG_COUNT */
  /*100*/  {/*baseAddr*/ 0x10b8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RX_FRAG_COUNT */

    /* BR_TX_HOLD_COUNT */
  /*101*/  {/*baseAddr*/ 0x10c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TX_HOLD_COUNT */

    /* BR_RX_SMD_ERR_COUNT */
  /*102*/  {/*baseAddr*/ 0x10c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_SMD_ERR_COUNT */

    /* BR_RX_ASSY_ERR_COUNT */
  /*103*/  {/*baseAddr*/ 0x10d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_ASSY_ERR_COUNT */

    /* BR_RX_ASSY_OK_COUNT */
  /*104*/  {/*baseAddr*/ 0x10d8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*RX_ASSY_OK_COUNT */

    /* BR_RX_VERIFY_COUNT */
  /*105*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_VERIFY_COUNT_GOOD */
  /*106*/  {/*baseAddr*/ 0x10e0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*RX_VERIFY_COUNT_BAD */

    /* BR_RX_RESPONSE_COUNT */
  /*107*/  {/*baseAddr*/ 0x10e4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*RX_RESP_COUNT_GOOD */
  /*108*/  {/*baseAddr*/ 0x10e4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*RX_RESP_COUNT_BAD */

    /* BR_TX_VERIF_COUNT */
  /*109*/  {/*baseAddr*/ 0x10e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*TX_VERIF_COUNT */
  /*110*/  {/*baseAddr*/ 0x10e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 8 }, /*TX_RESP_COUNT */

    /* EMAC_COMMAND_CONFIG */
  /*111*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_ENA */
  /*112*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_ENA */
  /*113*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PAUSE_FWD */
  /*114*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PAUSE_IGNORE */
  /*115*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*TX_ADDR_INS */
  /*116*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*SW_RESET */
  /*117*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CNTL_FRAME_ENA */
  /*118*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*EMAC_PAUSE_MASK_P */
  /*119*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*EMAC_PAUSE_MASK_E */
  /*120*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*121*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*PAUSE_PFC_COMP */
  /*122*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_FLUSH */
  /*123*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 26, /*fieldLen*/ 1 }, /*TX_FIFO_RESET */
  /*124*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*EMAC_TX_PAUSE_DIS */
  /*125*/  {/*baseAddr*/ 0x1108,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 29, /*fieldLen*/ 1 }, /*EMAC_RX_PAUSE_DIS */

    /* EMAC_MAC_ADDR_0 */
  /*126*/  {/*baseAddr*/ 0x110c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*MAC_ADDRESS_0 */

    /* EMAC_MAC_ADDR_1 */
  /*127*/  {/*baseAddr*/ 0x1110,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*MAC_ADDRESS_1 */

    /* EMAC_FRM_LENGTH */
  /*128*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*FRM_LENGTH */
  /*129*/  {/*baseAddr*/ 0x1114,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*TX_MTU */

    /* EMAC_RX_FIFO_SECTIONS */
  /*130*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*RX_SECTION_FULL */
  /*131*/  {/*baseAddr*/ 0x111c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*RX_SECTION_EMPTY */

    /* EMAC_TX_FIFO_SECTIONS */
  /*132*/  {/*baseAddr*/ 0x1120,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TX_SECTION_FULL */
  /*133*/  {/*baseAddr*/ 0x1120,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 4 }, /*TX_SECTION_EMPTY */

    /* EMAC_CL01_PAUSE_QUANTA */
  /*134*/  {/*baseAddr*/ 0x1154,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_PAUSE_QUANTA */
  /*135*/  {/*baseAddr*/ 0x1154,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_PAUSE_QUANTA */

    /* EMAC_CL23_PAUSE_QUANTA */
  /*136*/  {/*baseAddr*/ 0x1158,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_PAUSE_QUANTA */
  /*137*/  {/*baseAddr*/ 0x1158,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_PAUSE_QUANTA */

    /* EMAC_CL45_PAUSE_QUANTA */
  /*138*/  {/*baseAddr*/ 0x115c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_PAUSE_QUANTA */
  /*139*/  {/*baseAddr*/ 0x115c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_PAUSE_QUANTA */

    /* EMAC_CL67_PAUSE_QUANTA */
  /*140*/  {/*baseAddr*/ 0x1160,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_PAUSE_QUANTA */
  /*141*/  {/*baseAddr*/ 0x1160,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_PAUSE_QUANTA */

    /* EMAC_CL01_QUANTA_THRESH */
  /*142*/  {/*baseAddr*/ 0x1164,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL0_QUANTA_THRESH */
  /*143*/  {/*baseAddr*/ 0x1164,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL1_QUANTA_THRESH */

    /* EMAC_CL23_QUANTA_THRESH */
  /*144*/  {/*baseAddr*/ 0x1168,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL2_QUANTA_THRESH */
  /*145*/  {/*baseAddr*/ 0x1168,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL3_QUANTA_THRESH */

    /* EMAC_CL45_QUANTA_THRESH */
  /*146*/  {/*baseAddr*/ 0x116c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL4_QUANTA_THRESH */
  /*147*/  {/*baseAddr*/ 0x116c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL5_QUANTA_THRESH */

    /* EMAC_CL67_QUANTA_THRESH */
  /*148*/  {/*baseAddr*/ 0x1170,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*CL6_QUANTA_THRESH */
  /*149*/  {/*baseAddr*/ 0x1170,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 16, /*fieldLen*/ 16 }, /*CL7_QUANTA_THRESH */

    /* EMAC_RX_PAUSE_STATUS */
  /*150*/  {/*baseAddr*/ 0x1174,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*EMAC_RX_PAUSE_STATUS */

};

